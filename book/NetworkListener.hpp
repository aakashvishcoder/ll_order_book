#pragma once
#include "LockFreeQueue.hpp"
#include <atomic>
#include <string>
class NetworkListener {
public:
    NetworkListener(int port, SPSCQueue<Order, 1024*1024>& queue) 
        : port_(port), queue_(queue), running_(false), server_fd_(-1), epoll_fd_(-1) {}
    
    ~NetworkListener();
    void start();
    void stop();
private:
    int port_;
    int server_fd_;
    int epoll_fd_;
    SPSCQueue<Order,1024 * 1024>& queue_;
    std::atomic<bool> running_;

    void runLoop();
};