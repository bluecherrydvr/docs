// MediaConverter v1.5.05.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CMediaConverterApp:
// See MediaConverter v1.5.05.cpp for the implementation of this class
//

class CMediaConverterApp : public CWinApp
{
public:
	CMediaConverterApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};
extern CMediaConverterApp theApp;

