#if !defined(AFX_DLGCTRLGPIO_H__15799786_59A4_49C3_A54C_85A375A8C2CA__INCLUDED_)
#define AFX_DLGCTRLGPIO_H__15799786_59A4_49C3_A54C_85A375A8C2CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCtrlGPIO.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlGPIO dialog

class CDlgCtrlGPIO : public CDialog
{
// Construction
public:
	CDlgCtrlGPIO(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCtrlGPIO)
	enum { IDD = IDD_DLG_CTRL_GPIO };
	//}}AFX_DATA
	int m_bufRadMode[NUM_GPIO_ALL_PORT];
	BOOL m_bufChOut[NUM_GPIO_ALL_PORT];
	BOOL m_bufChIn[NUM_GPIO_ALL_PORT];
	CComboBox m_bufComIntMode[NUM_GPIO_ALL_PORT];


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCtrlGPIO)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	afx_msg void OnModeChange();
	//{{AFX_MSG(CDlgCtrlGPIO)
	virtual BOOL OnInitDialog();
	afx_msg void OnButWrite();
	afx_msg void OnButRead();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCTRLGPIO_H__15799786_59A4_49C3_A54C_85A375A8C2CA__INCLUDED_)
