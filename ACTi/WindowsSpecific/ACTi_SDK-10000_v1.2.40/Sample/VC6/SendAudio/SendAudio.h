// SendAudio.h : main header file for the SENDAUDIO application
//

#if !defined(AFX_SENDAUDIO_H__F6EB3CE9_E490_4B83_868A_34CE2B74072B__INCLUDED_)
#define AFX_SENDAUDIO_H__F6EB3CE9_E490_4B83_868A_34CE2B74072B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSendAudioApp:
// See SendAudio.cpp for the implementation of this class
//

class CSendAudioApp : public CWinApp
{
public:
	CSendAudioApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendAudioApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSendAudioApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDAUDIO_H__F6EB3CE9_E490_4B83_868A_34CE2B74072B__INCLUDED_)
