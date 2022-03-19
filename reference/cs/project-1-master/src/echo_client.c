/******************************************************************************
* echo_client.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo client.  The  *
*              client connects to an arbitrary <host,port> and sends input    *
*              from stdin.                                                    *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netinet/ip.h>
#define ECHO_PORT 9999
#define BUF_SIZE 8192*10

int main(int argc, char* argv[])
{

    if (argc != 4)
    {
        fprintf(stderr, "usage: %s <server-ip> <port> <msgFile>", argv[0]);
        return EXIT_FAILURE;
    }
    char buf[BUF_SIZE];

    int status, sock;
    struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
    struct addrinfo *servinfo;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s \n", gai_strerror(status));
        return EXIT_FAILURE;
    }


    if((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    {
        fprintf(stderr, "Socket failed");
        return EXIT_FAILURE;
    }

    if (connect (sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        fprintf(stderr, "Connect");
        return EXIT_FAILURE;
    }
    //printf("sleep for 5 seconds\n");
    //sleep(5);
    char msg[BUF_SIZE];
    // fgets(msg, BUF_SIZE, stdin);
    int fd_in;
    fd_in = open(argv[3], O_RDONLY);
	if(fd_in < 0) {
	    printf("Failed to open the file!\n");
		return 0;
    }
    int bytes_received;
    fd_in = open(argv[3], O_RDONLY);
    read(fd_in, msg, BUF_SIZE);
    fprintf(stdout, "Sending\n%s", msg);
    send(sock, msg , strlen(msg), 0);
    
    memset(msg,0,BUF_SIZE);
    char path[100];
    
    int test=0;
    if(!test)
    for(int i=2;i<=20;++i){
        memset(path,0,100);
        memset(msg,0,BUF_SIZE);
        strcpy(path,argv[3]);
        sprintf(path+strlen(path),"%d",i);
        fd_in = open(path, O_RDONLY);
        read(fd_in, msg, BUF_SIZE);
        fprintf(stdout, "Sending:%d\n%s",i,msg);
        send(sock, msg , strlen(msg), 0);
    }
    
    
    
    
    int count=1;
    memset(buf, 0, BUF_SIZE);
    printf("Received:\n");
    while((bytes_received = recv(sock, buf, BUF_SIZE, 0)) > 1)
    {
        buf[bytes_received] = '\0';
        fprintf(stdout, "%s",buf);
        memset(buf, 0, BUF_SIZE);
    }
    freeaddrinfo(servinfo);
    close(sock);
    return EXIT_SUCCESS;
}
