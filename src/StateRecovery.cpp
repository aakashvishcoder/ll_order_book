#include "book/StateRecovery.h"

#include "book/AuditLog.h"
#include <fstream>
#include <iostream>

void recoverState(OrderBook& book, const std::string& log_file_path) {
    std::ifstream file(log_file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "No audit log found at " << log_file_path << ". Starting with empty state.\n";
        return;
    }

    AuditEvent event;
    while (file.read(reinterpret_cast<char*>(&event), sizeof(event))) {
        if (event.type == EventType::ADD) {
            Order order(event.order_id, event.price, event.quantity, event.side, OrderType::LIMIT);
            book.addOrder(std::move(order));
        } else if (event.type == EventType::CANCEL) {
            book.cancelOrder(event.order_id);
        }
    }

    std::cout << "State recovered from audit log: " << log_file_path << "\n";
}
