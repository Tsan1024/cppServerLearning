#include "server.h"
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

#define MAX_EVENTS 10

Server::Server(int port, int buffer_size, int max_pending_connections)
    : port(port), buffer_size(buffer_size),
      max_pending_connections(max_pending_connections),
      addrlen(sizeof(address)), epoll_manager(MAX_EVENTS) {
    logger = spdlog::stdout_color_mt("server");
    logger->set_level(spdlog::level::info);
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
    epoll_manager.add(*server_channel);

    logger->info("Server is running and waiting for connections...");
}

void Server::run() {
    while (true) {
        epoll_manager.wait(-1);
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
    epoll_manager.add(*client_channel);

    // Initiate asynchronous read
    struct aiocb *cb = new struct aiocb;
    memset(cb, 0, sizeof(struct aiocb));
    cb->aio_fildes = client_fd;
    cb->aio_buf = malloc(buffer_size);
    cb->aio_nbytes = buffer_size;
    cb->aio_sigevent.sigev_notify = SIGEV_THREAD;
    cb->aio_sigevent.sigev_notify_function = [](sigval_t sigval) {
        Server *server = static_cast<Server *>(sigval.sival_ptr);
        server->read_complete(sigval);
    };
    cb->aio_sigevent.sigev_value.sival_ptr = this;

    if (aio_read(cb) == -1) {
        logger->error("aio_read failed");
        close(client_fd);
        free(cb->aio_buf);
        delete cb;
    }
}

void Server::read_complete(sigval_t sigval) {
    struct aiocb *cb = (struct aiocb *)sigval.sival_ptr;
    int ret = aio_return(cb);
    if (ret > 0) {
        logger->info("Read data: {}", std::string((char *)cb->aio_buf, ret));
        std::string response =
            "server: " + std::string((char *)cb->aio_buf, ret);

        // Initiate asynchronous write
        memset(cb, 0, sizeof(struct aiocb));
        cb->aio_fildes = cb->aio_fildes;
        cb->aio_buf = strdup(response.c_str());
        cb->aio_nbytes = response.length();
        cb->aio_sigevent.sigev_notify = SIGEV_THREAD;
        cb->aio_sigevent.sigev_notify_function = [](sigval_t sigval) {
            Server *server = static_cast<Server *>(sigval.sival_ptr);
            server->write_complete(sigval);
        };
        cb->aio_sigevent.sigev_value.sival_ptr = this;

        if (aio_write(cb) == -1) {
            logger->error("aio_write failed");
            close(cb->aio_fildes);
            free((void *)cb->aio_buf);
            delete cb;
        }
    } else {
        logger->error("aio_read failed or client disconnected");
        close(cb->aio_fildes);
        free(cb->aio_buf);
        delete cb;
    }
}

void Server::write_complete(sigval_t sigval) {
    struct aiocb *cb = (struct aiocb *)sigval.sival_ptr;
    int ret = aio_return(cb);
    if (ret != -1) {
        logger->info("Sent data: {}",
                     std::string((char *)cb->aio_buf, cb->aio_nbytes));
    } else {
        logger->error("aio_write failed");
    }
    close(cb->aio_fildes);
    free((void *)cb->aio_buf);
    delete cb;
}

void Server::handle_client(std::shared_ptr<Channel> client_channel) {
    // 不再需要实现此方法，因为I/O操作已在read_complete和write_complete中处理
}
