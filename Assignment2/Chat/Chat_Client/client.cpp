#include <winsock2.h>
#include <process.h>
#include <cstdio>
#include <cstdlib>
#include <conio.h>

// #include "DES.h"
#include "DES.cpp"

#pragma comment(lib, "ws2_32.lib")

#pragma warning (disable: 4996)

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

// #define RECV_OVER 1
// #define RECV_YET 0

char userName[16] = {0};
// int iStatus = RECV_YET;


unsigned __stdcall ThreadRecv(void *param) {
    char buf[128] = {0};
    while (true) {
        int ret = recv(*(SOCKET *) param, buf, sizeof(buf), 0);
        if (ret == SOCKET_ERROR) {
            Sleep(500);
            continue;
        }
        if (strlen(buf) != 0) {
            printf("%s\n", buf);
            // iStatus = RECV_OVER;
        } else {
            Sleep(100);
        }
    }
    // return 0;
}

//发送数据
unsigned __stdcall ThreadSend(void *param) {
    char buf[128] = {0};
    int ret = 0;
    while (true) {
        int c = getch();
        if (c == 72 || c == 0 || c == 68) {
            continue;
        }
        printf("%s: ", userName);
        gets(buf);
        ret = send(*(SOCKET *) param, buf, sizeof(buf), 0);
        if (ret == SOCKET_ERROR) {
            return 1;
        }
    }
    // return 0;
}

// Connect to the chatting server
int ConnectChatServer() {
    WSADATA wsaData = {0};
    auto ClientSocket = INVALID_SOCKET;
    SOCKADDR_IN ServerAddr = {0};
    USHORT uPort = 18000;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
        return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("wVersion was not 2.2\n");
        return -1;
    }

    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ClientSocket == INVALID_SOCKET) {
        printf("socket failed with error code: %d\n", WSAGetLastError());
        return -1;
    }

    char IP[32] = {0};
    strcpy(IP, "127.0.0.1");

    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(uPort);
    ServerAddr.sin_addr.S_un.S_addr = inet_addr(IP);

    printf("connecting......\n");

    if (SOCKET_ERROR == connect(ClientSocket, (SOCKADDR *) &ServerAddr, sizeof(ServerAddr))) {
        printf("connect failed with error code: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return -1;
    }
    printf("Connecting server successfully IP:%s Port:%d\n", IP, htons(ServerAddr.sin_port));
    printf("Please input your UserName: ");
    gets(userName);
    send(ClientSocket, userName, sizeof(userName), 0);
    printf("\n");
    _beginthreadex(nullptr, 0, ThreadRecv, &ClientSocket, 0, nullptr); //启动接收和发送消息线程
    _beginthreadex(nullptr, 0, ThreadSend, &ClientSocket, 0, nullptr);
    for (int k = 0; k < 1000; k++) {
        Sleep(10000000);
    }
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}

int main() {
    ConnectChatServer();
    return 0;
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic pop