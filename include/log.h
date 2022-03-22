#ifndef __LOG__
#define __LOG__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include "msg_response.h"
#include "utils.h"


#define LOG_ROW_SIZE 500

void error_log(char* state_code_str, Request * request);
void access_log(Request * request, int sendRet);


#endif