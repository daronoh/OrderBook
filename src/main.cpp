#include <iostream>

#include "OrderBook.h"

int main() {
  Orderbook orderbook;
  OrderId orderId = 1;
  orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId,
                                             Side::Buy, 100, 10));
  orderbook.AddOrder(std::make_shared<Order>(OrderType::FillOrKill, ++orderId,
                                             Side::Sell, 100, 15));
  std::cout << "After executing order: " << orderbook.Size() << std::endl;
  return 0;
}