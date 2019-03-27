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

#define BUFF_SIZE 1024
#define SERV_PORT 80

void handle_sig() {
    printf("child exit\n");
    wait(NULL);
}

char *http_res_tmpl = "HTTP/1.1 200 OK\r\n"
                      "Server: Cleey's Server V1.0\r\n"
                      "Accept-Ranges: bytes\r\n"
                      "Content-Length: %d\r\n"
                      "Connection: close\r\n"
                      "Content-Type: %s\r\n\r\n";

void http_send(int sock_client, char *content) {
    char HTTP_HEADER[BUFF_SIZE], HTTP_INFO[BUFF_SIZE];
    int len = strlen(content);
    sprintf(HTTP_HEADER, http_res_tmpl, len, "text/html");
    len = sprintf(HTTP_INFO, "%s%s", HTTP_HEADER, content);

    write(sock_client, HTTP_INFO, (size_t) len);
}

int main(void) {
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    char buf[BUFF_SIZE];
    char str[INET_ADDRSTRLEN];
    int i, n;

    struct sigaction act;
    act.sa_handler = handle_sig;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, NULL);


    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, 20);

    printf("Accepting connections ...\n");
    while (1) {
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &cliaddr_len);

        n = fork();
        if (n == -1) {
            perror("call to fork");
            exit(1);
        } else if (n == 0) { // child
            close(listenfd);

            n = read(connfd, buf, BUFF_SIZE);
            if (n == 0) {
                printf("the other side has been closed.\n");
                break;
            }
            printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
                   ntohs(cliaddr.sin_port));
            fputs(buf, stdout);
            http_send(connfd, "hello world!");
            close(connfd);
            exit(0);
        } else {  // parent
            close(connfd);
        }
    }
}
