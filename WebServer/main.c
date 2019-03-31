#pragma clang diagnostic push

#pragma clang diagnostic ignored "-Wpointer-bool-conversion"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <winsock.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <stdbool.h>
#include <stdint.h>


#define DEFAULT_PORT 8080
#define BUF_LENGTH 1024
#define MIN_BUF 128
#define USER_ERROR -1
#define SERVER "Server: derpy\r\n"


int file_not_found(SOCKET sAccept);

int method_not_implemented(SOCKET sAccept);

int file_ok(SOCKET sAccept, long flen, char *path);

int sendFile(SOCKET sAccept, FILE *resource);

int sendDynamicPage(SOCKET sAccept);

int customized_error_page(SOCKET sAccept);

char *matchXMLWhitelist(const char *ipAddr);

char *matchXMLBlacklist(const char *ipAddr);

char *matchXMLClass(const char *ipAddr);


struct fileType {
    char expansion[100];
    char type[100];
};

struct fileType file_type[] =
        {
                {".html",     "text/html"},
                {".gif",      "imag/gif"},
                {".jpg",      "image/jpg"},
                {".png",      "image/png"},
                {".mp3",      "audio/mp3"},
                {".mp4",      "video/mp4"},
                {".ico",      "image/x-icon"},
                {(char) NULL, (char) NULL}
        };

boolean isDynamic = false;

char serverAddr[32];
char serverPort[5];
char clientAddr[32];
char clientPort[5];

char *getExpansion(const char *expansion) {
    struct fileType *type;
    for (type = file_type; type->expansion; type++) {
        if (strcmp(type->expansion, expansion) == 0) {
            return type->type;
        }
    }
    return NULL;
}


DWORD WINAPI SimpleHTTPServer(LPVOID lparam) {
    SOCKET sAccept = (SOCKET) (LPVOID) lparam;
    char recv_buf[BUF_LENGTH];
    char method[MIN_BUF];
    char url[MIN_BUF];
    char path[_MAX_PATH];
    int i, j;

    // Clear the buffer
    memset(recv_buf, 0, sizeof(recv_buf));

    if (recv(sAccept, recv_buf, sizeof(recv_buf), 0) == SOCKET_ERROR)   //接收错误
    {
        printf("recv() Failed:%d\n", WSAGetLastError());
        return USER_ERROR;
    } else { // Connection Successful
        printf("recv data from client:%s\n", recv_buf);
    }

    if ((clientAddr, "") != 0) {
        method_not_implemented(sAccept);
        closesocket(sAccept);
        printf("501 Not Implemented.\nSocket connection closed.\n");
        printf("====================\n\n");
        return USER_ERROR;
    }

    // Handle the data received
    i = 0;
    j = 0;
    // Read Method out of the request-line
    while (' ' != recv_buf[j] && (i < sizeof(method) - 1)) {
        method[i] = recv_buf[j];
        i++;
        j++;
    }
    method[i] = '\0';

    // If the method is either not "GET" or "HEAD", respond with "501 Not Implemented"
    if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0) {
        method_not_implemented(sAccept);
        closesocket(sAccept);
        printf("501 Not Implemented.\nSocket connection closed.\n");
        printf("====================\n\n");
        return USER_ERROR;
    }
    printf("method: %s\n", method);

    // Read Request-URI out of the request-line
    // Convert path separators '/' into directory separator '\' on Windows.
    // Only static requests are currently accepted
    i = 0;
    while ((' ' == recv_buf[j]) && (j < sizeof(recv_buf))) {
        j++;
    }
    while (' ' != recv_buf[j] && (i < sizeof(recv_buf) - 1) && (j < sizeof(recv_buf))) {
        if (recv_buf[j] == '/') {
            url[i] = '\\';
        } else if (recv_buf[j] == ' ') {
            break;
        } else {
            url[i] = recv_buf[j];
        }
        i++;
        j++;
    }
    url[i] = '\0';

    // By default it's index.html
    if (strcmp(url, "\\") == 0) {
        isDynamic = true;
    }
    printf("url: %s\n", url);

    // _getcwd: Gets the current working directory
    _getcwd(path, _MAX_PATH);
    strcat(path, url);
    printf("path: %s\n", path);

    // Open the requested file with "rb" mode: Open file for reading.
    // "r" mode should do the same, but cases on the Internets says it won't work
    FILE *resource = fopen(path, "rb");

    // if the file is not exist, respond with "404 Not Found"
    if (resource == NULL && isDynamic == false) {
        file_not_found(sAccept);
        // 如果method是GET，则发送自定义的file not found页面
        if (0 == strcmp(method, "GET")) {
            customized_error_page(sAccept);
        }
        closesocket(sAccept); //释放连接套接字，结束与该客户的通信
        printf("404 Not Found.\nSocket connection closed.\n");
        printf("====================\n\n");
        return USER_ERROR;
    }

    // Calculate the length of file
    long flen;
    if (isDynamic == true) {
        flen = BUF_LENGTH;
    } else {
        fseek(resource, 0, SEEK_SET);
        fseek(resource, 0, SEEK_END);
        flen = ftell(resource);
        printf("file length: %ld\n", flen);
        fseek(resource, 0, SEEK_SET);
    }

    // Respond with "200 OK"
    char *pFile;
    pFile = strrchr(path, '.');

    file_ok(sAccept, flen, pFile);

    // If the method is "GET", Send the requested file
    if (0 == strcmp(method, "GET")) {
        if (isDynamic == true) {
            if (0 == sendDynamicPage(sAccept)) {
                printf("dynamic webpage send successfully.\n");
            } else {
                printf("dynamic webpage send failed.\n");
            }
        } else {
            if (0 == sendFile(sAccept, resource)) {
                printf("file send successfully.\n");
            } else {
                printf("file send failed.\n");
            }
        }
        char buffer[BUF_LENGTH];
        memset(buffer, 0, BUF_LENGTH);
    }
    fclose(resource);
    // Close the socket communication (connection)
    closesocket(sAccept);
    printf("200 OK.\nSocket connection closed.\n");
    printf("====================\n\n");

    return 0;

}


// 发送404 file_not_found报头
int file_not_found(SOCKET sAccept) {
    char send_buf[MIN_BUF];
    //  time_t timep;
    //  time(&timep);
    sprintf(send_buf, "HTTP/1.1 404 NOT FOUND\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    //  sprintf(send_buf, "Date: %s\r\n", ctime(&timep));
    //  send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Connection: keep-alive\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, SERVER);
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Content-Type: text/html\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    return 0;
}

// 501 NOT Implemented
int method_not_implemented(SOCKET sAccept) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "HTTP/1.1 501 NOT Implemented\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "Connection: keep-alive\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, SERVER);
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "Content-Type: text/html\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    return 0;
}

// 发送200 ok报头
int file_ok(SOCKET sAccept, long flen, char *path) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "HTTP/1.1 200 OK\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Connection: keep-alive\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, SERVER);
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "Content-Length: %ld\r\n", flen);
    send(sAccept, send_buf, strlen(send_buf), 0);
    char *type = NULL;
    if (isDynamic == true) {
        sprintf(type, "text/html");
    } else {
        type = getExpansion(path);
    }
    sprintf(send_buf, "Content-Type: %s\r\n", type);
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    return 0;
}

// Customized error page
int customized_error_page(SOCKET sAccept) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "<BODY><h1 align='center'>404</h1><br/><h1 align='center'>file can not found.</h1>\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    sprintf(send_buf, "</BODY></HTML>\r\n");
    send(sAccept, send_buf, strlen(send_buf), 0);
    return 0;
}

// Send requested resources
int sendFile(SOCKET sAccept, FILE *resource) {
    char send_buf[BUF_LENGTH];
    size_t bytes_read = 0;
    while (1) {
        memset(send_buf, 0, sizeof(send_buf));       //缓存清0
        bytes_read = fread(send_buf, sizeof(char), sizeof(send_buf), resource);
        //  printf("send_buf: %s\n",send_buf);
        if (SOCKET_ERROR == send(sAccept, send_buf, bytes_read, 0)) {
            printf("send() Failed:%d\n", WSAGetLastError());
            return USER_ERROR;
        }
        if (feof(resource)) {
            return 0;
        }
    }
}

// Send dynamic webpage
int sendDynamicPage(SOCKET sAccept) {
    char response[BUF_LENGTH];
    int len = BUF_LENGTH;
    memset(response, 0, sizeof(response));
    strcat(response,
           "<html lang='en'><head><title>Hello from PC</title><meta charset=\"utf-8\"/></head>");
    strcat(response, "");
    strcat(response, "<body><h1>Alloha, World!</h1>\r\n");
    strcat(response, "<p>Server IP: ");
    strcat(response, serverAddr);
    strcat(response, ":");
    strcat(response, serverPort);
    strcat(response, "</p>\r\n");
    strcat(response, "<p>Client IP: ");
    strcat(response, clientAddr);
    strcat(response, ":");
    strcat(response, clientPort);
    strcat(response, "</p>\r\n\r\n");
    strcat(response, "<h2>This is a C WebServer on PC.</h2>\r\n");
    if (SOCKET_ERROR == send(sAccept, response, len, 0)) {
        printf("send() Failed:%d\n", WSAGetLastError());
        return USER_ERROR;
    }
    return 0;
}

char *matchXMLClass(const char *ipAddr) {
    char path[] = "\\ipList\\ipClass.xml";
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Open xml file failed.\n");
        return "Failure";
    }
    char buff[50];
    while (fgets(buff, 50, fp)) {
        if (strstr(buff, "<class>") != NULL) {

        }
    }
    getchar();
}

int main() {
    WSADATA wsaData;
    SOCKET sListen, sAccept;        //服务器监听套接字，连接套接字
    int serverport = DEFAULT_PORT;  //服务器端口号
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
    ser.sin_port = htons(serverport);          //服务器端口号
    ser.sin_addr.s_addr = htonl(INADDR_ANY);   //服务器IP地址，默认使用本机IP

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
    while (1) {
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
        DWORD ThreadID;
        CreateThread(NULL, 0, SimpleHTTPServer, (LPVOID) sAccept, 0, &ThreadID);
    }
    closesocket(sListen);
    WSACleanup();
    return 0;
}
