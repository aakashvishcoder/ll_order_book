#pragma once
#include "GPUSnapshot.h"
#include <vector>
#include <cstdint>

class GPUAnalytics {
public:
    GPUAnalytics(int max_simulations);
    ~GPUAnalytics();

    std::vector<double> runStressTest(const GPUBookSnapshot& snapshot, const std::vector<uint64_t>& simulated_sizes, bool is_buy);
private:
    GPUPriceLevel* d_bids;
    GPUPriceLevel* d_asks;
    uint64_t* d_sizes;
    double* d_results;

    int max_simulations;
};