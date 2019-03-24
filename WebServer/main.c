#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <winsock.h>

#define PORT 8080

int main(){

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_socket, 5);

    SOCKET client_socket = accept(server_socket, NULL, NULL);

    char buf[1024];
    read((int) client_socket, buf, 1024);

    printf("%s",buf);

    char status[] = "HTTP/1.0 200 OK\r\n";
    char header[] = "Server: DWBServer\r\nContent-Type: text/html;charset=utf-8\r\n\r\n";
    char body[] = "<html><head><title>C语言构建小型Web服务器</title></head><body><h2>欢迎</h2><p>Hello，World</p></body></html>";

    write((int) client_socket, status, sizeof(status));
    write((int) client_socket, header, sizeof(header));
    write((int) client_socket, body, sizeof(body));

    close((int) client_socket);
    close((int) server_socket);

    return 0;
}