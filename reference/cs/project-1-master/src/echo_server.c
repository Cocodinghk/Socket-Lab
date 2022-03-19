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
#define BUF_SIZE 8192
#define LOG_SIZE 200
#define MAX_SOCK 1024
#define BACKLOG 30
#define HTTP_VERSION "HTTP/1.1"
#define ERROR_LOG_FILE "./log/error_log.txt"
#define COMMOM_LOG_FILE "./log/common_log.txt"
FILE *common_log_file,*error_log_file;
int close_socket(int sock){
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
void analyze_type(char *extension,char *type){

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
    FILE *file=common_log_file;
    time_t t;
    char log_to_write[LOG_SIZE],stringtime[100],path[50];
    //file=fopen(COMMOM_LOG_FILE, "a");
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
    fflush(file);
    //fclose(file);
    return ;
}
void error_log(int state,Request *req){
    FILE *file=error_log_file;
    time_t t;
    char log_to_write[LOG_SIZE],stringtime[100],path[50];
    memset(path,0,50);
    //file=fopen(ERROR_LOG_FILE, "a");
    time(&t);
    struct tm* current_time = localtime(&t);
    strftime(log_to_write,100,"127.0.0.1 - - [%d/%b/%Y:%H:%M:%S +0800]", current_time);
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
    fflush(file);
    //fclose(file);
}
void send_respond(int client_sock,int state,Request *req){

    common_log(state,req);
    char respond[BUF_SIZE];
    memset(respond,0,BUF_SIZE);
    char path[1000],extension[10],type[20];
    memset(path,0,1000);
    memset(extension,0,10);
    memset(type,0,20);
    struct stat file_state;
    if(state==400){
        strcat(respond,HTTP_VERSION);
        strcat(respond," ");
        strcat(respond,"400 Bad Request");
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);
        error_log(400,req);
        return ;
    }
    else if(state==408){  //time-out
        strcat(respond,HTTP_VERSION);
        strcat(respond," ");
        strcat(respond,"408 Request Timeout");
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);
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
        
        int file=open(path,O_RDONLY);
        stat(path,&file_state);
        int len=file_state.st_size;
        sendfile(client_sock,file,NULL,len);
        //send file
        /*
        int file=open(path,O_RDONLY);
        stat(path,&file_state);
        int len=file_state.st_size;
        sendfile(client_sock,file,NULL,BUF_SIZE-strlen(respond));
        //printf("process1\n");
        
        
        int file=open(path,O_RDONLY);
        char res_body[BUF_SIZE];
        memset(res_body,0,BUF_SIZE);
        //printf("process2\n");
        read(file,res_body,BUF_SIZE-strlen(respond));
        //printf("process2.5\n");
        sprintf(respond+strlen(respond),"%s",res_body);
        //printf("process3\n");
        send(client_sock,respond,BUF_SIZE,0);
        //printf("process4\n");
        
        */
        
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
        printf("404 start\n");
        strcat(respond,req->http_version);
        strcat(respond," ");
        strcat(respond,"404 Not Found");
        strcat(respond,"\r\n");
        send(client_sock,respond,strlen(respond),0);\
        printf("404 send suc\n");
        error_log(404,req);
        printf("404 log suc\n");
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
}
int analyze_req(int sock,int client_sock,char *buffer,int buff_size){

    Request *req = parse(buffer,buff_size,client_sock);
    printf("analyze-parse succ\n");
    if(req==NULL){
        printf("req is null\n");
        send_respond(client_sock,400,req);

    }
    else if(strcmp(req->http_version,HTTP_VERSION)){
        send_respond(client_sock,505,req);
    }
    else if(!strcmp(req->http_method,"GET")){
        printf("analyze-GET compare succ\n");
        char path[1000];
        memset(path,0,1000);
        strcat(path,".");
        strcat(path,req->http_uri);
        printf("strcat suc:%s\n",path);
        if(access(path,F_OK)){
            printf("file_f_ok");
            send_respond(client_sock,404,req);
        }
        else if(access(path,R_OK)){
            printf("file_r_ok");
            send_respond(client_sock,404,req);
        }
        else {
            printf("send respond 1\n");
            send_respond(client_sock,1,req);
        }
        printf("analyze-GET compare end\n");
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
    printf("analyze-send succ\n");
    
    
    
    //free
    if(req==NULL){
        return 0;
    }
    else {
        int flag=0;
        for(int i=0;i<req->header_count;++i){
            if(!strcmp(req->headers[i].header_name,"Connection")){
                if(!strcmp(req->headers[i].header_value,"close")){
                    printf("cclose\n");
                    flag=-1;
                    break;
                }

            }
    
        }
        if(req->headers!=NULL)
            free(req->headers);
        free(req);
        return flag;
    }
    return 0;

}
int main(int argc, char* argv[])
{

    int count=0;
    int server_sock, client_sock,temp_sock;
    int sock[MAX_SOCK];
    int sock_num=0;
    int max_sock=0;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE*2];
    char temp_buf[BUF_SIZE*2];
    int time=0;
    int time_limit;
    int ret,on;
	struct timeval last[MAX_SOCK];
	struct timeval now;
    struct timeval select_time;
    fd_set sock_set;
    common_log_file=fopen(COMMOM_LOG_FILE,"a");
    error_log_file=fopen(ERROR_LOG_FILE,"a");
    fprintf(stdout, "----- Echo Server -----\n");
    if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }
    //set  socket
    on = 1;
    readret = setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    time_limit=2000;//time 2000s
    
    setsockopt(server_sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&time_limit,sizeof(int) );
    setsockopt(server_sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&time_limit,sizeof(int));
    int nRecvBuf=32*1024;//设置为32K

    setsockopt(server_sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
    //non-blocking
    int flags = fcntl(server_sock, F_GETFL, 0);
    fcntl(server_sock, F_SETFL, flags | O_NONBLOCK);


    //printf("start %d %d\n", )
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(server_sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }
    printf("bind success\n");
    if (listen(server_sock, BACKLOG))
    {
        close_socket(server_sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }
    printf("listen success\n");
    memset(sock,0,MAX_SOCK);
    sock_num=1;
    select_time.tv_sec = 1;
    select_time.tv_usec = 0;
    while (1)
    {

        gettimeofday(&now,NULL);
        FD_ZERO(&sock_set);
        FD_SET(server_sock,&sock_set);
        /*
        temp_sock=accept(server_sock, (struct sockaddr *) &cli_addr,&cli_size);
        if(temp_sock>0){
            printf("accept success\n");
            for(int i=0;i<MAX_SOCK;++i){
                if(!sock[i]){
                    sock[i]=temp_sock;
                    gettimeofday(&last[i],NULL);
                    sock_num++;
                    printf("now sock_num%d\n",sock_num);
                    break;
                }
            }
        }*/
        //printf("check acce block\n");
        temp_sock=accept(server_sock, (struct sockaddr *) &cli_addr,&cli_size);
        max_sock=0;
        while(temp_sock>0){
            max_sock=max_sock>server_sock?max_sock:server_sock;
            printf("accept success\n");
            for(int i=0;i<MAX_SOCK;++i){
                if(!sock[i]){
                    sock[i]=temp_sock;
                    gettimeofday(&last[i],NULL);
                    sock_num++;
                    printf("now sock_num%d\n",sock_num);
                    break;
                }
            }
            temp_sock=accept(server_sock, (struct sockaddr *) &cli_addr,&cli_size);
        }
        for(int i=0;i<MAX_SOCK;++i){
            //printf("check time out:%d\n",i);
            if(!sock[i])continue;
            //printf("check time out:%d, !sock[%d]\n",i,i);
            time=(now.tv_sec-last[i].tv_sec)*1000000+(now.tv_usec-last[i].tv_usec);//微秒
            if(time>=30000000)     //set time-out 1s  plus 1x0s
            {
                send_respond(sock[i],408,NULL);//
                sock[i]=0;
                sock_num--;
                printf("error: time out:%d\n",i);
            }
            FD_SET(sock[i],&sock_set);
            max_sock=sock[i]>max_sock?sock[i]:max_sock;
        }
        //printf("check sele block\n");
        readret = select(max_sock + 1, &sock_set, NULL, NULL,&select_time);
        if(readret<=0){
            //printf("select error readret:%d sock_num%d\n",readret,sock_num);
            continue;
        }
        //printf("select success ,readret:%d\n",readret);
        cli_size = sizeof(cli_addr);
        for(int i=0;i<MAX_SOCK;++i){
            
            if(!sock[i])continue;
            
            memset(buf, 0, BUF_SIZE*2);
            memset(temp_buf, 0, BUF_SIZE*2);
            if(FD_ISSET(sock[i],&sock_set)){
                //printf("now check sock %d\n",i);
                printf("check recv block\n");
                if(readret=recv(sock[i],temp_buf,BUF_SIZE,0)){
                    gettimeofday(&last[i],NULL);
                    strcat(buf,temp_buf);
                    int last_req=0,req_end;
                    for(req_end=3;req_end<BUF_SIZE;++req_end){
                        if(req_end-last_req+1>=4&&buf[req_end-3]=='\r'&&buf[req_end-2]=='\n'&&buf[req_end-1]=='\r'&&buf[req_end]=='\n'){
                            while(BUF_SIZE-1-req_end>=2&&buf[req_end+1]=='\r'&&buf[req_end+2]=='\n')req_end+=2;
                            printf("%d %d  string:%s\n",last_req,req_end,buf);
                            int ff=last_req;
                            last_req=req_end+1;
                            printf("check analyez block\n");
                            if(analyze_req(server_sock,sock[i],buf+ff,req_end-ff+1)==-1){
                                close_socket(sock[i]);
                                FD_CLR(sock[i],&sock_set);
                                sock[i]=0;
                                sock_num--;
                                printf("sock %d closed by client\n",sock[i]);
                                memset(temp_buf, 0, BUF_SIZE*2);
                                memset(buf, 0, BUF_SIZE*2);
                                break;
                            }
                            printf("recv &analyze suc %d\n",++count);
                        }
                        if(buf[req_end]=='\0'){
                            memset(temp_buf, 0, BUF_SIZE*2);
                            memset(buf, 0, BUF_SIZE*2);
                            break;
                        }
                        if(req_end==BUF_SIZE-1){
                            memset(temp_buf, 0, BUF_SIZE*2);
                            strcpy(temp_buf,buf+last_req);
                            memset(buf, 0, BUF_SIZE*2);
                            strcpy(buf,temp_buf);
                        }
                    }
                    memset(temp_buf, 0, BUF_SIZE*2);
                }
                else if(readret==0){
                    close_socket(sock[i]);
                    FD_CLR(sock[i],&sock_set);
                    sock[i]=0;
                    sock_num--;
                    printf("sock %d closed by client\n",sock[i]);
                    continue;
                    
                }
                
                
            }
        }        
    }

    close_socket(server_sock);
    fclose(common_log_file);
    fclose(error_log_file);
    return EXIT_SUCCESS;
}
