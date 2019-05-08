#include <winsock2.h>
#include <process.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctime>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

#pragma warning (disable: 4996)
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#define ALREADY_SEND true
#define HAVENOT_SEND false

int g_iStatus = HAVENOT_SEND;
auto g_ServerSocket = INVALID_SOCKET;
SOCKADDR_IN g_ClientAddr = {0};
int g_iClientAddrLen = sizeof(g_ClientAddr);
HANDLE g_hRecv1 = nullptr;
HANDLE g_hRecv2 = nullptr;


typedef struct _Client {
    SOCKET sClient;
    char buf[128];
    char userName[16];
    char IP[20];
    UINT_PTR flag;
} Client;

Client g_Client[2] = {0};

void addLog(const std::string &addLine) {
    std::ofstream write;
    std::ifstream read;

    std::string line;
    time_t now = time(nullptr);
    std::string time = ctime(&now);
    time = time.substr(0, time.length() - 1); // Remove \n at the end
    line = time + "," + addLine;

    write.open("Chat.csv", std::ios::app);
    write << line << std::endl;
    write.close();
    read.close();
}

unsigned __stdcall ThreadSend(void *param) {
    int ret = 0;
    int flag = *(int *) param;
    // auto client = INVALID_SOCKET;
    char temp[128] = {0};
    memcpy(temp, g_Client[!flag].buf, sizeof(temp));
    // sprintf(g_Client[flag].buf, "%s: %s", g_Client[!flag].userName, temp);

    sprintf(g_Client[flag].buf, "%s", temp);
    printf("%s\n", g_Client[flag].buf);
    std::string logLine = "Client, ";
    logLine += g_Client[flag].IP;
    logLine += ":";
    logLine += g_Client[flag].userName;
    logLine += ", ";
    logLine += g_Client[flag].userName;
    logLine += ", Client send msg: ";
    logLine += g_Client[flag].buf;
    logLine += ".";
    addLog(logLine);

    if (strlen(temp) != 0 && g_iStatus == HAVENOT_SEND) {
        ret = send(g_Client[flag].sClient, g_Client[flag].buf, sizeof(g_Client[flag].buf), 0);
    }
    if (ret == SOCKET_ERROR) {
        return 1;
    }
    g_iStatus = ALREADY_SEND;
    return 0;
}

unsigned __stdcall ThreadRecv(void *param) {
    auto client = INVALID_SOCKET;
    int flag = 0;
    if (*(int *) param == g_Client[0].flag) {
        client = g_Client[0].sClient;
    } else if (*(int *) param == g_Client[1].flag) {
        client = g_Client[1].sClient;
    }
    char temp[128] = {0};
    while (true) {
        memset(temp, 0, sizeof(temp));
        int ret = recv(client, temp, sizeof(temp), 0);
        if (ret == SOCKET_ERROR) {
            continue;
        }
        g_iStatus = HAVENOT_SEND;
        flag = client == g_Client[0].sClient ? 1 : 0;
        memcpy(g_Client[!flag].buf, temp, sizeof(g_Client[!flag].buf));
        _beginthreadex(nullptr, 0, ThreadSend, &flag, 0, nullptr);
    }
}

unsigned __stdcall ThreadManager(void *param) {
    while (true) {
        if (send(g_Client[0].sClient, "", sizeof(""), 0) == SOCKET_ERROR) {
            if (g_Client[0].sClient != 0) {
                CloseHandle(g_hRecv1);
                CloseHandle(g_hRecv2);
                printf("Client Disconnected at IP: %s, %s.\n", g_Client[0].IP, g_Client[0].userName);
                std::string logLine = "Client, ";
                logLine += g_Client[0].IP;
                logLine += ", ";
                logLine += g_Client[0].userName;
                logLine += ", Client disconnected from server.";
                addLog(logLine);
                closesocket(g_Client[0].sClient);
                g_Client[0] = {0};
            }
        }
        if (send(g_Client[1].sClient, "", sizeof(""), 0) == SOCKET_ERROR) {
            if (g_Client[1].sClient != 0) {
                CloseHandle(g_hRecv1);
                CloseHandle(g_hRecv2);
                printf("Client Disconnected at IP: %s, %s.\n", g_Client[1].IP, g_Client[1].userName);
                std::string logLine = "Client, ";
                logLine += g_Client[1].IP;
                logLine += ", ";
                logLine += g_Client[1].userName;
                logLine += ", Client disconnected from server.";
                addLog(logLine);
                closesocket(g_Client[1].sClient);
                g_Client[1] = {0};
            }
        }
        Sleep(1000);
    }
}

unsigned __stdcall ThreadAccept(void *param) {
    int i = 0;
    int temp1 = 0, temp2 = 0;
    _beginthreadex(nullptr, 0, ThreadManager, nullptr, 0, nullptr);
    while (true) {
        while (i < 2) {
            if (g_Client[i].flag != 0) {
                ++i;
                continue;
            }

            // Accept client's connection
            if ((g_Client[i].sClient = accept(g_ServerSocket, (SOCKADDR *) &g_ClientAddr, &g_iClientAddrLen)) ==
                INVALID_SOCKET) {
                printf("accept failed with error code: %d\n", WSAGetLastError());
                closesocket(g_ServerSocket);
                WSACleanup();
                return -1;
            }
            recv(g_Client[i].sClient, g_Client[i].userName, sizeof(g_Client[i].userName), 0); //接收用户名
            printf("Successfully establish a connection from IP: %s, Port: %d, %s\n", inet_ntoa(g_ClientAddr.sin_addr),
                   htons(g_ClientAddr.sin_port), g_Client[i].userName);
            std::string logLine = "Client, ";
            logLine += inet_ntoa(g_ClientAddr.sin_addr);
            logLine += ":";
            logLine += std::to_string(htons(g_ClientAddr.sin_port));
            logLine += ", ";
            logLine += g_Client[i].userName;
            logLine += ", Client connected to server.";
            addLog(logLine);
            memcpy(g_Client[i].IP, inet_ntoa(g_ClientAddr.sin_addr), sizeof(g_Client[i].IP)); //记录客户端IP
            g_Client[i].flag = g_Client[i].sClient; //不同的socke有不同UINT_PTR类型的数字来标识
            i++;
        }
        i = 0;

        if (g_Client[0].flag != 0 && g_Client[1].flag != 0) {
            if (g_Client[0].flag != temp1) {
                if (g_hRecv1) {
                    CloseHandle(g_hRecv1);
                }
                g_hRecv1 = (HANDLE) _beginthreadex(nullptr, 0, ThreadRecv, &g_Client[0].flag, 0, nullptr); //开启2个接收消息的线程
            }
            if (g_Client[1].flag != temp2) {
                if (g_hRecv2) {
                    CloseHandle(g_hRecv2);
                }
                g_hRecv2 = (HANDLE) _beginthreadex(nullptr, 0, ThreadRecv, &g_Client[1].flag, 0, nullptr);
            }
        }

        temp1 = g_Client[0].flag;
        temp2 = g_Client[1].flag;

        Sleep(3000);
    }
}

int StartServer() {
    //存放套接字信息的结构
    WSADATA wsaData = {0};
    SOCKADDR_IN ServerAddr = {0}; // Sever Address
    USHORT uPort = 18000;         // Sever Port

    //初始化套接字
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
        return -1;
    }
    //判断版本
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("wVersion was not 2.2\n");
        return -1;
    }
    //创建套接字
    g_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_ServerSocket == INVALID_SOCKET) {
        printf("socket failed with error code: %d\n", WSAGetLastError());
        return -1;
    }

    //设置服务器地址
    ServerAddr.sin_family = AF_INET;//连接方式
    ServerAddr.sin_port = htons(uPort);//服务器监听端口
    ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//任何客户端都能连接这个服务器

    //绑定服务器
    if (SOCKET_ERROR == bind(g_ServerSocket, (SOCKADDR *) &ServerAddr, sizeof(ServerAddr))) {
        printf("bind failed with error code: %d\n", WSAGetLastError());
        closesocket(g_ServerSocket);
        return -1;
    }
    //设置监听客户端连接数
    if (SOCKET_ERROR == listen(g_ServerSocket, 20000)) {
        printf("listen failed with error code: %d\n", WSAGetLastError());
        closesocket(g_ServerSocket);
        WSACleanup();
        return -1;
    }

    addLog("Server, localhost, , Server started.");

    _beginthreadex(nullptr, 0, ThreadAccept, nullptr, 0, nullptr);
    for (int k = 0; k < 100; k++) {
        Sleep(10000000);
        // Keep the TCP connection on
    }

    for (auto &j : g_Client) {
        if (j.sClient != INVALID_SOCKET) {
            closesocket(j.sClient);
        }
    }
    closesocket(g_ServerSocket);
    WSACleanup();
    addLog("Server, localhost, , Server program ended.");
    return 0;
}

int main() {
    StartServer();
    return 0;
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop