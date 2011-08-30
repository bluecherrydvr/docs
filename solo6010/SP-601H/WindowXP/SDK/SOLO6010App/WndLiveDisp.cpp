// WndLiveDisp.cpp : implementation file
//

#include "stdafx.h"
#include "SOLO6010App.h"
#include "WndLiveDisp.h"
#include "DlgCtrlLive.h"
#include "DlgSettingMosaicVMotion.h"
#include "SOLO6010.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgCtrlLive glDlgCtrlLive;
CDlgSettingMosaicVMotion glDlgSettingMosaicVMotion;
CDialog *pDlgCurShow;

HRESULT WINAPI EnumAttachedCallback (LPDIRECTDRAWSURFACE7 lpdds,
	LPDDSURFACEDESC2 lpddsd, LPVOID lpContext);

void CALLBACK CbFn_FlipOverlay (void *pContext, int idxBuf, int numBuf, int idxField)
{
	CWndLiveDisp *pWnd = (CWndLiveDisp *)pContext;
	int flagField, i;

	if (idxField == LIVE_BUF_FIELD_EVEN)
	{
		flagField = DDFLIP_EVEN;
	}
	else if (idxField == LIVE_BUF_FIELD_ODD)
	{
		flagField = DDFLIP_ODD;
	}
	else	// LIVE_BUF_STAT_FRAME
	{
		flagField = 0;
	}
	
	pWnd->m_bufPDDSOverlay[0]->Flip (pWnd->m_bufPDDSOverlay[pWnd->m_bufIdxMemPtrToOverlay[idxBuf]], DDFLIP_WAIT |flagField);

	for (i=0; i<numBuf; i++)
	{
		if (pWnd->m_bufIdxMemPtrToOverlay[i] == 0)
		{
			pWnd->m_bufIdxMemPtrToOverlay[i] = pWnd->m_bufIdxMemPtrToOverlay[idxBuf];
			pWnd->m_bufIdxMemPtrToOverlay[idxBuf] = 0;
			break;
		}
	}
}

void CALLBACK CbFn_SysToOverlayAndFlip (void *pContext, int idxBuf, int numBuf, int idxField)
{
	CWndLiveDisp *pWnd = (CWndLiveDisp *)pContext;
	int flagField;

	if (idxField == LIVE_BUF_FIELD_EVEN)
	{
		flagField = DDFLIP_EVEN;
	}
	else if (idxField == LIVE_BUF_FIELD_ODD)
	{
		flagField = DDFLIP_ODD;
	}
	else	// LIVE_BUF_STAT_FRAME
	{
		flagField = 0;
	}

	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;

	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);
	ddRVal = pWnd->m_bufPDDSOverlay[1]->Lock (NULL, &ddSD2, DDLOCK_SURFACEMEMORYPTR, NULL);

	if (ddRVal != DD_OK)
	{
		if (ddRVal == DDERR_SURFACELOST)
		{
			ddRVal = pWnd->m_bufPDDSOverlay[1]->Restore ();
			if (ddRVal == DD_OK)
			{
				ddRVal = pWnd->m_bufPDDSOverlay[1]->Lock (NULL, &ddSD2, DDLOCK_SURFACEMEMORYPTR, NULL);
			}
			
			if (ddRVal != DD_OK)
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	DWORD i, nAdd;
	BYTE *pSrc, *pDest;

	if (idxField == LIVE_BUF_FIELD_EVEN)
	{
		pSrc = pWnd->m_pBufLiveImg[idxBuf];
		pDest = (BYTE *)ddSD2.lpSurface;
		nAdd = pWnd->m_nPitchOverlay *2;
		for (i=0; i<pWnd->m_szV_LiveImg /16; i++)
		{
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
		}
	}
	else if (idxField == LIVE_BUF_FIELD_ODD)
	{
		pSrc = pWnd->m_pBufLiveImg[idxBuf] +pWnd->m_nPitchOverlay;
		pDest = (BYTE *)ddSD2.lpSurface +pWnd->m_nPitchOverlay;
		nAdd = pWnd->m_nPitchOverlay *2;
		for (i=0; i<pWnd->m_szV_LiveImg /16; i++)
		{
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
		}
	}
	else	// LIVE_BUF_STAT_FRAME
	{
		pSrc = pWnd->m_pBufLiveImg[idxBuf];
		pDest = (BYTE *)ddSD2.lpSurface;
		nAdd = pWnd->m_nPitchOverlay;
		for (i=0; i<pWnd->m_szV_LiveImg /8; i++)
		{
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
			FastMemcpy (pDest, pSrc, pWnd->m_nPitchLiveImg); pSrc += nAdd; pDest += nAdd;
		}
	}

	pWnd->m_bufPDDSOverlay[1]->Unlock (NULL);

	pWnd->m_bufPDDSOverlay[0]->Flip (NULL, DDFLIP_WAIT |flagField);
}

void CALLBACK CbFn_VMotionAlarm (void *pContext, unsigned long flagCurVMotion)
{
	if (pContext != NULL)
	{
		((CWnd *)pContext)->PostMessage (WM_DOO_V_MOTION_EVENT, flagCurVMotion, 0);
	}
}

void CALLBACK CbFn_CamBlkDetAlarm (void *pContext, unsigned long flagChBlockDetected)
{
	if (pContext != NULL)
	{
		((CWnd *)pContext)->PostMessage (WM_DOO_CAM_BLK_DET_ALARM, flagChBlockDetected, 0);
	}
}

void CALLBACK CbFn_AdvMotDetAlarm (void *pContext, unsigned long flagAdvMotDet, unsigned char *bufMotionMap)
{
	if (pContext != NULL)
	{
		((CWnd *)pContext)->PostMessage (WM_DOO_ADV_MOT_DET_ALARM, flagAdvMotDet, (LPARAM)bufMotionMap);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWndLiveDisp

CWndLiveDisp::CWndLiveDisp()
{
	m_bLButDn = FALSE;
	m_bRButDn = FALSE;
	m_bLRButDnMouseMove = FALSE;

	m_pDDraw = NULL;
	m_pDDClipper = NULL;
	m_pDDSPrimary = NULL;
	int i;
	for (i=0; i<MAX_DDRAW_SURF; i++)
	{
		m_bufPDDSOverlay[i] = NULL;
	}

	m_flagVMotion = 0;
	m_flagCamBlkDet = 0;
	m_flagAdvMotDet = 0;
	m_numCreatedOverlaySurf = 0;

	memset (m_bufAdvMotDetMap, 0, SZ_BUF_V_MOTION_TH_IN_USHORT *NUM_LIVE_DISP_WND);
}

CWndLiveDisp::~CWndLiveDisp()
{
}


BEGIN_MESSAGE_MAP(CWndLiveDisp, CWnd)
	ON_MESSAGE(WM_DOO_V_MOTION_EVENT, OnVMotionEvent)
	ON_MESSAGE(WM_DOO_CAM_BLK_DET_ALARM, OnCamBlkDetEvent)
	ON_MESSAGE(WM_DOO_ADV_MOT_DET_ALARM, OnAdvMotDetEvent)
	//{{AFX_MSG_MAP(CWndLiveDisp)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWndLiveDisp message handlers

int CWndLiveDisp::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	glDlgCtrlLive.Create (IDD_DLG_CTRL_LIVE, this);
	glDlgSettingMosaicVMotion.Create (IDD_DLG_SETTING_MOSAIC_VMOTION, this);
	
	CRect rcWnd, rcDlg;

	GetWindowRect (&rcWnd);
	glDlgCtrlLive.GetWindowRect (&rcDlg);

	glDlgCtrlLive.SetWindowPos (0,
		rcWnd.right,
		rcWnd.top,
		0, 0,
		SWP_NOSIZE |SWP_NOZORDER);

	m_fntCaption.CreateFont (20, 15, 0, 0, FW_BOLD, 0, 0, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
		DEFAULT_QUALITY, FIXED_PITCH, FF_DONTCARE);

	glPWndLiveDisp = this;

	return 0;
}

void CWndLiveDisp::OnDestroy() 
{
	EndLiveDisplay (TRUE);

	glPWndLiveDisp = NULL;

	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	
	ReleaseDDraw ();

	m_fntCaption.DeleteObject ();
}

void CWndLiveDisp::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	(*this.*DrawBorderAndText) ();

	// Do not call CWnd::OnPaint() for painting messages
}

#define DRAW_OVERLAPPED_TEXT(text, rect, clrText, clrBack)	srcDC.SetTextColor (clrBack);\
	rect.OffsetRect (2, 0);\
	srcDC.DrawText (text, &rect, DT_TOP |DT_LEFT);\
	rect.OffsetRect (0, 2);\
	srcDC.DrawText (text, &rect, DT_TOP |DT_LEFT);\
	rect.OffsetRect (-2, 0);\
	srcDC.DrawText (text, &rect, DT_TOP |DT_LEFT);\
	srcDC.SetTextColor (clrText);\
	rect.OffsetRect (0, -2);\
	srcDC.DrawText (text, &rect, DT_TOP |DT_LEFT)


void CWndLiveDisp::DrawBT_Live ()
{
	static volatile BOOL bDrawing = FALSE;

	if (bDrawing == TRUE)
	{
		return;
	}
	bDrawing = TRUE;

	CDC *pDC, srcDC;
	CBitmap biMain;

	pDC = GetDC ();
	biMain.CreateCompatibleBitmap (pDC, glPCurGInfo->szVideo.cx +1, glPCurGInfo->szVideo.cy +1);
	srcDC.CreateCompatibleDC (pDC);
	srcDC.SelectObject (&biMain);

	CRect rcClient, rcTmp;
	GetClientRect (&rcClient);

	srcDC.FillSolidRect (0, 0, glPCurGInfo->szVideo.cx +1, glPCurGInfo->szVideo.cy +1, glPCurGInfo->clrOverlay);

	int i, j, k, numDiv, szCellH, szCellV, idxPos, numMulPos, numMulSz;
	COLORREF clrBorder;
	CString strCaption;

	numDiv = GL_NUM_DIV_FROM_DISP_MODE[glPCurSelS6010->LIVE_GetSplitMode ()];

	szCellH = (glPCurGInfo->szVideo.cx /numDiv) /8 *8;
	szCellV = (glPCurGInfo->szVideo.cy /numDiv) /8 *8;
	
	srcDC.SetBkMode (TRANSPARENT);
	srcDC.SelectObject (&m_fntCaption);

	for (i=0; i<GL_NUM_DISP_FROM_DISP_MODE[glPCurSelS6010->LIVE_GetSplitMode ()]; i++)
	{
		if (i == glPCurGInfo->idxLiveSelWnd)
		{
			clrBorder = CLR_BORDER_SEL;
		}
		else
		{
			if (glPCurSelS6010->LIVE_IsWndLiveOrDec (i) == TRUE)
			{	// Live
				clrBorder = CLR_BORDER_LIVE;
			}
			else
			{	// HW Decoder
				clrBorder = CLR_BORDER_HWMP4D;
			}
		}

		numMulPos = GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND[glPCurSelS6010->LIVE_GetSplitMode ()][i];
		numMulSz = GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND[glPCurSelS6010->LIVE_GetSplitMode ()][i];
		idxPos = GL_IDX_POS_FROM_DISP_MODE_IDX_WND[glPCurSelS6010->LIVE_GetSplitMode ()][i];

		srcDC.Draw3dRect (szCellH *numMulPos *(idxPos %numDiv),
			szCellV *numMulPos *(idxPos /numDiv),
			szCellH *numMulSz,
			szCellV *numMulSz, clrBorder, clrBorder);

		if (glPCurGInfo->bLiveDispCh == TRUE)
		{
			rcTmp.left = szCellH *numMulPos *(idxPos %numDiv);
			rcTmp.top = szCellV *numMulPos *(idxPos /numDiv);
			rcTmp.right = rcTmp.left +szCellH *numMulSz;
			rcTmp.bottom = rcTmp.top +szCellV *numMulSz;

			if (glPCurSelS6010->LIVE_IsWndLiveOrDec (i) == TRUE)
			{	// Live
				strCaption.Format (" Ch:%d", glPCurSelS6010->LIVE_GetChFromIdxWnd (i) +1);
				DRAW_OVERLAPPED_TEXT(strCaption, rcTmp,
					CLR_LIVE_CH_TEXT, CLR_LIVE_CH_OUTLINE);

				if (((m_flagVMotion >>glPCurSelS6010->LIVE_GetChFromIdxWnd (i)) &0x01) == 0x01)
				{
					strCaption.Format ("\n MOTION ALARM");
					DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_MOTION_EVT_TEXT, CLR_MOTION_EVT_OUTLINE);
				}

				if (((m_flagCamBlkDet >>glPCurSelS6010->LIVE_GetChFromIdxWnd (i)) &0x01) == 0x01)
				{
					strCaption.Format ("\n\n CAMERA BLOCK ALARM");
					DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_MOTION_EVT_TEXT, CLR_MOTION_EVT_OUTLINE);
				}

				if (((m_flagAdvMotDet >>glPCurSelS6010->LIVE_GetChFromIdxWnd (i)) &0x01) == 0x01)
				{
					strCaption.Format ("\n\n\n ADVANCED MOTION ALARM");
					DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_MOTION_EVT_TEXT, CLR_MOTION_EVT_OUTLINE);
				}

				// Advanced Motion Detection Grid[S]
				if (glPCurGInfo->ADVMD_bShowGrid == TRUE)
				{
					int numBlkH, numBlkV, szBlkH, szBlkV;
					BYTE *pBufMotionMap;

					numBlkH = glPCurGInfo->szVideo.cx /16;
					numBlkV = glPCurGInfo->szVideo.cy /16;
					szBlkH = rcTmp.Width () /numBlkH;
					szBlkV = rcTmp.Height () /numBlkV;

					m_crtAccAdvMotDetMap.Lock ();

					for (j=0; j<numBlkV; j++)
					{
						pBufMotionMap = &m_bufAdvMotDetMap[SZ_BUF_V_MOTION_TH_IN_USHORT *glPCurSelS6010->LIVE_GetChFromIdxWnd (i) +64 *j];
						for (k=0; k<numBlkH; k++)
						{
							if (*pBufMotionMap == 0xff)
							{
								srcDC.Draw3dRect (rcTmp.left +k *szBlkH +1, rcTmp.top +j *szBlkV +1, szBlkH -2, szBlkV -2, RGB(255, 30, 30), RGB(255, 30, 30));
								srcDC.Draw3dRect (rcTmp.left +k *szBlkH +2, rcTmp.top +j *szBlkV +2, szBlkH -4, szBlkV -4, RGB(255, 30, 30), RGB(255, 30, 30));
							}
							else if (*pBufMotionMap != 0)
							{
								srcDC.Draw3dRect (rcTmp.left +k *szBlkH +1, rcTmp.top +j *szBlkV +1, szBlkH -2, szBlkV -2, RGB(100, 250, 100), RGB(100, 250, 100));
							}

							pBufMotionMap++;
						}
					}

					m_crtAccAdvMotDetMap.Unlock ();
				}
				// Advanced Motion Detection Grid[E]
			}
			else
			{	// HW Decoder
				if (glPCurSelS6010->LIVE_GetDecChFromIdxWnd (i) == INVALID_CH_IDX)
				{
					strCaption.Format (" -----");
				}
				else
				{
					strCaption.Format (" Ch:%d",
						glPCurSelS6010->LIVE_GetDecChFromIdxWnd (i) +1);
				}
				DRAW_OVERLAPPED_TEXT(strCaption, rcTmp,
					CLR_HWMP4D_CH_TEXT, CLR_HWMP4D_CH_OUTLINE);
			}
		}
	}

	pDC->SetStretchBltMode (BLACKONWHITE);
	pDC->StretchBlt (0, 0, rcClient.Width (), rcClient.Height (), &srcDC,
		0, 0, glPCurGInfo->szVideo.cx, glPCurGInfo->szVideo.cy, SRCCOPY);
	ReleaseDC (pDC);
	
	biMain.DeleteObject ();
	bDrawing = FALSE;
}

void CWndLiveDisp::DrawBT_SettingMosaicVMotion ()
{
	static volatile BOOL bDrawing = FALSE;

	if (bDrawing == TRUE)
	{
		return;
	}
	bDrawing = TRUE;

	CDC *pDC, srcDC;
	CBitmap biMain;

	pDC = GetDC ();
	biMain.CreateCompatibleBitmap (pDC, glPCurGInfo->szVideo.cx, glPCurGInfo->szVideo.cy);
	srcDC.CreateCompatibleDC (pDC);
	srcDC.SelectObject (&biMain);

	CRect rcClient, rcTmp;
	GetClientRect (&rcClient);

	srcDC.SetBkMode (TRANSPARENT);
	srcDC.SelectObject (&m_fntCaption);

	srcDC.FillSolidRect (0, 0,
		glPCurGInfo->szVideo.cx, glPCurGInfo->szVideo.cy, glPCurGInfo->clrOverlay);

	int idxCh;
	CString strCaption;

	rcTmp.left = 0;
	rcTmp.top = 0;
	rcTmp.right = glPCurGInfo->szVideo.cx;
	rcTmp.bottom = glPCurGInfo->szVideo.cy;

	if (((m_flagVMotion >>glPCurSelS6010->LIVE_GetChFromIdxWnd (0)) &0x01) == 0x01)
	{
		strCaption.Format ("\n MOTION ALARM");
		DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_MOTION_EVT_TEXT, CLR_MOTION_EVT_OUTLINE);
	}

	if (((m_flagCamBlkDet >>glPCurSelS6010->LIVE_GetChFromIdxWnd (0)) &0x01) == 0x01)
	{
		strCaption.Format ("\n\n CAMERA BLOCK ALARM");
		DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_MOTION_EVT_TEXT, CLR_MOTION_EVT_OUTLINE);
	}

	if (((m_flagAdvMotDet >>glPCurSelS6010->LIVE_GetChFromIdxWnd (0)) &0x01) == 0x01)
	{
		strCaption.Format ("\n\n\n ADVANCED MOTION ALARM");
		DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_MOTION_EVT_TEXT, CLR_MOTION_EVT_OUTLINE);
	}

	// Advanced Motion Detection Grid[S]
	if (glPCurGInfo->ADVMD_bShowGrid == TRUE)
	{
		int i, j, szBlkH, szBlkV;
		BYTE *pBufMotionMap;

		szBlkH = rcClient.Width () /16;
		szBlkV = rcClient.Height () /16;

		m_crtAccAdvMotDetMap.Lock ();

		for (i=0; i<szBlkV; i++)
		{
			pBufMotionMap = &m_bufAdvMotDetMap[SZ_BUF_V_MOTION_TH_IN_USHORT *glPCurSelS6010->LIVE_GetChFromIdxWnd (0) +64 *i];
			for (j=0; j<szBlkH; j++)
			{
				if (*pBufMotionMap == 0xff)
				{
					srcDC.Draw3dRect (j *16 +1, i *16 +1, 14, 14, RGB(255, 30, 30), RGB(255, 30, 30));
					srcDC.Draw3dRect (j *16 +2, i *16 +2, 12, 12, RGB(255, 30, 30), RGB(255, 30, 30));
				}
				else if (*pBufMotionMap != 0)
				{
					srcDC.Draw3dRect (j *16 +1, i *16 +1, 14, 14, RGB(100, 250, 100), RGB(100, 250, 100));
				}

				pBufMotionMap++;
			}
		}

		m_crtAccAdvMotDetMap.Unlock ();
	}
	// Advanced Motion Detection Grid[E]

	idxCh = glPCurSelS6010->LIVE_GetChFromIdxWnd (0);
	if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_MOSAIC)
	{	// Mosaic Region Draw
		strCaption.Format (" Ch:%d SETTING MOSAIC PROPERTY", idxCh +1);
		DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_LIVE_CH_TEXT, CLR_LIVE_CH_OUTLINE);

		if (glPCurGInfo->MOSAIC_bufBSet[idxCh] == 1)
		{
			int posX, posY, szX, szY;

			posX = glPCurGInfo->MOSAIC_bufRect[idxCh].left /8 *8 -glPCurSelS6010->GetVInOffset_H ();
			posY = glPCurGInfo->MOSAIC_bufRect[idxCh].top /8 *8;
			szX = (glPCurGInfo->MOSAIC_bufRect[idxCh].right +7) /8 *8 -glPCurSelS6010->GetVInOffset_H () -posX;
			szY = (glPCurGInfo->MOSAIC_bufRect[idxCh].bottom +7) /8 *8 -posY;

			srcDC.Draw3dRect (posX, posY, szX, szY, RGB(0, 0, 255), RGB(0, 0, 255));
		}
	}
	else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_V_MOTION)
	{	// V-Motion Region Draw
		strCaption.Format (" Ch:%d SETTING VIDEO MOTION PROPERTY", idxCh +1);
		DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_LIVE_CH_TEXT, CLR_LIVE_CH_OUTLINE);

		if (glPCurSelS6010->VMOTION_GetOnOff (idxCh) == TRUE &&
			glPCurGInfo->VM_bExecThSetting == TRUE)
		{
			int i, j, k, idxColor;
			for (i=0; i<glPCurGInfo->szVideo.cy; i+=16)
			{
				for (j=0; j<glPCurGInfo->szVideo.cx; j+=16)
				{
					idxColor = IDX_VM_TH_8;
					for (k=0; k<NUM_VM_TH; k++)
					{
						if (glPCurSelS6010->VMOTION_GetThreshold (idxCh)[(i /16 *(1024 /16)) +j /16] <= GL_VM_TH_IDX_TO_VAL[k])
						{
							idxColor = k;
							break;
						}
					}

					srcDC.Draw3dRect (j, i, 16, 16, GL_BUF_CLR_VAL_FROM_VM_TH_IDX[idxColor], GL_BUF_CLR_VAL_FROM_VM_TH_IDX[idxColor]);
				}
			}

			srcDC.Draw3dRect (glPCurGInfo->VM_rcCurSelTh.left /16 *16,
				glPCurGInfo->VM_rcCurSelTh.top /16 *16,
				(glPCurGInfo->VM_rcCurSelTh.right -(glPCurGInfo->VM_rcCurSelTh.left /16 *16) +15) /16 *16,
				(glPCurGInfo->VM_rcCurSelTh.bottom -(glPCurGInfo->VM_rcCurSelTh.top /16 *16) +15) /16 *16,
				RGB(0, 0, 255),
				RGB(0, 0, 255));
		}
	}
	else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_ADV_MOT_DET)
	{	// V-Motion Region Draw
		strCaption.Format (" Ch:%d SETTING ADVANCED MOTION DETECTION PROPERTY", idxCh +1);
		DRAW_OVERLAPPED_TEXT(strCaption, rcTmp, CLR_LIVE_CH_TEXT, CLR_LIVE_CH_OUTLINE);

		if (glPCurGInfo->ADVMD_bufBUse[idxCh] == TRUE && glPCurGInfo->ADVMD_bExecThSetting == TRUE)
		{
			int i, j, k, idxColor;
			for (i=0; i<glPCurGInfo->szVideo.cy; i+=16)
			{
				for (j=0; j<glPCurGInfo->szVideo.cx; j+=16)
				{
					idxColor = IDX_VM_TH_8;
					for (k=0; k<NUM_VM_TH; k++)
					{
						if (glPCurGInfo->ADVMD_bbbufMotionTh[idxCh][glPCurGInfo->ADVMD_idxCurSelCheckItem][(i /16 *(1024 /16)) +j /16]
							<= GL_VM_TH_IDX_TO_VAL[k])
						{
							idxColor = k;
							break;
						}
					}

					srcDC.Draw3dRect (j, i, 16, 16,
						GL_BUF_CLR_VAL_FROM_VM_TH_IDX[idxColor],
						GL_BUF_CLR_VAL_FROM_VM_TH_IDX[idxColor]);
				}
			}

			srcDC.Draw3dRect (glPCurGInfo->ADVMD_rcCurSelTh.left /16 *16,
				glPCurGInfo->ADVMD_rcCurSelTh.top /16 *16,
				(glPCurGInfo->ADVMD_rcCurSelTh.right
				-(glPCurGInfo->ADVMD_rcCurSelTh.left /16 *16) +15) /16 *16,
				(glPCurGInfo->ADVMD_rcCurSelTh.bottom 
				-(glPCurGInfo->ADVMD_rcCurSelTh.top /16 *16) +15) /16 *16,
				RGB(0, 0, 255),
				RGB(0, 0, 255));
		}
	}

	pDC->BitBlt (0, 0, rcClient.Width (), rcClient.Height (), &srcDC,
		0, 0, SRCCOPY);
	ReleaseDC (pDC);
	
	biMain.DeleteObject ();
	bDrawing = FALSE;
}

void CWndLiveDisp::SetVariablesFromInfoDDraw (INFO_DDRAW_SURF *pIDDrawSurf)
{
	int i;
	for (i=0; i<pIDDrawSurf->numSurf; i++)
	{
		m_pBufLiveImg[i] = (BYTE *)pIDDrawSurf->lpSurf[i];
	}

	m_nPitchOverlay = pIDDrawSurf->nPitch;
	m_nPitchLiveImg = pIDDrawSurf->szH *2;
	m_szV_LiveImg = pIDDrawSurf->szV;
}

void CWndLiveDisp::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	m_bLButDn = TRUE;
	m_bLRButDnMouseMove = FALSE;
	m_ptOldMove = point;

	SetCapture ();

	if (glPCurGInfo->bSetMosaicVM == TRUE)
	{
		if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_V_MOTION)
		{	// V-Motion Threshold Cell Update Start
			glPCurGInfo->VM_rcCurSelTh.left = (unsigned short)point.x;
			glPCurGInfo->VM_rcCurSelTh.right = (unsigned short)point.x;
			glPCurGInfo->VM_rcCurSelTh.top = (unsigned short)point.y;
			glPCurGInfo->VM_rcCurSelTh.bottom = (unsigned short)point.y;
		}
		else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_ADV_MOT_DET)
		{
			glPCurGInfo->ADVMD_rcCurSelTh.left = (unsigned short)point.x;
			glPCurGInfo->ADVMD_rcCurSelTh.right = (unsigned short)point.x;
			glPCurGInfo->ADVMD_rcCurSelTh.top = (unsigned short)point.y;
			glPCurGInfo->ADVMD_rcCurSelTh.bottom = (unsigned short)point.y;
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CWndLiveDisp::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	if (m_bLButDn == TRUE)
	{
		m_bLButDn = FALSE;

		if (m_bLRButDnMouseMove == FALSE)	// No Move +Click
		{
			if (glPCurGInfo->bSetMosaicVM == TRUE)
			{
				pDlgCurShow->SendMessage (WM_DOO_LIVE_DISP_WND_CLICK,
					0, 0);
			}
			else
			{	// Select Channel Change
				glPCurGInfo->idxLiveSelWnd = GetWndIdxFromMousePos (point);

				(*this.*DrawBorderAndText) ();
				pDlgCurShow->SendMessage (WM_DOO_LIVE_DISP_WND_CLICK,
					glPCurGInfo->idxLiveSelWnd, 0);
			}
		}
		else	// Mouse Move(Drag) +Click
		{
			if (glPCurGInfo->bSetMosaicVM == TRUE)
			{	// Mosaic & Vmotion Setting Update
				pDlgCurShow->SendMessage (WM_DOO_LIVE_DISP_WND_CLICK,
					0, 0);
			}
			else
			{
			}
		}

		ReleaseCapture ();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CWndLiveDisp::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	pDlgCurShow->ShowWindow (SW_SHOW);
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

void CWndLiveDisp::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	m_bLButDn = TRUE;
	m_bLRButDnMouseMove = FALSE;
	m_ptOldMove = point;

	SetCapture ();

	CWnd::OnRButtonDown(nFlags, point);
}

void CWndLiveDisp::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_bRButDn == TRUE)
	{
		m_bRButDn = FALSE;

		if (m_bLRButDnMouseMove == FALSE)	// No Move +Click
		{
			if (glPCurGInfo->bSetMosaicVM == TRUE)
			{
			}
			else
			{
			}
		}
		else	// Mouse Move(Drag) +Click
		{
			if (glPCurGInfo->bSetMosaicVM == TRUE)
			{
			}
			else
			{
			}
		}

		ReleaseCapture ();
	}

	CWnd::OnRButtonUp(nFlags, point);
}

void CWndLiveDisp::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_bLButDn == TRUE)
	{
		m_bLRButDnMouseMove = TRUE;

		if (glPCurGInfo->bSetMosaicVM == TRUE)
		{
			if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_MOSAIC)
			{
				int idxCh = glPCurSelS6010->LIVE_GetChFromIdxWnd (0);
				if (glPCurGInfo->MOSAIC_bufBSet[idxCh] == 1)
				{
					CPoint ptTmp = point;
					CRect rcClient;
					GetClientRect (&rcClient);

					if (ptTmp.x < 0)
					{
						ptTmp.x = 0;
					}
					if (ptTmp.x > rcClient.right)
					{
						ptTmp.x = rcClient.right;
					}
					if (ptTmp.y < 0)
					{
						ptTmp.y = 0;
					}
					if (ptTmp.y > rcClient.bottom)
					{
						ptTmp.y = rcClient.bottom;
					}
					if (m_ptOldMove.x < ptTmp.x)
					{
						glPCurGInfo->MOSAIC_bufRect[idxCh].left = (unsigned short)m_ptOldMove.x;
						glPCurGInfo->MOSAIC_bufRect[idxCh].right = (unsigned short)ptTmp.x;
					}
					else
					{
						glPCurGInfo->MOSAIC_bufRect[idxCh].left = (unsigned short)ptTmp.x;
						glPCurGInfo->MOSAIC_bufRect[idxCh].right = (unsigned short)m_ptOldMove.x;
					}
					if (m_ptOldMove.y < ptTmp.y)
					{
						glPCurGInfo->MOSAIC_bufRect[idxCh].top = (unsigned short)m_ptOldMove.y;
						glPCurGInfo->MOSAIC_bufRect[idxCh].bottom = (unsigned short)ptTmp.y;
					}
					else
					{
						glPCurGInfo->MOSAIC_bufRect[idxCh].top = (unsigned short)ptTmp.y;
						glPCurGInfo->MOSAIC_bufRect[idxCh].bottom = (unsigned short)m_ptOldMove.y;
					}
				}
			}
			else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_V_MOTION)
			{
				int idxCh = glPCurSelS6010->LIVE_GetChFromIdxWnd (0);
				if (glPCurSelS6010->VMOTION_GetOnOff (idxCh) == TRUE)
				{
					CPoint ptTmp = point;
					CRect rcClient;
					GetClientRect (&rcClient);

					if (ptTmp.x < 0)
					{
						ptTmp.x = 0;
					}
					if (ptTmp.x > rcClient.right)
					{
						ptTmp.x = rcClient.right;
					}
					if (ptTmp.y < 0)
					{
						ptTmp.y = 0;
					}
					if (ptTmp.y > rcClient.bottom)
					{
						ptTmp.y = rcClient.bottom;
					}
					if (m_ptOldMove.x < ptTmp.x)
					{
						glPCurGInfo->VM_rcCurSelTh.left = (unsigned short)m_ptOldMove.x;
						glPCurGInfo->VM_rcCurSelTh.right = (unsigned short)ptTmp.x;
					}
					else
					{
						glPCurGInfo->VM_rcCurSelTh.left = (unsigned short)ptTmp.x;
						glPCurGInfo->VM_rcCurSelTh.right = (unsigned short)m_ptOldMove.x;
					}
					if (m_ptOldMove.y < ptTmp.y)
					{
						glPCurGInfo->VM_rcCurSelTh.top = (unsigned short)m_ptOldMove.y;
						glPCurGInfo->VM_rcCurSelTh.bottom = (unsigned short)ptTmp.y;
					}
					else
					{
						glPCurGInfo->VM_rcCurSelTh.top = (unsigned short)ptTmp.y;
						glPCurGInfo->VM_rcCurSelTh.bottom = (unsigned short)m_ptOldMove.y;
					}
				}
			}
			else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_ADV_MOT_DET)
			{
				int idxCh = glPCurSelS6010->LIVE_GetChFromIdxWnd (0);
				if (glPCurGInfo->ADVMD_bufBUse[idxCh] == TRUE)
				{
					CPoint ptTmp = point;
					CRect rcClient;
					GetClientRect (&rcClient);

					if (ptTmp.x < 0)
					{
						ptTmp.x = 0;
					}
					if (ptTmp.x > rcClient.right)
					{
						ptTmp.x = rcClient.right;
					}
					if (ptTmp.y < 0)
					{
						ptTmp.y = 0;
					}
					if (ptTmp.y > rcClient.bottom)
					{
						ptTmp.y = rcClient.bottom;
					}
					if (m_ptOldMove.x < ptTmp.x)
					{
						glPCurGInfo->ADVMD_rcCurSelTh.left = (unsigned short)m_ptOldMove.x;
						glPCurGInfo->ADVMD_rcCurSelTh.right = (unsigned short)ptTmp.x;
					}
					else
					{
						glPCurGInfo->ADVMD_rcCurSelTh.left = (unsigned short)ptTmp.x;
						glPCurGInfo->ADVMD_rcCurSelTh.right = (unsigned short)m_ptOldMove.x;
					}
					if (m_ptOldMove.y < ptTmp.y)
					{
						glPCurGInfo->ADVMD_rcCurSelTh.top = (unsigned short)m_ptOldMove.y;
						glPCurGInfo->ADVMD_rcCurSelTh.bottom = (unsigned short)ptTmp.y;
					}
					else
					{
						glPCurGInfo->ADVMD_rcCurSelTh.top = (unsigned short)ptTmp.y;
						glPCurGInfo->ADVMD_rcCurSelTh.bottom = (unsigned short)m_ptOldMove.y;
					}
				}
			}
			
			(*this.*DrawBorderAndText) ();
		}
		else
		{
			CRect rcWnd;
			GetWindowRect (&rcWnd);

			rcWnd.OffsetRect (point -m_ptOldMove);
			MoveWindow (&rcWnd);

			RecalcOverlayDestRect ();
			(*this.*DrawBorderAndText) ();
			UpdateOverlay ();
		}
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

BOOL CWndLiveDisp::InitDDraw ()
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

	return TRUE;
}

HRESULT WINAPI EnumAttachedCallback (LPDIRECTDRAWSURFACE7 lpdds,
	LPDDSURFACEDESC2 lpddsd, LPVOID lpContext)
{
	CWndLiveDisp *pWnd = (CWndLiveDisp *)lpContext;

	if (lpdds == pWnd->m_bufPDDSOverlay[0])
	{
        goto EXIT_ENUM_FUNC;
	}
	pWnd->m_bufPDDSOverlay[pWnd->m_idxTmp++] = lpdds;
	
    lpdds->EnumAttachedSurfaces (pWnd, EnumAttachedCallback);    

EXIT_ENUM_FUNC:
    lpdds->Release ();
    return DDENUMRET_OK;
}

BOOL CWndLiveDisp::CreateOverlaySurface (int szOverlayH, int szOverlayV, int numSurface)
{
	if (numSurface > MAX_DDRAW_SURF)
	{
		return FALSE;
	}
	
	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;

	ZeroMemory (&ddSD2, sizeof(ddSD2));
	ddSD2.dwSize = sizeof(ddSD2);

	ddSD2.dwWidth = szOverlayH;
	ddSD2.dwHeight = szOverlayV;
	if (numSurface == 1)
	{
		ddSD2.dwFlags = DDSD_CAPS |DDSD_HEIGHT |DDSD_WIDTH
			|DDSD_PIXELFORMAT;
		ddSD2.ddsCaps.dwCaps = DDSCAPS_OVERLAY |DDSCAPS_VIDEOMEMORY;
	}
	else
	{
		ddSD2.dwFlags = DDSD_CAPS |DDSD_HEIGHT |DDSD_WIDTH
			|DDSD_PIXELFORMAT |DDSD_BACKBUFFERCOUNT;
		ddSD2.ddsCaps.dwCaps = DDSCAPS_OVERLAY |DDSCAPS_VIDEOMEMORY
			|DDSCAPS_FLIP |DDSCAPS_COMPLEX;
		ddSD2.dwBackBufferCount = numSurface -1;
	}

	DDPIXELFORMAT ddPF;
	ZeroMemory (&ddPF, sizeof(ddPF));
	ddPF.dwSize = sizeof(DDPIXELFORMAT);
	ddPF.dwFlags = DDPF_FOURCC;
	ddPF.dwFourCC = MAKEFOURCC('U','Y','V','Y');

	ddSD2.ddpfPixelFormat = ddPF;
	ddRVal = m_pDDraw->CreateSurface (&ddSD2, &m_bufPDDSOverlay[0], NULL);
	if (ddRVal != DD_OK)
	{
		return FALSE;
	}

	m_idxTmp = 1;
    ddRVal = m_bufPDDSOverlay[0]->EnumAttachedSurfaces (this, EnumAttachedCallback);
    if (ddRVal != DD_OK)
	{
        return FALSE;
	}

	return TRUE;
}

void CWndLiveDisp::ReleaseDDraw ()
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
}

void CWndLiveDisp::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	if (glPCurGInfo->bOnLive == TRUE)
	{
		RecalcOverlayDestRect ();
		(*this.*DrawBorderAndText) ();
		UpdateOverlay ();
	}
}

BOOL CWndLiveDisp::StartLiveDisplay (INFO_DDRAW_SURF *pIDDrawSurf)
{
	HRESULT ddRVal;
	DDSURFACEDESC2 ddSD2;
	BYTE *pSurf;
	int i, j;

	// Set live overlay display mode[S]
	switch (glPCurGInfo->modeLiveDisp)
	{
	case LIVE_DISP_1_OVERLAY_60_FI:
		pIDDrawSurf->numSurf = 1;
		pIDDrawSurf->bField = TRUE;
		pIDDrawSurf->bFlip = FALSE;
		pIDDrawSurf->bUse2StepCopyMode = FALSE;
		pIDDrawSurf->bUse1FieldInFrmMode = FALSE;
		break;
	case LIVE_DISP_1_OVERLAY_30_FR:
		pIDDrawSurf->numSurf = 1;
		pIDDrawSurf->bField = FALSE;
		pIDDrawSurf->bFlip = FALSE;
		pIDDrawSurf->bUse2StepCopyMode = FALSE;
		pIDDrawSurf->bUse1FieldInFrmMode = FALSE;
		break;
	case LIVE_DISP_N_OVERLAY_60_FI_FLIP:
		pIDDrawSurf->numSurf = glPCurGInfo->numOverSurf;
		pIDDrawSurf->bField = TRUE;
		pIDDrawSurf->bFlip = TRUE;
		pIDDrawSurf->bUse2StepCopyMode = FALSE;
		pIDDrawSurf->bUse1FieldInFrmMode = FALSE;
		break;
	case LIVE_DISP_N_OVERLAY_30_FR_FLIP:
		pIDDrawSurf->numSurf = glPCurGInfo->numOverSurf;
		pIDDrawSurf->bField = FALSE;
		pIDDrawSurf->bFlip = TRUE;
		pIDDrawSurf->bUse2StepCopyMode = FALSE;
		pIDDrawSurf->bUse1FieldInFrmMode = FALSE;
		break;
	case LIVE_DISP_SYS_TO_OVERLAY_60_FI_FLIP:
		pIDDrawSurf->numSurf = 2;
		pIDDrawSurf->bField = TRUE;
		pIDDrawSurf->bFlip = TRUE;
		pIDDrawSurf->bUse2StepCopyMode = TRUE;
		pIDDrawSurf->bUse1FieldInFrmMode = FALSE;
		break;
	case LIVE_DISP_SYS_TO_OVERLAY_30_FR_FLIP:
		pIDDrawSurf->numSurf = 2;
		pIDDrawSurf->bField = FALSE;
		pIDDrawSurf->bFlip = TRUE;
		pIDDrawSurf->bUse2StepCopyMode = TRUE;
		pIDDrawSurf->bUse1FieldInFrmMode = FALSE;
		break;
	case LIVE_DISP_N_OVERLAY_30_FI_FLIP:
		pIDDrawSurf->numSurf = glPCurGInfo->numOverSurf;
		pIDDrawSurf->bField = FALSE;
		pIDDrawSurf->bFlip = TRUE;
		pIDDrawSurf->bUse2StepCopyMode = FALSE;
		pIDDrawSurf->bUse1FieldInFrmMode = TRUE;
		break;
	}
	// Set live overlay display mode[E]

	// Overlay surface create[S]
	if (CreateOverlaySurface (SZ_OVERLAY_WIDTH, glPCurGInfo->szVideo.cy, pIDDrawSurf->numSurf) == FALSE)
	{
		AddReport ("Overlay Surf Create Fail");
		return FALSE;
	}
	// Overlay surface create[E]

	// Set UpdateOverlay() property[S]
	ZeroMemory (&m_ddOvFX, sizeof(m_ddOvFX));
	m_ddOvFX.dwSize = sizeof(m_ddOvFX);
	m_ddOvFX.dckDestColorkey.dwColorSpaceLowValue = glPCurGInfo->clrOverlay;
	m_ddOvFX.dckDestColorkey.dwColorSpaceHighValue = 0;
	m_ddOvFX.dwDDFX = DDOVERFX_ARITHSTRETCHY |DDOVERFX_DEINTERLACE;

	m_maskUpdateOverlay = DDOVER_SHOW |DDOVER_DDFX |DDOVER_KEYDESTOVERRIDE;
	if (pIDDrawSurf->bFlip == 0 || pIDDrawSurf->bField == 1)
	{
		m_maskUpdateOverlay |= DDOVER_INTERLEAVED;
	}
	if (glPCurGInfo->modeDeinterlacing == LIVE_DI_BOB && glPCurGInfo->modeLiveDisp != LIVE_DISP_N_OVERLAY_30_FI_FLIP)
	{
		m_maskUpdateOverlay |= DDOVER_BOB;
	}

	// Make overlay source rect[S]
	m_rcOverlaySrc.left = 0;
	m_rcOverlaySrc.top = 0;
	m_rcOverlaySrc.right = m_rcOverlaySrc.left +glPCurGInfo->szVideo.cx;

	if (glPCurGInfo->modeLiveDisp == LIVE_DISP_N_OVERLAY_30_FI_FLIP)
	{
		m_rcOverlaySrc.bottom = m_rcOverlaySrc.top +glPCurGInfo->szVideo.cy /2;
	}
	else
	{
		m_rcOverlaySrc.bottom = m_rcOverlaySrc.top +glPCurGInfo->szVideo.cy;
	}
	// Make overlay source rect[E]

	RecalcOverlayDestRect ();
	// Set UpdateOverlay() property[E]

	// Get overlay surfaces pointer & make them black[S]
	m_numCreatedOverlaySurf = pIDDrawSurf->numSurf;

	for (i=0; i<pIDDrawSurf->numSurf; i++)
	{
		ddRVal = m_bufPDDSOverlay[i]->PageLock (0);
		if (ddRVal != DD_OK)
		{
			AddReport ("%dth Overlay Surf PageLock() Fail");

			for (j=0; j<i; j++)
			{
				m_bufPDDSOverlay[j]->PageUnlock (0);
			}
			return FALSE;
		}
		
		ZeroMemory (&ddSD2, sizeof(ddSD2));
		ddSD2.dwSize = sizeof(ddSD2);
		ddRVal = m_bufPDDSOverlay[i]->Lock (NULL, &ddSD2, DDLOCK_SURFACEMEMORYPTR, NULL);
		if (ddRVal == DDERR_SURFACELOST)
		{
			ddRVal = m_bufPDDSOverlay[i]->Restore ();
			if (ddRVal == DD_OK)
			{
				ddRVal = m_bufPDDSOverlay[i]->Lock (NULL, &ddSD2, DDLOCK_SURFACEMEMORYPTR, NULL);
			}
		}

		if (ddRVal != DD_OK)
		{
			return FALSE;
		}

		pIDDrawSurf->lpSurf[i] = ddSD2.lpSurface;
		m_bufIdxMemPtrToOverlay[i] = i;	// Overlay Memory Index -> Overlay Surface Index
		if (i == 0)
		{
			pIDDrawSurf->szH = (unsigned short)ddSD2.dwWidth;
			pIDDrawSurf->szV = (unsigned short)ddSD2.dwHeight;
			pIDDrawSurf->nPitch = ddSD2.lPitch;
		}

		pSurf = (BYTE *)ddSD2.lpSurface;
		for (j=0; j<pIDDrawSurf->nPitch; j+=4)
		{
			pSurf[j +0] = 0x7f;
			pSurf[j +1] = 0x00;
			pSurf[j +2] = 0x7f;
			pSurf[j +3] = 0x00;
		}
		for (j=1; j<glPCurGInfo->szVideo.cy; j++)
		{
			memcpy (&pSurf[j *pIDDrawSurf->nPitch], pSurf, pIDDrawSurf->nPitch);
		}
		m_bufPDDSOverlay[i]->Unlock (NULL);
	}
	// Get overlay surfaces pointer & make them black[E]

	(*this.*DrawBorderAndText) ();
	UpdateOverlay ();

	return TRUE;
}

void CWndLiveDisp::EndLiveDisplay (BOOL bUpdateOverlay)
{
	HRESULT ddRVal;

	if (m_bufPDDSOverlay[0] == NULL)
	{
		return;
	}
	
	int i;

	for (i=0; i<m_numCreatedOverlaySurf; i++)
	{
//		m_bufPDDSOverlay[i]->Unlock (NULL);
	}

	if (bUpdateOverlay == TRUE)
	{
		ddRVal = m_bufPDDSOverlay[0]->UpdateOverlay (NULL, m_pDDSPrimary, NULL, DDOVER_HIDE, NULL);
	}

	for (i=0; i<m_numCreatedOverlaySurf; i++)
	{
		m_bufPDDSOverlay[i]->PageUnlock (0);
	}

	m_bufPDDSOverlay[0]->Release ();
	m_bufPDDSOverlay[0] = NULL;
}

__inline void CWndLiveDisp::RecalcOverlayDestRect ()
{
	CRect rcWnd;

	GetClientRect (&rcWnd);
	ClientToScreen (&rcWnd);

	memcpy (&m_rcOverlayDest, &rcWnd, sizeof(CRect));
}

void CWndLiveDisp::ZoomOverlayDestHSize (int nTimes)
{
	static int oldTimes = 1;

	if (oldTimes == nTimes)
	{
		return;
	}

	oldTimes = nTimes;

	m_rcOverlaySrc.left = 0;
	m_rcOverlaySrc.top = 0;

	switch (nTimes)
	{
	default:
	case 1:
		m_rcOverlaySrc.right = m_rcOverlaySrc.left +glPCurGInfo->szVideo.cx;
		break;
	case 2:
		m_rcOverlaySrc.right = m_rcOverlaySrc.left +glPCurGInfo->szVideo.cx /2;
		break;
	}
	m_rcOverlaySrc.bottom = m_rcOverlaySrc.top +glPCurGInfo->szVideo.cy;

	UpdateOverlay ();
}

void CWndLiveDisp::UpdateOverlay ()
{
	if (m_bufPDDSOverlay[0] != NULL)
	{
		m_bufPDDSOverlay[0]->UpdateOverlay (&m_rcOverlaySrc, m_pDDSPrimary,
			&m_rcOverlayDest, m_maskUpdateOverlay,
			&m_ddOvFX);
	}
}

void CWndLiveDisp::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
	if (glPCurGInfo->bSetMosaicVM == TRUE)
	{
		ModifyStyle (WS_THICKFRAME, WS_DLGFRAME);

		int hRes = GetSystemMetrics (SM_CXSCREEN);
		int vRes = GetSystemMetrics (SM_CYSCREEN);
		int sizeFrmH = GetSystemMetrics (SM_CXDLGFRAME);
		int sizeFrmV = GetSystemMetrics (SM_CYDLGFRAME);
		int szWndH = glPCurGInfo->szVideo.cx +sizeFrmH *2;
		int szWndV = glPCurGInfo->szVideo.cy +sizeFrmV *2;

		CRect rcDlg;
		glDlgSettingMosaicVMotion.GetWindowRect (&rcDlg);

		MoveWindow ((hRes -szWndH) /2, (vRes -(szWndV +rcDlg.Height ())) /2,
			szWndH, szWndV);

		glDlgSettingMosaicVMotion.SetWindowPos (0,
			(hRes -rcDlg.Width ()) /2,
			(vRes -(szWndV +rcDlg.Height ())) /2 +szWndV,
			0, 0,
			SWP_NOSIZE |SWP_NOZORDER);

		DrawBorderAndText = &CWndLiveDisp::DrawBT_SettingMosaicVMotion;
		pDlgCurShow = &glDlgSettingMosaicVMotion;
	}
	else
	{
		ModifyStyle (WS_DLGFRAME, WS_THICKFRAME);
		
		DrawBorderAndText = &CWndLiveDisp::DrawBT_Live;
		pDlgCurShow = &glDlgCtrlLive;

		RecalcOverlayDestRect ();
		(*this.*DrawBorderAndText) ();
		UpdateOverlay ();
	}
	
	if (bShow == TRUE)
	{
		pDlgCurShow->ShowWindow (SW_SHOW);
	}
	else
	{
		pDlgCurShow->ShowWindow (SW_HIDE);
	}
}

BOOL CWndLiveDisp::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return TRUE;
	return CWnd::OnEraseBkgnd(pDC);
}

int CWndLiveDisp::GetWndIdxFromMousePos (CPoint point)
{
	CRect rcClient;

	GetClientRect (&rcClient);

	point.x = point.x *glPCurGInfo->szVideo.cx /rcClient.Width ();
	point.y = point.y *glPCurGInfo->szVideo.cy /rcClient.Height ();

	int i, numDiv, numMulPos, numMulSz, idxPos, szCellH, szCellV, nSX, nEX, nSY, nEY;

	numDiv = GL_NUM_DIV_FROM_DISP_MODE[glPCurSelS6010->LIVE_GetSplitMode ()];

	szCellH = (glPCurGInfo->szVideo.cx /numDiv) /8 *8;
	szCellV = (glPCurGInfo->szVideo.cy /numDiv) /8 *8;

	for (i=0; i<GL_NUM_DISP_FROM_DISP_MODE[glPCurSelS6010->LIVE_GetSplitMode ()]; i++)
	{
		numMulPos = GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND[glPCurSelS6010->LIVE_GetSplitMode ()][i];
		numMulSz = GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND[glPCurSelS6010->LIVE_GetSplitMode ()][i];
		idxPos = GL_IDX_POS_FROM_DISP_MODE_IDX_WND[glPCurSelS6010->LIVE_GetSplitMode ()][i];

		nSX = szCellH *numMulPos *(idxPos %numDiv);
		nEX = nSX +szCellH *numMulSz;
		nSY = szCellV *numMulPos *(idxPos /numDiv);
		nEY = nSY +szCellV *numMulSz;

		if (point.x >= nSX && point.x < nEX &&
			point.y >= nSY && point.y < nEY)
		{
			return i;
		}
	}

	return 0;
}

LRESULT CWndLiveDisp::OnVMotionEvent (WPARAM wParam, LPARAM lParam)
{
	static WPARAM nflagOld = 0;
	
//	AddReport ("%0d, %08x (%d, %08x)", wParam, lParam, nStatOld, nMaskChOld);
	if (nflagOld != wParam)
	{
		m_flagVMotion = wParam;

		Invalidate (FALSE);
	}
	nflagOld = wParam;

	return 0;
}

LRESULT CWndLiveDisp::OnCamBlkDetEvent (WPARAM wParam, LPARAM lParam)
{
	static WPARAM nflagOld = 0;
	
//	AddReport ("%0d, %08x (%d, %08x)", wParam, lParam, nStatOld, nMaskChOld);
	if (nflagOld != wParam)
	{
		m_flagCamBlkDet = wParam;

		Invalidate (FALSE);
	}
	nflagOld = wParam;

	return 0;
}

LRESULT CWndLiveDisp::OnAdvMotDetEvent (WPARAM wParam, LPARAM lParam)
{
	static WPARAM nflagOld = 0;
	BOOL bUpdateWnd = FALSE;
	
	if (glPCurGInfo->ADVMD_bShowGrid == TRUE && lParam != 0)
	{
		m_crtAccAdvMotDetMap.Lock ();
		memcpy (m_bufAdvMotDetMap, (BYTE *)lParam, SZ_BUF_V_MOTION_TH_IN_USHORT *NUM_LIVE_DISP_WND);
		m_crtAccAdvMotDetMap.Unlock ();

		bUpdateWnd = TRUE;
	}
	if (nflagOld != wParam)
	{
		m_flagAdvMotDet = wParam;

		if (bUpdateWnd == FALSE)
		{
			m_crtAccAdvMotDetMap.Lock ();
			memcpy (m_bufAdvMotDetMap, (BYTE *)lParam, SZ_BUF_V_MOTION_TH_IN_USHORT *NUM_LIVE_DISP_WND);
			m_crtAccAdvMotDetMap.Unlock ();
		}

		bUpdateWnd = TRUE;
	}
	nflagOld = wParam;

	if (bUpdateWnd == TRUE)
	{
		Invalidate (FALSE);
	}

	return 0;
}
