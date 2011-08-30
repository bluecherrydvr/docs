// MemoryMapMakerDlg.h : header file
//

#if !defined(AFX_MEMORYMAPMAKERDLG_H__0F35F837_C66A_4D36_A9F9_147CDFE570DA__INCLUDED_)
#define AFX_MEMORYMAPMAKERDLG_H__0F35F837_C66A_4D36_A9F9_147CDFE570DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMemoryMapMakerDlg dialog

class CMemoryMapMakerDlg : public CDialog
{
// Construction
public:
	CMemoryMapMakerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMemoryMapMakerDlg)
	enum { IDD = IDD_MEMORYMAPMAKER_DIALOG };
	CComboBox	m_comMP4Enc_numCh;
	CComboBox	m_comMP4Enc_codeBufMul;
	CComboBox	m_comMP4Dec_numCh;
	CComboBox	m_comMP4Dec_codeBufMul;
	CComboBox	m_comJPEGEnc_codeBufMul;
	CComboBox	m_comEncOSD_numCh;
	BOOL	m_chEncOSD_bUseVirtualCh;
	BOOL	m_chMP4Enc_bUseVirtualCh;
	BOOL	m_chMP4Enc_bUseCIFHD1;
	BOOL	m_chMP4Enc_bUseFullD1;
	BOOL	m_chUseEncOSD;
	BOOL	m_chUseG723Enc;
	BOOL	m_chUseJPEGEnc;
	BOOL	m_chUseMP4Dec;
	BOOL	m_chUseVideoMotion;
	BOOL	m_chUseVideoOSG;
	CString	m_edStrMemMap;
	CString	m_staJPEGEnc_szCodeBuf;
	CString	m_staMP4Dec_szCodeBuf;
	CString	m_staMP4Enc_szCodeBuf;
	CString	m_staSzTotalSDRAM;
	//}}AFX_DATA

	int m_MP4Enc_numCh;
	int m_MP4Dec_numCh;
	int m_EncOSD_numCh;
	int m_MP4Enc_codeBufMul;
	int m_MP4Dec_codeBufMul;
	int m_JPEGEnc_codeBufMul;

	CFont m_fntEdMemMap;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMemoryMapMakerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMemoryMapMakerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnChangeSettings();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMORYMAPMAKERDLG_H__0F35F837_C66A_4D36_A9F9_147CDFE570DA__INCLUDED_)
