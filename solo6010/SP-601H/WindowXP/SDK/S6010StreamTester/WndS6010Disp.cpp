// WndS6010Disp.cpp : implementation file
//

#include "stdafx.h"
#include "S6010StreamTester.h"
#include "WndS6010Disp.h"
#include "colorspace.h"

#include "DlgPlayerBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWndS6010Disp *glPWndDisp;

/////////////////////////////////////////////////////////////////////////////
// CWndS6010Disp

CWndS6010Disp::CWndS6010Disp()
{
	m_pDDraw = NULL;
	m_pDDClipper = NULL;
	m_pDDSPrimary = NULL;
	m_pDDSOffImage = NULL;
	m_pDDSBack = NULL;
	m_pDDSTemp = NULL;

	colorspace_init ();

	yv12_to_bgra = yv12_to_bgra_mmx;
	uyvy_to_yv12 = uyvy_to_yv12_mmx;
	yv12_to_yuyv = yv12_to_yuyv_mmx;

	m_bLButDn = FALSE;

	m_hFontOff = CreateFont (16, 12, 0, 0, FW_NORMAL, 0, 0, 0,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH, NULL);

	m_hPenRed = CreatePen (PS_SOLID, 1, RGB(255, 50, 50));

	m_idxOldSelDisp = -1;
	m_idxOldDispMode = -1;

	m_bPainting = FALSE;
}

CWndS6010Disp::~CWndS6010Disp()
{
	ReleaseDDraw ();

	DeleteObject (m_hFontOff);
	DeleteObject (m_hPenRed);
}


BEGIN_MESSAGE_MAP(CWndS6010Disp, CWnd)
	//{{AFX_MSG_MAP(CWndS6010Disp)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_KEYUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWndS6010Disp message handlers

int CWndS6010Disp::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	SetTimer (1, 100, NULL);

	glPWndDisp = this;

	return 0;
}

void CWndS6010Disp::OnDestroy() 
{
	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CWndS6010Disp::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here

	if (m_bPainting == FALSE)
	{
		BltBackToPrimaryWithDrawingBorder ();
	}
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CWndS6010Disp::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	m_ptLDnStart = point;
	m_ptLDnCur = point;

	ClientToScreen (&m_ptLDnStart);
	ClientToScreen (&m_ptLDnCur);

	m_bLButDn = TRUE;
	SetCapture ();

	CWnd::OnLButtonDown(nFlags, point);
}

void CWndS6010Disp::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_bLButDn == TRUE)
	{
		m_ptLDnCur = point;
		ClientToScreen (&m_ptLDnCur);

		if (m_ptLDnCur.x -m_ptLDnStart.x != 0 ||
			m_ptLDnCur.y -m_ptLDnStart.y != 0)
		{
			glPDlgPB->SendMessage (WM_DOO_DISP_WND_MOUSE_DRAG,
				m_ptLDnCur.x -m_ptLDnStart.x, m_ptLDnCur.y -m_ptLDnStart.y);
		}
		m_ptLDnStart = m_ptLDnCur;
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CWndS6010Disp::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_bLButDn == TRUE)
	{
		m_bLButDn = FALSE;
		ReleaseCapture ();

		int i, nNumDisp, nNumDivH, nNumDivV;
		CRect rcDisp;

		GetClientRect (&rcDisp);

		if (point.x < rcDisp.left || point.x >= rcDisp.right ||
			point.y < rcDisp.top || point.y >= rcDisp.bottom)
		{
			goto EXIT_OnLButtonUp;
		}

		nNumDisp = GL_DISP_MODE_TO_NUM_DISP[glPDlgPB->m_idxCurDispMode];

		if (glPDlgPB->m_idxCurDispMode == IDX_DISP_MODE_32)
		{
			nNumDivH = 8;
			nNumDivV = 4;
		}
		else
		{
			nNumDivH = GL_DISP_MODE_TO_NUM_DIV[glPDlgPB->m_idxCurDispMode];
			nNumDivV = GL_DISP_MODE_TO_NUM_DIV[glPDlgPB->m_idxCurDispMode];
		}

		for (i=0; i<nNumDisp; i++)
		{
			if (point.x >= rcDisp.left +(rcDisp.Width () /nNumDivH) *(i %nNumDivH) &&
				point.y >= rcDisp.top +(rcDisp.Height () /nNumDivV) *(i /nNumDivH) &&
				point.x < rcDisp.left +(rcDisp.Width () /nNumDivH) *(i %nNumDivH +1) &&
				point.y < rcDisp.top +(rcDisp.Height () /nNumDivV) *(i /nNumDivH +1))
			{
				glPDlgPB->SendMessage (WM_DOO_DISP_WND_L_CLICK, i, 0);
				break;
			}
		}
	}
EXIT_OnLButtonUp:

	CWnd::OnLButtonUp(nFlags, point);
}

void CWndS6010Disp::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	glPDlgPB->SendMessage (WM_DOO_DISP_WND_L_DBLCLK, 0, 0);
	CWnd::OnLButtonDblClk(nFlags, point);
}

BOOL CWndS6010Disp::InitDDraw (int szImgMaxH, int szImgMaxV)
{
	ReleaseDDraw ();

	HRESULT ddRVal;

	ddRVal = DirectDrawCreateEx (NULL, (void **)&m_pDDraw, IID_IDirectDraw7, NULL);
	if (ddRVal != DD_OK)
	{
		return FALSE;
	}

	ddRVal = m_pDDraw->SetCooperativeLevel (this->m_hWnd, DDSCL_NORMAL);
	if (ddRVal != DD_OK)
	{
		return FALSE;
	}

	DDSURFACEDESC2 ddSD2;

	// Primary Surface & Clipper[S]...
	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);
	ddSD2.dwFlags = DDSD_CAPS;
	ddSD2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_pDDSPrimary, NULL);
	if (ddRVal != DD_OK)
	{
		return FALSE;
	}

	ddRVal = m_pDDraw->CreateClipper (0, &m_pDDClipper, NULL);
	if (ddRVal != DD_OK)
	{
		return FALSE;
	}
	m_pDDClipper->SetHWnd (0, this->m_hWnd);
	m_pDDSPrimary->SetClipper (m_pDDClipper);

	if (CreateOffscreenSurface (szImgMaxH, szImgMaxV) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CWndS6010Disp::CreateOffscreenSurface (int szOffscrH, int szOffscrV)
{
	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;
	m_szResolution.cx = GetSystemMetrics (SM_CXSCREEN);
	m_szResolution.cy = GetSystemMetrics (SM_CYSCREEN);

	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);
	ddSD2.dwFlags = DDSD_CAPS |DDSD_HEIGHT |DDSD_WIDTH/* |DDSD_PIXELFORMAT*/;
//	ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_VIDEOMEMORY |DDSCAPS_NONLOCALVIDMEM;
	ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_VIDEOMEMORY |DDSCAPS_LOCALVIDMEM;
//	ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_SYSTEMMEMORY;

	ddSD2.dwWidth = m_szResolution.cx;
	ddSD2.dwHeight = m_szResolution.cy;

/*	DDPIXELFORMAT ddPF;
	ZeroMemory (&ddPF, sizeof(ddPF));
	ddPF.dwSize = sizeof(DDPIXELFORMAT);
	ddPF.dwFlags = DDPF_FOURCC;
	ddPF.dwFourCC = MAKEFOURCC('Y','V','1','2');
	ddSD2.ddpfPixelFormat = ddPF;
*/
	ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_pDDSOffImage, NULL);
	if (ddRVal != DD_OK)
	{
		ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_SYSTEMMEMORY;
		ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_pDDSOffImage, NULL);
		if (ddRVal != DD_OK)
		{
			return FALSE;
		}
	} 

	ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_pDDSBack, NULL);
	if (ddRVal != DD_OK)
	{
		return FALSE;
	}

	ddSD2.dwWidth = szOffscrH;
	ddSD2.dwHeight = szOffscrV;

	m_szTempSurfV = szOffscrV;
	ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_pDDSTemp, NULL);
	if (ddRVal != DD_OK)
	{
		return FALSE;
	}

	return TRUE;
}

void CWndS6010Disp::ReleaseDDraw ()
{
	if (m_pDDraw != NULL)
	{
		if (m_pDDSPrimary != NULL)
		{
			m_pDDSPrimary->Release ();
			m_pDDSPrimary = NULL;
		}

		if (m_pDDClipper != NULL)
		{
			m_pDDClipper->Release ();
			m_pDDClipper = NULL;
		}

		if (m_pDDSOffImage != NULL)
		{
			m_pDDSOffImage->Release ();
			m_pDDSOffImage = NULL;
		}

		if (m_pDDSBack != NULL)
		{
			m_pDDSBack->Release ();
			m_pDDSBack = NULL;
		}

		if (m_pDDSTemp != NULL)
		{
			m_pDDSTemp->Release ();
			m_pDDSTemp = NULL;
		}

		m_pDDraw->Release ();
		m_pDDraw = NULL;
	}
}

void CWndS6010Disp::BltBackToPrimaryWithDrawingBorder ()
{
	HRESULT ddRVal;
	CRect rcSrc;

	m_bPainting = TRUE;

	rcSrc.SetRect (0, 0, m_rcDisp.Width (), m_rcDisp.Height ());

	ddRVal = m_pDDSOffImage->Blt (&rcSrc, m_pDDSBack, &rcSrc, DDBLT_WAIT, NULL);
	if (ddRVal == DDERR_SURFACELOST)
	{
		ddRVal = m_pDDSOffImage->Restore ();
		if (ddRVal == DD_OK)
		{
			ddRVal = m_pDDSOffImage->Blt (&rcSrc, m_pDDSBack, &rcSrc, DDBLT_WAIT, NULL);
		}
	}

	// Draw Border[S]...
	HDC hDC;
	ddRVal = m_pDDSOffImage->GetDC (&hDC);

	if (ddRVal == DD_OK)
	{
		SelectObject (hDC, m_hPenRed);

		static int nNumDivH, nNumDivV, posUL[2], posUR[2], posDR[2], posDL[2];

		if (m_idxOldSelDisp != glPDlgPB->m_idxCurSelDisp ||
			m_idxOldDispMode != glPDlgPB->m_idxCurDispMode)
		{
			if (glPDlgPB->m_idxCurDispMode == IDX_DISP_MODE_32)
			{
				nNumDivH = 8;
				nNumDivV = 4;
			}
			else
			{
				nNumDivH = GL_DISP_MODE_TO_NUM_DIV[glPDlgPB->m_idxCurDispMode];
				nNumDivV = GL_DISP_MODE_TO_NUM_DIV[glPDlgPB->m_idxCurDispMode];
			}

			posUL[0] = (m_rcDisp.Width () /nNumDivH) *(glPDlgPB->m_idxCurSelDisp %nNumDivH);
			posUL[1] = (m_rcDisp.Height () /nNumDivV) *(glPDlgPB->m_idxCurSelDisp /nNumDivH);

			posUR[0] = (m_rcDisp.Width () /nNumDivH) *(glPDlgPB->m_idxCurSelDisp %nNumDivH +1) -1;
			posUR[1] = (m_rcDisp.Height () /nNumDivV) *(glPDlgPB->m_idxCurSelDisp /nNumDivH);

			posDR[0] = (m_rcDisp.Width () /nNumDivH) *(glPDlgPB->m_idxCurSelDisp %nNumDivH +1) -1;
			posDR[1] = (m_rcDisp.Height () /nNumDivV) *(glPDlgPB->m_idxCurSelDisp /nNumDivH +1) -1;

			posDL[0] = (m_rcDisp.Width () /nNumDivH) *(glPDlgPB->m_idxCurSelDisp %nNumDivH);
			posDL[1] = (m_rcDisp.Height () /nNumDivV) *(glPDlgPB->m_idxCurSelDisp /nNumDivH +1) -1;

			m_idxOldSelDisp = glPDlgPB->m_idxCurSelDisp;
			m_idxOldDispMode = glPDlgPB->m_idxCurDispMode;
		}

		// Draw selected display area
		MoveToEx (hDC, posUL[0], posUL[1], NULL);
		LineTo (hDC, posUR[0], posUR[1]);
		LineTo (hDC, posDR[0], posDR[1]);
		LineTo (hDC, posDL[0], posDL[1]);
		LineTo (hDC, posUL[0], posUL[1]);

		m_pDDSOffImage->ReleaseDC (hDC);
	}
	// Draw Border[E]...

	ddRVal = m_pDDSPrimary->Blt (&m_rcDisp, m_pDDSOffImage, &rcSrc, DDBLT_DONOTWAIT, NULL);
	if (ddRVal == DDERR_SURFACELOST)
	{
		ddRVal = m_pDDSPrimary->Restore ();
		if (ddRVal == DD_OK)
		{
			ddRVal = m_pDDSPrimary->Blt (&m_rcDisp, m_pDDSOffImage, &rcSrc, DDBLT_DONOTWAIT, NULL);
		}
	}

	m_bPainting = FALSE;
}

LPVOID CWndS6010Disp::LockAndGetTempSurface ()
{
	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;
	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);
	
	ddRVal = m_pDDSTemp->Lock (NULL, &ddSD2, DDLOCK_WRITEONLY, NULL);
	m_nTempPitch = ddSD2.lPitch;

	return ddSD2.lpSurface;
}

LPVOID CWndS6010Disp::LockAndGetBackSurface ()
{
	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;
	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);
	
	ddRVal = m_pDDSBack->Lock (NULL, &ddSD2, DDLOCK_WRITEONLY, NULL);
	m_nBackPitch = ddSD2.lPitch;

	return ddSD2.lpSurface;
}

void CWndS6010Disp::FillTempBlack ()
{
	BYTE *pTempSurf = (BYTE *)LockAndGetTempSurface ();
	DWORD nPitchTemp = GetTempPitch ();
	memset (pTempSurf, 0, nPitchTemp *m_szTempSurfV);
	UnlockTempSurface ();
}

void CWndS6010Disp::FillBackBlack ()
{
	BYTE *pBackSurf = (BYTE *)LockAndGetBackSurface ();
	DWORD nPitchBack = GetBackPitch ();
	memset (pBackSurf, 0, nPitchBack *m_szResolution.cy);
	UnlockBackSurface ();
}

void CWndS6010Disp::BltTempToBack (RECT rcDest, RECT rcSrc, BOOL bUseInfo, char *strInfo)
{
	HRESULT ddRVal;

	ddRVal = m_pDDSBack->Blt (&rcDest, m_pDDSTemp, &rcSrc, DDBLT_WAIT, NULL);
	if (ddRVal == DDERR_SURFACELOST)
	{
		ddRVal = m_pDDSBack->Restore ();
		if (ddRVal == DD_OK)
		{
			ddRVal = m_pDDSBack->Blt (&rcDest, m_pDDSTemp, &rcSrc, DDBLT_WAIT, NULL);
		}
	}

	if (bUseInfo == TRUE)
	{
		HDC hDC;
		ddRVal = m_pDDSBack->GetDC (&hDC);

		if (ddRVal == DD_OK)
		{
			SelectObject (hDC, m_hFontOff);
			SetBkMode (hDC, TRANSPARENT);

			SetTextColor (hDC, RGB(0, 0, 0));
			::ExtTextOut (hDC, rcDest.left +INFO_CH_OFFSET_X +1,
				rcDest.top +INFO_CH_OFFSET_Y, ETO_CLIPPED, &rcDest,
				strInfo, strlen (strInfo), NULL);
			::ExtTextOut (hDC, rcDest.left +INFO_CH_OFFSET_X,
				rcDest.top +INFO_CH_OFFSET_Y +1, ETO_CLIPPED, &rcDest,
				strInfo, strlen (strInfo), NULL);
			::ExtTextOut (hDC, rcDest.left +INFO_CH_OFFSET_X +1,
				rcDest.top +INFO_CH_OFFSET_Y +1, ETO_CLIPPED, &rcDest,
				strInfo, strlen (strInfo), NULL);

			SetTextColor (hDC, RGB(255, 255, 255));
			::ExtTextOut (hDC, rcDest.left +INFO_CH_OFFSET_X,
				rcDest.top +INFO_CH_OFFSET_Y, ETO_CLIPPED, &rcDest,
				strInfo, strlen (strInfo), NULL);

			m_pDDSBack->ReleaseDC (hDC);
		}
	}
}

void CWndS6010Disp::YV12toYUYV (BYTE *bufSrc, BYTE *bufDest, int szH, int szV, int nPitch)
{
	safe_packed_conv(
		bufDest, nPitch, &bufSrc[0], &bufSrc[szH *szV], &bufSrc[szH *szV *5 /4],
		szH, szH >>1, szH, szV, 0,
		yv12_to_yuyv,
		yv12_to_yuyv_c, 4);
	_asm emms;
}

void CWndS6010Disp::YV12toBGRA (BYTE *bufSrc, BYTE *bufDest, int szH, int szV, int nPitch)
{
	safe_packed_conv(
		bufDest, nPitch, &bufSrc[0], &bufSrc[szH *szV], &bufSrc[szH *szV *5 /4],
		szH, szH >>1, szH, szV, 0,
		yv12_to_bgra,
		yv12_to_bgra_c, 4);
	_asm emms;
}

void CWndS6010Disp::DrawMotionData (int nPitch, int szH, int szV,
	BYTE *bufDisp, BYTE *pBufMotion)
{
	int i, j, k, l;

	int szMbH = szH /16;
	int szMbV = szV /16;
	BYTE *pCur;

	for (i=0, k=0; i<szMbV; i++)
	{
		for (j=0; j<szMbH; j++, k++)
		{
			if (((pBufMotion[k /8] >>(k %8)) &0x1) == 0x1)
			{
				pCur = &bufDisp[i *nPitch *16 +j *4 *16] +2;

				for (l=0; l<16; l++)
				{
					*pCur = 0xff;
					pCur += 4;
				}
				pCur += (nPitch -(16 *4));
				for (l=0; l<14; l++)
				{
					*pCur = 0xff;
					pCur += 15 *4;
					*pCur = 0xff;
					pCur += (nPitch -(16 *4)) +4;
				}
				for (l=0; l<16; l++)
				{
					*pCur = 0xff;
					pCur += 4;
				}
			}
		}
	}
}

void CWndS6010Disp::DrawWMData (int nPitch, int szH, int szV,
	BYTE *bufDisp, BYTE *pBufWM)
{
	int i, j, l;

	int szMbH = szH /16;
	int szMbV = szV /16;
	BYTE *pCur;

	for (i=0; i<szMbV; i++)
	{
		for (j=0; j<szMbH; j++)
		{
			if (pBufWM[i *szMbH +j] != 0x1)
			{
				pCur = &bufDisp[i *nPitch *16 +j *4 *16] +0;

				for (l=0; l<16; l++)
				{
					*pCur = 0xff;
					pCur += 4;
				}
				pCur += (nPitch -(16 *4));
				for (l=0; l<14; l++)
				{
					*pCur = 0xff;
					pCur += 15 *4;
					*pCur = 0xff;
					pCur += (nPitch -(16 *4)) +4;
				}
				for (l=0; l<16; l++)
				{
					*pCur = 0xff;
					pCur += 4;
				}
			}
		}
	}
}

void CWndS6010Disp::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnTimer(nIDEvent);
}

void CWndS6010Disp::OnMove(int x, int y) 
{
	CWnd::OnMove(x, y);
	
	// TODO: Add your message handler code here
	
	GetClientRect (&m_rcDisp);
	ClientToScreen (&m_rcDisp);

	m_idxOldSelDisp = -1;
	m_idxOldDispMode = -1;
}

void CWndS6010Disp::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	GetClientRect (&m_rcDisp);
	ClientToScreen (&m_rcDisp);

	m_idxOldSelDisp = -1;
	m_idxOldDispMode = -1;
}

void CWndS6010Disp::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default

	glPDlgPB->OnKeyDownFromWndDisp (nChar, nRepCnt, nFlags);
	
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}
