#ifndef __UTILS__
#define __UTILS__


#include "parse.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

#define GET 0
#define POST 1
#define HEAD 2
#define MALFORMED -1





int msg_resp(int client_sock, char* buf, int readRet);
int getReqType(char* reqType);
int msgResp_501(int client_sock);
int msgResp_400(int client_sock);
int msgResp_ECHO(int client_sock, char* buf, int readRet);
int msgResp_GET(int client_sock, Request* request);
int msgResp_POST(int client_sock, Request* request);
int msgResp_HEAD(int client_sock, Request* request);


#endif