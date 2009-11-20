// ChannelParam.cpp : 實作檔
//

#include "stdafx.h"
#include "EncodeDvrFCD.h"
#include "ChannelParam.h"
#include "DVRSystem.h"
#include "EncodeDvrFCDDlg.h"
#include "EncodeSetting.h"


// CChannelParam

IMPLEMENT_DYNAMIC(CChannelParam, CWnd)

CChannelParam::CChannelParam(CDVRSystem* pDVRSystem, int iBoardIndex, int iChannelIndex)
{
	m_pDVRSystem = pDVRSystem;

	m_iBoardIndex = iBoardIndex;
	m_iChannelIndex = iChannelIndex;
	m_bIsClosingChannel = false;	

	m_video_res_decimation = SDVR_VIDEO_RES_DECIMATION_EQUAL;

	memset(&m_chan_def, 0, sizeof(m_chan_def));	

	m_ChVdoDataType = CHANNEL_VDO_DATA_TYPE_ENC;
	m_vppMode = SDVR_VPP_MODE_SLATERAL;

	memset(&m_chan_handle, 0, sizeof(m_chan_handle));
	memset(&m_video_enc_chan_param, 0, sizeof(m_video_enc_chan_param));
	memset(&m_alarm_video_enc_params, 0, sizeof(m_alarm_video_enc_params));	

	// default value.
	m_avg_bitrate = 1000;
	m_max_bitrate = 2000;
	m_bitrate_control = SDVR_BITRATE_CONTROL_CBR;
	m_gop = 30;
	m_quality = 50;

	m_bIsModify = false ;
	m_res_decimation = SDVR_VIDEO_RES_DECIMATION_EQUAL;
	m_iSelEncodeCHIdx = -1;
	m_bIsChannelEnableReady = FALSE;

	for(int i = 0 ; i<16 ;i++)
	{
		m_bCheckPrimary[i] = false;
		m_bCheckSecond[i]  = false ;
	}

}

CChannelParam::~CChannelParam()
{
}


BEGIN_MESSAGE_MAP(CChannelParam, CWnd)
END_MESSAGE_MAP()



// CChannelParam 訊息處理常式
BOOL CChannelParam::SetChannelParamAndCreate(CHANNEL_VDO_DATA_TYPE ChVdoDataType)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[64];	

	m_ChVdoDataType = ChVdoDataType;

	m_chan_def.board_index = m_iBoardIndex;
	m_chan_def.chan_num = m_iChannelIndex;

	if (!m_pDVRSystem->CheckVideoFormatSize())
	{
		sdvr_rc = SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
		sprintf_s(szDebug, sizeof(szDebug), "Board(%d) not support your video standard size setting.\n", m_iBoardIndex);
		OutputDebugString(szDebug);
		return sdvr_rc;
	}
	
	if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_RAW)
	{
		m_chan_def.chan_type = SDVR_CHAN_TYPE_ENCODER;
		m_chan_def.video_format_primary = SDVR_VIDEO_ENC_NONE;
		m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_NONE;
		m_chan_def.audio_format = SDVR_AUDIO_ENC_G711;
	}
	else if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_ENC)
	{
		m_EncodeSetting.m_iEncodeCh = m_iChannelIndex ;

		m_EncodeSetting.DoModal();

		m_chan_def.chan_type = SDVR_CHAN_TYPE_ENCODER;
		m_bCheckPrimary[m_iChannelIndex] = m_EncodeSetting.m_bCheckPrimary[m_iChannelIndex];
		m_bCheckSecond[m_iChannelIndex]  = m_EncodeSetting.m_bCheckSecondary[m_iChannelIndex] ;
		m_chan_def.video_format_primary = m_EncodeSetting.m_chan_def.video_format_primary;
		m_chan_def.video_format_secondary = m_EncodeSetting.m_chan_def.video_format_secondary;
		m_chan_def.audio_format = m_EncodeSetting.m_chan_def.audio_format;

		m_chan_def.vpp_mode = SDVR_VPP_MODE_SLATERAL;
	
		sdvr_rc = sdvr_create_chan(&m_chan_def, &m_chan_handle);	
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_create_chan(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
			OutputDebugString(szDebug);		
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_create_chan(%d) = %d\n", m_iChannelIndex, sdvr_rc);
			OutputDebugString(szDebug);		
		}

		m_pDVRSystem->SetOSD(m_iChannelIndex);
	}

	return TRUE;
}

BOOL CChannelParam::EnableRecChannel(bool bEnable , int subEnc)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[64];

	if(subEnc == SDVR_ENC_PRIMARY)
	{

		if(bEnable)
		{
			m_pDVRSystem->SetOSD(m_iChannelIndex);
		}

		applyVideoEncoding(subEnc , bEnable ,m_bIsModify);
		
		sdvr_sub_encoders_e sub_encoder = (sdvr_sub_encoders_e)subEnc;

		sdvr_rc = sdvr_enable_encoder(m_chan_handle, sub_encoder, bEnable);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_enable_encoder(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
			OutputDebugString(szDebug);				
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_enable_encoder(%d) = %d\n", m_iChannelIndex, sdvr_rc);
			OutputDebugString(szDebug);
			return FALSE;
		}
	}
	
	if(subEnc == SDVR_ENC_SECONDARY)
	{
		if(bEnable)
		{
			m_pDVRSystem->SetOSD(m_iChannelIndex);
		}

		applyVideoEncoding(subEnc , bEnable ,m_bIsModify);
	
		sdvr_sub_encoders_e sub_encoder = (sdvr_sub_encoders_e)subEnc;

		sdvr_rc = sdvr_enable_encoder(m_chan_handle, sub_encoder, bEnable);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_enable_encoder(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
			OutputDebugString(szDebug);				
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_enable_encoder(%d) = %d\n", m_iChannelIndex, sdvr_rc);
			OutputDebugString(szDebug);
			return FALSE;
		}
	}
}

BOOL CChannelParam::EnableChannel(bool bEnable)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[64];
	if(m_iChannelIndex > 15)
		m_iChannelIndex = m_iChannelIndex -16 ;
	
	if(bEnable)
	{
		m_pDVRSystem->SetOSD(m_iChannelIndex);
	}

	if(m_pDVRSystem->m_VideoStandard == SDVR_VIDEO_STD_D1_NTSC )
		m_frame_rate = 30 ;
	else if(m_pDVRSystem->m_VideoStandard == SDVR_VIDEO_STD_D1_PAL)
		m_frame_rate = 25 ;


	sdvr_rc = sdvr_stream_raw_video(m_chan_handle, m_pDVRSystem->m_video_res_decimation[m_iChannelIndex], m_frame_rate, bEnable);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
			"sdvr_stream_raw_video(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
		OutputDebugString(szDebug);					
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
			"sdvr_stream_raw_video(%d) = %d\n", m_iChannelIndex, sdvr_rc);
		OutputDebugString(szDebug);	
	    return FALSE;
	}
	
	sdvr_rc = sdvr_stream_raw_audio(m_chan_handle, bEnable);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
		"sdvr_stream_raw_audio(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
		OutputDebugString(szDebug);					
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug),
			"sdvr_stream_raw_audio(%d) = %d\n", m_iChannelIndex, sdvr_rc);
		OutputDebugString(szDebug);
		return FALSE;
	}

	return TRUE;	
}

sdvr_err_e CChannelParam::CloseChannel()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[200];
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
	

	sdvr_rc = sdvr_stream_raw_video(m_chan_handle, m_pDVRSystem->m_video_res_decimation[m_iChannelIndex], m_frame_rate, false);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_video_false(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
		OutputDebugString(szDebug);				
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_video_false false(%d) fail(%d).....\n", m_iChannelIndex, sdvr_rc);
		OutputDebugString(szDebug);		
		return sdvr_rc;
	}

	sdvr_rc = sdvr_stream_raw_audio(m_chan_handle, false);		
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_audio_false(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
		OutputDebugString(szDebug);			
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), "sdvr_stream_raw_audio_false(%d) fail(%d)\n", m_iChannelIndex, sdvr_rc);
		OutputDebugString(szDebug);		
		return sdvr_rc;
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
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_destroy_chan(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
			OutputDebugString(szDebug);				
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), "sdvr_destroy_chan(%d) fail(%d).....\n", m_iChannelIndex, sdvr_rc);
			OutputDebugString(szDebug);	
			return sdvr_rc;
		}
	}

	return sdvr_rc;
}

sdvr_err_e CChannelParam::applyVideoEncoding(int subEnc , bool bRecord , bool m_bIsModify)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;
	char szDebug[80];
	
	sdvr_venc_e EncoderType = SDVR_VIDEO_ENC_NONE ;
	sdvr_sub_encoders_e sub_encoder = (sdvr_sub_encoders_e)subEnc;

	// share param	
	m_video_enc_chan_param.res_decimation = SDVR_VIDEO_RES_DECIMATION_EQUAL;

	m_pDVRSystem->CheckVideoFormatSize();

	if((bRecord) && (!m_bIsModify))
	{
		m_CameraSetup.m_pMainDlg = m_pMainDlg ;
		m_CameraSetup.m_iSubEn = subEnc ;
		m_CameraSetup.m_iCh = m_iChannelIndex ;		
		m_CameraSetup.DoModal();		

		if (subEnc == 0)
		{
			EncoderType = m_EncodeSetting.m_chan_def.video_format_primary;
			sub_encoder = SDVR_ENC_PRIMARY;
		}
		else if (subEnc == 1)
		{
			EncoderType = m_EncodeSetting.m_chan_def.video_format_secondary;
			sub_encoder = SDVR_ENC_SECONDARY;
		}

		m_video_enc_chan_param.frame_rate = m_CameraSetup.m_iFrame;

		switch (EncoderType)
		{
			case SDVR_VIDEO_ENC_JPEG:
				{
					// For now is_image_style is alwasy 0 which means AVI Motion JPEG
					m_video_enc_chan_param.encoder.jpeg.is_image_style = 0;
					m_video_enc_chan_param.encoder.jpeg.quality = m_CameraSetup.m_iQuality;
					break;
				}
			case SDVR_VIDEO_ENC_H264:
				{
					m_video_enc_chan_param.encoder.h264.bitrate_control = m_CameraSetup.m_iBitrate;
					m_video_enc_chan_param.encoder.h264.avg_bitrate = m_CameraSetup.m_iAverbit;
					m_video_enc_chan_param.encoder.h264.max_bitrate = m_CameraSetup.m_iMaxbit;
					m_video_enc_chan_param.encoder.h264.quality = m_CameraSetup.m_iQuality;
					m_video_enc_chan_param.encoder.h264.gop = m_CameraSetup.m_iGOPSize;
					break;
				}
			case SDVR_VIDEO_ENC_MPEG4:
				{
					/*m_video_enc_chan_param.encoder.mpeg4.bitrate_control = m_bitrate_control;
					m_video_enc_chan_param.encoder.mpeg4.avg_bitrate = m_CameraSetup.m_iAverbit;			
					m_video_enc_chan_param.encoder.mpeg4.max_bitrate = m_CameraSetup.m_iMaxbit;
					m_video_enc_chan_param.encoder.mpeg4.quality = m_CameraSetup.m_iQuality;
					m_video_enc_chan_param.encoder.mpeg4.gop = m_CameraSetup.m_iGOPSize;						
					break;*/
				}
			default:
				return SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
		}

		sdvr_rc =  sdvr_set_video_encoder_channel_params(m_chan_handle, sub_encoder, &m_video_enc_chan_param);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_video_encoder_channel_params(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
			//OutputDebugString(szDebug);				
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_video_encoder_channel_params(%d) = %d\n", m_iChannelIndex, sdvr_rc);
			//OutputDebugString(szDebug);		
		}
		return sdvr_rc;
	}

	if((bRecord) && (m_bIsModify))
	{
		m_VideoEncoder.m_pMainDlg = m_pMainDlg ;
		m_VideoEncoder.m_iSubEncoder = subEnc ;
		m_VideoEncoder.m_iChEncoding = m_iChannelIndex;
		m_VideoEncoder.DoModal();

		if (subEnc == 0)
		{
			EncoderType = m_EncodeSetting.m_chan_def.video_format_primary;
			sub_encoder = SDVR_ENC_PRIMARY;
		}
		else if (subEnc == 1)
		{
			EncoderType = m_EncodeSetting.m_chan_def.video_format_secondary;
			sub_encoder = SDVR_ENC_SECONDARY;
		}

		m_video_enc_chan_param.frame_rate = m_VideoEncoder.m_iFrameRate;

		switch (EncoderType)
		{
			case SDVR_VIDEO_ENC_JPEG:
				{
					// For now is_image_style is alwasy 0 which means AVI Motion JPEG
					m_video_enc_chan_param.encoder.jpeg.is_image_style = 0;
					m_video_enc_chan_param.encoder.jpeg.quality = m_VideoEncoder.m_iQuality;
					break;
				}
			case SDVR_VIDEO_ENC_H264:
				{
					m_video_enc_chan_param.encoder.h264.bitrate_control = m_bitrate_control;
					m_video_enc_chan_param.encoder.h264.avg_bitrate = m_VideoEncoder.m_iAverBitRate;
					m_video_enc_chan_param.encoder.h264.max_bitrate = m_VideoEncoder.m_iMaxBitRate;
					m_video_enc_chan_param.encoder.h264.quality = m_VideoEncoder.m_iQuality;
					m_video_enc_chan_param.encoder.h264.gop = m_VideoEncoder.m_iGOP;
					break;
				}
			case SDVR_VIDEO_ENC_MPEG4:
				{
					/*m_video_enc_chan_param.encoder.mpeg4.bitrate_control = m_bitrate_control;
					m_video_enc_chan_param.encoder.mpeg4.avg_bitrate = m_VideoEncoder.m_iAverBitRate;			
					m_video_enc_chan_param.encoder.mpeg4.max_bitrate = m_VideoEncoder.m_iMaxBitRate;
					m_video_enc_chan_param.encoder.mpeg4.quality = m_VideoEncoder.m_iQuality;
					m_video_enc_chan_param.encoder.mpeg4.gop = m_VideoEncoder.m_iGOP;						
					break;*/
				}
			default:
				return SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
		}

		sdvr_rc =  sdvr_set_video_encoder_channel_params(m_chan_handle, sub_encoder, &m_video_enc_chan_param);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_video_encoder_channel_params(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
			//OutputDebugString(szDebug);				
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_video_encoder_channel_params(%d) = %d\n", m_iChannelIndex, sdvr_rc);
			//OutputDebugString(szDebug);		
		}

		sdvr_rc = sdvr_enable_encoder(m_chan_handle, sub_encoder, true);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_video_encoder_channel_params(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
			//OutputDebugString(szDebug);				
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_set_video_encoder_channel_params(%d) = %d\n", m_iChannelIndex, sdvr_rc);
			//OutputDebugString(szDebug);		
		}
		return sdvr_rc;
	}
}
