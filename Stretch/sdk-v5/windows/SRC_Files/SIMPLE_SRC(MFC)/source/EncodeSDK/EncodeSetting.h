#pragma once
#include "afxwin.h"
#include "sdvr_sdk.h"

// CEncodeSetting dialog

class CDvrFCDDlg;
class CEncodeSetting : public CDialog
{
	DECLARE_DYNAMIC(CEncodeSetting)

public:
	CEncodeSetting(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEncodeSetting();
	CDvrFCDDlg				*m_pMainDlg;
	sdvr_chan_def_t			m_chan_def;
	int						m_iVdoEncoderPri;
	int						m_iAdoEncoder;
	int						m_iPriH264 ;
	int						m_iSecH264 ;
	int						m_iAdoG711 ;
	bool					m_bCheckSecondary[MAX_BOARD_CHANNEL];
	bool					m_bCheckPrimary[MAX_BOARD_CHANNEL];
	bool					m_bCheckPriH264[MAX_BOARD_CHANNEL];
	bool					m_bCheckPriMJPEG[MAX_BOARD_CHANNEL];
	bool					m_bCheckSecH264[MAX_BOARD_CHANNEL];
	bool					m_bCheckSecMJPEG[MAX_BOARD_CHANNEL];
	int						m_iEncodeCh;

// Dialog Data
	enum { IDD = IDD_ENCODER_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCheckPrimaryVdoEncoder();
	afx_msg void OnBnClickedRadioPriH264();
	afx_msg void OnBnClickedRadioPriMjpeg();
	afx_msg void OnBnClickedRadioPriMpeg4();
	afx_msg void OnBnClickedRadioSecH264();
	afx_msg void OnBnClickedRadioSecMjpeg();
	afx_msg void OnBnClickedRadioSecMpeg4();
	afx_msg void OnBnClickedCheckSecondaryVdoEncoder();
	afx_msg void OnBnClickedCheckAdoFormat();
	afx_msg void OnBnClickedRadioG711();
	afx_msg void OnBnClickedRadioG726();
	afx_msg void OnBnClickedOk();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
