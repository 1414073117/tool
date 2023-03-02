#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0) {
        perror("socket");
        return 0;
    }
    printf( "套接字描述符:%d\n", sock_fd);

    struct sockaddr_in addr;                        //定义地址信息的结构体
    addr.sin_family = AF_INET;                      //使用ipv4
    addr.sin_port = htons(27098);                   //指定端口27089，需要将端口转换成大端字节序
    addr.sin_addr.s_addr = inet_addr("192.168.3.1"); // IP地址

    int ret = bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)); //绑定地址信息
    if (ret < 0)
    {
        perror("bind");
        return 0;
    }

    char buf[1024] = "i am client";

    struct sockaddr_in dest_addr; // client的地址信息结构
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(27089);
    dest_addr.sin_addr.s_addr = inet_addr("192.168.2.1");
    sendto(sock_fd, buf, strlen(buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    close(sock_fd); //关闭套接字
    return 0;
}