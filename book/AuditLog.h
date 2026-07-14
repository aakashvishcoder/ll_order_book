#pragma once
#include <cstdio>
#include <string>
#include <chrono>
#include <mutex>
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
    explicit AuditLog(const std::string& file_path = "audit.log")
        : file_(std::fopen(file_path.c_str(), "ab")) {}

    ~AuditLog() {
        if (file_ != nullptr) {
            std::fclose(file_);
        }
    }

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
    std::FILE* file_;
    std::mutex write_mutex_;

    void writeToFile(const AuditEvent& e) {
        if (file_ == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(write_mutex_);
        std::fwrite(&e, sizeof(e), 1, file_);
    }
};