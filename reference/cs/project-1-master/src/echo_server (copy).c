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
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "parse.h"
#define ECHO_PORT 9999
#define BUF_SIZE 4096
#define LOG_SIZE 200
#define HTTP_VERSION "HTTP/1.1"
#define error_log_file "./log/error_log.txt"
#define common_log_file "./log/common_log.txt"
int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}
int get_extension(char *path,char *extension){

    size_t len = strlen(path);
    for (int i=len-1;i>= 0;--i)
    {
        if (path[i] == '.')
        {
            strncpy(extension,path+(i+1),len-1-i);
            return 1;
        }
    }

    return 0;
}
void analyze_type(char *extension,char *type)
{

    if (!strcmp(extension,"html"))
    {
        strcpy(type,"text/html");
    }
    else if (!strcmp(extension,"css"))
    {
        strcpy(type,"text/css");
    }
    else if (!strcmp(extension,"png"))
    {
        strcpy(type,"image/png");
    }
    else if (!strcmp(extension,"jpeg"))
    {
        strcpy(type,"image/jpeg");
    }
    else if (!strcmp(extension,"gif"))
    {
        strcpy(type,"image/gif");
    }
    else
    {
        strcpy(type,"unknown/undifined");
    }

}
void common_log(int state,Request *req){
    FILE *file;
    time_t t;
    char log_to_write[LOG_SIZE],stringtime[100],path[50];
    file=fopen(common_log_file, "a");
    time(&t);
    struct tm* current_time = localtime(&t);
    strftime(log_to_write,100,"[%c] ", current_time);
    if(state==400){
        strcat(log_to_write,"400 Bad Request");
        strcat(log_to_write,"\r\n");    
    }
    else {
        strcat(log_to_write,"\"");
        strcat(log_to_write,req->http_method);
        strcat(log_to_write," ");
        strcat(log_to_write,req->http_uri);
        strcat(log_to_write," ");
        strcat(log_to_write,req->http_version);
        strcat(log_to_write,"\" ");
        if(state>=100){
            sprintf(log_to_write+strlen(log_to_write),"%d\r\n",state);
        }
        else
            strcat(log_to_write,"200\r\n");
    }
    fwrite(log_to_write,strlen(log_to_write),1,file);
    fclose(file);
    return ;
}
void error_log(int state,Request *req){
    FILE *file;
    time_t t;
    char log_to_write[LOG_SIZE],stringtime[100],path[50];
    file=fopen(error_log_file, "a");
    time(&t);
    struct tm* current_time = localtime(&t);
    strftime(log_to_write,100,"[%c] ", current_time);
    strcat(log_to_write,"[core:error]");
    
    if(state==400){
        strcat(log_to_write,"Bad Request");
        strcat(log_to_write,"\r\n");
    }
    else if(state==408){  //time-out
        strcat(log_to_write,"Request Timeout");
        strcat(log_to_write,"\r\n");
    }
    else if(state==404){
        strcat(path,".");
        strcat(path,req->http_uri);
        strcat(log_to_write,"Not Found :");
        strcat(log_to_write,path);
        strcat(log_to_write,"\r\n");
    }
    else if(state==505){
        strcat(log_to_write,"HTTP Version Not Supported :");
        strcat(log_to_write,req->http_version);
        strcat(log_to_write,"\r\n");
    }
    else if(state==501){
        strcat(log_to_write,"Not Implemented :");
        strcat(log_to_write,req->http_method);
        strcat(log_to_write,"\r\n");
    }
    fwrite(log_to_write,strlen(log_to_write),1,file);
    fclose(file);
}
void send_respond(int client_sock,int state,Request *req){

    common_log(state,req);
    char *respond=(char*)malloc(sizeof(char)*BUF_SIZE);
    char path[1000],extension[10],type[20];
    struct stat file_state;
    if(state==400){
        strcat(respond,HTTP_VERSION);
        strcat(respond," ");
        strcat(respond,"400 Bad Request");
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);
        free(respond);
        error_log(400,req);
        return ;
    }
    else if(state==408){  //time-out
        strcat(respond,HTTP_VERSION);
        strcat(respond," ");
        strcat(respond,"408 Request Timeout");
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);
        free(respond);
        error_log(408,req);
        return ;
    }
    strcat(path,".");
    strcat(path,req->http_uri);
    if(state==1){
        stat(path, &file_state);
        strcat(respond,HTTP_VERSION);
        strcat(respond," ");
        strcat(respond,"200 OK");
        strcat(respond,"\r\n");

        strcat(respond,"Content-Type:");
        get_extension(path,extension);
        analyze_type(extension,type);
        strcat(respond,type);

        strcat(respond,"\r\n");

        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);

        //send file
        int file=open(path,O_RDONLY);
        stat(path,&file_state);
        int len=file_state.st_size;
        sendfile(client_sock,file,NULL,len);
    }
    else if(state==2){
        stat(path, &file_state);
        strcat(respond,HTTP_VERSION);
        strcat(respond," ");
        strcat(respond,"200 OK");
        strcat(respond,"\r\n");

        strcat(respond,"Content-Type:");
        get_extension(path,extension);
        analyze_type(extension,type);
        strcat(respond,type);
        strcat(respond,"\r\n");

        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);

        //send file
        //int file=open(path,O_RDONLY);
        //stat(path,&fiele_state);
        //int len=file_state.st_size;
        //sendfile(client_sock,file,NULL,len);
    }
    else if(state==3){
        strcat(respond,"-----echo-----\r\n");
        strcat(respond,req->http_method);
        strcat(respond," ");
        strcat(respond,req->http_uri);
        strcat(respond," ");
        strcat(respond,HTTP_VERSION);
        strcat(respond,"\r\n");
        for(int i=0;i<req->header_count;++i){
            strcat(respond,req->headers[i].header_name);
            strcat(respond,":");
            strcat(respond,req->headers[i].header_value);
            strcat(respond,"\r\n");
        }
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);
    }
    else if(state==404){
        strcat(respond,req->http_version);
        strcat(respond," ");
        strcat(respond,"404 Not Found");
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);
        error_log(404,req);
    }
    else if(state==505){
        strcat(respond,HTTP_VERSION);
        strcat(respond," ");
        strcat(respond,"505 HTTP Version Not Supported");
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);
        error_log(505,req);
    }
    else if(state==501){
        strcat(respond,req->http_version);
        strcat(respond," ");
        strcat(respond,"501 Not Implemented");
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);
        error_log(501,req);
    }

    free(respond);

}
void analyze_req(int sock,int client_sock,char *buffer,int buff_size){

    Request *req = parse(buffer,buff_size,sock);

    if(req==NULL){
        printf("req is null\n");
        send_respond(client_sock,400,req);

    }
    else if(strcmp(req->http_version,HTTP_VERSION)){
        send_respond(client_sock,505,req);
    }
    else if(!strcmp(req->http_method,"GET")){
        char path[1000];
        strcat(path,".");
        strcat(path,req->http_uri);
        if(access(path,F_OK)){
            //printf("file_f_ok");
            send_respond(client_sock,404,req);
        }
        else if(access(path,R_OK)){
            //printf("file_r_ok");
            send_respond(client_sock,404,req);
        }
        else send_respond(client_sock,1,req);
    }
    else if(!strcmp(req->http_method,"HEAD")){
        send_respond(client_sock,2,req);
    }
    else if(!strcmp(req->http_method,"POST")){
        send_respond(client_sock,3,req);
    }
    else{
        send_respond(client_sock,501,req);
    }

    //free
    if(req==NULL){
        free(req);
        return ;
    }
    else {
        free(req->headers);
        free(req);
        return ;
    }

}
int main(int argc, char* argv[])
{


    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];
    int time=0;
	struct timeval last;
	struct timeval now;
    fprintf(stdout, "----- Echo Server -----\n");
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    //non-blocking
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);


    //printf("start %d %d\n", )
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
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

    while (1)
    {
        cli_size = sizeof(cli_addr);
        gettimeofday(&last,NULL);
        if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,&cli_size)) == -1&&(errno==EAGAIN||errno==EWOULDBLOCK)){
            continue;    
        }
        else {
            readret = 0;
            while((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1)
            {
                gettimeofday(&now,NULL);
                time=(now.tv_sec-last.tv_sec)*1000000+(now.tv_usec-last.tv_usec);//微秒
        		if(time>=1000000)     //set time-out 1s
        		{
        			send_respond(client_sock,408,NULL);//
        			return EXIT_FAILURE;
        		}
                analyze_req(sock,client_sock,buf,readret);
                if (0)//send(client_sock, buf, readret, 0) != readret
                {
                    close_socket(client_sock);
                    close_socket(sock);
                    fprintf(stderr, "Error sending to client.\n");
                    return EXIT_FAILURE;
                }

                memset(buf, 0, BUF_SIZE);
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
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
