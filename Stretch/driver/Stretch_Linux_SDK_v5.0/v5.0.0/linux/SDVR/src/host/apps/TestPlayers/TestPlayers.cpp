/************************************************************************
 *
 * Copyright 2008 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/

#include "stdafx.h"
#include "TestPlayers.h"
#include "MainFrm.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestPlayersApp

BEGIN_MESSAGE_MAP(CTestPlayersApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_OPENSTREAM, OnFileOpenStream)
	ON_COMMAND(ID_ACTION_PLAY, OnActionPlay)
	ON_COMMAND(ID_ACTION_STOP, OnActionStop)
	ON_COMMAND(ID_ACTION_PAUSE, OnActionPause)
	ON_COMMAND(ID_ACTION_FASTFWD, OnActionFastFwd)
	ON_COMMAND(ID_ACTION_FASTREV, OnActionFastRew)
	ON_COMMAND(ID_ACTION_STEPFWD, OnActionStepFwd)
	ON_COMMAND(ID_ACTION_STEPBWD, OnActionStepBwd)
	ON_COMMAND(ID_ACTION_CAPTURE, OnActionCapture)
	ON_COMMAND(ID_ACTION_SETDECODE, OnActionDecode)
	ON_COMMAND(ID_ACTION_ZOOM, OnActionZoom)
END_MESSAGE_MAP()


// CTestPlayersApp construction

CTestPlayersApp::CTestPlayersApp()
{
	// Place all significant initialization in InitInstance
	m_iNumPlayers = 4;
	m_ppChilds = NULL;
}


// The one and only CTestPlayersApp object

CTestPlayersApp theApp;

// CTestPlayersApp initialization

BOOL CTestPlayersApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Stretch Demo Player"));
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMDIFrameWnd* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create main MDI frame window
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	// try to load shared MDI menus and accelerator table
	// add additional member variables and load calls for
	//	additional menu types your application may need
	HINSTANCE hInst = AfxGetResourceHandle();
	m_hMDIMenu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_TestPlayersTYPE));
	m_hMDIAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_TestPlayersTYPE));

	DWORD ret = splayer_init();
	if (ret != SPLAYER_NOERROR)	{
		AfxMessageBox("Stretch player init failed", MB_OK);
	}
	m_bInitialized = false;

	// The main window has been initialized, so show and update it
	pFrame->ShowWindow(m_nCmdShow);
	pFrame->UpdateWindow();
	return TRUE;
}


// CTestPlayersApp message handlers
int CTestPlayersApp::ExitInstance() 
{
	// handle additional resources you may have added
	if (m_hMDIMenu != NULL)
		FreeResource(m_hMDIMenu);
	if (m_hMDIAccel != NULL)
		FreeResource(m_hMDIAccel);

	splayer_release();

	return CWinApp::ExitInstance();
}

void CTestPlayersApp::Close()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	for (int i=0; i<m_iNumPlayers; i++) {
		CChildFrame *pChild = (CChildFrame *)pFrame->MDIGetActive();
		pChild->Close();
		pFrame->MDINext();
	}
	if (m_ppChilds != NULL) {
		delete m_ppChilds;
		m_ppChilds = NULL;
	}
	m_bInitialized = false;
}

void CTestPlayersApp::OnFileNew() 
{
	if (m_bInitialized)
		return;

	CNewDialog dlg;
	if (dlg.DoModal() == IDOK) {
		m_iNumPlayers = dlg.GetNumChannles();
	}

	m_ppChilds = new CChildFrame*[m_iNumPlayers];
	if (m_ppChilds == NULL)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	for (int i=0; i<m_iNumPlayers; i++) {
		// create a new MDI child window
		pFrame->CreateNewChild(
			RUNTIME_CLASS(CChildFrame), IDR_TestPlayersTYPE, m_hMDIMenu, m_hMDIAccel);
		CChildFrame *pChild = (CChildFrame *)pFrame->MDIGetActive();
		pChild->SetPortNumber(i);
		m_ppChilds[i] = pChild;
	}
	pFrame->MDITile(MDITILE_HORIZONTAL);
	m_bInitialized = true;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
	CWnd *pCtrl = GetDlgItem(IDC_EDIT1);
	CString info;
	info = info + "Build: " + __DATE__ + ", " + __TIME__;
	pCtrl->SetWindowText(info);
	return CDialog::OnInitDialog();
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CTestPlayersApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// new dialog
CNewDialog::CNewDialog() : CDialog(CNewDialog::IDD)
{
}

void CNewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_iIndex);
}

BOOL CNewDialog::OnInitDialog()
{
	m_iIndex = 0;
	return CDialog::OnInitDialog();
}

int CNewDialog::GetNumChannles()
{
	switch (m_iIndex) {
	case 1:
		return 9;
	case 2:
		return 16;
	default:
		return 4;
	}
}

BEGIN_MESSAGE_MAP(CNewDialog, CDialog)
END_MESSAGE_MAP()


// new dialog
CZoomDialog::CZoomDialog() : CDialog(CZoomDialog::IDD)
{
	m_iWindow = 0;
	m_iLeft = 0;
	m_iTop = 0;
	m_iWidth = 0;
	m_iHeight = 0;
}

void CZoomDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_iLeft);
	DDX_Text(pDX, IDC_EDIT2, m_iTop);
	DDX_Text(pDX, IDC_EDIT3, m_iWidth);
	DDX_Text(pDX, IDC_EDIT4, m_iHeight);
	DDX_Text(pDX, IDC_EDIT5, m_iWindow);
}

BOOL CZoomDialog::OnInitDialog()
{
	return CDialog::OnInitDialog();
}

BEGIN_MESSAGE_MAP(CZoomDialog, CDialog)
END_MESSAGE_MAP()


// CTestPlayersApp message handlers
void CTestPlayersApp::OnFileOpen()
{
	if (!m_bInitialized)
		return;

	CString strFilter = "Raw Files (*.raw)|*.raw|MOV Files (*.mov)|*.mov|MP4 Files (*.mp4)|*.mp4;|All Files (*.*)|*.*||";
	CFileDialog dlg(TRUE, "raw\nmov\nmp4", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
	if (IDOK == dlg.DoModal()) {
		CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
		CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
		if (!pChild->OpenFile(dlg.GetFileName())) {
			CString msg;
			msg.Format("Error opening file");
			::MessageBox(pFrame->m_hWnd, msg, "Error", MB_ICONEXCLAMATION);
		}
	}
}

void CTestPlayersApp::OnFileOpenStream()
{
	if (!m_bInitialized)
		return;

	CString strFilter = "Raw Files (*.raw)|*.raw;|All Files (*.*)|*.*||";
	CFileDialog dlg(TRUE, "raw", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
	if (IDOK == dlg.DoModal()) {
		CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
		CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
		if (!pChild->OpenStream(dlg.GetFileName())) {
		}
	}
}

void CTestPlayersApp::OnActionPlay()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->Play();
}

void CTestPlayersApp::OnActionStop()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->Stop();
}

void CTestPlayersApp::OnActionPause()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->Pause();
}

void CTestPlayersApp::OnActionFastFwd()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->FastForward();
}

void CTestPlayersApp::OnActionFastRew()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->FastRewind();
}

void CTestPlayersApp::OnActionStepFwd()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->StepForward();
}

void CTestPlayersApp::OnActionStepBwd()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->StepBackward();
}

void CTestPlayersApp::OnActionCapture()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->CaptureFrame();
}

void CTestPlayersApp::OnActionDecode()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->ToggleDecode();
}

void CTestPlayersApp::OnActionZoom()
{
	if (!m_bInitialized)
		return;

	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CChildFrame* pChild = (CChildFrame *)pFrame->MDIGetActive();
	pChild->Zoom();
}

void CTestPlayersApp::HandleDecodedFrame(sx_int32 port, splayer_frame_t *frame)
{
	if (!m_bInitialized)
		return;

	for (int i=0; i<m_iNumPlayers; i++) {
		if (m_ppChilds[i]->GetPortNumber() == port) {
			return m_ppChilds[i]->HandleDecodedFrame(frame);
		}
	}
}

void DecodeFrameCallback(sx_int32 port, splayer_frame_t *frame)
{
	theApp.HandleDecodedFrame(port, frame);
}
