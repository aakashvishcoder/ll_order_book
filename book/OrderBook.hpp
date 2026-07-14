#pragma once
#include "Order.hpp"
#include "MemoryPool.hpp"
#include "AuditLog.hpp"
#include <map>
#include <list>
#include <unordered_map>
#include <optional>
#include <vector>

struct PriceLevel {
    Order* head= nullptr;
    Order* tail = nullptr;
    uint64_t total_quantity=0;
    void push_back(Order* order) {
        order->prev=tail;
        order->next = nullptr;
        if (tail) tail->next= order;
        else head=order;
        tail=order;
        total_quantity += order->quantity;
    }

    void remove(Order* order) {
        if (order->prev) order->prev->next= order->next;
        else head= order->next;

        if(order->next) order->next->prev = order->prev;
        else tail = order->prev;
        
        total_quantity -=order->quantity;
        order->prev= order->next = nullptr;
    }
};

class OrderBook {
public:
    OrderBook(MemoryPool<Order>& pool, AuditLog& logger)
        : pool_(pool), logger_(logger) {}
    bool addOrder(Order&& order);
    bool cancelOrder(uint64_t order_id_);

    std::optional<uint64_t> getBestBid() const;
    std::optional<uint64_t> getBestAsk() const;
    std::vector<std::pair<uint64_t,uint64_t>> getDepth(Side side, size_t levels) const;
private:
    using PriceLevel = std::list<Order>;
    std::map<uint64_t, PriceLevel, std::greater<uint64_t>> bids_;
    std::map<uint64_t,PriceLevel,std::less<uint64_t>> asks_;
    MemoryPool<Order>& pool_; AuditLog& logger_;
    std::unordered_map<uint64_t, Order*> order_map_;
    void matchOrder(Order& incoming_order);

    void removeFromBook(Order* order);
};