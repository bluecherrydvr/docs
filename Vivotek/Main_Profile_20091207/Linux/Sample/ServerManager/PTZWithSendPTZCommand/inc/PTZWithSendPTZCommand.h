#ifndef _PTZ_SEND_COMMAND_H_
#define _PTZ_SEND_COMMAND_H_

#ifdef _WIN32
#include "tchar.h"
#endif

#ifdef _LINUX
#include <stdlib.h>
#include <string.h>
#define MAX_PATH                256
#endif


#define CONST_MAX_DEVIDE	20

// for device property
#define MAX_HOSTNAME_LEN	1024
#define MAX_HTTPPORT_LEN	1024
#define MAX_USERNAME_LEN	1024
#define MAX_PASSWORD_LEN	1024
#define MAX_TIMEOUT_LEN		1024

#endif // _PTZ_SEND_COMMAND_H_

