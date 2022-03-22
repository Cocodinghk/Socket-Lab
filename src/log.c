#include "log.h"


char* ACCESS_LOG_PATH = "./log/access_log.txt";
char* ERROR_LOG_PATH = "./log/error_log.txt";

void access_log(Request * request, int sendRet){

    char log_row[LOG_ROW_SIZE];

    time_t cur_t;
    time(&cur_t);
    struct tm* cur_time = localtime(&cur_t);
    strftime(log_row, 100, "127.0.0.1 - - [%d/%b/%Y:%H:%M:%S +0800] \"", cur_time);

    strcat(log_row, request->http_method);
    strcat(log_row, " ");

    strcat(log_row, request->http_uri);
    strcat(log_row, " ");

    strcat(log_row, request->http_version);
    strcat(log_row, "\" ");

    strcat(log_row, "200");
    strcat(log_row, " ");

    char sendRet_str[300];
    sprintf(sendRet_str, "%d", sendRet);
    strcat(log_row, sendRet_str);

    strcat(log_row, "\n");

    FILE *access_log_file = fopen(ACCESS_LOG_PATH, "a+");
    fwrite(log_row, strlen(log_row), 1, access_log_file);
    fclose(access_log_file);
    return;
}

void error_log(char* state_code_str, Request * request){
    int state_code = get_state_code_type(state_code_str);
    FILE *error_log_file = fopen(ERROR_LOG_PATH, "a+");
    char log_row[LOG_ROW_SIZE];

    pid_t pid;
    pid = getpid();

    pthread_t tid;
    tid = pthread_self(); //线程id  

    time_t cur_t;
    time(&cur_t);
    struct tm* cur_time = localtime(&cur_t);
    strftime(log_row, 100, "[%d/%b/%Y:%H:%M:%S +0800] ", cur_time);

    strcat(log_row, "[core:error]");
    strcat(log_row, " ");

    strcat(log_row, "[pid ");
    char pid_str[30];
    sprintf(pid_str, "%d", pid);
    strcat(log_row, pid_str);

    strcat(log_row, ":tid ");
    char tid_str[30];
    sprintf(tid_str, "%d", tid);
    strcat(log_row, tid_str);
    strcat(log_row, "] ");

    strcat(log_row, "[client 127.0.0.1]");
    strcat(log_row, " ");

    switch(state_code){
        case _400_:
            strcat(log_row, "400：Bad Request");
            break;
        case _404_:
            strcat(log_row, "404：File Not Found File does not exist: ");
            strcat(log_row, request->http_uri);
            break;
        case _501_:
            strcat(log_row, "501：Not Implemented");
            break;
        case _505_:
            strcat(log_row, "505：HTTP Version Not Supported");
            break;
        default:
            break;
    }

    strcat(log_row, "\n");

    fwrite(log_row, strlen(log_row), 1, error_log_file);
    fclose(error_log_file);
    return;
}