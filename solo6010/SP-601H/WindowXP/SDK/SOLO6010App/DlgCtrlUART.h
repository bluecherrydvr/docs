#if !defined(AFX_DLGCTRLUART_H__00387109_6F0F_4FD5_882D_A7CAE268EE2D__INCLUDED_)
#define AFX_DLGCTRLUART_H__00387109_6F0F_4FD5_882D_A7CAE268EE2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCtrlUART.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlUART dialog

class CDlgCtrlUART : public CDialog
{
// Construction
public:
	CDlgCtrlUART(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCtrlUART)
	enum { IDD = IDD_DLG_CTRL_UART };
	CString	m_edReadData;
	CString	m_staSzRead;
	CString	m_edWriteData;
	int		m_radBaudRate;
	int		m_radDataSize;
	int		m_radParity;
	int		m_radStopBit;
	int		m_radPort;
	//}}AFX_DATA

	int m_bufBaudRate[NUM_UART_PORT];
	int m_bufDataSize[NUM_UART_PORT];
	int m_bufParity[NUM_UART_PORT];
	int m_bufStopBit[NUM_UART_PORT];

	int m_idxOldPort;

	void UpdateDlgItemFromCurPort ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCtrlUART)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCtrlUART)
	virtual BOOL OnInitDialog();
	afx_msg void OnButRead();
	afx_msg void OnButSetProperty();
	afx_msg void OnButWrite();
	afx_msg void OnRadPort0();
	afx_msg void OnRadPort1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCTRLUART_H__00387109_6F0F_4FD5_882D_A7CAE268EE2D__INCLUDED_)
