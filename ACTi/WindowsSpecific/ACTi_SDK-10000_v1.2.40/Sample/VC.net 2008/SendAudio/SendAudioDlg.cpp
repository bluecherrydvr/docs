// SendAudioDlg.cpp : implementation file
//


#include "stdafx.h"
#include "SendAudio.h"
#include "SendAudioDlg.h"
#include "SDK10000.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSendAudioDlg dialog
#define BLOCK_LEN	4096


CSendAudioDlg::CSendAudioDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendAudioDlg::IDD, pParent)
	, m_Path(_T(""))
	, m_id(_T(""))
	, m_pwd(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSendAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_Addr);
	DDX_Text(pDX, IDC_EDIT1, m_Path);
	DDX_Text(pDX, IDC_EDIT4ID, m_id);
	DDX_Text(pDX, IDC_EDIT4Password, m_pwd);
}

BEGIN_MESSAGE_MAP(CSendAudioDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CSendAudioDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CSendAudioDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSendAudioDlg message handlers

BOOL CSendAudioDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_Addr.SetAddress (172, 16, 1, 123);
	m_id = "Admin";
	m_pwd = "123456";
	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSendAudioDlg::OnPaint() 
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
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSendAudioDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ProcessMessages () 
{
	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void CSendAudioDlg::OnBnClickedOk()
{
	UpdateData (TRUE);

	BYTE addr[4];
	m_Addr.GetAddress (addr[0], addr[1], addr[2], addr[3]);

	char s_Addr[50];

	sprintf (s_Addr, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);

	char szAudioTokenHolder[128];
	ZeroMemory( szAudioTokenHolder, 128 );

	HANDLE h = KOpenInterface(); 

	MEDIA_CONNECTION_CONFIG mcc;
	memset(&mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG)); 
	strcpy(mcc.UniCastIP, s_Addr); 
	mcc.ContactType = CONTACT_TYPE_UNICAST_PREVIEW; 
	mcc.HTTPPort = 80; 
	mcc.RegisterPort = 6000; 
	mcc.ControlPort = 6001;
	mcc.StreamingPort = 6002; 
	mcc.ChannelNumber = 0; 

	strcpy(mcc.MultiCastIP, s_Addr); 
	mcc.MultiCastPort = 5000; 

	strcpy(mcc.Password, m_pwd); 
	strcpy(mcc.UserID, m_id); 

	KSetMediaConfig(h, &mcc); 

	if( h ) { 
		if( KGetAudioToken( h, szAudioTokenHolder ) ) { 

			CFile file;
			if( file.Open( m_Path, CFile::modeRead | CFile::typeBinary ) )
			{           
				BYTE soundData[BLOCK_LEN];
				UINT nLen = BLOCK_LEN;
				while( nLen == BLOCK_LEN)
				{
					ZeroMemory( soundData, BLOCK_LEN );
					nLen = file.Read( soundData, nLen );
					if( h )
						KSendAudio( h, soundData, nLen );
					else 
						break;

					ProcessMessages ();

					Sleep( 280 );
				}
				file.Close();
			}
		} 
	} 


	if( h ) { 
		KFreeAudioToken( h ); 

		KCloseInterface( h );
	}
}

void CSendAudioDlg::OnBnClickedButton1()
{
	CFileDialog cFile (TRUE, "*.wav", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Wave File (*.wav)|*.wav|All File (*.*)|*.*||",this);

	if (cFile.DoModal () == IDOK) {
		m_Path = cFile.GetPathName ();

		UpdateData (FALSE);
	}
}
