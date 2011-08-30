#if !defined(AFX_DLGCTRLPLAYER_H__F67C7CBA_522F_4FB6_AB5A_0B1D6EEB8F4A__INCLUDED_)
#define AFX_DLGCTRLPLAYER_H__F67C7CBA_522F_4FB6_AB5A_0B1D6EEB8F4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCtrlPlayer.h : header file
//

#include "IndexButton.h"

enum
{
	DLCP_BUT_OPEN_VID_FILE,
	DLCP_BUT_OPEN_AUD_FILE,
	DLCP_BUT_SPLIT_1,
	DLCP_BUT_SPLIT_4,
	DLCP_BUT_SPLIT_6,
	DLCP_BUT_SPLIT_7,
	DLCP_BUT_SPLIT_8,
	DLCP_BUT_SPLIT_9,
	DLCP_BUT_SPLIT_10,
	DLCP_BUT_SPLIT_13,
	DLCP_BUT_SPLIT_16,
	DLCP_BUT_PLAY,
	DLCP_BUT_PAUSE,
	DLCP_BUT_STOP,
	DLCP_BUT_BW_1_FRM,
	DLCP_BUT_FW_1_FRM,
	DLCP_BUT_BW_N_FRM,
	DLCP_BUT_FW_N_FRM,
	DLCP_BUT_SET_POS_START,
	DLCP_BUT_SET_POS_END,
	DLCP_BUT_PS_SLOW,
	DLCP_BUT_PS_NORMAL,
	DLCP_BUT_PS_FAST,
	DLCP_BUT_CH_REAL_0,
	DLCP_BUT_CH_REAL_1,
	DLCP_BUT_CH_REAL_2,
	DLCP_BUT_CH_REAL_3,
	DLCP_BUT_CH_REAL_4,
	DLCP_BUT_CH_REAL_5,
	DLCP_BUT_CH_REAL_6,
	DLCP_BUT_CH_REAL_7,
	DLCP_BUT_CH_REAL_8,
	DLCP_BUT_CH_REAL_9,
	DLCP_BUT_CH_REAL_10,
	DLCP_BUT_CH_REAL_11,
	DLCP_BUT_CH_REAL_12,
	DLCP_BUT_CH_REAL_13,
	DLCP_BUT_CH_REAL_14,
	DLCP_BUT_CH_REAL_15,
	DLCP_BUT_CH_VIRT_0,
	DLCP_BUT_CH_VIRT_1,
	DLCP_BUT_CH_VIRT_2,
	DLCP_BUT_CH_VIRT_3,
	DLCP_BUT_CH_VIRT_4,
	DLCP_BUT_CH_VIRT_5,
	DLCP_BUT_CH_VIRT_6,
	DLCP_BUT_CH_VIRT_7,
	DLCP_BUT_CH_VIRT_8,
	DLCP_BUT_CH_VIRT_9,
	DLCP_BUT_CH_VIRT_10,
	DLCP_BUT_CH_VIRT_11,
	DLCP_BUT_CH_VIRT_12,
	DLCP_BUT_CH_VIRT_13,
	DLCP_BUT_CH_VIRT_14,
	DLCP_BUT_CH_VIRT_15,
	DLCP_BUT_INFO_CH,
	DLCP_BUT_INFO_FRM,
	DLCP_BUT_INFO_TIME,
	DLCP_BUT_USE_DEF_CS,
	DLCP_BUT_SZ_D4,
	DLCP_BUT_SZ_D2,
	DLCP_BUT_SZ_X1,
	DLCP_BUT_SZ_X2,
	DLCP_BUT_SZ_X3,
	DLCP_BUT_SZ_FULL,
	DLCP_BUT_CH_M4_FLT_DEBLK,
	DLCP_BUT_CH_M4_FLT_DERNG,
	DLCP_BUT_CH_IMG_FLT_DEINT,
	DLCP_BUT_CH_IMG_FLT_UPSC,
	DLCP_BUT_CH_IMG_FLT_DNSC,
	DLCP_BUT_SNAPSHOT,
	DLCP_BUT_QUIT,
	NUM_DLCP_BUTTONS
};

class CPlayerManager;
class CWndPlayer;

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlPlayer dialog

class CDlgCtrlPlayer : public CDialog
{
// Construction
public:
	CDlgCtrlPlayer(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCtrlPlayer)
	enum { IDD = IDD_DLG_CTRL_PLAYER };
	CSpinButtonCtrl	m_spNumFBWStep;
	CStatic	m_staPlaySpeed;
	CStatic	m_staNumStepFBW;
	CStatic	m_staCurTime;
	CStatic	m_staCurFrm;
	CSliderCtrl	m_sliPlayer;
	CString	m_edVidFilePath;
	CString	m_edAudFilePath;
	//}}AFX_DATA
	CIndexButton m_butAll[NUM_DLCP_BUTTONS];
	BOOL m_bUseDefCS;
	int m_idxPlaySpeed;
	BOOL m_bDispCh;
	BOOL m_bDispFrm;
	BOOL m_bDispTime;
	int m_numStepFBW;

	DWORD m_curPosFrm;
	DWORD m_curSzFrm;
	volatile DWORD m_curTimeMSec;
	volatile DWORD m_oldTimeMSec;

public:
	void SetPlayerManager (CPlayerManager *pPlayer);
	void SetPlayerWnd (CWndPlayer *pWndPlayer);
	
private:
	CPlayerManager *m_pPlayer;
	CWndPlayer *m_pWndPlayer;

	void UpdateDlgCtrlFromFile ();
	void UpdatePlaySpeed (int idxPlaySpeed);
	void UpdateChannelButtons ();
	void UpdateInfoDispButtons ();
	void UpdateMP4PostFilterButtons ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCtrlPlayer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	afx_msg LRESULT OnIndexButtonClick (WPARAM wParam, LPARAM lParam);
	// Generated message map functions
	//{{AFX_MSG(CDlgCtrlPlayer)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDeltaposSpNumFwBwStep(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCTRLPLAYER_H__F67C7CBA_522F_4FB6_AB5A_0B1D6EEB8F4A__INCLUDED_)
