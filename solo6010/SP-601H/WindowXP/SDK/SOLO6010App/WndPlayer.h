#if !defined(AFX_WNDPLAYER_H__1BA4D4A8_4E96_43F9_AD97_A47D92702E1A__INCLUDED_)
#define AFX_WNDPLAYER_H__1BA4D4A8_4E96_43F9_AD97_A47D92702E1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndPlayer.h : header file
//

#include <ddraw.h>
#include "PlayerManager.h"

enum
{
	WNDPL_DRAW_NULL,
	WNDPL_DRAW_OVERLAY,
	WNDPL_DRAW_OFFSCREEN,
	WNDPL_NUM_DRAW_MODE
};

enum
{
	WNDPL_CRT_ACC_BACK_SURFACE,
	WNDPL_NUM_CRITICAL_SECTION
};

enum
{
	WNDPL_SZ_WND_DIV4,
	WNDPL_SZ_WND_DIV2,
	WNDPL_SZ_WND_X1,
	WNDPL_SZ_WND_X2,
	WNDPL_SZ_WND_X3,
	WNDPL_SZ_WND_FULL,
	NUM_WNDPL_SZ_WND	
};

#define WNDPL_COLOR_BKGND			RGB(5, 0, 5)	// RGB(0, 0, 0) is overlapped with Intel VGA overlay color
#define WNDPL_COLOR_PEN_SEL_WND		RGB(255, 0, 0)
#define WNDPL_COLOR_PEN_BORDER		RGB(5, 0, 5)
#define WNDPL_COLOR_INFO_BACK		RGB(5, 0, 5)
#define WNDPL_COLOR_INFO_TEXT		RGB(255, 255, 255)

class CPlayerManager;

/////////////////////////////////////////////////////////////////////////////
// CWndPlayer window

class CWndPlayer : public CWnd
{
// Construction
public:
	CWndPlayer();

// Attributes
public:

// Operations
public:
	void SetPlayerManager (CPlayerManager *pPlayer);
	BOOL IsSupportedCS (int idxCS);
	BOOL SetCS (int idxCS);

	void ChangeSplitMode (int idxSplitMode);
	void ClearScreen (COLORREF clrBack);
	BYTE *LockDecImgBuf (DWORD &nPitch);
	__inline void UnlockDecImgBuf () { m_pDDSTemp->Unlock (NULL); }
	void UnlockAndBltDecImgToBackSurface (int idxWnd, DWORD szImgH, DWORD szImgV, DWORD idxFrm, INFO_IMG_POST_FILTER *pIPF);
	void Draw (BOOL bUpdateBorder = FALSE);

	void ChangeWndSize (int idxSize);
	void SetInfoDispState (BOOL BDispCh, BOOL bDispFrm, BOOL bDispTime);

private:
	// general[S]
	CPlayerManager *m_pPlayer;
	int m_idxSplitMode;
	int m_modeDraw;
	CRITICAL_SECTION m_bufCrt[WNDPL_NUM_CRITICAL_SECTION];
	// general[E]

	// colorspace[S]
	int m_idxCS;
	LPDWORD m_bufSupportedCS;
	DWORD m_numSupportedCS;
	// colorspace[E]

	// window area[S]
	CRect m_rcClient;
	CRect m_rcWnd;
	CSize m_szWnd;
	// window area[E]

	// mouse control[S]
	BOOL m_bLButDn;
	BOOL m_bRButDn;
	CPoint m_ptOldMove;
	BOOL m_bLRButDnMouseMove;
	// mouse control[E]

	// DDraw[S]
	LPDIRECTDRAW7 m_pDDraw;
	LPDIRECTDRAWCLIPPER m_pDDClipper;
	LPDIRECTDRAWSURFACE7 m_pDDSPrimary;
	LPDIRECTDRAWSURFACE7 m_pDDSBack;
	LPDIRECTDRAWSURFACE7 m_pDDSTemp;
	LPDIRECTDRAWSURFACE7 m_pDDSForPostFlt;
	// DDraw[E]

	// GDI object[S]
	HPEN m_hPenSelWnd;
	HPEN m_hPenBorder;
	HFONT m_hFontInfo;
	// GDI object[E]

	// ETC[S]
	BOOL m_bInfoDisp;
	BOOL m_bInfoDisp_Ch;
	BOOL m_bInfoDisp_Frame;
	BOOL m_bInfoDisp_Time;
	// ETC[E]

	BOOL InitDDraw ();
	void DestroyDDraw ();
	
	BOOL CreateOffscreenSurfaces ();
	void DestroyOffscreenSurfaces ();
	void FillBackSurface (COLORREF clrFill);

	void GetBltRect ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndPlayer)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWndPlayer();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWndPlayer)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDPLAYER_H__1BA4D4A8_4E96_43F9_AD97_A47D92702E1A__INCLUDED_)
