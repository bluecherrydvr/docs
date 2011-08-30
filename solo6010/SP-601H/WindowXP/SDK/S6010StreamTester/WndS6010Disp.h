#if !defined(AFX_WNDS6010DISP_H__4D61B022_6F58_4F77_AA65_5B1C8896CA55__INCLUDED_)
#define AFX_WNDS6010DISP_H__4D61B022_6F58_4F77_AA65_5B1C8896CA55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndS6010Disp.h : header file
//

#include <ddraw.h>

#define WM_S6010_DISP_L_CLICK	(WM_USER +50)
#define WM_S6010_DISP_L_DBLCLK	(WM_USER +51)

#define INFO_CH_OFFSET_X	6
#define INFO_CH_OFFSET_Y	4

/////////////////////////////////////////////////////////////////////////////
// CWndS6010Disp window

class CWndS6010Disp : public CWnd
{
// Construction
public:
	CWndS6010Disp();

// Attributes
public:

private:
	LPDIRECTDRAW7 m_pDDraw;
	LPDIRECTDRAWCLIPPER m_pDDClipper;
	LPDIRECTDRAWSURFACE7 m_pDDSPrimary;
	LPDIRECTDRAWSURFACE7 m_pDDSOffImage;
	LPDIRECTDRAWSURFACE7 m_pDDSBack;
	LPDIRECTDRAWSURFACE7 m_pDDSTemp;

	int m_szTempSurfV;
	CRect m_rcDisp;
	SIZE m_szResolution;

	DWORD m_nTempPitch;
	DWORD m_nBackPitch;

	BOOL m_bLButDn;

	HPEN m_hPenRed;
	HFONT m_hFontOff;
	HFONT m_hFontOff2;
	int m_idxOldSelDisp;
	int m_idxOldDispMode;

	volatile BOOL m_bPainting;

	CPoint m_ptLDnStart;
	CPoint m_ptLDnCur;

// Operations
public:
	BOOL InitDDraw (int szImgMaxH, int szImgMaxV);

	void BltBackToPrimaryWithDrawingBorder ();
	LPVOID LockAndGetTempSurface ();
	__inline void UnlockTempSurface () { m_pDDSTemp->Unlock (NULL); }
	LPVOID LockAndGetBackSurface ();
	__inline void UnlockBackSurface () { m_pDDSBack->Unlock (NULL); }
	void FillTempBlack ();
	void FillBackBlack ();
	__inline DWORD GetTempPitch () { return m_nTempPitch; }
	__inline DWORD GetBackPitch () { return m_nBackPitch; }
	void BltTempToBack (RECT rcDest, RECT rcSrc, BOOL bUseInfo, char *strInfo);

	// Colorspace[S]...
	void YV12toYUYV (BYTE *bufSrc, BYTE *bufDest, int szH, int szV, int nPitch);
	void YV12toBGRA (BYTE *bufSrc, BYTE *bufDest, int szH, int szV, int nPitch);
	// Colorspace[E]...

	void DrawMotionData (int nPitch, int szH, int szV,
		BYTE *bufDisp, BYTE *pBufMotion);
	void DrawWMData (int nPitch, int szH, int szV,
		BYTE *bufDisp, BYTE *pBufWM);

private:
	BOOL CreateOffscreenSurface (int szOffscrH, int szOffscrV);
	void ReleaseDDraw ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndS6010Disp)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWndS6010Disp();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWndS6010Disp)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDS6010DISP_H__4D61B022_6F58_4F77_AA65_5B1C8896CA55__INCLUDED_)
