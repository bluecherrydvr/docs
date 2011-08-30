// SOLO6010App.h : main header file for the SOLO6010APP application
//

#if !defined(AFX_SOLO6010APP_H__5963F668_19C0_481D_9757_A15D17E5F4A1__INCLUDED_)
#define AFX_SOLO6010APP_H__5963F668_19C0_481D_9757_A15D17E5F4A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSOLO6010AppApp:
// See SOLO6010App.cpp for the implementation of this class
//

class CSOLO6010AppApp : public CWinApp
{
public:
	CSOLO6010AppApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSOLO6010AppApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSOLO6010AppApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOLO6010APP_H__5963F668_19C0_481D_9757_A15D17E5F4A1__INCLUDED_)
