#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //创建UDP套接字
    if (sock_fd < 0)
    {
        perror("socket");
        return 0;
    }
    printf("套接字描述符:%d\n", sock_fd);

    struct sockaddr_in addr;                        //定义地址信息的结构体
    addr.sin_family = AF_INET;                      //使用ipv4
    addr.sin_port = htons(27089);                   //指定端口27089，需要将端口转换成大端字节序
    addr.sin_addr.s_addr = inet_addr("192.168.2.1"); // IP地址

    int ret = bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)); //绑定地址信息
    if (ret < 0)
    {
        perror("bind");
        return 0;
    }

    while (1) //循环接收数据
    {
        char buf[1024] = {0};         //缓冲区
        struct sockaddr_in peer_addr; //定义地址信息结构
        socklen_t len = sizeof(peer_addr);
        ssize_t recv_size = recvfrom(sock_fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&peer_addr, &len);
        if (recv_size < 0)
        {
            continue; //继续接收
        }
        printf("收到消息： \"%s\" ，来自 %s:%d\n", buf, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
        memset(buf, '\0', sizeof(buf));
    }

    close(sock_fd);
    return 0;
}