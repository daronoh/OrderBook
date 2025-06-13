#include "OrderBook.h"

#include <mutex>
#include <numeric>
#include <optional>

Orderbook::Orderbook()
    : ordersPruneThread_{[this]() { PruneGoodForDayOrders(); }} {}

Orderbook::~Orderbook() {
  // ensures proper cleaning up of the thread on shutdown
  shutdown_.store(true, std::memory_order_release); // done with my writes
  shutdownConditionVariable_.notify_one(); // signals to wait_for to wake up
  ordersPruneThread_.join();
}

Trades Orderbook::AddOrder(OrderPointer order) {
  std::scoped_lock ordersLock{ordersMutex_};

  // Order already exists
  if (orders_.contains(order->GetOrderId()))
    return {};

  // Market order turns into a fill and kill of the worst current price
  if (order->GetOrderType() == OrderType::Market) {
    if (order->GetSide() == Side::Buy && !asks_.empty()) {
      const auto &[worstAsk, _] = *asks_.rbegin();
      order->ToFillAndKill(worstAsk);
    } else if (order->GetSide() == Side::Sell && !bids_.empty()) {
      const auto &[worstBid, _] = *bids_.rbegin();
      order->ToFillAndKill(worstBid);
    } else
      return {};
  }

  // Fill And Kill
  if (order->GetOrderType() == OrderType::FillAndKill &&
      !CanMatch(order->GetSide(), order->GetPrice()))
    return {};

  // Fill Or Kill
  if (order->GetOrderType() == OrderType::FillOrKill &&
      !CanFullyFill(order->GetSide(), order->GetPrice(),
                    order->GetInitialQuantity()))
    return {};

  OrderPointers::iterator iter;

  // adding the order into a level in bids_ or asks_
  if (order->GetSide() == Side::Buy) {
    auto &bidLevel = bids_[order->GetPrice()];
    bidLevel.push_back(order);
    iter = std::next(bidLevel.begin(), bidLevel.size() - 1);
  } else {
    auto &askLevel = asks_[order->GetPrice()];
    askLevel.push_back(order);
    iter = std::next(askLevel.begin(), askLevel.size() - 1);
  }

  // adding the order into orders_
  orders_.insert({order->GetOrderId(), OrderEntry{order, iter}});

  OnOrderAdded(order);

  return MatchOrders();
}

void Orderbook::CancelOrders(const OrderIds &orderIds) {
  std::scoped_lock ordersLock{ordersMutex_};

  for (const auto orderId : orderIds)
    CancelOrderInternal(orderId);
}

void Orderbook::CancelOrder(OrderId orderId) {
  std::scoped_lock ordersLock{ordersMutex_};
  CancelOrderInternal(orderId);
}

void Orderbook::CancelOrderInternal(OrderId orderId) {
  if (!orders_.contains(orderId))
    return;

  const auto [order, orderIter] = orders_.at(orderId);
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

  OnOrderCancelled(order);
}

Trades Orderbook::ModifyOrder(const OrderModify &order) {
  OrderType orderType;
  Side side;

  {
    std::scoped_lock ordersLock{ordersMutex_};

    if (!orders_.contains(order.GetOrderId()))
      return {};

    const auto &[existingOrder, _] = orders_.at(order.GetOrderId());
    orderType = existingOrder->GetOrderType();
    side = existingOrder->GetSide();
  }
  CancelOrder(order.GetOrderId());
  return AddOrder(order.ToOrderPointer(orderType, side));
}

void Orderbook::OnOrderAdded(OrderPointer order) {
  UpdateLevelData(order->GetPrice(), order->GetInitialQuantity(),
                  LevelData::Action::Add);
}

void Orderbook::OnOrderCancelled(OrderPointer order) {
  UpdateLevelData(order->GetPrice(), order->GetInitialQuantity(),
                  LevelData::Action::Remove);
}

void Orderbook::OnOrderMatched(Price price, Quantity quantity,
                               bool isFullyFilled) {
  UpdateLevelData(price, quantity,
                  isFullyFilled ? LevelData::Action::Remove
                                : LevelData::Action::Match);
}

void Orderbook::UpdateLevelData(Price price, Quantity quantity,
                                LevelData::Action action) {
  auto &data = data_[price];

  data.count_ += action == LevelData::Action::Remove ? -1
                 : action == LevelData::Action::Add  ? 1
                                                     : 0;
  if (action == LevelData::Action::Remove ||
      action == LevelData::Action::Match) {
    data.quantity_ -= quantity;
  } else {
    data.quantity_ += quantity;
  }

  if (data.count_ == 0)
    data_.erase(price);
}

std::size_t Orderbook::Size() const {
  std::scoped_lock ordersLock{ordersMutex_};
  return orders_.size();
}

OrderbookPriceLevelInfos Orderbook::GetOrderInfos() const {
  PriceLevelInfos bidInfos, askInfos;
  bidInfos.reserve(orders_.size());
  askInfos.reserve(orders_.size());

  auto CreateLevelInfos = [](Price price, const OrderPointers &orders) {
    return PriceLevelInfo{
        price,
        std::accumulate(orders.begin(), orders.end(), static_cast<Quantity>(0),
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

bool Orderbook::CanFullyFill(Side side, Price price, Quantity quantity) const {
  if (!CanMatch(side, price))
    return false;

  std::optional<Price> threshold;

  if (side == Side::Buy) {
    // guaranteed to have at least 1 ask due to a match existing
    const auto [askPrice, _] = *asks_.begin();
    threshold = askPrice;
  } else {
    // guaranteed to have at least 1 buy bid to a match existing
    const auto [bidPrice, _] = *bids_.begin();
    threshold = bidPrice;
  }

  for (const auto &[levelPrice, LevelData] : data_) {
    if (threshold.has_value() &&
        ((side == Side::Buy && threshold.value() > levelPrice) ||
         (side == Side::Sell && threshold.value() < levelPrice)))
      continue;

    if ((side == Side::Buy && levelPrice > price) ||
        (side == Side::Sell && levelPrice < price))
      continue;

    if (quantity <= LevelData.quantity_)
      return true;

    quantity -= LevelData.quantity_;
  }

  return false;
}

Trades Orderbook::MatchOrders() {
  Trades trades;
  trades.reserve(orders_.size());

  while (true) {
    if (bids_.empty() || asks_.empty())
      break; // one side is empty, cannot match

    auto &[bestBid, levelBids] = *bids_.begin();
    auto &[bestAsk, levelAsks] = *asks_.begin();

    if (bestBid < bestAsk)
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

      trades.emplace_back(bid->GetOrderId(), ask->GetOrderId(), tradeQuantity,
                          ask->GetPrice()); // trade done at ask price

      OnOrderMatched(bid->GetPrice(), tradeQuantity, bid->IsFilled());
      OnOrderMatched(ask->GetPrice(), tradeQuantity, ask->IsFilled());
    }

    if (levelBids.empty()) {
      bids_.erase(bestBid); // entire level of bestBid is filled
    }

    if (levelAsks.empty()) {
      asks_.erase(bestAsk); // entire level of bestAsk is filled
    }
  }

  // for FillAndKill orders
  if (!bids_.empty()) {
    auto &[_, bids] = *bids_.begin();
    auto &order = bids.front();
    if (order->GetOrderType() == OrderType::FillAndKill)
      CancelOrderInternal(order->GetOrderId());
  }

  if (!asks_.empty()) {
    auto &[_, asks] = *asks_.begin();
    auto &order = asks.front();
    if (order->GetOrderType() == OrderType::FillAndKill)
      CancelOrderInternal(order->GetOrderId());
  }

  return trades;
}

void Orderbook::PruneGoodForDayOrders() {
  using namespace std::chrono;

  while (true) {
    auto now_eastern =
        system_clock::now() + Constants::EASTERN_OFFSET_EDT; // assume EDT

    auto today_eastern = floor<days>(now_eastern);
    auto market_close_today = today_eastern + Constants::MARKET_CLOSE_HOUR;

    const auto next_end =
        (now_eastern >= market_close_today)
            ? today_eastern + days(1) + Constants::MARKET_CLOSE_HOUR
            : market_close_today;

    const auto till =
        next_end - now_eastern + milliseconds(100); // 100 ms buffer

    {
      // needs unique_lock for wait_for
      std::unique_lock ordersLock{ordersMutex_};

      if (shutdown_.load(std::memory_order_acquire) || // can see your writes
          shutdownConditionVariable_.wait_for(ordersLock, till) ==
              std::cv_status::no_timeout)
        return;
    }

    OrderIds orderIds;

    {
      // scoped_lock lesser overhead compared to unique_lock
      // create lock outside of for loop for performance reasons
      std::scoped_lock ordersLock{ordersMutex_};

      for (const auto &[_, entry] : orders_) {
        const auto &[order, x] = entry;

        if (order->GetOrderType() != OrderType::GoodForDay)
          continue;

        orderIds.push_back(order->GetOrderId());
      }
    }

    CancelOrders(orderIds);
  }
}
