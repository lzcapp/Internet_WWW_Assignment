#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "OCDFAInspection"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

#pragma comment(lib, "ws2_32.lib")

#pragma warning (disable: 4996)

#include <winsock2.h>
#include <process.h>
#include <cstdio>
#include <cstdlib>
#include <conio.h>
#include <comutil.h>

// #include "des.h"
#include "des.cpp"
#include "client.h"

// #define RECV_OVER 1
// #define RECV_YET 0

char myName[128] = {0};
char urName[128] = {0};
// int iStatus = RECV_YET;


std::string unicode_string(const std::wstring &wstr) {
    std::string ret;
    std::mbstate_t state = {};
    const wchar_t *src = wstr.data();
    size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::unique_ptr< char [] > buff(new char[len + 1]);
        len = std::wcsrtombs(buff.get(), &src, len, &state);
        if (static_cast<size_t>(-1) != len) {
            ret.assign(buff.get(), len);
        }
    }
    return ret;
}

std::wstring string_unicode(const std::string & str) {
    std::wstring ret;
    std::mbstate_t state = {};
    const char *src = str.data();
    size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::unique_ptr< wchar_t [] > buff(new wchar_t[len + 1]);
        len = std::mbsrtowcs(buff.get(), &src, len, &state);
        if (static_cast<size_t>(-1) != len) {
            ret.assign(buff.get(), len);
        }
    }
    return ret;
}

unsigned __stdcall ThreadRecv(void *param) {
    char buf[262144];
    auto ClientSocket = *(SOCKET *) param;
    while (true) {
        int ret = recv(ClientSocket, buf, sizeof(buf), 0);
        if (ret == SOCKET_ERROR) {
            Sleep(500);
            continue;
        }
        if (strlen(buf) != 0) {
            /*char head[10] = {0};
            strncpy(head, buf, 4);
            if (strcmp(head, "NAME") == 0) {
                strcpy(urName, buf);
                char msg[128] = {0};
                strcpy(msg, "NAME: ");
                strcat(msg, myName);
                int sta = send(ClientSocket, msg, sizeof(msg), 0);
                if (sta == SOCKET_ERROR) {
                    return 1;
                }
            } else if (strcmp(head, "METH") == 0) {
                strcpy(urName, buf);
                char msg[128];
                strcpy(msg, "METHOD: DES");
                int sta = send(ClientSocket, msg, sizeof(msg), 0);
                if (sta == SOCKET_ERROR) {
                    return 1;
                }
            } else if (strcmp(head, "TEST") == 0) {
                char testCipher[128];
                ret = recv(ClientSocket, testCipher, sizeof(testCipher), 0);
                if (ret == SOCKET_ERROR) {
                    return -1;
                }
                if (strlen(testCipher) != 0) {
                    string tmp;
                    tmp = testCipher;
                    tmp = tmp.substr(5);
                    char *p = const_cast<char *>(tmp.data());
                    msg_decryption(p);
                    if (msg_result == "TEST_MSG_DES") {
                        printf("Connection Successful!\n");
                    }
                    char msg[128];
                    strcpy(msg, "TEST_MSG_DES");
                    int sta = send(ClientSocket, msg, sizeof(msg), 0);
                    if (sta == SOCKET_ERROR) {
                        return 1;
                    }
                } else {
                    return -1;
                }
            }*/
            msg_decryption(buf);
            //string msg = UTF8_To_string();
            basic_string<wchar_t> result = reinterpret_cast<basic_string<wchar_t> &&>(msg_result);
            string msg = unicode_string(result);
            printf("%s: %s\n", urName, msg.c_str());
            msg_result = "";
            // iStatus = RECV_OVER;
        } else {
            Sleep(100);
        }
    }
    // return 0;
}

//发送数据
unsigned __stdcall ThreadSend(void *param) {
    char input[1024];
    int ret = 0;
    while (true) {
        int c = getch();
        if (c == 72 || c == 0 || c == 68) {
            continue;
        }
        printf("%s: ", myName);
        gets(input);
        wstring msg = reinterpret_cast<basic_string<wchar_t> &&>(input);
        printf(input);
        printf("\n");
        msg = string_unicode(input);
        //std::cout << msg << std::endl;
        msg_result = "";
        char *buf = (char *) msg.c_str();
        msg_encryption(buf);
        // std::cout << "" << std::endl;
        // std::cout << msg_result << std::endl;
        char *msg_en = (char *) msg_result.c_str();
        //char *msg_en = const_cast<char *>(msg.c_str());
        //printf("%d\n", msg_result.length());
        ret = send(*(SOCKET *) param, msg_en, msg_result.length(), 0);
        //ret = send(*(SOCKET *) param, msg_en, sizeof(msg_en), 0);
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
    // Server at localhost 127.0.0.1
    strcpy(IP, "127.0.0.1");

    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(uPort);
    ServerAddr.sin_addr.S_un.S_addr = inet_addr(IP);

    printf("Connecting...\n");

    if (SOCKET_ERROR == connect(ClientSocket, (SOCKADDR *) &ServerAddr, sizeof(ServerAddr))) {
        printf("connect failed with error code: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return -1;
    }
    printf("Connect to chat server successfully.\n");
    printf("\t=> IP: %s, Port: %d\n\n", IP, htons(ServerAddr.sin_port));
    printf("My name is: ");
    gets(myName);
    sayHello(ClientSocket);
    _beginthreadex(nullptr, 0, ThreadRecv, &ClientSocket, 0, nullptr);
    _beginthreadex(nullptr, 0, ThreadSend, &ClientSocket, 0, nullptr);
    for (int k = 0; k < 1000; k++) {
        Sleep(10000000);
    }
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}

int sayHello(SOCKET ClientSocket) {
    string name;
    name += myName;
    auto msg = name.c_str();
    int sta = send(ClientSocket, msg, sizeof(msg), 0);
    if (sta == SOCKET_ERROR) {
        return 1;
    }
    char buf[128];
    int ret = recv(ClientSocket, buf, sizeof(buf), 0);
    printf("%s\n", buf);
    if (ret == SOCKET_ERROR) {
        return -1;
    }
    if (buf[0] == 'N') {
        strcpy(urName, buf);
    } else {
        return -1;
    }
    handShaking(ClientSocket);
}

int handShaking(SOCKET ClientSocket) {
    char msg[128];
    strcpy(msg, "METHOD: DES");
    int sta = send(ClientSocket, msg, sizeof(msg), 0);
    if (sta == SOCKET_ERROR) {
        return 1;
    }
    char buf[128];
    int ret = recv(ClientSocket, buf, sizeof(buf), 0);
    if (ret == SOCKET_ERROR) {
        return -1;
    }
    if (strlen(buf) != 0) {
        if (strcmp(buf, "METHOD: DES") == 0) {
            // genKey();
            // char key_send[128];
            // strcat(key_send, "KEY: ");
            // char *key = (char *) k.c_str();
            // strcat(key_send, key);
            // sta = send(ClientSocket, msg, sizeof(msg), 0);
            // if (sta == SOCKET_ERROR) {
            //    return 1;
            // }
            char temp_msg[1024] = {0};
            strcpy(temp_msg, "TEST_MSG_DES");
            msg_encryption(temp_msg);
            strcpy(msg, "TEST: ");
            strcat(msg, (char *) msg_result.c_str());
            int sta = send(ClientSocket, msg, sizeof(msg), 0);
            if (sta == SOCKET_ERROR) {
                return 1;
            }
            char testCipher[128];
            ret = recv(ClientSocket, testCipher, sizeof(testCipher), 0);
            if (ret == SOCKET_ERROR) {
                return -1;
            }
            if (strlen(testCipher) != 0) {
                if (strcmp(testCipher, "TEST_MSG_DES") == 0) {
                    printf("Connection Successful!\n");
                }
            } else {
                return -1;
            }
        }
    } else {
        return -1;
    }
}

int main() {
    ConnectChatServer();
    /*
    char msg[1024];
    strcpy(msg, "caonima");
    msg_encryption(msg);
    strcpy(msg, "813DB7B34102D5E2");
    msg_decryption(msg);
    */
    return 0;
}

// #pragma clang diagnostic pop //
#pragma clang diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic pop
#pragma clang diagnostic pop