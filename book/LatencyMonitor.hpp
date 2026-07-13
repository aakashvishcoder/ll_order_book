#pragma once
#include <atomic>
#include <chrono>
#include <iostream>
class LatencyMonitor {
public:
    void recordLatency(uint64_t nanos) {
        count_.fetch_add(1, std::memory_order_relaxed);

        uint64_t current_min= min_.load(std::memory_order_relaxed);
        while (nanos < current_min && !min_.compare_exchange_weak(current_min, nanos,std::memory_order_relaxed));
        uint64_t current_max =max_.load(std::memory_order_relaxed);
        while(nanos <current_max&& !max_.compare_exchange_weak(current_max,nanos,std::memory_order_relaxed));


        total_nanos_.fetch_add(nanos,std::memory_order_relaxed);
    }
    void printStatus() const {
        uint64_t count=count_.load();
        if (count==0 ) return;
        std::cout <<"Orders processed: " << count<< "\n"
                << "Min latency: " << min_.load()<< " ns\n"
                <<"Max latency: " << max_.load()<< " ns\n"
                << "Avg latency: "<< (total_nanos_.load() / count) << " ns\n";
    }
private:
    std::atomic<uint64_t> count_{0};
    std::atomic<uint64_t> min_{UINT64_MAX};
    std::atomic<uint64_t> max_{0};
    std::atomic<uint64_t> total_nanos_{0};
};