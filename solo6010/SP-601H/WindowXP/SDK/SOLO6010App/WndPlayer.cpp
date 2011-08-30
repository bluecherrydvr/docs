// WndPlayer.cpp : implementation file
//

#include "stdafx.h"
#include "solo6010app.h"
#include "WndPlayer.h"
#include "DlgCtrlPlayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgCtrlPlayer glDlgCtrlPlayer;

/////////////////////////////////////////////////////////////////////////////
// CWndPlayer

CWndPlayer::CWndPlayer()
{
	m_pPlayer = NULL;
	m_modeDraw = WNDPL_DRAW_OFFSCREEN;
	m_idxSplitMode = LIVE_SPLIT_MODE_1;

	m_idxCS = PM_CS_BGRA;
	m_bufSupportedCS = NULL;
	m_numSupportedCS = 0;

	m_bLButDn = FALSE;
	m_bRButDn = FALSE;
	m_bLRButDnMouseMove = FALSE;

	m_pDDraw = NULL;
	m_pDDClipper = NULL;
	m_pDDSPrimary = NULL;
	m_pDDSBack = NULL;
	m_pDDSTemp = NULL;
	m_pDDSForPostFlt = NULL;

	m_hPenSelWnd = CreatePen (PS_SOLID, 1, WNDPL_COLOR_PEN_SEL_WND);
	m_hPenBorder = CreatePen (PS_SOLID, 1, WNDPL_COLOR_PEN_BORDER);

	m_hFontInfo = CreateFont (20, 15, 0, 0, FW_BOLD, 0, 0, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
		DEFAULT_QUALITY, FIXED_PITCH |FF_DONTCARE, NULL);

	m_bInfoDisp = TRUE;
	m_bInfoDisp_Ch = TRUE;
	m_bInfoDisp_Frame = TRUE;
	m_bInfoDisp_Time = TRUE;

	int i;
	
	// Criticalsection create[S]
	for (i=0; i<WNDPL_NUM_CRITICAL_SECTION; i++)
	{
		InitializeCriticalSection (&m_bufCrt[i]);
	}
	// Criticalsection create[E]
}

CWndPlayer::~CWndPlayer()
{
	DeleteObject (m_hPenSelWnd);
	DeleteObject (m_hPenBorder);
	DeleteObject (m_hFontInfo);
}


BEGIN_MESSAGE_MAP(CWndPlayer, CWnd)
	//{{AFX_MSG_MAP(CWndPlayer)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWndPlayer message handlers

int CWndPlayer::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	glDlgCtrlPlayer.Create (IDD_DLG_CTRL_PLAYER, this);

	CRect rcWnd, rcDlg;

	GetWindowRect (&rcWnd);
	glDlgCtrlPlayer.GetWindowRect (&rcDlg);

	glDlgCtrlPlayer.SetWindowPos (0,
		rcWnd.right, rcWnd.top,
		0, 0, SWP_NOSIZE |SWP_NOZORDER);

	InitDDraw ();
	GetBltRect ();

	if (CreateOffscreenSurfaces () == FALSE)
	{
		AddReport ("ERROR: Offscreen surface creation fail");

		m_modeDraw = WNDPL_DRAW_NULL;
	}
	else
	{
		ClearScreen (WNDPL_COLOR_BKGND);
	}
	
	return 0;
}

void CWndPlayer::OnDestroy() 
{
	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	
	DestroyOffscreenSurfaces ();
	DestroyDDraw ();
}

void CWndPlayer::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages

	Draw ();
}

void CWndPlayer::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
	if (bShow == TRUE)
	{
		glDlgCtrlPlayer.ShowWindow (SW_SHOW);
	}
	else
	{
		glDlgCtrlPlayer.ShowWindow (SW_HIDE);
	}
}

void CWndPlayer::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	if (m_modeDraw == WNDPL_DRAW_NULL)
	{
		return;
	}

	BOOL bPlaying = FALSE;
	if (m_pPlayer != NULL && m_pPlayer->GetPlayMode () == PM_MODE_PLAYING)
	{
		bPlaying = TRUE;
		m_pPlayer->Pause ();
	}

	GetBltRect ();

	DestroyOffscreenSurfaces ();
	CreateOffscreenSurfaces ();

	FillBackSurface (WNDPL_COLOR_BKGND);
	
	if (m_pPlayer != NULL && m_pPlayer->IsVideoFileOpen () == TRUE)
	{
		m_pPlayer->SetFramePos (m_pPlayer->GetFramePos ());

		if (bPlaying == TRUE)
		{
			m_pPlayer->Play ();
		}
	}

	Invalidate (FALSE);
}

void CWndPlayer::OnMove(int x, int y) 
{
	CWnd::OnMove(x, y);
	
	// TODO: Add your message handler code here
	
	GetBltRect ();
	Invalidate (FALSE);
}

BOOL CWndPlayer::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CWnd::OnEraseBkgnd(pDC);
}

void CWndPlayer::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_bLButDn == TRUE)
	{
		m_bLRButDnMouseMove = TRUE;

		CRect rcWnd;
		GetWindowRect (&rcWnd);

		rcWnd.OffsetRect (point -m_ptOldMove);
		MoveWindow (&rcWnd);
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CWndPlayer::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	m_bLButDn = TRUE;
	m_bLRButDnMouseMove = FALSE;
	m_ptOldMove = point;

	SetCapture ();

	CWnd::OnLButtonDown(nFlags, point);
}

void CWndPlayer::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_bLButDn == TRUE)
	{
		m_bLButDn = FALSE;

		if (m_bLRButDnMouseMove == FALSE)	// No Move +Click
		{
			int i;
			int numDiv, numMulPos, numMulSz, idxPos, szCellH, szCellV, nSX, nEX, nSY, nEY;

			numDiv = GL_NUM_DIV_FROM_DISP_MODE[m_idxSplitMode];
			szCellH = m_rcClient.Width () /numDiv;
			szCellV = m_rcClient.Height () /numDiv;

			for (i=0; i<GL_NUM_DISP_FROM_DISP_MODE[m_idxSplitMode]; i++)
			{
				numMulPos = GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][i];
				numMulSz = GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][i];
				idxPos = GL_IDX_POS_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][i];

				nSX = szCellH *numMulPos *(idxPos %numDiv);
				nEX = nSX +szCellH *numMulSz;
				nSY = szCellV *numMulPos *(idxPos /numDiv);
				nEY = nSY +szCellV *numMulSz;

				if (point.x >= nSX && point.x < nEX &&
					point.y >= nSY && point.y < nEY)
				{
					break;
				}
			}
			if (i < GL_NUM_DISP_FROM_DISP_MODE[m_idxSplitMode])
			{
				m_pPlayer->SetSelWnd (i);
				Draw (TRUE);
			}
		}
		else	// Mouse Move(Drag) +Click
		{
		}

		ReleaseCapture ();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CWndPlayer::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

void CWndPlayer::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	m_bLButDn = TRUE;
	m_bLRButDnMouseMove = FALSE;
	m_ptOldMove = point;

	SetCapture ();

	CWnd::OnRButtonDown(nFlags, point);
}

void CWndPlayer::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_bRButDn == TRUE)
	{
		m_bRButDn = FALSE;

		if (m_bLRButDnMouseMove == FALSE)	// No Move +Click
		{
		}
		else	// Mouse Move(Drag) +Click
		{
		}

		ReleaseCapture ();
	}

	CWnd::OnRButtonUp(nFlags, point);
}

void CWndPlayer::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnRButtonDblClk(nFlags, point);
}

void CWndPlayer::SetPlayerManager (CPlayerManager *pPlayer)
{
	m_pPlayer = pPlayer;

	glDlgCtrlPlayer.SetPlayerManager (pPlayer);
	glDlgCtrlPlayer.SetPlayerWnd (this);
}

BOOL CWndPlayer::IsSupportedCS (int idxCS)
{
	if (idxCS == PM_CS_BGRA)	// BGRA (32bit) is always supported.
	{
		return TRUE;
	}

	DWORD i;

	for (i=1; i<m_numSupportedCS; i++)
	{
		if (m_bufSupportedCS[i] == PM_FOURCC_CODE_FROM_IDX_CS[idxCS])
		{
			break;
		}
	}
	if (i < m_numSupportedCS)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CWndPlayer::SetCS (int idxCS)
{
	if (m_modeDraw == WNDPL_DRAW_NULL)
	{
		return FALSE;
	}

	if (IsSupportedCS (idxCS) == TRUE)
	{
		if (m_idxCS != idxCS)
		{
			m_idxCS = idxCS;

			DestroyOffscreenSurfaces ();
			CreateOffscreenSurfaces ();

			FillBackSurface (WNDPL_COLOR_BKGND);
		}
		return TRUE;
	}

	return FALSE;
}

void CWndPlayer::ChangeSplitMode (int idxSplitMode)
{
	m_idxSplitMode = idxSplitMode;

	ClearScreen (WNDPL_COLOR_BKGND);
}

void CWndPlayer::ClearScreen (COLORREF clrBack)
{
	FillBackSurface (clrBack);

	HRESULT ddRVal;

	EnterCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
	if (m_pDDSBack == NULL)
	{
		LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
		return;
	}
	
	ddRVal = m_pDDSPrimary->Blt (&m_rcWnd, m_pDDSBack, &m_rcClient, DDBLT_WAIT, NULL);
	if (ddRVal == DDERR_SURFACELOST)
	{
		ddRVal = m_pDDSPrimary->Restore ();
		if (ddRVal == DD_OK)
		{
			ddRVal = m_pDDSPrimary->Blt (&m_rcWnd, m_pDDSBack, &m_rcClient, DDBLT_WAIT, NULL);
		}
	}

	LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
}

BYTE *CWndPlayer::LockDecImgBuf (DWORD &nPitch)
{
	if (m_pDDSTemp == NULL)
	{
		return NULL;
	}

	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;
	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);
	
	ddRVal = m_pDDSTemp->Lock (NULL, &ddSD2, DDLOCK_WRITEONLY, NULL);

	if (ddRVal != DD_OK)
	{
		AddReport ("ERROR: m_pDDSTemp lock fail (%08x)", ddRVal);
		return NULL;
	}

	nPitch = ddSD2.lPitch;

	return (BYTE *)ddSD2.lpSurface;
}

void CWndPlayer::UnlockAndBltDecImgToBackSurface (int idxWnd, DWORD szImgH, DWORD szImgV, DWORD idxFrm, INFO_IMG_POST_FILTER *pIPF)
{
	if (m_pDDSTemp == NULL)
	{
		return;
	}

	m_pDDSTemp->Unlock (NULL);

	CRect rcSrc, rcDest;

	rcSrc.SetRect (0, 0, szImgH, szImgV);

	DWORD numDiv, numMulPos, numMulSz, idxPos, szCellH, szCellV, nSX, nEX, nSY, nEY;
		
	numDiv = GL_NUM_DIV_FROM_DISP_MODE[m_idxSplitMode];

	szCellH = m_szWnd.cx /numDiv;
	szCellV = m_szWnd.cy /numDiv;

	numMulPos = GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][idxWnd];
	numMulSz = GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][idxWnd];
	idxPos = GL_IDX_POS_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][idxWnd];

	nSX = szCellH *numMulPos *(idxPos %numDiv);
	nEX = nSX +szCellH *numMulSz;
	nSY = szCellV *numMulPos *(idxPos /numDiv);
	nEY = nSY +szCellV *numMulSz;

	rcDest.SetRect (nSX, nSY, nEX, nEY);

	EnterCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
	if (m_pDDSBack == NULL)
	{
		LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
		return;
	}

	HRESULT ddRVal;

	if (0 && pIPF->bUsePostFilter == TRUE)
	{
		// Temporary Code[S]
		DDSURFACEDESC2 ddSD2_Temp, ddSD2_PF;
		ZeroMemory (&ddSD2_Temp, sizeof(ddSD2_Temp));
		ZeroMemory (&ddSD2_PF, sizeof(ddSD2_PF));
		ddSD2_Temp.dwSize = sizeof(ddSD2_Temp);
		ddSD2_PF.dwSize = sizeof(ddSD2_PF);

		BYTE *pBufSrc, *pBufDest;
		
		if (rcDest.Width () > rcSrc.Width () *3 /2 &&
			rcDest.Height () > rcSrc.Height () *3 /2)
		{	// H x2, V x2
			ddRVal = m_pDDSTemp->Lock (NULL, &ddSD2_Temp, DDLOCK_WRITEONLY, NULL);
			ddRVal = m_pDDSForPostFlt->Lock (NULL, &ddSD2_PF, DDLOCK_WRITEONLY, NULL);

			pBufSrc = (BYTE *)ddSD2_Temp.lpSurface;
			pBufDest = (BYTE *)ddSD2_PF.lpSurface;

			m_pDDSTemp->Unlock (NULL);
			m_pDDSForPostFlt->Unlock (NULL);

			ddRVal = m_pDDSBack->Blt (&rcDest, m_pDDSForPostFlt, &rcSrc, DDBLT_WAIT, NULL);
		}
		else if (rcDest.Width () *3 /2 < rcSrc.Width () &&
			rcDest.Height () *3 /2 < rcSrc.Height ())
		{	// H /2, V /2
			ddRVal = m_pDDSTemp->Lock (NULL, &ddSD2_Temp, DDLOCK_WRITEONLY, NULL);
			ddRVal = m_pDDSForPostFlt->Lock (NULL, &ddSD2_PF, DDLOCK_WRITEONLY, NULL);

			pBufSrc = (BYTE *)ddSD2_Temp.lpSurface;
			pBufDest = (BYTE *)ddSD2_PF.lpSurface;

			m_pDDSTemp->Unlock (NULL);
			m_pDDSForPostFlt->Unlock (NULL);

			ddRVal = m_pDDSBack->Blt (&rcDest, m_pDDSForPostFlt, &rcSrc, DDBLT_WAIT, NULL);
		}
		else
		{	// No Flt
			ddRVal = m_pDDSBack->Blt (&rcDest, m_pDDSTemp, &rcSrc, DDBLT_WAIT, NULL);
		}
		// Temporary Code[S]
	}
	else
	{
		ddRVal = m_pDDSBack->Blt (&rcDest, m_pDDSTemp, &rcSrc, DDBLT_WAIT, NULL);
	}

	if (ddRVal != DD_OK)
	{
		AddReport ("ERROR: UnlockAndBltDecImgToBackSurface (): Blt fail (%08x)", ddRVal);
	}

	// draw border & info[S]
	HDC hDC;
	ddRVal = m_pDDSBack->GetDC (&hDC);

	if (ddRVal == DD_OK)
	{
		numDiv = GL_NUM_DIV_FROM_DISP_MODE[m_idxSplitMode];
		szCellH = m_rcClient.Width () /numDiv;
		szCellV = m_rcClient.Height () /numDiv;

		nSX = szCellH *numMulPos *(idxPos %numDiv);
		nEX = nSX +szCellH *numMulSz -1;
		nSY = szCellV *numMulPos *(idxPos /numDiv);
		nEY = nSY +szCellV *numMulSz -1;

		// draw border[S]
		if (idxWnd == m_pPlayer->GetSelWnd ())
		{
			SelectObject (hDC, m_hPenSelWnd);
		}
		else
		{
			SelectObject (hDC, m_hPenBorder);
		}

		MoveToEx (hDC, nSX, nSY, NULL);
		LineTo (hDC, nEX, nSY);
		LineTo (hDC, nEX, nEY);
		LineTo (hDC, nSX, nEY);
		LineTo (hDC, nSX, nSY);
		// draw border[E]

		// info display[S]
		if (m_bInfoDisp == TRUE)
		{
			RECT rcInfo;

			rcInfo.left = nSX +3;
			rcInfo.top = nSY +3;
			rcInfo.right = nEX -3;
			rcInfo.bottom = nEY -3;

			CString strInfo, strTmp;
			const ITEM_PM_VIDEO_INDEX *pConstIdx = m_pPlayer->GetVideoFileIndex ();
			pConstIdx += idxFrm;

			if (m_bInfoDisp_Ch == TRUE)
			{
				strTmp.Format ("Ch%02d  ", pConstIdx->idxCh +1);
				strInfo += strTmp;
			}

			if (m_bInfoDisp_Frame == TRUE)
			{
				strTmp.Format ("%d/ %d", pConstIdx->idxFrmInCh +1, m_pPlayer->GetVideoFrameNum (pConstIdx->idxCh));
				strInfo += strTmp;
			}

			if (strInfo.IsEmpty () == FALSE)
			{
				strInfo += "\n";
			}

			if (m_bInfoDisp_Time == TRUE)
			{
#ifdef __APP_DEBUG_MODE__
				DWORD nDiffTime = pConstIdx->timeMSec;
#else
				DWORD nDiffTime = pConstIdx->timeMSec -m_pPlayer->GetVideoStartTime ();
#endif
				strTmp.Format ("%02d:%02d:%02d.%03d",
					nDiffTime /1000 /60 /60, (nDiffTime /1000 /60) %60, (nDiffTime /1000) %60, nDiffTime %1000);
				strInfo += strTmp;
			}

			SetBkMode (hDC, TRANSPARENT);

			SetTextColor (hDC, WNDPL_COLOR_INFO_BACK);
			OffsetRect (&rcInfo, 1, 0);
			DrawText (hDC, LPCTSTR(strInfo), strInfo.GetLength (), &rcInfo, DT_TOP |DT_LEFT);
			OffsetRect (&rcInfo, 0, 1);
			DrawText (hDC, LPCTSTR(strInfo), strInfo.GetLength (), &rcInfo, DT_TOP |DT_LEFT);
			OffsetRect (&rcInfo, -1, 0);
			DrawText (hDC, LPCTSTR(strInfo), strInfo.GetLength (), &rcInfo, DT_TOP |DT_LEFT);

			SetTextColor (hDC, WNDPL_COLOR_INFO_TEXT);
			OffsetRect (&rcInfo, 0, -1);
			DrawText (hDC, LPCTSTR(strInfo), strInfo.GetLength (), &rcInfo, DT_TOP |DT_LEFT);
		}
		// info display[E]
		
		m_pDDSBack->ReleaseDC (hDC);
	}
	// draw border & info[E]

	LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
}

void CWndPlayer::Draw (BOOL bUpdateBorder)
{
	HRESULT ddRVal;

	if (m_pDDSPrimary == NULL)
	{
		return;
	}

	if (m_modeDraw == WNDPL_DRAW_OVERLAY)
	{
	}
	else if (m_modeDraw == WNDPL_DRAW_OFFSCREEN)
	{
		EnterCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
		if (m_pDDSBack == NULL)
		{
			LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
			return;
		}

		if (bUpdateBorder == TRUE)
		{	// redraw border
			HDC hDC;
			ddRVal = m_pDDSBack->GetDC (&hDC);

			if (ddRVal == DD_OK)
			{
				int i;
				DWORD numDiv, numMulPos, numMulSz, idxPos, szCellH, szCellV, nSX, nEX, nSY, nEY;

				numDiv = GL_NUM_DIV_FROM_DISP_MODE[m_idxSplitMode];
				szCellH = m_rcClient.Width () /numDiv;
				szCellV = m_rcClient.Height () /numDiv;

				SelectObject (hDC, m_hPenBorder);
				for (i=0; i<GL_NUM_DISP_FROM_DISP_MODE[m_idxSplitMode]; i++)
				{
					if (i == m_pPlayer->GetSelWnd ())
					{
						continue;
					}

					numMulPos = GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][i];
					numMulSz = GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][i];
					idxPos = GL_IDX_POS_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][i];

					nSX = szCellH *numMulPos *(idxPos %numDiv);
					nEX = nSX +szCellH *numMulSz -1;
					nSY = szCellV *numMulPos *(idxPos /numDiv);
					nEY = nSY +szCellV *numMulSz -1;

					MoveToEx (hDC, nSX, nSY, NULL);
					LineTo (hDC, nEX, nSY);
					LineTo (hDC, nEX, nEY);
					LineTo (hDC, nSX, nEY);
					LineTo (hDC, nSX, nSY);
				}
				SelectObject (hDC, m_hPenSelWnd);

				numMulPos = GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][m_pPlayer->GetSelWnd ()];
				numMulSz = GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][m_pPlayer->GetSelWnd ()];
				idxPos = GL_IDX_POS_FROM_DISP_MODE_IDX_WND[m_idxSplitMode][m_pPlayer->GetSelWnd ()];

				nSX = szCellH *numMulPos *(idxPos %numDiv);
				nEX = nSX +szCellH *numMulSz -1;
				nSY = szCellV *numMulPos *(idxPos /numDiv);
				nEY = nSY +szCellV *numMulSz -1;

				MoveToEx (hDC, nSX, nSY, NULL);
				LineTo (hDC, nEX, nSY);
				LineTo (hDC, nEX, nEY);
				LineTo (hDC, nSX, nEY);
				LineTo (hDC, nSX, nSY);
				
				m_pDDSBack->ReleaseDC (hDC);
			}
		}
		
		ddRVal = m_pDDSPrimary->Blt (&m_rcWnd, m_pDDSBack, &m_rcClient, DDBLT_WAIT, NULL);
		if (ddRVal == DDERR_SURFACELOST)
		{
			ddRVal = m_pDDSPrimary->Restore ();
			if (ddRVal == DD_OK)
			{
				ddRVal = m_pDDSPrimary->Blt (&m_rcWnd, m_pDDSBack, &m_rcClient, DDBLT_WAIT, NULL);
			}
		}

		LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
	}
}

void CWndPlayer::ChangeWndSize (int idxSize)
{
	int hRes = GetSystemMetrics (SM_CXSCREEN);
	int vRes = GetSystemMetrics (SM_CYSCREEN);
	int sizeFrmH = GetSystemMetrics (SM_CXDLGFRAME);
	int sizeFrmV = GetSystemMetrics (SM_CYDLGFRAME);
	int szWndH, szWndV, posX, posY;

	switch (idxSize)
	{
	case WNDPL_SZ_WND_DIV4:
		szWndH = glPCurGInfo->szVideo.cx /4 +sizeFrmH *2;
		szWndV = glPCurGInfo->szVideo.cy /4 +sizeFrmV *2;
		break;
	case WNDPL_SZ_WND_DIV2:
		szWndH = glPCurGInfo->szVideo.cx /2 +sizeFrmH *2;
		szWndV = glPCurGInfo->szVideo.cy /2 +sizeFrmV *2;
		break;
	case WNDPL_SZ_WND_X1:
		szWndH = glPCurGInfo->szVideo.cx +sizeFrmH *2;
		szWndV = glPCurGInfo->szVideo.cy +sizeFrmV *2;
		break;
	case WNDPL_SZ_WND_X2:
		szWndH = glPCurGInfo->szVideo.cx *2 +sizeFrmH *2;
		szWndV = glPCurGInfo->szVideo.cy *2 +sizeFrmV *2;
		break;
	case WNDPL_SZ_WND_X3:
		szWndH = glPCurGInfo->szVideo.cx *3 +sizeFrmH *2;
		szWndV = glPCurGInfo->szVideo.cy *3 +sizeFrmV *2;
		break;
	case WNDPL_SZ_WND_FULL:
		szWndH = hRes;
		szWndV = vRes;
		break;
	}

	CRect rcWindow;
	GetWindowRect (&rcWindow);

	posX = rcWindow.left +(rcWindow.Width () -szWndH) /2;
	posY = rcWindow.top +(rcWindow.Height () -szWndV) /2;

	if (szWndH > hRes)
	{
		szWndH = hRes;
	}
	if (szWndV > vRes)
	{
		szWndV = vRes;
	}
	if (posX < 0)
	{
		posX = 0;
	}
	if (posY < 0)
	{
		posY = 0;
	}

	if (idxSize == WNDPL_SZ_WND_FULL)
	{
		MoveWindow (-sizeFrmH, -sizeFrmV,
			hRes +sizeFrmH *2, vRes +sizeFrmV *2);
	}
	else
	{
		MoveWindow (posX, posY, szWndH, szWndV);
	}
}

void CWndPlayer::SetInfoDispState (BOOL BDispCh, BOOL bDispFrm, BOOL bDispTime)
{
	m_bInfoDisp_Ch = BDispCh;
	m_bInfoDisp_Frame = bDispFrm;
	m_bInfoDisp_Time = bDispTime;

	if (m_bInfoDisp_Ch == TRUE ||
		m_bInfoDisp_Frame == TRUE ||
		m_bInfoDisp_Time == TRUE)
	{
		m_bInfoDisp = TRUE;
	}
	else
	{
		m_bInfoDisp = FALSE;
	}
}

BOOL CWndPlayer::InitDDraw ()
{
	DestroyDDraw ();
	
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

	// primary surface & clipper[S]
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
	// primary surface & clipper[E]

	// colorspace check[S]
	m_numSupportedCS = 0;
	m_pDDraw->GetFourCCCodes (&m_numSupportedCS, NULL);
	m_bufSupportedCS = new DWORD[m_numSupportedCS];
	m_pDDraw->GetFourCCCodes (&m_numSupportedCS, m_bufSupportedCS);
	// colorspace check[E]

	return TRUE;
}

void CWndPlayer::DestroyDDraw ()
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

		m_pDDraw->Release ();
		m_pDDraw = NULL;
	}

	delete[] m_bufSupportedCS;
}

BOOL CWndPlayer::CreateOffscreenSurfaces ()
{
	EnterCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);

	int i;
	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;
	DDPIXELFORMAT ddPF;

	CRect rcClient;
	GetClientRect (&rcClient);

	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);
	ddSD2.dwFlags = DDSD_CAPS |DDSD_HEIGHT |DDSD_WIDTH;
	ddSD2.dwWidth = rcClient.Width ();
	ddSD2.dwHeight = rcClient.Height ();

	for (i=0; i<2; i++)
	{
		switch (i)
		{
		case 0:
			ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_VIDEOMEMORY |DDSCAPS_LOCALVIDMEM;
			break;
		case 1:
			ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_SYSTEMMEMORY;
			break;
		}

		ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_pDDSBack, NULL);
		if (ddRVal == DD_OK)
		{
			break;
		}
	}

	if (ddRVal != DD_OK)
	{
		m_pDDSBack = NULL;

		AddReport ("ERROR: m_pDDSBack creation fail (%08x)", ddRVal);
		goto ERROR_EXIT;
	}


	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);
	ddSD2.dwFlags = DDSD_CAPS |DDSD_HEIGHT |DDSD_WIDTH;

	if (m_idxCS != PM_CS_BGRA)	// colorspace set
	{
		ddSD2.dwFlags |= DDSD_PIXELFORMAT;

		ZeroMemory (&ddPF, sizeof(ddPF));

		ddPF.dwSize = sizeof(DDPIXELFORMAT);
		ddPF.dwFlags = DDPF_FOURCC;
		ddPF.dwFourCC = PM_FOURCC_CODE_FROM_IDX_CS[m_idxCS];

		ddSD2.ddpfPixelFormat = ddPF;
	}

	// Create m_pDDSTemp[S]
	ddSD2.dwWidth = MAX_SZ_REC_IMAGE_H;
	ddSD2.dwHeight = MAX_SZ_REC_IMAGE_V;

	for (i=0; i<3; i++)
	{
		switch (i)
		{
		case 0:
			ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_VIDEOMEMORY |DDSCAPS_NONLOCALVIDMEM;
			break;
		case 1:
			ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_SYSTEMMEMORY;
			break;
		case 2:
			ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_VIDEOMEMORY |DDSCAPS_LOCALVIDMEM;
			break;
		}

		ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_pDDSTemp, NULL);
		if (ddRVal == DD_OK)
		{
			break;
		}
	}

	if (ddRVal != DD_OK)
	{
		m_pDDSBack->Release ();
		m_pDDSBack = NULL;
		m_pDDSTemp = NULL;

		AddReport ("ERROR: m_pDDSTemp creation fail (%08x)", ddRVal);
		goto ERROR_EXIT;
	} 
	// Create m_pDDSTemp[E]

	// Create m_pDDSForPostFlt[E]
	ddSD2.dwWidth = MAX_SZ_REC_IMAGE_H;
	ddSD2.dwHeight = MAX_SZ_REC_IMAGE_V;

	for (i=0; i<3; i++)
	{
		switch (i)
		{
		case 0:
			ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_VIDEOMEMORY |DDSCAPS_NONLOCALVIDMEM;
			break;
		case 1:
			ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_SYSTEMMEMORY;
			break;
		case 2:
			ddSD2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |DDSCAPS_VIDEOMEMORY |DDSCAPS_LOCALVIDMEM;
			break;
		}

		ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_pDDSForPostFlt, NULL);
		if (ddRVal == DD_OK)
		{
			break;
		}
	}

	if (ddRVal != DD_OK)
	{
		m_pDDSBack->Release ();
		m_pDDSBack = NULL;
		m_pDDSTemp->Release ();
		m_pDDSTemp = NULL;
		m_pDDSForPostFlt = NULL;

		AddReport ("ERROR: m_pDDSForPostFlt creation fail (%08x)", ddRVal);
		goto ERROR_EXIT;
	} 
	// Create m_pDDSForPostFlt[E]

	LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);

	return TRUE;

ERROR_EXIT:
	LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);

	return FALSE;
}

void CWndPlayer::DestroyOffscreenSurfaces ()
{
	EnterCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);

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
	if (m_pDDSForPostFlt != NULL)
	{
		m_pDDSForPostFlt->Release ();
		m_pDDSForPostFlt = NULL;
	}

	LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
}

void CWndPlayer::FillBackSurface (COLORREF clrFill)
{
	EnterCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);

	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;
	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);

	int valR, valG, valB, valY, valU, valV;

	valR = GetRValue (clrFill);
	valG = GetGValue (clrFill);
	valB = GetBValue (clrFill);

	valY = int( 0.299 *valR +0.587 *valG +0.114 *valB);
	valU = int(-0.169 *valR -0.331 *valG +0.500 *valB +128);
	valV = int( 0.500 *valR -0.419 *valG -0.081 *valB +128);
	if (valY < 0)	valY = 0;
	if (valU < 0)	valU = 0;
	if (valV < 0)	valV = 0;
	if (valY > 0xff)	valY = 0xff;
	if (valU > 0xff)	valU = 0xff;
	if (valV > 0xff)	valV = 0xff;

	ddRVal = m_pDDSBack->Lock (NULL, &ddSD2, DDLOCK_WRITEONLY, NULL);

	int i;
	BYTE *pBackSurf = (BYTE *)ddSD2.lpSurface;

	switch (m_idxCS)
	{
	case PM_CS_BGRA:
	case PM_CS_I420:
	case PM_CS_YV12:
		for (i=0; i<ddSD2.lPitch; i+=4)
		{
			pBackSurf[i +0] = BYTE(valB);	// B
			pBackSurf[i +1] = BYTE(valG);	// G
			pBackSurf[i +2] = BYTE(valR);	// R
			pBackSurf[i +3] = 0;			// A
		}
		break;
	case PM_CS_YUY2:
		for (i=0; i<ddSD2.lPitch; i+=4)
		{
			pBackSurf[i +0] = BYTE(valY);	// Y0
			pBackSurf[i +1] = BYTE(valU);	// U0
			pBackSurf[i +2] = BYTE(valY);	// Y1
			pBackSurf[i +3] = BYTE(valV);	// V0
		}
		break;
	case PM_CS_UYVY:
		for (i=0; i<ddSD2.lPitch; i+=4)
		{
			pBackSurf[i +0] = BYTE(valU);	// Y0
			pBackSurf[i +1] = BYTE(valY);	// U0
			pBackSurf[i +2] = BYTE(valV);	// Y1
			pBackSurf[i +3] = BYTE(valY);	// V0
		}
		break;
	}

	for (i=1; i<int(ddSD2.dwHeight); i++)
	{
		memcpy (&pBackSurf[i *ddSD2.lPitch], &pBackSurf[0], ddSD2.lPitch);
	}
	
	m_pDDSBack->Unlock (NULL);

/*	ddRVal = m_pDDSTemp->Lock (NULL, &ddSD2, DDLOCK_WRITEONLY, NULL);

	if (ddRVal == DD_OK)
	{
		CRect rcSrc;

		switch (m_idxCS)
		{
		case PM_CS_BGRA:
		case PM_CS_I420:
		case PM_CS_YV12:
			((BYTE *)ddSD2.lpSurface)[0] = BYTE(valB);	// B
			((BYTE *)ddSD2.lpSurface)[1] = BYTE(valG);	// G
			((BYTE *)ddSD2.lpSurface)[2] = BYTE(valR);	// R
			((BYTE *)ddSD2.lpSurface)[3] = 0;			// A

			rcSrc.SetRect (0, 0, 1, 1);
			break;
		case PM_CS_YUY2:
			((BYTE *)ddSD2.lpSurface)[0] = BYTE(valY);	// Y0
			((BYTE *)ddSD2.lpSurface)[1] = BYTE(valU);	// U0
			((BYTE *)ddSD2.lpSurface)[2] = BYTE(valY);	// Y1
			((BYTE *)ddSD2.lpSurface)[3] = BYTE(valV);	// V0

			rcSrc.SetRect (0, 0, 2, 1);
			break;
		case PM_CS_UYVY:
			((BYTE *)ddSD2.lpSurface)[0] = BYTE(valU);	// U0
			((BYTE *)ddSD2.lpSurface)[1] = BYTE(valY);	// Y0
			((BYTE *)ddSD2.lpSurface)[2] = BYTE(valV);	// V1
			((BYTE *)ddSD2.lpSurface)[3] = BYTE(valY);	// Y0

			rcSrc.SetRect (0, 0, 2, 1);
			break;
		}

		m_pDDSTemp->Unlock (NULL);

		ddRVal = m_pDDSBack->Blt (&m_rcClient, m_pDDSTemp, &rcSrc, DDBLT_WAIT, NULL);
		if (ddRVal == DDERR_SURFACELOST)
		{
			ddRVal = m_pDDSBack->Restore ();
			if (ddRVal == DD_OK)
			{
				ddRVal = m_pDDSBack->Blt (&m_rcClient, m_pDDSTemp, &rcSrc, DDBLT_WAIT, NULL);
			}
		}
	}
	else
	{
		AddReport ("ERROR: FillBackSurface(): m_pDDSTemp lock fail (%08x)", ddRVal);
	}
*/
	LeaveCriticalSection (&m_bufCrt[WNDPL_CRT_ACC_BACK_SURFACE]);
}

void CWndPlayer::GetBltRect ()
{
	GetClientRect (&m_rcClient);

	m_rcWnd = m_rcClient;
	ClientToScreen (&m_rcWnd);

	m_szWnd.cx = m_rcWnd.Width ();
	m_szWnd.cy = m_rcWnd.Height ();
}
