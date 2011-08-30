#if !defined(AFX_DLGG723PLAYCTRL_H__DF032836_D0E0_4194_9637_63CC0F1564C6__INCLUDED_)
#define AFX_DLGG723PLAYCTRL_H__DF032836_D0E0_4194_9637_63CC0F1564C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgG723PlayCtrl.h : header file
//

#include "IndexButton.h"

#define SLIDER_RESOLUTION	5

/////////////////////////////////////////////////////////////////////////////
// CDlgG723PlayCtrl dialog

class CDlgG723PlayCtrl : public CDialog
{
// Construction
public:
	CDlgG723PlayCtrl(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgG723PlayCtrl)
	enum { IDD = IDD_DLG_G723_PLAY_CTRL };
	CSliderCtrl	m_sliPlayer;
	CString	m_edStrG723FilePath;
	//}}AFX_DATA
	CIndexButton m_bufCButCh[NUM_CHANNEL];
	int m_idxSelCh;

	int m_curPosSli;
	int m_maxPosSli;

	void ChangeSliderState ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgG723PlayCtrl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	afx_msg LRESULT OnIndexButtonClick (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateFilePath (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateSliderPos (WPARAM wParam, LPARAM lParam);
	//{{AFX_MSG(CDlgG723PlayCtrl)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnButOpenG723File();
	afx_msg void OnButPlay();
	afx_msg void OnButPause();
	afx_msg void OnButStop();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnOK();
	afx_msg void OnButRawSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGG723PLAYCTRL_H__DF032836_D0E0_4194_9637_63CC0F1564C6__INCLUDED_)
