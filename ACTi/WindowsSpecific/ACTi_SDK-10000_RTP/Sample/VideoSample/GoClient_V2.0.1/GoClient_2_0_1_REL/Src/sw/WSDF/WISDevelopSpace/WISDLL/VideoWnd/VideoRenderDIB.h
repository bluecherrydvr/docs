/**************************************************************************************
 * Video Render DIB
 * created by weimin, Aug 15, 2002
 **************************************************************************************/

#ifndef VIDEORENDERER_DIB_H
#define VIDEORENDERER_DIB_H

#include "BaseVideoRender.h"

class CVideoRenderDIB : public CBaseVideoRender
{
private:

	int					m_width;
	int					m_height;
	int					m_videoMode;
	int					m_pitch;

	int					m_fullscreenWidth;
	int					m_fullscreenHeight;

	int					m_aspectRatioX;				
	int					m_aspectRatioY;				
	BOOL				m_bFullScreen;

	HWND				m_hwndPlayback;
	int					m_x, m_y, m_w, m_h;
	BYTE				*m_pDecodeBuf;

public:

	CVideoRenderDIB();
	~CVideoRenderDIB();


	BOOL	Init(HWND hwnd, unsigned int width, unsigned int height, int preferredMode);
	BOOL	InitFullscreen(HWND hwnd, unsigned int width, unsigned int height, int preferredMode);

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
