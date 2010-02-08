// MediaConverter v1.5.05Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "MediaConverter.h"
#include "MediaConverterDlg.h"
#include <time.h>

#ifdef _DEBUG
#include "vld.h"
//#include ".\mediaconverterdlg.h"
#define new DEBUG_NEW
#endif

#include "CFraw.h"
//#include ".\mediaconverterdlg.h"

// CMediaConverterDlg dialog---Steve1008
CString csLastFolder = NULL ;
static char BASED_CODE szFilter[] = "(*.raw;*.mp4)|*.raw;*.mp4|\0";
//static char BASED_CODE szFilter[] = "RAW Files (*.raw)|*.raw|\0";
//static char BASED_CODE szFilter[] = "RAW Files (*.raw)|*.raw|MP4 Files(*.mp4)|*.mp4|\0";


DWORD WINAPI ThreadEnter( LPVOID p )
{
	CMediaConverterDlg* pdlg = (CMediaConverterDlg*)p;
	pdlg->RunProcess();
	return 0;
}

LRESULT CMediaConverterDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CString szMsg ;
	switch( message )
	{
	case FILE_CLOSE:
		OnFileClose();
		//m_Message.AddString("FILE_CLOSE") ;
		break;
	case WM_TIMER:
		OnFileCloseTimer();
		break;
	case PRINT_INFO:
		szMsg.Format("PRINT_INFO %d %d",wParam,lParam) ;
		//m_Message.AddString(szMsg) ;
		OnPrintInfo((DWORD)wParam, (DWORD)lParam);
		break;
	//case PRINT_MP4INFO:
	//	OnPrintMP4Info();
		break;
	}
	return CDialog::WindowProc( message, wParam, lParam);
}

CMediaConverterDlg::CMediaConverterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaConverterDlg::IDD, pParent)
	, m_csOSDText(_T(""))
	, m_FolderName(_T(""))
	, m_FileName(_T(""))
	, m_OutFileName( _T(""))
	, m_nConvertedFile(0)
	, m_nIFrame(0)
	, m_nPFrame(0)
	, m_nAFrame(0)
	, m_tFileEndDateTime(0)
	, m_tFileBeginDateTime(0)
	, m_tFileDateTimeZone(0)
	, m_tSpecifyBeginDateTime(0)
	, m_tSpecifyEndDateTime(0)

{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMediaConverterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK2, m_OSD1);
	DDX_Control(pDX, IDC_CHECK3, m_OSD2);
	DDX_Control(pDX, IDC_RADIO5, m_OSD1_1);
	DDX_Control(pDX, IDC_RADIO6, m_OSD1_2_1);
	DDX_Control(pDX, IDC_EDIT3, m_OSD1_2_2);
	DDX_Control(pDX, IDC_STATIC_2, m_OSD1_2_3);
	DDX_Control(pDX, IDC_RADIO7, m_OSD2_1);
	DDX_Control(pDX, IDC_RADIO8, m_OSD2_2);
	DDX_Control(pDX, IDC_RADIO9, m_OSD2_3);
	DDX_Control(pDX, IDC_CHECK1, m_OSD);
	DDX_Text(pDX, IDC_EDIT3, m_csOSDText);
	DDV_MaxChars(pDX, m_csOSDText, 20);
	DDX_Control(pDX, IDC_LIST1, m_Message);
	DDX_Text(pDX, IDC_EDIT2, m_FolderName);
	DDX_Text(pDX, IDC_EDIT1, m_FileName);
	DDX_Text(pDX, IDC_EDIT5, m_nConvertedFile);
	DDX_Text(pDX, IDC_EDIT6, m_nIFrame);
	DDX_Text(pDX, IDC_EDIT7, m_nPFrame);
	DDX_Control(pDX, IDC_RADIO3, m_SRC1);
	DDX_Control(pDX, IDC_RADIO4, m_SRC2);

	DDX_Control(pDX, IDC_EDIT8, m_edtWriteDstPath);
	DDX_Control(pDX, IDC_EDIT9, m_edtUSh);

	DDX_Control(pDX, IDC_EDIT11, m_edtUSm);
	DDX_Control(pDX, IDC_EDIT12, m_edtUSs);
	DDX_Control(pDX, IDC_EDIT10, m_edtUSendh);
	DDX_Control(pDX, IDC_EDIT13, m_edtUSendm);
	DDX_Control(pDX, IDC_EDIT14, m_edtUSends);

	DDX_Control(pDX, IDC_EDIT15, m_edtFileBeginDateTime);
	DDX_Control(pDX, IDC_EDIT16, m_edtFileEndDateTime);
	DDX_Control(pDX, IDC_RADIO10, m_chkRADIO10);
	DDX_Control(pDX, IDC_RADIO13, m_Rcmp4tojpg);
	DDX_Control(pDX, IDC_RADIO11, m_chkRADIO11);
	DDX_Control(pDX, IDC_RADIO12, m_chkRADIO12);
}

BEGIN_MESSAGE_MAP(CMediaConverterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_RADIO10, OnBnClickedRadio10)
	ON_BN_CLICKED(IDC_RADIO11, OnBnClickedRadio11)
    ON_BN_CLICKED(IDC_RADIO12, OnBnClickedRadio12)
  

	ON_BN_CLICKED(IDC_RADIO3, OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnBnClickedRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnBnClickedRadio8)
	ON_BN_CLICKED(IDC_RADIO9, OnBnClickedRadio9)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	
	
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	ON_BN_CLICKED(IDC_RADIO13, OnBnClickedRadio13)
END_MESSAGE_MAP()


// CMediaConverterDlg message handlers

BOOL CMediaConverterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	
   	CheckRadioButton( IDC_RADIO3, IDC_RADIO4, IDC_RADIO3 );
	//CheckRadioButton( IDC_RADIO10, IDC_RADIO12, IDC_RADIO11 );
	CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO5 );
	CheckRadioButton( IDC_RADIO7, IDC_RADIO9, IDC_RADIO7 );
    GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
   
	 m_nFileSrcType = FILE_SRC_RAW;
	 //m_nFileExtType = FILE_EXT_AVI ;

	m_bConvertFile   = true ;
	m_bConvertFolder = false ;

	m_hThread = NULL;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMediaConverterDlg::OnPaint() 
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
HCURSOR CMediaConverterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMediaConverterDlg::OnBnClickedRadio11()
{
	CheckRadioButton( IDC_RADIO10, IDC_RADIO13,IDC_RADIO11 );
	
   
	//strcpy( szFilter, "RAW Files (*.raw)|*.raw|\0" ) ;
	strcpy( szFilter, "(*.raw;*.mp4)|*.raw;*.mp4|\0" ) ;
	
	m_nFileSrcType = FILE_SRC_RAW;
	m_nFileExtType = FILE_EXT_AVI;
	m_OSD.EnableWindow( TRUE);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	
}

void CMediaConverterDlg::OnBnClickedRadio12()
{
    CheckRadioButton( IDC_RADIO10, IDC_RADIO13, IDC_RADIO12 );
	

	//strcpy( szFilter, "RAW Files (*.raw)|*.raw|\0" ) ;
	strcpy( szFilter, "(*.raw;*.mp4)|*.raw;*.mp4|\0" ) ;
	m_nFileSrcType = FILE_SRC_RAW;
	m_nFileExtType  = FILE_EXT_JPG ;
	m_OSD.EnableWindow( TRUE);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
  
	
}

void CMediaConverterDlg::OnBnClickedRadio10()
{

    CheckRadioButton( IDC_RADIO10, IDC_RADIO13,IDC_RADIO10);
	

	//strcpy( szFilter, "RAW Files (*.raw)|*.raw|\0" ) ;
	strcpy( szFilter, "(*.raw;*.mp4)|*.raw;*.mp4|\0" ) ;
	m_nFileSrcType = FILE_SRC_RAW;
	m_nFileExtType = FILE_EXT_RAW ;

	m_OSD.SetCheck( 0);
	OnBnClickedCheck1();
	m_OSD.EnableWindow( FALSE);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
    
	
}


void CMediaConverterDlg::OnBnClickedRadio13()
{
    CheckRadioButton( IDC_RADIO10, IDC_RADIO13, IDC_RADIO13 );
   
	//strcpy( szFilter, "MP4 Files (*.mp4)|*.mp4|\0" ) ;
	strcpy( szFilter, "(*.raw;*.mp4)|*.raw;*.mp4|\0" ) ;
	m_nFileSrcType = FILE_SRC_MP4;
	m_nFileExtType = FILE_EXT_JPG;
	//m_nFileExtType = FILE_EXT_MP4 ;
      
    OnBnClickedCheck1();
	m_OSD.EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
    
	//
	m_edtFileBeginDateTime.EnableWindow(FALSE);
    m_edtFileEndDateTime.EnableWindow(FALSE);
    m_edtUSh.EnableWindow(FALSE);
    m_edtUSm.EnableWindow(FALSE);
    m_edtUSs.EnableWindow(FALSE);
    m_edtUSendh.EnableWindow(FALSE);
    m_edtUSendm.EnableWindow(FALSE);
    m_edtUSends.EnableWindow(FALSE);
    m_Rcmp4tojpg.EnableWindow(TRUE);
	//
}




void CMediaConverterDlg::OnBnClickedRadio3()
{
	CheckRadioButton( IDC_RADIO3, IDC_RADIO4, IDC_RADIO3 );
    if( m_FileExt == "raw" )
	{
	    m_chkRADIO10.EnableWindow(TRUE);
		m_chkRADIO11.EnableWindow(TRUE);
	    m_chkRADIO12.EnableWindow(TRUE);
         

	}else if( m_FileExt == "mp4" )
	{
        m_chkRADIO10.EnableWindow(FALSE);
		m_chkRADIO11.EnableWindow(FALSE);
	    m_chkRADIO12.EnableWindow(FALSE);
		m_Rcmp4tojpg.EnableWindow(TRUE);
	
	}
	//
	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT1)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT8)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
	//
   
	
	//Specify Begin Time
	GetDlgItem(IDC_EDIT9)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT11)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT12)->EnableWindow(TRUE);
    
	//Specify End Time
    GetDlgItem(IDC_EDIT10)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT13)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT14)->EnableWindow(TRUE);
  
	m_bConvertFile   = true ;
	m_bConvertFolder = false ;

	m_chkRADIO10.EnableWindow( TRUE);
}

void CMediaConverterDlg::OnBnClickedRadio4()
{
	CheckRadioButton( IDC_RADIO3, IDC_RADIO4, IDC_RADIO4 );
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	//GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT8)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT2)->EnableWindow(TRUE);
	//
	m_Rcmp4tojpg.EnableWindow(FALSE);
    
	//Specify Begin Time
    GetDlgItem(IDC_EDIT9)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT11)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT12)->EnableWindow(FALSE);
    
    //Specify End Time
    GetDlgItem(IDC_EDIT10)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT13)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT14)->EnableWindow(FALSE);
 
	m_bConvertFile   = false ;
	m_bConvertFolder = true ;
    //Steve
	CheckRadioButton( IDC_RADIO10, IDC_RADIO13, IDC_RADIO11 );
	m_chkRADIO10.EnableWindow( FALSE);

}

void CMediaConverterDlg::OnBnClickedRadio5()
{
	CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO5 );
	GetDlgItem(IDC_EDIT3)->EnableWindow(FALSE);
}

void CMediaConverterDlg::OnBnClickedRadio6()
{
	CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO6 );
	GetDlgItem(IDC_EDIT3)->EnableWindow(TRUE);
}

void CMediaConverterDlg::OnBnClickedRadio7()
{
	CheckRadioButton( IDC_RADIO7, IDC_RADIO9, IDC_RADIO7 );
	m_cAVIWriter.SetOSDTimeType( 1 ) ;
	m_cJPGWriter.SetJPGOSDTimeType( 1);
	m_cMP4Writer.SetMP4OSDTimeType( 1);
}

void CMediaConverterDlg::OnBnClickedRadio8()
{
	CheckRadioButton( IDC_RADIO7, IDC_RADIO9, IDC_RADIO8 );
	m_cAVIWriter.SetOSDTimeType( 2 );
	m_cJPGWriter.SetJPGOSDTimeType( 2);
	m_cMP4Writer.SetMP4OSDTimeType( 2);
}

void CMediaConverterDlg::OnBnClickedRadio9()
{
	CheckRadioButton( IDC_RADIO7, IDC_RADIO9, IDC_RADIO9 );
	m_cAVIWriter.SetOSDTimeType( 3 );
	m_cJPGWriter.SetJPGOSDTimeType( 3);
	m_cMP4Writer.SetMP4OSDTimeType( 3);
}

void CMediaConverterDlg::OnBnClickedCheck1()
{
	BOOL bOSD = m_OSD.GetCheck() ;
	m_cAVIWriter.SetOSD( bOSD );
	m_cJPGWriter.SetJPGOSD( bOSD);
	m_cMP4Writer.SetMP4OSD( bOSD );

	GetDlgItem(IDC_CHECK2)->EnableWindow(bOSD);
	GetDlgItem(IDC_CHECK3)->EnableWindow(bOSD);

	m_OSD1.SetCheck(0) ;
	m_OSD2.SetCheck(0) ;

	OnBnClickedCheck2();
	OnBnClickedCheck3();
}

void CMediaConverterDlg::OnBnClickedCheck2()
{
	BOOL bOSD = m_OSD1.GetCheck() ;
	GetDlgItem(IDC_RADIO5)->EnableWindow(bOSD);
	GetDlgItem(IDC_RADIO6)->EnableWindow(bOSD);
	GetDlgItem(IDC_STATIC_2)->EnableWindow(bOSD);

	if( !bOSD )
	{
		CheckRadioButton( IDC_RADIO5, IDC_RADIO6, IDC_RADIO5 );
		GetDlgItem(IDC_EDIT3)->EnableWindow(bOSD);
		m_csOSDText ="" ;
	}
	UpdateData(FALSE) ;
}

void CMediaConverterDlg::OnBnClickedCheck3()
{
	BOOL bOSD = m_OSD2.GetCheck() ;
	GetDlgItem(IDC_RADIO7)->EnableWindow(bOSD);
	GetDlgItem(IDC_RADIO8)->EnableWindow(bOSD);
	GetDlgItem(IDC_RADIO9)->EnableWindow(bOSD);
	CheckRadioButton( IDC_RADIO7, IDC_RADIO9, IDC_RADIO7 );
	m_cAVIWriter.SetOSDTimeType( m_OSD2.GetCheck() ) ;
	m_cJPGWriter.SetJPGOSDTimeType( m_OSD2.GetCheck());
	m_cMP4Writer.SetMP4OSDTimeType( m_OSD2.GetCheck());
}

void CMediaConverterDlg::OnBnClickedButton2()
{   
	//steve0831
	CFolderDialog dlg( _T( "Browse Folder" ), m_FolderName, this,0 );    
    //
	
	//
	if( dlg.DoModal() == IDOK  )
    {    
		m_FolderName = dlg.GetFolderPath(); 	
        
		{
			CString strFolderName =  m_FolderName.GetString();
			
			//int nIdx = strFolderName.ReverseFind( '\\');
			//strFolderName.SetAt( nIdx, '\0');
			m_WriteFolderName = strFolderName.GetString();
			m_edtWriteDstPath.SetWindowText( m_WriteFolderName.GetBuffer());
		}
	 
 
		UpdateData(FALSE) ;
	}
	
}

void CMediaConverterDlg::DisplayFileDateTime( CMpeg4Reader * pFileReader )
{
	if( pFileReader )
	{
		RawFileTail_t TailInfo;
		memset( &TailInfo, 0, sizeof( TailInfo));
		LPVOID p = &TailInfo;
		pFileReader->GetRawTail( TailInfo);
		

				
		// UTC TimeCode
		time_t dwFileBeginTime  = TailInfo.dwBeginTime;// + (TailInfo.dwTimeZone-12)*60*60; 
		time_t dwFileEndTime    = TailInfo.dwEndTime;// + (TailInfo.dwTimeZone-12)*60*60; 
		DWORD dwTimeZone = TailInfo.dwTimeZone;
        //Steve0903
		if(dwTimeZone > 25)
		{
		AfxMessageBox( "Raw File TimeZone faile !!");
		return ;
		}
		
		// Local TimeCode
		DWORD dwLocalBeginTime = dwFileBeginTime + (dwTimeZone-12)*60*60; 
		DWORD dwLocalEndTime = dwFileEndTime + (dwTimeZone-12)*60*60;

		struct tm begintm = *(gmtime( (time_t *)&dwLocalBeginTime ));
		struct tm endtm   = *(gmtime( (time_t *)&dwLocalEndTime));

		char token[256];
			
		sprintf( token, "%04d%02d%02d %02d:%02d:%02d",
		begintm.tm_year+1900, begintm.tm_mon+1, begintm.tm_mday,
		begintm.tm_hour, begintm.tm_min, begintm.tm_sec);
		m_edtFileBeginDateTime.SetWindowText( token);

		sprintf( token, "%02d", begintm.tm_hour);
		m_edtUSh.SetWindowText( token);

		sprintf( token, "%02d", begintm.tm_min );
		m_edtUSm.SetWindowText( token);

		sprintf( token, "%02d", begintm.tm_sec );
		m_edtUSs.SetWindowText( token);
		
		
        sprintf( token, "%04d%02d%02d %02d:%02d:%02d",
        endtm.tm_year+1900, endtm.tm_mon+1, endtm.tm_mday,
		endtm.tm_hour, endtm.tm_min, endtm.tm_sec);
		m_edtFileEndDateTime.SetWindowText( token);

		sprintf( token, "%02d", endtm.tm_hour );
		m_edtUSendh.SetWindowText( token);

		sprintf( token, "%02d", endtm.tm_min );
		m_edtUSendm.SetWindowText( token);

		sprintf( token, "%02d", endtm.tm_sec );
		m_edtUSends.SetWindowText( token);

		
		
        m_tFileBeginDateTime = dwFileBeginTime;
		m_tFileEndDateTime   = dwFileEndTime;
		m_tFileDateTimeZone = dwTimeZone;


		m_tSpecifyBeginDateTime = dwFileBeginTime;
		m_tSpecifyEndDateTime = dwFileEndTime;
		
	}
}

void CMediaConverterDlg::HandleUserSpecifyDateTime( void)
{   
   
	char token[256];

	if( m_tFileBeginDateTime == 0 ||
		m_tFileEndDateTime == 0 )
		return;

	DWORD dwLocalBeginTime = m_tFileBeginDateTime + (m_tFileDateTimeZone-12)*60*60; 
	DWORD dwLocalEndTime = m_tFileEndDateTime + (m_tFileDateTimeZone-12)*60*60; 

	struct tm begintm = *(gmtime( (time_t *)&dwLocalBeginTime));
	struct tm endtm = *(gmtime( (time_t *)&dwLocalEndTime));

	{
		m_edtUSh.GetWindowText( token, sizeof(token));
		begintm.tm_hour = atoi( token);
		m_edtUSm.GetWindowText( token, sizeof(token));
		begintm.tm_min = atoi( token);
		m_edtUSs.GetWindowText( token, sizeof(token));
		begintm.tm_sec = atoi( token);
	}

	{
		m_edtUSendh.GetWindowText( token, sizeof(token));
		endtm.tm_hour = atoi( token);
		m_edtUSendm.GetWindowText( token, sizeof(token));
		endtm.tm_min = atoi( token);
		m_edtUSends.GetWindowText( token, sizeof(token));
		endtm.tm_sec = atoi( token);
	}

	timeb localtb;
	ftime( &localtb);

	// to Local
	m_tSpecifyBeginDateTime = mktime( &begintm) - (localtb.timezone)*60;
	m_tSpecifyEndDateTime = mktime( &endtm) - (localtb.timezone)*60;

	// to UTC
	m_tSpecifyBeginDateTime = m_tSpecifyBeginDateTime - (m_tFileDateTimeZone-12)*60*60;
	m_tSpecifyEndDateTime =  m_tSpecifyEndDateTime - (m_tFileDateTimeZone-12)*60*60;

	
}

void CMediaConverterDlg::OnBnClickedButton1()
{
	CFileDialog f( TRUE, NULL, NULL, OFN_EXPLORER, szFilter, this, 0 ) ;
   
	if( f.DoModal() == IDOK  )
    {   
	  
		m_FileName  = f.GetFileName();
		m_PathName  = f.GetPathName();
		m_FileExt   = f.GetFileExt();
		{
			CString strFolderName = m_PathName.GetString();
			int nIdx = strFolderName.ReverseFind( '\\');
			strFolderName.SetAt( nIdx, '\0');
			m_WriteFolderName = strFolderName.GetString();
			m_edtWriteDstPath.SetWindowText( m_WriteFolderName.GetBuffer());
		}

		if( m_FileExt == "raw" )
		{			         
			m_Rcmp4tojpg.SetCheck(0);
            m_Rcmp4tojpg.EnableWindow(FALSE);
			//
			//GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
			m_SRC2.EnableWindow(TRUE);
		
            //
			m_chkRADIO10.EnableWindow(TRUE);
            m_chkRADIO11.EnableWindow(TRUE);
            
            m_chkRADIO12.EnableWindow(TRUE); 
            //
            m_edtFileBeginDateTime.EnableWindow(TRUE);
			m_edtFileEndDateTime.EnableWindow(TRUE);
			m_edtUSh.EnableWindow(TRUE);
			m_edtUSm.EnableWindow(TRUE);
			m_edtUSs.EnableWindow(TRUE);
			m_edtUSendh.EnableWindow(TRUE);
			m_edtUSendm.EnableWindow(TRUE);
			m_edtUSends.EnableWindow(TRUE);
			//
			if( m_PathName.GetLength() )
			{
				CMpeg4Reader * pFileReader = NULL;
				pFileReader = new CMpeg4Reader();
				if( pFileReader )
				{
					MEDIA_CONNECTION_CONFIG mcc;
					memset(&mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
					strcpy(mcc.PlayFileName, m_PathName.GetBuffer());
					mcc.ContactType = CONTACT_TYPE_PLAYBACK;
					pFileReader->SetMediaConfig(&mcc);

					if( !pFileReader->OpenFile() )
					{
						char msg[256];
						sprintf( msg, "filename:%s not found or not raw file!!",
							m_PathName.GetBuffer() );
						AfxMessageBox( msg);
						return ;
					}
					else
					{
						DisplayFileDateTime( pFileReader );
					}
					delete pFileReader;
					pFileReader = NULL;
				}
			}
		}
		else if( m_FileExt == "mp4" )
		{	
			GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
			//
			m_chkRADIO10.SetCheck(0);
			m_chkRADIO11.SetCheck(0);
			m_chkRADIO12.SetCheck(0);
			m_SRC2.EnableWindow(FALSE);
			//
            m_chkRADIO10.EnableWindow(FALSE);
            m_chkRADIO11.EnableWindow(FALSE);
            m_chkRADIO12.EnableWindow(FALSE);
			m_Rcmp4tojpg.EnableWindow(TRUE);
			
			//
			m_edtFileBeginDateTime.EnableWindow(FALSE);
			m_edtFileEndDateTime.EnableWindow(FALSE);
			m_edtUSh.EnableWindow(FALSE);
			m_edtUSm.EnableWindow(FALSE);
			m_edtUSs.EnableWindow(FALSE);
			m_edtUSendh.EnableWindow(FALSE);
			m_edtUSendm.EnableWindow(FALSE);
			m_edtUSends.EnableWindow(FALSE);
			//
			
		}
		else
		{
			;
		}
		UpdateData(FALSE);
	}
	

}

BOOL CMediaConverterDlg::CheckSpecifyDateTime()
{
	if( m_tSpecifyBeginDateTime < m_tFileBeginDateTime )
	{
		AfxMessageBox( "begin time lesser the file begin time !");
		return FALSE;
	}

	if( m_tSpecifyEndDateTime > m_tFileEndDateTime )
	{
		AfxMessageBox( "end time greater the file end time !");
		return FALSE;
	}

	if( m_tSpecifyBeginDateTime > m_tSpecifyEndDateTime )
	{
		AfxMessageBox( "begin time greater the end time !");
		return FALSE;
	}
	return TRUE;
}


void CMediaConverterDlg::OnBnClickedButton3()
{
	UpdateData() ;
	
	//Converter File count(IDC_5) 
	m_nConvertedFile = 0 ; 

	//耝filename save to csTmp
	CString csTmp = m_FileName.GetString();   
	csTmp.MakeLower() ; 
    // make filename _ ぇ玡name and _ 砞0
	csTmp = csTmp.Left( csTmp.Find("_",0) ) ; 

	  //Convert file ,if file is NULL, Error
	if( m_SRC1.GetCheck() && m_FileName.Compare("")==0 )         
	{
		MessageBox( "Please select a file!", "Error", MB_OK ) ;
		return ;
	}
	//Convert folder ,if folder is NULL,Error
	else if( m_SRC2.GetCheck() && m_FolderName.Compare("")==0 )    
	{
		MessageBox( "Please select a folder!", "Error", MB_OK ) ;
		return ;
	}
	m_Message.ResetContent();
	m_Message.InsertString( 0, "Converting ... " );

	if( m_OSD1_1.GetCheck() )
	{   
          csTmp.Replace("Camera-","") ; 
		  //
          csTmp.Replace("channel","") ; 
          //簿埃channel 痙channel Use Channel ID (eg: Channel3 displays as 3) (IDC_RADI05)
		  //!strncmp( csTmp, "channel", strlen("channel")) )
		  if( csTmp.GetLength() )
		{
                            
			if( m_nFileExtType == FILE_EXT_AVI)            
			{   
				m_cAVIWriter.SetOSDText( csTmp);
			}
			else if(m_nFileExtType == FILE_EXT_JPG)
			{   
				m_cJPGWriter.SetJPGOSDText( csTmp);
			}
            else if(m_nFileExtType == FILE_EXT_MP4)
			{   
				m_cMP4Writer.SetMP4OSDText( csTmp);
			}
		}		
		else                                                
		{
			;
			//m_cAVIWriter.SetOSDText( csTmp);
			//m_cJPGWriter.SetJPGOSDText( csTmp);
		}                               
	}
	else if( m_OSD1_2_1.GetCheck() )
	{   

		m_cAVIWriter.SetOSDText( m_csOSDText) ;            
		m_cJPGWriter.SetJPGOSDText( m_csOSDText);  
		m_cMP4Writer.SetMP4OSDText( m_csOSDText);
	}

	//m_cAVIWriter.m_bOSDTIme  = m_bOSDTime ;
 	

	
	HandleUserSpecifyDateTime();
	if(!m_bConvertFolder)
	{
		if( !CheckSpecifyDateTime() )
		{
			return;
		}
	}  

	m_hThread = CreateThread( NULL, 0, ThreadEnter, this, 0, (LPDWORD)&m_dwThreadID);

	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE); 
	


	if(NULL == m_hThread)
	{   
		 /* m_hThread == NULL ConvertFile false ,ConvertFolder false , Convertend */
		AfxMessageBox("Fail to start convert thread");
		m_bConvertFile = false;                              
		m_bConvertFolder = false;                         
		m_bConvertEnd = true;                              
	}

}

void CMediaConverterDlg::OnPrintInfo(DWORD arg1, DWORD arg2)
{
	
	
	char szMessage[128] = {0};

    
	switch( arg1 )
	{
       
	case 0 :
		{
			//////////////////////////////////////////////////////
			sprintf(szMessage, "Convert Source File : %s", m_FileName);
			m_Message.AddString("") ;
			m_Message.AddString(szMessage);
			memset(szMessage, 0x00, 128);
		    ///////////////////////////////////////////////////////
		   	sprintf(szMessage, "I Frame Count : %d",m_nIFrame);// m_nIFrame);
			m_Message.AddString(szMessage);
			memset(szMessage, 0x00, 128);
			sprintf(szMessage, "P Frame Count : %d", m_nPFrame);//m_nPFrame);
			m_Message.AddString(szMessage);
			memset(szMessage, 0x00, 128);
			sprintf(szMessage, "Audio Frame Count : %d",m_nAFrame );// dwAFrame;
			m_Message.AddString(szMessage);
		}
		break ;
	case 1 :
		
		m_nPFrame = arg2 ;
		break ;
	case 2 :
	
		m_nIFrame = arg2 ;
		break ;
	case 3 :
		m_nAFrame = arg2 ;
		break ;
	case 4 :
		m_nConvertedFile = arg2 ;
		break;
	case 5:
		{
   			char szMessage[256] = {0};
			sprintf(szMessage, "Convert Result : %s", m_OutFileName.GetBuffer() );  
			m_Message.AddString(szMessage);
		}
		break ;
	}
	
	UpdateData(FALSE) ;
}

void CMediaConverterDlg::OnPrintMP4Info()
{   	  
   	char szMessage[128] = {0};
	sprintf(szMessage, "Convert Result : %s", m_FileName);  
	m_Message.AddString(szMessage);
   
}

//m_tSpecifyBeginDateTime, m_tSpecifyEndDateTime
bool CMediaConverterDlg::ConvertRawToSingleRawFile( void )
{  
	CString sRAW;
	CString sTemp;
	sTemp = m_FileName;
	//パ.计程癬﹍┕玡计
	int iIndex = sTemp.ReverseFind('.');     
	sTemp.SetAt(iIndex, '\0');             
	sRAW = sTemp;
	//sRAW.Insert(iIndex+1, "raw.bak");

	//char szToken[256];
	//m_edtUSh.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSm.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSs.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSendh.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSendm.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSends.GetWindowText( szToken, sizeof(szToken));
    
	{
		char szFileName[512];

		DWORD dwUTCBeginTime = m_tFileBeginDateTime + (m_tFileDateTimeZone-12)*60*60; 
		DWORD dwUTCEndTime = m_tFileEndDateTime + (m_tFileDateTimeZone-12)*60*60; 

		struct tm begintm = *(gmtime( (time_t *)&dwUTCBeginTime));
		struct tm endtime = *(gmtime( (time_t *)&dwUTCEndTime));

		sprintf( szFileName, "%s_%04d%02d%02d_%02d%02d%02d-%02d%02d%02d.raw",
			sRAW.GetBuffer(),
			begintm.tm_yday+1900, begintm.tm_mon+1, begintm.tm_mday,
			begintm.tm_hour, begintm.tm_min, begintm.tm_sec,
			endtime.tm_hour, endtime.tm_min, endtime.tm_sec );
		sRAW = szFileName;

	}

	m_OutFileName = m_WriteFolderName + "\\" + sRAW;

	CFraw *pFraw = new CFraw;
	pFraw->ConvertFile( sRAW.GetBuffer(), m_PathName.GetBuffer(), m_WriteFolderName.GetBuffer(),
		m_tSpecifyBeginDateTime, m_tSpecifyEndDateTime);

	sRAW.ReleaseBuffer() ;
    m_FileName.ReleaseBuffer() ;

	AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 0, 0);

	{
		AfxGetApp()->GetMainWnd()->PostMessage( PRINT_INFO, 5, (DWORD)0);
	}

	if( pFraw )
		delete pFraw;

	return true;
}

bool CMediaConverterDlg::ConvertRawToSingleAVIFile( const time_t tSpecifyBeginDateTime, const time_t tSpecifyEndDateTime )
{  
	CString sAVI;
	CString sTemp;
	sTemp = m_FileName;
	//パ.计程癬﹍┕玡计
	int iIndex = sTemp.ReverseFind('.');     
	sTemp.SetAt(iIndex+1, '\0');             
	sAVI = sTemp;
	sAVI.Insert(iIndex+1, "avi");

	//char szToken[256];
	//m_edtUSh.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSm.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSs.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSendh.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSendm.GetWindowText( szToken, sizeof(szToken));
	//m_edtUSends.GetWindowText( szToken, sizeof(szToken));
    
	m_OutFileName = m_WriteFolderName + "\\" + sAVI;
	m_cAVIWriter.CreateAVIFile( sAVI.GetBuffer(), m_PathName.GetBuffer(),
		m_WriteFolderName.GetBuffer(), tSpecifyBeginDateTime, tSpecifyEndDateTime);

	sAVI.ReleaseBuffer() ;
    m_FileName.ReleaseBuffer() ;

	AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 0, 0);

	{
		AfxGetApp()->GetMainWnd()->PostMessage( PRINT_INFO, 5, (DWORD)0);
	}

	return true;
}

bool CMediaConverterDlg::ConvertRawToMultiJPGFile( const time_t tSpecifyBeginDateTime, const time_t tSpecifyEndDateTime)
{
 
	CString sJPG;
	
	sJPG = m_FileName;
	int iIndex = sJPG.ReverseFind('.');
	sJPG.SetAt(iIndex, '\0');
    //sJPG.Insert(iIndex+1, "jpg");

	/*char szToken[256];
	m_edtUSh.GetWindowText( szToken, sizeof(szToken));
	m_edtUSm.GetWindowText( szToken, sizeof(szToken));
	m_edtUSs.GetWindowText( szToken, sizeof(szToken));
	m_edtUSendh.GetWindowText( szToken, sizeof(szToken));
	m_edtUSendm.GetWindowText( szToken, sizeof(szToken));
	m_edtUSends.GetWindowText( szToken, sizeof(szToken));*/
    
	m_OutFileName = m_WriteFolderName + "\\" + sJPG;
	m_cJPGWriter.ConvertMultiJPGFile( sJPG.GetBuffer(),
		m_PathName.GetBuffer(), m_WriteFolderName.GetBuffer(),
		tSpecifyBeginDateTime, tSpecifyEndDateTime);

	sJPG.ReleaseBuffer() ;
	m_FileName.ReleaseBuffer() ;

	AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 0, 0);
	{
		AfxGetApp()->GetMainWnd()->PostMessage( PRINT_INFO, 5, (DWORD)0);
	}

	return true;
}

bool CMediaConverterDlg::ConvertMP4ToMultiJPGFile()
{   
   
	CString sMP4ToJPG;
	CString sTemp;
	sTemp = m_FileName;
	int iIndex = sTemp.ReverseFind('.');
	sTemp.SetAt(iIndex, '\0');
	sMP4ToJPG = sTemp;
    sMP4ToJPG.Insert(iIndex , ".jpg");

	m_OutFileName = m_WriteFolderName + "\\" + sMP4ToJPG;
	m_cMP4Writer.ConvertMP4ToJpg( m_FileName , m_WriteFolderName.GetBuffer() );
    
	sMP4ToJPG.ReleaseBuffer() ;
	m_FileName.ReleaseBuffer() ;

	AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 0, 0);
	{
		AfxGetApp()->GetMainWnd()->PostMessage( PRINT_INFO, 5, (DWORD)0);
	}

	return true;
}



void CMediaConverterDlg::RunProcess()
{
	m_bConvertEnd = false;
	SetTimer(FILE_CLOSE_TIMER_ID, 1000, NULL);

	m_cAVIWriter.SetProcessing( true);
	m_cJPGWriter.SetProcessing( true);
    m_cMP4Writer.SetProcessing( true);
	if( m_bConvertFile )
	{
		switch( m_nFileSrcType )
		{
       
		case FILE_SRC_RAW:
			switch( m_nFileExtType )
			{
               
				case FILE_EXT_RAW:
					ConvertRawToSingleRawFile();
					break;
				case FILE_EXT_JPG:
					ConvertRawToMultiJPGFile( m_tSpecifyBeginDateTime, m_tSpecifyEndDateTime );
					break;
				case FILE_EXT_AVI:
					ConvertRawToSingleAVIFile( m_tSpecifyBeginDateTime, m_tSpecifyEndDateTime );
					break;
				default:
					break;
			}
			break;
		case FILE_SRC_MP4:
			
			switch( m_nFileExtType )
			{
				
				case FILE_EXT_JPG:
					ConvertMP4ToMultiJPGFile();
					break;
				default:
					break;
			}
			break;
		}

		AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 4, 1);
		AfxGetApp()->GetMainWnd()->PostMessage(FILE_CLOSE, 0, 0);
	}
	//-----------------------------------------------------------------
	else if( m_bConvertFolder  )
	{
		bool bFind = NULL ;
		
		CFileFind ffFile;
           	
	
	    switch( m_nFileSrcType )
		{
		case FILE_SRC_RAW:
			{
				bFind = ffFile.FindFile( m_FolderName + "\\*.raw", 0);
			
					  
				if( !bFind )
				{
					AfxGetApp()->GetMainWnd()->PostMessage(FILE_CLOSE, 0, 0);
					ffFile.Close();
					return;
				}
			
				switch( m_nFileExtType )
				{
				
				case FILE_EXT_JPG:
					m_cJPGWriter.SetConvertFolderFlag(true);
					break;
				case FILE_EXT_AVI:
					m_cAVIWriter.SetConvertFolderFlag(true);
                    break;
				default:
					break;
				}
			}
			break;
		case FILE_SRC_MP4:
			{
				bFind = ffFile.FindFile( m_FolderName + "\\*.mp4", 0);
			
							 
				if( !bFind )
				{
					AfxGetApp()->GetMainWnd()->PostMessage(FILE_CLOSE, 0, 0);
					ffFile.Close();
					return;
				}
				switch( m_nFileExtType )
				{
				case FILE_EXT_JPG:
				    m_cMP4Writer.SetConvertFolderFlag(true);
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}

		do
		{
		             
			bFind = ffFile.FindNextFile();
           	//CString LocalFileName = ffFile.GetFilePath().GetString();
			m_FileName = ffFile.GetFileName().GetString();
			m_PathName = ffFile.GetFilePath().GetString();
			
			if ( ffFile.IsDots() || ffFile.IsDirectory() )
			continue;	   
			
				switch( m_nFileSrcType )
				{
				case FILE_SRC_RAW:
					{
						//bFind = ffFile.FindFile( m_FolderName + "\\*.raw", 0);
						
					
						switch( m_nFileExtType )
						{
							bFind = ffFile.FindFile( m_FolderName + "\\*.raw", 0);
						case FILE_EXT_JPG:
							ConvertRawToMultiJPGFile();
							break;
						default:
							break;
						}
						switch( m_nFileExtType )
						{
							bFind = ffFile.FindFile( m_FolderName + "\\*.raw", 0);
						case FILE_EXT_RAW:
							ConvertRawToSingleRawFile();
							break;
						case FILE_EXT_JPG:
							ConvertRawToMultiJPGFile();
							break;
						case FILE_EXT_AVI:
							ConvertRawToSingleAVIFile();
							break;
						default:
							break;
						}
					}
					break;
				case FILE_SRC_MP4:
					{
						//bFind = ffFile.FindFile( m_FolderName + "\\*.mp4", 0);
						
						
						switch( m_nFileExtType )
						{
						bFind = ffFile.FindFile( m_FolderName + "\\*.mp4", 0);
						case FILE_EXT_JPG:
							ConvertMP4ToMultiJPGFile();
							break;
						default:
							break;
						}
					}
					break;
				default:
					break;
				}
				
				AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 3, ++m_nConvertedFile);
				
			}while( !m_bConvertEnd && bFind && m_bConvertFolder );
			ffFile.Close();
			AfxGetApp()->GetMainWnd()->PostMessage(FILE_CLOSE, 0, 0);
         	
	  }   
}

void CMediaConverterDlg::OnFileClose()
{
	m_bConvertEnd = true;
}

void CMediaConverterDlg::OnFileCloseTimer()
{
	if(m_bConvertEnd)
	{
		m_bConvertEnd = false;
		KillTimer(FILE_CLOSE_TIMER_ID);
		OnBnClickedButton4();
	}
}
void CMediaConverterDlg::OnBnClickedButton4()
{
	m_bConvertEnd = true; 
	m_cAVIWriter.SetProcessing(false);
	m_cJPGWriter.SetProcessing(false);
	m_cMP4Writer.SetProcessing(false);
	KillTimer(FILE_CLOSE_TIMER_ID);
	if(WaitForSingleObject(m_hThread, INFINITE))
	{
	}
	m_cAVIWriter.SetConvertFolderFlag(false);
    m_cJPGWriter.SetConvertFolderFlag(false);
	m_cMP4Writer.SetConvertFolderFlag(false);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	if(m_hThread)
	{ 
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}





void CMediaConverterDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	CFolderDialog dlg(  _T( "Browse Folder" ), m_FolderName, this );

    if( dlg.DoModal() == IDOK  )
    {       
        m_WriteFolderName = dlg.GetFolderPath() ;
	    m_edtWriteDstPath.SetWindowText( m_WriteFolderName.GetBuffer());
		UpdateData(FALSE) ;
	}
	
}

void CMediaConverterDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	m_bConvertEnd = true; 
	m_cAVIWriter.SetProcessing(false);
	m_cJPGWriter.SetProcessing(false);
	m_cMP4Writer.SetProcessing(false);
	KillTimer(FILE_CLOSE_TIMER_ID);
	if(WaitForSingleObject(m_hThread, INFINITE))
	{
	}
	m_cAVIWriter.SetConvertFolderFlag(false);
    m_cJPGWriter.SetConvertFolderFlag(false);
	m_cMP4Writer.SetConvertFolderFlag(false);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	if(m_hThread)
	{ 
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	OnCancel();

}


