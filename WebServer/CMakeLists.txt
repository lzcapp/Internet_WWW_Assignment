cmake_minimum_required(VERSION 3.13)
project(WebServer C)

set(CMAKE_C_STANDARD 99)

link_libraries(ws2_32)

add_executable(WebServer main.c)