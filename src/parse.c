#include "../include/parse.h"

/**
* Given a char buffer returns the parsed request headers
*/
#define BUF_SIZE 81920
extern void yyrestart(FILE *);
Requests * parse(char *buffer, int size, int socketFd) {
  //Differant states in the state machine
	enum {
		STATE_START = 0, STATE_CR, STATE_CRLF, STATE_CRLFCR, STATE_CRLFCRLF
	};

	// int i = 0, state = STATE_CRLFCRLF;
	// size_t offset = 0;
	// char ch;
	char buf[BUF_SIZE] = "";
	memset(buf, 0, BUF_SIZE);
	strcpy(buf, buffer);

	// state = STATE_START;
	// while (state != STATE_CRLFCRLF) {
	// 	char expected = 0;

	// 	if (i == size)
	// 		break;

	// 	ch = buffer[i++];
	// 	buf[offset++] = ch;

	// 	switch (state) {
	// 	case STATE_START:
	// 	case STATE_CRLF:
	// 		expected = '\r';
	// 		break;
	// 	case STATE_CR:
	// 	case STATE_CRLFCR:
	// 		expected = '\n';
	// 		break;
	// 	default:
	// 		state = STATE_START;
	// 		continue;
	// 	}

	// 	if (ch == expected)
	// 		state++;
	// 	else
	// 		state = STATE_START;

	// }

	

    //Valid End State
	if (1) {
		Requests *requests = (Requests *) malloc(sizeof(Requests));
		requests->request_count = 1;
		requests->requests_ = (Request *) malloc(sizeof(Request) * requests->request_count);
		requests->requests_[0].format_correctness = 1; 
        requests->requests_[0].header_count = 0;        //TODO You will need to handle resizing this in parser.y
        requests->requests_[0].headers = (Request_header *) malloc(sizeof(Request_header) * (requests->requests_[0].header_count + 1));
		yyrestart(NULL);
		set_parsing_options(buf, size, requests);
		if (yyparse() == SUCCESS) {
			printf("Successfully returned!\n\n");
            return requests;
		}
		printf("Requests parsing failed!\n\n");
		return NULL;
	}
    //TODO Handle Malformed Requests

	memset(buf, 0, BUF_SIZE);
    printf("Requests can't match the automata!\n");
	return NULL;
}
