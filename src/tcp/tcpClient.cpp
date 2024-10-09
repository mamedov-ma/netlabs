#include <arpa/inet.h>
#include <array>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int receive_some(int channel, void* data, size_t size)
{
    auto bytes = reinterpret_cast<char*>(data);
    size_t bytes_received = 0;

    while (bytes_received < size) {
        int result = ::recv(channel, &bytes[bytes_received], size - bytes_received, 0);

        if (result <= 0) {
            return result;
        }
        bytes_received += result;
    }

    return bytes_received;
}

int send_some(int channel, const void* data, size_t size)
{
    auto bytes = reinterpret_cast<const char*>(data);
    size_t bytes_sent = 0;

    while (bytes_sent < size) {
        int result = ::send(channel, &bytes[bytes_sent], size - bytes_sent, 0);

        if (result <= 0) {
            return result;
        }
        bytes_sent += result;
    }

    return bytes_sent;
}

int main()
{
    int channel = ::socket(AF_INET, SOCK_STREAM, 0);
    if (channel == -1) {
        std::cerr << "socket() failed." << std::endl;
        return 1;
    }

    std::string addressString;
    int port;
    std::cout << "Введите адрес сервера: ";
    std::cin >> addressString;
    std::cout << "Введите порт сервера: ";
    std::cin >> port;

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(addressString.c_str());
    address.sin_port = htons(port);

    if (::connect(channel, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
        std::cerr << "connect() failed." << std::endl;
        ::close(channel);
        return 1;
    }

    std::string message = "hello from my app\n";
    int sentBytes = send_some(channel, &message[0], message.size());
    if (sentBytes <= 0) {
        std::cerr << "send_some() failed." << std::endl;
        ::close(channel);
        return 1;
    }

    std::array<char, 128> receivedData{};
    int receivedBytes = receive_some(channel, &receivedData[0], receivedData.size());
    if (receivedBytes <= 0) {
        std::cerr << "receive_some() failed." << std::endl;
        ::close(channel);
        return 1;
    }

    std::cout << "Получено сообщение: "
              << std::string(receivedData.begin(), receivedData.begin() + receivedBytes)
              << std::endl;

    ::close(channel);

    return 0;
}
