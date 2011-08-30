// S6010StreamTester.h : main header file for the S6010STREAMTESTER application
//

#if !defined(AFX_S6010STREAMTESTER_H__E717E0EA_3964_480B_9C41_DECCA000AEBB__INCLUDED_)
#define AFX_S6010STREAMTESTER_H__E717E0EA_3964_480B_9C41_DECCA000AEBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CS6010StreamTesterApp:
// See S6010StreamTester.cpp for the implementation of this class
//

class CS6010StreamTesterApp : public CWinApp
{
public:
	CS6010StreamTesterApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CS6010StreamTesterApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CS6010StreamTesterApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_S6010STREAMTESTER_H__E717E0EA_3964_480B_9C41_DECCA000AEBB__INCLUDED_)
