#include "OrderBookManager.hpp"
#include "LockFreeQueue.hpp"
#include "LatencyMonitor.hpp"
#include "AuditLog.hpp"
#include "MemoryPool.hpp"
#include <thread>
#include <random>
#include <iostream>
#include <atomic>

int main() {
    // 1. Initialize Core Components
    AuditLog logger;
    MemoryPool<Order> pool(2000000); // Pre-allocate 2M orders
    OrderBookManager manager;
    
    // Create instruments
    manager.createInstrument("AAPL");
    manager.createInstrument("MSFT");
    
    OrderBook* aapl_book = manager.getBook("AAPL");
    if (!aapl_book) return -1;

    SPSCQueue<Order, 1024 * 1024> queue;
    LatencyMonitor monitor;
    std::atomic<bool> running{true};

    // 2. Matching Engine Thread
    std::thread matching_thread([&]() {
        Order order;
        while (running.load() || queue.pop(order)) {
            if (queue.pop(order)) {
                auto start = std::chrono::high_resolution_clock::now();
                
                // Route to correct book (simulating multi-instrument)
                aapl_book->addOrder(std::move(order));
                
                auto end = std::chrono::high_resolution_clock::now();
                monitor.recordLatency(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
            }
        }
    });

    // 3. Market Data Simulator (Producer)
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
            std::this_thread::yield(); 
        }
    }

    running = false;
    matching_thread.join();

    // 4. Print Results
    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    std::cout << "\n=== HFT Order Book Stress Test ===\n";
    std::cout << "Throughput: " << (TOTAL_ORDERS / (duration_ms / 1000.0)) << " orders/sec\n";
    monitor.printStats();
    
    auto depth = aapl_book->getDepth(Side::BUY, 5);
    std::cout << "Top 5 Bid Levels (Price, Qty):\n";
    for(const auto& level : depth) {
        std::cout << "  " << level.first << " : " << level.second << "\n";
    }

    return 0;
}