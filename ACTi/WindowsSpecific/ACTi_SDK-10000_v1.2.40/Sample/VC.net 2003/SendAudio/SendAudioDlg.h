// SendAudioDlg.h : header file
//

#pragma once


// CSendAudioDlg dialog
class CSendAudioDlg : public CDialog
{
// Construction
public:
	CSendAudioDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SENDAUDIO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


	CIPAddressCtrl m_Addr;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	CString m_Path;
	CString m_id;
	CString m_pwd;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
