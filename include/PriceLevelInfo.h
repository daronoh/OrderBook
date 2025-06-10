#pragma once

#include "Usings.h"

struct PriceLevelInfo
{
    Price price_;
    Quantity quantity_;
};

using PriceLevelInfos = std::vector<PriceLevelInfo>;