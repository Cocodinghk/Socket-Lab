#include "utils.h"

char* HTTP_VERSION_ =  "HTTP/1.1";
char* SERVER_NAME_ = "SERVER";

int msg_resp(int client_sock, char* buf, int readRet){

    Request *request = parse(buf, strlen(buf), client_sock);
    int sendRet = -1;

    if(request == NULL) {
        sendRet = msgResp_400(client_sock);
        return sendRet;
        }

    int reqType = getReqType(request->http_method);

    switch (reqType){
        case GET: 
            sendRet = msgResp_ECHO(client_sock, buf, readRet);
            break;
        case POST: 
            sendRet = msgResp_ECHO(client_sock, buf, readRet);
            break;
        case HEAD: 
            sendRet = msgResp_ECHO(client_sock, buf, readRet); 
            break;
        case MALFORMED: 
            sendRet = msgResp_501(client_sock);
            break;
        default: break;
    }
    
    free(request->headers);
    free(request);
    return sendRet;
}

int getReqType(char* reqType){
    if(!strcmp(reqType, "GET")){
        return GET;
    }
    else if(!strcmp(reqType, "POST")){       
        return POST;
    } 
    else if(!strcmp(reqType, "HEAD")){
        return HEAD;
    }
    else{
        return MALFORMED;
    }
}


int msgResp_501(int client_sock)
{
    char msgResp[1024];
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 501 Not Implemented\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    return sendRet;
}

int msgResp_400(int client_sock)
{
    char msgResp[1024];
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 400 Bad request\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp),0);
    return sendRet;
}

int msgResp_ECHO(int client_sock, char* buf, int readRet){
    int sendRet = send(client_sock, buf, strlen(buf), 0);
    return sendRet;
}

int msgResp_GET(int client_sock, Request* request){

    return 0;
}

int msgResp_POST(int client_sock, Request* request){

    return 0;
}

int msgResp_HEAD(int client_sock, Request* request){

    return 0;
}