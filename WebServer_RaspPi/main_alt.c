#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <jmorecfg.h>
#include <stdbool.h>
#include <pthread.h>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-bool-conversion"
#define MIN_BUF 128
#define _MAX_PATH 126
#define BUFF_SIZE 1024
#define SERV_PORT 8080
#define MAX_SIZE 32
#define int_ERROR -1
#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"
#define USER_ERROR -1
#define SERVER "Server: derpy\r\n"

char *http_res_tmpl = "HTTP/1.1 200 OK\r\n";

char serverAddr[32];
char serverPort[5];
char clientAddr[32];
char clientPort[5];

struct fileType {
    char expansion[100];
    char type[100];
};

struct fileType file_type[] =
        {
                {".html",              "text/html"},
                {".gif",               "imag/gif"},
                {".jpg",               "image/jpg"},
                {".png",               "image/png"},
                {".mp3",               "audio/mp3"},
                {".mp4",               "video/mp4"},
                {".ico",               "image/x-icon"},
                {(char) (char *) NULL, (char) (char *) NULL}
        };

char *getExpansion(const char *expansion) {
    struct fileType *type;
    for (type = file_type; type->expansion; type++) {
        if (strcmp(type->expansion, expansion) == 0) {
            return type->type;

        }
    }
    return NULL;
}

boolean isDynamic = false;

void handle_sig() {
    printf("child exit\n");
    wait(NULL);
}

/*
char *http_res = "HTTP/1.1 200 OK\r\n"
                      "Connection: keep-alive\r\n"
                      "Accept-Ranges: bytes\r\n"
                      "Content-Length: %d\r\n"
                      "Connection: close\r\n"
                      "Content-Type: %s\r\n\r\n";
*/

void raspiStill();

int file_not_found(int sAccept);

int method_not_implemented(int sAccept);

int file_ok(int sAccept, long flen, char *path);

int sendFile(int sAccept, FILE *resource);

int sendDynamicPage(int sAccept);

int customized_error_page(int sAccept);

void raspiStill() {
    system("raspistill -o image.jpg -w 640 -h 480 -rot 180");
}

int http_send(int sock_client) {
    int sAccept = sock_client;
    char recv_buf[BUFF_SIZE];
    char method[MIN_BUF];
    char url[MIN_BUF];
    char path[260]; // linux MAX_PATH
    int i, j;

    memset(recv_buf, 0, sizeof(recv_buf));

    /*if (recv(sAccept, recv_buf, sizeof(recv_buf), 0) == -1)   //接收错误
    {
        printf("recv() Failed.\n");
        return -1;
    } else { // Connection Successful
        printf("recv data from client:%s\n", recv_buf);
    }*/
    recv(sAccept, recv_buf, sizeof(recv_buf), 0);
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
        close(sAccept);
        printf("501 Not Implemented.\nSocket connection closed.\n");
        printf("====================\n\n");
        return -1;
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

    // By default it's dynamic webpage
    if (strcmp(url, "\\") == 0) {
        isDynamic = true;
    }
    printf("url: %s\n", url);

    // _getcwd: Gets the current working directory
    getcwd(path, _MAX_PATH);
    strcat(path, url);
    printf("path: %s\n", path);

    // Open the requested file with "rb" mode: Open file for reading.
    // "r" mode should do the same, but cases on the Internets says it won't work
    FILE *resource = fopen(path, "rb");

    // if the file is not exist, respond with "404 Not Found"
    if (resource == NULL && isDynamic == false) {
        file_not_found(sAccept);
        // 如果method是GET，则发送自定义的file not found页面
        if (strcmp(method, "GET") == 0) {
            customized_error_page(sAccept);
        }
        close(sAccept); //释放连接套接字，结束与该客户的通信
        printf("404 Not Found.\nSocket connection closed.\n");
        printf("====================\n\n");
        return USER_ERROR;
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
    close(sAccept);
    printf("200 OK.\nSocket connection closed.\n");
    printf("====================\n\n");

    return 0;

}

// 发送404 file_not_found报头
int file_not_found(int sAccept) {
    char send_buf[MIN_BUF];
    //  time_t timep;
    //  time(&timep);
    sprintf(send_buf, "HTTP/1.1 404 NOT FOUND\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    //  sprintf(send_buf, "Date: %s\r\n", ctime(&timep));
    //  send(sAccept, send_buf, strlen(send_buf), 0);
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
int method_not_implemented(int sAccept) {
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
int file_ok(int sAccept, long flen, char *path) {
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
int customized_error_page(int sAccept) {
    char send_buf[MIN_BUF];
    sprintf(send_buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "<BODY><h1 align='center'>404</h1><br/><h1 align='center'>file can not found.</h1>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    sprintf(send_buf, "</BODY></HTML>\r\n");
    send(sAccept, send_buf, (int) strlen(send_buf), 0);
    return 0;
}

// Send requested resources
int sendFile(int sAccept, FILE *resource) {
    char send_buf[BUFF_SIZE];
    size_t bytes_read = 0;
    while (1) {
        memset(send_buf, 0, sizeof(send_buf));       //缓存清0
        bytes_read = fread(send_buf, sizeof(char), sizeof(send_buf), resource);
        //  printf("send_buf: %s\n",send_buf);
        if (int_ERROR == send(sAccept, send_buf, (int) bytes_read, 0)) {
            printf("send() Failed:%d\n", -1);
            return USER_ERROR;
        }
        if (feof(resource)) {
            return 0;
        }
    }
}

// Send dynamic webpage
int sendDynamicPage(int sAccept) {
    char response[BUFF_SIZE];
    int len = BUFF_SIZE;
    memset(response, 0, sizeof(response));

    int fd;
    double temp = 0;
    char tempbuf[MAX_SIZE];
    fd = open(TEMP_PATH, O_RDONLY);
    if (fd >= 0) {
        if (read(fd, tempbuf, MAX_SIZE) < 0) {
        }
    }
    temp = atoi(tempbuf) / 1000.0; // NOLINT(cert-err34-c)
    char resp[1024];
    char tempstr[20];
    strcat(response,
           "<html lang='en'><head><title>Alloha from RaspPi</title><meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"1\"/></head>");
    strcat(resp, "");
    strcat(resp, "<body><h1>Alloha, World!</h1>\r\n");
    strcat(resp, "<p>Server IP: ");
    strcat(resp, serverAddr);
    strcat(resp, ":");
    strcat(resp, serverPort);
    strcat(resp, "</p>\r\n");
    strcat(resp, "<p>Client IP: ");
    strcat(resp, clientAddr);
    strcat(resp, ":");
    strcat(response, clientPort);
    strcat(response, "</p>\r\n\r\n");
    strcat(response, "<h2>This is a Raspberry Pi @Laurence's Dorm.</h2>\r\n");
    strcat(response, "<h3>&#x1f321;&nbsp;Temperature: ");
    sprintf(tempstr, "%.2f", temp);
    strcat(response, tempstr);
    strcat(response, "℃</h3>\r\n<h3>");
    strcat(response, "&#x231a; \t Time: ");
    char *cur_time = (char *) malloc(21 * sizeof(char));
    time_t current_time;
    struct tm *now_time;
    time(&current_time);
    now_time = localtime(&current_time);
    char Year[6] = {0};
    char Month[4] = {0};
    char Day[4] = {0};
    char Hour[4] = {0};
    char Min[4] = {0};
    char Sec[4] = {0};
    strftime(Year, sizeof(Year), "%Y-", now_time);
    strftime(Month, sizeof(Month), "%m-", now_time);
    strftime(Day, sizeof(Day), "%d ", now_time);
    strftime(Hour, sizeof(Hour), "%H:", now_time);
    strftime(Min, sizeof(Min), "%M:", now_time);
    strftime(Sec, sizeof(Sec), "%S", now_time);
    strncat(cur_time, Year, 5);
    strncat(cur_time, Month, 3);
    strncat(cur_time, Day, 3);
    strncat(cur_time, Hour, 3);
    strncat(cur_time, Min, 3);
    strncat(cur_time, Sec, 3);
    strcat(response, cur_time);
    strcat(response, "</h3>\r\n");
    raspiStill();
    strcat(response, "<h3><img src=\"/image.jpg\"></h3>");
    strcat(response, "</body></html>");

    if (int_ERROR == send(sAccept, response, (size_t) len, 0)) {
        printf("send() Failed:%d\n", -1);
        return USER_ERROR;
    }
    return 0;
}

/*
    int len = strlen(content);
    sprintf(HTTP_HEADER, http_res_tmpl, len, "text/html; charset=utf-8");
    len = sprintf(HTTP_INFO, "%s\r\n\r\n%s", HTTP_HEADER, content);

    write(sock_client, HTTP_INFO, (size_t) len);
}
*/

int main() {
    int listenfd, connectfd;
    pthread_t thread; //id of thread
    ARG *arg;//pass this var to the thread
    struct sockaddr_in server; //server's address info
    struct sockaddr_in client; //client's
    int sin_size;

    //create tcp socket

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("creating socket failed.");
        exit(1);
    }

    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("bind.... ");
    if (bind(listenfd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("bind error.");
        exit(1);
    }

    printf("listen.... ");
    if (listen(listenfd, 5) == -1) {
        perror("listen() error ");
        exit(1);
    }

    sin_size = sizeof(struct sockaddr_in);
    while (1) {
//accept() using main thread
        printf("accepting.... ");
        if ((connectfd = accept(listenfd,
                                (struct sockaddr *) &client,
                                (socklen_t *) &sin_size)) == -1) {
            printf("accept() error ");
        }


        arg = new
        ARG;
        arg->connfd = connectfd;
        memcpy((void *) &arg->client, &client, sizeof(client));

        if (pthread_create(&thread, NULL, start_routine, (void *) arg)) {
            perror("pthread_create() error");
            exit(1);
        }
    }
    close(listenfd);
}