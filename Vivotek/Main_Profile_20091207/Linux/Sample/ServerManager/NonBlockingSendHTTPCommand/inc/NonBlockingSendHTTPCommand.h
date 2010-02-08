#ifndef _NOBLOCKING_SEND_HTTP_COMMAND_H_
#define _NOBLOCKING_SEND_HTTP_COMMAND_H_

#ifdef _WIN32
#include "tchar.h"
#endif

#ifdef _LINUX
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#define MAX_PATH                256
#endif

#define CONST_MAX_DEVIDE	20

// for device property
#define MAX_HOSTNAME_LEN	1024
#define MAX_HTTPPORT_LEN	1024
#define MAX_USERNAME_LEN	1024
#define MAX_PASSWORD_LEN	1024
#define MAX_TIMEOUT_LEN		1024

typedef struct _TCBContent 
{
	#ifdef _WIN32
        HANDLE hEvent;
        #else
        sem_t  hEvent;
        #endif
	
	char szNotice[MAX_PATH];
}TCBContent, *PTCBContent;

#endif // _NOBLOCKING_SEND_HTTP_COMMAND_H_
