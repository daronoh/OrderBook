#include "OrderBook.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <tuple>
#include <variant>

Trades Orderbook::AddOrder(OrderPointer order) {
  if (orders_.contains(order->GetOrderId()))
    return {};

  if (order->GetOrderType() == OrderType::FillAndKill &&
      !CanMatch(order->GetSide(), order->GetPrice()))
    return {};

  OrderPointers::iterator iter;

  if (order->GetSide() == Side::Buy) {
    auto &bidLevel = bids_[order->GetPrice()];
    bidLevel.push_back(order);
    iter = std::next(bidLevel.begin(), bidLevel.size() - 1);
  } else {
    auto &askLevel = asks_[order->GetPrice()];
    askLevel.push_back(order);
    iter = std::next(askLevel.begin(), askLevel.size() - 1);
  }

  orders_.insert({order->GetOrderId(), OrderEntry{order, iter}});
  return MatchOrders();
}

void Orderbook::CancelOrder(OrderId orderId) {
  if (!orders_.contains(orderId))
    return;

  const auto &[order, orderIter] = orders_.at(orderId);
  orders_.erase(orderId);

  if (order->GetSide() == Side::Buy) {
    auto price = order->GetPrice();
    auto &bidLevel = bids_.at(price);
    bidLevel.erase(orderIter);

    if (bidLevel.empty())
      bids_.erase(price);
  } else {
    auto price = order->GetPrice();
    auto &askLevel = asks_.at(price);
    askLevel.erase(orderIter);
    if (askLevel.empty())
      asks_.erase(price);
  }
}

Trades Orderbook::ModifyOrder(OrderModify order) {
  if (!orders_.contains(order.GetOrderId()))
    return {};

  const auto &[existingOrder, _] = orders_.at(order.GetOrderId());
  CancelOrder(order.GetOrderId());
  return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType(),
                                       existingOrder->GetSide()));
}

std::size_t Orderbook::Size() const { return orders_.size(); }

OrderbookPriceLevelInfos Orderbook::GetOrderInfos() const {
  PriceLevelInfos bidInfos, askInfos;
  bidInfos.reserve(orders_.size());
  askInfos.reserve(orders_.size());

  auto CreateLevelInfos = [](Price price, const OrderPointers &orders) {
    return PriceLevelInfo{
        price, std::accumulate(
                   orders.begin(), orders.end(), static_cast<Quantity>(0),
                   [](std::size_t runningSum, const OrderPointer &order) {
                     return runningSum + order->GetRemainingQuantity();
                   })};
  };

  for (const auto &[price, orders] : bids_)
    bidInfos.push_back(CreateLevelInfos(price, orders));

  for (const auto &[price, orders] : asks_)
    askInfos.push_back(CreateLevelInfos(price, orders));

  return OrderbookPriceLevelInfos{bidInfos, askInfos};
}

bool Orderbook::CanMatch(Side side, Price price) const {
  if (side == Side::Buy) {
    if (asks_.empty())
      return false;

    const auto &[bestAsk, _] = *asks_.begin();
    return price >= bestAsk;
  } else {
    if (bids_.empty())
      return false;

    const auto &[bestBid, _] = *bids_.begin();
    return price <= bestBid;
  }
}

Trades Orderbook::MatchOrders() {
  Trades trades;
  trades.reserve(orders_.size());

  while (true) {
    if (bids_.empty() || asks_.empty())
      break; // one side is empty, cannot match

    auto &[bidPrice, levelBids] = *bids_.begin();
    auto &[askPrice, levelAsks] = *asks_.begin();

    if (bidPrice < askPrice)
      break; // best bid cannot match best ask

    while (levelBids.size() && levelAsks.size()) {
      auto &bid = levelBids.front();
      auto &ask = levelAsks.front();

      Quantity tradeQuantity =
          std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

      bid->Fill(tradeQuantity);
      ask->Fill(tradeQuantity);

      if (bid->IsFilled()) {
        // one bid in the current level is filled
        levelBids.pop_front();
        orders_.erase(bid->GetOrderId());
      }

      if (ask->IsFilled()) {
        // one ask in the current level is filled
        levelAsks.pop_front();
        orders_.erase(ask->GetOrderId());
      }

      if (levelBids.empty())
        bids_.erase(bidPrice); // entire level of bids is filled

      if (levelAsks.empty())
        asks_.erase(askPrice); // entire level of asks is filled

      trades.emplace_back(TradeInfo{bid->GetOrderId(), bid->GetPrice()},
                          TradeInfo{ask->GetOrderId(), ask->GetPrice()},
                          tradeQuantity);
    }
  }

  // for FillAndKill orders
  if (!bids_.empty()) {
    auto &[_, bids] = *bids_.begin();
    auto &order = bids.front();
    if (order->GetOrderType() == OrderType::FillAndKill)
      CancelOrder(order->GetOrderId());
  }

  if (!asks_.empty()) {
    auto &[_, asks] = *asks_.begin();
    auto &order = asks.front();
    if (order->GetOrderType() == OrderType::FillAndKill)
      CancelOrder(order->GetOrderId());
  }

  return trades;
}