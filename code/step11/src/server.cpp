#include "server.h"

#include <arpa/inet.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

#define MAX_EVENTS 10

Server::Server(int port, int buffer_size, int max_pending_connections,
               int num_reactor_threads)
    : port(port), buffer_size(buffer_size),
      max_pending_connections(max_pending_connections),
      addrlen(sizeof(address)), next_reactor(0) {
    logger = spdlog::stdout_color_mt("server");
    logger->set_level(spdlog::level::info);
    reactor_threads.resize(num_reactor_threads);
    for (int i = 0; i < num_reactor_threads; ++i) {
        reactor_threads[i] =
            std::unique_ptr<EpollManager>(new EpollManager(MAX_EVENTS));
    }
    init();
}

Server::~Server() { close(server_fd); }

void Server::init() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
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

    auto server_channel = std::make_shared<Channel>(server_fd);
    server_channel->setEvents(EPOLLIN);
    server_channel->setReadCallback([this, server_channel]() {
        logger->info("Connecting...");
        this->accept_connection(server_channel);
    });

    reactor_threads[0]->add(*server_channel);

    logger->info("Server is running and waiting for connections...");
}

void Server::run() {
    while (true) {
        for (auto &reactor : reactor_threads) {
            reactor->wait(100); // 每个Reactor线程处理自己的事件
        }
    }
}

void Server::accept_connection(std::shared_ptr<Channel> server_channel) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd < 0) {
        logger->error("accept failed");
        return;
    }

    logger->info("Connection from {}:{}", inet_ntoa(client_addr.sin_addr),
                 ntohs(client_addr.sin_port));

    auto client_channel = std::make_shared<Channel>(client_fd);
    client_channel->setEvents(EPOLLIN);
    client_channel->setReadCallback(
        [this, client_channel]() { this->handle_client(client_channel); });

    int reactor_index = next_reactor++ % reactor_threads.size();
    reactor_threads[reactor_index]->add(*client_channel);
}

void Server::handle_client(std::shared_ptr<Channel> client_channel) {
    char buffer[buffer_size] = {0};
    int valread = read(client_channel->getFd(), buffer, buffer_size - 1);
    if (valread <= 0) {
        if (valread == 0) {
            logger->info("Client disconnected");
        } else {
            logger->error("read error");
        }
        close(client_channel->getFd());
        return;
    }

    buffer[valread] = '\0';
    std::string response = "server: " + std::string(buffer);
    send(client_channel->getFd(), response.c_str(), response.length(), 0);
    logger->info("Sent data: {}", response);

    if (strcmp(buffer, "exit") == 0) {
        logger->info("Received exit message, closing connection");
        close(client_channel->getFd());
    }

    memset(buffer, 0, buffer_size); // 清空buffer放在最后
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
