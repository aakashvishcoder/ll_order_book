#pragma once
#include <cstdint>
#include <string_view>
#include <optional>
#include "Order.h"

class FixParser{
public:
    static std::optional<Order> parseNewOrderSingle(const char* data, size_t len) {}
private:
    static uint64_t fast_atoi(std::string_view str) {
        uint64_t res=0;
        for (char c: str) {
            if (c < '0'|| c>'9')break;
            res = res*10 + (c-'0');
        }
        return res;
    }
};