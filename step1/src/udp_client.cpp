#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const char *SERVER_ADDRESS = "127.0.0.1";
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(serv_addr);
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    while (true) {
        std::cin.getline(buffer, sizeof(buffer));
        sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr,
               addr_len);

        int len = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                           (struct sockaddr *)&serv_addr, &addr_len);
        buffer[len] = '\0';
        std::cout << "收到数据: " << buffer << std::endl;

        if (strcmp(buffer, "exit") == 0) {
            std::cout << "客户端请求断开连接" << std::endl;
            break;
        }

        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    return 0;
}
