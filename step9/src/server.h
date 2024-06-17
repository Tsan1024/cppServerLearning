#ifndef SERVER_H
#define SERVER_H

#include "epollManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "threadPool.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Server {
  public:
    Server(int port, int buffer_size, int max_pending_connections,
           int max_thread_pools);
    ~Server();
    void run();

  private:
    void init();
    void handle_client(int client_socket);

    int server_fd;
    int port;
    int buffer_size;
    int max_pending_connections;
    struct sockaddr_in address;
    socklen_t addrlen;
    std::shared_ptr<spdlog::logger> logger;
    ThreadPool::ThreadPool thread_pool;
    EpollManager epoll_manager;
};

#endif // SERVER_H
