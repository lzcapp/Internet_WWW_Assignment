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

#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"
#define MAX_SIZE 32
#define BUFF_SIZE 1024
#define SERV_PORT 8080

void handle_sig() {
    printf("child exit\n");
    wait(NULL);
}

char *http_res_tmpl = "HTTP/1.1 200 OK\r\n"
                      "Connection: keep-alive\r\n"
                      "Accept-Ranges: bytes\r\n"
                      "Content-Length: %d\r\n"
                      "Connection: close\r\n"
                      "Content-Type: %s\r\n\r\n";

void http_send(int sock_client, char *content) {
    char HTTP_HEADER[BUFF_SIZE], HTTP_INFO[BUFF_SIZE];
    int len = strlen(content);
    sprintf(HTTP_HEADER, http_res_tmpl, len, "text/html; charset=utf-8");
    len = sprintf(HTTP_INFO, "%s\r\n\r\n%s", HTTP_HEADER, content);

    write(sock_client, HTTP_INFO, (size_t) len);
}

int main(void) {
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    char buf[BUFF_SIZE];
    char str[INET_ADDRSTRLEN];
    int n;

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
            int fd;
            double temp = 0;
            char tempbuf[MAX_SIZE];
            fd = open(TEMP_PATH, O_RDONLY);
            if (fd >= 0) {
                if (read(fd, tempbuf, MAX_SIZE) < 0) {
                }
            }
            temp = atoi(tempbuf) / 1000.0; // NOLINT(cert-err34-c)
            char response[1024];
            char tempstr[20];
            strcat(response,
                   "<html lang='en'><head><title>Alloha from RaspPi</title><meta charset=\"utf-8\" http-equiv=\"refresh\" content=\"1\"/></head>");
            strcat(response, "");
            strcat(response, "<body><h1>Alloha, World!</h1>\r\n");
            strcat(response, "<p>Server IP: ");
            strcat(response, inet_ntoa(servaddr.sin_addr));
            strcat(response, ":");
            uint16_t serverPort = ntohs(servaddr.sin_port);
            char serverPortNum[5];
            sprintf(serverPortNum, "%d", serverPort);
            strcat(response, serverPortNum);
            strcat(response, "</p>\r\n");
            strcat(response, "<p>Client IP: ");
            strcat(response, inet_ntoa(cliaddr.sin_addr));
            strcat(response, ":");
            uint16_t clientPort = ntohs(cliaddr.sin_port);
            char clientPortNum[10];
            sprintf(clientPortNum, "%d", clientPort);
            strcat(response, clientPortNum);
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
            strcat(response, "</h3>\r\n</body></html>");
            http_send(connfd, response);
            close(connfd);
            exit(0);
        } else { // parent
            close(connfd);
        }
    }
}
