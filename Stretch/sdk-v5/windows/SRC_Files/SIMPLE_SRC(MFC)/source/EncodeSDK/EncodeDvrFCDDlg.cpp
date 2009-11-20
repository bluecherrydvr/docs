// DvrFCDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EncodeDvrFCD.h"
#include "EncodeDvrFCDDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define THREADKILL_TO			5000
#define THREADEXIT_SUCCESS		0x1234

void av_frame_callback(sdvr_chan_handle_t handle,
					   sdvr_frame_type_e frame_type, sx_bool primary_frame);


CDvrFCDDlg* g_pMainDlg;

// CDvrFCDDlg dialog
CDvrFCDDlg::CDvrFCDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDvrFCDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	int i=0;
	m_pMenu = NULL;

	int iBoardIndex=0;

	m_iBoardCount = 0;	

	g_pMainDlg = this;

	for (iBoardIndex=0; iBoardIndex < MAX_SUPPORT_BOARD; iBoardIndex++)
	{
		m_pDVRSystem[iBoardIndex] = NULL;
		m_iVdoCount[iBoardIndex] = 0;
		m_iAdoCount[iBoardIndex] = 0;
		m_VideoStandard[iBoardIndex] = SDVR_VIDEO_STD_NONE;
		m_bIsInitDvrBoardOK[iBoardIndex] = false;		
	}

	m_bIsInitAllDvrBoardOK = false;
	m_bShowBoard[0] = true;
	for(i=1; i < MAX_SUPPORT_BOARD ; i++)
	{
		m_bShowBoard[i] = false;
	}

	for (i = 0; i < MAX_EVENT_UI_CONTROL; i++)
	{
		m_hEventUIControl[i] = NULL;		
	}

	for (i = 0; i < MAX_BOARD_CHANNEL; i++)
	{
		m_bInitDrawOK[i] = FALSE;
		bCheckPrimary[i] = false;
		bCheckSecond[i] = false;
		bCheckPriH264[i] = false;
		bCheckSecH264[i] = false;
		bCheckPriMJPEG[i] = false;
		bCheckSecMJPEG[i] = false;
		m_pDirectDrawDisplay[i] = NULL;
		m_iVdoWidth_Raw[i] = 720;
		m_iVdoHeight_Raw[i] = 480;
		m_pbyYUVBuffer[i] = NULL;
		m_pMp4SaveFile[i] = NULL;
		m_pAviSaveFile[i] = NULL;
		m_pMp4SaveFile_sec[i] = NULL;
		m_pAviSaveFile_sec[i] = NULL;
	}

	for(i = 0 ; i < MAX_SUPPORT_CHANNEL ; i++)
	{
		m_bIsConnectChannelOK[i] = false;
		m_bIsEncodeChOK[i] = false;
	}

	m_bProcessIsClosing = false;
	m_bIsUseControlAll = false;
	m_bIsUnInitBoard = false;

	m_pWaveOut = NULL;
	m_iPlayAudioCh = -1;

	m_hUIControlThread = NULL;

	m_iEventOpenBoard = -1;
	m_iEventEncodeCh = -1 ;
	m_iEventConnectCh = -1 ;
	
	memset(m_szCurExeDir, 0, sizeof(m_szCurExeDir));

	GetCurrentDirectory(sizeof(m_szCurExeDir), m_szCurExeDir);

}

void CDvrFCDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_RADIO_BOARD1, m_bShowBoard[0]);
}

BEGIN_MESSAGE_MAP(CDvrFCDDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()	
	ON_MESSAGE(WM_MODIFY_CHANNEL_SETTING_DLG, OnModifyChannelSettingDlg)
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
	m_pMenu = NULL;
	m_pMenu = new CMenu();
	m_pMenu->LoadMenuA(IDR_MENU_CONTROL);
	
	int i=0;
	SetMenu(m_pMenu);	// Add the new menu
	
	DWORD dwID=0;
	for (i=0; i<MAX_EVENT_UI_CONTROL; i++)
	{
		m_hEventUIControl[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(m_hEventUIControl);
	}

	m_pWaveOut = new CWaveOut(this);
	if(m_pWaveOut)
	{
		m_pWaveOut->WaveOutOpen();
	}

	for(i=0 ; i<MAX_SUPPORT_BOARD; i++)
	{
		GetDlgItem(IDC_RADIO_BOARD1 + i)->ShowWindow(SW_HIDE);
	}

	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	sdvr_rc = sdvr_sdk_close();	

	Sleep(100);

	sdvr_rc = sdvr_sdk_init();	

	Sleep(100);

	char szDebug[80];
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_sdk_init() = SDVR_ERR_NONE\n");
		//OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_sdk_init() = %d\n", sdvr_rc);
		//OutputDebugString(szDebug);
	}
	
	m_iBoardCount = sdvr_get_board_count();

	if (m_iBoardCount>=1)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_get_board_count() = %d\n", m_iBoardCount);
		//OutputDebugString(szDebug);	
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
	

	for(i=0; i<MAX_SUPPORT_CHANNEL; i++)
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_01+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_01+i, MF_UNCHECKED);

		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+i, MF_UNCHECKED);
	}

	sdvr_rc = sdvr_enable_auth_key(false);

	m_hUIControlThread = (HANDLE) CreateThread(NULL, 0, Thread_UIControl, this, 0, &dwID);

	sdvr_set_av_frame_callback(av_frame_callback);	

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

	Sleep(200);

 	if ((sdvr_rc = m_pDVRSystem[iBoardIndex]->GetBoardParam()) != SDVR_ERR_NONE)
	{
		return sdvr_rc;
	}
		
	if ((sdvr_rc = m_pDVRSystem[iBoardIndex]->SetBoardParam()) != SDVR_ERR_NONE)
	{
		return sdvr_rc;
	}

	Sleep(100);

	if ((sdvr_rc = m_pDVRSystem[iBoardIndex]->OpenBoard()) != SDVR_ERR_NONE)
	{
		return sdvr_rc;
	}			

	m_iVdoCount[iBoardIndex] = m_pDVRSystem[iBoardIndex]->m_iVdoCount;
	m_iAdoCount[iBoardIndex] = m_pDVRSystem[iBoardIndex]->m_iAdoCount;
	m_VideoStandard[iBoardIndex] = m_pDVRSystem[iBoardIndex]->m_VideoStandard;


	return sdvr_rc;
}


sdvr_err_e CDvrFCDDlg::UnInitBoard(int iBoardIndex)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	int i=0;

	if (m_pDVRSystem[iBoardIndex])
	{
		m_bIsUnInitBoard = true;

		for (i=0; i<MAX_BOARD_CHANNEL; i++)
		{		
			SetEncodeChannel(i, false);
			Sleep(10);
			SetPreviewChannel(i ,false);
			Sleep(10);

			m_bIsConnectChannelOK[i] = false;
			m_bIsEncodeChOK[i] = false ;
		}

		
		sdvr_rc = m_pDVRSystem[iBoardIndex]->CloseBoard();

		delete m_pDVRSystem[iBoardIndex];
		m_pDVRSystem[iBoardIndex] = NULL;		
	}			

	return sdvr_rc;
}

void CDvrFCDDlg::OnClose()
{		
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[64];

	m_bProcessIsClosing = true;	

	int i=0;
	for (i=0; i<m_iBoardCount; i++)
	{
		if (m_pDVRSystem[i])
		{
			m_iEventOpenBoard = i;
			UnInitBoard(i);
		}
	}	

	Sleep(100);

	for (i=0; i<MAX_BOARD_CHANNEL; i++)
	{
		if (m_pDirectDrawDisplay[i])
			CloseDrawDib(i);		
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
		sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_sdk_close() = SDVR_ERR_NONE\n");
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_sdk_close() = %d\n", sdvr_rc);
		OutputDebugString(szDebug);
	}	

	if(m_pWaveOut)
	{
		m_pWaveOut->WaveOutClose();
		delete m_pWaveOut;
		m_pWaveOut = NULL;
	}

	CDialog::OnClose();
}


DWORD WINAPI CDvrFCDDlg::Thread_UIControl(LPVOID pParam)
{
	CDvrFCDDlg* pMainDlg = (CDvrFCDDlg*) pParam;
	BOOL bExit = FALSE;

	DWORD	dwResult = 0;	
	int		EventID = 0;
	int     iBoardIndex = 0;
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	int i=0;

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
					pMainDlg->SetInitDvrMenuState(false, false);
					pMainDlg->SetEnableChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1 , -1);

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
						pMainDlg->SetInitDvrMenuState(true, false);
						break;
					}

					if(iBoardCount == pMainDlg->m_iBoardCount-1 )
					{
						int i;
						for(i = 0 ; i< pMainDlg->m_iBoardCount ;i++)
						{
							pMainDlg->m_pDVRSystem[i]->SetOSDFontTable();
							pMainDlg->SetEnableChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1 , i);

							sprintf_s(szDebug, sizeof(szDebug),"All board SetOSDFontTable OK!");
							pMainDlg->SetWindowText(szDebug);
						}
						pMainDlg->SetInitDvrMenuState(true, true);
						pMainDlg->m_bIsInitAllDvrBoardOK = true;

						sprintf_s(szDebug, sizeof(szDebug),"All board Connect OK!");
						pMainDlg->SetWindowText(szDebug);
					}
				}				
			}
			else if (EventID == ENAME_UNINIT_BOARD)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_UNINIT_BOARD]);

				int iBoardCount ;
				int iOriEventOpenBoard = pMainDlg->m_iEventOpenBoard;

				for(iBoardCount =0 ; iBoardCount < pMainDlg->m_iBoardCount ; iBoardCount++)
				{
					pMainDlg->SetInitDvrMenuState(false, false);	
					pMainDlg->SetEnableChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1, -1);
					
					pMainDlg->m_iEventOpenBoard = iBoardCount;
					if ((sdvr_rc = pMainDlg->UnInitBoard(iBoardCount)) == SDVR_ERR_NONE)
					{
						pMainDlg->m_bIsInitDvrBoardOK[iBoardCount] = false;					
					}
					else
					{
						pMainDlg->m_bIsInitDvrBoardOK[iBoardCount] = true;
					}

					pMainDlg->SetInitDvrMenuState(true, pMainDlg->m_bIsInitDvrBoardOK[iBoardCount]);
					pMainDlg->SetEnableChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1, iBoardCount);
					pMainDlg->SetEncodeChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1, iBoardCount);
					pMainDlg->m_bIsInitAllDvrBoardOK = false;
				}

				pMainDlg->m_iEventOpenBoard = iOriEventOpenBoard;
			}
 			else if (EventID == ENAME_START_PREVIEW_CH)
			{				
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_START_PREVIEW_CH]);
				pMainDlg->SetEnableChMenuState(false, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventConnectCh], pMainDlg->m_iEventConnectCh , pMainDlg->m_iEventOpenBoard);
				
 				if(pMainDlg->SetPreviewChannel(pMainDlg->m_iEventConnectCh ,true))
				{
					pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventConnectCh] = true;
				}
 				else
				{
					pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventConnectCh] = false;
				}

				pMainDlg->SetEnableChMenuState(true, pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventConnectCh] , pMainDlg->m_iEventConnectCh , pMainDlg->m_iEventOpenBoard);
				pMainDlg->SetEncodeChMenuState(true, false, pMainDlg->m_iEventConnectCh , pMainDlg->m_iEventOpenBoard);
			}
			else if (EventID == ENAME_STOP_PREVIEW_CH)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_STOP_PREVIEW_CH]);
				
				if (pMainDlg->SetPreviewChannel(pMainDlg->m_iEventConnectCh , false))
				{
					pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventConnectCh] = true;					
				}
				else	
				{
					pMainDlg->m_bIsConnectChannelOK[pMainDlg->m_iEventConnectCh] = false;
				}
			
				pMainDlg->SetEnableChMenuState(true, false, pMainDlg->m_iEventConnectCh , pMainDlg->m_iEventOpenBoard);
				pMainDlg->SetEncodeChMenuState(false, false, pMainDlg->m_iEventConnectCh, pMainDlg->m_iEventOpenBoard);
							
			}
			else if (EventID == ENAME_START_ENCODE_CH)
			{				
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_START_ENCODE_CH]);

				if (pMainDlg->SetEncodeChannel(pMainDlg->m_iEventEncodeCh, true))
				{
					pMainDlg->m_bIsEncodeChOK[pMainDlg->m_iEventEncodeCh] = true;					
				}
				else	// disconnect fail.
				{
					pMainDlg->m_bIsEncodeChOK[pMainDlg->m_iEventEncodeCh] = false;
				}

				pMainDlg->SetEncodeChMenuState(true, pMainDlg->m_bIsEncodeChOK[pMainDlg->m_iEventEncodeCh], pMainDlg->m_iEventEncodeCh, pMainDlg->m_iEventOpenBoard);
			}
			else if (EventID == ENAME_STOP_ENCODE_CH)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_STOP_ENCODE_CH]);

				if (pMainDlg->SetEncodeChannel(pMainDlg->m_iEventEncodeCh , false))
				{
					pMainDlg->m_bIsEncodeChOK[pMainDlg->m_iEventEncodeCh] = true;
				}
				else
				{
					pMainDlg->m_bIsEncodeChOK[pMainDlg->m_iEventEncodeCh] = false;
				}
				pMainDlg->SetEncodeChMenuState(true, pMainDlg->m_bIsEncodeChOK[pMainDlg->m_iEventEncodeCh], pMainDlg->m_iEventEncodeCh, pMainDlg->m_iEventOpenBoard);
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


/////////////////////////////////////////////////////////////////////////////////////////////////
//              C A L L    B A C K       F U N C T I O N
////////////////////////////////////////////////////////////////////////////////////////////////
void av_frame_callback(sdvr_chan_handle_t handle, sdvr_frame_type_e frame_type, sx_bool is_primary)
{
	sdvr_av_buffer_t  *av_buffer;
	sdvr_yuv_buffer_t *yuv_buffer;

	char szDebug[64];	

	bool bfind = false;
	int i=0, j=0;
	int iCh = 0;	
	int iBoardIndex = 0;

	iBoardIndex = sdvr_get_board_index(handle);
	iCh		   = sdvr_get_chan_num(handle);

	

	sprintf_s(szDebug, sizeof(szDebug), 
			"CH%d come frame_type = %d\n", iCh, frame_type);
	//OutputDebugString(szDebug);

	// Don't receive any new frame while we are trying to close the system
	// RAW_VIDEO
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

	if (g_pMainDlg->m_pDVRSystem[iBoardIndex] && g_pMainDlg->m_pDVRSystem[iBoardIndex]->m_pEncodeChParam[iCh])//Encode Channel
	{
		if (g_pMainDlg->m_pDVRSystem[iBoardIndex]->m_pEncodeChParam[iCh]->m_bIsClosingChannel)
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

			g_pMainDlg->m_pDVRSystem[iBoardIndex]->m_pEncodeChParam[iCh]->m_bIsClosingChannel = false;

			return;
		}
	}
	
	switch (frame_type)
	{
	case SDVR_FRAME_RAW_AUDIO:		
	case SDVR_FRAME_RAW_VIDEO:
	case SDVR_FRAME_AUDIO_ENCODED:
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

BOOL CDvrFCDDlg::PreTranslateMessage(MSG* pMsg)
{	
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		{
			return true;
		}		
	}	

	if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		int i=0;
		for (i=0; i<MAX_BOARD_CHANNEL; i++)
		{
			if (pMsg->hwnd == GetDlgItem(IDC_STATIC_DISPLAY_01+i)->m_hWnd)
			{
				if (m_bIsEncodeChOK[i] && !m_bIsUseControlAll)
				{
					PostMessage(WM_MODIFY_CHANNEL_SETTING_DLG, m_iEventEncodeCh, i);	// Here don't use SendMessage.
				}

				break;
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

bool CDvrFCDDlg::SetPreviewChannel(int iCh , bool bEnable)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	int iSelEncodeCh = -1 ;

	if(iCh > 15)
		iCh = iCh % MAX_BOARD_CHANNEL;
		
	if (m_pDVRSystem[m_iEventOpenBoard])
	{
		if(bEnable)
		{
 			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->CreateEncodeChannel(iCh);

			iSelEncodeCh = iCh ;

			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StartEnableChannel(iSelEncodeCh);
			if(sdvr_rc != SDVR_ERR_NONE)
			{
				m_bIsConnectChannelOK[m_iEventConnectCh] = false;
				sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteEncodeChannel(iSelEncodeCh);		
				CloseDrawDib(iCh);
				return false;
			}

			m_pDVRSystem[m_iEventOpenBoard]->m_pMainDlg->SetChannel(true, iCh) ; 
			m_bIsConnectChannelOK[m_iEventConnectCh] = true ;

			bCheckPrimary[iCh] = m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iCh]->m_bCheckPrimary[iCh] ;
			bCheckSecond[iCh] = m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iCh]->m_bCheckSecond[iCh] ;

			bCheckPriH264[iCh] = m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iCh]->m_EncodeSetting.m_bCheckPriH264[iCh];
			bCheckPriMJPEG[iCh] = m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iCh]->m_EncodeSetting.m_bCheckPriMJPEG[iCh];
			bCheckSecH264[iCh] = m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iCh]->m_EncodeSetting.m_bCheckSecH264[iCh];
			bCheckSecMJPEG[iCh] = m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iCh]->m_EncodeSetting.m_bCheckSecMJPEG[iCh];

			GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->Invalidate();
			return true;
		}
		else
		{
			if(bCheckSecond[iCh])
			{
				for(int iSubEncodeCh = 0 ; iSubEncodeCh < MAX_ENCODE_CHANNEL ; iSubEncodeCh++)
				{
					sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StopEncodeChannel(iCh , iSubEncodeCh);
					StopRecording(iCh);
				}
			}
			else if(bCheckPrimary[iCh])
			{
				int iSubEncodeCh = 0 ;
				sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StopEncodeChannel(iCh , iSubEncodeCh);
				StopRecording(iCh);
			}

			m_bIsConnectChannelOK[m_iEventConnectCh] = false;
			m_bIsEncodeChOK[m_iEventEncodeCh] = false;
			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteEncodeChannel(iCh);
		
			GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->Invalidate();
			return false;
		}		
	}
	else
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{			
			int iSelEncodeCh = iCh;
			
			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StopEncodeChannel(iSelEncodeCh , 0);
			StopRecording(iSelEncodeCh);
			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteEncodeChannel(iSelEncodeCh);
			CloseDrawDib(iCh);
		}
	}

	GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->Invalidate();

	if (sdvr_rc != SDVR_ERR_NONE)
	{
		CloseDrawDib(iCh);
		return false;
	}

	return true;
}


//If want to do the decode action for the encode channel
//It is important to enable the decode channel before enabling the encode channel
bool CDvrFCDDlg::SetEncodeChannel(int iCh , bool bRecord)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	int iSelEncodeCh = -1 ;

	if(iCh > 15)
		iCh = iCh % MAX_BOARD_CHANNEL;
			
	if(m_pDVRSystem[m_iEventOpenBoard])
	{
		iSelEncodeCh = iCh ;
	
		if((bRecord)&&(bCheckPrimary[iCh]))
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			char szFileName[100];

			SetEncodePath();

			wsprintf(szFileName, "Board %d PRI_CH%02d-%04d%02d%02d-%02d%02d%02d", m_iEventOpenBoard,
				iSelEncodeCh+1, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			
			sdvr_venc_e EncoderType = m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iCh]->m_chan_def.video_format_primary ;
			if(EncoderType == SDVR_VIDEO_ENC_JPEG)
				StartRecording_Primary(iSelEncodeCh, szFileName, SDVR_VIDEO_ENC_JPEG);
			else if(EncoderType == SDVR_VIDEO_ENC_H264)
				StartRecording_Primary(iSelEncodeCh, szFileName, SDVR_VIDEO_ENC_H264);
						
			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StartEncodeChannel(iSelEncodeCh , SDVR_ENC_PRIMARY);
					
			m_bIsEncodeChOK[m_iEventEncodeCh] = true;
			//return true;
		}
		
		if((bRecord)&&(bCheckSecond[iCh]))
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			char szFileName[100];

			wsprintf(szFileName, "Board %d SEC_CH%02d-%04d%02d%02d-%02d%02d%02d", m_iEventOpenBoard,
			iSelEncodeCh+1, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			sdvr_venc_e EncoderType = m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iCh]->m_chan_def.video_format_secondary ;
			if(EncoderType == SDVR_VIDEO_ENC_JPEG)
				StartRecording_Secondary(iSelEncodeCh, szFileName, SDVR_VIDEO_ENC_JPEG);
			else if(EncoderType == SDVR_VIDEO_ENC_H264)
				StartRecording_Secondary(iSelEncodeCh, szFileName, SDVR_VIDEO_ENC_H264);
			
			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StartEncodeChannel(iSelEncodeCh , SDVR_ENC_SECONDARY);
			m_bIsEncodeChOK[m_iEventEncodeCh] = true;
			return true;
		}
		
		if(!bRecord)
		{
			for(int i = 0 ; i< 2 ; i++)
			{
				sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StopEncodeChannel(iCh , i);
				StopRecording(iCh);
			}

			if(m_bIsUnInitBoard)
			{
				sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteEncodeChannel(iSelEncodeCh);
				CloseDrawDib(iCh);
				m_bIsUnInitBoard = false;
			}

			m_bIsEncodeChOK[m_iEventEncodeCh] = false;
			return false;
		}	
	}
	
	GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->Invalidate();

	if (sdvr_rc != SDVR_ERR_NONE)
	{
		CloseDrawDib(iCh);
		return false;
	}

	return true;
}

bool CDvrFCDDlg::SetInitDvrMenuState(bool bShow, bool bCheck)
{
	if (bShow)
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD , MF_ENABLED);		
	}
	else
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD , MF_GRAYED);		
	}

	if (bCheck)
	{
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD, MF_CHECKED);		
	}
	else
	{
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_ALL_DVRBOARD , MF_UNCHECKED);		
	}	

	return true;
}

bool CDvrFCDDlg::SetEnableChMenuState(bool bShow, bool bCheck, int iCh, int iBoard)
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
						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_01+i, MF_ENABLED);
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_01+i, MF_UNCHECKED);
					}
				}
				if(iBoard ==1)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_17+i, MF_ENABLED);
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_17+i, MF_UNCHECKED);
					}
				}
				if(iBoard ==2)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_33+i, MF_ENABLED);
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_33+i, MF_UNCHECKED);
					}
				}
				if(iBoard ==3)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_49+i, MF_ENABLED);
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_49+i, MF_UNCHECKED);
					}
				}

			}		
		}
		else
		{
			for (i=0; i<MAX_BOARD_CHANNEL; i++)
			{
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_01+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_01+i, MF_UNCHECKED);					
				
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_17+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_17+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_33+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_33+i, MF_UNCHECKED);					

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_49+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_49+i, MF_UNCHECKED);
			}
		}	
	}	
	else
	{
		/*if (m_pDVRSystem[m_iEventOpenBoard] && (iCh>=m_pDVRSystem[m_iEventOpenBoard]->m_iVdoCount))
		{
			return true;
		}*/		

		if(iBoard ==0)
		{
			if (bCheck)
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_01+iCh, MF_CHECKED);
			else
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_01+iCh, MF_UNCHECKED);
		}
		
		if(iBoard ==1)
		{
			if (bCheck)
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_17+iCh, MF_CHECKED);
			else
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_17+iCh, MF_UNCHECKED);
		}
		if(iBoard ==2)
		{
			if (bCheck)
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_33+iCh, MF_CHECKED);
			else
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_33+iCh, MF_UNCHECKED);
		}

		if(iBoard ==3)
		{
			if (bCheck)
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_49+iCh, MF_CHECKED);
			else
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENABLE_CH_49+iCh, MF_UNCHECKED);
		}
		
	}

	return true;
}

bool CDvrFCDDlg::SetEncodeChMenuState(bool bShow, bool bCheck, int iCh , int iBoard)
{
	int i=0;

	if(iCh > 15)
		iCh = iCh % MAX_BOARD_CHANNEL ;	

	if (iCh == -1)
	{
		if (bShow)
		{
			if(iBoard ==0)
			{
				for (i=0; i<MAX_BOARD_CHANNEL; i++)
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+i, MF_ENABLED);
					CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+i, MF_UNCHECKED);
				}
			}
			if(iBoard ==1)
			{
				for (i=0; i<MAX_BOARD_CHANNEL; i++)
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE17+i, MF_ENABLED);
					CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE17+i, MF_UNCHECKED);
				}
			}		
			if(iBoard ==2)
			{
				for (i=0; i<MAX_BOARD_CHANNEL; i++)
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE33+i, MF_ENABLED);
					CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE33+i, MF_UNCHECKED);
				}
			}
			if(iBoard ==3)
			{
				for (i=0; i<MAX_BOARD_CHANNEL; i++)
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE49+i, MF_ENABLED);
					CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE49+i, MF_UNCHECKED);
				}
			}		
		}
		else
		{
			for (i=0; i<MAX_BOARD_CHANNEL; i++)
			{
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+i, MF_UNCHECKED);
		
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE17+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE17+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE33+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE33+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE49+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE49+i, MF_UNCHECKED);
			}		
		}	
	}	
	else
	{
		if(iBoard ==0)
		{
			if (bCheck)
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+iCh, MF_CHECKED);
			else
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+iCh, MF_UNCHECKED);
			
			if (bShow)
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+iCh, MF_ENABLED);
			else
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE01+iCh, MF_GRAYED);
		}
		if(iBoard ==1)
		{
			if (bCheck)
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE17+iCh, MF_CHECKED);
			else
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE17+iCh, MF_UNCHECKED);
			
			if (bShow)
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE17+iCh, MF_ENABLED);
			else
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE17+iCh, MF_GRAYED);
		}
		if(iBoard ==2)
		{
			if (bCheck)
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE33+iCh, MF_CHECKED);
			else
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE33+iCh, MF_UNCHECKED);

			if (bShow)
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE33+iCh, MF_ENABLED);
			else
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE33+iCh, MF_GRAYED);
		}
		if(iBoard ==3)
		{
			if (bCheck)
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE49+iCh, MF_CHECKED);
			else
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE49+iCh, MF_UNCHECKED);

			if (bShow)
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE49+iCh, MF_ENABLED);
			else
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODECHANNEL_ENCODE49+iCh, MF_GRAYED);
		}		
	}

	return true;
}

bool CDvrFCDDlg::SetChannel(bool bConnect, int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

 	if (bConnect)
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{						
			CloseDrawDib(iCh);			
			m_iFrameCount[iCh] = 0;
			InitDrawDib(iCh , m_iEventOpenBoard);						
		}		
	}
	else
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{			
			//sdvr_rc = ResetMotion(iCh);
			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DisConnectChannel(iCh);
			CloseDrawDib(iCh);			
		}
	}
	

	GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->Invalidate();
		
	if (sdvr_rc != SDVR_ERR_NONE)
	{
		CloseDrawDib(iCh);		
		return false;
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
	else if ((wParam >= ID_ENABLE_CH_01) && (wParam <= ID_ENABLE_CH_16))
	{
		m_iEventOpenBoard = 0 ;
 		m_iEventConnectCh = (int)(wParam - ID_ENABLE_CH_01);
		
		if(!m_bIsConnectChannelOK[m_iEventConnectCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_PREVIEW_CH]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_PREVIEW_CH]);
		}	
	}
	else if ((wParam >= ID_ENABLE_CH_17) && (wParam <= ID_ENABLE_CH_32))
	{
		m_iEventOpenBoard = 1 ;
		m_iEventConnectCh = (int)(wParam - ID_ENABLE_CH_01);

		if(!m_bIsConnectChannelOK[m_iEventConnectCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_PREVIEW_CH]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_PREVIEW_CH]);
		}	
	}
	else if ((wParam >= ID_ENABLE_CH_33) && (wParam <= ID_ENABLE_CH_48))
	{
		m_iEventOpenBoard = 2 ;
		m_iEventConnectCh = (int)(wParam - ID_ENABLE_CH_01);

		if(!m_bIsConnectChannelOK[m_iEventConnectCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_PREVIEW_CH]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_PREVIEW_CH]);
		}	
	}
	else if ((wParam >= ID_ENABLE_CH_49) && (wParam <= ID_ENABLE_CH_64))
	{
		m_iEventOpenBoard = 3 ;
		m_iEventConnectCh = (int)(wParam - ID_ENABLE_CH_01);

		if(!m_bIsConnectChannelOK[m_iEventConnectCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_PREVIEW_CH]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_PREVIEW_CH]);
		}	
	}
	else if ((wParam >= ID_ENCODECHANNEL_ENCODE01)&& (wParam <= ID_ENCODECHANNEL_ENCODE16))
	{
		m_iEventOpenBoard = 0 ;
		m_iEventEncodeCh = (int)(wParam - ID_ENCODECHANNEL_ENCODE01);

		if(!m_bIsEncodeChOK[m_iEventEncodeCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_ENCODE_CH]);
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_ENCODE_CH]);
		}	
	}
	else if ((wParam >= ID_ENCODECHANNEL_ENCODE17)&& (wParam <= ID_ENCODECHANNEL_ENCODE32))
	{
		m_iEventOpenBoard = 1 ;
		m_iEventEncodeCh = (int)(wParam - ID_ENCODECHANNEL_ENCODE01);

		if(!m_bIsEncodeChOK[m_iEventEncodeCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_ENCODE_CH]);
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_ENCODE_CH]);
		}	
	}
	else if ((wParam >= ID_ENCODECHANNEL_ENCODE33)&& (wParam <= ID_ENCODECHANNEL_ENCODE48))
	{
		m_iEventOpenBoard = 2 ;
		m_iEventEncodeCh = (int)(wParam - ID_ENCODECHANNEL_ENCODE01);

		if(!m_bIsEncodeChOK[m_iEventEncodeCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_ENCODE_CH]);
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_ENCODE_CH]);
		}	
	}
	else if ((wParam >= ID_ENCODECHANNEL_ENCODE49)&& (wParam <= ID_ENCODECHANNEL_ENCODE64))
	{
		m_iEventOpenBoard = 3 ;
		m_iEventEncodeCh = (int)(wParam - ID_ENCODECHANNEL_ENCODE01);

		if(!m_bIsEncodeChOK[m_iEventEncodeCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_ENCODE_CH]);
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_ENCODE_CH]);
		}	
	}
	
	Invalidate();

	return CDialog::OnCommand(wParam, lParam);
}

HRESULT CDvrFCDDlg::OnModifyChannelSettingDlg(WPARAM wParam, LPARAM lParam)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	int GetChannel = (int)lParam ;
	
	if((bCheckPrimary[GetChannel])&&(bCheckSecond[GetChannel]))
	{
		for(int iSubEnc= 0 ; iSubEnc < MAX_ENCODE_CHANNEL ;iSubEnc++)
		{
			m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[GetChannel]->applyVideoEncoding(iSubEnc,true,true);
		}
	}
	else if(bCheckPrimary[GetChannel])
	{
		int iSubEnc = 0 ;
		m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[GetChannel]->applyVideoEncoding(iSubEnc,true,true);
	}
	else if(bCheckSecond[GetChannel])
	{
		int iSubEnc = 1 ;
		m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[GetChannel]->applyVideoEncoding(iSubEnc,true,true);
	}


	return S_OK;
}


BOOL CDvrFCDDlg::InitDrawDib(int iCh , int iBoard)
{	
	m_pbyYUVBuffer[iCh] = new BYTE[m_iVdoWidth_Raw[iCh]*m_iVdoHeight_Raw[iCh]*2];
	
	m_pDirectDrawDisplay[iCh] = NULL;
	m_pDirectDrawDisplay[iCh] = new CDirectDraw();

	m_bInitDrawOK[iCh] = m_pDirectDrawDisplay[iCh]->InitDirectDraw(m_iVdoWidth_Raw[iCh], 
			m_iVdoHeight_Raw[iCh], GetDlgItem(IDC_STATIC_DISPLAY_01+iCh));
	
	return TRUE;
}

BOOL CDvrFCDDlg::CloseDrawDib(int iCh)
{
	m_bInitDrawOK[iCh] = FALSE;

	if(m_pDirectDrawDisplay[iCh])
	{	
		m_pDirectDrawDisplay[iCh]->CloseDirectDraw();

		delete m_pDirectDrawDisplay[iCh];
		m_pDirectDrawDisplay[iCh] = NULL;
	}

	if (m_pbyYUVBuffer[iCh])
	{
		delete m_pbyYUVBuffer[iCh];
		m_pbyYUVBuffer[iCh] = NULL;
	}

	return TRUE;
}

bool CDvrFCDDlg::acquireBuffer(int iBoard, int iCh, sdvr_frame_type_e frame_type, sx_bool is_primary, sdvr_chan_handle_t handle)
{
	sdvr_av_buffer_t *av_buf;
	sdvr_yuv_buffer_t *yuv_buf;
	sdvr_err_e err;

	char szDebug[200];

	if(m_pDVRSystem[iBoard]->m_pEncodeChParam[iCh])
	{
		if (frame_type == SDVR_FRAME_RAW_VIDEO)//Enable RAW Video
		{
			err = sdvr_get_yuv_buffer(handle, &yuv_buf);

			if (err == SDVR_ERR_NONE && yuv_buf)
			{
				m_iFrameCount[iCh]++;		

				m_iVdo_YBufSize[iCh] = yuv_buf->y_data_size;
				m_iVdo_UBufSize[iCh] = yuv_buf->u_data_size;
				m_iVdo_VBufSize[iCh] = yuv_buf->v_data_size;

				sprintf_s(szDebug, sizeof(szDebug), "CH%d come acquireBuffer(), frame_type = %d, m_iFrameCount = %d, size y(%d), u(%d), v(%d)\n", 
					iCh, yuv_buf->frame_type, m_iFrameCount[iCh], yuv_buf->y_data_size, yuv_buf->u_data_size, yuv_buf->v_data_size);
				if(m_bInitDrawOK[iCh])
				{
					if(m_bShowBoard[iBoard])
						DrawVideoYUV(iCh, (char*)yuv_buf->y_data, (char*)yuv_buf->u_data, (char*)yuv_buf->v_data);
				}

				sdvr_release_yuv_buffer(yuv_buf);
			}
		}
		else if (frame_type == SDVR_FRAME_RAW_AUDIO)
		{
			err = sdvr_get_av_buffer(handle, SDVR_FRAME_RAW_AUDIO, &av_buf);
			if (err == SDVR_ERR_NONE)
			{
				sdvr_release_av_buffer(av_buf);
			}			
		}
		else if ((frame_type == SDVR_FRAME_H264_IDR) || (frame_type == SDVR_FRAME_H264_I) ||
				(frame_type == SDVR_FRAME_H264_B) || (frame_type == SDVR_FRAME_H264_SPS) ||
				(frame_type == SDVR_FRAME_H264_PPS)|| (frame_type == SDVR_FRAME_MPEG4_I) ||
				(frame_type == SDVR_FRAME_MPEG4_P) || (frame_type == SDVR_FRAME_MPEG4_B) ||
				(frame_type == SDVR_FRAME_MPEG4_VOL) || (frame_type == SDVR_FRAME_H264_P) )
		{

			if(is_primary == 0)
			{
				err = sdvr_get_av_buffer(handle,SDVR_FRAME_VIDEO_ENCODED_SECONDARY, &av_buf);

				if (err == SDVR_ERR_NONE && av_buf)
				{
					PutSecondaryMp4Packet(iCh, frame_type, av_buf);
					sdvr_release_av_buffer(av_buf);
				}				
			}

			if(is_primary == 1)
			{			
				err = sdvr_get_av_buffer(handle,SDVR_FRAME_VIDEO_ENCODED_PRIMARY, &av_buf);
				if (err == SDVR_ERR_NONE && av_buf)
				{
					PutPrimaryMp4Packet(iCh, frame_type, av_buf);
					sdvr_release_av_buffer(av_buf);
				}
			}
		
		}
		else if ((frame_type == SDVR_FRAME_G711) || (frame_type == SDVR_FRAME_AUDIO_ENCODED))
		{
			err = sdvr_get_av_buffer(handle, SDVR_FRAME_AUDIO_ENCODED, &av_buf);

			if (err == SDVR_ERR_NONE && av_buf)
			{
				PutEncAudioPacket(iCh, frame_type, av_buf);
				sdvr_release_av_buffer(av_buf);
			}
					
		}
		else if (frame_type == SDVR_FRAME_JPEG)
		{
			
			if(is_primary == 0)
			{
				err = sdvr_get_av_buffer(handle,SDVR_FRAME_VIDEO_ENCODED_SECONDARY, &av_buf);
				if (err == SDVR_ERR_NONE && av_buf)
				{
					PutEncSecondaryMJPEGPacket(iCh, frame_type, av_buf);
					sdvr_release_av_buffer(av_buf);
				}
			}

			if(is_primary == 1)
			{
				err = sdvr_get_av_buffer(handle,SDVR_FRAME_VIDEO_ENCODED_PRIMARY, &av_buf);
				
				if (err == SDVR_ERR_NONE && av_buf)
				{
					PutEncPrimaryMJPEGPacket(iCh, frame_type, av_buf);
					sdvr_release_av_buffer(av_buf);
				}
			}
		}
		sprintf_s(szDebug, sizeof(szDebug),
			"CH%d come acquireBuffer(), frame_type(%d)\n",iCh, frame_type);
			//OutputDebugString(szDebug);
	}

	return true;
}

void CDvrFCDDlg::DrawVideoYUV(int iCh, char * y, char * u, char *v)
{
	CWnd* cWndIDisplayItem = GetDlgItem(IDC_STATIC_DISPLAY_01+iCh);

	m_pDirectDrawDisplay[iCh]->DrawYUV(y, u, v, m_iVdoWidth_Raw[iCh], m_iVdoHeight_Raw[iCh], cWndIDisplayItem);
}

bool CDvrFCDDlg::StartRecording_Primary(int iCh, char szFileName[MAX_PATH], sdvr_venc_e EncoderType)
{
	if(EncoderType == SDVR_VIDEO_ENC_NONE)
		return false;

	int	iFrameRate = 30;
	if(m_pDVRSystem[m_iEventOpenBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_NTSC)
	{
		iFrameRate = 30;
	}
	else if(m_pDVRSystem[m_iEventOpenBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_PAL)
	{
		iFrameRate = 25;
	}

	if((EncoderType == SDVR_VIDEO_ENC_H264) || (EncoderType == SDVR_VIDEO_ENC_MPEG4))
	{
		strcat(szFileName, ".mov");
		mp4_track_info_t track;
		mp4_info_t info;

		memset(&info, 0, sizeof(info));

		m_pMp4SaveFile[iCh] = new mp4File_t;

		m_pMp4SaveFile[iCh]->vtrack = -1;
		m_pMp4SaveFile[iCh]->atrack = -1;
		m_pMp4SaveFile[iCh]->FileHandle = 0;

		mp4_packet_t	pkt_v;
		
		info.file_name = szFileName;
		info.mux_mode = MP4_RECORD;
		info.file_mode = MODE_MOV;

		// Timescale is based on 90k Hz clock.
		m_pMp4SaveFile[iCh]->FileHandle = mp4mux_open(&info);

		if (m_pMp4SaveFile[iCh]->FileHandle == 0) 
		{
			delete m_pMp4SaveFile[iCh];
			m_pMp4SaveFile[iCh] = NULL;

			char szMsg[_MAX_PATH];
			wsprintf(szMsg, "Error: failed to open file(%s) for recording.", szFileName);
			MessageBox(szMsg);

			return false;
		}

		// Add Video Track to mp4 file
		memset(&track, 0, sizeof(mp4_track_info_t));
		track.type = MP4_TRACK_VIDEO;
		track.codec_id = CODEC_ID_H264; //(venc == SDVR_VIDEO_ENC_H264) ? CODEC_ID_H264 : CODEC_ID_MPEG4;
		
		// Timescale is based on 90k Hz clock.
		track.timescale = 90000; //30000; 
		track.width = m_iVdoWidth_Enc[iCh];
		
		// Make sure that the video height is multiple of 16;
		int vHeight;
		vHeight =  m_iVdoHeight_Enc[iCh];;
		vHeight /=  16;
		vHeight *= 16;
		track.height = vHeight;
		track.sample_size = 0;
		track.language = 0;

		if(m_pDVRSystem[m_iEventOpenBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_NTSC)
		{
			track.default_duration = 3003; // 3003 for NTSC, 3600 fo PAL
		}
		else if(m_pDVRSystem[m_iEventOpenBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_PAL)
		{
			track.default_duration = 3600; // 3003 for NTSC, 3600 fo PAL
		}

		track.default_size = 0; // variable frame size

		if (mp4mux_add_track(m_pMp4SaveFile[iCh]->FileHandle, &track) == MP4MUX_ERR_OK)
		{
			m_pMp4SaveFile[iCh]->vtrack = track.track_id;
		}
		else
		{
			mp4mux_close(m_pMp4SaveFile[iCh]->FileHandle);

			delete m_pMp4SaveFile[iCh];
			m_pMp4SaveFile[iCh] = NULL;

			char szMsg[_MAX_PATH];
			wsprintf(szMsg, "Error: failed to add video track for file(%s)", szFileName);
			MessageBox(szMsg);

			return false;
		}

		// Add audio Track to mp4 file
		sx_int8  mp4mux_lang[4] = {'e', 'n', 'g', '\0'};
		memset(&track, 0, sizeof(mp4_track_info_t));
		track.type = MP4_TRACK_AUDIO;
		track.codec_id = CODEC_ID_PCM_MULAW;

		// Audio sampling rate
		track.timescale = 8000;
		track.channels = 2; // a stereo recording
		track.sample_size = 1 * track.channels;
		track.default_duration = 2560 / track.channels; // packet size in bytes / channels
		track.default_size = 1 * track.channels; // one byte per channel for PCM

		track.language = mp4mux_iso639_to_lang(mp4mux_lang, 1);

		// add track
		if (mp4mux_add_track(m_pMp4SaveFile[iCh]->FileHandle, &track) == MP4MUX_ERR_OK)
		{
			m_pMp4SaveFile[iCh]->atrack = track.track_id;
		}
		else
		{
			mp4mux_close(m_pMp4SaveFile[iCh]->FileHandle);

			delete m_pMp4SaveFile[iCh];
			m_pMp4SaveFile[iCh] = NULL;

			char szMsg[_MAX_PATH];
			wsprintf(szMsg, "Error: failed to add audio track for file(%s)", szFileName);
			MessageBox(szMsg);

			return false;
		}
	}

	if(EncoderType == SDVR_VIDEO_ENC_JPEG)
	{
		strcat(szFileName, ".avi");

		m_pAviSaveFile[iCh] = new aviFile_t;

		m_pAviSaveFile[iCh]->frameLen = 0;
		// TODO: For now we are limiting each avi file to 
		//       1 Hr worth of frames.
		m_pAviSaveFile[iCh]->maxFramesAllowed = iFrameRate * 60 * 60;
		
		m_pAviSaveFile[iCh]->totalFramesWrtn = 0;

		// save the mjpeg frames into an avi file.
		{
			int scale  = 1;     // We assume scale is always 1

			if (!avi_open(m_pAviSaveFile[iCh], szFileName, scale, iFrameRate, 
							m_iVdoWidth_Enc[iCh], m_iVdoHeight_Enc[iCh], m_pAviSaveFile[iCh]->maxFramesAllowed))
			{
				delete m_pAviSaveFile[iCh];
				m_pAviSaveFile[iCh] = NULL;

				char szMsg[_MAX_PATH];
				wsprintf(szMsg, "Error: failed to open file(%s) for recording.", szFileName);
				MessageBox(szMsg);

				return false;
			}
		}
	}

	return true;
}


bool CDvrFCDDlg::StartRecording_Secondary(int iCh, char szFileName[MAX_PATH], sdvr_venc_e EncoderType)
{
	if(EncoderType == SDVR_VIDEO_ENC_NONE)
		return false;

	int	iFrameRate = 30;
	if(m_pDVRSystem[m_iEventOpenBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_NTSC)
	{
		iFrameRate = 30;
	}
	else if(m_pDVRSystem[m_iEventOpenBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_PAL)
	{
		iFrameRate = 25;
	}

	if((EncoderType == SDVR_VIDEO_ENC_H264) || (EncoderType == SDVR_VIDEO_ENC_MPEG4))
	{
		strcat(szFileName, ".mov");
		mp4_track_info_t track_sec;
		mp4_info_t info_sec;

		memset(&info_sec, 0, sizeof(info_sec));

		m_pMp4SaveFile_sec[iCh] = new mp4File_t_sec;

		m_pMp4SaveFile_sec[iCh]->vtrack = -1;
		m_pMp4SaveFile_sec[iCh]->atrack = -1;
		m_pMp4SaveFile_sec[iCh]->FileHandle = 0;

		mp4_packet_t	pkt_v_sec;
		
		info_sec.file_name = szFileName;
		info_sec.mux_mode = MP4_RECORD;
		info_sec.file_mode = MODE_MOV;

		// Timescale is based on 90k Hz clock.
		m_pMp4SaveFile_sec[iCh]->FileHandle = mp4mux_open(&info_sec);

		if (m_pMp4SaveFile_sec[iCh]->FileHandle == 0) 
		{
			delete m_pMp4SaveFile_sec[iCh];
			m_pMp4SaveFile_sec[iCh] = NULL;

			char szMsg[_MAX_PATH];
			wsprintf(szMsg, "Error: failed to open file(%s) for recording.", szFileName);
			MessageBox(szMsg);

			return false;
		}

		// Add Video Track to mp4 file
		memset(&track_sec, 0, sizeof(mp4_track_info_t));
		track_sec.type = MP4_TRACK_VIDEO;
		track_sec.codec_id = CODEC_ID_H264; //(venc == SDVR_VIDEO_ENC_H264) ? CODEC_ID_H264 : CODEC_ID_MPEG4;
		
		// Timescale is based on 90k Hz clock.
		track_sec.timescale = 90000; //30000; 
		track_sec.width = m_iVdoWidth_Enc[iCh];

		// Make sure that the video height is multiple of 16;
		int vHeight;
		vHeight =  m_iVdoHeight_Enc[iCh];;
		vHeight /=  16;
		vHeight *= 16;
		track_sec.height = vHeight;
		track_sec.sample_size = 0;
		track_sec.language = 0;

		if(m_pDVRSystem[m_iEventOpenBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_NTSC)
		{
			track_sec.default_duration = 3003; // 3003 for NTSC, 3600 fo PAL
		}
		else if(m_pDVRSystem[m_iEventOpenBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_PAL)
		{
			track_sec.default_duration = 3600; // 3003 for NTSC, 3600 fo PAL
		}

		track_sec.default_size = 0; // variable frame size

		if (mp4mux_add_track(m_pMp4SaveFile_sec[iCh]->FileHandle, &track_sec) == MP4MUX_ERR_OK)
		{
			m_pMp4SaveFile_sec[iCh]->vtrack = track_sec.track_id;
		}
		else
		{
			mp4mux_close(m_pMp4SaveFile_sec[iCh]->FileHandle);

			delete m_pMp4SaveFile_sec[iCh];
			m_pMp4SaveFile_sec[iCh] = NULL;

			char szMsg[_MAX_PATH];
			wsprintf(szMsg, "Error: failed to add video track for file(%s)", szFileName);
			MessageBox(szMsg);

			return false;
		}

		// Add audio Track to mp4 file
		sx_int8  mp4mux_lang_sec[4] = {'e', 'n', 'g', '\0'};
		memset(&track_sec, 0, sizeof(mp4_track_info_t));
		track_sec.type = MP4_TRACK_AUDIO;
		track_sec.codec_id = CODEC_ID_PCM_MULAW;

		// Audio sampling rate
		track_sec.timescale = 8000;
		track_sec.channels = 2; // a stereo recording
		track_sec.sample_size = 1 * track_sec.channels;
		track_sec.default_duration = 2560 / track_sec.channels; // packet size in bytes / channels
		track_sec.default_size = 1 * track_sec.channels; // one byte per channel for PCM

		track_sec.language = mp4mux_iso639_to_lang(mp4mux_lang_sec, 1);

		// add track
		if (mp4mux_add_track(m_pMp4SaveFile_sec[iCh]->FileHandle, &track_sec) == MP4MUX_ERR_OK)
		{
			m_pMp4SaveFile_sec[iCh]->atrack = track_sec.track_id;
		}
		else
		{
			mp4mux_close(m_pMp4SaveFile_sec[iCh]->FileHandle);

			delete m_pMp4SaveFile_sec[iCh];
			m_pMp4SaveFile_sec[iCh] = NULL;

			char szMsg[_MAX_PATH];
			wsprintf(szMsg, "Error: failed to add audio track for file(%s)", szFileName);
			MessageBox(szMsg);

			return false;
		}
	}

	if(EncoderType == SDVR_VIDEO_ENC_JPEG)
	{
		strcat(szFileName, ".avi");

		m_pAviSaveFile_sec[iCh] = new aviFile_t_sec;

		m_pAviSaveFile_sec[iCh]->frameLen = 0;
		// TODO: For now we are limiting each avi file to 
		//       1 Hr worth of frames.
		m_pAviSaveFile_sec[iCh]->maxFramesAllowed = iFrameRate * 60 * 60;
		
		m_pAviSaveFile_sec[iCh]->totalFramesWrtn = 0;

		// save the mjpeg frames into an avi file.
		{
			int scale  = 1;     // We assume scale is always 1

			if (!avi_open_sec(m_pAviSaveFile_sec[iCh], szFileName, scale, iFrameRate, 
							m_iVdoWidth_Enc[iCh], m_iVdoHeight_Enc[iCh], m_pAviSaveFile_sec[iCh]->maxFramesAllowed))
			{
				delete m_pAviSaveFile_sec[iCh];
				m_pAviSaveFile_sec[iCh] = NULL;

				char szMsg[_MAX_PATH];
				wsprintf(szMsg, "Error: failed to open file(%s) for recording.", szFileName);
				MessageBox(szMsg);

				return false;
			}
		}
	}

	return true;
}

bool CDvrFCDDlg::PutSecondaryMp4Packet(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf_sec)
{
	if(!m_pMp4SaveFile_sec[iCh])
		return false;
	if(!m_pMp4SaveFile_sec[iCh]->FileHandle)
		return false;

	sx_int32 err;

	mp4_packet_t pkt_v_sec;
	sdvr_firmware_ver_t  version;

	memset(&pkt_v_sec, 0, sizeof(mp4_packet_t));
	pkt_v_sec.data = buf_sec->payload;
	pkt_v_sec.size = buf_sec->payload_size;
	pkt_v_sec.sample_count = 1;

	// 64 bit timestamp only exists in 3.2.x.x or higher of firmware
	sx_uint64 *ptr64TS;
	ptr64TS = (sx_uint64 *)&buf_sec->timestamp;
	
	pkt_v_sec.pts = pkt_v_sec.dts =  *ptr64TS;

	//pkt_v.pts = pkt_v.dts = (sx_uint64)buf->timestamp | ((sx_uint64)buf->timestamp_high << 32);

	if((buf_sec->frame_type == SDVR_FRAME_H264_IDR) ||
		(buf_sec->frame_type == SDVR_FRAME_MPEG4_I))
	{
		pkt_v_sec.flags |= PKT_FLAG_KEY_FRAME;
	}	

	err = mp4mux_put_packet(m_pMp4SaveFile_sec[iCh]->FileHandle, m_pMp4SaveFile_sec[iCh]->vtrack, &pkt_v_sec);

	return true;
}

bool CDvrFCDDlg::PutPrimaryMp4Packet(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf)
{
	if(!m_pMp4SaveFile[iCh])
		return false;
	if(!m_pMp4SaveFile[iCh]->FileHandle)
		return false;

	sx_int32 err;

	mp4_packet_t pkt_v;
	sdvr_firmware_ver_t  version;

	memset(&pkt_v, 0, sizeof(mp4_packet_t));
	pkt_v.data = buf->payload;
	pkt_v.size = buf->payload_size;
	pkt_v.sample_count = 1;      // set always to 1

	// 64 bit timestamp only exists in 3.2.x.x or higher of firmware
	sx_uint64 *ptr64TS;
	ptr64TS = (sx_uint64 *)&buf->timestamp;
	pkt_v.pts = pkt_v.dts =  *ptr64TS; //buf->timestamp;  //(*i).mp4File.dts;

	//pkt_v.pts = pkt_v.dts = (sx_uint64)buf->timestamp | ((sx_uint64)buf->timestamp_high << 32);

	if((buf->frame_type == SDVR_FRAME_H264_IDR) ||
		(buf->frame_type == SDVR_FRAME_MPEG4_I))
	{
		pkt_v.flags |= PKT_FLAG_KEY_FRAME;
	}
	
	err = mp4mux_put_packet(m_pMp4SaveFile[iCh]->FileHandle, m_pMp4SaveFile[iCh]->vtrack, &pkt_v);

	return true;
}

bool CDvrFCDDlg::PutEncAudioPacket(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf)
{
	if(!m_pMp4SaveFile[iCh])
		return false;
	if(!m_pMp4SaveFile[iCh]->FileHandle)
		return false;

	sdvr_aenc_e  aenc = SDVR_AUDIO_ENC_G711;
	sdvr_firmware_ver_t  version;

	// NOTE: TODO: Currently we are saving audio frame only with
	//       h.264 & MPEG4 video frames. The audio will be ignored for
	//       Any other video codec types.

	if(aenc == SDVR_AUDIO_ENC_G711)
	{
		mp4_packet_t pkt_a;

		memset(&pkt_a, 0, sizeof(mp4_packet_t));
		pkt_a.data = buf->payload;
		pkt_a.size = buf->payload_size;

		// The 64 bit timestamp only exist in 3.2.x.x or higher of firmware
		pkt_a.pts = pkt_a.dts = (sx_uint64)buf->timestamp | ((sx_uint64)buf->timestamp_high << 32);

		// The payload contains 2560 samples of 8 bytes. The payload_size
		// was already re-asjusted in the SDK when it was decoded.
		pkt_a.sample_count = buf->payload_size;

		pkt_a.flags = 0;        // This field is unused for audio.

		mp4mux_put_packet(m_pMp4SaveFile[iCh]->FileHandle, m_pMp4SaveFile[iCh]->atrack, &pkt_a);
	}

	return true;
}

bool CDvrFCDDlg::PutEncPrimaryMJPEGPacket(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf)
{
	if(!m_pAviSaveFile[iCh])
		return false;

	if (frame_type == SDVR_FRAME_JPEG)
	{
		// since we pre allocate a buffer to hold the jpeg
		// image indecies, we should check to not record
		// more than we can save the image indecies.
		if (m_pAviSaveFile[iCh]->totalFramesWrtn < m_pAviSaveFile[iCh]->maxFramesAllowed)
		{
			avi_add(m_pAviSaveFile[iCh], buf->payload, buf->payload_size);

			return true;
		}
		else
		{
			char szMsg[_MAX_PATH];
			wsprintf(szMsg, "Error: failed to add video(M-Jpeg) for file, over Max Frame(%d)\n", 
					m_pAviSaveFile[iCh]->maxFramesAllowed);
			OutputDebugString(szMsg);
		}
	}

	return false;
}

bool CDvrFCDDlg::PutEncSecondaryMJPEGPacket(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf)
{
	if(!m_pAviSaveFile_sec[iCh])
		return false;

	if (frame_type == SDVR_FRAME_JPEG)
	{
		// since we pre allocate a buffer to hold the jpeg
		// image indecies, we should check to not record
		// more than we can save the image indecies.
		if (m_pAviSaveFile_sec[iCh]->totalFramesWrtn < m_pAviSaveFile_sec[iCh]->maxFramesAllowed)
		{
			avi_add_sec(m_pAviSaveFile_sec[iCh], buf->payload, buf->payload_size);

			return true;
		}
		else
		{
			char szMsg[_MAX_PATH];
			wsprintf(szMsg, "Error: failed to add video(M-Jpeg) for file, over Max Frame(%d)\n", 
					m_pAviSaveFile_sec[iCh]->maxFramesAllowed);
			OutputDebugString(szMsg);
		}
	}

	return false;
}



bool CDvrFCDDlg::StopRecording(int iCh)
{
	if((m_pMp4SaveFile[iCh]) && (m_pMp4SaveFile[iCh]->FileHandle))
	{
		mp4mux_close(m_pMp4SaveFile[iCh]->FileHandle);
		delete m_pMp4SaveFile[iCh];
		m_pMp4SaveFile[iCh] = NULL;

		return true;
	}

	if((m_pMp4SaveFile_sec[iCh]) && (m_pMp4SaveFile_sec[iCh]->FileHandle))
	{
		mp4mux_close(m_pMp4SaveFile_sec[iCh]->FileHandle);
		delete m_pMp4SaveFile_sec[iCh];
		m_pMp4SaveFile_sec[iCh] = NULL;

		return true;
	}

	if(m_pAviSaveFile[iCh])
	{
		avi_close(m_pAviSaveFile[iCh]);

		delete m_pAviSaveFile[iCh];
		m_pAviSaveFile[iCh] = NULL;

		return true;
	}

	if(m_pAviSaveFile_sec[iCh])
	{
		avi_close_sec(m_pAviSaveFile_sec[iCh]);

		delete m_pAviSaveFile_sec[iCh];
		m_pAviSaveFile_sec[iCh] = NULL;

		return true;
	}

}

BOOL CDvrFCDDlg::SetEncodePath()
{
	char DirName[256];
	char* p = "C:\\EncodeDvrFCD Record";
	char* q = DirName; 

	while(*p)
	{
		if (('\\' == *p) || ('/' == *p))
		{
			if (':' != *(p-1))
			{
				CreateDirectory(DirName, NULL);
			}
		}
		*q++ = *p++;
		*q = '\0';
	}

	CreateDirectory(DirName, NULL);

	SetCurrentDirectory(DirName);

return true;

}

void CDvrFCDDlg::OnBnClickedRadioBoard1()
{
	memset(&m_bShowBoard,0,sizeof(m_bShowBoard));
	m_bShowBoard[0]= true ;
}

void CDvrFCDDlg::OnBnClickedRadioBoard2()
{
	memset(&m_bShowBoard,0,sizeof(m_bShowBoard));
	m_bShowBoard[1]= true ;
}

void CDvrFCDDlg::OnBnClickedRadioBoard3()
{
	memset(&m_bShowBoard,0,sizeof(m_bShowBoard));
	m_bShowBoard[2]= true ;
}

void CDvrFCDDlg::OnBnClickedRadioBoard4()
{
	memset(&m_bShowBoard,0,sizeof(m_bShowBoard));
	m_bShowBoard[3]= true ;
}
