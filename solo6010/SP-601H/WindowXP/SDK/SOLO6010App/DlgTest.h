#if !defined(AFX_DLGTEST_H__3D4C403F_3F1B_4EF5_A7E2_6C3DFA572127__INCLUDED_)
#define AFX_DLGTEST_H__3D4C403F_3F1B_4EF5_A7E2_6C3DFA572127__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgTest.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgTest dialog

class CDlgTest : public CDialog
{
// Construction
public:
	CDlgTest(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgTest)
	enum { IDD = IDD_DLG_TEST };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	CComboBox m_comChannel;
	CComboBox m_comSzImg;
	int m_chBRec;
	CComboBox m_comQuality;
	CComboBox m_comFPS;
	int m_edSzGOP;
	CComboBox m_comBR;
	CComboBox m_comCBR_Buf;

	SETTING_MP4_REC m_setMP4Rec;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTest)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgTest)
	afx_msg void OnButChangeRecSetting();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComChannel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGTEST_H__3D4C403F_3F1B_4EF5_A7E2_6C3DFA572127__INCLUDED_)
