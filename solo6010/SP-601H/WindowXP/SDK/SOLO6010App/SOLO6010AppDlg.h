// SOLO6010AppDlg.h : header file
//

#if !defined(AFX_SOLO6010APPDLG_H__68CF7B95_7F1A_4E5E_9C46_2B907E9DAE5E__INCLUDED_)
#define AFX_SOLO6010APPDLG_H__68CF7B95_7F1A_4E5E_9C46_2B907E9DAE5E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SOLO6010.h"
#include "WndLiveDisp.h"
#include "RecManager.h"
#include "HWDecManager.h"
#include "PlayerManager.h"
#include "WndPlayer.h"

/////////////////////////////////////////////////////////////////////////////
// CSOLO6010AppDlg dialog

class CSOLO6010AppDlg : public CDialog
{
// Construction
public:
	CSOLO6010AppDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSOLO6010AppDlg)
	enum { IDD = IDD_SOLO6010APP_DIALOG };
	CComboBox	m_comCurS6010;
	//}}AFX_DATA
	char m_strCurDir[MAX_PATH];
	CButton	m_bufChRecS6010[NUM_MAX_S6010];

	int m_numS6010;
	int m_curS6010;
	CSOLO6010 *m_bufPClS6010[NUM_MAX_S6010];
	CRecManager m_bufClRecManager[NUM_MAX_S6010];
	CHWDecManager m_bufClHWDecManager[NUM_MAX_S6010];
	CWndLiveDisp m_wndLiveDisp;

	CPlayerManager m_manPlayer;
	CWndPlayer m_wndPlayer;

	BOOL m_bPlayerOn;

	BOOL OpenS6010 (int idxS6010);
	BOOL InitS6010 (int idxS6010);
	void CloseS6010 (int idxS6010);
	void EnableCtrlFromS6010Stat ();

	void ToggleLive ();

	void WriteConfig ();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSOLO6010AppDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	afx_msg LRESULT OnGetMsgAppDlgCtrl (WPARAM wParam, LPARAM lParam);
	// Generated message map functions
	//{{AFX_MSG(CSOLO6010AppDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnButLive();
	afx_msg void OnButPlayer();
	afx_msg void OnButRecord();
	afx_msg void OnButSettingRecord();
	afx_msg void OnButSettingGeneral();
	afx_msg void OnSelchangeComCurS6010();
	afx_msg void OnButGpio();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButCtrlRegister();
	afx_msg void OnButCtrlUart();
	afx_msg void OnClose();
	afx_msg void OnButTest();
	afx_msg void OnButCtrlI2c();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOLO6010APPDLG_H__68CF7B95_7F1A_4E5E_9C46_2B907E9DAE5E__INCLUDED_)
