#pragma once
#include "OrderBookManager.h"
#include "NetworkListener.h"

class SmartOrderRouter{
public:
    
    SmartOrderRouter(OrderBookManager& manager, NetworkListener& external_exchange_feed)
        : manager_(manager), external_feed_(external_exchange_feed) {}

    void routeOrder(Order order, const std::string& symbol) {
        OrderBook* local_book = manager_.getBook(symbol);
        if (local_book == nullptr) {
            external_feed_.sendOrder(order);
            return;
        }

        auto best_price = (order.side == Side::BUY) ? local_book->getBestAsk() : local_book->getBestBid();
        if (best_price.has_value() && isPriceAcceptable(order, best_price.value())) {
            local_queue_.push(order);
        } else {
            external_feed_.sendOrder(order);
        }
    }
private:
    OrderBookManager& manager_;
    NetworkListener& external_feed_;
    SPSCQueue<Order, 1024*1024> local_queue_;

    bool isPriceAcceptable(const Order& order, uint64_t market_price){
        return (order.side ==Side::BUY) ? (market_price<= order.price+5) : (market_price>= order.price-5);
    }
};