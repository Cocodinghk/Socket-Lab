#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SUCCESS 0

//Header field
typedef struct
{
	char header_name[512];
	char header_value[1024];
} Request_header;

//HTTP Request Header
typedef struct
{
	char http_version[16];
	char http_method[16];
	char http_uri[512];
	Request_header *headers;
	int header_count;
	int judge;
} Request;

typedef struct
{
	Request *reqlis;
	int count;
} Serverreq;

Serverreq * parse(char *buffer, int size,int socketFd);

// functions decalred in parser.y
int yyparse();
void set_parsing_options(char *buf, size_t i, Serverreq *serverreq);
