#pragma once

#include "sdvr_sdk.h"
#include "ChannelParam.h"

#define		MAX_CHIP_NUM	8

typedef enum 
{	
	BOARD_VIDEO_STD_D1_PAL = 0,
	BOARD_VIDEO_STD_D1_NTSC,
	BOARD_VIDEO_STD_CIF_PAL,
	BOARD_VIDEO_STD_CIF_NTSC,
	BOARD_VIDEO_STD_2CIF_PAL,
	BOARD_VIDEO_STD_2CIF_NTSC,
	BOARD_VIDEO_STD_4CIF_PAL,
	BOARD_VIDEO_STD_4CIF_NTSC,
	BOARD_VIDEO_STD_QCIF_PAL,
	BOARD_VIDEO_STD_QCIF_NTSC,
	MAX_BOARD_SUPPORT_VIDEO_STANDARD
} BOARD_SUPPORT_VIDEO_STANDARD;


class CDvrFCDDlg;
// CDVRSystem

class CDVRSystem : public CWnd
{
	DECLARE_DYNAMIC(CDVRSystem)

public:
	CDVRSystem(CDvrFCDDlg* pMainDlg, int iBoardIndex);
	virtual ~CDVRSystem();

	CDvrFCDDlg					*m_pMainDlg;
	int							m_iBoardIndex;
	int							m_iVdoCount;
	int							m_iAdoCount;
		
	int							m_iBoardSetWidth;
	int							m_iBoardSetHeight;
	bool						m_bIsPAL ; 

	sdvr_video_std_e			m_VideoStandard;	
	int							m_iStatFPSInterval;
	int							m_iTimeOut;				//PCI driver timeout, seconds.
	bool						m_bDebugEnable;
	bool						m_bDebugAVFrames;
	bool						m_bDebugSDK;
	bool						m_bH264SCE;			// ????
	sdvr_sdk_params_t			m_sdk_params;
	bool						m_bBoardConnected;
	bool						m_bSupportVideoStds[MAX_BOARD_SUPPORT_VIDEO_STANDARD];		// now max support 10 sdvr_video_std_e define.

	sdvr_firmware_ver_t			m_fwVersion;	
	sdvr_board_attrib_t			m_BoardAttrib;
	sdvr_pci_attrib_t			m_PCIAttrib;
	sdvr_board_config_t			m_BoardConfig;
	sx_uint32					m_supported_video_stds;

	// Main function
	sdvr_err_e					UpgradeFW(char* pszFWpath = "");	// Load firmware from file and upload into board;
	sdvr_err_e					GetBoardParam();			// read bord and PCI information.
	sdvr_err_e					SetBoardParam();				// set video standard, etc.....	
	sdvr_err_e					OpenBoard();			// connect to board and get PCI information.
	sdvr_err_e					CloseBoard();			// disconnect board.

	//Encode Channel
	CChannelParam				*m_pEncodeChParam[MAX_BOARD_CHANNEL];
	sdvr_err_e					CreateEncodeChannel(int iCh);	// Set parameter and Start Encode to channel.
	sdvr_err_e					DeleteEncodeChannel(int iCh);	// Stop Encode to channel.
	
	sdvr_err_e					StartEncodeChannel(int iCh , int subEnc);	// Set parameter and Start Encode to channel.
	sdvr_err_e					StartEnableChannel(int iCh );
	sdvr_err_e					StopEncodeChannel(int iCh , int subEnc);	// Stop Encode to channel.
	sdvr_err_e					StopPreviewChannel(int iCh);
	BOOL						SetVideoEncodeParam(int iCh);	
	int							m_iTotalHWEncodeChannel[MAX_CHIP_NUM];

	//Connect Channel
	sdvr_err_e					ConnectChannel(int iCh);	// Set parameter and connect to channel.
	sdvr_err_e					DisConnectChannel(int iCh);	// Disconnect to channel.
	CChannelParam				*m_pChannelParam[MAX_BOARD_CHANNEL];
	bool						m_bEnableChannel[MAX_BOARD_CHANNEL];

	sdvr_video_res_decimation_e   m_video_res_decimation[MAX_BOARD_CHANNEL];
	sdvr_sdk_params_t &getSDKParams() {return m_sdk_params;}

	// Normal function	
	void						GetBoardSupportVideoStandard(sx_uint32 supported_video_stds);	// Reverse to well-known flag.
	bool						getDebugEnable() {return m_bDebugEnable; }
	void						setDebugEnable(bool bEnable) {m_bDebugEnable = bEnable; }	
	bool						getDebugAVFrames() {return m_bDebugAVFrames; }
	void						setDebugAVFrames(bool bAVFrames) {m_bDebugAVFrames = bAVFrames; }
	bool						getDebugSDK() {return m_bDebugSDK;}
	void						setDebugSDK(bool bDebugSDK) {m_bDebugSDK = bDebugSDK; }	
	void						updateSDKDebugSettings(char *debug_file_name = NULL);		// only for debug message and set PCI timeout.	
	bool						CheckVideoFormatSize();	// use m_bEnableChannel flag and board support format to set format size.

	//OSD Function
	BOOL						m_bEnableOSD[MAX_BOARD_CHANNEL][SDVR_MAX_OSD];
	char						m_szOSDText[MAX_BOARD_CHANNEL][SDVR_MAX_OSD][SDVR_MAX_OSD_EX_TEXT];	
	sdvr_osd_config_ex_t		m_OSD_Config_ex[MAX_BOARD_CHANNEL][SDVR_MAX_OSD];
	sdvr_font_table_t			m_font_table;

	char						m_szFontTable[_MAX_FNAME];
	sdvr_err_e					SetOSD(int iCh);
	sdvr_err_e					SetOSDFontTable();
	bool						GetOSDFontString();
	sdvr_err_e					ResetFrameRate(int iCh , int iFrame);
	int							m_iFrameRate[MAX_BOARD_CHANNEL];

protected:
	DECLARE_MESSAGE_MAP()
};


