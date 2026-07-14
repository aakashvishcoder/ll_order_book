#pragma once
#include <fstream>
#include <string>
#include <chrono>
enum class EventType : uint8_t { ADD, CANCEL, FILL };

struct AuditEvent{
    uint64_t timestamp_ns;
    EventType type;
    uint64_t order_id;
    uint64_t price;
    uint64_t quantity;
};

class AuditLog {
public:
    void logEvent(EventType type, uint64_t order_id, uint64_t price, uint64_t qty) {
        AuditEvent event;
        event.timestamp_ns =std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        event.type = type;
        event.order_id= order_id;
        event.price=price;
        event.quantity = qty;
        
        writeToFile(event);
    }
private:    
    std::ofstream file_;
    void writeToFile(const AuditEvent& e) {
        file_.write(reinterpret_cast<const char*>(&e), sizeof(e));
    }
};