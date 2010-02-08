// ArchivePlayerDlg.cpp : 實作檔
//
#include "stdafx.h"
#include "ArchivePlayer.h"
#include "ArchivePlayerDlg.h"

//#include ".\archiveplayerdlg.h"
#include <atlimage.h>
#include "SeqHeader.h"
#include "xvid.h"
#include "MediaUtility.h"
#include <time.h>
#include ".\archiveplayerdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SS_JPG 1
#define SS_BMP 0

#define PLAY_RATE_RANGE_MIN	0
#define PLAY_RATE_RANGE_MAX 5

/*
// Image callback
void CALLBACK ImgCallback( DWORD UserParam, BYTE* buf, DWORD len, DWORD dwWidth, DWORD dwHeight )
{
	CArchivePlayerDlg* p =  (CArchivePlayerDlg*) UserParam;

	//CAdapterTesterDlg *p = (CAdapterTesterDlg*)UserParam;
	LPBITMAPINFO lpl = (LPBITMAPINFO)buf;

	//CMO( &buf[44], len-44, dwWidth, dwHeight );

	//lpl->bmiHeader.biBitCount = 24;
	//lpl->bmiHeader.biSizeImage = dwWidth*dwHeight*3;
	//for( int y = 0; y < dwHeight; y++ )
	//{
	//for( int x = 0; x < dwWidth; x++ )
	//{
	//buf[ 44 + (y*dwWidth*3) + (x*3) ] = aryImage2[y][x];
	//buf[ 44 + (y*dwWidth*3) + (x*3) + 1 ] = aryImage2[y][x];
	//buf[ 44 + (y*dwWidth*3) + (x*3) + 2 ] = aryImage2[y][x];
	//}	
	//}

	//if( p->m_oPTZDlg )
	//	p->m_oPTZDlg->SetImage( buf, len, dwWidth, dwHeight );

	lpl->bmiHeader.biBitCount = 32;
	lpl->bmiHeader.biSizeImage = dwWidth*dwHeight*4;
	lpl->bmiHeader.biHeight = abs(lpl->bmiHeader.biHeight);


	if(	p->m_bFullScreenInRatio == true && p->m_pFullScreen)
	{
		HDC	 hdc = NULL;
		HWND hWnd = p->m_hFullSCWnd;

		//           左上起'x','y' ,窗'寬''高'  圖起'x','y' ,窗'寬''高'
		StretchDIBits( hdc, 0, 0, 500, 500, 0, 0, 500, 500, (const void*)&lpl[1], lpl, DIB_RGB_COLORS, SRCCOPY );

		::ReleaseDC( hWnd, hdc );
	}

	////建出inverse image 用的 buffer
	//if(m_BufOfInverseRGBAimage == NULL)
	//{
	//	m_BufOfInverseRGBAimage = new BYTE[dwHeight*dwWidth*4];
	//}


	////用 copy memory 去翻正 image
	//for(UINT i = 0; i<dwHeight; ++i)
	//{
	//	BYTE* p_temp = &( m_BufOfInverseRGBAimage[((dwHeight-1)-i) * dwWidth*4 ] );
	//	memcpy( p_temp, &(buf[44+(i*dwWidth*4)]), (dwWidth*4) );
	//}

	


}
*/
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
	if(pAP->m_bFullScreenInRatio != true && pAP->m_mri.hWnd == pAP->m_hFullSCWnd)
	{
		pAP->SetRender();
		pAP->m_checkstrtch.SetCheck(0);
		pAP->OnBnClickedCheck1();

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
	, m_sFileName(_T(""))
	, m_sBeginTime(_T(""))
	, m_sCurrentTime(_T(""))
   
	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_cSCTime.m_MainHwnd = m_hwnd;
	m_pFullScreen = NULL;
	m_BufOfInverseRGBAimage = NULL;
	m_bFullScreenInRatio = false;


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
	DDX_Text(pDX, IDC_EDIT1, m_sFileName);
	DDX_Text(pDX, IDC_STATIC2, m_sBeginTime);
	DDX_Text(pDX, IDC_STATIC3, m_sEndTime);
	DDX_Text(pDX, IDC_STATIC4, m_sCurrentTime);
	DDX_Control(pDX, IDC_BUTTON6, m_cBnPicSave);
	DDX_Control(pDX, IDC_EDIT2, m_edtWriteDstPath);
	DDX_Control(pDX, IDC_EDIT3, m_FileName);
	DDX_Control(pDX, IDC_BUTTON7, m_cBnPre);
	DDX_Control(pDX, IDC_CHECK1, m_checkstrtch);
	DDX_Control(pDX, IDC_BUTTON8, m_FullScreenControl);
	DDX_Control(pDX, IDC_BUTTON5, m_Openfilename);
	//DDX_Control(pDX, IDC_BUTTON10, m_jpgbmpfile);
	DDX_Control(pDX, IDC_EDIT4, m_resolutionf);
	DDX_Control(pDX, IDC_EDIT8, m_videotypef);
	DDX_Control(pDX, IDC_EDIT9, m_starttimef);
	DDX_Control(pDX, IDC_EDIT10, m_endtimef);
	DDX_Control(pDX, IDC_EDIT6, m_frameratef);

	DDX_Control(pDX, IDC_CHECK2, m_CheckOSDFileName);
	DDX_Control(pDX, IDC_CHECK3, m_CheckOSDDateTime);
	
}

BEGIN_MESSAGE_MAP(CArchivePlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	//ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnNMReleasedcaptureSlider1)
	//ON_NOTIFY(NM_CLICK, IDC_SLIDER1, OnNMOnClickSlider1)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER2, OnNMReleasedcaptureSlider2)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnNMCustomdrawSlider2)
    ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnBnClickedButton8)
//	ON_BN_CLICKED(IDC_BUTTON9, OnBnClickedButton9)
//	ON_BN_CLICKED(IDC_BUTTON10, OnBnClickedButton10)
    ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
//ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnNMCustomdrawSlider1)
//    ON_BN_CLICKED(IDC_BUTTON10, OnBnClickedButton10)


ON_WM_LBUTTONDOWN()
ON_MESSAGE( WM_SLIDER_MLBD, OnSLIDER_MLBD )
ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// implementation for WM_MYMESSAGE handler
LRESULT CArchivePlayerDlg::OnSLIDER_MLBD( WPARAM wParam, LPARAM lParam ) 
{
	//OutputDebugString("Here get LB Down message\n");
	//CPoint point((int) wParam, (int) lParam);
	//OnLButtonDown(MK_LBUTTON,  point);
	return 0L;
}


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


	//m_strOutString = "";

	// Disable progress border
//	this->m_cSCTime._EnableBorders(false);
	//this->m_cSCTime._SetPos(0);
	//m_cSCTime.SetPos(0);

	// Set range for playrate
	m_cSCPlayRate.SetRange(PLAY_RATE_RANGE_MIN, PLAY_RATE_RANGE_MAX);
    
	//Steve0915
    //memset(&m_WriteFolderName, 0x00, 4096);
	//memset(&m_szFilePath, 0x00, 4096);
	char szFileName[2048] = {0};
	//DWORD dwRet = GetModuleFileName(NULL, m_WriteFolderName/*m_szFilePath*/, 2048);
	//if(0 == dwRet)
	//	return FALSE;
	//char *mid = m_WriteFolderName/*m_szFilePath*/ + dwRet;
	//// 找執行檔名稱的起始點
	//while (*(--mid) != '\\' && mid > m_WriteFolderName/*m_szFilePath*/);
	//if (mid <= m_WriteFolderName/*m_szFilePath*/)
	//{
	//	return FALSE;
	//}
	//*(++mid) = '\0';
	
	//strcpy(szFileName,m_szFilePath);
	//strcat(szFileName, "rec.raw\0");
	
	//Steve0916===============================
	strcpy(szFileName, m_WriteFolderName);
	m_sFileName = szFileName;
    //=========================================

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
	m_cSCTime.m_dwSlidePos = 0;
	m_bFBF = false;
	m_nFontSize = 0;
	m_nFileExtType = FILE_EXT_RAW;
	m_bTimeCodeChange = false;
	m_hBKBrush = CreateSolidBrush(RGB(0xE4,0xE4,0xE4));
	m_hPen = CreatePen(PS_SOLID, NULL, RGB(0, 0, 0));

	m_bSaveBuffer = new BYTE[1920*1080*4+1024];
	memset(m_bSaveBuffer, 0x00, 1920*1080*4+1024);
	m_dwSaveBufferLen = 0;
    //
	m_bFullWindow = true;
	//
    
	//
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


/*
void CArchivePlayerDlg::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	if( m_hKMpeg4 )
	{

		m_cSCTime.m_dwSlidePos = m_cSCTime.GetPos();
					
		KPause( m_hKMpeg4 );
		KSetCurrentTime(m_hKMpeg4, m_dwBeginTime+m_cSCTime.m_dwSlidePos);
		KPlay( m_hKMpeg4 );       
		
	}
	 
	*pResult = 0;	 	
}
*/
/*
void CArchivePlayerDlg::OnNMOnClickSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	if( m_hKMpeg4 )
	{
		POINT ClickPos;
		GetCursorPos(&ClickPos);
		//m_dwDuration*(ClickPos.x/(411-7));
		//0 to m_dwDuration  
		//7 to 411 slider  (the screen width)
		m_cSCTime.SetPos( m_dwDuration*((float)(ClickPos.x-7)/(float)(411)) );
		m_cSCTime.m_dwSlidePos = m_cSCTime.GetPos();

		//OutputDebugString("Clicked\n");

		KPause( m_hKMpeg4 );
		KSetCurrentTime(m_hKMpeg4, m_dwBeginTime+m_cSCTime.m_dwSlidePos);
		KPlay( m_hKMpeg4 );       

	}

	*pResult = 0;	 
}
*/
void CArchivePlayerDlg::OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	if(m_hKMpeg4)
	{
		int nPos = m_cSCPlayRate.GetPos();
		if(0 == nPos)
		{
			KPause(m_hKMpeg4);
			m_bFBF = true;
		    m_cBnBack.EnableWindow(true);
			m_cBnPre.EnableWindow(true);
			m_FullScreenControl.EnableWindow(false);
			m_checkstrtch.EnableWindow(false);
			m_cBnForward.EnableWindow(false);
			m_cSCTime.EnableWindow(false);
			
		}
		else
		{   
			
			KSetPlayRate(m_hKMpeg4, nPos-1);
			m_bFBF = false;
			KPlay(m_hKMpeg4);
			m_cBnBack.EnableWindow(false);
			m_cBnPre.EnableWindow(false);
			m_FullScreenControl.EnableWindow(true);
			m_checkstrtch.EnableWindow(true);
			m_cBnForward.EnableWindow(true);
			m_cSCTime.EnableWindow(true);
			onPlayRaw();
		}
   
		
	}
	*pResult = 0;
}

void CArchivePlayerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
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
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "Raw Date Files (*.raw)|*.raw|MP4 Files (*.mp4)|*.mp4|\0";

	//CFileDialog fd( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL,  0 );
	CFileDialog fd( TRUE, NULL, NULL, OFN_EXPLORER, szFilter, NULL, 0 );
	
	bool bOpenThanPlay = false;

	if( fd.DoModal() == IDOK )
	{   
        m_sFileName1 =fd.GetFileName();
		m_sFileName = fd.GetPathName();
		
		{
			CString strFolderName = m_sFileName.GetString();
			int nIdx = strFolderName.ReverseFind( '\\');
			strFolderName.SetAt( nIdx, '\0');
			m_WriteFolderName = strFolderName.GetString();
			m_edtWriteDstPath.SetWindowText( m_WriteFolderName.GetBuffer());
			m_FileName.SetWindowText( m_WriteFolderName.GetBuffer());
		}
        //Steve0912
		{
			CString strFolderName = m_sFileName1.GetString();
			//int nIdx = strFolderName.ReverseFind( '\\');
			//strFolderName.SetAt( nIdx, '\0');
			m_WriteFolderName = strFolderName.GetString();
			//m_edtWriteDstPath.SetWindowText( m_WriteFolderName.GetBuffer());
			m_FileName.SetWindowText( m_WriteFolderName.GetBuffer());
		}
		bOpenThanPlay = true;		
		
	}

	UpdateData( FALSE );

	if(bOpenThanPlay == true)
	{
		
		OnBnClickedButton4();
	    m_cBnBack.EnableWindow(false);
		m_cBnPre.EnableWindow(false);
		m_checkstrtch.SetCheck(1);
	}

}

// Play
void CArchivePlayerDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	if(!FileNameCheck())
		return;   
  
  
	    int IisCheck =m_checkstrtch.GetCheck();
      
       
		if(IisCheck)
		{
			//KEnableStretchMode( m_hKMpeg4 , true );
			SetRender();
            m_checkstrtch.SetCheck(1);		
		}
			
//

	if(FILE_EXT_RAW == m_nFileExtType)
	{   
		//		
		
        m_checkstrtch.EnableWindow(true);
	    //
		onPlayRaw();
        m_FullScreenControl.EnableWindow(true);
	    m_cBnBack.EnableWindow(false);
	    m_cBnPre.EnableWindow(false);
        
	    m_Openfilename.EnableWindow(false);
		
     		            
	}
	else if(FILE_EXT_MP4 == m_nFileExtType)
	{  	

		m_FullScreenControl.EnableWindow(false);
	    
		if(m_hKMpeg4)
		{
             ResetAll();
			 ReadMP4File(m_sFileName);	
			 OnBnClickedButton4();
			 
		    
		}else if(NULL == m_hKMpeg4)
		{
			//onPlayMP4();
			//ResetAll();
			ReadMP4File(m_sFileName);			
			
            
		}
	}
	else
	{
		AfxMessageBox("Unknown file");
	}

		
}

void CArchivePlayerDlg::onPlayRaw()
{
    

	m_bSnapShoot = false;
	memset(&m_mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
	strcpy(m_mcc.PlayFileName, m_sFileName);
	m_mcc.ContactType = CONTACT_TYPE_PLAYBACK;

	if(m_bFileComplete)
	{
		ResetAll();
		 
	}

	if(NULL == m_hKMpeg4)
	{
		m_hKMpeg4 = KOpenInterface();
		KSetSmoothFastPlayback( m_hKMpeg4, true );
		m_cSCTime.m_hKMpeg4 = m_hKMpeg4;
      
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
		if(KSetMediaConfig(m_hKMpeg4, &m_mcc))
		{
			KSetTimeCodeCallback(m_hKMpeg4, (DWORD)this, TimeCodeCB);
			KSetFilePlayCompeleteCallback(m_hKMpeg4, (DWORD)this ,FilePlayCompletetCB);
            //save jpg-bmp
			KSetImageCallback(m_hKMpeg4, (DWORD)this, ImageCB);
			KSetAfterRenderCallback(m_hKMpeg4, DWORD(this), AfterRenderCB );
			KSetRenderInfo(m_hKMpeg4, &m_mri);
			
          
			KSetRawDataCallback( m_hKMpeg4, (DWORD)this, CArchivePlayerDlg::OnRawDataCallBack );
			if(KConnect(m_hKMpeg4))
			{
				if(KStartStreaming(m_hKMpeg4))
				{
                    
					char szBegin[64] = {0};
					char szEnd[64] = {0};
					KGetBeginTime(m_hKMpeg4, m_dwBeginTime);
					m_cSCTime.m_dwBeginTime = m_dwBeginTime;
					GetTimeToStr(m_dwBeginTime, szBegin);
					GetDlgItem( IDC_STATIC2 )->SetWindowText(szBegin);
					m_sBeginTime = szBegin;
					
					//Steve0903====================================== 
					char token[256];						
					sprintf( token, "%s",szBegin );
					GetDlgItem( IDC_EDIT9 )->SetWindowText( token);
                   //================================================
					
					KGetEndTime(m_hKMpeg4, m_dwEndTime);
					GetTimeToStr(m_dwEndTime, szEnd);
					GetDlgItem( IDC_STATIC3 )->SetWindowText(szEnd);
					m_sEndTime = szEnd;
                    
					//Steve0912=======================================
					sprintf( token, "%s",szEnd );
					GetDlgItem( IDC_EDIT10 )->SetWindowText( token);
                    //================================================
										
					m_dwDuration = m_dwEndTime - m_dwBeginTime;
					m_cSCTime.m_dwDuration = m_dwDuration;
					m_cSCTime.SetRange(0, m_dwDuration);
									
					m_cSCTime.SetPos(0);
					
					//this->m_cSCTime._SetRange(0, m_dwDuration);
					m_cSCTime.m_dwSlidePos = 0;
					
                  
					KSetMute(m_hKMpeg4, false);

					SetRender();
					OnBnEnable(true);
					KPlay(m_hKMpeg4);
					
					                                                          
					m_dwPlayRate = RATE_1_0;
					m_bForward = true;
                    
                   					                   			
					return;
				}
				else
				{
					AfxMessageBox("Start stream fail\0");
				}
			}
			else
			{
				AfxMessageBox("Open file fail\0");
			}
		}
		else
		{
			AfxMessageBox("Unable to set Media Configuration\0");
		}
		
	}	
	else
	{
		AfxMessageBox("Invalid handle\0");
	}
    
	
	ResetAll();
}

void CArchivePlayerDlg::onPlayMP4()
{
	int iToken = 0;
	int iResolution = 0;
	DWORD dwVideoWidth = 0;
	DWORD dwVideoHeight = 0;
	char caTemp[1024] = {0};
	char* caArray[32];

	strcpy(caTemp, m_sFileName);

	// Is the file exist?
	FILE *fp = fopen(caTemp, "rb");  

	if (fp)  
	{
		fclose(fp);

		iToken = StringToken(caTemp, "_", (char**)caArray);
		iResolution = atoi(caArray[2]);

		CDC* thedc = m_cDisplay.GetDC();
		if (!thedc)
		{
			AfxMessageBox("Display Error");
			return;
		}
		HDC hDrawDC = thedc->m_hDC;
		RECT rc;
		RECT rect;
		m_cDisplay.GetClientRect(&rc);

		if( iResolution == 0 )
		{
			rect.top = 0;
			rect.left = 0;
			rect.right = rc.right + rc.left;
			rect.bottom = rc.bottom + rc.top;
		}
		else if( iResolution == 1 )
		{
			rect.top = ( rc.bottom - rc.top )/2 - 240/2;
			rect.left = ( rc.right-rc.left)/ 2 - 352/2;		
			rect.right = rect.left + 352 ;
			rect.bottom = rect.top + 240;
		}
		else if( iResolution == 2 )
		{
			rect.top = ( rc.bottom - rc.top )/2 - 112/2;
			rect.left = ( rc.right-rc.left)/ 2 - 160/2;		
			rect.right = rect.left + 160;
			rect.bottom = rect.top + 112;
		}
		else if( iResolution == 3 )
		{
			rect.top = 0;
			rect.left = 0;
			rect.right = rc.right + rc.left;
			rect.bottom = rc.bottom + rc.top;
		}
		else if( iResolution == 4 )
		{
			rect.top = ( rc.bottom - rc.top )/2 - 288/2;
			rect.left = ( rc.right-rc.left)/ 2 - 352/2;		
			rect.right = rect.left + 352;
			rect.bottom = rect.top + 288;
		}

		else if( iResolution == 5 )
		{
			rect.top = ( rc.bottom - rc.top )/2 - 144/2;
			rect.left = ( rc.right-rc.left)/ 2 - 176/2;		
			rect.right = rect.left + 176;
			rect.bottom = rect.top + 144;
		}

		else if( iResolution == 6 )
		{
			rect.top = ( rc.bottom - rc.top )/2 - 120/2;
			rect.left = ( rc.right-rc.left)/ 2 - 176/2;		
			rect.right = rect.left + 176;
			rect.bottom = rect.top + 120;
		}
		else if( iResolution == 64 )
		{
			rect.top = ( rc.bottom - rc.top )/2 - 480/2;
			rect.left = ( rc.right-rc.left)/ 2 - 640/2;		
			rect.right = rect.left + 640;
			rect.bottom = rect.top + 480;
		}
		else if( iResolution == 192 )
		{
			rect.top = ( rc.bottom - rc.top )/2 - 480/2;
			rect.left = ( rc.right-rc.left)/ 2 - 640/2;		
			rect.right = rect.left + 640;
			rect.bottom = rect.top + 480;
		}
		else
		{
			rect.top = 0;
			rect.left = 0;
			rect.right = rc.right + rc.left;
			rect.bottom = rc.bottom + rc.top;
		}
		m_cDisplay.ReleaseDC(thedc);
		DisplayMP4(m_sFileName, hDrawDC, rect);
		DisplayMP4Clear();
	}
	else
	{
		AfxMessageBox("File does not exist");
	}
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
		m_cSCTime.m_dwSlidePos+=1;
	}
	else
	{
		if(m_cSCTime.m_dwSlidePos > 0)
			m_cSCTime.m_dwSlidePos-=1;
		else
			m_cSCTime.m_dwSlidePos = 0;
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
	m_cSCTime.m_dwSlidePos = 0;
	m_dwBeginTime = 0;
	m_dwEndTime = 0;
	m_dwDuration = 0;
	m_dwPlayRate = RATE_1_0;
	GetDlgItem( IDC_STATIC2 )->SetWindowText("\0");
	GetDlgItem( IDC_STATIC3 )->SetWindowText("\0");
	GetDlgItem( IDC_STATIC4 )->SetWindowText("\0");
	

	OnBnEnable(false);
	m_Openfilename.EnableWindow(true);

    m_resolutionf.SetWindowText("");
    m_videotypef.SetWindowText("");
    m_starttimef.SetWindowText("");
    m_endtimef.SetWindowText("");
    m_frameratef.SetWindowText("");

	
}

void CArchivePlayerDlg::OnImageCB(BYTE* pBuf, DWORD nBufLen, DWORD dwWidth, DWORD dwHeight)
{
	if(NULL != pBuf && nBufLen > 0)
	{
		CopyMemory(m_bSaveBuffer, pBuf, nBufLen);
		m_dwSaveBufferLen = nBufLen;
	}

	CDC* pDC = m_cDisplay.GetDC();

	//m_strOutString = "Hello World!";
	//pDC->TextOut(0,0,m_strOutString);

	ReleaseDC(pDC);
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
		m_cSCTime.m_hKMpeg4 = NULL;
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
	m_cSCTime.m_dwSlidePos = 0;
	m_bTimeCodeChange = false;   
	
	OnBnEnable(false);
	
	
	
}

void CArchivePlayerDlg::OnBnEnable(bool bEnable)
{
	m_cBnPause.EnableWindow(bEnable);
	m_cBnBack.EnableWindow(bEnable);
	m_cBnStop.EnableWindow(bEnable);
	//steve0902
	m_cBnPre.EnableWindow(bEnable);
	m_checkstrtch.EnableWindow(bEnable);
	m_FullScreenControl.EnableWindow(bEnable);
//	m_jpgbmpfile.EnableWindow(bEnable);

	//m_CheckOSDFileName.EnableWindow(bEnable);
	//m_CheckOSDDateTime.EnableWindow(bEnable);
	m_CheckOSDFileName.SetCheck(1);
	m_CheckOSDDateTime.SetCheck(1);

	if(!bEnable)
	{
		m_cSCTime.SetPos(0);
//		this->m_cSCTime._SetPos(0);
        m_checkstrtch.SetCheck(0);
           
	}
	m_cSCTime.EnableWindow(bEnable);
	m_cSCPlayRate.EnableWindow(bEnable);
	m_cBnPicSave.EnableWindow(bEnable);
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

		//KEnableStretchMode( m_hKMpeg4 , true );
		
		int nR = mvc.dwVideoResolution;       
				
		     
		//Steve0912===============================================================================//
		DWORD dwVideoWidth, dwVideoHeight;
		GetVideoResolution( mvc.dwVideoResolution, &dwVideoWidth, &dwVideoHeight );
		{		
			//sprintf( token, "%d : %dx%d",  mvc.dwVideoResolution, dwVideoWidth, dwVideoHeight);
            char token[256];
			sprintf( token,"%dx%d", dwVideoWidth, dwVideoHeight);
			GetDlgItem( IDC_EDIT4 )->SetWindowText( token);

			sprintf( token,"%d",mvc.dwFps );
			GetDlgItem( IDC_EDIT6)->SetWindowText( token);
			
		}		

	
            
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
		else if( nR == 70 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 240/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 320/2;		
			m_mri.rect.right = 320;
			m_mri.rect.bottom = 240;
			m_nFontSize = 12;
		}
		else if( nR == 71 )
		{
			m_mri.rect.top = ( rc.bottom - rc.top )/2 - 120/2;
			m_mri.rect.left = ( rc.right-rc.left)/ 2 - 160/2;		
			m_mri.rect.right = 160;
			m_mri.rect.bottom = 120;
			m_nFontSize = 0;
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


		if( m_bFullWindow )
		{
			
			m_mri.rect.top = 0;
			m_mri.rect.left = 0;
			m_mri.rect.right = rc.right - rc.left;
			m_mri.rect.bottom = rc.bottom - rc.top;
			m_nFontSize = 20;			
				
		}

      
		KSetRenderInfo(m_hKMpeg4, &m_mri);
		//Sleep(100);
	}
		
      //m_checkstrtch.SetCheck(1);		
	
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
    m_FullScreenControl.EnableWindow(false);
	m_checkstrtch.EnableWindow(false);
	//m_bFullWindow = true;

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
 
	m_Openfilename.EnableWindow(true);
//	m_jpgbmpfile.EnableWindow(true);
	m_cBnForward.EnableWindow(true);

	m_resolutionf.SetWindowText("");
    m_videotypef.SetWindowText("");
    m_starttimef.SetWindowText("");
    m_endtimef.SetWindowText("");
    m_frameratef.SetWindowText("");	
    m_checkstrtch.SetCheck(1);
	
		
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
	
	//要改成save as
	if(FileNameCheck())
	{
		if(FILE_EXT_MP4 == m_nFileExtType)
		{
			ConvertRaw2Image(m_sFileName);
			
			/*if(SS_JPG== m_nFileType)
			{
				ConvertRaw2Image(m_sFileName);
			}
			else if (SS_BMP == m_nFileType)
			{
				ConvertRaw2Image(m_sFileName);
			}
			*/
		}
		else
		{
			m_bSnapShoot = true;
			SaveImage();
		}
		
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
					// This API dosn't work===steve
					//KSetTextOut(m_hKMpeg4, 0, 0, 0, szTextOut, nTextLen, true, false, false, "Arial", m_nFontSize, RGB(255, 255, 0), 1, RGB(0, 0, 255));
			      			
			}
		}
		if(m_bTimeCodeChange)
		{
			m_cSCTime.SetPos(m_cSCTime.m_dwSlidePos);
			m_bTimeCodeChange = false;
		}
//		this->m_cSCTime._SetPos(m_dwSlidePos);
	}
}

void CArchivePlayerDlg::GetVideoResolution( DWORD dwResolutionIndex, DWORD* dwWidth, DWORD* dwHeight )
{
	switch(dwResolutionIndex )
	{
	case 0:
		*dwWidth = 720;
		*dwHeight = 480;
		break;
	case 1:
		*dwWidth = 352;
		*dwHeight = 240;
		break;
		//Steve
    case 70:
        *dwWidth = 320;	
        *dwHeight = 240;					
		break;
    case 71:
        *dwWidth = 160;	
        *dwHeight = 120;					
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
	case 6: 
		*dwWidth = 176;	
		*dwHeight = 120;
		break;
	case 64: 
		*dwWidth = 640;	
		*dwHeight = 480;
		break;
	case 65: 
		*dwWidth = 1280;	
		*dwHeight = 720;
		break;
	case 66: 
		*dwWidth = 1280;	
		*dwHeight = 960;
		break;
	case 67: 
		*dwWidth = 1280;	
		*dwHeight = 1024;
		break;
	case 68: 
		*dwWidth = 1920;	
		*dwHeight = 1080;
		break;
    
	case 192: // VGA
		*dwWidth = 640;	
		*dwHeight = 480;
		break;
	default :				
		*dwWidth = 720;	
		*dwHeight = 480;


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

bool CArchivePlayerDlg::ReadMP4File(CString csFileName)
{

	
	
	RefreshDisplayArea();    
 
	char caTemp[1024] = {0};
	char* caArray[32];
	BYTE* pRawBuffer = new BYTE [1920*1080];
	BYTE* pOutBuf = new BYTE [1920*1080*4+1024];
	memset(pRawBuffer, 0x00, 1920*1080);
	memset(pOutBuf, 0x00, 1920*1080*4+1024);

	int iToken = 0;
	int iResolution = 0;
	DWORD dwVideoWidth = 0;
	DWORD dwVideoHeight = 0;
  

	strcpy(caTemp, csFileName.GetBuffer());
	csFileName.ReleaseBuffer();

	iToken = StringToken(caTemp, "_", (char**)caArray);

	iResolution = atoi(caArray[2]);

	GetVideoResolution( iResolution, &dwVideoWidth, &dwVideoHeight );

	strcpy(caTemp, csFileName.GetBuffer());
	csFileName.ReleaseBuffer();
    		
	// Is the file exist?
	FILE *fp = fopen(caTemp, "rb");  
	int iCount = 0;
	int iSize = 0;

	if (fp)  
	{
		do
		{
			iSize = 0;
			iSize = fread(pRawBuffer+21+iCount, 1, 1024, fp);
			iCount+=iSize;
		}
		while(!(iSize < 1024));
		fclose(fp);

		int iiiSize = sizeof(VariableHeader[iResolution]);
		iCount+=iiiSize;

		memcpy(pRawBuffer, VariableHeader[iResolution], iiiSize);
       

		//XVID
		xvid_gbl_init_t xvid_gbl_init;
		xvid_dec_create_t   m_xvidDecHandle;
		BITMAPINFO* pBmi = NULL;
		int xvidret;
		memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init));
		memset(&m_xvidDecHandle, 0, sizeof(m_xvidDecHandle));

		m_xvidDecHandle.version = XVID_VERSION;
		m_xvidDecHandle.height = 0;
		m_xvidDecHandle.width = 0;
		xvid_gbl_init.version = XVID_VERSION;
		xvidret = xvid_global(0, XVID_GBL_INIT, &xvid_gbl_init, 0);

		xvidret = xvid_decore(NULL, XVID_DEC_CREATE, &m_xvidDecHandle, NULL);

		mf_PrepareDibBuffer(&pBmi, dwVideoWidth, dwVideoHeight);
      

		// Decode video frame
		LPBITMAPINFOHEADER lpbih;
		xvid_dec_frame_t    xvidDecFrame;
		int iSyncLen = 0;

		xvidDecFrame.output.csp = XVID_CSP_BGR;

		xvidDecFrame.output.plane[0] = pOutBuf;
		xvidDecFrame.output.stride[0] = dwVideoWidth*3;
		xvidDecFrame.bitstream = pRawBuffer+iSyncLen;
		xvidDecFrame.length = iCount-iSyncLen;

		xvidDecFrame.general = XVID_LOWDELAY;
		xvidDecFrame.version = XVID_VERSION;

		xvidret = xvid_decore(m_xvidDecHandle.handle, XVID_DEC_DECODE, &xvidDecFrame, 0);

		// Draw

    	CDC* thedc = m_cDisplay.GetDC();
		if (!thedc)
		{
			if(NULL != pBmi)
				delete pBmi;
			pBmi = NULL;
			delete [] pRawBuffer;
			pRawBuffer = NULL;
			delete [] pOutBuf;
			pOutBuf = NULL;

			xvidret = xvid_decore(m_xvidDecHandle.handle, XVID_DEC_DESTROY, 0, 0);

			return false;
		}
        
		
       
		HDC hDrawDC = thedc->m_hDC;
		RECT mp4rc;
		int nFontSize;
		GetDisplayInfo(iResolution, mp4rc, nFontSize);

		// Draw Pic
		DWORD dwWidth = mp4rc.right - mp4rc.left;
		DWORD dwHeight = mp4rc.bottom - mp4rc.top;
		lpbih = (LPBITMAPINFOHEADER) pBmi;
		::SetStretchBltMode(hDrawDC, COLORONCOLOR);
		::StretchDIBits(hDrawDC,mp4rc.left,mp4rc.bottom,dwWidth,-dwHeight, 0 ,0, lpbih->biWidth, lpbih->biHeight, (const void*)pOutBuf, pBmi, DIB_RGB_COLORS, SRCCOPY);
	  
		// Write Text
#if 1	
		
		HFONT hFont = ::CreateFont(nFontSize, 0, 0, 0,400, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , "Verdana" ) ;
		::SetBkMode( hDrawDC, TRANSPARENT );
		::SelectObject( hDrawDC, hFont); 
		::SetTextColor( hDrawDC, RGB(0, 0, 255));
		::FillRect( hDrawDC, CRect(0,0,dwWidth,nFontSize), CBrush(RGB(0, 0, 0) ) ) ;
	    //::TextOut( hDrawDC,mp4rc.left,mp4rc.top, m_sCurrentTime , m_sCurrentTime.GetLength());
		::TextOut( hDrawDC,5,0,m_sFileName1,m_sFileName1.GetLength());
		

		DeleteObject(hFont);
		m_cDisplay.ReleaseDC(thedc);
		
#endif 
		if(NULL != pBmi)
			delete pBmi;
		pBmi = NULL;
		delete [] pRawBuffer;
		pRawBuffer = NULL;
		delete [] pOutBuf;
		pOutBuf = NULL;

		xvidret = xvid_decore(m_xvidDecHandle. handle, XVID_DEC_DESTROY, 0, 0);
 
		return true;
	}
	else
	{ 
		// can't find the file
	}                                                                

	delete [] pRawBuffer;
	pRawBuffer = NULL;
	delete [] pOutBuf;
	pOutBuf = NULL;
	return false;
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

	int nLen = m_sFileName.GetLength();
	if(0 != nLen)
	{
		char szlog[128] = {0};
		int nFindRaw = m_sFileName.Find("raw", nLen - 4);
		if(nFindRaw < 0)
			nFindRaw = m_sFileName.Find("RAW", nLen - 4);
		int nFindMp4 = m_sFileName.Find("mp4", nLen - 4);
		if(nFindMp4 < 0)
			nFindMp4 = m_sFileName.Find("MP4", nLen - 4);

		if(nFindRaw >= 0)
		{
			m_nFileExtType = FILE_EXT_RAW;
			return true;
		}
		else if(nFindMp4 >= 0)
		{
			m_nFileExtType = FILE_EXT_MP4;
			return true;
		}
		else
		{
			m_nFileExtType = FILE_EXT_NONE;
			return false;
		}
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
	pSaveBuffer = new BYTE[1920*1080*4+1024];
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
         
		//Steve0916=====================================================
		 int iIndex = m_sFileName.ReverseFind('.');     
         m_sFileName.SetAt(iIndex, '\0');          
         //==============================================================
		
		 if( m_nFileType == 0)
		{
			//m_szFilePath
		   	_sntprintf(szFileName, MAX_PATH, _T("%s_%s.bmp\0"), m_sFileName, szTime);
		}
		else 
		{   
            _sntprintf(szFileName, MAX_PATH, _T("%s_%s.jpg\0"), m_sFileName, szTime);
			
		}
	
		/*if( m_nFileType == 0 )
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
		}*/
		if( m_nFileType == 0 )
		{
			CImage cImage;
			HBITMAP hBMP; 

			HDC hDC = ::GetDC( NULL );

			hBMP = CreateDIBitmap( hDC, &(lpbih->bmiHeader), CBM_INIT, pSaveBuffer+sizeof(lpbih->bmiHeader), lpbih, DIB_RGB_COLORS );
			::ReleaseDC( NULL, hDC );
			cImage.Attach(hBMP);
			//
			HDC HDCImage = cImage.GetDC();
			HFONT hFont = ::CreateFont(20, 0, 0, 0, 20, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , "Arial" ) ;
			::SetBkColor(HDCImage, RGB(0, 0, 0));
			//::SetBkMode( HDCImage, TRANSPARENT );
			::SelectObject( HDCImage, hFont); 
			::SetTextColor( HDCImage, RGB(255, 255, 255) );
			std::string::size_type idx;
			std::string strDateTime = "";
			std::string strPlayFileName = m_mcc.PlayFileName;
			idx = strPlayFileName.rfind('\\');
			if(idx != std::string::npos)
			{
				if(m_CheckOSDFileName.GetCheck())
				{
					strDateTime =strPlayFileName.substr(idx+1);
					strDateTime += "   ";
				}
			}
			if(m_CheckOSDDateTime.GetCheck())
			{
				strDateTime += m_sCurrentTime;
			}
			if( m_CheckOSDFileName.GetCheck() || m_CheckOSDDateTime.GetCheck() )
			{
				
				::TextOut( HDCImage, 5,  0, strDateTime.c_str(), strDateTime.size());
			}
			cImage.ReleaseDC();
			//


			cImage.Save( szFileName );
			DeleteObject(hBMP);
		}

		if( m_nFileType == 1 )
		{
			CImage cImage;
			HBITMAP hBMP; 

			HDC hDC = ::GetDC( NULL );
			hBMP = CreateDIBitmap( hDC, &(lpbih->bmiHeader), CBM_INIT, pSaveBuffer+sizeof(lpbih->bmiHeader), lpbih, DIB_RGB_COLORS );
			::ReleaseDC( NULL, hDC );
			cImage.Attach(hBMP);
			//
			HDC HDCImage = cImage.GetDC();
			HFONT hFont = ::CreateFont(20, 0, 0, 0, 20, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , "Arial" ) ;
			::SetBkColor(HDCImage, RGB(0, 0, 0));
			//::SetBkMode( HDCImage, TRANSPARENT );
			::SelectObject( HDCImage, hFont); 
			::SetTextColor( HDCImage, RGB(255, 255, 255) );
			std::string::size_type idx;
			std::string strDateTime = "";
			std::string strPlayFileName = m_mcc.PlayFileName;
			idx = strPlayFileName.rfind('\\');
			if(idx != std::string::npos)
			{
				if(m_CheckOSDFileName.GetCheck())
				{
					strDateTime =strPlayFileName.substr(idx+1);
					strDateTime += "   ";
				}
			}
			if(m_CheckOSDDateTime.GetCheck())
			{
				strDateTime += m_sCurrentTime;
			}
			if( m_CheckOSDFileName.GetCheck() || m_CheckOSDDateTime.GetCheck() )
			{
				
				::TextOut( HDCImage, 5,  0, strDateTime.c_str(), strDateTime.size());
			}
			cImage.ReleaseDC();
			//

			cImage.Save( szFileName );
			DeleteObject(hBMP);
		}
	} 

	delete [] pSaveBuffer;
	pSaveBuffer = NULL;
	m_bSnapShoot = false;
}

void CArchivePlayerDlg::OnBnClickedButton7()
{		
	
	if(m_hKMpeg4)
	{
		
		if(m_bFBF)
		{			
			KStepNextFrame(m_hKMpeg4);
		}
		else
		{
			KSetPlayDirection(m_hKMpeg4, true);
			KPlay(m_hKMpeg4);
			
		}
	}
	m_bForward = true;
  
}

void CArchivePlayerDlg::OnBnClickedButton8()
{
	//m_bSnapShoot = false;
	//memset(&m_mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
	//strcpy(m_mcc.PlayFileName, m_sFileName);
	//m_mcc.ContactType = CONTACT_TYPE_PLAYBACK;

	//if(m_bFileComplete)
	//{
	//	ResetAll();
	//}

	//if(NULL == m_hKMpeg4)
	//{
	//	m_hKMpeg4 = KOpenInterface();
	//	KSetSmoothFastPlayback( m_hKMpeg4, true );
	//}
	//else
	//{
	//	if(m_bFBF)
	//	{
	//		KStepNextFrame(m_hKMpeg4);
	//		m_bForward = true;
	//		
	//		return;
	//	}
	//	KSetPlayDirection(m_hKMpeg4, true);
	//	m_bForward = true;
	//	KPlay(m_hKMpeg4);
	//	KEnableFullScreen(m_hKMpeg4, true);
	//	
 //       return;
	//}

	int IisCheck = m_checkstrtch.GetCheck();
	if(IisCheck)
	{
		m_bFullWindow = true;//(m_bFullWindow==TRUE)?FALSE:TRUE;
		KEnableFullScreen(m_hKMpeg4, m_bFullWindow);
	}else 
	{
		m_bFullScreenInRatio = true;
		if(!m_pFullScreen)
		{
			m_pFullScreen = new CVideoWindow( &m_bFullScreenInRatio );
		}
		m_hFullSCWnd = m_pFullScreen->GetSafeHwnd();
		m_pFullScreen->EnableWindow();
		m_pFullScreen->ShowWindow( SW_MAXIMIZE );
		//
        CDC* pDC = m_pFullScreen->GetDC();
		CBrush aBrush(BLACK_BRUSH); 
		CBrush* pOldBrush;
		pOldBrush = pDC->SelectObject(&aBrush);
		pDC->Rectangle(0,0,GetSystemMetrics( SM_CXSCREEN ),GetSystemMetrics( SM_CYSCREEN ));
		pDC->SelectObject(pOldBrush);
		
		ReleaseDC(pDC);
		//
		MEDIA_VIDEO_CONFIG mvc;
		memset(&mvc, 0x00, sizeof(MEDIA_VIDEO_CONFIG));
		KGetVideoConfig(m_hKMpeg4, &mvc);
		int nR = mvc.dwVideoResolution;       
		DWORD dwVideoWidth, dwVideoHeight;
		GetVideoResolution( mvc.dwVideoResolution, &dwVideoWidth, &dwVideoHeight );

		RECT rc;
		m_cDisplay.GetClientRect(&rc);

		//set mri
		m_mri.DrawerInterface = DGDI;
		m_mri.hWnd = m_hFullSCWnd;

		DWORD dwMaxScreenWidth = GetSystemMetrics( SM_CXSCREEN );
		DWORD dwMaxScreenHeight = GetSystemMetrics( SM_CYSCREEN );
		if( dwVideoHeight > dwMaxScreenHeight )
		{
			//m_mri.rect.left = float(dwMaxScreenWidth)*0.5f - float(dwVideoWidth)*0.5f;
			//m_mri.rect.right = dwVideoWidth;

			m_mri.rect.top = 0;
			m_mri.rect.bottom = dwMaxScreenHeight;

			DWORD dwRatioWidthShouldBe = DWORD(float(dwMaxScreenHeight) * ((float)dwVideoWidth/(float)dwVideoHeight));

			m_mri.rect.left = float(dwMaxScreenWidth)*0.5f - float(dwRatioWidthShouldBe)*0.5f;;
			m_mri.rect.right = dwRatioWidthShouldBe;
		}
		else if( dwVideoWidth >= dwMaxScreenWidth )
		{
			// change ratio
			m_mri.rect.top = 0;
			m_mri.rect.left = 0;
			m_mri.rect.right = dwMaxScreenWidth;

			//(dwScreenWidth) * (dwMaxScreenWidth);
			DWORD dwRatioHeightShouldBe = DWORD(float(dwMaxScreenWidth) * ((float)dwVideoHeight/(float)dwVideoWidth));
			m_mri.rect.top = float(dwMaxScreenHeight)*0.5f - float(dwRatioHeightShouldBe)*0.5f;
			m_mri.rect.bottom = dwRatioHeightShouldBe;
		}
		else if( dwVideoWidth < dwMaxScreenWidth )
		{
			//float(dwMaxScreenWidth - dwVideoWidth)*0.5f;

			m_mri.rect.left = float(dwMaxScreenWidth)*0.5f - float(dwVideoWidth)*0.5f;
			m_mri.rect.right = dwVideoWidth;

			m_mri.rect.top = float(dwMaxScreenHeight)*0.5f - float(dwVideoHeight)*0.5f;
			m_mri.rect.bottom = dwVideoHeight;
		}		
		// change ratio
		//m_mri.rect.top = 0;
		//m_mri.rect.left = 0;
		//m_mri.rect.right = dwMaxScreenWidth;

		////(dwScreenWidth) * (dwMaxScreenWidth);
		//DWORD dwRatioHeightShouldBe = DWORD(float(dwMaxScreenWidth) * ((float)dwVideoHeight/(float)dwVideoWidth));
		//m_mri.rect.top = float(dwMaxScreenHeight)*0.5f - float(dwRatioHeightShouldBe)*0.5f;
		//m_mri.rect.bottom = dwRatioHeightShouldBe;


		KSetRenderInfo(m_hKMpeg4, &m_mri);


		//KSetImageCallback(m_hKMpeg4, (DWORD)this, ImgCallback);
		//m_bFullScreenInRatio = true;
		//if(!m_pFullScreen)
		//{
		//	m_pFullScreen = new CVideoWindow( &m_bFullScreenInRatio );
		//}
		//m_hFullSCWnd = m_pFullScreen->GetSafeHwnd();
		//m_pFullScreen->EnableWindow();
		//m_pFullScreen->ShowWindow( SW_MAXIMIZE );
		

		//CWnd* pCwnd = GetForegroundWindow();

		//pCwnd->SetWindowPos(
		//	NULL,
		//	100,
		//	100,
		//	900,
		//	900,
		//	SWP_SHOWWINDOW );

		//RECT DrawRect;
		//DrawRect.top = 0;
		//DrawRect.bottom = 720;
		//DrawRect.left = 0;
		//DrawRect.right = 1280;
		//pCwnd->MoveWindow(&DrawRect, TRUE);

		//MoveWindow( FullScreenHWND,
		//	100,
		//	100,
		//	200,
		//	200,
		//	true);
	}


}

//void CArchivePlayerDlg::OnBnClickedButton9()
//{
//	// TODO: Add your control notification handler code here
//
//	m_bFullWindow = (m_bFullWindow==TRUE)?FALSE:TRUE;
//
//	
//	if( m_bFullWindow )
//	{
//		RECT rc;
//		m_cDisplay.GetClientRect(&rc);
//		RefreshDisplayArea();
//
//		m_mri.rect.top = 0;
//		m_mri.rect.left = 0;
//		m_mri.rect.right = rc.right - rc.left;
//		m_mri.rect.bottom = rc.bottom - rc.top;
//		m_nFontSize = 20;
//
//		KSetRenderInfo(m_hKMpeg4, &m_mri);
//		Sleep(100);
//	}
//	else
//	{
//		SetRender();
//	}
//}



void CArchivePlayerDlg::OnBnClickedCheck1()
{		
	int IisCheck =m_checkstrtch.GetCheck();

	if(IisCheck )
	{
		//KEnableStretchMode( m_hKMpeg4 , true );
		SetRender();
		
	}
	else 
	{
		//KEnableStretchMode( m_hKMpeg4 , false );

		MEDIA_VIDEO_CONFIG mvc;
		memset(&mvc, 0x00, sizeof(MEDIA_VIDEO_CONFIG));
		KGetVideoConfig(m_hKMpeg4, &mvc);
		int nR = mvc.dwVideoResolution;       
		DWORD dwVideoWidth, dwVideoHeight;
		GetVideoResolution( mvc.dwVideoResolution, &dwVideoWidth, &dwVideoHeight );		
	
		
		RECT rc;
		m_cDisplay.GetClientRect(&rc);		
		//RefreshDisplayArea();

		//int XLEN = GetSystemMetrics( SM_CXSCREEN );
        //int YLEN = GetSystemMetrics( SM_CYSCREEN );	
		
		//change ratio
	    
		m_mri.rect.top = 0;
		m_mri.rect.left = 0;
		m_mri.rect.right = rc.right - rc.left;
        //m_mri.rect.bottom = rc.bottom - rc.top;
       // m_nFontSize = 20;

		//(rc.right - rc.left) * (dwVideoHeight/dwVideoWidth);
		if( mvc.dwVideoResolution == 1 || mvc.dwVideoResolution == 4 || mvc.dwVideoResolution == 70  )
		{
			DWORD dwRatioHeightShouldBe = DWORD(float(rc.right - rc.left) * ( (float)dwVideoHeight/(float)dwVideoWidth)/2.2);
			m_mri.rect.top = float(rc.bottom - rc.top)*0.5f - float(dwRatioHeightShouldBe)*0.5f;
			m_mri.rect.bottom = dwRatioHeightShouldBe;
	        
			DWORD dwRatioWidthShouldBe = DWORD(float(rc.right - rc.left) * ( (float)dwVideoHeight/(float)dwVideoWidth)/1.7);
			m_mri.rect.left = float(rc.right - rc.left)*0.5f - float(dwRatioWidthShouldBe)*0.5f;
			m_mri.rect.right = dwRatioWidthShouldBe;
			
			KSetRenderInfo(m_hKMpeg4, &m_mri);
			//Sleep(10);
		}else if( mvc.dwVideoResolution == 0 || mvc.dwVideoResolution == 3 || mvc.dwVideoResolution == 64 || mvc.dwVideoResolution == 192 || mvc.dwVideoResolution == 65 || mvc.dwVideoResolution == 67)
		{
		    DWORD dwRatioHeightShouldBe = DWORD(float(rc.right - rc.left) * ( (float)dwVideoHeight/(float)dwVideoWidth));
			m_mri.rect.top = float(rc.bottom - rc.top)*0.5f - float(dwRatioHeightShouldBe)*0.5f;
			m_mri.rect.bottom = dwRatioHeightShouldBe;
			KSetRenderInfo(m_hKMpeg4, &m_mri);
			//Sleep(10);
		}else if( mvc.dwVideoResolution == 2 || mvc.dwVideoResolution == 6  || mvc.dwVideoResolution == 5 || mvc.dwVideoResolution == 71) 
		{
            DWORD dwRatioHeightShouldBe = DWORD(float(rc.right - rc.left) * ( (float)dwVideoHeight/(float)dwVideoWidth)/4);
			m_mri.rect.top = float(rc.bottom - rc.top)*0.5f - float(dwRatioHeightShouldBe)*0.5f;
			m_mri.rect.bottom = dwRatioHeightShouldBe;
	        
			DWORD dwRatioWidthShouldBe = DWORD(float(rc.right - rc.left) * ( (float)dwVideoHeight/(float)dwVideoWidth)/3);
			m_mri.rect.left = float(rc.right - rc.left)*0.5f - float(dwRatioWidthShouldBe)*0.5f;
			m_mri.rect.right = dwRatioWidthShouldBe;
			
			KSetRenderInfo(m_hKMpeg4, &m_mri);
		}

	}

	RefreshDisplayArea();
	
	
	//m_bFullWindow = (m_bFullWindow==TRUE)?FALSE:TRUE;

	/*if( m_bFullWindow )
	{
		RECT rc;
		m_cDisplay.GetClientRect(&rc);
		RefreshDisplayArea();

		m_mri.rect.top = 0;
		m_mri.rect.left = 0;
		m_mri.rect.right = rc.right - rc.left;
		m_mri.rect.bottom = rc.bottom - rc.top;
		m_nFontSize = 20;

		KSetRenderInfo(m_hKMpeg4, &m_mri);
		Sleep(10);

	    	
	}*/
		   
  
}



//void CArchivePlayerDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
//	// TODO: Add your control notification handler code here
//
//	OutputDebugString( "OnNMCustomdrawSlider1()");
//	*pResult = 0;
//}

//void CArchivePlayerDlg::OnBnClickedButton10()
//{
//	 
//	 char BASED_CODE szFilter[] = "jpg Files (*.jpg)|*.jpg|bmp Files (*.bmp)|*.bmp|\0";
//
//	//CFileDialog fp( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL,  0 );
//	  CFileDialog fp( TRUE, NULL, NULL, OFN_EXPLORER, szFilter , this, 0 );
//	 
//	
//	if( fp.DoModal() == IDOK )
//	{   
//         m_sFileName = fp.GetFileName() ;
//	  
//		 UpdateData( FALSE );
//	}
//
//	
//}




void CArchivePlayerDlg::OnLButtonDown(UINT nFlags, CPoint point)
{

	//用不到了
	//if( m_hKMpeg4 )
	//{
	//	//POINT ClickPos;
	//	//GetCursorPos(&ClickPos);
	//	//m_dwDuration*((float)(ClickPos.x-7)/(float)(411))
	//	//0 to m_dwDuration  
	//	//7 to 411 slider  (the screen width)
	//	m_cSCTime.SetPos( m_dwDuration*((float)(point.x-25)/(float)(720-25)) );
	//	m_cSCTime.m_dwSlidePos = m_cSCTime.GetPos();

	//	char tempString[256];
	//	sprintf(tempString,"x=%d,y=%d\n",point.x,point.y);
	//	OutputDebugString(tempString);

	//	KPause( m_hKMpeg4 );
	//	KSetCurrentTime(m_hKMpeg4, m_dwBeginTime+m_cSCTime.m_dwSlidePos);
	//	sprintf(tempString,"slider move:%d  %d+%d\n",m_dwBeginTime+m_cSCTime.m_dwSlidePos,m_dwBeginTime,m_cSCTime.m_dwSlidePos);
	//	OutputDebugString(tempString);
	//	KPlay( m_hKMpeg4 );       

	//}

	CDialog::OnLButtonDown(nFlags, point);
}

//抓不到 message
void CArchivePlayerDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	switch(nChar)
	{
	case VK_ESCAPE:
		if(m_bFullScreenInRatio == true)
		{
			m_bFullScreenInRatio = false;
			m_pFullScreen->ShowWindow( SW_HIDE );
			m_pFullScreen->CloseWindow();
			SetRender();
		}
		//KSetImageCallback(m_hKMpeg4,  0, NULL );
		break;
	case VK_F1:
		//SetWindowText("Whatever");
		SetRender();
		OutputDebugString("Here get Q Key Down message\n");
		break;
	//case VK_RETURN:
	//	SetWindowText("You pressed Enter");
	//	break;
	//case VK_F1:
	//	SetWindowText("Help is not available at the moment");
	//	break;
	//case VK_DELETE:
	//	SetWindowText("Can't Delete This");
	//	break;
	default:
		//SetWindowText("Whatever");
		CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
	}


	
}
