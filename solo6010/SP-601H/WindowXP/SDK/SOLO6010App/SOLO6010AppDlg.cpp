// SOLO6010AppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SOLO6010App.h"
#include "SOLO6010AppDlg.h"
#include "DlgSettingRecord.h"
#include "DlgSettingGeneral.h"
#include "DlgCtrlGPIO.h"
#include "DlgCtrlRegister.h"
#include "DlgCtrlUART.h"
#include "DlgTest.h"
#include "DlgCtrlI2C.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INFO_APP_GLOBAL glBufGInfo[NUM_MAX_S6010];

#define STR_BUT_REC_TEXT_REC_START	"Start"
#define STR_BUT_REC_TEXT_REC_STOP	"Stop"

INFO_APP_GLOBAL *glPCurGInfo;
CSOLO6010AppDlg *glPDlgApp;
CWndLiveDisp *glPWndLiveDisp;
CSOLO6010 *glPCurSelS6010;
CDlgCtrlLive *glPDlgCtrlLive;
CRecManager *glPCurRecManager;
CHWDecManager *glPCurHWDecManager;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSOLO6010AppDlg dialog

CSOLO6010AppDlg::CSOLO6010AppDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSOLO6010AppDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSOLO6010AppDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	SetFastMemcpy ();

	int i, j, k, l;
	CFile fp;
	char strConfigFilePath[MAX_PATH];

	GetCurrentDirectory (MAX_PATH, m_strCurDir);
	if (m_strCurDir[strlen (m_strCurDir) -1] == '\\')
	{
		sprintf (strConfigFilePath, "%s%s", m_strCurDir, STR_G_INFO_FILE_NAME);
	}
	else
	{
		sprintf (strConfigFilePath, "%s\\%s", m_strCurDir, STR_G_INFO_FILE_NAME);
	}
	if (fp.Open (strConfigFilePath, CFile::modeRead) == FALSE ||
		fp.GetLength () != sizeof(INFO_APP_GLOBAL) *NUM_MAX_S6010)
	{	// init INFO_APP_GLOBAL (first time)
		for (i=0; i<NUM_MAX_S6010; i++)
		{
			memset (&glBufGInfo[i], 0, sizeof(INFO_APP_GLOBAL));

			glBufGInfo[i].typeVideo = VIDEO_TYPE_NTSC;
			glBufGInfo[i].szVideo.cx = VIDEO_IN_H_SIZE;
			glBufGInfo[i].szVideo.cy = 480;

			strcpy (glBufGInfo[i].strRecDir, "C:\\");

			// SETTING_REC Init[S]
			for (j=0; j<NUM_VID_TOTAL_ENC_CH; j++)
			{
				glBufGInfo[i].setRec.setMP4[j].nQuality = 2;
				glBufGInfo[i].setRec.setMP4[j].nKeyFrmInterval = 15;
				glBufGInfo[i].setRec.setMP4[j].nMotionTh = 200;
			}

			glBufGInfo[i].setRec.setG723.idxSampRate = G723_SAMPLING_RATE_8K;
			for (j=0; j<NUM_G723_CH; j++)
			{
				glBufGInfo[i].setRec.setG723.bufBEnc[j] = 0;
			}
			glBufGInfo[i].setRec.modeRecMP4 = REC_DATA_RCV_BY_RAW_DATA;
			glBufGInfo[i].setRec.modeRecJPEG = REC_DATA_RCV_BY_RAW_DATA;
			glBufGInfo[i].setRec.modeRecG723 = REC_DATA_RCV_BY_RAW_DATA;
			glBufGInfo[i].setRec.nNotifyBufLevelMP4 = DEFAULT_REC_BUF_NOTIFY_LEVEL;
			glBufGInfo[i].setRec.nNotifyBufLevelJPEG = DEFAULT_REC_BUF_NOTIFY_LEVEL;
			glBufGInfo[i].setRec.nNotifyBufLevelG723 = DEFAULT_REC_BUF_NOTIFY_LEVEL;
			// SETTING_REC Init[E]

			// Live Init[S]
			glBufGInfo[i].idxVCard = VCARD_OTHER;
			// Live Init[S]

			// Mosaic init[S]
			glBufGInfo[i].MOSAIC_nStrength = MIN_MOSAIC_STRENGTH;
			// Mosaic init[E]

			// Video Motion Init[S]
			for (j=0; j<NUM_LIVE_DISP_WND; j++)
			{
				glBufGInfo[i].VM_bufBUse[j] = FALSE;
				for (k=0; k<SZ_BUF_V_MOTION_TH_IN_USHORT; k++)
				{
					glBufGInfo[i].VM_bbufTh[j][k] = NUM_DEF_V_MOTION_TH;
				}
			}
			glBufGInfo[i].VM_minCntTh = NUM_DEF_MIN_DET_CNT;
			glBufGInfo[i].VM_bShowGrid = FALSE;
			glBufGInfo[i].VM_bExecThSetting = FALSE;
			glBufGInfo[i].VM_rcCurSelTh.left = 0;
			glBufGInfo[i].VM_rcCurSelTh.right = 0;
			glBufGInfo[i].VM_rcCurSelTh.top = 0;
			glBufGInfo[i].VM_rcCurSelTh.bottom = 0;

			glBufGInfo[i].VM_clrModeBorderY = IDX_VM_GRID_CLR_CLEAR;
			glBufGInfo[i].VM_clrModeBorderCb = IDX_VM_GRID_CLR_CLEAR;
			glBufGInfo[i].VM_clrModeBorderCr = IDX_VM_GRID_CLR_CLEAR;
			glBufGInfo[i].VM_clrModeBarY = IDX_VM_GRID_CLR_CLEAR;
			glBufGInfo[i].VM_clrModeBarCb = IDX_VM_GRID_CLR_CLEAR;
			glBufGInfo[i].VM_clrModeBarCr = IDX_VM_GRID_CLR_CLEAR;
			glBufGInfo[i].VM_clrValBorderY = 0;
			glBufGInfo[i].VM_clrValBorderCb = 0;
			glBufGInfo[i].VM_clrValBorderCr = 0;
			glBufGInfo[i].VM_clrValBarY = 0;
			glBufGInfo[i].VM_clrValBarCb = 0;
			glBufGInfo[i].VM_clrValBarCr = 0;
			// Video Motion Init[E]

			// Camera Block Detection Init[S]
			for (j=0; j<NUM_LIVE_DISP_WND; j++)
			{
				glBufGInfo[i].CBD_bufBUseCBD[j] = FALSE;
				glBufGInfo[i].CBD_bufNumAlarmLevel[j] = DEF_ALARM_LEVEL;
			}
			glBufGInfo[i].CBD_timeCheckInterval = DEF_CHECK_INTERVAL;
			// Camera Block Detection Init[E]

			// Advanced Motion Detection Init[S]
			for (j=0; j<NUM_LIVE_DISP_WND; j++)
			{
				glBufGInfo[i].ADVMD_bufBUse[j] = FALSE;
				glBufGInfo[i].ADVMD_bufNumMotionCheck[j] = 1;

				glBufGInfo[i].ADVMD_bbufMotDataDist[j][0] = 12;
				glBufGInfo[i].ADVMD_bbufMotDataDist[j][1] = 1;
				glBufGInfo[i].ADVMD_bbufMotDataDist[j][2] = 1;
				glBufGInfo[i].ADVMD_bbufMotDataDist[j][3] = 1;

				for (k=0; k<MAX_ADV_MOT_CHECK_ITEM; k++)
				{
					for (l=0; l<SZ_BUF_V_MOTION_TH_IN_USHORT; l++)
					{
						glBufGInfo[i].ADVMD_bbbufMotionTh[j][k][l] = NUM_DEF_V_MOTION_TH;
					}
				}

				glBufGInfo[i].ADVMD_bufCntMinDet[j] = 1;
			}	
	
			glBufGInfo[i].ADVMD_bExecThSetting = FALSE;
			glBufGInfo[i].ADVMD_rcCurSelTh.left = 0;
			glBufGInfo[i].ADVMD_rcCurSelTh.top = 0;
			glBufGInfo[i].ADVMD_rcCurSelTh.right = 0;
			glBufGInfo[i].ADVMD_rcCurSelTh.bottom = 0;
			// Advanced Motion Detection Init[E]

			// Video Matrix Init[S]
			for (j=0; j<NUM_LIVE_DISP_WND; j++)
			{
				glBufGInfo[i].bufIdxCamToWndCh[j] = j;
			}
			// Video Matrix Init[E]

			// Live Display Mode[S]
			glBufGInfo[i].idxVCard = VCARD_OTHER;
			glBufGInfo[i].modeLiveDisp = LIVE_DISP_N_OVERLAY_60_FI_FLIP;
			glBufGInfo[i].modeDeinterlacing = LIVE_DI_BOB;
			glBufGInfo[i].numOverSurf = NUM_DEF_FLIP_OVER_60FPS_SURF;
			// Live Display Mode[E]

			// Video Loss[S]
			glBufGInfo[i].VL_checkInterval = 500;
			// Video Loss[E]

			// SW Player[S]
			glBufGInfo[i].bUseMP4Flt_Deblk = TRUE;
			glBufGInfo[i].bUseMP4Flt_Dering = TRUE;

			glBufGInfo[i].bUseImgFlt_Deint = TRUE;
			glBufGInfo[i].bUseImgFlt_UpSc = TRUE;
			glBufGInfo[i].bUseImgFlt_DnSc = TRUE;
			// SW Player[E]

			// Encoder OSD[S]
			glBufGInfo[i].ENC_OSD_bShowCh = TRUE;
			glBufGInfo[i].ENC_OSD_bShowDate = TRUE;
			glBufGInfo[i].ENC_OSD_bShowTime = TRUE;

			glBufGInfo[i].ENC_OSD_bUseShadowText = TRUE;

			glBufGInfo[i].ENC_OSD_colorY = 0xff;
			glBufGInfo[i].ENC_OSD_colorU = 0x80;
			glBufGInfo[i].ENC_OSD_colorV = 0x80;
			
			glBufGInfo[i].ENC_OSD_offsetH = 0;
			glBufGInfo[i].ENC_OSD_offsetV = 0;
			// Encoder OSD[E]

			// GPIO[S]
			for (j=0; j<NUM_GPIO_ALL_PORT; j++)
			{
				if (GPIO_PORT(j) &GPIO_INIT_MODE_IN)
				{
					glBufGInfo[i].GPIO_bufMode[j] = GPIO_MODE_IN_OR_INT;
				}
				else if (GPIO_PORT(j) &GPIO_INIT_MODE_OUT)
				{
					glBufGInfo[i].GPIO_bufMode[j] = GPIO_MODE_OUT;
				}
				else if (GPIO_PORT(j) &GPIO_INIT_MODE_I2C)
				{
					glBufGInfo[i].GPIO_bufMode[j] = GPIO_MODE_I2C;
				}
				else if (GPIO_PORT(j) &GPIO_INIT_MODE_ETC)
				{
					glBufGInfo[i].GPIO_bufMode[j] = GPIO_MODE_ETC;
				}
				
				glBufGInfo[i].GPIO_bufIntCfg[j] = GPIO_INT_CFG_DISABLE_INTERRUPT;
			}
			// GPIO[E]
		}
	}
	else
	{	// load INFO_APP_GLOBAL
		fp.Read (glBufGInfo, sizeof(INFO_APP_GLOBAL) *NUM_MAX_S6010);
		fp.Close ();
	}

	m_numS6010 = 0;
	m_curS6010 = 0;

	for (i=0; i<NUM_MAX_S6010; i++)
	{
		m_bufPClS6010[i] = NULL;

		glBufGInfo[i].bOpenS6010 = FALSE;

		glBufGInfo[i].bOpenS6010 = FALSE;
		glBufGInfo[i].bInitS6010 = FALSE;

		glBufGInfo[i].bOnLive = FALSE;
		glBufGInfo[i].bOnRec = FALSE;

		// Info Live Init[S]
		glBufGInfo[i].idxLiveSelWnd = 0;
		glBufGInfo[i].bLiveDispCh = TRUE;
		glBufGInfo[i].bSetMosaicVM = FALSE;
		glBufGInfo[i].idxModeSetMosaic = IDX_SETM_MODE_MOSAIC;
		if (glBufGInfo[i].idxVCard == VCARD_INTEL_ONBOARD)
		{
			glBufGInfo[i].clrOverlay = COLOR_OVERLAY_DEST_INTEL;
		}
		else	// VCARD_OTHER
		{
			glBufGInfo[i].clrOverlay = COLOR_OVERLAY_DEST;
		}
		glBufGInfo[i].bAlwaysTop = 0;
		// Info Live Init[E]
	}

	m_bPlayerOn = FALSE;
}

void CSOLO6010AppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSOLO6010AppDlg)
	DDX_Control(pDX, IDC_COM_CUR_S6010, m_comCurS6010);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CH_REC_S6010_0, m_bufChRecS6010[0]);
	DDX_Control(pDX, IDC_CH_REC_S6010_1, m_bufChRecS6010[1]);
	DDX_Control(pDX, IDC_CH_REC_S6010_2, m_bufChRecS6010[2]);
	DDX_Control(pDX, IDC_CH_REC_S6010_3, m_bufChRecS6010[3]);
	DDX_Control(pDX, IDC_CH_REC_S6010_4, m_bufChRecS6010[4]);
	DDX_Control(pDX, IDC_CH_REC_S6010_5, m_bufChRecS6010[5]);
}

BEGIN_MESSAGE_MAP(CSOLO6010AppDlg, CDialog)
	ON_MESSAGE(WM_DOO_APP_DLG_CTRL, OnGetMsgAppDlgCtrl)
	//{{AFX_MSG_MAP(CSOLO6010AppDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUT_LIVE, OnButLive)
	ON_BN_CLICKED(IDC_BUT_PLAYER, OnButPlayer)
	ON_BN_CLICKED(IDC_BUT_RECORD, OnButRecord)
	ON_BN_CLICKED(IDC_BUT_SETTING_RECORD, OnButSettingRecord)
	ON_BN_CLICKED(IDC_BUT_SETTING_GENERAL, OnButSettingGeneral)
	ON_CBN_SELCHANGE(IDC_COM_CUR_S6010, OnSelchangeComCurS6010)
	ON_BN_CLICKED(IDC_BUT_CTRL_GPIO, OnButGpio)
	ON_BN_CLICKED(IDC_BUT_CTRL_REGISTER, OnButCtrlRegister)
	ON_BN_CLICKED(IDC_BUT_CTRL_UART, OnButCtrlUart)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUT_TEST, OnButTest)
	ON_BN_CLICKED(IDC_BUT_CTRL_I2C, OnButCtrlI2c)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSOLO6010AppDlg message handlers

BOOL CSOLO6010AppDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	int i;

	// Init SOLO6010[S]
	m_numS6010 = CSOLO6010::GENERAL_GetNumOfS6010 ();
	if (m_numS6010 == 0)
	{
		AfxMessageBox ("SOLO6010 PCI card does not exist.\n"\
			"Please install SOLO6010 PCI card first.");
	}
	else
	{
		for (i=0; i<m_numS6010; i++)
		{
			if (OpenS6010 (i) == TRUE)
			{
				InitS6010 (i);
			}
		}
		for (i=0; i<m_numS6010; i++)
		{
			if (glBufGInfo[i].bInitS6010 == TRUE)
			{
				m_curS6010 = i;
				break;
			}
		}
	}
	// Init SOLO6010[E]

	char strTmp[MAX_PATH];

	// Dialog item init[S]
	for (i=0; i<m_numS6010; i++)
	{
		if (glBufGInfo[i].bInitS6010 == TRUE)
		{
			sprintf (strTmp, "SOLO6010-%d", i +1);
		}
		else
		{
			sprintf (strTmp, "SOLO6010-%d (NOT WORKING)", i +1);
		}

		m_comCurS6010.AddString (strTmp);
	}
	m_comCurS6010.SetCurSel (m_curS6010);

	for (; i<NUM_MAX_S6010; i++)
	{
		m_bufChRecS6010[i].ShowWindow (SW_HIDE);
	}
	m_bufChRecS6010[0].SetCheck (TRUE);

	EnableCtrlFromS6010Stat ();
	// Dialog item init[E]
	
	// Current global information pointer init[S]
	glPCurGInfo = &glBufGInfo[m_curS6010];
	glPCurSelS6010 = m_bufPClS6010[m_curS6010];
	glPCurRecManager = &m_bufClRecManager[m_curS6010];
	glPCurHWDecManager = &m_bufClHWDecManager[m_curS6010];
	// Current global information pointer init[E]

	// Class register for live and player window[S]
	CString strClsName = AfxRegisterWndClass (CS_DBLCLKS |CS_OWNDC,
      ::LoadCursor(NULL, IDC_ARROW),
	  (HBRUSH)::GetStockObject(WHITE_BRUSH),
      ::LoadIcon(NULL, IDI_APPLICATION));

	int hRes = GetSystemMetrics (SM_CXSCREEN);
	int vRes = GetSystemMetrics (SM_CYSCREEN);
	int sizeFrmH = GetSystemMetrics (SM_CXDLGFRAME);
	int sizeFrmV = GetSystemMetrics (SM_CYDLGFRAME);
	int szWndH = glPCurGInfo->szVideo.cx +sizeFrmH *2;
	int szWndV = glPCurGInfo->szVideo.cy +sizeFrmV *2;
	// Class register for live and player window[E]

	// Live wnd & ctrl create[S]...
	m_wndLiveDisp.CreateEx (0, strClsName, NULL, WS_POPUP |WS_THICKFRAME,
      (hRes -szWndH) /2, (vRes -szWndV) /2,
	  szWndH, szWndV, this->GetSafeHwnd (), NULL);
	m_wndLiveDisp.InitDDraw ();
	// Live wnd & ctrl create[E]...

	// Player init & player window create[S]
	m_wndPlayer.CreateEx (0, strClsName, NULL, WS_POPUP |WS_THICKFRAME,
      (hRes -szWndH) /2, (vRes -szWndV) /2,
	  szWndH, szWndV, this->GetSafeHwnd (), NULL);

	m_manPlayer.InitPlayer (&m_wndPlayer);
	m_wndPlayer.SetPlayerManager (&m_manPlayer);
	// Player init & player window create[E]

	// reposition this dialog[S]
	SetWindowPos (NULL, (hRes -szWndH) /2, (vRes -szWndV) /2 +szWndV, 0, 0, SWP_NOOWNERZORDER |SWP_NOSIZE |SWP_NOZORDER);
	// reposition this dialog[E]

	glPDlgApp = this;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSOLO6010AppDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSOLO6010AppDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSOLO6010AppDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSOLO6010AppDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
	WriteConfig ();
}

void CSOLO6010AppDlg::OnButLive() 
{
	// TODO: Add your control notification handler code here
	
	glBufGInfo[m_curS6010].bSetMosaicVM = FALSE;

	ToggleLive ();
}

void CSOLO6010AppDlg::ToggleLive ()
{
	if (glBufGInfo[m_curS6010].bOnLive == FALSE)	// Live start
	{
		glPWndLiveDisp->ShowWindow (SW_SHOW);

		INFO_DDRAW_SURF iDDrawSurf;
		if (glPWndLiveDisp->StartLiveDisplay (&iDDrawSurf) == FALSE)
		{
			glPWndLiveDisp->ShowWindow (SW_HIDE);
			return;
		}

		if (glBufGInfo[m_curS6010].modeLiveDisp == LIVE_DISP_SYS_TO_OVERLAY_60_FI_FLIP ||
			glBufGInfo[m_curS6010].modeLiveDisp == LIVE_DISP_SYS_TO_OVERLAY_30_FR_FLIP)
		{
			m_bufPClS6010[m_curS6010]->LIVE_RegisterCallback (CbFn_SysToOverlayAndFlip, glPWndLiveDisp);
		}
		else
		{
			m_bufPClS6010[m_curS6010]->LIVE_RegisterCallback (CbFn_FlipOverlay, glPWndLiveDisp);
		}
		m_bufPClS6010[m_curS6010]->VMOTION_RegisterAlarmCallback (CbFn_VMotionAlarm, glPWndLiveDisp);
		m_bufPClS6010[m_curS6010]->CAM_BLK_DET_RegisterAlarmCallback (CbFn_CamBlkDetAlarm, glPWndLiveDisp);
		m_bufPClS6010[m_curS6010]->ADV_MOT_RegisterAlarmCallback (CbFn_AdvMotDetAlarm, glPWndLiveDisp);
		m_bufPClS6010[m_curS6010]->LIVE_SetProp ();
		m_bufPClS6010[m_curS6010]->LIVE_Start (&iDDrawSurf);

		glPWndLiveDisp->SetVariablesFromInfoDDraw (&iDDrawSurf);

		glBufGInfo[m_curS6010].bOnLive = TRUE;

		m_comCurS6010.EnableWindow (FALSE);
	}
	else	// Live end
	{
		glBufGInfo[m_curS6010].bOnLive = FALSE;
		m_bufPClS6010[m_curS6010]->LIVE_UnregisterCallback ();
		m_bufPClS6010[m_curS6010]->LIVE_End ();
		m_bufPClS6010[m_curS6010]->VMOTION_UnregisterAlarmCallback ();
		m_bufPClS6010[m_curS6010]->CAM_BLK_DET_UnregisterAlarmCallback ();
		m_bufPClS6010[m_curS6010]->ADV_MOT_UnregisterAlarmCallback ();
		if (glPWndLiveDisp != NULL) glPWndLiveDisp->EndLiveDisplay (TRUE);
		if (glPWndLiveDisp != NULL) glPWndLiveDisp->ShowWindow (SW_HIDE);

		if (glBufGInfo[m_curS6010].bOnRec == FALSE)
		{
			m_comCurS6010.EnableWindow (TRUE);
		}
	}
}


void CSOLO6010AppDlg::OnButPlayer() 
{
	// TODO: Add your control notification handler code here
	
	if (m_bPlayerOn == FALSE)	// Player start
	{
		m_wndPlayer.ShowWindow (SW_SHOW);
		m_bPlayerOn = TRUE;
	}
	else	// Player end
	{
		m_wndPlayer.ShowWindow (SW_HIDE);
		m_manPlayer.CloseFile ();
		m_bPlayerOn = FALSE;
	}
}

void CSOLO6010AppDlg::OnButRecord() 
{
	// TODO: Add your control notification handler code here
	
	int i, numRecS6010, idxFirstRecS6010;

	numRecS6010 = 0;
	idxFirstRecS6010 = -1;
	for (i=0; i<m_numS6010; i++)
	{
		if (m_bufChRecS6010[i].GetCheck () == TRUE)
		{
			numRecS6010++;
			if (idxFirstRecS6010 == -1)
			{
				idxFirstRecS6010 = i;
			}
		}
	}

	if (numRecS6010 == 0)
	{
		AfxMessageBox ("No Solo6010 was selected.\nSelect Solo6010 first.");
		return;
	}

	if (glBufGInfo[idxFirstRecS6010].bOnRec == FALSE)	// Record Start...
	{
		for (i=0; i<m_numS6010; i++)
		{
			if (m_bufChRecS6010[i].GetCheck () == TRUE)
			{
				if (glBufGInfo[i].bOpenS6010 == FALSE ||
					glBufGInfo[i].bInitS6010 == FALSE)
				{
					continue;
				}

				if (m_bufClRecManager[i].StartRec (&glBufGInfo[i].setRec) == FALSE)
				{
					continue;
				}
				glBufGInfo[i].bOnRec = TRUE;
			}
		}

		GetDlgItem (IDC_BUT_RECORD)->SetWindowText (STR_BUT_REC_TEXT_REC_STOP);
		for (i=0; i<m_numS6010; i++)
		{
			m_bufChRecS6010[i].EnableWindow (FALSE);
		}
	}
	else	// Record Stop...
	{
		for (i=0; i<m_numS6010; i++)
		{
			if (m_bufChRecS6010[i].GetCheck () == TRUE)
			{
				m_bufClRecManager[i].StopRec ();
			}
		}
		for (i=0; i<m_numS6010; i++)
		{
			if (m_bufChRecS6010[i].GetCheck () == TRUE)
			{
				m_bufClRecManager[i].EndRec ();
				glBufGInfo[i].bOnRec = FALSE;
			}
		}

		GetDlgItem (IDC_BUT_RECORD)->SetWindowText (STR_BUT_REC_TEXT_REC_START);
		for (i=0; i<m_numS6010; i++)
		{
			m_bufChRecS6010[i].EnableWindow (TRUE);
		}
	}
}

void CSOLO6010AppDlg::OnButSettingRecord() 
{
	// TODO: Add your control notification handler code here
	
	CDlgSettingRecord dlg;

	if (m_numS6010 > 0)
	{
		if (dlg.DoModal () == IDOK)
		{
			WriteConfig ();
		}
	}
}

void CSOLO6010AppDlg::OnButSettingGeneral() 
{
	// TODO: Add your control notification handler code here
	
	CDlgSettingGeneral dlg;
	if (m_numS6010 > 0 && dlg.DoModal () == IDOK && dlg.m_bChangedGlGInfo == TRUE)
	{
		CloseS6010 (m_curS6010);
		OpenS6010 (m_curS6010);
		InitS6010 (m_curS6010);
		
		glPCurSelS6010 = m_bufPClS6010[m_curS6010];

		WriteConfig ();
	}
}

BOOL CSOLO6010AppDlg::OpenS6010 (int idxS6010)
{
	m_bufPClS6010[idxS6010] = new CSOLO6010(idxS6010);
	if (m_bufPClS6010[idxS6010]->GENERAL_Open () == TRUE)
	{	// SOLO6010 open ok
		glBufGInfo[idxS6010].bOpenS6010 = TRUE;
	}
	else
	{	// SOLO6010 open fail
		delete m_bufPClS6010[idxS6010];
		m_bufPClS6010[idxS6010] = NULL;
		glBufGInfo[idxS6010].bOpenS6010 = FALSE;
		return FALSE;
	}

	return TRUE;
}

BOOL CSOLO6010AppDlg::InitS6010 (int idxS6010)
{
	int i, j, nTmp;

	if (glBufGInfo[idxS6010].bOpenS6010 == FALSE)
	{
		glBufGInfo[idxS6010].bInitS6010 = FALSE;
		return FALSE;
	}
	
	// Solo6010 Init[S]
	if (m_bufPClS6010[idxS6010]->GENERAL_Init (glBufGInfo[idxS6010].typeVideo) == FALSE)
	{
		glBufGInfo[idxS6010].bInitS6010 = FALSE;
		return FALSE;
	}
	// Solo6010 Init[E]

	// Mosaic Init[S]
	m_bufPClS6010[idxS6010]->MOSAIC_SetStrength (glBufGInfo[idxS6010].MOSAIC_nStrength);
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		m_bufPClS6010[idxS6010]->MOSAIC_SetOnOffAndRect (i, glBufGInfo[idxS6010].MOSAIC_bufBSet[i],
			&glBufGInfo[idxS6010].MOSAIC_bufRect[i]);
	}
	// Mosaic Init[E]

	// Video Motion Init[S]
	m_bufPClS6010[idxS6010]->VMOTION_SetMinDetectThreshold (glBufGInfo[idxS6010].VM_minCntTh);
	m_bufPClS6010[idxS6010]->VMOTION_SetGridOnOff (glBufGInfo[idxS6010].VM_bShowGrid);

	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		m_bufPClS6010[idxS6010]->VMOTION_SetOnOff (i, glBufGInfo[idxS6010].VM_bufBUse[i]);
		m_bufPClS6010[idxS6010]->VMOTION_SetThreshold (i, glBufGInfo[idxS6010].VM_bbufTh[i]);

		m_bufPClS6010[idxS6010]->VMOTION_SetBorderColor (
			glBufGInfo[idxS6010].VM_clrModeBorderY, glBufGInfo[idxS6010].VM_clrValBorderY,
			glBufGInfo[idxS6010].VM_clrModeBorderCb, glBufGInfo[idxS6010].VM_clrValBorderCb,
			glBufGInfo[idxS6010].VM_clrModeBorderCr, glBufGInfo[idxS6010].VM_clrValBorderCr);
		m_bufPClS6010[idxS6010]->VMOTION_SetBarColor (
			glBufGInfo[idxS6010].VM_clrModeBarY, glBufGInfo[idxS6010].VM_clrValBarY,
			glBufGInfo[idxS6010].VM_clrModeBarCb, glBufGInfo[idxS6010].VM_clrValBarCb,
			glBufGInfo[idxS6010].VM_clrModeBarCr, glBufGInfo[idxS6010].VM_clrValBarCr);
	}
	// Video Motion Init[E]

	// Camera Block Detection Init[S]
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		m_bufPClS6010[idxS6010]->CAM_BLK_DET_SetOnOff (i, glBufGInfo[idxS6010].CBD_bufBUseCBD[i]);
		m_bufPClS6010[idxS6010]->CAM_BLK_DET_SetAlarmLevel (i, glBufGInfo[idxS6010].CBD_bufNumAlarmLevel[i]);
		m_bufPClS6010[idxS6010]->CAM_BLK_DET_SetCheckInterval (glBufGInfo[idxS6010].CBD_timeCheckInterval);
	}
	// Camera Block Detection Init[E]

	// Advanced Motion Detection Init[S]
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		nTmp = glBufGInfo[idxS6010].ADVMD_bufNumMotionCheck[i];

		m_bufPClS6010[idxS6010]->ADV_MOT_SetNumMotChkItem (i, nTmp);
		m_bufPClS6010[idxS6010]->ADV_MOT_SetMinDetectThreshold (i, glBufGInfo[idxS6010].ADVMD_bufCntMinDet[i]);

		for (j=0; j<nTmp; j++)
		{
			m_bufPClS6010[idxS6010]->ADV_MOT_SetMotChkDist (i, j, glBufGInfo[idxS6010].ADVMD_bbufMotDataDist[i][j]);
			m_bufPClS6010[idxS6010]->ADV_MOT_SetThreshold (i, j, glBufGInfo[idxS6010].ADVMD_bbbufMotionTh[i][j]);
		}
	}

	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		m_bufPClS6010[idxS6010]->ADV_MOT_SetOnOff (i, glBufGInfo[idxS6010].ADVMD_bufBUse[i]);
	}
	// Advanced Motion Detection Init[E]

	// Video Matrix Init[S]
	m_bufPClS6010[idxS6010]->VMATRIX_Set (glBufGInfo[idxS6010].bufIdxCamToWndCh);
	// Video Matrix Init[E]

	// Record Manager Init[S]
	m_bufClRecManager[idxS6010].Init (m_bufPClS6010[idxS6010],
		glBufGInfo[idxS6010].setRec.modeRecMP4, glBufGInfo[idxS6010].setRec.modeRecJPEG,
		glBufGInfo[idxS6010].setRec.modeRecG723, glBufGInfo[idxS6010].setRec.nNotifyBufLevelMP4,
		glBufGInfo[idxS6010].setRec.nNotifyBufLevelJPEG, glBufGInfo[idxS6010].setRec.nNotifyBufLevelG723);
	// Record Manager Init[E]

	// Hardware Decoder Manager Init[S]
	m_bufClHWDecManager[idxS6010].InitPlayer (m_bufPClS6010[idxS6010]);
	// Hardware Decoder Manager Init[E]

	// GPIO Init[S]
	DWORD maskPort_ModeIn, maskPort_ModeOut, maskPort_ModeI2C, maskPort_ModeETC;

	maskPort_ModeIn = 0;
	maskPort_ModeOut = 0;
	maskPort_ModeI2C = 0;
	maskPort_ModeETC = 0;
	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		if (glBufGInfo[idxS6010].GPIO_bufMode[i] == GPIO_MODE_IN_OR_INT)
		{
			maskPort_ModeIn |= GPIO_PORT(i);
		}
		else if (glBufGInfo[idxS6010].GPIO_bufMode[i] == GPIO_MODE_OUT)
		{
			maskPort_ModeOut |= GPIO_PORT(i);
		}
		else if (glBufGInfo[idxS6010].GPIO_bufMode[i] == GPIO_MODE_I2C)
		{
			maskPort_ModeI2C |= GPIO_PORT(i);
		}
		else if (glBufGInfo[idxS6010].GPIO_bufMode[i] == GPIO_MODE_ETC)
		{
			maskPort_ModeETC |= GPIO_PORT(i);
		}
	}
	m_bufPClS6010[idxS6010]->GPIO_EnableInterrupt (GPIO_ALL_PORT, GPIO_INT_CFG_DISABLE_INTERRUPT);

	m_bufPClS6010[idxS6010]->GPIO_SetMode (GPIO_MODE_IN_OR_INT, maskPort_ModeIn);
	m_bufPClS6010[idxS6010]->GPIO_SetMode (GPIO_MODE_OUT, maskPort_ModeOut);
	m_bufPClS6010[idxS6010]->GPIO_SetMode (GPIO_MODE_I2C, maskPort_ModeI2C);
	m_bufPClS6010[idxS6010]->GPIO_SetMode (GPIO_MODE_ETC, maskPort_ModeETC);

	DWORD maskPort_IntRising, maskPort_IntFalling, maskPort_IntBoth, maskPort_IntLowLevel;

	maskPort_IntRising = 0;
	maskPort_IntFalling = 0;
	maskPort_IntBoth = 0;
	maskPort_IntLowLevel = 0;

	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		if (glBufGInfo[idxS6010].GPIO_bufIntCfg[i] == GPIO_INT_CFG_RISING_EDGE)
		{
			maskPort_IntRising |= GPIO_PORT(i);
		}
		else if (glBufGInfo[idxS6010].GPIO_bufIntCfg[i] == GPIO_INT_CFG_FALLING_EDGE)
		{
			maskPort_IntFalling |= GPIO_PORT(i);
		}
		else if (glBufGInfo[idxS6010].GPIO_bufIntCfg[i] == GPIO_INT_CFG_BOTH_EDGE)
		{
			maskPort_IntBoth |= GPIO_PORT(i);
		}
		else if (glBufGInfo[idxS6010].GPIO_bufIntCfg[i] == GPIO_INT_CFG_LOW_LEVEL)
		{
			maskPort_IntLowLevel |= GPIO_PORT(i);
		}
	}
	m_bufPClS6010[idxS6010]->GPIO_EnableInterrupt (maskPort_IntRising, GPIO_INT_CFG_RISING_EDGE);
	m_bufPClS6010[idxS6010]->GPIO_EnableInterrupt (maskPort_IntFalling, GPIO_INT_CFG_FALLING_EDGE);
	m_bufPClS6010[idxS6010]->GPIO_EnableInterrupt (maskPort_IntBoth, GPIO_INT_CFG_BOTH_EDGE);
	m_bufPClS6010[idxS6010]->GPIO_EnableInterrupt (maskPort_IntLowLevel, GPIO_INT_CFG_LOW_LEVEL);
	// GPIO Init[S]

	glBufGInfo[idxS6010].bInitS6010 = TRUE;	// Initialize OK

	return TRUE;
}

void CSOLO6010AppDlg::EnableCtrlFromS6010Stat ()
{
	if (glBufGInfo[m_curS6010].bOpenS6010 == TRUE &&
		glBufGInfo[m_curS6010].bInitS6010 == TRUE)
	{
		GetDlgItem (IDC_BUT_LIVE)->EnableWindow (TRUE);
		GetDlgItem (IDC_BUT_RECORD)->EnableWindow (TRUE);
		GetDlgItem (IDC_BUT_CTRL_GPIO)->EnableWindow (TRUE);
	}
	else
	{
		GetDlgItem (IDC_BUT_LIVE)->EnableWindow (FALSE);
		GetDlgItem (IDC_BUT_RECORD)->EnableWindow (FALSE);
		GetDlgItem (IDC_BUT_CTRL_GPIO)->EnableWindow (FALSE);
	}
}

void CSOLO6010AppDlg::CloseS6010 (int idxS6010)
{
	if (glBufGInfo[idxS6010].bOnRec == TRUE)
	{
		OnButRecord ();
	}
	if (glBufGInfo[idxS6010].bOnLive == TRUE)
	{
		OnButLive ();
	}

	m_bufClHWDecManager[idxS6010].DestroyPlayer ();

	if (m_bufPClS6010[idxS6010] != NULL)
	{
		m_bufPClS6010[idxS6010]->GENERAL_Close ();

		delete m_bufPClS6010[idxS6010];
		m_bufPClS6010[idxS6010] = NULL;
	}

	glBufGInfo[idxS6010].bInitS6010 = FALSE;
	glBufGInfo[idxS6010].bOpenS6010 = FALSE;
}

LRESULT CSOLO6010AppDlg::OnGetMsgAppDlgCtrl (WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDX_APP_MSG_ON_BUT_LIVE:
		ToggleLive ();
		break;
	case IDX_APP_MSG_ON_BUT_PLAYER:
		OnButPlayer ();
		break;
	case IDX_APP_MSG_ON_BUT_RECORD:
		OnButRecord ();
		break;
	}

	return 0;
}

void CSOLO6010AppDlg::OnSelchangeComCurS6010() 
{
	// TODO: Add your control notification handler code here
	
	m_curS6010 = m_comCurS6010.GetCurSel ();

	if (glBufGInfo[m_curS6010].bInitS6010 == FALSE)
	{
		AfxMessageBox ("This card is not working. Operation with this card may cause system crash.");
	}
	
	glPCurGInfo = &glBufGInfo[m_curS6010];
	glPCurSelS6010 = m_bufPClS6010[m_curS6010];
	glPCurRecManager = &m_bufClRecManager[m_curS6010];
	glPCurHWDecManager = &m_bufClHWDecManager[m_curS6010];
}

void CSOLO6010AppDlg::OnButGpio() 
{
	// TODO: Add your control notification handler code here
	
	CDlgCtrlGPIO dlg;
	dlg.DoModal ();
}

void CSOLO6010AppDlg::WriteConfig ()
{
	FILE *fp;
	char strConfigFilePath[MAX_PATH];

	// Save current global info[S]
	if (m_strCurDir[strlen (m_strCurDir) -1] == '\\')
	{
		sprintf (strConfigFilePath, "%s%s", m_strCurDir, STR_G_INFO_FILE_NAME);
	}
	else
	{
		sprintf (strConfigFilePath, "%s\\%s", m_strCurDir, STR_G_INFO_FILE_NAME);
	}

	fp = fopen (strConfigFilePath, "wb");
	fwrite (glBufGInfo, 1, sizeof(INFO_APP_GLOBAL) *NUM_MAX_S6010, fp);
	fclose (fp);
	// Save current global info[E]
}


void CSOLO6010AppDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	int i;

	// SOLO6010 destroy[S]
	for (i=0; i<m_numS6010; i++)
	{
		CloseS6010 (i);
	}
	// SOLO6010 destroy[E]

	CDialog::OnOK();
}

void CSOLO6010AppDlg::OnCancel() 
{
	// TODO: Add extra validation here
	
	OnOK ();
}

void CSOLO6010AppDlg::OnButCtrlRegister() 
{
	// TODO: Add your control notification handler code here
	
	CDlgCtrlRegister dlg;
	dlg.DoModal ();
}

void CSOLO6010AppDlg::OnButCtrlUart() 
{
	// TODO: Add your control notification handler code here
	
	CDlgCtrlUART dlg;
	dlg.DoModal ();
}

void CSOLO6010AppDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	// SOLO6010 destroy[S]
	int i;
	
	for (i=0; i<m_numS6010; i++)
	{
		CloseS6010 (i);
	}
	// SOLO6010 destroy[E]

	CDialog::OnClose();
}

void CSOLO6010AppDlg::OnButTest() 
{
	// TODO: Add your control notification handler code here

	CDlgTest dlg;
	dlg.DoModal ();
}

void CSOLO6010AppDlg::OnButCtrlI2c() 
{
	// TODO: Add your control notification handler code here
	
	CDlgCtrlI2C dlg;
	dlg.DoModal ();
}
