#pragma once
#include "OrderBook.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <shared_mutex>

class OrderBookManager {
public:
    void createInstrument(const std::string& symbol) {
        std::unique_lock lock(rw_mutex_);
        books_.emplace(symbol, std::make_unique<OrderBook>());
    }

    OrderBook* getBook(const std::string& symbol) {
        std::shared_lock lock(rw_mutex_);
        auto it=books_.find(symbol);
        return (it !=books_.end())? it->second.get(): nullptr;
    }
private:
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> books_;
    std::shared_mutex rw_mutex_;
};