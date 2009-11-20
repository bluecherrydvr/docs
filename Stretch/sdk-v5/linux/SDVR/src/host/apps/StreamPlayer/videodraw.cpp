/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * SDVR, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF SDVR, INC.
 *
 ************************************************************************/

#include "videodraw.h"


CVideoDraw::CVideoDraw():
    m_iMaxWidth(1280),
    m_iMaxHeight(720),
    m_pDirectDraw(NULL),
    m_pPrimarySurface(NULL),
    m_pOffscreenSurface(NULL),
    m_pClipper(NULL),
    m_bInitDirectDraw(false)
{
}

CVideoDraw::~CVideoDraw()
{
    ReleaseDirectDraw();
}

void CVideoDraw::ReleaseDirectDraw()
{
    if (m_pPrimarySurface) {
        IDirectDrawSurface_Release(m_pPrimarySurface);
        m_pPrimarySurface = NULL;
    }

    if (m_pOffscreenSurface) {
        IDirectDrawSurface_Release(m_pOffscreenSurface);
        m_pOffscreenSurface = NULL;
    }

    if (m_pClipper) {
        IDirectDrawClipper_Release(m_pClipper);
        m_pClipper = NULL;
    }

    if (m_pDirectDraw){
        IDirectDraw_Release(m_pDirectDraw);
        m_pDirectDraw = NULL;
    }

    m_bInitDirectDraw = false;
}

sdvr_player_err_e CVideoDraw::InitDirectDraw()
{
    HRESULT hr;
    DDSURFACEDESC ddsd;
    DDPIXELFORMAT dd_format;
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

    // Create the offscreen surface
    ZeroMemory(&dd_format, sizeof(dd_format));
    dd_format.dwSize = sizeof(dd_format);
    dd_format.dwFlags = DDPF_FOURCC ;
    dd_format.dwFourCC = '21VY';
   
    
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT ;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
   
    ddsd.dwWidth = m_iMaxWidth;
    ddsd.dwHeight = m_iMaxHeight;
    ddsd.ddpfPixelFormat = dd_format ;
    hr = IDirectDraw_CreateSurface(m_pDirectDraw, &ddsd, &m_pOffscreenSurface, 0);
    if (hr != DD_OK ) {
        ReleaseDirectDraw();
        return SDVR_PLAYER_CREATE_OFFSCREEN_ERROR;
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

sdvr_player_err_e CVideoDraw::CreateSurface(LPDIRECTDRAWSURFACE *pSurface, sx_int32 width, sx_int32 height)
{
    DDSURFACEDESC ddsd;
    DDPIXELFORMAT dd_format;
    HRESULT hr;

    ZeroMemory(&dd_format, sizeof(dd_format));
    dd_format.dwSize = sizeof(dd_format);
    dd_format.dwFlags = DDPF_FOURCC ;
    dd_format.dwFourCC = '21VY';
   
    
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT ;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
   
    ddsd.dwWidth = width;
    ddsd.dwHeight = height;
    ddsd.ddpfPixelFormat = dd_format ;
    hr = IDirectDraw_CreateSurface(m_pDirectDraw, &ddsd, pSurface, 0);
    if (hr != DD_OK ) {
        return SDVR_PLAYER_CREATE_OFFSCREEN_ERROR;
    }
 
    return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CVideoDraw::LockSurface(sx_uint8 *y, sx_uint8 *u, sx_uint8 *v,
                                          sx_int32 y_stride, sx_int32 u_stride, sx_int32 v_stride,
                                          sx_int32 width, sx_int32 height,
                                          LPDIRECTDRAWSURFACE pSavedSurface)
{
    HRESULT hr;	
	DDSURFACEDESC ddsd;
    DDBLTFX ddbfx;
    DWORD flag = DDBLT_WAIT;
    RECT src_rect;
    sx_uint8 *p; 
    sx_int32 i, pitch;
  
    // check if surface is lost
    if (IDirectDrawSurface_IsLost(m_pOffscreenSurface) != DD_OK)
        IDirectDrawSurface_Restore(m_pOffscreenSurface);

	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hr = IDirectDrawSurface_Lock( m_pOffscreenSurface, NULL, &ddsd, (DDLOCK_NOSYSLOCK|DDLOCK_WAIT), NULL);
	if (hr != DD_OK)
        return SDVR_PLAYER_LOCK_SURFACE_ERROR;

	
	/****************************
	NOTE: DirectX requires us to set the buffer
	ass Y V U
	****************************/

	/**********************
	Setup the Y Buffer
	***********************/
	pitch = ddsd.lPitch;
	p = (sx_uint8 *)(ddsd.lpSurface);
	for (i = 0; i < height;i++)
	{
		memcpy(p, y, width);
		p += pitch;
		y += y_stride;
	}

	/**********************
	Setup the v Buffer
	***********************/

	p = (sx_uint8 *)(ddsd.lpSurface) + pitch * m_iMaxHeight;
	for ( i = 0; i < height/2; i++)
	{
		memcpy(p, v, width/2);
		p += pitch/2;
		v += v_stride;
	}

	/**********************
	Setup the u Buffer
	***********************/

	p = (sx_uint8 *)(ddsd.lpSurface) + pitch * m_iMaxHeight/4*5;
	for ( i = 0; i < height/2; i++)
	{
		memcpy(p, u, width/2);
		p += pitch/2;
		u += u_stride;
	}

	IDirectDrawSurface_Unlock( m_pOffscreenSurface, NULL );
 
    src_rect.left = 0;
	src_rect.top = 0;
	src_rect.right = width;
	src_rect.bottom = height;

   
    ZeroMemory(&ddbfx, sizeof(DDBLTFX));
    ddbfx.dwSize = sizeof(DDBLTFX);  

    hr = IDirectDrawSurface_Blt(pSavedSurface, &src_rect, m_pOffscreenSurface, &src_rect, flag, &ddbfx);
    if (hr != DD_OK)
        return SDVR_PLAYER_BLT_ERROR;
  
   
    return SDVR_PLAYER_NOERROR;

}

sdvr_player_err_e CVideoDraw::DrawFrame(HWND hWnd, sx_int32 width, sx_int32 height, LPDIRECTDRAWSURFACE pSavedSurface)
{
    HRESULT hr;	
    DDBLTFX ddbfx;
    DWORD flag = DDBLT_WAIT;
    RECT dst_rect;
    RECT src_rect;

    GetWindowRect(hWnd, &dst_rect);      
    src_rect.left = 0;
	src_rect.top = 0;
	src_rect.right = width;
	src_rect.bottom = height;

    hr = IDirectDrawClipper_SetHWnd(m_pClipper, 0, hWnd);
    if(hr != DD_OK)
        return SDVR_PLAYER_DRAW_FRAME_ERROR;

    hr = IDirectDrawSurface_SetClipper(m_pPrimarySurface, m_pClipper);
    if(hr != DD_OK)
        return SDVR_PLAYER_DRAW_FRAME_ERROR;

    ZeroMemory(&ddbfx, sizeof(DDBLTFX));
    ddbfx.dwSize = sizeof(DDBLTFX);  

    hr = IDirectDrawSurface_Blt(m_pPrimarySurface, &dst_rect, pSavedSurface, &src_rect, flag, &ddbfx);
    if (hr != DD_OK)
        return SDVR_PLAYER_BLT_ERROR;

    IDirectDrawSurface_SetClipper(m_pPrimarySurface, NULL);

    return SDVR_PLAYER_NOERROR;
   
}