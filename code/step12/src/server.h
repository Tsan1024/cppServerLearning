#ifndef SERVER_H
#define SERVER_H

#include "channel.h"
#include "epollManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <aio.h>
#include <asm-generic/siginfo.h>
#include <memory>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <vector>

class Server {
  public:
    Server(int port, int buffer_size, int max_pending_connections);
    ~Server();
    void init();
    void run();
    void accept_connection(std::shared_ptr<Channel> server_channel);
    void handle_client(std::shared_ptr<Channel> client_channel);
    void read_complete(sigval_t sigval);
    void write_complete(sigval_t sigval);

  private:
    int server_fd;
    int port;
    int buffer_size;
    int max_pending_connections;
    socklen_t addrlen;
    struct sockaddr_in address;
    EpollManager epoll_manager;
    std::shared_ptr<spdlog::logger> logger;
};

#endif // SERVER_H
