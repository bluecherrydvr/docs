/**************************************************************************************
 * Video Render
 * created by weimin, Aug 6, 2002
 **************************************************************************************/

#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H


#include "BaseVideoRender.h"
#include "VideoRenderOverlay.h"
#include "VideoRenderRGB.h"
#include "VideoRenderDIB.h"

class CVideoRender : public CBaseVideoRender
{
	CVideoRenderOverlay *overlayRender;
	CVideoRenderRGB     *rgbRender;
	CVideoRenderDIB     *dibRender;
	CVideoRender        *render;

public:

	CVideoRender();
	~CVideoRender();

	BOOL	Init(HWND hwnd, unsigned int width, unsigned int height, int preferedMode);
	BOOL	InitFullscreen(HWND hwnd, unsigned int width, unsigned int height, int preferedMode);

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