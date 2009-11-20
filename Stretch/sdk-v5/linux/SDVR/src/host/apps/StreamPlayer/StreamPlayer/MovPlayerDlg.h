// MovPlayerDlg.h : header file
//

#pragma once

#include "../sdvr_player.h"


// CMovPlayerDlg dialog
class CMovPlayerDlg : public CDialog
{
// Construction
public:
	CMovPlayerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MOVPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
#if 0
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#endif
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnDestroy();

private:
	void UpdateUI();

private:
	sx_int32 m_duration;       
	double m_avgTimePerFrame;
	UINT_PTR m_graphTimer;
	sx_int32 m_totalFrames;
	LONG m_nPort;
	bool	m_bIsStreaming;

public:
	afx_msg LRESULT OnPlayerEvent(WPARAM, LPARAM);
	afx_msg void OnBnClickedRtpStream();
};
