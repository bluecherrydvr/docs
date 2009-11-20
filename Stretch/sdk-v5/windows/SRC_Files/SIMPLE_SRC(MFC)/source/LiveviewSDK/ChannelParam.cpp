// ChannelParam.cpp : 實作檔
//

#include "stdafx.h"
#include "DvrFCD.h"
#include "DvrFCDDlg.h"
#include "ChannelParam.h"


#include "DVRSystem.h"
#include "ChannelParam.h"



CDVRSystem  *  g_pParentDVRSystem;


// CChannelParam

IMPLEMENT_DYNAMIC(CChannelParam, CWnd)

CChannelParam::CChannelParam(CDVRSystem* pDVRSystem, int iBoardIndex, int iChannelIndex)
{	
	m_pDVRSystem = pDVRSystem;
	g_pParentDVRSystem = pDVRSystem;	

	m_iBoardIndex = iBoardIndex;
	m_iChannelIndex = iChannelIndex;
	m_bIsClosingChannel = false;		

	memset(&m_chan_def, 0, sizeof(m_chan_def));
	memset(&m_chan_handle, 0, sizeof(m_chan_handle));	

	// default value.
	m_avg_bitrate = 1000;
	m_max_bitrate = 2000;
	m_bitrate_control = SDVR_BITRATE_CONTROL_CBR;
	m_gop = 30;
	m_quality = 50;	

	int i=0, j=0;

	m_bEnableMD = FALSE;
	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{	
		memset(&m_bFinishMDRectOK[i], false, sizeof(m_bFinishMDRectOK));
		memset(&m_bFinishMDRectOK_Old[i], false, sizeof(m_bFinishMDRectOK_Old));
		
		m_iMDThreshold[i] = 20;

		for (j=0; j<MAX_REGION_PER_LAYER; j++)
		{
			m_rcMDRect[i][j].left = -1;
			m_rcMDRect[i][j].top = -1;
			m_rcMDRect[i][j].right = -1;
			m_rcMDRect[i][j].bottom = -1;			
		}		

		m_pMDRegionListArray[i] = NULL;
	}

	m_iRegionMapWidth = 0;
	m_iRegionMapLine = 0;
	m_iMDValueFrequency = 0;
}

CChannelParam::~CChannelParam()
{
}


BEGIN_MESSAGE_MAP(CChannelParam, CWnd)
END_MESSAGE_MAP()



// CChannelParam 訊息處理常式
sdvr_err_e CChannelParam::SetChannelParam()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];	
	memset(&szDebug, 0, sizeof(szDebug));
	
	m_chan_def.board_index = m_iBoardIndex;
	m_chan_def.chan_num = m_iChannelIndex;
	
	m_pDVRSystem->m_pMainDlg->SendMessage(WM_CREATE_CHANNEL_SETTING_DLG, m_iBoardIndex, m_iChannelIndex);		

	if (!m_pDVRSystem->CheckVideoFormatSize(m_iChannelIndex))
	{
		sdvr_rc = SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) not support your video standard size setting.", m_iBoardIndex+1);
		OutputDebugString(szDebug);
		return sdvr_rc;
	}
	
	m_chan_def.chan_type = SDVR_CHAN_TYPE_ENCODER;
	m_chan_def.video_format_primary = SDVR_VIDEO_ENC_NONE;
	m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_NONE;
	m_chan_def.audio_format = SDVR_AUDIO_ENC_G711;		

	sdvr_rc = sdvr_create_chan(&m_chan_def, &m_chan_handle);	
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_create_chan(%d) = SDVR_ERR_NONE.", m_iChannelIndex+1);
		OutputDebugString(szDebug);		
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_create_chan(%d) = %d.", m_iChannelIndex+1, sdvr_rc);
		OutputDebugString(szDebug);	
		return sdvr_rc;
	}

	if(m_pDVRSystem->m_bIsPAL)
		m_pDVRSystem->m_iFrameRate[m_iChannelIndex] = 25 ;
	else
		m_pDVRSystem->m_iFrameRate[m_iChannelIndex] = 30 ;
	
	sdvr_rc = sdvr_stream_raw_video(m_chan_handle, m_pDVRSystem->m_video_res_decimation[m_iChannelIndex], m_pDVRSystem->m_iFrameRate[m_iChannelIndex], true);		
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_video(%d) = SDVR_ERR_NONE.", m_iChannelIndex+1);
		OutputDebugString(szDebug);		
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_video(%d) fail(%d).", m_iChannelIndex+1, sdvr_rc);
		OutputDebugString(szDebug);	
		return sdvr_rc;
	}

	sdvr_rc = sdvr_stream_raw_audio(m_chan_handle, true);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_audio(%d) = SDVR_ERR_NONE.", m_iChannelIndex+1);
		OutputDebugString(szDebug);					
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_audio(%d) fail(%d).", m_iChannelIndex+1, sdvr_rc);
		OutputDebugString(szDebug);	
		return sdvr_rc;
	}
	
	m_pDVRSystem->SetOSD(m_iChannelIndex);

	int i=0;
	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		if (m_pDVRSystem->m_bIsPAL)
		{
			m_iRegionMapWidth = SDVR_REGION_MAP_D1_WIDTH;
			m_iRegionMapLine = SDVR_REGION_MAP_LINE_PAL;
		}
		else
		{
			m_iRegionMapWidth = SDVR_REGION_MAP_D1_WIDTH;
			m_iRegionMapLine = SDVR_REGION_MAP_LINE_NTSC;
		}

		m_pMDRegionListArray[i] = new char[m_iRegionMapWidth*m_iRegionMapLine];
	}	

	return sdvr_rc;
}


sdvr_err_e CChannelParam::CloseChannel()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];
	memset(&szDebug, 0, sizeof(szDebug));
	sdvr_av_buffer_t  *av_buffer;	
	sdvr_yuv_buffer_t *yuv_buffer;	

	if (sdvr_get_yuv_buffer(m_chan_handle, &yuv_buffer) == SDVR_ERR_NONE)
	{
		sdvr_release_yuv_buffer(yuv_buffer);
	}

	if (sdvr_get_av_buffer(m_chan_handle, SDVR_FRAME_VIDEO_ENCODED_PRIMARY, &av_buffer) == SDVR_ERR_NONE)
	{
		sdvr_release_av_buffer(av_buffer);
	}
	
	sdvr_rc = sdvr_stream_raw_video(m_chan_handle, m_pDVRSystem->m_video_res_decimation[m_iChannelIndex], m_pDVRSystem->m_iFrameRate[m_iChannelIndex], false);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_video_false(%d) = SDVR_ERR_NONE.", m_iChannelIndex+1);
		OutputDebugString(szDebug);
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_video_false false(%d) fail(%d).", m_iChannelIndex+1, sdvr_rc);
		OutputDebugString(szDebug);				
	}

	sdvr_rc = sdvr_stream_raw_audio(m_chan_handle, false);		
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_audio_false(%d) = SDVR_ERR_NONE.", m_iChannelIndex+1);
		OutputDebugString(szDebug);			
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_audio_false(%d) fail(%d).", m_iChannelIndex+1, sdvr_rc);
		OutputDebugString(szDebug);				
	}	

	Sleep(30);

	if (sdvr_get_yuv_buffer(m_chan_handle, &yuv_buffer) == SDVR_ERR_NONE)
	{
		sdvr_release_yuv_buffer(yuv_buffer);
	}

	if (sdvr_get_av_buffer(m_chan_handle, SDVR_FRAME_VIDEO_ENCODED_PRIMARY, &av_buffer) == SDVR_ERR_NONE)
	{
		sdvr_release_av_buffer(av_buffer);
	}

	if (m_chan_handle)
	{	
		sdvr_rc = sdvr_destroy_chan(m_chan_handle);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_destroy_chan(%d) = SDVR_ERR_NONE.", m_iChannelIndex+1);
			OutputDebugString(szDebug);				
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_destroy_chan(%d) fail(%d).", m_iChannelIndex+1, sdvr_rc);
			OutputDebugString(szDebug);	
			return sdvr_rc;
		}

		int i=0;
		for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
		{
			if (m_pMDRegionListArray[i])
			{
				delete m_pMDRegionListArray[i];
				m_pMDRegionListArray[i] = NULL;
			}
		}

	}

	return sdvr_rc;
}
