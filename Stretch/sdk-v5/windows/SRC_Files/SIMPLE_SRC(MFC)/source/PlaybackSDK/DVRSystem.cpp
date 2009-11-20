// DVRSystem.cpp : 實作檔
//

#include "stdafx.h"
#include "PlayDvrFCD.h"
#include "PlayDvrFCDDlg.h"
#include "DVRSystem.h"

// CDVRSystem

IMPLEMENT_DYNAMIC(CDVRSystem, CWnd)

CDVRSystem::CDVRSystem(CDvrFCDDlg* pMainDlg, int iBoardIndex)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;

	m_iBoardIndex = iBoardIndex;
	m_pMainDlg = pMainDlg;	

	int i = 0, j = 0;
	for (i = 0; i < MAX_BOARD_CHANNEL; i++)
	{
		m_pEncodeChParam[i] = NULL;
		m_pDecodeChParam[i] = NULL;

		m_video_res_decimation[i] = SDVR_VIDEO_RES_DECIMATION_EQUAL;

		for (j = 0; j < SDVR_MAX_OSD; j++)
		{
			memset(&m_OSD_Config_ex[i][j], 0, sizeof(m_OSD_Config_ex[i][j]));

			m_bEnableOSD[i][j] = TRUE;

			wsprintf(m_szOSDText[i][j], "Camera%02d", i+1);

			m_OSD_Config_ex[i][j].dts_format = SDVR_OSD_DTS_YMD_24H; //sdvr_dts_style_e enum
			m_OSD_Config_ex[i][j].location_ctrl = SDVR_LOC_BOTTOM_LEFT; //sdvr_location_e enum

			m_OSD_Config_ex[i][j].text_len = strlen(m_szOSDText[i][j]);

			// ANSI(char) to Unicode(WCHAR) 
			int nIndex = MultiByteToWideChar(CP_ACP, 0, m_szOSDText[i][j] , -1, NULL, 0); 
			WCHAR* pUnicode = new WCHAR[nIndex+1]; 
			MultiByteToWideChar(CP_ACP, 0, m_szOSDText[i][j] , -1, pUnicode, nIndex); 

			memcpy(&m_OSD_Config_ex[i][j].text, pUnicode, strlen(m_szOSDText[i][j])*2);

			if (pUnicode)
			{
				delete pUnicode;
				pUnicode = NULL;
			}

			m_OSD_Config_ex[i][j].top_left_x = 0;	//For Custom using
			m_OSD_Config_ex[i][j].top_left_y = 0;	//For Custom using

			m_OSD_Config_ex[i][j].translucent = 50;  //0 ~ 255
		}
	}

	for (i = 0; i < MAX_CHIP_NUM; i++)
	{
		m_iTotalHWEncodeChannel[i] = 0;
		m_iTotalHWDecodeChannel[i] = 0;
	}

	m_supported_video_stds = 0;
	memset(&m_bSupportVideoStds, false, sizeof(m_bSupportVideoStds));	
	m_bBoardConnected = false;	
	memset(&m_sdk_params, 0, sizeof(m_sdk_params));
	memset(&m_fwVersion, 0, sizeof(m_fwVersion));
	memset(&m_BoardAttrib, 0, sizeof(m_BoardAttrib));
	memset(&m_PCIAttrib, 0, sizeof(m_PCIAttrib));
	memset(&m_BoardConfig, 0, sizeof(m_BoardConfig));

	m_VideoStandard = SDVR_VIDEO_STD_D1_NTSC;

	setDebugEnable(false);
	setDebugSDK(false);

	// By default we don't debug A/V frames. It uses lots of
	// harddisk space
	setDebugAVFrames(false);

	// By default Start Code Encoded is on to be done
	// on the host side.
	m_bH264SCE = true;

	// The default Frame Rate calculation sampling is 3 seconds
	m_iStatFPSInterval = 3;

	// The default PCI driver timeout is ?? seconds
	m_iTimeOut = 10;	

	m_bIsPAL = false;

	memset(&m_font_table, 0, sizeof(sdvr_font_table_t));
}

CDVRSystem::~CDVRSystem()
{
}


BEGIN_MESSAGE_MAP(CDVRSystem, CWnd)
END_MESSAGE_MAP()



// CDVRSystem 訊息處理常式
sdvr_err_e CDVRSystem::UpgradeFW(char* pszFWpath)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[100];	

	bool bNeed = true;

	if (pszFWpath == "")
	{
		bNeed = false;
	}	

	sprintf_s(szDebug, sizeof(szDebug), 
			"board(%d) is upgrading firmware.....", m_iBoardIndex);
	m_pMainDlg->SetWindowText(szDebug);	

	if (bNeed)
	{			
		sdvr_rc = sdvr_upgrade_firmware(m_iBoardIndex, pszFWpath);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_upgrade_firmware(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
			//OutputDebugString(szDebug);

			sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) upgrade firmware OK.....", m_iBoardIndex);
			m_pMainDlg->SetWindowText(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_upgrade_firmware(%d) = %d, fail......\n", m_iBoardIndex, sdvr_rc);
			//OutputDebugString(szDebug);

			sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) upgrade firmware fail.....", m_iBoardIndex);
			m_pMainDlg->SetWindowText(szDebug);
		}

	}		

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::OpenBoard()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	char szDebug[100];

	int iRetryCount = 0;

	sprintf_s(szDebug, sizeof(szDebug), 
				"board(%d) is connecting.....", m_iBoardIndex);
	m_pMainDlg->SetWindowText(szDebug);

	while (1)
	{		
		sdvr_rc = sdvr_board_connect(m_iBoardIndex, m_VideoStandard, m_bH264SCE);	
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_board_connect(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
			OutputDebugString(szDebug);		

			sprintf_s(szDebug, sizeof(szDebug), 
						"board(%d) board connect OK.....", m_iBoardIndex);
			m_pMainDlg->SetWindowText(szDebug);

			break;
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_board_connect(%d) = %d\n", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);		

			sprintf_s(szDebug, sizeof(szDebug), 
						"board(%d) board connect fail, retry.....", m_iBoardIndex);
			m_pMainDlg->SetWindowText(szDebug);
		}

		iRetryCount+=1;		

		if (iRetryCount>3)
		{
			break;
		}

		Sleep(50);
	}

	if ((sdvr_rc == SDVR_ERR_NONE) || (sdvr_rc == SDVR_DRV_ERR_BOARD_IN_USE))
	{		
		m_bBoardConnected = true;		

		////////////////////////////////////////////////////
		time_t Nowtime;

		SYSTEMTIME	stNow;
		GetLocalTime(&stNow);

		ULARGE_INTEGER ulTime;
		SystemTimeToFileTime(&stNow, (FILETIME*)&ulTime);

		ulTime.QuadPart = ulTime.QuadPart - 116444736000000000; // 100-nanoseconds since 1601
		Nowtime = (ulTime.QuadPart / 10000000);
		////////////////////////////////////////////////////

		sdvr_rc = sdvr_set_date_time(m_iBoardIndex, Nowtime);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_date_time(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_date_time(%d) = %d\n, fail.....", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);
		}

		sdvr_rc = sdvr_get_firmware_version_ex(m_iBoardIndex, &m_fwVersion);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_get_firmware_version_ex(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_get_firmware_version_ex(%d) = %d, fail.....\n", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);
		}		

		sdvr_rc = sdvr_set_watchdog_state(m_iBoardIndex, 0);	//	Disable watchdog.
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_watchdog_state(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_watchdog_state(%d) = %d, fail.....\n", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);
		}
	}
	else
	{
		m_bBoardConnected = false;
		sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) board connect retry over 5 times, fail.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}	

	return sdvr_rc;
}


sdvr_err_e CDVRSystem::CreateEncodeChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (!m_pEncodeChParam[iCh])
	{
		m_pEncodeChParam[iCh] = new CChannelParam(this, m_iBoardIndex, iCh);

		if (m_pEncodeChParam[iCh]->SetChannelParamAndCreate(CHANNEL_VDO_DATA_TYPE_ENC))
		{

		}
	}
	else
	{
		sdvr_rc = SDVR_DRV_ERR_CHANNEL_IN_USE;
	}	

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::StartEncodeChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (m_pEncodeChParam[iCh])
	{
		m_pEncodeChParam[iCh]->EnableChannel(true);

		m_iTotalHWEncodeChannel[iCh/4] ++;
	}
	else
	{
		sdvr_rc = SDVR_DRV_ERR_CHANNEL_IN_USE;
	}	

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::DeleteEncodeChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (m_pEncodeChParam[iCh])
	{
		m_pEncodeChParam[iCh]->CloseChannel();

		delete m_pEncodeChParam[iCh];
		m_pEncodeChParam[iCh] = NULL;
	}

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::StopEncodeChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (m_pEncodeChParam[iCh])
	{
		m_pEncodeChParam[iCh]->m_bIsClosingChannel = true;

		int iWhileLoopCnt = 0;

		//Wait Callback Function ending
		while(m_pEncodeChParam[iCh]->m_bIsClosingChannel)
		{
			Sleep(10);
		}

		if(m_pEncodeChParam[iCh]->EnableChannel(false))
		{
			m_iTotalHWEncodeChannel[iCh/4] --;
		}
	}

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::CreateDecodeChannel(int iCh, int iWidth, int iHeight)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (!m_pDecodeChParam[iCh])
	{
		m_pDecodeChParam[iCh] = new CChannelParam(this, m_iBoardIndex, iCh);

		if (m_pDecodeChParam[iCh]->SetChannelParamAndCreate(CHANNEL_VDO_DATA_TYPE_DEC))
		{	
			sdvr_rc = SDVR_ERR_NONE;
		}		
	}
	else
	{
		sdvr_rc = SDVR_DRV_ERR_CHANNEL_IN_USE;
	}	

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::StartDecodeChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (m_pDecodeChParam[iCh])
	{
		if(m_pDecodeChParam[iCh]->EnableChannel(true))
		{
			m_iTotalHWDecodeChannel[iCh/4] ++;

			sdvr_rc = SDVR_ERR_NONE;
		}
		else
		{
			sdvr_rc = SDVR_ERR_DECODER_NOT_ENABLED;
		}
	}
	else
	{
		sdvr_rc = SDVR_DRV_ERR_CHANNEL_IN_USE;
	}	

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::DeleteDecodeChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (m_pDecodeChParam[iCh])
	{
		if(m_pDecodeChParam[iCh]->CloseChannel())
		{
			delete m_pDecodeChParam[iCh];
			m_pDecodeChParam[iCh] = NULL;
		}
	}

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::StopDecodeChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (m_pDecodeChParam[iCh])
	{
		m_pDecodeChParam[iCh]->m_bIsClosingChannel = true;

		int iWhileLoopCnt = 0;

		/*Decode channel not need to do the action.
		//Wait Callback Function ending
		while(m_pDecodeChParam[iCh]->m_bIsClosingChannel)
		{
			Sleep(10);
		}
		*/

		if(m_pDecodeChParam[iCh]->EnableChannel(false))
		{
			m_iTotalHWDecodeChannel[iCh/4] --;
		}
	}

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::CloseBoard()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[64];

	sdvr_rc = sdvr_set_watchdog_state(m_iBoardIndex, 0);	//	Disable watchdog.

	sdvr_rc = sdvr_board_disconnect(m_iBoardIndex);	
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_board_disconnect(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
		OutputDebugString(szDebug);		
	}
	else	 // Although fail but seems not have any bad outcome.
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_board_disconnect(%d) = %d\n", m_iBoardIndex, sdvr_rc);
		OutputDebugString(szDebug);				
	}

	sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) disconnect OK.....", m_iBoardIndex);
	m_pMainDlg->SetWindowText(szDebug);

	m_bBoardConnected = false;

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::GetBoardParam()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[80]; 
	int iRetryCount = 0;	

	sprintf_s(szDebug, sizeof(szDebug), 
				"board(%d) is getting board config.....", m_iBoardIndex);
	m_pMainDlg->SetWindowText(szDebug);

	sdvr_rc = sdvr_get_board_attributes(m_iBoardIndex, &m_BoardAttrib);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_get_board_attributes(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
		//OutputDebugString(szDebug);

		sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) get board attributes OK.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_get_board_attributes(%d) = %d, fail.....\n", m_iBoardIndex, sdvr_rc);
		OutputDebugString(szDebug);

		sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) get board attributes fail.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}

	GetBoardSupportVideoStandard(m_BoardAttrib.supported_video_stds);

	sdvr_rc = sdvr_get_sdk_params(&m_sdk_params);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_get_sdk_params(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
		//OutputDebugString(szDebug);

		sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) get sdk params OK.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_get_sdk_params(%d) = %d, fail.....\n", m_iBoardIndex, sdvr_rc);
		OutputDebugString(szDebug);

		sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) get sdk params fail.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}	

	sdvr_rc = sdvr_get_pci_attrib(m_iBoardIndex, &m_PCIAttrib);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_get_pci_attrib(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
		//OutputDebugString(szDebug);

		sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) get pci attrib OK.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_get_pci_attrib(%d) = %d, fail.....\n", m_iBoardIndex, sdvr_rc);
		OutputDebugString(szDebug);

		sprintf_s(szDebug, sizeof(szDebug), 
					"board(%d) get pci attrib fail.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}


	while (1)
	{
		sdvr_rc = sdvr_get_board_config(m_iBoardIndex, &m_BoardConfig);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_get_board_config(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
			//OutputDebugString(szDebug);

			sprintf_s(szDebug, sizeof(szDebug), 
						"board(%d) get board config OK.....", m_iBoardIndex);
			m_pMainDlg->SetWindowText(szDebug);

			break;
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_get_board_config(%d) = %d\n", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);

			sprintf_s(szDebug, sizeof(szDebug), 
						"board(%d) get board config fail, retry.....", m_iBoardIndex);
			m_pMainDlg->SetWindowText(szDebug);		

			Sleep(50);		
		}

		iRetryCount+=1;

		if (iRetryCount>3)
		{
			return sdvr_rc;
		}
	}	

	m_iVdoCount = m_BoardConfig.num_cameras_supported;
	m_iAdoCount = m_BoardConfig.num_microphones_supported;
	m_VideoStandard = m_BoardConfig.camera_type;

	DWORD dwVdoStd = m_BoardAttrib.supported_video_stds;

	//If it support NTSC & PAL, refer that no input source
	//Set default to NTSC
	if(dwVdoStd & 0x0001) //PAL
	{
		m_VideoStandard = SDVR_VIDEO_STD_D1_PAL;
	}
	
	if((dwVdoStd >> 1) & 0x0001) //NTSC
	{
		m_VideoStandard = SDVR_VIDEO_STD_D1_NTSC;
	}
	else
	{

	}

	//m_VideoStandard = SDVR_VIDEO_STD_D1_NTSC;

	return sdvr_rc;
}


sdvr_err_e CDVRSystem::SetBoardParam()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[80]; 

	//m_VideoStandard = m_VideoStandard;		// D1_NTSC or 4CIF_NTSC, etc......
	
	m_sdk_params.timeout = m_iTimeOut;	
	/*sdvr_rc = sdvr_set_sdk_params(&m_sdk_params);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_set_sdk_params(%d) = SDVR_ERR_NONE\n", m_iBoardIndex);
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_set_sdk_params(%d) = %d, fail.......\n", m_iBoardIndex, sdvr_rc);
		OutputDebugString(szDebug);
	}*/	

	setDebugEnable(m_bDebugEnable);
	setDebugAVFrames(m_bDebugAVFrames);
	setDebugSDK(m_bDebugSDK);		

	if (!CheckVideoFormatSize())
	{
		sdvr_rc = SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
		sprintf_s(szDebug, sizeof(szDebug), 
					"Board(%d) not support your video standard setting.\n", m_iBoardIndex);
		OutputDebugString(szDebug);
	}

	return sdvr_rc;
}

sdvr_err_e CDVRSystem::SetOSD(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	int i=0;
	for (i=0; i<SDVR_MAX_OSD; i++)
	{
		BOOL bCanSupportNewOSD = TRUE;

		if (m_pEncodeChParam[iCh]->m_chan_def.vpp_mode == SDVR_VPP_MODE_ANALYTICS)
		{
			bCanSupportNewOSD = FALSE;
		}

		sdvr_rc = sdvr_osd_text_config_ex(m_pEncodeChParam[iCh]->m_chan_handle, i, &m_OSD_Config_ex[iCh][i]);
		sdvr_rc = sdvr_osd_text_show(m_pEncodeChParam[iCh]->m_chan_handle, i, 
											m_bEnableOSD[iCh][i] && bCanSupportNewOSD);
	}


	return sdvr_rc;
}

bool CDVRSystem::GetOSDFontString()
{
	bool	bRtn = false;
	CString MyStr;
	//CString strFilePath;

	TCHAR szFilters[] =	_T ("Font files (*.bdf)|*.bdf");
	MyStr.Format("Select Font File(*.bdf) for upload.\n");

	SetCurrentDirectory(m_pMainDlg->m_szCurExeDir);

	CFileDialog dlg(TRUE, MyStr, NULL,
		OFN_HIDEREADONLY, szFilters);		

	if (dlg.DoModal () == IDOK)
	{
		strcpy(m_szFontTable, dlg.GetPathName().GetString());

		CString sTemp = m_szFontTable;

		strcpy(m_szFontTable, sTemp.GetString());

		m_font_table.font_table_id = 8;		// custom id = 8~15;	

		bRtn = true;
	}

	SetCurrentDirectory(m_pMainDlg->m_szCurExeDir);

	return bRtn;
}

sdvr_err_e CDVRSystem::SetOSDFontTable()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	if (m_font_table.font_table_id != 0)
	{
		m_font_table.font_file = m_szFontTable;

		sdvr_rc = sdvr_osd_set_font_table(&m_font_table);	
	}
	else
	{
		sdvr_rc = sdvr_osd_use_font_table(0);	
	}

	return sdvr_rc;
}

void CDVRSystem::updateSDKDebugSettings(char *debug_file_name)
{
	m_sdk_params.debug_flag =  DEBUG_FLAG_ALL - DEBUG_FLAG_OUTPUT_TO_SCREEN;

	if (!getDebugEnable())
		m_sdk_params.debug_flag -= DEBUG_FLAG_DEBUGGING_ON;
	else
		m_sdk_params.debug_flag |= DEBUG_FLAG_DEBUGGING_ON;

	if (!getDebugSDK())
		m_sdk_params.debug_flag -= DEBUG_FLAG_GENERAL_SDK;
	else
		m_sdk_params.debug_flag |= DEBUG_FLAG_GENERAL_SDK;

	if (!getDebugAVFrames())
		m_sdk_params.debug_flag -= DEBUG_FLAG_VIDEO_FRAME;
	else
		m_sdk_params.debug_flag |= DEBUG_FLAG_VIDEO_FRAME;

	if (debug_file_name)
		m_sdk_params.debug_file_name = debug_file_name;

}

bool CDVRSystem::CheckVideoFormatSize()
{
	int i=0, j=0;	
	int iRealVideoStandard = SDVR_VIDEO_STD_NONE;
	int iVdoWidth_Raw = 0;
	int iVdoHeight_Raw = 0;	
	int iTransVdoStandard = (int)m_VideoStandard;
	int iBase=0;

	for (i=0; i<MAX_BOARD_SUPPORT_VIDEO_STANDARD; i++)
	{
		if (m_bSupportVideoStds[i])
		{
			iBase = 1<<i;

			if (iBase == iTransVdoStandard)
			{
				iRealVideoStandard = iTransVdoStandard;
				break;
			}
		}	
	}

	switch (iRealVideoStandard)
	{
	case SDVR_VIDEO_STD_D1_PAL:
			  iVdoWidth_Raw = 720;
			  iVdoHeight_Raw = 576;
			  m_bIsPAL = true;
		break;
	case SDVR_VIDEO_STD_D1_NTSC:
			  iVdoWidth_Raw = 720;
			  iVdoHeight_Raw = 480;
			  break;
	case SDVR_VIDEO_STD_CIF_PAL:
			  iVdoWidth_Raw = 352;
			  iVdoHeight_Raw = 288;
			  m_bIsPAL = true;
			  break;
	case SDVR_VIDEO_STD_CIF_NTSC:
			  iVdoWidth_Raw = 352;
			  iVdoHeight_Raw = 240;
			  break;
	case SDVR_VIDEO_STD_2CIF_PAL:
			  iVdoWidth_Raw = 704;
			  iVdoHeight_Raw = 288;
			  m_bIsPAL = true;
			  break;
	case SDVR_VIDEO_STD_2CIF_NTSC:
			  iVdoWidth_Raw = 704;
			  iVdoHeight_Raw = 240;
			  break;
	case SDVR_VIDEO_STD_4CIF_PAL:
			  iVdoWidth_Raw = 704;
			  iVdoHeight_Raw = 576;
			  m_bIsPAL = true;
			  break;
	case SDVR_VIDEO_STD_4CIF_NTSC:
			  iVdoWidth_Raw = 704;
			  iVdoHeight_Raw = 480;
			  break;
	case SDVR_VIDEO_STD_QCIF_PAL:
			  iVdoWidth_Raw = 176;
			  iVdoHeight_Raw = 144;
			  m_bIsPAL = true;
			  break;
	case SDVR_VIDEO_STD_QCIF_NTSC:
			  iVdoWidth_Raw = 176;
			  iVdoHeight_Raw = 112;
			  break;
	case SDVR_VIDEO_STD_NONE:
			iVdoWidth_Raw = 0;
			iVdoHeight_Raw = 0;
		break;
	}

	if (iVdoWidth_Raw == 0)
	{
		return false;
	}		

	for (i=0; i<MAX_BOARD_CHANNEL; i++)	// Notice: All channel in one board must the same format.
	{		
		if (m_video_res_decimation[i] == SDVR_VIDEO_RES_DECIMATION_EQUAL)
		{
			m_pMainDlg->m_iVdoWidth_Raw[i] = iVdoWidth_Raw;
			m_pMainDlg->m_iVdoHeight_Raw[i] = iVdoHeight_Raw;
		}
		else if (m_video_res_decimation[i] == SDVR_VIDEO_RES_DECIMATION_FOURTH)
		{
			m_pMainDlg->m_iVdoWidth_Raw[i] = iVdoWidth_Raw/2;
			m_pMainDlg->m_iVdoHeight_Raw[i] = iVdoHeight_Raw/2;
		}
		else if (m_video_res_decimation[i] == SDVR_VIDEO_RES_DECIMATION_SIXTEENTH)
		{
			m_pMainDlg->m_iVdoWidth_Raw[i] = iVdoWidth_Raw/4;
			m_pMainDlg->m_iVdoHeight_Raw[i] = iVdoHeight_Raw/4;
		}
		else
		{
			m_pMainDlg->m_iVdoWidth_Raw[i] = 0;
			m_pMainDlg->m_iVdoHeight_Raw[i] = 0;

			return false;
		}
			
	}
	
	return true;
}

void CDVRSystem::GetBoardSupportVideoStandard(sx_uint32 supported_video_stds)
{
	DWORD dwCk = supported_video_stds;

	int i = 0;
	for(i = 0; i < MAX_BOARD_SUPPORT_VIDEO_STANDARD; i++)
	{
		if(dwCk & 0x01)
		{
			*(m_bSupportVideoStds+i) = true;
		}
		else
		{
			*(m_bSupportVideoStds+i) = false;
		}

		dwCk = dwCk >> 1;
	}	
}

