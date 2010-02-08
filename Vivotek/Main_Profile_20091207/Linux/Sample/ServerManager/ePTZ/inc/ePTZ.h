#ifndef _EPTZ_H_
#define _EPTZ_H_

#ifdef _WIN32
#include "tchar.h"
#endif

#ifdef _LINUX
#include <stdlib.h>
#include <string.h>
#define MAX_PATH                256
#define MAKELONG(a, b) ((LONG) (((WORD) (a)) | ((DWORD) ((WORD) (b))) << 16)) 
#endif


#define CONST_MAX_DEVIDE	20

// for device property
#define MAX_HOSTNAME_LEN	1024
#define MAX_HTTPPORT_LEN	1024
#define MAX_USERNAME_LEN	1024
#define MAX_PASSWORD_LEN	1024
#define MAX_TIMEOUT_LEN		1024

// for camera move
#define MAX_STREAM_LEN		8
#define MAX_XCOORDINATE_LEN	4096
#define MAX_YCOORDINATE_LEN	4096
#define MAX_WIDTH_LEN		4096
#define MAX_HEIGHT_LEN		4096

#endif // _EPTZ_H_
