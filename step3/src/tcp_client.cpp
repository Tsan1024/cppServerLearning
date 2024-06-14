#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Client {
  public:
    Client(const char *server_address, int port, int buffer_size);
    ~Client();
    void run();

  private:
    void connect_to_server();
    void handle_communication();

    const char *server_address;
    int port;
    int buffer_size;
    int sock;
    struct sockaddr_in serv_addr;
    std::shared_ptr<spdlog::logger> logger;
    std::vector<char> buffer;
};

Client::Client(const char *server_address, int port, int buffer_size)
    : server_address(server_address), port(port), buffer_size(buffer_size),
      sock(0), buffer(buffer_size) {
    logger = spdlog::stdout_color_mt("client");
    logger->set_level(spdlog::level::info);
    connect_to_server();
}

Client::~Client() { close(sock); }

void Client::connect_to_server() {
    // 创建socket文件描述符
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        logger->error("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // 将地址转换成二进制形式
    if (inet_pton(AF_INET, server_address, &serv_addr.sin_addr) <= 0) {
        logger->error("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // 连接服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        logger->error("Connection failed");
        exit(EXIT_FAILURE);
    }

    logger->info("Connected to server {}:{}", server_address, port);
}

void Client::run() { handle_communication(); }

void Client::handle_communication() {
    while (true) {
        // 发送数据
        std::cin.getline(buffer.data(), buffer.size());
        logger->info("Sending data: {}", buffer.data());
        send(sock, buffer.data(), strlen(buffer.data()), 0);

        // 检查退出条件
        if (strcmp(buffer.data(), "exit") == 0) {
            logger->info("Received exit message, closing connection");
            break;
        }

        // 读取服务器响应
        int valread = read(sock, buffer.data(), buffer_size - 1);
        if (valread > 0) {
            buffer[valread] = '\0'; // 确保缓冲区以空字符结尾
            logger->info("Received data: {}", buffer.data());
        } else if (valread == 0) {
            logger->info("Server disconnected");
            break;
        } else {
            perror("read");
            break;
        }

        // 清空缓冲区
        std::fill(buffer.begin(), buffer.end(), 0);
    }
}

int main() {
    const char *SERVER_ADDRESS = "127.0.0.1";
    const int PORT = 8080;
    const int BUFFER_SIZE = 1024;

    Client client(SERVER_ADDRESS, PORT, BUFFER_SIZE);
    client.run();

    return 0;
}
