#include "book/LatencyMonitor.h"
#include "book/OrderBook.h"
#include "book/LockFreeQueue.h"
#include "book/AuditLog.h"
#include "book/MemoryPool.h"
#include <thread>
#include <random>
#include <iostream>

int main() {
    MemoryPool<Order> pool(2000000);
    AuditLog logger;
    OrderBook book(pool, logger);
    SPSCQueue<Order, 1024 * 1024> queue; // 1M queue
    LatencyMonitor monitor;
    std::atomic<bool> running{true};

    // Consumer Thread (Matching Engine)
    std::thread matching_thread([&]() {
        // Seed with a valid object; pop() will overwrite before processing.
        Order order(0, 0, 0, Side::BUY, OrderType::LIMIT);
        while (running.load()) {
            if (queue.pop(order)) {
                auto start = std::chrono::high_resolution_clock::now();
                book.addOrder(std::move(order));
                auto end = std::chrono::high_resolution_clock::now();
                monitor.recordLatency(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
            } else {
                std::this_thread::yield();
            }
        }

        while (queue.pop(order)) {
            auto start = std::chrono::high_resolution_clock::now();
            book.addOrder(std::move(order));
            auto end = std::chrono::high_resolution_clock::now();
            monitor.recordLatency(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        }
    });

    // Producer Thread (Market Data Simulator)
    std::mt19937 rng(42);
    std::uniform_int_distribution<uint64_t> price_dist(10000, 10100);
    std::uniform_int_distribution<uint64_t> qty_dist(1, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);

    const int TOTAL_ORDERS = 1000000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TOTAL_ORDERS; ++i) {
        Order order(i, price_dist(rng), qty_dist(rng), 
                    side_dist(rng) == 0 ? Side::BUY : Side::SELL, OrderType::LIMIT);
        
        while (!queue.push(std::move(order))) {
            std::this_thread::yield(); // Queue full, wait
        }
    }

    running = false;
    matching_thread.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    std::cout << "=== Stress Test Results ===\n";
    std::cout << "Throughput: " << (TOTAL_ORDERS / (duration_ms / 1000.0)) << " orders/sec\n";
    monitor.printStatus();
    std::cout << "Best Bid: " << book.getBestBid().value_or(0) << "\n";
    std::cout << "Best Ask: " << book.getBestAsk().value_or(0) << "\n";

    return 0;
}