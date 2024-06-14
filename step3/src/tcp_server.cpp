#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Server {
  public:
    Server(int port, int buffer_size, int max_pending_connections);
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
};

Server::Server(int port, int buffer_size, int max_pending_connections)
    : port(port), buffer_size(buffer_size),
      max_pending_connections(max_pending_connections),
      addrlen(sizeof(address)) {
    logger = spdlog::stdout_color_mt("server");
    logger->set_level(spdlog::level::info);
    init();
}

Server::~Server() { close(server_fd); }

void Server::init() {
    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        logger->error("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 绑定地址和端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        logger->error("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听传入连接
    if (listen(server_fd, max_pending_connections) < 0) {
        logger->error("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    logger->info("Waiting for connections...");
}

void Server::run() {
    int new_socket;
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 &addrlen)) < 0) {
            logger->error("accept failed");
            continue;
        }
        logger->info("Connection from {}:{}", inet_ntoa(address.sin_addr),
                     ntohs(address.sin_port));
        handle_client(new_socket);
    }
}

void Server::handle_client(int client_socket) {
    char buffer[buffer_size] = {0};
    while (true) {
        int valread = read(client_socket, buffer, buffer_size);
        if (valread <= 0) {
            logger->info("Client disconnected");
            close(client_socket);
            break;
        }

        buffer[valread] = '\0'; // 确保缓冲区以空字符结尾

        // 发送响应
        std::string response = "server: ";
        response.append(buffer);
        send(client_socket, response.c_str(), response.length(), 0);
        logger->info("Sent data: {}", response);

        // 检查退出条件
        if (strcmp(buffer, "exit") == 0) {
            logger->info("Received exit message, closing connection");
            break;
        }

        // 清空缓冲区
        memset(buffer, 0, buffer_size);
    }
    close(client_socket);
}

int main() {
    const int PORT = 8080;
    const int BUFFER_SIZE = 1024;
    const int MAX_PENDING_CONNECTIONS = 3;

    Server server(PORT, BUFFER_SIZE, MAX_PENDING_CONNECTIONS);
    server.run();

    return 0;
}
