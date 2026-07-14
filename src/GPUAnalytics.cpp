#include "book/GPUAnalytics.h"
#include <cuda_runtime.h>
#include <stdexcept>
#include <iostream>

// Forward declare the CUDA kernel
extern "C" void simulateMarketImpact(
    const GPUPriceLevel*, int, const GPUPriceLevel*, int,
    const uint64_t*, double*, int, bool);

GPUAnalytics::GPUAnalytics(int max_simulations) : max_simulations(max_simulations) {
    // Pre-allocate GPU memory to avoid allocation latency during runtime
    cudaMalloc(&d_bids, 50 * sizeof(GPUPriceLevel));
    cudaMalloc(&d_asks, 50 * sizeof(GPUPriceLevel));
    cudaMalloc(&d_sizes, max_simulations * sizeof(uint64_t));
    cudaMalloc(&d_results, max_simulations * sizeof(double));
}

GPUAnalytics::~GPUAnalytics() {
    cudaFree(d_bids);
    cudaFree(d_asks);
    cudaFree(d_sizes);
    cudaFree(d_results);
}

std::vector<double> GPUAnalytics::runStressTest(const GPUBookSnapshot& snapshot,
                                                const std::vector<uint64_t>& simulated_sizes,
                                                bool is_buy) {
    int num_sims = simulated_sizes.size();
    if (num_sims > max_simulations) throw std::runtime_error("Exceeded max GPU simulations");

    // 1. Copy Order Book Snapshot to GPU (Async for lower latency)
    cudaMemcpy(d_bids, snapshot.bids, snapshot.bid_levels * sizeof(GPUPriceLevel), cudaMemcpyHostToDevice);
    cudaMemcpy(d_asks, snapshot.asks, snapshot.ask_levels * sizeof(GPUPriceLevel), cudaMemcpyHostToDevice);
    cudaMemcpy(d_sizes, simulated_sizes.data(), num_sims * sizeof(uint64_t), cudaMemcpyHostToDevice);

    // 2. Launch Kernel
    int threadsPerBlock = 256;
    int blocksPerGrid = (num_sims + threadsPerBlock - 1) / threadsPerBlock;
    
    simulateMarketImpact<<<blocksPerGrid, threadsPerBlock>>>(
        d_bids, snapshot.bid_levels, 
        d_asks, snapshot.ask_levels, 
        d_sizes, d_results, 
        num_sims, is_buy
    );

    // 3. Copy Results back to CPU
    std::vector<double> results(num_sims);
    cudaMemcpy(results.data(), d_results, num_sims * sizeof(double), cudaMemcpyDeviceToHost);

    return results;
}