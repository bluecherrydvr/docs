// IndexButton.cpp : implementation file
//

#include "stdafx.h"
#include "S6010StreamTester.h"
#include "IndexButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIndexButton

CIndexButton::CIndexButton()
{
	m_id = 0;
}

CIndexButton::~CIndexButton()
{
}


BEGIN_MESSAGE_MAP(CIndexButton, CButton)
	//{{AFX_MSG_MAP(CIndexButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIndexButton message handlers

void CIndexButton::OnClicked() 
{
	// TODO: Add your control notification handler code here
	
	GetParent ()->SendMessage (WM_DOO_INDEX_BUTTON_CLICK, m_id, 0);
}

void CIndexButton::SetID (int nID)
{
	m_id = nID;
}
