// GoClient.h : main header file for the GOCLIENT application
//

#if !defined(AFX_GOCLIENT_H__91DD3854_E880_49B6_BC91_858288A205A0__INCLUDED_)
#define AFX_GOCLIENT_H__91DD3854_E880_49B6_BC91_858288A205A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CGoClientApp:
// See GoClient.cpp for the implementation of this class
//

class CGoClientApp : public CWinApp
{
public:
	CGoClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGoClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//#####@@@@@
//#define LIVE_DEBUG 1

#ifdef LIVE_DEBUG
extern HANDLE errFid;
extern char* errBuf;
#define errBufSize 2000
extern unsigned long numWritten;
#define WriteBuf do { errBuf[errBufSize-1] = '\0'; WriteFile(errFid, errBuf, strlen(errBuf), &numWritten, NULL); } while (0)
#define WriteErr(s) do { _snprintf(errBuf, errBufSize, (s)); WriteBuf; } while (0)
#define WriteErr2(f,s) do { _snprintf(errBuf, errBufSize, (f), (s)); WriteBuf; } while (0)
#else
#define errBufSize 1
#define WriteErr(s)
#define WriteErr2(f,s)
#endif
//#####@@@@@

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOCLIENT_H__91DD3854_E880_49B6_BC91_858288A205A0__INCLUDED_)
