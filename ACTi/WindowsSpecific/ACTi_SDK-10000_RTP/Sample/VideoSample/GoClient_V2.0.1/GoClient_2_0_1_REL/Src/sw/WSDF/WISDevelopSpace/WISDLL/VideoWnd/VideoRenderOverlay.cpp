/**************************************************************************************
 * Video render overlay implement
 * created by weimin, Aug 5, 2002
 **************************************************************************************/

#include "stdafx.h"
#include "VideoRenderOverlay.h"
#include "DebugPrint.h"

DDPIXELFORMAT ddpfOverlayFormats[] = 
{   
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0},  // UYVY
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0},  // YUY2
};

int videoModes[] = 
{
	DDRAW_UYVY,
	DDRAW_YUY2,
};

CVideoRenderOverlay::CVideoRenderOverlay()
{
	this->m_aspectRatioX = 0;
	this->m_aspectRatioY = 0;
	this->lpdd         = NULL;
	this->lpddsPrimary = NULL;
	lpddsPrimary1 = NULL;
	this->lpddsOverlay = NULL;
	this->lpddsOverlayBack = NULL;
}

CVideoRenderOverlay::~CVideoRenderOverlay()
{
	Close();
}

BOOL CVideoRenderOverlay::AreOverlaysSupported()
{

    DDCAPS  capsDrv;
    HRESULT ddrval;
	
    // Get driver capabilities to determine Overlay support.

    ZeroMemory(&capsDrv, sizeof(capsDrv));
    capsDrv.dwSize = sizeof(capsDrv);
 
    ddrval = this->lpdd->GetCaps(&capsDrv, NULL);
    
	if (FAILED(ddrval))
		return FALSE;
 
    // Does the driver support overlays in the current mode? 
    // Overlay related APIs will fail without hardware support.

    if (!(capsDrv.dwCaps & DDCAPS_OVERLAY))
        return FALSE;
	
	if (capsDrv.dwMaxVisibleOverlays <= capsDrv.dwCurrVisibleOverlays)
		return FALSE;

    return TRUE;
}


BOOL CVideoRenderOverlay::Init(HWND hwnd, unsigned int width, unsigned int height, int preferredMode)
{
	if(!hwnd || width <= 0 || height <= 0) return FALSE;

	if (this->lpdd && this->lpddsPrimary)
	{
		m_bFullScreen = FALSE;
		return TRUE;
	}

	HRESULT         ddrval;
	DDSURFACEDESC2  ddsd;
	DDSURFACEDESC2  ddsdOverlay;
	DWORD           i;

	this->m_invertFlag  = FALSE;

	ddrval = DirectDrawCreateEx(NULL, (VOID**)&this->lpdd, IID_IDirectDraw7, NULL);
	if( FAILED(ddrval))
	{
		OutputDebugString("DirectDrawCreateEx failed");
		return FALSE;
	}

	// Set Normal Cooperative Mode
	ddrval = this->lpdd->SetCooperativeLevel(NULL, DDSCL_NORMAL);
	if( FAILED(ddrval)) 
	{
	    this->lpdd->Release();
		this->lpdd=NULL;
		OutputDebugString("SetCooperativeLevel Failed");
		return FALSE;
	}

	// And create the primary surface
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize  = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddrval = this->lpdd->CreateSurface(&ddsd, &this->lpddsPrimary, NULL );
	if(FAILED(ddrval)) 
	{
		this->lpdd->Release();
		this->lpdd=NULL;
		OutputDebugString("Create Primary Surface failed");
		return FALSE;
	}

	if(!this->AreOverlaysSupported()) 
	{
	    this->lpdd->Release();
		this->lpdd=NULL;
		this->lpddsPrimary->Release();
		this->lpddsPrimary = NULL;
		OutputDebugString("Overlays are not supported");
		return FALSE;
	}

	// Create a Clipper to avoid the overlay staying on top
	ddrval = this->lpdd->CreateClipper(0, &this->lpddClipper, NULL);
	if(FAILED(ddrval)){
		OutputDebugString("Create Clipper failed");
		return FALSE;
	}
	ddrval = this->lpddClipper->SetHWnd(0, hwnd);
	if(FAILED(ddrval)){
		OutputDebugString("SetHWnd failed");
		return FALSE;
	}
	ddrval = this->lpddsPrimary->SetClipper(this->lpddClipper);

	this->lpddClipper->Release();
	this->lpddClipper = NULL;

	// Now create the Overlay
	// First try the best configuration
	ZeroMemory(&ddsdOverlay, sizeof(DDSURFACEDESC2));
	ddsdOverlay.dwSize  = sizeof(DDSURFACEDESC2);
	ddsdOverlay.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsdOverlay.dwFlags= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;
	ddsdOverlay.dwWidth	= width;
	ddsdOverlay.dwHeight= height;
    ddsdOverlay.dwBackBufferCount = 1;

	i = 0;
	m_bNeedFlip = TRUE;

	DDPIXELFORMAT ddFormat = ddpfOverlayFormats[0];
	if (preferredMode == DDRAW_YUY2)
		ddFormat =  ddpfOverlayFormats[1];
	ddsdOverlay.ddpfPixelFormat = ddFormat;
	this->m_videoMode = preferredMode;
  	ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);

	if (FAILED(ddrval))
	{
		do 
		{
			ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
			this->m_videoMode = videoModes[i];
  			ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);
		} while(FAILED(ddrval) && (++i < 2));
	}

	// If it failed try a more simple overlay
	if(FAILED(ddrval)) 
	{
		ZeroMemory(&ddsdOverlay, sizeof(DDSURFACEDESC2));
		ddsdOverlay.dwSize     = sizeof(DDSURFACEDESC2);
		
		ddsdOverlay.dwWidth           = width;
		ddsdOverlay.dwHeight          = height;
	    ddsdOverlay.dwBackBufferCount = 0;
		
		ddsdOverlay.ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
		ddsdOverlay.dwFlags           = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    
		// Try to create the overlay surface
		i = 0;
		do 
		{
			ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
			this->m_videoMode = videoModes[i];
	 		ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);
		} while(FAILED(ddrval) && (++i < 2));
    
		m_bNeedFlip = FALSE;
		if (FAILED(ddrval)) 
		{
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
		    this->lpdd->Release();
			this->lpdd=NULL;
			OutputDebugString("Create Overlay failed");
	        return FALSE;
		}
	}
	else
	{
		DDSCAPS2       ddscaps;
		ZeroMemory(&ddscaps, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if( FAILED( ddrval = lpddsOverlay->GetAttachedSurface( &ddscaps, &lpddsOverlayBack ) ) )
		{
			OutputDebugString("Get back surface failed");
			return FALSE;
		}
	}

	if(FAILED(ddrval)) 
		return FALSE;

	this->m_width  = width;
	this->m_height = height;
	this->m_bpp    = 16;

	// Now get the Physical Depth
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize     = sizeof(DDSURFACEDESC2);
	ddrval = this->lpddsPrimary->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);
	if(FAILED(ddrval))
	{
		OutputDebugString("Lock primary failed");
		return FALSE;
	}
	this->lpddsPrimary->Unlock(NULL);

	this->m_physicalDepth = ddsd.ddpfPixelFormat.dwRGBBitCount; 
	
	// Get the screen size
	ddrval = this->lpdd->GetDisplayMode(&ddsd);
	this->m_fullscreenWidth  = ddsd.dwWidth;
	this->m_fullscreenHeight = ddsd.dwHeight;
	this->m_hwndPlayback = hwnd;
	this->m_bFullScreen = FALSE;
	this->m_aspectRatioX = width;
	this->m_aspectRatioY = height;

	return TRUE;
}

BOOL CVideoRenderOverlay::InitFullscreen(HWND hwnd, unsigned int width, unsigned int height, int preferredMode)
{
	if(!hwnd || width <= 0 || height <= 0) return FALSE;
	
	if (this->lpdd && this->lpddsPrimary)
	{
		m_bFullScreen = TRUE;
		return TRUE;
	}

	HRESULT         ddrval;
	DDSURFACEDESC2  ddsd;
	DDSURFACEDESC2  ddsdOverlay;
	DWORD           i;

	this->m_invertFlag  = FALSE;

	ddrval = DirectDrawCreateEx(NULL, (VOID**)&this->lpdd, IID_IDirectDraw7, NULL);
	if( FAILED(ddrval))
		return FALSE;

	// Check for Overlay Support
	if(!this->AreOverlaysSupported()) 
	{
	    this->lpdd->Release();
		this->lpdd=NULL;
		return FALSE;
	}
	 
	// Set Normal Cooperative Mode
	ddrval = this->lpdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
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
	this->m_fullscreenWidth  = ddDesc.dwWidth;
	this->m_fullscreenHeight = ddDesc.dwHeight;

	// And create the primary surface
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize  = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddrval = this->lpdd->CreateSurface(&ddsd, &this->lpddsPrimary, NULL );
	if(FAILED(ddrval)) 
	{
		this->lpdd->Release();
		this->lpdd=NULL;
		return FALSE;
	}

	 // Now create the Overlay
	 // First try the best configuration
	ZeroMemory(&ddsdOverlay, sizeof(DDSURFACEDESC2));
	ddsdOverlay.dwSize  = sizeof(DDSURFACEDESC2);
	ddsdOverlay.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
	ddsdOverlay.dwFlags= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;
	ddsdOverlay.dwWidth           = width;
	ddsdOverlay.dwHeight          = height;
    ddsdOverlay.dwBackBufferCount = 1;

	m_bNeedFlip = TRUE;
	i = 0;
	do 
	{
		ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
		this->m_videoMode = videoModes[i];
  		ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);
	} while(FAILED(ddrval) && (++i < 2));

	// If it failed try a more simple overlay
	if(FAILED(ddrval)) 
	{
		ZeroMemory(&ddsdOverlay, sizeof(DDSURFACEDESC2));
		ddsdOverlay.dwSize     = sizeof(DDSURFACEDESC2);
		ddsdOverlay.dwWidth           = width;
		ddsdOverlay.dwHeight          = height;
	    ddsdOverlay.dwBackBufferCount = 0;
		ddsdOverlay.ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
		ddsdOverlay.dwFlags           = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    
		// Try to create the overlay surface
		i = 0;
		do 
		{
			ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
			this->m_videoMode = videoModes[i];
	 		ddrval = this->lpdd->CreateSurface(&ddsdOverlay, &this->lpddsOverlay, NULL);
		} while(FAILED(ddrval) && (++i < 2));
    
		m_bNeedFlip = FALSE;
		if (FAILED(ddrval)) 
		{
			this->lpddsPrimary->Release();
			this->lpddsPrimary = NULL;
		    this->lpdd->Release();
			this->lpdd=NULL;
	        return FALSE;
		}
	}
	else
	{
		DDSCAPS2       ddscaps;
		ZeroMemory(&ddscaps, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if( FAILED( ddrval = lpddsOverlay->GetAttachedSurface( &ddscaps, &lpddsOverlayBack ) ) )
			return FALSE;
	}

	if(FAILED(ddrval))
		return FALSE;

	this->m_width  = width;
	this->m_height = height;
	this->m_bpp    = 16;
	this->m_hwndPlayback = hwnd;
	this->m_bFullScreen = TRUE;
	this->m_aspectRatioX = width;
	this->m_aspectRatioY = height;

	return TRUE;
}

int	CVideoRenderOverlay::GetVideoMode()
{
	return this->m_videoMode;
}

BOOL CVideoRenderOverlay::GetFullScreenMode()
{
	return this->m_bFullScreen;
}

BOOL CVideoRenderOverlay::Stop()
{
	if(this->lpdd && this->lpddsOverlay) 
		this->lpddsOverlay->UpdateOverlay(NULL, this->lpddsPrimary, NULL, DDOVER_HIDE, NULL);

	return TRUE;
}

RECT *CVideoRenderOverlay::GetFullscreenRects()
{
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

	if(this->m_fullscreenWidth * height / width > this->m_fullscreenHeight) 
	{
		this->m_fullRects[0].left   = 0;
		this->m_fullRects[0].right  = (this->m_fullscreenWidth - (width * this->m_fullscreenHeight / height)) / 2;
		this->m_fullRects[0].top    = 0;
		this->m_fullRects[0].bottom = this->m_fullscreenHeight + 10;

		this->m_fullRects[1].left   = (this->m_fullscreenWidth - (width * this->m_fullscreenHeight / height)) / 2;
		this->m_fullRects[1].right  = (width * this->m_fullscreenHeight / height);
		this->m_fullRects[1].top    = 0;
		this->m_fullRects[1].bottom = this->m_fullscreenHeight + 10;

		this->m_fullRects[2].left   = this->m_fullRects[1].left + this->m_fullRects[1].right;
		this->m_fullRects[2].right  = (this->m_fullscreenWidth - (width * this->m_fullscreenHeight / height)) / 2;
		this->m_fullRects[2].top    = 0;
		this->m_fullRects[2].bottom = this->m_fullscreenHeight + 10;
	}
	else 
	{
		this->m_fullRects[0].left   = 0;
		this->m_fullRects[0].right  = this->m_fullscreenWidth;
		this->m_fullRects[0].top    = 0;
		this->m_fullRects[0].bottom = (this->m_fullscreenHeight - (this->m_fullscreenWidth * height / width)) / 2;

		this->m_fullRects[1].left   = 0;
		this->m_fullRects[1].right  = this->m_fullscreenWidth;
		this->m_fullRects[1].top    = (this->m_fullscreenHeight - (this->m_fullscreenWidth * height / width)) / 2;
		this->m_fullRects[1].bottom = this->m_fullscreenWidth * height / width;

		this->m_fullRects[2].left   = 0;
		this->m_fullRects[2].right  = this->m_fullscreenWidth;
		this->m_fullRects[2].top    = this->m_fullRects[1].top + this->m_fullRects[1].bottom;
		this->m_fullRects[2].bottom = (this->m_fullscreenHeight - (this->m_fullscreenWidth * height / width)) / 2;
	}
	
	return this->m_fullRects;
}

int CVideoRenderOverlay::Draw(unsigned char *buffer, RECT *rect, int invertFlag)
{
	if(buffer && rect && this->lpdd && this->lpddsPrimary && this->lpddsOverlay) 
	{
		POINT           pt;
	    HRESULT         ddrval;
	    RECT            rs, rd;
		DDOVERLAYFX     ovfx;
		DDSURFACEDESC2  ddsd;
		DDCAPS          capsDrv;
		unsigned int    uStretchFactor1000, i;
		unsigned int    uDestSizeAlign, uSrcSizeAlign;
		DWORD           dwUpdateFlags;
 
		// First copy the image into the Overlays backbuffer...
		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize     = sizeof(DDSURFACEDESC2);
		if (m_bNeedFlip)
			ddrval = this->lpddsOverlayBack->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		else
			ddrval = this->lpddsOverlay->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if(FAILED(ddrval))
			return FALSE;

		if(invertFlag) 
		{
			for(i = 0; i < ddsd.dwHeight; i++)
				memcpy((char *) ddsd.lpSurface + i * ddsd.lPitch, 
					buffer + (m_height - i - 1) * ddsd.dwWidth * m_bpp / 8, ddsd.dwWidth * m_bpp / 8);
		}
		else 
		{
			if(ddsd.dwWidth == ddsd.lPitch)
				memcpy(ddsd.lpSurface, buffer, this->m_width * this->m_height * this->m_bpp/8);
			else 
			{
				for(i = 0; i < ddsd.dwHeight; i++)
					memcpy((char *) ddsd.lpSurface + i * ddsd.lPitch, 
						buffer + i * ddsd.dwWidth * m_bpp / 8, ddsd.dwWidth * m_bpp / 8);
			}
		}
		
		if (m_bNeedFlip)
			this->lpddsOverlayBack->Unlock(NULL);
		else
			this->lpddsOverlay->Unlock(NULL);

		// Get driver capabilities
		ZeroMemory(&capsDrv, sizeof(DDCAPS));
		capsDrv.dwSize    = sizeof(DDCAPS);
		ddrval = this->lpdd->GetCaps(&capsDrv, NULL);
		if (FAILED(ddrval))
			return FALSE;
    
	    /* 
		 * We need to check the minimum stretch.  Many display adpators require that
		 * the overlay be stretched by a minimum amount.  The stretch factor will 
		 * usually vary with the display mode (including changes in refresh rate).
		 * The stretch factor is returned x1000.
         * 
		 */

		uStretchFactor1000 = capsDrv.dwMinOverlayStretch > 1000 ? capsDrv.dwMinOverlayStretch : 1000;
    
		/*
		 * Grab any alignment restrictions.  The DDCAPS struct contains a series of
		 * alignment fields that are not clearly defined. They are intended for
		 * overlay use.  It's important to observe alignment restrictions.
		 * Many adapters with overlay capabilities require the overlay image be
		 * located on 4 or even 8 byte boundaries, and have similar restrictions
		 * on the overlay width (for both source and destination areas).
		 * 
		 */

		uDestSizeAlign = capsDrv.dwAlignSizeDest;
	    uSrcSizeAlign =  capsDrv.dwAlignSizeSrc;
    
	    dwUpdateFlags = DDOVER_SHOW | DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;
    
		if (capsDrv.dwCKeyCaps & DDCKEYCAPS_SRCOVERLAY)
			dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;
    
		ZeroMemory(&ovfx, sizeof(DDOVERLAYFX));
		ovfx.dwSize     = sizeof(DDOVERLAYFX);

		ovfx.dckSrcColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORREF;
		ovfx.dckSrcColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORREF;

		// Compute the destination
		pt.x = 0;
		pt.y = 0;
		ClientToScreen(this->m_hwndPlayback, &pt);
	    rd.left   = pt.x + rect->left; 
		rd.top    = pt.y + rect->top; 
		rd.right  = pt.x + rect->left + (rect->right  * uStretchFactor1000 + 999) / 1000;
		rd.bottom = pt.y + rect->bottom * uStretchFactor1000 / 1000;

		if (capsDrv.dwCaps & DDCAPS_ALIGNSIZEDEST && uDestSizeAlign)
			rd.right = (int)((rd.right + uDestSizeAlign - 1) / uDestSizeAlign) * uDestSizeAlign;

		/*
		 * To setup properly the source rectangle  
		 * we need to know if we have to perform clipping
		 * at the edges of the screen...
		 *
		 */

		rs.left   = 0; 
		rs.top    = 0; 
		rs.right  = this->m_width;
		rs.bottom = this->m_height;

		if(rd.right < this->m_fullscreenWidth && rd.left > 0 && rd.top > 0 && rd.bottom < this->m_fullscreenHeight) 
		{
		}
		else 
		{
			if(rd.right > this->m_fullscreenWidth) 
			{
				rs.right  = (this->m_fullscreenWidth - rd.left) * this->m_width / (rd.right - rd.left);
				rd.right = this->m_fullscreenWidth;
			}
			if(rd.left < 0) 
			{
				rs.left = -rd.left * this->m_width / (rd.right - rd.left);
				rd.left = 0;
			}
			if(rd.bottom > this->m_fullscreenHeight) 
			{
				rs.bottom  = (this->m_fullscreenHeight - rd.top) * this->m_height / (rd.bottom - rd.top);
				rd.bottom = this->m_fullscreenHeight;
			}
			if(rd.top < 0) 
			{
				rs.top = -rd.top * this->m_height / (rd.bottom - rd.top);
				rd.top = 0;
			}
		}

   
		// Apply any size alignment restrictions if necessary.

		if (capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC && uSrcSizeAlign)
			rs.right -= rs.right % uSrcSizeAlign;

		// Now Update the Overlay
		ddrval = this->lpddsOverlay->UpdateOverlay(&rs, this->lpddsPrimary, &rd, dwUpdateFlags, &ovfx);
    
		if(FAILED(ddrval))
			return FALSE;
    
		return TRUE;
	}

	return FALSE;
}

int CVideoRenderOverlay::DrawFullscreen(unsigned char *buffer, int invertFlag, int desktop)
{
	if(buffer && this->lpdd && this->lpddsPrimary && this->lpddsOverlay) 
	{
		HRESULT         ddrval;
	    RECT            rs, rd;
		DDOVERLAYFX     ovfx;
		DDSURFACEDESC2  ddsd;
		DDCAPS          capsDrv;
		unsigned int    uStretchFactor1000, i;
		unsigned int    uDestSizeAlign, uSrcSizeAlign;
		DWORD           dwUpdateFlags;
 
		// First copy the image into the Overlays backbuffer...
		ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
		ddsd.dwSize     = sizeof(DDSURFACEDESC2);
		if (m_bNeedFlip)
			ddrval = this->lpddsOverlayBack->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		else
			ddrval = this->lpddsOverlay->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if(FAILED(ddrval))
			return FALSE;

		if(invertFlag) 
		{
			for(i = 0; i < ddsd.dwHeight; i++)
				memcpy((char *) ddsd.lpSurface + i * ddsd.lPitch, 
					buffer + (m_height - i - 1) * ddsd.dwWidth * m_bpp / 8, ddsd.dwWidth * m_bpp/8);
		}
		else 
		{
			if(ddsd.dwWidth == ddsd.lPitch)
				memcpy(ddsd.lpSurface, buffer, this->m_width * this->m_height * this->m_bpp/8);
			else 
			{
				for(i = 0; i < ddsd.dwHeight; i++)
					memcpy((char *) ddsd.lpSurface + i * ddsd.lPitch, 
						buffer + i * ddsd.dwWidth * m_bpp/8, ddsd.dwWidth * m_bpp/8);
			}
		}

		if (m_bNeedFlip)
			this->lpddsOverlayBack->Unlock(NULL);
		else
			this->lpddsOverlay->Unlock(NULL);

	    // Get driver capabilities
		ZeroMemory(&capsDrv, sizeof(DDCAPS));
		capsDrv.dwSize    = sizeof(DDCAPS);
		ddrval = this->lpdd->GetCaps(&capsDrv, NULL);
		if (FAILED(ddrval))
			return FALSE;
    
	    /* 
		 * We need to check the minimum stretch.  Many display adpators require that
		 * the overlay be stretched by a minimum amount.  The stretch factor will 
		 * usually vary with the display mode (including changes in refresh rate).
		 * The stretch factor is returned x1000.
         * 
		 */

		uStretchFactor1000 = capsDrv.dwMinOverlayStretch > 1000 ? capsDrv.dwMinOverlayStretch : 1000;
    
		/*
		 * Grab any alignment restrictions.  The DDCAPS struct contains a series of
		 * alignment fields that are not clearly defined. They are intended for
		 * overlay use.  It's important to observe alignment restrictions.
		 * Many adapters with overlay capabilities require the overlay image be
		 * located on 4 or even 8 byte boundaries, and have similar restrictions
		 * on the overlay width (for both source and destination areas).
		 * 
		 */

		uDestSizeAlign = capsDrv.dwAlignSizeDest;
	    uSrcSizeAlign =  capsDrv.dwAlignSizeSrc;
    
	    dwUpdateFlags = DDOVER_SHOW | DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;
    
		if (capsDrv.dwCKeyCaps & DDCKEYCAPS_SRCOVERLAY)
			dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;
    
		
		ZeroMemory(&ovfx, sizeof(DDOVERLAYFX));
		ovfx.dwSize     = sizeof(DDOVERLAYFX);

		ovfx.dckSrcColorkey.dwColorSpaceLowValue  = DD_OVERLAY_COLORREF;
		ovfx.dckSrcColorkey.dwColorSpaceHighValue = DD_OVERLAY_COLORREF;
		
	    rs.left   = 0; 
		rs.top    = 0; 
		rs.right  = this->m_width;
		rs.bottom = this->m_height;
    
		/*
		 * Apply any size alignment 
		 * restrictions if necessary.
		 *
		 */

		if (capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC && uSrcSizeAlign)
			rs.right -= rs.right % uSrcSizeAlign;
    
		/*
		 * Compute the destination
		 */

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


		if(this->m_fullscreenWidth * height / width > this->m_fullscreenHeight) 
		{
		    rd.left   = (this->m_fullscreenWidth - (this->m_fullscreenHeight * width / this->m_height)) / 2; 
			rd.top    = 0; 
			rd.right  = rd.left + (this->m_fullscreenHeight * width / height);
			rd.bottom = this->m_fullscreenHeight; 
		}
		else 
		{	
		    rd.left   = 0; 
			rd.top    = (this->m_fullscreenHeight - (this->m_fullscreenWidth * height / width)) / 2; 
			rd.right  = this->m_fullscreenWidth;
			rd.bottom = rd.top + (this->m_fullscreenWidth * height / width); 
		}

		if (capsDrv.dwCaps & DDCAPS_ALIGNSIZEDEST && uDestSizeAlign)
			rd.right = (int)((rd.right + uDestSizeAlign - 1) / uDestSizeAlign) * uDestSizeAlign;

		// Now Update the Overlay
		ddrval = this->lpddsOverlay->UpdateOverlay(&rs, this->lpddsPrimary, &rd, dwUpdateFlags, &ovfx);
    
		if(FAILED(ddrval))
			return FALSE;
   
		return TRUE;
	}
 
	return FALSE;
}

BOOL CVideoRenderOverlay::Close()
{
	EndDDPaint(0);
	if(this->lpdd) {
		
		HRESULT hr = this->lpdd->RestoreDisplayMode();
		PrintHex("RestoreDisplayMode", hr);
		this->lpdd->SetCooperativeLevel(this->m_hwndPlayback, DDSCL_NORMAL);
	}

	if(this->lpddsOverlayBack) {
	
		this->lpddsOverlayBack->Release();
		this->lpddsOverlayBack = NULL;
	}

	if(this->lpddsOverlay) {
	
		this->lpddsOverlay->Release();
		this->lpddsOverlay = NULL;
	}

	if(this->lpddsPrimary) {
	
		this->lpddsPrimary->Release();
		this->lpddsPrimary = NULL;
	}

	if(this->lpdd) {
	
		this->lpdd->Release();
		this->lpdd = NULL;
	}

	this->m_invertFlag  = FALSE;

	return TRUE;
}

unsigned char *CVideoRenderOverlay::BeginDDPaint(int &lPitch)
{
    HRESULT             ddrval;
    DDSURFACEDESC2      ddsd;
    LPBYTE              pSurf;
    
    if (lpddsOverlay == NULL)
        return NULL;
    
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
	ddsd.dwSize     = sizeof(DDSURFACEDESC2);

	int i = 0;
	while(i++ < 1)
	{
		if (m_bNeedFlip)
			ddrval = this->lpddsOverlayBack->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		else
			ddrval = this->lpddsOverlay->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

		if (ddrval == DD_OK)
			break;
		else
		{
			EndDDPaint(0);
			if (ddrval == DDERR_SURFACELOST)
			{
//				ddrval = this->lpdd->RestoreAllSurfaces();
				ddrval = this->lpddsPrimary->Restore();
				ddrval = this->lpddsOverlay->Restore();
				ddrval = this->lpddsOverlayBack->Restore();
				if (FAILED(ddrval))
					return NULL;
			}
			else
			{
				if( ddrval != DDERR_WASSTILLDRAWING )
					return NULL;
			}
		}
	}
    
    lPitch = ddsd.lPitch;
    pSurf=(LPBYTE)ddsd.lpSurface;

	return pSurf;
}

BOOL CVideoRenderOverlay::EndDDPaint(BOOL draw)
{
    if (lpddsOverlay == NULL)
        return NULL;

	if (m_bNeedFlip)
		lpddsOverlayBack->Unlock(NULL);     
	else
		lpddsOverlay->Unlock(NULL);     

	if(m_bNeedFlip && draw)
	{
		HRESULT hr = lpddsOverlay->Flip( NULL, DDFLIP_WAIT );
 	}
    return TRUE;
}

BOOL CVideoRenderOverlay::Display(int x, int y, int w, int h)
{
	if (x == 0 && y == 0 && w == 0 && h == 0) return TRUE;
	if(this->lpdd && this->lpddsPrimary && this->lpddsOverlay && (!m_bNeedFlip || (m_bNeedFlip && this->lpddsOverlayBack))) 
	{
	    HRESULT         ddrval;
	    RECT            rs, rd;
		DDOVERLAYFX     ovfx;
		DDCAPS          capsDrv;
		unsigned int    uStretchFactor1000;
		DWORD           dwUpdateFlags;
 
		// Get driver capabilities
		ZeroMemory(&capsDrv, sizeof(DDCAPS));
		capsDrv.dwSize    = sizeof(DDCAPS);
		ddrval = this->lpdd->GetCaps(&capsDrv, NULL);
		if (FAILED(ddrval))
			return FALSE;
    
	    /* 
		 * We need to check the minimum stretch.  Many display adpators require that
		 * the overlay be stretched by a minimum amount.  The stretch factor will 
		 * usually vary with the display mode (including changes in refresh rate).
		 * The stretch factor is returned x1000.
         * 
		 */

		uStretchFactor1000 = capsDrv.dwMinOverlayStretch > 1000 ? capsDrv.dwMinOverlayStretch : 1000;

	    dwUpdateFlags = DDOVER_SHOW; 
    
/*		if (capsDrv.dwCKeyCaps & DDCKEYCAPS_SRCOVERLAY)
			dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;
*/  
		ZeroMemory(&ovfx, sizeof(DDOVERLAYFX));
		ovfx.dwSize     = sizeof(DDOVERLAYFX);

		if (capsDrv.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY)
		{
			ovfx.dckDestColorkey.dwColorSpaceLowValue  = DDColorMatch(DD_OVERLAY_COLORREF); 
			ovfx.dckDestColorkey.dwColorSpaceHighValue = DDColorMatch(DD_OVERLAY_COLORREF); 
			dwUpdateFlags |= DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;
		}

		if (m_bFullScreen)
		{
			rs.left   = 0; 
			rs.top    = 0; 
			rs.right  = this->m_width;
			rs.bottom = this->m_height;
    
			// Compute the destination

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

			if(this->m_fullscreenWidth * height / width > this->m_fullscreenHeight) 
			{
				rd.left   = (this->m_fullscreenWidth - (this->m_fullscreenHeight * width / this->m_height)) / 2; 
				rd.top    = 0; 
				rd.right  = rd.left + (this->m_fullscreenHeight * width / height);
				rd.bottom = this->m_fullscreenHeight; 
			}
			else 
			{	
				rd.left   = 0; 
				rd.top    = (this->m_fullscreenHeight - (this->m_fullscreenWidth * height / width)) / 2; 
				rd.right  = this->m_fullscreenWidth;
				rd.bottom = rd.top + (this->m_fullscreenWidth * height / width); 
			}
		}
		else
		{
			POINT pt;
			pt.x = 0;
			pt.y = 0;
			ClientToScreen(this->m_hwndPlayback, &pt);

			// Compute the destination
			if (m_aspectRatioX == 0)
			{
				rd.left   = pt.x + x; 
				rd.top    = pt.y + y; 
				rd.right  = pt.x + x + ((x + w)  * uStretchFactor1000 + 999) / 1000;
				rd.bottom = pt.y + y + h * uStretchFactor1000 / 1000;
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
				rd.right  = pt.x + nx + (width  * uStretchFactor1000 + 999) / 1000;
				rd.bottom = pt.y + ny + height * uStretchFactor1000 / 1000;
			}

			// To setup properly the source rectangle we need to know if we have to perform clipping at the edges of the screen...
			rs.left   = 0; 
			rs.top    = 0; 
			rs.right  = this->m_width;
			rs.bottom = this->m_height;

			if(rd.right < this->m_fullscreenWidth && rd.left > 0 && rd.top > 0 && rd.bottom < this->m_fullscreenHeight) 
			{
			}
			else 
			{
				if(rd.right > this->m_fullscreenWidth) 
				{
					rs.right  = (this->m_fullscreenWidth - rd.left) * this->m_width / (rd.right - rd.left);
					rd.right = this->m_fullscreenWidth;
				}
				if(rd.left < 0) 
				{
					rs.left = -rd.left * this->m_width / (rd.right - rd.left);
					rd.left = 0;
				}
				if(rd.bottom > this->m_fullscreenHeight) 
				{
					rs.bottom  = (this->m_fullscreenHeight - rd.top) * this->m_height / (rd.bottom - rd.top);
					rd.bottom = this->m_fullscreenHeight;
				}
				if(rd.top < 0) 
				{
					rs.top = -rd.top * this->m_height / (rd.bottom - rd.top);
					rd.top = 0;
				}
			}
  		}
		// Apply any size alignment restrictions if necessary.

		if( (capsDrv.dwCaps & DDCAPS_ALIGNBOUNDARYSRC) && 
			(capsDrv.dwAlignBoundarySrc) )
		{
			rs.left = (rs.left + capsDrv.dwAlignBoundarySrc / 2) & 
							-(signed) (capsDrv.dwAlignBoundarySrc);
		}

		if( (capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC) && 
			(capsDrv.dwAlignSizeSrc) )
		{
			rs.right = rs.left + (rs.right - rs.left + capsDrv.dwAlignSizeSrc / 2) & 
							 -(signed) (capsDrv.dwAlignSizeSrc);
		}

		if( (capsDrv.dwCaps & DDCAPS_ALIGNBOUNDARYDEST) && 
			(capsDrv.dwAlignBoundaryDest) )
		{
			rd.left = ( rd.left + capsDrv.dwAlignBoundaryDest / 2 ) & 
						   -(signed) (capsDrv.dwAlignBoundaryDest);
		}

		if( (capsDrv.dwCaps & DDCAPS_ALIGNSIZEDEST) && 
			(capsDrv.dwAlignSizeDest) )
		{
			rd.right = rd.left + (rd.right - rd.left) & 
							-(signed) (capsDrv.dwAlignSizeDest);
		}
 
		// Now Update the Overlay
		ddrval = this->lpddsOverlay->UpdateOverlay(&rs, this->lpddsPrimary, &rd, dwUpdateFlags, &ovfx);
    
		if(FAILED(ddrval))
			return FALSE;
    
		m_x = x; m_y = y;
		m_w = w; m_h = h;
		return TRUE;
	}

	return FALSE;
}

DWORD CVideoRenderOverlay::DDColorMatch(COLORREF rgb)
{
    COLORREF                rgbT;
    HDC                     hdc;
    DWORD                   dw = CLR_INVALID;
    DDSURFACEDESC2          ddsd;
    HRESULT                 hres;

    //
    //  Use GDI SetPixel to color match for us
    //
    if (rgb != CLR_INVALID && lpddsPrimary->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, rgb);       // Set our value
        lpddsPrimary->ReleaseDC(hdc);
    }
    //
    // Now lock the surface so we can read back the converted color
    //
    ddsd.dwSize = sizeof(ddsd);
    while ((hres = lpddsPrimary->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING);

    if (hres == DD_OK)
    {
        dw = *(DWORD *) ddsd.lpSurface;                 // Get DWORD
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
        lpddsPrimary->Unlock(NULL);
    }
    //
    //  Now put the color that was there back.
    //
    if (rgb != CLR_INVALID && lpddsPrimary->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        lpddsPrimary->ReleaseDC(hdc);
    }
    return dw;
}

BOOL CVideoRenderOverlay::SetAspectRatio(int aspectX, int aspectY)
{
	this->m_aspectRatioX = aspectX;
	this->m_aspectRatioY = aspectY;
	ShowUpdate();
	return TRUE;
}

BOOL CVideoRenderOverlay::GetAspectRatio(int &aspectX, int &aspectY)
{
	aspectX = m_aspectRatioX;
	aspectY = m_aspectRatioY;
	return TRUE;
}

BOOL CVideoRenderOverlay::ShowUpdate()
{
	return Display(m_x, m_y, m_w, m_h);
}
