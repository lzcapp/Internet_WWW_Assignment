#include <winsock2.h>
#include <process.h>
#include <cstdio>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#pragma warning (disable: 4996)
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#define SEND_OVER true
#define SEND_YET  false

int g_iStatus = SEND_YET;
auto g_ServerSocket = INVALID_SOCKET;
SOCKADDR_IN g_ClientAddr = {0};
int g_iClientAddrLen = sizeof(g_ClientAddr);
// bool g_bCheckConnect = false;
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

unsigned __stdcall ThreadSend(void *param) {
    int ret = 0;
    int flag = *(int *) param;
    // auto client = INVALID_SOCKET;
    char temp[128] = {0};
    memcpy(temp, g_Client[!flag].buf, sizeof(temp));
    sprintf(g_Client[flag].buf, "%s: %s", g_Client[!flag].userName, temp);

    if (strlen(temp) != 0 && g_iStatus == SEND_YET) {
        ret = send(g_Client[flag].sClient, g_Client[flag].buf, sizeof(g_Client[flag].buf), 0);
    }
    if (ret == SOCKET_ERROR) {
        return 1;
    }
    g_iStatus = SEND_OVER;
    return 0;
}

unsigned __stdcall ThreadRecv(void *param) {
    auto client = INVALID_SOCKET;
    int flag = 0;
    if (*(int *) param == g_Client[0].flag)
    {
        client = g_Client[0].sClient;
        // flag = 0;
    } else if (*(int *) param == g_Client[1].flag) {
        client = g_Client[1].sClient;
        // flag = 1;
    }
    char temp[128] = {0};  //临时数据缓冲区
    while (true) {
        memset(temp, 0, sizeof(temp));
        int ret = recv(client, temp, sizeof(temp), 0); //接收数据
        if (ret == SOCKET_ERROR) {
            continue;
        }
        g_iStatus = SEND_YET;                                //设置转发状态为未转发
        flag = client == g_Client[0].sClient ? 1 : 0;        //这个要设置，否则会出现自己给自己发消息的BUG
        memcpy(g_Client[!flag].buf, temp, sizeof(g_Client[!flag].buf));
        _beginthreadex(nullptr, 0, ThreadSend, &flag, 0, nullptr); //开启一个转发线程,flag标记着要转发给哪个客户端
        //这里也可能是导致CPU使用率上升的原因。
    }
    // return 0;
}

unsigned __stdcall ThreadManager(void *param) {
    while (true) {
        if (send(g_Client[0].sClient, "", sizeof(""), 0) == SOCKET_ERROR) {
            if (g_Client[0].sClient != 0) {
                CloseHandle(g_hRecv1); //这里关闭了线程句柄，但是测试结果断开连C/S接后CPU仍然疯涨
                CloseHandle(g_hRecv2);
                printf("Disconnect from IP: %s,UserName: %s\n", g_Client[0].IP, g_Client[0].userName);
                closesocket(g_Client[0].sClient);   //这里简单的判断：若发送消息失败，则认为连接中断(其原因有多种)，关闭该套接字
                g_Client[0] = {0};
            }
        }
        if (send(g_Client[1].sClient, "", sizeof(""), 0) == SOCKET_ERROR) {
            if (g_Client[1].sClient != 0) {
                CloseHandle(g_hRecv1);
                CloseHandle(g_hRecv2);
                printf("Disconnect from IP: %s,UserName: %s\n", g_Client[1].IP, g_Client[1].userName);
                closesocket(g_Client[1].sClient);
                g_Client[1] = {0};
            }
        }
        Sleep(2000); //2s检查一次
    }
    // return 0;
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
            //如果有客户端申请连接就接受连接
            if ((g_Client[i].sClient = accept(g_ServerSocket, (SOCKADDR *) &g_ClientAddr, &g_iClientAddrLen)) ==
                INVALID_SOCKET) {
                printf("accept failed with error code: %d\n", WSAGetLastError());
                closesocket(g_ServerSocket);
                WSACleanup();
                return -1;
            }
            recv(g_Client[i].sClient, g_Client[i].userName, sizeof(g_Client[i].userName), 0); //接收用户名
            printf("Successfuuly got a connection from IP:%s ,Port: %d,UerName: %s\n",
                   inet_ntoa(g_ClientAddr.sin_addr), htons(g_ClientAddr.sin_port), g_Client[i].userName);
            memcpy(g_Client[i].IP, inet_ntoa(g_ClientAddr.sin_addr), sizeof(g_Client[i].IP)); //记录客户端IP
            g_Client[i].flag = g_Client[i].sClient; //不同的socke有不同UINT_PTR类型的数字来标识
            i++;
        }
        i = 0;

        if (g_Client[0].flag != 0 && g_Client[1].flag != 0)                  //当两个用户都连接上服务器后才进行消息转发
        {
            if (g_Client[0].flag != temp1)     //每次断开一个连接后再次连上会新开一个线程，导致cpu使用率上升,所以要关掉旧的
            {
                if (g_hRecv1) {                  //这里关闭了线程句柄，但是测试结果断开连C/S接后CPU仍然疯涨
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

        temp1 = g_Client[0].flag; //防止ThreadRecv线程多次开启
        temp2 = g_Client[1].flag;

        Sleep(3000);
    }
    // return 0;
}

int StartServer() {
    //存放套接字信息的结构
    WSADATA wsaData = {0};
    SOCKADDR_IN ServerAddr = {0};             //服务端地址
    USHORT uPort = 18000;                       //服务器监听端口

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

    _beginthreadex(nullptr, 0, ThreadAccept, nullptr, 0, nullptr);
    for (int k = 0; k < 100; k++) { //让主线程休眠，不让它关闭TCP连接.
        Sleep(10000000);
    }

    //关闭套接字
    for (auto &j : g_Client) {
        if (j.sClient != INVALID_SOCKET) {
            closesocket(j.sClient);
        }
    }
    closesocket(g_ServerSocket);
    WSACleanup();
    return 0;
}

int main() {
    StartServer();
    return 0;
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop