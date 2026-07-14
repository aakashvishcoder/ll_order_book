#pragma once
#include "LockFreeQueue.h"
#include <atomic>
#include <string>
class NetworkListener {
public:
    NetworkListener(int port, SPSCQueue<Order, 1024*1024>& queue) 
        : port_(port), queue_(queue), running_(false), server_fd_(-1), epoll_fd_(-1) {}
    
    ~NetworkListener();
    void start();
    void stop();
    bool sendOrder(Order order) {
        return queue_.push(order);
    }
private:
    int port_;
    int server_fd_;
    int epoll_fd_;
    SPSCQueue<Order,1024 * 1024>& queue_;
    std::atomic<bool> running_;

    void runLoop();
};

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    // Use WSAPoll or IOCP for Windows
#else
    #include <sys/epoll.h>
    #include <unistd.h>
    // Use epoll for Linux
#endif

void NetworkListener::start() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    // Initialize WinSock socket...
#else
    server_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    // Initialize epoll...
#endif
}