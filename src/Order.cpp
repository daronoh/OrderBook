#include "Order.h"

Quantity Order::GetFilledQuantity() const {
    return GetInitialQuantity() - GetRemainingQuantity();
  }

void Order::Fill(Quantity quantity) {
    if (quantity > GetRemainingQuantity())
      throw std::logic_error(fmt::format(
          "Order ({}) cannot be filled for more than its remaining quantity.",
          GetOrderId()));

    remainingQuantity_ -= quantity;
  }

bool Order::IsFilled() const { return GetRemainingQuantity() == 0; }