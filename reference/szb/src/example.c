/* C declarations used in actions */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "parse.h"

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

void GET(char * buf,char* buf_send)
{
    strcpy(buf_send,buf);
}

void POST(char * buf,char* buf_send)
{
    strcpy(buf_send,buf);
}

void HEAD(char * buf,char* buf_send)
{
    strcpy(buf_send,buf);
}

void use_method(char* met,char* buf,char* buf_send)
{
   int len_method = strlen(met);
   if(len_method>4)
   {
       strcpy(buf_send,"501 Method Unimplemented.\r\n\r\n");
   }
   else if(len_method == 4)
   {
       if(met[0] == 'P'&&met[1]=='O'&&met[2]=='S'&&met[3]=='T')
       {
           POST(buf,buf_send);
       }
       else if(met[0] == 'H'&&met[1]=='E'&&met[2]=='A'&&met[3]=='D')
       {
           HEAD(buf,buf_send);
       }
       else{
           strcpy(buf_send,"HTTP/1.1 501 Method Unimplemented\r\n\r\n");
       }
   }
   else if(len_method == 3)
   {
       if(met[0] == 'G'&&met[1]=='E'&&met[2]=='T')
       {
           GET(buf,buf_send);
       }
       else
       {
           strcpy(buf_send,"HTTP/1.1 501 Method Unimplemented\r\n\r\n");
       }
   }
   else
   {
       strcpy(buf_send,"HTTP/1.1 501 Method Unimplemented\r\n\r\n");
   }
   
}

int main(int argc, char **argv){
  //Read from the file the sample
  int fd_in = open(argv[1], O_RDONLY);
  int index;
  char buf[8192];
	if(fd_in < 0) {
		printf("Failed to open the file\n");
		return 0;
	}
  int readRet = read(fd_in,buf,8192);
  //Parse the buffer to the parse function. You will need to pass the socket fd and the buffer would need to
  //be read from that fd
  Request *request = parse(buf,readRet,fd_in);
  //Just printing everything
  printf("Http Method %s\n",request->http_method);
  printf("Http Version %s\n",request->http_version);
  printf("Http Uri %s\n",request->http_uri);
  for(index = 0;index < request->header_count;index++){
    printf("Request Header\n");
    printf("Header name %s Header Value %s\n",request->headers[index].header_name,request->headers[index].header_value);
  }
  free(request->headers);
  free(request);
  return 0;
}
