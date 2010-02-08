// ATCP20.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "ATCP20.h"
#include ".\xadp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CATCP20App::CATCP20App()
{
}

CATCP20App::~CATCP20App()
{
}

CATCP20App theApp;

extern "C" __declspec(dllexport) void XGetVersion( HANDLE h, char* VersionInfo )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	char MyVersion [] = "V 1.0.08\0";
	memcpy( VersionInfo, MyVersion, strlen( MyVersion ) + 1 );
}
extern "C" __declspec(dllexport) HANDLE XInit()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = new XADP();
	return (HANDLE)p;
}

extern "C" __declspec(dllexport) void XExit( HANDLE h )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) delete p;
}
extern "C" __declspec(dllexport) bool XConnect( HANDLE h )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) return p->Connect();
	return false;
}
extern "C" __declspec(dllexport) void XDisconnect( HANDLE h )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) p->Disconnect();
}


extern "C" __declspec(dllexport) void XSetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) p->SetMediaConfig( pMediaCfg );
}


extern "C" __declspec(dllexport) void XSetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) p->SetVideoConfig( pVideoCfg );
}

extern "C" __declspec(dllexport) void XGetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) p->GetVideoConfig( pVideoCfg );
}

extern "C" __declspec(dllexport) bool XStartStreaming( HANDLE h )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) return p->StartStreaming();
	return false;
}

extern "C" __declspec(dllexport) void XStopStreaming( HANDLE h )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) 
		p->StopStreaming();
}

extern "C" __declspec(dllexport) int XGetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) return p->GetNextFrame( nDataType, pFrameBuffer, nBufferSize );	
	return 0;
}
extern "C" __declspec(dllexport) int XGetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) return p->GetNextIFrame( pFrameBuffer, nBufferSize );
	return 0;
}

extern "C" __declspec(dllexport) int XGetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize )
{
	return 0;
}
extern "C" __declspec(dllexport) int XGetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize )
{
	return 0;
}


extern "C" __declspec(dllexport) bool XSendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) return p->SendCommand( pMediaCommand );
	return false;
}


extern "C" __declspec(dllexport) void XSetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) p->SetControlDataCallBack( UserParam, fnCallBack );
}

extern "C" __declspec(dllexport) void XSetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) p->SetMotionInfo( Motioninfo );
}

extern "C" __declspec(dllexport) void XGetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) p->GetMotionInfo( Motioninfo );
}

extern "C" __declspec(dllexport) DWORD XGetBeginTime( HANDLE h )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) return p->GetBeginTime();
	return 0;
}

extern "C" __declspec(dllexport) DWORD XGetEndTime( HANDLE h )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) return p->GetEndTime();
	return 0;
}

extern "C" __declspec(dllexport) void XSetDebugMessageLevel( HANDLE h, int nDebugLevel )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	XADP* p = (XADP*)h;
	if( p ) p->SetDebugMessageLevel( nDebugLevel );
}
