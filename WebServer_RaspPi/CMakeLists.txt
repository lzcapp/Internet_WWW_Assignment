cmake_minimum_required(VERSION 3.13)

project(WebServer_RaspPi C)

set(CMAKE_C_STANDARD 11)

SET(BUILD_SHARED_LIBS ON)

add_executable(WebServer_RaspPi main.c)