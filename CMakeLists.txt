cmake_minimum_required(VERSION 3.14.4)
project(net-labs)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(server src/tcp/tcpClient.cpp)
add_executable(client src/tcp/tcpServer.cpp)
add_executable(udp src/udp/udp.cpp)