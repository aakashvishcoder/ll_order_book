#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>

template <typename T>
class MemoryPool {
public:
    explicit MemoryPool(size_t capacity) : capacity_(capacity), pool_(capacity), next_free_(0) {}

    T* allocate() {
        if (next_free_ >=capacity) throw std::runtime_error("memory pool exhausted");
        return &pool_[next_free_++];
    }
    void deallocate(T* ptr) {
        //mainting a free-list in future portions of the project
    }
private:
    size_t capacity_;
    std::vector<T> pool_;
    size_t next_free_;
};