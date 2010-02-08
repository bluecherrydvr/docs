#ifndef _GET_SET_NETWORK_INFO_
#define _GET_SET_NETWORK_INFO_

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

// error define
#define E_GETVALUE_FAIL		001
#define E_SETVALUE_FAIL		002
#define E_DEVICE_HANDLE	    003

#endif // _GET_SET_NETWORK_INFO_
