#include "msg_response.h"
#include "log.h"

#define LPRINTF(...) printf(__VA_ARGS__)

char* HTTP_VERSION_ =  "HTTP/1.1";
char* SERVER_NAME_ = "SERVER";


/*
    主方法
*/

int msg_resp(int client_sock, char* buf, int readRet){

    Request *request = parse(buf, strlen(buf), client_sock);
    int sendRet = -1;
    
    if(request == NULL) {
        sendRet = msgResp_400(client_sock);
        error_log("400", request);
        return sendRet;
        }
    

    int reqType = get_reqType(request->http_method);

    switch (reqType){
        case GET: 
            sendRet = msgResp_GET(client_sock, request);
            break;
        case POST: 
            sendRet = msgResp_ECHO(client_sock, buf, readRet, request); 
            break;
        case HEAD: 
            sendRet = msgResp_HEAD(client_sock, request);
            break;
        case NOTIMPLEMENTED: 
            sendRet = msgResp_501(client_sock);
            error_log("501", request);
            break;
        default: break;
    }

    free(request->headers);
    free(request);
    return sendRet;
}


/*
    方法响应
*/



int msgResp_GET(int client_sock, Request* request){


    int sendRet;
    char msgResp[RESPONSE_SIZE];

    char filePath[PATH_SIZE];
    char date_stamp[DATE_STAMP_SIZE];
    get_dateStamp(date_stamp);
    char content_length[CONTENT_LENGTH_SIZE];
    get_contentLength(content_length, filePath);
    char content_type[CONTENT_TYPE_SIZE];
    get_contentType(content_type, filePath);
    char last_modified[LAST_MODIFIED_SIZE];
    get_lastModified(last_modified, filePath);
    char connection_type[CONNECTION_TYPE_SIZE];
    get_connectionType(connection_type, request);
    char body[BODY_SIZE];

    strcpy(filePath, request->http_uri);
    if(access(filePath, F_OK) == -1){
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
        return sendRet;
    }
    
    if(strcmp(request->http_version, HTTP_VERSION_)) {
        sendRet = msgResp_505(client_sock);
        error_log("505", request);
        return sendRet;
    }
    

    int fp = open(filePath, O_RDONLY);
    if (fp < 0){ 
        sendRet = msgResp_404(client_sock);
        return sendRet;
    }
    else{
        read(fp, body, BODY_SIZE);
    }
    close(fp);

        
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 200 OK\r\n");


    strcat(msgResp, "Server: ");
    strcat(msgResp, SERVER_NAME_);
    strcat(msgResp, "\r\n");
    
    strcat(msgResp, "Date: ");
    strcat(msgResp, date_stamp);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Content-Length: ");
    strcat(msgResp, content_length);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Content-type: ");
    strcat(msgResp,  content_type);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Last-modified: ");
    strcat(msgResp, last_modified);
    strcat(msgResp, "\r\n");

    if (!strcmp(connection_type, "close")) strcat(msgResp, "Connection: close\r\n");
    else strcat(msgResp, "Connection: keep-alive\r\n");

    strcat(msgResp, "\r\n");

    strcat(msgResp, body);

    sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    access_log(request, sendRet);

    return sendRet;
}

int msgResp_POST(int client_sock, Request* request){


    int sendRet;
    char msgResp[RESPONSE_SIZE];

    char filePath[PATH_SIZE];
    char date_stamp[DATE_STAMP_SIZE];
    get_dateStamp(date_stamp);
    char content_length[CONTENT_LENGTH_SIZE];
    get_contentLength(content_length, filePath);
    char content_type[CONTENT_TYPE_SIZE];
    get_contentType(content_type, filePath);
    char last_modified[LAST_MODIFIED_SIZE];
    get_lastModified(last_modified, filePath);
    char connection_type[CONNECTION_TYPE_SIZE];
    get_connectionType(connection_type, request);
    char body[BODY_SIZE];

    strcpy(filePath, request->http_uri);
    if(access(filePath, F_OK) == -1){
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
        return sendRet;
    }
    
    if(strcmp(request->http_version, HTTP_VERSION_)) {
        sendRet = msgResp_505(client_sock);
        error_log("505", request);
        return sendRet;
    }
    

    int fp = open(filePath, O_RDONLY);
    if (fp < 0){ 
        sendRet = msgResp_404(client_sock);
        return sendRet;
    }
    else{
        read(fp, body, BODY_SIZE);
    }
    close(fp);

        
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 200 OK\r\n");


    strcat(msgResp, "Server: ");
    strcat(msgResp, SERVER_NAME_);
    strcat(msgResp, "\r\n");
    
    strcat(msgResp, "Date: ");
    strcat(msgResp, date_stamp);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Content-Length: ");
    strcat(msgResp, content_length);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Content-type: ");
    strcat(msgResp,  content_type);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Last-modified: ");
    strcat(msgResp, last_modified);
    strcat(msgResp, "\r\n");

    if (!strcmp(connection_type, "close")) strcat(msgResp, "Connection: close\r\n");
    else strcat(msgResp, "Connection: keep-alive\r\n");

    strcat(msgResp, "\r\n");

    strcat(msgResp, body);

    sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    access_log(request, sendRet);

    return sendRet;
}


int msgResp_HEAD(int client_sock, Request* request){
    int sendRet;
    char msgResp[RESPONSE_SIZE];

    char filePath[PATH_SIZE];
    char date_stamp[DATE_STAMP_SIZE];
    get_dateStamp(date_stamp);
    char content_length[CONTENT_LENGTH_SIZE];
    get_contentLength(content_length, filePath);
    char content_type[CONTENT_TYPE_SIZE];
    get_contentType(content_type, filePath);
    char last_modified[LAST_MODIFIED_SIZE];
    get_lastModified(last_modified, filePath);
    char connection_type[CONNECTION_TYPE_SIZE];
    get_connectionType(connection_type, request);

    strcpy(filePath, request->http_uri);
    if(access(filePath, F_OK) == -1){
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
        return sendRet;
    }

    if(strcmp(request->http_version, HTTP_VERSION_)) {
        sendRet = msgResp_505(client_sock);
        error_log("505", request);
        return sendRet;
    }

    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 200 OK\r\n");

    strcat(msgResp, "Server: ");
    strcat(msgResp, SERVER_NAME_);
    strcat(msgResp, "\r\n");
    
    strcat(msgResp, "Date: ");
    strcat(msgResp, date_stamp);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Content-Length: ");
    strcat(msgResp, content_length);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Content-type: ");
    strcat(msgResp,  content_type);
    strcat(msgResp, "\r\n");

    strcat(msgResp, "Last-modified: ");
    strcat(msgResp, last_modified);
    strcat(msgResp, "\r\n");

    if (!strcmp(connection_type, "close")) strcat(msgResp, "Connection: close\r\n\r\n");
    else strcat(msgResp, "Connection: keep-alive\r\n\r\n");

    strcat(msgResp, "\r\n");

    sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    access_log(request, sendRet);
    return sendRet;
}



int msgResp_ECHO(int client_sock, char* buf, int readRet, Request * request){
    if(strcmp(request->http_version, HTTP_VERSION_)) {
        int sendRet = msgResp_505(client_sock);
        return sendRet;
    }
    int sendRet = send(client_sock, buf, strlen(buf), 0);
    return sendRet;
}



/*
    状态码响应.
*/



int msgResp_501(int client_sock){
    char msgResp[RESPONSE_SIZE];
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 501 Not Implemented\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    
    return sendRet;
}

int msgResp_400(int client_sock){
    char msgResp[RESPONSE_SIZE];
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 400 Bad request\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp),0);
    return sendRet;
}

int msgResp_404(int client_sock){
    char msgResp[RESPONSE_SIZE];
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 404 Not Found\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    return sendRet;
}

int msgResp_500(int client_sock){
    char msgResp[RESPONSE_SIZE];
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 500 Internal Server Error\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    return sendRet;
}

int msgResp_505(int client_sock){
    char msgResp[RESPONSE_SIZE];
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 505 HTTP Version Not Supported\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    return sendRet;
}