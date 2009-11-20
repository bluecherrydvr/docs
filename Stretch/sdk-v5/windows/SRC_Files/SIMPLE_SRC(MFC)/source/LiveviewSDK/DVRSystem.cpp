// DVRSystem.cpp : 實作檔
//

#include "stdafx.h"
#include "DvrFCD.h"
#include "DvrFCDDlg.h"
#include "DVRSystem.h"


CDVRSystem*	  g_pDVRSystem;
// CDVRSystem

IMPLEMENT_DYNAMIC(CDVRSystem, CWnd)

CDVRSystem::CDVRSystem(CDvrFCDDlg* pMainDlg, int iBoardIndex)
{
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	m_iBoardIndex = iBoardIndex;
	m_pMainDlg = pMainDlg;	

	g_pDVRSystem = this;

	int i=0, j=0;

	for (i=0; i<MAX_BOARD_CHANNEL; i++)
	{
		m_bEnableChannel[i] = false;	
		m_pChannelParam[i] = NULL;		
		m_video_res_decimation[i] = SDVR_VIDEO_RES_DECIMATION_CIF;		
		m_iFrameRate[i] = 30;
		m_bMute[i] = TRUE;
		for (j=0; j<SDVR_MAX_OSD; j++)
		{
			m_bEnableOSD[i][j] = FALSE;
			memset(&m_szOSDText[i][j], 0, sizeof(m_szOSDText[i][j]));
			memset(&m_OSD_Config_ex[i][j], 0, sizeof(m_OSD_Config_ex[i][j]));	
			m_OSD_Config_ex[i][j].translucent = 100;
		}
		m_OSD_Config_ex[i][0].dts_format = SDVR_OSD_DTS_MDY_12H;
		m_OSD_Config_ex[i][0].location_ctrl = SDVR_LOC_BOTTOM_RIGHT;
		m_bEnableOSD[i][0] = TRUE;
	}
	
	memset(&m_bSupportVideoStds, false, sizeof(m_bSupportVideoStds));	
	m_bBoardConnected = false;	
	memset(&m_sdk_params, 0, sizeof(m_sdk_params));
	memset(&m_fwVersion, 0, sizeof(m_fwVersion));
	memset(&m_BoardAttrib, 0, sizeof(m_BoardAttrib));
	memset(&m_PCIAttrib, 0, sizeof(m_PCIAttrib));
	memset(&m_BoardConfig, 0, sizeof(m_BoardConfig));

	m_VideoStandard = SDVR_VIDEO_STD_D1_NTSC;		// default
	
	setDebugEnable(false);
	setDebugSDK(false);
	// By default we don't debug A/V frames. It uses lots of
	// harddisk space
	setDebugAVFrames(false);

	// By default Start Code Encoded is on to be done
	// on the host side.
	m_bH264SCE = true;		

	// The default PCI driver timeout is ?? seconds
	m_iTimeOut = 5;	

	m_bIsPAL = false;	

	memset(&m_font_table, 0, sizeof(m_font_table));
	m_font_table.font_table_format = SDVR_FT_FORMAT_BDF;
	m_strFontTable = "";

	m_iBoardSetWidth = 0;
	m_iBoardSetHeight = 0;
	
	for (i=0; i<MAX_RELAYS_SENSORS; i++)
	{
		m_bEnableSensors[i] = FALSE;
		m_bEnableRelays[i] = FALSE;
		m_iTriggerMode[i] = SENSOR_EDGE_TRIGGER;
	}
}

CDVRSystem::~CDVRSystem()
{
}


BEGIN_MESSAGE_MAP(CDVRSystem, CWnd)
END_MESSAGE_MAP()


sdvr_err_e CDVRSystem::PreGetPCIAttribute(sdvr_pci_attrib_t* pPCI_Attribute)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];
	bool bBoardConnectOK = false;

	sdvr_rc = sdvr_get_pci_attrib(m_iBoardIndex, pPCI_Attribute);

	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "CS6System::board(%d) Pre get pci attribute OK.....\n", m_iBoardIndex);		
		//		OutputDebugString(szDebug);		
	}
	else
	{	
		sprintf_s(szDebug, sizeof(szDebug), 
			"CS6System::board(%d) Pre get PCI attribute Fail(%d).....\n", m_iBoardIndex, sdvr_rc);		
		//OutputDebugString(szDebug);	

		sprintf_s(szDebug, sizeof(szDebug), 
			"PreGetPCIAttribute(Board=%d) fail!, First Connect Board.\n", m_iBoardIndex);
		OutputDebugString(szDebug);		

		sdvr_rc = sdvr_board_connect(m_iBoardIndex, m_VideoStandard, m_bH264SCE);

		if (sdvr_rc == SDVR_ERR_NONE)
		{				
			sprintf_s(szDebug, sizeof(szDebug), 
				"PreGetPCIAttribute(Board=%d) fail!, Re-Do it.\n", m_iBoardIndex);
			OutputDebugString(szDebug);

			sdvr_rc = sdvr_get_pci_attrib(m_iBoardIndex, pPCI_Attribute);
			if (sdvr_rc != SDVR_ERR_NONE)
			{
				sprintf_s(szDebug, sizeof(szDebug), 
					"PreGetPCIAttribute(Board=%d) do sdvr_get_pci_attrib still fail(%d)!\n", m_iBoardIndex, sdvr_rc);
				OutputDebugString(szDebug);
			}
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"PreGetPCIAttribute(Board=%d) sdvr_board_connect(%d), Re-Do it!\n", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);
		}

		sdvr_rc = sdvr_board_disconnect(m_iBoardIndex);		

	}	

	return sdvr_rc;
}




// CDVRSystem 訊息處理常式
sdvr_err_e CDVRSystem::UpgradeFW(char* pszFWpath)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];	
	memset(&szDebug, 0, sizeof(szDebug));

	bool bNeed = true;

	if (pszFWpath == "")
	{
		bNeed = false;
	}	

	sprintf_s(szDebug, sizeof(szDebug), "board(%d) is upgrading firmware.",m_iBoardIndex);
	m_pMainDlg->SetWindowText(szDebug);
	
	if (bNeed)
	{			
		sdvr_rc = sdvr_upgrade_firmware(m_iBoardIndex, pszFWpath);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), "board(%d) upgrade firmware OK.", m_iBoardIndex);
			m_pMainDlg->SetWindowText(szDebug);			
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "board(%d) upgrade firmware fail(%d).", m_iBoardIndex, sdvr_rc);
			m_pMainDlg->SetWindowText(szDebug);
		}
	}		

	return sdvr_rc;
}


sdvr_err_e CDVRSystem::OpenBoard()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];	
	memset(&szDebug, 0, sizeof(szDebug));

	sprintf_s(szDebug, sizeof(szDebug), "board(%d) is connecting.", m_iBoardIndex);
	m_pMainDlg->SetWindowText(szDebug);

	sdvr_rc = sdvr_enable_auth_key(false);
		
	sdvr_rc = sdvr_board_connect(m_iBoardIndex, m_VideoStandard, m_bH264SCE);	
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) board connect OK.", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) board connect fail(%d).", m_iBoardIndex, sdvr_rc);
		m_pMainDlg->SetWindowText(szDebug);
		OutputDebugString(szDebug);
	}	

	if ((sdvr_rc == SDVR_ERR_NONE) || (sdvr_rc == SDVR_DRV_ERR_BOARD_IN_USE))
	{		
		m_bBoardConnected = true;	

		time_t pt;
		SYSTEMTIME st;
		GetLocalTime(&st);
		FILETIME ft;
		SystemTimeToFileTime( &st, &ft );

		LONGLONG ll;

		ULARGE_INTEGER ui;
		ui.LowPart = ft.dwLowDateTime;
		ui.HighPart = ft.dwHighDateTime;

		ll = (ft.dwHighDateTime << 32) + ft.dwLowDateTime;

		pt = (DWORD)((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);

		sdvr_rc = sdvr_set_date_time(m_iBoardIndex, pt);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_set_date_time(%d) = SDVR_ERR_NONE.", m_iBoardIndex);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_set_date_time(%d) = %d, fail.", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);
		}

		sdvr_rc = sdvr_get_firmware_version_ex(m_iBoardIndex, &m_fwVersion);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_get_firmware_version_ex(%d) = SDVR_ERR_NONE, fw_ver(%d.%d.%d.%d), build_date(%d/%d/%d), bootloder_ver(%d.%d), bsp_ver(%d.%d).", 
				m_iBoardIndex+1, m_fwVersion.fw_major, m_fwVersion.fw_minor, m_fwVersion.fw_revision, m_fwVersion.fw_build,
				m_fwVersion.fw_build_year, m_fwVersion.fw_build_month, m_fwVersion.fw_build_day,
				m_fwVersion.bootloader_major, m_fwVersion.bootloader_minor, m_fwVersion.bsp_major, m_fwVersion.bsp_minor);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_get_firmware_version_ex(%d) = %d, fail.", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);
		}		

		sdvr_rc = sdvr_set_watchdog_state(m_iBoardIndex, 0);	//	Disable watchdog.
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_set_watchdog_state(%d) = SDVR_ERR_NONE.", m_iBoardIndex);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_set_watchdog_state(%d) fail(%d).", m_iBoardIndex, sdvr_rc);
			OutputDebugString(szDebug);
		}			
	}
	else
	{
		m_bBoardConnected = false;
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) board connect fail(%d).",m_iBoardIndex, sdvr_rc);
		m_pMainDlg->SetWindowText(szDebug);
		OutputDebugString(szDebug);
	}	

	return sdvr_rc;
}


sdvr_err_e CDVRSystem::ConnectChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	if (iCh>=m_iVdoCount)
	{
		sdvr_rc = SDVR_DRV_ERR_NO_CHANNELS;
		return sdvr_rc;
	}

	if (!m_bEnableChannel[iCh])
	{
		m_pChannelParam[iCh] = NULL;
		m_pChannelParam[iCh] = new CChannelParam(this, m_iBoardIndex, iCh);

		sdvr_rc = m_pChannelParam[iCh]->SetChannelParam();

		if (sdvr_rc == SDVR_ERR_NONE)
		{		
			m_bEnableChannel[iCh] = true;
			sprintf_s(szDebug, sizeof(szDebug), "Channel(%d) connect OK.", iCh+1);
			m_pMainDlg->SetWindowText(szDebug);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "Channel(%d) connect fail(%d).", iCh+1, sdvr_rc);
			m_pMainDlg->SetWindowText(szDebug);			
			OutputDebugString(szDebug);
		}
	}
	else
	{
		sdvr_rc = SDVR_DRV_ERR_CHANNEL_NOT_ACTIVE;
	}	

	return sdvr_rc;
}


sdvr_err_e CDVRSystem::DisConnectChannel(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	if (iCh>=m_iVdoCount)
	{
		sdvr_rc = SDVR_DRV_ERR_NO_CHANNELS;
		return sdvr_rc;
	}

	if (m_pChannelParam[iCh])
	{
		m_bEnableChannel[iCh] = false;
		m_pChannelParam[iCh]->m_bIsClosingChannel = true;	

		sdvr_rc = m_pChannelParam[iCh]->CloseChannel();

		if (sdvr_rc == SDVR_ERR_NONE)
		{				
			sprintf_s(szDebug, sizeof(szDebug), "Channel(%d) disconnect OK.", iCh+1);
			m_pMainDlg->SetWindowText(szDebug);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "Channel(%d) disconnect fail(%d).", iCh+1, sdvr_rc);
			m_pMainDlg->SetWindowText(szDebug);			
			OutputDebugString(szDebug);
		}

		delete m_pChannelParam[iCh];
		m_pChannelParam[iCh] = NULL;		
	}	

	return sdvr_rc;
}



sdvr_err_e CDVRSystem::CloseBoard()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	sdvr_rc = sdvr_set_watchdog_state(m_iBoardIndex, 0);	//	Disable watchdog.

	sdvr_rc = sdvr_board_disconnect(m_iBoardIndex);	
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_board_disconnect(%d) = SDVR_ERR_NONE.", m_iBoardIndex+1);
		OutputDebugString(szDebug);		
	}
	else	 // Although fail but seems not have any bad outcome.
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_board_disconnect(%d) = %d.", m_iBoardIndex+1, sdvr_rc);
		OutputDebugString(szDebug);				
	}

	sprintf_s(szDebug, sizeof(szDebug), 
		"All board disconnect OK.....");
	m_pMainDlg->SetWindowText(szDebug);

	m_bBoardConnected = false;
	return sdvr_rc;
}


sdvr_err_e CDVRSystem::GetBoardParam()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200]; 	
	memset(&szDebug, 0, sizeof(szDebug));

	sprintf_s(szDebug, sizeof(szDebug), "board(%d) is getting board config.", m_iBoardIndex);
	m_pMainDlg->SetWindowText(szDebug);
	//OutputDebugString(szDebug);

	sdvr_rc = sdvr_get_board_attributes(m_iBoardIndex, &m_BoardAttrib);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) get board attributes OK.", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
		//OutputDebugString(szDebug);

		sprintf_s(szDebug, sizeof(szDebug), 
			"board(%d) get board attributes OK.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
			"sdvr_get_board_attributes(%d) = %d, fail.....\n", m_iBoardIndex, sdvr_rc);
		//OutputDebugString(szDebug);

		sprintf_s(szDebug, sizeof(szDebug), 
			"board(%d) get board attributes fail.....", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
	}
	GetBoardSupportVideoStandard(m_BoardAttrib.supported_video_stds);	
	
	sdvr_rc = sdvr_get_sdk_params(&m_sdk_params);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) get sdk params OK.",m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) get sdk params fail(%d).",m_iBoardIndex, sdvr_rc);
		m_pMainDlg->SetWindowText(szDebug);
		OutputDebugString(szDebug);
	}	

	sdvr_rc = sdvr_get_pci_attrib(m_iBoardIndex, &m_PCIAttrib);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) get pci attrib OK.", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) get pci attrib fail(%d).", m_iBoardIndex, sdvr_rc);
		m_pMainDlg->SetWindowText(szDebug);
		OutputDebugString(szDebug);
	}
	
	sdvr_rc = sdvr_get_board_config(m_iBoardIndex, &m_BoardConfig);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) get board config OK.", m_iBoardIndex);
		m_pMainDlg->SetWindowText(szDebug);
		OutputDebugString(szDebug);		
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "board(%d) get board config fail(%d).", m_iBoardIndex, sdvr_rc);
		m_pMainDlg->SetWindowText(szDebug);		
		OutputDebugString(szDebug);		
	}	

	m_iVdoCount = m_BoardConfig.num_cameras_supported;
	m_iAdoCount = m_BoardConfig.num_microphones_supported;
	m_VideoStandard = m_BoardConfig.camera_type;		// just means now board setting.

	return sdvr_rc;
}


sdvr_err_e CDVRSystem::SetBoardParam()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200]; 
	memset(&szDebug, 0, sizeof(szDebug));
	
	m_pMainDlg->SendMessage(WM_CREATE_BOARD_SETTING_DLG, m_iBoardIndex);
	
	m_sdk_params.timeout = m_iTimeOut;	
	/*sdvr_rc = sdvr_set_sdk_params(&m_sdk_params);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_set_sdk_params(%d) = SDVR_ERR_NONE.", m_iBoardIndex+1);
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_set_sdk_params(%d) = %d, fail.", m_iBoardIndex+1, sdvr_rc);
		OutputDebugString(szDebug);
	}*/

	if (!CheckVideoFormatSize(m_iBoardIndex))
	{
		sdvr_rc = SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
		sprintf_s(szDebug, sizeof(szDebug), 
			"Board(%d) not support your video standard setting.\n", m_iBoardIndex);
		OutputDebugString(szDebug);
	}

	setDebugEnable(m_bDebugEnable);
	setDebugAVFrames(m_bDebugAVFrames);
	setDebugSDK(m_bDebugSDK);	

	return sdvr_rc;
}


sdvr_err_e CDVRSystem::SetOSD(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	int i=0;
	for (i=0; i<SDVR_MAX_OSD; i++)
	{
		BOOL bCanSupportNewOSD = TRUE;

		if (m_pChannelParam[iCh]->m_chan_def.vpp_mode == SDVR_VPP_MODE_ANALYTICS)
		{
			bCanSupportNewOSD = FALSE;
		}

		sdvr_rc = sdvr_osd_text_config_ex(m_pChannelParam[iCh]->m_chan_handle, i, &m_OSD_Config_ex[iCh][i]);
		sdvr_rc = sdvr_osd_text_show(m_pChannelParam[iCh]->m_chan_handle, i, m_bEnableOSD[iCh][i]&&bCanSupportNewOSD);
	}


	return sdvr_rc;
}


sdvr_err_e CDVRSystem::ResetFrameRate(int iCh)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	sdvr_rc = sdvr_stream_raw_video(m_pChannelParam[iCh]->m_chan_handle, m_video_res_decimation[iCh], m_iFrameRate[iCh], true);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) channel(%d) ResetFrameRates(%d) = SDVR_ERR_NONE.", m_iBoardIndex+1, iCh+1, m_iFrameRate[iCh]);
		OutputDebugString(szDebug);		
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) channel(%d) ResetFrameRate(%d) fail(%d).", m_iBoardIndex+1, iCh+1, m_iFrameRate[iCh], sdvr_rc);
		OutputDebugString(szDebug);		
	}


	return sdvr_rc;
}


sdvr_err_e CDVRSystem::SetFontTable()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));

	if (m_font_table.font_table_id != 0)
	{
		m_font_table.font_file = (char*)m_strFontTable.GetString();		

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
	char szDebug[200];	
	memset(&szDebug, 0, sizeof(szDebug));

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


bool CDVRSystem::CheckVideoFormatSize(int iCh)
{
	int i=0, j=0;		
	int iVdoWidth_Raw = 0;
	int iVdoHeight_Raw = 0;		

	switch (m_VideoStandard)
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

	m_iBoardSetWidth = iVdoWidth_Raw;
	m_iBoardSetHeight = iVdoHeight_Raw;
			
	if (m_video_res_decimation[iCh] == SDVR_VIDEO_RES_DECIMATION_EQUAL)
	{
		m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = iVdoWidth_Raw;
		m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = iVdoHeight_Raw;
	}
	else if (m_video_res_decimation[iCh] == SDVR_VIDEO_RES_DECIMATION_CIF)
	{
		if (m_bIsPAL)
		{
			m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = 352;
			m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = 288;
		}
		else
		{
			m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = 352;
			m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = 240;
		}			
	}
	else if (m_video_res_decimation[iCh] == SDVR_VIDEO_RES_DECIMATION_QCIF)
	{
		if (m_bIsPAL)
		{
			m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = 176;
			m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = 144;
		}
		else
		{
			m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = 176;
			m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = 112;	
		}
	}
	else if (m_video_res_decimation[iCh] == SDVR_VIDEO_RES_DECIMATION_HALF)
	{
		m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = iVdoWidth_Raw;
		m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = iVdoHeight_Raw/2;	
	}
	else if (m_video_res_decimation[iCh] == SDVR_VIDEO_RES_DECIMATION_DCIF)
	{
		if (m_bIsPAL)
		{
			m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = 528;
			m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = 384;
		}
		else
		{
			m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = 528;
			m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = 320;	
		}
	}
	else
	{
		m_pMainDlg->m_iVdoWidth_Raw[m_iBoardIndex][iCh] = 0;
		m_pMainDlg->m_iVdoHeight_Raw[m_iBoardIndex][iCh] = 0;

		return false;
	}	
	
	return true;
}


void CDVRSystem::GetBoardSupportVideoStandard(sx_uint32 supported_video_stds)
{
	DWORD dwCk = supported_video_stds;

	int i = 0;
	for(i=0; i<MAX_BOARD_SUPPORT_VIDEO_STANDARD; i++)
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
