#include "channel.h" // 假设 Channel 类的定义在这个文件中
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
