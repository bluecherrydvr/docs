// AdapterTesterDlg.cpp : 實作檔
//

#include "stdafx.h"

#include "AdapterTester.h"
#include "AdapterTesterDlg.h"
#include "StreamingConfig.h"
#include "SDK10000.h"
#include ".\adaptertesterdlg.h"


#ifdef _DEBUG
#include "vld.h"
//#include ".\adaptertesterdlg.h"
#define new DEBUG_NEW
#endif

// CAdapterTesterDlg 對話方塊
MEDIA_CONNECTION_CONFIG mcc;
MEDIA_RENDER_INFO mri;
STREAMING_ENGINE_CONFIG sec;

bool gbUsingStreaming = false;
//CFileBuffer g_FB;
bool gbBreak = false;
HANDLE hThread = NULL;
CAdapterTesterDlg* gPDlg = NULL;
bool gbForward = true;


// for missing object

typedef struct
{
	int ScaleMode;
	int ScaleHeight;
	int ScaleWidth;
	BYTE* Picture;
}ImageScaleInfo;

int intRGB;
int intR, intG, intB;
int intNRGB;
int intX, intY;
int intLX, intLY;
int intI;
int intCount;
int intImgWidth, intImgHeight;

int aryFilter[3][3];
long lngTT;
long lngTTO;
BYTE aryImage[1920][1920];
BYTE aryImage2[1920][1920];
BYTE arySobel[1920][1920];

ImageScaleInfo picColor;

CPoint	m_cpStart;
CPoint	m_cpStop;

void CMO( BYTE* buf, DWORD len, DWORD dwWidth, DWORD dwHeight )
{
	BYTE *pRGB;
	intI = 0;

	picColor.ScaleWidth = dwWidth;
	picColor.ScaleHeight = dwHeight;

	DWORD dwidx = 0;
	for( intY = 0; intY < picColor.ScaleHeight; intY++ )
	{
		for( intX = 0; intX < picColor.ScaleWidth; intX++ )
		{
			dwidx = ( intX * 4 ) + ( intY * picColor.ScaleWidth * 4 );
			intR = buf[ dwidx + 2 ];
			intG = buf[ dwidx + 1 ];
			intB = buf[ dwidx + 0 ];
			intNRGB = (intR + intG + intB) / 3;
			aryImage[intX][intY] = intNRGB;
		}
	}

	/* 3 x 3 橫向邊緣運算
	 *		1,  2,  1
	 *		0,  0,  0
	 *	   -1, -2, -1	
	 */
	
	aryFilter[0][0] = 1;
	aryFilter[1][0] = 2;
	aryFilter[2][0] = 1;
	aryFilter[0][1] = 0;
	aryFilter[1][1] = 0;
	aryFilter[2][1] = 0;
	aryFilter[0][2] = -1;
	aryFilter[1][2] = -2;
	aryFilter[2][2] = -1;

	picColor.ScaleMode = 3;

	for( intY = 1; intY <= picColor.ScaleHeight - 2; intY ++ )
	{
		for( intX = 1; intX <= picColor.ScaleWidth - 2; intX++ )
		{
			intNRGB = 0;
			for( intLY = -1; intLY <= 1; intLY++ )
			{
				for( intLX = -1; intLX <= 1; intLX++ )
				{
					intNRGB = intNRGB + (aryImage[ intX + intLX ][ intY + intLY ] * aryFilter[ intLX + 1 ][ intLY + 1 ] );
				}
			}
			intNRGB = abs(intNRGB);
			arySobel[intX][intY] = intNRGB;
			aryImage2[intY][intX] = intNRGB;
		}
	}

	lngTT = 0;
	for( intY = (m_cpStart.y - 94); intY <= (m_cpStop.y - 94); intY ++ )
	{
		for( intX = (m_cpStart.x - 189); intX <= (m_cpStop.x - 189); intX ++ )
		{
			if( arySobel[intX][intY] > 200 )
			{
				lngTT++;
			}
		}
	}
	
	intI = intI + 1;
	char log[32];
	sprintf( log, "%d\n", lngTT );
	OutputDebugString( log );
	if( lngTT < 600 )
	{
		// gone
		Sleep( 100 );
	}
}

void EPTZMove( DWORD dwUserParam, int nXSrc, int nYSrc, int nWidth, int nHeight )
{
	KDigitalPTZTo( (HANDLE)dwUserParam, nXSrc, nYSrc, nWidth, nHeight );
}

void CALLBACK ImgCallback( DWORD UserParam, BYTE* buf, DWORD len, DWORD dwWidth, DWORD dwHeight )
{

	CAdapterTesterDlg *p = (CAdapterTesterDlg*)UserParam;
	LPBITMAPINFO lpl = (LPBITMAPINFO)buf;
/*
	CMO( &buf[44], len-44, dwWidth, dwHeight );

	lpl->bmiHeader.biBitCount = 24;
	lpl->bmiHeader.biSizeImage = dwWidth*dwHeight*3;
	for( int y = 0; y < dwHeight; y++ )
	{
		for( int x = 0; x < dwWidth; x++ )
		{
			buf[ 44 + (y*dwWidth*3) + (x*3) ] = aryImage2[y][x];
			buf[ 44 + (y*dwWidth*3) + (x*3) + 1 ] = aryImage2[y][x];
			buf[ 44 + (y*dwWidth*3) + (x*3) + 2 ] = aryImage2[y][x];
		}	
	}
*/
	if( p->m_oPTZDlg )
		p->m_oPTZDlg->SetImage( buf, len, dwWidth, dwHeight );
	
	lpl->bmiHeader.biBitCount = 32;
	lpl->bmiHeader.biSizeImage = dwWidth*dwHeight*4;
}

void CALLBACK QVL( DWORD UserParam, bool bR1Videoloss, bool bR2Videoloss, bool bR3Videoloss, bool bR4Videoloss )
{
	bool a1 = bR1Videoloss;
	bool a2 = bR2Videoloss;
	bool a3 = bR3Videoloss;
	bool a4 = bR4Videoloss;
	Sleep( 0 );
}

void WINAPI TimeCodeCB(  DWORD UserParam, DWORD TimeCode )
{
	CAdapterTesterDlg* pd = (CAdapterTesterDlg*)UserParam;
	if( TimeCode > 0xffff0000 )
	{
		Sleep(0);
	}
}

void WINAPI MotionDetectionCB( DWORD UserParam, bool Motion1, bool Motion2, bool Motion3 )
{
	CAdapterTesterDlg* pd = (CAdapterTesterDlg*)UserParam;
	if( Motion1 && pd ) pd->m_bMotion1Enable = Motion1;
	if( Motion2 && pd ) pd->m_bMotion2Enable = Motion2;
	if( Motion3 && pd ) pd->m_bMotion3Enable = Motion3;
}

void WINAPI DICB( DWORD UserParam, bool DI1, bool DI2 )
{
	CAdapterTesterDlg* pd = (CAdapterTesterDlg*)UserParam;
	if( DI1 ) pd->m_bDI1Enable = DI1;
	if( DI2 ) pd->m_bDI2Enable = DI2;
}

void WINAPI ResolutionChangeCB( DWORD UserParam, int nResolution )
{
	CAdapterTesterDlg* pd = (CAdapterTesterDlg*)UserParam;
	pd->ResolutionChange( nResolution );	
}

void CALLBACK NetWorkLoss( DWORD UserParam )
{
	Sleep(0);
}

void CALLBACK VideoLoss( DWORD UserParam )
{
	Sleep(0);
}

CAdapterTesterDlg::CAdapterTesterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdapterTesterDlg::IDD, pParent)
	, m_RPort(6000)
	, m_CPort(6001)
	, m_VPort(6002)
	, m_HPort(80)
	, m_channel(0)
	, m_UID(_T("Admin"))
	, m_PWD(_T("123456"))
	, m_MPort(5000)
	, m_RTSPPort(7070)
	, m_VROM(5000)
	, m_AROM(5002)
	, m_SSPort1(6005)
	, m_SSPort2(6006)
	, m_connecttimeout(0)
	, m_filename(_T("c:\\rec.raw"))
	, m_recordfilename(_T("c:\\rec.raw"))
	, m_Brightness(0)
	, m_Contrast(0)
	, m_Saturation(0)
	, m_Hue(0)
	, m_Resolution(0)
	, m_FPS(0)
	, m_Bitrate(0)
	, m_VariableFPS(0)
	, m_M1_X(0)
	, m_M1_Y(0)
	, m_M1_W(0)
	, m_M1_H(0)
	, m_M2_X(0)
	, m_M2_Y(0)
	, m_M2_W(0)
	, m_M2_H(0)
	, m_M3_X(0)
	, m_M3_Y(0)
	, m_M3_W(0)
	, m_M3_H(0)
	, m_M1_S(0)
	, m_M2_S(0)
	, m_M3_S(0)
	, m_NTSCPAL(_T(""))
	, m_ver(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	h = 0;
	m_bMotion1Enable = false;
	m_bMotion2Enable = false;
	m_bMotion3Enable = false;
	m_bDI1Enable = false;
	m_bDI2Enable = false;
//	WSADATA wsaData;
//	WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	gPDlg = this;
	m_bMotionEnable = 0;
	
	m_oPTZDlg = new CEPTZDlg();
}

void CAdapterTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC2, m_pc);
	DDX_Control(pDX, IDC_COMBO1, m_contacttype);
	DDX_Control(pDX, IDC_IPADDRESS1, m_UIP);
	DDX_Text(pDX, IDC_EDIT5, m_RPort);
	DDX_Text(pDX, IDC_EDIT6, m_CPort);
	DDX_Text(pDX, IDC_EDIT7, m_VPort);
	DDX_Text(pDX, IDC_EDIT10, m_HPort);
	DDX_Control(pDX, IDC_IPADDRESS2, m_MIP);
	DDX_Text(pDX, IDC_EDIT1, m_channel);
	DDX_Text(pDX, IDC_EDIT3, m_UID);
	DDX_Text(pDX, IDC_EDIT4, m_PWD);
	DDX_Text(pDX, IDC_EDIT12, m_MPort);
	DDX_Text(pDX, IDC_EDIT13, m_RTSPPort);
	DDX_Text(pDX, IDC_EDIT14, m_VROM);
	DDX_Text(pDX, IDC_EDIT15, m_AROM);
	DDX_Text(pDX, IDC_EDIT8, m_SSPort1);
	DDX_Text(pDX, IDC_EDIT9, m_SSPort2);
	DDX_Text(pDX, IDC_EDIT11, m_connecttimeout);
	DDX_Text(pDX, IDC_EDIT2, m_filename);
	DDX_Control(pDX, IDC_SLIDER1, m_volume);
	DDX_Text(pDX, IDC_EDIT16, m_recordfilename);
	DDX_Control(pDX, IDC_COMBO2, m_PlayRate);
	DDX_Text(pDX, IDC_EDIT18, m_Brightness);
	DDX_Text(pDX, IDC_EDIT19, m_Contrast);
	DDX_Text(pDX, IDC_EDIT20, m_Saturation);
	DDX_Text(pDX, IDC_EDIT21, m_Hue);
	DDX_Text(pDX, IDC_EDIT22, m_Resolution);
	DDX_Text(pDX, IDC_EDIT23, m_FPS);
	DDX_Text(pDX, IDC_EDIT24, m_Bitrate);
	DDX_Text(pDX, IDC_EDIT26, m_VariableFPS);
	DDX_Text(pDX, IDC_EDIT25, m_M1_X);
	DDX_Text(pDX, IDC_EDIT27, m_M1_Y);
	DDX_Text(pDX, IDC_EDIT28, m_M1_W);
	DDX_Text(pDX, IDC_EDIT29, m_M1_H);
	DDX_Text(pDX, IDC_EDIT30, m_M2_X);
	DDX_Text(pDX, IDC_EDIT31, m_M2_Y);
	DDX_Text(pDX, IDC_EDIT32, m_M2_W);
	DDX_Text(pDX, IDC_EDIT33, m_M2_H);
	DDX_Text(pDX, IDC_EDIT34, m_M3_X);
	DDX_Text(pDX, IDC_EDIT35, m_M3_Y);
	DDX_Text(pDX, IDC_EDIT36, m_M3_W);
	DDX_Text(pDX, IDC_EDIT37, m_M3_H);

	DDX_Control(pDX, IDC_STATIC_M1, m_Color_M1 );
	DDX_Control(pDX, IDC_STATIC_M2, m_Color_M2 );
	DDX_Control(pDX, IDC_STATIC_M3, m_Color_M3 );
	DDX_Text(pDX, IDC_EDIT38, m_M1_S);
	DDX_Text(pDX, IDC_EDIT39, m_M2_S);
	DDX_Text(pDX, IDC_EDIT40, m_M3_S);
	DDX_Control(pDX, IDC_COMBO3, m_DIDefault);
	DDX_Control(pDX, IDC_STATIC_DI1, m_Color_D1 );
	DDX_Control(pDX, IDC_STATIC_DI2, m_Color_D2 );
	DDX_Control(pDX, IDC_SLIDER2, m_CurrectPos);
	DDX_Text(pDX, IDC_EDIT17, m_NTSCPAL);
	DDX_Control(pDX, IDC_COMBO4, m_TCPType);
//	DDX_Text(pDX, IDC_EDIT41, m_51CH);
	DDX_Control(pDX, IDC_COMBO5, m_usingse);
	DDX_Text(pDX, IDC_VERSION_INFO, m_ver);
}

BEGIN_MESSAGE_MAP(CAdapterTesterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnNMReleasedcaptureSlider1)
	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDCANCEL3, OnBnClickedCancel3)
	ON_BN_CLICKED(IDCANCEL4, OnBnClickedCancel4)
	ON_BN_CLICKED(IDCANCEL2, OnBnClickedCancel2)
	ON_BN_CLICKED(IDC_RADIO13, OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO14, OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnBnClickedRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnBnClickedRadio8)
	ON_BN_CLICKED(IDC_BUTTON8, OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, OnBnClickedButton9)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	ON_BN_CLICKED(IDCANCEL5, OnBnClickedCancel5)
	ON_BN_CLICKED(IDCANCEL6, OnBnClickedCancel6)
	ON_BN_CLICKED(IDC_BUTTON11, OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON13, OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON15, OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON17, OnBnClickedButton17)
	ON_BN_CLICKED(IDC_BUTTON19, OnBnClickedButton19)
	ON_BN_CLICKED(IDC_BUTTON21, OnBnClickedButton21)
	ON_BN_CLICKED(IDC_BUTTON23, OnBnClickedButton23)
	ON_BN_CLICKED(IDC_BUTTON25, OnBnClickedButton25)
	ON_BN_CLICKED(IDC_BUTTON10, OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON27, OnBnClickedButton27)
	ON_BN_CLICKED(IDC_BUTTON28, OnBnClickedButton28)
	ON_BN_CLICKED(IDC_BUTTON14, OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON12, OnBnClickedButton12)
	ON_BN_CLICKED(IDC_RADIO10, OnBnClickedRadio10)
	ON_BN_CLICKED(IDC_RADIO11, OnBnClickedRadio11)
	ON_CBN_SELCHANGE(IDC_COMBO3, OnCbnSelchangeCombo3)
	ON_BN_CLICKED(IDC_BUTTON16, OnBnClickedButton16)
	ON_BN_CLICKED(IDC_BUTTON18, OnBnClickedButton18)
	ON_BN_CLICKED(IDC_BUTTON20, OnBnClickedButton20)
	ON_BN_CLICKED(IDC_BUTTON22, OnBnClickedButton22)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER2, OnNMReleasedcaptureSlider2)
	ON_BN_CLICKED(IDC_RADIO15, OnBnClickedRadio15)
	ON_BN_CLICKED(IDC_RADIO23, OnBnClickedRadio23)
	ON_BN_CLICKED(IDC_RADIO24, OnBnClickedRadio24)
	ON_BN_CLICKED(IDC_RADIO53, OnBnClickedRadio53)
	ON_BN_CLICKED(IDC_RADIO54, OnBnClickedRadio54)
	ON_BN_CLICKED(IDC_RADIO55, OnBnClickedRadio55)
	ON_BN_CLICKED(IDC_RADIO56, OnBnClickedRadio56)
	ON_BN_CLICKED(IDC_BUTTON24, OnBnClickedButton24)
	ON_CBN_SELCHANGE(IDC_COMBO5, OnCbnSelchangeCombo5)
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CAdapterTesterDlg 訊息處理常式

BOOL CAdapterTesterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_contacttype.InsertString( 0, "CONTACT_TYPE_UNICAST_WOC_PREVIEW" );
	m_contacttype.InsertString( 1, "CONTACT_TYPE_MULTICAST_WOC_PREVIEW" );
	//m_contacttype.InsertString( 2, "CONTACT_TYPE_RTSP_PREVIEW" );
	m_contacttype.InsertString( 2, "--------------------------" );
	m_contacttype.InsertString( 3, "CONTACT_TYPE_CONTROL_ONLY" );
	m_contacttype.InsertString( 4, "CONTACT_TYPE_UNICAST_PREVIEW" );
	m_contacttype.InsertString( 5, "CONTACT_TYPE_MULTICAST_PREVIEW" );
	m_contacttype.InsertString( 6, "CONTACT_TYPE_PLAYBACK" );
	m_contacttype.InsertString( 7, "CONTACT_TYPE_CARD_PREVIEW" );

	m_PlayRate.InsertString( 0, "0.5" );
	m_PlayRate.InsertString( 1, "1.0" );
	m_PlayRate.InsertString( 2, "2.0" );
	m_PlayRate.InsertString( 3, "4.0" );
	m_PlayRate.InsertString( 4, "8.0" );

	m_PlayRate.SetCurSel(1);
	m_contacttype.SetCurSel(0);
	m_volume.SetRange( 0, 100 );
//	m_UIP.SetAddress( 218, 219, 242, 82 );
	m_UIP.SetAddress( 172, 16, 1, 42 ); 
	//m_UIP.SetAddress( 192, 168, 2, 123 ); 
	//m_UIP.SetAddress( 172, 16, 3, 15 );
	m_MIP.SetAddress( 228, 5, 6, 1 );
	
	CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1 );
	CheckRadioButton( IDC_RADIO13, IDC_RADIO15, IDC_RADIO13 );
	CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO5 );
	CheckRadioButton( IDC_RADIO7, IDC_RADIO8, IDC_RADIO7 );
	CheckRadioButton( IDC_RADIO23, IDC_RADIO24, IDC_RADIO23 );
	CheckRadioButton( IDC_RADIO55, IDC_RADIO56, IDC_RADIO55 );
	GetDlgItem( IDC_VOLUME )->SetWindowText( "Volume :" );
	GetDlgItem( IDC_STATIC_Audio )->SetWindowText( "Audio transferability Off" );

	m_TCPType.InsertString( 0, "Auto Select By UniCast IP" );
	m_TCPType.InsertString( 1, "TCP 1.0" );
	m_TCPType.InsertString( 2, "TCP 2.0" );
	m_TCPType.SetCurSel(0);

	m_DIDefault.InsertString( 0, "0x00" );
	m_DIDefault.InsertString( 1, "0x03" );
	SetTimer( 1001, 300, NULL );

	m_usingse.InsertString( 0, "Not using Streaming Engine" );
	m_usingse.InsertString( 1, "Using Streaming Engine" );
	m_usingse.SetCurSel(0);

	CheckRadioButton( IDC_RADIO53, IDC_RADIO54, IDC_RADIO53 );

	char verinfo[32];

	KGetVersion( verinfo );
	m_ver = verinfo;
	UpdateData(FALSE);

	/*
	SetWindowLong( this->GetSafeHwnd(), GWL_EXSTYLE,
	GetWindowLong( this->GetSafeHwnd(), GWL_EXSTYLE) ^ 0x80000 );
	HINSTANCE hInst = LoadLibrary("User32.DLL"); 
	if(hInst) 
	{ 
		typedef BOOL (WINAPI *MYFUNC)(HWND,COLORREF,BYTE,DWORD); 
		MYFUNC fun = NULL;

		fun=(MYFUNC)GetProcAddress(hInst, "SetLayeredWindowAttributes");
		if(fun)fun(this->GetSafeHwnd(),0,(256*0.9),2); 
		FreeLibrary(hInst); 
	}
	*/
	return TRUE;
}

void CAdapterTesterDlg::OnPaint() 
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
HCURSOR CAdapterTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAdapterTesterDlg::OnBnClickedOk()
{
	if( h )
		KPlay( h );
}

void CAdapterTesterDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	
//	WSACleanup();
	CDialog::OnClose();
}

void CAdapterTesterDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if( h )
	{
		KSetImageCallback( h, 0, NULL );
		KSetDICallback( h, 0, NULL );
		KSetMotionDetectionCallback( h, 0, NULL );
		KSetNetworkLossCallback( h, 0, NULL );
		KStop( h );
		KStopStreaming( h );
		KDisconnect( h );
		KCloseInterface( h );
		h = 0;
	}
	if( m_oPTZDlg )
	{
		CEPTZDlg *pp = m_oPTZDlg;
		m_oPTZDlg = NULL;
		pp->SetEPTZMoveCallback( 0, NULL);
		delete pp;
	}
	
	OnCancel();
}

void CAdapterTesterDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "Raw Date Files (*.raw)|*.raw|\0";

	//CFileDialog fd( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL,  0 );
	CFileDialog fd( TRUE, NULL, NULL, OFN_EXPLORER, szFilter, NULL );

	if( fd.DoModal() == IDOK )
	{
		sprintf( mcc.PlayFileName, "%s\0", fd.GetPathName() );
		m_filename = mcc.PlayFileName;
	}

	else
	{
		memset( mcc.PlayFileName, 0, 256 );
		m_filename = "";
	}
	UpdateData( FALSE );
}

void CAdapterTesterDlg::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// This feature requires Windows XP or greater.
	// The symbol _WIN32_WINNT must be >= 0x0501.
	// TODO: Add your control notification handler code here
	if( h )
	{
		CString cs;
		int nv =  m_volume.GetPos();
		KSetVolume( h, nv, nv );
		cs.Format( "Volume : %d", m_volume.GetPos() );
		GetDlgItem( IDC_VOLUME )->SetWindowText( cs );
	}
	*pResult = 0;
}

void CAdapterTesterDlg::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	if( h )
		KSetDecodeIFrameOnly( h, false );
}

void CAdapterTesterDlg::OnBnClickedRadio2()
{
	// TODO: Add your control notification handler code here
	if( h )
		KSetDecodeIFrameOnly( h, true );
}

void CAdapterTesterDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	if( h )
		KPause( h );
}
extern "C" __declspec(dllimport) void KSetDebugMessageLevel( HANDLE h, int nDebugLevel );
void CAdapterTesterDlg::OnBnClickedOk2()
{
	//BYTE bbb = 0x03 ^ 0x00;
	UpdateData();

	BYTE bip[4];
	int nLvolume, nRvolume;

	memset( &mcc, 0, sizeof(mcc) );
	m_UIP.GetAddress( bip[0], bip[1], bip[2], bip[3] );
	sprintf( mcc.UniCastIP, "%d.%d.%d.%d\0", bip[0], bip[1], bip[2], bip[3] );

	m_MIP.GetAddress( bip[0], bip[1], bip[2], bip[3] );
	sprintf( mcc.MultiCastIP, "%d.%d.%d.%d\0", bip[0], bip[1], bip[2], bip[3] );

	mcc.ContactType		= m_contacttype.GetCurSel()+1;
	mcc.ChannelNumber	= m_channel;
	mcc.RegisterPort	= m_RPort;
	mcc.ControlPort		= m_CPort;
	mcc.StreamingPort	= m_VPort;
	mcc.HTTPPort		= m_HPort;
	mcc.MultiCastPort	= m_MPort;
	mcc.RTSPPort		= m_RTSPPort;
	mcc.VideoRTPOverMCastPort = m_VROM;
	mcc.VideoRTPOverMCastPort = m_AROM;
	mcc.SearchPortC2S	= m_SSPort1;
	mcc.SearchPortS2C	= m_SSPort2;
	mcc.ConnectTimeOut	= m_connecttimeout;

	sprintf( mcc.PlayFileName,"%s\0", m_filename );

	memcpy( mcc.UserID, m_UID.GetBuffer(m_UID.GetLength()), m_UID.GetLength() );
	mcc.UserID[ m_UID.GetLength() ] = 0;
	m_UID.ReleaseBuffer();

	memcpy( mcc.Password, m_PWD.GetBuffer(m_PWD.GetLength()), m_PWD.GetLength() );
	mcc.Password[ m_PWD.GetLength() ] = 0;
	m_PWD.ReleaseBuffer();

	//if( h )
	//{
	//	//KSetNetworkLossCallback( h, 0, NULL );
	//	KSetDICallback( h, 0, NULL );
	//	KSetMotionDetectionCallback( h, 0, NULL );
	//	KSetResolutionChangeCallback( h, 0, NULL );
	//	KStop( h );
	//	KStopStreaming( h );
	//	KDisconnect( h );
	//	KCloseInterface( h );
	//	h = 0;
	//}

	if( !h )
	h = KOpenInterface();
	//KSetDebugMessageLevel( h, 2 );
	if( h )
	{	
		//KSetQuadSetVideoLossCallback( h, (DWORD)this, QVL ); 
		KSetPrerecordTime( h, 10 );
		KSetImageCallback( h, (DWORD)this, ImgCallback );
		KSetDICallback( h, (DWORD)this, DICB );
		KSetMotionDetectionCallback( h, (DWORD)this, MotionDetectionCB );
		KSetResolutionChangeCallback( h, (DWORD)this, ResolutionChangeCB );
		KSetNetworkLossCallback( h, (DWORD)this, NetWorkLoss );
		KSetVideoLossCallback ( h, (DWORD)this, VideoLoss );
		CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1 );
		CheckRadioButton( IDC_RADIO13, IDC_RADIO15, IDC_RADIO13 );
		CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO5 );
		CheckRadioButton( IDC_RADIO7, IDC_RADIO8, IDC_RADIO7 );
		CheckRadioButton( IDC_RADIO53, IDC_RADIO54, IDC_RADIO53 );
		CheckRadioButton( IDC_RADIO55, IDC_RADIO56, IDC_RADIO55 );
		mri.DrawerInterface = DGDI;
		m_DIDefault.SetCurSel(1);
		KSetDIDefaultValue( h, DI_DEFAULT_IS_HIGH );

		if( !gbUsingStreaming ) 
		{
			int itcptype = m_TCPType.GetCurSel();
			if( itcptype == 0 )
			{
				//MEDIA_PORT_INFO mri;
				//KGetPortInfoByHTTP( h, &mri, mcc.UniCastIP, mcc.HTTPPort, mcc.UserID, mcc.Password );
				itcptype = KGetTCPTypeByHTTP( h, mcc.UniCastIP, mcc.HTTPPort, mcc.UserID, mcc.Password );
				KSetTCPType( h, itcptype );
			}
			else if( itcptype == 1 )KSetTCPType( h, 1 );
			else if( itcptype == 2 )KSetTCPType( h, 2 );
			KSetMediaConfig( h, &mcc );
		}
		else 
			KSetStreamingEngineMediaConfig( h, &sec, &mcc );

		m_CurrectPos.SetPos( 0 );
		m_dwCurrentPos = 0;
		//if( mcc.ContactType == CONTACT_TYPE_PLAYBACK )
			KSetTimeCodeCallback( h, (DWORD)this, TimeCodeCB );

		KSetSequenceHeaderChecker( h, true, 1 );
		if( KConnect( h ) )
		{
			MEDIA_MOTION_INFO mmi;
			memset( &mmi, 0, sizeof(MEDIA_MOTION_INFO) );
			//KGetMotionInfo( h, &mmi );
			m_M1_S = mmi.dwSensitive[0];
			m_M1_X = mmi.dwRange[0][0];
			m_M1_Y = mmi.dwRange[0][1];
			m_M1_W = mmi.dwRange[0][2];
			m_M1_H = mmi.dwRange[0][3];

			m_M2_S = mmi.dwSensitive[1];
			m_M2_X = mmi.dwRange[1][0];
			m_M2_Y = mmi.dwRange[1][1];
			m_M2_W = mmi.dwRange[1][2];
			m_M2_H = mmi.dwRange[1][3];

			m_M3_S = mmi.dwSensitive[2];
			m_M3_X = mmi.dwRange[2][0];
			m_M3_Y = mmi.dwRange[2][1];
			m_M3_W = mmi.dwRange[2][2];
			m_M3_H = mmi.dwRange[2][3];

			m_bMotionEnable = mmi.dwEnable;

			if( m_bMotionEnable == 1 )
				CheckRadioButton( IDC_RADIO10, IDC_RADIO11, IDC_RADIO10 );
			else
				CheckRadioButton( IDC_RADIO10, IDC_RADIO11, IDC_RADIO11 );

			DWORD begt = 0;
			DWORD endt = 0;
			KGetBeginTime( h, begt );
			KGetEndTime( h, endt );
			if( endt - begt > 0 )
				m_CurrectPos.SetRange( 0, endt - begt );

			MEDIA_VIDEO_CONFIG VideoConfig;
			memset( &VideoConfig, 0, sizeof(MEDIA_VIDEO_CONFIG) );
			bool bstatr = KStartStreaming( h );

			if( bstatr )
			{
				/*
				KGetVideoConfig( h, &VideoConfig );
				m_Brightness = VideoConfig.dwVideoBrightness;
				m_Contrast = VideoConfig.dwVideoContrast;
				m_Saturation = VideoConfig.dwVideoSaturation;
				m_Hue = VideoConfig.dwVideoHue;
				m_Resolution = VideoConfig.dwVideoResolution;
				m_FPS = VideoConfig.dwFps;
				m_Bitrate = VideoConfig.dwBitsRate;
				*/

				if( mcc.ContactType != CONTACT_TYPE_CONTROL_ONLY )
					ResolutionChange( m_Resolution );

				if( VideoConfig.dwTvStander == 0 )
					m_NTSCPAL = "NTSC\0";

				else
				if( VideoConfig.dwTvStander == 1 )
					m_NTSCPAL = "PAL\0";

				else
					m_NTSCPAL = "";

				KGetVolume( h, nLvolume, nRvolume );
				m_volume.SetPos( nLvolume );
				CString cs;
				cs.Format( "Volume : %d", nLvolume );
				GetDlgItem( IDC_VOLUME )->SetWindowText( cs );
				KPlay( h );
			}

			else
			{
				if( h )
				{
					KSetTimeCodeCallback( h, 0, NULL );
					KSetDICallback( h, 0, NULL );
					KSetMotionDetectionCallback( h, 0, NULL );
					KStop( h );
					KStopStreaming( h );
					KDisconnect( h );
					KCloseInterface( h );
					h = 0;
				}
			}
		}

		else
		{
			if( h )
			{
				KDisconnect( h );
				KCloseInterface( h );
				h = 0;
			}
		}
	}
	UpdateData(FALSE);
}

void CAdapterTesterDlg::OnBnClickedButton3()
{
	if( h )
	{
		KSetTimeCodeCallback( h, 0, NULL );
		KSetDICallback( h, 0, NULL );
		KSetMotionDetectionCallback( h, 0, NULL );

		KStop( h );
		KStopStreaming( h );
		KDisconnect( h );
		//KCloseInterface( h );
		//h = 0;
	}
}

void CAdapterTesterDlg::ShowDebugMessage( TCHAR *pFormat, ...)
{
	va_list ArgList;
	TCHAR tBuffer[1024];
	TCHAR tBuffer2[2048];
	SYSTEMTIME	lst;

	try
	{
		GetLocalTime( &lst );
		va_start( ArgList, pFormat );
		(void)wvsprintf( tBuffer, pFormat, ArgList );
		va_end( ArgList );
		wsprintf( tBuffer2, "%04d%02d%02d %02d%02d%02d %s \0", lst.wYear, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, tBuffer );
		OutputDebugString( tBuffer2 );
	}

	catch (...) 
	{
		OutputDebugString( "DebugMsg Error\0" );
	}
}

void CAdapterTesterDlg::OnBnClickedCancel3()
{
	char holderip[16];
	if( h )
	{
		if( KGetAudioToken( h, holderip ) )
		{
			if( KStartAudioTransfer( h ) )
			{
				GetDlgItem( IDC_STATIC_Audio )->SetWindowText( "Audio transferability On" );
			}
		}
	}
}

void CAdapterTesterDlg::OnBnClickedCancel4()
{
	if( h )
	{
		KStopAudioTransfer( h );
		KFreeAudioToken( h );
		GetDlgItem( IDC_STATIC_Audio )->SetWindowText( "Audio transferability Off" );
	}
}

void CAdapterTesterDlg::OnBnClickedCancel2()
{
	KDigitalPTZEnable( h, true );
	if( m_oPTZDlg )
	{
		m_oPTZDlg->SetEPTZMoveCallback( (DWORD)h, EPTZMove );
		m_oPTZDlg->DoModal();
	}
	KDigitalPTZEnable( h, false );
	return;

	static bool bb = false;
	if( bb )
	{
		bb = false;
	}
	else
	{
		bb = true;
	}
	if(h) KDigitalPTZEnable( h, bb );
	return;

	static b = false;
	if( h )
	{
		if( b )
			b = false;
		else
			b = true;
		KEnableFullScreen( h, b );
		//KQuadSetMotionDetectionEnable( h, 0x01 );
		//KQuadSetMotionSensitive( h, 1, 0 );
		/*
		char rr[1024];
		DWORD dwrl = 1024;
		char url[] = "http://172.16.1.191/cgi-bin/system?USER=Admin&PWD=123456&SYSTEM_INFO";
		//KSendURLCommand( h, url, strlen(url), rr, dwrl );
		KSendCommandToSE( h, url, strlen(url), rr, dwrl );
		//Firmware Version = A4Q-P0V-V1.04.02-AC

		//KStop( h );
		//KStopStreaming( h );
		//KDisconnect( h );
		KEnableFullScreen( h, true );
		//KSendCommandToStreamingEngine( h, "0006ANET        http://172.16.1.218/A2100?user=Admin&pwd=123456&cid=99\0" );
		*/
	}
}

void CAdapterTesterDlg::OnBnClickedRadio3()
{
	CheckRadioButton( IDC_RADIO13, IDC_RADIO15, IDC_RADIO13 );
	if( h )
		KSetCODECType( h, XVIDCODEC, 0 );
}

void CAdapterTesterDlg::OnBnClickedRadio4()
{
	CheckRadioButton( IDC_RADIO13, IDC_RADIO15, IDC_RADIO14 );
	if( h )
		KSetCODECType( h, WISCODEC, 0 );
}

void CAdapterTesterDlg::OnBnClickedRadio5()
{
	CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO5 );
	if( h )
		KEnableDecoder( h, true );
}

void CAdapterTesterDlg::OnBnClickedRadio6()
{
	CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO6 );
	if( h )
		KEnableDecoder( h, false );
}

void CAdapterTesterDlg::OnBnClickedRadio7()
{
	CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO5 );
	CheckRadioButton( IDC_RADIO7, IDC_RADIO8, IDC_RADIO7 );
	if( h )
	{
		KEnableDecoder( h, true );
		KEnableRender( h, true );
	}
}

void CAdapterTesterDlg::OnBnClickedRadio8()
{
	CheckRadioButton( IDC_RADIO7, IDC_RADIO8, IDC_RADIO8 );
	if( h )
		KEnableRender( h, false );
}

void CAdapterTesterDlg::OnBnClickedButton8()
{
	UpdateData();
	if( h )
		KStartRecord( h, m_recordfilename.GetBuffer(m_recordfilename.GetLength()) );
	m_recordfilename.ReleaseBuffer();
}

void CAdapterTesterDlg::OnBnClickedButton9()
{
	MP4FILE_RECORD_INFO mm;
	if( h )
		KStopRecord( h, &mm );
	Sleep(0);
}

void CAdapterTesterDlg::OnCbnSelchangeCombo2()
{
	// TODO: Add your control notification handler code here
	if( h )
		KSetPlayRate( h, m_PlayRate.GetCurSel() );
}

void CAdapterTesterDlg::OnBnClickedButton5()
{
	if( h )
	{
		gbForward = true;
		KSetPlayDirection( h, true );
	}
}

void CAdapterTesterDlg::OnBnClickedButton4()
{
	if( h )
	{
		gbForward = false;
		KSetPlayDirection( h, false );
	}
}

void CAdapterTesterDlg::OnBnClickedButton7()
{
	if( h )
		KStepNextFrame( h );
}

void CAdapterTesterDlg::OnBnClickedButton6()
{
	if( h )
		KStepPrevFrame( h );
}

void CAdapterTesterDlg::OnBnClickedCancel5()
{
	for( int o=0;o<5;o++)
	{
		for( int i = 0; i < 1280-720; i+=10)
		{
			if( h ) KDigitalPTZTo( h, i, 0, 720, 480 );
			MSG msg;
			if( PeekMessage( &msg, NULL, 0, 0,PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			Sleep( 30 );
		}

		for( i = 0; i < 720-480; i+=10)
		{
			if( h ) KDigitalPTZTo( h, 1280-720, i, 720, 480 );
			MSG msg;
			if( PeekMessage( &msg, NULL, 0, 0,PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			Sleep( 30 );
		}

		for( i = 1280-720; i > 0; i-=10)
		{
			if( h ) KDigitalPTZTo( h, i, 720-480, 720, 480 );
			MSG msg;
			if( PeekMessage( &msg, NULL, 0, 0,PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			Sleep( 30 );
		}

		for( i = 720-480; i > 0; i-=10)
		{
			if( h ) KDigitalPTZTo( h, 0, i, 720, 480 );
			MSG msg;
			if( PeekMessage( &msg, NULL, 0, 0,PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			Sleep( 30 );
		}
	}
	return;

	if( h )
	{
		/*
		BYTE bt  = KGetDIOStatusByHTTP( h, "172.16.1.42", 80, "Admin", "123456" );
		char res[2048];
		DWORD dwlen = 2048;
		CString cmd = "http://172.16.1.42/cgi-bin/mpeg4/?USER=Admin&PWD=123456&VIDEO_STATUS";
		KSendURLCommand( h, cmd.GetBuffer(), cmd.GetLength(), res, dwlen );
		cmd.ReleaseBuffer();
		*/
		//KConnect( h );
		//KStartStreaming( h );
		//KPlay( h );
		//GetDialogItem()

	}
}

void CAdapterTesterDlg::OnBnClickedCancel6()
{
	if( h ) KDigitalPTZTo( h, 50, 50, 150, 150 );
	return;

	CString oStrTemp = "0006ANET        http://172.16.1.197/A2100?user=Admin&pwd=123456&cid=99&ch=01";
	CString oStrTemp2;
	oStrTemp.MakeUpper();
	int n1 = 0, chid = 0;;
	if( ( n1 = oStrTemp.Find("CH=") ) >= 0 )
	{
		oStrTemp2 = oStrTemp.Mid(n1+3);
		chid = atoi( oStrTemp2.GetBuffer(oStrTemp2.GetLength()) );
		oStrTemp2.ReleaseBuffer();
	}
}


void CAdapterTesterDlg::OnBnClickedButton11()
{
	// TODO: Add your control notification handler code here
	MEDIA_VIDEO_CONFIG VideoConfig;
	if( h )
	{
		KGetVideoConfig( h, &VideoConfig );
		m_Brightness = VideoConfig.dwVideoBrightness;
		m_Contrast = VideoConfig.dwVideoContrast;
		m_Saturation = VideoConfig.dwVideoSaturation;
		m_Hue = VideoConfig.dwVideoHue;
		m_Resolution = VideoConfig.dwVideoResolution;
		m_FPS = VideoConfig.dwFps;
		m_Bitrate = VideoConfig.dwBitsRate;

		if( VideoConfig.dwTvStander == 0 )
			m_NTSCPAL = "NTSC\0";

		else
		if( VideoConfig.dwTvStander == 1 )
			m_NTSCPAL = "PAL\0";

		else
			m_NTSCPAL = "";
		UpdateData( FALSE );
	}
}
void CAdapterTesterDlg::OnBnClickedButton13()
{
	UpdateData();
	//m_Brightness
	if( h )
		KSetBrightness( h, m_Brightness );
}

void CAdapterTesterDlg::OnBnClickedButton15()
{
	UpdateData();
	if( h )
		KSetContrast( h, m_Contrast );
}

void CAdapterTesterDlg::OnBnClickedButton17()
{
	UpdateData();
	if( h )
		KSetSaturation( h, m_Saturation );
}

void CAdapterTesterDlg::OnBnClickedButton19()
{
	UpdateData();
	if( h )
		KSetHue( h, m_Hue );
}

void CAdapterTesterDlg::OnBnClickedButton21()
{
	UpdateData();
	if( h )
		KSetResolution( h, m_Resolution );
}

void CAdapterTesterDlg::OnBnClickedButton23()
{
	UpdateData();
	if( h )
		KSetFPS( h, m_FPS );
}

void CAdapterTesterDlg::OnBnClickedButton25()
{
	UpdateData();
	if( h )
		KSetBitRate( h, m_Bitrate );
}

void CAdapterTesterDlg::OnBnClickedButton10()
{
	UpdateData();
	MEDIA_VIDEO_CONFIG VideoConfig;
	if( h )
	{
		VideoConfig.dwVideoBrightness = m_Brightness;
		VideoConfig.dwVideoContrast = m_Contrast;
		VideoConfig.dwVideoSaturation = m_Saturation;
		VideoConfig.dwVideoHue = m_Hue;
		VideoConfig.dwVideoResolution = m_Resolution;
		VideoConfig.dwFps = m_FPS;
		VideoConfig.dwBitsRate = m_Bitrate;
		if( m_NTSCPAL == "NTSC" )
			VideoConfig.dwTvStander = 0;

		else
		if( m_NTSCPAL == "PAL" )
			VideoConfig.dwTvStander = 1;
		KSetVideoConfig( h, &VideoConfig );	
	}
}

void CAdapterTesterDlg::OnBnClickedButton27()
{
	if( h )
		KSaveReboot( h );
}

void CAdapterTesterDlg::OnBnClickedButton28()
{
	UpdateData();
	if( h )
		KSetVariableFPS( h, m_VariableFPS );
}


void CAdapterTesterDlg::OnBnClickedButton14()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	MEDIA_MOTION_INFO mmi;

	if( h )
	{
		
		mmi.dwRangeCount = 3;
		mmi.dwSensitive[0] = m_M1_S;
		mmi.dwRange[0][0] = m_M1_X;
		mmi.dwRange[0][1] = m_M1_Y;
		mmi.dwRange[0][2] = m_M1_W;
		mmi.dwRange[0][3] = m_M1_H;

		mmi.dwSensitive[1] = m_M2_S;
		mmi.dwRange[1][0] = m_M2_X;
		mmi.dwRange[1][1] = m_M2_Y;
		mmi.dwRange[1][2] = m_M2_W;
		mmi.dwRange[1][3] = m_M2_H;

		mmi.dwSensitive[2] = m_M3_S;
		mmi.dwRange[2][0] = m_M3_X;
		mmi.dwRange[2][1] = m_M3_Y;
		mmi.dwRange[2][2] = m_M3_W;
		mmi.dwRange[2][3] = m_M3_H;

		mmi.dwEnable = m_bMotionEnable;
		KSetMotionInfo( h, &mmi );
	}

}

void CAdapterTesterDlg::OnBnClickedButton12()
{
	UpdateData();
	MEDIA_MOTION_INFO mmi;

	if( h )
	{
		KGetMotionInfo( h, &mmi );
		m_M1_S = mmi.dwSensitive[0];
		m_M1_X = mmi.dwRange[0][0];
		m_M1_Y = mmi.dwRange[0][1];
		m_M1_W = mmi.dwRange[0][2];
		m_M1_H = mmi.dwRange[0][3];

		m_M2_S = mmi.dwSensitive[1];
		m_M2_X = mmi.dwRange[1][0];
		m_M2_Y = mmi.dwRange[1][1];
		m_M2_W = mmi.dwRange[1][2];
		m_M2_H = mmi.dwRange[1][3];

		m_M3_S = mmi.dwSensitive[2];
		m_M3_X = mmi.dwRange[2][0];
		m_M3_Y = mmi.dwRange[2][1];
		m_M3_W = mmi.dwRange[2][2];
		m_M3_H = mmi.dwRange[2][3];
	}
	UpdateData( FALSE );
}

void CAdapterTesterDlg::OnBnClickedRadio10()
{
	// TODO: Add your control notification handler code here
	m_bMotionEnable = 1;
	CheckRadioButton( IDC_RADIO10, IDC_RADIO11, IDC_RADIO10 );
}

void CAdapterTesterDlg::OnBnClickedRadio11()
{
	m_bMotionEnable = 0;
	CheckRadioButton( IDC_RADIO10, IDC_RADIO11, IDC_RADIO11 );
}

void CAdapterTesterDlg::OnCbnSelchangeCombo3()
{
	switch( m_DIDefault.GetCurSel() )
	{
	case 0:
		if( h )
			KSetDIDefaultValue( h , DI_DEFAULT_IS_LOW );
		break;
	case 1:
		if( h )
			KSetDIDefaultValue( h , DI_DEFAULT_IS_HIGH );
		break;
	}
}

void CAdapterTesterDlg::OnBnClickedButton16()
{
	if( h )
		KSendDO( h, DO_OUTPUT_1 );
}

void CAdapterTesterDlg::OnBnClickedButton18()
{
	if( h )
		KSendDO( h, DO_OUTPUT_2 );
}

void CAdapterTesterDlg::OnBnClickedButton20()
{
	if( h )
		KSendDO( h, DO_OUTPUT_CLEAN );
}

void CAdapterTesterDlg::OnBnClickedButton22()
{
	if( h )
		KSendDO( h, DO_OUTPUT_BOTH );
}

void CAdapterTesterDlg::OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	//int i = 500;//m_CurrectPos.GetNumTics();

	if( h )
	{
		m_dwCurrentPos = m_CurrectPos.GetPos();
		KPause( h );
		KSetCurrentPosition( h, m_dwCurrentPos );
		KPlay( h );
		//m_CurrectPos.SetPos( m_dwCurrentPos );
	}
	*pResult = 0;
}

void CAdapterTesterDlg::OnBnClickedRadio15()
{
	CheckRadioButton( IDC_RADIO13, IDC_RADIO15, IDC_RADIO15 );
	if( h )
		KSetCODECType( h, IPPCODEC, 0 );
}

void CAdapterTesterDlg::ResolutionChange( int nR )
{
	RECT rc;
	m_pc.GetWindowRect( &rc );

	if( nR == 0 )
	{
		mri.rect.top = 0;
		mri.rect.left = 0;
		mri.rect.right = rc.right - rc.left;
		mri.rect.bottom = rc.bottom - rc.top;
	}

	else
	if( nR == 1 )
	{
		mri.rect.top = ( rc.bottom - rc.top )/2 - 240/2;
		mri.rect.left = ( rc.right-rc.left)/ 2 - 352/2;		
		mri.rect.right = 352 ;
		mri.rect.bottom = 240;
	}

	else
	if( nR == 2 )
	{
		mri.rect.top = ( rc.bottom - rc.top )/2 - 112/2;
		mri.rect.left = ( rc.right-rc.left)/ 2 - 160/2;		
		mri.rect.right = 160;
		mri.rect.bottom = 112;
	}

	else
	if( nR == 3 )
	{
		mri.rect.top = 0;
		mri.rect.left = 0;
		mri.rect.right = rc.right - rc.left;
		mri.rect.bottom = rc.bottom - rc.top;
	}

	if( nR == 4 )
	{
		mri.rect.top = ( rc.bottom - rc.top )/2 - 288/2;
		mri.rect.left = ( rc.right-rc.left)/ 2 - 352/2;		
		mri.rect.right = 352;
		mri.rect.bottom = 288;
	}

	else
	if( nR == 5 )
	{
		mri.rect.top = ( rc.bottom - rc.top )/2 - 144/2;
		mri.rect.left = ( rc.right-rc.left)/ 2 - 176/2;		
		mri.rect.right = 176;
		mri.rect.bottom = 144;
	}

	else
	if( nR == 6 )
	{
		mri.rect.top = ( rc.bottom - rc.top )/2 - 120/2;
		mri.rect.left = ( rc.right-rc.left)/ 2 - 176/2;		
		mri.rect.right = 176;
		mri.rect.bottom = 120;
	}

	else
	if( nR == 64 )
	{
		mri.rect.top = ( rc.bottom - rc.top )/2 - 480/2;
		mri.rect.left = ( rc.right-rc.left)/ 2 - 640/2;		
		mri.rect.right = 640;
		mri.rect.bottom = 480;
	}

	else
	{
		mri.rect.top = 0;
		mri.rect.left = 0;
		mri.rect.right = rc.right - rc.left;
		mri.rect.bottom = rc.bottom - rc.top;
	}
	
	mri.hWnd = m_pc.m_hWnd;
		
	if( h )
		KSetRenderInfo( h, &mri );
	Invalidate();
}
void CAdapterTesterDlg::OnBnClickedRadio23()
{
	CheckRadioButton( IDC_RADIO23, IDC_RADIO24, IDC_RADIO23 );
	m_recordfilename = "c:\\rec.raw";
	if( h )
		KSetFileWriterType( h, FRAW );
	UpdateData( FALSE );
}

void CAdapterTesterDlg::OnBnClickedRadio24()
{
	CheckRadioButton( IDC_RADIO23, IDC_RADIO24, IDC_RADIO24 );
	m_recordfilename = "c:\\rec.avi";
	if( h )
		KSetFileWriterType( h, FAVI );
	UpdateData( FALSE );
}

void CAdapterTesterDlg::OnBnClickedRadio53()
{
	CheckRadioButton( IDC_RADIO53, IDC_RADIO54, IDC_RADIO53 );
	CheckRadioButton( IDC_RADIO55, IDC_RADIO56, IDC_RADIO55 );
	if( h )
		KSetDrawerType( h, DGDI );
}

void CAdapterTesterDlg::OnBnClickedRadio54()
{
	CheckRadioButton( IDC_RADIO53, IDC_RADIO54, IDC_RADIO54 );
	CheckRadioButton( IDC_RADIO55, IDC_RADIO56, IDC_RADIO55 );
	if( h )
		KSetDrawerType( h, DXDRAW );
}

void CAdapterTesterDlg::OnBnClickedRadio55()
{
	CheckRadioButton( IDC_RADIO55, IDC_RADIO56, IDC_RADIO55 );
	if( h )
		KEnableStretchMode( h , true );
	Invalidate();
}

void CAdapterTesterDlg::OnBnClickedRadio56()
{
	CheckRadioButton( IDC_RADIO55, IDC_RADIO56, IDC_RADIO56 );
	if( h )
		KEnableStretchMode( h , false );
	Invalidate();
}


void CAdapterTesterDlg::OnBnClickedButton24()
{
	static bool bmute = false;

	if( bmute )
	{
		if( h )
		{
			KSetMute( h, bmute );
			bmute = false;
		}
	}

	else
	{
		if( h )
		{
			KSetMute( h, bmute );
			bmute = true;
		}
	}
}

void CAdapterTesterDlg::OnCbnSelchangeCombo5()
{
	// TODO: Add your control notification handler code here
	gbUsingStreaming = false;
	if( m_usingse.GetCurSel() == 1 )
	{
		CStreamingConfig sc;
		if( sc.DoModal() == IDOK )
		{
			BYTE IP[4];
			memcpy( IP , &sc.m_IP, 4 );
			sprintf( sec.szServerIP, "%d.%d.%d.%d\0", IP[3], IP[2], IP[1], IP[0] );
			sprintf( sec.szUserID, "%s\0", sc.m_ID );
			sprintf( sec.szUserPwd, "%s\0", sc.m_PW );
			sec.dwControlPort = sc.m_CP;
			sec.dwStreamingPort = sc.m_SP;
			gbUsingStreaming = true;
		}
	}
}

void CAdapterTesterDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if( gPDlg )
	{
		switch( nIDEvent ) 
		{
		case 1001:
			if( gPDlg->m_bMotion1Enable ) gPDlg->m_Color_M1.SetBkColor( LIGHTGREEN );
			else gPDlg->m_Color_M1.SetBkColor( GREEN );
			if( gPDlg->m_bMotion2Enable ) gPDlg->m_Color_M2.SetBkColor( LIGHTGREEN );
			else gPDlg->m_Color_M2.SetBkColor( GREEN );
			if( gPDlg->m_bMotion3Enable ) gPDlg->m_Color_M3.SetBkColor( LIGHTGREEN );
			else gPDlg->m_Color_M3.SetBkColor( GREEN );
			if( gPDlg->m_bDI1Enable ) gPDlg->m_Color_D1.SetBkColor( LIGHTGREEN );
			else gPDlg->m_Color_D1.SetBkColor( GREEN );
			if( gPDlg->m_bDI2Enable ) gPDlg->m_Color_D2.SetBkColor( LIGHTGREEN );
			else gPDlg->m_Color_D2.SetBkColor( GREEN );
			gPDlg->m_bDI1Enable = false;
			gPDlg->m_bDI2Enable = false;
			gPDlg->m_bMotion1Enable = false;
			gPDlg->m_bMotion2Enable = false;
			gPDlg->m_bMotion3Enable = false;
			break;
		default:
			break;
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CAdapterTesterDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CAdapterTesterDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bDraging = true;
	m_cpStart.x = point.x;
	m_cpStart.y = point.y;
	CDialog::OnLButtonDown(nFlags, point);
}

void CAdapterTesterDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if( m_bDraging )
	{
		m_bDraging = false;
		m_cpStop.x = point.x;
		m_cpStop.y = point.y;
	}
	CDialog::OnLButtonUp(nFlags, point);
}
