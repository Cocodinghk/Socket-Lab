#ifndef __CGI__
#define __CGI__
#define ENVP_NUM   30
#define ENVP_SIZE   500
#define FILE_NAME "./cgi-bin/cgi.py"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "parse.h"
#include "stdio.h"

void execve_error_handler(FILE *fp);
void my_set_ENVP(char **envp, Request* request, char *cgiquery, int client_sock);
void free_ENVP(char **ENVP);

#endif