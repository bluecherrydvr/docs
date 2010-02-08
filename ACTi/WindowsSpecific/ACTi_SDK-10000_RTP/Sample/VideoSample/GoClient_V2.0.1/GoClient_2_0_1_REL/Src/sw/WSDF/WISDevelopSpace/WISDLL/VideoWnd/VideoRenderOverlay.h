/**************************************************************************************
 * Video render overlay
 * created by weimin, Aug 5, 2002
 **************************************************************************************/

#ifndef VIDEORENDERER_OVERLAY_H
#define VIDEORENDERER_OVERLAY_H

#include <ddraw.h>

#include "BaseVideoRender.h"
// Overlay Destination ColorKey
#define DD_OVERLAY_COLORREF            RGB(10, 0, 10) // 32, 24, and 16 bpp

class CVideoRenderOverlay : public CBaseVideoRender
{
private:
	DWORD					m_invertFlag;

	LPDIRECTDRAW7			lpdd;
	LPDIRECTDRAW7			lpdd1;
	LPDIRECTDRAWSURFACE7	lpddsPrimary;
	LPDIRECTDRAWSURFACE7	lpddsPrimary1;
	LPDIRECTDRAWCLIPPER		lpddClipper;
	LPDIRECTDRAWSURFACE7	lpddsOverlay;
	LPDIRECTDRAWSURFACE7	lpddsOverlay1;
	LPDIRECTDRAWSURFACE7	lpddsOverlayBack;
	LPDIRECTDRAWSURFACE7	lpddsOverlayBack1;

	int						m_width;
	int						m_height;
	int						m_fullscreenWidth;
	int						m_fullscreenHeight;
	int						m_videoMode;
	DWORD					m_bpp;
	DWORD					m_physicalDepth;

	int						m_aspectRatioX;				
	int						m_aspectRatioY;				

	RECT					m_fullRects[3];
	HWND					m_hwndPlayback;
	BOOL					m_bFullScreen;
	int						m_x, m_y, m_w, m_h;
	BOOL					m_bNeedFlip;

public:

	CVideoRenderOverlay();
	~CVideoRenderOverlay();

	BOOL	AreOverlaysSupported();
	DWORD	DDColorMatch(COLORREF rgb);

	BOOL	Init(HWND hwnd, unsigned int width, unsigned int height, int preferredMode);
	BOOL	InitFullscreen(HWND hwnd, unsigned int width, unsigned int height, int preferredMode);

	int		GetVideoMode();
	BOOL	GetFullScreenMode();
	BOOL	Stop();
	RECT	*GetFullscreenRects(); 

	int		Draw(unsigned char *buffer, RECT *rect, int invertFlag);
	int		DrawFullscreen(unsigned char *buffer, int invertFlag, int desktopMode);

	BYTE	*BeginDDPaint(int &lPitch);
	BOOL	EndDDPaint(BOOL draw = FALSE);
	BOOL	Display(int x, int y, int w, int h);
	BOOL	SetAspectRatio(int aspectX, int aspectY);
	BOOL	GetAspectRatio(int &aspectX, int &aspectY);
	BOOL	ShowUpdate();

	BOOL	Close();
};

#endif

