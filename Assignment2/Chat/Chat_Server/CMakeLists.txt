cmake_minimum_required(VERSION 3.14)

project(Chat_Server)

set(CMAKE_C_STANDARD 11)

link_libraries(ws2_32)

add_executable(Chat_Server server.cpp)