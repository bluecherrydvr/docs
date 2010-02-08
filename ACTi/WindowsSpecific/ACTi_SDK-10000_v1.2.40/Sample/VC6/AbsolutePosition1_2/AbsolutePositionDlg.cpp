// AbsolutePositionDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "AbsolutePosition.h"
#include "AbsolutePositionDlg.h"
#include ".\absolutepositiondlg.h"
#include <math.h>

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

// CAbsolutePositionDlg 對話方塊

void CheckOutPTZPositionData( BYTE* pBuf, int ndataLen, int &nPan, int &nTilt, int &nZoom )
{
	BYTE bPanHeader[3] = { 0x00, 0x01, 0x09 };
	BYTE bTiltHeader[3] = { 0x00, 0x01, 0x0a };
	BYTE bZoomHeader[3] = { 0x00, 0x01, 0xb2 };

	BYTE TmpBuf[6];
	BYTE checkSum = 0;
	int nPos = 0;
	int nSize = ndataLen;

	while( nPos < nSize - 6 )
	{
		memcpy( TmpBuf, &pBuf[nPos], 6 );
		if( memcmp( TmpBuf, bZoomHeader, 3 ) == 0 )
		{
			nPos += 6;
			checkSum = 0;
			checkSum ^= TmpBuf[0];
			checkSum ^= TmpBuf[1];
			checkSum ^= TmpBuf[2];
			checkSum ^= TmpBuf[3];
			checkSum ^= TmpBuf[4];
			if( checkSum == TmpBuf[5] )
			{
				//	High byte             Low byte
				if( TmpBuf[4] == 0x00 && TmpBuf[3] == 0x00 )	nZoom = 1;
				else if( TmpBuf[4] == 0x17 && TmpBuf[3] == 0x60 )	nZoom = 2;
				else if( TmpBuf[4] == 0x21 && TmpBuf[3] == 0x4C )	nZoom = 3;
				else if( TmpBuf[4] == 0x27 && TmpBuf[3] == 0x22 )	nZoom = 4;
				else if( TmpBuf[4] == 0x2B && TmpBuf[3] == 0x22 )	nZoom = 5;
				else if( TmpBuf[4] == 0x2E && TmpBuf[3] == 0x20 )	nZoom = 6;
				else if( TmpBuf[4] == 0x30 && TmpBuf[3] == 0x80 )	nZoom = 7;
				else if( TmpBuf[4] == 0x32 && TmpBuf[3] == 0x78 )	nZoom = 8;
				else if( TmpBuf[4] == 0x34 && TmpBuf[3] == 0x26 )	nZoom = 9;
				else if( TmpBuf[4] == 0x35 && TmpBuf[3] == 0x9E )	nZoom = 10;
				else if( TmpBuf[4] == 0x36 && TmpBuf[3] == 0xEE )	nZoom = 11;
				else if( TmpBuf[4] == 0x38 && TmpBuf[3] == 0x1C )	nZoom = 12;
				else if( TmpBuf[4] == 0x39 && TmpBuf[3] == 0x2E )	nZoom = 13;
				else if( TmpBuf[4] == 0x3A && TmpBuf[3] == 0x26 )	nZoom = 14;
				else if( TmpBuf[4] == 0x3B && TmpBuf[3] == 0x08 )	nZoom = 15;
				else if( TmpBuf[4] == 0x3B && TmpBuf[3] == 0xD4 )	nZoom = 16;
				else if( TmpBuf[4] == 0x3C && TmpBuf[3] == 0x8C )	nZoom = 17;
				else if( TmpBuf[4] == 0x3D && TmpBuf[3] == 0x2E )	nZoom = 18;
				else if( TmpBuf[4] == 0x40 && TmpBuf[3] == 0x00 )	nZoom = 19;
				else if( TmpBuf[4] == 0x60 && TmpBuf[3] == 0x00 )	nZoom = 20;
				else if( TmpBuf[4] == 0x6A && TmpBuf[3] == 0x80 )	nZoom = 21;
				else if( TmpBuf[4] == 0x70 && TmpBuf[3] == 0x00 )	nZoom = 22;
				else if( TmpBuf[4] == 0x73 && TmpBuf[3] == 0x00 )	nZoom = 23;
				else if( TmpBuf[4] == 0x75 && TmpBuf[3] == 0x40 )	nZoom = 24;
				else if( TmpBuf[4] == 0x76 && TmpBuf[3] == 0xC0 )	nZoom = 25;
				else if( TmpBuf[4] == 0x78 && TmpBuf[3] == 0x00 )	nZoom = 26;
				else if( TmpBuf[4] == 0x78 && TmpBuf[3] == 0xC0 )	nZoom = 27;
				else if( TmpBuf[4] == 0x79 && TmpBuf[3] == 0x80 )	nZoom = 28;
				else if( TmpBuf[4] == 0x7A && TmpBuf[3] == 0x00 )	nZoom = 29;
				else if( TmpBuf[4] == 0x7A && TmpBuf[3] == 0xC0 )	nZoom = 30;
			}
		}
		else if( memcmp( TmpBuf, bTiltHeader, 3 ) == 0 )
		{
			nPos += 6;
			checkSum = 0;
			checkSum ^= TmpBuf[0];
			checkSum ^= TmpBuf[1];
			checkSum ^= TmpBuf[2];
			checkSum ^= TmpBuf[3];
			checkSum ^= TmpBuf[4];
			if( checkSum == TmpBuf[5] )
				nTilt = (TmpBuf[4] & 0x7F) * 0x100 + TmpBuf[3];
		}
		else if( memcmp( TmpBuf, bPanHeader, 3 ) == 0 )
		{
			nPos += 6;
			checkSum = 0;
			checkSum ^= TmpBuf[0];
			checkSum ^= TmpBuf[1];
			checkSum ^= TmpBuf[2];
			checkSum ^= TmpBuf[3];
			checkSum ^= TmpBuf[4];
			if( checkSum == TmpBuf[5] )
				nPan = (TmpBuf[4] & 0x7F) * 0x100 + TmpBuf[3];
		}
		else nPos++;
	}
}

void WINAPI RSDCB( DWORD UserParam, BYTE* buf, DWORD len)
{
	//char sz[1024] = {0};
	//int n = 0;

	//n += sprintf( &sz[n], "GetPTZPos Return Value " );
	//for(int i = 0; i < len; i++)
	//{
	//	n += sprintf( &sz[n], "0x%02x ", buf[i]);
	//}
	//sz[n] = '\n';
	//OutputDebugString(sz);

	CAbsolutePositionDlg* p = reinterpret_cast<CAbsolutePositionDlg*>(UserParam);
	p->OnRSDCB(buf, len);
}

CAbsolutePositionDlg::CAbsolutePositionDlg(CWnd* pParent /*=NULL*/)
: CDialog(CAbsolutePositionDlg::IDD, pParent)
, m_dwRatio(0)
, m_dwPan(0)
, m_dwTilt(0)
, m_dwQuadrant(1)
, m_fSlope(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ICatchedPan = 0;
	m_ICatchedTilt = 0;
	m_ICatchedZoom = 0;
}

void CAbsolutePositionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC1, m_pc);
	DDX_Text(pDX, IDC_EDIT1, m_dwRatio);
	DDV_MinMaxUInt(pDX, m_dwRatio, 1, 30);
	DDX_Text(pDX, IDC_EDIT2, m_dwPan);
	DDX_Text(pDX, IDC_EDIT3, m_dwTilt);
	DDX_Control(pDX, IDC_IPADDRESS1, m_IPCtrl);
	DDX_Control(pDX, IDC_STATIC2, m_pc2);


	DDX_Text(pDX, IDC_EDIT4, m_ICatchedPan);
	DDX_Text(pDX, IDC_EDIT5, m_ICatchedTilt);
	DDX_Text(pDX, IDC_EDIT6, m_ICatchedZoom);
}

BEGIN_MESSAGE_MAP(CAbsolutePositionDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
//	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
	
	ON_EN_CHANGE(IDC_EDIT4, OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, OnEnChangeEdit5)
	ON_EN_CHANGE(IDC_EDIT6, OnEnChangeEdit6)
END_MESSAGE_MAP()


// CAbsolutePositionDlg 訊息處理常式

BOOL CAbsolutePositionDlg::OnInitDialog()
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
	m_hKMpeg4 = NULL;
	memset(m_szRSDCB, 0x00, 128);
	m_dwRSDLen = 0;
	m_dwRatio = 1;
	m_dwPan = 1;
	m_dwTilt = 23;
	//m_IPCtrl.SetAddress( 208, 24, 52, 20 );
	m_IPCtrl.SetAddress( 172, 16, 1, 85 );






	UpdateData(FALSE);
	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CAbsolutePositionDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAbsolutePositionDlg::OnPaint() 
{
	RECT rc2;
	m_pc2.GetClientRect(&rc2);
	int nMidx = (rc2.left + rc2.right)/2;
	int nMidy = (rc2.top + rc2.bottom)/2;
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

	// Big circle
	m_pc2.GetDC()->Ellipse(rc2.top, rc2.left, rc2.right, rc2.bottom);

	//// Cross
	HDC RanderDC;
	RanderDC = ::GetDC(m_pc2.GetSafeHwnd());
	MoveToEx( RanderDC, rc2.left, (rc2.bottom-rc2.top)/2, NULL);
	LineTo	( RanderDC, rc2.right, (rc2.bottom-rc2.top)/2); 
	MoveToEx( RanderDC, (rc2.right - rc2.left)/2, rc2.top, NULL);
	LineTo	( RanderDC, (rc2.right - rc2.left)/2 , rc2.bottom); 
	::ReleaseDC(m_pc2.GetSafeHwnd(), RanderDC);

	//// small circle.
	m_pc2.GetDC()->Ellipse(m_ClickPt.x, m_ClickPt.y, m_ClickPt.x + 5, m_ClickPt.y +5);
}

//當使用者拖曳最小化視窗時，系統呼叫這個功能取得游標顯示。
HCURSOR CAbsolutePositionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAbsolutePositionDlg::Disconnect()
{
	if(m_hKMpeg4)
	{
		KSetRS232DataCallback(m_hKMpeg4, 0, NULL);

		KStop(m_hKMpeg4);
		KStopStreaming(m_hKMpeg4);
		KDisconnect(m_hKMpeg4);
		KCloseInterface(m_hKMpeg4);

		m_hKMpeg4 = NULL;
	}
	m_fSlope = 0;
}

void CAbsolutePositionDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	Disconnect();

	UpdateData();

	MEDIA_CONNECTION_CONFIG mcc;
	memset(&mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
	mcc.ContactType = CONTACT_TYPE_UNICAST_PREVIEW;
	mcc.ControlPort = 6001;
	mcc.StreamingPort = 6002;
	mcc.RegisterPort = 6000;
	strcpy(mcc.UserID, "Admin\0");
	strcpy(mcc.Password, "123456\0");
	mcc.HTTPPort = 80;

	BYTE bip[4];
	m_IPCtrl.GetAddress( bip[0], bip[1], bip[2], bip[3] );
	sprintf( mcc.UniCastIP, "%d.%d.%d.%d\0", bip[0], bip[1], bip[2], bip[3] );

	mcc.ConnectTimeOut = 3;
	mcc.ChannelNumber = 0;

	RECT rc;
	m_pc.GetClientRect(&rc);
	MEDIA_RENDER_INFO mri;
	mri.rect.top = 0;
	mri.rect.left = 0;
	mri.rect.right = rc.right - rc.left;
	mri.rect.bottom = rc.bottom - rc.top;
	mri.hWnd = m_pc.m_hWnd;
	mri.DrawerInterface = DGDI;

	RECT rc2;
	m_pc2.GetClientRect(&rc2);
	m_rcClick.top = rc2.top + 355;
	m_rcClick.left = rc2.left + 750;
	m_rcClick.right = rc2.right + 750;
	m_rcClick.bottom = rc2.bottom + 355;
	m_MidPoint.y = (m_rcClick.top + m_rcClick.bottom)/2;
	m_MidPoint.x = (m_rcClick.right + m_rcClick.left)/2;

	m_hKMpeg4 = KOpenInterface();

	if(NULL == m_hKMpeg4)
		return;

	KSetRS232DataCallback(m_hKMpeg4, (DWORD)this, RSDCB);
	KSetTCPType(m_hKMpeg4, 2);
	KSetTargetCameraIsQuad(m_hKMpeg4, false);
	KSetRenderInfo(m_hKMpeg4, &mri);
	if(KSetMediaConfig(m_hKMpeg4, &mcc))
	{
		if(KConnect(m_hKMpeg4))
		{
			if(KStartStreaming(m_hKMpeg4))
			{
				KPlay(m_hKMpeg4);
				KSendRS232Setting(m_hKMpeg4, RS232_SET_N81, BAUD_RATE_9600BPS, 0);


				// let us load ptz protocol file here
				// Set PTZ Protocol enabled
				bool bResult = KEnablePTZProtocol(m_hKMpeg4, true);

				m_strPTZfileName = ".\\DynaColor_DynaColor.ptz";
				memset(&m_mPTZ, 0, sizeof(MEDIA_PTZ_PROTOCOL));
				m_mPTZ.dwAddressID = 1; // most camera device be set to 1 
				sprintf(m_mPTZ.szProtocolFileName, "%s", m_strPTZfileName.c_str());
				m_mPTZ.nSourceType = PTZ_MEDIA_SOURCE_FILE;
				if(!KPTZLoadProtocol(m_hKMpeg4,&m_mPTZ))
				{
					MessageBox( "Failed to open ptz protocol file!","Protocol");
				}



				GoHome();
				return;
			}
			else
			{
				AfxMessageBox("KStartStreaming FAIL");
			}
		}
		else
		{
			AfxMessageBox("KConnect FAIL");
		}
	}
	else
	{
		AfxMessageBox("KSetMediaConfig FAIL");
	}
	Disconnect();
}

void CAbsolutePositionDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	Disconnect();
}


void CAbsolutePositionDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	Disconnect();
	OnCancel();
}

void CAbsolutePositionDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	if(NULL == m_hKMpeg4)
		return;

	UpdateData();

	if(!CheckPTZValue(m_dwPan, m_dwTilt, m_dwRatio))
		return;

	BYTE bCommand[16] = {0};
	DWORD dwLen = 0;

	//m_cAbsolute.SetPTZPos(m_dwPan, m_dwTilt, m_dwRatio, bCommand, dwLen);
	KPTZGetAbsPTZCommandByUnit(m_hKMpeg4,
		"SETABSOLUTEPTZ", 
		0, 
		0,
		true,    //is ccw
		m_dwPan,
		m_dwTilt,
		m_dwRatio,      
		bCommand,
		dwLen);
	KSendPTZCommand(m_hKMpeg4, bCommand, dwLen);
}

void CAbsolutePositionDlg::OnRSDCB(BYTE* buf, DWORD len)
{
	memcpy(m_szRSDCB+m_dwRSDLen, buf, len);
	m_dwRSDLen += len;
	m_dwRSDLen = m_dwRSDLen % 128;
}


void CAbsolutePositionDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	for(int nOnce = 0; nOnce < 1; nOnce++)
	{
		if(NULL != m_hKMpeg4)
		{
			UpdateData();
			if(!CheckPTZValue(m_dwPan, m_dwTilt, m_dwRatio))
				break;

			if(point.x >= m_rcClick.left && point.x <= m_rcClick.right && point.y >= m_rcClick.top && point.y <= m_rcClick.bottom)
			{
				double pi = 3.1415926535;
				double fAdj = abs((double)m_MidPoint.x - (double)point.x);
				double fOpp = abs((double)m_MidPoint.y - (double)point.y);
				if(fAdj == 0)
					fAdj = 1;
				if(fOpp == 0)
					fOpp = 1;
				double fTemp = abs((fAdj * fAdj) + (fOpp * fOpp));
				double fHyp = sqrt(fTemp);
				double fSin = sin(fOpp/fHyp);
				double fTan = tan(fOpp/fAdj);
				double fAngle = atan2(fAdj, fOpp)*180/pi;
				double fSlope = fOpp/fAdj;

				DWORD dwPanStartPos = 1;
				DWORD dwMaxPanPos = 1600 + dwPanStartPos; //  3601
				double dwStepPerDeg = 1600/360;
				DWORD dwNewPan = dwStepPerDeg * fAngle;

				bool bIsCCW = false;
				// Click on mid - ignore.
				if(point == m_MidPoint)
				{
				}
				// Left top
				if(point.x < m_MidPoint.x && point.y < m_MidPoint.y)
				{
					dwNewPan = dwStepPerDeg * (90 - fAngle);
					dwNewPan += 1200;
					// MSB for Pan left
					if(m_dwQuadrant == 1)
						bIsCCW = true;//dwNewPan = dwNewPan | 0x00008000;
					if(4 == m_dwQuadrant)
					{
						if(m_fSlope != 0)
						{
							if(m_fSlope > fSlope)
								bIsCCW = true;//dwNewPan = dwNewPan | 0x00008000;
						}
					}
					m_fSlope = fSlope;
					m_dwQuadrant = 4;
				}
				// Right top
				else if (point.x > m_MidPoint.x && point.y < m_MidPoint.y)
				{
					// MSB for Pan left
					if(m_dwQuadrant == 2)
						bIsCCW = true;//dwNewPan = dwNewPan | 0x00008000;
					if(1 == m_dwQuadrant)
					{
						if(m_fSlope != 0)
						{
							if(m_fSlope < fSlope)
								bIsCCW = true;//dwNewPan = dwNewPan | 0x00008000;
						}
					}
					m_fSlope = fSlope;
					m_dwQuadrant = 1;
				}
				// Left bottom
				else if(point.x < m_MidPoint.x && point.y > m_MidPoint.y)
				{
					dwNewPan += 800;
					// MSB for Pan left
					if(m_dwQuadrant == 4)
						bIsCCW = true;//dwNewPan = dwNewPan | 0x00008000;
					if(3 == m_dwQuadrant)
					{
						if(m_fSlope != 0)
						{
							if(m_fSlope < fSlope)
								bIsCCW = true;//dwNewPan = dwNewPan | 0x00008000;
						}
					}
					m_fSlope = fSlope;
					m_dwQuadrant = 3;
				}
				// Right bottom
				else if(point.x > m_MidPoint.x && point.y > m_MidPoint.y)
				{
					dwNewPan = dwStepPerDeg * (90 - fAngle);
					dwNewPan += 400;
					// MSB for Pan left
					if(m_dwQuadrant == 3)
						bIsCCW = true;//dwNewPan = dwNewPan | 0x00008000;
					if(2 == m_dwQuadrant)
					{
						if(m_fSlope != 0)
						{
							if(m_fSlope > fSlope)
								bIsCCW = true;//dwNewPan = dwNewPan | 0x00008000;
						}
					}
					m_fSlope = fSlope;
					m_dwQuadrant = 2;
				}
				else
				{
					break;
				}

				BYTE bCommand[16] = {0};
				DWORD dwLen = 0;

				//m_cAbsolute.SetPTZPos(dwNewPan, m_dwTilt, m_dwRatio, bCommand, dwLen);
				KPTZGetAbsPTZCommandByUnit(m_hKMpeg4,
					"SETABSOLUTEPTZ", 
					0, 
					0,
					bIsCCW,    //is ccw
					dwNewPan,
					m_dwTilt,
					m_dwRatio,      
					bCommand,
					dwLen);
				KSendPTZCommand(m_hKMpeg4, bCommand, dwLen);
				//equal this //DWORD test = dwNewPan - 0x00008000;
				m_dwPan = dwNewPan & 0x00000FFF;
				m_ClickPt.x = point.x - 750;
				m_ClickPt.y = point.y - 355;

				Invalidate();
				UpdateData(FALSE);
			}
		}
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CAbsolutePositionDlg::OnBnClickedButton5()
{
	GoHome();
}

void CAbsolutePositionDlg::GoHome()
{
	if(NULL == m_hKMpeg4)
		return;

	BYTE bCommand[16] = {0};
	DWORD dwLen = 0;

	//m_cAbsolute.SetPTZPos(1, 23, 1, bCommand, dwLen);

	KPTZGetAbsPTZCommandByUnit(m_hKMpeg4,
		"SETABSOLUTEPTZ", 
		0, 
		0,
		false,
		1,
		23,
		1,       
		bCommand,
		dwLen);

	KSendPTZCommand(m_hKMpeg4, bCommand, dwLen);
	m_dwPan = 1;
	m_dwTilt = 23;
	m_dwRatio = 1;
	m_dwQuadrant = 1;
	m_ClickPt.x = m_MidPoint.x;
	m_ClickPt.y = m_MidPoint.y;
	double fAdj = abs((double)m_MidPoint.x - 1);
	double fOpp = abs((double)m_MidPoint.y - 23);
	m_fSlope = fOpp/fAdj;


	Invalidate();
	UpdateData(FALSE);
}

bool CAbsolutePositionDlg::CheckPTZValue(int nPan, int nTilt, int nRatio)
{
	if(nPan > 1600 || nPan < 1)
		return false;
	if(nTilt > 223 || nTilt < 23)
		return false;
	if(nRatio > 30 || nRatio < 1)
		return false;
	return true;
}

void CAbsolutePositionDlg::OnBnClickedButton6()
{
	if(NULL == m_hKMpeg4)
		return;

	BYTE bCommand[16] = {0};
	DWORD dwLen = 0;

	//m_cAbsolute.GetPTZPos(bCommand, dwLen);
	KPTZGetRequestAbsPTZCommand(m_hKMpeg4, 0, bCommand, dwLen); // param 1  is reserved. It's 0 for now 
	KSendPTZCommand(m_hKMpeg4, bCommand, dwLen);
}

void CAbsolutePositionDlg::OnBnClickedButton7()
{
	//BYTE bPan[18] = { 0x00, 0x01, 0xb2, 0x00, 0x00, 0xb3, 0x00, 0x01, 0x0a, 0x17, 0x00, 0x1c, 0x00, 0x01, 0x09, 0x0a, 0x00, 0x02 };
	
	m_ICatchedPan = 0;
	m_ICatchedTilt = 0;
	m_ICatchedZoom = 0;
	CheckOutPTZPositionData( m_szRSDCB, 128, m_ICatchedPan, m_ICatchedTilt, m_ICatchedZoom );
	UpdateData(FALSE);
	return;

}

void CAbsolutePositionDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAbsolutePositionDlg::OnEnChangeEdit4()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAbsolutePositionDlg::OnEnChangeEdit5()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CAbsolutePositionDlg::OnEnChangeEdit6()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
