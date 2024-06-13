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

    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定地址和端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听传入连接
    if (listen(server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "等待连接..." << std::endl;
    while (1) {
        // 接受连接
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 &addrlen)) < 0) {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        std::cout << "连接来自 " << inet_ntoa(address.sin_addr) << ":"
                  << ntohs(address.sin_port) << std::endl;

        // 读取数据
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        std::cout << "收到数据: " << buffer << std::endl;

        // 发送响应
        std::string response = "server: ";
        response.append(buffer);
        if (strcmp(buffer, "exit") == 0) {
            std::cout << "接收到退出消息，断开连接" << std::endl;
            break;
        }
        send(new_socket, response.c_str(), strlen(response.c_str()), 0);
        std::cout << "回送数据: " << response << std::endl;
        memset(buffer, 0, sizeof(buffer));
        if (valread == 0) {
            close(new_socket);
            std::cout << "客户端断开。" << std::endl; // 有些问题
        }
    }

    // 关闭连接
    close(new_socket);
    close(server_fd);

    return 0;
}
