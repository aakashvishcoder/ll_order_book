#pragma once
#include "OrderBook.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <shared_mutex>

class OrderBookManager {
public:
    explicit OrderBookManager(size_t pool_capacity = 2000000)
        : pool_(pool_capacity) {}

    void createInstrument(const std::string& symbol) {
        std::unique_lock lock(rw_mutex_);
        books_.emplace(symbol, std::make_unique<OrderBook>(pool_, logger_));
    }

    OrderBook* getBook(const std::string& symbol) {
        std::shared_lock lock(rw_mutex_);
        auto it=books_.find(symbol);
        return (it !=books_.end())? it->second.get(): nullptr;
    }
private:
    MemoryPool<Order> pool_;
    AuditLog logger_;
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> books_;
    std::shared_mutex rw_mutex_;
};