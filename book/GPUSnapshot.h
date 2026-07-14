#pragma once
#include <cstdint>
struct GPUPriceLevel {
    uint64_t price;
    uint64_t quantity;
};

struct GPUBookSnapshot {
    GPUPriceLevel bids[50];
    GPUPriceLevel asks[50];
    int bid_levels;
    int ask_levels;
};