#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;
const int MAX_PENDING_CONNECTIONS = 3;

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 创建一个stdout日志器
    auto logger = spdlog::stdout_color_mt("server");
    logger->set_level(spdlog::level::info); // 设置日志级别为info

    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        logger->error("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定地址和端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        logger->error("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听传入连接
    if (listen(server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        logger->error("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    logger->info("Waiting for connections...");

    // 接受连接
    if ((new_socket =
             accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        logger->error("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    logger->info("Connection from {}:{}", inet_ntoa(address.sin_addr),
                 ntohs(address.sin_port));
    // 处理客户端连接
    while (true) {
        // 读取数据
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            logger->info("Client disconnected");
            close(new_socket);
            break;
        }

        buffer[valread] = '\0'; // 确保缓冲区以空字符结尾
        logger->info("Received data: {}", buffer);

        // 发送响应
        std::string response = "server: ";
        response.append(buffer);
        send(new_socket, response.c_str(), response.length(), 0);
        logger->info("Sent data: {}", response);

        // 检查退出条件
        if (strcmp(buffer, "exit") == 0) {
            logger->info("Received exit message, closing connection");
            break;
        }

        // 清空缓冲区
        memset(buffer, 0, BUFFER_SIZE);
    }

    // 关闭连接
    close(new_socket);
    close(server_fd);

    return 0;
}