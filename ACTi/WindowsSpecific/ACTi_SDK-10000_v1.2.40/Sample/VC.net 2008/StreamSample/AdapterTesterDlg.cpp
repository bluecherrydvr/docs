// AdapterTesterDlg.cpp : 實作檔
//

#include "stdafx.h"

#include "AdapterTester.h"
#include "AdapterTesterDlg.h"
#include "StreamingConfig.h"

#ifdef _DEBUG
//#include "vld.h"
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

void CALLBACK DICBEX ( DWORD UserParam, DI_EX_CALLBACK_DATA di )
{
	CAdapterTesterDlg* pd = (CAdapterTesterDlg*)UserParam;
	if( di.DIStatus[0] ) pd->m_bDI1Enable = di.DIStatus[0];
	if( di.DIStatus[1] ) pd->m_bDI2Enable = di.DIStatus[1];

}

void CALLBACK ImgCallback( DWORD UserParam, BYTE* buf, DWORD len, DWORD dwWidth, DWORD dwHeight )
{
	return;
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
	/*
	char  m_szTextLayout[16][256];
	DWORD dwTime = TimeCode;
	if( dwTime == 0 ) dwTime = 1;
	struct tm * newTime;
	newTime = gmtime( (time_t*)&dwTime );
	int h = newTime->tm_sec;
	int nYear = newTime->tm_year + 1900;
	sprintf( m_szTextLayout[1], "%04d\0", nYear );
	while( nYear > 1000 ) nYear -= 1000;
	while( nYear > 100 ) nYear -= 100;
	sprintf( m_szTextLayout[2], "%02d\0", nYear );
	sprintf( m_szTextLayout[3], "%02d\0", newTime->tm_mon + 1 );
	sprintf( m_szTextLayout[4], "%02d\0", newTime->tm_mday );
	sprintf( m_szTextLayout[5], "%02d\0", newTime->tm_hour );
	sprintf( m_szTextLayout[6], "%02d\0", newTime->tm_min );
	sprintf( m_szTextLayout[7], "%02d\0", newTime->tm_sec );

	switch( newTime->tm_wday )
	{
	case 0:
		sprintf( m_szTextLayout[8], "Sun\0" );
		break;
	case 1:
		sprintf( m_szTextLayout[8], "Mon\0" );
		break;
	case 2:
		sprintf( m_szTextLayout[8], "Tue\0" );
		break;
	case 3:
		sprintf( m_szTextLayout[8], "Wen\0" );
		break;
	case 4:
		sprintf( m_szTextLayout[8], "Thu\0" );
		break;
	case 5:
		sprintf( m_szTextLayout[8], "Fri\0" );
		break;
	case 6:
		sprintf( m_szTextLayout[8], "Sat\0" );
		break;
	}
	*/
}

void WINAPI QuadMotionDetection( DWORD UserParam, bool Motion1, bool Motion2, bool Motion3, bool Motion4 )
{
	CAdapterTesterDlg* pd = (CAdapterTesterDlg*)UserParam;
	if( Motion1 && pd ) pd->m_bMotion1Enable = Motion1;
	if( Motion2 && pd ) pd->m_bMotion2Enable = Motion2;
	if( Motion3 && pd ) pd->m_bMotion3Enable = Motion3;
	if( Motion4 && pd ) pd->m_bMotion4Enable = Motion4;
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
	, m_channel(1)
	, m_UID(_T("Admin"))
	, m_PWD(_T("123456"))
	, m_MPort(5000)
	, m_RTSPPort(7070)
	, m_RTPVideoTrack(1)
	, m_RTPAudioTrack(0)
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
	, m_M4_X(0)
	, m_M4_Y(0)
	, m_M4_W(0)
	, m_M4_H(0)
	, m_M1_S(0)
	, m_M2_S(0)
	, m_M3_S(0)
	, m_M4_S(0)
	, m_NTSCPAL(_T(""))
	, m_ver(_T(""))
	, m_SID(0)
	, m_ContactTypeIndex(0)
	, m_IsQuadDevice(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	h = 0;
	m_bMotion1Enable = false;
	m_bMotion2Enable = false;
	m_bMotion3Enable = false;
	m_bMotion4Enable = false;
	m_bDI1Enable = false;
	m_bDI2Enable = false;
//	WSADATA wsaData;
//	WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	gPDlg = this;
	m_bMotionEnable = 0;
	
	m_dwTime1 = m_dwTime2 = m_dwTime3 = m_dwTime4 = 0;
	m_dwThreshold1 = m_dwThreshold2 = m_dwThreshold3 = m_dwThreshold4 = 0;
	m_bEnable1 = m_bEnable2 = m_bEnable3 = m_bEnable4 = 0;

	m_QuadDeviceMode = SINGLE_MODE;

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
	DDX_Text(pDX, IDC_EDIT8, m_RTPVideoTrack);
	DDX_Text(pDX, IDC_EDIT9, m_RTPAudioTrack);
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
	DDX_Text(pDX, IDC_EDIT46, m_M4_X);
	DDX_Text(pDX, IDC_EDIT47, m_M4_Y);
	DDX_Text(pDX, IDC_EDIT48, m_M4_W);
	DDX_Text(pDX, IDC_EDIT41, m_M4_H);

	DDX_Control(pDX, IDC_STATIC_M1, m_Color_M1 );
	DDX_Control(pDX, IDC_STATIC_M2, m_Color_M2 );
	DDX_Control(pDX, IDC_STATIC_M3, m_Color_M3 );
	DDX_Control(pDX, IDC_STATIC_M4, m_Color_M4 );
	DDX_Text(pDX, IDC_EDIT38, m_M1_S);
	DDX_Text(pDX, IDC_EDIT39, m_M2_S);
	DDX_Text(pDX, IDC_EDIT40, m_M3_S);
	DDX_Text(pDX, IDC_EDIT49, m_M4_S);
	DDX_Control(pDX, IDC_COMBO3, m_DIDefault);
	DDX_Control(pDX, IDC_STATIC_DI1, m_Color_D1 );
	DDX_Control(pDX, IDC_STATIC_DI2, m_Color_D2 );
	DDX_Control(pDX, IDC_SLIDER2, m_CurrectPos);
	DDX_Text(pDX, IDC_EDIT17, m_NTSCPAL);
	DDX_Control(pDX, IDC_COMBO4, m_TCPType);
	//	DDX_Text(pDX, IDC_EDIT41, m_51CH);
	DDX_Control(pDX, IDC_COMBO5, m_usingse);
	DDX_Text(pDX, IDC_VERSION_INFO, m_ver);
	DDX_Text(pDX, IDC_EDIT45, m_SID);
	DDV_MinMaxUInt(pDX, m_SID, 0, 255);
	DDX_CBIndex(pDX, IDC_COMBO1, m_ContactTypeIndex);
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

	m_contacttype.InsertString( 0, "CONTACT_TYPE_UNICAST_WOC_PREVIEW" );		m_contacttype.SetItemData (0, CONTACT_TYPE_UNICAST_WOC_PREVIEW);
	m_contacttype.InsertString( 1, "CONTACT_TYPE_MULTICAST_WOC_PREVIEW" );		m_contacttype.SetItemData (1, CONTACT_TYPE_MULTICAST_WOC_PREVIEW);
	m_contacttype.InsertString( 2, "CONTACT_TYPE_RTSP_PREVIEW" );				m_contacttype.SetItemData (2, CONTACT_TYPE_RTSP_PREVIEW);
	m_contacttype.InsertString( 3, "CONTACT_TYPE_CONTROL_ONLY" );				m_contacttype.SetItemData (3, CONTACT_TYPE_CONTROL_ONLY);
	m_contacttype.InsertString( 4, "CONTACT_TYPE_UNICAST_PREVIEW" );			m_contacttype.SetItemData (4, CONTACT_TYPE_UNICAST_PREVIEW);
	m_contacttype.InsertString( 5, "CONTACT_TYPE_MULTICAST_PREVIEW" );			m_contacttype.SetItemData (5, CONTACT_TYPE_MULTICAST_PREVIEW);
	m_contacttype.InsertString( 6, "CONTACT_TYPE_PLAYBACK" );					m_contacttype.SetItemData (6, CONTACT_TYPE_PLAYBACK);
	m_contacttype.InsertString( 7, "CONTACT_TYPE_CARD_PREVIEW" );				m_contacttype.SetItemData (7, CONTACT_TYPE_CARD_PREVIEW);

	m_contacttype.InsertString( 8, "CONTACT_TYPE_RTSP_RTPUDP_PREVIEW" );		m_contacttype.SetItemData (8, CONTACT_TYPE_RTSP_RTPUDP_PREVIEW);
	m_contacttype.InsertString( 9, "CONTACT_TYPE_RTSP_RTPUDP_WOC_PREVIEW" );	m_contacttype.SetItemData (9, CONTACT_TYPE_RTSP_RTPUDP_WOC_PREVIEW);

	m_contacttype.InsertString( 10, "CONTACT_TYPE_HTTP_WOC_PREVIEW" );			m_contacttype.SetItemData (10, CONTACT_TYPE_HTTP_WOC_PREVIEW);
	m_contacttype.InsertString( 11, "CONTACT_TYPE_HTTP_PREVIEW" );				m_contacttype.SetItemData (11, CONTACT_TYPE_HTTP_PREVIEW);
	m_contacttype.InsertString( 12, "CONTACT_TYPE_HTTP_CONTROL_ONLY" );			m_contacttype.SetItemData (12, CONTACT_TYPE_HTTP_CONTROL_ONLY);

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
	CFileDialog fd( TRUE, NULL, NULL, OFN_EXPLORER, szFilter, NULL, 0 );

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

	mcc.ContactType		= m_contacttype.GetItemData(m_ContactTypeIndex);
	mcc.ChannelNumber	= m_channel;
	mcc.RegisterPort	= m_RPort;
	mcc.ControlPort		= m_CPort;
	mcc.StreamingPort	= m_VPort;
	mcc.HTTPPort		= m_HPort;
	mcc.MultiCastPort	= m_MPort;
	mcc.RTSPPort		= m_RTSPPort;
	mcc.SearchPortC2S	= 6005;
	mcc.SearchPortS2C	= 6006;
	mcc.ConnectTimeOut	= m_connecttimeout;
	mcc.TCPVideoStreamID = m_SID;
	mcc.RTPVideoTrackNumber = m_RTPVideoTrack;
	mcc.RTPAudioTrackNumber = m_RTPAudioTrack;
	
	sprintf( mcc.PlayFileName,"%s\0", m_filename );

	memcpy( mcc.UserID, m_UID.GetBuffer(), m_UID.GetLength() );
	mcc.UserID[ m_UID.GetLength() ] = 0;
	m_UID.ReleaseBuffer();

	memcpy( mcc.Password, m_PWD.GetBuffer(), m_PWD.GetLength() );
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

	if( h )
		KCloseInterface( h );
	h = KOpenInterface();

	if( h )
	{	
		int tt = CheckDeviceType (h);
		m_IsQuadDevice = tt!=_SINGLE_CHANNEL_VIDEO_SERVER;

		KSetTargetDeviceType( h, (DeviceType)tt);
//		KSetTargetCameraIsQuad( h, true );
//		KSetQuadVideoLossCallback( h, (DWORD)this, QVL ); 
		//KSetPrerecordTime( h, 10 );
		//KSetImageCallback( h, (DWORD)this, ImgCallback );
		KSetDICallback( h, (DWORD)this, DICB );
		//KSetDICallbackEx( h, (DWORD)this, DICBEX );
		
		if (m_IsQuadDevice) {
			m_QuadDeviceMode = AutoDetectQuadMode ();
			KSetQuadMotionDetectionCallback( h, (DWORD)this, QuadMotionDetection );
		}
		else
			KSetMotionDetectionCallback( h, (DWORD)this, MotionDetectionCB );
		
		KSetResolutionChangeCallback( h, (DWORD)this, ResolutionChangeCB );
		KSetNetworkLossCallback( h, (DWORD)this, NetWorkLoss );
		//KSetVideoLossCallback ( h, (DWORD)this, VideoLoss );
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
		if( mcc.ContactType == CONTACT_TYPE_PLAYBACK )
			KSetTimeCodeCallback( h, (DWORD)this, TimeCodeCB );

		//KSetSequenceHeaderChecker( h, true, 1 );
		if( KConnect( h ) )
		{
			MEDIA_MOTION_INFO_EX mmi;
			memset( &mmi, 0, sizeof(MEDIA_MOTION_INFO_EX) );
			//KGetMotionInfo( h, &mmi );
			m_M1_S = mmi.dwSensitive[0];
			m_M1_X = mmi.dwRange[0][0];
			m_M1_Y = mmi.dwRange[0][1];
			m_M1_W = mmi.dwRange[0][2];
			m_M1_H = mmi.dwRange[0][3];
			m_dwTime1 = mmi.dwTime[0];
			m_dwThreshold1 = mmi.dwThreshold[0];
			m_bEnable1 = mmi.bEnable[0];

			m_M2_S = mmi.dwSensitive[1];
			m_M2_X = mmi.dwRange[1][0];
			m_M2_Y = mmi.dwRange[1][1];
			m_M2_W = mmi.dwRange[1][2];
			m_M2_H = mmi.dwRange[1][3];
			m_dwTime2 = mmi.dwTime[1];
			m_dwThreshold2 = mmi.dwThreshold[1];
			m_bEnable2 = mmi.bEnable[1];

			m_M3_S = mmi.dwSensitive[2];
			m_M3_X = mmi.dwRange[2][0];
			m_M3_Y = mmi.dwRange[2][1];
			m_M3_W = mmi.dwRange[2][2];
			m_M3_H = mmi.dwRange[2][3];
			m_dwTime3 = mmi.dwTime[2];
			m_dwThreshold3 = mmi.dwThreshold[2];
			m_bEnable3 = mmi.bEnable[2];

			m_M4_S = mmi.dwSensitive[2];
			m_M4_X = mmi.dwRange[2][0];
			m_M4_Y = mmi.dwRange[2][1];
			m_M4_W = mmi.dwRange[2][2];
			m_M4_H = mmi.dwRange[2][3];
			m_dwTime4 = mmi.dwTime[3];
			m_dwThreshold4 = mmi.dwThreshold[3];
			m_bEnable4 = mmi.bEnable[3];

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
			//KPlay( h );
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
				KSEStartStreaming( h );
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

		Invalidate();
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
	KEnableJitterLessMode( h, false );
	return;
	
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
		KStartRecord( h, m_recordfilename.GetBuffer() );
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
	KEnableJitterLessMode( h, true );
	return;
}

void CAdapterTesterDlg::OnBnClickedCancel6()
{
	
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
	MEDIA_MOTION_INFO_EX mmi;

	if( h )
	{
		if (m_IsQuadDevice && m_QuadDeviceMode==SINGLE_MODE) {
			mmi.dwRangeCount = 1;

			if (1 == ((m_channel==0)?1:m_channel)) {				
				mmi.dwSensitive[0] = m_M1_S;
				mmi.dwRange[0][0] = m_M1_X;
				mmi.dwRange[0][1] = m_M1_Y;
				mmi.dwRange[0][2] = m_M1_W;
				mmi.dwRange[0][3] = m_M1_H;
				mmi.dwTime[0] = m_dwTime1;
				mmi.dwThreshold[0] = m_dwThreshold1;
				mmi.bEnable[0] = m_bEnable1;
			}
			else if (2 == m_channel) {
				mmi.dwSensitive[0] = m_M2_S;
				mmi.dwRange[0][0] = m_M2_X;
				mmi.dwRange[0][1] = m_M2_Y;
				mmi.dwRange[0][2] = m_M2_W;
				mmi.dwRange[0][3] = m_M2_H;
				mmi.dwTime[0] = m_dwTime2;
				mmi.dwThreshold[0] = m_dwThreshold2;
				mmi.bEnable[0] = m_bEnable2;
			}
			else if (3 == m_channel) {
				mmi.dwSensitive[0] = m_M3_S;
				mmi.dwRange[0][0] = m_M3_X;
				mmi.dwRange[0][1] = m_M3_Y;
				mmi.dwRange[0][2] = m_M3_W;
				mmi.dwRange[0][3] = m_M3_H;
				mmi.dwTime[0] = m_dwTime3;
				mmi.dwThreshold[0] = m_dwThreshold3;
				mmi.bEnable[0] = m_bEnable3;

			}
			else if (4 == m_channel) {
				mmi.dwSensitive[0] = m_M4_S;
				mmi.dwRange[0][0] = m_M4_X;
				mmi.dwRange[0][1] = m_M4_Y;
				mmi.dwRange[0][2] = m_M4_W;
				mmi.dwRange[0][3] = m_M4_H;
				mmi.dwTime[0] = m_dwTime4;
				mmi.dwThreshold[0] = m_dwThreshold4;
				mmi.bEnable[0] = m_bEnable4;
			}
		}
		else {
			mmi.dwRangeCount = m_IsQuadDevice?4:3;
			mmi.dwSensitive[0] = m_M1_S;
			mmi.dwRange[0][0] = m_M1_X;
			mmi.dwRange[0][1] = m_M1_Y;
			mmi.dwRange[0][2] = m_M1_W;
			mmi.dwRange[0][3] = m_M1_H;
			mmi.dwTime[0] = m_dwTime1;
			mmi.dwThreshold[0] = m_dwThreshold1;
			mmi.bEnable[0] = m_bEnable1;

			mmi.dwSensitive[1] = m_M2_S;
			mmi.dwRange[1][0] = m_M2_X;
			mmi.dwRange[1][1] = m_M2_Y;
			mmi.dwRange[1][2] = m_M2_W;
			mmi.dwRange[1][3] = m_M2_H;
			mmi.dwTime[1] = m_dwTime2;
			mmi.dwThreshold[1] = m_dwThreshold2;
			mmi.bEnable[1] = m_bEnable2;

			mmi.dwSensitive[2] = m_M3_S;
			mmi.dwRange[2][0] = m_M3_X;
			mmi.dwRange[2][1] = m_M3_Y;
			mmi.dwRange[2][2] = m_M3_W;
			mmi.dwRange[2][3] = m_M3_H;
			mmi.dwTime[2] = m_dwTime3;
			mmi.dwThreshold[2] = m_dwThreshold3;
			mmi.bEnable[2] = m_bEnable3;

			mmi.dwSensitive[3] = m_M4_S;
			mmi.dwRange[3][0] = m_M4_X;
			mmi.dwRange[3][1] = m_M4_Y;
			mmi.dwRange[3][2] = m_M4_W;
			mmi.dwRange[3][3] = m_M4_H;		
			mmi.dwTime[3] = m_dwTime4;
			mmi.dwThreshold[3] = m_dwThreshold4;
			mmi.bEnable[3] = m_bEnable4;
		}

		mmi.dwEnable = m_bMotionEnable;
		KSetMotionInfoEx( h, &mmi );
	}

}

void CAdapterTesterDlg::OnBnClickedButton12()
{
	UpdateData();
	MEDIA_MOTION_INFO_EX mmi;

	if( h )
	{
		KGetMotionInfoEx( h, &mmi );
		
		if (m_IsQuadDevice && m_QuadDeviceMode==SINGLE_MODE) {
			if (1 == ((m_channel==0)?1:m_channel)) {
				m_M1_S = mmi.dwSensitive[0];
				m_M1_X = mmi.dwRange[0][0];
				m_M1_Y = mmi.dwRange[0][1];
				m_M1_W = mmi.dwRange[0][2];
				m_M1_H = mmi.dwRange[0][3];
				m_dwTime1 = mmi.dwTime[0];
				m_dwThreshold1 = mmi.dwThreshold[0];
				m_bEnable1 = mmi.bEnable[0];
			}
			else if (2 == m_channel) {
				m_M2_S = mmi.dwSensitive[0];
				m_M2_X = mmi.dwRange[0][0];
				m_M2_Y = mmi.dwRange[0][1];
				m_M2_W = mmi.dwRange[0][2];
				m_M2_H = mmi.dwRange[0][3];
				m_dwTime2 = mmi.dwTime[0];
				m_dwThreshold2 = mmi.dwThreshold[0];
				m_bEnable2 = mmi.bEnable[0];
			}
			else if (3 == m_channel) {
				m_M3_S = mmi.dwSensitive[0];
				m_M3_X = mmi.dwRange[0][0];
				m_M3_Y = mmi.dwRange[0][1];
				m_M3_W = mmi.dwRange[0][2];
				m_M3_H = mmi.dwRange[0][3];
				m_dwTime3 = mmi.dwTime[0];
				m_dwThreshold3 = mmi.dwThreshold[0];
				m_bEnable3 = mmi.bEnable[0];
			}
			else if (4 == m_channel) {
				m_M4_S = mmi.dwSensitive[0];
				m_M4_X = mmi.dwRange[0][0];
				m_M4_Y = mmi.dwRange[0][1];
				m_M4_W = mmi.dwRange[0][2];
				m_M4_H = mmi.dwRange[0][3];
				m_dwTime4 = mmi.dwTime[0];
				m_dwThreshold4 = mmi.dwThreshold[0];
			}
		}
		else {
			m_M1_S = mmi.dwSensitive[0];
			m_M1_X = mmi.dwRange[0][0];
			m_M1_Y = mmi.dwRange[0][1];
			m_M1_W = mmi.dwRange[0][2];
			m_M1_H = mmi.dwRange[0][3];
			m_dwTime1 = mmi.dwTime[0];
			m_dwThreshold1 = mmi.dwThreshold[0];
			m_bEnable1 = mmi.bEnable[0];

			m_M2_S = mmi.dwSensitive[1];
			m_M2_X = mmi.dwRange[1][0];
			m_M2_Y = mmi.dwRange[1][1];
			m_M2_W = mmi.dwRange[1][2];
			m_M2_H = mmi.dwRange[1][3];
			m_dwTime2 = mmi.dwTime[1];
			m_dwThreshold2 = mmi.dwThreshold[1];
			m_bEnable2 = mmi.bEnable[1];

			m_M3_S = mmi.dwSensitive[2];
			m_M3_X = mmi.dwRange[2][0];
			m_M3_Y = mmi.dwRange[2][1];
			m_M3_W = mmi.dwRange[2][2];
			m_M3_H = mmi.dwRange[2][3];
			m_dwTime3 = mmi.dwTime[2];
			m_dwThreshold3 = mmi.dwThreshold[2];
			m_bEnable3 = mmi.bEnable[2];

			m_M4_S = mmi.dwSensitive[3];
			m_M4_X = mmi.dwRange[3][0];
			m_M4_Y = mmi.dwRange[3][1];
			m_M4_W = mmi.dwRange[3][2];
			m_M4_H = mmi.dwRange[3][3];
			m_dwTime4 = mmi.dwTime[3];
			m_dwThreshold4 = mmi.dwThreshold[3];
			m_bEnable4 = mmi.bEnable[3];
		}

		m_bMotionEnable = mmi.dwEnable;
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
			if( gPDlg->m_bMotion4Enable ) gPDlg->m_Color_M4.SetBkColor( LIGHTGREEN );
			else gPDlg->m_Color_M4.SetBkColor( GREEN );
			if( gPDlg->m_bDI1Enable ) gPDlg->m_Color_D1.SetBkColor( LIGHTGREEN );
			else gPDlg->m_Color_D1.SetBkColor( GREEN );
			if( gPDlg->m_bDI2Enable ) gPDlg->m_Color_D2.SetBkColor( LIGHTGREEN );
			else gPDlg->m_Color_D2.SetBkColor( GREEN );
			gPDlg->m_bDI1Enable = false;
			gPDlg->m_bDI2Enable = false;
			gPDlg->m_bMotion1Enable = false;
			gPDlg->m_bMotion2Enable = false;
			gPDlg->m_bMotion3Enable = false;
			gPDlg->m_bMotion4Enable = false;
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

bool CAdapterTesterDlg::RunOpenURL (char *SessionName, char *szCommand, bool bCheckStatus, char* szResBuffer, int nBufSize, int *pLen)
{
	bool bRet = false;

	CHttpFile*	pIF = NULL;
	char		cHTTPSessionName[64];

	CInternetSession*	pIS = NULL;

	sprintf( cHTTPSessionName, "%s%d\0", SessionName, rand());
	pIS = new CInternetSession( cHTTPSessionName );

	if( pIS && szCommand )
	{
		try
		{
			pIF = (CHttpFile *)pIS->OpenURL( szCommand, 1, INTERNET_FLAG_TRANSFER_ASCII|INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE );

			if( pIF && bCheckStatus)
			{
				DWORD dwRet = 0;

				pIF->QueryInfoStatusCode( dwRet );
				if( dwRet == HTTP_STATUS_OK )
				{
					if( szResBuffer )
					{
						int nlen = pIF->Read( szResBuffer , nBufSize-1);
						szResBuffer [nlen] = 0;

						if (pLen) {
							*pLen = nlen;
						}
					}
					bRet = true;
				}
			}
		}
		catch (CInternetException *e) 
		{
			e->Delete();
		}
	}

	if( pIF )
	{
		pIF->Close();
		delete pIF;
	}

	if( pIS )
	{
		pIS->Close();
		delete pIS;
	}

	return bRet;
}

int CAdapterTesterDlg::CheckDeviceType (HANDLE sdk)
{
	int hr = _SINGLE_CHANNEL_VIDEO_SERVER;
	char httpcmd[1024];
	char httpresult[1024];

	sprintf( httpcmd, "http://%s:%d/cgi-bin/system?USER=%s&PWD=%s&SYSTEM_INFO", mcc.UniCastIP, mcc.HTTPPort, mcc.UserID, mcc.Password);

	int nlen = 0;
	if (RunOpenURL ("CAdapterTesterDlg", httpcmd, true, httpresult, 1024, &nlen) && nlen) {
		if( memcmp( &httpresult[19], "A4Q", 3 ) == 0 )
			hr = _ACD2000Q_VIDEO_SERVER;
		else
			hr = _SINGLE_CHANNEL_VIDEO_SERVER;
	}

	return hr;
}

int CAdapterTesterDlg::AutoDetectQuadMode ()
{
	QuadDeviceMode hr = SINGLE_MODE;
	char httpcmd[1024];
	char httpresult[1024];

	sprintf( httpcmd, "http://%s:%d/cgi-bin/encoder?USER=%s&PWD=%s&VIDEO_STREAM", mcc.UniCastIP, mcc.HTTPPort, mcc.UserID, mcc.Password);
	
	int nlen = 0;
	if (RunOpenURL ("CAdapterTesterDlg", httpcmd, true, httpresult, 1024, &nlen) && nlen) {
		CString str = httpresult;
		if (str==_T("VIDEO_STREAM='SINGLE'\xa")) 
			hr = SINGLE_MODE;		
		else if (str==_T("VIDEO_STREAM='QUAD'\xa")) 
			hr = QUAD_MODE;		
		else if (str==_T("VIDEO_STREAM='SEQUENTIAL'\xa")) 
			hr = SEQUENTIAL_MODE;		
	}

	return hr;

}