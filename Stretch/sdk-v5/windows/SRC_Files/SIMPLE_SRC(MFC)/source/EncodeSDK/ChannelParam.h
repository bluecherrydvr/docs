#pragma once

#include "sdvr_sdk.h"
#include "CameraSetup.h"
#include "VideoEncoder.h"
#include "EncodeSetting.h"


// CChannelParam
class CDVRSystem;
class CVideoEncoder;
class CDvrFCDDlg;
class CEncodeSetting;

class CChannelParam : public CWnd
{
	DECLARE_DYNAMIC(CChannelParam)

public:
	CChannelParam(CDVRSystem* pDVRSystem, int iBoardIndex, int iChannelIndex);
	virtual ~CChannelParam();

	CDVRSystem							*m_pDVRSystem;
	CDvrFCDDlg							*m_pMainDlg;
	CCameraSetup						m_CameraSetup;	
	CVideoEncoder						m_VideoEncoder;	
	CEncodeSetting						m_EncodeSetting ;	

	BOOL								m_bIsChannelEnableReady;
	int									m_iSelEncodeCHIdx;
	int									m_iBoardIndex;
	int									m_iChannelIndex;
	
	sdvr_chan_def_t						m_chan_def;	
	sdvr_vpp_mode_e						m_vppMode;
	sdvr_chan_handle_t					m_chan_handle;
	
	bool								m_bIsClosingChannel;
	bool								m_bIsModify ; 
	bool								m_bCheckPrimary[MAX_BOARD_CHANNEL];
	bool								m_bCheckSecond[MAX_BOARD_CHANNEL];
		
	sdvr_video_enc_chan_params_t		m_video_enc_chan_param;
	sdvr_alarm_video_enc_params_t		m_alarm_video_enc_params;
	CHANNEL_VDO_DATA_TYPE				m_ChVdoDataType;		// Raw, encode or decode.
	sdvr_video_res_decimation_e			m_video_res_decimation;	

	// basic video encode type config.
	sx_uint8							m_frame_rate;
	sx_uint8							m_res_decimation;
	sx_uint16							m_avg_bitrate;
	sx_uint16							m_max_bitrate;
	sx_uint8							m_bitrate_control;
	sx_uint8							m_gop;
	sx_uint8							m_quality;

	// Main function
	BOOL								SetChannelParamAndCreate(CHANNEL_VDO_DATA_TYPE ChVdoDataType);
	BOOL								EnableChannel( bool bEnable);
	BOOL								EnableRecChannel(bool bEnable , int subEnc);

	sdvr_err_e							CloseChannel();	

	sdvr_err_e							applyVideoEncoding(int subEnc , bool bRecord , bool m_bIsModify);

protected:
	DECLARE_MESSAGE_MAP()
};


