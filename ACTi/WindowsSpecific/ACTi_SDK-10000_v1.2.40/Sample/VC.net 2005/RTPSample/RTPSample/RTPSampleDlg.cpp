// RTPSampleDlg.cpp : 實作檔
//

#include "stdafx.h"
#include <process.h>
#include "RTPSample.h"
#include "RTPSampleDlg.h"

#include "H264.h"
#include "head.h"
#include "Utility.h"
#include "version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HANDLE m_hVedioViewEvent;
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


// CRTPSampleDlg 對話方塊



CRTPSampleDlg::CRTPSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRTPSampleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRTPSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VW, m_ctlPicture);
	DDX_Control(pDX, IDC_BUTTON1, m_ctlRTPStart);
	DDX_Control(pDX, IDC_EDIT2, m_ctlSsysInfo);
	DDX_Control(pDX, IDC_CHECK1, m_ctlBeOnlyRTPTest);
	DDX_Control(pDX, IDC_EDIT1, m_edtMrl);
}

BEGIN_MESSAGE_MAP(CRTPSampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
END_MESSAGE_MAP()


// CRTPSampleDlg 訊息處理常式

BOOL CRTPSampleDlg::OnInitDialog()
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

	m_edtMrl.SetWindowText( "rtsp://Admin:123456@172.16.1.100:7070");

	FILE *fp = fopen( "config.dat", "r");
	if( fp )
	{
		char token[512];
		fgets( token, sizeof( token), fp);
		fclose( fp);

		int len = (int)strlen( token);
		for( int i=0; i<len; i++ )
		{
			if( token[i] == '\n' )
				token[i] = 0;
		}
		m_edtMrl.SetWindowText( token);
	}

		m_Bmi = NULL;
		m_BitmapData = NULL;

	m_nDecodeID = -1;
	m_nDecodeID = XCInit();

	m_pSound = new CSoundPlayer();

	m_hVedioViewEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
	ResetEvent( m_hVedioViewEvent);

	UpdateData(false);

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CRTPSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRTPSampleDlg::OnPaint() 
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
HCURSOR CRTPSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void RTCPSessionProc( LPVOID lpParam)
{
	int ret;
	CRTCPSession *pRTCPSession = dynamic_cast< CRTCPSession * >( (CRTCPSession *)lpParam);
	if( pRTCPSession == NULL ) return;

	CRTCPSession &RTCPSession = *pRTCPSession;
	RTCP_HOST_CONF &RtcpConf = RTCPSession.GetRtcpConf();

	RtcpConf.State = RTCP_HOST_STATE_RUN;

	/* handle the RTCP TX/RX tasks */
	do {
		ret = RTCPSession.RTCPHostHandler();
	} while( RtcpConf.State == RTCP_HOST_STATE_RUN );

	/* Send Bye RTCP packets to server */
	RTCPSession.RTCPSendByePkt();

	/* leave the thread now, close all RTCP Sockets */
	RTCPSession.CloseSession( &RtcpConf);
	
	/* wait for a while to let thread's memory be free */
	Sleep( 1000);
	
	/* return the RTCP Host State to INIT */
	RtcpConf.State = RTCP_HOST_STATE_INIT;

}

void RTPVedioSessionTestProc( LPVOID lpParam)
{
	CRTPSampleDlg *dlg = ( CRTPSampleDlg *)lpParam;
	CRTPSession *pRTPVedioSession = dlg->GetRTPSession();
	if( pRTPVedioSession == NULL ) return;

	CRTPSession &RTPVedioSession = *pRTPVedioSession;

	RTP_HOST_CONF &RtpConf = RTPVedioSession.GetRtpConf();

	/* set the RTP State to Run State now */
	RtpConf.Media[0].State = RTP_HOST_STATE_RUN;

	do {
		RTPVedioSession.RTPVideoFrameHunting();
	}while( RtpConf.Media[0].State == RTP_HOST_STATE_RUN);
	
	/* leave the thread now, close all RTCP Sockets */
	if( RtpConf.Media[0].Sock) closesocket( RtpConf.Media[0].Sock);
	RtpConf.Media[0].Sock = 0;

	/* wait for a while to let thread's memory be free */
	Sleep(1*1000);

	/* return the RTP Host State to INIT */
	RtpConf.Media[0].State = RTP_HOST_STATE_INIT;
}

void RTPVedioSessionProc( LPVOID lpParam)
{
	CRTPSampleDlg *dlg = ( CRTPSampleDlg *)lpParam;
	CRTPSession *pRTPVedioSession = dlg->GetRTPSession();
	CRTCPSession *pRTCPSession = dlg->GetRTCPSession();

	if( pRTPVedioSession == NULL ) return;
	if( pRTCPSession == NULL ) return;

	CRTPSession &RTPVedioSession = *pRTPVedioSession;
	CRTCPSession &RTCPSession = *pRTCPSession;

	RTP_HOST_CONF &RtpConf = RTPVedioSession.GetRtpConf();
	RTCP_HOST_CONF &RtcpConf = RTCPSession.GetRtcpConf();

	/* set the RTP State to Run State now */
	RtpConf.Media[0].State = RTP_HOST_STATE_RUN;
	RTP_FRAME *Frame;
	do
	{
		Frame = RTPVedioSession.NewFrames( 0, 1024*1024);
		if( Frame == NULL )
			continue;
		if( RTPVedioSession.RTPVideoIFrameHunting( Frame, &RtcpConf) == RTP_OK )
		{
			/* Get the I Frame !!, Star to read the Video Frame now */
			if( RTPVedioSession.PushNewFrames( 0, Frame) == FALSE )
			{
				RTPVedioSession.FreeFrames( 0, Frame);
				continue;
			}
			else
			{
				SetEvent( m_hVedioViewEvent);
			}
			do
			{
				/* Setup the Frame */
				Frame = RTPVedioSession.NewFrames( 0, 1024*1024);
				if( Frame == NULL ) break;

				if( RTPVedioSession.RTPFrameVideoGet( Frame, &RtcpConf) == RTP_OK)
				{
					/* get the RTP Video Frame !! */
					if( RTPVedioSession.PushNewFrames( 0, Frame) == FALSE )
					{
						RTPVedioSession.FreeFrames( 0, Frame);
					}
					else
					{
						SetEvent( m_hVedioViewEvent);
					}

				}
				else
				{
					RTPVedioSession.FreeFrames( 0, Frame);
					break;
				}

				//Sleep(10);
			}while(RtpConf.Media[0].State == RTP_HOST_STATE_RUN);
			
		}
		else
		{
			RTPVedioSession.FreeFrames( 0, Frame);
		}
	}while( RtpConf.Media[0].State == RTP_HOST_STATE_RUN);
	
	/* leave the thread now, close all RTCP Sockets */
	if( RtpConf.Media[0].Sock)
		closesocket( RtpConf.Media[0].Sock);
	RtpConf.Media[0].Sock = 0;

	/* wait for a while to let thread's memory be free */
	Sleep(1*1000);

	/* return the RTP Host State to INIT */
	RtpConf.Media[0].State = RTP_HOST_STATE_INIT;
}

void RTPAudioSessionProc( LPVOID lpParam)
{
	CRTPSampleDlg *dlg = ( CRTPSampleDlg *)lpParam;
	CRTPSession *pRTPAudioSession = dlg->GetRTPSession();
	CRTCPSession *pRTCPSession = dlg->GetRTCPSession();

	if( pRTPAudioSession == NULL ) return;
	if( pRTCPSession == NULL ) return;

	CRTPSession &RTPAudioSession = *pRTPAudioSession;
	CRTCPSession &RTCPSession = *pRTCPSession;

	RTP_HOST_CONF &RtpConf = RTPAudioSession.GetRtpConf();
	RTCP_HOST_CONF &RtcpConf = RTCPSession.GetRtcpConf();

	RTP_FRAME Frame;
	unsigned char buf[2000]; /* max audio frame buffer */

	/* set the RTP State to Run State now */
	RtpConf.Media[1].State = RTP_HOST_STATE_RUN;

	/* handle the RTCP TX/RX tasks */
	/* Setup the Frame */
	Frame.Len  = sizeof(buf);
	Frame.Buf  = buf;

	do {
		if( RTPAudioSession.RTPAudio1stFrameHunting( &Frame, &RtcpConf) == RTP_OK )
		{
			/* Get the 1st Audio Frame !!, Star to read the Audio Frames now */
			do {
				/* Setup the Frame */
				Frame.Len  = sizeof(buf);
				Frame.Buf  = buf;
				if( RTPAudioSession.RTPFrameAudioGet(&Frame, &RtcpConf) == RTP_OK)
				{
					if( dlg->GetSound() )
					{
						dlg->GetSound()->Write( Frame.Buf+28, Frame.Len- 28);
					}
				}
				else
				{
					break;
				}
			}while( RtpConf.Media[1].State == RTP_HOST_STATE_RUN );
		}
		else
		{
			;
		}
	}while( RtpConf.Media[1].State == RTP_HOST_STATE_RUN);
	/* leave the thread now, close all RTCP Sockets */
	if( RtpConf.Media[1].Sock )
		closesocket(RtpConf.Media[1].Sock);
	RtpConf.Media[1].Sock = 0;

	/* wait for a while to let thread's memory be free */
	Sleep( 1000);

	/* return the RTP Host State to INIT */
	RtpConf.Media[1].State = RTP_HOST_STATE_INIT;
}

void RTPVedioViewProc( LPVOID lpParam)
{

	CRTPSampleDlg *dlg = ( CRTPSampleDlg *)lpParam;
	CRTPSession *pRTPVedioSession = dlg->GetRTPSession();
	if( pRTPVedioSession == NULL ) return;

	CRTPSession &RTPVedioSession = *pRTPVedioSession;
	RTP_HOST_CONF &RtpConf = RTPVedioSession.GetRtpConf();

	RtpConf.Media[0].Data.SCRFrms = 0;
	RtpConf.Media[0].Data.SCRRTSs = 0;

	RTP_FRAME *Frame;
	int framesize = 0;

	BYTE * pFrameBuffer = NULL;
	DWORD dwFrameBufferSize = 0;
	DWORD dwFrameBufferLen = 0;

	do
	{
		if( WaitForSingleObject( m_hVedioViewEvent, 2*1000) == WAIT_TIMEOUT )
			continue;

		ResetEvent( m_hVedioViewEvent);
		while( (Frame = RTPVedioSession.PopFrames( 0)) != NULL )
		{
			if( Frame->Len < (int) B2_FRAME_LEN )
			{
				RTPVedioSession.FreeFrames( 0, Frame);
				continue;
			}

			switch( Frame->Type )
			{
			case 1:	//MPEG4
			case 5:
				{
					if( CRTPSession::GetRTPVideoFrameType( Frame->Buf) == RTP_FRAME_B2 )
					{
						dlg->Display( Frame->Buf, Frame->Len);
					}
					else
					{
						dlg->Display( Frame->Buf+B2_FRAME_LEN, Frame->Len-B2_FRAME_LEN);
					}

				}
				break;
			case 4:
				{
					if( !pFrameBuffer )
					{
						pFrameBuffer = new BYTE[1024*1024];
						dwFrameBufferSize = 1024*1024;
						dwFrameBufferLen = 0;
					}

					dwFrameBufferLen = 0;
					int len = 0;
					if( CRTPSession::GetRTPVideoFrameType( Frame->Buf) == RTP_FRAME_B2 )
					{
						memcpy( pFrameBuffer, Frame->Buf, B2_FRAME_LEN);
						dwFrameBufferLen = B2_FRAME_LEN;

						len = fnTrans( Frame->Buf+B2_FRAME_LEN, Frame->Len-B2_FRAME_LEN,
							pFrameBuffer+dwFrameBufferLen, dwFrameBufferSize - dwFrameBufferLen);
						if( len > 0 )
						{
							dwFrameBufferLen += len;
							dlg->Display( pFrameBuffer, dwFrameBufferLen);
						}
					}
					else
					{
						len = fnTrans( Frame->Buf+B2_FRAME_LEN, Frame->Len-B2_FRAME_LEN,
							pFrameBuffer, dwFrameBufferSize );
						if( len > 0 )
						{
							dwFrameBufferLen += len;
							dlg->Display( pFrameBuffer, dwFrameBufferLen);
						}
					}
				}
				break;

			case 2:
				{
					if( dlg->GetSound() )
					{
						int len = AUDIO_B2_FRAME_LEN;
						if( CRTPSession::GetRTPVideoFrameType( Frame->Buf) == RTP_FRAME_B2 )
						{
							dlg->GetSound()->Write( Frame->Buf+AUDIO_B2_FRAME_LEN, Frame->Len-AUDIO_B2_FRAME_LEN);
						}
						else
						{
							dlg->GetSound()->Write( Frame->Buf, Frame->Len);
						}
					}
				}
				break;
			default:
				break;
			}

			if( RtpConf.Media[0].Data.SCRRTSs == 0 )
			{
				RtpConf.Media[0].Data.SCRRTSs =  (unsigned long)time(NULL);
				RtpConf.Media[0].Data.SCRFrms = 0;
			}
			else
			{
				RtpConf.Media[0].Data.SCRFrms ++;
			}
			RTPVedioSession.FreeFrames( 0, Frame);
		}

	}while( RtpConf.Media[0].State == RTP_HOST_STATE_RUN ||
		RtpConf.Media[1].State == RTP_HOST_STATE_RUN );

	if( pFrameBuffer )
		delete [] pFrameBuffer;
	pFrameBuffer = NULL;
}

void CRTPSampleDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	UpdateData();


	char szUnicast[64];
	char szUserID[64];
	char szPassWD[64];
	int nPortNum = 0;
	char szFilePath[256];
	char szMrl[512];
	{
		m_edtMrl.GetWindowText( szMrl, sizeof( szMrl));
		if( strncmp( szMrl, "rtsp://", strlen( "rtsp://")) )
		{
			return;
		}

		char * pszMrl = szMrl+strlen( "rtsp://");
		{
			char * ptr = pszMrl;
			{
				char * ptr1= strchr( ptr, '@');
				if( ptr1 )
				{
					char * ptr2= strchr( ptr, ':');
					if( ptr2 && ptr2 < ptr1 )
					{
						strncpy( szUserID, ptr, ptr2-ptr);
						szUserID[ptr2-ptr] = 0;

						ptr2++;
						strncpy( szPassWD, ptr2, ptr1-ptr2);
						szPassWD[ptr1-ptr2] = 0;
					}
					else
					{
						strncpy( szUserID, ptr, ptr1-ptr);
						szUserID[ptr2-ptr] = 0;
						szPassWD[0] = 0;
					}
					ptr = ptr1+1;
				}
				else
				{
					strcpy( szUserID, "Admin\0"); //default
					strcpy( szPassWD, "123456\0"); //default
				}
			}

			//ptr
			{
				char * ptr1= strchr( ptr, '/');
				if( ptr1 )
				{
					char * ptr2= strchr( ptr, ':');
					if( ptr2 && ptr2 < ptr1 )
					{
						strncpy( szUnicast, ptr, ptr2-ptr);
						szUnicast[ptr2-ptr] = 0;

						ptr2++;
						char buf[64];
						strncpy( buf, ptr2, ptr1-ptr2);
						buf[ptr1-ptr2] = 0;
						nPortNum = atoi( buf);

						strcpy( szFilePath, ptr1);
						szFilePath[strlen(ptr1)] = 0;
					}
					else
					{
						strncpy( szUnicast, ptr, ptr1-ptr);
						szUnicast[ptr1-ptr] = 0;
						nPortNum = 7070; //default

						strcpy( szFilePath, ptr1);
						szFilePath[strlen(ptr1)] = 0;
					}
				}
				else
				{
					char * ptr2= strchr( ptr, ':');
					if( ptr2 )
					{
						strncpy( szUnicast, ptr, ptr2-ptr);
						szUnicast[ptr2-ptr] = 0;

						ptr2++;
						char buf[64];
						sprintf( buf, "%s", ptr2);
						nPortNum = atoi( buf);
					}
					else
					{
						sprintf( szUnicast, "%s", ptr );
						nPortNum = 7070; //default
					}
					sprintf( szFilePath, "");
				}
			}
		}

	}

	char localhostaddr[32];
	{
		char *host_name = NULL;
		struct hostent *remoteHost;
		remoteHost = gethostbyname( NULL);
		if (remoteHost == NULL)
		{
			int dwError = WSAGetLastError();
			switch( dwError )
			{
			case WSAHOST_NOT_FOUND:
				break;
			case WSANO_DATA:
				break;
			default:
				break;
			}
			return;
		}
        switch ( remoteHost->h_addrtype)
		{
        case AF_INET:
			break;
        case AF_INET6:
			break;
        case AF_NETBIOS:
			break;
        default:
			break;
        }
		struct in_addr addr;

		addr.s_addr = *(u_long *) remoteHost->h_addr_list[0];
		sprintf( localhostaddr, "%s", inet_ntoa(addr) );
	}

	{
		char addrstr[512];
		sprintf( addrstr, "%s", szMrl);

		FILE *fp = fopen( "config.dat", "w+");
		if( fp )
		{
			fputs( addrstr, fp);
			fputs( "\n", fp);
			fclose( fp);
		}
	}
/*
	char szUnicast[64];
	char szUserID[64];
	char szPassWD[64];
	int nPortNum = 0;
	char szFilePath[256];
	char szMrl[512];
*/
	m_RTSPSession.InitRTSPConf( szUnicast, nPortNum, szFilePath, szUserID, szPassWD, localhostaddr );
	RTSP_HOST_CONF &RtspConf = m_RTSPSession.GetRtspConf();

	if( m_RTSPSession.ConnectToRTSPSrv() == SOCKET_ERROR )
	{
		UpdateData(false);
		return;
	}

	RTCP_HOST_CONF &RtcpConf = m_RTCPSession.GetRtcpConf();
	m_RTCPSession.InitRTCPHostConf( &RtspConf );
	if( m_RTCPSession.CreateRTCPHostSockets() == SOCKET_ERROR )
	{
		UpdateData(false);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return;
	}

	RTP_HOST_CONF &RtpConf = m_RTPSession.GetRtpConf();
	m_RTPSession.InitRTPHostConf( &RtspConf );
	if( m_RTPSession.CreateRTPHostSockets() == SOCKET_ERROR )
	{
		m_RTCPSession.CloseSession( &RtcpConf );
		UpdateData(false);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return;
	}

	if( m_RTSPSession.RTSPHostPlayHandle() == RTSP_OK )
	{
			m_RTPSession.RTPSeqNumUpdate( RtspConf.Info.Media[0].Seq, RtspConf.Info.Media[1].Seq );
			; // create RTCP Thread
			m_hRtcpSessionThread = ( HANDLE ) _beginthread( RTCPSessionProc, 0, &m_RTCPSession);
			if( m_hRtcpSessionThread == NULL )
			{
			}

			if( m_ctlBeOnlyRTPTest.GetCheck() )
			{
				m_hRtpASessionThread = NULL;
				m_hVedioViewThread = NULL;
				m_hRtpVSessionThread = ( HANDLE ) _beginthread( RTPVedioSessionTestProc, 0, this);
				if( m_hRtpVSessionThread == NULL )
				{
				}
			}
			else
			{
				; // create RTP Vedio Thread
				m_hRtpVSessionThread = ( HANDLE ) _beginthread( RTPVedioSessionProc, 0, this);
				if( m_hRtpVSessionThread == NULL )
				{
				}

				; // create RTP Audio Thread
				m_hRtpASessionThread = ( HANDLE ) _beginthread( RTPAudioSessionProc, 0, this);
				if( m_hRtpASessionThread == NULL )
				{
				}

				m_hVedioViewThread = ( HANDLE ) _beginthread( RTPVedioViewProc, 0, this);
				if( m_hVedioViewThread == NULL )
				{
				}

				for(int  i = 0 ; i < 2; i ++)
				{
					Sleep(1);
					if( m_RTSPSession.IsRTSPSrvAlive() == RTSP_ERR)
					{
						//printf("RTSP Server was down!!\n");
						break;
					}
				}
			}

#if 0
			/* Send the TERADOWN command to leave the RTSP/RTP/RTCP session */
			if( m_RTSPSession.RTSPTeardownCmdSend() == RTSP_ERR)
			{
				printf("Fail to send TEARDOWN Cmd\n");
			}
#endif
	}
	m_ctlRTPStart.EnableWindow( FALSE);
	m_ctlBeOnlyRTPTest.EnableWindow( FALSE);

	SetTimer( 0, 3000, NULL );
	UpdateData(false);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}



BOOL CRTPSampleDlg::PrepareDibBuffer( LPBITMAPINFO *lplpbi, DWORD dwX, DWORD dwY)
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

void CRTPSampleDlg::Display( BYTE * lpFrameBuffer, int FrameBufferLen, int w, int h )
{
	B2_FRAME *B2 = NULL;

	B2 = (B2_FRAME *) lpFrameBuffer;
	if( (B2->Head.Key[0]==0x00)&&(B2->Head.Key[1]==0x00) &&
		(B2->Head.Key[2]==0x01)&&(B2->Head.Key[3]==0xb2))
	{

		if( m_Bmi == NULL || m_FrameResolution != B2->Data.Resolution )
		{
			m_FrameResolution = B2->Data.Resolution;
			if( m_Bmi != NULL ) delete m_Bmi;
			m_Bmi = NULL;

			switch( (int)B2->Data.Resolution )
			{
			case XNTSC720x480:
				m_VIEW_WIDTH = 720;
				m_VIEW_HEIGHT = 480;
				break;
			case XNTSC352x240:
				m_VIEW_WIDTH = 352;
				m_VIEW_HEIGHT = 240;
				break;
			case XNTSC160x112:
				m_VIEW_WIDTH = 160;
				m_VIEW_HEIGHT = 112;
				break;
			case XNTSC640x480:
				m_VIEW_WIDTH = 640;
				m_VIEW_HEIGHT = 480;
				break;
			case XNTSC1280x720:
				m_VIEW_WIDTH = 1280;
				m_VIEW_HEIGHT = 720;
				break;
			case XNTSC1280x960:
				m_VIEW_WIDTH = 1280;
				m_VIEW_HEIGHT = 960;
				break;
			case XNTSC1280x1024:
				m_VIEW_WIDTH = 1280;
				m_VIEW_HEIGHT = 1024;
				break;
			case XNTSC1920x1080:
				m_VIEW_WIDTH = 1920;
				m_VIEW_HEIGHT = 1080;
				break;
			default:
				m_VIEW_WIDTH = 1280;
				m_VIEW_HEIGHT = 720;
				break;
			}
			if( m_BitmapData != NULL )
			{
				delete [] m_BitmapData;
				m_BitmapData = NULL;
			}
			m_BitmapData = new unsigned char[ m_VIEW_WIDTH*m_VIEW_HEIGHT*4];
			PrepareDibBuffer( &m_Bmi, m_VIEW_WIDTH, m_VIEW_HEIGHT);
		}

		VideoDisply( m_Bmi, lpFrameBuffer+B2_FRAME_LEN,
			FrameBufferLen-B2_FRAME_LEN, m_BitmapData);
	}
	else
	{
		if( m_Bmi == NULL )
		{
			m_VIEW_WIDTH = w;
			m_VIEW_HEIGHT = h;

			if( m_BitmapData != NULL )
			{
				delete [] m_BitmapData;
				m_BitmapData = NULL;
			}
			m_BitmapData = new unsigned char[ m_VIEW_WIDTH*m_VIEW_HEIGHT*4];
			PrepareDibBuffer( &m_Bmi, m_VIEW_WIDTH, m_VIEW_HEIGHT);
		}
		VideoDisply( m_Bmi, lpFrameBuffer, FrameBufferLen, m_BitmapData);
	}
}

void CRTPSampleDlg::VideoDisply( BITMAPINFO* Bmi, BYTE * pInBuf, DWORD nInBufLen, unsigned char * BitmapData )
{

	LPBITMAPINFO lpNewBmi = NULL;
	if( m_nDecodeID == -1 ) return;
	{
		lpNewBmi = XDecode( m_nDecodeID, 640, 480, pInBuf, nInBufLen);
			//HWND hWnd = NULL, PRECT pRt = NULL, bool bReSync = false);
		if( !lpNewBmi ) return;
	}

	int ret = 0;
	HDC hDrawDC = NULL;
	DWORD dwWidth, dwHeight;
	CDC* thedc = m_ctlPicture.GetDC();

	{
		
		if(!thedc)
		{
			return ;
		}
		hDrawDC = thedc->m_hDC;

		RECT rect;
		m_ctlPicture.GetWindowRect(&rect);
		dwWidth = rect.right - rect.left;
		dwHeight = rect.bottom - rect.top;
	}

	ret = ::SetStretchBltMode( hDrawDC, COLORONCOLOR);

	int i_bm_h = abs(lpNewBmi->bmiHeader.biHeight);
	ret = ::StretchDIBits( hDrawDC,
		0, 0,
		dwWidth, dwHeight,
		0 ,0,
		lpNewBmi->bmiHeader.biWidth , abs(lpNewBmi->bmiHeader.biHeight),
		(const void*)&lpNewBmi[1], lpNewBmi, DIB_RGB_COLORS, SRCCOPY);


	m_ctlPicture.ReleaseDC(thedc);
}

void CRTPSampleDlg::CloseRTPSession( void)
{
	// RTP/RTSP
	RTSP_HOST_CONF &RtspConf = m_RTSPSession.GetRtspConf();

	m_RTPSession.CloseMedia();
	m_RTCPSession.CloseMedia();
	Sleep(1000);

	//SetEvent( m_hVedioViewEvent);
	WaitForSingleObject( m_hRtcpSessionThread, INFINITE);
	WaitForSingleObject( m_hRtpVSessionThread, INFINITE);
	WaitForSingleObject( m_hRtpASessionThread, INFINITE);
	WaitForSingleObject( m_hVedioViewThread, INFINITE);

	//CloseHandle( m_hVedioViewEvent);
	if( RtspConf.Sock != INVALID_SOCKET )
	{
		if( m_RTSPSession.RTSPTeardownCmdSend() == RTSP_ERR)
		{
			//printf("Fail to send TEARDOWN Cmd\n");
		}
	}

	m_RTCPSession.CloseSession();
	m_RTPSession.CloseSession();
	m_RTSPSession.CloseSession();
}

void CRTPSampleDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	
	CloseRTPSession();

	if( m_nDecodeID >= 0 )
		XCExit( m_nDecodeID);
	m_nDecodeID = -1;

	if( m_pSound )
		delete m_pSound;
	m_pSound = NULL;

	if( m_BitmapData != NULL )
	{
		delete [] m_BitmapData;
	}
	m_BitmapData = NULL;
	if( m_Bmi != NULL )
	{
		delete m_Bmi;
	}
	m_Bmi = NULL;

	OnCancel();
}

void CRTPSampleDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void CRTPSampleDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CRTPSession *pRTPVedioSession = GetRTPSession();
	CRTCPSession *pRTCPSession = GetRTCPSession();

	if( pRTPVedioSession == NULL ) return;
	if( pRTCPSession == NULL ) return;

	CRTPSession &RTPVedioSession = *pRTPVedioSession;
	CRTCPSession &RTCPSession = *pRTCPSession;

	RTP_HOST_CONF &RtpConf = RTPVedioSession.GetRtpConf();
	RTCP_HOST_CONF &RtcpConf = RTCPSession.GetRtcpConf();

	
	if( RtpConf.Media[0].Data.RFrms > 300 )
	{
		char token[512];
		sprintf( token, "ErrRRPkts:%d RFPs:%d / %2.2f fps SCREEN: %d / %2.2f fps",
			RtpConf.Media[0].Data.ErrRRPkts, RtpConf.Media[0].Data.RFrms,
			(double)RtpConf.Media[0].Data.RFrms/(time(NULL)-RtpConf.Media[0].Data.RTSs),
			RtpConf.Media[0].Data.SCRFrms,
			(double)RtpConf.Media[0].Data.SCRFrms/(time(NULL)-RtpConf.Media[0].Data.SCRRTSs)
			);
		RtpConf.Media[0].Data.SCRRTSs = 0;
		RtpConf.Media[0].Data.RTSs = 0;
		RtpConf.Media[0].Data.RFrms = 0;
		RtpConf.Media[0].Data.SCRFrms = 0;
		m_ctlSsysInfo.SetWindowText( token);
		CDialog::OnTimer(nIDEvent);
	}


}

void CRTPSampleDlg::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
}

