#pragma once
#include <cstdint>
#include <chrono>

enum class Side: uint8_t {BUY, SELL};
enum class OrderType: uint8_t { LIMIT, MARKET };

struct Order {
    uint64_t order_id;
    uint64_t price;
    uint64_t quantity;
    Side side;
    OrderType type;
    uint64_t timestamp;

    Order(uint64_t id, uint64_t p, uint64_t q, Side s, OrderType t) 
        : order_id(id), price(p), quantity(q), side(s), type(t),
        timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count()) {}
};