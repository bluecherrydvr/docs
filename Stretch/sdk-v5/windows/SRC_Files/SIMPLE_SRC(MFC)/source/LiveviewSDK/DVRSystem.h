#pragma once

#include "sdvr_sdk.h"
#include "ChannelParam.h"



class CDvrFCDDlg;
// CDVRSystem

class CDVRSystem : public CWnd
{
	DECLARE_DYNAMIC(CDVRSystem)

public:
	CDVRSystem(CDvrFCDDlg* pMainDlg, int iBoardIndex);
	virtual ~CDVRSystem();

	CDvrFCDDlg*			m_pMainDlg;	

	int				m_iBoardSetWidth;
	int				m_iBoardSetHeight;

	bool				m_bIsPAL;
	int				m_iBoardIndex;
	int				m_iVdoCount;
	int				m_iAdoCount;	

	sdvr_video_std_e m_VideoStandard;	
//	int				m_iStatFPSInterval;
	int				m_iTimeOut;				//PCI driver timeout, seconds.
	bool				m_bDebugEnable;
	bool				m_bDebugAVFrames;
	bool				m_bDebugSDK;
	bool				m_bH264SCE;			// ????
	sdvr_sdk_params_t m_sdk_params;
	bool            m_bBoardConnected;
	bool				m_bSupportVideoStds[MAX_BOARD_SUPPORT_VIDEO_STANDARD];		// now max support 10 sdvr_video_std_e define.

	sdvr_firmware_ver_t	m_fwVersion;	
	sdvr_board_attrib_t	m_BoardAttrib;
	sdvr_pci_attrib_t		m_PCIAttrib;
	sdvr_board_config_t	m_BoardConfig;	

	// Main function
	sdvr_err_e PreGetPCIAttribute(sdvr_pci_attrib_t* pPCI_Attribute);
	sdvr_err_e UpgradeFW(char* pszFWpath = "");	// Load firmware from file and upload into board;
	sdvr_err_e GetBoardParam();			// read bord and PCI information.
	sdvr_err_e SetBoardParam();				// set video standard, etc.....	
	sdvr_err_e OpenBoard();			// connect to board and get PCI information.
	sdvr_err_e CloseBoard();			// disconnect board.


	// Channel
	CChannelParam* m_pChannelParam[MAX_BOARD_CHANNEL];
	sdvr_err_e ConnectChannel(int iCh);	// Set parameter and connect to channel.
	sdvr_err_e DisConnectChannel(int iCh);	// Disconnect to channel.
	BOOL     SetVideoEncodeParam(int iCh);	
	bool      m_bEnableChannel[MAX_BOARD_CHANNEL];
	sdvr_video_res_decimation_e   m_video_res_decimation[MAX_BOARD_CHANNEL];	
	int		  m_iFrameRate[MAX_BOARD_CHANNEL];	
	BOOL     m_bMute[MAX_BOARD_CHANNEL];
	BOOL	  m_bEnableSensors[MAX_RELAYS_SENSORS];
	BOOL	  m_bEnableRelays[MAX_RELAYS_SENSORS];
	int		  m_iTriggerMode[MAX_RELAYS_SENSORS];


	// Normal function	
	void      GetBoardSupportVideoStandard(sx_uint32 supported_video_stds);	// Reverse to well-known flag.

	bool getDebugEnable() {return m_bDebugEnable; }
	void setDebugEnable(bool bEnable) {m_bDebugEnable = bEnable; }	

	bool getDebugAVFrames() {return m_bDebugAVFrames; }
	void setDebugAVFrames(bool bAVFrames) {m_bDebugAVFrames = bAVFrames; }

	bool getDebugSDK() {return m_bDebugSDK;}
	void setDebugSDK(bool bDebugSDK) {m_bDebugSDK = bDebugSDK; }	

	void updateSDKDebugSettings(char *debug_file_name = NULL);		// only for debug message and set PCI timeout.
	
	bool	 CheckVideoFormatSize(int iCh);	// use m_bEnableChannel flag and board support format to set format size.	


	// OSD setting, all sample are available only in firmware version 3.2.0.0 or later. 
	// These parameters can re-get by call sdvr_get_osd_text();
	sdvr_err_e				SetOSD(int iCh);
	BOOL					m_bEnableOSD[MAX_BOARD_CHANNEL][SDVR_MAX_OSD];
	char                    m_szOSDText[MAX_BOARD_CHANNEL][SDVR_MAX_OSD][SDVR_MAX_OSD_EX_TEXT];	
	sdvr_osd_config_ex_t     m_OSD_Config_ex[MAX_BOARD_CHANNEL][SDVR_MAX_OSD];
	CString					m_szOSD_Text[MAX_BOARD_CHANNEL][SDVR_MAX_OSD];
	sdvr_font_table_t			m_font_table;
	CString					m_strFontTable;
	sdvr_err_e				SetFontTable();
	sdvr_err_e				ResetFrameRate(int iCh);
	

protected:
	DECLARE_MESSAGE_MAP()
};


