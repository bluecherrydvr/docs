/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_debug.h
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:define debug information.


	Other:
	Function list:
	History:

*******************************************************************
*/

#ifndef _ISIL_DEBUG_
#define _ISIL_DEBUG_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "isil_type.h"


#define BUG_NORMAL 0
#define BUG_WARNING 1
#define BUG_ERROR 	2

S32 iISIL_BugLevel ;

//#define ISIL_NEED_DEBUG
#ifdef ISIL_NEED_DEBUG

#define ISIL_TRACE(level, format, args...)\
	do{ \
		if (level >= iISIL_BugLevel) \
			printf(format, ## args); \
			printf("\n");\
	} while (0)
#else
#define ISIL_TRACE(level, format, args...)
#endif




#ifdef __cplusplus
}
#endif
#endif

