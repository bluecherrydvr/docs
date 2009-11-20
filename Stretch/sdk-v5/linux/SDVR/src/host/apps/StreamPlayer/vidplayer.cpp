/************************************************************************
*
* Copyright 2007 Stretch, Inc. All rights reserved.
*
* THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
* SDVR, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
* THE PRIOR EXPRESS WRITTEN PERMISSION OF SDVR, INC.
*
************************************************************************/

#include <assert.h>
#include "st_queue.h"
#include "stplayer.h"
#include "stsplitter.h"
#include "systime.h"
#include "streamplayer.h"
#include "vidplayer.h"

// abort thread if decoder failed
#define EXIT_IF_DECODER_FAILED 1

// abort thread if directdraw failed
#define EXIT_IF_DDRAW_FAILED 1

// video thread
DWORD WINAPI VideoThread( LPVOID lpParam )
{
	CVideoPlayer &player = *((CVideoPlayer *)lpParam);
	sdvr_player_err_e err;
	sx_bool bAbort = false;
	MSG msg;

	while (!bAbort) {
		// get message
		if (::GetMessage(&msg, NULL, 0, 0) <= 0)
			break;

		switch (msg.message) {
		case WM_QUIT:
			bAbort = true;
			break;
		case WM_PLAYER_UI: // UI message
			player.UIHandler(msg);
			break;
		case WM_TIMER:
			if (msg.wParam == player.m_uiTimerId)
				err = player.UpdateDisplay();
			else
				DispatchMessage(&msg);
			break;
		default:
			DispatchMessage(&msg);
		}
	}
	return 0;
}

CVideoPlayer::CVideoPlayer():
	m_h264(NULL),
	m_width(0),
	m_height(0),
	m_pDirectDraw(NULL),
	m_pPrimarySurface(NULL),
	m_pOffscreenSurface(NULL),
	m_pClipper(NULL),
	m_bInitDirectDraw(false),
	m_pDisplayPic(NULL)
{
	m_eState = UNINITIALIZED;
	m_hThread = NULL;
	m_uiTimerId = 0;
	m_bDropFrame = false;
}

CVideoPlayer::~CVideoPlayer()
{
	Close();    
}

sdvr_player_err_e CVideoPlayer::Open(CStreamPlayer *pPlayer)
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;

	m_pPlayer = pPlayer;

	// initialize DirectDraw
	err = InitDirectDraw();
	if (err != SDVR_PLAYER_NOERROR)
		return err;

	// initialize decoder
	err = InitDecoder();
	if (err != SDVR_PLAYER_NOERROR)
		return err;

	// create thread
	m_hThread = CreateThread(NULL,		// security descriptor
		0,					// stack size
		VideoThread,		// thread function
		this,				// thread parameter
		0,					// start thread immediately
		&m_dwThreadId);		// thread id

	if (m_hThread == NULL)
		return SDVR_PLAYER_THREAD_ERROR;

	m_eState = STOP;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CVideoPlayer::Close()
{
	Stop();

	::PostThreadMessage(m_dwThreadId, WM_QUIT, NULL, NULL);
	WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
	m_hThread = NULL;

	if (m_pDisplayPic != NULL) {
		// put picture back to empty queue
		vdec264_put_picture_empty(m_h264, m_pDisplayPic);
		m_pDisplayPic = NULL;
	}
	CloseDecoder();

	ReleaseDirectDraw();
	m_eState = UNINITIALIZED;
	return SDVR_PLAYER_NOERROR;
}

void CVideoPlayer::ReleaseDirectDraw()
{
	if (m_pOffscreenSurface) {
		IDirectDrawSurface_Release(m_pOffscreenSurface);
		m_pOffscreenSurface = NULL;
	}

	if (m_pPrimarySurface) {
		IDirectDrawSurface_Release(m_pPrimarySurface);
		m_pPrimarySurface = NULL;
	}

	if (m_pClipper) {
		IDirectDrawClipper_Release(m_pClipper);
		m_pClipper = NULL;
	}

	if (m_pDirectDraw){
		IDirectDraw_Release(m_pDirectDraw);
		m_pDirectDraw = NULL;
	}
}

sdvr_player_err_e CVideoPlayer::InitDirectDraw()
{
	HRESULT hr;
	DDSURFACEDESC ddsd;
	DDCAPS   ddcaps;

	if (m_bInitDirectDraw)
		return SDVR_PLAYER_NOERROR;

	// initialize directdraw
	hr = DirectDrawCreate(NULL, &m_pDirectDraw, NULL);
	if ( hr != DD_OK ) {
		m_pDirectDraw = NULL;
		return SDVR_PLAYER_CREATE_DDRAW_ERROR;
	}

	ddcaps.dwSize = sizeof( ddcaps );
	hr = IDirectDraw_GetCaps(m_pDirectDraw, &ddcaps, NULL);
	if ( hr != DD_OK ) {
		ReleaseDirectDraw();
		return SDVR_PLAYER_CREATE_DDRAW_ERROR;
	}

	hr = IDirectDraw_SetCooperativeLevel(m_pDirectDraw, 0, DDSCL_NORMAL);
	if ( hr != DD_OK ) {
		ReleaseDirectDraw();
		return SDVR_PLAYER_CREATE_DDRAW_ERROR;
	}

	// Create the primary surface
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddsd.dwFlags = DDSD_CAPS;

	hr = IDirectDraw_CreateSurface(m_pDirectDraw, &ddsd, &m_pPrimarySurface, 0);
	if (hr != DD_OK ){
		ReleaseDirectDraw();
		return SDVR_PLAYER_CREATE_DDRAW_ERROR;   
	}

	// Create Clipper
	hr = IDirectDraw_CreateClipper(m_pDirectDraw, 0, &m_pClipper, NULL);
	if( hr != DD_OK ){
		ReleaseDirectDraw();
		return SDVR_PLAYER_CREATE_DDRAW_ERROR;
	}

	m_bInitDirectDraw = true;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CVideoPlayer::CreateSurface(sx_int32 width, sx_int32 height)
{
	DDSURFACEDESC ddsd;
	DDPIXELFORMAT dd_format;
	HRESULT hr;

	// create offscreen surface 
	if (m_pOffscreenSurface) {
		IDirectDrawSurface_Release(m_pOffscreenSurface);
		m_pOffscreenSurface = NULL;
	}

	ZeroMemory(&dd_format, sizeof(dd_format));
	dd_format.dwSize = sizeof(dd_format);
	dd_format.dwFlags = DDPF_FOURCC ;
	dd_format.dwFourCC = '21VY';


	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT ;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

	ddsd.dwWidth = m_dwOffscreenWidth = width;
	ddsd.dwHeight = m_dwOffscreenHeight = height;
	ddsd.ddpfPixelFormat = dd_format ;
	hr = IDirectDraw_CreateSurface(m_pDirectDraw, &ddsd, &m_pOffscreenSurface, 0);

	// if failed to create surface in video memory, try system memory next
	if (hr != DD_OK ) {
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT ;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		ddsd.dwWidth = m_dwOffscreenWidth = width;
		ddsd.dwHeight = m_dwOffscreenHeight = height;
		ddsd.ddpfPixelFormat = dd_format ;
		hr = IDirectDraw_CreateSurface(m_pDirectDraw, &ddsd, &m_pOffscreenSurface, 0);
		if (hr != DD_OK )
			return SDVR_PLAYER_CREATE_OFFSCREEN_ERROR;
	}

	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CVideoPlayer::InitDecoder()
{
	vdec264_err_e err = vdec264_open(&m_h264);
	if(err != VDEC264_OK)
		return SDVR_PLAYER_VIDEO_DECODER_INIT_ERROR; 

	return SDVR_PLAYER_NOERROR;
}

void CVideoPlayer::CloseDecoder()  
{
	if (m_h264) {
		vdec264_close(m_h264);
		m_h264 = NULL;
	}
}

// UI message handler
sdvr_player_err_e 
CVideoPlayer::UIHandler(MSG &msg)
{
	switch (msg.wParam) {
	case PLAYER_CMD_PLAY:
		_Play((HWND)msg.lParam);
		break;
	case PLAYER_CMD_STOP:
		_Stop();
		break;
	case PLAYER_CMD_PAUSE:
		_Pause();
		break;
	}
	// response by setting event
	SetEvent(m_UIEventId);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CVideoPlayer::Play(HWND hWnd)
{
	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_PLAY, (LPARAM)hWnd);
	DWORD ret = WaitForSingleObject(m_UIEventId, INFINITE);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CVideoPlayer::Stop()
{
	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_STOP, NULL);
	WaitForSingleObject(m_UIEventId, INFINITE);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CVideoPlayer::Pause()
{
	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_PAUSE, NULL);
	WaitForSingleObject(m_UIEventId, INFINITE);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CVideoPlayer::_Play(HWND hWnd)
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;

	m_hWnd = hWnd;
	switch (m_eState) {
	case STOP:
	case PAUSE:
		SetRefreshTimer(1);
		m_eState = PLAY;
		break;
	default:
		break;
	}

	return err;
}

// stop play
sdvr_player_err_e CVideoPlayer::_Stop()
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;

	switch (m_eState) {
	case PLAY:
		KillTimer(m_hWnd, m_uiTimerId);
		m_eState = STOP;
		break;
	default:
		break;
	}
	return err;
}

// pause play
sdvr_player_err_e CVideoPlayer::_Pause()
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;

	switch (m_eState) {
	case PLAY:
		KillTimer(m_hWnd, m_uiTimerId);
		m_eState = PAUSE;
		break;
	default:
		break;
	}
	return err;
}

// lock the surface and set the buffer data 
sdvr_player_err_e CVideoPlayer::LockSurface(vdec264_picture_t *picture)
{
	HRESULT hr;	
	DDSURFACEDESC ddsd;
	sx_int32 width, height;
	sdvr_player_err_e err;

	width = picture->width;
	height = picture->height;

	if(m_width != width || m_height != height) {
		err = CreateSurface(width, height);
		if (err != SDVR_PLAYER_NOERROR)
			return err;

		m_width = width;
		m_height = height;
	}

	// check if surface is lost
	if (IDirectDrawSurface_IsLost(m_pOffscreenSurface) != DD_OK)
		IDirectDrawSurface_Restore(m_pOffscreenSurface);

	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hr = IDirectDrawSurface_Lock( m_pOffscreenSurface, NULL, &ddsd, (DDLOCK_NOSYSLOCK|DDLOCK_WAIT), NULL);
	if (hr == DD_OK)
	{
		sx_uint8 * p,*q; 
		sx_int32 i, stride;

		/****************************
		NOTE: DirectX requires us to set the buffer
		ass Y V U
		****************************/

		/**********************
		Setup the Y Buffer
		***********************/
		int pitch = ddsd.lPitch;
		p = (sx_uint8 *)(ddsd.lpSurface);
		q = picture->plane[0];
		stride = picture->padded_width[0];
		for (i = 0; i < height;i++)
		{
			memcpy(p, q, width);
			p += pitch;
			q += stride;
		}

		/**********************
		Setup the v Buffer
		***********************/

		p = (sx_uint8 *)(ddsd.lpSurface) + pitch * m_dwOffscreenHeight;
		q = picture->plane[2];
		stride = picture->padded_width[2];
		for ( i = 0; i < height/2; i++)
		{
			memcpy(p, q, width/2);
			p += pitch/2;
			q += stride;
		}

		/**********************
		Setup the u Buffer
		***********************/

		p = (sx_uint8 *)(ddsd.lpSurface) + pitch * m_dwOffscreenHeight/4*5;
		q = picture->plane[1];
		stride = picture->padded_width[1];
		for ( i = 0; i < height/2; i++)
		{
			memcpy(p, q, width/2);
			p += pitch/2;
			q += stride;
		}

		IDirectDrawSurface_Unlock( m_pOffscreenSurface, NULL );

		return SDVR_PLAYER_NOERROR;
	}

	return SDVR_PLAYER_LOCK_SURFACE_ERROR;
}


sdvr_player_err_e CVideoPlayer::DrawFrame()
{
	HRESULT hr;	
	DDBLTFX ddbfx;
	DWORD flag = DDBLT_WAIT;
	RECT dst_rect;
	RECT src_rect;

	GetWindowRect(m_hWnd, &dst_rect); 

	dst_rect.bottom = dst_rect.top + m_height;
	dst_rect.right  = dst_rect.left + m_width;

	src_rect.left = 0;
	src_rect.top = 0;
	src_rect.right = m_width;
	src_rect.bottom = m_height;

	hr = IDirectDrawClipper_SetHWnd(m_pClipper, 0, m_hWnd);
	if(hr != DD_OK)
		return SDVR_PLAYER_DRAW_FRAME_ERROR;

	hr = IDirectDrawSurface_SetClipper(m_pPrimarySurface, m_pClipper);
	if(hr != DD_OK)
		return SDVR_PLAYER_DRAW_FRAME_ERROR;

	ZeroMemory(&ddbfx, sizeof(DDBLTFX));
	ddbfx.dwSize = sizeof(DDBLTFX);  

	hr = IDirectDrawSurface_Blt(m_pPrimarySurface, &dst_rect, m_pOffscreenSurface, &src_rect, flag, &ddbfx);
	if(hr != DD_OK)
		return SDVR_PLAYER_BLT_ERROR;

	IDirectDrawSurface_SetClipper(m_pPrimarySurface, NULL);

	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CVideoPlayer::UpdateDisplay()
{
	vdec264_err_e decoder_err;
	sdvr_player_err_e player_err;
	sx_int32 size, consumed_bytes;
	vdec264_picture_t *picture;
	sx_uint8 *payload;

	if (m_eState != PLAY) {
		return SDVR_PLAYER_NOERROR;
	}

	// display decoded picture if available
	if (m_pDisplayPic != NULL) {
		// calibrate system time if is master
		if (IsMaster())
			m_pPlayer->CalibrateSystemTime((double)m_i64LastTS / 100000.0);

		if (!m_bDropFrame) {
			player_err = LockSurface(m_pDisplayPic);
			if (player_err == SDVR_PLAYER_NOERROR) {
				player_err = DrawFrame();
				if (player_err != SDVR_PLAYER_NOERROR) {
					// handle error
				}
			}
		}
		// put picture back to empty queue
		vdec264_put_picture_empty(m_h264, m_pDisplayPic);
		m_pDisplayPic = NULL;
	}

	// decode next frame
	// get buffer from video queue
	CFrameBuffer *frame = m_pPlayer->GetFrameQueue().GetTail();
	if (frame == NULL) {
		SetRefreshTimer(5);
		return SDVR_PLAYER_NOERROR;
	}

	// send end of file message if we get a null frame
	if (frame->type == NULL_FRAME) {
		m_pPlayer->SendFileEndMessage();
		SetRefreshTimer(100);
		return SDVR_PLAYER_NOERROR;
	}

	// ASSUME: we have one frame at most for each packet
	// ASSUME: we don't have B frames
	size = frame->size;
	payload = frame->payload;
	while (size > 0) {
		decoder_err = vdec264_decode(m_h264, payload, size, &consumed_bytes);
		if (decoder_err != VDEC264_OK)
			break;
		payload += consumed_bytes;
		size -= consumed_bytes;
	}
	m_pPlayer->GetFrameQueue().CommitTail();

	// get decoded picture
	decoder_err = vdec264_get_picture_full(m_h264, &picture);
	if (decoder_err != VDEC264_OK) {
		// handle error
	}

	if (picture) {
		m_bDropFrame = false;
		m_pDisplayPic = picture;
		// set timer according to time stamp
#if 0
		double dTime = m_pPlayer->GetSystemTimeFromTS((double)frame->timestamp / 100000.0) 
			- m_pPlayer->GetSystemTime();
#else
		double dTime = 0.015;
#endif

		if (dTime > 0.05)
			SetRefreshTimer(50);
		else if (dTime > 0.0001)
			SetRefreshTimer((UINT)(dTime * 1000.0 + 0.5));
		else {
			m_bDropFrame = true;
			SetRefreshTimer(1);
		}
		m_i64LastTS = frame->timestamp;
	}
	else {
		// no decoded picture, wait some time to check again
		m_pDisplayPic = NULL;
		m_uiTimerId = ::SetTimer(NULL, m_uiTimerId, 1, NULL);
	}

	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CVideoPlayer::Refresh()
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;

	if (m_pOffscreenSurface)
		err = DrawFrame();

	return err;
}

void CVideoPlayer::SetRefreshTimer(sx_uint32 uiMilliSec)
{
	m_uiTimerId = ::SetTimer(NULL, m_uiTimerId, uiMilliSec, NULL);
}

sx_bool CVideoPlayer::IsMaster() 
{
	return m_pPlayer->IsMaster();
}
