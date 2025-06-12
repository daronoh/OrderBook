#pragma once

#include <chrono>
#include <limits>

#include "Usings.h"

struct Constants {
  static const Price InvalidPrice = std::numeric_limits<Price>::quiet_NaN();
  static constexpr auto EASTERN_OFFSET_EST = std::chrono::hours(-5);
  static constexpr auto EASTERN_OFFSET_EDT = std::chrono::hours(-4);
  static constexpr auto MARKET_CLOSE_HOUR = std::chrono::hours(16);
};