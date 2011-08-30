#if !defined(AFX_WNDLIVEDISP_H__4F8EDCBD_BF33_475F_87D3_58FE6E35B5D5__INCLUDED_)
#define AFX_WNDLIVEDISP_H__4F8EDCBD_BF33_475F_87D3_58FE6E35B5D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndLiveDisp.h : header file
//

#include <ddraw.h>
#include "afxmt.h"

#define COLOR_OVERLAY_DEST			RGB(14, 0, 14)
#define COLOR_OVERLAY_DEST_INTEL	RGB(0, 0, 0)

void CALLBACK CbFn_FlipOverlay (void *pContext, int idxBuf, int numBuf, int idxField);
void CALLBACK CbFn_SysToOverlayAndFlip (void *pContext, int idxBuf, int numBuf, int idxField);
void CALLBACK CbFn_VMotionAlarm (void *pContext, unsigned long flagCurVMotion);
void CALLBACK CbFn_CamBlkDetAlarm (void *pContext, unsigned long flagChBlockDetected);
void CALLBACK CbFn_AdvMotDetAlarm (void *pContext, unsigned long flagAdvMotDet, unsigned char *bufMotionMap);

const COLORREF GL_BUF_CLR_VAL_FROM_VM_TH_IDX[NUM_VM_TH] =
{
	RGB(20, 20, 20),	// IDX_VM_TH_0
	RGB(128, 0, 0),	// IDX_VM_TH_1
	RGB(0, 0, 128),	// IDX_VM_TH_2
	RGB(0, 128, 0),	// IDX_VM_TH_3
	RGB(180, 180, 180),	// IDX_VM_TH_4
	RGB(255, 150, 150),	// IDX_VM_TH_5
	RGB(150, 150, 255),	// IDX_VM_TH_6
	RGB(150, 255, 150),	// IDX_VM_TH_7
	RGB(255, 255, 255),	// IDX_VM_TH_8
};

#define CLR_BORDER_LIVE		RGB(0, 1, 0)
#define CLR_BORDER_HWMP4D	RGB(0, 0, 150)
#define CLR_BORDER_SEL		RGB(255, 50, 50)

#define CLR_LIVE_CH_OUTLINE		RGB(0, 1, 0)
#define CLR_LIVE_CH_TEXT		RGB(255, 255, 255)
#define CLR_HWMP4D_CH_OUTLINE	RGB(0, 1, 0)
#define CLR_HWMP4D_CH_TEXT		RGB(200, 200, 255)
#define CLR_MOTION_EVT_OUTLINE	RGB(0, 1, 0)
#define CLR_MOTION_EVT_TEXT		RGB(255, 0, 0)

#define CLR_HWDEC_CH_BACK	RGB(0, 0, 150)
#define CLR_BORDER			RGB(255, 50, 50)

/////////////////////////////////////////////////////////////////////////////
// CWndLiveDisp window

class CWndLiveDisp : public CWnd
{
// Construction
public:
	CWndLiveDisp();

// Attributes
public:
	BOOL m_bLButDn;
	BOOL m_bRButDn;
	CPoint m_ptOldMove;
	BOOL m_bLRButDnMouseMove;

	LPDIRECTDRAW7 m_pDDraw;
	LPDIRECTDRAWCLIPPER m_pDDClipper;
	LPDIRECTDRAWSURFACE7 m_pDDSPrimary;
	LPDIRECTDRAWSURFACE7 m_bufPDDSOverlay[MAX_DDRAW_SURF];
	int m_idxTmp;			// for EnumAttachedCallback() function
	int m_numCreatedOverlaySurf;
	unsigned char m_bufIdxMemPtrToOverlay[MAX_DDRAW_SURF];

	// for mode "LIVE_DISP_SYS_TO_OVERLAY_60_FI_FLIP" and "LIVE_DISP_SYS_TO_OVERLAY_30_FR_FLIP"[S]
	BYTE *m_pBufLiveImg[MAX_DDRAW_SURF];
	DWORD m_nPitchOverlay;
	DWORD m_nPitchLiveImg;
	DWORD m_szV_LiveImg;
	// for mode "LIVE_DISP_SYS_TO_OVERLAY_60_FI_FLIP" and "LIVE_DISP_SYS_TO_OVERLAY_30_FR_FLIP"[E]

	CRect m_rcOverlaySrc;
	CRect m_rcOverlayDest;
	CRect m_rcOverlayAsClient;
	DDOVERLAYFX m_ddOvFX;
	DWORD m_maskUpdateOverlay;

	CFont m_fntCaption;
	DWORD m_flagVMotion;
	DWORD m_flagCamBlkDet;
	DWORD m_flagAdvMotDet;
	BYTE m_bufAdvMotDetMap[SZ_BUF_V_MOTION_TH_IN_USHORT *NUM_LIVE_DISP_WND];
	CCriticalSection m_crtAccAdvMotDetMap;

// Operations
public:
	BOOL InitDDraw ();
	BOOL StartLiveDisplay (INFO_DDRAW_SURF *pIDDrawSurf);
	void EndLiveDisplay (BOOL bUpdateOverlay);
	__inline void RecalcOverlayDestRect ();
	void ZoomOverlayDestHSize (int nTimes);
	void UpdateOverlay ();

	void (CWndLiveDisp::*DrawBorderAndText) ();
	void DrawBT_Live ();
	void DrawBT_SettingMosaicVMotion ();

	// for mode "LIVE_DISP_SYS_TO_OVERLAY_60_FI_FLIP" and "LIVE_DISP_SYS_TO_OVERLAY_30_FR_FLIP"[S]
	void SetVariablesFromInfoDDraw (INFO_DDRAW_SURF *pIDDrawSurf);
	// for mode "LIVE_DISP_SYS_TO_OVERLAY_60_FI_FLIP" and "LIVE_DISP_SYS_TO_OVERLAY_30_FR_FLIP"[S]

private:
	BOOL CreateOverlaySurface (int szOverlayH, int szOverlayV, int numSurface);
	void ReleaseDDraw ();
	int GetWndIdxFromMousePos (CPoint point);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndLiveDisp)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWndLiveDisp();

	// Generated message map functions
protected:
	afx_msg LRESULT OnVMotionEvent (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCamBlkDetEvent (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAdvMotDetEvent (WPARAM wParam, LPARAM lParam);
	//{{AFX_MSG(CWndLiveDisp)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDLIVEDISP_H__4F8EDCBD_BF33_475F_87D3_58FE6E35B5D5__INCLUDED_)
