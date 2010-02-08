/**************************************************************************************
 * Video Render RGB
 * created by weimin, Aug 15, 2002
 **************************************************************************************/

#include "stdafx.h"
#include "VideoRenderDIB.h"
#include "DebugPrint.h"

CVideoRenderDIB::CVideoRenderDIB()
{
	this->m_videoMode = DDRAW_NULL;
	m_aspectRatioX = 0;
	m_aspectRatioY = 0;
	m_pDecodeBuf = NULL;
}

CVideoRenderDIB::~CVideoRenderDIB()
{
	Close();
}

BOOL CVideoRenderDIB::Init(HWND hwnd, unsigned int width, unsigned int height, int preferredMode)
{
	if (!hwnd || width <= 0 || height <= 0)
		return FALSE;

	if (m_pDecodeBuf && preferredMode == m_videoMode)
	{
		m_bFullScreen = FALSE;
		return TRUE;
	}

	m_hwndPlayback = hwnd;
	switch(preferredMode)
	{
	case DDRAW_DIB24:
		m_pitch = width * 3;
		break;
	case DDRAW_DIB32:
		m_pitch = width * 4;
		break;
	default:
		return FALSE;
	}

	m_pDecodeBuf = new BYTE[m_pitch * height];
	m_width = width;
	m_height = height;
	m_fullscreenWidth = GetSystemMetrics(SM_CXSCREEN);
	m_fullscreenHeight = GetSystemMetrics(SM_CYSCREEN);
	m_videoMode = preferredMode;
	m_bFullScreen = FALSE;
	m_x = 0; m_y = 0;
	m_w = m_width; m_h = m_height;
	this->m_aspectRatioX = width;
	this->m_aspectRatioY = height;

	return TRUE;
}

// Init the renderer for fullscreen drawing
BOOL CVideoRenderDIB::InitFullscreen(HWND hwnd, unsigned int width, unsigned int height, int preferredMode)
{
	if (Init(hwnd, width, height, preferredMode) == FALSE)
		return FALSE;
	m_bFullScreen = TRUE;    
	return TRUE;
}

int CVideoRenderDIB::GetVideoMode()
{
	return this->m_videoMode;
}

BOOL CVideoRenderDIB::GetFullScreenMode()
{
	return this->m_bFullScreen;
}

BOOL CVideoRenderDIB::Stop()
{
	return TRUE;
}

RECT *CVideoRenderDIB::GetFullscreenRects()
{
	return NULL;
}

BOOL CVideoRenderDIB::Draw(unsigned char *buffer, RECT *rect, int invertFlag)
{
	return TRUE;
}

BOOL CVideoRenderDIB::DrawFullscreen(unsigned char *buffer, int invertFlag, int desktop)
{
	return TRUE;
}

BOOL CVideoRenderDIB::Close()
{
	if (m_pDecodeBuf)
	{
		delete m_pDecodeBuf;
		m_pDecodeBuf = NULL;
	}

	m_videoMode = DDRAW_NULL;
	return TRUE;
}

BYTE *CVideoRenderDIB::BeginDDPaint(int &lPitch)
{
	lPitch = m_pitch;

	return m_pDecodeBuf;
}

BOOL CVideoRenderDIB::EndDDPaint(BOOL draw)
{
	if (draw && m_hwndPlayback)
	{
		RECT r;
		GetClientRect(m_hwndPlayback, &r);
		Display(r.left, r.top, r.right- r.left, r.bottom - r.top);
	}
	return TRUE;
}

BOOL CVideoRenderDIB::Display(int x, int y, int w, int h)
{
	if (w == 0 && h == 0) return TRUE;

	RECT dst;
	int width, height;

	if (m_bFullScreen)
	{
		if(m_aspectRatioY < m_aspectRatioX) 
		{
			width  = this->m_width;
			height = this->m_width * m_aspectRatioY / m_aspectRatioX;
		}
		else if (m_aspectRatioX != 0)
		{
			height = this->m_height;
			width  = this->m_height * m_aspectRatioX / m_aspectRatioY;
		}
		else
		{
			height = this->m_height;
			width  = this->m_width;
		}


		if(this->m_fullscreenWidth * height / width > this->m_fullscreenHeight) {

		    dst.left   = (this->m_fullscreenWidth - (this->m_fullscreenHeight * width / height)) / 2; 
			dst.top    = 0; 
			dst.right  = dst.left + (this->m_fullscreenHeight * width / height);
			dst.bottom = this->m_fullscreenHeight; 
		}
		else {

			dst.left   = 0;
			dst.top    = (this->m_fullscreenHeight - (this->m_fullscreenWidth * height / width)) / 2;
			dst.right  = this->m_fullscreenWidth;
			dst.bottom = (this->m_fullscreenHeight - (this->m_fullscreenWidth * height / width))/2 + (this->m_fullscreenWidth * height / width);
		}
	}
	else
	{
		// Compute the destination
		if (m_aspectRatioX == 0)
		{
			dst.left   = x; 
			dst.top    = y; 
			dst.right  = x + w;
			dst.bottom = y + h;
		}
		else
		{
			int nx, ny, width, height;
			int ratio1 = w * m_aspectRatioY / m_aspectRatioX; //w * m_height * m_aspectRatioY / m_aspectRatioX / m_width;
			int ratio2 = h * m_aspectRatioX / m_aspectRatioY; //h * m_width * m_aspectRatioX / m_aspectRatioY / m_height;
			if (ratio1 > h)
			{
				height = h;
				width = ratio2;
				nx = x + (w - width) / 2;
				ny = y;
			}
			else
			{
				width  = w;
				height = ratio1;
				nx = x;
				ny = y + (h - height) / 2;
			}
			dst.left   = nx; 
			dst.top    = ny; 
			dst.right  = nx + width;
			dst.bottom = ny + height;
		}
	}
	if (this->m_hwndPlayback) 
	{
		width	= dst.right - dst.left; 
		height	= dst.bottom - dst.top;

		BITMAPINFO FrameInfo;
		FrameInfo.bmiHeader.biSize = 40;
		FrameInfo.bmiHeader.biWidth = m_width;
		FrameInfo.bmiHeader.biHeight = m_height;
		FrameInfo.bmiHeader.biPlanes = 1;
		FrameInfo.bmiHeader.biBitCount = (m_videoMode == DDRAW_DIB24) ? 24 : 32;
		FrameInfo.bmiHeader.biCompression = BI_RGB;
		FrameInfo.bmiHeader.biSizeImage = 0;
		FrameInfo.bmiHeader.biXPelsPerMeter = 0; 
		FrameInfo.bmiHeader.biYPelsPerMeter = 0;
		FrameInfo.bmiHeader.biClrUsed = 0;
		FrameInfo.bmiHeader.biClrImportant = 0;

		HDC hDC = GetDC(m_hwndPlayback);
		if (ABS(width - m_width) <= 2 && ABS(height - m_height) <= 2)
		{
			int i = SetDIBitsToDevice(hDC, 
				dst.left, 
				dst.top, 
				FrameInfo.bmiHeader.biWidth, 
				FrameInfo.bmiHeader.biHeight,
				0, 
				0, 
				0, 
				FrameInfo.bmiHeader.biHeight, 
				m_pDecodeBuf, 
				&FrameInfo, 
				DIB_RGB_COLORS
				);
		}
		else
		{
			SetStretchBltMode(hDC, COLORONCOLOR);
			StretchDIBits(hDC,
				dst.left, 
				dst.top, 
				width, 
				height,
				0, 
				0, 
				FrameInfo.bmiHeader.biWidth, 
				FrameInfo.bmiHeader.biHeight, 
				m_pDecodeBuf, 
				&FrameInfo, 
				DIB_RGB_COLORS,
				SRCCOPY
				);
			PrintDualInt("width", width, "height", height);
			PrintDualInt("left", dst.left, "top", dst.top);
		}
		ReleaseDC(m_hwndPlayback, hDC);
	}

	m_x = x; m_y = y;
	m_w = w; m_h = h;
	return TRUE;
}

BOOL CVideoRenderDIB::SetAspectRatio(int aspectX, int aspectY)
{
	this->m_aspectRatioX = aspectX;
	this->m_aspectRatioY = aspectY;
	ShowUpdate();
	InvalidateRect(m_hwndPlayback, NULL, TRUE);
	UpdateWindow(m_hwndPlayback);
	return TRUE;
}

BOOL CVideoRenderDIB::GetAspectRatio(int &aspectX, int &aspectY)
{
	aspectX = m_aspectRatioX;
	aspectY = m_aspectRatioY;
	return TRUE;
}

BOOL CVideoRenderDIB::ShowUpdate()
{
	return Display(m_x, m_y, m_w, m_h);
}
