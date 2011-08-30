// MemoryMapMaker.h : main header file for the MEMORYMAPMAKER application
//

#if !defined(AFX_MEMORYMAPMAKER_H__7E6DA207_B3C9_4EAB_86DD_CE8FE9D9B532__INCLUDED_)
#define AFX_MEMORYMAPMAKER_H__7E6DA207_B3C9_4EAB_86DD_CE8FE9D9B532__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMemoryMapMakerApp:
// See MemoryMapMaker.cpp for the implementation of this class
//

class CMemoryMapMakerApp : public CWinApp
{
public:
	CMemoryMapMakerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMemoryMapMakerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMemoryMapMakerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMORYMAPMAKER_H__7E6DA207_B3C9_4EAB_86DD_CE8FE9D9B532__INCLUDED_)
