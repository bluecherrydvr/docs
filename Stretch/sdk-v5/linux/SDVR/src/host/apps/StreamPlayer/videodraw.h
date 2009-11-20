/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * SDVR, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF SDVR, INC.
 *
 ************************************************************************/

#ifndef __VIDEODRAW_H__
#define __VIDEODRAW_H__

#include <windows.h>
#include <ddraw.h>
#include "sdvr_player.h"

class CVideoDraw
{
public:
    CVideoDraw();
    ~CVideoDraw();
    sdvr_player_err_e InitDirectDraw();
    void ReleaseDirectDraw();
    sdvr_player_err_e CreateSurface(LPDIRECTDRAWSURFACE *pSurface, sx_int32 width, sx_int32 height);
    sdvr_player_err_e LockSurface(sx_uint8 *y, sx_uint8 *u, sx_uint8 *v,
                                  sx_int32 y_stride, sx_int32 u_stride, sx_int32 v_stride,
                                  sx_int32 width, sx_int32 height,
                                  LPDIRECTDRAWSURFACE pSavedSurface);
    sdvr_player_err_e DrawFrame(HWND hWnd, sx_int32 width, sx_int32 height, LPDIRECTDRAWSURFACE pSavedSurface);
   
private:
    LPDIRECTDRAW            m_pDirectDraw;      // directdraw object
    LPDIRECTDRAWSURFACE     m_pPrimarySurface;  // directdraw primary surface
    LPDIRECTDRAWSURFACE     m_pOffscreenSurface;// directdraw offscreen surface
    LPDIRECTDRAWCLIPPER     m_pClipper;         // directdraw clipper
    sx_int32                m_iMaxWidth;         // maximum surface width
    sx_int32                m_iMaxHeight;        // maximum surface height
    sx_bool                 m_bInitDirectDraw;
};


#endif