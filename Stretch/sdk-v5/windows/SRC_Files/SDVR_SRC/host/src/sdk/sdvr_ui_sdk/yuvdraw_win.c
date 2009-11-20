#include "sdvr_ui_sdk.h"
#include <dvr_common.h>
#include <sct.h>
#include "sdk.h"
#include "sdvr_ui_sdk_version.h"
#include <ddraw.h>
#include "yuvdraw.h"

/* 
    Maximum width and height of a video frame allowed.
    The user sets this limit when calling sdvr_ui_init()
*/
sx_uint32 m_max_width = 720;
sx_uint32 m_max_height = 576;

/*
    The width and height of the current video frame being displayed.
*/
sx_uint32 m_width = 0;
sx_uint32 m_height = 0;

/*
    The default color key is white.
*/
sdvr_ui_color_key_t    m_keyColor = sdvr_rgb(255,255,255);


/*
    A flag to indicate if the video card supports color key.
    NOTE: Currently some nvida cards don't support color keys.
*/
int  m_color_key_supported = 1;

static DDCAPS   m_caps;
static LPDIRECTDRAW m_direct_draw = 0;
static LPDIRECTDRAWSURFACE m_primary_surface;
static LPDIRECTDRAWSURFACE m_overlay_surface;
static LPDIRECTDRAWCLIPPER m_clipper;

/************************************************************************/

/* create a offscreen surface */
sdvr_ui_surface create_sdvr_ui_surface( int width, int height )
{
    DDSURFACEDESC ddsd;
    LPDIRECTDRAWSURFACE os;

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwHeight = height;
    ddsd.dwWidth = width;
    if (IDirectDraw_CreateSurface(m_direct_draw, &ddsd, &os, 0) != DD_OK)
        return NULL;

    return os;
}

void release_sdvr_ui_surface( sdvr_ui_surface os)
{
    if (os)
        IDirectDrawSurface_Release( os );
}

sdvr_err_ui_e init_draw(sx_uint32 max_width, sx_uint32 max_lines)
{
    HRESULT hr;
    DDSURFACEDESC ddsd;
    DDPIXELFORMAT dd_format;

    if ( m_direct_draw != 0)
        return SDVR_ERR_UI_INIT;

    hr = DirectDrawCreate(0, &m_direct_draw, 0);
    if ( hr != DD_OK )
    {
        m_direct_draw = NULL;
        return SDVR_ERR_UI_INIT;
    }
    hr = IDirectDraw_GetCaps(m_direct_draw, &m_caps, 0);
    hr = IDirectDraw_SetCooperativeLevel(m_direct_draw, 0, DDSCL_NORMAL);
    // Create the primary surface
    
    ZeroMemory(&ddsd,sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS|DDSD_CKDESTBLT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    ddsd.ddckCKDestBlt.dwColorSpaceLowValue = m_keyColor;
    ddsd.ddckCKDestBlt.dwColorSpaceHighValue = m_keyColor;
    hr = IDirectDraw_CreateSurface(m_direct_draw, &ddsd, &m_primary_surface, 0);
    if (hr != DD_OK )
    {
        m_color_key_supported = 0;
        ddsd.dwFlags = DDSD_CAPS;
        hr = IDirectDraw_CreateSurface(m_direct_draw, &ddsd, &m_primary_surface, 0);
        if ( hr != DD_OK )
        {
            close_draw();
            return SDVR_ERR_UI_INIT;
        }
    }

    ZeroMemory(&dd_format, sizeof(dd_format));
    dd_format.dwSize = sizeof(dd_format);
    dd_format.dwFlags = DDPF_FOURCC ;
    dd_format.dwFourCC = '21VY';

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 1;
    ddsd.dwWidth = m_max_width = max_width;
    ddsd.dwHeight = m_max_height = max_lines;
    ddsd.ddpfPixelFormat = dd_format ;
    hr = IDirectDraw_CreateSurface(m_direct_draw, &ddsd, &m_overlay_surface, 0);
    if (hr != DD_OK )
    {
        close_draw();
        return SDVR_ERR_UI_INIT;
    }

    //Create Clipper
    hr = IDirectDraw_CreateClipper(m_direct_draw, 0, &m_clipper, NULL);
    if( hr != DD_OK )
    {
        close_draw();
        return SDVR_ERR_UI_INIT;
    }
        
    return SDVR_ERR_NONE;
}

/* Free the global directdraw pointers*/
    
sdvr_err_ui_e close_draw()  
{
    release_sdvr_ui_surface(m_overlay_surface);
    m_overlay_surface = NULL;

    if (m_primary_surface)
        IDirectDrawSurface_Release(m_primary_surface);
    m_primary_surface = NULL;

    if (m_direct_draw)
        IDirectDraw_Release(m_direct_draw);
    m_direct_draw = NULL;
    return SDVR_ERR_NONE;
}


int ddraw_bitblt( LPDIRECTDRAWSURFACE dst_surface, RECT* dst_rect, LPDIRECTDRAWSURFACE src_surface, RECT* src_rect, int use_color_key)
{    
    DDBLTFX ddbfx;
    DWORD flag = DDBLT_WAIT;
    HRESULT hr;

    if (src_surface == 0 || dst_surface == 0)
        return 0;

    ZeroMemory(&ddbfx, sizeof(DDBLTFX));
    ddbfx.dwSize = sizeof(DDBLTFX);
    ddbfx.ddckDestColorkey.dwColorSpaceLowValue = m_keyColor;
    ddbfx.ddckDestColorkey.dwColorSpaceHighValue = m_keyColor;

    if (m_color_key_supported && use_color_key )
    {
        flag |= DDBLT_KEYDESTOVERRIDE;
    }

    hr = IDirectDrawSurface_Blt(dst_surface, dst_rect,src_surface, src_rect, flag, &ddbfx);
    return hr == DD_OK;
}

/*set the buffer data*/
sdvr_err_ui_e draw_set_yuv_buffer(sdvr_ui_surface surface, char * y, char * u, char *v, unsigned int width, unsigned int height)
{
	HRESULT hr;	
	DDSURFACEDESC ddsd;
	if ( width > m_max_width || height > m_max_height)
    {
        return SDVR_ERR_UI_VIDEO_SIZE;
    }

	m_width = width;
	m_height = height;
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	hr = IDirectDrawSurface_Lock( m_overlay_surface,NULL,&ddsd,(DDLOCK_NOSYSLOCK|DDLOCK_WAIT), NULL);
	if (hr == DD_OK)
	{
		char * p,*q; 
		sx_uint32 i;

		/****************************
		NOTE: DirectX requires us to set the buffer
		ass Y V U
		****************************/

		/**********************
		Setup the Y Buffer
		***********************/
		int pitch = ddsd.lPitch;
		p = (char *)(ddsd.lpSurface);
		q = y;
		for (i = 0; i< m_height;i++)
		{
			memcpy(p,q,m_width);
			p+=pitch;
			q+=m_width;
		}

		/**********************
		Setup the v Buffer
		***********************/

		p = (char *)(ddsd.lpSurface) + pitch * m_max_height;
		q = v;
		for ( i = 0; i< m_height/2; i++)
		{
			memcpy(p,q,m_width/2);
			p+=pitch/2;
			q+=m_width/2;
		}

		/**********************
		Setup the u Buffer
		***********************/

		p = (char *)(ddsd.lpSurface) + pitch * m_max_height/4*5;
		q = u;
		for ( i = 0; i< m_height/2; i++)
		{
			memcpy(p,q,m_width/2);
			p+=pitch/2;
			q+=m_width/2;
		}

		IDirectDrawSurface_Unlock( m_overlay_surface,NULL );
		if (surface)
		{
			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = width;
			rect.bottom = height;
			ddraw_bitblt(surface,&rect,m_overlay_surface,&rect,0);
		}
		return SDVR_UI_ERR_NONE;
	}
	return SDVR_ERR_UI_SET_BUFFER;
}

sdvr_err_ui_e draw_yuv(sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region,
					   sdvr_ui_surface src_surface, sdvr_ui_region_t *src_region,
					   sdvr_ui_preview_cb preview_cb)
{
	RECT dst_rect;
    RECT src_rect;
    POINT   pt;
    DWORD flag = DDBLT_WAIT;
    LPDIRECTDRAWSURFACE os = src_surface;
	int ok;

	if (src_surface == NULL)
		os = m_overlay_surface; 	

    if (os == NULL)
        return SDVR_ERR_UI_NO_INIT;
    if (m_clipper == NULL)
        return SDVR_ERR_UI_NO_INIT;

    if (!region || !IsWindow(hwnd))
        return SDVR_ERR_UI_INVALID_PARAM;

    if ( (src_region == NULL) && (m_width == 0 || m_height == 0))
        return SDVR_ERR_UI_NO_BUFFER;
        
    pt.x = region->top_left_x;
    pt.y = region->top_left_y;
    ClientToScreen(hwnd,&pt);
    dst_rect.left = pt.x;
    dst_rect.top = pt.y;
    dst_rect.right = dst_rect.left + region->width;
    dst_rect.bottom = dst_rect.top + region->height;

	if( src_region == NULL)
	{
		src_rect.left = 0;
		src_rect.top = 0;
		src_rect.right = m_width;
		src_rect.bottom = m_height;
	}
	else
	{
		src_rect.left = src_region->top_left_x;
		src_rect.top = src_region->top_left_y;
		src_rect.right = src_region->width+src_rect.left;
		src_rect.bottom = src_region->height+src_rect.top;
	}

	if (preview_cb != NULL)
	{
		HDC hdc;
		if (IDirectDrawSurface_GetDC(os, &hdc) == DD_OK)
		{
			preview_cb(hdc);
			IDirectDrawSurface_ReleaseDC(os, hdc);
		}
	}

	IDirectDrawClipper_SetHWnd(m_clipper, 0, hwnd);
	IDirectDrawSurface_SetClipper(m_primary_surface,m_clipper);
	ok = ddraw_bitblt(m_primary_surface,&dst_rect,os,&src_rect,0);
	IDirectDrawSurface_SetClipper(m_primary_surface,NULL);
    return (ok ? SDVR_ERR_NONE : SDVR_ERR_UI_DRAW_YUV);
}

sdvr_err_ui_e clear_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region)
{
	RECT dst_rect;
    POINT   pt;
    DDBLTFX ddbfx;
    
    if (!region || !IsWindow(hwnd))
        return SDVR_ERR_UI_INVALID_PARAM;

    IDirectDrawClipper_SetHWnd(m_clipper, 0, hwnd);
    pt.x = region->top_left_x;
    pt.y = region->top_left_y;
    ClientToScreen(hwnd,&pt);
    dst_rect.left = pt.x;
    dst_rect.top = pt.y;
    dst_rect.right = dst_rect.left + region->width;
    dst_rect.bottom = dst_rect.top + region->height;
    IDirectDrawSurface_SetClipper(m_primary_surface,m_clipper);
    ZeroMemory(&ddbfx, sizeof(DDBLTFX));
    ddbfx.dwSize = sizeof(DDBLTFX);
    ddbfx.dwFillColor  = m_keyColor;

    IDirectDrawSurface_Blt(m_primary_surface, &dst_rect,NULL,&dst_rect,DDBLT_COLORFILL | DDBLT_WAIT,&ddbfx);
    IDirectDrawSurface_SetClipper(m_primary_surface,NULL);
    return SDVR_ERR_NONE;
}

sdvr_err_ui_e draw_frame(sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region,
                         sdvr_ui_color_key_t rgb_color, int line_style)
{
    HDC hdc;
    HPEN hpen,hpenOld;
    HBRUSH hbrush, hbrushOld;
    LOGBRUSH lb;

    hdc = GetDC(hwnd);
    if (hdc != NULL)
    {
        SetBkColor(hdc, m_keyColor);
        hpen = CreatePen(line_style, 1, rgb_color);
        hpenOld = SelectObject(hdc, hpen);

        lb.lbStyle = BS_HOLLOW;
        hbrush = CreateBrushIndirect(&lb);
        hbrushOld = SelectObject(hdc, hbrush);

        Rectangle(hdc,
            region->top_left_x, region->top_left_y,
            region->top_left_x + region->width + 1,
            region->top_left_y + region->height + 1);

        DeleteObject(SelectObject(hdc, hbrushOld));
        DeleteObject(SelectObject(hdc, hpenOld));
        ReleaseDC(hwnd, hdc);
    }

    return SDVR_ERR_NONE;
}

void draw_set_key_color( sdvr_ui_color_key_t color_key )
{
    m_keyColor = color_key;
}

