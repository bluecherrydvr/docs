#include "stdafx.h"
#include "VideoWnd.h"

int playerCreate(CWnd *pParentWnd, int windowStyle)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CVideoWnd *videoWnd = new CVideoWnd();

	videoWnd->CreateWnd(pParentWnd, windowStyle);

	return (int)videoWnd;
}

int playerInit(int hdlPlayer, int VideoWidth, int VideoHeight, int preferredMode = DDRAW_UYVY)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return 0;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	videoWnd->CreateRender(VideoWidth, VideoHeight, preferredMode);

	return 1;
}

void playerDestroy(int hdlPlayer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;
	videoWnd->DestroyWindow();

	delete videoWnd;
}

void playerClose(int hdlPlayer, BOOL bShowWindow)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;
	
	videoWnd->Close();
	videoWnd->ShowWindow(bShowWindow);
}

int playerGetVideoMode(int hdlPlayer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return DDRAW_NULL;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	int mode = videoWnd->GetVideoMode();
	return (mode != DDRAW_NULL ? mode : DDRAW_DIB24);
}

int playerGetPitch(int hdlPlayer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return 0;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	return videoWnd->GetPitch();
}


BYTE *playerBeginDraw(int hdlPlayer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return NULL;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	return videoWnd->BeginDraw();
}

void playerEndDraw(int hdlPlayer, BOOL draw)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	videoWnd->EndDraw(draw);
}

void playerSetFullScreen(int hdlPlayer, BOOL bFullScreen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	videoWnd->SetFullScreen(bFullScreen);
}

void playerZoom(int hdlPlayer, double zoomX, double zoomY)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	videoWnd->Zoom(zoomX, zoomY);
}

HWND playerGetWindow(int hdlPlayer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return NULL;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	return videoWnd->m_hWnd;
}

void playerSetWindowStyle(int hdlPlayer, int windowStyle)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	videoWnd->SetWindowStyle(windowStyle);
}

void playerEnableDefaultMenu(int hdlPlayer, BOOL bEnable)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	videoWnd->m_bShowPopupMenu = bEnable;
}

void playerSetCallback(int hdlPlayer, int callback, int param)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!hdlPlayer) return;
	CVideoWnd *videoWnd = (CVideoWnd *)hdlPlayer;

	videoWnd->SetCallback(callback, param);
}


