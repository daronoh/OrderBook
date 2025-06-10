#pragma once

#include "Order.h"

class OrderModify {
public:
  OrderModify(OrderId orderId, Price price, Quantity quantity)
      : orderId_{orderId}, price_{price}, quantity_{quantity} {}

  OrderId GetOrderId() const { return orderId_; }
  Price GetPrice() const { return price_; }
  Quantity GetQuantity() const { return quantity_; }
  OrderPointer ToOrderPointer(OrderType type, Side side) const {
    return std::make_shared<Order>(type, GetOrderId(), side, GetPrice(),
                                   GetQuantity());
  }

private:
  OrderId orderId_;
  Price price_;
  Quantity quantity_;
};
