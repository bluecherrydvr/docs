/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * SDVR, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF SDVR, INC.
 *
 ************************************************************************/

#ifndef __VIDPLAYER_H__
#define __VIDPLAYER_H__

#include <windows.h>
#include <ddraw.h>
#include "sdvr_player.h"
#include "vdec264.h"
#include "common.h"
#include "renderer.h"

class CStretchPlayer;

class CVideoPlayer : public CRenderer
{
public:
	CVideoPlayer();
	~CVideoPlayer();

	sdvr_player_err_e Open(CStreamPlayer *pPlayer);
	sdvr_player_err_e Close();
	sdvr_player_err_e Play(HWND hWnd);
	sdvr_player_err_e Stop();
	sdvr_player_err_e Pause();
	sdvr_player_err_e Refresh();
	sx_bool IsMaster();

protected:
	sdvr_player_err_e UIHandler(MSG &msg);
	sdvr_player_err_e _Play(HWND hWnd);
	sdvr_player_err_e _Stop();
	sdvr_player_err_e _Pause();
	sdvr_player_err_e _Refresh();

    sdvr_player_err_e InitDirectDraw();
    void ReleaseDirectDraw();
    sdvr_player_err_e  InitDecoder();
    void  CloseDecoder();
    sdvr_player_err_e  UpdateDisplay();
    sdvr_player_err_e  LockSurface(vdec264_picture_t *picture);
    sdvr_player_err_e  DrawFrame();
    sdvr_player_err_e  CreateSurface(sx_int32 width, sx_int32 height);
	void SetRefreshTimer(sx_uint32 uiMilliSec);

	// thread function
    friend DWORD WINAPI VideoThread(LPVOID lpParam);

private:
    sx_int32                m_width;            // display width
    sx_int32                m_height;           // display height
    vdec264_t               m_h264;             // video decoder handle
	vdec264_picture_t		*m_pDisplayPic;		// display picture
	UINT_PTR				m_uiTimerId;		// timer id
	sx_int64				m_i64LastTS;		// last display timestamp
	sx_bool					m_bDropFrame;		// drop frame flag

    LPDIRECTDRAW            m_pDirectDraw;      // directdraw object
    LPDIRECTDRAWSURFACE     m_pPrimarySurface;  // directdraw primary surface
    LPDIRECTDRAWSURFACE     m_pOffscreenSurface;// directdraw offscreen surface
    LPDIRECTDRAWCLIPPER     m_pClipper;         // directdraw clipper
    sx_bool                 m_bInitDirectDraw;
    DWORD                   m_dwOffscreenWidth; // offscreen width
    DWORD                   m_dwOffscreenHeight;// offscreen height 
};

#endif