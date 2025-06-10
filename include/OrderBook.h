#pragma once

#include <map>
#include <unordered_map>

#include "Order.h"
#include "OrderModify.h"
#include "OrderbookPriceLevelInfos.h"
#include "Trade.h"
#include "Usings.h"

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