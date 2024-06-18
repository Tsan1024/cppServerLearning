#include "epollManager.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>

EpollManager::EpollManager(int max_events)
    : max_events(max_events), events(max_events) {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        spdlog::error("Failed to create epoll file descriptor: {}",
                      strerror(errno));
        throw std::runtime_error("epoll_create1 failed");
    }
}

EpollManager::~EpollManager() { close(epoll_fd); }

void EpollManager::add(Channel &channel) {
    struct epoll_event event;
    event.events = channel.getEvents();
    event.data.ptr = &channel;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, channel.getFd(), &event) == -1) {
        spdlog::error("Failed to add fd to epoll: {}, error: {}",
                      channel.getFd(), strerror(errno));
        throw std::runtime_error("epoll_ctl failed");
    }
}

void EpollManager::wait(int timeout) {
    int num_fds = epoll_wait(epoll_fd, events.data(), max_events, timeout);
    if (num_fds == -1) {
        spdlog::error("epoll_wait failed: {}", strerror(errno));
        throw std::runtime_error("epoll_wait failed");
    }
    for (int i = 0; i < num_fds; i++) {
        Channel *channel = static_cast<Channel *>(events[i].data.ptr);
        channel->setRevents(events[i].events);
        channel->handleEvent();
    }
}
