#ifndef __PARSE__
#define __PARSE__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define SUCCESS 0
#define REQUESTS_NUM 100
#define BUF_SIZE 81920

//Header field
typedef struct
{
	char header_name[4096];
	char header_value[4096];
} Request_header;

//HTTP Request Header
typedef struct
{
	char http_version[50];
	char http_method[50];
	char http_uri[4096];
	Request_header *headers;
	int header_count;
	int format_correctness;
} Request;

typedef struct
{
	Request *requests_;
	int request_count;
} Requests;


Requests* parse(char *buffer, int size,int socketFd);

// functions decalred in parser.y
int yyparse();
void set_parsing_options(char *buf, size_t i, Requests *requests);


#endif