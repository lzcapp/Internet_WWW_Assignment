cmake_minimum_required(VERSION 3.14)

project(Chat_Client)

set(CMAKE_CXX_STANDARD 14)

link_libraries(ws2_32)

add_executable(Chat_Client client.cpp)