#include "channel.h"
#include <spdlog/spdlog.h>

Channel::Channel(int fd) : fd(fd), events(0), revents(0) {
    spdlog::info("Channel created for fd: {}", fd);
}

void Channel::setReadCallback(EventCallback cb) {
    readCallback = std::move(cb);
    spdlog::info("Read callback set for fd: {}", fd);
}

void Channel::setWriteCallback(EventCallback cb) {
    writeCallback = std::move(cb);
    spdlog::info("Write callback set for fd: {}", fd);
}

void Channel::setErrorCallback(EventCallback cb) {
    errorCallback = std::move(cb);
    spdlog::info("Error callback set for fd: {}", fd);
}

void Channel::handleEvent() {
    spdlog::debug("Handling events for fd: {}", fd);
    if (revents & (EPOLLERR | EPOLLHUP)) {
        spdlog::error("Error or hangup on fd: {}", fd);
        if (errorCallback)
            errorCallback();
    }
    if (revents & EPOLLIN) {
        spdlog::info("Read event for fd: {}", fd);
        if (readCallback)
            readCallback();
    }
    if (revents & EPOLLOUT) {
        spdlog::info("Write event for fd: {}", fd);
        if (writeCallback)
            writeCallback();
    }
}

void Channel::setEvents(uint32_t ev) {
    events = ev;
    spdlog::info("Events set for fd: {}, events: {}", fd, events);
}

void Channel::setRevents(uint32_t rev) {
    revents = rev;
    spdlog::info("Revents set for fd: {}, revents: {}", fd, revents);
}

int Channel::getFd() const { return fd; }
uint32_t Channel::getEvents() const { return events; }
uint32_t Channel::getRevents() const { return revents; }
