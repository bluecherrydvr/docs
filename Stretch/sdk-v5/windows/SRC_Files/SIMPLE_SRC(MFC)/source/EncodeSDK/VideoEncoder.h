#pragma once
#include "afxwin.h"

// CVideoEncoder dialog

class CDvrFCDDlg;
class CVideoEncoder : public CDialog
{
	DECLARE_DYNAMIC(CVideoEncoder)

public:
	CVideoEncoder(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoEncoder();
	CDvrFCDDlg	*m_pMainDlg;
	int			m_iBoardIndex;
	int			m_iChannelIndex;
	bool		m_bVdoEncoderPri;
	bool		m_bVdoEncoderSec;
	int			m_iFrameRate;
	int			m_iGOP;
	int			m_iAverBitRate;
	int			m_iMaxBitRate;
	int			m_iQuality;
	CString		strFormatEncoding;
	int			m_iSubEncoder;
	int			m_iChEncoding;
	int			m_iSelBitRate;


// Dialog Data
	enum { IDD = IDD_VDO_ENCODER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual				BOOL OnInitDialog();
	CComboBox			m_cBitRateComboBox;

	afx_msg void		OnBnClickedEncoderPrimary();
	afx_msg void		OnBnClickedEncoderSecondary();
	afx_msg void		OnCbnSelchangeComboDecimation();
	afx_msg void		OnEnChangeEditFrame();
	afx_msg void		OnEnChangeEditGopSize();
	afx_msg void		OnCbnSelchangeComboBitrateCtl();
	afx_msg void		OnEnChangeEditAvgBitrate();
	afx_msg void		OnEnChangeEditMaxBitrate();
	afx_msg void		OnEnChangeEditQuality();
public:
	afx_msg void OnEnChangeEditFormatEncode();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
