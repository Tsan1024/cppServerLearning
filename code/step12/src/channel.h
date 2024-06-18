#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <sys/epoll.h>

class Channel {
  public:
    using EventCallback = std::function<void()>;

    Channel(int fd);
    void setReadCallback(EventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setErrorCallback(EventCallback cb);
    void handleEvent();
    void setEvents(uint32_t ev);
    void setRevents(uint32_t rev);
    int getFd() const;
    uint32_t getEvents() const;
    uint32_t getRevents() const;

  private:
    int fd;
    uint32_t events;
    uint32_t revents;
    EventCallback readCallback;
    EventCallback writeCallback;
    EventCallback errorCallback;
};

#endif // CHANNEL_H
