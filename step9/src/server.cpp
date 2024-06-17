#include "server.h"
#define MAX_EVENTS 10
Server::Server(int port, int buffer_size, int max_pending_connections,
               int max_thread_pools)
    : port(port), buffer_size(buffer_size),
      max_pending_connections(max_pending_connections),
      addrlen(sizeof(address)), thread_pool(max_thread_pools),
      epoll_manager(MAX_EVENTS) {
    logger = spdlog::stdout_color_mt("server");
    logger->set_level(spdlog::level::info);
    init();
}

Server::~Server() { close(server_fd); }

void Server::init() {
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        logger->error("socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        logger->error("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, max_pending_connections) < 0) {
        logger->error("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    epoll_manager.add(server_fd, EPOLLIN);
    logger->info("Waiting for connections...");
}

void Server::run() {
    while (true) {
        auto events = epoll_manager.wait(-1);
        for (auto &event : events) {
            if (event.data.fd == server_fd) {
                int new_socket;
                if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                         &addrlen)) < 0) {
                    logger->error("accept failed");
                    continue;
                }
                logger->info("Connection from {}:{}",
                             inet_ntoa(address.sin_addr),
                             ntohs(address.sin_port));
                epoll_manager.add(new_socket, EPOLLIN | EPOLLET);
            } else {
                int client_socket = event.data.fd;
                thread_pool.enqueue(
                    [this, client_socket]() { handle_client(client_socket); });
            }
        }
    }
}

void Server::handle_client(int client_socket) {
    char buffer[buffer_size] = {0};
    while (true) {
        int valread = read(client_socket, buffer, buffer_size);
        if (valread <= 0) {
            if (valread == 0) {
                logger->info("Client disconnected");
            } else {
                logger->error("read error");
            }
            close(client_socket);
            break;
        }

        buffer[valread] = '\0';

        std::string response = "server: ";
        response.append(buffer);
        send(client_socket, response.c_str(), response.length(), 0);
        logger->info("Sent data: {}", response);

        if (strcmp(buffer, "exit") == 0) {
            logger->info("Received exit message, closing connection");
            close(client_socket);
            break;
        }

        memset(buffer, 0, buffer_size);
    }
    close(client_socket);
}

int main() {
    const int PORT = 8080;
    const int BUFFER_SIZE = 1024;
    const int MAX_PENDING_CONNECTIONS = 3;
    const int THREAD_POOL_SIZE = 5;

    Server server(PORT, BUFFER_SIZE, MAX_PENDING_CONNECTIONS, THREAD_POOL_SIZE);
    server.run();

    return 0;
}
