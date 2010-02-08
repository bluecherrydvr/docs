#ifndef _PRESET_LOCATION_H_
#define _PRESET_LOCATION_H_

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

// for camera move
#define MAX_STREAM_LEN		8
#define MAX_XCOORDINATE_LEN	4096
#define MAX_YCOORDINATE_LEN	4096
#define MAX_WIDTH_LEN		4096
#define MAX_HEIGHT_LEN		4096

// for ptz 
#define MAX_POSNAME_LEN		1024

// error define
#define E_MOVE_CAMERA			 001
#define E_SAVE_PRESET_LOCATION   002
#define E_RECALL_PRESET_LOCATION 003
#define E_PATROL_PRESET_LOCATION 004

#endif // _PRESET_LOCATION_H_
