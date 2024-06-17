#include "epollManager.h"
#include <stdexcept>
#include <unistd.h>

EpollManager::EpollManager(int max_events)
    : max_events(max_events), events(max_events) {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("epoll_create1 failed");
    }
}

EpollManager::~EpollManager() { close(epoll_fd); }

void EpollManager::add(int fd, uint32_t events) {
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        throw std::runtime_error("epoll_ctl failed");
    }
}

std::vector<struct epoll_event> EpollManager::wait(int timeout) {
    int num_fds = epoll_wait(epoll_fd, events.data(), max_events, timeout);
    if (num_fds == -1) {
        throw std::runtime_error("epoll_wait failed");
    }
    return std::vector<struct epoll_event>(events.begin(),
                                           events.begin() + num_fds);
}
