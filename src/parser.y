/**
 * @file parser.y
 * @brief Grammar for HTTP
 * @author Rajul Bhatnagar (2016)
 */

%{
#include "parse.h"

/* Define YACCDEBUG to enable debug messages for this lex file */
//#define YACCDEBUG
#define YYERROR_VERBOSE
#ifdef YACCDEBUG
#include <stdio.h>
#define YPRINTF(...) printf(__VA_ARGS__)
#else
#define YPRINTF(...)
#endif

/* yyparse() calls yyerror() on error */
void yyerror (const char *s);

void set_parsing_options(char *buf, size_t siz, Requests *parsing_requests);

/* yyparse() calls yylex() to get tokens */
extern int yylex();


/*
** Global variables required for parsing from buffer
** instead of stdin:
*/

/* Pointer to the buffer that contains input */
char *parsing_buf;

/* Current position in the buffer */
int parsing_offset;

/* Buffer size */
size_t parsing_buf_siz;


/* Current parsing_request Header Struct */
Requests* parsing_requests;

%}


/* Various types values that we can get from lex */
%union {
	char str[8192];
	int i;
}

%start requests_

/*
 * Tokens that yacc expects from lex, essentially these are the tokens
 * declared in declaration section of lex file.
 */
%token t_crlf
%token t_backslash
%token t_slash
%token t_digit
%token t_dot
%token t_token_char
%token t_lws
%token t_colon
%token t_separators
%token t_sp
%token t_ws
%token t_ctl

/* Type of value returned for these tokens */
%type<str> t_crlf
%type<i> t_backslash
%type<i> t_slash
%type<i> t_digit
%type<i> t_dot
%type<i> t_token_char
%type<str> t_lws
%type<i> t_colon
%type<i> t_separators
%type<i> t_sp
%type<str> t_ws
%type<i> t_ctl

/*
 * Followed by this, you should have types defined for all the intermediate
 * rules that you will define. These are some of the intermediate rules:
 */
%type<i> allowed_char_for_token
%type<i> allowed_char_for_text
%type<str> ows
%type<str> token
%type<str> text

%%

/*
** The following 2 rules define a token.
*/

/*
 * Rule 1: Allowed characters in a token
 *
 * An excerpt from RFC 2616:
 * --
 * token          = 1*<any CHAR except CTLs or separators>
 * --
 */
allowed_char_for_token:
t_token_char; |
t_digit {
	$$ = '0' + $1;
}; |
t_dot;

/*
 * Rule 2: A token is a sequence of all allowed token chars.
 */
token:
allowed_char_for_token {
	YPRINTF("token: Matched rule 1.\n");
	snprintf($$, 8192, "%c", $1);
}; |
token allowed_char_for_token {
	YPRINTF("token: Matched rule 2.\n");
	memcpy($$, $1, strlen($1));
	$$[strlen($1)] = $2;
	$$[strlen($1) + 1] = 0;
    // snprintf($$, 8192, "%s%c", $1, $2);
};

/*
** The following 2 rules define text.
*/
/*
 *
 * Rule 3: Allowed characters in text
 *
 * An excerpt from RFC 2616, section 2.2:
 * --
 * The TEXT rule is only used for descriptive field contents and values
 * that are not intended to be interpreted by the message parser. Words
 * of *TEXT MAY contain characters from character sets other than ISO-
 * 8859-1 [22] only when encoded according to the rules of RFC 2047
 * [14].
 *
 * TEXT = <any OCTET except CTLs, but including LWS>
 * --
 *
 */

allowed_char_for_text:
allowed_char_for_token; |
t_separators {
	$$ = $1;
}; |
t_colon {
	$$ = $1;
}; |
t_slash {
	$$ = $1;
};

/*
 * Rule 4: Text is a sequence of characters allowed in text as per RFC. May
 * 	   also contains spaces.
 */
text: allowed_char_for_text {
	YPRINTF("text: Matched rule 1.\n");
	snprintf($$, 8192, "%c", $1);
}; |
text ows allowed_char_for_text {
	YPRINTF("text: Matched rule 2.\n");
	memcpy($$, $1, strlen($1));
	memcpy($$ + strlen($1), $2, strlen($2));
	$$[strlen($1) + strlen($2)] = $3;
	$$[strlen($1) + strlen($2) + 1] = 0;
	// snprintf($$, 8192, "%s%s%c", $1, $2, $3);
};

/*
 * Rule 5: Optional white spaces
 */
ows: {
	YPRINTF("OWS: Matched rule 1\n");
	$$[0]=0;
}; |
t_sp {
	YPRINTF("OWS: Matched rule 2\n");
	snprintf($$, 8192, "%c", $1);
}; |
t_ws {
	YPRINTF("OWS: Matched rule 3\n");
	snprintf($$, 8192, "%s", $1);
};
t_sps: t_sp t_sps{};
| t_sp;
/*
	error_request_line
	1. extra t_crlf after 1st t_sp
	2. extra t_colon
	3. text instead of token
	4. missing token
	5. missing first text
	6. missing second text (including t_sp next to it)
	7. extra t_sps ahead

	error_request_header
	1. missing key
	2. missing t_colon
	3. missing value
*/

request_line: token t_sp text t_sp text t_crlf {
	YPRINTF("request_Line:\n%s\n%s\n%s\n",$1, $3,$5);
	// printf("Got request_line!\n");
	int request_num = parsing_requests->request_count - 1;
    strcpy(parsing_requests->requests_[request_num].http_method, $1);
	strcpy(parsing_requests->requests_[request_num].http_uri, $3);
	strcpy(parsing_requests->requests_[request_num].http_version, $5);
}
|token t_sp t_crlf text t_sp text t_crlf {parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
|token t_colon t_sp text t_sp text t_crlf {parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
|text t_sp text t_sp text t_crlf {parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
|text t_sp text t_crlf {parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
|token t_sp text t_crlf{parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
|token t_sp text t_crlf {parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
|t_sps token t_sp text t_sp t_crlf{parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};

request_header: token ows t_colon ows text ows t_crlf {
	YPRINTF("request_Header:\n%s\n%s\n",$1,$5);
	// printf("Got request_header!\n");
	int request_num = parsing_requests->request_count - 1;
    strcpy(parsing_requests->requests_[request_num].headers[parsing_requests->requests_[request_num].header_count].header_name, $1);
	strcpy(parsing_requests->requests_[request_num].headers[parsing_requests->requests_[request_num].header_count].header_value, $5);
	parsing_requests->requests_[request_num].header_count++;
	parsing_requests->requests_[request_num].headers = (Request_header *) realloc(parsing_requests->requests_[request_num].headers, sizeof(Request_header)*(parsing_requests->requests_[request_num].header_count + 1));
}
|ows t_colon ows text ows t_crlf {parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
|token ows text ows t_crlf {parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
|token ows t_colon ows t_crlf {parsing_requests->requests_[parsing_requests->request_count - 1].format_correctness = 0;};
;

request_headers: request_headers request_header{
	// printf("Got request_headers!\n");
};
|{
	// printf("Got request_headers!\n");
	};




request: request_line request_headers t_crlf{
	YPRINTF("parsing_request: Matched Success.\n");
	parsing_requests->request_count ++;
	parsing_requests->requests_ = (Request *) realloc(parsing_requests->requests_, sizeof(Request) * (parsing_requests->request_count));
	int request_num = parsing_requests->request_count - 1;
	parsing_requests->requests_[request_num].format_correctness = 1; 
	parsing_requests->requests_[request_num].header_count = 0;        //TODO You will need to handle resizing this in parser.y
	parsing_requests->requests_[request_num].headers = (Request_header *) malloc(sizeof(Request_header) * (parsing_requests->requests_[request_num].header_count + 1));
};

requests : requests request{
	YPRINTF("parsing_requests: Matched Success.\n");
	// printf("1. Requests++!\n");
};
|request{
	YPRINTF("parsing_requests: Matched Success.\n");
	// printf("2. Requests++!\n");
};

requests_ : requests t_crlf{
	return SUCCESS;
};


%%

/* C code */

void set_parsing_options(char *buf, size_t siz, Requests *requests)
{
    parsing_buf = buf;
	parsing_offset = 0;
	parsing_buf_siz = siz;
    parsing_requests = requests;
}

void yyerror (const char *s) {fprintf (stderr, "%s\n", s);}
