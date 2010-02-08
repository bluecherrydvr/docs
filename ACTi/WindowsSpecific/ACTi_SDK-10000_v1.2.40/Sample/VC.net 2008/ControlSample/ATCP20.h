// ATCP20.h : main header file for the ATCP20 DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include ".\xadp.h"

// CATCP20App
// See ATCP20.cpp for the implementation of this class
//

class CATCP20App : public CWinApp
{
public:
	CATCP20App();
	~CATCP20App();
};

extern "C" __declspec(dllexport) void XGetVersion( HANDLE h, char* VersionInfo );
extern "C" __declspec(dllexport) HANDLE XInit();
extern "C" __declspec(dllexport) void XExit( HANDLE h );
extern "C" __declspec(dllexport) bool XConnect( HANDLE h );
extern "C" __declspec(dllexport) void XDisconnect( HANDLE h );
extern "C" __declspec(dllexport) void XSetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
extern "C" __declspec(dllexport) void XSetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
extern "C" __declspec(dllexport) void XGetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
extern "C" __declspec(dllexport) bool XStartStreaming( HANDLE h );
extern "C" __declspec(dllexport) void XStopStreaming( HANDLE h );
extern "C" __declspec(dllexport) int XGetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
extern "C" __declspec(dllexport) int XGetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
extern "C" __declspec(dllexport) int XGetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
extern "C" __declspec(dllexport) int XGetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
extern "C" __declspec(dllexport) bool XSendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
extern "C" __declspec(dllexport) void XSetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
extern "C" __declspec(dllexport) void XSetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
extern "C" __declspec(dllexport) void XGetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
extern "C" __declspec(dllexport) DWORD XGetBeginTime( HANDLE h );
extern "C" __declspec(dllexport) DWORD XGetEndTime( HANDLE h );
extern "C" __declspec(dllexport) void XSetDebugMessageLevel( HANDLE h, int nDebugLevel );