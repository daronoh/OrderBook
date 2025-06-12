#pragma once

#include <condition_variable>
#include <map>
#include <thread>
#include <unordered_map>

#include "Order.h"
#include "OrderModify.h"
#include "OrderbookPriceLevelInfos.h"
#include "Trade.h"
#include "Usings.h"

class Orderbook {
public:
  Orderbook();
  ~Orderbook();

  Trades AddOrder(OrderPointer order);
  void CancelOrder(OrderId orderId);
  Trades ModifyOrder(const OrderModify &order);

  std::size_t Size() const;
  OrderbookPriceLevelInfos GetOrderInfos() const;

private:
  // representation of order and location in orderbook
  struct OrderEntry {
    OrderPointer order_{nullptr};
    OrderPointers::iterator location_;
  };

  // for book-keeping
  struct LevelData {
    Quantity quantity_{};
    Quantity count_{};

    enum class Action {
      Add,
      Remove,
      Match,
    };
  };

  // no need to specify side for data_ because guaranteed to have bids lower
  // than asks if they exist, otherwise it wouldve been matched already
  std::unordered_map<Price, LevelData> data_;
  std::map<Price, OrderPointers, std::greater<Price>> bids_;
  std::map<Price, OrderPointers, std::less<Price>> asks_;
  std::unordered_map<OrderId, OrderEntry> orders_;
  mutable std::mutex ordersMutex_;
  std::thread ordersPruneThread_;
  std::condition_variable shutdownConditionVariable_;
  std::atomic<bool> shutdown_{false};

  void PruneGoodForDayOrders();

  void CancelOrders(const OrderIds &orderIds);
  void CancelOrderInternal(OrderId orderId);

  bool CanMatch(Side side, Price price) const;
  bool CanFullyFill(Side side, Price price, Quantity) const;
  Trades MatchOrders();

  void OnOrderCancelled(OrderPointer order);
  void OnOrderAdded(OrderPointer order);
  void OnOrderMatched(Price price, Quantity quantity, bool isFullyFilled);
  void UpdateLevelData(Price price, Quantity quantity,
                       LevelData::Action action);
};