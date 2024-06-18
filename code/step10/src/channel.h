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
    int fd;           // 文件描述符
    uint32_t events;  // 注册的事件
    uint32_t revents; // 返回的事件

    EventCallback readCallback;  // 读事件回调
    EventCallback writeCallback; // 写事件回调
    EventCallback errorCallback; // 错误事件回调
};

#endif // CHANNEL_H
