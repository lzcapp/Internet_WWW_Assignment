// Suppress the warning message C4996, function superseded by newer functionality
#pragma clang diagnostic ignored "-Wnonportable-include-path"
#pragma warning(disable:4996)

// Places a (static) library-search record in the object file
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <winsock.h>
/*
#include <winsock2.h>
*/
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>

// Define macros for our web server
#define DEFAULT_PORT 8080
#define BUF_LENGTH 1024
#define MIN_BUF 128
#define USER_ERROR -1
#define SERVER "Server: csr_http1.1\r\n"

int file_not_found(SOCKET sAccept);

int method_not_implemented(SOCKET sAccept);

int file_ok(SOCKET sAccept, long flen);

/*
int customized_error_page(SOCKET sAccept);
*/

int send_file(SOCKET sAccept, FILE *resource);

// DWORD: "Double Word", 32-bit unsigned integer
// LPVOID: pointer to a void object, Windows API typedef for void*
DWORD WINAPI SimpleHTTPServer(LPVOID lparam) {
    SOCKET sAccept = (SOCKET) lparam;
    char recv_buf[BUF_LENGTH];
    char method[MIN_BUF];
    char url[MIN_BUF];
    char path[_MAX_PATH];
    int i, j;

    // Clear the buffer
    memset(recv_buf, 0, sizeof(recv_buf));

    if (recv(sAccept, recv_buf, sizeof(recv_buf), 0) == SOCKET_ERROR) // Connection failed
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
    if (stricmp(method, "GET") != 0 && stricmp(method, "HEAD") != 0) {
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
    if (strcmp(url, "\\") == 0) {
        strcpy(url, "\\index.html");
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
    if (resource == NULL) {
        file_not_found(sAccept);
        // 如果method是GET，则发送自定义的file not found页面
        /*
        if (0 == stricmp(method, "GET")) {
            send_not_found(sAccept);
        }
        */
        closesocket(sAccept); //释放连接套接字，结束与该客户的通信
        printf("404 Not Found.\nSocket connection closed.\n");
        printf("====================\n\n");
        return (DWORD) USER_ERROR;
    }

    // Calculate the length of file
    fseek(resource, 0, SEEK_SET);
    fseek(resource, 0, SEEK_END);
    long flen = ftell(resource);
    printf("file length: %ld\n", flen);
    fseek(resource, 0, SEEK_SET);

    // Respond with "200 OK"
    file_ok(sAccept, flen);

    // If the method is "GET", Send the requested file
    if (0 == stricmp(method, "GET")) {
        if (0 == send_file(sAccept, resource)) {
            printf("file send successfully.\n");
        } else {
            printf("file send unsuccessfully.\n");
        }
    }
    fclose(resource);

    // Close the socket communication (connection)
    closesocket(sAccept);
    printf("200 OK.\nSocket connection closed.\n");
    printf("====================\n\n");

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

// 200 OK
int file_ok(SOCKET sAccept, long flen) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "HTTP/1.1 200 OK\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "Connection: keep-alive\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, SERVER);
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "Content-Length: %ld\r\n", flen);
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "Content-Type: text/html\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    return 0;
}

/*
// Customized error page
int customized_error_page(SOCKET sAccept) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "<BODY><h1 align='center'>404</h1><br/><h1 align='center'>file not found.</h1>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "</BODY></HTML>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    return 0;
}
*/

// Send requested resources
int send_file(SOCKET sAccept, FILE *resource) {
    char send_buf[BUF_LENGTH];
    while (1) {
        memset(send_buf, 0, sizeof(send_buf)); // Flush the buffer
        fgets(send_buf, sizeof(send_buf), resource);
        //  printf("send_buf: %s\n",send_buf);
        if (SOCKET_ERROR == send(sAccept, send_buf, (int) strlen(send_buf), 0)) {
            printf("send() Failed:%d\n", WSAGetLastError());
            return USER_ERROR;
        }
        if (feof(resource)) {
            return 0;
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET sListen, sAccept;
    int serverport = DEFAULT_PORT;
    struct sockaddr_in server, client;
    int iLen;

    printf("--------------------\n");
    printf("Server Listening... \n");
    printf("--------------------\n");

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
    server.sin_family = AF_INET;
    server.sin_port = htons((u_short) serverport);
    // htonl(): converts the unsigned integer hostlong from host byte order to network byte order.
    // INADDR_ANY: binds the socket to all available interfaces
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind listening socket to the server address
    if (bind(sListen, (LPSOCKADDR) &server, sizeof(server)) == SOCKET_ERROR) {
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
        iLen = sizeof(client);
        sAccept = accept(sListen, (struct sockaddr *) &client, &iLen);
        if (sAccept == INVALID_SOCKET) {
            printf("accept() Failed:%d\n", WSAGetLastError());
            break;
        }
        // Create thread for client browser's request
        DWORD ThreadID;
        CreateThread(NULL, 0, SimpleHTTPServer, (LPVOID) sAccept, 0, &ThreadID);
    }
    closesocket(sListen);
    WSACleanup();
    return 0;
}