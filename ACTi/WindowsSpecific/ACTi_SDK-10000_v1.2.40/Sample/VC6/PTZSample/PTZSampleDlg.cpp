// PTZSampleDlg.cpp : implementation file
//
#include "stdafx.h"
#include "PTZSample.h"
#include "PTZSampleDlg.h"
#include ".\ptzsampledlg.h"
#include "afxinet.h"
#include ".\SDK10000.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


bool SendURL(char *pcaCommand)
{
	
	int Len = 1024;
	char Buffer[1024];
	char *lpRet = Buffer;
	int *pBuffer = &Len;

	CInternetSession *pIS = NULL;
	CHttpFile *pIF = NULL;
	CString oStrR;

	oStrR.Format("%s", pcaCommand);

	// initialize 
	pIS = new CInternetSession("PTZ");

	// use
	if( !pIS ) pIS = new CInternetSession("PTZ");

	pIF = (CHttpFile *)pIS->OpenURL(oStrR, 1, INTERNET_FLAG_TRANSFER_ASCII|INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE );

	if( !pIF ) return false;
	else
	{
		DWORD dwRet = 0;
		pIF->QueryInfoStatusCode(dwRet);
		if (dwRet == HTTP_STATUS_OK)
		{
			if(( lpRet )&&(pBuffer)) 
				*pBuffer = pIF->Read(lpRet ,(UINT)*pBuffer);
			
		}
		else
		{
			if(( lpRet )&&(pBuffer)) *pBuffer = 0;
		}
		pIF->Close();
		delete pIF;
		pIF = NULL;
		delete pIS;
		pIS = NULL;
		return true; 
	}
}

// CPTZSampleDlg dialog



CPTZSampleDlg::CPTZSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPTZSampleDlg::IDD, pParent)
    , m_bConnected(false)
    , m_iSendMode(0)
    , m_csLastCommand(_T(""))
    , m_bTest(false)
    , m_ctrlCommandCount(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPTZSampleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT4, m_ctrlAddrID);
    DDX_Control(pDX, IDC_EDIT9, m_ctrlProtocol);
    DDX_Control(pDX, IDC_EDIT7, m_ctrlPTZCmdParam1);
    DDX_Control(pDX, IDC_EDIT8, m_ctrlPTZCmdParam2);
    DDX_Control(pDX, IDC_EDIT5, m_ctrlPTZURLCmd);
    DDX_Control(pDX, IDC_COMBO1, m_ctrlPTZCmdList);
    DDX_Control(pDX, IDC_COMBO4Parity, m_ctrlParity);
    DDX_Control(pDX, IDC_COMBO4Baudrate, m_ctrlBaudrate);
    DDX_Control(pDX, IDC_STATIC4VW, m_ctrlVW);
    DDX_Control(pDX, IDC_COMBO4Vendor, m_ctrlVendorList);
    DDX_Control(pDX, IDC_COMBO4Protocol, m_ctrlProtocolList);
    DDX_Control(pDX, IDC_COMBO8, m_ctrlSendMode);
	DDX_Text(pDX, IDC_EDIT10, m_fPanDegree);
	DDX_Text(pDX, IDC_EDIT11, m_fTilteDegree);
	DDX_Text(pDX, IDC_EDIT12, m_fZoomDegree);
	DDX_Control(pDX, IDC_COMBO2, m_ctrlIsCCW);

}

BEGIN_MESSAGE_MAP(CPTZSampleDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
    ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton4selefile)
    ON_WM_LBUTTONDOWN()
    ON_BN_CLICKED(IDC_BUTTON4TILTUP, OnBnClickedButton4tiltup)
    ON_BN_CLICKED(IDC_BUTTON4LEFTDOWN, OnBnClickedButton4leftdown)
    ON_BN_CLICKED(IDC_BUTTON4PANLEFT, OnBnClickedButton4panleft)
    ON_BN_CLICKED(IDC_BUTTON4LEFTUP, OnBnClickedButton4leftup)
    ON_BN_CLICKED(IDC_BUTTON4STOP, OnBnClickedButton4stop)
    ON_BN_CLICKED(IDC_BUTTON4TILTDOWN, OnBnClickedButton4tiltdown)
    ON_BN_CLICKED(IDC_BUTTON4RIGHTUP, OnBnClickedButton4rightup)
    ON_BN_CLICKED(IDC_BUTTON4PANRIGHT, OnBnClickedButton4panright)
    ON_BN_CLICKED(IDC_BUTTON4RIGHTDOWN, OnBnClickedButton4rightdown)
    ON_BN_CLICKED(IDC_BUTTON4Connect, OnBnClickedButton4connect)
    ON_BN_CLICKED(IDC_BUTTON4Disconnect, OnBnClickedButton4disconnect)
    ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
    ON_BN_CLICKED(IDC_BUTTON4SeleFIle, OnBnClickedButton4selefile)
    ON_BN_CLICKED(IDC_RADIO4File, OnBnClickedRadio4file)
    ON_BN_CLICKED(IDC_RADIO4Buildin, OnBnClickedRadio4buildin)
    ON_EN_CHANGE(IDC_EDIT4AddrID, OnEnChangeEdit4addrid)
    ON_CBN_SELCHANGE(IDC_COMBO4Vendor, OnCbnSelchangeCombo4vendor)
    ON_BN_CLICKED(IDC_BUTTON4ZOOMIN, OnBnClickedButton4zoomin)
    ON_BN_CLICKED(IDC_BUTTON4ZOOMSTOP, OnBnClickedButton4zoomstop)
    ON_BN_CLICKED(IDC_BUTTON4ZOOMOUT, OnBnClickedButton4zoomout)
    ON_BN_CLICKED(IDC_BUTTON4FOCUSIN, OnBnClickedButton4focusin)
    ON_BN_CLICKED(IDC_BUTTON4FOCUSSTOP, OnBnClickedButton4focusstop)
    ON_BN_CLICKED(IDC_BUTTON4FOCUSOUT, OnBnClickedButton4focusout)
    ON_BN_CLICKED(IDC_BUTTON15, OnBnClickedButton15)
    ON_BN_CLICKED(IDC_BUTTON16, OnBnClickedButton16)
    ON_BN_CLICKED(IDC_BUTTON4STOP2, OnBnClickedButton4stop2)
    ON_BN_CLICKED(IDC_BUTTON4UP2, OnBnClickedButton4up2)
    ON_BN_CLICKED(IDC_BUTTON4LEFT2, OnBnClickedButton4left2)
    ON_BN_CLICKED(IDC_BUTTON4RIGHT2, OnBnClickedButton4right2)
    ON_BN_CLICKED(IDC_BUTTON4DOWN2, OnBnClickedButton4down2)
    ON_BN_CLICKED(IDC_BUTTON4OSDON, OnBnClickedButton4osdon)
    ON_BN_CLICKED(IDC_BUTTON4OSDOFF, OnBnClickedButton4osdoff)
    ON_BN_CLICKED(IDC_BUTTON4OSDLEAVE, OnBnClickedButton4osdleave)
    ON_BN_CLICKED(IDC_BUTTON4BLCON, OnBnClickedButton4blcon)
    ON_BN_CLICKED(IDC_BUTTON4BLCON4BLCOFF, OnBnClickedButton4blcon4blcoff)
    ON_BN_CLICKED(IDC_BUTTON4BLCON4DandNOn, OnBnClickedButton4blcon4dandnon)
    ON_BN_CLICKED(IDC_BUTTON4BLCON4DandNOff, OnBnClickedButton4blcon4dandnoff)
    ON_BN_CLICKED(IDC_BUTTON4BLCON4DayNightAuto, OnBnClickedButton4blcon4daynightauto)
    ON_BN_CLICKED(IDC_BUTTON4BLCON4DayNightAutoOff, OnBnClickedButton4blcon4daynightautooff)
    ON_BN_CLICKED(IDC_BUTTON14, OnBnClickedButton14)
    ON_BN_CLICKED(IDC_BUTTON17, OnBnClickedButton17)
    ON_BN_CLICKED(IDC_BUTTON18, OnBnClickedButton18)
    ON_BN_CLICKED(IDC_BUTTON19, OnBnClickedButton19)
    ON_BN_CLICKED(IDC_BUTTON20, OnBnClickedButton20)
    ON_BN_CLICKED(IDC_BUTTON21, OnBnClickedButton21)
    ON_BN_CLICKED(IDC_BUTTON22, OnBnClickedButton22)
    ON_BN_CLICKED(IDC_BUTTON23, OnBnClickedButton23)
    ON_BN_CLICKED(IDC_BUTTON24, OnBnClickedButton24)
    ON_BN_CLICKED(IDC_BUTTON25, OnBnClickedButton25)
    ON_BN_CLICKED(IDC_BUTTON26, OnBnClickedButton26)
    ON_BN_CLICKED(IDC_BUTTON27, OnBnClickedButton27)
    ON_BN_CLICKED(IDC_BUTTON28, OnBnClickedButton28)
    ON_BN_CLICKED(IDC_BUTTON29, OnBnClickedButton29)
    ON_BN_CLICKED(IDC_BUTTON30, OnBnClickedButton30)
    ON_BN_CLICKED(IDC_BUTTON31, OnBnClickedButton31)
    ON_BN_CLICKED(IDC_BUTTON32, OnBnClickedButton32)
    ON_BN_CLICKED(IDC_BUTTON33, OnBnClickedButton33)
    ON_BN_CLICKED(IDC_BUTTON34, OnBnClickedButton34)
    ON_BN_CLICKED(IDC_BUTTON35, OnBnClickedButton35)
    ON_BN_CLICKED(IDC_BUTTON36, OnBnClickedButton36)
    ON_BN_CLICKED(IDC_BUTTON37, OnBnClickedButton37)
    ON_BN_CLICKED(IDC_BUTTON38, OnBnClickedButton38)
    ON_BN_CLICKED(IDC_BUTTON39, OnBnClickedButton39)
    ON_BN_CLICKED(IDC_BUTTON40, OnBnClickedButton40)
    ON_BN_CLICKED(IDC_BUTTON41, OnBnClickedButton41)
    ON_BN_CLICKED(IDC_BUTTON42, OnBnClickedButton42)
    ON_BN_CLICKED(IDC_BUTTON43, OnBnClickedButton43)
    ON_BN_CLICKED(IDC_BUTTON44, OnBnClickedButton44)
    ON_BN_CLICKED(IDC_BUTTON45, OnBnClickedButton45)
    ON_BN_CLICKED(IDC_BUTTON46, OnBnClickedButton46)
    ON_BN_CLICKED(IDC_BUTTON47, OnBnClickedButton47)
    ON_BN_CLICKED(IDC_BUTTON48, OnBnClickedButton48)
    ON_BN_CLICKED(IDC_BUTTON49, OnBnClickedButton49)
    ON_BN_CLICKED(IDC_BUTTON50, OnBnClickedButton50)
    ON_BN_CLICKED(IDC_BUTTON51, OnBnClickedButton51)
    ON_BN_CLICKED(IDC_BUTTON52, OnBnClickedButton52)
    ON_BN_CLICKED(IDC_BUTTON53, OnBnClickedButton53)
    ON_BN_CLICKED(IDC_BUTTON54, OnBnClickedButton54)
    ON_BN_CLICKED(IDC_BUTTON55, OnBnClickedButton55)
    ON_BN_CLICKED(IDC_BUTTON56, OnBnClickedButton56)
    ON_BN_CLICKED(IDC_BUTTON57, OnBnClickedButton57)
    ON_BN_CLICKED(IDC_BUTTON58, OnBnClickedButton58)
    ON_BN_CLICKED(IDC_BUTTON59, OnBnClickedButton59)
    ON_BN_CLICKED(IDC_BUTTON60, OnBnClickedButton60)
    ON_BN_CLICKED(IDC_BUTTON61, OnBnClickedButton61)
    ON_BN_CLICKED(IDC_BUTTON62, OnBnClickedButton62)
    ON_BN_CLICKED(IDC_BUTTON63, OnBnClickedButton63)
    ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
    ON_BN_CLICKED(IDOK4, OnBnClickedOk4)
	ON_EN_CHANGE(IDC_EDIT4URLCmdOutput, OnEnChangeEdit4urlcmdoutput)
	ON_CBN_SELCHANGE(IDC_COMBO4Command, OnCbnSelchangeCombo4command)
	ON_EN_CHANGE(IDC_EDIT4Param1, OnEnChangeEdit4param1)
	ON_EN_CHANGE(IDC_EDIT4Param2, OnEnChangeEdit4param2)
	ON_EN_CHANGE(IDC_EDIT10, OnEnChangeEdit10)
	ON_EN_CHANGE(IDC_EDIT11, OnEnChangeEdit11)
	ON_EN_CHANGE(IDC_EDIT12, OnEnChangeEdit12)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
	ON_CBN_SELCHANGE(IDC_COMBO4Baudrate, OnCbnSelchangeCombo4baudrate)
	ON_CBN_SELCHANGE(IDC_COMBO4Parity, OnCbnSelchangeCombo4parity)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnCbnSelchangeCombo2)
	END_MESSAGE_MAP()


// CPTZSampleDlg message handlers

BOOL CPTZSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    // Init member variable
    //m_hPTZ = NULL;
    m_hNet = NULL;
    m_csFileName = "";

    m_ctrlParity.SetCurSel(2);
    m_ctrlBaudrate.SetCurSel(3);

    GetDlgItem(IDC_EDIT4ProtocolFileName)->EnableWindow(FALSE);
    GetDlgItem(IDC_COMBO4Vendor)->EnableWindow(FALSE);
    GetDlgItem(IDC_COMBO4Protocol)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON4SeleFIle)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);

    GetDlgItem(IDC_RADIO4File)->EnableWindow(FALSE);
    GetDlgItem(IDC_RADIO4Buildin)->EnableWindow(FALSE);
    SetDlgItemText(IDC_EDIT4ServerIP , "172.16.1.85");
    SetDlgItemText(IDC_EDIT4ID , "Admin");
    SetDlgItemText(IDC_EDIT4Password , "123456");

    m_ctrlSendMode.SetCurSel(0);

    m_bConnected = false;
    BOOL bRet = false;
    // bRet = m_ctrlVW.SetWindowPos(NULL, 0, 0, 720, 480, SWP_NOOWNERZORDER|SWP_NOMOVE);

    /*
    m_ctrlVW.SetWindowPos(NULL, 0, 0,
        720, 480, SWP_NOMOVE);
        */

    DWORD dwLastError;
    dwLastError = GetLastError();
    
	SetDlgItemText(IDC_EDIT10 , "0.0");
	SetDlgItemText(IDC_EDIT11 , "0.0");
	SetDlgItemText(IDC_EDIT12 , "0.0");
	m_fPanDegree = 0.0f;
	m_fTilteDegree = 0.0f;
	m_fZoomDegree = 0.0f;

	m_ctrlIsCCW.SetCurSel(0);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPTZSampleDlg::OnPaint() 
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
HCURSOR CPTZSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Exit button : exit
void CPTZSampleDlg::OnBnClickedCancel()
{
	
    if(m_hNet)
    {
        //Obsolete//PTZCloseInterface(m_hPTZ);
		//KPTZUnloadProtocol(m_hNet);
		KStop( m_hNet );
		KStopStreaming( m_hNet );
		KDisconnect( m_hNet );
		KCloseInterface( m_hNet );
        m_hNet = NULL;
    }
    
    OnCancel();
}

char caURLCommand[1024];
char* GetURLCommand(char *pcaServerIP, char *pcaAccount, char *pcaPassword, const char *pcaCommand)
{
	sprintf(caURLCommand, "http://");
	strcat(caURLCommand, pcaServerIP);
	strcat(caURLCommand, "/cgi-bin/mpeg4?USER=");

	strcat(caURLCommand, pcaAccount);
	strcat(caURLCommand, "&PWD=");
	strcat(caURLCommand, pcaPassword);
	strcat(caURLCommand, "&SERIAL_HEX=");
	strcat(caURLCommand, pcaCommand);

	return caURLCommand;
}


// Get button : Get URL command of PTZ 
void CPTZSampleDlg::OnBnClickedOk2()
{

	CString csServerIP;
	CString csID;
	CString csPassword;
	// GetDlgItemStri
	GetDlgItemText(IDC_EDIT4ServerIP, csServerIP);
	GetDlgItemText(IDC_EDIT4ID, csID);
	GetDlgItemText(IDC_EDIT4Password, csPassword);

    char caURLCmd[2048];
	char temp[128];
    unsigned char pBuffer[1024];
    int iPTZParam1 = 0;
    int iPTZParam2 = 0;

    DWORD dwStringLen = 1024;
    CString csPTZCmd;


	m_ctrlPTZCmdParam1.GetWindowText(caURLCmd, sizeof(caURLCmd));
	iPTZParam1 = atoi(caURLCmd);
	m_ctrlPTZCmdParam2.GetWindowText(caURLCmd, sizeof(caURLCmd));
	iPTZParam2 = atoi(caURLCmd);

	m_ctrlPTZCmdList.GetWindowText(csPTZCmd);

	if(KPTZGetCommand(m_hNet, csPTZCmd.GetBuffer(), m_mPTZ.dwAddressID, iPTZParam1, iPTZParam2, pBuffer, dwStringLen))
	{
		std::string strHex2Char;
		strHex2Char.clear();
		// hex to string
		for(int i = 0; i<dwStringLen; ++i)
		{
			sprintf(temp,"%02X",pBuffer[i]);
			
			strHex2Char += temp; 

		}
		m_ctrlPTZURLCmd.SetWindowText(GetURLCommand(csServerIP.GetBuffer(), csID.GetBuffer(), csPassword.GetBuffer(), strHex2Char.c_str()));
	}

	/*
    if(m_hPTZ)
    {
        m_ctrlPTZCmdParam1.GetWindowText(caURLCmd, sizeof(caURLCmd));
        iPTZParam1 = atoi(caURLCmd);
        m_ctrlPTZCmdParam2.GetWindowText(caURLCmd, sizeof(caURLCmd));
        iPTZParam2 = atoi(caURLCmd);

        // int iCurSel = m_ctrlPTZCmdList.GetCurSel();
        m_ctrlPTZCmdList.GetWindowText(csPTZCmd);
        pBuffer =  PTZGetStringURL(m_hPTZ, csPTZCmd.GetBuffer(), iPTZParam1, iPTZParam2, &dwStringLen);
        
        csPTZCmd.ReleaseBuffer();

        if(pBuffer!=NULL)
		{
           // m_ctrlPTZURLCmd.SetWindowText(GetURLCommand(m_sInfo.caServerIP, m_sInfo.caAccount, m_sInfo.caPassword, pBuffer));
		}
    }
	*/
    
}

// Submit button : Submit thhe URL command of PTZ
void CPTZSampleDlg::OnBnClickedOk()
{
    char caTemp[2048];
    m_ctrlPTZURLCmd.GetWindowText(caTemp, sizeof(caTemp));
    SendURL(caTemp);
    // OnOK();
}
void CPTZSampleDlg::OnEnChangeEdit4addrid()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    GetDlgItem(IDC_RADIO4File)->EnableWindow();
    GetDlgItem(IDC_RADIO4Buildin)->EnableWindow();
}



///////////////////////////////////////////////////////////////////////////////
// Connect
void CPTZSampleDlg::OnBnClickedButton4connect()
{
    // char ServerIP[64];
    // char Account[16];
    // char Password[16];
    CString csServerIP;
    CString csID;
    CString csPassword;

    BYTE c81 = 0;
    BYTE cBaudRate = 0;
    if(!m_bConnected)
    {
        // Set the Baudrate
        // CString i = m_RS232;

        // IDC_COMBO4Parity

        // this->getdlg
        int i = m_ctrlParity.GetCurSel();

        switch(i)
        {
        case 0: //(081)
            c81 = RS232_SET_O81;
            break;
        case 1: //E81
            c81 = RS232_SET_E81;
            break;
        case 2: //N81
            c81 = RS232_SET_N81;
            break;
        case -1:
            MessageBox("Please input the RS232 setting N81, 081 or E81!", "RS232");
            return;
            break;
        }

        i = m_ctrlBaudrate.GetCurSel();

        switch(i)
        {
        case 0: // BAUD_RATE_1200BPS
            cBaudRate = BAUD_RATE_1200BPS;
            break;
        case 1: // BAUD_RATE_2400BPS
            cBaudRate = BAUD_RATE_2400BPS;
            break;
        case 2: // BAUD_RATE_4800BPS
            cBaudRate = BAUD_RATE_4800BPS;
            break;
        case 3: // BAUD_RATE_9600BPS
            cBaudRate = BAUD_RATE_9600BPS;
            break;
        case 4: // BAUD_RATE_19200BPS
            cBaudRate = BAUD_RATE_19200BPS;
            break;
        case 5: // BAUD_RATE_38400BPS
            cBaudRate = BAUD_RATE_38400BPS;
            break;		
        case 6: // BAUD_RATE_57600BPS
            cBaudRate = BAUD_RATE_57600BPS;
            break;
        case 7: // BAUD_RATE_115200BPS
            cBaudRate = BAUD_RATE_115200BPS;
            break;
        case 8: // BAUD_RATE_230400BPS
            cBaudRate = BAUD_RATE_230400BPS;
            break;

        case -1:
            MessageBox("Please input the Baudrate setting 1200, 2400 ... or 230400!", "RS232");
            return;
            break;
        }

        // GetDlgItemStri
        GetDlgItemText(IDC_EDIT4ServerIP, csServerIP);
        GetDlgItemText(IDC_EDIT4ID, csID);
        GetDlgItemText(IDC_EDIT4Password, csPassword);
        
        csServerIP.ReleaseBuffer();
        // OpenInterface
        if ((m_hNet = KOpenInterface()) == NULL) 
        {
            // DWORD dwRet = netGetLastError(m_hNet);
            MessageBox("Cann't OpenInteface !!", "Connect2Server");
            return;
        }


        MEDIA_RENDER_INFO RenderInfo;
        RECT VWRect;
        m_ctrlVW.GetWindowRect(&VWRect);

        // memcpy(&RenderInfo.rect, &VWRect, sizeof(RECT));  // set position
        RenderInfo.rect.left = 0;
        RenderInfo.rect.top = 0;
        RenderInfo.rect.right = VWRect.right - VWRect.left;
        RenderInfo.rect.bottom = VWRect.bottom - VWRect.top;

        RenderInfo.hWnd = m_ctrlVW.GetSafeHwnd();           // Set hwnd
        RenderInfo.DrawerInterface = DGDI;              // Render use the GDI

        // Set Render
        KSetRenderInfo( m_hNet, &RenderInfo );

        MEDIA_CONNECTION_CONFIG mcc = {0};
        mcc.RegisterPort = 6000;				// Register port setting
        mcc.ControlPort = 6001;					// Control port setting
        mcc.StreamingPort = 6002;				// Stream(Video) port setting
        mcc.MultiCastPort = 5000;				// Multicast port setting
        mcc.HTTPPort = 80;						// HTTP port setting;
        mcc.SearchPortC2S = 6005;
        mcc.SearchPortS2C = 6006;
        mcc.RTSPPort = 7070;

        strcpy(mcc.UserID, csID.GetBuffer());		    // User login name
        csID.ReleaseBuffer();
        strcpy(mcc.Password, csPassword.GetBuffer());		    // User password
        csPassword.ReleaseBuffer();
        mcc.ContactType = CONTACT_TYPE_UNICAST_PREVIEW;	// Connect type, Unicast with control
		mcc.ConnectTimeOut = 3;
        mcc.ChannelNumber = 0;
        // mcc.ContactType = CONTACT_TYPE_CONTROL_ONLY;	// Connect type, Unicast with control

        strcpy(mcc.UniCastIP, csServerIP);

        int iTCPType = KGetTCPTypeByHTTP( m_hNet, mcc.UniCastIP, 80, "Admin", "123456" );

        KSetTCPType( m_hNet, iTCPType );


        KEnableDecoder( m_hNet, true );
        KEnableRender( m_hNet, true );


        if(KSetMediaConfig(m_hNet, &mcc))
        {
            if(KConnect(m_hNet))
            {

                if(KStartStreaming(m_hNet))
                {
                    // KSetControlDataCallback( m_hNet, (DWORD)this, ControlDataCallback );
                    // KSetRS232DataCallback( m_hNet, (DWORD)this, RS232DataCallback );

                    KPlay(m_hNet);
                    KSendRS232Setting(m_hNet, c81, cBaudRate, 0);
                    // PTZSetServerHandle(m_hPTZ, m_hNet);
                    m_bConnected = TRUE;


					// Set PTZ Protocol enabled
					bool bResult = KEnablePTZProtocol(m_hNet, true);

                    return;
                }
            }

        }
        MessageBox("[n/vPTZTool] Didn't connect to any video server!\n");


    }

    
}
void CPTZSampleDlg::OnBnClickedButton4disconnect()
{

	//Set PTZ Protocol disabled
	KEnablePTZProtocol(m_hNet, false);

    if(m_bConnected)
    {
        KStop(m_hNet);
        KStopStreaming(m_hNet);
        KDisconnect(m_hNet);
        m_bConnected = false;
    }

    if(m_hNet)
    {
        KCloseInterface(m_hNet);
        m_hNet = NULL;
    }
}




void CPTZSampleDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnLButtonDown(nFlags, point);
}




void CPTZSampleDlg::SendMoveCommand(char *pcaCommand, int iParam1)
{
	/*
    DWORD dwRet = 0;
    BYTE byBuffer[64];
    BYTE *pbyBuf;
    char *pcaBuf;
    
	
    if(m_hPTZ)
    {
        if(m_iSendMode == 0) // RS232
        {
            pbyBuf = PTZGetString(m_hPTZ, pcaCommand, iParam1, 0, &dwRet);

            if(((strcmp(pcaCommand, "PANTILTSTOP") == 0)||(strcmp(pcaCommand, "ZOOMSTOP") == 0)) && (pbyBuf == NULL))
            {
                pbyBuf = PTZGetString(m_hPTZ, m_csLastCommand.GetBuffer(), 0, 0, &dwRet);
                m_csLastCommand.ReleaseBuffer();
            }
            
            if(pbyBuf)
                if(m_hNet)
                {
                    KSendPTZCommand(m_hNet, pbyBuf, (WORD)dwRet);
                    m_csLastCommand = pcaCommand;
                    m_ctrlCommandCount++;
                    SetDlgItemInt(IDC_EDIT4CommandCount, m_ctrlCommandCount);
                }
        }else // URL
        {
            pcaBuf =  PTZGetStringURL(m_hPTZ, pcaCommand, iParam1, 0, &dwRet);
            if(((strcmp(pcaCommand, "PANTILTSTOP") == 0)||(strcmp(pcaCommand, "ZOOMSTOP") == 0)) && (pcaBuf == NULL))
            {
                pcaBuf = PTZGetStringURL(m_hPTZ, m_csLastCommand.GetBuffer(), 0, 0, &dwRet);
                m_csLastCommand.ReleaseBuffer();
            }
            if(pcaBuf)
            {
                //SendURL(GetURLCommand(m_sInfo.caServerIP, m_sInfo.caAccount, m_sInfo.caPassword, pcaBuf));
                m_csLastCommand = pcaCommand;
                m_ctrlCommandCount++;
                SetDlgItemInt(IDC_EDIT4CommandCount, m_ctrlCommandCount);
            }
            // SendURL((char*)pbyBuf);            
        }
    }
	*/
}




// Load protocol from res
void CPTZSampleDlg::OnBnClickedButton4()
{
    CString csServerIP;
    CString csID;
    CString csPassword;
    CString csAddrID;
    CString csVendor;
    CString csProtocol;

    // TODO: Add your control notification handler code here
    UpdateData();

    // if PTZ was created delete it.
	/*
    if(m_hPTZ)
    {
        PTZCloseInterface(m_hPTZ);
    }
	*/

    // GetDlgItemStri
    GetDlgItemText(IDC_EDIT4ServerIP, csServerIP);
    GetDlgItemText(IDC_EDIT4ID, csID);
    GetDlgItemText(IDC_EDIT4Password, csPassword);
    GetDlgItemText(IDC_EDIT4AddrID, csAddrID);

    // server IP
    //strcpy(m_sInfo.caServerIP, csServerIP.GetBuffer());
    csServerIP.ReleaseBuffer();
    // ID
    //strcpy(m_sInfo.caAccount, csID.GetBuffer());
    csID.ReleaseBuffer();
    // Password
    //strcpy(m_sInfo.caPassword, csPassword.GetBuffer());
    csPassword.ReleaseBuffer();
    // AddrID
    //m_sInfo.dwAddrID = atoi(csAddrID.GetBuffer());
	m_mPTZ.dwAddressID = atoi(csAddrID.GetBuffer());
    csAddrID.ReleaseBuffer();

    /*
    sprintf(m_sInfo.caFilename, "%s", m_csFileName.GetBuffer());
    m_csFileName.ReleaseBuffer();
    */
    int iCurSel = m_ctrlVendorList.GetCurSel();
    m_ctrlVendorList.GetLBText(iCurSel, csVendor);
    iCurSel = m_ctrlProtocolList.GetCurSel();
    m_ctrlProtocolList.GetLBText(iCurSel, csProtocol);
    

    //strcpy(m_sInfo.caVender, csVendor.GetBuffer());
	strcpy(m_mPTZ.szVender, csVendor.GetBuffer());
    csVendor.ReleaseBuffer();
    //strcpy(m_sInfo.caProtocol, csProtocol.GetBuffer());
	strcpy(m_mPTZ.szProtocol, csProtocol.GetBuffer());
    csProtocol.ReleaseBuffer();


    m_iSendMode = m_ctrlSendMode.GetCurSel();    
    if(m_iSendMode == 0)
    {
        //m_sInfo.dwMode = SerialPort;
    }
    else
    {
        //m_sInfo.dwMode = URL; //  Sending mode
    }
	//m_mPTZ.dwAddressID = 1; // most camera device be set to 1 
	m_mPTZ.nSourceType = PTZ_MEDIA_SOURCE_RS;
	if(KPTZLoadProtocol(m_hNet,&m_mPTZ))
	{
		Sleep(1); // only for test
	}

    //m_hPTZ = PTZOpenInterface(&m_sInfo);
	/*
    if(m_hPTZ)
    {

        // Add command to PTZ list control
        DWORD dwStringLen;
        void *pBuffer;
        char caTemp[2048];
        int iPos = 0;
        CString resToken;

        pBuffer =  PTZEnumerate(m_hPTZ, PTZ_ENUM_COMMAND, &dwStringLen);

        strcpy(caTemp, (char *)pBuffer);

        CString csCmdList = caTemp;

        resToken = csCmdList.Tokenize(",", iPos);
        while (resToken != "")
        {
            // printf("Resulting token: %s\n", resToken);
            resToken = csCmdList.Tokenize(",", iPos);
            if(resToken != "")
                m_ctrlPTZCmdList.AddString(resToken);
        };
        m_ctrlPTZCmdList.SetCurSel(0);
    }
	*/
}


// Select Protocol file
void CPTZSampleDlg::OnBnClickedButton4selefile()
{
       
    static char BASED_CODE szFilter[] = "PTZ Files (*.PTZ)|*.PTZ";
    CString csServerIP;
    CString csID;
    CString csPassword;
    CString csAddrID;


   CFileDialog *oFD = new CFileDialog(TRUE, "PTZ", NULL, OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilter, this);
                                                                                                    
   if(oFD->DoModal() == IDOK)
   {
       //memset(&m_sInfo, 0, sizeof(m_sInfo));
	   memset(&m_mPTZ, 0, sizeof(m_mPTZ));

       // Get file name of PTZ protocol
       m_csFileName = oFD->GetPathName();
       m_ctrlProtocol.SetWindowText(oFD->GetFileName());
       UpdateData();

       // if PTZ was created delete it.
	   /*
       if(m_hPTZ)
       {
           PTZCloseInterface(m_hPTZ);
       }
	   */

       // GetDlgItemStri
       GetDlgItemText(IDC_EDIT4ServerIP, csServerIP);
       GetDlgItemText(IDC_EDIT4ID, csID);
       GetDlgItemText(IDC_EDIT4Password, csPassword);
       GetDlgItemText(IDC_EDIT4AddrID, csAddrID);

       // server IP
       //strcpy(m_sInfo.caServerIP, csServerIP.GetBuffer());
       csServerIP.ReleaseBuffer();
       // ID
       //strcpy(m_sInfo.caAccount, csID.GetBuffer());
       csID.ReleaseBuffer();
       // Password
       //strcpy(m_sInfo.caPassword, csPassword.GetBuffer());
       csPassword.ReleaseBuffer();
       // AddrID
       //m_sInfo.dwAddrID = atoi(csAddrID.GetBuffer());
	   m_mPTZ.dwAddressID = atoi(csAddrID.GetBuffer());//1; // most camera device be set to 1 
       csAddrID.ReleaseBuffer();

       //sprintf(m_sInfo.caFilename, "%s", m_csFileName.GetBuffer());
	   sprintf(m_mPTZ.szProtocolFileName, "%s", m_csFileName.GetBuffer());
       m_csFileName.ReleaseBuffer();

       m_iSendMode = m_ctrlSendMode.GetCurSel();
       if(m_iSendMode == 0)
       {
           //m_sInfo.dwMode = SerialPort;
       }
       else
       {
           //m_sInfo.dwMode = URL; //  Sending mode
       }
       
	   m_mPTZ.nSourceType = PTZ_MEDIA_SOURCE_FILE;
	   if(!KPTZLoadProtocol(m_hNet,&m_mPTZ))
	   {
			Sleep(1);// FAILED  
	   }

       //m_hPTZ = PTZOpenInterfaceWithFile(&m_sInfo);
       //if(!m_hPTZ) MessageBox( "Open PTZ protocol file fail! Please select correct file!","Protocol");
       
       delete oFD;

   }
   else
   {
       MessageBox( "Never open protocol file!","Protocol");
       delete oFD;
       return;
   }
   


   // Add command to PTZ list control
	

   DWORD dwStringLen;
   void *pBuffer;
   char caTemp[2048];
   int iPos = 0;
   CString resToken;

   //pBuffer =  PTZEnumerate(m_hPTZ, PTZ_ENUM_COMMAND, &dwStringLen);

   //strcpy(caTemp, (char *)pBuffer);

   CString csCmdList =  _T("17,PANLEFT,PANRIGHT,TILTUP,TILTDOWN,MOVEUPLEFT,MOVEUPRIGHT,MOVEDOWNLEFT,MOVEDOWNRIGHT,ZOOMIN,ZOOMOUT,ZOOMSTOP,FOCUSIN,FOCUSOUT,FOCUSSTOP,FOCUSAUTO,PRESETGOTO,PRESETSET");

   resToken = csCmdList.Tokenize(",", iPos);
   while (resToken != "")
   {
       // printf("Resulting token: %s\n", resToken);
       resToken = csCmdList.Tokenize(",", iPos);
       if(resToken != "")
           m_ctrlPTZCmdList.AddString(resToken);
   };
   m_ctrlPTZCmdList.SetCurSel(1);

   

}

// Load File From
void CPTZSampleDlg::OnBnClickedRadio4file()
{
    GetDlgItem(IDC_EDIT4ProtocolFileName)->EnableWindow();
    GetDlgItem(IDC_BUTTON4SeleFIle)->EnableWindow();
    GetDlgItem(IDC_COMBO4Vendor)->EnableWindow(FALSE);
    GetDlgItem(IDC_COMBO4Protocol)->EnableWindow(FALSE);
    
    
}

void CPTZSampleDlg::OnBnClickedRadio4buildin()
{
	
    DWORD dwLen = 0;
    int iPos = 0;
    CString resToken;
    char caVendor[2048];
    GetDlgItem(IDC_EDIT4ProtocolFileName)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON4SeleFIle)->EnableWindow(FALSE);
    GetDlgItem(IDC_COMBO4Vendor)->EnableWindow();
    GetDlgItem(IDC_COMBO4Protocol)->EnableWindow();
    GetDlgItem(IDC_BUTTON4)->EnableWindow();
    char *pBuffer = new char[2048] ;
	dwLen = 2048;
	if(!KPTZEnumerateVender(m_hNet, pBuffer, dwLen))
	{
		//FAILED
	}


    memcpy(caVendor, pBuffer, dwLen);
    caVendor[dwLen] = '\0';
    
    CString csVendorList = caVendor;

   resToken = csVendorList.Tokenize(",", iPos);
   while (resToken != "")
   {
       resToken = csVendorList.Tokenize(",", iPos);
       if(resToken != "")
       {
           m_ctrlVendorList.AddString(resToken);
       }
   };
   m_ctrlVendorList.SetCurSel(0);
   delete [] pBuffer;
   OnCbnSelchangeCombo4vendor();
	
    

}



void CPTZSampleDlg::OnCbnSelchangeCombo4vendor()
{
    CString csVendor;
    CString csProctol;
    int iCurSel = m_ctrlVendorList.GetCurSel();
    m_ctrlVendorList.GetLBText(iCurSel, csVendor);

    DWORD dwLen =0;
    int iPos = 0;
    CString resToken;
    char caProtocol[2048];


    //void* KPTZEnumerateProtocol(char *pVender, DWORD *dwLen);

	char *pBuffer = new char[2048] ;
	if(!KPTZEnumerateProtocol(m_hNet,csVendor.GetBuffer(), pBuffer,dwLen))
	{
		//FAILED
	}


    csVendor.ReleaseBuffer();

    memcpy(caProtocol, pBuffer, dwLen);
    caProtocol[dwLen] = '\0';

    CString csProtocolList = caProtocol;

    resToken = csProtocolList.Tokenize(",", iPos);

    m_ctrlProtocolList.Clear();
    int ggg = m_ctrlProtocolList.GetCount();
    int bbb = 0;
    for(int i = ggg-1 ; i >= 0 ; i--)
    {
        bbb = m_ctrlProtocolList.DeleteString(i);
        
    }
    

    while (resToken != "")
    {
        resToken = csProtocolList.Tokenize(",", iPos);
        if(resToken != "")
        {
            m_ctrlProtocolList.AddString(resToken);
        }
    };
	delete [] pBuffer;
    m_ctrlProtocolList.SetCurSel(0);
}


// DL
void CPTZSampleDlg::OnBnClickedButton4leftdown()
{
    //SendMoveCommand("MOVEDOWNLEFT", 3);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_MOVE_DOWN_LEFT);
}

// UL
void CPTZSampleDlg::OnBnClickedButton4leftup()
{
    //SendMoveCommand("MOVEUPLEFT", 3);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_MOVE_UP_LEFT);
}
// STOP
void CPTZSampleDlg::OnBnClickedButton4stop()
{
    //SendMoveCommand("PANTILTSTOP", 0);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 0, PTZ_MOVE_STOP);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 0, PTZ_MOVE_UP);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 0, PTZ_MOVE_LEFT);
}

// U
void CPTZSampleDlg::OnBnClickedButton4tiltup()
{
    //SendMoveCommand("TILTUP", 3);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_MOVE_UP);
}
//DOWN
void CPTZSampleDlg::OnBnClickedButton4tiltdown()
{
    //SendMoveCommand("TILTDOWN", 3);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_MOVE_DOWN);
}

// UP RIGHT
void CPTZSampleDlg::OnBnClickedButton4rightup()
{
    //SendMoveCommand("MOVEUPRIGHT", 3);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_MOVE_UP_RIGHT);
}
// RIGHT
void CPTZSampleDlg::OnBnClickedButton4panright()
{
    //SendMoveCommand("PANRIGHT", 3);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_MOVE_RIGHT);
}

// L
void CPTZSampleDlg::OnBnClickedButton4panleft()
{
    //SendMoveCommand("PANLEFT", 3);
	if(!KPTZMove(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_MOVE_LEFT))
	{
		Sleep(1); // for debug
	}
}

// DOWN RIGHT
void CPTZSampleDlg::OnBnClickedButton4rightdown()
{
    //SendMoveCommand("MOVEDOWNRIGHT", 3);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_MOVE_DOWN_RIGHT);
}






void CPTZSampleDlg::OnBnClickedButton4zoomin()
{
    //SendMoveCommand("ZOOMIN", 1);
	KPTZZoom(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_ZOOM_IN);

	/*
	BYTE bCommand[16] = {0};
	DWORD dwLen = 0;
	bCommand[0] = 0x01; // Receiver ID 
	bCommand[1] = 0x00; // Transmitter ID
	bCommand[2] = 0xC2; // OP code
	bCommand[3] = 1 & 0x000000FF; // data 0 
	bCommand[4] = (1 & 0x0000FF00)>>8; // data 1
	bCommand[5] = 23 & 0x000000FF; // data 0 
	bCommand[6] = (23 & 0x0000FF00)>>8; // data 1
	bCommand[7] = 0x80; // data 0 
	bCommand[8] = 0x30; // data 1
	bCommand[9] = 0x00; // Check sum

	bCommand[9] = bCommand[0]^bCommand[1]^bCommand[2]^bCommand[3]^bCommand[4]^bCommand[5]^bCommand[6]^bCommand[7]^bCommand[8];

	dwLen = 10;
	KSendPTZCommand(m_hNet, bCommand, dwLen);
	*/
}

void CPTZSampleDlg::OnBnClickedButton4zoomstop()
{
    //SendMoveCommand("ZOOMSTOP", 0);
	KPTZZoom(m_hNet, m_mPTZ.dwAddressID, 0, PTZ_ZOOM_STOP);
}

void CPTZSampleDlg::OnBnClickedButton4zoomout()
{
    //SendMoveCommand("ZOOMOUT", 1);

	KPTZZoom(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_ZOOM_OUT);

	/*
	BYTE bCommand[16] = {0};
	DWORD dwLen = 0;
	bCommand[0] = 0x01; // Receiver ID 
	bCommand[1] = 0x00; // Transmitter ID
	bCommand[2] = 0xC0; // OP code
	bCommand[3] = 1 & 0x000000FF; // data 0 
	bCommand[4] = (1 & 0x0000FF00)>>8; // data 1
	bCommand[5] = 23 & 0x000000FF; // data 0 
	bCommand[6] = (23 & 0x0000FF00)>>8; // data 1
	bCommand[7] = 0x00; // Check sum

	bCommand[7] = bCommand[0]^bCommand[1]^bCommand[2]^bCommand[3]^bCommand[4]^bCommand[5]^bCommand[6];

	dwLen = 8;
	KSendPTZCommand(m_hNet, bCommand, dwLen);
	*/
}

void CPTZSampleDlg::OnBnClickedButton4focusin()
{
    //SendMoveCommand("FOCUSIN", 1);
	KPTZFocus(m_hNet, m_mPTZ.dwAddressID, PTZ_FOCUS_IN);
}

void CPTZSampleDlg::OnBnClickedButton4focusstop()
{
    //SendMoveCommand("FOCUSSTOP", 0);
	KPTZFocus(m_hNet, m_mPTZ.dwAddressID, PTZ_FOCUS_STOP);
}

void CPTZSampleDlg::OnBnClickedButton4focusout()
{
    //SendMoveCommand("FOCUSOUT", 1);
	KPTZFocus(m_hNet, m_mPTZ.dwAddressID, PTZ_FOCUS_OUT);
}
// Iris On
void CPTZSampleDlg::OnBnClickedButton15()
{
    //SendMoveCommand("IRISOPEN", 1);
	KPTZIris(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_IRIS_OPEN);

}
// Iris Off
void CPTZSampleDlg::OnBnClickedButton16()
{
    //SendMoveCommand("IRISCLOSE", 1);
	KPTZIris(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_IRIS_CLOSE);
}

// 
void CPTZSampleDlg::OnBnClickedButton4stop2()
{
    //SendMoveCommand("OSDENTER", 0);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_ENTER);
}
// OSD UP
void CPTZSampleDlg::OnBnClickedButton4up2()
{
    //SendMoveCommand("OSDUP", 0);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_UP);
}

// OSD LEFT
void CPTZSampleDlg::OnBnClickedButton4left2()
{
    //SendMoveCommand("OSDLEFT", 0);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_LEFT);
}
// OSD RIGHT
void CPTZSampleDlg::OnBnClickedButton4right2()
{
    //SendMoveCommand("OSDRIGHT", 0);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_RIGHT);
}
// OSD DOWN
void CPTZSampleDlg::OnBnClickedButton4down2()
{
    //SendMoveCommand("OSDDOWN", 0);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_DOWN);
}
// OSD ON
void CPTZSampleDlg::OnBnClickedButton4osdon()
{
    //SendMoveCommand("OSDON", 0);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_ON);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_ENTER);
}
// OSD OFF
void CPTZSampleDlg::OnBnClickedButton4osdoff()
{
    //SendMoveCommand("OSDUP", 0);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_OFF);
}
// OSD LEAVE
void CPTZSampleDlg::OnBnClickedButton4osdleave()
{
    //SendMoveCommand("OSDLEAVE", 0);
	KPTZOSD(m_hNet, m_mPTZ.dwAddressID, PTZ_OSD_LEAVE);
}

void CPTZSampleDlg::OnBnClickedButton4blcon()
{
    //SendMoveCommand("BLC", 1);
	KPTZBLC(m_hNet, m_mPTZ.dwAddressID, PTZ_BLC_ON);
}

void CPTZSampleDlg::OnBnClickedButton4blcon4blcoff()
{
    //SendMoveCommand("BLC", 0);
	KPTZBLC(m_hNet, m_mPTZ.dwAddressID, PTZ_BLC_OFF);
}

void CPTZSampleDlg::OnBnClickedButton4blcon4dandnon()
{
    //SendMoveCommand("DAYNIGHT", 1);
	KPTZDayNight(m_hNet, m_mPTZ.dwAddressID, PTZ_DN_ON);
}

void CPTZSampleDlg::OnBnClickedButton4blcon4dandnoff()
{
    //SendMoveCommand("DAYNIGHT", 0);
	KPTZDayNight(m_hNet, m_mPTZ.dwAddressID, PTZ_DN_OFF);
}

void CPTZSampleDlg::OnBnClickedButton4blcon4daynightauto()
{
    //SendMoveCommand("DAYNIGHTAUTO", 1);
	KPTZDayNight(m_hNet, m_mPTZ.dwAddressID, PTZ_DN_AUTO_ON);
}

void CPTZSampleDlg::OnBnClickedButton4blcon4daynightautooff()
{
    //SendMoveCommand("DAYNIGHTAUTO", 0);
	KPTZDayNight(m_hNet, m_mPTZ.dwAddressID, PTZ_DN_AUTO_OFF);
}

void CPTZSampleDlg::OnBnClickedButton14()
{
    //SendMoveCommand("PRESETSET", 1);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton17()
{
    //SendMoveCommand("PRESETSET", 2);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 2, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton18()
{
    //SendMoveCommand("PRESETSET", 3);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 3, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton19()
{
    //SendMoveCommand("PRESETSET", 4);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 4, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton20()
{
    //SendMoveCommand("PRESETSET", 5);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 5, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton21()
{
    //SendMoveCommand("PRESETSET", 6);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 6, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton22()
{
    //SendMoveCommand("PRESETSET", 7);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 7, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton23()
{
    //SendMoveCommand("PRESETSET", 8);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 8, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton24()
{
    //SendMoveCommand("PRESETSET", 9);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 9, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton25()
{
    //SendMoveCommand("PRESETSET", 10);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 10, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton26()
{
    //SendMoveCommand("PRESETSET", 11);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 11, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton27()
{
    //SendMoveCommand("PRESETSET", 12);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 12, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton28()
{
    //SendMoveCommand("PRESETSET", 13);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 13, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton29()
{
    //SendMoveCommand("PRESETSET", 14);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 14, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton30()
{
    //SendMoveCommand("PRESETSET", 15);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 15, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton31()
{
    //SendMoveCommand("PRESETSET", 16);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 16, PTZ_PRESET_SET);
}

void CPTZSampleDlg::OnBnClickedButton32()
{
    //SendMoveCommand("PRESETGOTO", 1);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 1, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton33()
{
    //SendMoveCommand("PRESETGOTO", 2);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 2, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton34()
{
    //SendMoveCommand("PRESETGOTO", 3);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 3, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton35()
{
    //SendMoveCommand("PRESETGOTO", 4);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 4, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton36()
{
    //SendMoveCommand("PRESETGOTO", 5);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 5, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton37()
{
    //SendMoveCommand("PRESETGOTO", 6);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 6, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton38()
{
    //SendMoveCommand("PRESETGOTO", 7);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 7, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton39()
{
    //SendMoveCommand("PRESETGOTO", 8);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 8, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton40()
{
    //SendMoveCommand("PRESETGOTO", 9);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 9, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton41()
{
    //SendMoveCommand("PRESETGOTO", 10);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 10, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton42()
{
    //SendMoveCommand("PRESETGOTO", 11);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID,11, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton43()
{
    //SendMoveCommand("PRESETGOTO", 12);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 12, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton44()
{
    //SendMoveCommand("PRESETGOTO", 13);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 13, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton45()
{
    //SendMoveCommand("PRESETGOTO", 14);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 14, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton46()
{
    //SendMoveCommand("PRESETGOTO", 15);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 15, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton47()
{
    //SendMoveCommand("PRESETGOTO", 16);
	KPTZPreset(m_hNet, m_mPTZ.dwAddressID, 16, PTZ_PRESET_GOTO);
}

void CPTZSampleDlg::OnBnClickedButton48()
{
    //SendMoveCommand("PRESETCLEAR", 1);
}

void CPTZSampleDlg::OnBnClickedButton49()
{
    //SendMoveCommand("PRESETCLEAR", 2);
}

void CPTZSampleDlg::OnBnClickedButton50()
{
    //SendMoveCommand("PRESETCLEAR", 3);
}

void CPTZSampleDlg::OnBnClickedButton51()
{
    //SendMoveCommand("PRESETCLEAR", 4);
}

void CPTZSampleDlg::OnBnClickedButton52()
{
    //SendMoveCommand("PRESETCLEAR", 5);
}

void CPTZSampleDlg::OnBnClickedButton53()
{
    //SendMoveCommand("PRESETCLEAR", 6);
}

void CPTZSampleDlg::OnBnClickedButton54()
{
    //SendMoveCommand("PRESETCLEAR", 7);
}

void CPTZSampleDlg::OnBnClickedButton55()
{
    //SendMoveCommand("PRESETCLEAR", 8);
}

void CPTZSampleDlg::OnBnClickedButton56()
{
    //SendMoveCommand("PRESETCLEAR", 9);
}

void CPTZSampleDlg::OnBnClickedButton57()
{
    //SendMoveCommand("PRESETCLEAR", 10);
}

void CPTZSampleDlg::OnBnClickedButton58()
{
    //SendMoveCommand("PRESETCLEAR", 11);
}

void CPTZSampleDlg::OnBnClickedButton59()
{
    //SendMoveCommand("PRESETCLEAR", 12);
}

void CPTZSampleDlg::OnBnClickedButton60()
{
    //SendMoveCommand("PRESETCLEAR", 13);
}

void CPTZSampleDlg::OnBnClickedButton61()
{
    //SendMoveCommand("PRESETCLEAR", 14);
}

void CPTZSampleDlg::OnBnClickedButton62()
{
    //SendMoveCommand("PRESETCLEAR", 15);
}

void CPTZSampleDlg::OnBnClickedButton63()
{
    //SendMoveCommand("PRESETCLEAR", 16);
}


DWORD WINAPI ThreadProc( LPVOID lpParam ) 
{
    CPTZSampleDlg *p = (CPTZSampleDlg *)lpParam;

    p->Testing();

    return 0;
}
void CPTZSampleDlg::Testing()
{
	
    m_bTest = true;

	//SendMoveCommand("PANLEFT", 5);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 5, PTZ_MOVE_LEFT);
	Sleep(100);
	//SendMoveCommand("PANRIGHT", 5);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 5, PTZ_MOVE_RIGHT);
	Sleep(100);
	// SendMoveCommand("PANUP", 5);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 5, PTZ_MOVE_UP);
	Sleep(100);
	// SendMoveCommand("PANDOWN", 5);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 5, PTZ_MOVE_DOWN);
	Sleep(100);
	// SendMoveCommand("PANTILTSTOP", 5);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 0, PTZ_MOVE_UP);
	KPTZMove(m_hNet, m_mPTZ.dwAddressID, 0, PTZ_MOVE_LEFT);
    
	
}
void CPTZSampleDlg::OnBnClickedButton3()
{
    HANDLE hThread = CreateThread( 
        NULL,              // default security attributes
        0,                 // use default stack size  
        ThreadProc,        // thread function 
        this,             // argument to thread function 
        0,                 // use default creation flags 
        0);   // returns the thread identifier 

    if(hThread)
    {
        CloseHandle(hThread);

    }
}

void CPTZSampleDlg::OnBnClickedButton5()
{
    m_bTest = false;
}

void CPTZSampleDlg::OnBnClickedOk4()
{
    // TODO: Add your control notification handler code here
}

void CPTZSampleDlg::OnEnChangeEdit4urlcmdoutput()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CPTZSampleDlg::OnCbnSelchangeCombo4command()
{
	// TODO: Add your control notification handler code here
}

void CPTZSampleDlg::OnEnChangeEdit4param1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CPTZSampleDlg::OnEnChangeEdit4param2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CPTZSampleDlg::OnEnChangeEdit10()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CPTZSampleDlg::OnEnChangeEdit11()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CPTZSampleDlg::OnEnChangeEdit12()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CPTZSampleDlg::OnBnClickedButton6()
{
	// Pan and Tilt and Zoom
	BYTE bCommand[128] = {0};
	DWORD dwLen = 0;
	bool IsCCW = false;
	IsCCW = !(bool)m_ctrlIsCCW.GetCurSel();

	UpdateData();
	
	KPTZGetAbsPTZCommand(m_hNet,
		"SETABSOLUTEPTZ", 
		0, 
		0,
		IsCCW,
		m_fPanDegree,
		m_fTilteDegree,
		m_fZoomDegree,
		bCommand,
		dwLen);
	
	// marked ver1.2 functions below tested and succeed

	/*// catch PTZ command by unit (provided by hardware), a second choice besides degree
	KPTZGetAbsPTZCommandByUnit(m_hNet,
		"SETABSOLUTEPTZ", 
		0, 
		0,
		IsCCW,
		89,
		40,
		3,       
		bCommand,
		dwLen);
	*/
	
	/*//test UNIT to degree and degree to UNIT
	int iPan = 800;
	int iTilt = 100;
	int iZoom = 15;
	float fPan = 0;
	float fTilt = 0;
	float fZoom = 0;
	KPTZUnitToDegree(m_hNet,
					iPan,
					iTilt,
					iZoom,
					fPan,
					fTilt,
					fZoom);

	KPTZDegreeToUnit(m_hNet, 
					fPan,
					fTilt,
					fZoom,
					iPan,
					iTilt,
					iZoom);
	*/
	/*
	//Test catching unit from callback buffer
	BYTE bPan[18] = { 0x00, 0x01, 0xb2, 0x00, 0x00, 0xb3, 0x00, 0x01, 0x0a, 0x17, 0x00, 0x1c, 0x00, 0x01, 0x09, 0x0a, 0x00, 0x02 };
	int np, nt, nz;
	KPTZGetUnitFromBuffer( m_hNet, bPan, 18, np, nt, nz );
	*/

	/*//Test Request of camera ptz status
	//get this command , and set it to camera. 
	//Camera will send ptz status back (in several packet),
	//and you may gather those packet in one buffer, and parse it by KPTZGetUnitFromBuffer()

	KPTZGetRequestAbsPTZCommand(m_hNet, 0, bCommand, dwLen);
	*/
	


	KSendPTZCommand(m_hNet, bCommand, dwLen);
}

void CPTZSampleDlg::OnBnClickedButton7()
{
	//Pan and Tilt only
	BYTE bCommand[128] = {0};
	DWORD dwLen = 0;
	bool IsCCW = false;
	IsCCW = !(bool)m_ctrlIsCCW.GetCurSel();

	UpdateData();
	
	KPTZGetAbsPTZCommand(m_hNet,
		"SETABSOLUTEPT", 
		0, 
		0,
		IsCCW,
		m_fPanDegree,
		m_fTilteDegree,
		m_fZoomDegree,       //zoom is dummy input
		bCommand,
		dwLen);


	KSendPTZCommand(m_hNet, bCommand, dwLen);
}
 
void CPTZSampleDlg::OnCbnSelchangeCombo4baudrate()
{
	// TODO: Add your control notification handler code here
}

void CPTZSampleDlg::OnCbnSelchangeCombo4parity()
{
	// TODO: Add your control notification handler code here
}

void CPTZSampleDlg::OnCbnSelchangeCombo2()
{
	// TODO: Add your control notification handler code here
}
