#include "utils.h"
#include "msg_response.h"



void get_dateStamp(char *date_stamp){

    time_t sys_time_;
    struct tm * timeinfo;

    time(&sys_time_);
    timeinfo = localtime(&sys_time_);
    strftime(date_stamp, DATE_STAMP_SIZE, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

}

void get_contentLength(char* content_length, const char* filePath){
    struct stat state;
    stat(filePath, &state);
    sprintf(content_length, "%lld", state.st_size);

}

void get_contentType(char* content_type, const char* filePath){
    char fileExtension[FILE_EXTENSION_SIZE] = "";
    get_fileExtension(fileExtension, filePath);

    if (!strcmp(fileExtension, "html")) strcpy(content_type, "text/html");
    else if (!strcmp(fileExtension, "gif")) strcpy(content_type, "image/gif");
    else if (!strcmp(fileExtension, "jpeg")) strcpy(content_type, "image/jpeg");
    else if (!strcmp(fileExtension, "css")) strcpy(content_type, "text/css");
    else if (!strcmp(fileExtension, "png")) strcpy(content_type, "image/png");
    else strcpy(content_type, "application/octet-stream");

}

void get_fileExtension(char* fileExtention, const char *filePath){
    
    
    int length = strlen(filePath);
    int i = length - 1;
    while(i){
        if (filePath[i] == '.' )
            break;
        i--;
    }
    if(i >= 0)
        strcpy(fileExtention, filePath + i + 1);
    else
        strcpy(fileExtention, "\0");

}

void get_lastModified(char* last_modified, const char* filePath){
    struct stat st;
    struct tm *curr_gmt_time = NULL;
    stat(filePath, &st);
    curr_gmt_time = gmtime(&st.st_mtime);
    strftime(last_modified, LAST_MODIFIED_SIZE, "%a, %d %b %Y %H:%M:%S %Z", curr_gmt_time);
    
}

void get_connectionType(char* connection_type, Request * request){
    char header[HEADER_SIZE] = "";
    strcpy(header, "Connection");
    for (int i = 0; i < request->header_count; i++)
        if (!strcmp(request->headers[i].header_name, header))
        {
            strcpy(connection_type, request->headers[i].header_value);
            return;
        }
}

int get_reqType(char* reqType){
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
        return NOTIMPLEMENTED;
    }
}


int get_state_code_type(char* state_code){
    if(!strcmp(state_code, "400")){
        return _400_;
    }
    else if(!strcmp(state_code, "404")){       
        return _404_;
    } 
    else if(!strcmp(state_code, "501")){
        return _501_;
    }
    else if(!strcmp(state_code, "505")){
        return _505_;
    }
    else return NOTIMPLEMENTED;
}
