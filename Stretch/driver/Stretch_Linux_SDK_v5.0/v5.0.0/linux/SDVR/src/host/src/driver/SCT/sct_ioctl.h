/**********************************************************************\
*                                                                      *
* Copyright (c) 2005 Stretch, Inc. All rights reserved.                *
*                                                                      *
* THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF *
* STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT *
* THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.                *
*                                                                      *
\**********************************************************************/

///////////////////////////////////////////////////////////////////////
//	Author :	Sandip Bidaye
//	Date    :   11-Jun-2005
//
///////////////////////////////////////////////////////////////////////


#if !defined(__SCT_IOCTL_H__)
#define __SCT_IOCTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <winioctl.h>
#include <setupapi.h>
#include <initguid.h>

#include <stdio.h>
#include <stdlib.h>

// Application specific header files
#include "..\Inc\guids.h"
#include "..\Inc\ioctls.h"
#include "..\..\include\systypes.h"
#include "..\..\include\sct_msg.h"


/**********************************************************************
    INTERNAL : IOCTL API.
**********************************************************************/
BOOL
Stretch_IoControl( DWORD	        dwIoCode, 
                   HANDLE	        hDev,
                   DWORD	        dwNumBytes, 
                   unsigned char *  Buf,
                   OVERLAPPED *     pOverlapped );

BOOL
Stretch_IoControlEx( DWORD              dwIoCode, 
                     HANDLE	            hDev,
                     DWORD	            dwNumInBytes, 
                     unsigned char *    inBuf,
                     DWORD	            dwNumOutBytes, 
                     unsigned char *    outBuf,
                     OVERLAPPED *       pOverlapped );

BOOL	
Stretch_CloseDevice( HANDLE hDev );


#ifdef __cplusplus
  }
#endif

#endif // __SCT_IOCTL_H__
