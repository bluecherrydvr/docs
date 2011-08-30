#if !defined(AFX_DLGMAINCONTROL_H__ECB568D2_2BE0_49E6_B703_A7D4CC431491__INCLUDED_)
#define AFX_DLGMAINCONTROL_H__ECB568D2_2BE0_49E6_B703_A7D4CC431491__INCLUDED_

#include "IndexButton.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDlgMainControl dialog

class CDlgMainControl : public CDialog
{
// Construction
public:
	CDlgMainControl(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgMainControl)
	enum { IDD = IDD_DLG_MAIN_CONTROL };
	CSliderCtrl	m_sliPlayerAll;
	CSliderCtrl	m_sliPlayer;
	CString	m_edStrStreamFilePath;
	//}}AFX_DATA

	CIndexButton m_butAll[NUM_ALL_BUTTONS];

	int m_idxCurSelChBut;
	int m_idxCurZoom[NUM_ZOOM];

	void ZoomButtonDn (int wParam);

	void OnSnapshotButDn ();
	void OnAVISaveButDn ();

	void ChangeChannelButState (int idxSelCh);
	void ChangeSliderState ();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgMainControl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	afx_msg LRESULT OnUpdateFilePath (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIndexButtonClick (WPARAM wParam, LPARAM lParam);
	// Generated message map functions
	//{{AFX_MSG(CDlgMainControl)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMAINCONTROL_H__ECB568D2_2BE0_49E6_B703_A7D4CC431491__INCLUDED_)
