/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_type.h
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-16
	Description:define some data type for self.


	Other:
	Function list:
	History:

*******************************************************************
*/

#ifndef ISIL_TYPE_H_
#define ISIL_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>

typedef unsigned int   U32;
typedef unsigned short  	U16;
typedef unsigned char   	U08;
typedef int          	    S32;
typedef short int           S16;
typedef char            	S08;
typedef unsigned long long 	U64;
typedef long long 			S64;

#define ISIL_SUCCESS  0
#define ISIL_FAIL	-1
#define ISIL_READ_ERROR  -2
#define ISIL_NO_CONNECT 0x80

#define ISIL_TRUE 1
#define ISIL_FALSE 0

#ifdef __cplusplus
 }
#endif

#endif


