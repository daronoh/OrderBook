#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

enum class OrderType { GoodTillCancel, FillAndKill };

enum class Side { Buy, Sell };

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

// Price Level: a specific price point at which one or more buy or sell orders
// are placed.
struct PriceLevelInfo {
  Price price_;
  Quantity quantity_;
};

using PriceLevelInfos = std::vector<PriceLevelInfo>;

class OrderbookPriceLevelInfos {
public:
  OrderbookPriceLevelInfos(const PriceLevelInfos &bids,
                           const PriceLevelInfos &asks);

  const PriceLevelInfos &GetBids() const;
  const PriceLevelInfos &GetAsks() const;

private:
  PriceLevelInfos bids_;
  PriceLevelInfos asks_;
};

// Order objects that make up the orderbook
class Order {
public:
  Order(OrderType orderType, OrderId orderId, Side side, Price price,
        Quantity quantity);

  OrderId GetOrderId() const;
  Side GetSide() const;
  Price GetPrice() const;
  OrderType GetOrderType() const;
  Quantity GetInitialQuantity() const;
  Quantity GetRemainingQuantity() const;
  Quantity GetFilledQuantity() const;
  void Fill(Quantity quantity);
  bool IsFilled() const;

private:
  OrderType orderType_;
  OrderId orderId_;
  Side side_;
  Price price_;
  Quantity initialQuantity_;
  Quantity remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>; // look into using vector + flag
                                               // for better performance

// representation of an order that is to be modified
class OrderModify {
public:
  OrderModify(OrderId orderId, Price price, Quantity quantity);

  OrderId GetOrderId() const;
  Price GetPrice() const;
  Quantity GetQuantity() const;

  OrderPointer ToOrderPointer(OrderType type, Side side) const;

private:
  OrderId orderId_;
  Price price_;
  Quantity quantity_;
};

// information regarding a particular side of a trade
struct TradeInfo {
  OrderId orderId_;
  Price price_;
};

// Trade: A matched order
class Trade {
public:
  Trade(const TradeInfo &tradeBid, const TradeInfo &tradeAsk, Quantity quantity);

  const TradeInfo &GetTradeBid() const;
  const TradeInfo &GetTradeAsk() const;
  Quantity GetQuantity() const;

private:
  TradeInfo tradeBid_;
  TradeInfo tradeAsk_;
  Quantity quantity_;
};

using Trades = std::vector<Trade>;

class Orderbook {
public:
  Trades AddOrder(OrderPointer order);
  void CancelOrder(OrderId orderId);
  Trades ModifyOrder(OrderModify order);
  std::size_t Size() const;
  OrderbookPriceLevelInfos GetOrderInfos() const;

private:
  // representation of order and location in orderbook
  struct OrderEntry {
    OrderPointer order_{nullptr};
    OrderPointers::iterator location_;
  };

  std::map<Price, OrderPointers, std::greater<Price>> bids_; // list of bids
  std::map<Price, OrderPointers, std::less<Price>> asks_;    // list of asks
  std::unordered_map<OrderId, OrderEntry> orders_;

  bool CanMatch(Side side, Price price) const;
  Trades MatchOrders();
};