/************************************************************************
 *
 * Copyright 2008 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/


#pragma once
class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:
	void UpdateUIState(splayer_state_e eState);

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CToolBar    m_wndToolBar1;
	CSliderCtrl m_slider;

private:
	UINT_PTR	m_nTimer;
	bool		m_bEnableOpen;
	bool		m_bEnableOpenStream;
	bool		m_bEnablePlay;
	bool		m_bEnablePause;
	bool		m_bEnableStop;
	bool		m_bEnableFastFwd;
	bool		m_bEnableFastRew;
	bool		m_bEnableStepFwd;
	bool		m_bEnableStepBwd;
	bool		m_bEnableCapture;
	bool		m_bEnableDecode;
	bool		m_bEnableZoom;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnToolbarUpdateOpen(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateOpenStream(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdatePlay(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdatePause(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateStop(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateFastFwd(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateFastRew(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateStepFwd(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateStepBwd(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateCapture(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateDecode(CCmdUI *pCmdUI);
	afx_msg void OnToolbarUpdateZoom(CCmdUI *pCmdUI);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos,	CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()
};


