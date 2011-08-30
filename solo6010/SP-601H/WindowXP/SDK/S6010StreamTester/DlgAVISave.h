#if !defined(AFX_DLGAVISAVE_H__49879987_BAC5_4231_BEDD_B36710A3DAAD__INCLUDED_)
#define AFX_DLGAVISAVE_H__49879987_BAC5_4231_BEDD_B36710A3DAAD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAVISave.h : header file
//

enum
{
	IDX_OUT_RAW,
	IDX_OUT_VOL_RAW,
	IDX_OUT_AVI,
	NUM_OUT_MODE
};

/////////////////////////////////////////////////////////////////////////////
// CDlgAVISave dialog

class CDlgAVISave : public CDialog
{
// Construction
public:
	CDlgAVISave(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgAVISave)
	enum { IDD = IDD_DLG_AVI_SAVE };
	int		m_radSelCh;
	int		m_radOutMode;
	//}}AFX_DATA
	CButton	m_bufRadCh[MAX_ENC_CHANNEL];


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAVISave)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAVISave)
	afx_msg void OnButMake();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGAVISAVE_H__49879987_BAC5_4231_BEDD_B36710A3DAAD__INCLUDED_)
