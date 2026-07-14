#include "NetworkListener.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>

void NetworkListener::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
    int opt_ =1;
    setsockopt(server_fd_, SOL_SOCKET,SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family= AF_INET;
    address.sin_addr.s_addr= INADDR_ANY;
    address.sin_port = htons(port_);

    bind(server_fd_, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd_, 10);

    epoll_fd_= epoll_create1(0);
    struct epoll_event ev;
    ev.events= EPOLLIN;
    ev.data.fd= server_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &ev);

    running_=true;
    runLoop();
}

void NetworkListener::runLoop() {
    const int MAX_EVENTS= 64;
    struct epoll_event events[MAX_EVENTS];
    std::vector<char> buffer(4096);

    while(running_) {
        int n =epoll_wait(epoll_fd_, events, MAX_EVENTS,100);
        for(int i =0; i < n; i++){
            if (events[i].data.fd== server_fd_) {
                int client_fd = accept4(server_fd_, nullptr, nullptr, SOCK_NONBLOCK);
                if (client_fd >=0) {
                    struct epoll_event ev;
                    ev.events= EPOLLIN| EPOLLET;
                    ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev);
                }
            } else {
                int client_fd = events[i].data.fd;
                ssize_t count =read(client_fd, buffer.data(), buffer.size());
                if (count<=0) {
                    close(client_fd);
                    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, nullptr);
                } else {
                    auto order= FixParser::parseNewOrderSingle(buffer.data(),count);
                    if( order.has_value()) {
                        while(!queue.push(std::move(order.value()))) {
                            //hft = drop or block
                        }
                    }
                }
            }
        }
    }
}

void NetworkListener::stop() {
    running_ = false;
}

NetworkListener::~NetworkListener() {
    if(server_fd_>=0) close(server_fd_);
    if (epoll_fd_ >= 0) close(epoll_fd_);
}