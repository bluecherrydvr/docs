// DvrFCDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PlayDvrFCD.h"
#include "PlayDvrFCDDlg.h"

#include "DecodeSetupDlg.h"

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
	int i=0, j=0;
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
		//m_bIsDecodeChOK[i] = false;		
		m_iVdoWidth_Raw[i] = 720;
		m_iVdoHeight_Raw[i] = 480;

		for (j=0; j<MAX_SUPPORT_BOARD; j++)
		{
			m_pbyYUVBuffer[j][i] = NULL;	
			m_pDirectDrawDisplay[j][i] = NULL;
			m_bInitDrawOK[j][i] = FALSE;		
		}			
	}

	for(i=0 ; i< MAX_SUPPORT_CHANNEL ; i++)
	{
		m_bIsEncodeChOK[i] = false;
		m_bIsDecodeChOK[i] = false;

		m_pMp4OpenFile[i] = NULL;
		m_PlaySeq[i] = 1;
		m_iSendCounter[i] = 0;

		m_hThreadGetMovPacketData[i] = NULL;
		m_bOpenMovFileReady[i] = false;		
	}

	m_pWaveOut = NULL;
	m_iPlayAudioCh = -1;

	m_hUIControlThread = NULL;

	m_iEventOpenBoard = m_iEventEncodeCh = -1;	

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

	SetMenu(m_pMenu);	// Add the new menu

	int i=0;

	for(i=0; i<MAX_BOARD_CHANNEL; i++)
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODE_CH_01+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_ENCODE_CH_01+i, MF_UNCHECKED);

		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_01+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_01+i, MF_UNCHECKED);
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_17+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_17+i, MF_UNCHECKED);
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_33+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_33+i, MF_UNCHECKED);
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_49+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_49+i, MF_UNCHECKED);

		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_UNCHECKED);
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_17+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_17+i, MF_UNCHECKED);
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_33+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_33+i, MF_UNCHECKED);
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_49+i, MF_GRAYED);
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_49+i, MF_UNCHECKED);
	}

	for(i=0 ; i<MAX_SUPPORT_BOARD; i++)
	{
		GetDlgItem(IDC_RADIO_BOARD1 + i)->ShowWindow(SW_HIDE);
	}

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

	m_hUIControlThread = (HANDLE) CreateThread(NULL, 0, Thread_UIControl, this, 0, &dwID);

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
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_sdk_init() = %d\n", sdvr_rc);
		OutputDebugString(szDebug);
	}

	m_iBoardCount = sdvr_get_board_count();

	if (m_iBoardCount>=1)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_get_board_count() = %d\n", m_iBoardCount);
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

	EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_DVRBOARD, MF_ENABLED);
	CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_DVRBOARD, MF_UNCHECKED);
	
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
		//if((m_iEventOpenBoard != -1) &&
		//	(m_pDVRSystem[m_iEventOpenBoard]))
		{
			int iBoard = 0;
			for (iBoard = 0; iBoard < MAX_SUPPORT_BOARD; iBoard++)
			{
				if(m_bShowBoard[iBoard])
				{
					int iCh = 0;
					for (iCh = 0; iCh < MAX_BOARD_CHANNEL; iCh++)
					{
						if(m_pbyYUVBuffer[iBoard][iCh])
						{
							DrawVideo(iCh, m_pbyYUVBuffer[iBoard][iCh], iBoard);
						}
					}
				}
			}
		}

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

	/*
	CSetVdoStdDlg	SetVdoStdDlg;
	SetVdoStdDlg.DoModal();

	if(SetVdoStdDlg.m_iVdoStdFmt == 0)
	{
		m_pDVRSystem[iBoardIndex]->m_VideoStandard = SDVR_VIDEO_STD_D1_NTSC;
	}
	else if(SetVdoStdDlg.m_iVdoStdFmt == 1)
	{
		m_pDVRSystem[iBoardIndex]->m_VideoStandard = SDVR_VIDEO_STD_D1_PAL;
	}
	*/

	//m_pDVRSystem[iBoardIndex]->m_VideoStandard = SDVR_VIDEO_STD_D1_NTSC;

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
	int iStartCh = MAX_BOARD_CHANNEL*iBoardIndex;

	if (m_pDVRSystem[iBoardIndex])
	{
		for (i=iStartCh; i<(iStartCh+MAX_BOARD_CHANNEL); i++)
		{		
			if(m_bIsDecodeChOK[i])
			{
				SetDecodeChannel(false, i);
				Sleep(10);
				m_bIsDecodeChOK[i] = false;
			}
		}

		sdvr_rc = m_pDVRSystem[iBoardIndex]->CloseBoard();

		delete m_pDVRSystem[iBoardIndex];
		m_pDVRSystem[iBoardIndex] = NULL;		
	}			

	return sdvr_rc;
}

void CDvrFCDDlg::OnClose()
{		
	OutputDebugString("OnClose()\n");

	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[64];

	int i=0, j=0;
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
		for (j=0; j<MAX_SUPPORT_BOARD; j++)		
			CloseDrawDib(i, j);
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
	int      iBoardIndex = 0;
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
					pMainDlg->SetDecodeChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1 , -1);

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
							pMainDlg->SetDecodeChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1 , i);

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
					pMainDlg->SetDecodeChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1, -1);

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
					pMainDlg->SetDecodeChMenuState(pMainDlg->m_bIsInitDvrBoardOK[iBoardCount], false, -1, iBoardCount);
					pMainDlg->m_bIsInitAllDvrBoardOK = false;
				}		
				pMainDlg->m_iEventOpenBoard = iOriEventOpenBoard;
			}
			else if (EventID == ENAME_START_DECODE_CH)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_START_DECODE_CH]);

				pMainDlg->SetDecodeChMenuState(true, pMainDlg->m_bIsDecodeChOK[pMainDlg->m_iEventDecodeCh],
					                           pMainDlg->m_iEventDecodeCh ,pMainDlg->m_iEventOpenBoard );
				
				if (pMainDlg->SetDecodeChannel(true, pMainDlg->m_iEventDecodeCh))
				{
					pMainDlg->m_bIsDecodeChOK[pMainDlg->m_iEventDecodeCh] = true;
				}
				else
				{
					pMainDlg->m_bIsDecodeChOK[pMainDlg->m_iEventDecodeCh] = false;
				}		

				pMainDlg->SetDecodeChMenuState(true, pMainDlg->m_bIsDecodeChOK[pMainDlg->m_iEventDecodeCh], 
					                            pMainDlg->m_iEventDecodeCh,pMainDlg->m_iEventOpenBoard);
			}
			else if (EventID == ENAME_STOP_DECODE_CH)
			{
				ResetEvent(pMainDlg->m_hEventUIControl[ENAME_STOP_DECODE_CH]);

				pMainDlg->SetDecodeChMenuState(true, pMainDlg->m_bIsDecodeChOK[pMainDlg->m_iEventDecodeCh],
					                           pMainDlg->m_iEventDecodeCh ,pMainDlg->m_iEventOpenBoard);

				if (pMainDlg->SetDecodeChannel(false, pMainDlg->m_iEventDecodeCh))	// disconnect ok.
				{
					pMainDlg->m_bIsDecodeChOK[pMainDlg->m_iEventDecodeCh] = false;					
				}
				else	// disconnect fail.
				{
					pMainDlg->m_bIsDecodeChOK[pMainDlg->m_iEventDecodeCh] = true;
				}

				pMainDlg->SetDecodeChMenuState(true, pMainDlg->m_bIsDecodeChOK[pMainDlg->m_iEventDecodeCh], 
					                           pMainDlg->m_iEventDecodeCh ,pMainDlg->m_iEventOpenBoard);
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
	if (g_pMainDlg->m_pDVRSystem[iBoardIndex]->m_pEncodeChParam[iCh])//Encode Channel
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
	if (g_pMainDlg->m_pDVRSystem[iBoardIndex]->m_pDecodeChParam[iCh])//Decode Channel
	{
		if (g_pMainDlg->m_pDVRSystem[iBoardIndex]->m_pDecodeChParam[iCh]->m_bIsClosingChannel)
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

			g_pMainDlg->m_pDVRSystem[iBoardIndex]->m_pDecodeChParam[iCh]->m_bIsClosingChannel = false;

			return;
		}
	}

	switch (frame_type)
	{
	case SDVR_FRAME_RAW_AUDIO:		
	case SDVR_FRAME_RAW_VIDEO:
		{
		g_pMainDlg->acquireBuffer(iBoardIndex, iCh, frame_type, is_primary, handle);
		break;
		}
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

bool CDvrFCDDlg::DeterminIfDoDecodeAction(int iCh, bool *bIsHwDecodeAction, bool *bIsDecodeFile, 
										  int *iSelEncodeCh, char szFileFullPath[_MAX_PATH])
{
	bool				bRtn = false;
	strcpy(szFileFullPath, "");

	SetCurrentDirectory(m_szCurExeDir);

	CDecodeSetupDlg		DecodeSetupDlg(this);

	DecodeSetupDlg.m_iDecodeChIdx = iCh;

	if(DecodeSetupDlg.DoModal())
	{
		*bIsHwDecodeAction = !DecodeSetupDlg.m_bDecodeType;

		if(DecodeSetupDlg.m_bInputType == FALSE) //Decode FileName Channel
		{
			if(strcmp(DecodeSetupDlg.m_sFileFullPath.operator LPCTSTR(), "") != NULL) //Decode Actual File
			{
				*bIsDecodeFile = true;
				strcpy(szFileFullPath, DecodeSetupDlg.m_sFileFullPath.operator LPCTSTR());

				bRtn = true;
			}
		}
		else	//Decode the Encode Channel
		{
			int iSelEncodeCHIdx = DecodeSetupDlg.m_iSelEncodeCHIdx;
			if(iSelEncodeCHIdx != -1)
			{
				*iSelEncodeCh = iSelEncodeCHIdx;
				*bIsDecodeFile = false;
				bRtn = true;
			}
		}
	}

	SetCurrentDirectory(m_szCurExeDir);

	return bRtn;
}

//If want to do the decode action for the encode channel
//It is important to enable the decode channel before enabling the encode channel
bool CDvrFCDDlg::SetDecodeChannel(bool bConnect, int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	if(iCh > 15)
		iCh = iCh % MAX_BOARD_CHANNEL;

	if (bConnect)
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{		
			bool	bIsDecodeFile = false;
			int		iSelEncodeCh = -1;
			bool	bIsHwDecodeAction = true;
			char	szFileFullPath[_MAX_PATH];

			memset(szFileFullPath, 0, sizeof(szFileFullPath));

			if(DeterminIfDoDecodeAction(iCh, &bIsHwDecodeAction, &bIsDecodeFile, &iSelEncodeCh, szFileFullPath))
			{
				sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->CreateDecodeChannel(iCh);

				if (sdvr_rc == SDVR_ERR_NONE)
				{
					CloseDrawDib(iCh, m_iEventOpenBoard);
					
					Sleep(50);

					InitDrawDib(iCh , m_iEventOpenBoard);

					//Decode Way: HW or SW
					m_pDVRSystem[m_iEventOpenBoard]->m_pDecodeChParam[iCh]->m_bIsHwDecodeAction = bIsHwDecodeAction;

					//Each Chip: Max => Two Encode actions and two Decode actions
					//If you want to decode the Encode Channel, 
					//you must create decode channel first before creating the encode channel
					if(bIsDecodeFile == false) //Decode the Encode Channel Data
					{
						sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StartDecodeChannel(iCh);
						if(sdvr_rc == SDVR_ERR_NONE)
						{
							m_pDVRSystem[m_iEventOpenBoard]->m_pDecodeChParam[iCh]->m_iSelEncodeCHIdx = iSelEncodeCh;

							//The Encode channel is running now
							if(m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iSelEncodeCh])
							{
							}
							else //The Encode channel isn't running now, So must start it automatically
							{
								///////////////////////////////////////////////////////////////////
								//Start Encode Channel
								sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->CreateEncodeChannel(iSelEncodeCh);

								if (sdvr_rc == SDVR_ERR_NONE)
								{
									m_bIsEncodeChOK[m_iEventOpenBoard*16+iSelEncodeCh] = true;
								}
								else
								{
									m_bIsEncodeChOK[m_iEventOpenBoard*16+iSelEncodeCh] = false;

									sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StopDecodeChannel(iCh);
									sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteDecodeChannel(iCh);

									CloseDrawDib(m_iEventOpenBoard, iCh);

									return false;
								}		
								///////////////////////////////////////////////////////////////////
							}

							m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iSelEncodeCh]->m_iDecodeCHIdx = iCh;

							sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StartEncodeChannel(iSelEncodeCh);
							if(sdvr_rc != SDVR_ERR_NONE)
							{
								m_bIsEncodeChOK[m_iEventOpenBoard*16+iSelEncodeCh] = false;

								sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteEncodeChannel(iSelEncodeCh);

								sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StopDecodeChannel(iCh);
								sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteDecodeChannel(iCh);

								CloseDrawDib(m_iEventOpenBoard, iCh);
								return false;
							}
						}
						else
						{
							sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteDecodeChannel(iCh);

							CloseDrawDib(m_iEventOpenBoard, iCh);
							return false;
						}
					}
					else
					{
						OpenMovFile(m_iEventOpenBoard*16+iCh, szFileFullPath);

						sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StartDecodeChannel(iCh);

						if(sdvr_rc != SDVR_ERR_NONE)
						{
							CloseMovFile(m_iEventOpenBoard*16+iCh);

							sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteDecodeChannel(iCh);

							CloseDrawDib(m_iEventOpenBoard, iCh);
							return false;
						}
					}
				}
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{			
			int iSelEncodeCh = -1;

			if(m_pDVRSystem[m_iEventOpenBoard]->m_pDecodeChParam[iCh])
			{
				iSelEncodeCh = m_pDVRSystem[m_iEventOpenBoard]->m_pDecodeChParam[iCh]->m_iSelEncodeCHIdx;
			}

			if(iSelEncodeCh != -1) //No Encode Channel to start recording
			{
				if(m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iSelEncodeCh])
				{
					sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StopEncodeChannel(iSelEncodeCh);
				}
			}

			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->StopDecodeChannel(iCh);

			if(iSelEncodeCh != -1) //If one Encode Channel is recording now
			{
				//StopRecording(m_iEventOpenBoard*16+iCh);
			}

			CloseMovFile(m_iEventOpenBoard*16+iCh);

			sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteDecodeChannel(iCh);

			if(m_pDVRSystem[m_iEventOpenBoard]->m_pEncodeChParam[iSelEncodeCh])
			{
				sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DeleteEncodeChannel(iSelEncodeCh);
			}

			CloseDrawDib(m_iEventOpenBoard, iCh);
		}
	}

	GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->Invalidate();

	if (sdvr_rc != SDVR_ERR_NONE)
	{
		CloseDrawDib(m_iEventOpenBoard, iCh);
		return false;
	}

	return true;
}

bool CDvrFCDDlg::SetRecording(bool bRecord, int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	if (bRecord)
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{						
			//sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->ConnectChannel(iCh);				
		}
	}
	else
	{
		if (m_pDVRSystem[m_iEventOpenBoard])
		{			
			//sdvr_rc = m_pDVRSystem[m_iEventOpenBoard]->DisConnectChannel(iCh);

		}
	}

	if (sdvr_rc != SDVR_ERR_NONE)
	{
		return false;
	}


	return true;
}

bool CDvrFCDDlg::SetInitDvrMenuState(bool bShow, bool bCheck)
{
	if (bShow)
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_DVRBOARD, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_DVRBOARD, MF_GRAYED);
	}

	if (bCheck)
	{
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_DVRBOARD, MF_CHECKED);
	}
	else
	{
		CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_INIT_DVRBOARD, MF_UNCHECKED);
	}	

	return true;
}

bool CDvrFCDDlg::SetDecodeChMenuState(bool bShow, bool bCheck, int iCh , int iBoard)
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
						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_01+i, MF_ENABLED);
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_01+i, MF_UNCHECKED);

						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_GRAYED);			
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_UNCHECKED);
					}
				}
				if(iBoard ==1)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_17+i, MF_ENABLED);
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_17+i, MF_UNCHECKED);

						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_17+i, MF_GRAYED);			
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_17+i, MF_UNCHECKED);
					}
				}
				if(iBoard ==2)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_33+i, MF_ENABLED);
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_33+i, MF_UNCHECKED);

						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_33+i, MF_GRAYED);			
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_33+i, MF_UNCHECKED);
					}
				}
				if(iBoard ==3)
				{
					if (m_pDVRSystem[iBoard] && (i<m_pDVRSystem[iBoard]->m_iVdoCount))
					{
						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_49+i, MF_ENABLED);
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_49+i, MF_UNCHECKED);

						EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_49+i, MF_GRAYED);			
						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_49+i, MF_UNCHECKED);
					}
				}
			}
		}
		else
		{
			for (i=0; i<MAX_BOARD_CHANNEL; i++)
			{
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_01+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_01+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_17+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_17+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_17+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_17+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_33+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_33+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_33+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_33+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_49+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_49+i, MF_UNCHECKED);

				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_49+i, MF_GRAYED);			
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_49+i, MF_UNCHECKED);
			}
		}	
	}	
	else
	{
		/*if (m_pDVRSystem[iBoard] && (iCh>=m_pDVRSystem[iBoard]->m_iVdoCount))
		{
			return true;
		}*/		

		if (bCheck)
		{
			if(iBoard ==0)
			{
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_01+iCh, MF_CHECKED);

				if(m_iPlayAudioCh == -1) //No any Channel to Play Audio
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+iCh, MF_ENABLED);
				}
			}
			if(iBoard ==1)
			{
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_17+iCh, MF_CHECKED);

				if(m_iPlayAudioCh == -1) //No any Channel to Play Audio
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_17+iCh, MF_ENABLED);
				}
			}
			if(iBoard ==2)
			{
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_33+iCh, MF_CHECKED);

				if(m_iPlayAudioCh == -1) //No any Channel to Play Audio
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_33+iCh, MF_ENABLED);
				}
			}
			if(iBoard ==3)
			{
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_49+iCh, MF_CHECKED);

				if(m_iPlayAudioCh == -1) //No any Channel to Play Audio
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_49+iCh, MF_ENABLED);
				}
			}
		}
		else
		{
			if(iBoard ==0)
			{
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_01+iCh, MF_UNCHECKED);
			}
			else if(iBoard ==1)
			{
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_17+iCh, MF_UNCHECKED);
			}
			if(iBoard ==2)
			{
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_33+iCh, MF_UNCHECKED);
			}
			if(iBoard ==3)
			{
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_DECODE_CH_49+iCh, MF_UNCHECKED);
			}			

			if(m_iPlayAudioCh == iCh) //The decode Channel is playing audio
			{
				//Must to stop audio playing
				m_iPlayAudioCh = -1;

				int i = 0;
				for (i = 0; i < MAX_BOARD_CHANNEL; i++)
				{
					if(m_iPlayAudioCh == -1) //No any channel to play audio
					{
						if(m_bIsDecodeChOK[i]) //The Channel is decoding action.
						{
							EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_ENABLED);
						}
						else
						{
							EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_GRAYED);
						}

						CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_UNCHECKED);
					}
				}
			}
			else
			{
				EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_GRAYED);
			}
		}		
	}

	return true;
}

BOOL CDvrFCDDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_INIT_DVRBOARD)
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
	else if ((wParam >= ID_DECODE_CH_01) && (wParam <= ID_DECODE_CH_16))
	{
		m_iEventOpenBoard = 0 ;
		m_iEventDecodeCh = (int)(wParam - ID_DECODE_CH_01);

		if(!m_bIsDecodeChOK[m_iEventDecodeCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_DECODE_CH]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_DECODE_CH]);
		}	
	}
	else if ((wParam >= ID_DECODE_CH_17) && (wParam <= ID_DECODE_CH_32))
	{
		m_iEventOpenBoard = 1 ;
		m_iEventDecodeCh = (int)(wParam - ID_DECODE_CH_01);

		if(!m_bIsDecodeChOK[m_iEventDecodeCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_DECODE_CH]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_DECODE_CH]);
		}	
	}
	else if ((wParam >= ID_DECODE_CH_33) && (wParam <= ID_DECODE_CH_48))
	{
		m_iEventOpenBoard = 2 ;
		m_iEventDecodeCh = (int)(wParam - ID_DECODE_CH_01);

		if(!m_bIsDecodeChOK[m_iEventDecodeCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_DECODE_CH]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_DECODE_CH]);
		}	
	}
	else if ((wParam >= ID_DECODE_CH_49) && (wParam <= ID_DECODE_CH_64))
	{
		m_iEventOpenBoard = 3 ;
		m_iEventDecodeCh = (int)(wParam - ID_DECODE_CH_01);

		if(!m_bIsDecodeChOK[m_iEventDecodeCh])
		{			
			SetEvent(m_hEventUIControl[ENAME_START_DECODE_CH]);			
		}
		else
		{
			SetEvent(m_hEventUIControl[ENAME_STOP_DECODE_CH]);
		}	
	}
	else if ((wParam >= ID_AUDIO_CH_01) && (wParam <= ID_AUDIO_CH_16))
	{
		int iChkPlayAudioCh = (int)(wParam - ID_AUDIO_CH_01);

		if(m_iPlayAudioCh == iChkPlayAudioCh) //Want to stop audio playing
		{
			m_iPlayAudioCh = -1;
		}
		else
		{
			m_iPlayAudioCh = iChkPlayAudioCh;
		}
		
		int i = 0;
		for (i = 0; i < MAX_BOARD_CHANNEL; i++)
		{
			if(m_iPlayAudioCh == -1) //No any channel to play audio
			{
				if(m_bIsDecodeChOK[i]) //The Channel is decoding action.
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_ENABLED);
				}
				else
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_GRAYED);
				}
				
				CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_UNCHECKED);
			}
			else //To set current channel to play audio
			{
				if(i == m_iPlayAudioCh)
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_ENABLED);
					CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_CHECKED);
				}
				else
				{
					EnableMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_GRAYED);
					CheckMenuItem(m_pMenu->GetSafeHmenu(), ID_AUDIO_CH_01+i, MF_UNCHECKED);
				}
			}
		}	
	}	

	return CDialog::OnCommand(wParam, lParam);
}


BOOL CDvrFCDDlg::InitDrawDib(int iCh , int iBoard)
{	
	m_pbyYUVBuffer[iBoard][iCh] = new BYTE[m_iVdoWidth_Raw[iCh]*m_iVdoHeight_Raw[iCh]*2];

	m_pDirectDrawDisplay[iBoard][iCh] = NULL;
	m_pDirectDrawDisplay[iBoard][iCh] = new CDirectDraw();

	m_bInitDrawOK[iBoard][iCh] = m_pDirectDrawDisplay[iBoard][iCh]->InitDirectDraw(m_iVdoWidth_Raw[iCh], m_iVdoHeight_Raw[iCh], 
											GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->m_hWnd);

	return TRUE;
}

BOOL CDvrFCDDlg::CloseDrawDib(int iCh , int iBoard)
{
	m_bInitDrawOK[iBoard][iCh] = FALSE;

	if(m_pDirectDrawDisplay[iBoard][iCh])
	{	
		m_pDirectDrawDisplay[iBoard][iCh]->CloseDirectDraw();

		delete m_pDirectDrawDisplay[iBoard][iCh];
		m_pDirectDrawDisplay[iBoard][iCh] = NULL;
	}

	if (m_pbyYUVBuffer[iBoard][iCh])
	{
		delete m_pbyYUVBuffer[iBoard][iCh];
		m_pbyYUVBuffer[iBoard][iCh] = NULL;
	}

	return TRUE;
}

void CDvrFCDDlg::DrawVideo(int iCh, BYTE *pBuffer, int iBoard)
{
	RECT	rc;
	GetDlgItem(IDC_STATIC_DISPLAY_01)->GetClientRect(&rc);	
	
	if (m_pDirectDrawDisplay[iBoard][iCh])
	{
		m_bInitDrawOK[iBoard][iCh] = m_pDirectDrawDisplay[iBoard][iCh]->DrawYV12((BYTE*)pBuffer, 
			m_iVdoWidth_Raw[iCh], m_iVdoHeight_Raw[iCh], 
			GetDlgItem(IDC_STATIC_DISPLAY_01+iCh), 
			GetDlgItem(IDC_STATIC_DISPLAY_01+iCh)->m_hWnd, rc);
	}	
}

bool CDvrFCDDlg::acquireBuffer(int iBoard, int iCh, sdvr_frame_type_e frame_type, sx_bool is_primary, sdvr_chan_handle_t handle)
{
	sdvr_av_buffer_t *av_buf;
	sdvr_yuv_buffer_t *yuv_buf;
	sdvr_err_e err;

	char szDebug[100];

	if (frame_type == SDVR_FRAME_RAW_AUDIO)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
				"CH%d come acquireBuffer(), frm_type(SDVR_FRAME_RAW_AUDIO)\n", iCh);
		OutputDebugString(szDebug);
	}

	//Encode Channel
	if(m_pDVRSystem[iBoard]->m_pEncodeChParam[iCh])
	{
		if ((m_pDVRSystem[iBoard]->m_pEncodeChParam[iCh]->m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_RAW) ||
			(m_pDVRSystem[iBoard]->m_pEncodeChParam[iCh]->m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_ENC))
		{
			if (frame_type == SDVR_FRAME_RAW_VIDEO)
			{
				if(m_pDVRSystem[iBoard]->m_pDecodeChParam[iCh] == NULL) //Decode function
				{
					err = sdvr_get_yuv_buffer(handle, &yuv_buf);

					if (err == SDVR_ERR_NONE && yuv_buf)
					{
						if (m_pbyYUVBuffer[iBoard][iCh])
						{
							memset(m_pbyYUVBuffer[iBoard][iCh], 0, sizeof(m_pbyYUVBuffer[iBoard][iCh]));			
							memcpy(m_pbyYUVBuffer[iBoard][iCh], yuv_buf->y_data, yuv_buf->y_data_size);

							memcpy((m_pbyYUVBuffer[iBoard][iCh] + yuv_buf->y_data_size), yuv_buf->u_data, yuv_buf->u_data_size);
							memcpy(m_pbyYUVBuffer[iBoard][iCh] + yuv_buf->y_data_size + yuv_buf->u_data_size, 
								yuv_buf->v_data, yuv_buf->v_data_size);

							if(m_bInitDrawOK[iBoard][iCh] && m_bShowBoard[iBoard])
								DrawVideo(iCh, m_pbyYUVBuffer[iBoard][iCh], iBoard);
						}					

						sdvr_release_yuv_buffer(yuv_buf);
					}
				}
			}
			else if (frame_type == SDVR_FRAME_RAW_AUDIO)
			{
				sprintf_s(szDebug, sizeof(szDebug), 
						"CH%d come acquireBuffer(), frm_type(SDVR_FRAME_RAW_AUDIO)\n", iCh);
				OutputDebugString(szDebug);

				err = sdvr_get_av_buffer(handle, SDVR_FRAME_RAW_AUDIO, &av_buf);
				if (err == SDVR_ERR_NONE)
				{
					sdvr_release_av_buffer(av_buf);
				}			
			}
			else //Encode Video
			{
					//H.264
				if (((frame_type == SDVR_FRAME_H264_I) || (frame_type == SDVR_FRAME_H264_P) ||
					(frame_type == SDVR_FRAME_H264_B) || (frame_type == SDVR_FRAME_H264_SPS) ||
					(frame_type == SDVR_FRAME_H264_PPS)) ||
					//MPEG4
					((frame_type == SDVR_FRAME_MPEG4_I) || (frame_type == SDVR_FRAME_MPEG4_P) ||
					(frame_type == SDVR_FRAME_MPEG4_VOL)))
				{
					err = sdvr_get_av_buffer(handle, SDVR_FRAME_VIDEO_ENCODED_PRIMARY, &av_buf);

					if (err == SDVR_ERR_NONE && av_buf)
					{
						sprintf_s(szDebug, sizeof(szDebug), 
							"CH%d come acquireBuffer(), frame_type(%d)\n", 
							iCh, frame_type);
						//OutputDebugString(szDebug);

						//H.264
						if ((frame_type == SDVR_FRAME_H264_I) || (frame_type == SDVR_FRAME_H264_P) ||
							(frame_type == SDVR_FRAME_H264_B) || (frame_type == SDVR_FRAME_H264_SPS) ||
							(frame_type == SDVR_FRAME_H264_PPS))
						{
							if(m_pDVRSystem[iBoard]->m_pEncodeChParam[iCh]->m_iDecodeCHIdx != -1)//HW Decode
							{
								int iDecodeCHIdx = m_pDVRSystem[iBoard]->m_pEncodeChParam[iCh]->m_iDecodeCHIdx;

								if(m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecodeCHIdx])
								{
									sdvr_av_buffer_t *En_frame_buffer;

									err = sdvr_alloc_av_buffer(
													m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecodeCHIdx]->m_chan_handle, 
													&En_frame_buffer);
									if ((err == SDVR_ERR_NONE) && En_frame_buffer)
									{
										En_frame_buffer->motion_detected = 
															m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecodeCHIdx]->m_seq++;

										memcpy(&En_frame_buffer->payload[0], 
												&av_buf->payload[0], av_buf->payload_size);

										// todo: get the video encoder type from the player
										//       write now we only support h.264 decoder
										En_frame_buffer->frame_type = SDVR_FRAME_H264_IDR;

										// Set the buffer size and 
										// send the video frame from the PCI buffer to the FW.
										En_frame_buffer->payload_size = av_buf->payload_size;

										En_frame_buffer->timestamp = av_buf->timestamp;
										En_frame_buffer->timestamp_high = av_buf->timestamp_high;

										err = sdvr_send_av_frame(En_frame_buffer);

										// Free any buffer that was not sent
										err = sdvr_release_av_buffer(En_frame_buffer);
									}
								}
							}
						}

						sdvr_release_av_buffer(av_buf);
					}
				}
				else if ((frame_type == SDVR_FRAME_G711) ||
							(frame_type == SDVR_FRAME_AUDIO_ENCODED))
				{
					err = sdvr_get_av_buffer(handle, SDVR_FRAME_AUDIO_ENCODED, &av_buf);

					if (err == SDVR_ERR_NONE && av_buf)
					{
						if(m_pDVRSystem[iBoard]->m_pEncodeChParam[iCh]->m_iDecodeCHIdx != -1)//HW Decode
						{
							int iDecodeCHIdx = m_pDVRSystem[iBoard]->m_pEncodeChParam[iCh]->m_iDecodeCHIdx;

							if((m_iPlayAudioCh != -1) &&
								(iDecodeCHIdx == m_iPlayAudioCh))
							{
								sdvr_av_buffer_t *En_frame_buffer;

								err = sdvr_alloc_av_buffer(
										m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecodeCHIdx]->m_chan_handle, 
										&En_frame_buffer);
								if (err == SDVR_ERR_NONE && En_frame_buffer)
								{
									memcpy(&En_frame_buffer->payload[0], 
										&av_buf->payload[0], av_buf->payload_size);

									En_frame_buffer->frame_type = SDVR_FRAME_G711;

									// Set the buffer size and 
									// send the video frame from the PCI buffer to the FW.
									En_frame_buffer->payload_size = av_buf->payload_size;

									/*
									En_frame_buffer->timestamp = av_buf->timestamp;
									En_frame_buffer->timestamp_high = av_buf->timestamp_high;
									*/

									err = sdvr_send_av_frame(En_frame_buffer);

									// Free any buffer that was not sent
									err = sdvr_release_av_buffer(En_frame_buffer);
								}
							}
						}

						sdvr_release_av_buffer(av_buf);
					}
				}
				else if (frame_type == SDVR_FRAME_JPEG)
				{
					err = sdvr_get_av_buffer(handle, SDVR_FRAME_VIDEO_ENCODED_PRIMARY, &av_buf);

					if (err == SDVR_ERR_NONE && av_buf)
					{
						sdvr_release_av_buffer(av_buf);
					}
				}
				else //Other Frame Type
				{
				}
			}
		}
	}

	//Decode Channel
	if(m_pDVRSystem[iBoard]->m_pDecodeChParam[iCh])
	{
		//if (m_pDVRSystem[iBoard]->m_pDecodeChParam[iCh]->m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_DEC)
		{
			if (frame_type == SDVR_FRAME_RAW_VIDEO)
			{
				err = sdvr_get_yuv_buffer(handle, &yuv_buf);

				if (err == SDVR_ERR_NONE && yuv_buf)
				{
					if (m_pbyYUVBuffer[iBoard][iCh])
					{
						memset(m_pbyYUVBuffer[iBoard][iCh], 0, sizeof(m_pbyYUVBuffer[iBoard][iCh]));			
						memcpy(m_pbyYUVBuffer[iBoard][iCh], yuv_buf->y_data, yuv_buf->y_data_size);

						memcpy((m_pbyYUVBuffer[iBoard][iCh] + yuv_buf->y_data_size), yuv_buf->u_data, yuv_buf->u_data_size);
						memcpy(m_pbyYUVBuffer[iBoard][iCh] + yuv_buf->y_data_size + yuv_buf->u_data_size, 
							yuv_buf->v_data, yuv_buf->v_data_size);

						if(m_bInitDrawOK[iBoard][iCh] && m_bShowBoard[iBoard])
						{							
//							TRACE("chuck look iBoard %d \n" ,iBoard);	
							DrawVideo(iCh, m_pbyYUVBuffer[iBoard][iCh], iBoard);							
						}
					}

					sdvr_release_yuv_buffer(yuv_buf);
				}
			}
			else if (frame_type == SDVR_FRAME_RAW_AUDIO)
			{
				sprintf_s(szDebug, sizeof(szDebug), 
						"CH%d come acquireBuffer(), frm_type(SDVR_FRAME_RAW_AUDIO)\n", iCh);
				OutputDebugString(szDebug);

				err = sdvr_get_av_buffer(handle, SDVR_FRAME_RAW_AUDIO, &av_buf);
				if (err == SDVR_ERR_NONE)
				{
					if((g_pMainDlg->m_pWaveOut) && (m_iPlayAudioCh == iCh))
					{
						g_pMainDlg->m_pWaveOut->WriteAudioBlock((BYTE *)av_buf->payload, av_buf->payload_size);
					}

					sdvr_release_av_buffer(av_buf);
				}			
			}
			else
			{
				sprintf_s(szDebug, sizeof(szDebug), 
						"CH%d come acquireBuffer(), Other frm_type(%d)\n", iCh, frame_type);
				//OutputDebugString(szDebug);
			}
		}
	}

	return true;
}

bool CDvrFCDDlg::OpenMovFile(int iCh, char szFileName[MAX_PATH])
{
	mp4_track_info_t track;
	mp4_info_t info;

	memset(&info, 0, sizeof(info));	

	m_pMp4OpenFile[iCh] = new mp4File_t;

	m_pMp4OpenFile[iCh]->vtrack = -1;
	m_pMp4OpenFile[iCh]->atrack = -1;
	m_pMp4OpenFile[iCh]->FileHandle = 0;

	info.file_name = szFileName;
	info.mux_mode = MP4_READ_PLAY;
	info.file_mode = MODE_MOV;

	// Timescale is based on 90k Hz clock.
	m_pMp4OpenFile[iCh]->FileHandle = mp4mux_open(&info);

	if (m_pMp4OpenFile[iCh]->FileHandle == 0) 
	{
		delete m_pMp4OpenFile[iCh];
		m_pMp4OpenFile[iCh] = NULL;

		char szMsg[_MAX_PATH];
		wsprintf(szMsg, "Error: failed to open file(%s) for playback.", szFileName);
		MessageBox(szMsg);

		return false;
	}

	m_PlaySeq[iCh] = 1;
	for (int i = 0; i < info.nb_tracks; i++)
	{
		if (!mp4mux_get_track_info(m_pMp4OpenFile[iCh]->FileHandle, i, &track))
		{
			if (track.type == MP4_TRACK_AUDIO)
			{
				switch (track.codec_id)
				{
					case CODEC_ID_PCM_MULAW:
						m_pMp4OpenFile[iCh]->atrack = track.track_id;

						/*
						//todo: for now we don't care about audio so don't even get it
						mp4mux_disable_track(m_pMp4OpenFile[iCh]->FileHandle, 
													m_pMp4OpenFile[iCh]->atrack);
						*/
						break;
					default:
							m_pMp4OpenFile[iCh]->atrack = -1;
							break;
				}
			}
			else if (track.type == MP4_TRACK_VIDEO)
			{
				switch (track.codec_id)
				{
					case CODEC_ID_H264:
						m_pMp4OpenFile[iCh]->vtrack = track.track_id;
						break;
	
					default:
						m_pMp4OpenFile[iCh]->vtrack = -1;
						break; //printf("Unsupported video codec\n");
				}
			}
			else
			{
				; //printf("Unsupported track type\n");
			}
		}
	}

	m_iSendCounter[iCh] = 0;

	m_hKillThreadGetMovPacketData[iCh] = CreateEvent(NULL, TRUE, FALSE, NULL);
	ResetEvent(m_hKillThreadGetMovPacketData[iCh]);

	m_PacketThreadStructure[iCh].ChannelID = iCh;
	m_PacketThreadStructure[iCh].BoardIndex = m_iEventOpenBoard;
	m_PacketThreadStructure[iCh].pp = this;  

	DWORD dwThreadID;
	m_hThreadGetMovPacketData[iCh] = (HANDLE)CreateThread(NULL, 0, ThreadGetMovPacketData, 
														(PVOID)&m_PacketThreadStructure[iCh], 0, &dwThreadID);
	if(!m_hThreadGetMovPacketData[iCh])
	{
		mp4mux_close(m_pMp4OpenFile[iCh]->FileHandle);

		delete m_pMp4OpenFile[iCh];
		m_pMp4OpenFile[iCh] = NULL;

		char szMsg[_MAX_PATH];
		wsprintf(szMsg, "Error: Create ThreadGetMovPacketData(%d) fail!", iCh);
		MessageBox(szMsg);

		return false;
	}

	//To notify ThreadGetMovPacketData to start working
	m_bOpenMovFileReady[iCh] = true;

	return true;
}

bool CDvrFCDDlg::CloseMovFile(int iCh)
{
	if(!m_pMp4OpenFile[iCh])
		return false;
	if(!m_pMp4OpenFile[iCh]->FileHandle)
		return false;

	SetEvent(m_hKillThreadGetMovPacketData[iCh]);

	if(WaitForSingleObject(m_hThreadGetMovPacketData[iCh], 3000) != WAIT_OBJECT_0)
	{
		TerminateThread(m_hThreadGetMovPacketData[iCh], THREADEXIT_SUCCESS);
		Sleep(10);

		char sz[100];
		wsprintf(sz,"TerminateThraed m_hThreadGetMovPacketData(%d)!\n", iCh);
		OutputDebugString(sz);
	}

	CloseHandle(m_hKillThreadGetMovPacketData[iCh]);
	CloseHandle(m_hThreadGetMovPacketData[iCh]);

	mp4mux_close(m_pMp4OpenFile[iCh]->FileHandle);

	delete m_pMp4OpenFile[iCh];
	m_pMp4OpenFile[iCh] = NULL;

	return true;
}

bool CDvrFCDDlg::SendOneAVEncFrame(int iBoard, int iCh)
{
	bool bSuccess = false;

	sx_int32		mux_err;
	mp4_packet_t	pkt;
	sdvr_err_e		err;
	sdvr_av_buffer_t *frame_buffer;
	int iDecCh = iCh % MAX_BOARD_CHANNEL;

	char szMsg[_MAX_PATH];

	 if(m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecCh])
	{
		err = sdvr_alloc_av_buffer(m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecCh]->m_chan_handle, 
									&frame_buffer);
		if (err != SDVR_ERR_NONE)
		{
			wsprintf(szMsg, "Error: CH%d failed to sdvr_alloc_av_buffer [error=%d (%s)]", 
				iCh, err, sdvr_get_error_text(err));
			//MessageBox(szMsg);
	//		OutputDebugString(szMsg);

			bSuccess = false;
		}
		else
		{
			m_iSendCounter[iCh]++;

			// Get the new pack into the PCI allocated buffer
			pkt.data = &frame_buffer->payload[0];
			pkt.size = m_pDVRSystem[iBoard]->getSDKParams().dec_buf_size;

			// If we reached to the end of file stop decoding on
			// the file and also return back the buffer obtained from the SCT
			if ((mux_err = mp4mux_get_packet(m_pMp4OpenFile[iCh]->FileHandle, &pkt)) != MP4MUX_ERR_OK)
			{
				m_iSendCounter[iCh]--;

				if (mux_err == MP4MUX_ERR_END_OF_FILE)
				{
					bSuccess = true;
				}
			}
			else
			{
				if (pkt.track_id == m_pMp4OpenFile[iCh]->atrack)
				{
					/*It is important that current FW don't support the function to decode audio.
					/////////////////////////////////////////////////////////////////////////////
					// send the audio frame from the PCI buffer to the FW.
					frame_buffer->frame_type = SDVR_FRAME_G711;

					frame_buffer->payload_size = pkt.size;
					err = sdvr_send_av_frame(frame_buffer);
					*/

					frame_buffer->frame_type = SDVR_FRAME_G711;

					frame_buffer->payload_size = pkt.size;

					//To use SW audio decode now
					short *pOutbuf;
					pOutbuf = new short[8000];

					g711_t *      g711_ulaw;  // Use for North America and Japan
					g711_config_t config_ulaw = {G711_ULAW, 1, 0};

					/*g711_encode_stereo(g711_ulaw, frame_buf->payload_size/2, 
								(short *)&frame_buf->payload[0], &frame_buf->payload[0]);*/

					g711_open(&config_ulaw, &g711_ulaw);
					g711_decode(g711_ulaw, frame_buffer->payload_size/2, 
								(short *)&frame_buffer->payload[0], pOutbuf);
					g711_close(g711_ulaw);

					if((m_pWaveOut) && (m_iPlayAudioCh == iCh))
					{
						m_pWaveOut->WriteAudioBlock((BYTE *)pOutbuf, frame_buffer->payload_size);
					}

					delete pOutbuf;
				}
				else if (pkt.track_id == m_pMp4OpenFile[iCh]->vtrack) 
				{
					// overwrite with start code
					pkt.data[0] = 0;
					pkt.data[1] = 0;
					pkt.data[2] = 0;
					pkt.data[3] = 1; 
					frame_buffer->motion_detected = m_PlaySeq[iCh]++;

					// todo: get the video encoder type from the player
					//       write now we only support h.264 decoder
					frame_buffer->frame_type = SDVR_FRAME_H264_IDR;

					// Set the buffer size and 
					// send the video frame from the PCI buffer to the FW.
					frame_buffer->payload_size = pkt.size;

					// The following OSD functionality only exist in 3.2.x.x or higher of firmware
					frame_buffer->timestamp = (sx_uint32)(pkt.dts & 0xffffffff);
					frame_buffer->timestamp_high = (sx_uint32)(pkt.dts >> 32);

					err = sdvr_send_av_frame(frame_buffer);
				}

				if (err != SDVR_ERR_NONE)
				{
					wsprintf(szMsg, "Error: CH%d failed to send frame for decoding[error=%d (%s)]\n", 
						iCh, err, sdvr_get_error_text(err));
					//MessageBox(szMsg);
					OutputDebugString(szMsg);
				}
				else
				{
					bSuccess = true;

					wsprintf(szMsg, "Error: CH%d success to send frame for decoding[error=%d (%s)]\n", 
						iCh, err, sdvr_get_error_text(err));
					//MessageBox(szMsg);
//					OutputDebugString(szMsg);
				}
			}

			// Free any buffer that was not sent
			err = sdvr_release_av_buffer(frame_buffer);
		}
	}
	else
	{
		bSuccess = false;
	}

	return bSuccess;
}

DWORD WINAPI CDvrFCDDlg::ThreadGetMovPacketData(LPVOID pParam)
{
	PACKET_TRREAD_STRUCT * tp = (PACKET_TRREAD_STRUCT *)pParam;

	CDvrFCDDlg* pp = (CDvrFCDDlg*)tp->pp;
	int iCh = tp->ChannelID;
	int iBoard = tp->BoardIndex;
	int iDecCh = iCh % MAX_BOARD_CHANNEL;

	DWORD dwWaitRet = 0;

	sx_int32		mux_err;
	mp4_packet_t	pkt;
	sdvr_err_e		err;
	sdvr_av_buffer_t *frame_buffer;

	char szMsg[_MAX_PATH];

	int timerInterval = 5;
	int dt = 0, dt_max = 0, dt_min = 9999, ndt = 0;
	DWORD dwSendOneAVEncFrameInterval = 0;
	bool bSendSuccess = true;

	while(1)
	{
		dwWaitRet = WaitForSingleObject(pp->m_hKillThreadGetMovPacketData[iCh], 5);
		if(dwWaitRet == WAIT_OBJECT_0)
		{
			ResetEvent(pp->m_hKillThreadGetMovPacketData[iCh]);
			break;
		}

		if((pp->m_pDVRSystem[iBoard]) && (pp->m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecCh]) &&
			(pp->m_bOpenMovFileReady[iCh] == true) && //mov file is ready now
			(pp->m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecCh]->m_bIsChannelEnableReady == TRUE) &&
			(pp->m_pDVRSystem[iBoard]->m_pDecodeChParam[iDecCh]->m_bIsClosingChannel == false))
		{
			if(dwSendOneAVEncFrameInterval == 0)
			{
				ndt = 0;
				dwSendOneAVEncFrameInterval = timeGetTime();
			}

			bSendSuccess = pp->SendOneAVEncFrame(iBoard, iCh);
			dt = timeGetTime() - dwSendOneAVEncFrameInterval;

			// Update stats
			if (dt > dt_max) dt_max = dt;
			if (dt < dt_min) dt_min = dt;
			++ndt;

			// Get the timer interval and adjust it for the time spent in
			// reading and sending

			if(pp->m_pDVRSystem[iBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_NTSC)
			{
				timerInterval = 33;	//NTSC: 33, PAL: 40
			}
			else if(pp->m_pDVRSystem[iBoard]->m_VideoStandard == SDVR_VIDEO_STD_D1_PAL)
			{
				timerInterval = 40;	//NTSC: 33, PAL: 40
			}

			timerInterval = (dt > ndt*timerInterval) ? 0 : ndt * timerInterval - dt;

			if (!bSendSuccess)
			{
				--ndt;
				timerInterval = 1;
			}

			Sleep(timerInterval);
		}
	}

	return 1;
}
void CDvrFCDDlg::OnBnClickedRadioBoard1()
{
	int i;
	for( i=0 ; i< MAX_BOARD_CHANNEL ; i++)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_01+i)->Invalidate();
	}

	memset(&m_bShowBoard,0,sizeof(m_bShowBoard));
	m_bShowBoard[0]= true ;
}

void CDvrFCDDlg::OnBnClickedRadioBoard2()
{
	int i;
	for( i=0 ; i< MAX_BOARD_CHANNEL ; i++)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_01+i)->Invalidate();
	}

	memset(&m_bShowBoard,0,sizeof(m_bShowBoard));
	m_bShowBoard[1]= true ;
}

void CDvrFCDDlg::OnBnClickedRadioBoard3()
{
	int i;
	for( i=0 ; i< MAX_BOARD_CHANNEL ; i++)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_01+i)->Invalidate();
	}

	memset(&m_bShowBoard,0,sizeof(m_bShowBoard));
	m_bShowBoard[2]= true ;
}

void CDvrFCDDlg::OnBnClickedRadioBoard4()
{
	int i;
	for( i=0 ; i< MAX_BOARD_CHANNEL ; i++)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_01+i)->Invalidate();
	}

	memset(&m_bShowBoard,0,sizeof(m_bShowBoard));
	m_bShowBoard[3]= true ;
}
