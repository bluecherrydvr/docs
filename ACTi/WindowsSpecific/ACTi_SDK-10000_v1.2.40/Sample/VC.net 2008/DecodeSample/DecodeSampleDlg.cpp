// DecodeSampleDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "DecodeSample.h"
#include "DecodeSampleDlg.h"
#include <Winsock2.h>
#include ".\decodesampledlg.h"
#include "SDK10000.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// utility function
void SwitchResolution(DWORD& dwWidth, DWORD& dwHeight , unsigned char resolutionInHeader)
{
	switch( resolutionInHeader )
	{
	case 0:
		dwWidth = 720;
		dwHeight = 480;
		break;
	case 1:
		dwWidth = 352;
		dwHeight = 240;
		break;
	case 2:
		dwWidth = 160;
		dwHeight = 112;
		break;
	case 3:
		dwWidth = 720;
		dwHeight = 576;
		break;
	case 4:
		dwWidth = 352;
		dwHeight = 288;
		break;
	case 5:
		dwWidth = 176;
		dwHeight = 144;
		break;
	case 6:
		dwWidth = 176;
		dwHeight = 120;
		break;
	case 64:
	case 192:
		dwWidth = 640;
		dwHeight = 480;
		break;
	case 65:
		dwWidth = 1280;
		dwHeight = 720;
		break;
	case 66:
		dwWidth = 1280;
		dwHeight = 960;
		break;
	case 67:
		dwWidth = 1280;
		dwHeight = 1024;
		break;
	case 68:
		dwWidth = 1920;
		dwHeight = 1080;
		break;
	case 70:
		dwWidth = 320;
		dwHeight = 240;
		break;
	case 71:
		dwWidth = 160;
		dwHeight = 120;
		break;

	default:
		break;
	}

}


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


// CDecodeSampleDlg 對話方塊



CDecodeSampleDlg::CDecodeSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecodeSampleDlg::IDD, pParent)
	, m_cStrUserName(_T(""))
	, m_cStrPwd(_T(""))
	, m_cStrBMPSavePath(_T(""))
	, m_bIOnly(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDecodeSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ccbConnTypeCtrl);
	DDX_Control(pDX, IDC_COMBO2, m_ccbUserTypeCtrl);
	DDX_Control(pDX, IDC_IPADDRESS1, m_cIPACtrl);
	DDX_Text(pDX, IDC_EDIT1, m_cStrUserName);
	DDX_Text(pDX, IDC_EDIT2, m_cStrPwd);
	DDX_Text(pDX, IDC_EDIT3, m_cStrBMPSavePath);
	DDX_Control(pDX, IDC_BUTTON1, m_cBtnSave);
	DDX_Control(pDX, IDOK, m_cBtnStartStop);
	DDX_Control(pDX, IDC_STATIC_VW, m_cCtrlVW);
	DDX_Check(pDX, IDC_CHECK2, m_bIOnly);
}

BEGIN_MESSAGE_MAP(CDecodeSampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
END_MESSAGE_MAP()


// CDecodeSampleDlg 訊息處理常式

BOOL CDecodeSampleDlg::OnInitDialog()
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
	// Init. Connection type control
	m_ccbConnTypeCtrl.InsertString(0, "Unicast with Control");
	m_ccbConnTypeCtrl.SetCurSel(0);

	// Init User type control
	m_ccbUserTypeCtrl.InsertString(0, "Admin");
	m_ccbUserTypeCtrl.SetCurSel(0);

	// Init IP Address control
	DWORD dwIP = inet_addr("172.16.1.0");
	BYTE b1 = dwIP;
	BYTE b2 = dwIP >> 8;
	BYTE b3 = dwIP >> 16;
	BYTE b4 = dwIP >> 24;
	m_cIPACtrl.SetAddress(b1, b2, b3, b4);

	// Init User name
	m_cStrUserName = "Admin";

	// Init Password
	m_cStrPwd = "123456";
	
	// Init path to save BMP file
	m_cStrBMPSavePath = "c:\\";

	// Init button save BMP
	m_cBtnSave.EnableWindow(false);
	m_bSaveBMP = false;



	// Update data
	UpdateData(false);
	
	// Init SDK handle
	hSDK = NULL;
	

	memset(&m_xvidDecHandle, 0, sizeof(m_xvidDecHandle));
	//Move Xvid init to InitXvid() function
	m_bStart = false;
	m_pInBuf = NULL;
	m_pOutBuf = NULL;
	m_pAudioBuf = NULL;

	m_bDebugMessageShow = false;


	m_Bmi = NULL;


	m_pSound = new CSoundPlayer();


	// H.264 part
	m_H264_hinstDecoderAdapter = NULL;

	m_H264_lpfnCInit = NULL;
	m_H264_lpfnCExit = NULL;
	m_H264_lpfnCDecode = NULL;
	m_H264_lpfnXSetChannel = NULL;
	m_H264_lpfnXWriteAudio = NULL;
	m_H264_lpfnXReadAudio = NULL;
	m_H264_lpfnXPlayAudio = NULL;

	//
	m_H264_nDecoderHandle = -1;

	InitH264();


	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}


bool CDecodeSampleDlg::InitH264()
{
	//H264
	m_H264_hinstDecoderAdapter = LoadLibrary( DECODER_IH264 );
	if( m_H264_hinstDecoderAdapter )
	{
		m_H264_lpfnCInit				= (XCInit)	GetProcAddress( m_H264_hinstDecoderAdapter, "XCInit"  );
		m_H264_lpfnCExit				= (XCExit)	GetProcAddress( m_H264_hinstDecoderAdapter, "XCExit"  );
		m_H264_lpfnCDecode				= (XDecode)	GetProcAddress( m_H264_hinstDecoderAdapter, "XDecode" );	
		m_H264_lpfnXSetChannel			= (XSetChannel) GetProcAddress( m_H264_hinstDecoderAdapter, "XSetChannel" );	
		m_H264_lpfnXWriteAudio			= (XWriteAudio)	GetProcAddress( m_H264_hinstDecoderAdapter, "XWriteAudio" );	
		m_H264_lpfnXReadAudio			= (XReadAudio)	GetProcAddress( m_H264_hinstDecoderAdapter, "XReadAudio" );	
		m_H264_lpfnXPlayAudio			= (XPlayAudio)	GetProcAddress( m_H264_hinstDecoderAdapter, "XPlayAudio" );	
	}
	else
	{
		return false;
	}

	m_H264_nDecoderHandle = m_H264_lpfnCInit();

	return true;
}

void CDecodeSampleDlg::ReleaseH264()
{
	if( m_H264_nDecoderHandle >= 0 )
		m_H264_lpfnCExit( m_H264_nDecoderHandle );
	m_H264_nDecoderHandle = -1;
}

void CDecodeSampleDlg::InitXvid( DWORD dwWidth, DWORD dwHeight )
{
	//XVID
	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;
	xvid_gbl_init_t xvid_gbl_init;
	int xvidret;
	memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init));
	memset(&m_xvidDecHandle, 0, sizeof(m_xvidDecHandle));

	m_xvidDecHandle.version = XVID_VERSION;
	m_xvidDecHandle.height = 0;
	m_xvidDecHandle.width = 0;
	xvid_gbl_init.version = XVID_VERSION;
	xvidret = xvid_global(0, XVID_GBL_INIT, &xvid_gbl_init, 0);

	xvidret = xvid_decore(NULL, XVID_DEC_CREATE, &m_xvidDecHandle, NULL);

	mf_PrepareDibBuffer(&m_Bmi, m_dwWidth, m_dwHeight);
}


void CDecodeSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDecodeSampleDlg::OnPaint() 
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
HCURSOR CDecodeSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//#ifndef DATA_TYPE_MPEG4_VIDEO
//#define DATA_TYPE_MPEG4_VIDEO 0x01
//#endif
//#ifndef DATA_TYPE_AUDIO
//#define DATA_TYPE_AUDIO 0x02
//#endif

#define HEAD_MSG_B2_VIDEO_MPEG4  	            0x01
#define HEAD_MSG_B2_AUDIO_8KPCM  	            0x02
#define HEAD_MSG_B2_AUDIO_TIMESTAMP_8KPCM 	0x03
#define HEAD_MSG_B2_VIDEO_MJPEG   	            0x04
#define HEAD_MSG_B2_VIDEO_H264   	            0x05


void WINAPI RAWCB(DWORD UserParam, DWORD dwdataType, BYTE* buf, DWORD len)
{
	CDecodeSampleDlg* pcDSD = (CDecodeSampleDlg*) UserParam;
	switch(dwdataType)
	{
		// MPEG4 Video
	case HEAD_MSG_B2_VIDEO_MPEG4:
		pcDSD->VideoDataHandler(buf, len);
		break;
		// Audio
	case HEAD_MSG_B2_AUDIO_8KPCM:
		pcDSD->AudioDataHandler(buf, len);
		break;
	case HEAD_MSG_B2_AUDIO_TIMESTAMP_8KPCM:
		pcDSD->AudioDataHandler(buf, len);
		break;

	case HEAD_MSG_B2_VIDEO_MJPEG:
		// we do nothing here, this sample ignores M-JPEG
		break;

	case HEAD_MSG_B2_VIDEO_H264:
		{
			LPBITMAPINFO lpBMP = NULL;
			VIDEO_B2_FRAME* pB2_Header = NULL;
			DWORD dwWidth = 0;
			DWORD dwHeight = 0;
			pB2_Header = (VIDEO_B2_FRAME*) buf;
			SwitchResolution( dwWidth, dwHeight, pB2_Header->prdata.resolution);
			lpBMP = pcDSD->m_H264_lpfnCDecode( pcDSD->m_H264_nDecoderHandle, dwWidth, dwHeight, &buf[44], len-44, NULL, NULL, false ); 
			pcDSD->H264VideoDisply(lpBMP);
		}
		break;

	default:
		break;
	}
}

void CDecodeSampleDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	UpdateData();
	
	if(NULL == hSDK)
	{
		hSDK = KOpenInterface();
		KEnableDecoder(hSDK, false);
		KEnableRender(hSDK, false);
		KSetRawDataCallback(hSDK, (DWORD)this, RAWCB);
	}

	if(!m_bStart)
	{
		// Set config file
		MEDIA_CONNECTION_CONFIG mcc;
		memset(&mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
		mcc.RegisterPort = 6000;				// Register port setting
		mcc.ControlPort = 6001;					// Control port setting
		mcc.StreamingPort = 6002;				// Stream(Video) port setting
		mcc.MultiCastPort = 5000;				// Multicast port setting
		mcc.HTTPPort = 80;						// HTTP port setting;
		mcc.ChannelNumber = 0;
		strcpy(mcc.UserID, m_cStrUserName);		// User login name
		strcpy(mcc.Password, m_cStrPwd);		// User password
		mcc.ContactType = CONTACT_TYPE_UNICAST_PREVIEW;	// Connect type, Unicast with control
		DWORD dwUniIP, dwTemp;
		m_cIPACtrl.GetAddress(dwTemp);
		char szIP[16] = {0};
		dwUniIP = ReverseIP(dwTemp);
		TransformIp(szIP, dwUniIP, false);
		strcpy(mcc.UniCastIP, szIP);
		if(KSetMediaConfig(hSDK, &mcc))
		{
			if(KConnect(hSDK))
			{
				if(KStartStreaming(hSDK))
				{
					KPlay(hSDK);
					m_cBtnSave.EnableWindow();
					m_cBtnStartStop.SetWindowText("STOP");
					m_bStart = true;
					UpdateData(false);
					SetCursor(LoadCursor(NULL, IDC_ARROW));
					return;
				}
			}
		}
	}

	if(hSDK)
	{
		KStop(hSDK);
		KStopStreaming(hSDK);
		KDisconnect(hSDK);
		m_cBtnSave.EnableWindow(FALSE);
		m_cBtnStartStop.SetWindowText("START");
		m_bStart = false;
		m_bSaveBMP = false;
	}

	UpdateData(false);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}

DWORD CDecodeSampleDlg::CheckIorP( char* pRawData, int Datalen )
{
	int i = 0;
	char cIorP;
	DWORD dwB6 = 0xB6010000;
	do 
	{
		if( memcmp( &pRawData[i], &dwB6, 4 ) == 0 )
		{
			cIorP = pRawData[i+4];
			cIorP &= 0xC0;
			if( cIorP == 64 )	// PFrame
				return 2;
			if( cIorP == 0 )	// IFrame
				return 1;
		}

	} while( Datalen > i++ );
	return 0;
}

void CDecodeSampleDlg::VideoDataHandler(BYTE* buf, DWORD len)
{
	//b2 header pointer
	VIDEO_B2_FRAME* pB2_Header = NULL;
	pB2_Header = (VIDEO_B2_FRAME*) buf;
	// find resolution
	SwitchResolution( m_dwWidth, m_dwHeight, pB2_Header->prdata.resolution);

	// If it is first b2 frame.. 
	if( m_pInBuf == NULL)
	{
		// Others
		m_pInBuf = new char[m_dwWidth*m_dwHeight*4];
		m_pOutBuf = new char[m_dwWidth*m_dwHeight*4];
		m_pAudioBuf = new char[1024*1000];
		memset(m_pInBuf, 0x00, m_dwWidth*m_dwHeight*4);
		memset(m_pOutBuf, 0x00, m_dwWidth*m_dwHeight*4);
		memset(m_pAudioBuf, 0x00, 1024*1000);

		m_dwBufLen = 0;
		InitXvid(m_dwWidth,m_dwHeight);
	}


	// Reset buffer
	memset(m_pInBuf, 0x00, m_dwWidth*m_dwHeight*4);
	m_dwBufLen = 0;

	// Skip the B2 header
	m_dwBufLen = len - 44;
	memcpy(m_pInBuf, buf + 44, len - 44);
	
	DWORD dwFrameType = CheckIorP((char*)buf, len);

	bool bValid = false;
	CString csFrame;




	if(1 == dwFrameType)
	{
		bValid = true;
		csFrame = "I Frame";
	}
	else if(2 == dwFrameType)
	{
		if(m_bIOnly)
		{
			return;
		}
		bValid = true;
		csFrame = "P Frame";
	}
	else
	{
		csFrame = "Unknow data";
		bValid = false;
	}

	if(m_bDebugMessageShow)
	{
		CString csDebug;
		csDebug.Format("Get %s\n", csFrame);
		OutputDebugString(csDebug);
	}


	if(bValid)
	{
		VideoDisply();
	}
}

void CDecodeSampleDlg::AudioDataHandler(BYTE* buf, DWORD len)
{
	// Audio data handle
	if(m_pSound)
	{
		if( len == 548 || len == 652 || len == 468)
			m_pSound->Write( buf+12, len - 12 );
		else if( len == 564 || len == 668 || len == 484)
			m_pSound->Write( buf+28, len - 28 );
	}
}

void CDecodeSampleDlg::VideoDisply()
{
	// Decode video frame
	LPBITMAPINFOHEADER lpbih;
	xvid_dec_frame_t    xvidDecFrame;
	int iSyncLen = 0;

	xvidDecFrame.output.csp = XVID_CSP_BGR;

	xvidDecFrame.output.plane[0] = m_pOutBuf;
	xvidDecFrame.output.stride[0] = m_dwWidth*3;
	xvidDecFrame.bitstream = m_pInBuf+iSyncLen;
	xvidDecFrame.length = m_dwBufLen-iSyncLen;

	xvidDecFrame.general = XVID_LOWDELAY;
	xvidDecFrame.version = XVID_VERSION;

	int xvidret = xvid_decore(m_xvidDecHandle.handle, XVID_DEC_DECODE, &xvidDecFrame, 0);
	
	// Draw
	CDC* thedc = m_cCtrlVW.GetDC();
	if (!thedc)
		return ;
	m_hDrawDC = thedc->m_hDC;
	RECT rect;
	m_cCtrlVW.GetWindowRect(&rect);
	DWORD dwWidth = rect.right - rect.left, dwHeight = rect.bottom - rect.top;
	lpbih = (LPBITMAPINFOHEADER) m_Bmi;
	::SetStretchBltMode(m_hDrawDC, COLORONCOLOR);
	::StretchDIBits(m_hDrawDC, 0, dwHeight, dwWidth, -dwHeight, 0 ,0, lpbih->biWidth, (lpbih->biHeight), (const void*)m_pOutBuf, m_Bmi, DIB_RGB_COLORS, SRCCOPY);
	m_cCtrlVW.ReleaseDC(thedc);
	
	// Save BMP file
	if (m_bSaveBMP) 
	{
		m_cBtnSave.SetState(m_bSaveBMP = FALSE);
		time_t tTime;
		tTime = time(NULL);
		struct tm *pTmTime = localtime(&tTime);
		TCHAR szFilename[MAX_PATH], szTime[24];
		_tcsftime(szTime, sizeof(szTime), _T("%Y%m%d_%H%M%S"), pTmTime);
		_sntprintf(szFilename, MAX_PATH, _T("%sBitmap%s.bmp\0"), m_cStrBMPSavePath, szTime);

		// Create a file to hold the bitmap
		HANDLE hf = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, 
			NULL, CREATE_ALWAYS, NULL, NULL );
		if (INVALID_HANDLE_VALUE == hf) {
			return ;
		}

		long BufferSize = m_dwWidth*m_dwHeight*3;
		// Write out the file header
		BITMAPFILEHEADER bfh;
		memset( &bfh, 0, sizeof( bfh ) );
		bfh.bfType = 'MB';
		bfh.bfSize = sizeof( bfh ) + BufferSize + sizeof( BITMAPINFOHEADER );
		bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );

		DWORD Written = 0;
		WriteFile( hf, &bfh, sizeof( bfh ), &Written, NULL );

		// Write the bitmap format
		//
		BITMAPINFOHEADER bih;
		memset( &bih, 0, sizeof( bih ) );
		bih.biSize = sizeof( bih );
		bih.biWidth = m_dwWidth;
		bih.biHeight = -m_dwHeight;	//Save from down to up
		bih.biPlanes = 1;
		bih.biBitCount = 24;

		Written = 0;
		WriteFile( hf, &bih, sizeof( bih ), &Written, NULL );

		// Write the bitmap bits
		//
		Written = 0;
		WriteFile( hf, xvidDecFrame.output.plane[0], BufferSize, &Written, NULL );

		CloseHandle( hf );
	}
}

void CDecodeSampleDlg::H264VideoDisply(LPBITMAPINFO lpBMP)
{
	// Draw
	CDC* thedc = m_cCtrlVW.GetDC();
	if (!thedc)
		return ;
	m_hDrawDC = thedc->m_hDC;
	RECT rect;
	m_cCtrlVW.GetWindowRect(&rect);
	DWORD dwWidth = rect.right - rect.left, dwHeight = rect.bottom - rect.top;
	//lpbih = (LPBITMAPINFOHEADER) m_Bmi;
	::SetStretchBltMode(m_hDrawDC, COLORONCOLOR);
	::StretchDIBits(m_hDrawDC, 0, dwHeight, dwWidth, -dwHeight, 0 ,0, abs(lpBMP->bmiHeader.biWidth), abs(lpBMP->bmiHeader.biHeight), (const void*)lpBMP->bmiColors, lpBMP, DIB_RGB_COLORS, SRCCOPY);
	m_cCtrlVW.ReleaseDC(thedc);
	
	// Save BMP file
	if (m_bSaveBMP) 
	{
		m_cBtnSave.SetState(m_bSaveBMP = FALSE);
		time_t tTime;
		tTime = time(NULL);
		struct tm *pTmTime = localtime(&tTime);
		TCHAR szFilename[MAX_PATH], szTime[24];
		_tcsftime(szTime, sizeof(szTime), _T("%Y%m%d_%H%M%S"), pTmTime);
		_sntprintf(szFilename, MAX_PATH, _T("%sBitmap%s.bmp\0"), m_cStrBMPSavePath, szTime);

		// Create a file to hold the bitmap
		HANDLE hf = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, 
			NULL, CREATE_ALWAYS, NULL, NULL );
		if (INVALID_HANDLE_VALUE == hf) {
			return ;
		}

		long BufferSize = abs(lpBMP->bmiHeader.biWidth)*abs(lpBMP->bmiHeader.biHeight)*4;
		// Write out the file header
		BITMAPFILEHEADER bfh;
		memset( &bfh, 0, sizeof( bfh ) );
		bfh.bfType = 'MB';
		bfh.bfSize = sizeof( bfh ) + BufferSize + sizeof( BITMAPINFOHEADER );
		bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );

		DWORD Written = 0;
		WriteFile( hf, &bfh, sizeof( bfh ), &Written, NULL );

		// Write the bitmap format
		//
		//BITMAPINFOHEADER bih;
		//memset( &bih, 0, sizeof( bih ) );
		//bih.biSize = sizeof( bih );
		//bih.biWidth = m_dwWidth;
		//bih.biHeight = -m_dwHeight;	//Save from down to up
		//bih.biPlanes = 1;
		//bih.biBitCount = 24;

		Written = 0;
		bool bResult = false;
		bResult = WriteFile( hf, &(lpBMP->bmiHeader), sizeof( lpBMP->bmiHeader ), &Written, NULL );

		// Write the bitmap bits
		//
		Written = 0;
		bResult = WriteFile( hf, (LPCVOID)&(lpBMP->bmiColors[0]), BufferSize, &Written, NULL );

		CloseHandle( hf );
	}

}

void CDecodeSampleDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if(m_bStart)
	{
		m_bStart = false;
		KStop(hSDK);
		KStopStreaming(hSDK);
		KDisconnect(hSDK);
	}

	if(hSDK)
	{
		KCloseInterface(hSDK);
		hSDK = NULL;
	}

	if(m_pOutBuf)
	{
		delete [] m_pOutBuf;
		m_pOutBuf = NULL;
	}

	if(m_pInBuf)
	{
		delete [] m_pInBuf;
		m_pInBuf = NULL;
	}

	if(m_pAudioBuf)
	{
		delete [] m_pAudioBuf;
		m_pAudioBuf = NULL;
	}

	if (m_Bmi != NULL) 
	{
		delete m_Bmi;
		m_Bmi = NULL;
	}

	ReleaseH264();

	if(m_xvidDecHandle.handle)
	{
		int xvidret = xvid_decore(m_xvidDecHandle.handle, XVID_DEC_DESTROY, 0, 0);
	}

	if(m_pSound)
		delete m_pSound;

	OnCancel();
}

void CDecodeSampleDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	m_bSaveBMP = !m_bSaveBMP;
}

bool CDecodeSampleDlg::mf_PrepareDibBuffer(LPBITMAPINFO *lplpbi, DWORD dwX, DWORD dwY)
{
	DWORD				cbSize;
	DWORD				dwActX;
	HDC					hdc;
	LPBITMAPINFOHEADER	lpbih;
	
	dwActX = (dwX * 24 / 8 + 3) >> 2 << 2;
	cbSize = sizeof(BITMAPINFO) + dwActX * dwY;

	//Allocate buffer
	*lplpbi = new BITMAPINFO;

	lpbih = (LPBITMAPINFOHEADER)*lplpbi;

	hdc = ::GetDC(NULL);
	GetDeviceCaps(hdc, BITSPIXEL); 

	//Fill information
	lpbih->biSize = sizeof(BITMAPINFOHEADER);
	lpbih->biWidth = dwX;
	lpbih->biHeight = dwY;
	lpbih->biPlanes = 1;
	lpbih->biBitCount = 24;
	lpbih->biCompression = BI_RGB;
	lpbih->biSizeImage = 0;
	lpbih->biXPelsPerMeter = (GetDeviceCaps(hdc, HORZRES) * 1000) / GetDeviceCaps(hdc, HORZSIZE);
	lpbih->biYPelsPerMeter = (GetDeviceCaps(hdc, VERTRES) * 1000) / GetDeviceCaps(hdc, VERTSIZE);
	lpbih->biClrUsed = 0;
	lpbih->biClrImportant = 0;
	::ReleaseDC(NULL, hdc);
	
	return	TRUE;
}

// If flag is true then conver DWORD IP to string otherwise convert String IP to DWORD.
bool CDecodeSampleDlg::TransformIp(char* addr,DWORD &dwIP,bool flag) 
{ 
	//Char to DWORD
	if (flag) 
	{ 
		dwIP = inet_addr(addr); 
		return true; 
	} 
	//DWORD to Char
	else 
	{ 
		char* ipaddr = NULL; 
		in_addr inaddr; 
		inaddr. s_addr=dwIP; 
		ipaddr= inet_ntoa(inaddr); 
		strcpy(addr,ipaddr); 
		return true; 
	} 
} 

void CDecodeSampleDlg::OnBnClickedCheck2()
{
	// TODO: Add your control notification handler code here
	UpdateData();
}
