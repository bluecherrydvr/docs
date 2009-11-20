// DvrFCDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DvrFCD.h"
#include "DvrFCDDlg.h"
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define THREADKILL_TO			5000
#define THREADEXIT_SUCCESS		0x1234

void av_frame_callback(sdvr_chan_handle_t handle,
					   sdvr_frame_type_e frame_type, sx_bool isPrimary);

void video_alarm_callback(sdvr_chan_handle_t handle,
						  sdvr_video_alarm_e alarm_type, sx_uint32 data);

void sensors_callback(sx_uint32 board_index, sx_uint32 sensors_map);

void signals_callback(sx_uint32 board_index, sdvr_signal_info_t * signal_info);

CDvrFCDDlg* g_pMainDlg;

// CDvrFCDDlg dialog




CDvrFCDDlg::CDvrFCDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDvrFCDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	int i=0, j=0;
	m_pMenu = NULL;	
	
	int iBoardIndex=0;

	m_iBoardCount = 0;	

	g_pMainDlg = this;

	for (iBoardIndex=0; iBoardIndex<MAX_SUPPORT_BOARD; iBoardIndex++)
	{
		m_pDVRSystem[iBoardIndex] = NULL;
		m_iVdoCount[iBoardIndex] = 0;
		m_iAdoCount[iBoardIndex] = 0;
		m_bIsInitDvrBoardOK[iBoardIndex] = false;	
		m_bRS485Init[i] = false;
	}

	m_bIsInitAllDvrBoardOK = false;
	m_iRadioBtnBoard = 0;

	for (i=0; i<MAX_EVENT_UI_CONTROL; i++)
	{
		m_hEventUIControl[i] = NULL;		
	}

	
	
	for (i=0; i<MAX_SUPPORT_BOARD; i++)
	{
		for (j=0; j<MAX_BOARD_CHANNEL; j++)
		{
			m_pDirectDrawDisplay[i][j] = NULL;
			m_iVdoWidth_Raw[i][j] = 720;
			m_iVdoHeight_Raw[i][j] = 480;		
			m_bInitDrawOK[i][j] = FALSE;	
			m_pWaveOut[i][j] = NULL;
			m_bOpenWaveOutOK[i][j] = FALSE;
			m_iVdo_YBufSize[i][j] = 0;
			m_iVdo_UBufSize[i][j] = 0;
			m_iVdo_VBufSize[i][j] = 0;	

			m_image_ctrl[i][j].brightness = 100;
			m_image_ctrl[i][j].contrast = 100;
			m_image_ctrl[i][j].hue = 100;
			m_image_ctrl[i][j].saturation = 100;
			m_image_ctrl[i][j].sharpness = 5;
			m_bIsConnectChannelOK[i][j] = false;
		}
	}	

	m_hUIControlThread = NULL;

	m_iEventOpenBoard = m_iEventConnectCh = -1;	
	m_bCanDoCloseProcess = true;		// some API is a blocked-function, avoid suddenly click close.
	m_bProcessIsClosing = false;
	m_bIsUseControlAll = false;
	m_bCreateAndInitialRegionSettingDlgOK = FALSE;
	m_bCreateAndInitialChannelSettingDlgOK = FALSE;
	m_pRegionSettingDlg = NULL;
	m_pChannelSettingDlg = NULL;

	m_iBaudRate = 9600;
	m_iAddress = 1;	
	m_iSpeed = 1;
	m_pRelaysAndSensorsDlg = NULL;	
}

void CDvrFCDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Check(pDX, IDC_RADIO_BOARD1, m_bShowBoard[0]);
	//DDX_Check(pDX, IDC_RADIO_BOARD2, m_bShowBoard[1]);
	//DDX_Check(pDX, IDC_RADIO_BOARD3, m_bShowBoard[2]);
	//DDX_Check(pDX, IDC_RADIO_BOARD4, m_bShowBoard[3]);
	DDX_Control(pDX, IDC_RADIO_BOARD1, m_RadioBtnBoard);
}

BEGIN_MESSAGE_MAP(CDvrFCDDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()	
	ON_MESSAGE(WM_CREATE_BOARD_SETTING_DLG, OnCreateBoardSettingDlg)
	ON_MESSAGE(WM_CREATE_CHANNEL_SETTING_DLG, OnCreateChannelSettingDlg)
	ON_MESSAGE(WM_MODIFY_CHANNEL_SETTING_DLG, OnModifyChannelSettingDlg)
	ON_MESSAGE(WM_CREATE_REGION_SETTING_DLG, OnCreateReginSettingDlg)
	ON_MESSAGE(WM_MD_MODIFY_REGION, OnMD_Modify)
	ON_MESSAGE(WM_MD_ENABLE, OnMD_Enable)
	ON_MESSAGE(WM_PTZ_ACTION, OnPTZ_Action)
	ON_MESSAGE(WM_CREATE_RS_DLG, OnCreateRSDlg)
	ON_MESSAGE(WM_CONFIG_SENSORS, OnConfigSensors)
	ON_MESSAGE(WM_RELAY_TRIGGER, OnRelayTrigger)
	ON_MESSAGE(WM_GET_PROTECT_ID, OnGetProtectID)	
	ON_BN_CLICKED(IDC_RADIO_BOARD1, &CDvrFCDDlg::OnBnClickedRadioBoard1)
	ON_BN_CLICKED(IDC_RADIO_BOARD2, &CDvrFCDDlg::OnBnClickedRadioBoard2)
	ON_BN_CLICKED(IDC_RADIO_BOARD3, &CDvrFCDDlg::OnBnClickedRadioBoard3)
	ON_BN_CLICKED(IDC_RADIO_BOARD4, &CDvrFCDDlg::OnBnClickedRadioBoard4)
END_MESSAGE_MAP()


// CDvrFCDDlg message handlers

BOOL CDvrFCDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));
	int i=0;	

	m_pMenu = NULL;
	m_pMenu = new CMenu();
	m_pMenu->LoadMenuA(IDR_MENU_CONTROL);

	SetMenu(m_pMenu);	// Add the new menu

	CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD, MF_UNCHECKED);
	
	for(i=0; i<MAX_BOARD_CHANNEL; i++)
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECTCHANNEL_01+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECTCHANNEL_01+i, MF_UNCHECKED);	

		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECTCHANNEL_17+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECTCHANNEL_17+i, MF_UNCHECKED);

		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECTCHANNEL_33+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECTCHANNEL_33+i, MF_UNCHECKED);	

		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECTCHANNEL_49+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECTCHANNEL_49+i, MF_UNCHECKED);
	}

	for(i=0; i<MAX_SUPPORT_BOARD; i++)
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_CONNECT_ALL_BOARD_01+i, MF_GRAYED);
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DISCONNECT_ALL_BOARD_01+i, MF_GRAYED);
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_RELAYSSENSORS_BOARD01+i, MF_GRAYED);
	}

	for(i=0 ; i<MAX_SUPPORT_BOARD; i++)
	{
		GetDlgItem(IDC_RADIO_BOARD1 + i)->ShowWindow(SW_HIDE);
	}	

	DWORD dwID=0;
	for (i=0; i<MAX_EVENT_UI_CONTROL; i++)
	{
		m_hEventUIControl[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(m_hEventUIControl[i]);
	}

	m_hUIControlThread = (HANDLE) CreateThread(NULL, 0, Thread_UIControl, this, 0, &dwID);

	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	sdvr_rc = sdvr_sdk_init();	

	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_sdk_init() = SDVR_ERR_NONE.");
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_sdk_init() = %d.", sdvr_rc);
		OutputDebugString(szDebug);
	}

	m_iBoardCount = sdvr_get_board_count();

	if (m_iBoardCount>=1)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_get_board_count() = %d.", m_iBoardCount);
		OutputDebugString(szDebug);	
	}
	else
	{
		if (m_pMenu)
		{
			delete m_pMenu;
			m_pMenu = NULL;
		}

		sdvr_rc = sdvr_sdk_close();

		MessageBox("NO any board exist, program will close!!");
		PostQuitMessage(0);
		return FALSE;
	}

	for(i=0 ; i<m_iBoardCount; i++)
	{
		GetDlgItem(IDC_RADIO_BOARD1 + i)->ShowWindow(SW_SHOW);		
	}

	sdvr_set_av_frame_callback(av_frame_callback);
	sdvr_set_video_alarm_callback(video_alarm_callback);
	sdvr_set_sensor_callback(sensors_callback);
	sdvr_set_signals_callback(signals_callback);

	int j=0;
	for (i=0; i<MAX_SUPPORT_BOARD; i++)
	{
		for (j=0; j<MAX_BOARD_CHANNEL; j++)
		{
			OpenWaveOut(j, i);
		}
	}

	m_RadioBtnBoard.SetCheck(BST_CHECKED);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDvrFCDDlg::OnPaint()
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
HCURSOR CDvrFCDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

sdvr_err_e CDvrFCDDlg::InitBoard(int iBoardIndex)
{		
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	m_pDVRSystem[iBoardIndex] = NULL;
	m_pDVRSystem[iBoardIndex] = new CDVRSystem(this, iBoardIndex);	

	//chuck test
	CString MyStr;
	CHAR szFilters[] = "firmware files (*.rom)|*.rom";
	MyStr.Format("Select firmware(*.rom) for upgrade.\n");
	CFileDialog dlg (TRUE, MyStr, "*.rom",
		OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, szFilters);

	CString strFilePath;

	if (dlg.DoModal () == IDOK)
	{
		strFilePath = dlg.GetPathName();
	}
	else
	{
		sdvr_rc = SDVR_ERR_LOAD_FIRMWARE;
		return sdvr_rc;
	}

	char* pszFWpath = "";

	pszFWpath = strFilePath.GetBuffer();
	strFilePath.ReleaseBuffer();

	if ((sdvr_rc = m_pDVRSystem[iBoardIndex]->UpgradeFW(pszFWpath)) != SDVR_ERR_NONE)
	{
		return sdvr_rc;
	}


	Sleep(50);

	if ((sdvr_rc = m_pDVRSystem[iBoardIndex]->GetBoardParam()) != SDVR_ERR_NONE)
	{
		return sdvr_rc;
	}	

	Sleep(50);

	if ((sdvr_rc = m_pDVRSystem[iBoardIndex]->SetBoardParam()) != SDVR_ERR_NONE)
	{
		return sdvr_rc;
	}

	Sleep(50);

	if ((sdvr_rc = m_pDVRSystem[iBoardIndex]->OpenBoard()) != SDVR_ERR_NONE)
	{
		return sdvr_rc;
	}			

	m_iVdoCount[iBoardIndex] = m_pDVRSystem[iBoardIndex]->m_iVdoCount;
	m_iAdoCount[iBoardIndex] = m_pDVRSystem[iBoardIndex]->m_iAdoCount;
	
	return sdvr_rc;
}


sdvr_err_e CDvrFCDDlg::UnInitBoard(int iBoardIndex)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[200];			
	memset(&szDebug, 0, sizeof(szDebug));
	int i=0;

	if (m_pDVRSystem[iBoardIndex])
	{
		for (i=0; i<MAX_BOARD_CHANNEL; i++)
		{		
			SetChannel(false, i);	
			Sleep(10);
			m_bIsConnectChannelOK[iBoardIndex][i] = false;
		}		

		sdvr_rc = m_pDVRSystem[iBoardIndex]->CloseBoard();

		delete m_pDVRSystem[iBoardIndex];
		m_pDVRSystem[iBoardIndex] = NULL;		
		m_bRS485Init[iBoardIndex] = false;
	}			

	return sdvr_rc;
}


void CDvrFCDDlg::OnClose()
{		
	if (!m_bCanDoCloseProcess)
	{
		return;
	}	

	int i=0, j=0;
	
	m_bProcessIsClosing = true;
	m_bIsUseControlAll = true;

	ShowWindow(SW_HIDE);	

	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[200];	
	memset(&szDebug, 0, sizeof(szDebug));
	
	for (i=0; i<m_iBoardCount; i++)
	{
		if (m_pDVRSystem[i])
		{
			m_iEventOpenBoard = i;
			UnInitBoard(i);
		}
	}

	Sleep(50);

	for (i=0; i<MAX_BOARD_CHANNEL; i++)
	{
		for (j=0; j<MAX_SUPPORT_BOARD; j++)
		{
			CloseDrawDib(i, j);
			CloseWaveOut(i, j);
		}		
	}	

	SetEvent(m_hEventUIControl[ENAME_KILL_UI_CONTROL_THREAD]);

	if (WaitForSingleObject(m_hUIControlThread, THREADKILL_TO) != WAIT_OBJECT_0)
	{
		TerminateThread(m_hUIControlThread, THREADEXIT_SUCCESS);
	}
	CloseHandle(m_hUIControlThread);
	m_hUIControlThread = NULL;	

	for (i=0; i<MAX_EVENT_UI_CONTROL; i++)
	{
		CloseHandle(m_hEventUIControl[i]);
		m_hEventUIControl[i] = NULL;
	}	

	if(m_pMenu)
	{
		delete m_pMenu;
		m_pMenu = NULL;
	}

	sdvr_rc = sdvr_sdk_close();
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_sdk_close() = SDVR_ERR_NONE.");
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_sdk_close() = %d.", sdvr_rc);
		OutputDebugString(szDebug);
	}

	CDialog::OnClose();
}


DWORD WINAPI CDvrFCDDlg::Thread_UIControl(LPVOID pParam)
{
	CDvrFCDDlg* pMainDlg = (CDvrFCDDlg*) pParam;
	BOOL bExit = FALSE;

	DWORD	dwResult = 0;	
	int		EventID = 0;
	int      iBoardIndex = 0;
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	int i=0;
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	while (!bExit)
	{
		dwResult = WaitForMultipleObjects(MAX_EVENT_UI_CONTROL, pMainDlg->m_hEventUIControl, FALSE, 15);	
		if((dwResult >= WAIT_OBJECT_0) && (dwResult < (WAIT_OBJECT_0 + MAX_EVENT_UI_CONTROL)))
		{
			EventID = dwResult-WAIT_OBJECT_0;

			if (EventID == ENAME_INIT_BOARD)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_INIT_BOARD]);	

				char szDebug[100];

				int iBoardCount ;

				for(iBoardCount =0 ; iBoardCount < pMainDlg->m_iBoardCount ; iBoardCount++)
				{
					pMainDlg->SetInitDvrMenuState(false, false, iBoardCount);
					pMainDlg->SetConnectChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1 , -1);

					sprintf_s(szDebug, sizeof(szDebug), 
						"board(%d) is initialing.....", iBoardCount);
					pMainDlg->SetWindowText(szDebug);

					if ((sdvr_rc = pMainDlg->InitBoard(iBoardCount)) == SDVR_ERR_NONE)
					{
						pMainDlg->m_bIsInitDvrBoardOK[iBoardCount] = true;

						sprintf_s(szDebug, sizeof(szDebug), 
							"board(%d) initial OK.....", iBoardCount);
						pMainDlg->SetWindowText(szDebug);
					}				
					else
					{
						sdvr_rc = pMainDlg->UnInitBoard(iBoardCount);										
						if(sdvr_rc == SDVR_ERR_NONE)
						{
							pMainDlg->m_bIsInitDvrBoardOK[iBoardCount] = false;

							sprintf_s(szDebug, sizeof(szDebug), 
								"board(%d) initial fail.....", iBoardCount);
							pMainDlg->SetWindowText(szDebug);
						}						
					}
				}

				for(iBoardCount =0 ; iBoardCount < pMainDlg->m_iBoardCount ; iBoardCount++)
				{
					if(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount] == true)
					{

					}
					else
					{
						sprintf_s(szDebug, sizeof(szDebug),"All board initial fail.....");
						pMainDlg->SetWindowText(szDebug);
						pMainDlg->SetInitDvrMenuState(true, false, iBoardCount);
						break;
					}

					if(iBoardCount == pMainDlg->m_iBoardCount-1 )
					{
						int i;
						for(i = 0 ; i< pMainDlg->m_iBoardCount ;i++)
						{
							pMainDlg->m_pDVRSystem[i]->SetFontTable();
							pMainDlg->SetConnectChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1 , i);

							sprintf_s(szDebug, sizeof(szDebug),"All board SetOSDFontTable OK!");
							pMainDlg->SetWindowText(szDebug);

							pMainDlg->SetInitDvrMenuState(true, true, i);
						}
						
						pMainDlg->m_bIsInitAllDvrBoardOK = true;
						sprintf_s(szDebug, sizeof(szDebug),"All board Connect OK!");
						pMainDlg->SetWindowText(szDebug);
					}
				}					
			}
			else if (EventID == ENAME_UNINIT_BOARD)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_UNINIT_BOARD]);

				int iBoardCount = 0;
				int iOriEventOpenBoard = pMainDlg->m_iEventOpenBoard;

				for(iBoardCount =0 ; iBoardCount < pMainDlg->m_iBoardCount ; iBoardCount++)
				{
					pMainDlg->SetInitDvrMenuState(false, false, iBoardCount);	
					pMainDlg->SetConnectChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1, -1);

					pMainDlg->m_iEventOpenBoard = iBoardCount;
					if ((sdvr_rc = pMainDlg->UnInitBoard(iBoardCount)) == SDVR_ERR_NONE)
					{
						pMainDlg->m_bIsInitDvrBoardOK[iBoardCount] = false;					
					}
					else
					{
						pMainDlg->m_bIsInitDvrBoardOK[iBoardCount] = true;
					}

					pMainDlg->SetInitDvrMenuState(true, pMainDlg->m_bIsInitDvrBoardOK[iBoardCount] ,iBoardCount);
					pMainDlg->SetConnectChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1, iBoardCount);
					pMainDlg->m_bIsInitAllDvrBoardOK = false;
				}	
				
				pMainDlg->m_iEventOpenBoard = iOriEventOpenBoard;
			}
			else if (EventID == ENAME_CONNECT_CHANNEL)
			{				
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_CONNECT_CHANNEL]);

				pMainDlg->m_bCanDoCloseProcess = false;

				pMainDlg->SetConnectChMenuState(false, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][pMainDlg->m_iEventConnectCh], pMainDlg->m_iEventConnectCh , pMainDlg->m_iEventOpenBoard);				

				if (pMainDlg->SetChannel(true, pMainDlg->m_iEventConnectCh))
				{
					pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][pMainDlg->m_iEventConnectCh % MAX_BOARD_CHANNEL] = true;
				}
				else
				{
					pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][pMainDlg->m_iEventConnectCh % MAX_BOARD_CHANNEL] = false;
				}		
				pMainDlg->SetConnectChMenuState(true, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][pMainDlg->m_iEventConnectCh % MAX_BOARD_CHANNEL], pMainDlg->m_iEventConnectCh , pMainDlg->m_iEventOpenBoard);				

				pMainDlg->m_bCanDoCloseProcess = true;
			}
			else if (EventID == ENAME_DISCONNECT_CHANNEL)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_DISCONNECT_CHANNEL]);
				
				pMainDlg->m_bCanDoCloseProcess = false;

				pMainDlg->SetConnectChMenuState(false, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][pMainDlg->m_iEventConnectCh % MAX_BOARD_CHANNEL], pMainDlg->m_iEventConnectCh , pMainDlg->m_iEventOpenBoard);					

				if (pMainDlg->SetChannel(false, pMainDlg->m_iEventConnectCh))	// disconnect ok.
				{
					pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][pMainDlg->m_iEventConnectCh % MAX_BOARD_CHANNEL] = false;					
				}
				else	// disconnect fail.
				{
					pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][pMainDlg->m_iEventConnectCh % MAX_BOARD_CHANNEL] = true;
				}
				pMainDlg->SetConnectChMenuState(true, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][pMainDlg->m_iEventConnectCh % MAX_BOARD_CHANNEL], pMainDlg->m_iEventConnectCh , pMainDlg->m_iEventOpenBoard);				

				pMainDlg->m_bCanDoCloseProcess = true;
			}
			else if (EventID == ENAME_CONNECT_ALL)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_CONNECT_ALL]);	

				pMainDlg->m_bCanDoCloseProcess = false;

				for (i=0; i<MAX_BOARD_CHANNEL; i++)
				{																		
					pMainDlg->SetConnectChMenuState(false, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i], i , pMainDlg->m_iEventOpenBoard);	
				}				
				pMainDlg->m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_01, MF_GRAYED);
				pMainDlg->m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_01, MF_GRAYED);	
				pMainDlg->m_pMenu->EnableMenuItem(ID_INIT_ALL_DVRBOARD, MF_GRAYED);
								
				for (i=0; i<pMainDlg->m_iBoardCount; i++)
				{
					pMainDlg->m_pMenu->EnableMenuItem(ID_RELAYSSENSORS_BOARD01+i, MF_GRAYED);
				}

				for (i=0; i<MAX_BOARD_CHANNEL; i++)
				{
					if (!pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i])
					{
						if (pMainDlg->SetChannel(true, i))
						{
							pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i] = true;
						}
						else
						{
							pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i] = false;
						}								
						Sleep(10);
					}
				}

				for (i=0; i<MAX_BOARD_CHANNEL; i++)
				{																	
					pMainDlg->SetConnectChMenuState(true, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i], i , pMainDlg->m_iEventOpenBoard);	
				}
				pMainDlg->m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_01, MF_ENABLED);
				pMainDlg->m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_01, MF_ENABLED);	
				pMainDlg->m_pMenu->EnableMenuItem(ID_INIT_ALL_DVRBOARD, MF_ENABLED);

				for (i=0; i<pMainDlg->m_iBoardCount; i++)
				{
					pMainDlg->m_pMenu->EnableMenuItem(ID_RELAYSSENSORS_BOARD01+i, MF_ENABLED);
				}
				
				pMainDlg->m_bIsUseControlAll = false;
				pMainDlg->m_bCanDoCloseProcess = true;
			}
			else if (EventID == ENAME_DISCONNECT_ALL)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_DISCONNECT_ALL]);	

				pMainDlg->m_bCanDoCloseProcess = false;
	
				for (i=0 ;i<MAX_BOARD_CHANNEL; i++)
				{					
					pMainDlg->SetConnectChMenuState(false, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i], i , pMainDlg->m_iEventOpenBoard);														
				}
				pMainDlg->m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_01, MF_GRAYED);
				pMainDlg->m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_01, MF_GRAYED);
				pMainDlg->m_pMenu->EnableMenuItem(ID_INIT_ALL_DVRBOARD, MF_GRAYED);
				pMainDlg->m_pMenu->EnableMenuItem(ID_RELAYSSENSORS_BOARD01, MF_GRAYED);

				for (i=0; i<MAX_BOARD_CHANNEL; i++)
				{
					if (pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i])
					{
						if (pMainDlg->SetChannel(false, i))	// disconnect ok.
						{
							pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i] = false;					
						}
						else	// disconnect fail.
						{
							pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i] = true;
						}					
						Sleep(10);
					}
				}

				for (i=0 ;i<MAX_BOARD_CHANNEL; i++)
				{			
					pMainDlg->SetConnectChMenuState(true, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventOpenBoard][i], i , pMainDlg->m_iEventOpenBoard);	
				}
				pMainDlg->m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_01, MF_ENABLED);
				pMainDlg->m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_01, MF_ENABLED);
				pMainDlg->m_pMenu->EnableMenuItem(ID_INIT_ALL_DVRBOARD, MF_ENABLED);
				pMainDlg->m_pMenu->EnableMenuItem(ID_RELAYSSENSORS_BOARD01, MF_ENABLED);

				pMainDlg->m_bIsUseControlAll = false;
				pMainDlg->m_bCanDoCloseProcess = true;
			}
			else if (EventID == ENAME_KILL_UI_CONTROL_THREAD)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_KILL_UI_CONTROL_THREAD]);				
				bExit = TRUE;
				break;
			}
		}
	}

	return 1;
}


bool CDvrFCDDlg::SetChannel(bool bConnect, int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	if(iCh > 15)
		iCh = iCh % MAX_BOARD_CHANNEL;

	if (bConnect)
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{						
			//CloseDrawDib(iCh);	
			
			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->ConnectChannel(iCh);	
			if (sdvr_rc == SDVR_ERR_NONE)
			{
				CloseDrawDib(iCh, m_iEventOpenBoard);	
				InitDrawDib(iCh , m_iEventOpenBoard);						
				sdvr_rc =  sdvr_set_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, 
					                              SDVR_ICFLAG_ALL, &m_image_ctrl[m_iEventOpenBoard][iCh]);			
			}
			else
			{
				m_pDVRSystem[m_iEventOpenBoard]->DisConnectChannel(iCh);	
			}
		}
	}
	else
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{			
			sdvr_rc = ResetMotion(iCh);
			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DisConnectChannel(iCh);
			Sleep(20);
			//CloseDrawDib(iCh);			
		}

		m_image_ctrl[m_iEventOpenBoard][iCh].brightness = 100;
		m_image_ctrl[m_iEventOpenBoard][iCh].contrast = 100;
		m_image_ctrl[m_iEventOpenBoard][iCh].hue = 100;
		m_image_ctrl[m_iEventOpenBoard][iCh].saturation = 100;
		m_image_ctrl[m_iEventOpenBoard][iCh].sharpness = 5;
	}

	GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->Invalidate();

	if (sdvr_rc != SDVR_ERR_NONE)
	{
		CloseDrawDib(iCh, m_iEventOpenBoard);		
		return false;
	}


	return true;
}


bool CDvrFCDDlg::SetInitDvrMenuState(bool bShow, bool bCheck , int iBoard)
{
	if (bShow)
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD, MF_ENABLED);	
	}
	else
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD, MF_GRAYED);		
	}

	if (bCheck)
	{
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD, MF_CHECKED);	
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_RELAYSSENSORS_BOARD01 + iBoard, MF_ENABLED);
	}
	else
	{
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD, MF_UNCHECKED);	
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_RELAYSSENSORS_BOARD01 + iBoard, MF_GRAYED);
	}	

	return true;
}


bool CDvrFCDDlg::SetConnectChMenuState(bool bShow, bool bCheck, int iCh, int iBoard)
{
	int i=0;

	if(iCh > 15)
		iCh = iCh % MAX_BOARD_CHANNEL ;	

	if (iCh == -1)
	{
		if (bShow)
		{
			for (i=0; i<MAX_BOARD_CHANNEL; i++)
			{
				if(iBoard ==0)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_01+i, MF_ENABLED);
						m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_01+i, MF_UNCHECKED);

						m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_01, MF_ENABLED);
						m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_01, MF_ENABLED);
					}
				}
				if(iBoard ==1)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_17+i, MF_ENABLED);
						m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_17+i, MF_UNCHECKED);

						m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_02, MF_ENABLED);
						m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_02, MF_ENABLED);
					}	
				}			
				if(iBoard ==2)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_33+i, MF_ENABLED);
						m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_33+i, MF_UNCHECKED);

						m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_03, MF_ENABLED);
						m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_03, MF_ENABLED);
					}
				}
				if(iBoard ==3)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_49+i, MF_ENABLED);
						m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_49+i, MF_UNCHECKED);

						m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_04, MF_ENABLED);
						m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_04, MF_ENABLED);
					}	
				}			
			}			
		}
		else
		{
			for (i=0; i<MAX_BOARD_CHANNEL; i++)
			{
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_01+i, MF_GRAYED);			
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_01+i, MF_UNCHECKED);
					
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_17+i, MF_GRAYED);			
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_17+i, MF_UNCHECKED);

				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_33+i, MF_GRAYED);			
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_33+i, MF_UNCHECKED);
		
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_49+i, MF_GRAYED);			
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_49+i, MF_UNCHECKED);								
			}

			for (i=0; i<MAX_SUPPORT_BOARD; i++)
			{
				m_pMenu->EnableMenuItem(ID_CONNECT_ALL_BOARD_01+i, MF_GRAYED);	
				m_pMenu->EnableMenuItem(ID_DISCONNECT_ALL_BOARD_01+i, MF_GRAYED);
			}		
		}	
	}	
	else
	{
		if (m_pDVRSystem[iBoard] && (iCh>=m_pDVRSystem[iBoard]->m_iVdoCount))
		{
			return true;
		}		

		if(iBoard ==0)
		{
			if (bShow)
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_01+iCh, MF_ENABLED);
			else
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_01+iCh, MF_GRAYED);

			if (bCheck)
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_01+iCh, MF_CHECKED);
			else
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_01+iCh, MF_UNCHECKED);			
		}
		if(iBoard ==1)
		{
			if (bShow)
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_17+iCh, MF_ENABLED);
			else
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_17+iCh, MF_GRAYED);
			
			if (bCheck)
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_17+iCh, MF_CHECKED);
			else
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_17+iCh, MF_UNCHECKED);
		}		
		if(iBoard ==2)
		{
			if (bShow)
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_33+iCh, MF_ENABLED);
			else
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_33+iCh, MF_GRAYED);

			if (bCheck)
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_33+iCh, MF_CHECKED);
			else
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_33+iCh, MF_UNCHECKED);			
		}
		if(iBoard ==3)
		{
			if (bShow)
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_49+iCh, MF_ENABLED);
			else
				m_pMenu->EnableMenuItem(ID_CONNECTCHANNEL_49+iCh, MF_GRAYED);

			if (bCheck)
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_49+iCh, MF_CHECKED);
			else
				m_pMenu->CheckMenuItem(ID_CONNECTCHANNEL_49+iCh, MF_UNCHECKED);					
		}
		
	}

	return true;
}


BOOL CDvrFCDDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{	
	if (wParam == ID_INIT_ALL_DVRBOARD)
	{
		if(!m_bIsInitAllDvrBoardOK)
		{
			SetEvent(m_hEventUIControl[ENAME_INIT_BOARD]);
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_UNINIT_BOARD]);
		}
	}
	else if ((wParam>=ID_CONNECTCHANNEL_01) && (wParam<=ID_CONNECTCHANNEL_16))
	{
		m_iEventOpenBoard = 0 ;
		m_iEventConnectCh = (int)(wParam - ID_CONNECTCHANNEL_01);
		if(!m_bIsConnectChannelOK[m_iEventOpenBoard][m_iEventConnectCh % MAX_BOARD_CHANNEL])
		{			
			SetEvent(m_hEventUIControl[ENAME_CONNECT_CHANNEL]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_DISCONNECT_CHANNEL]);
		}	
	}
	else if ((wParam>=ID_CONNECTCHANNEL_17) && (wParam<=ID_CONNECTCHANNEL_32))
	{
		m_iEventOpenBoard = 1 ;
		m_iEventConnectCh = (int)(wParam - ID_CONNECTCHANNEL_01);
		if(!m_bIsConnectChannelOK[m_iEventOpenBoard][m_iEventConnectCh % MAX_BOARD_CHANNEL])
		{			
			SetEvent(m_hEventUIControl[ENAME_CONNECT_CHANNEL]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_DISCONNECT_CHANNEL]);
		}	
	}
	else if ((wParam>=ID_CONNECTCHANNEL_33) && (wParam<=ID_CONNECTCHANNEL_48))
	{
		m_iEventOpenBoard = 2 ;
		m_iEventConnectCh = (int)(wParam - ID_CONNECTCHANNEL_01);
		if(!m_bIsConnectChannelOK[m_iEventOpenBoard][m_iEventConnectCh % MAX_BOARD_CHANNEL])
		{			
			SetEvent(m_hEventUIControl[ENAME_CONNECT_CHANNEL]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_DISCONNECT_CHANNEL]);
		}	
	}
	else if ((wParam>=ID_CONNECTCHANNEL_49) && (wParam<=ID_CONNECTCHANNEL_64))
	{
		m_iEventOpenBoard = 3 ;
		m_iEventConnectCh = (int)(wParam - ID_CONNECTCHANNEL_01);
		if(!m_bIsConnectChannelOK[m_iEventOpenBoard][m_iEventConnectCh % MAX_BOARD_CHANNEL])
		{			
			SetEvent(m_hEventUIControl[ENAME_CONNECT_CHANNEL]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_DISCONNECT_CHANNEL]);
		}	
	}
	else if (wParam == ID_CONNECT_ALL_BOARD_01)
	{
		m_iEventOpenBoard = 0 ;
		m_bIsUseControlAll = true;
		SetEvent(m_hEventUIControl[ENAME_CONNECT_ALL]);
	}
	else if (wParam == ID_DISCONNECT_ALL_BOARD_01)
	{
		m_iEventOpenBoard = 0 ;
		m_bIsUseControlAll = true;
		SetEvent(m_hEventUIControl[ENAME_DISCONNECT_ALL]);
	}
	else if (wParam == ID_CONNECT_ALL_BOARD_02)
	{
		m_iEventOpenBoard = 1 ;
		m_bIsUseControlAll = true;
		SetEvent(m_hEventUIControl[ENAME_CONNECT_ALL]);
	}
	else if (wParam == ID_DISCONNECT_ALL_BOARD_02)
	{
		m_iEventOpenBoard = 1 ;
		m_bIsUseControlAll = true;
		SetEvent(m_hEventUIControl[ENAME_DISCONNECT_ALL]);
	}
	else if (wParam == ID_CONNECT_ALL_BOARD_03)
	{
		m_iEventOpenBoard = 2 ;
		m_bIsUseControlAll = true;
		SetEvent(m_hEventUIControl[ENAME_CONNECT_ALL]);
	}
	else if (wParam == ID_DISCONNECT_ALL_BOARD_03)
	{
		m_iEventOpenBoard = 2 ;
		m_bIsUseControlAll = true;
		SetEvent(m_hEventUIControl[ENAME_DISCONNECT_ALL]);
	}
	else if (wParam == ID_DISCONNECT_ALL_BOARD_04)
	{
		m_iEventOpenBoard = 3 ;
		m_bIsUseControlAll = true;
		SetEvent(m_hEventUIControl[ENAME_DISCONNECT_ALL]);
	}
	else if (wParam == ID_DISCONNECT_ALL_BOARD_04)
	{
		m_iEventOpenBoard = 3 ;
		m_bIsUseControlAll = true;
		SetEvent(m_hEventUIControl[ENAME_DISCONNECT_ALL]);
	}
	else if ((wParam >= ID_RELAYSSENSORS_BOARD01)&&(wParam <= ID_RELAYSSENSORS_BOARD04))
	{
		m_iEventOpenBoard = (int)(wParam - ID_RELAYSSENSORS_BOARD01);
		SendMessage(WM_CREATE_RS_DLG, NULL, 0);
	}
	else if ((wParam >= ID_GETPROTECTID_DVRBOARD01)&&(wParam <= ID_GETPROTECTID_DVRBOARD04))
	{
		int iGetID = (int)(wParam - ID_GETPROTECTID_DVRBOARD01);
		SendMessage(WM_GET_PROTECT_ID, iGetID, 0);
	}

	return CDialog::OnCommand(wParam, lParam);
}


BOOL CDvrFCDDlg::InitDrawDib(int iCh , int iBoard)
{
	m_pDirectDrawDisplay[iBoard][iCh] = NULL;
	m_pDirectDrawDisplay[iBoard][iCh] = new CDirectDraw();
	CWnd* cWndIDisplayItem = GetDlgItem(IDC_STATIC_DISPLAY_01+iCh);

	TRACE("InitDrawDib(%d) width = %d, height = %d\n", iCh+1, m_iVdoWidth_Raw[iBoard][iCh], m_iVdoHeight_Raw[iBoard][iCh]);

	m_bInitDrawOK[iBoard][iCh] = m_pDirectDrawDisplay[iBoard][iCh]->InitDirectDraw(m_iVdoWidth_Raw[iBoard][iCh],
								m_iVdoHeight_Raw[iBoard][iCh], cWndIDisplayItem);

	return m_bInitDrawOK[iBoard][iCh];	
}

BOOL CDvrFCDDlg::CloseDrawDib(int iCh, int iBoard)
{
	m_bInitDrawOK[iBoard][iCh] = FALSE;

	if(m_pDirectDrawDisplay[iBoard][iCh])
	{	
		m_pDirectDrawDisplay[iBoard][iCh]->CloseDirectDraw();

		delete m_pDirectDrawDisplay[iBoard][iCh];
		m_pDirectDrawDisplay[iBoard][iCh] = NULL;
	}

	return TRUE;
}


void CDvrFCDDlg::DrawVideoYUV(int iCh, char * y, char * u, char *v, int iBoard)
{
	CWnd* cWndIDisplayItem = GetDlgItem(IDC_STATIC_DISPLAY_01+iCh);

	if (m_pDirectDrawDisplay[iBoard][iCh] && m_bInitDrawOK[iBoard][iCh])	
		m_pDirectDrawDisplay[iBoard][iCh]->DrawYUV(y, u, v, m_iVdoWidth_Raw[iBoard][iCh], m_iVdoHeight_Raw[iBoard][iCh], cWndIDisplayItem);
}


bool CDvrFCDDlg::acquireBuffer(int iBoard, int iCh, sdvr_frame_type_e frame_type, sx_bool is_primary, sdvr_chan_handle_t handle)
{
	sdvr_av_buffer_t *av_buf;
	sdvr_yuv_buffer_t *yuv_buf;
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	SYSTEMTIME  m_st, stData;
	ULARGE_INTEGER m_ft;

	memset(&m_st, 0, sizeof(SYSTEMTIME));
	memset(&m_ft, 0, sizeof(ULARGE_INTEGER));
	GetLocalTime(&m_st);
	SystemTimeToFileTime(&m_st, (FILETIME*)&m_ft);

	char szDebug[250];	
	memset(&szDebug, 0, sizeof(szDebug));
	
	if (frame_type == SDVR_FRAME_RAW_VIDEO)
	{
		sdvr_rc = sdvr_get_yuv_buffer(handle, &yuv_buf);

		if (sdvr_rc == SDVR_ERR_NONE && yuv_buf)
		{
			sprintf_s(szDebug, sizeof(szDebug), "CH %d come acquireBuffer(), frame_type = %d, is_primary = %d, width = %d, height = %d, size y(%d), u(%d), v(%d), seq number = %d.", 
				iCh+1, yuv_buf->frame_type, is_primary, yuv_buf->active_width, yuv_buf->active_height, yuv_buf->y_data_size, yuv_buf->u_data_size, yuv_buf->v_data_size, yuv_buf->seq_number);
//			OutputDebugString(szDebug);	

			m_iVdo_YBufSize[iBoard][iCh] = yuv_buf->y_data_size;
			m_iVdo_UBufSize[iBoard][iCh] = yuv_buf->u_data_size;
			m_iVdo_VBufSize[iBoard][iCh] = yuv_buf->v_data_size;		

			yuv_buf->timestamp_64 = m_ft.QuadPart + yuv_buf->timestamp_64;			
			FileTimeToSystemTime((FILETIME*)&yuv_buf->timestamp_64, &stData);

			if (yuv_buf->motion_detected > 0)
			{
				sprintf_s(szDebug, sizeof(szDebug), "CH %d come yuv_buf->motion_detected() = %d.", iCh+1, yuv_buf->motion_detected);
			//	OutputDebugString(szDebug);
			}						

			if (m_iRadioBtnBoard == iBoard)
			{
				if (is_primary == 1)
				{					
					DrawVideoYUV(iCh, (char*)yuv_buf->y_data, (char*)yuv_buf->u_data, (char*)yuv_buf->v_data, iBoard);
				}				

				if (m_bCreateAndInitialRegionSettingDlgOK && m_pRegionSettingDlg && (iCh == m_pRegionSettingDlg->m_iChannelIndex) && (is_primary == 1))
				{				
					m_pRegionSettingDlg->DrawVideoYUV((char*)yuv_buf->y_data, (char*)yuv_buf->u_data, (char*)yuv_buf->v_data);
				}

				if (m_bCreateAndInitialChannelSettingDlgOK && m_pChannelSettingDlg && (iCh == m_pChannelSettingDlg->m_iChannelIndex) && (is_primary == 0))
				{				
					m_pChannelSettingDlg->DrawVideoYUV((char*)yuv_buf->y_data, (char*)yuv_buf->u_data, (char*)yuv_buf->v_data);
				}		
			}			

			/*if(m_bInitDrawOK[iBoard][iCh])
			{				
				if (is_primary == 1)
				{
					if(m_bShowBoard[iBoard])
						DrawVideoYUV(iCh, (char*)yuv_buf->y_data, (char*)yuv_buf->u_data, (char*)yuv_buf->v_data, iBoard);
				}				

				if (m_bCreateAndInitialRegionSettingDlgOK && m_pRegionSettingDlg && (iCh == m_pRegionSettingDlg->m_iChannelIndex) && (is_primary == 1))
				{
					if(m_bShowBoard[iBoard])
						m_pRegionSettingDlg->DrawVideoYUV((char*)yuv_buf->y_data, (char*)yuv_buf->u_data, (char*)yuv_buf->v_data);
				}
			
				if (m_bCreateAndInitialChannelSettingDlgOK && m_pChannelSettingDlg && (iCh == m_pChannelSettingDlg->m_iChannelIndex) && (is_primary == 0))
				{
					if(m_bShowBoard[iBoard])
						m_pChannelSettingDlg->DrawVideoYUV((char*)yuv_buf->y_data, (char*)yuv_buf->u_data, (char*)yuv_buf->v_data);
				}				
			}	*/							

			sdvr_release_yuv_buffer(yuv_buf);
		}
	}
	else if (frame_type == SDVR_FRAME_RAW_AUDIO)
	{
		sprintf_s(szDebug, sizeof(szDebug), "CH %d come acquireBuffer(), frame_type = SDVR_FRAME_RAW_AUDIO.", iCh+1);
//		OutputDebugString(szDebug);
		sdvr_rc = sdvr_get_av_buffer(handle, SDVR_FRAME_RAW_AUDIO, &av_buf);
		
		if (sdvr_rc == SDVR_ERR_NONE && av_buf)
		{			
			if (m_bOpenWaveOutOK[iBoard][iCh] && (!m_pDVRSystem[iBoard]->m_bMute[iCh]))
			{
				m_pWaveOut[iBoard][iCh]->WriteAudioBlock(av_buf->payload, av_buf->payload_size);
			}

			sdvr_release_av_buffer(av_buf);
		}		
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "CH %d come acquireBuffer(), m_ChVdoDataType = S6_CHANNEL_VDO_DATA_TYPE_RAW, error frame_type = %d.", 
			iCh+1, frame_type);
		OutputDebugString(szDebug);
	}		


	return true;
}


BOOL CDvrFCDDlg::OpenWaveOut(int iCh, int iBoard)
{
	m_pWaveOut[iBoard][iCh] = NULL;
	m_pWaveOut[iBoard][iCh] = new CWaveOut();

	if(m_pWaveOut[iBoard][iCh])
	{
		WAVEFORMATEX	wfx;

		wfx.wFormatTag     = WAVE_FORMAT_PCM;
		wfx.wBitsPerSample  = 16;     // sample size /
		wfx.nChannels       = 1;      // channels    /
		wfx.cbSize          = 0;      // size of _extra_ info /
		wfx.nBlockAlign     = 2;
		wfx.nSamplesPerSec  = 8000;
		wfx.nAvgBytesPerSec = wfx.nBlockAlign*wfx.nSamplesPerSec;

		m_bOpenWaveOutOK[iBoard][iCh] = m_pWaveOut[iBoard][iCh]->WaveOutOpen(wfx);
	}

	return m_bOpenWaveOutOK[iBoard][iCh];
}


BOOL CDvrFCDDlg::CloseWaveOut(int iCh, int iBoard)
{
	m_bOpenWaveOutOK[iBoard][iCh] = FALSE;

	if(m_pWaveOut[iBoard][iCh])
	{		
		m_pWaveOut[iBoard][iCh]->WaveOutClose();
		delete m_pWaveOut[iBoard][iCh];
		m_pWaveOut[iBoard][iCh] = NULL;
	}

	return TRUE;
}


BOOL CDvrFCDDlg::PreTranslateMessage(MSG* pMsg)
{	
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return true;
		}		
	}	

	if (pMsg->message == WM_RBUTTONDOWN)
	{
		int i=0;
		for (i=0; i<MAX_BOARD_CHANNEL; i++)
		{
			if (pMsg->hwnd == GetDlgItem(IDC_STATIC_DISPLAY_01+i)->m_hWnd)
			{
				if (m_bIsConnectChannelOK[m_iRadioBtnBoard][i] && !m_bIsUseControlAll)
				{
					PostMessage(WM_MODIFY_CHANNEL_SETTING_DLG, m_iRadioBtnBoard, i);	// Here don't use SendMessage.
				}
				
				break;
			}
		}
	}
	else if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		int i=0;
		for (i=0; i<MAX_BOARD_CHANNEL; i++)
		{
			if (pMsg->hwnd == GetDlgItem(IDC_STATIC_DISPLAY_01+i)->m_hWnd)
			{
				if (m_bIsConnectChannelOK[m_iRadioBtnBoard][i] && !m_bIsUseControlAll)
				{
					PostMessage(WM_CREATE_REGION_SETTING_DLG, i, 0);	// Here don't use SendMessage.
				}

				break;
			}
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}


HRESULT CDvrFCDDlg::OnCreateBoardSettingDlg(WPARAM wParam, LPARAM lParam)
{	
	CBoardSettingDlg	        BoardSettingDlg;
	
	BoardSettingDlg.m_pMainDlg = this;
	BoardSettingDlg.m_iBoardIndex = (int)wParam;
	BoardSettingDlg.m_iBtnVideoStandard = 0;

	if (BoardSettingDlg.DoModal() == IDOK)
	{
		
	} 

	return S_OK;
}


HRESULT CDvrFCDDlg::OnCreateChannelSettingDlg(WPARAM wParam, LPARAM lParam)
{	
	if (m_bIsUseControlAll)
	{
		return S_OK;
	}
	
	m_pChannelSettingDlg = NULL;
	m_pChannelSettingDlg = new CChannelSettingDlg;

	m_pChannelSettingDlg->m_pMainDlg = this;	
	m_pChannelSettingDlg->m_iBoardIndex = (int)wParam;
	m_pChannelSettingDlg->m_iChannelIndex = (int)lParam;
	m_pChannelSettingDlg->m_bIsModify = false;

	if (m_pChannelSettingDlg->DoModal() == IDOK)
	{		
	} 

	if (m_pChannelSettingDlg)
	{
		delete m_pChannelSettingDlg;
		m_pChannelSettingDlg = NULL;
	}

	return S_OK;
}


HRESULT CDvrFCDDlg::OnModifyChannelSettingDlg(WPARAM wParam, LPARAM lParam)
{	
	if (m_bIsUseControlAll)
	{
		return S_OK;
	}

	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	m_pChannelSettingDlg = NULL;
	m_pChannelSettingDlg = new CChannelSettingDlg;	

	m_pChannelSettingDlg->m_pMainDlg = this;	
	m_pChannelSettingDlg->m_iBoardIndex = (int)wParam;
	m_pChannelSettingDlg->m_iChannelIndex = (int)lParam;		

	sdvr_rc = sdvr_stream_raw_video_secondary(m_pDVRSystem[m_iRadioBtnBoard]->m_pChannelParam[m_pChannelSettingDlg->m_iChannelIndex]->m_chan_handle,
											 SDVR_VIDEO_RES_DECIMATION_EQUAL, 15, true);


	// must set primary again.
	m_pDVRSystem[m_pChannelSettingDlg->m_iBoardIndex]->ResetFrameRate(m_pChannelSettingDlg->m_iChannelIndex);

	m_pChannelSettingDlg->m_iVdoWidth_Raw = m_pDVRSystem[m_iRadioBtnBoard]->m_iBoardSetWidth;
	m_pChannelSettingDlg->m_iVdoHeight_Raw = m_pDVRSystem[m_iRadioBtnBoard]->m_iBoardSetHeight;
	m_pChannelSettingDlg->m_bIsModify = true;
	//m_pChannelSettingDlg->m_iFrameRate = m_pDVRSystem[m_iEventOpenBoard]->m_iFrameRate[m_pChannelSettingDlg->m_iChannelIndex];

	if (m_pChannelSettingDlg->DoModal() == IDOK)
	{
		sdvr_rc = sdvr_stream_raw_video_secondary(m_pDVRSystem[m_iRadioBtnBoard]->m_pChannelParam[m_pChannelSettingDlg->m_iChannelIndex]->m_chan_handle,
			SDVR_VIDEO_RES_DECIMATION_EQUAL, 15, false);
	
		m_pDVRSystem[m_pChannelSettingDlg->m_iBoardIndex]->SetOSD(m_pChannelSettingDlg->m_iChannelIndex);
		m_pDVRSystem[m_pChannelSettingDlg->m_iBoardIndex]->ResetFrameRate(m_pChannelSettingDlg->m_iChannelIndex);
	} 

	if (m_pChannelSettingDlg)
	{
		delete m_pChannelSettingDlg;
		m_pChannelSettingDlg = NULL;
	}

	return S_OK;
}


HRESULT CDvrFCDDlg::OnCreateReginSettingDlg(WPARAM wParam, LPARAM lParam)
{	
	if (m_bIsUseControlAll)
	{
		return S_OK;
	}
	
	m_pRegionSettingDlg = NULL;
	m_pRegionSettingDlg = new CRegionSetting;

	m_pRegionSettingDlg->m_pMainDlg = this;
	m_pRegionSettingDlg->m_iChannelIndex = (int)wParam;
	m_pRegionSettingDlg->m_iVdoWidth_Raw = m_iVdoWidth_Raw[m_iRadioBtnBoard][m_pRegionSettingDlg->m_iChannelIndex];
	m_pRegionSettingDlg->m_iVdoHeight_Raw = m_iVdoHeight_Raw[m_iRadioBtnBoard][m_pRegionSettingDlg->m_iChannelIndex];

	if (m_pRegionSettingDlg->DoModal() == IDOK)
	{
		
	}	

	if (m_pRegionSettingDlg)
	{
		delete m_pRegionSettingDlg;
		m_pRegionSettingDlg = NULL;
	}

	return S_OK;
}


HRESULT CDvrFCDDlg::OnGetProtectID(WPARAM wParam, LPARAM lParam)
{
	int iBoard = (int) wParam;
	sx_uint32 iProtectID = 0;
	CString strText = "";
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;		

	if (!m_bIsInitDvrBoardOK[iBoard])
	{
		strText.Format("Board(%d) not connect yet!!.", iBoard+1);
		MessageBox(strText);
	}
	else
	{
		sdvr_rc = sdvr_get_protect_id(iBoard, &iProtectID);	
		sdvr_firmware_ver_t fwVersion = m_pDVRSystem[iBoard]->m_fwVersion;		

		strText.Format("Board(%d) ProtectID = 0x%X\nFw_ver : %d.%d.%d.%d\nBuild_date : %d/%d/%d\nBootloder_ver : %d.%d\nBsp_ver : %d.%d", 
			iBoard+1, iProtectID, fwVersion.fw_major, fwVersion.fw_minor, fwVersion.fw_revision, fwVersion.fw_build,
			fwVersion.fw_build_year, fwVersion.fw_build_month, fwVersion.fw_build_day,
			fwVersion.bootloader_major, fwVersion.bootloader_minor, fwVersion.bsp_major, fwVersion.bsp_minor);
		MessageBox(strText);
	}	

	return S_OK;
}



HRESULT CDvrFCDDlg::OnMD_Modify(WPARAM wParam, LPARAM lParam)
{
	int iCh = (int) wParam;
	int RegionID = (int) lParam;
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	sx_uint8 *regions_map;

	RectReverse2MB(iCh);

	int iMaxMB = m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iRegionMapWidth*m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iRegionMapLine;
	regions_map = (sx_uint8 *)calloc(iMaxMB, sizeof(sx_uint8)); 
	int i=0, j=0;

	for (i=0; i<iMaxMB; i++)
	{
		for (j=0; j<SDVR_MAX_MD_REGIONS; j++)
		{
			if (m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_pMDRegionListArray[j][i])
			{
				regions_map[i]+= (int) pow(2.0, j);
			}			
		}
	}
	
	sdvr_rc = sdvr_set_regions_map(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, SDVR_REGION_MOTION, regions_map);

	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_set_regions_map OK.", iCh+1);
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_change_region(%d) fail(%d).", iCh+1, sdvr_rc);	
		OutputDebugString(szDebug);
	}

	if (regions_map)
	{
		free(regions_map);
	}

	return S_OK;
}

HRESULT CDvrFCDDlg::OnMD_Enable(WPARAM wParam, LPARAM lParam)
{
	int iCh = (int) wParam;	
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	sdvr_rc = sdvr_enable_motion_detection_ex(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle,
											m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_bEnableMD,
											m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDThreshold[0],
											m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDThreshold[1],
											m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDThreshold[2],
											m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDThreshold[3]);

	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_enable_motion_detection OK.", iCh+1);		
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_enable_motion_detection fail(%d).", iCh+1, sdvr_rc);		
		OutputDebugString(szDebug);
	}

	sdvr_rc = sdvr_set_motion_value_frequency(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, 
		m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDValueFrequency);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_set_motion_value_frequency(%d) OK.", iCh+1, 
			m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDValueFrequency);		
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_set_motion_value_frequency(%d) fail(%d).", iCh+1, 
			m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDValueFrequency, sdvr_rc);		
		OutputDebugString(szDebug);
	}

	return S_OK;
}


void CDvrFCDDlg::RectReverse2MB(int iCh)
{
	int i=0, j=0, x=0, y=0;

	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));
	CRect rcReverse;
	CRect rcRegion;

	float flDrawWidth = m_rcRegionSettingDlgDrawArea.Width();
	float flDrawHeight = m_rcRegionSettingDlgDrawArea.Height();		

	int iMaxMB = m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iRegionMapWidth*m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iRegionMapLine;

	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		for (j=0; j<iMaxMB; j++)
		{
			m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_pMDRegionListArray[i][j] = 0;
		}
	}

	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		for (j=0; j<iMaxMB; j++)
		{			
			if (m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_bFinishMDRectOK[i][j])
			{
				rcRegion = m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_rcMDRect[i][j];
				
				// Reverse to video source coordinate.
				rcReverse.left = m_pDVRSystem[m_iEventOpenBoard]->m_iBoardSetWidth/flDrawWidth*rcRegion.left;	
				rcReverse.top = m_pDVRSystem[m_iEventOpenBoard]->m_iBoardSetHeight/flDrawHeight*rcRegion.top;
				rcReverse.right = m_pDVRSystem[m_iEventOpenBoard]->m_iBoardSetWidth/flDrawWidth*rcRegion.right;
				rcReverse.bottom = m_pDVRSystem[m_iEventOpenBoard]->m_iBoardSetHeight/flDrawHeight*rcRegion.bottom;

				// Find MB index.
				rcReverse.left = rcReverse.left/16;	// Start MB X index.
				rcReverse.top = rcReverse.top/16;  // Start MB Y index.
				rcReverse.right = rcReverse.right/16; // End MB X index.
				rcReverse.bottom = rcReverse.bottom/16; // End MB Y index.

				for (x=rcReverse.left; x<=rcReverse.right; x++)
				{
					for (y=rcReverse.top; y<=rcReverse.bottom; y++)
					{
						m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_pMDRegionListArray[i][y*m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iRegionMapWidth+x] = true;				
					}
				}					
			}
						
		}
	}	
}


sdvr_err_e CDvrFCDDlg::ResetMotion(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));
	int i=0;	

	if ( m_pDVRSystem[m_iEventOpenBoard] &&
		m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh] && 
		m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle)
	{
		sx_uint8 *regions_map;		

		int iMaxMB = m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iRegionMapWidth*m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iRegionMapLine;
		regions_map = (sx_uint8 *)calloc(iMaxMB, sizeof(sx_uint8)); 
		int x=0;

		for (x=0; x<iMaxMB; x++)
		{			
			regions_map[x] = 0;				
		}

		sdvr_rc = sdvr_set_regions_map(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, SDVR_REGION_MOTION, regions_map);

		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_set_regions_map OK.", iCh+1);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_change_region(%d) fail(%d)\n", iCh+1, sdvr_rc);	
			OutputDebugString(szDebug);
		}

		free(regions_map);

		sdvr_rc = sdvr_enable_motion_detection_ex(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, 
												false,
												m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDThreshold[0],
												m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDThreshold[1],
												m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDThreshold[2],
												m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_iMDThreshold[3]);

		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_enable_motion_detection OK.", iCh+1);		
			OutputDebugString(szDebug);
			m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_bEnableMD = FALSE;			
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "CH %d sdvr_enable_motion_detection fail(%d).", iCh+1, sdvr_rc);		
			OutputDebugString(szDebug);
		}
	}

	return sdvr_rc;
}



HRESULT CDvrFCDDlg::OnPTZ_Action(WPARAM wParam, LPARAM lParam)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	BOOL bIsNormalAction = TRUE;

	int iCh = (int)wParam;
	PTZ_ACTION PtzAction = (PTZ_ACTION)lParam;

	BYTE btSendData[7];
	ZeroMemory(btSendData,sizeof(btSendData));

	//===================================//
#define SYNCH_BYTE      0
#define ADDRESS_BYTE    1
#define COMMAND1_BYTE   2
#define COMMNAD2_BYTE   3
#define DATA1_BYTE      4
#define DATA2_BYTE      5
#define CHECK_SUM_BYTE  6
	//==================================//
	btSendData[SYNCH_BYTE] = 0xFF;        //this parameter is Fix.
	btSendData[ADDRESS_BYTE] = m_iAddress;

	WORD wPelcoParam = 0;
	LPBYTE pbyParam = NULL;

	switch (PtzAction)
	{
	case PTZ_INIT:
		bIsNormalAction = FALSE;		
		break;

	case	 PTZ_UNINIT:
		break;

	case	 PTZ_UP:
		wPelcoParam = PELCO_TILT_UP;		
		break;

	case	 PTZ_LEFT:
		wPelcoParam = PELCO_PAN_LEFT;	
		break;

	case	 PTZ_RIGHT:
		wPelcoParam = PELCO_PAN_RIGHT;	
		break;

	case	 PTZ_DOWN:
		wPelcoParam = PELCO_TILT_DOWN;	
		break;

	case	 PTZ_FOCUS_FAR:
		wPelcoParam = PELCO_FOCUS_FAR;	
		break;

	case	 PTZ_FOCUS_NEAR:
		wPelcoParam = PELCO_FOCUS_NEAR;	
		break;

	case	 PTZ_ZOOM_IN:
		wPelcoParam = PELCO_ZOOM_IN;	
		break;

	case	 PTZ_ZOOM_OUT:
		wPelcoParam = PELCO_ZOOM_OUT;	
		break;

	case	 PTZ_IRIS_LARGE:
		wPelcoParam = PELCO_IRIS_LARGE;	
		break;

	case	 PTZ_IRIS_SMALL:
		wPelcoParam = PELCO_IRIS_SMALL;	
		break;

	case	 PTZ_ACTION_STOP:
		wPelcoParam = PELCO_STOP_ALL_ACTION;	
		break;

	default:
		break;
	}

	if (bIsNormalAction)
	{
		pbyParam = (LPBYTE)&wPelcoParam;
		btSendData[COMMAND1_BYTE] = *pbyParam;
		btSendData[COMMNAD2_BYTE] = *(pbyParam+1);
		if (m_iSpeed == 0)
		{
			btSendData[DATA1_BYTE] = PELCO_PAN_90_DEGREE;
			btSendData[DATA2_BYTE] = PELCO_TILT_22_DEGREE;
		}
		else if (m_iSpeed == 1)
		{
			btSendData[DATA1_BYTE] = PELCO_PAN_180_DEGREE;
			btSendData[DATA2_BYTE] = PELCO_TILT_90_DEGREE;
		}
		else if (m_iSpeed == 2)
		{
			btSendData[DATA1_BYTE] = PELCO_PAN_360_DEGREE;
			btSendData[DATA2_BYTE] = PELCO_TILT_360_DEGREE;
		}

		btSendData[CHECK_SUM_BYTE] = btSendData[ADDRESS_BYTE] + btSendData[COMMAND1_BYTE]
		+ btSendData[COMMNAD2_BYTE] + btSendData[DATA1_BYTE]
		+ btSendData[DATA2_BYTE];
		sdvr_rc = sdvr_write_uart(m_iEventOpenBoard, sizeof(btSendData), btSendData);
	}
	else
	{
		sdvr_rc = sdvr_init_uart(m_iEventOpenBoard, m_iBaudRate, 8, 1, false, 0);
		if (sdvr_rc == SDVR_ERR_NONE)
		{			
			m_bRS485Init[m_iEventOpenBoard] = true;			
		}
		else
		{			
			m_bRS485Init[m_iEventOpenBoard] = false;
			return S_FALSE;
		}
	}

	return S_OK;
}


HRESULT CDvrFCDDlg::OnCreateRSDlg(WPARAM wParam, LPARAM lParam)
{
	if (m_pRelaysAndSensorsDlg)		// is opened
	{
		return S_OK;
	}

	m_pRelaysAndSensorsDlg = NULL;
	m_pRelaysAndSensorsDlg = new CRelaysAndSensorsDlg;
	m_pRelaysAndSensorsDlg->m_pMainDlg = this;
	m_pRelaysAndSensorsDlg->m_iBoardIndex = (int) wParam;

	if (m_pRelaysAndSensorsDlg->DoModal() == IDOK)
	{
	}


	if (m_pRelaysAndSensorsDlg)
	{
		delete m_pRelaysAndSensorsDlg;
		m_pRelaysAndSensorsDlg = NULL;
	}

	return S_OK;
}


HRESULT CDvrFCDDlg::OnConfigSensors(WPARAM wParam, LPARAM lParam)
{
	int sensor_enable_map = (int) wParam;
	int edge_triggered_map = (int) lParam;
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	sdvr_rc = sdvr_config_sensors(m_iEventOpenBoard , sensor_enable_map, edge_triggered_map);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_config_sensors OK.");		
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_config_sensors fail(%d).", sdvr_rc);		
		OutputDebugString(szDebug);
	}

	return S_OK;
}


HRESULT CDvrFCDDlg::OnRelayTrigger(WPARAM wParam, LPARAM lParam)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));
	int iTriggerIndex = (int) wParam;
	bool bTrigger = (sx_bool) lParam;

	sdvr_rc = sdvr_trigger_relay(m_iEventOpenBoard, iTriggerIndex, bTrigger);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "Relay(%d) sdvr_trigger_relay(%d) OK.", iTriggerIndex, bTrigger);		
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "Relay(%d) sdvr_trigger_relay(%d) fail(%d).", iTriggerIndex, bTrigger, sdvr_rc);		
		OutputDebugString(szDebug);
	}

	return S_OK;
}


void CDvrFCDDlg::ConfigBrightness(int iCh, BOOL bGet)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (bGet)
	{
		sdvr_rc = sdvr_get_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, &m_image_ctrl[m_iEventOpenBoard][iCh]);		
	}
	else
	{		
		sdvr_rc = sdvr_set_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, SDVR_ICFLAG_BRIGHTNESS, &m_image_ctrl[m_iEventOpenBoard][iCh]);
	}	
}


void CDvrFCDDlg::ConfigContrast(int iCh, BOOL bGet)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	
	if (bGet)
	{
		sdvr_rc = sdvr_get_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, &m_image_ctrl[m_iEventOpenBoard][iCh]);		
	}
	else
	{		
		sdvr_rc = sdvr_set_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, SDVR_ICFLAG_CONTRAST, &m_image_ctrl[m_iEventOpenBoard][iCh]);
	}
		
}


void CDvrFCDDlg::ConfigSaturation(int iCh, BOOL bGet)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	
	if (bGet)
	{
		sdvr_rc = sdvr_get_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, &m_image_ctrl[m_iEventOpenBoard][iCh]);		
	}
	else
	{		
		sdvr_rc = sdvr_set_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, SDVR_ICFLAG_SATURATION, &m_image_ctrl[m_iEventOpenBoard][iCh]);
	}		
}


void CDvrFCDDlg::ConfigHue(int iCh, BOOL bGet)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	
	if (bGet)
	{
		sdvr_rc = sdvr_get_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, &m_image_ctrl[m_iEventOpenBoard][iCh]);		
	}
	else
	{		
		sdvr_rc = sdvr_set_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, SDVR_ICFLAG_HUE, &m_image_ctrl[m_iEventOpenBoard][iCh]);
	}
}


void CDvrFCDDlg::ConfigSharpness(int iCh, BOOL bGet)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	
	if (bGet)
	{
		sdvr_rc = sdvr_get_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, &m_image_ctrl[m_iEventOpenBoard][iCh]);	
	}
	else
	{		
		sdvr_rc = sdvr_set_video_in_params(m_pDVRSystem[m_iEventOpenBoard]->m_pChannelParam[iCh]->m_chan_handle, SDVR_ICFLAG_SHARPNESS, &m_image_ctrl[m_iEventOpenBoard][iCh]);
	}		
}


void CDvrFCDDlg::DrawMotionAlarm(int iCh)
{
	CWnd* pcWndItemDisplay = GetDlgItem(IDC_STATIC_DISPLAY_01+iCh);
	CClientDC dcDisplay(pcWndItemDisplay);	

	CBrush brushMDAlarm(RGB(255, 0, 0));
	CBrush* oldBrush;
	CString strText;
	CRect rcDisplay;
	pcWndItemDisplay->GetClientRect(&rcDisplay);	

	rcDisplay.left = rcDisplay.left+1;
	rcDisplay.top = rcDisplay.top+1;
	rcDisplay.right = rcDisplay.right-1;
	rcDisplay.bottom = rcDisplay.bottom-1;

	oldBrush = dcDisplay.SelectObject(&brushMDAlarm);	
	
	dcDisplay.FrameRect(rcDisplay, &brushMDAlarm);
	rcDisplay.left = rcDisplay.left+1;
	rcDisplay.top = rcDisplay.top+1;
	rcDisplay.right = rcDisplay.right-1;
	rcDisplay.bottom = rcDisplay.bottom-1;
	dcDisplay.FrameRect(rcDisplay, &brushMDAlarm);

	strText.Format("CH %d Motion Detect Alarm!!!", iCh+1);
	dcDisplay.TextOut(rcDisplay.left+3, rcDisplay.top+3, strText);	

	dcDisplay.SelectObject(oldBrush);	
}


void CDvrFCDDlg::GetSensorArrayTrueFlag(int iValue)
{
	DWORD dwCk = iValue;
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	int i = 0;
	for(i=0; i<MAX_RELAYS_SENSORS; i++)
	{
		if(dwCk & 0x01)
		{			
			sprintf_s(szDebug, sizeof(szDebug), "Board(%d) receive sensor(%d) trigger.", m_iEventOpenBoard+1, i+1);
			OutputDebugString(szDebug);
			if (m_pRelaysAndSensorsDlg)
			{
				m_pRelaysAndSensorsDlg->ShowSensorAlarm(i);
			}
		}
		else
		{		
		}

		dwCk = dwCk >> 1;		
	}	
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//              C A L L    B A C K       F U N C T I O N
////////////////////////////////////////////////////////////////////////////////////////////////
void av_frame_callback(sdvr_chan_handle_t handle, sdvr_frame_type_e frame_type, sx_bool is_primary)
{
	sdvr_av_buffer_t  *av_buffer;
	sdvr_yuv_buffer_t *yuv_buffer;

	char szDebug[200];	
	memset(&szDebug, 0, sizeof(szDebug));

	bool bfind = false;
	int i=0, j=0;
	int iCh = 0;	
	int iBoardIndex = 0;

	iBoardIndex = sdvr_get_board_index(handle);
	iCh		   = sdvr_get_chan_num(handle);	

	// Don't receive any new frame while we are trying to close the system
	if (g_pMainDlg->m_bProcessIsClosing)
	{
		if (frame_type == SDVR_FRAME_RAW_VIDEO)
		{
			if (sdvr_get_yuv_buffer(handle, &yuv_buffer) == SDVR_ERR_NONE)
				sdvr_release_yuv_buffer(yuv_buffer);
		}
		else
		{
			if (sdvr_get_av_buffer(handle, is_primary ? SDVR_FRAME_VIDEO_ENCODED_PRIMARY : SDVR_FRAME_VIDEO_ENCODED_SECONDARY, 
				&av_buffer) == SDVR_ERR_NONE)
			{
				sdvr_release_av_buffer(av_buffer);
			}
		}
		return;
	}	

	switch (frame_type)	// This is a raw video live view demo, so we will only receive SDVR_FRAME_RAW_AUDIO and SDVR_FRAME_RAW_VIDEO.
	{
	case SDVR_FRAME_AUDIO_ENCODED:
	case SDVR_FRAME_RAW_AUDIO:		
	case SDVR_FRAME_RAW_VIDEO:
	case SDVR_FRAME_H264_IDR:
	case SDVR_FRAME_H264_I:
	case SDVR_FRAME_H264_P:
	case SDVR_FRAME_H264_B:
	case SDVR_FRAME_H264_SPS:
	case SDVR_FRAME_H264_PPS:
	case SDVR_FRAME_MPEG4_I:
	case SDVR_FRAME_MPEG4_P:
	case SDVR_FRAME_MPEG4_VOL:		
	case SDVR_FRAME_JPEG:	
		g_pMainDlg->acquireBuffer(iBoardIndex, iCh, frame_type, is_primary, handle);
		break;

	default:
		break;
	}
}


void video_alarm_callback(sdvr_chan_handle_t handle, sdvr_video_alarm_e alarm_type, sx_uint32 data)
{
	int iCh = 0;	
	int iBoardIndex = 0;
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	iBoardIndex = sdvr_get_board_index(handle);
	iCh		   = sdvr_get_chan_num(handle);

	switch (alarm_type)
	{
	case SDVR_VIDEO_ALARM_MOTION:
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) CH(%d) receive SDVR_VIDEO_ALARM_MOTION.", iBoardIndex+1, iCh+1);
		//		OutputDebugString(szDebug);
		g_pMainDlg->DrawMotionAlarm(iCh);
		if (g_pMainDlg->m_bCreateAndInitialRegionSettingDlgOK && g_pMainDlg->m_pRegionSettingDlg && (iCh == g_pMainDlg->m_pRegionSettingDlg->m_iChannelIndex))
		{
			g_pMainDlg->m_pRegionSettingDlg->DrawMotionAlarm();
		}

		if (g_pMainDlg->m_bCreateAndInitialChannelSettingDlgOK && g_pMainDlg->m_pChannelSettingDlg && (iCh == g_pMainDlg->m_pChannelSettingDlg->m_iChannelIndex))
		{
			g_pMainDlg->m_pChannelSettingDlg->DrawMotionAlarm();
		}

		break;

	case SDVR_VIDEO_ALARM_BLIND:		
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) CH(%d) receive SDVR_VIDEO_ALARM_BLIND.", iBoardIndex+1, iCh+1);
		//		OutputDebugString(szDebug);
		break;

	case SDVR_VIDEO_ALARM_NIGHT:
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) CH(%d) receive SDVR_VIDEO_ALARM_NIGHT.", iBoardIndex+1, iCh+1);
		//		OutputDebugString(szDebug);
		break;

	case SDVR_VIDEO_ALARM_LOSS:
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) CH(%d) receive SDVR_VIDEO_ALARM_LOSS.", iBoardIndex+1, iCh+1);
		//		OutputDebugString(szDebug);
		break;

	case SDVR_VIDEO_ALARM_DETECTED:	
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) CH(%d) receive SDVR_VIDEO_ALARM_DETECTED.", iBoardIndex+1, iCh+1);
		//		OutputDebugString(szDebug);		
		break;

	default:
		break;
	}

}


void sensors_callback(sx_uint32 board_index, sx_uint32 sensors_map)
{
	g_pMainDlg->GetSensorArrayTrueFlag(sensors_map);
}


void signals_callback(sx_uint32 board_index, sdvr_signal_info_t *signal_info)
{
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));
	sprintf_s(szDebug, sizeof(szDebug), "Board(%d) receive signals_callback.", board_index+1);
	OutputDebugString(szDebug);	
}

void CDvrFCDDlg::OnBnClickedRadioBoard1()
{
	int i;
	for(i =0 ;i< MAX_BOARD_CHANNEL ; i++)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_01+i)->Invalidate();
	}
	/*memset(&m_bShowBoard, 0, sizeof(m_bShowBoard));
	m_bShowBoard[0] = TRUE;*/
	
	m_iRadioBtnBoard = 0;
}

void CDvrFCDDlg::OnBnClickedRadioBoard2()
{
	int i;
	for(i =0 ;i< MAX_BOARD_CHANNEL ; i++)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_01+i)->Invalidate();
	}
	/*memset(&m_bShowBoard, 0, sizeof(m_bShowBoard));
	m_bShowBoard[1] = TRUE;*/
	
	m_iRadioBtnBoard = 1;
}

void CDvrFCDDlg::OnBnClickedRadioBoard3()
{
	int i;
	for(i =0 ;i< MAX_BOARD_CHANNEL ; i++)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_01+i)->Invalidate();
	}
	/*memset(&m_bShowBoard, 0, sizeof(m_bShowBoard));
	m_bShowBoard[2] = TRUE;*/
	
	m_iRadioBtnBoard = 2;
}

void CDvrFCDDlg::OnBnClickedRadioBoard4()
{
	int i;
	for(i =0 ;i< MAX_BOARD_CHANNEL ; i++)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_01+i)->Invalidate();
	}
	/*memset(&m_bShowBoard, 0, sizeof(m_bShowBoard));
	m_bShowBoard[3] = TRUE;*/

	m_iRadioBtnBoard = 3;
}
