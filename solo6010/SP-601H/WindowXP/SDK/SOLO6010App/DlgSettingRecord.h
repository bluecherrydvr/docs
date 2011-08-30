#if !defined(AFX_DLGSETTINGRECORD_H__59285EC9_C4F0_472F_8DF5_7127407FFC77__INCLUDED_)
#define AFX_DLGSETTINGRECORD_H__59285EC9_C4F0_472F_8DF5_7127407FFC77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSettingRecord.h : header file
//

const char *const GL_BUF_IMG_SZ[NUM_VIDEO_TYPE][NUM_IMG_SZ] = {
	"704x240 Field",
	"352x240 Field",
	"176x112 Field",
	"704x480 Frame",

	"704x288 Field",
	"352x288 Field",
	"176x144 Field",
	"704x576 Frame",
};

enum
{
	IDX_CBR_BR_VBR,
	IDX_CBR_BR_64K,
	IDX_CBR_BR_128K,
	IDX_CBR_BR_256K,
	IDX_CBR_BR_512K,
	IDX_CBR_BR_1M,
	IDX_CBR_BR_2M,
	IDX_CBR_BR_3M,
	IDX_CBR_BR_4M,
	IDX_CBR_BR_5M,
	IDX_CBR_BR_6M,
	IDX_CBR_BR_7M,
	IDX_CBR_BR_8M,
	IDX_CBR_BR_9M,
	IDX_CBR_BR_10M,
	NUM_CBR_BR
};
const char *const STR_BITRATE_BUF[NUM_CBR_BR] = {
	"VBR",
	"64K",
	"128K",
	"256K",
	"512K",
	"1M",
	"2M",
	"3M",
	"4M",
	"5M",
	"6M",
	"7M",
	"8M",
	"9M",
	"10M",
};
const int BUF_BITRATE_COM_IDX_TO_REAL_VAL[NUM_CBR_BR] = {
	0, // "VBR",
	(64 *1024 /8), // "64K",
	(128 *1024 /8), // "128K",
	(256 *1024 /8), // "256K",
	(512 *1024 /8), // "512K",
	(1 *1024 *1024 /8), // "1M",
	(2 *1024 *1024 /8), // "2M",
	(3 *1024 *1024 /8), // "3M",
	(4 *1024 *1024 /8), // "4M",
	(5 *1024 *1024 /8), // "5M",
	(6 *1024 *1024 /8), // "6M",
	(7 *1024 *1024 /8), // "7M",
	(8 *1024 *1024 /8), // "8M",
	(9 *1024 *1024 /8), // "9M",
	(10 *1024 *1024 /8), // "10M",
};
int GetBitrateComIdxFromRealVal (int nBitrate);

enum
{
	IDX_CBR_BUF_SZ_1_SEC,
	IDX_CBR_BUF_SZ_2_SEC,
	IDX_CBR_BUF_SZ_3_SEC,
	IDX_CBR_BUF_SZ_4_SEC,
	IDX_CBR_BUF_SZ_5_SEC,
	IDX_CBR_BUF_SZ_6_SEC,
	IDX_CBR_BUF_SZ_7_SEC,
	IDX_CBR_BUF_SZ_8_SEC,
	IDX_CBR_BUF_SZ_9_SEC,
	IDX_CBR_BUF_SZ_10_SEC,
	IDX_CBR_BUF_SZ_15_SEC,
	IDX_CBR_BUF_SZ_20_SEC,
	IDX_CBR_BUF_SZ_25_SEC,
	IDX_CBR_BUF_SZ_30_SEC,
	NUM_CBR_BUF_SZ
};
const char *const STR_CBR_BUF_SZ_BUF[NUM_CBR_BUF_SZ] = {
	"1 sec",
	"2 sec",
	"3 sec",
	"4 sec",
	"5 sec",
	"6 sec",
	"7 sec",
	"8 sec",
	"9 sec",
	"10 sec",
	"15 sec",
	"20 sec",
	"25 sec",
	"30 sec",
};
const int BUF_CBR_BUF_SZ_COM_IDX_TO_REAL_VAL[NUM_CBR_BUF_SZ] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30
};
int GetCBR_BufSzComIdxFromRealVal (int nBitrate, int szCBR_Buf);

enum
{
	IDX_MODE_V_CH_REAL,
	IDX_MODE_V_CH_VIRTUAL,
};

const char *const STR_G723_SR[NUM_G723_SR] = {
	"8KHz",
	"16KHz",
};

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingRecord dialog

class CDlgSettingRecord : public CDialog
{
// Construction
public:
	CDlgSettingRecord(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSettingRecord)
	enum { IDD = IDD_DLG_SETTING_RECORD };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// MPEG4[S]
	CComboBox m_comSzImg[NUM_VID_REAL_ENC_CH +1];
	CComboBox m_comQuality[NUM_VID_REAL_ENC_CH +1];
	CComboBox m_comFPS[NUM_VID_REAL_ENC_CH +1];
	BOOL m_chBIBP[NUM_VID_REAL_ENC_CH +1];
	int m_edSzGOP[NUM_VID_REAL_ENC_CH +1];
	BOOL m_chBRec[NUM_VID_REAL_ENC_CH +1];
	CComboBox m_comBR[NUM_VID_REAL_ENC_CH +1];
	CComboBox m_comCBR_Buf[NUM_VID_REAL_ENC_CH +1];
	CButton m_cbutBIBP[NUM_VID_REAL_ENC_CH +1];
	// MPEG4[E]

	// JPEG[S]
	BOOL m_chJPEG_BRec[NUM_VID_REAL_ENC_CH +1];
	// JPEG[E]
	
	// G.723[S]
	CComboBox m_comG723SR;
	BOOL m_chG723BRec[NUM_G723_CH_PAIR];
	// G.723[E]

	SETTING_REC m_tmpSetRec;

	int m_idxModeVideoCh;

	void SetMP4DlgItemFromVideoChMode ();
	BOOL GetMP4RecInfoFromDlgItem ();

	void SetJPEGDlgItemFromVideoChMode ();
	BOOL GetJPEGRecInfoFromDlgItem ();

	void SetG723DlgItem ();
	BOOL GetG723DlgItem ();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSettingRecord)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	afx_msg void OnChangeEdSzGOP();
	// Generated message map functions
	//{{AFX_MSG(CDlgSettingRecord)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChBIbpA();
	afx_msg void OnChBRecA();
	afx_msg void OnSelchangeComBrA();
	afx_msg void OnSelchangeComCbrBufA();
	afx_msg void OnSelchangeComFpsA();
	afx_msg void OnSelchangeComImgQualityA();
	afx_msg void OnSelchangeComSzImgA();
	afx_msg void OnChangeEdMotionThA();
	afx_msg void OnChangeEdSzGopA();
	afx_msg void OnRadRealChannel();
	afx_msg void OnRadVirtChannel();
	afx_msg void OnChBJRecA();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETTINGRECORD_H__59285EC9_C4F0_472F_8DF5_7127407FFC77__INCLUDED_)
