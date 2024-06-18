#ifndef EPOLLMANAGER_H
#define EPOLLMANAGER_H

#include "channel.h"
#include <sys/epoll.h>
#include <vector>

class EpollManager {
  public:
    EpollManager(int max_events);
    ~EpollManager();
    void add(Channel &channel);
    void wait(int timeout);

  private:
    int epoll_fd;
    int max_events;
    std::vector<struct epoll_event> events;
};

#endif // EPOLLMANAGER_H
