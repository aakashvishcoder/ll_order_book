#pragma once
#include <string>
#include "LockFreeQueue.h"
#include "OrderBook.hpp"
#include "Order.h"

struct MarketDataUpdate {
    uint64_t timestamp_ns;
    std::string symbol;
    uint64_t best_bid;
    uint64_t best_ask;
    uint64_t bid_size;
    uint64_t ask_size;
};

class MarketDataBroadcaster {
public:
    MarketDataBroadcaster() = default;

    void publishUpdate(const std::string& symbol, const OrderBook& book){
        MarketDataUpdate update;
        //update.timestamp_ns = current time;
        update.symbol =symbol;
        auto bid= book.getBestBid();
        auto ask=book.getBestAsk();

        update.best_bid = bid.value_or(0);
        update.best_ask= ask.value_or(0);

        //calculate sizes
        outbound_queue_.push(update);
    }
    bool consumeUpdate(MarketDataUpdate& out) {
        return outbound_queue_.pop(out);
    }
private:
    SPSCQueue<MarketDataUpdate, 65536> outbound_queue_;
};