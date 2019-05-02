#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SERV_PORT 5019
#define USER_ERROR -1

char serverAddr[32];
char serverPort[5];
char clientAddr[32];
char clientPort[5];

int main() {
    WSADATA wsaData;
    SOCKET sListen, sAccept;        //服务器监听套接字，连接套接字
    int serverport = SERV_PORT;     //服务器端口号
    struct sockaddr_in ser, cli;    //服务器地址，客户端地址
    int iLen;

    printf("-------------------\n");
    printf("Server Listening...\n");
    printf("-------------------\n");

    // Load Winsock protocol
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to load Winsock.\n");
        return USER_ERROR;
    }

    // Create socket for listening to requests
    sListen = socket(AF_INET, SOCK_STREAM, 0);
    if (sListen == INVALID_SOCKET) {
        printf("Create socket() Failed: %d\n", WSAGetLastError());
        return USER_ERROR;
    }

    // Create the address for server
    ser.sin_family = AF_INET;
    ser.sin_port = htons((u_short) serverport);     //服务器端口号
    ser.sin_addr.s_addr = htonl(INADDR_ANY);        //服务器IP地址，默认使用本机IP

    //Bind listening socket to the server address
    if (bind(sListen, (LPSOCKADDR) &ser, sizeof(ser)) == SOCKET_ERROR) {
        printf("bind() failed: %d\n", WSAGetLastError());
        return USER_ERROR;
    }

    // Listening via the socket
    if (listen(sListen, 5) == SOCKET_ERROR) {
        printf("listen() failed: %d\n", WSAGetLastError());
        return USER_ERROR;
    }

    // Wait for client's request
    while (true) {
        // Accept the request
        iLen = sizeof(cli);
        sAccept = accept(sListen, (struct sockaddr *) &cli, &iLen);
        if (sAccept == INVALID_SOCKET) {
            printf("accept() Failed:%d\n", WSAGetLastError());
            break;
        }
        sprintf(serverAddr, "%s", inet_ntoa(ser.sin_addr));
        sprintf(serverPort, "%d", ntohs(ser.sin_port));
        sprintf(clientAddr, "%s", inet_ntoa(cli.sin_addr));
        sprintf(clientPort, "%d", ntohs(cli.sin_port));

        // Create thread for client browser's request
        // DWORD ThreadID;
        // CreateThread(NULL, 0, SimpleHTTPServer, (LPVOID) sAccept, 0, &ThreadID);
    }
    closesocket(sListen);
    WSACleanup();
    return 0;
}

#pragma clang diagnostic pop