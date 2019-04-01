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
#include <stdlib.h>
#include <io.h>
#include <ctype.h>


#define BUFF_SIZE 1024
#define SERV_PORT 8080

#define MIN_BUF 128
#define USER_ERROR -1
#define SERVER "Server: derpy\r\n"


int file_not_found(SOCKET sAccept);

int method_not_implemented(SOCKET sAccept);

int file_ok(SOCKET sAccept, long flen, char *path);

int sendFile(SOCKET sAccept, FILE *resource);

int sendDynamicPage(SOCKET sAccept);

int customized_error_page(SOCKET sAccept);

int searchTag(FILE *fp, char *tagName, const char *ipAddr);

char *trim(char *str);

int matchXMLList(const char *listType, const char *ipAddr);

char *matchXMLClass(const char *ipAddr, char *result);


// 403 Forbidden
int forbidden(SOCKET sAccept);

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
                {".css",      "text/css"},
                {".js",       "application/javascript"},
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

int matchXMLList(const char *listType, const char *ipAddr) {
    char path[_MAX_PATH];
    char fileLoc[] = "\\ipList\\";
    _getcwd(path, _MAX_PATH);
    strcat(path, fileLoc);
    strcat(path, "ip");
    strcat(path, listType);
    strcat(path, ".xml");
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Open xml file failed.\n");
        return -1;
    }
    FILE *fpTag, *fpSearch;
    char buff[50];
    while (fgets(buff, 50, fp)) {
        if (strstr(buff, "<ip>") != NULL) {
            fpTag = fp;
            fpSearch = fp;
            if (searchTag(fpTag, "ip", ipAddr) == 0) {
                // Get match in xml
                while (fgets(buff, 50, fpSearch)) {
                    if (strstr(buff, "<enable>") != NULL) {
                        fgets(buff, 50, fpSearch);
                        char *temp;
                        return strtol(buff, &temp, 10);
                    }
                }
            }
        }
    }
    return -1;
}

DWORD WINAPI SimpleHTTPServer(LPVOID lparam) {
    SOCKET sAccept = (SOCKET) (LPVOID) lparam;
    char recv_buf[BUFF_SIZE];
    char method[MIN_BUF];
    char url[MIN_BUF];
    char path[_MAX_PATH];
    int i, j;

    // Clear the buffer
    memset(recv_buf, 0, sizeof(recv_buf));

    if (recv(sAccept, recv_buf, sizeof(recv_buf), 0) == SOCKET_ERROR)   //接收错误
    {
        printf("recv() Failed:%d\n", WSAGetLastError());
        return (DWORD) USER_ERROR;
    } else { // Connection Successful
        printf("recv data from client:%s\n", recv_buf);
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
        return (DWORD) USER_ERROR;
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
        strcat(url, "index.html");
    }
    if (strcmp(url, "\\info") == 0) {
        isDynamic = true;
    }
    printf("url: %s\n", url);

    if (strcmp(url, "\\public.html") == 0) {
        if (matchXMLList("BlackList", clientAddr) == 1) {
            forbidden(sAccept);
            closesocket(sAccept);
            printf("403 Forbidden.\nSocket connection closed.\n");
            printf("====================\n\n");
            return (DWORD) USER_ERROR;
        }
    }

    if (strcmp(url, "\\insider.html") == 0) {
        if (matchXMLList("WhiteList", clientAddr) == -1) {
            forbidden(sAccept);
            closesocket(sAccept);
            printf("403 Forbidden.\nSocket connection closed.\n");
            printf("====================\n\n");
            return (DWORD) USER_ERROR;
        }
    }

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

        if (strcmp(method, "GET") == 0) {
            customized_error_page(sAccept);
        }
        closesocket(sAccept);
        printf("404 Not Found.\nSocket connection closed.\n");
        printf("====================\n\n");
        return (DWORD) USER_ERROR;
    }

    // Calculate the length of file
    long flen;
    if (isDynamic == true) {
        flen = BUFF_SIZE;
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
    if (strcmp(method, "GET") == 0) {
        if (isDynamic == true) {
            if (sendDynamicPage(sAccept) == 0) {
                printf("dynamic webpage send successfully.\n");
            } else {
                printf("dynamic webpage send failed.\n");
            }
        } else {
            if (sendFile(sAccept, resource) == 0) {
                printf("file send successfully.\n");
            } else {
                printf("file send failed.\n");
            }
        }
        char buffer[BUFF_SIZE];
        memset(buffer, 0, BUFF_SIZE);
    }
    fclose(resource);
    // Close the socket communication (connection)
    closesocket(sAccept);
    printf("200 OK.\nSocket connection closed.\n");
    printf("====================\n\n");

    return 0;
}

// 403 Forbidden
int forbidden(SOCKET sAccept) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "HTTP/1.1 403 Forbidden\r\n");
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

// 404 NOT FOUND
int file_not_found(SOCKET sAccept) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "HTTP/1.1 404 NOT FOUND\r\n");
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
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "Connection: keep-alive\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, SERVER);
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "Content-Length: %ld\r\n", flen);
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    char *type = NULL;
    if (isDynamic == true) {
        sprintf(type, "text/html");
    } else {
        type = getExpansion(path);
    }
    sprintf(send_buf, "Content-Type: %s\r\n", type);
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    return 0;
}

// Customized error page
int customized_error_page(SOCKET sAccept) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "<HTML><TITLE>ERROR</TITLE>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "<BODY><h1 align='center'>404</h1><br/><h1 align='center'>file can not found.</h1>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "</BODY></HTML>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    return 0;
}

// Send requested resources
int sendFile(SOCKET sAccept, FILE *resource) {
    char send_buf[BUFF_SIZE];
    size_t bytes_read = 0;
    while (1) {
        memset(send_buf, 0, sizeof(send_buf));       //缓存清0
        bytes_read = fread(send_buf, sizeof(char), sizeof(send_buf), resource);
        //  printf("send_buf: %s\n",send_buf);
        if (SOCKET_ERROR == send(sAccept, send_buf, (int) bytes_read, 0)) {
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
    char response[BUFF_SIZE];
    int len = BUFF_SIZE;
    memset(response, 0, sizeof(response));
    strcat(response, "<html><head><title>Hello from PC</title>");
    strcat(response, "<meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"5\"/>");
    strcat(response, "<style type=\"text/css\">body {font-family:\"Times New Roman\";text-align: center;");
    strcat(response, "background-color: #07080C;");
    strcat(response, "color: rgb(114, 255, 109);");
    strcat(response, "margin-left: 10em;");
    strcat(response, "margin-right: 10em;}");
    strcat(response, "h1{font-size: 3em;} h2{font-size: 2.5em;font-style: italic;}");
    strcat(response, "p{font-size: 1.8em;text-align: left;margin-left: 10em;margin-right: 10em;}");
    strcat(response, "</style></head>\r\n");
    strcat(response, "<body><h1>Alloha, World!</h1>\r\n");
    strcat(response, "<h2>This is a C WebServer on PC.</h2>\r\n");
    strcat(response, "<p>· Server IP: ");
    strcat(response, serverAddr);
    strcat(response, ":");
    strcat(response, serverPort);
    strcat(response, "</p>\r\n");
    strcat(response, "<p>· Client IP: ");
    strcat(response, clientAddr);
    strcat(response, ":");
    strcat(response, clientPort);
    strcat(response, "</p>\r\n");
    strcat(response, "<p>· Client is: ");
    char result[100];
    matchXMLClass(clientAddr, result);
    strcpy(result, result + 3);
    strcat(response, result);
    strcat(response, "</p>\r\n");
    strcat(response, "<p>· Client in ");
    strcat(response, "<br/>&emsp;&emsp;· Black List: ");
    char isBlack[4], isWhite[4];
    if (matchXMLList("BlackList", clientAddr) == 0) {
        strcpy(isBlack, "No");
    } else {
        strcpy(isBlack, "Yes");
    }
    strcat(response, isBlack);
    if (matchXMLList("WhiteList", clientAddr) == 0) {
        strcpy(isWhite, "No");
    } else {
        strcpy(isWhite, "Yes");
    }
    strcat(response, "<br/>&emsp;&emsp;· White List: ");
    strcat(response, isWhite);
    strcat(response,
           "<br/><br/><img src=\"http://www.zhmb.org.cn/jeecms/static/common/images/sj4.jpg\" width=\"100%\">");
    strcat(response, "</body>");
    if (SOCKET_ERROR == send(sAccept, response, len, 0)) {
        printf("send() Failed:%d\n", WSAGetLastError());
        return USER_ERROR;
    }
    isDynamic = false;
    return 0;
}

char *matchXMLClass(const char *ipAddr, char *result) {
    char path[_MAX_PATH];
    char fileLoc[] = "\\ipList\\ipClass.xml";
    _getcwd(path, _MAX_PATH);
    strcat(path, fileLoc);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Open xml file failed.\n");
        return "Failed";
    }
    FILE *fpTag;
    char buff[50];
    while (fgets(buff, 50, fp)) {
        if (strstr(buff, "<class>") != NULL) {
            fpTag = fp;
            if (searchTag(fpTag, "class", ipAddr) == 0) {
                // Get match in xml
                while (fgets(buff, 50, fp)) {
                    if (strstr(buff, "<description>") != NULL) {
                        fgets(result, 100, fp);
                        return trim(result);
                    }
                }
            }
        }
    }
    return "ERROR";
}

int searchTag(FILE *fp, char *tagName, const char *ipAddr) {
    char buff[50];
    char endTag[10];
    strcat(endTag, "</");
    strcat(endTag, tagName);
    strcat(endTag, ">");
    while (fgets(buff, 50, fp)) {
        if (strstr(buff, endTag) == NULL) {
            if (strstr(buff, ipAddr) != NULL) {
                return 0;
            }
        } else {
            return -1;
        }
    }
    return -1;
}

char *trim(char *str) {
    int start, end, i;
    if (str) {
        for (start = 0; isspace(str[start]); start++) {}
        for (end = (int) (strlen(str) - 1); isspace(str[end]); end--) {}
        for (i = start; i <= end; i++) {
            str[i - start] = str[i];
        }
        str[end - start + 1] = '\0';
        return (str);
    } else {
        return NULL;
    }
}

int main() {
    WSADATA wsaData;
    SOCKET sListen, sAccept;        //服务器监听套接字，连接套接字
    int serverport = SERV_PORT;  //服务器端口号
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
    ser.sin_port = htons((u_short) serverport);          //服务器端口号
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
