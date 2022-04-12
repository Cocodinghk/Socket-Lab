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
#include <unistd.h>
#include "parse.h"
#include "msg_response.h"
#include <stdbool.h>
#define ECHO_PORT 9999


int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int server_sock, client_sock;
    ssize_t readret;
    socklen_t client_size;
    struct sockaddr_in server_addr, client_addr;
    char buf[BUF_SIZE];
    char inet_buf[INET_ADDRSTRLEN];

    fprintf(stdout, "----- Echo Server -----\n");
    
    /* all networked programs must create a socket */
    if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }
    else printf("Server socket created : %d\n\n", server_sock);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ECHO_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr))){
        close_socket(server_sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }

    if (listen(server_sock, 5)){
        close_socket(server_sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }



    fd_set readfds, allfds; 
    int maxfd;
    int clients[FD_SETSIZE];
    int counter_max = -1, counter;
    for (int i = 0; i < FD_SETSIZE; i++)
        clients[i] = -1;
    FD_ZERO(&allfds);
    FD_SET(server_sock, &allfds);
    maxfd = server_sock;
    int ready_fd_num;

    int nRecvBuf = 256*1024;
    setsockopt(server_sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
    int nSendBuf = 256*1024;
    setsockopt(server_sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
    int nZero=0;
    setsockopt(server_sock, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
    setsockopt(server_sock, SOL_SOCKET, SO_RCVBUF, (char *)&nZero, sizeof(nZero));

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        printf("\n\n-------------Start to check&select-------------\n");
        readfds = allfds;
        ready_fd_num = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ready_fd_num == 0)
        {
            continue;
        }
        if(ready_fd_num < 0){
            fprintf(stderr, "Error in select.\n");
            return EXIT_FAILURE;
        }
        
        printf("Got ready_fd_num: %d\n", ready_fd_num);

        if(FD_ISSET(server_sock, &readfds))
        {
            client_size = sizeof(client_addr);
            client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
            if (client_sock < 0)
            {
                fprintf(stderr, "Error accepting connection.\n");
                return EXIT_FAILURE;
            }
            else printf("Client connected : %d\n", client_sock);

            inet_ntop(AF_INET, &client_addr.sin_addr, inet_buf, sizeof(inet_buf));
            ntohs(client_addr.sin_port);
        
            for(counter  = 0; counter < FD_SETSIZE; counter ++)
                if(clients[counter] < 0){
                    clients[counter] = client_sock;
                    break;
                }
        
            if (counter == FD_SETSIZE){
                fprintf(stderr, "Error! Too many clients.\n");
                return EXIT_FAILURE;
            }

            FD_SET(client_sock, &allfds);

            if (client_sock > maxfd)
                maxfd = client_sock;
            if (counter > counter_max)
                counter_max = counter;
            if (--ready_fd_num == 0)
                continue;
        }

        printf("-------------Start to respond-----------\n");
        for(counter = 0; counter <= counter_max; counter++){
            
            client_sock = clients[counter];
            if (client_sock < 0)   
                continue;
            if (FD_ISSET(client_sock, &readfds)){
                readret = read(client_sock, buf, BUF_SIZE); 
                printf("the client %d 's readret is %zd\n", client_sock, readret);
                // printf("%s\n", buf);
                if (readret < 0){
                    fprintf(stderr, "Error reading from socket.\n");
                }
                else if (readret == 0)
                {
                    close_socket(client_sock);
                    printf("Close client %d\n", client_sock);
                    FD_CLR(client_sock, &allfds);
                    clients[counter] = -1;
                }
                else
                {
                    strcat(buf, "\r\n");
                    readret += 2;
                    int sendrets = msgs_resp(client_sock, buf, readret);
                    if (sendrets < 0){
                        close_socket(server_sock);
                        close_socket(client_sock);
                        fprintf(stderr, "Error sending message.\n");
                        return EXIT_FAILURE;
                    }
                }
                memset(buf, 0, BUF_SIZE);
                if (--ready_fd_num == 0)
                    break;
            }
        }
    }
    close_socket(server_sock);

    return EXIT_SUCCESS;
}
