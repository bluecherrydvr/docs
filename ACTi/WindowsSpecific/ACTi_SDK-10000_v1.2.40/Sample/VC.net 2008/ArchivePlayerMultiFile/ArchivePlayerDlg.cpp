// ArchivePlayerDlg.cpp : 實作檔
//
#include "stdafx.h"
#include "ArchivePlayer.h"
#include "ArchivePlayerDlg.h"
#include ".\archiveplayerdlg.h"
#include <atlimage.h>
#include "SeqHeader.h"
//#include "xvid.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SS_JPG 1
#define SS_BMP 0

#define PLAY_RATE_RANGE_MIN	0
#define PLAY_RATE_RANGE_MAX 5

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


// CArchivePlayerDlg 對話方塊

void WINAPI TimeCodeCB(  DWORD UserParam, DWORD TimeCode )
{
	CArchivePlayerDlg* pAP = (CArchivePlayerDlg*)UserParam;
	if(pAP)
	{
		pAP->OnTimeCodeCB(TimeCode);
	}
}

void WINAPI FilePlayCompletetCB(DWORD UserParam) 
{
	CArchivePlayerDlg* pAP = (CArchivePlayerDlg*)UserParam;
	if(pAP)
	{
		pAP->OnFilePlayCompleteCB();
	}
}

void WINAPI ImageCB( DWORD UserParam, BYTE* buf, DWORD len, DWORD dwWidth, DWORD dwHeight )
{
	CArchivePlayerDlg* pAP = (CArchivePlayerDlg*)UserParam;

	if(pAP)
	{
		pAP->OnImageCB( (BYTE*)buf, len, dwWidth, dwHeight);
	}
}

void WINAPI AfterRenderCB( DWORD UserParam)
{
	CArchivePlayerDlg* pAP = (CArchivePlayerDlg*)UserParam;
	if(pAP)
	{
		pAP->OnAfterRenderCB();
	}
}

CArchivePlayerDlg::CArchivePlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CArchivePlayerDlg::IDD, pParent)
	//, m_sFileName(_T(""))
	, m_sBeginTime(_T(""))
	, m_sCurrentTime(_T(""))
	, m_sRawFile1Name(_T(""))
	, m_sRawFile2Name(_T(""))
	, m_sRawFile3Name(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CArchivePlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, m_cSCTime);
	DDX_Control(pDX, IDC_SLIDER2, m_cSCPlayRate);
	DDX_Control(pDX, IDC_STATIC1, m_cDisplay);
	DDX_Control(pDX, IDC_BUTTON1, m_cBnBack);
	DDX_Control(pDX, IDC_BUTTON4, m_cBnForward);
	DDX_Control(pDX, IDC_BUTTON2, m_cBnStop);
	DDX_Control(pDX, IDC_BUTTON3, m_cBnPause);
	
	DDX_Text(pDX, IDC_STATIC2, m_sBeginTime);
	DDX_Text(pDX, IDC_STATIC3, m_sEndTime);
	DDX_Text(pDX, IDC_STATIC4, m_sCurrentTime);
	DDX_Control(pDX, IDC_BUTTON6, m_cBnPicSave);

	DDX_Text(pDX, IDC_EDIT2, m_sRawFile1Name);
	DDX_Text(pDX, IDC_EDIT3, m_sRawFile2Name);
	DDX_Text(pDX, IDC_EDIT4, m_sRawFile3Name);
}

BEGIN_MESSAGE_MAP(CArchivePlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnNMReleasedcaptureSlider1)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER2, OnNMReleasedcaptureSlider2)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	//ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	//ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, OnBnClickedButton9)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnNMCustomdrawSlider1)
	ON_STN_CLICKED(IDC_STATIC4, OnStnClickedStatic4)
END_MESSAGE_MAP()


// CArchivePlayerDlg 訊息處理常式

BOOL CArchivePlayerDlg::OnInitDialog()
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

	// Disable progress border
//	this->m_cSCTime._EnableBorders(false);
	//this->m_cSCTime._SetPos(0);
	//m_cSCTime.SetPos(0);

	// Set range for playrate
	m_cSCPlayRate.SetRange(PLAY_RATE_RANGE_MIN, PLAY_RATE_RANGE_MAX);

	memset(&m_szFilePath, 0x00, 4096);
	char szFileName[2048] = {0};
	DWORD dwRet = GetModuleFileName(NULL, m_szFilePath, 2048);
	if(0 == dwRet)
		return FALSE;
	char *mid = m_szFilePath + dwRet;
	// 找執行檔名稱的起始點
	while (*(--mid) != '\\' && mid > m_szFilePath);
	if (mid <= m_szFilePath)
	{
		return FALSE;
	}
	*(++mid) = '\0';
	strcpy(szFileName, m_szFilePath);
	strcat(szFileName, "rec.raw\0");
	

	m_cSCPlayRate.SetPos(RATE_1_0 + 1);  // We add 1 here because first Pos is Frame by Frame.
	
	memset(&m_mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
	memset(&m_mri, 0x00, sizeof(MEDIA_RENDER_INFO));
	m_bFileComplete = false;
	m_dwBeginTime = 0;
	m_dwEndTime = 0;
	m_dwPlayRate = RATE_1_0;
	m_nFileType = SS_JPG;
	CheckDlgButton(IDC_RADIO2, BST_CHECKED);
	m_bSnapShoot = false;
	m_bForward = true;
	m_dwSlidePos = 0;
	m_bFBF = false;
	m_nFontSize = 0;
	m_nFileExtType = FILE_EXT_NONE;
	m_bTimeCodeChange = false;
	m_hBKBrush = CreateSolidBrush(RGB(0xE4,0xE4,0xE4));
	m_hPen = CreatePen(PS_SOLID, NULL, RGB(0, 0, 0));

	m_bSaveBuffer = new BYTE[1920*1080*4+1024];
	memset(m_bSaveBuffer, 0x00, 1920*1080*4+1024);
	m_dwSaveBufferLen = 0;

	UpdateData(FALSE);
	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CArchivePlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CArchivePlayerDlg::OnPaint() 
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
		RefreshDisplayArea();
		CDialog::OnPaint();
	}
	
}

//當使用者拖曳最小化視窗時，系統呼叫這個功能取得游標顯示。
HCURSOR CArchivePlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CArchivePlayerDlg::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{

	if( m_hKMpeg4 )
	{
		m_dwSlidePos = m_cSCTime.GetPos();
		KPause( m_hKMpeg4 );
		KSetCurrentTime(m_hKMpeg4, m_dwSlidePos);//when it's multiple raw playback, time is absolute. ( 0 ~ sum of file time (in sec)  )   //m_dwBeginTime+m_dwSlidePos);
		KPlay( m_hKMpeg4 );
	}
	*pResult = 0;
}

void CArchivePlayerDlg::OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{

	if(m_hKMpeg4)
	{
		int nPos = m_cSCPlayRate.GetPos();
		if(0 == nPos)
		{
			KPause(m_hKMpeg4);
			m_bFBF = true;
		}
		else
		{
			KSetPlayRate(m_hKMpeg4, nPos-1);
			m_bFBF = false;
			KPlay(m_hKMpeg4);
		}
	}
	*pResult = 0;
}

void CArchivePlayerDlg::OnBnClickedCancel()
{

	ResetAll();
	if(m_bSaveBuffer)
		delete [] m_bSaveBuffer;
	m_bSaveBuffer = NULL;
	DeleteObject(m_hBKBrush);
	m_hBKBrush = NULL;
	DeleteObject(m_hPen);
	m_hPen = NULL;
	OnCancel();
}

// Browse
void CArchivePlayerDlg::OnBnClickedButton5()
{

	//static char BASED_CODE szFilter[] = "Raw Date Files (*.raw)|*.raw|MP4 Files (*.mp4)|*.mp4|\0";

	////CFileDialog fd( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL,  0 );
	//CFileDialog fd( TRUE, NULL, NULL, OFN_EXPLORER, szFilter, NULL, 0 );
	//
	//if( fd.DoModal() == IDOK )
	//{
	//	m_sFileName = fd.GetPathName();
	//}

	UpdateData( FALSE );
}

// Play
void CArchivePlayerDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	if(!FileNameCheck())
		return;
	if(FILE_EXT_MRAW == m_nFileExtType)
	{
		onPlayMultiRaw();
	}

	else
	{
		AfxMessageBox("Unknown file");
	}
}



void CArchivePlayerDlg::onPlayMultiRaw()
{

	m_bSnapShoot = false;


	if(m_bFileComplete)
	{
		ResetAll();
	}

	if(NULL == m_hKMpeg4)
	{
		m_hKMpeg4 = KOpenInterface();
	}
	else
	{
		if(m_bFBF)
		{
			KStepNextFrame(m_hKMpeg4);
			m_bForward = true;
			return;
		}
		KSetPlayDirection(m_hKMpeg4, true);
		m_bForward = true;
		KPlay(m_hKMpeg4);
		return;
	}

	if(m_hKMpeg4)
	{
		memset(&m_mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
		m_mcc.ContactType = CONTACT_TYPE_PLAYBACK;
		//set media configure for multiple file playback
		if(!KSetMultipleMediaConfig(m_hKMpeg4 , &m_mcc))
		{
			AfxMessageBox("KSetMultipleMediaConfig fail\0");
			return;
		}

		// add multiple file into playing list
		char filename[256];
		if(m_sRawFile1Name.GetLength() > 0)
		{
			memset (filename,0,256);
			strcpy(filename,m_sRawFile1Name);
			if(!KAddMultipleMedia( m_hKMpeg4, 1, filename))
			{
				AfxMessageBox("Open raw file 1 fail\0");
				return;
			}
			
		}
		if(m_sRawFile2Name.GetLength() > 0)
		{
			memset (filename,0,256);
			strcpy(filename,m_sRawFile2Name);
			if(!KAddMultipleMedia( m_hKMpeg4, 2, filename))
			{
				AfxMessageBox("Open raw file 2 fail\0");
				return;
			}
		}
		if(m_sRawFile3Name.GetLength() > 0)
		{
			memset (filename,0,256);
			strcpy(filename,m_sRawFile3Name);
			if(!KAddMultipleMedia( m_hKMpeg4, 3, filename))
			{
				AfxMessageBox("Open raw file 3 fail\0");
				return;
			}
		}


		KSetTimeCodeCallback(m_hKMpeg4, (DWORD)this, TimeCodeCB);
		KSetFilePlayCompeleteCallback(m_hKMpeg4, (DWORD)this ,FilePlayCompletetCB);
		KSetImageCallback(m_hKMpeg4, (DWORD)this, ImageCB);
		KSetAfterRenderCallback(m_hKMpeg4, DWORD(this), AfterRenderCB );
		KSetRenderInfo(m_hKMpeg4, &m_mri);

		if(!KConnect(m_hKMpeg4))
		{
			AfxMessageBox("Connect raw files fail\0");
			return;
		}
		//KStartStreaming in raw playing case can't fail
		if(KStartStreaming(m_hKMpeg4)) 
		{
			char szBegin[64] = {0};
			char szEnd[64] = {0};
			// get begin time from first video file
			KGetNthBeginTimeFromMultipleMedia( m_hKMpeg4, 1,  m_dwBeginTime );
			
			GetTimeToStr(m_dwBeginTime, szBegin);
			GetDlgItem( IDC_STATIC2 )->SetWindowText(szBegin);
			m_sBeginTime = szBegin;
			// get end time from last (3rd) video file
			KGetNthEndTimeFromMultipleMedia( m_hKMpeg4, 3, m_dwEndTime );
			                                                     
			GetTimeToStr(m_dwEndTime, szEnd);
			GetDlgItem( IDC_STATIC3 )->SetWindowText(szEnd);
			m_sEndTime = szEnd;
			// In multiple file case, The end time is the sum of all file, and the begin time always zero.
			KGetEndTime(m_hKMpeg4, m_dwDuration); 

			m_cSCTime.SetRange(0, m_dwDuration);
			m_cSCTime.SetPos(0);
			// this->m_cSCTime._SetRange(0, m_dwDuration);
			m_dwSlidePos = 0;

			KSetMute(m_hKMpeg4, false);

			SetRender();
			OnBnEnable(true);
			KPlay(m_hKMpeg4);

			m_dwPlayRate = RATE_1_0;
			m_bForward = true;



			return;
		}



	}
	// failed
	ResetAll();
}



void CArchivePlayerDlg::GetTimeToStr(time_t time, char *buf )
{
	struct tm *newtime;
	if(0 > time)
	{
		buf[0] = '\0';
		return;
	}
	newtime = gmtime((time_t *)&time);
	sprintf( buf, "%04d/%02d/%02d %02d:%02d:%02d\0", newtime->tm_year+1900, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec );
	/*tt = localtime( &time );
	strcpy(buf, asctime(tt));*/
	//strftime( buf, 64, "%Y/%m/%d %H:%M:%S\0", asctime(tt) ); 
}

void CArchivePlayerDlg::OnTimeCodeCB(DWORD TimeCode)
{
	char szTimeBuf[64] = {0};
	GetTimeToStr(TimeCode, szTimeBuf);
	GetDlgItem( IDC_STATIC4 )->SetWindowText(szTimeBuf);
	m_sCurrentTime = szTimeBuf;
	if(m_bForward)
	{
		m_dwSlidePos+=1;
	}
	else
	{
		if(m_dwSlidePos > 0)
			m_dwSlidePos-=1;
		else
			m_dwSlidePos = 0;
	}
	m_bTimeCodeChange = true;
	//m_cSCTime.SetPos(m_dwSlidePos);
	//this->m_cSCTime._SetPos(m_dwSlidePos);
}

void CArchivePlayerDlg::OnFilePlayCompleteCB()
{
	m_bFileComplete = true;
	m_cSCTime.SetPos(0);

//	this->m_cSCTime._SetPos(0);
	m_dwSlidePos = 0;
	KSetCurrentTime(m_hKMpeg4, m_dwSlidePos);

	m_dwBeginTime = 0;
	m_dwEndTime = 0;
	m_dwDuration = 0;
	m_dwPlayRate = RATE_1_0;
	GetDlgItem( IDC_STATIC2 )->SetWindowText("\0");
	GetDlgItem( IDC_STATIC3 )->SetWindowText("\0");
	GetDlgItem( IDC_STATIC4 )->SetWindowText("\0");
	OnBnEnable(false);
}

void CArchivePlayerDlg::OnImageCB(BYTE* pBuf, DWORD nBufLen, DWORD dwWidth, DWORD dwHeight)
{
	if(NULL != pBuf && nBufLen > 0)
	{
		CopyMemory(m_bSaveBuffer, pBuf, nBufLen);
		m_dwSaveBufferLen = nBufLen;
	}
}

void CArchivePlayerDlg::ResetAll()
{
	if(m_hKMpeg4)
	{
		KSetImageCallback(m_hKMpeg4,  0, NULL );
		KSetTimeCodeCallback(m_hKMpeg4,  0, NULL );
		KSetFilePlayCompeleteCallback(m_hKMpeg4,  0, NULL );
		KSetAfterRenderCallback(m_hKMpeg4, 0, NULL );
		KStop(m_hKMpeg4);
		KStopStreaming(m_hKMpeg4);
		KDisconnect(m_hKMpeg4);
		KCloseInterface(m_hKMpeg4);
		m_hKMpeg4 = NULL;
	}
	m_dwPlayRate = RATE_1_0;
	m_cSCPlayRate.SetPos(RATE_1_0 + 1);
	m_bFileComplete = false;
	m_dwBeginTime = 0;
	m_dwEndTime = 0;
	m_bSnapShoot = false;
	GetDlgItem( IDC_STATIC2 )->SetWindowText("\0");			// Begin time
	GetDlgItem( IDC_STATIC3 )->SetWindowText("\0");			// End time
	GetDlgItem( IDC_STATIC4 )->SetWindowText("\0");			// Current time
	m_dwDuration = 0;
	m_bFBF = false;
	m_bForward = true;
	m_dwSlidePos = 0;
	m_bTimeCodeChange = false;

	OnBnEnable(false);
}

void CArchivePlayerDlg::OnBnEnable(bool bEnable)
{
	m_cBnPause.EnableWindow(bEnable);
	m_cBnBack.EnableWindow(bEnable);
	m_cBnStop.EnableWindow(bEnable);
	if(!bEnable)
	{
		m_cSCTime.SetPos(0);
//		this->m_cSCTime._SetPos(0);
	}
	m_cSCTime.EnableWindow(bEnable);
	m_cSCPlayRate.EnableWindow(bEnable);
	//m_cBnPicSave.EnableWindow(bEnable);
}

void CArchivePlayerDlg::SetRender()
{
	if(m_hKMpeg4)
	{
		m_mri.DrawerInterface = DGDI;
		m_mri.hWnd = m_cDisplay.m_hWnd;
		
		MEDIA_VIDEO_CONFIG mvc;
		memset(&mvc, 0x00, sizeof(MEDIA_VIDEO_CONFIG));
		KGetVideoConfig(m_hKMpeg4, &mvc);
		int nR = mvc.dwVideoResolution;
		RECT rc;
		m_cDisplay.GetClientRect(&rc);
		RefreshDisplayArea();

		if( nR == 0 )
		{
			m_mri.rect.top = 0;
			m_mri.rect.left = 0;
			m_mri.rect.right = rc.right - rc.left;
			m_mri.rect.bottom = rc.bottom - rc.top;
			m_nFontSize = 20;
		}
		else if( nR == 1 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 240/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 352/2;		
			m_mri.rect.right = 352 ;
			m_mri.rect.bottom = 240;
			m_nFontSize = 12;
		}
		else if( nR == 2 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 112/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 160/2;		
			m_mri.rect.right = 160;
			m_mri.rect.bottom = 112;
			m_nFontSize = 0;
		}
		else if( nR == 3 )
		{
			m_mri.rect.top = 0;
			m_mri.rect.left = 0;
			m_mri.rect.right = rc.right - rc.left;
			m_mri.rect.bottom = rc.bottom - rc.top;
			m_nFontSize = 20;
		}
		else if( nR == 4 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 288/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 352/2;		
			m_mri.rect.right = 352;
			m_mri.rect.bottom = 288;
			m_nFontSize = 12;
		}

		else if( nR == 5 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 144/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 176/2;		
			m_mri.rect.right = 176;
			m_mri.rect.bottom = 144;
			m_nFontSize = 0;
		}

		else if( nR == 6 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 120/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 176/2;		
			m_mri.rect.right = 176;
			m_mri.rect.bottom = 120;
			m_nFontSize = 0;
		}
		else if( nR == 64 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 480/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 640/2;		
			m_mri.rect.right = 640;
			m_mri.rect.bottom = 480;
			m_nFontSize = 20;
		}
		else if( nR == 192 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 480/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 640/2;		
			m_mri.rect.right = 640;
			m_mri.rect.bottom = 480;
			m_nFontSize = 20;
		}
		else
		{
			m_mri.rect.top = 0;
			m_mri.rect.left = 0;
			m_mri.rect.right = rc.right - rc.left;
			m_mri.rect.bottom = rc.bottom - rc.top;
			m_nFontSize = 20;
		}
		
		KSetRenderInfo(m_hKMpeg4, &m_mri);
		Sleep(100);
	}
}

// Play back direction
void CArchivePlayerDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	if(m_hKMpeg4)
	{
		if(m_bFBF)
		{
			KStepPrevFrame(m_hKMpeg4);
		}
		else
		{
			KSetPlayDirection(m_hKMpeg4, false);
			KPlay(m_hKMpeg4);
		}
	}
	m_bForward = false;
}

// Pause
void CArchivePlayerDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	if(m_hKMpeg4)
	{
		KPause(m_hKMpeg4);
	}
}

// Stop
void CArchivePlayerDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	ResetAll();
}

// Radio button JPG
void CArchivePlayerDlg::OnBnClickedRadio2()
{
	// TODO: Add your control notification handler code here
	CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO2 );
	m_nFileType = SS_JPG;
}

void CArchivePlayerDlg::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1 );
	m_nFileType = SS_BMP;
}

// Save snapshoot
void CArchivePlayerDlg::OnBnClickedButton6()
{
	if(FileNameCheck())
	{

		m_bSnapShoot = true;
		SaveImage();
		
	}
}

void CArchivePlayerDlg::OnAfterRenderCB()
{
	if(m_hKMpeg4)
	{
		if(strlen(m_mcc.PlayFileName) > 0)
		{
			char szTextOut[2048] = {0};
			sprintf(szTextOut, "%s %s", m_mcc.PlayFileName, m_sCurrentTime);
			int nTextLen = strlen(szTextOut);
			if(nTextLen > 0)
			{
				if(0 != m_nFontSize)
				{
					// This API dosn't work
					//KSetTextOut(m_hKMpeg4, 0, 0+m_mri.rect.left, 0+m_mri.rect.top, szTextOut, nTextLen, false, false, false, "Arial", m_nFontSize, RGB(0, 0, 255), BKMODE_TRANSPARENT, RGB(255, 255, 0));
			
				}
			}
		}
		if(m_bTimeCodeChange)
		{
			m_cSCTime.SetPos(m_dwSlidePos);
			m_bTimeCodeChange = false;
		}
//		this->m_cSCTime._SetPos(m_dwSlidePos);
	}
}

void CArchivePlayerDlg::GetVideoResolution( DWORD dwResolutionIndex, DWORD* dwWidth, DWORD* dwHeight )
{
	switch(dwResolutionIndex)
	{
	case 0:
		*dwWidth = 720;
		*dwHeight = 480;
		break;
	case 1:
		*dwWidth = 352;
		*dwHeight = 240;
		break;
	case 2:
		*dwWidth = 160;
		*dwHeight = 112;
		break;
	case 3:
		*dwWidth = 720;
		*dwHeight = 576;
		break;
	case 4:
		*dwWidth = 352;
		*dwHeight = 288;
		break;
	case 5:
		*dwWidth = 176;
		*dwHeight = 144;
		break;
	}
}

int CArchivePlayerDlg::StringToken(LPTSTR str, LPCTSTR szToken, char** szArray)
{
	int pos;
	int iCount = 0;
	// char caTok[16];

	while (str && *str && str != '\0') 
    {
		szArray[iCount ++] = str;
		if ((pos = strcspn(str, szToken)) < 0) 
        {
			break;
		}
		str += (pos);
		// if(*str == '\0' || *str == '\r' || *str == '\n')// if null-terminated
		if( *str == '\0' )// if null-terminated
			break;
		*str = 0;
		str ++;
		while (*str) 
        {
			if (*str != 0x20 && *str != '\t' )
				break;
			str ++;
		}
	}
	return iCount;
}


bool CArchivePlayerDlg::mf_PrepareDibBuffer(LPBITMAPINFO *lplpbi, DWORD dwX, DWORD dwY)
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

bool CArchivePlayerDlg::FileNameCheck()
{
	UpdateData();
	int nLen = m_sRawFile1Name.GetLength();
	nLen += m_sRawFile2Name.GetLength();
	nLen += m_sRawFile3Name.GetLength();
	if( nLen > 0)
	{
		m_nFileExtType = FILE_EXT_MRAW;
		return true;
	}

	else
	{
		m_nFileExtType = FILE_EXT_NONE;
		AfxMessageBox("No file input");
		return false;
	}

	return false;
}

void CArchivePlayerDlg::RefreshDisplayArea()
{
	RECT rc;
	m_cDisplay.GetClientRect(&rc);
	CDC* pCDC = m_cDisplay.GetDC();
	HDC hdc = pCDC->m_hDC;
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, m_hBKBrush);
	FillRect(hdc, &rc, m_hBKBrush);
	HPEN hOldPen = (HPEN)pCDC->SelectObject(m_hPen);
	pCDC->Rectangle(&rc);
	pCDC->SelectObject(hOldPen);
	SelectObject(hdc, hOldBrush);
	m_cDisplay.ReleaseDC(pCDC);
}

void CArchivePlayerDlg::GetDisplayInfo(int nResolution, RECT& rect, int& nFontSize)
{
	RECT rc;
	m_cDisplay.GetClientRect(&rc);

	if( nResolution == 0 )
	{
		rect.top = 0;
		rect.left = 0;
		rect.right = rc.right + rc.left;
		rect.bottom = rc.bottom + rc.top;
		nFontSize = 20;
	}
	else if( nResolution == 1 )
	{
		rect.top = ( rc.bottom - rc.top )/2 - 240/2;
		rect.left = ( rc.right-rc.left)/ 2 - 352/2;		
		rect.right = rect.left + 352 ;
		rect.bottom = rect.top + 240;
		nFontSize = 12;
	}
	else if( nResolution == 2 )
	{
		rect.top = ( rc.bottom - rc.top )/2 - 112/2;
		rect.left = ( rc.right-rc.left)/ 2 - 160/2;		
		rect.right = rect.left + 160;
		rect.bottom = rect.top + 112;
		nFontSize = 0;
	}
	else if( nResolution == 3 )
	{
		rect.top = 0;
		rect.left = 0;
		rect.right = rc.right + rc.left;
		rect.bottom = rc.bottom + rc.top;
		nFontSize = 20;
	}
	else if( nResolution == 4 )
	{
		rect.top = ( rc.bottom - rc.top )/2 - 288/2;
		rect.left = ( rc.right-rc.left)/ 2 - 352/2;		
		rect.right = rect.left + 352;
		rect.bottom = rect.top + 288;
		nFontSize = 12;
	}

	else if( nResolution == 5 )
	{
		rect.top = ( rc.bottom - rc.top )/2 - 144/2;
		rect.left = ( rc.right-rc.left)/ 2 - 176/2;		
		rect.right = rect.left + 176;
		rect.bottom = rect.top + 144;
		nFontSize = 0;
	}

	else if( nResolution == 6 )
	{
		rect.top = ( rc.bottom - rc.top )/2 - 120/2;
		rect.left = ( rc.right-rc.left)/ 2 - 176/2;		
		rect.right = rect.left + 176;
		rect.bottom = rect.top + 120;
		nFontSize = 0;
	}
	else if( nResolution == 64 )
	{
		rect.top = ( rc.bottom - rc.top )/2 - 480/2;
		rect.left = ( rc.right-rc.left)/ 2 - 640/2;		
		rect.right = rect.left + 640;
		rect.bottom = rect.top + 480;
		nFontSize = 20;
	}
	else if( nResolution == 192 )
	{
		rect.top = ( rc.bottom - rc.top )/2 - 480/2;
		rect.left = ( rc.right-rc.left)/ 2 - 640/2;		
		rect.right = rect.left + 640;
		rect.bottom = rect.top + 480;
		nFontSize = 20;
	}
	else
	{
		rect.top = 0;
		rect.left = 0;
		rect.right = rc.right + rc.left;
		rect.bottom = rc.bottom + rc.top;
		nFontSize = 20;
	}
}

void CArchivePlayerDlg::SaveImage()
{
	BYTE* pSaveBuffer ;
	pSaveBuffer = new BYTE[720*576*4+1024];
	char pathtocreate[1024] ={0};
	bool bgetpath = false;
	HANDLE hFind = NULL;
	UINT64 dwRet = 0;
	CopyMemory( pSaveBuffer, m_bSaveBuffer, m_dwSaveBufferLen );

	if (pSaveBuffer && m_dwSaveBufferLen > 0) 
	{
		LPBITMAPINFO lpbih = (LPBITMAPINFO)pSaveBuffer ;

		BITMAPFILEHEADER	oHeader ;
		oHeader.bfType = 0x4d42 ;
		oHeader.bfReserved1 = 0; 
		oHeader.bfReserved2 = 0; 
		oHeader.bfSize    = (DWORD)(sizeof(BITMAPFILEHEADER) + (lpbih->bmiHeader).biSize + (lpbih->bmiHeader).biSizeImage) ;
		oHeader.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + (lpbih->bmiHeader).biSize) ;

		time_t tTime;
		tTime = time(NULL);
		struct tm *pTmTime = localtime(&tTime);
		TCHAR szTime[24] = {0};
		_tcsftime(szTime, sizeof(szTime), _T("%Y%m%d_%H%M%S"), pTmTime);
		char szFileName[2048] = {0};

		if(SS_BMP ==  m_nFileType)
		{
			_sntprintf(szFileName, MAX_PATH, _T("%s%s.bmp\0"), m_szFilePath, szTime);
		}
		else
		{
			_sntprintf(szFileName, MAX_PATH, _T("%s%s.jpg\0"), m_szFilePath, szTime);
		}

		if( m_nFileType == SS_BMP )
		{
			CFile oImage;
			if( oImage.Open(szFileName, CFile::modeCreate | CFile::modeWrite ) )
			{
				oImage.Write( &oHeader, sizeof(BITMAPFILEHEADER) );

				int tmpHeight = lpbih->bmiHeader.biHeight ;
				int i ;

				if ( tmpHeight > 0 ) 
				{
					dwRet = lpbih->bmiHeader.biSize + (lpbih->bmiHeader.biWidth*lpbih->bmiHeader.biHeight*4);
					oImage.Write( pSaveBuffer, lpbih->bmiHeader.biSize + (lpbih->bmiHeader.biWidth*lpbih->bmiHeader.biHeight*4) ) ;
				} 
				else 
				{
					lpbih->bmiHeader.biHeight = -tmpHeight;
					oImage.Write( pSaveBuffer, lpbih->bmiHeader.biSize );
					for( i = lpbih->bmiHeader.biHeight-1 ; i >= 0 ; i-- ) 
					{
						oImage.Write( (pSaveBuffer+lpbih->bmiHeader.biSize + (i*lpbih->bmiHeader.biWidth*4)), lpbih->bmiHeader.biWidth*4) ;
					}
					dwRet = lpbih->bmiHeader.biSize + (lpbih->bmiHeader.biHeight * lpbih->bmiHeader.biWidth * 4);
				}

				oImage.Close();
			}
		}

		if( m_nFileType == SS_JPG )
		{
			CImage cImage;
			HBITMAP hBMP; 

			HDC hDC = ::GetDC( NULL );
			hBMP = CreateDIBitmap( hDC, &(lpbih->bmiHeader), CBM_INIT, pSaveBuffer+sizeof(lpbih->bmiHeader), lpbih, DIB_RGB_COLORS );
			::ReleaseDC( NULL, hDC );
			cImage.Attach(hBMP);
			
			cImage.Save( szFileName );
			DeleteObject(hBMP);
		}
	} 

	delete [] pSaveBuffer;
	pSaveBuffer = NULL;
	m_bSnapShoot = false;
}
void CArchivePlayerDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CArchivePlayerDlg::OnBnClickedButton7()
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "Raw Date Files (*.raw)|*.raw|\0";

	//CFileDialog fd( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL,  0 );
	CFileDialog fd( TRUE, NULL, NULL, OFN_EXPLORER, szFilter, NULL, 0 );

	if( fd.DoModal() == IDOK )
	{
		m_sRawFile1Name = fd.GetPathName();
	}

	UpdateData( FALSE );
}

void CArchivePlayerDlg::OnBnClickedButton8()
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "Raw Date Files (*.raw)|*.raw|\0";

	//CFileDialog fd( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL,  0 );
	CFileDialog fd( TRUE, NULL, NULL, OFN_EXPLORER, szFilter, NULL, 0 );

	if( fd.DoModal() == IDOK )
	{
		m_sRawFile2Name = fd.GetPathName();
	}

	UpdateData( FALSE );
}

void CArchivePlayerDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "Raw Date Files (*.raw)|*.raw|\0";

	//CFileDialog fd( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL,  0 );
	CFileDialog fd( TRUE, NULL, NULL, OFN_EXPLORER, szFilter, NULL, 0 );

	if( fd.DoModal() == IDOK )
	{
		m_sRawFile3Name = fd.GetPathName();
	}

	UpdateData( FALSE );
}

void CArchivePlayerDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CArchivePlayerDlg::OnStnClickedStatic4()
{
	// TODO: Add your control notification handler code here
}
