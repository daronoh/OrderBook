#pragma once

#include "TradeInfo.h"

class Trade {
public:
  Trade(const TradeInfo &tradeBid, const TradeInfo &tradeAsk, Quantity quantity)
      : tradeBid_{tradeBid}, tradeAsk_{tradeAsk}, quantity_{quantity} {}

  const TradeInfo &GetTradeBid() const { return tradeBid_; }
  const TradeInfo &GetTradeAsk() const { return tradeAsk_; }
  Quantity GetQuantity() const { return quantity_; }

private:
  TradeInfo tradeBid_;
  TradeInfo tradeAsk_;
  Quantity quantity_;
};

using Trades = std::vector<Trade>;
