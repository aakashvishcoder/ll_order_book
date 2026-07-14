#pragma once
#include "OrderBook.h"
#include <vector>

struct ImbalanceMetrics{
    double top_of_book_imbalance;
    double depth_5_imbalance;
};

class AnalyticsEngine{
public:
    ImbalanceMetrics calculate(const OrderBook& book) {
        ImbalanceMetrics metrics;

        auto bid_depth= book.getDepth(Side::BUY, 1);
        auto ask_depth = book.getDepth(Side::SELL, 1);

        uint64_t bid_vol = bid_depth.empty()? 0: bid_depth[0].second;
        uint64_t ask_vol = ask_depth.empty() ? 0: ask_depth[0].second;

        metrics.top_of_book_imbalance = calculateRatio(bid_vol, ask_vol);

        bid_depth= book.getDepth(Side::BUY, 5);
        ask_depth = book.getDepth(Side::SELL, 5);

        uint64_t total_bid=0, total_ask=0;
        for(const auto& lvl: bid_depth) total_bid += lvl.second;
        for(const auto& lvl: ask_depth) total_ask+= lvl.second;
        metrics.depth_5_imbalance= calculateRatio(total_bid, total_ask);
        return metrics;
    }
private:
    double calculateRatio(uint64_t bid, uint64_t ask) {
        if(bid + ask==0) return 0.0;
        return static_cast<double>(bid-ask)/static_cast<double>(bid+ask);
    }
};