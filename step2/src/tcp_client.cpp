#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const char *SERVER_ADDRESS = "127.0.0.1";
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

int main() {
    // 创建一个stdout日志器
    auto logger = spdlog::stdout_color_mt("client");
    logger->set_level(spdlog::level::info); // 设置日志级别为info

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // 创建socket文件描述符
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        logger->error("Socket creation error");
        return -1;
    }

    // 设置服务器地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 将地址转换成二进制形式
    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
        logger->error("Invalid address/ Address not supported");
        return -1;
    }

    // 连接服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        logger->error("Connection failed");
        return -1;
    }

    logger->info("Connected to server {}:{}", SERVER_ADDRESS, PORT);

    while (1) {
        // 发送数据
        std::cin.getline(buffer, sizeof(buffer));
        logger->info("Sending data: {}", buffer);
        send(sock, buffer, strlen(buffer), 0);

        // 检查退出条件
        if (strcmp(buffer, "exit") == 0) {
            logger->info("Received exit message, closing connection");
            break;
        }

        // 读取服务器响应
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread > 0) {
            buffer[valread] = '\0'; // 确保缓冲区以空字符结尾
            logger->info("Received data: {}", buffer);
        } else if (valread == 0) {
            logger->info("Server disconnected");
            break;
        } else {
            perror("read");
            break;
        }

        // 清空缓冲区
        memset(buffer, 0, sizeof(buffer));
    }

    // 关闭连接
    close(sock);

    return 0;
}