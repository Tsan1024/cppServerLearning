#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

int main() {
    int server_fd;
    struct sockaddr_in address, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (true) {
        int len = recvfrom(server_fd, buffer, BUFFER_SIZE, 0,
                           (struct sockaddr *)&client_addr, &client_addr_len);
        buffer[len] = '\0';
        std::cout << "收到数据: " << buffer << std::endl;

        std::string response = "server: ";
        response.append(buffer);

        sendto(server_fd, response.c_str(), response.length(), 0,
               (struct sockaddr *)&client_addr, client_addr_len);
        std::cout << "回送数据: " << response << std::endl;

        if (strcmp(buffer, "exit") == 0) {
            std::cout << "接收到退出消息，停止服务器" << std::endl;
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

    close(server_fd);
    return 0;
}
