#pragma once
#include "afxwin.h"


// CDecodeSetupDlg dialog

class CDecodeSetupDlg : public CDialog
{
	DECLARE_DYNAMIC(CDecodeSetupDlg)

	CDvrFCDDlg* m_pMainDlg;

	int			m_iDecodeChIdx;
	int			m_iSelEncodeCHIdx;

	int			m_iTotalHWEncodeChannel;
	int			m_iTotalHWDecodeChannel;

public:
	CDecodeSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDecodeSetupDlg();

// Dialog Data
	enum { IDD = IDD_DECODE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonBrowse();
	BOOL m_bInputType;
	afx_msg void OnBnClickedRadioSourceInput();
	CString m_sFileFullPath;
	CComboBox m_cEncodeChComboBox;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	BOOL m_bDecodeType;
	afx_msg void OnBnClickedRadioDecodeType();
};
