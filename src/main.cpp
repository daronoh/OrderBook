#include <iostream>

#include "OrderBook.h"

int main() {
  Orderbook orderbook;
  OrderId orderId = 1;
  orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId,
                                             Side::Buy, 100, 10));
  std::cout << "After adding buy order: " << orderbook.Size() << std::endl;
  orderbook.CancelOrder(orderId);
  std::cout << "After cancelling buy order: " << orderbook.Size() << std::endl;

  orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel,
                                             ++orderId, Side::Buy, 100, 10));
  orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel,
                                             ++orderId, Side::Sell, 100, 10));
  std::cout << "After executing order: " << orderbook.Size() << std::endl;
  return 0;
}