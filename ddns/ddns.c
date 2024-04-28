#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 1024

int Get_http_x(char *ip, int portno, char *data) {
    int sockfd;
    struct hostent *server;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in server_addr;
    int bytes_sent, bytes_received;

    /* 创建套接字 */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* 设置服务器地址 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    /* 连接服务器 */
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    /* 发送HTTP请求 */
    bytes_sent = send(sockfd, data, strlen(data), 0);
    if (bytes_sent < 0) {
        perror("ERROR sending message");
        exit(1);
    }

    /* 接收服务器响应 */
    memset(buffer, 0, MAX_BUFFER_SIZE);
    bytes_received = recv(sockfd, buffer, MAX_BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("ERROR receiving message");
        exit(1);
    }

    printf("%s\n", buffer);
    close(sockfd);
    return 0;
}

int main(int argc, char *argv[])
{
    int portno = 80;
    char *ip = "10.90.7.245";
    char *message = "GET / HTTP/1.1\r\nHost: 10.90.7.245\r\n\r\n";
    Get_http_x(ip, portno, message);
}
