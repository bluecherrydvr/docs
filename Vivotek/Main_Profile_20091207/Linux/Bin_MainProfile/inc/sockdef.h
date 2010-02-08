/*
 *******************************************************************************
 * $Header: /RD_1/Project/PNX1300_PSOS/Farseer/common/src/sockdef.h 2     05/03/15 8:56a Joe $
 *
 *  Copyright (c) 2000-2003 Vivotek Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VIVOTEK INC.                                                    |
 *  +-----------------------------------------------------------------+
 *
 * $History: sockdef.h $
 * 
 * *****************  Version 2  *****************
 * User: Joe          Date: 05/03/15   Time: 8:56a
 * Updated in $/RD_1/Project/PNX1300_PSOS/Farseer/common/src
 * Support MSVC multicast add_membership function
 * 
 * *****************  Version 11  *****************
 * User: Yun          Date: 04/03/10   Time: 10:57a
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Merge with SOC project
 * 
 * *****************  Version 10  *****************
 * User: Joe          Date: 03/12/17   Time: 1:27p
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Change Network_Release() for WIN32
 * 
 * *****************  Version 9  *****************
 * User: Joe          Date: 03/12/02   Time: 10:15a
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Add DNSresolve() declare.
 * 
 * *****************  Version 8  *****************
 * User: Joe          Date: 03/10/03   Time: 10:39a
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Support new Network interface.
 * 
 * *****************  Version 7  *****************
 * User: Joe          Date: 03/09/22   Time: 4:41p
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Add definitions for LINUX
 * 
 * *****************  Version 6  *****************
 * User: Joe          Date: 03/09/19   Time: 1:07p
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Change Network_Init() to new interface.
 * 
 * *****************  Version 5  *****************
 * User: Joe          Date: 03/08/23   Time: 10:14a
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Add compatible with socket.h
 * 
 * *****************  Version 4  *****************
 * User: Joe          Date: 03/08/19   Time: 1:52p
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * change windows.h to winsock.h
 * 
 * *****************  Version 2  *****************
 * User: Joe          Date: 03/08/11   Time: 10:55a
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * 1. Add shutdownsocket definitions for Win32 system
 * 2. Ad SD_BOTH definitions.
 * 
 * *****************  Version 1  *****************
 * User: Joe          Date: 03/07/22   Time: 10:17a
 * Created in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * socket layer unified definitions.
 *
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2003 Vivotek, Inc. All rights reserved.
 *
 * \file
 * sockdef.h
 *
 * \brief
 * Socket interface definitions for all platform
 * (_WIN32_, _PSOS_TRIMEDIA, _SCKEMU)
 *
 * \date
 * 2003/07/16
 *
 * \author
 * Joe Wu
 *
 *
 *******************************************************************************
 */

#ifndef _SOCK_DEF_H_
#define _SOCK_DEF_H_

#if defined(_WIN32_)
//#include <windows.h>
#include <winsock.h>

#if defined(_MSC_VER)
#undef IP_OPTIONS
#undef IP_HDRINCL
#undef IP_TOS    
#undef IP_TTL    
#undef IP_MULTICAST_IF 
#undef IP_MULTICAST_TTL
#undef IP_MULTICAST_LOOP
#undef IP_ADD_MEMBERSHIP
#undef IP_DROP_MEMBERSHIP
#undef IP_DONTFRAGMENT   
#undef IP_ADD_SOURCE_MEMBERSHIP 
#undef IP_DROP_SOURCE_MEMBERSHIP
#undef IP_BLOCK_SOURCE          
#undef IP_UNBLOCK_SOURCE        
#undef IP_PKTINFO               


#define IP_OPTIONS      			1 /* set/get IP options */
#define IP_HDRINCL      			2 /* header is included with data */
#define IP_TOS          			3 /* IP type of service and preced*/
#define IP_TTL          			4 /* IP time to live */
#define IP_MULTICAST_IF     		9 /* set/get IP multicast i/f  */
#define IP_MULTICAST_TTL       		10 /* set/get IP multicast ttl */
#define IP_MULTICAST_LOOP      		11 /*set/get IP multicast loopback */
#define IP_ADD_MEMBERSHIP      		12 /* add an IP group membership */
#define IP_DROP_MEMBERSHIP     		13/* drop an IP group membership */
#define IP_DONTFRAGMENT     		14 /* don't fragment IP datagrams */
#define IP_ADD_SOURCE_MEMBERSHIP  	15 /* join IP group/source */
#define IP_DROP_SOURCE_MEMBERSHIP 	16 /* leave IP group/source */
#define IP_BLOCK_SOURCE           	17 /* block IP group/source */
#define IP_UNBLOCK_SOURCE         	18 /* unblock IP group/source */
#define IP_PKTINFO                	19 /* receive packet information for ipv4*/

#endif

#define SD_RECEIVE	0X0
#define SD_SEND		0X1
#define SD_BOTH     0x2
#define shutdownsocket(socket, method)  shutdown(socket, method)

// IP Installation Method
#define Network_GetIP_BySystem 	0
#define Network_GetIP_ByPing 	1
#define Network_GetIP_ByDHCP 	2

// Network Parameters
#define Network_Params_BySystem 0
#define Network_Params_ByDHCP 	1

// Added by Jeffrey 2005/03/14 to prevent from joining multicast group failed
#ifdef IP_ADD_MEMBERSHIP
#undef IP_ADD_MEMBERSHIP
#define IP_ADD_MEMBERSHIP 12
#endif

__inline int Network_Init(int IPInstallation, int NetworkParameters)
{
	WSADATA wsaData;
	return WSAStartup( 0x202, &wsaData);
}

__inline Network_GetIP(unsigned long *ulNewIP, unsigned long ulTimeoutInSec)
{
	char szHostName[64];
	struct hostent *phostent;

   	if(gethostname(szHostName, sizeof(szHostName)) == SOCKET_ERROR)
    {
       	return SOCKET_ERROR;
    }
    phostent = (struct hostent *)gethostbyname(szHostName);
    *ulNewIP = *(unsigned long *)(phostent->h_addr_list[0]);

    return S_OK;
}

__inline int Network_SetIP(unsigned long ulNewIP)
{
    return -1;
}

__inline int Network_Release(void)
{
    return WSACleanup();
}
// Added by Joe 2003/11/21, for DNS resolve
#ifndef Iid_SZ
/* internal or IP address format */
#define Iid_SZ 4
typedef union {
    unsigned long   l; // Bill, for alignment
    unsigned char   c[Iid_SZ];
    unsigned short  s[Iid_SZ>>1];
//    unsigned long   l; // Bill, move to front for alignment
}   Iid;
#endif

int				DNSresolve(char *fullname, Iid *iidp);

// Added by Jeffrey 2007/03/07
#ifdef errno
#undef errno
#endif
#define	errno		WSAGetLastError()

#ifndef EWOULDBLOCK
#define EWOULDBLOCK	WSAEWOULDBLOCK
#endif

#ifndef EINPROGRESS
#define EINPROGRESS	WSAEWOULDBLOCK
#endif

#ifndef SEND_FLAGS
#define SEND_FLAGS  0
#endif

#ifndef RECV_FLAGS
#define RECV_FLAGS  0
#endif

#elif defined(_PSOS_TRIMEDIA)

#ifdef _SCKEMU
#include "socketemu.h"
#else
#include "socket.h"
#define SD_RECEIVE	0x0
#define SD_SEND		0x1
#define SD_BOTH     0x2
#define shutdownsocket(socket, method)  shutdown(socket, method)

#endif

// Added by Jeffrey 2007/03/07
#ifndef SEND_FLAGS
#define SEND_FLAGS  0
#endif

#ifndef RECV_FLAGS
#define RECV_FLAGS  0
#endif

#elif defined(_LINUX)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
/* Modified by CCH, 2004/02/09 */
#include <netdb.h>

// IP Installation Method
#define Network_GetIP_BySystem 	0
#define Network_GetIP_ByPing 	1
#define Network_GetIP_ByDHCP 	2

// Network Parameters
#define Network_Params_BySystem 0
#define Network_Params_ByDHCP 	1

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SD_BOTH      SHUT_RDWR

#define shutdownsocket(socket, method)  shutdown(socket, method)
#define WSAGetLastError() errno
#define ioctlsocket(fd, cmd, arg)  ioctl(fd, cmd, arg)
#define closesocket(fd) close(fd)

#define Network_Init(IPInstallation, NetworkParameters)			S_OK
#define Network_GetIP(ulNewIP, ulTimeoutInSec)					S_OK
#define Network_SetIP(ulNewIP)									S_OK
#define Network_Release()       								S_OK

// Added by Jeffrey 2007/03/07
#ifndef SEND_FLAGS
#define SEND_FLAGS  MSG_NOSIGNAL
#endif

#ifndef RECV_FLAGS
#define RECV_FLAGS	MSG_NOSIGNAL
#endif


#else
#error Please define _WIN32_ or _PSOS_TRIMEDIA or _LINUX
#endif

#endif

