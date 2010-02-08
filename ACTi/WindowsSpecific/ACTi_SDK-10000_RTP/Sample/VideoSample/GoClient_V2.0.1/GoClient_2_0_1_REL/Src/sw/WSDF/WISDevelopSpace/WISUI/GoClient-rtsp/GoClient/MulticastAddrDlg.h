#include "afxwin.h"
#if !defined(AFX_MULTICASTADDRDLG_H__96DA82F2_3368_4C1C_A34F_8C0A2C35A074__INCLUDED_)
#define AFX_MULTICASTADDRDLG_H__96DA82F2_3368_4C1C_A34F_8C0A2C35A074__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MulticastAddrDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMulticastAddrDlg dialog

class CMulticastAddrDlg : public CDialog
{
// Construction
public:
	CMulticastAddrDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMulticastAddrDlg)
	enum { IDD = IDD_MCADDRDLG };

	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMulticastAddrDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMulticastAddrDlg)
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:

	CString	m_rtp_port;
	CString m_mcast_addr;
	int m_radio;
    // Txfer audio or not
    BOOL DoAudio;
    CComboBox AudioSetting;
    afx_msg void OnCbnSelchangeCombo1();
    int AudioIdx;
    CString m_Audio_rtp_port;
};
struct NAME_ENUM_PAIR
{
	char name[30];
	int enumValue;
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTICASTADDRDLG_H__96DA82F2_3368_4C1C_A34F_8C0A2C35A074__INCLUDED_)
