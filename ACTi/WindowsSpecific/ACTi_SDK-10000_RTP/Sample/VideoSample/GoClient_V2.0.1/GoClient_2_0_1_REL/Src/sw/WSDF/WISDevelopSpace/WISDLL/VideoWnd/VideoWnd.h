#if !defined(AFX_VIDEOWND_H__2F1F61D7_116D_4D31_B381_3CFF18368C8B__INCLUDED_)
#define AFX_VIDEOWND_H__2F1F61D7_116D_4D31_B381_3CFF18368C8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoWnd.h : header file
//

#include "VideoRender.h"

typedef int (*COMMANDCALLBACK)(int lParam, int nID);
/////////////////////////////////////////////////////////////////////////////
// CVideoWnd window

class CVideoWnd : public CWnd
{
// Construction
public:
	CVideoWnd();
	void CreateWnd(CWnd *pParentWnd, int windowStyle = WS_POPUPWINDOW|WS_CAPTION|WS_THICKFRAME|WS_MAXIMIZEBOX|WS_MINIMIZEBOX); 
	void CreateRender(int VideoWidth, int VideoHeight, int preferredMode = DDRAW_UYVY);

	DWORD	GetVideoMode();
	int		GetPitch();
	BYTE	*BeginDraw();
	void	EndDraw(BOOL draw = FALSE);
	void	SetFullScreen(BOOL bFullScreen);
	BOOL	IsFullScreen();
	void	Zoom(double zoomX, double zoomY);
	void	SetWindowStyle(int style);
	void	SetCallback(int callback, int param);
	void	Close();


// Attributes
public:

	CVideoRender m_render;

	int		m_Pitch;
	int		m_videoMode;
	RECT	m_rect;
	int		m_moveX;
	int		m_moveY;
	BOOL	m_ontop;
	BOOL	m_bIsSurface;
	BOOL	m_bShowPopupMenu;
	BOOL	m_bInited;

	int		m_VideoWidth, m_VideoHeight;
	CWnd	*m_pParentWnd;
	BYTE	*m_pDDBuf;

	int		m_BorderSizeX, m_BorderSizeY;
	int		m_CaptionHeight;
	int		m_windowStyle;

	COMMANDCALLBACK m_cmdCallback;
	int		m_cmdParam;
	int		m_ppLevel;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CVideoWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CVideoWnd)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnFullscreen();
	afx_msg void OnZoomHalf();
	afx_msg void OnZoomDouble();
	afx_msg void OnZoomOriginal();
	afx_msg void OnRatioKeep();
	afx_msg void OnRatio43();
	afx_msg void OnRatioWide();
	afx_msg void OnRatioFree();
	afx_msg void OnAlwaysontop();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRatioThin();
	afx_msg void OnVwAbout();
	afx_msg void OnPpNone();
	afx_msg void OnPpSlight();
	afx_msg void OnPpMedium();
	afx_msg void OnPpHeavy();
	afx_msg void OnPpSuperheavy();
	afx_msg void OnSnapshot();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOWND_H__2F1F61D7_116D_4D31_B381_3CFF18368C8B__INCLUDED_)
