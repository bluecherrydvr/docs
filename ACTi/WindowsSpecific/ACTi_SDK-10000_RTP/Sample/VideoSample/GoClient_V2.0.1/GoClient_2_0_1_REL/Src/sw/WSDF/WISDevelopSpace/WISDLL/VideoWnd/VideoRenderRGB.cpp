/**************************************************************************************
 * Video Render RGB
 * created by weimin, Aug 5, 2002
 **************************************************************************************/

#include "stdafx.h"
#include "VideoRenderRGB.h"

CVideoRenderRGB::CVideoRenderRGB()
{
	this->lpdd         = NULL;
	this->lpddClipper  = NULL;
	this->lpddsPrimary = NULL;
	this->lpddsBack    = NULL;

	this->m_bpp          = 0;
	this->m_videoMode    = DDRAW_NULL;
	m_aspectRatioX = 0;
	m_aspectRatioY = 0;
}

CVideoRenderRGB::~CVideoRenderRGB()
{
	Close();
}

BOOL CVideoRenderRGB::HasRGBSupported()
{

    DDCAPS  capsDrv;
    HRESULT ddrval;
	
    // Get driver capabilities to determine BLT support.

    ZeroMemory(&capsDrv, sizeof(capsDrv));
    capsDrv.dwSize = sizeof(capsDrv);
 
    ddrval = this->lpdd->GetCaps(&capsDrv, NULL);
    
	if (FAILED(ddrval))
		return FALSE;
 
    // Does the driver support BLT in the current mode? 

    if (!(capsDrv.dwCaps & DDCAPS_BLT))
        return FALSE;

	if (!((capsDrv.dwFXCaps & DDFXCAPS_BLTSHRINKX) && (capsDrv.dwFXCaps & DDFXCAPS_BLTSHRINKY)
		&& (capsDrv.dwFXCaps & DDFXCAPS_BLTSTRETCHX) && (capsDrv.dwFXCaps & DDFXCAPS_BLTSTRETCHY)))
		return FALSE;

    return TRUE;
}

BOOL CVideoRenderRGB::Init(HWND hwnd, unsigned int width, unsigned int height, int preferredMode)
{
	HRESULT        ddrval;
	DDSURFACEDESC2 ddsd;

	if (this->lpdd && this->lpddsPrimary)
	{
		m_bFullScreen = FALSE;
		return TRUE;
	}

	if(hwnd && width > 0 && height > 0) 
	{
		ddrval = DirectDrawCreateEx(NULL, (void **) &this->lpdd, IID_IDirectDraw7, NULL);
		if(FAILED(ddrval))
			return FALSE;

		ddrval = lpdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL);

		if(FAILED(ddrval) || !HasRGBSupported()) 
		{
			this->lpdd->Release();
			this->lpdd = NULL;
			return FALSE;
		}
	
		// Create the primary surface

		ZeroMemory(&ddsd, sizeof(ddsd));
	    ddsd.dwSize     = sizeof( ddsd );
	    ddsd.dwFlags           = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
	    ddrval = this->lpdd->CreateSurface(&ddsd, &this->lpddsPrimary, NULL);
		if(FAILED(ddrval)) 
		{
			this->lpdd->Release();
			this->lpdd = NULL;
			return FALSE;
		}

		// Now set Clipping

		ddrval = this->lpdd->CreateClipper(0, &this->lpddClipper, NULL);
		if(FAILED(ddrval)) 
		{
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			this->lpdd->Release();
			this->lpdd = NULL;
			return FALSE;
		}

	    ddrval = this->lpddClipper->SetHWnd(0, hwnd);

	    if(FAILED(ddrval)) 
		{
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			this->lpdd->Release();
			this->lpdd = NULL;
			return FALSE;
		}

		ddrval = this->lpddsPrimary->SetClipper(this->lpddClipper);

		if(ddrval != DD_OK) 
		{
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			this->lpdd->Release();
			this->lpdd = NULL;
			return FALSE;
		}
	
		// Finally Create Back Surface
		ZeroMemory(&ddsd, sizeof(ddsd));
    	ddsd.dwSize     = sizeof(ddsd);
		ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.dwWidth  = width;
		ddsd.dwHeight = height;
		if(this->lpdd->CreateSurface(&ddsd, &this->lpddsBack, NULL) != DD_OK) 
		{
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			this->lpdd->Release();
			this->lpdd = NULL;
			return FALSE;
		}

		this->m_width  = width;
		this->m_height = height;

		this->m_hwndPlayback = hwnd;

		// Now Get the video mode
		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize     = sizeof(DDSURFACEDESC2);
		
		ddrval = this->lpddsBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);

		if(FAILED(ddrval))
			return FALSE;

		this->lpddsBack->Unlock(NULL);

		switch(ddsd.ddpfPixelFormat.dwRGBBitCount) 
		{
		case 8:
		case 16:
			return FALSE;
			break;
		case 24:
			this->m_bpp       = 3;
			this->m_videoMode = DDRAW_RGB24;
			break;
		case 32:
			this->m_bpp       = 4;
			this->m_videoMode = DDRAW_RGB32;
			break;
		}

		this->m_bFullScreen = FALSE;
		m_x = 0; m_y = 0;
		m_w = m_width; m_y = m_height;
		this->m_aspectRatioX = width;
		this->m_aspectRatioY = height;
		return TRUE;
	}

	return FALSE;
}

// Init the renderer for fullscreen drawing
BOOL CVideoRenderRGB::InitFullscreen(HWND hwnd, unsigned int width, unsigned int height, int preferredMode)
{
	if (this->lpdd && this->lpddsPrimary)
	{
		Close();
	}

	if(hwnd && width > 0 && height > 0) 
	{
		HRESULT         ddrval;
	    DDSURFACEDESC2  ddsd;
    
	    ddrval = DirectDrawCreateEx(NULL, (VOID**)&this->lpdd, IID_IDirectDraw7, NULL);
		if( FAILED(ddrval))
			return FALSE;
    
	    // Set Exclusive Cooperative Mode
		ddrval = this->lpdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL); //DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
		if( FAILED(ddrval)) 
		{
	        this->lpdd->Release();
			this->lpdd = NULL;
			return FALSE;
		}

		// Go to fullscreen

		DDSURFACEDESC2 ddDesc;
		memset(&ddDesc, 0, sizeof(DDSURFACEDESC2));
		ddDesc.dwSize    = sizeof(DDSURFACEDESC2);

		// We need to know witch resolution we are in...
		ddrval = this->lpdd->GetDisplayMode(&ddDesc);
		if(FAILED(ddrval)) 
		{
	        this->lpdd->Release();
			this->lpdd=NULL;
			return FALSE;
		}

		// Store the fullscreen mode
		this->m_fullscreenWidth  = ddDesc.dwWidth;
		this->m_fullscreenHeight = ddDesc.dwHeight;
		this->m_fullscreenVideoHeight = height * this->m_fullscreenWidth / width;

		// And create the primary surface
	    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize  = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_CAPS;// | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY; /*| DDSCAPS_FLIP | DDSCAPS_COMPLEX*/;
		//ddsd.dwBackBufferCount = 1;
		ddrval = this->lpdd->CreateSurface(&ddsd, &this->lpddsPrimary, NULL );
		if(FAILED(ddrval)) 
		{
	        this->lpdd->Release();
			this->lpdd=NULL;
			return FALSE;
		}

		// Get the back buffer
		ZeroMemory(&ddsd, sizeof(ddsd));
    	ddsd.dwSize     = sizeof(ddsd);
		ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth  = width;
		ddsd.dwHeight = height;
		if(this->lpdd->CreateSurface(&ddsd, &this->lpddsBack, NULL) != DD_OK) 
		{
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
			this->lpdd->Release();
			this->lpdd = NULL;
			return FALSE;
		}

		// Create a Clipper to avoid the overlay staying on top
	    ddrval = this->lpdd->CreateClipper(0, &this->lpddClipper, NULL);
		if(FAILED(ddrval))
			return FALSE;
	
	    ddrval = this->lpddClipper->SetHWnd(0, hwnd);
		if(FAILED(ddrval))
			return FALSE;
	
	    ddrval = this->lpddsPrimary->SetClipper(this->lpddClipper);
		if(FAILED(ddrval))
			return FALSE;

		// Black out the primary
		this->m_width  = width;
		this->m_height = height;
		this->m_hwndPlayback = hwnd;
		this->m_bFullScreen = TRUE;
		m_x = 0; m_y = 0;
		m_w = m_width; m_h = m_height;
		return TRUE;
	}
    
	return FALSE;
}

int CVideoRenderRGB::GetVideoMode()
{
	return this->m_videoMode;
}

BOOL CVideoRenderRGB::GetFullScreenMode()
{
	return this->m_bFullScreen;
}

BOOL CVideoRenderRGB::Stop()
{
	return TRUE;
}

RECT *CVideoRenderRGB::GetFullscreenRects()
{
	return NULL;
}

BOOL CVideoRenderRGB::Draw(unsigned char *buffer, RECT *rect, int invertFlag)
{
	HRESULT        ddrval;
	DDSURFACEDESC2 desc;
	DWORD          i;

	if(this->lpdd && this->lpddsBack && this->lpddsPrimary && buffer) 
	{
		ZeroMemory(&desc, sizeof(DDSURFACEDESC2));
		desc.dwSize     = sizeof(DDSURFACEDESC2);
		ddrval = this->lpddsBack->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);
		if(FAILED(ddrval))
			return FALSE;

		// Copy the pixels
		for(i = 0; i < desc.dwHeight; i++)
			memcpy((char *) desc.lpSurface + i * desc.lPitch, 
				buffer + (desc.dwHeight - i - 1) * this->m_bpp * this->m_width, this->m_width * this->m_bpp);

		this->lpddsBack->Unlock(NULL);

		
		// Now Blit to the window
		if(this->m_hwndPlayback) 
		{
			RECT                rcRect;
			RECT                destRect;
			POINT               pt;

	        rcRect.left   = 0;
		    rcRect.top    = 0;
			rcRect.right  = this->m_width;
			rcRect.bottom = this->m_height;

			GetClientRect( this->m_hwndPlayback, &destRect );

			// squeeze rect so that we have space for controls
			destRect.left   += rect->left;
			destRect.right  =  rect->left + rect->right;

			destRect.top    += rect->top;
			destRect.bottom  = rect->bottom;

			pt.x = pt.y = 0;
			ClientToScreen( this->m_hwndPlayback, &pt );
			OffsetRect(&destRect, pt.x, pt.y);

			while( 1 )
			{
				/*Max add 2004-04-23 add*/
				if(this->lpddsPrimary)
				ddrval = this->lpddsPrimary->Blt( &destRect, this->lpddsBack, &rcRect, DDBLT_ASYNC | DDBLT_WAIT, NULL);

				if( ddrval == DD_OK )
					break;

				if( ddrval == DDERR_SURFACELOST )
				{
					/*Max add 2004-04-23 add*/
					if(this->lpdd)
					if(!this->lpdd->RestoreAllSurfaces())
						return FALSE;
				}	

				if( ddrval != DDERR_WASSTILLDRAWING )
					return FALSE;
			}

			if(ddrval != DD_OK)
				return FALSE;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CVideoRenderRGB::DrawFullscreen(unsigned char *buffer, int invertFlag, int desktop)
{
	DDSURFACEDESC2 desc;
	DWORD          i;
	HRESULT        ddrval;

	if(buffer && this->lpdd && this->lpddsPrimary) 
	{
		ZeroMemory(&desc, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		this->lpddsBack->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);
		for(i=0; i < desc.dwHeight; i++) 
			memcpy((char *) desc.lpSurface + i * desc.lPitch, 
				buffer + (desc.dwHeight - i - 1) * this->m_bpp * this->m_width, this->m_width * this->m_bpp);

		this->lpddsBack->Unlock(NULL);

		// Now Blit to the Back Buffer

		RECT rcRect, dst;

		rcRect.left   = 0;
		rcRect.top    = 0;
		rcRect.right  = this->m_width;
		rcRect.bottom = this->m_height;

		int width, height;
		if(m_aspectRatioY < m_aspectRatioX) 
		{
			width  = this->m_width;
			height = this->m_width * m_aspectRatioY / m_aspectRatioX;
		}
		else 
		{
			height = this->m_height;
			width  = this->m_height * m_aspectRatioX / m_aspectRatioY;
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

		ddrval = this->lpddsPrimary->Blt( &dst, this->lpddsBack, &rcRect, DDBLT_ASYNC, NULL);
	
		return TRUE;
	}

	return FALSE;
}

BOOL CVideoRenderRGB::Close()
{
	if(this->lpdd) {
		
		this->lpdd->RestoreDisplayMode();
		this->lpdd->SetCooperativeLevel(this->m_hwndPlayback, DDSCL_NORMAL);
	}

	if ( this->lpddClipper )
	{
		this->lpddClipper->Release();
		this->lpddClipper = NULL;
	}

	if(this->lpddsBack) {
	
		this->lpddsBack->Release();
		this->lpddsBack = NULL;
	}

	if(this->lpddsPrimary) {
	
		this->lpddsPrimary->Release();
		this->lpddsPrimary = NULL;
	}

	if(this->lpdd) {
	
		this->lpdd->Release();
		this->lpdd = NULL;
	}

	return TRUE;
}

BYTE *CVideoRenderRGB::BeginDDPaint(int &lPitch)
{
	DDSURFACEDESC2 desc;
	
	/*Max modified 2004-04-23 for fix GoclientPlugin crash*/
	//if(this->lpdd && this->lpddsPrimary) 
	if(this->lpdd && this->lpddsPrimary && this->lpddsBack)
	{
		ZeroMemory(&desc, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		this->lpddsBack->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);
		lPitch = desc.lPitch;
		return (LPBYTE)desc.lpSurface;
	}
	return NULL;

}

BOOL CVideoRenderRGB::EndDDPaint(BOOL draw)
{
	if (this->lpddsBack == NULL)
		return FALSE;

	this->lpddsBack->Unlock(NULL);
	
	if (draw)
		Display(m_x, m_y, m_w, m_h);
	return TRUE;
}

BOOL CVideoRenderRGB::Display(int x, int y, int w, int h)
{
	HRESULT        ddrval;

	if (w == 0 && h == 0) return TRUE;
	if (m_bFullScreen)
	{
		RECT rcRect, dst;

		rcRect.left   = 0;
		rcRect.top    = 0;
		rcRect.right  = this->m_width;
		rcRect.bottom = this->m_height;

		int width, height;
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

		DDBLTFX ddBltFx;
		ZeroMemory(&ddBltFx, sizeof(ddBltFx));
		ddBltFx.dwSize = sizeof(ddBltFx);
		ddBltFx.dwDDFX = DDBLTFX_MIRRORUPDOWN;

//		ddrval = this->lpddsPrimary->Blt( &dst, this->lpddsBack, &rcRect, DDBLT_ASYNC | DDBLT_DDFX, &ddBltFx);
		/*Max add 2004-04-23 for fix GoClientPlugin Crash*/
		if(this->lpddsPrimary && this->lpddsBack)
		ddrval = this->lpddsPrimary->Blt( &dst, this->lpddsBack, &rcRect, DDBLT_ASYNC, &ddBltFx);
	
	}
	else
	{
		if (this->m_hwndPlayback) 
		{
			RECT                rcRect;
			RECT                rd;
			POINT               pt;

	        rcRect.left   = 0;
		    rcRect.top    = 0;
			rcRect.right  = this->m_width;
			rcRect.bottom = this->m_height;

			pt.x = 0;
			pt.y = 0;
			ClientToScreen(this->m_hwndPlayback, &pt);

			// Compute the destination
			if (m_aspectRatioX == 0)
			{
				rd.left   = pt.x + x; 
				rd.top    = pt.y + y; 
				rd.right  = pt.x + x + w;
				rd.bottom = pt.y + y + h;
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
				rd.left   = pt.x + nx; 
				rd.top    = pt.y + ny; 
				rd.right  = pt.x + nx + width;
				rd.bottom = pt.y + ny + height;
			}


			while( 1 )
			{
				DDBLTFX ddBltFx;
				ZeroMemory(&ddBltFx, sizeof(ddBltFx));
				ddBltFx.dwSize = sizeof(ddBltFx);
				ddBltFx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
//				ddrval = this->lpddsPrimary->Blt( &rd, this->lpddsBack, &rcRect, DDBLT_ASYNC | DDBLT_WAIT | DDBLT_DDFX , &ddBltFx);
				/*Max add 2004-04-23 for fix GoClientPlugin Crash*/
				if(this->lpddsPrimary && this->lpddsBack)
				ddrval = this->lpddsPrimary->Blt( &rd, this->lpddsBack, &rcRect, DDBLT_ASYNC | DDBLT_WAIT, &ddBltFx);

				if( ddrval == DD_OK )
					break;

				else if( ddrval == DDERR_SURFACELOST )
				{
					/*Max add 2004-04-23 for fix GoClientPlugin Crash*/
					if(this->lpdd)
					if(!this->lpdd->RestoreAllSurfaces())
						return FALSE;
				}	

				if( ddrval != DDERR_WASSTILLDRAWING )
					return FALSE;
			}

			if(ddrval != DD_OK)
				return FALSE;

		}
	}

	m_x = x; m_y = y;
	m_w = w; m_h = h;
	return TRUE;
}

BOOL CVideoRenderRGB::SetAspectRatio(int aspectX, int aspectY)
{
	this->m_aspectRatioX = aspectX;
	this->m_aspectRatioY = aspectY;
	ShowUpdate();
	InvalidateRect(m_hwndPlayback, NULL, TRUE);
	UpdateWindow(m_hwndPlayback);
	return TRUE;
}

BOOL CVideoRenderRGB::GetAspectRatio(int &aspectX, int &aspectY)
{
	aspectX = m_aspectRatioX;
	aspectY = m_aspectRatioY;
	return TRUE;
}

BOOL CVideoRenderRGB::ShowUpdate()
{
	return Display(m_x, m_y, m_w, m_h);
}
