// ChannelParam.cpp : 實作檔
//

#include "stdafx.h"
#include "PlayDvrFCD.h"
#include "ChannelParam.h"


#include "DVRSystem.h"
#include "ChannelParam.h"

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

	m_ChVdoDataType = CHANNEL_VDO_DATA_TYPE_RAW;
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

	if(m_pDVRSystem->m_VideoStandard == SDVR_VIDEO_STD_D1_NTSC)
	{
		m_frame_rate = 30;
		m_iVdoWidth_Dec = 720;
		m_iVdoHeight_Dec = 480;
	}
	else if(m_pDVRSystem->m_VideoStandard == SDVR_VIDEO_STD_D1_PAL)
	{
		m_frame_rate = 25;
		m_iVdoWidth_Dec = 720;
		m_iVdoHeight_Dec = 576;
	}

	m_res_decimation = SDVR_VIDEO_RES_DECIMATION_EQUAL;

	m_seq = 1; //Alice
	m_iDecodeCHIdx = -1;
	m_iSelEncodeCHIdx = -1;

	m_bIsHwDecodeAction = TRUE;

	m_bIsChannelEnableReady = FALSE;	
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

	if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_RAW)
	{
		m_chan_def.chan_type = SDVR_CHAN_TYPE_ENCODER;
		m_chan_def.video_format_primary = SDVR_VIDEO_ENC_NONE;
		m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_NONE;
		m_chan_def.audio_format = SDVR_AUDIO_ENC_NONE;
	}
	else if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_ENC)
	{
		m_chan_def.chan_type = SDVR_CHAN_TYPE_ENCODER;

		m_chan_def.video_format_primary = SDVR_VIDEO_ENC_H264;
		//m_chan_def.video_format_primary = SDVR_VIDEO_ENC_JPEG;
		//m_chan_def.video_format_primary = SDVR_VIDEO_ENC_MPEG4;

		//m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_H264;
		m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_NONE;
		m_chan_def.audio_format = SDVR_AUDIO_ENC_G711;
	}
	else if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_DEC)
	{
		m_chan_def.chan_type = SDVR_CHAN_TYPE_DECODER;
		m_chan_def.video_format_primary = SDVR_VIDEO_ENC_H264;

		// There is no secondary decoder
		m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_NONE;
		m_chan_def.audio_format = SDVR_AUDIO_ENC_G711;

		/*
		//The size must suit the input source's.
		if((m_iVdoWidth_Dec == 720) && (m_iVdoHeight_Dec == 480))
		{
			m_chan_def.video_size = SDVR_VIDEO_SIZE_720x480;
		}
		else if((m_iVdoWidth_Dec == 720) && (m_iVdoHeight_Dec == 576))
		{
			m_chan_def.video_size = SDVR_VIDEO_SIZE_720x576;
		}
		*/
	}

	m_chan_def.vpp_mode = SDVR_VPP_MODE_SLATERAL;

	sdvr_rc = sdvr_create_chan(&m_chan_def, &m_chan_handle);	
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_ENC)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_create_chan(Encode CH:%d) = SDVR_ERR_NONE\n", 
						m_iChannelIndex);
		}
		else if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_DEC)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_create_chan(Decode CH:%d) = SDVR_ERR_NONE\n", 
				m_iChannelIndex);
		}
	}
	else
	{
		if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_ENC)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_create_chan(Encode CH:%d) = %d\n", 
				m_iChannelIndex, sdvr_rc);
		}
		else if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_DEC)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_create_chan(Decode CH:%d) = %d\n", 
				m_iChannelIndex, sdvr_rc);
		}
	}

	OutputDebugString(szDebug);

	if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_DEC)
		sdvr_rc = sdvr_set_decoder_size(m_chan_handle, m_iVdoWidth_Dec, m_iVdoHeight_Dec);	
	

	return TRUE;
}

BOOL CChannelParam::EnableChannel(bool bEnable)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	
	char szDebug[64];	

	if ((m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_RAW) ||
		(m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_ENC))
	{
		if(bEnable)
		{
			m_pDVRSystem->SetOSD(m_iChannelIndex);
		}
	}

	if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_RAW)
	{
		sdvr_rc = sdvr_stream_raw_video(m_chan_handle, m_pDVRSystem->m_video_res_decimation[m_iChannelIndex], 
										m_video_enc_chan_param.frame_rate, bEnable);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_stream_raw_video(CH:%d, Enable:%d) = SDVR_ERR_NONE\n", 
						m_iChannelIndex, bEnable);
			OutputDebugString(szDebug);					
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
				"sdvr_stream_raw_video(CH%d, Enable:%d) = %d\n", 
				m_iChannelIndex, bEnable, sdvr_rc);
			OutputDebugString(szDebug);	

			return FALSE;
		}

		sdvr_rc = sdvr_stream_raw_audio(m_chan_handle, bEnable);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_stream_raw_audio(CH:%d, Enable:%d) = SDVR_ERR_NONE\n", 
						m_iChannelIndex, bEnable);
			OutputDebugString(szDebug);					
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_stream_raw_audio(CH:%d, Enable:%d) = %d\n", 
						m_iChannelIndex, sdvr_rc, bEnable);
			OutputDebugString(szDebug);

			return FALSE;
		}
	}
	else if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_ENC)
	{
		applyVideoEncoding(0);
		applyAlarmVideoEncoding(0);

		sdvr_rc = sdvr_enable_encoder(m_chan_handle, SDVR_ENC_PRIMARY, bEnable);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_enable_encoder(CH:%d, Enable:%d) = SDVR_ERR_NONE\n", 
						m_iChannelIndex, bEnable);
			OutputDebugString(szDebug);				
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_enable_encoder(CH:%d, Enable:%d) = %d\n", 
						m_iChannelIndex, sdvr_rc, bEnable);
			OutputDebugString(szDebug);

			return FALSE;
		}
	}
	else if (m_ChVdoDataType == CHANNEL_VDO_DATA_TYPE_DEC)
	{
		sdvr_rc = sdvr_stream_raw_video(m_chan_handle, m_pDVRSystem->m_video_res_decimation[m_iChannelIndex], 
											m_frame_rate, bEnable);		
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_stream_raw_video(CH:%d, Enable:%d) = SDVR_ERR_NONE\n", 
						m_iChannelIndex, bEnable);
			OutputDebugString(szDebug);					
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_stream_raw_video(CH:%d, Enable:%d) = %d\n", 
						m_iChannelIndex, bEnable, sdvr_rc);
			OutputDebugString(szDebug);		

			return FALSE;
		}

		sdvr_rc = sdvr_stream_raw_audio(m_chan_handle, bEnable);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_stream_raw_audio(CH:%d, Enable:%d) = SDVR_ERR_NONE\n", 
						m_iChannelIndex, bEnable);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_stream_raw_audio(CH:%d, Enable:%d) = %d\n", 
						m_iChannelIndex, sdvr_rc, bEnable);
			OutputDebugString(szDebug);

			return FALSE;
		}

		sdvr_rc = sdvr_enable_decoder(m_chan_handle, bEnable);
		if (sdvr_rc == SDVR_ERR_NONE)
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_enable_decoder(CH:%d, Enable:%d) = SDVR_ERR_NONE\n", 
						m_iChannelIndex, bEnable);
			OutputDebugString(szDebug);
		}
		else
		{
			sprintf_s(szDebug, sizeof(szDebug), 
						"sdvr_enable_decoder(CH:%d, Enable:%d) = %d\n", 
						m_iChannelIndex, sdvr_rc, bEnable);
			OutputDebugString(szDebug);

			return FALSE;
		}
	}

	m_bIsChannelEnableReady = bEnable;

	return TRUE;
}

BOOL CChannelParam::CloseChannel()
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

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

	if (sdvr_get_av_buffer(m_chan_handle, SDVR_FRAME_VIDEO_ENCODED_SECONDARY, &av_buffer) == SDVR_ERR_NONE)
	{
		sdvr_release_av_buffer(av_buffer);
	}

	if (sdvr_get_av_buffer(m_chan_handle, SDVR_FRAME_AUDIO_ENCODED, &av_buffer) == SDVR_ERR_NONE)
	{
		sdvr_release_av_buffer(av_buffer);
	}

	if (m_chan_handle)
	{
		sdvr_rc = sdvr_destroy_chan(m_chan_handle);
	}

	return TRUE;
}

sdvr_err_e CChannelParam::applyVideoEncoding(int subEnc)
{
	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;		
	char szDebug[80];	

	sdvr_venc_e EncoderType = SDVR_VIDEO_ENC_NONE;
	sdvr_sub_encoders_e sub_encoder = SDVR_ENC_PRIMARY;

	if (subEnc == 0)
	{
		EncoderType = m_chan_def.video_format_primary;
		sub_encoder = SDVR_ENC_PRIMARY;
	}
	else if (subEnc == 1)
	{
		EncoderType = m_chan_def.video_format_secondary;
		sub_encoder = SDVR_ENC_SECONDARY;
	}

	// share param	
	m_video_enc_chan_param.res_decimation = m_pDVRSystem->m_video_res_decimation[m_iChannelIndex];

	m_video_enc_chan_param.frame_rate = m_frame_rate;

	switch (EncoderType)
	{
	case SDVR_VIDEO_ENC_JPEG:
		{
			// For now is_image_style is alwasy 0 which means AVI Motion JPEG
			m_video_enc_chan_param.encoder.jpeg.is_image_style = 0;
			m_video_enc_chan_param.encoder.jpeg.quality = m_quality;
			break;
		}
	case SDVR_VIDEO_ENC_H264:
		{
			m_video_enc_chan_param.encoder.h264.bitrate_control = m_bitrate_control;
			m_video_enc_chan_param.encoder.h264.avg_bitrate = m_avg_bitrate;
			m_video_enc_chan_param.encoder.h264.max_bitrate = m_max_bitrate;
			m_video_enc_chan_param.encoder.h264.quality = m_quality;
			m_video_enc_chan_param.encoder.h264.gop = m_gop;			
			break;
		}
	case SDVR_VIDEO_ENC_MPEG4:
		{
			m_video_enc_chan_param.encoder.mpeg4.bitrate_control = m_bitrate_control;
			m_video_enc_chan_param.encoder.mpeg4.avg_bitrate = m_avg_bitrate;			
			m_video_enc_chan_param.encoder.mpeg4.max_bitrate = m_max_bitrate;
			m_video_enc_chan_param.encoder.mpeg4.quality = m_quality;
			m_video_enc_chan_param.encoder.mpeg4.gop = m_gop;						
			break;
		}

	default:
		return SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
	}

	sdvr_rc =  sdvr_set_video_encoder_channel_params(m_chan_handle, sub_encoder, &m_video_enc_chan_param);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_set_video_encoder_channel_params(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
		OutputDebugString(szDebug);				
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_set_video_encoder_channel_params(%d) = %d\n", m_iChannelIndex, sdvr_rc);
		OutputDebugString(szDebug);		
	}


	return sdvr_rc;
}


sdvr_err_e CChannelParam::applyAlarmVideoEncoding(int subEnc)
{
	char szDebug[80];

	sdvr_err_e sdvr_rc = SDVR_ERR_NONE;	

	sdvr_sub_encoders_e sub_encoder = SDVR_ENC_PRIMARY;
	sdvr_venc_e EncoderType = SDVR_VIDEO_ENC_NONE;

	if (subEnc == 0)
	{
		EncoderType = m_chan_def.video_format_primary;
		sub_encoder = SDVR_ENC_PRIMARY;
	}
	else if (subEnc == 1)
	{
		EncoderType = m_chan_def.video_format_secondary;
		sub_encoder = SDVR_ENC_SECONDARY;
	}

	// share param
	m_alarm_video_enc_params.enable = 0;
	m_alarm_video_enc_params.min_on_seconds = 60;
	m_alarm_video_enc_params.min_off_seconds = 30;

	m_alarm_video_enc_params.frame_rate = m_frame_rate;

	switch (EncoderType)
	{
	case SDVR_VIDEO_ENC_JPEG:
		{
			// For now is_image_style is alwasy 0 which means AVI Motion JPEG
			m_alarm_video_enc_params.encoder.jpeg.is_image_style = 0;
			m_alarm_video_enc_params.encoder.jpeg.quality = 50;
			break;
		}
	case SDVR_VIDEO_ENC_H264:
		{
			m_alarm_video_enc_params.encoder.h264.gop = 30;
			m_alarm_video_enc_params.encoder.h264.bitrate_control = 1;
			m_alarm_video_enc_params.encoder.h264.avg_bitrate = 1000;
			m_alarm_video_enc_params.encoder.h264.max_bitrate = 2000;
			m_alarm_video_enc_params.encoder.h264.quality = 50;
			break;
		}
	case SDVR_VIDEO_ENC_MPEG4:
		{
			m_alarm_video_enc_params.encoder.mpeg4.avg_bitrate = 1000;
			m_alarm_video_enc_params.encoder.mpeg4.bitrate_control = 1;
			m_alarm_video_enc_params.encoder.mpeg4.gop = 30;
			m_alarm_video_enc_params.encoder.mpeg4.max_bitrate = 2000;
			m_alarm_video_enc_params.encoder.mpeg4.quality =50;
			break;
		}

	default:
		return SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
	}	

	sdvr_rc = sdvr_set_alarm_video_encoder_params(m_chan_handle, sub_encoder, &m_alarm_video_enc_params);
	if (sdvr_rc == SDVR_ERR_NONE)
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_set_alarm_video_encoder_params(%d) = SDVR_ERR_NONE\n", m_iChannelIndex);
		OutputDebugString(szDebug);		
	}
	else
	{
		sprintf_s(szDebug, sizeof(szDebug), 
					"sdvr_set_alarm_video_encoder_params(%d) = %d\n", m_iChannelIndex, sdvr_rc);
		OutputDebugString(szDebug);		
	}


	return sdvr_rc;

}

