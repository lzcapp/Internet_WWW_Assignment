# Internet_WorldWideWeb
Team repo for course: Internet and the World Wide Web.

## Assignment Description and Requirements
Students are required to develop a Web Server by using Windows Socket
Programming C++. The program should be able to serve multiple requests
concurrently from different Web Browsers. The Web document stored in the Web
Server could be HTML file, image or any other resources. The sample programs
(namely, “server.cpp” and “client.cpp”) can be downloaded from the course Web site.
The following is the illustration of the interactions between server and client:
- [x] User input the IP address of the Web Server in the address bar of a Web Browser. 
  The Web Browser will make a request to the Web Server as follows: <br/>
   ```
   GET /index.html HTTP/1.0
   ```
- [x] The Web Server should accept the request, and give a proper response to the Web Browser as follows: <br/>
   ``` 
   HTTP/1.0 200 OK
   Content-Type: TEXT/HMTL
   Content-Length: 100
   ...
   ``` 
- [ ] When the Web Browser receives the requested Web Page, it will interpret the content and display on the screen.
   - [x] HTML
   - [ ] Image
   - [ ] Video
- [ ] Extra features:
   - [ ] Black list / White list
   - [ ] Admin edit page
   - [ ] Change server directory path and port number
   - [ ] Password for specified directory
   - [ ] IP filtering
   - [ ] Dynamic advertisement
   - [ ] VIP
 - [ ] Cross-platform:
   - [ ] Raspberry Pi

## Request Method and Respond Status Code 
   - [Request Metod](https://github.com/RainySummerLuo/Internet_WWW_Assignment/wiki/Method-for-Request)
   - [Status Code](https://github.com/RainySummerLuo/Internet_WWW_Assignment/wiki/Status-Code-for-Response)

## Building Enviornment 
   - [Jetbrains CLion](https://www.jetbrains.com/clion/)
      - [JetBrains学生免费授权计划](https://www.jetbrains.com/zh/student/)
   - [MinGW-W64 GCC-8.1.0 (x86_64-8.1.0-posix-seh-rt_v6-rev0)](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download)

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
   - [HTTP/1.1 - W3C](https://www.w3.org/Protocols/HTTP/1.1/rfc2616bis/draft-lafon-rfc2616bis-03.html)
   - [ip(7) - Linux manual page](http://man7.org/linux/man-pages/man7/ip.7.html)
   - [C语言实现简单Web服务器](https://www.jianshu.com/p/592b631e1ff1)
   - [Linux C小项目 —— 简单的web服务器](https://blog.csdn.net/trb331617/article/details/79264933)
   - [C语言开发Linux下web服务器](https://blog.csdn.net/yueguanghaidao/article/details/8450938)
      - [Linux-C-Web-Server](https://github.com/Skycrab/Linux-C-Web-Server)
