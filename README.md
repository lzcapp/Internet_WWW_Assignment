# Internet_WorldWideWeb
Team repo for course: Internet and the World Wide Web.

## Assignment Description and Requirements
Students are required to develop a Web Server by using Windows Socket
Programming C++. The program should be able to serve multiple requests
concurrently from different Web Browsers. The Web document stored in the Web
Server could be HTML file, image or any other resources. The sample programs
(namely, “server.cpp” and “client.cpp”) can be downloaded from the course Web site.
The following is the illustration of the interactions between server and client:
1. User input the IP address of the Web Server in the address bar of a Web Browser. 
   The Web Browser will make a request to the Web Server as follows:
   ```
   GET /index.html HTTP/1.0
   ```
2. The Web Server should accept the request, and give a proper response to the Web Browser as follows:
   ```
   HTTP/1.0 200 OK
   Content-Type: TEXT/HMTL
   Content-Length: 100
   ...
   ```
3. When the Web Browser receives the requested Web Page, it will interpret the content and display on the screen.

## CMake File
   ```
   cmake_minimum_required(VERSION 3.13)
   project(WebServer C)
   set(CMAKE_C_STANDARD 99)
   link_libraries(ws2_32)
   add_executable(WebServer main.c)
   ```

## File Path
   For CLion Project: 
   ```
   ...\Internet_WWW_Assignment\WebServer\cmake-build-debug\
   ```

## Reference
   - [C语言实现简单Web服务器](https://www.jianshu.com/p/592b631e1ff1)
   - [Linux C小项目 —— 简单的web服务器](https://blog.csdn.net/trb331617/article/details/79264933)
