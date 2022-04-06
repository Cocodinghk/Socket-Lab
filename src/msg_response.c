#include "msg_response.h"
#include "log.h"

char* HTTP_VERSION_ =  "HTTP/1.1";
char* SERVER_NAME_ = "LISO";


/*
    主方法
*/


int msgs_resp(int client_sock, char* buf, int readRet){
    Requests *requests = parse(buf, strlen(buf), client_sock);
    if (requests == NULL) return 0;
    int request_count = requests->request_count;
    int sendrets = 0;

    for(int i = 0; i < request_count - 1; i ++){
        // printf("responsing the %dth msg...\n", i + 1);
        int sendret = msg_resp(client_sock, buf, readRet, requests->requests_ + i);
        sendrets += sendret;
        // printf("Successfully responsed the %dth msg : %d\n", i + 1, sendret);
        // for(int i = 0; i < 1000; i++)
        //     for(int j = 0; j < 1000; j ++);
    }

    printf("Successfully responsed %d msg to client %d : %d\n", request_count - 1, client_sock, sendrets);

    for(int i = 0; i < request_count - 1; i ++){
        free(requests->requests_[i].headers);
    }   
    free(requests->requests_);
    free(requests);
    
    return sendrets;
}


int msg_resp(int client_sock, char* buf, int readRet, Request *request){
    

    int sendRet = 1;
    
    if(request->format_correctness == 0) {
        sendRet = msgResp_400(client_sock);
        error_log("400", request);
        return sendRet;
        }
    
    
    int reqType = get_reqType(request->http_method);

    switch (reqType){
        case GET: 
            sendRet = msgResp_GET(client_sock, request);
            // sendRet = msgResp_ECHO(client_sock, buf, readRet, request); 
            break;
        case POST: 
            sendRet = msgResp_ECHO(client_sock, buf, readRet, request); 
            break;
        case HEAD: 
            sendRet = msgResp_HEAD(client_sock, request);
            // sendRet = msgResp_ECHO(client_sock, buf, readRet, request); 
            break;
        case NOTIMPLEMENTED: 
            sendRet = msgResp_501(client_sock);
            error_log("501", request);
            break;
        default: break;
    }


    return sendRet;
}


/*
    方法响应
*/



int msgResp_GET(int client_sock, Request* request){

    int sendRet = 0;
    char msgResp[RESPONSE_SIZE] = "";


    char filePath[PATH_SIZE] = "";
    strcpy(filePath, request->http_uri);
    char body[BODY_SIZE] = "";

    if (!strcmp(filePath, "/")){
        strcpy(filePath, "./static_site/index.html");
    }
    else{
        char temp[PATH_SIZE] = "";
        strcpy(temp, filePath);
        strcpy(filePath, "./static_site");
        strcat(filePath, temp);
    };

    struct stat st;
    int temp = stat(filePath, &st);
    if (temp == -1)
    {
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
        return sendRet;
    }

    int fp = open(filePath, O_RDONLY);
    if (fp < 0){ 
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
        return sendRet;
    }
    else{
        read(fp, body, BODY_SIZE);
    }
    close(fp);

    if(strcmp(request->http_version, HTTP_VERSION_)) {
        sendRet = msgResp_505(client_sock);
        error_log("505", request);
        return sendRet;
    }

    char date_stamp[DATE_STAMP_SIZE] = "";
    get_dateStamp(date_stamp);
    char content_length[CONTENT_LENGTH_SIZE] = "";
    get_contentLength(content_length, filePath);
    char content_type[CONTENT_TYPE_SIZE] = "";
    get_contentType(content_type, filePath);
    char last_modified[LAST_MODIFIED_SIZE] = "";
    get_lastModified(last_modified, filePath);
    char connection_type[CONNECTION_TYPE_SIZE] = "";
    get_connectionType(connection_type, request);

    
    
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
    char msgResp[RESPONSE_SIZE] = "";

    char filePath[PATH_SIZE];
    char date_stamp[DATE_STAMP_SIZE] = "";
    get_dateStamp(date_stamp);
    char content_length[CONTENT_LENGTH_SIZE] = "";
    get_contentLength(content_length, filePath);
    char content_type[CONTENT_TYPE_SIZE] = "";
    get_contentType(content_type, filePath);
    char last_modified[LAST_MODIFIED_SIZE] = "";
    get_lastModified(last_modified, filePath);
    char connection_type[CONNECTION_TYPE_SIZE] = "";
    get_connectionType(connection_type, request);
    char body[BODY_SIZE] = "";

    strcpy(filePath, request->http_uri);
    // if(access(filePath, F_OK) == -1){
    //     sendRet = msgResp_404(client_sock);
        error_log("404", request);
    //     return sendRet;
    // }
    
    if(strcmp(request->http_version, HTTP_VERSION_)) {
        sendRet = msgResp_505(client_sock);
        error_log("505", request);
        return sendRet;
    }
    

    int fp = open(filePath, O_RDONLY);
    if (fp < 0){ 
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
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
    char msgResp[RESPONSE_SIZE] = "";

    char filePath[PATH_SIZE] = "";
    strcpy(filePath, request->http_uri);

    struct stat st;
    if(stat(filePath, &st) == -1)
    {
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
        return sendRet;
    }

    int fp = open(filePath, O_RDONLY);
    if (fp < 0){ 
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
        return sendRet;
    }
    close(fp);
    
    if(strcmp(request->http_version, HTTP_VERSION_)) {
        sendRet = msgResp_505(client_sock);
        error_log("505", request);
        return sendRet;
    }
    
    char date_stamp[DATE_STAMP_SIZE] = "";
    get_dateStamp(date_stamp);
    char content_length[CONTENT_LENGTH_SIZE] = "";
    get_contentLength(content_length, filePath);
    char content_type[CONTENT_TYPE_SIZE] = "";
    get_contentType(content_type, filePath);
    char last_modified[LAST_MODIFIED_SIZE] = "";
    get_lastModified(last_modified, filePath);
    char connection_type[CONNECTION_TYPE_SIZE] = "";
    get_connectionType(connection_type, request);
    
        
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


    sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    access_log(request, sendRet);

    return sendRet;
}


int msgResp_ECHO(int client_sock, char* buf, int readRet, Request * request){
    if(strcmp(request->http_version, HTTP_VERSION_)) {
        int sendRet = msgResp_505(client_sock);
        error_log("505", request);
        return sendRet;
    }
    int sendRet = send(client_sock, buf, strlen(buf), 0);
    access_log(request, sendRet);
    return sendRet;
}



/*
    状态码响应.
*/



int msgResp_501(int client_sock){
    char msgResp[RESPONSE_SIZE] = "";
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 501 Not Implemented\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    
    return sendRet;
}

int msgResp_400(int client_sock){
    char msgResp[RESPONSE_SIZE] = "";
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 400 Bad request\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp),0);
    return sendRet;
}

int msgResp_404(int client_sock){
    char msgResp[RESPONSE_SIZE] = "";
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 404 Not Found\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    return sendRet;
}

int msgResp_500(int client_sock){
    char msgResp[RESPONSE_SIZE] = "";
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 500 Internal Server Error\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    return sendRet;
}

int msgResp_505(int client_sock){
    char msgResp[RESPONSE_SIZE] = "";
    
    strcat(msgResp, HTTP_VERSION_);
    strcat(msgResp, " 505 HTTP Version Not Supported\r\n\r\n");
    int sendRet = send(client_sock, msgResp, strlen(msgResp), 0);
    return sendRet;
}