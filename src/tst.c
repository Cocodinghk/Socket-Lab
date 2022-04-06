/* select server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "wrap.h"
#define MAXLINE 80
#define SERV_PORT 8000
int main(int argc, char *argv[])
{
int i, maxi, maxfd;
int listenfd, connfd, sockfd;
int nready, client[FD_SETSIZE];
ssize_t n;

//两个集合
fd_set rset, allset;

char buf[MAXLINE];
char str[INET_ADDRSTRLEN]; /* #define INET_ADDRSTRLEN 16 */
socklen_t cliaddr_len;
struct sockaddr_in cliaddr, servaddr;

//创建套接字
listenfd = Socket(AF_INET, SOCK_STREAM, 0);

//绑定
bzero(&servaddr, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
servaddr.sin_port = htons(SERV_PORT);
Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

//监听
Listen(listenfd, 20); /* 默认最大128 */

//需要接收最大文件描述符
maxfd = listenfd;

//数组初始化为-1
maxi = -1;
for (i = 0; i < FD_SETSIZE; i++)
client[i] = -1;

//集合清零
FD_ZERO(&allset);

//将listenfd加入allset集合
FD_SET(listenfd, &allset);

for ( ; ; )
{
//关键点3
rset = allset; /* 每次循环时都重新设置select监控信号集 */

//select返回rest集合中发生的事件的总数 参数1：最大文件描述符+1
nready = select(maxfd+1, &rset, NULL, NULL, NULL);
if (nready < 0)
perr_exit("select error");

//listenfd是否在rset集合中
if (FD_ISSET(listenfd, &rset))
{
    //accept接收
    cliaddr_len = sizeof(cliaddr);
    //accept返回通信套接字，当前非阻塞，因为select已经发生读写事件
    connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
    printf("received from %s at PORT %d\n",
    inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
    ntohs(cliaddr.sin_port));

    //关键点1
    for (i = 0; i < FD_SETSIZE; i++)
    if (client[i] < 0)
    {
    client[i] = connfd; /* 保存accept返回的通信套接字connfd存到client[]里 */
    break;
    }

    /* 是否达到select能监控的文件个数上限 1024 */
    if (i == FD_SETSIZE) {
    fputs("too many clients\n", stderr);
    exit(1);
    }

    //关键点2
    FD_SET(connfd, &allset); /*添加一个新的文件描述符到监控信号集里 */

    //更新最大文件描述符数
    if (connfd > maxfd)
    maxfd = connfd; /* select第一个参数需要 */
    if (i > maxi)
    maxi = i; /* 更新client[]最大下标值 */

    /* 如果没有更多的就绪文件描述符继续回到上面select阻塞监听,负责处理未处理完的就绪文件描述符 */
    if (--nready == 0)
    continue;
}

for (i = 0; i <= maxi; i++)
{
//检测clients 哪个有数据就绪
if ( (sockfd = client[i]) < 0)
continue;

//sockfd（connd）是否在rset集合中
if (FD_ISSET(sockfd, &rset))
{
//进行读数据 不用阻塞立即读取（select已经帮忙处理阻塞环节）
if ( (n = Read(sockfd, buf, MAXLINE)) == 0)
{
/* 无数据情况 client关闭链接，服务器端也关闭对应链接 */
Close(sockfd);
FD_CLR(sockfd, &allset); /*解除select监控此文件描述符 */
client[i] = -1;
} else
{
//有数据
int j;
for (j = 0; j < n; j++)
buf[j] = toupper(buf[j]);
Write(sockfd, buf, n);//写回客户端
}
if (--nready == 0)
break;
}
}

}
close(listenfd);
return 0;
}