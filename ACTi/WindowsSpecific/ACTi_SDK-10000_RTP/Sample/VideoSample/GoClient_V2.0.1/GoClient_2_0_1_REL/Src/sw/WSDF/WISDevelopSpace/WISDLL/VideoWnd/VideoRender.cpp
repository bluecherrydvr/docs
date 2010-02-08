/**************************************************************************************
 * Video Render
 * created by weimin, Aug 6, 2002
 **************************************************************************************/

#include "stdafx.h"
#include "VideoRender.h"
#include	<../WSDF.H>
#include	<../MultiMedia.H>


CVideoRender::CVideoRender()
{
	this->overlayRender = new CVideoRenderOverlay();
	this->rgbRender     = new CVideoRenderRGB();
	this->dibRender     = new CVideoRenderDIB();

	this->render        = NULL;
}

CVideoRender::~CVideoRender()
{
	delete this->overlayRender;
	delete this->rgbRender;
	delete this->dibRender;
}

BOOL CVideoRender::Init(HWND hwnd, 
						unsigned int width, 
						unsigned int height, 
						int preferredMode)
{
	this->render = NULL;

	switch(preferredMode) 
	{
	case DDRAW_DIB24:
	case DDRAW_DIB32:
		if(this->dibRender->Init(hwnd, width, height, preferredMode) == TRUE) 
		{
			this->render = (CVideoRender *) this->dibRender;
			return TRUE;
		}
		else
			return FALSE;
		break;

	case DDRAW_RGB24:
	case DDRAW_RGB32:
		if(this->rgbRender->Init(hwnd, width, height, preferredMode) == TRUE) 
		{
			this->render = (CVideoRender *) this->rgbRender;
			return TRUE;
		}
		else
		{
			if(this->dibRender->Init(hwnd, width, height, preferredMode) == TRUE) 
			{
				this->render = (CVideoRender *) this->dibRender;
				return TRUE;
			}
			else
				return FALSE;
		}
		break;

	case DDRAW_YV12:
	case DDRAW_UYVY:
	case DDRAW_YUY2:
		if(this->overlayRender->Init(hwnd, width, height, preferredMode) == TRUE) 
		{
			this->render = (CVideoRender *) this->overlayRender;
			return TRUE;
		}
		else 
		{
			// Try the standard render here!
			if(this->rgbRender->Init(hwnd, width, height, DDRAW_RGB24) == TRUE) 
			{
				this->render = (CVideoRender *) this->rgbRender;
				return TRUE;
			}
			else
			{
				if(this->dibRender->Init(hwnd, width, height, DDRAW_DIB24) == TRUE) 
				{
					this->render = (CVideoRender *) this->dibRender;
					return TRUE;
				}
				return FALSE;
			}
		}
		break;
	}

	return FALSE;
}

BOOL CVideoRender::InitFullscreen(HWND hwnd, 
								  unsigned int width, 
								  unsigned int height,
								  int preferredMode)
{
	this->render = NULL;

	switch(preferredMode) 
	{
	case DDRAW_DIB24:
	case DDRAW_DIB32:
		if(this->dibRender->InitFullscreen(hwnd, width, height, preferredMode) == TRUE) 
		{
			this->render = (CVideoRender *) this->dibRender;
			return TRUE;
		}
		else
			return FALSE;
		break;

	case DDRAW_RGB24:
	case DDRAW_RGB32:
		if(this->rgbRender->InitFullscreen(hwnd, width, height, preferredMode) == TRUE) 
		{
			this->render = (CVideoRender *) this->rgbRender;
			return TRUE;
		}
		else
		{
			if(this->dibRender->InitFullscreen(hwnd, width, height, preferredMode) == TRUE) 
			{
				this->render = (CVideoRender *) this->dibRender;
				return TRUE;
			}
			return FALSE;
		}
		break;

	case DDRAW_YV12:
	case DDRAW_UYVY:
	case DDRAW_YUY2:
		if(this->overlayRender->InitFullscreen(hwnd, width, height, preferredMode) == TRUE) 
		{
			this->render = (CVideoRender *) this->overlayRender;
			return TRUE;
		}
		else 
		{
			// Try the standard render here!
			if(this->rgbRender->InitFullscreen(hwnd, width, height, DDRAW_RGB24) == TRUE) 
			{
				this->render = (CVideoRender *) this->rgbRender;
				return TRUE;
			}
			else
			{
				if(this->dibRender->InitFullscreen(hwnd, width, height, DDRAW_DIB24) == TRUE) 
				{
					this->render = (CVideoRender *) this->dibRender;
					return TRUE;
				}
				return FALSE;
			}
		}
		break;
	}

	return FALSE;
}

BOOL CVideoRender::Stop()
{
	if(this->render)
		return this->render->Stop();

	return FALSE;
}

RECT *CVideoRender::GetFullscreenRects()
{
	if(this->render)
		return this->render->GetFullscreenRects();

	return NULL;
}

int CVideoRender::GetVideoMode()
{
	if(this->render)
		return this->render->GetVideoMode();

	return DDRAW_NULL;
}

BOOL CVideoRender::GetFullScreenMode()
{
	if(this->render)
		return this->render->GetFullScreenMode();
	return FALSE;
}

BOOL CVideoRender::Draw(unsigned char *buffer, RECT *rect, int invertFlag)
{
	if(this->render)
		return this->render->Draw(buffer, rect, invertFlag);

	return FALSE;
}

BOOL CVideoRender::DrawFullscreen(unsigned char *buffer, int invertFlag, int desktop)
{
	if(this->render)
		return this->render->DrawFullscreen(buffer, invertFlag, desktop);

	return FALSE;
}

BOOL CVideoRender::Close()
{
	if(this->render) 
	{
		this->render->Close();
		this->render = NULL;
		return TRUE;
	}

	return FALSE;
}

BYTE *CVideoRender::BeginDDPaint(int &lPitch)
{
	if(this->render) 
		return this->render->BeginDDPaint(lPitch);

	return FALSE;
}

BOOL CVideoRender::EndDDPaint(BOOL draw)
{
	if(this->render) 
		return this->render->EndDDPaint(draw);

	return FALSE;
}

BOOL CVideoRender::Display(int x, int y, int w, int h)
{
	if(this->render) 
		return this->render->Display(x, y, w, h);

	return FALSE;
}

BOOL CVideoRender::SetAspectRatio(int aspectX, int aspectY)
{
	if(this->render) 
		return this->render->SetAspectRatio(aspectX, aspectY);

	return FALSE;
}

BOOL CVideoRender::GetAspectRatio(int &aspectX, int &aspectY)
{
	if(this->render) 
		return this->render->GetAspectRatio(aspectX, aspectY);

	return FALSE;
}

BOOL CVideoRender::ShowUpdate()
{
	if(this->render) 
		return this->render->ShowUpdate();

	return FALSE;
}

