/**************************************************************************************
 * Video Render
 * created by weimin, Aug 6, 2002
 **************************************************************************************/

#ifndef BASE_VIDEORENDERER_H
#define BASE_VIDEORENDERER_H

#include	<../WSDF.H>
#include	<../MultiMedia.H>


class CBaseVideoRender
{
	virtual BOOL	Init(HWND hwnd, unsigned int width, unsigned int height, int preferredMode) {return TRUE;};
	virtual	BOOL	InitFullscreen(HWND hwnd, unsigned int width, unsigned int height, int preferredMode) {return TRUE;};

	virtual int		GetVideoMode() = 0;
	virtual BOOL	GetFullScreenMode() = 0;
	virtual BOOL	Stop() = 0;
	virtual RECT	*GetFullscreenRects() = 0; 

	virtual BOOL	Draw(unsigned char *buffer, RECT *rect, int invertFlag) = 0;
	virtual BOOL	DrawFullscreen(unsigned char *buffer, int invertFlag, int desktopMode) = 0;

	virtual BYTE	*BeginDDPaint(int &lPitch) = 0;
	virtual BOOL	EndDDPaint(BOOL draw = FALSE) = 0;
	virtual BOOL	Display(int x, int y, int w, int h) = 0;
	virtual BOOL	SetAspectRatio(int aspectX, int aspectY) = 0;
	virtual BOOL	GetAspectRatio(int &aspectX, int &aspectY) = 0;
	virtual BOOL	ShowUpdate() = 0;

	virtual BOOL	Close() = 0;

};

#endif
