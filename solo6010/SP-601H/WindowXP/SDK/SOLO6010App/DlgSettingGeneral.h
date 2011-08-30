#if !defined(AFX_DLGSETTINGGENERAL_H__F3009E9D_2247_4855_A7B1_F74868E0A59D__INCLUDED_)
#define AFX_DLGSETTINGGENERAL_H__F3009E9D_2247_4855_A7B1_F74868E0A59D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSettingGeneral.h : header file
//

const char *const GL_STR_CAM[NUM_LIVE_DISP_WND] = 
{
	"Cam 01",
	"Cam 02",
	"Cam 03",
	"Cam 04",
	"Cam 05",
	"Cam 06",
	"Cam 07",
	"Cam 08",
	"Cam 09",
	"Cam 10",
	"Cam 11",
	"Cam 12",
	"Cam 13",
	"Cam 14",
	"Cam 15",
	"Cam 16",
};

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingGeneral dialog

class CDlgSettingGeneral : public CDialog
{
// Construction
public:
	CDlgSettingGeneral(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSettingGeneral)
	enum { IDD = IDD_DLG_SETTING_GENERAL };
	CSpinButtonCtrl	m_spNumOverSurf;
	int		m_radVideoMode;
	CString	m_edStrRecDir;
	int		m_radVCard;
	int		m_radLiveDispMode;
	int		m_radModeDeinterlacing;
	int		m_radRecMethodMP4;
	int		m_radRecMethodG723;
	CString	m_staNumOverSurf;
	BOOL	m_chEncOSD_bUseCh;
	BOOL	m_chEncOSD_bUseTime;
	BOOL	m_chEncOSD_bUseDate;
	BOOL	m_chEncOSD_bUseShadowText;
	BYTE	m_edEncOSD_colorY;
	BYTE	m_edEncOSD_colorU;
	BYTE	m_edEncOSD_colorV;
	BYTE	m_edEncOSD_offsetH;
	BYTE	m_edEncOSD_offsetV;
	//}}AFX_DATA
	CComboBox m_comVDInCh[NUM_LIVE_DISP_WND];

	INFO_APP_GLOBAL m_tmpGlGInfo;
	BOOL m_bChangedGlGInfo;
	int m_numOverSurf;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSettingGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSettingGeneral)
	afx_msg void OnButOpenRecDir();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButSetMosaicMotion();
	afx_msg void OnDeltaposSpNumOverSurf(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButDefaultNumSurf();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETTINGGENERAL_H__F3009E9D_2247_4855_A7B1_F74868E0A59D__INCLUDED_)
