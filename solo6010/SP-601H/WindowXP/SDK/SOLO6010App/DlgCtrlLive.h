#if !defined(AFX_DLGCTRLLIVE_H__3D6AE7BC_31FF_4F34_B695_58E585C10800__INCLUDED_)
#define AFX_DLGCTRLLIVE_H__3D6AE7BC_31FF_4F34_B695_58E585C10800__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCtrlLive.h : header file
//

#include "IndexButton.h"

enum
{
	IDX_DCL_BUT_LIVE_DISP_1,
	IDX_DCL_BUT_LIVE_DISP_4,
	IDX_DCL_BUT_LIVE_DISP_6,
	IDX_DCL_BUT_LIVE_DISP_7,
	IDX_DCL_BUT_LIVE_DISP_8,
	IDX_DCL_BUT_LIVE_DISP_9,
	IDX_DCL_BUT_LIVE_DISP_10,
	IDX_DCL_BUT_LIVE_DISP_13,
	IDX_DCL_BUT_LIVE_DISP_16,
	IDX_DCL_BUT_CH_0,
	IDX_DCL_BUT_CH_1,
	IDX_DCL_BUT_CH_2,
	IDX_DCL_BUT_CH_3,
	IDX_DCL_BUT_CH_4,
	IDX_DCL_BUT_CH_5,
	IDX_DCL_BUT_CH_6,
	IDX_DCL_BUT_CH_7,
	IDX_DCL_BUT_CH_8,
	IDX_DCL_BUT_CH_9,
	IDX_DCL_BUT_CH_10,
	IDX_DCL_BUT_CH_11,
	IDX_DCL_BUT_CH_12,
	IDX_DCL_BUT_CH_13,
	IDX_DCL_BUT_CH_14,
	IDX_DCL_BUT_CH_15,
	IDX_DCL_BUT_DISZ_DIV_4,
	IDX_DCL_BUT_DISZ_DIV_2,
	IDX_DCL_BUT_DISZ_X_1,
	IDX_DCL_BUT_DISZ_X_2,
	IDX_DCL_BUT_DISZ_X_3,
	IDX_DCL_BUT_DISZ_FULL,
	IDX_DCL_BUT_CH_DISP,
	IDX_DCL_BUT_CH_ALWAYS_TOP,
	IDX_DCL_BUT_HIDE,
	IDX_DCL_BUT_QUIT,
	IDX_DCL_BUT_CLR_DEF_HUE,
	IDX_DCL_BUT_CLR_DEF_SAT,
	IDX_DCL_BUT_CLR_DEF_CON,
	IDX_DCL_BUT_CLR_DEF_BRI,
	IDX_DCL_BUT_L_OR_D_0,
	IDX_DCL_BUT_L_OR_D_1,
	IDX_DCL_BUT_L_OR_D_2,
	IDX_DCL_BUT_L_OR_D_3,
	IDX_DCL_BUT_L_OR_D_4,
	IDX_DCL_BUT_L_OR_D_5,
	IDX_DCL_BUT_L_OR_D_6,
	IDX_DCL_BUT_L_OR_D_7,
	IDX_DCL_BUT_L_OR_D_8,
	IDX_DCL_BUT_L_OR_D_9,
	IDX_DCL_BUT_L_OR_D_10,
	IDX_DCL_BUT_L_OR_D_11,
	IDX_DCL_BUT_L_OR_D_12,
	IDX_DCL_BUT_L_OR_D_13,
	IDX_DCL_BUT_L_OR_D_14,
	IDX_DCL_BUT_L_OR_D_15,
	IDX_DCL_BUT_HWD_OPEN_VID_FILE,
	IDX_DCL_BUT_HWD_OPEN_AUD_FILE,
	IDX_DCL_BUT_HWD_PLAY,
	IDX_DCL_BUT_HWD_PAUSE,
	IDX_DCL_BUT_HWD_STOP,
	IDX_DCL_BUT_HWD_BACK_PLAY,
	IDX_DCL_BUT_HWD_BW_1_FRM,
	IDX_DCL_BUT_HWD_FW_1_FRM,
	IDX_DCL_BUT_HWD_BW_N_FRM,
	IDX_DCL_BUT_HWD_FW_N_FRM,
	IDX_DCL_BUT_HWD_SET_POS_START,
	IDX_DCL_BUT_HWD_SET_POS_END,
	IDX_DCL_BUT_HWD_PS_SLOW,
	IDX_DCL_BUT_HWD_PS_NORMAL,
	IDX_DCL_BUT_HWD_PS_FAST,
	IDX_DCL_BUT_HWD_CH_REAL_0,
	IDX_DCL_BUT_HWD_CH_REAL_1,
	IDX_DCL_BUT_HWD_CH_REAL_2,
	IDX_DCL_BUT_HWD_CH_REAL_3,
	IDX_DCL_BUT_HWD_CH_REAL_4,
	IDX_DCL_BUT_HWD_CH_REAL_5,
	IDX_DCL_BUT_HWD_CH_REAL_6,
	IDX_DCL_BUT_HWD_CH_REAL_7,
	IDX_DCL_BUT_HWD_CH_REAL_8,
	IDX_DCL_BUT_HWD_CH_REAL_9,
	IDX_DCL_BUT_HWD_CH_REAL_10,
	IDX_DCL_BUT_HWD_CH_REAL_11,
	IDX_DCL_BUT_HWD_CH_REAL_12,
	IDX_DCL_BUT_HWD_CH_REAL_13,
	IDX_DCL_BUT_HWD_CH_REAL_14,
	IDX_DCL_BUT_HWD_CH_REAL_15,
	IDX_DCL_BUT_HWD_CH_VIRT_0,
	IDX_DCL_BUT_HWD_CH_VIRT_1,
	IDX_DCL_BUT_HWD_CH_VIRT_2,
	IDX_DCL_BUT_HWD_CH_VIRT_3,
	IDX_DCL_BUT_HWD_CH_VIRT_4,
	IDX_DCL_BUT_HWD_CH_VIRT_5,
	IDX_DCL_BUT_HWD_CH_VIRT_6,
	IDX_DCL_BUT_HWD_CH_VIRT_7,
	IDX_DCL_BUT_HWD_CH_VIRT_8,
	IDX_DCL_BUT_HWD_CH_VIRT_9,
	IDX_DCL_BUT_HWD_CH_VIRT_10,
	IDX_DCL_BUT_HWD_CH_VIRT_11,
	IDX_DCL_BUT_HWD_CH_VIRT_12,
	IDX_DCL_BUT_HWD_CH_VIRT_13,
	IDX_DCL_BUT_HWD_CH_VIRT_14,
	IDX_DCL_BUT_HWD_CH_VIRT_15,
	IDC_DCL_BUT_USE_S6010_DI_FLT,
	IDC_DCL_BUT_CHANGE_LIVE_MODE_AUTO,
	IDC_DCL_BUT_VLOSS_CAM_0,
	IDC_DCL_BUT_VLOSS_CAM_1,
	IDC_DCL_BUT_VLOSS_CAM_2,
	IDC_DCL_BUT_VLOSS_CAM_3,
	IDC_DCL_BUT_VLOSS_CAM_4,
	IDC_DCL_BUT_VLOSS_CAM_5,
	IDC_DCL_BUT_VLOSS_CAM_6,
	IDC_DCL_BUT_VLOSS_CAM_7,
	IDC_DCL_BUT_VLOSS_CAM_8,
	IDC_DCL_BUT_VLOSS_CAM_9,
	IDC_DCL_BUT_VLOSS_CAM_10,
	IDC_DCL_BUT_VLOSS_CAM_11,
	IDC_DCL_BUT_VLOSS_CAM_12,
	IDC_DCL_BUT_VLOSS_CAM_13,
	IDC_DCL_BUT_VLOSS_CAM_14,
	IDC_DCL_BUT_VLOSS_CAM_15,
	NUM_DCL_BUTTONS
};

enum
{
	IDX_COLOR_HUE,
	IDX_COLOR_SAT,
	IDX_COLOR_CON,
	IDX_COLOR_BRI,
	NUM_COLOR_ITEM
};
#define NUM_DEF_COLOR_VAL	0x80

enum
{
	IDX_DCL_TIMER_DET_VLOSS = 1,
	IDX_DCL_TIMER_UPDATE_PLAYER_STAT
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlLive dialog

class CDlgCtrlLive : public CDialog
{
// Construction
public:
	CDlgCtrlLive(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCtrlLive)
	enum { IDD = IDD_DLG_CTRL_LIVE };
	CSpinButtonCtrl	m_spNumFBWStep;
	CStatic	m_staPlaySpeed;
	CStatic	m_staNumStepFBW;
	CStatic	m_staCurTime;
	CStatic	m_staCurFrm;
	CSliderCtrl	m_sliPlayer;
	CString	m_edVidFilePath;
	CString	m_edAudFilePath;
	//}}AFX_DATA
	CStatic	m_bufStaColor[NUM_COLOR_ITEM];
	CSliderCtrl	m_bufSliColor[NUM_COLOR_ITEM];

	CIndexButton m_butAll[NUM_DCL_BUTTONS];

	int m_idxPlaySpeed;
	int m_numStepFBW;
	DWORD m_curPosFrm;
	DWORD m_curSzFrm;
	volatile DWORD m_curTimeMSec;
	volatile DWORD m_oldTimeMSec;

	BOOL m_bChangeLiveModeAuto;

	void GetLiveColorAndUpdateCtrl ();
	void SetLiveColorAndUpdateCtrl (int idxColor, int nValue);

	void OnColorDefButDn (WPARAM wParam);
	void SetDispWndModeFromFlag ();
	void OnDispWndModeButDn (WPARAM wParam);

	void OnHWPlayerCtrlButDn (WPARAM wParam);
	void UpdateHWPlayerCtrl (BOOL bInitSlider = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCtrlLive)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	afx_msg LRESULT OnIndexButtonClick (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDispWndClick (WPARAM wParam, LPARAM lParam);
	// Generated message map functions
	//{{AFX_MSG(CDlgCtrlLive)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDeltaposSpNumFwBwStep(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCTRLLIVE_H__3D6AE7BC_31FF_4F34_B695_58E585C10800__INCLUDED_)
