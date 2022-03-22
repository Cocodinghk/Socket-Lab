#ifndef __UTILS__
#define __UTILS__


#include "parse.h"
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>


int get_reqType(char* reqType);
int get_state_code_type(char* state_code);
void get_contentLength(char* content_length, const char* filePath);
void get_contentType(char* content_type, const char* filePath);
void get_fileExtension(char* fileExtention, const char *filePath);
void get_lastModified(char* last_modified, const char* filePath);
void get_connectionType(char* connection_type, Request * request);
void get_dateStamp();

#endif