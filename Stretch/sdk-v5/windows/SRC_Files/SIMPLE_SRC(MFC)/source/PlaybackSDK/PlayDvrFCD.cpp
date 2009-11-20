// DvrFCD.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PlayDvrFCD.h"
#include "PlayDvrFCDDlg.h"
#include "Tlhelp32.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDvrFCDApp

BEGIN_MESSAGE_MAP(CDvrFCDApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDvrFCDApp construction

CDvrFCDApp::CDvrFCDApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDvrFCDApp object

CDvrFCDApp theApp;
BOOL        SearchSystemProcessAndTerminate();

// CDvrFCDApp initialization

BOOL CDvrFCDApp::InitInstance()
{
	if (SearchSystemProcessAndTerminate() == TRUE)
	{
		AfxMessageBox("The other process is accessing board, program will close!!", MB_OK);

		return FALSE;
	}
	/*
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	*/

	CDvrFCDDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
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


BOOL SearchSystemProcessAndTerminate()
{
	PROCESSENTRY32  ProcessEntry = {0};
	HANDLE          hProcessSnapshot;
	int             nIndex = 0;
	CString         strText;	

	hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE != hProcessSnapshot)
	{
		ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hProcessSnapshot, &ProcessEntry) == TRUE)
		{
			do
			{
				if((_strnicmp(ProcessEntry.szExeFile, "DVRCP.exe", 
					strlen(ProcessEntry.szExeFile)) == NULL) ||
					(_strnicmp(ProcessEntry.szExeFile, "EncodeDvrFCD.exe", 
					strlen(ProcessEntry.szExeFile)) == NULL) ||
					(_strnicmp(ProcessEntry.szExeFile, "DvrFCD.exe", 
					strlen(ProcessEntry.szExeFile)) == NULL))
				{					
					return TRUE;
				}				

				nIndex++;
				Sleep(20);

			} while (Process32Next(hProcessSnapshot, &ProcessEntry) == TRUE);
		}

		CloseHandle(hProcessSnapshot);
	}

	return FALSE;
}