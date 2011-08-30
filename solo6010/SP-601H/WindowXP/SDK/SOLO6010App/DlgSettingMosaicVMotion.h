#if !defined(AFX_DLGSETTINGMOSAICVMOTION_H__3ADDCFCD_8A87_4621_AA9C_8C5D57827F59__INCLUDED_)
#define AFX_DLGSETTINGMOSAICVMOTION_H__3ADDCFCD_8A87_4621_AA9C_8C5D57827F59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSettingMosaicVMotion.h : header file
//

#include "IndexButton.h"

enum
{
	IDX_DSMVM_BUT_MO_CH_0,
	IDX_DSMVM_BUT_MO_CH_1,
	IDX_DSMVM_BUT_MO_CH_2,
	IDX_DSMVM_BUT_MO_CH_3,
	IDX_DSMVM_BUT_MO_CH_4,
	IDX_DSMVM_BUT_MO_CH_5,
	IDX_DSMVM_BUT_MO_CH_6,
	IDX_DSMVM_BUT_MO_CH_7,
	IDX_DSMVM_BUT_MO_CH_8,
	IDX_DSMVM_BUT_MO_CH_9,
	IDX_DSMVM_BUT_MO_CH_10,
	IDX_DSMVM_BUT_MO_CH_11,
	IDX_DSMVM_BUT_MO_CH_12,
	IDX_DSMVM_BUT_MO_CH_13,
	IDX_DSMVM_BUT_MO_CH_14,
	IDX_DSMVM_BUT_MO_CH_15,

	IDX_DSMVM_BUT_VM_CH_0,
	IDX_DSMVM_BUT_VM_CH_1,
	IDX_DSMVM_BUT_VM_CH_2,
	IDX_DSMVM_BUT_VM_CH_3,
	IDX_DSMVM_BUT_VM_CH_4,
	IDX_DSMVM_BUT_VM_CH_5,
	IDX_DSMVM_BUT_VM_CH_6,
	IDX_DSMVM_BUT_VM_CH_7,
	IDX_DSMVM_BUT_VM_CH_8,
	IDX_DSMVM_BUT_VM_CH_9,
	IDX_DSMVM_BUT_VM_CH_10,
	IDX_DSMVM_BUT_VM_CH_11,
	IDX_DSMVM_BUT_VM_CH_12,
	IDX_DSMVM_BUT_VM_CH_13,
	IDX_DSMVM_BUT_VM_CH_14,
	IDX_DSMVM_BUT_VM_CH_15,

	IDX_DSMVM_BUT_CBD_CH_0,
	IDX_DSMVM_BUT_CBD_CH_1,
	IDX_DSMVM_BUT_CBD_CH_2,
	IDX_DSMVM_BUT_CBD_CH_3,
	IDX_DSMVM_BUT_CBD_CH_4,
	IDX_DSMVM_BUT_CBD_CH_5,
	IDX_DSMVM_BUT_CBD_CH_6,
	IDX_DSMVM_BUT_CBD_CH_7,
	IDX_DSMVM_BUT_CBD_CH_8,
	IDX_DSMVM_BUT_CBD_CH_9,
	IDX_DSMVM_BUT_CBD_CH_10,
	IDX_DSMVM_BUT_CBD_CH_11,
	IDX_DSMVM_BUT_CBD_CH_12,
	IDX_DSMVM_BUT_CBD_CH_13,
	IDX_DSMVM_BUT_CBD_CH_14,
	IDX_DSMVM_BUT_CBD_CH_15,

	IDX_DSMVM_BUT_AMD_CH_0,
	IDX_DSMVM_BUT_AMD_CH_1,
	IDX_DSMVM_BUT_AMD_CH_2,
	IDX_DSMVM_BUT_AMD_CH_3,
	IDX_DSMVM_BUT_AMD_CH_4,
	IDX_DSMVM_BUT_AMD_CH_5,
	IDX_DSMVM_BUT_AMD_CH_6,
	IDX_DSMVM_BUT_AMD_CH_7,
	IDX_DSMVM_BUT_AMD_CH_8,
	IDX_DSMVM_BUT_AMD_CH_9,
	IDX_DSMVM_BUT_AMD_CH_10,
	IDX_DSMVM_BUT_AMD_CH_11,
	IDX_DSMVM_BUT_AMD_CH_12,
	IDX_DSMVM_BUT_AMD_CH_13,
	IDX_DSMVM_BUT_AMD_CH_14,
	IDX_DSMVM_BUT_AMD_CH_15,

	IDX_DSMVM_BUT_OK,
	IDX_DSMVM_BUT_CANCEL,

	NUM_DSMVM_BUT
};

const unsigned short GL_VM_TH_IDX_TO_VAL[NUM_VM_TH] = {
	NUM_DEF_V_MOTION_TH >>3,
	NUM_DEF_V_MOTION_TH >>2,
	NUM_DEF_V_MOTION_TH >>1,
	NUM_DEF_V_MOTION_TH,
	NUM_DEF_V_MOTION_TH <<1,
	NUM_DEF_V_MOTION_TH <<2,
	NUM_DEF_V_MOTION_TH <<3,
	NUM_DEF_V_MOTION_TH <<4,
	NUM_DEF_V_MOTION_TH <<5,
};

const char* const GL_COM_STR_IDX_VM_GRID_CLR[NUM_VM_GRID_CLR] =
{
	"Clear",
	"Alpha",
	"Set",
};

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingMosaicVMotion dialog

class CDlgSettingMosaicVMotion : public CDialog
{
// Construction
public:
	CDlgSettingMosaicVMotion(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSettingMosaicVMotion)
	enum { IDD = IDD_DLG_SETTING_MOSAIC_VMOTION };
	//}}AFX_DATA
	CIndexButton m_butAll[NUM_DSMVM_BUT];

	// Mosaic[S]
	CComboBox m_MOSAIC_comStrength;

	BOOL m_MOSAIC_bufTmpBSet[NUM_LIVE_DISP_WND];
	RECT m_MOSAIC_bufTmpRc[NUM_LIVE_DISP_WND];
	// Mosaic[E]

	// Video Motion[S]
	CButton m_VM_cbutBShowGrid;
	int m_VM_edCntMinDet;

	CComboBox m_VM_bufComClr[IDX_VM_GRID_ITEM][NUM_VM_CLR];
	int m_VM_bufEdClr[IDX_VM_GRID_ITEM][NUM_VM_CLR];
	CButton m_VM_bufRadTh[NUM_VM_TH];

	unsigned short m_VM_idxCurSelTh;
	
	void VM_ChangeCurSelTh (int idxMotionTh);
	void VM_SetColorMode (int idxItem, int idxColor);
	void VM_SetColorData (int idxItem, int idxColor);
	// Video Motion[E]

	// Advanced Motion Detection[S]
	int m_ADVMD_idxCurSelCh;
	unsigned short m_ADVMD_idxCurSelTh;

	CButton m_ADVMD_bufRadNumChkItem[MAX_ADV_MOT_CHECK_ITEM];
	CButton m_ADVMD_bufRadCurChkItem[MAX_ADV_MOT_CHECK_ITEM];
	CButton m_ADVMD_bufRadTh[NUM_VM_TH];
	int m_ADVMD_edCntMinDet;
	CComboBox m_ADVMD_comChkInterval;
	CButton m_ADVMD_cbutBShowGrid;

	void ADVMD_ChangeCurSelTh (int idxTh);
	void ADVMD_ChangeNumChkItem (int idxNumChkItem);
	void ADVMD_ChangeCurChkItem (int idxCurChkItem);
	// Advanced Motion Detection[E]

	int GetVMThIdxFromRealTh (int nRealTh);

	void EnableDlgItemFromMode ();

	void SetDlgItemState_Mosaic ();
	void SetDlgItemState_VM ();
	void SetDlgItemState_ADVMD ();

	void EnableItem_Mosaic (BOOL bEnable);
	void EnableItem_VM (BOOL bEnable, BOOL bExecThSetting = FALSE);
	void EnableItem_ADVMD (BOOL bEnable, BOOL bExecThSetting = FALSE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSettingMosaicVMotion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	afx_msg LRESULT OnIndexButtonClick (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDispWndClick (WPARAM wParam, LPARAM lParam);
	// Generated message map functions
	//{{AFX_MSG(CDlgSettingMosaicVMotion)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnVmChBShowGrid();
	afx_msg void OnVmRadTh0();
	afx_msg void OnVmRadTh1();
	afx_msg void OnVmRadTh2();
	afx_msg void OnVmRadTh3();
	afx_msg void OnVmRadTh4();
	afx_msg void OnVmRadTh5();
	afx_msg void OnVmRadTh6();
	afx_msg void OnVmRadTh7();
	afx_msg void OnVmRadTh8();
	afx_msg void OnSelchangeVmComClrBoxY();
	afx_msg void OnSelchangeVmComClrBoxCr();
	afx_msg void OnSelchangeVmComClrBoxCb();
	afx_msg void OnSelchangeVmComClrAreaY();
	afx_msg void OnSelchangeVmComClrAreaCr();
	afx_msg void OnSelchangeVmComClrAreaCb();
	afx_msg void OnChangeVmEdClrBoxY();
	afx_msg void OnChangeVmEdClrBoxCr();
	afx_msg void OnChangeVmEdClrBoxCb();
	afx_msg void OnChangeVmEdClrAreaY();
	afx_msg void OnChangeVmEdClrAreaCr();
	afx_msg void OnChangeVmEdClrAreaCb();
	afx_msg void OnVmChBExecSetTh();
	afx_msg void OnChangeVmEdCntMinDet();
	afx_msg void OnSelchangeMosaicComStrength();
	afx_msg void OnAdvmdChBExecSetTh();
	afx_msg void OnSelchangeAdvmdComChkInterval();
	afx_msg void OnChangeAdvmdEdCntMinDet();
	afx_msg void OnAdvmdRadTh0();
	afx_msg void OnAdvmdRadTh1();
	afx_msg void OnAdvmdRadTh2();
	afx_msg void OnAdvmdRadTh3();
	afx_msg void OnAdvmdRadTh4();
	afx_msg void OnAdvmdRadTh5();
	afx_msg void OnAdvmdRadTh6();
	afx_msg void OnAdvmdRadTh7();
	afx_msg void OnAdvmdRadTh8();
	afx_msg void OnAdvmdRadNumChkItem0();
	afx_msg void OnAdvmdRadNumChkItem1();
	afx_msg void OnAdvmdRadNumChkItem2();
	afx_msg void OnAdvmdRadNumChkItem3();
	afx_msg void OnAdvmdRadCurChkItem0();
	afx_msg void OnAdvmdRadCurChkItem1();
	afx_msg void OnAdvmdRadCurChkItem2();
	afx_msg void OnAdvmdRadCurChkItem3();
	afx_msg void OnAdvmdChBShowGrid();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETTINGMOSAICVMOTION_H__3ADDCFCD_8A87_4621_AA9C_8C5D57827F59__INCLUDED_)
