// SOLO6010App.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SOLO6010App.h"
#include "SOLO6010AppDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSOLO6010AppApp

BEGIN_MESSAGE_MAP(CSOLO6010AppApp, CWinApp)
	//{{AFX_MSG_MAP(CSOLO6010AppApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSOLO6010AppApp construction

CSOLO6010AppApp::CSOLO6010AppApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	ResetReport("C:\\SOLO6010AppLog.txt");

	glPDlgApp = NULL;
	glPWndLiveDisp = NULL;
	glPCurSelS6010 = NULL;
	glPCurRecManager = NULL;
	glPCurHWDecManager = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSOLO6010AppApp object

CSOLO6010AppApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSOLO6010AppApp initialization

BOOL CSOLO6010AppApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CSOLO6010AppDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
