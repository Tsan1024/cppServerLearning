#ifndef SERVER_H
#define SERVER_H

#include "channel.h"
#include "epollManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "threadPool.h"
#include <memory>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <vector>

class Server {
  public:
    Server(int port, int buffer_size, int max_pending_connections,
           int max_thread_pools);
    ~Server();

    void run();

  private:
    int port;
    int server_fd;
    int buffer_size;
    int max_pending_connections;
    socklen_t addrlen;
    struct sockaddr_in address;
    std::shared_ptr<spdlog::logger> logger;
    ThreadPool::ThreadPool thread_pool;
    EpollManager epoll_manager;

    void init();
    void accept_connection(std::shared_ptr<Channel> server_channel);
    void handle_client(std::shared_ptr<Channel> client_channel);
};

#endif // SERVER_H
