#include "book/StateRecovery.h"

#include "book/AuditLog.h"
#include <cstdio>
#include <iostream>

namespace {
constexpr uint64_t kMaxRecoveryEvents = 100000;

bool isValidSide(Side side) {
    return side == Side::BUY || side == Side::SELL;
}
}

void recoverState(OrderBook& book, const std::string& log_file_path) {
    std::FILE* file = std::fopen(log_file_path.c_str(), "rb");
    if (file == nullptr) {
        std::cout << "No audit log found at " << log_file_path << ". Starting with empty state.\n";
        return;
    }

    std::fseek(file, 0, SEEK_END);
    long file_size = std::ftell(file);
    if (file_size <= 0) {
        std::fclose(file);
        std::cout << "Audit log is empty. Starting with empty state.\n";
        return;
    }

    if (file_size < static_cast<long>(sizeof(AuditLogFileHeader))) {
        std::fclose(file);
        std::cout << "Audit log format is incompatible (missing header). Skipping recovery.\n";
        return;
    }

    std::fseek(file, 0, SEEK_SET);
    AuditLogFileHeader header{};
    if (std::fread(&header, sizeof(header), 1, file) != 1 || !isValidAuditHeader(header)) {
        std::fclose(file);
        std::cout << "Audit log header is invalid or legacy. Skipping recovery.\n";
        return;
    }

    const long event_bytes = file_size - static_cast<long>(sizeof(AuditLogFileHeader));
    if (event_bytes % static_cast<long>(sizeof(AuditEvent)) != 0) {
        std::fclose(file);
        std::cout << "Audit log format is incompatible or corrupted (payload mismatch). Skipping recovery.\n";
        return;
    }

    const uint64_t total_events = static_cast<uint64_t>(event_bytes / static_cast<long>(sizeof(AuditEvent)));
    if (total_events > kMaxRecoveryEvents) {
        std::fclose(file);
        std::cout << "Audit log contains " << total_events
                  << " events; maximum supported replay is " << kMaxRecoveryEvents
                  << ". Skipping recovery to protect startup stability.\n";
        return;
    }

    AuditEvent event;
    uint64_t replayed = 0;
    uint64_t skipped = 0;

    while (std::fread(&event, sizeof(event), 1, file) == 1) {
        if (event.type == EventType::ADD) {
            if (!isValidSide(event.side)) {
                ++skipped;
                continue;
            }
            Order order(event.order_id, event.price, event.quantity, event.side, OrderType::LIMIT);
            try {
                book.addOrder(std::move(order), false);
                ++replayed;
            } catch (...) {
                ++skipped;
            }
        } else if (event.type == EventType::CANCEL) {
            if (book.cancelOrder(event.order_id, false)) {
                ++replayed;
            } else {
                ++skipped;
            }
        } else {
            ++skipped;
        }
    }

    std::fclose(file);

    std::cout << "State recovered from audit log: " << log_file_path
              << " (replayed=" << replayed << ", skipped=" << skipped << ")\n";
}
