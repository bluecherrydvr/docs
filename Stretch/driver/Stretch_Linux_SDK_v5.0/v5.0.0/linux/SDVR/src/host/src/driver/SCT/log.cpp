/**********************************************************************\
*                                                                      *
* Copyright (c) 2005 Stretch, Inc. All rights reserved.                *
*                                                                      *
* THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF *
* STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT *
* THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.                *
*                                                                      *
\**********************************************************************/

// Logging functions for SCT DLL.

#include <windows.h>

#include <stdio.h>

#ifdef _DEBUG
void
Trace( const char * format, ... )
{
	va_list args ;
    va_start(args, format) ;
	
	int nBuf ;
    char szBuffer[4096] ;
#ifdef vsprintf_s
	nBuf = vsprintf_s(szBuffer, sizeof(szBuffer),format, args) ;
#else
	nBuf = vsprintf(szBuffer, format, args) ;
#endif

	OutputDebugStringA(szBuffer) ;	
    va_end(args) ;
}
#else
void Trace(const char * , ... ){};
#endif