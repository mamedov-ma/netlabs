#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_MESSAGE_SIZE 1024

std::string receiveMessage(int sockfd)
{
    char buffer[MAX_MESSAGE_SIZE];
    int bytesReceived = recv(sockfd, buffer, MAX_MESSAGE_SIZE, 0);
    if (bytesReceived > 0) {
        return std::string(buffer, bytesReceived);
    } else {
        return "";
    }
}

void sendMessage(int sockfd, std::string message, const char* ipAddress, int port)
{
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ipAddress);
    sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr*)&serverAddr,
        sizeof(serverAddr));
}

int main()
{
    std::string ipAddress;
    int port;
    std::cout << "Введите IP-адрес для прослушивания: ";
    std::cin >> ipAddress;
    std::cout << "Введите порт для прослушивания: ";
    std::cin >> port;

    int sockfd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return 1;
    }

    int enabled = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const char*)&enabled, sizeof(enabled));

    struct sockaddr_in serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = ::htons(port);
    serverAddr.sin_addr.s_addr = ::inet_addr(ipAddress.c_str());

    if (auto err = ::bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)); err == -1) {
        std::cerr << "Ошибка привязки сокета " << std::strerror(errno) << std::endl;
        return 1;
    }

    while (true) {
        std::cout << "\nВыберите действие: \n";
        std::cout << "1. Принять сообщение\n";
        std::cout << "2. Отправить сообщение\n";
        std::cout << "3. Завершить работу\n";
        std::cout << "Введите номер действия: ";
        int choice;
        std::cin >> choice;

        switch (choice) {
        case 1: {
            std::string receivedMessage = receiveMessage(sockfd);
            if (!receivedMessage.empty()) {
                std::cout << "Получено сообщение: " << receivedMessage << std::endl;
            } else {
                std::cout << "Сообщение не получено" << std::endl;
            }
            break;
        }
        case 2: {
            std::string recipientAddress;
            int recipientPort;
            std::string message;
            std::cout << "Введите IP-адрес получателя: ";
            std::cin >> recipientAddress;
            std::cout << "Введите порт получателя: ";
            std::cin >> recipientPort;
            std::cout << "Введите текст сообщения: ";
            std::cin.ignore();
            getline(std::cin, message);
            sendMessage(sockfd, message, recipientAddress.c_str(), recipientPort);
            std::cout << "Сообщение отправлено" << std::endl;
            break;
        }
        case 3: {
            std::cout << "Завершение работы" << std::endl;
            ::close(sockfd);
            return 0;
        }
        default: {
            std::cout << "Неверный выбор" << std::endl;
            break;
        }
        }
    }

    return 0;
}