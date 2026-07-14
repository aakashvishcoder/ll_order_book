#include "book/OrderBook.h"
#include <stdexcept>


bool OrderBook::addOrder(Order&& order) {
    if (order_map_.find(order.order_id) != order_map_.end()) {
        throw std::invalid_argument("duplicate order id");
    }

    logger_.logEvent(EventType::ADD, order.order_id, order.price, order.quantity, order.side);
    matchOrder(order);

    if (order.quantity > 0) {
        if (order.side == Side::BUY) {
            auto& level = bids_[order.price];
            level.push_back(std::move(order));
            order_map_[level.back().order_id] = &level.back();
        } else {
            auto& level = asks_[order.price];
            level.push_back(std::move(order));
            order_map_[level.back().order_id] = &level.back();
        }
        return false;
    }
    return true;
}

bool OrderBook::cancelOrder(uint64_t order_id) {
    auto it = order_map_.find(order_id);
    if (it ==order_map_.end()) return false;

    logger_.logEvent(EventType::CANCEL, order_id, it->second->price, it->second->quantity, it->second->side);
    removeFromBook(it->second);

    order_map_.erase(it);
    return true;
}

void OrderBook::removeFromBook(Order* order) {
    if (order->side == Side::BUY) {
        auto level_it = bids_.find(order->price);
        if (level_it != bids_.end()) {
            level_it->second.remove_if([order](const Order& o){
                return o.order_id == order->order_id;
            });
            if (level_it->second.empty()) {
                bids_.erase(level_it);
            }
        }
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it != asks_.end()) {
            level_it->second.remove_if([order](const Order& o){
                return o.order_id == order->order_id;
            });
            if (level_it->second.empty()) {
                asks_.erase(level_it);
            }
        }
    }
}
void OrderBook::matchOrder(Order& incoming) {
    if (incoming.side == Side::BUY) {
        while (incoming.quantity > 0 && !asks_.empty()) {
            auto best_level_it = asks_.begin();
            uint64_t best_price = best_level_it->first;
            bool can_match = incoming.price >= best_price || incoming.type == OrderType::MARKET;
            if (!can_match) break;

            auto& level_orders = best_level_it->second;
            while (incoming.quantity > 0 && !level_orders.empty()) {
                Order& resting = level_orders.front();
                uint64_t match_qty = std::min(incoming.quantity, resting.quantity);
                incoming.quantity -= match_qty;
                resting.quantity -= match_qty;

                if (resting.quantity == 0) {
                    order_map_.erase(resting.order_id);
                    level_orders.pop_front();
                }
            }

            if (level_orders.empty()) {
                asks_.erase(best_level_it);
            }
        }
    } else {
        while (incoming.quantity > 0 && !bids_.empty()) {
            auto best_level_it = bids_.begin();
            uint64_t best_price = best_level_it->first;
            bool can_match = incoming.price <= best_price || incoming.type == OrderType::MARKET;
            if (!can_match) break;

            auto& level_orders = best_level_it->second;
            while (incoming.quantity > 0 && !level_orders.empty()) {
                Order& resting = level_orders.front();
                uint64_t match_qty = std::min(incoming.quantity, resting.quantity);
                incoming.quantity -= match_qty;
                resting.quantity -= match_qty;

                if (resting.quantity == 0) {
                    order_map_.erase(resting.order_id);
                    level_orders.pop_front();
                }
            }

            if (level_orders.empty()) {
                bids_.erase(best_level_it);
            }
        }
    }
}

std::optional<uint64_t> OrderBook::getBestBid() const {
    return bids_.empty() ?std::nullopt : std::optional<uint64_t>(bids_.begin()->first);
}

std::optional<uint64_t> OrderBook::getBestAsk() const {
    return asks_.empty()? std::nullopt : std::optional<uint64_t>(asks_.begin()->first);
}
std::vector<std::pair<uint64_t, uint64_t>> OrderBook::getDepth(Side side, size_t levels) const {
    std::vector<std::pair<uint64_t, uint64_t>> depth;
    depth.reserve(levels);

    size_t count = 0;
    if (side == Side::BUY) {
        for (auto it = bids_.begin(); it != bids_.end() && count < levels; ++it, ++count) {
            uint64_t total_qty = 0;
            for (const auto& order : it->second) {
                total_qty += order.quantity;
            }
            depth.emplace_back(it->first, total_qty);
        }
    } else {
        for (auto it = asks_.begin(); it != asks_.end() && count < levels; ++it, ++count) {
            uint64_t total_qty = 0;
            for (const auto& order : it->second) {
                total_qty += order.quantity;
            }
            depth.emplace_back(it->first, total_qty);
        }
    }
    return depth;
}

GPUBookSnapshot OrderBook::getSnapshot() const {
    GPUBookSnapshot snap={};

    snap.bid_levels= std::min((size_t)50, bids_.size());
    snap.ask_levels = std::min((size_t)50, asks_.size());
    int i =0;
    for (auto it= bids_.begin(); it!= bids_.end() && i <snap.bid_levels; ++it, ++i){
        snap.bids[i].price= it->first;
        uint64_t qty=0;
        for (const auto& o : it->second) qty += o.quantity;
        snap.bids[i].quantity= qty;
    }
    i=0;

    for (auto it= asks_.begin(); it!= asks_.end() &&i < snap.ask_levels; ++it, ++i){
        snap.asks[i].price= it->first;

        uint64_t qty = 0;
        for (const auto& o: it->second) qty+= o.quantity;
        snap.asks[i].quantity = qty;
    }

    return snap;
}