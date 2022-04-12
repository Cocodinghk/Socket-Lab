#include "cgi.h"

void execve_error_handler(FILE* fp)
{
    switch (errno)
    {
        case E2BIG:
            fprintf(stderr, "The total number of bytes in the environment \
(envp) and argument list (argv) is too large.\n");
            char *erron_0 = "E2BIG\n";
            fwrite(erron_0, 1, strlen(erron_0), fp);
            return;
        case EACCES:
            fprintf(stderr, "Execute permission is denied for the file or a \
script or ELF interpreter.\n");
            char *erron_1 = "EACCES\n";
            fwrite(erron_1, 1, strlen(erron_1), fp);
            return;
        case EFAULT:
            fprintf(stderr, "filename points outside your accessible address \
space.\n");
            char *erron_2 = "EFAULT\n";
            fwrite(erron_2, 1, strlen(erron_2), fp);
            return;
        case EINVAL:
            fprintf(stderr, "An ELF executable had more than one PT_INTERP \
segment (i.e., tried to name more than one \
interpreter).\n");
            char *erron_3 = "EINVAL\n";
            fwrite(erron_3, 1, strlen(erron_3), fp);
            return;
        case EIO:
            fprintf(stderr, "An I/O error occurred.\n");
            char *erron_4 = "EIO\n";
            fwrite(erron_4, 1, strlen(erron_4), fp);
            return;
        case EISDIR:
            fprintf(stderr, "An ELF interpreter was a directory.\n");
            char *erron_5 = "EISDIR\n";
            fwrite(erron_5, 1, strlen(erron_5), fp);
            return;
        case ELOOP:
            fprintf(stderr, "Too many symbolic links were encountered in \
resolving filename or the name of a script \
or ELF interpreter.\n");
            char *erron_6 = "ELOOP\n";
            fwrite(erron_6, 1, strlen(erron_6), fp);
            return;
        case EMFILE:
            fprintf(stderr, "The process has the maximum number of files \
open.\n");
            char *erron_7 = "EMFILE\n";
            fwrite(erron_7, 1, strlen(erron_7), fp);
            return;
        case ENAMETOOLONG:
            fprintf(stderr, "filename is too long.\n");
            char *erron_8 = "ENAMETOOLONG\n";
            fwrite(erron_8, 1, strlen(erron_8), fp);
            return;
        case ENFILE:
            fprintf(stderr, "The system limit on the total number of open \
files has been reached.\n");
            char *erron_9 = "ENFILE\n";
            fwrite(erron_9, 1, strlen(erron_9), fp);

            return;
        case ENOENT:
            fprintf(stderr, "The file filename or a script or ELF interpreter \
does not exist, or a shared library needed for \
file or interpreter cannot be found.\n");
            char *erron_10 = "ENOENT\n";
            fwrite(erron_10, 1, strlen(erron_10), fp);
            return;
        case ENOEXEC:
            fprintf(stderr, "An executable is not in a recognised format, is \
for the wrong architecture, or has some other \
format error that means it cannot be \
executed.\n");
            char *erron_11 = "ENOEXEC\n";
            fwrite(erron_11, 1, strlen(erron_11), fp);
            return;
        case ENOMEM:
            fprintf(stderr, "Insufficient kernel memory was available.\n");
            char *erron_12 = "ENOMEM\n";
            fwrite(erron_12, 1, strlen(erron_12), fp);
            return;
        case ENOTDIR:
            fprintf(stderr, "A component of the path prefix of filename or a \
script or ELF interpreter is not a directory.\n");
            char *erron_13 = "ENOTDIR\n";
            fwrite(erron_13, 1, strlen(erron_13), fp);
            return;
        case EPERM:
            fprintf(stderr, "The file system is mounted nosuid, the user is \
not the superuser, and the file has an SUID or \
SGID bit set.\n");
            char *erron_14 = "EPERM\n";
            fwrite(erron_14, 1, strlen(erron_14), fp);
            return;
        case ETXTBSY:
            fprintf(stderr, "Executable was open for writing by one or more \
processes.\n");
            char *erron_15 = "ETXTBSY\n";
            fwrite(erron_15, 1, strlen(erron_15), fp);
            return;
        default:
            fprintf(stderr, "Unkown error occurred with execve().\n");
            char *erron_16 = "UNKNOWN\n";
            fwrite(erron_16, 1, strlen(erron_16), fp);
            return;
    }
}


void free_ENVP(char **ENVP) {
    int i = 0;
    while (ENVP[i] != NULL)
        free(ENVP[i++]);
}


void my_set_ENVP(char **envp, Request* request, char *queryinfo, int client_sock){

    struct sockaddr_in clientaddr1;
    memset(&clientaddr1, 0x00, sizeof(clientaddr1));
    socklen_t nl=sizeof(clientaddr1);
    getpeername(client_sock,(struct sockaddr*)&clientaddr1,&nl);
    char* client_addr=inet_ntoa(clientaddr1.sin_addr);



    int i = 0;
    char temp[100];

    envp[i++] = "GATEWAY_INTERFACE=CGI/1.1";
    // strcpy(envp[i++], "GATEWAY_INTERFACE=CGI/1.1");


    sprintf(temp, "PATH_INFO=%s", request->http_uri+8);
    envp[i++] = temp;

    sprintf(temp, "REQUEST_URI=%s", request->http_uri);
    envp[i++] = temp;
    if (*queryinfo != '\0') {
        sprintf(temp, "QUERY_STRING=%s", queryinfo);
        envp[i++] = temp;
    }
    envp[i++] = "SCRIPT_NAME=/cgi.py";
    // strcpy(envp[i++], "SCRIPT_NAME=/cgi.py");
    sprintf(temp, "REMOTE_ADDR=%s", client_addr);
    envp[i++] = temp;
    sprintf(temp, "REQUEST_METHOD=%s", request->http_method);
    envp[i++] = temp;
    sprintf(temp, "SERVER_PORT=%s", "80");
    envp[i++] = temp;
    envp[i++] = "SERVER_PROTOCOL=HTTP/1.1";
    envp[i++] = "SERVER_SOFTWARE=Liso/1.0";
    envp[i++] = "SERVER_NAME=Liso";
    // strcpy(envp[i++], "SERVER_PROTOCOL=HTTP/1.1");
    // strcpy(envp[i++], "SERVER_SOFTWARE=Liso/1.0");
    // strcpy(envp[i++], "SERVER_NAME=Liso");

    int header_cnt = request->header_count;
    Request_header* headers_ = request->headers;

    int cnt = 0;
    while (cnt < header_cnt) {       
        if (!strcasecmp(headers_[cnt].header_name, "Content-Length")) {
            sprintf(temp, "CONTENT_LENGTH=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Content-Type")) {
            sprintf(temp, "CONTENT_TYPE=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Accept")) {
            sprintf(temp, "HTTP_ACCEPT=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Referer")) {
            sprintf(temp, "HTTP_REFERER=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Accept-Encoding")) {
            sprintf(temp, "HTTP_ACCEPT_ENCODING=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Accept-Language")) {
            sprintf(temp, "HTTP_ACCEPT_LANGUAGE=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Accept-Charset")) {
            sprintf(temp, "HTTP_ACCEPT_CHARSET=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Host")) {
            sprintf(temp, "HTTP_HOST=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Cookie")) {
            sprintf(temp, "HTTP_COOKIE=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "User-Agent")) {
            sprintf(temp, "HTTP_USER_AGENT=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        } else if (!strcasecmp(headers_[cnt].header_name, "Connection")) {
            sprintf(temp, "HTTP_CONNECTION=%s", headers_[cnt].header_name);
            envp[i++] = temp;
        }
        cnt ++;
    }
    envp[i] = NULL;


}