#if !defined(AFX_DLGCTRLREGISTER_H__651B6324_6C67_439C_AE23_34C15789C6C2__INCLUDED_)
#define AFX_DLGCTRLREGISTER_H__651B6324_6C67_439C_AE23_34C15789C6C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCtrlRegister.h : header file
//

#define NUM_REG_IO_ITEM			4

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlRegister dialog

class CDlgCtrlRegister : public CDialog
{
// Construction
public:
	CDlgCtrlRegister(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCtrlRegister)
	enum { IDD = IDD_DLG_CTRL_REG_IO };
	CListBox	m_liRegMap;
	CString	m_edRegMapAddrFrom;
	CString	m_edRegMapAddrTo;
	//}}AFX_DATA
	CString	m_edBufAddr[NUM_REG_IO_ITEM];
	CString	m_edBufData[NUM_REG_IO_ITEM];

	CFont m_fntRegMap;

	void CheckAddrAlign ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCtrlRegister)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCtrlRegister)
	afx_msg void OnButRead0();
	afx_msg void OnButWrite0();
	afx_msg void OnButRead1();
	afx_msg void OnButWrite1();
	afx_msg void OnButRead2();
	afx_msg void OnButWrite2();
	afx_msg void OnButRead3();
	afx_msg void OnButWrite3();
	afx_msg void OnButReadRegMap();
	afx_msg void OnChSetEntireAddr();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCTRLREGISTER_H__651B6324_6C67_439C_AE23_34C15789C6C2__INCLUDED_)
