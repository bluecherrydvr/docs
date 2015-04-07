/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_common.h
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:define common function.


	Other:
	Function list:
	History:

*******************************************************************
*/


#ifndef	__ISIL_COMMON_H_
#define __ISIL_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "isil_type.h"

#define ISIL_ENABLE				1
#define ISIL_UNABLE				0

#define ISIL_CFG_PATH_LEN 		256
#define ISIL_NAME_LEN 			32
#define ISIL_SERIALNO_LEN		48
#define ISIL_PATHNAME_LEN		128
#define ISIL_IPADDR_LEN			64
#define ISIL_DDNS_IP_LEN 		32
#define ISIL_MACADDR_LEN			8

#define ISIL_USER_COUNT			4
#define ISIL_PASSWORD_LEN		16
#define ISIL_SUBTITLE_LEN		44
#define ISIL_DAYS_MAX			7
#define ISIL_TIMESEGMENT_MAX		4

#define ISIL_PTZ_PROTOCOL_MAX 	36
#define ISIL_PTZ_PRSET_MAX		128
#define CONNECT_TIMEOUT 		(200000)


#define ISIL_NET_DATA_PORT       (20000)

#define ISIL_GET_PARAMETER   0
#define ISIL_SET_PARAMETER   1

#define ISIL_MAX_MSG_LEN 2048

#define ISIL_UDP_MULTICAST_IP "224.0.0.1"
#define ISIL_UDP_MULTICAST_PORT 6789


#define ISILARRAY_LEN(array)     (sizeof(array)/sizeof((array[0])))
/*CRC check*/
extern U32 crc32(U32 crc, const U08 *buf, U32 len);

#ifdef __cplusplus
}
#endif //__cplusplus



#endif


