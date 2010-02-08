// TSearchServerDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "TSearchServer.h"
#include "TSearchServerDlg.h"
#include ".\tsearchserverdlg.h"
#include "ServerSearchV2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 對話方塊資料
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	// 程式碼實作
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


// CTSearchServerDlg 對話方塊



CTSearchServerDlg::CTSearchServerDlg(CWnd* pParent /*=NULL*/)
: CDialog(CTSearchServerDlg::IDD, pParent)
, m_hostname(_T(""))
, m_pid(_T(""))
, m_wip(_T(""))
, m_lip(_T(""))
, m_mip(_T(""))
, m_mac(_T(""))
, m_vt(0)
, m_sv(0)
, m_version(_T(""))
, m_mname(_T(""))
, m_sn(_T(""))
, m_ph(0)
, m_pc2s(0)
, m_ps2c(0)
, m_pc(0)
, m_pv(0)
, m_pm(0)
, m_pr(0)
, m_system(_T(""))
, m_type(_T(""))
, m_channel(0)
, m_mp(_T(""))
, m_aw(_T(""))
, m_at(_T(""))
, m_mt(_T(""))
, m_pt(_T(""))
, m_av(_T(""))
, m_rs(_T(""))
, m_br(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTSearchServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_EDIT1, m_hostname);
	DDX_Text(pDX, IDC_EDIT2, m_pid);
	DDX_Text(pDX, IDC_EDIT3, m_wip);
	DDX_Text(pDX, IDC_EDIT4, m_lip);
	DDX_Text(pDX, IDC_EDIT5, m_mip);
	DDX_Text(pDX, IDC_EDIT6, m_mac);
	DDX_Text(pDX, IDC_EDIT7, m_vt);
	DDX_Text(pDX, IDC_EDIT8, m_sv);
	DDX_Text(pDX, IDC_EDIT9, m_version);
	DDX_Text(pDX, IDC_EDIT10, m_mname);
	DDX_Text(pDX, IDC_EDIT11, m_sn);
	DDX_Text(pDX, IDC_EDIT12, m_ph);
	DDX_Text(pDX, IDC_EDIT13, m_pc2s);
	DDX_Text(pDX, IDC_EDIT14, m_ps2c);
	DDX_Text(pDX, IDC_EDIT16, m_pc);
	DDX_Text(pDX, IDC_EDIT17, m_pv);
	DDX_Text(pDX, IDC_EDIT18, m_pm);
	DDX_Text(pDX, IDC_EDIT15, m_pr);
	DDX_Text(pDX, IDC_EDIT19, m_system);
	DDX_Text(pDX, IDC_EDIT20, m_type);
	DDX_Text(pDX, IDC_EDIT21, m_channel);
	DDX_Text(pDX, IDC_EDIT22, m_mp);
	DDX_Text(pDX, IDC_EDIT23, m_aw);
	DDX_Text(pDX, IDC_EDIT24, m_at);
	DDX_Text(pDX, IDC_EDIT25, m_mt);
	DDX_Text(pDX, IDC_EDIT26, m_pt);
	DDX_Text(pDX, IDC_EDIT27, m_av);
	DDX_Text(pDX, IDC_EDIT28, m_rs);
	DDX_Text(pDX, IDC_EDIT29, m_br);
}

BEGIN_MESSAGE_MAP(CTSearchServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LIST1, OnLbnSelchangeList1)
END_MESSAGE_MAP()


// CTSearchServerDlg 訊息處理常式

BOOL CTSearchServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 將 "關於..." 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
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

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定
	WORD     wVersionRequested;
	WSADATA  wsaData;
	wVersionRequested = MAKEWORD( 2, 0 );
	WSAStartup( wVersionRequested, &wsaData );
	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CTSearchServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，以便繪製圖示。
// 對於使用文件/檢視模式的 MFC 應用程式，框架會自動完成此作業。

void CTSearchServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//當使用者拖曳最小化視窗時，系統呼叫這個功能取得游標顯示。
HCURSOR CTSearchServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

NET_SEARCHSERVER_V2 buf[512];
DWORD dwCnt = 512;
void CTSearchServerDlg::OnBnClickedOk()
{
	GetDlgItem( IDOK )->EnableWindow(FALSE);

	netSearchServer_V2( (char*)buf, &dwCnt );

	char cinfo[128];
	m_list.ResetContent();
	for( DWORD i = 0; i < dwCnt; i++ )
	{
		sprintf( cinfo, "Host Name[%s] Wan IP[%s]  Lan IP[%s]\0", buf[i].szHostName, buf[i].szWanIp, buf[i].szLanIp );
		m_list.InsertString( 0, cinfo );
	}

	sprintf( cinfo, "%d pic. Video Server(IP camera) found !\0", dwCnt );
	AfxMessageBox( cinfo );
	GetDlgItem( IDOK )->EnableWindow();
}


void CTSearchServerDlg::OnLbnSelchangeList1()
{

	m_hostname ="";
	m_pid="";
	m_wip="";
	m_lip="";
	m_mip="";
	m_mac="";
	m_vt = 0;
	m_sv =0;
	m_version="";
	m_mname="";
	m_sn="";
	m_ph=0;
	m_pc2s=0;
	m_ps2c=0;
	m_pc=0;
	m_pv=0;
	m_pm=0;
	m_pr=0;
	m_system="";
	m_type="";
	m_channel=0;
	m_mp="";
	m_aw="";
	m_at="";
	m_mt="";
	m_pt="";
	m_av="";
	m_rs="";
	m_br="";

	int i = dwCnt - m_list.GetCurSel() - 1;
	m_hostname = buf[ i ].szHostName;
	m_pid 	= buf[ i ].szProductID;
	m_wip 	= buf[ i ].szWanIp;
	m_lip 	= buf[ i ].szLanIp;
	m_mip 	= buf[ i ].szMultiCastIp;
	m_mac 	= buf[ i ].szMac;

	m_vt  	= buf[ i ].szVideoType;
	m_sv  	= buf[ i ].szSearchVersion;
	m_version = buf[ i ].szVersion;
	m_mname = buf[ i ].szModelName;
	m_sn = buf[ i ].szSerialNumber;

	m_ph = buf[ i ].wHPort;
	m_pc2s = buf[ i ].wSPortC2S;
	m_ps2c = buf[ i ].wSPortS2C;
	m_pc = buf[ i ].wCPort;
	m_pv = buf[ i ].wVPort;
	m_pm = buf[ i ].wMPort;
	m_pr = buf[ i ].wRPort;



	if( buf[ i ].szSystem == 'E' )
		m_system = "Embedded";
	else if( buf[ i ].szSystem == 'P' )
		m_system = "PC System";
	else 
		m_system = "";

	if( buf[ i ].szType == 'A' )
		m_type = "Encoder";
	else if( buf[ i ].szType == 'B' )
		m_type = "Decoder";
	else 
		m_type = "";

	if( m_system.GetLength() < 1 )
	{
		UpdateData( FALSE );
		return;
	}

	m_channel = buf[ i ].szChannel;

	switch( buf[ i ].szMultiplexing )
	{
	case 'X':
		m_mp = "None";
		break;
	case 'Q':
		m_mp = "Quad";
		break;
	case 'R':
		m_mp = "Rack Mount";
		break;
	case 'B':
		m_mp = "Blade";
		break;
	default:
		m_mp = "";
		break;
	}

	switch( buf[ i ].szAudioWay )
	{
	case 0:
		m_aw = "No Audio";
		break;
	case 1:
		m_aw = "1 way audio";
		break;
	case 2:
		m_aw = "2 way audio";
		break;
	default:
		m_aw = "";
		break;
	}

	switch( buf[ i ].szAudioType )
	{
	case 0:
		m_at = "PCM";
		break;
	case 1:
		m_at = "ADPCM";
		break;
	default:
		m_at = "";
		break;
	}

	switch( buf[ i ].szMotionType )
	{
	case 0:
		m_mt = "WIS";
		break;
	case 1:
		m_mt = "QUAD";
		break;
	default:
		m_mt = "";
		break;
	}

	switch( buf[ i ].szProtocolType )
	{
	case 0:
		m_pt = "Version 1.0";
		break;
	case 1:
		m_pt = "Version 2.0";
		break;
	default:
		m_pt = "";
		break;
	}

	switch( buf[ i ].szAnalogVideo )
	{
	case 'N':
		m_av = "NTSC";
		break;
	case 'P':
		m_av = "PAL";
		break;
	default:
		m_av = "";
		break;
	}

	switch( buf[ i ].szResolution )
	{
	case 0:
		m_rs = "720x480";
		break;
	case 1:
		m_rs = "352x240";
		break;
	case 2:
		m_rs = "160x112";
		break;
	case 3:
		m_rs = "720x576";
		break;
	case 4:
		m_rs = "352x288";
		break;
	case 5:
		m_rs = "176x144";
		break;
	default:
		m_rs = "";
		break;
	}

	switch( buf[ i ].szBitrate )
	{
	case 0:
		m_br = "28K";
		break;
	case 1:
		m_br = "56K";
		break;
	case 2:
		m_br = "128K";
		break;
	case 3:
		m_br = "256K";
		break;
	case 4:
		m_br = "384K";
		break;
	case 5:
		m_br = "500K";
		break;
	case 6:
		m_br = "750K";
		break;
	case 7:
		m_br = "1M";
		break;
	case 8:
		m_br = "1.2M";
		break;
	case 9:
		m_br = "1.5M";
		break;
	case 10:
		m_br = "2M";
		break;
	case 11:
		m_br = "2.5M";
		break;
	case 12:
		m_br = "3M";
		break;
	default:
		m_br = "";
		break;
	}

	UpdateData( FALSE );
}
