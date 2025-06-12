#include "Order.h"

#include <cmath>
#include <exception>
#include <format>

Quantity Order::GetFilledQuantity() const {
  return GetInitialQuantity() - GetRemainingQuantity();
}

void Order::Fill(Quantity quantity) {
  if (quantity > GetRemainingQuantity())
    throw std::logic_error(std::format(
        "Order ({}) cannot be filled for more than its remaining quantity.",
        GetOrderId()));

  remainingQuantity_ -= quantity;
}

bool Order::IsFilled() const { return GetRemainingQuantity() == 0; }

void Order::ToFillAndKill(Price price) {
  if (GetOrderType() != OrderType::Market)
    throw std::logic_error(std::format(
        "Order ({}) cannot have its price adjusted, only market orders can.",
        GetOrderId()));

  if (!std::isfinite(price))
    throw std::logic_error(
        std::format("Order ({}) must be a tradable price.", GetOrderId()));

  price_ = price;
  orderType_ = OrderType::FillAndKill;
}