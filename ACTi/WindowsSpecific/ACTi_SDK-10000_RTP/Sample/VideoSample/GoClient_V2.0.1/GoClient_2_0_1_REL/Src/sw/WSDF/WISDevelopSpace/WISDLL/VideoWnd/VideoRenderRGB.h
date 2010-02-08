/**************************************************************************************
 * Video Render RGB
 * created by weimin, Aug 5, 2002
 **************************************************************************************/

#ifndef VIDEORENDERER_RGB_H
#define VIDEORENDERER_RGB_H

#include <ddraw.h>
#include "BaseVideoRender.h"

class CVideoRenderRGB : public CBaseVideoRender
{
private:

	LPDIRECTDRAW7        lpdd;
	LPDIRECTDRAWSURFACE7 lpddsPrimary;
	LPDIRECTDRAWCLIPPER  lpddClipper;
	LPDIRECTDRAWSURFACE7 lpddsBack;
	LPDIRECTDRAWSURFACE7 lpddsImage;

	int					m_width;
	int					m_height;
	int					m_bpp;
	int					m_videoMode;

	int					m_fullscreenWidth;
	int					m_fullscreenHeight;
	BOOL				m_fullscreenFlip;
	int					m_fullscreenVideoHeight;

	int					m_aspectRatioX;				
	int					m_aspectRatioY;				
	BOOL				m_bFullScreen;

	HWND				m_hwndPlayback;
	int					m_x, m_y, m_w, m_h;

public:

	CVideoRenderRGB();
	~CVideoRenderRGB();


	BOOL	Init(HWND hwnd, unsigned int width, unsigned int height, int preferredMode);
	BOOL	InitFullscreen(HWND hwnd, unsigned int width, unsigned int height, int preferredMode);

	BOOL	HasRGBSupported();
	int		GetVideoMode();
	BOOL	GetFullScreenMode();
	BOOL	Stop();
	RECT	*GetFullscreenRects(); 

	BOOL	Draw(unsigned char *buffer, RECT *rect, int invertFlag);
	BOOL	DrawFullscreen(unsigned char *buffer, int invertFlag, int desktopMode);

	BYTE	*BeginDDPaint(int &lPitch);
	BOOL	EndDDPaint(BOOL draw = FALSE);
	BOOL	Display(int x, int y, int w, int h);
	BOOL	SetAspectRatio(int aspectX, int aspectY);
	BOOL	GetAspectRatio(int &aspectX, int &aspectY);
	BOOL	ShowUpdate();

	BOOL	Close();
};

#endif