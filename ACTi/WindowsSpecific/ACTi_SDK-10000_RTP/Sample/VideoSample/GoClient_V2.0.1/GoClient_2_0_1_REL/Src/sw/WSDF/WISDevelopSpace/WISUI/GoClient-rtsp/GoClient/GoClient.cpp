// GoClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GoClient.h"
#include "GoClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGoClientApp

BEGIN_MESSAGE_MAP(CGoClientApp, CWinApp)
	//{{AFX_MSG_MAP(CGoClientApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGoClientApp construction

CGoClientApp::CGoClientApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGoClientApp object

CGoClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGoClientApp initialization

//#####@@@@@
HANDLE errFid = NULL;
char* errBuf = NULL;
unsigned long numWritten;
//#####@@@@@
class dummy {
public:
	dummy() {
		if (errBuf == NULL) {
			errBuf = (char*)malloc(errBufSize);
			if (errFid == NULL) errFid = CreateFile("C:\\Finlayson\\err.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		WriteErr("dummy() static constructor\n");
	}
} staticDummy;
//#####@@@@@
BOOL CGoClientApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
#if 0
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif
    CGoClientDlg dlg;
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
