#pragma once
#include <cstdio>
#include <string>
#include <chrono>
#include <mutex>
#include <cstdint>
#include <cstring>
#include "Order.h"
enum class EventType : uint8_t { ADD, CANCEL, FILL };

struct AuditLogFileHeader {
    char magic[4];
    uint32_t version;
    uint32_t reserved;
};

constexpr uint32_t kAuditLogVersion = 1;
constexpr char kAuditLogMagic[4] = {'A', 'L', 'O', 'G'};

inline bool isValidAuditHeader(const AuditLogFileHeader& header) {
    return std::memcmp(header.magic, kAuditLogMagic, sizeof(kAuditLogMagic)) == 0
        && header.version == kAuditLogVersion;
}

struct AuditEvent{
    uint64_t timestamp_ns;
    EventType type;
    uint64_t order_id;
    uint64_t price;
    uint64_t quantity;
    Side side;
};

class AuditLog {
public:
    explicit AuditLog(const std::string& file_path = "audit.log")
        : file_path_(file_path), file_(nullptr) {
        initialize();
    }

    ~AuditLog() {
        if (file_ != nullptr) {
            std::fclose(file_);
        }
    }

    void logEvent(EventType type, uint64_t order_id, uint64_t price, uint64_t qty, Side side = Side::BUY) {
        AuditEvent event;
        event.timestamp_ns =std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        event.type = type;
        event.order_id= order_id;
        event.price=price;
        event.quantity = qty;
        event.side = side;
        
        writeToFile(event);
    }
private:    
    std::string file_path_;
    std::FILE* file_;
    std::mutex write_mutex_;

    void writeHeader() {
        AuditLogFileHeader header{};
        std::memcpy(header.magic, kAuditLogMagic, sizeof(kAuditLogMagic));
        header.version = kAuditLogVersion;
        header.reserved = 0;
        std::fwrite(&header, sizeof(header), 1, file_);
        std::fflush(file_);
    }

    void rotateLegacyFile() {
        const std::string legacy_path = file_path_ + ".legacy";
        std::remove(legacy_path.c_str());
        std::rename(file_path_.c_str(), legacy_path.c_str());
    }

    void initialize() {
        file_ = std::fopen(file_path_.c_str(), "rb+");
        if (file_ == nullptr) {
            file_ = std::fopen(file_path_.c_str(), "wb+");
            if (file_ == nullptr) {
                return;
            }
            writeHeader();
            std::fseek(file_, 0, SEEK_END);
            return;
        }

        std::fseek(file_, 0, SEEK_END);
        long file_size = std::ftell(file_);
        if (file_size == 0) {
            std::fseek(file_, 0, SEEK_SET);
            writeHeader();
            std::fseek(file_, 0, SEEK_END);
            return;
        }

        if (file_size < static_cast<long>(sizeof(AuditLogFileHeader))) {
            std::fclose(file_);
            file_ = nullptr;
            rotateLegacyFile();
            file_ = std::fopen(file_path_.c_str(), "wb+");
            if (file_ == nullptr) {
                return;
            }
            writeHeader();
            std::fseek(file_, 0, SEEK_END);
            return;
        }

        AuditLogFileHeader header{};
        std::fseek(file_, 0, SEEK_SET);
        if (std::fread(&header, sizeof(header), 1, file_) != 1 || !isValidAuditHeader(header)) {
            std::fclose(file_);
            file_ = nullptr;
            rotateLegacyFile();
            file_ = std::fopen(file_path_.c_str(), "wb+");
            if (file_ == nullptr) {
                return;
            }
            writeHeader();
            std::fseek(file_, 0, SEEK_END);
            return;
        }

        std::fseek(file_, 0, SEEK_END);
    }

    void writeToFile(const AuditEvent& e) {
        if (file_ == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(write_mutex_);
        std::fwrite(&e, sizeof(e), 1, file_);
    }
};