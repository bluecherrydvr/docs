//
#include "stdafx.h"
#include <stdio.h>
#include "MovPlayer.h"
#include ".\movplayerdlg.h"
#include <initguid.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_PLAYER_EVENT	WM_USER	+ 100
#define ID_GRAPHTIMER 10
 

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
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMovPlayerDlg dialog
CMovPlayerDlg::CMovPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMovPlayerDlg::IDD, pParent),
	m_nPort(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMovPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMovPlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()   
    ON_WM_DESTROY()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	
	ON_MESSAGE(WM_PLAYER_EVENT, OnPlayerEvent)
	ON_BN_CLICKED(IDC_RTP_STREAM, &CMovPlayerDlg::OnBnClickedRtpStream)
END_MESSAGE_MAP()


afx_msg LRESULT CMovPlayerDlg::OnPlayerEvent(WPARAM, LPARAM)
{
#if 0
	DWORD ret = sdvr_player_stop(m_nPort);
	ASSERT(ret == SDVR_PLAYER_NOERROR);
#endif
	UpdateUI();

	return 0;

}

// CMovPlayerDlg message handlers

BOOL CMovPlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	DWORD ret = sdvr_player_init();
	if(ret != SDVR_PLAYER_NOERROR)
	{
		AfxMessageBox("Stretch player init failed", MB_OK);
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CMovPlayerDlg::OnDestroy()
{
#if 0
	KillTimer(ID_GRAPHTIMER);
#endif
    sdvr_player_close_file(m_nPort);
	sdvr_player_release();
}

void CMovPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CMovPlayerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
        sdvr_player_refresh_play(m_nPort);
       
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMovPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#if 0
void CMovPlayerDlg::OnBnClickedPlay()
{
	DWORD ret;

	if (!m_bIsStreaming) {
		ret = sdvr_player_get_total_frames(m_nPort, &m_totalFrames);
		ASSERT(ret == SDVR_PLAYER_NOERROR);
		ret = sdvr_player_get_avg_time_per_frame(m_nPort, &m_avgTimePerFrame);
		ASSERT(ret == SDVR_PLAYER_NOERROR);
		ret = sdvr_player_get_file_time(m_nPort, &m_duration);
		ASSERT(ret == SDVR_PLAYER_NOERROR);
	}

	sdvr_player_state_e state;
	ret = sdvr_player_get_state(m_nPort, &state);
    ASSERT(ret == SDVR_PLAYER_NOERROR);

	ret = sdvr_player_play(m_nPort, GetDlgItem(IDC_VIDWIN)->m_hWnd);
	ASSERT(ret == SDVR_PLAYER_NOERROR);

	UpdateUI();
}
#endif

void CMovPlayerDlg::UpdateUI()
{
	sdvr_player_state_e state;
	sdvr_player_get_state(m_nPort, &state);
}


void CMovPlayerDlg::OnBnClickedRtpStream()
{
	// TODO: Add your control notification handler code here
	CString fileName;
	char szFilters[] =  "SDP Files (*.sdp)|*.sdp|";
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT]; 

	// Create an Open dialog; the default file name extension is ".my".
	CFileDialog fileDlg (TRUE, "sdp", "*.sdp", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters, this);
   
	// Display the file dialog. When user clicks OK, fileDlg.DoModal() 
	// returns IDOK.
	if( fileDlg.DoModal ()==IDOK )
	{
		fileName = fileDlg.GetPathName();
		if(fileName != "") {
            LPSTR strFileName = fileName.GetBuffer(0);
            DWORD ret = sdvr_player_open_sdp_file((char *)strFileName);

            ASSERT(ret == SDVR_PLAYER_NOERROR);
            ret = sdvr_player_register_file_end_msg(m_nPort, m_hWnd, WM_PLAYER_EVENT);
            ASSERT(ret == SDVR_PLAYER_NOERROR);

            UpdateUI();

			GetDlgItem(IDC_RTP_STREAM)->EnableWindow(false);

			_splitpath_s(strFileName, drive, dir, fname, ext);
			GetDlgItem(IDC_SDP_NAME)->SetWindowTextA(fname);

			m_bIsStreaming = true;

			sdvr_player_state_e state;
			ret = sdvr_player_get_state(m_nPort, &state);
			ASSERT(ret == SDVR_PLAYER_NOERROR);

			ret = sdvr_player_play(m_nPort, GetDlgItem(IDC_VIDWIN)->m_hWnd);
			ASSERT(ret == SDVR_PLAYER_NOERROR);

			UpdateUI();
		}
	}
}
