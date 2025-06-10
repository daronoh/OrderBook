#pragma once

#include "PriceLevelInfo.h"

class OrderbookPriceLevelInfos {
public:
  OrderbookPriceLevelInfos(const PriceLevelInfos &bids,
                           const PriceLevelInfos &asks)
      : bids_{bids}, asks_{asks} {}

  const PriceLevelInfos &GetBids() const { return bids_; }
  const PriceLevelInfos &GetAsks() const { return asks_; }

private:
  PriceLevelInfos bids_;
  PriceLevelInfos asks_;
};
