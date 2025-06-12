#pragma once

class Trade {
public:
  Trade(OrderId bidId, OrderId askId, Quantity quantity, Price price)
      : bidId_{bidId}, askId_{askId}, quantity_{quantity}, price_{price} {}

  OrderId GetBidId() const { return bidId_; }
  OrderId GetAskId() const { return askId_; }
  Quantity GetQuantity() const { return quantity_; }
  Price GetPrice() const { return price_; }

private:
  OrderId bidId_;
  OrderId askId_;
  Quantity quantity_;
  Price price_;
};

using Trades = std::vector<Trade>;
