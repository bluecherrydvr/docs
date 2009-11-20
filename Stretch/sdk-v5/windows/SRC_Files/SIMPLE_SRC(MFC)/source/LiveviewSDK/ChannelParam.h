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

	int				  m_iBoardIndex;
	int                m_iChannelIndex;

	sdvr_chan_def_t		m_chan_def;		
	sdvr_chan_handle_t   m_chan_handle;
	bool                 m_bIsClosingChannel;	

	// basic video encode type config.	
	sx_uint16           m_avg_bitrate;
	sx_uint16           m_max_bitrate;
	sx_uint8            m_bitrate_control;
	sx_uint8            m_gop;
	sx_uint8            m_quality;	

	BOOL			   m_bEnableMD;
	int				   m_iMDThreshold[SDVR_MAX_MD_REGIONS];
	bool  			   m_bFinishMDRectOK[SDVR_MAX_MD_REGIONS][MAX_REGION_PER_LAYER];	
	int                 m_iMDValueFrequency;

	char*              m_pMDRegionListArray[SDVR_MAX_MD_REGIONS];
	int				   m_iRegionMapWidth;
	int                m_iRegionMapLine;

	bool				   m_bFinishMDRectOK_Old[SDVR_MAX_MD_REGIONS][MAX_REGION_PER_LAYER];
	CRect			   m_rcMDRect[SDVR_MAX_MD_REGIONS][MAX_REGION_PER_LAYER];

	// Main function
	sdvr_err_e			SetChannelParam();
	sdvr_err_e           CloseChannel();			

protected:
	DECLARE_MESSAGE_MAP()
};


