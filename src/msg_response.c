#include "msg_response.h"
#include "log.h"
#include <string.h>
#include "cgi.h"

char* HTTP_VERSION_ =  "HTTP/1.1";
char* SERVER_NAME_ = "LISO";


/*
    主方法
*/


int msgs_resp(int client_sock, char* buf, int readRet){
    
    // 查找子串
    char* post_content_place = strstr(buf, "\r\n\r\n");
    char post_content[BUF_SIZE] = "";
    printf("%s\n", buf);
    if (post_content_place != NULL) {
        char new_buf[BUF_SIZE]= "";
        char* p = buf;
        int new_buf_count = 0;
        while(p != post_content_place){
            new_buf[new_buf_count] = *p;
            p += 1;
            new_buf_count += 1;
        }
        new_buf[new_buf_count] = '\0';
        strcat(new_buf, "\r\n\r\n\r\n");
        

        int post_content_count = 0;
        p = post_content_place + 4;
        while(*p != '\0'){
            post_content[post_content_count] = *p;
            p += 1;
            post_content_count += 1;
        }
        post_content[post_content_count] = '\0';

        // printf("buf\n%s\nbufend\n", new_buf);
        // printf("post_content\n%s\npost_content_end\n", post_content);
        strcpy(buf, new_buf);
    }
    

    Requests *requests = parse(buf, strlen(buf), client_sock);
    if (requests == NULL) return 0;
    int request_count = requests->request_count;
    int sendrets = 0;

    for(int i = 0; i < request_count - 1; i ++){
        // printf("responsing the %dth msg...\n", i + 1);
        int sendret = msg_resp(client_sock, buf, readRet, requests->requests_ + i, post_content);
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


int msg_resp(int client_sock, char* buf, int readRet, Request *request, char* post_content){
    

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
            sendRet = msgResp_POST(client_sock, request, post_content); 
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


    FILE *fp = fopen(filePath, "r");


    if (fp < 0){ 
        sendRet = msgResp_404(client_sock);
        error_log("404", request);
        return sendRet;
    }
    else{
        fread(body, 1, st.st_size, fp);
    }
    fclose(fp);

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

    strcat(msgResp, "Content-Type: ");
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
    // printf("%s\n", msgResp);
    return sendRet;
}

int msgResp_POST(int client_sock, Request* request, char* post_content ){
    char ENVP[ENVP_NUM][ENVP_SIZE];
    char* ARGV[] = {
        FILE_NAME,
        NULL
    };

    my_set_ENVP(ENVP, request, "", client_sock);


    int stdin_pipe[2];
    int stdout_pipe[2];
    if (pipe(stdin_pipe) < 0)
    {
        printf("pipe error\n");
        return EXIT_FAILURE;
    }

    if (pipe(stdout_pipe) < 0)
    {
        printf("pipe error\n");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        printf("Fork failed.\n");
        return EXIT_FAILURE;
    }

    if (pid == 0)
    {
        close(stdout_pipe[0]);
        close(stdin_pipe[1]);
        dup2(stdin_pipe[0], fileno(stdin));

        // dup2(stdout_pipe[1], fileno(stdout));
        // dup2(stdout_pipe[1], fileno(stderr));
        // dup2(fileno(stdout), client_sock);
        dup2(client_sock, fileno(stdout));
        
        FILE *fp = fopen("./cgi-bin/cgi_log.txt", "a");
        char* start = "Start\n";
        fwrite(start, 1, strlen(start), fp);
        
        /* pretty much no matter what, if it returns bad things happened... */
        if (execve(FILE_NAME, ARGV, ENVP))
        {
            execve_error_handler(fp);
            // char* error_msg = "execve error";
            // fwrite(error_msg, strlen(error_msg), 1, fp);
            // fclose(fp);
            return EXIT_FAILURE;
        }
        // char* success_msg = "execve success";
        // fwrite(success_msg, strlen(success_msg), 1, fp);
        // fclose(fp);
        // free_ENVP(ENVP); 

    }

    if (pid > 0)
    {
        // free_ENVP(ENVP);
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);
        int writeRet = write(stdin_pipe[1], post_content, strlen(post_content));
        if(writeRet < 0){
            printf("write error\n");
            return EXIT_FAILURE;
        }
        printf("Sent to child: %s\n", post_content);

        close(stdin_pipe[1]); 

        /* you want to be looping with select() telling you when to read */
        // while((readret = read(stdout_pipe[0], buf, BUF_SIZE-1)) > 0)
        // {
        //     buf[readret] = '\0'; /* nul-terminate string */
        //     fprintf(stdout, "Got from CGI: %s\n", buf);
        // }

        return EXIT_SUCCESS;
    }

    printf("Unknown error\n");
    return EXIT_FAILURE;

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