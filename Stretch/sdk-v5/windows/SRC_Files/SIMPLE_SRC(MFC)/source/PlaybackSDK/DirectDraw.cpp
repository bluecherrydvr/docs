// DirectDraw.cpp : implementation file
//

#include "stdafx.h"
#include "DirectDraw.h"
#include "memcpy.h"

// Define some macros
#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))

// CDirectDraw
IMPLEMENT_DYNAMIC(CDirectDraw, CWnd)
CDirectDraw::CDirectDraw()
{
	m_IsInitDirect=false;
}

CDirectDraw::~CDirectDraw()
{
}


BEGIN_MESSAGE_MAP(CDirectDraw, CWnd)
END_MESSAGE_MAP()

BOOL CDirectDraw::InitDirectDraw(USHORT usImgWidth, USHORT usImgHeight, HWND hWnd)
{
	HRESULT					ddRval;				

	if(DirectDrawCreateEx(NULL, (void**)&m_lpDD, IID_IDirectDraw7, NULL) != DD_OK)
	{	
		//TRACE("DirectDrawCreate fail!\n");
		return FALSE;
	}

	if (m_lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES) != DD_OK)
	{
		//TRACE("SetCooperativeLevel fail!\n");
		return FALSE;
	}

	ZeroMemory(&m_ddsds, sizeof(m_ddsds));
	m_ddsds.dwSize = sizeof(m_ddsds);
	m_ddsds.dwFlags = DDSD_CAPS;
	m_ddsds.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (m_lpDD->CreateSurface(&m_ddsds, &m_lpDDSPrimary, NULL) != DD_OK)
	{
		//TRACE("CreatePrimarySurface fail!\n");
		return FALSE;
	}

	//	RECT rc;
	//	GetClientRect(&rc);

	//	TRACE("top=%d left=%d bottom=%d right=%d\n", rc.top, rc.left, rc.bottom, rc.right);

	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);
	m_ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY; //DDSCAPS_OVERLAY DDSCAPS_OFFSCREENPLAIN;
	m_ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	m_ddsd.dwWidth = /*rc.right-rc.left;*/usImgWidth;//DRAW_WIDHT;
	m_ddsd.dwHeight = /*rc.bottom-rc.top;*/usImgHeight;//DRAW_HEIGHT;
	m_ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	m_ddsd.ddpfPixelFormat.dwFlags  = DDPF_FOURCC | DDPF_YUV ;
	m_ddsd.ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y','V','1','2');
	m_ddsd.ddpfPixelFormat.dwYUVBitCount = 8;
	if (m_lpDD->CreateSurface(&(m_ddsd), &m_lpDDSOffScr, NULL) != DD_OK)
	{
		//TRACE("CreateOffscreenSurface fail!\n");
		return FALSE;
	}

	//********************************************************************************************************************//

	if( FAILED( ddRval = m_lpDD->CreateClipper( 0, &m_lpClipper, NULL ) ) )
	{
		//TRACE("CreateClipper fail!\n");
		return E_FAIL;
	}

	if( FAILED( ddRval =m_lpDDSPrimary->SetClipper( m_lpClipper ) ) )
	{
		m_lpClipper->Release();
		m_lpClipper = NULL;
		//TRACE("SetClipper fail!\n");
		return E_FAIL;
	}

	if( FAILED( ddRval = m_lpClipper->SetHWnd( 0, hWnd ) ) )
	{
		m_lpClipper->Release();
		m_lpClipper = NULL;
		//TRACE("SetHWnd fail!\n");
		return E_FAIL;
	}

	return TRUE;
}




BOOL CDirectDraw::CloseDirectDraw()
{
	if(m_lpDD != NULL)
	{
		// Done with clipper
		if(m_lpClipper != NULL)
		{
			m_lpClipper->Release();
			m_lpClipper = NULL;
		}

		if(m_lpDDSPrimary != NULL)
		{
			m_lpDDSPrimary->Release();
			m_lpDDSPrimary = NULL;
		}


		if(m_lpDDSOffScr != NULL)
		{
			m_lpDDSOffScr->Release();
			m_lpDDSOffScr = NULL;
		}
		if(m_lpDD)
		{
			m_lpDD->Release();
			m_lpDD = NULL;
		}
	}
	return TRUE;
}




BOOL CDirectDraw::DrawYV12(LPBYTE lpYV12, USHORT usImgWidth, USHORT usImgHeight, CWnd* pcWnd, HWND hWnd, RECT rcdraw)
{
	RECT					rcClient;
	RECT					rctSour;			
	HRESULT			    ddRval;	

	pcWnd->GetClientRect(&rcClient);	

	LPBYTE lpY = lpYV12;
	LPBYTE lpU = lpYV12 + usImgWidth * usImgHeight;
	LPBYTE lpV = lpYV12 + usImgWidth * usImgHeight * 5 / 4;

	if (m_lpDDSOffScr == 0)
	{
		ATLTRACE(_T("%s Line:%d m_lpDDSOffScr == 0!\n"), __FILE__, __LINE__);
		return FALSE;
	}

	ddRval = m_lpDDSOffScr->Lock(NULL,&m_ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,NULL);
	while(ddRval == DDERR_WASSTILLDRAWING);
	if(ddRval != DD_OK)
	{
		//TRACE("LOCK Fail! 0x%X\n", ddRval);
		return FALSE;
	}
	LPBYTE lpSurf = (LPBYTE)m_ddsd.lpSurface;
	int nOffset = 0;

	if(lpSurf)
	{
		int i = 0;
		// fill Y data
		lpY += nOffset;
		for(i=0; i<(int)m_ddsd.dwHeight; i++)
		{
			if(m_ddsd.dwWidth > 64)
				memcpyMMX(lpSurf, lpY, m_ddsd.dwWidth);
			else
				CopyMemory(lpSurf, lpY, m_ddsd.dwWidth);
			lpY += usImgWidth;
			lpSurf += m_ddsd.lPitch;
		}

		// fill V data
		for(i=0; i<(int)m_ddsd.dwHeight/2; i++)
		{
			if(m_ddsd.dwWidth/2 > 64)
				memcpyMMX(lpSurf, lpV, m_ddsd.dwWidth / 2);
			else
				CopyMemory(lpSurf, lpV, m_ddsd.dwWidth / 2);
			lpV += usImgWidth / 2;
			lpSurf += m_ddsd.lPitch / 2;
		}

		// fill U data
		for(i=0; i<(int)m_ddsd.dwHeight/2; i++)
		{
			if(m_ddsd.dwWidth/2 > 64)
				memcpyMMX(lpSurf, lpU, m_ddsd.dwWidth / 2);
			else
				memcpy(lpSurf, lpU, m_ddsd.dwWidth / 2);
			lpU += usImgWidth / 2;
			lpSurf += m_ddsd.lPitch / 2;
		}
	}
	m_lpDDSOffScr->Unlock(NULL);

	rctSour.left = 0;
	rctSour.top = 0;	
	rctSour.right = usImgWidth;
	rctSour.bottom = usImgHeight;

	::ClientToScreen(hWnd, (LPPOINT)&rcClient.left);
	::ClientToScreen(hWnd, (LPPOINT)&rcClient.right);
	rcClient.right = rcClient.left + (rcdraw.right - rcdraw.left);
	rcClient.bottom = rcClient.top + (rcdraw.bottom - rcdraw.top);

	if ((m_lpDDSOffScr == 0) || (m_lpDDSPrimary == 0))
	{
		ATLTRACE(_T("%s Line:%d m_lpDDSPrimary == 0 || m_lpDDSOffScr == 0!\n"), __FILE__, __LINE__);
		return FALSE;
	}

	ddRval = m_lpDDSPrimary->Blt(&rcClient, m_lpDDSOffScr, &rctSour, DDBLT_WAIT, NULL);

	while(ddRval == DDERR_WASSTILLDRAWING);
	if(ddRval != DD_OK)
	{
		OutputDebugString("Blt fail!\n");
		return FALSE;
	}

	return TRUE;
}


void CDirectDraw::memcpyMMX(void *Dest, void *Src, size_t nBytes)
{
	__asm 
	{
		mov     esi, dword ptr[Src]
		mov     edi, dword ptr[Dest]
		mov     ecx, nBytes
			shr     ecx, 6                      // nBytes / 64
			align 8
CopyLoop:
		movq    mm0, qword ptr[esi]
		movq    mm1, qword ptr[esi+8*1]
		movq    mm2, qword ptr[esi+8*2]
		movq    mm3, qword ptr[esi+8*3]
		movq    mm4, qword ptr[esi+8*4]
		movq    mm5, qword ptr[esi+8*5]
		movq    mm6, qword ptr[esi+8*6]
		movq    mm7, qword ptr[esi+8*7]
		movq    qword ptr[edi], mm0
			movq    qword ptr[edi+8*1], mm1
			movq    qword ptr[edi+8*2], mm2
			movq    qword ptr[edi+8*3], mm3
			movq    qword ptr[edi+8*4], mm4
			movq    qword ptr[edi+8*5], mm5
			movq    qword ptr[edi+8*6], mm6
			movq    qword ptr[edi+8*7], mm7
			add     esi, 64
			add     edi, 64
			loop CopyLoop
			mov     ecx, nBytes
			and     ecx, 63
			cmp     ecx, 0
			je EndCopyLoop
			align 8
CopyLoop2:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
			inc esi
			inc edi
			dec ecx
			jne CopyLoop2
EndCopyLoop:
		emms	//Empty MMX state
	}
}



// CDirectDraw message handlers

