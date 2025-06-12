#pragma once

#include <list>
#include <memory>

#include "Constants.h"
#include "OrderType.h"
#include "Side.h"
#include "Usings.h"

class Order {
public:
  Order(OrderType orderType, OrderId orderId, Side side, Price price,
        Quantity quantity)
      : orderType_{orderType}, orderId_{orderId}, side_{side}, price_{price},
        initialQuantity_{quantity}, remainingQuantity_{quantity} {}

  Order(OrderId orderId, Side side, Quantity quantity)
      : Order(OrderType::Market, orderId, side, Constants::InvalidPrice,
              quantity) {}

  OrderId GetOrderId() const { return orderId_; }
  Side GetSide() const { return side_; }
  Price GetPrice() const { return price_; }
  OrderType GetOrderType() const { return orderType_; }
  Quantity GetInitialQuantity() const { return initialQuantity_; }
  Quantity GetRemainingQuantity() const { return remainingQuantity_; }
  Quantity GetFilledQuantity() const;
  bool IsFilled() const;
  void Fill(Quantity quantity);
	void ToFillAndKill(Price price);

private:
  OrderType orderType_;
  OrderId orderId_;
  Side side_;
  Price price_;
  Quantity initialQuantity_;
  Quantity remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;
// look into using vector + flag for better performance
