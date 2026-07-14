#include "FixParser.h"

std::optional<Order> FixParser::parseNewOrderSingle(const char* data, size_t len) {
    uint64_t cl_ord_id=0;
    uint64_t price=0; //10050 = $100.50
    uint64_t qty = 0;
    Side side = Side::BUY;
    const char* p= data;

    const char* end = data+len;

    while(p < end) {
        const char* tag_start = p;
        while (p < end && *p != '=') p++;
        if (p >= end) break;
        std::string_view tag(tag_start, p - tag_start);
        p++;

        const char* val_start = p;
        while (p < end && *p != 0x01) p++;
        std::string_view value(val_start, p - val_start);
        if (p < end) p++;

        if (tag=="11") {
            cl_ord_id= fast_atoi(value);
        } else if(tag =="54") {
            side = (value=="2") ? Side::SELL: Side::BUY;
        } else if (tag == "38") {
            qty=fast_atoi(value);
        } else if( tag =="44"){
            price = fast_atoi(value);
        }
    }

    if(cl_ord_id==0 ||qty==0) return std::nullopt;
    return Order(cl_ord_id, price,qty,side,OrderType::LIMIT);
}