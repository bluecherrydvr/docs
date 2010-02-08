// VideoWnd.h : main header file for the VIDEOWND DLL
//

#if !defined(AFX_VIDEOWND_H__60F8CDE0_4627_4275_885D_F6D58163FE39__INCLUDED_)
#define AFX_VIDEOWND_H__60F8CDE0_4627_4275_885D_F6D58163FE39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CVideoWndApp
// See VideoWnd.cpp for the implementation of this class
//

class CVideoWndApp : public CWinApp
{
public:
	CVideoWndApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoWndApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CVideoWndApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOWND_H__60F8CDE0_4627_4275_885D_F6D58163FE39__INCLUDED_)
