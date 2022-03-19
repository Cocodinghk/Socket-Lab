/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include "parse.h"

#define ECHO_PORT 9999
#define BUF_SIZE 4096

char stversion[] = "HTTP/1.1";
char stURI[] = "/home/project-1/www/static_site";
char css[] = "text/css";
char png[] = "image/png";
char jpg[] = "image/jpeg";
char gif[] = "image/gif";
char html[] = "text/html";
char *hashtext[100];
char timebuffer[64];
char timebuffer_modified[64];

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

char *Int2String(int num, char *str) //10进制
{
    int i = 0;   //指示填充str
    if (num < 0) //如果num为负数，将num变正
    {
        num = -num;
        str[i++] = '-';
    }
    //转换
    do
    {
        str[i++] = num % 10 + 48; //取num最低位 字符0~9的ASCII码是48~57；简单来说数字0+48=48，ASCII码对应字符'0'
        num /= 10;                //去掉最低位
    } while (num);                //num不为0继续循环

    str[i] = '\0';

    //确定开始调整的位置
    int j = 0;
    if (str[0] == '-') //如果有负号，负号不用调整
    {
        j = 1; //从第二位开始调整
        ++i;   //由于有负号，所以交换的对称轴也要后移1位
    }
    //对称交换
    for (; j < i / 2; j++)
    {
        //对称交换两端的值 其实就是省下中间变量交换a+b的值：a=a+b;b=a-b;a=a-b;
        str[j] = str[j] + str[i - 1 - j];
        str[i - 1 - j] = str[j] - str[i - 1 - j];
        str[j] = str[j] - str[i - 1 - j];
    }

    return str; //返回转换后的值
}

int exist(char *filename)
{
    int fp = open(filename, O_RDONLY);
    if (fp < 0)
    {
        return 0;
    }
    else
    {
        close(fp);
        return 1;
    }
}

int splitext(char *URI)
{
    int len = strlen(URI);
    for (int j = len - 1; j >= 0; j--)
    {
        if (URI[j] == '.')
        {
            return j;
        }
    }
    return -1;
}

void send2client(char *buf_send, int client_sock, int sock)
{
    if (send(client_sock, buf_send, strlen(buf_send), 0) != strlen(buf_send))
    {
        close_socket(client_sock);
        close_socket(sock);
        fprintf(stderr, "Error sending to client.\n");
        return EXIT_FAILURE;
    }
}

void getmycomputertime()
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = gmtime(&rawtime);
    strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%M:%S GMT\r\n", timeinfo);
}

void GET(char *buf, char *buf_send, Request *request, int client_sock, int sock, unsigned long addr)
{
    struct stat st;
    //implement the GET method
    char *URI = request->http_uri;
    char messagebody[BUF_SIZE];
    char *mimetype;
    if (strcmp(URI, "/") == 0)
    {
        URI = stURI;
    }
    if (stat(URI, &st) == -1)
    {
        perror("stat failed\n");
        strcpy(buf_send, "404 Not Found.\r\n\r\n");
        my_log(0, 2, addr, request->http_uri, 0);
        send2client(buf_send, client_sock, sock);
        return;
    }
    if (S_ISDIR(st.st_mode))
    {
        perror("this is a dir\n");
        strcat(URI, "/index.html");
    }
    if (!exist(URI))
    {
        strcpy(buf_send, "404 Not Found.\r\n\r\n");
        my_log(0, 2, addr, request->http_uri, 0);
        send2client(buf_send, client_sock, sock);
        return;
    }
    else
    {
        int readhetttt = 0;
        int pos = splitext(URI);
        char extension[10];
        //time transfer
        time_t t = st.st_mtime;
        struct tm *mytm = gmtime(&t);
        strftime(timebuffer_modified, sizeof(timebuffer_modified), "%a, %d %b %Y %H:%M:%S GMT\r\n", mytm);
        //time transfer
        //get mimetype
        for (int j = pos + 1; j < strlen(URI); j++)
        {
            extension[j - pos - 1] = URI[j];
        }
        mimetype = hashtext[extension[0]];
        //get mimetype
        int fp = open(URI, O_RDONLY);
        if (fp < 0)
        {
            strcpy(buf_send, "500 Internal Server Error.\r\n\r\n");
            my_log(0, 4, addr, request->http_uri, 0);
            send2client(buf_send, client_sock, sock);
            return;
        }
        else
        {
            readhetttt = read(fp, messagebody, BUF_SIZE);
        }
        char length_messagebody[20] = {0};
        Int2String(readhetttt, length_messagebody);
        //get time of my computer
        getmycomputertime();
        //get time of my computer
        strcpy(buf_send, "HTTP/1.1 200 OK\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Connection:close\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Content-Type:");
        strcat(buf_send, mimetype);
        strcat(buf_send, "\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Server:liso/1.1\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Date:");
        strcat(buf_send, timebuffer);
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Content-Length:");
        strcat(buf_send, length_messagebody);
        strcat(buf_send, "\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Last-Modified:");
        strcat(buf_send, timebuffer_modified);
        strcat(buf_send, "\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, messagebody);
        send2client(buf_send, client_sock, sock);
    }
    free(URI);
}

void POST(char *buf, char *buf_send, Request *request, int client_sock, int sock, unsigned long addr)
{
    getmycomputertime();
    strcpy(buf_send, "HTTP/1.1 200 OK\r\n");
    send2client(buf_send, client_sock, sock);
    strcpy(buf_send, "Connection:close\r\n");
    send2client(buf_send, client_sock, sock);
    strcpy(buf_send, "Server:liso/1.1\r\n");
    send2client(buf_send, client_sock, sock);
    strcpy(buf_send, "Date:");
    strcat(buf_send, timebuffer);
    send2client(buf_send, client_sock, sock);
    strcpy(buf_send, "Content-Length:0\r\n");
    strcat(buf_send, "\r\n");
    send2client(buf_send, client_sock, sock);
}

void HEAD(char *buf, char *buf_send, Request *request, int client_sock, int sock, unsigned long addr)
{
    struct stat st;
    //implement the GET method
    char *URI = request->http_uri;
    char *mimetype;
    if (strcmp(URI, "/") == 0)
    {
        URI = stURI;
    }
    if (stat(URI, &st) == -1)
    {
        perror("stat failed\n");
        strcpy(buf_send, "404 Not Found.\r\n\r\n");
        my_log(0, 2, addr, request->http_uri, 0);
        send2client(buf_send, client_sock, sock);
        return;
    }
    if (S_ISDIR(st.st_mode))
    {
        perror("this is a dir\n");
        strcat(URI, "/index.html");
    }
    if (!exist(URI))
    {
        strcpy(buf_send, "404 Not Found.\r\n\r\n");
        my_log(0, 2, addr, request->http_uri, 0);
        send2client(buf_send, client_sock, sock);

        return;
    }
    else
    {
        int readhetttt = 0;
        int pos = splitext(URI);
        char extension[10];
        //time transfer
        time_t t = st.st_mtime;
        struct tm *mytm = gmtime(&t);
        strftime(timebuffer_modified, sizeof(timebuffer_modified), "%a, %d %b %Y %H:%M:%S GMT\r\n", mytm);
        //time transfer
        //get mimetype
        for (int j = pos + 1; j < strlen(URI); j++)
        {
            extension[j - pos - 1] = URI[j];
        }
        mimetype = hashtext[extension[0]];
        //get mimetype
        int fp = open(URI, O_RDONLY);
        if (fp < 0)
        {
            strcpy(buf_send, "500 Internal Server Error.\r\n\r\n");
            my_log(0, 4, addr, request->http_uri, 0);
            send2client(buf_send, client_sock, sock);
            return;
        }
        else
        {
            close(fp);
        }
        int filesize = -1;
        //get time of my computer
        getmycomputertime();
        //get time of my computer
        //get fsize
        struct stat statbuff;
        if (stat(URI, &statbuff) == -1)
        {
            perror("stat failed\n");
            strcpy(buf_send, "404 Not Found.\r\n\r\n");
            my_log(0, 2, addr, request->http_uri, 0);
            send2client(buf_send, client_sock, sock);
            return;
        }
        else
        {
            filesize = statbuff.st_size;
        }
        char length_filesize[20] = {0};
        Int2String(filesize, length_filesize);
        //get fsize
        strcpy(buf_send, "HTTP/1.1 200 OK\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Connection:close\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Content-Type:");
        strcat(buf_send, mimetype);
        strcat(buf_send, "\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Server:liso/1.1\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Date:");
        strcat(buf_send, timebuffer);
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Content-Length:");
        strcat(buf_send, length_filesize);
        strcat(buf_send, "\r\n");
        send2client(buf_send, client_sock, sock);
        strcpy(buf_send, "Last-Modified:");
        strcat(buf_send, timebuffer_modified);
        strcat(buf_send, "\r\n");
        send2client(buf_send, client_sock, sock);
    }
    free(URI);
}

void use_method(char *met, char *buf, char *buf_send, Request *request, int client_sock, int sock, unsigned long addr)
{
    int len_method = strlen(met);
    if (len_method > 4)
    {
        strcpy(buf_send, "501 Method Unimplemented.\r\n\r\n");
        my_log(0, 4, addr, request->http_uri, 0);
        send2client(buf_send, client_sock, sock);
    }
    else if (len_method == 4)
    {
        if (met[0] == 'P' && met[1] == 'O' && met[2] == 'S' && met[3] == 'T')
        {
            my_log(1, 3, addr, request->http_uri, 0);
            POST(buf, buf_send, request, client_sock, sock, addr);
        }
        else if (met[0] == 'H' && met[1] == 'E' && met[2] == 'A' && met[3] == 'D')
        {
            my_log(1, 2, addr, request->http_uri, 0);
            HEAD(buf, buf_send, request, client_sock, sock, addr);
        }
        else
        {
            strcpy(buf_send, "HTTP/1.1 501 Method Unimplemented\r\n\r\n");
            my_log(0, 4, addr, request->http_uri, 0);
            send2client(buf_send, client_sock, sock);
        }
    }
    else if (len_method == 3)
    {
        if (met[0] == 'G' && met[1] == 'E' && met[2] == 'T')
        {
            my_log(1, 1, addr, request->http_uri, 0);
            GET(buf, buf_send, request, client_sock, sock, addr);
        }
        else
        {
            strcpy(buf_send, "HTTP/1.1 501 Method Unimplemented\r\n\r\n");
            my_log(0, 4, addr, request->http_uri, 0);
            send2client(buf_send, client_sock, sock);
        }
    }
    else
    {
        strcpy(buf_send, "HTTP/1.1 501 Method Unimplemented\r\n\r\n");
        my_log(0, 4, addr, request->http_uri, 0);
        send2client(buf_send, client_sock, sock);
    }
}

int check_http_version(char *http_version)
{

    if (strcasecmp(http_version, stversion) == 0)
    {
        return 1;
    }
    return 0;
}

void creathash(char *hashtext[])
{
    char pos = 'c';
    hashtext[pos] = css;
    pos = 'h';
    hashtext[pos] = html;
    pos = 'p';
    hashtext[pos] = png;
    pos = 'j';
    hashtext[pos] = jpg;
    pos = 'g';
    hashtext[pos] = gif;
}

void my_log(int a, int b, unsigned long ipaddr, char *requestaddr, int size)
{
    time_t curtime;
    struct tm *info;
    char buffer[80]; //strftime函数
    time(&curtime);  //时间
    info = localtime(&curtime);
    strftime(buffer, 80, "%d/%b/%Y:%X", info); //访问日志时间格式：[10/Oct/2000:13:55:36 -0700]
    pid_t process_id;
    process_id = getpid(); //进程id

    pthread_t tid;
    tid = pthread_self(); //线程id

    FILE *fp; //创建文档

    //#简单示例ErrorLogFormat "[%t] [%l] [pid %P] %F: %E: [client %a] %M"
    // %t 当前时间   %l 消息的日志级别   %P 当前进程的进程ID   %F 日志调用的源文件名和行号   %E APR/OS 错误状态代码和字符串
    //[Fri Sep 09 10:42:29.902022 2011] [core:error] [pid 35708:tid 4328636416] [client 72.15.99.187] File does not exist: /usr/local/apache2/htdocs/favicon.ico
    //               [%t]                   [%l]              [pid %P]                    %F:                 %E: [client %a] %M
    /*日志条目中的第一项是消息的日期和时间。
    接下来是生成消息的模块（在本例中为核心）和该消息的严重性级别。
    后面是进程 ID，如果合适，还有遇到该情况的进程的线程 ID。
    接下来，我们有发出请求的客户端地址。
    最后是详细的错误消息，在这种情况下，它表示对不存在的文件的请求*/

    if (a == 0 && b == 1)
    { //第一种错误400
        fp = fopen("log.txt", "a+");
        fprintf(fp, "%s %s %s %d %s %lu %s %lu %s %s\r\n", ctime(&curtime), "[core:error]", "[pid ", process_id, ":tid", tid, "] [client ", ipaddr, "] ", "400：Bad Request");
        /*                                                     %s               %s          %s          %d       %s    %lu       %s       %lu       %s                  %s                   
        */
        fclose(fp);
    }
    if (a == 0 && b == 2)
    { //第二种错误404
        fp = fopen("log.txt", "a+");
        fprintf(fp, "%s %s %s %d %s %lu %s %lu %s %s %s\r\n", ctime(&curtime), "[core:error]", "[pid ", process_id, ":tid", tid, "] [client ", ipaddr, "] ", "404：File Not Found File does not exist: ", requestaddr);
        /*                                                     %s               %s          %s          %d       %s    %lu       %s          %lu         %s                         %s                          %s
        */
        fclose(fp);
    }
    if (a == 0 && b == 3)
    { //第三种错误408
        fp = fopen("log.txt", "a+");
        fprintf(fp, "%s %s %s %d %s %lu %s %lu %s %s\r\n", ctime(&curtime), "[core:error]", "[pid ", process_id, ":tid", tid, "] [client ", ipaddr, "] ", "408：Request Time-out");
        /*                                                     %s               %s          %s          %d       %s    %lu       %s        %lu       %s             %s                   
        */
        fclose(fp);
    }
    if (a == 0 && b == 4)
    { //第四种错误501
        fp = fopen("log.txt", "a+");
        fprintf(fp, "%s %s %s %d %s %lu %s %lu %s %s\r\n", ctime(&curtime), "[core:error]", "[pid ", process_id, ":tid", tid, "] [client ", ipaddr, "] ", "501：Not Implemented");
        /*                                                     %s               %s          %s          %d       %s    %lu       %s        %lu       %s             %s                  
        */
        fclose(fp);
    }
    if (a == 0 && b == 5)
    { //第五种错误505
        fp = fopen("log.txt", "a+");
        fprintf(fp, "%s %s %s %d %s %lu %s %lu %s %s\r\n", ctime(&curtime), "[core:error]", "[pid ", process_id, ":tid", tid, "] [client ", ipaddr, "] ", "505：HTTP Version Not Supported");
        /*                                                     %s               %s          %s          %d       %s    %lu       %s        %lu       %s                     %s                   
        */
        fclose(fp);
    }

    //127.0.0.1 - frank [10/Oct/2000:13:55:36 -0700] "GET /apache_pb.gif HTTP/1.0" 200 2326
    if (a == 1 && b == 1)
    { //第一种请求GET
        fp = fopen("log.txt", "a+");
        fprintf(fp, "%lu %s %s %s %s %s %s %d\r\n", ipaddr, " - - [", buffer, " +0800] \"GET ", requestaddr, " HTTP/1.1", "200", size);
        /*                                         %lu      %s       %s       %s                %s           %s        %s     %d
        */
        fclose(fp);
    }
    if (a == 1 && b == 2)
    { //第二种请求HEAD
        fp = fopen("log.txt", "a+");
        fprintf(fp, "%lu %s %s %s %s %s %s %d\r\n", ipaddr, " - - [", buffer, " +0800] \"HEAD ", requestaddr, " HTTP/1.1", "200", size);
        /*                                         %lu     %s         %s       %s                %s           %s        %s     %d
        */
        fclose(fp);
    }
    if (a == 1 && b == 3)
    { //第三种请求POST
        fp = fopen("log.txt", "a+");
        fprintf(fp, "%lu %s %s %s %s %s %s %d\r\n", ipaddr, " - - [", buffer, " +0800] \"POST ", requestaddr, " HTTP/1.1", "200", size);
        /*                                         %lu    %s         %s       %s                 %s           %s        %s     %d
        */
        fclose(fp);
    }
}

int main(int argc, char *argv[])
{
    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];
    char buf_send[BUF_SIZE];
    int index;
    //no.2
    //no.2
    creathash(hashtext);
    fprintf(stdout, "----- Echo Server -----\n");

    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }

    if (listen(sock, 5))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }
    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        clock_t start, finish;
        int num_count = 0;
        cli_size = sizeof(cli_addr);
        if ((client_sock = accept(sock, (struct sockaddr *)&cli_addr,
                                  &cli_size)) == -1)
        {
            close(sock);
            fprintf(stderr, "Error accepting connection.\n");
            return EXIT_FAILURE;
        }
        start = clock();
        readret = 0;

        while ((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
        {
            double Times = 0;
            finish = clock();
            if (num_count == 0)
            {
                num_count = 1;
                Times = (double)(finish - start) / CLOCKS_PER_SEC;
                fprintf(stdout, "%f\n", Times);
            }

            //no.2
            //no.2
            //my code
            Serverreq *serverreq = parse(buf, readret, 0);
            printf("Http request %d\n",serverreq->count);
            for (int pipelining = 0; pipelining < serverreq->count; pipelining++)
            {
                if (Times > 0.0001)
                {
                    strcpy(buf_send, "HTTP/1.1 408 Request Timeout\r\n\r\n");
                    my_log(0, 3, cli_addr.sin_addr.s_addr, serverreq->reqlis[serverreq->count].http_uri, 0);
                    send2client(buf_send, client_sock, sock);
                }
                else if (! serverreq->reqlis[serverreq->count].judge)
                {
                    printf("Http Method %s\n", serverreq->reqlis[serverreq->count].http_method);
                    printf("Http Version %s\n",  serverreq->reqlis[serverreq->count].http_version);
                    printf("Http Uri %s\n",  serverreq->reqlis[serverreq->count].http_uri);
                    for (index = 0; index < serverreq->reqlis[serverreq->count].header_count; index++)
                    {
                        printf("Request Header\n");
                        printf("Header name %s Header Value %s\n",  serverreq->reqlis[serverreq->count].headers[index].header_name, serverreq->reqlis[serverreq->count].headers[index].header_value);
                    }
                    if (!check_http_version( serverreq->reqlis[serverreq->count].http_version))
                    {
                        strcpy(buf_send, "HTTP/1.1 505 HTTP version Not Supported\r\n\r\n");
                        my_log(0, 5, cli_addr.sin_addr.s_addr,  serverreq->reqlis[serverreq->count].http_uri, 0);
                        send2client(buf_send, client_sock, sock);
                    }
                    else
                    {
                        use_method( serverreq->reqlis[serverreq->count].http_method, buf, buf_send,  &serverreq->reqlis[serverreq->count], client_sock, sock, cli_addr.sin_addr.s_addr);
                    }
                }
                else
                {
                    strcpy(buf_send, "HTTP/1.1 400 Bad Request\r\n\r\n");
                    my_log(0, 1, cli_addr.sin_addr.s_addr,  serverreq->reqlis[serverreq->count].http_uri, 0);
                    send2client(buf_send, client_sock, sock);
                }
            }
            //my code
            malloc_trim(0);
        }

        if (readret == -1)
        {
            close_socket(client_sock);
            close_socket(sock);
            fprintf(stderr, "Error reading from client socket.\n");
            return EXIT_FAILURE;
        }

        if (close_socket(client_sock))
        {
            close_socket(sock);
            fprintf(stderr, "Error closing client socket.\n");
            return EXIT_FAILURE;
        }
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
