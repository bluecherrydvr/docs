#if !defined(AFX_INDEXBUTTON_H__49937F30_DE0D_4083_A634_B6255E0ACE8C__INCLUDED_)
#define AFX_INDEXBUTTON_H__49937F30_DE0D_4083_A634_B6255E0ACE8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IndexButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIndexButton window

class CIndexButton : public CButton
{
// Construction
public:
	CIndexButton();

// Attributes
public:
	int m_id;

	void SetID (int nID);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIndexButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIndexButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIndexButton)
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INDEXBUTTON_H__49937F30_DE0D_4083_A634_B6255E0ACE8C__INCLUDED_)
