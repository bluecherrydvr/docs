#pragma once

#include "sdvr_sdk.h"

// CChannelParam
class CDVRSystem;

class CChannelParam : public CWnd
{
	DECLARE_DYNAMIC(CChannelParam)

public:
	CChannelParam(CDVRSystem* pDVRSystem, int iBoardIndex, int iChannelIndex);
	virtual ~CChannelParam();

	CDVRSystem*	  m_pDVRSystem;

	int					m_seq;	//Alice

	BOOL				m_bIsChannelEnableReady;

	int					m_iVdoWidth_Dec;
	int					m_iVdoHeight_Dec;

	//if Encode Data not to decode: -1,
	//The flag is for Encode Channel, not decode channel
	int					m_iDecodeCHIdx;

	int					m_iSelEncodeCHIdx;

	BOOL				m_bIsHwDecodeAction;

	int					m_iBoardIndex;
	int					m_iChannelIndex;

	sdvr_chan_def_t		m_chan_def;	

	sdvr_vpp_mode_e		m_vppMode;
	sdvr_chan_handle_t   m_chan_handle;
	bool                 m_bIsClosingChannel;
	sdvr_video_enc_chan_params_t m_video_enc_chan_param;
	sdvr_alarm_video_enc_params_t m_alarm_video_enc_params;
	CHANNEL_VDO_DATA_TYPE m_ChVdoDataType;		// Raw, encode or decode.
	sdvr_video_res_decimation_e   m_video_res_decimation;	

	// basic video encode type config.
	sx_uint8            m_frame_rate;
	sx_uint8            m_res_decimation;
	sx_uint16           m_avg_bitrate;
	sx_uint16           m_max_bitrate;
	sx_uint8            m_bitrate_control;
	sx_uint8            m_gop;
	sx_uint8            m_quality;

	// Main function
	BOOL	SetChannelParamAndCreate(CHANNEL_VDO_DATA_TYPE ChVdoDataType);
	BOOL	EnableChannel(bool bEnable);

	BOOL	CloseChannel();	

	sdvr_err_e applyVideoEncoding(int subEnc);
	sdvr_err_e applyAlarmVideoEncoding(int subEnc);

protected:
	DECLARE_MESSAGE_MAP()
};


