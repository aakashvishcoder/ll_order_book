#pragma once
#include "Order.hpp"
#include <map>
#include <list>
#include <unordered_map>
#include <optional>
#include <vector>

class OrderBook {
public:
    bool addOrder(Order&& order);
    bool cancelOrder(uint64_t order_id_);

    std::optional<uint64_t> getBestBid() const;
    std::optional<uint64_t> getBestAsk() const;
    std::vector<std::pair<uint64_t,uint64_t>> getDepth(Side side, size_t levels) const;
private:
    using PriceLevel = std::list<Order>;
    std::map<uint64_t, PriceLevel, std::greater<uint64_t>> bids_;
    std::map<uint64_t,PriceLevel,std::less<uint64_t>> asks_;

    std::unordered_map<uint64_t, Order*> order_map_;
    void matchOrder(Order& incoming_order);

    void removeFromBook(Order* order);
};