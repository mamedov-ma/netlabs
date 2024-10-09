#include <arpa/inet.h>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

enum Type {
    TYPE_GET,
    TYPE_LIST,
    TYPE_ERROR
};

const uint32_t MAX_MESSAGE_LENGTH = 300;

void hex_dump(const char* data, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        if (i % 16 == 0) {
            std::cout << std::hex << std::setfill('0') << std::setw(8) << i << ": ";
        }
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)data[i] << " ";
        if (i % 4 == 3) {
            std::cout << " ";
        }
        if (i % 16 == 15) {
            std::cout << std::endl;
        }
    }
    if (length % 16 != 0) {
        std::cout << std::endl;
    }
}

void ask_endpoint(std::string& ip_address, int& port)
{
    std::cout << "Введите IP-адрес: ";
    std::cin >> ip_address;
    std::cout << "Введите порт: ";
    std::cin >> port;
}

bool send_some(int channel, const void* data, int size)
{
    int bytesSent = send(channel, (char*)data, size, 0);
    if (bytesSent == -1) {
        std::cerr << "Ошибка отправки данных: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

bool receive_some(int channel, void* data, int size)
{
    int bytesReceived = recv(channel, (char*)data, size, 0);
    if (bytesReceived == -1) {
        std::cerr << "Ошибка приема данных: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

bool send_error(int channel, const std::string& error)
{
    const uint32_t length = htonl(sizeof(Type) + error.size());
    send_some(channel, &length, sizeof(length));

    const Type type = TYPE_ERROR;
    send_some(channel, &type, sizeof(type));

    send_some(channel, error.c_str(), error.size());
    return true;
}

bool process_unexpected_message(int channel, uint32_t length, Type type)
{
    std::cerr << "Получено неожиданное сообщение: length = " << length << ", type = " << (int)type
              << std::endl;

    char buffer[MAX_MESSAGE_LENGTH];
    receive_some(channel, buffer, length);
    std::cout << "Содержимое сообщения:" << std::endl;
    hex_dump(buffer, length);

    return false;
}

bool serve_request(int channel)
{
    uint32_t length;
    receive_some(channel, &length, sizeof(length));

    length = ntohl(length);

    if (length > MAX_MESSAGE_LENGTH) {
        std::cerr << "Длина сообщения превышает максимально допустимую" << std::endl;
        send_error(channel, "Invalid message length");
        return false;
    }

    Type type;
    receive_some(channel, &type, sizeof(type));

    switch (type) {
    case TYPE_GET:
        // ... serve_file()
        return true; // Временная реализация
    case TYPE_LIST:
        // ... serve_list()
        return true; // Временная реализация
    default:
        return process_unexpected_message(channel, length, type);
    }
}

void serve_requests(int channel)
{
    while (serve_request(channel)) {}
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        std::cerr << "Ошибка создания сокета: " << strerror(errno) << std::endl;
        return 1;
    }

    std::string ip_address;
    int port;
    ask_endpoint(ip_address, port);

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server_address.sin_port = htons(port);

    if (bind(listener, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Ошибка привязки сокета: " << strerror(errno) << std::endl;
        close(listener);
        return 1;
    }

    if (listen(listener, 3) == -1) {
        std::cerr << "Ошибка перевода в режим слушателя: " << strerror(errno) << std::endl;
        close(listener);
        return 1;
    }

    sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);

    int channel = accept(listener, (sockaddr*)&client_address, &client_address_size);

    if (channel == -1) {
        std::cerr << "Ошибка при приеме подключения: " << strerror(errno) << std::endl;
        close(listener);
        return 1;
    }

    char client_ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), client_ip_address, INET_ADDRSTRLEN);
    std::cout << "Подключился клиент с адреса: " << client_ip_address << std::endl;

    serve_requests(channel);

    ::close(channel);
    std::clog << "info: client disconnected\n";
    ::close(listener);

    return 0;
}