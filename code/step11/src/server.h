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
           int num_reactor_threads);
    ~Server();
    void init();
    void run();
    void accept_connection(std::shared_ptr<Channel> server_channel);
    void handle_client(std::shared_ptr<Channel> client_channel);

  private:
    int server_fd;
    int port;
    int buffer_size;
    int max_pending_connections;
    socklen_t addrlen;
    struct sockaddr_in address;
    std::vector<std::unique_ptr<EpollManager>> reactor_threads;
    int next_reactor;
    std::shared_ptr<spdlog::logger> logger;
};

#endif // SERVER_H
