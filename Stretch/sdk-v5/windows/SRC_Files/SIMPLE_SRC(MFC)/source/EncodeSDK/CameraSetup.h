#pragma once
#include "sdvr_sdk.h"
#include "afxwin.h"


// CCameraSetup dialog
class CDvrFCDDlg;

class CCameraSetup : public CDialog
{
	DECLARE_DYNAMIC(CCameraSetup)

public:
	CCameraSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCameraSetup();

	sdvr_chan_def_t		m_chan_def;
	CDvrFCDDlg			*m_pMainDlg;
	int					m_iFrame ;
	int					m_iAverbit;
	int					m_iMaxbit;
	int					m_iQuality;
	int					m_iCKPriEncode;
	int					m_iGOPSize;
	CString				strFormat;
	int					m_iSubEn;
	int					m_iCh;
	int					m_iBitrate;

// Dialog Data
	enum { IDD = IDD_CAMERA_SET_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox			m_cBitRateLocationComboBox;
	CString				m_sFileFullPath;
	afx_msg void OnCbnSelchangeComboBitrate();
	afx_msg void OnEnChangeEditFrameRate();
	afx_msg void OnEnChangeEditAverbit();
	afx_msg void OnEnChangeEditMaxbit();
	afx_msg void OnEnChangeEditQuality();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeEditGop();
	afx_msg void OnStnClickedStaticSave();
	afx_msg void OnEnChangeEditFormat();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
