
#ifndef _ServerUtl_H
#define _ServerUtl_H
/*
#ifdef _USRDLL
#define DLLAPI __declspec(dllexport) __stdcall 
#else
#define DLLAPI  __stdcall
#endif*/

#include "typedef.h"
#include "errordef.h"
#include "app_error.h"
#include "typedef.h"
#include "common.h"

#ifndef MAX_PATH
#define	MAX_PATH		260
#endif // MAX_PATH

#define SERVERUTL_VERSION       MAKEFOURCC(6,2,0,27)
//--------------------------------------------------------------------------------
// const ANT and DEFINIED
//--------------------------------------------------------------------------------
#define SERVERUTL_BASE				100

#define	MAX_HOST_NAME_LEN			14
#define	MAX_DATE_LEN				11
#define	MAX_TIME_LEN				8
#define	MAX_PASS_LEN				14
#define	MAX_VERSION_LEN				32
#define	MAX_MODEL_LEN				32
#define	MAX_LANGUAGE_LEN			32
#define	MAX_WEB_VERSION				32

// PTZ command
#define SERVERUTL_PTZ_COMMANDS  SERVERUTL_BASE+100

typedef enum {
 SERVERUTL_PTZ_MOVE_UP=SERVERUTL_PTZ_COMMANDS+1,
 SERVERUTL_PTZ_MOVE_DOWN,
 SERVERUTL_PTZ_MOVE_LEFT,
 SERVERUTL_PTZ_MOVE_RIGHT,
 SERVERUTL_PTZ_MOVE_HOME,
 SERVERUTL_PTZ_ZOOM_TELE,
 SERVERUTL_PTZ_ZOOM_WIDE,
 SERVERUTL_PTZ_FOCUS_NEAR,
 SERVERUTL_PTZ_FOCUS_FAR,
 SERVERUTL_PTZ_FOCUS_AUTO,
 SERVERUTL_PTZ_PRESET_ADD,
 SERVERUTL_PTZ_PRESET_DEL,
 SERVERUTL_PTZ_RECALL,
 SERVERUTL_PTZ_PAN_SPEED,
 SERVERUTL_PTZ_TILT_SPEED,
 SERVERUTL_AUTO_PAN,
 SERVERUTL_AUTO_STOP,
 SERVERUTL_AUTO_PATROL,
 SERVERUTL_IRIS_OPEN,
 SERVERUTL_IRIS_CLOSE,
 SERVERUTL_IRIS_AUTO,
 SERVERUTL_PTZ_ZOOM_SPEED,
 SERVERUTL_PTZ_SET_HOME,
 SERVERUTL_PTZ_DEFAULT_HOME,
 SERVERUTL_FOCUS_SPEED,
} PTZCOMMANDS;

// Status callback code for non-blocking mode
#define	SERVERUTL_STATUS_SET_PROPERTY				100
#define	SERVERUTL_STATUS_UPDATE_LOCALCFG			101
#define	SERVERUTL_STATUS_UPDATE_REMOTECFG			102
#define	SERVERUTL_STATUS_GET_DI						103
#define	SERVERUTL_STATUS_SET_DO						104
#define	SERVERUTL_STATUS_READ_UART					105
#define	SERVERUTL_STATUS_WRITE_UART					106
#define	SERVERUTL_STATUS_PTZ_CMD					107
#define	SERVERUTL_STATUS_MOVE_CAMERA				108
#define	SERVERUTL_STATUS_HTTP_OPER					109
#define	SERVERUTL_STATUS_SET_PARAM					110
#define	SERVERUTL_STATUS_UPGRADE					111
#define	SERVERUTL_STATUS_READ_AF_WRITE_UART					112

// step code for upgrade
#define	SERVERUTL_UPG_UNPACK_FILE					1
#define	SERVERUTL_UPG_FTP_CONNECT					2
#define	SERVERUTL_UPG_RM_REMOTE_FILE				3
#define	SERVERUTL_UPG_CREATE_DIR					4
#define	SERVERUTL_UPG_CHANGE_DIR					5
#define	SERVERUTL_UPG_PUTTING_FILE					6
#define	SERVERUTL_UPG_PUT_FILE						7
#define	SERVERUTL_UPG_RESTART_SRV					8
#define	SERVERUTL_UPG_WAIT_REBOOT					9
#define	SERVERUTL_UPG_SERVER_REBOOTING				10
#define	SERVERUTL_UPG_SERVER_REBOOTED				11
#define	SERVERUTL_UPG_REMOVE_TMP_FILE				12
#define	SERVERUTL_UPG_TOTAL_SIZE					13
#define	SERVERUTL_UPG_FILE_SIZE						14
#define	SERVERUTL_UPG_CURR_UPG_SIZE					15
#define SERVERUTL_UPG_SERVER_RESET					16

// error code for status notify in non-blocking modee
#define	SERVERUTL_ERR_SUCCESS						0
#define	SERVERUTL_ERR_CONNECT_FAILED				200
#define	SERVERUTL_ERR_AUTH_FAILED					201
#define	SERVERUTL_ERR_READ_DATA_ERROR				202
#define	SERVERUTL_ERR_FTP_GETFILE					203
#define	SERVERUTL_ERR_FTP_PUTFILE					204
#define	SERVERUTL_ERR_OPEN_FILE						205
#define	SERVERUTL_ERR_ABORTING						206

// DIDO STATUS
#define SERVERUTL_DIDO_STATUS  SERVERUTL_BASE+200

typedef enum 
{
	DIDO_NONE = 0,
	DIDO_HIGH = SERVERUTL_DIDO_STATUS,
	DIDO_LOW
} DIDOSTATUS;

typedef enum 
{
	firmpackBIN,
	firmpackWEB,
	firmpackPKG,
} EFIRMWARE_PACKAGE_TYPE;



typedef enum
{  
	SECTIONINDEX1 = 0x10000,
    SECTIONINDEX2 =	0x20000,
	SECTIONINDEX3 =	0x30000,
    SECTIONINDEX4 =	0x40000,

	// [system] section
	eSystemResetSystem = 0,	// for 6000 servers, this is write only and is <reset>
	eSystemHostName = 1,
	eSystemSerialNumber = 2,
	eSystemCurrentDate = 3,
	eSystemCurrentTime = 4,
	eSystemTimeZone = 5,
	eSystemFirmwareVersion = 6,	// <software version> in 2000/3000
	eSystemLanguage = 7,
	eSystemPTZEnable1 = 8,		// ptz enable in 3000/6000, ptzenabled1 in 2000
		// only for 4 channels servers
	eSystemPTZEnable2 = 9,
		// Only for 6000
	eSystemSupportScriptVersion = 10,
	eSystemScriptVersion = 11,
	eSystemLedOff = 12,
		// for 3000 single board, 2000
	eSystemDemoEnabled = 13,	// for 3000 single board, this is in [System]/<demo enabled>. 
								// for 2000 it's in [DEMO]/<demo enalbed> 
		// for 2000/300
	eSystemUserName = 14,		// 6000 in security, 2000/3000 in system
	eSystemUserPwssword = 15,	// 6000 in security, 2000/3000 in system
		// 6000/3000
	eSystemEnableIR = 16,	// for 6000 in [System]/<enableir>, in PT 3000, it's in [Camera Control]/<I.R. Control>
	eSystemNtpServer = 17,	// system in 6000, network in 2000/3000
	eSystemNtpUpdateInterval = 18,	// for 6000/3000, for 2000 this is in [System Addon]
    eSystemWebpageVersion = 19,
    eSystemViewMode = 20,
	eSystemModelname = 21,
	eSystemLast	= eSystemModelname,


	// [network] section
	eNetworkResetIP = 41,				// install enabled for 2000/300
	eNetworkIPAddress = 42,
	eNetworkSubnetMask = 43,
	eNetworkRouter = 44,
	eNetworkDns1 = 45,
	eNetworkDns2 = 46,
	eNetworkSmtpServer1 = 47,
	eNetworkMailTo1 = 48,
	eNetworkMailUser1 = 49,
	eNetworkMailPass1 = 50,
	eNetworkSmtpServer2 = 51,
	eNetworkMailTo2 = 52,
	eNetworkMailUser2 = 53,
	eNetworkMailPass2 = 54,
	eNetworkMailReturnAddr = 55,
	eNetworkLocalFtpPort = 56,
	eNetworkFtpServer1 = 57,
	eNetworkFtpPort1 = 58,
	eNetworkFtpUser1 = 59,
	eNetworkFtpPass1 = 60,
	eNetworkFtpFolder1 = 61,
	eNetworkFtpPassive1 = 62,
	eNetworkFtpServer2 = 63,
	eNetworkFtpPort2 = 64,
	eNetworkFtpUser2 = 65,
	eNetworkFtpPass2 = 66,
	eNetworkFtpFolder2 = 67,
	eNetworkFtpPassive2 = 68,
	eNetworkHttpPort = 69,
		// only for 2000
	eNetworkBandwidthLimit = 70,
		// only for 3000
	eNetworkControlPort = 71,
		// only for 2000/3000
	eNetworkNtpEnabled = 72,
	eNetworkPPPEnabled = 73,
	eNetworkEthernetAddress = 74,
	eNetworkNtpServer = 75,	// system in 6000, network in 2000/3000
		// for 3000
	eNetworkLowBandAudioImprove = 76,	// 3000 in network, 6000 or PT in audio
		// only for 3000/6000
	eNetworkAudioPort = 77,
	eNetworkVideoPort = 78,
    eNetworkType = 79,
    eNetworkPPPoEUser = 80,
    eNetworkPPPoEPass = 81,
    eNetworkRTSPPort = 82,
    eNetworkAccessName = 83,
    eNetworkPublicIP = 84,  //Bruce add for VS2403  date:20050824 ,
    eNetworkPppoeStatus =85,  
    eNetworkLast = eNetworkPppoeStatus, //Bruce end here date:20050824 


	// [video] section
	eVideoCaptionText = 101,
	eVideoColoredVideo = 102,
	eVideoVideoQuality = 103,
	eVideoRateControl = 104,		// quality for 6000
	eVideoWhiteBlance = 105,		// only available for 6000 CMOS server, but will not check in this module
		// only for 2000
	eVideoDefaultSize = 106,
	eVideoOverlayTimeStamp = 107,
		// only for 4 channels 2000
	eVideoDefaultVideo = 108,
	eVideoIsOpen = 109,
	eVideoMapToCOM = 110,
	eVideoMapToID = 111,
		// for non-6000
	eVideoCameraModulation = 112,
	eVideoBrightness = 113,		// image in 6000, video in others
	eVideoContrast = 114,		// image in 6000, video in others
	eVideoHue = 115,				// image in 6000, video in others
	eVideoSaturation = 116,		// image in 6000, video in others
		// only for 3000
	eVideoMotionEnabled = 117,	// motion in 6000, video in 3000
		// only for 3000/6000
	eVideoBitRate = 118,
	eVideoMaxFrameRate = 119,
	eVideoFlip = 120,
	eVideoMirror = 121,
	eVideoVideoSize = 122,
		// only for 6000
	eVideoCodecType = 123,
	eVideoKeyInterval = 124,
	eVideoUDPSlowStart = 125,
	eVideoActualModulation = 126,
	eVideoMode = 127,
    eVideoDefaultSource	= 128,  //Bruce add for VS2403 video  date:20050824 
    eVideoEnable = 129, 
    eVideoImprintTimestamp = 130, 
    eVideoImageOverlay = 131 ,
	eVideoResolution = 132,
    eVideoLast = eVideoResolution,  //Bruce end here date:20050824 


	// [wireless] section
	eWirelessSSID = 151,
	eWirelessMode = 152,
	eWirelessChannel = 153,
	eWirelessTxRate = 154,
	eWirelessPreamble = 155,
	eWirelessEncrypt = 156,
	eWirelessAuthMode = 157,
	eWirelessKeyLength = 158,
	eWirelessKeyFormat = 159,
	eWirelessKeySelect = 160,
	eWirelessKey1 = 161,
	eWirelessKey2 = 162,
	eWirelessKey3 = 163,
	eWirelessKey4 = 164,
	eWirelessDomain = 165,
    eWirelessPreShareKey = 166,
    eWirelessAlgorithm = 167,
	eWirelessLast = eWirelessAlgorithm,

	// [ddns] & [upnp] section
	eDDNSEnabled = 191,
	eDDNSProvider = 192,
	eDDNSHostName = 193,
	eDDNSUserNameEmail = 194,
	eDDNSPasswordKey = 195,
	eUPNPEnabled = 196,
	eDDNSCustomEnable = 197,
	eDDNSServerName = 198,
	eUPNPPortForwarding = 199,
	eUPNPLast = eUPNPPortForwarding,

	// [ddnsvvtk] section for only 6k of VVTK
	eDDNSVVTKEmail = 220,
	eDDNSVVTKKey = 221,
	eDDNSVVTKHostName = 222,
	eDDNSVVTKServerName = 223,
	eDDNSVVTKDomainName = 224,
	eDDNSVVTKCustomServer = 225,
	eDDNSVVTKRegisterStatus = 226,
	eDDNSVVTKRequestKeyStatus = 227,
	eDDNSVVTKTempHostNameOnly = 228,
	eDDNSVVTKTempServerName	= 229,
	eDDNSVVTKTempDomainName	= 230,
	eDDNSVVTKTempCustomServer = 231,
	eDDNSVVTKLast = eDDNSVVTKTempCustomServer,

	// [feature] section
	eFeatureEvent = 241,
	eFeatureWPA2 = 242,
	eFeatureLast = eFeatureWPA2,
	
	// [camera control] section	only for PT/VS model in 3000/6000
	eCameraCtrlPanSpeed = 271,
	eCameraCtrlTiltSpeed = 272,
	eCameraCtrlPanPatrolSpeed = 273,
	eCameraCtrlDwellingTime = 274,
		// only for PT 3000
	eCameraCtrlPresets = 275,
	eCameraCtrlEnableIR = 276,	// for 6000 in [System]/<enableir>, in PT 3000, it's in [Camera Control]/<I.R. Control>
		// only for 6000
	eCameraCtrlAxisX = 277,
	eCameraCtrlAxisY = 278,
	eCameraCtrlLast = eCameraCtrlAxisY,

	// [Layout] section,	pnly for 2000/3000
	eLayoutFontColor = 311,
	eLayoutBackgroundColor = 312,
	eLayoutLogoType = 313,
	eLayoutBackgroundType = 314,
	eLayoutLogoSource = 315,
	eLayoutBackgroundSource = 316,
	eLayoutLogoLink = 317,
	eLayoutCOM1SpeedLinkName = 318,
		// only for 2000
	eLayoutLayoutType = 319,
		// only for 2000 4 channel
	eLayoutCOM2SpeedLinkName = 320,
	eLayoutButtonName = 321,
	eLayoutLinkColor = 322, //Bruce add here for VS2403 Layout 20050824
    eLayoutVideoLinkName = 323, 
    eLayoutLast = eLayoutVideoLinkName, 


	// [Alert] section, only for 2000/3000
	eAlertAppMode = 351,
	eAlertUploadMethod = 352,
	eAlertFileWithTimeSuffix = 353,
	eAlertTimeToSnapthotAfterEvent = 354,	// tehth second for 2000, second for 3000
	eAlertSecDelayBetweenEvent = 355,
	eAlertSecPeriodicSnapshot = 356,
	eAlertTimeToStartSnapshot = 357,
	eAlertTimeToStopSnapshot = 358,
		// only for 2000
	eAlertVisualAlert = 359,
	eAlertPercentOfObjSize = 360,
	eAlertPercentSensitivity = 361,
	eAlertScritpEnabled = 362,	// 2000 in alert, 6000 in App
	eAlertMDChannelEnabled = 363,
	eAlertSnapshotChannelEnabled = 364,
	eAlertSeriesSnapshotChannelEnabled = 365,
	eAlertLast = eAlertSeriesSnapshotChannelEnabled,

	// [DEMO] section, only for 2000 and single board model in 3000
	eDemoPtzEnabled = 391,
	eDemoDoCtrlEnabled = 392,
	eDemoDemoEnabled = 393,	// for 3000 single board, this is in [System]/<demo enabled>. for 2000 it's in [DEMO]/<demo enalbed> 
		// only for 4 channels servers
	eDemoVideoOpen = 394,
	eDemoLast = eDemoVideoOpen,

	// [Audio] section
		// only for PT 3000
	eAudioExternalMic = 421,
		// only for 3000
	eAudioMute = 422,
		// 6000 or PT 3000
	eAudioLowBandAudioImprove = 423,	// 3000 in network, 6000 or PT in audio
		// only for 6000
	eAudioEnableAEC = 424,
	eAudioSendClientAudio = 425,
	eAudioBitRate = 426,
	eAudioTransferMode = 427,
	eAudioSource = 428,
	/* ------only for 7k-------*/
	eAudioCodec = 429,
	eAudioAACBitRate = 430,
	eAudioAMRBitRate = 431,
	/* ------only for 7k-------*/
	eAudioLast = eAudioAMRBitRate,

	// [security] section
		// only for 6000
	eSecurityUserCount = 451,
	eSecurityUserAttribute = 452,
	eSecurityUserName = 453,	// security in 6000, system in 2000 or 3000
	eSecurityUserPwssword = 454,	// security in 6000, system in 2000 or 3000
	eSecurityLast = eSecurityUserPwssword,

	// [Serial] section
		// only for 2000/3000
	eSerialAutoDetectCamera = 481,
	eSerialDataBits1 = 482,
	eSerialStopBits1 = 483,
	eSerialParityBits1 = 484,
	eSerialBaudRate1 = 485,
	eSerialCCDModel1 = 486,
	eSerialUartMode1 = 487,
	eSerialSpeedLinkCmd1 = 488,	// other control command for IP31x1
	eSerialCustomCCDCmd1 = 489,	// PT control commands for IP31x1
	eSerialDataBits2 = 490,
	eSerialStopBits2 = 491,
	eSerialParityBits2 = 492,
	eSerialBaudRate2 = 493,
	eSerialCCDModel2 = 494,
	eSerialUartMode2 = 495,
	eSerialSpeedLinkCmd2 = 496,
	eSerialCustomCCDCmd2 = 497,
	eSerialEnableCameraControl = 498,	// only for IP31x1
	eSerialPtzDriver = 499, //Bruce add for VS2403 20050824
	eSerialSpeedLinkName1 = 500,
	eSerialCustomCmd = 501,   //add for VS2403 20051209
    eSerialLast	= eSerialCustomCmd, //Bruce end here 20051209

		// [Serial2] section
		// only for 2000/3000
	eSerial2AutoDetectCamera = 481 + SECTIONINDEX2, //131553
	eSerial2DataBits1 = 482 + SECTIONINDEX2,
	eSerial2StopBits1 = 483 + SECTIONINDEX2,
	eSerial2ParityBits1 = 484 + SECTIONINDEX2,
	eSerial2BaudRate1 = 485 + SECTIONINDEX2,
	eSerial2CCDModel1 = 486 + SECTIONINDEX2,
	eSerial2UartMode1 = 487 + SECTIONINDEX2,
	eSerial2SpeedLinkCmd1 = 488 + SECTIONINDEX2,	// other control command for IP31x1
	eSerial2CustomCCDCmd1 = 489 + SECTIONINDEX2,	// PT control commands for IP31x1
	eSerial2DataBits2 = 490 + SECTIONINDEX2,
	eSerial2StopBits2 = 491 + SECTIONINDEX2,
	eSerial2ParityBits2 = 492 + SECTIONINDEX2,
	eSerial2BaudRate2 = 493 + SECTIONINDEX2,
	eSerial2CCDModel2 = 494 + SECTIONINDEX2,
	eSerial2UartMode2 = 495 + SECTIONINDEX2,
	eSerial2SpeedLinkCmd2 = 496 + SECTIONINDEX2,
	eSerial2CustomCCDCmd2 = 497 + SECTIONINDEX2,
	eSerial2EnableCameraControl = 498 + SECTIONINDEX2,	// only for IP31x1
	eSerial2PtzDriver = 499 + SECTIONINDEX2, //Bruce add for VS2408 20050824
	eSerial2SpeedLinkName1 = 500 + SECTIONINDEX2,
	eSerial2CustomCmd = 501 + SECTIONINDEX2,   //add for VS2403 20051209
    eSerial2Last	= eSerial2CustomCmd, //Bruce end here 20051209

	// [App] section
		// only for 6000
	eAppScriptName = 521,
	eAppScritpEnabled = 522,	// 2000 in alert, 6000 in App
	eAppLast = eAppScritpEnabled,

	// [image] section
		// only for 6000
	eImageBrightness = 551,		// image in 6000, video in others
	eImageContrast = 552,		// image in 6000, video in others
	eImageHue = 553,			// image in 6000, video in others
	eImageSaturation = 554,		// image in 6000, video in others
	eImageLast = eImageSaturation,

	// [motion] section
		// only for 6000
	eMotionMotionEnabled = 581,	// motion in 6000, video in 3000
	eMotionWinEnabled = 582,  //Bruce add for VS2403
    eMotionWinName = 583, 
    eMotionWinLeft = 584, 
    eMotionWinTop = 585, 
    eMotionWinWidth	= 586,
    eMotionWinHeight = 587, 
    eMotionWinBjSize = 588, 
    eMotionWinSensitivity = 589, 
    eMotionWinRenew	= 590, 
    eMotionLast	= eMotionWinRenew,  //Bruce end here
   
	//[motion1] section
	eMotion1MotionEnabled = 581 + SECTIONINDEX1,	 
	eMotion1WinEnabled = 582 + SECTIONINDEX1,   
    eMotion1WinName = 583 + SECTIONINDEX1, 
    eMotion1WinLeft = 584 + SECTIONINDEX1, 
    eMotion1WinTop = 585 + SECTIONINDEX1, 
    eMotion1WinWidth	= 586 + SECTIONINDEX1,
    eMotion1WinHeight = 587 + SECTIONINDEX1, 
    eMotion1WinBjSize = 588 + SECTIONINDEX1 , 
    eMotion1WinSensitivity = 589 + SECTIONINDEX1, 
    eMotion1WinRenew	= 590 + SECTIONINDEX1 , 
    eMotion1Last	= eMotionWinRenew + SECTIONINDEX1 ,   

		//[motion2] section
	eMotion2MotionEnabled = 581 + SECTIONINDEX2,	 
	eMotion2WinEnabled = 582 + SECTIONINDEX2,   
    eMotion2WinName = 583 + SECTIONINDEX2, 
    eMotion2WinLeft = 584 + SECTIONINDEX2, 
    eMotion2WinTop = 585 + SECTIONINDEX2, 
    eMotion2WinWidth	= 586 + SECTIONINDEX2,
    eMotion2WinHeight = 587 + SECTIONINDEX2, 
    eMotion2WinBjSize = 588 + SECTIONINDEX2 , 
    eMotion2WinSensitivity = 589 + SECTIONINDEX2, 
    eMotion2WinRenew	= 590 + SECTIONINDEX2 , 
    eMotion2Last	= eMotionWinRenew + SECTIONINDEX2 ,   

		//[motion3] section
	eMotion3MotionEnabled = 581 + SECTIONINDEX3,	 
	eMotion3WinEnabled = 582 + SECTIONINDEX3,   
    eMotion3WinName = 583 + SECTIONINDEX3, 
    eMotion3WinLeft = 584 + SECTIONINDEX3, 
    eMotion3WinTop = 585 + SECTIONINDEX3, 
    eMotion3WinWidth	= 586 + SECTIONINDEX3,
    eMotion3WinHeight = 587 + SECTIONINDEX3, 
    eMotion3WinBjSize = 588 + SECTIONINDEX3 , 
    eMotion3WinSensitivity = 589 + SECTIONINDEX3, 
    eMotion3WinRenew	= 590 + SECTIONINDEX3 , 
    eMotion3Last	= eMotionWinRenew + SECTIONINDEX3 ,   

		//[motion4] section
	eMotion4MotionEnabled = 581 + SECTIONINDEX4,	 
	eMotion4WinEnabled = 582 + SECTIONINDEX4,   
    eMotion4WinName = 583 + SECTIONINDEX4, 
    eMotion4WinLeft = 584 + SECTIONINDEX4, 
    eMotion4WinTop = 585 + SECTIONINDEX4, 
    eMotion4WinWidth	= 586 + SECTIONINDEX4,
    eMotion4WinHeight = 587 + SECTIONINDEX4, 
    eMotion4WinBjSize = 588 + SECTIONINDEX4 , 
    eMotion4WinSensitivity = 589 + SECTIONINDEX4, 
    eMotion4WinRenew	= 590 + SECTIONINDEX4 , 
    eMotion4Last	= eMotionWinRenew + SECTIONINDEX4 ,   

	// [System Addon]
	eSysAddonNtpUpdateInterval = 611,	// for 6000/3000 in [system], for 2000 this is in [System Addon]
	eSysAddonLast = eSysAddonNtpUpdateInterval,

	// [camctr] only in 6000
	eCamCtrlPresetName = 641,
	eCamCtrlPresetPan = 642,
	eCamCtrlPresetTilt = 643,
	eCamCtrlPatrolName = 644,
    eCamCtrlAutoSpeed =	645,  //Bruce add for VS2403 20050823
    eCamCtrlZoomSpeed =	646,		
    eCamCtrlFocusSpeed	= 647,
    eCamCtrlPanSpeed = 648,
    eCamCtrlTiltSpeed = 649,
    eCamCtrlAxisX =	650,
    eCamCtrlAxisY =	651,
    eCamCtrlDwelling = 652,
    eCamCtrlOsdZoom	= 653,
    eCamCtrlHomeX =	654,	
    eCamCtrlHomeY =	655,
    eCamCtrlDefaultHome	= 656,
    eCamCtrlDrivers	= 657,
    eCamCtrlSerial = 658,	
    eCamCtrlCamerAid = 659,
    eCamCtrlIsPtz = 660,
    eCamCtrlLast = 	eCamCtrlIsPtz,  //Bruce end for VS2403 20050823
  // [camctrl2] only in VS2403
	eCamCtrl2PresetName = 641 + SECTIONINDEX2,
	eCamCtrl2PresetPan = 642 + SECTIONINDEX2,
	eCamCtrl2PresetTilt = 643 + SECTIONINDEX2,
	eCamCtrl2PatrolName = 644 + SECTIONINDEX2,
    eCamCtrl2AutoSpeed = 645 + SECTIONINDEX2,
    eCamCtrl2ZoomSpeed = 646 + SECTIONINDEX2,		
    eCamCtrl2FocusSpeed	= 647 + SECTIONINDEX2,
    eCamCtrl2PanSpeed = 648 + SECTIONINDEX2,
    eCamCtrl2TiltSpeed = 649 + SECTIONINDEX2,
    eCamCtrl2AxisX = 650 + SECTIONINDEX2,
    eCamCtrl2AxisY = 651 + SECTIONINDEX2,
    eCamCtrl2Dwelling = 652 + SECTIONINDEX2,
    eCamCtrl2OsdZoom = 653 + SECTIONINDEX2,
    eCamCtrl2HomeX = 654 + SECTIONINDEX2,	
    eCamCtrl2HomeY = 655 + SECTIONINDEX2,
    eCamCtrl2DefaultHome = 656 + SECTIONINDEX2,
    eCamCtrl2Drivers = 657 + SECTIONINDEX2,
    eCamCtrl2Serial = 658 + SECTIONINDEX2,	
    eCamCtrl2CamerAid = 659 + SECTIONINDEX2,
    eCamCtrl2IsPtz = 660 + SECTIONINDEX2,
    eCamCtrl2Last = eCamCtrlIsPtz + SECTIONINDEX2,

	  // [camctrl3] only in VS2403
	eCamCtrl3PresetName = 641 + SECTIONINDEX3,
	eCamCtrl3PresetPan = 642 + SECTIONINDEX3,
	eCamCtrl3PresetTilt = 643 + SECTIONINDEX3,
	eCamCtrl3PatrolName = 644 + SECTIONINDEX3,
    eCamCtrl3AutoSpeed = 645 + SECTIONINDEX3,
    eCamCtrl3ZoomSpeed = 646 + SECTIONINDEX3,		
    eCamCtrl3FocusSpeed	= 647 + SECTIONINDEX3,
    eCamCtrl3PanSpeed = 648 + SECTIONINDEX3,
    eCamCtrl3TiltSpeed = 649 + SECTIONINDEX3,
    eCamCtrl3AxisX = 650 + SECTIONINDEX3,
    eCamCtrl3AxisY = 651 + SECTIONINDEX3,
    eCamCtrl3Dwelling = 652 + SECTIONINDEX3,
    eCamCtrl3OsdZoom = 653 + SECTIONINDEX3,
    eCamCtrl3HomeX = 654 + SECTIONINDEX3,	
    eCamCtrl3HomeY = 655 + SECTIONINDEX3,
    eCamCtrl3DefaultHome = 656 + SECTIONINDEX3,
    eCamCtrl3Drivers = 657 + SECTIONINDEX3,
    eCamCtrl3Serial = 658 + SECTIONINDEX3,	
    eCamCtrl3CamerAid = 659 + SECTIONINDEX3,
    eCamCtrl3IsPtz = 660 + SECTIONINDEX3,
    eCamCtrl3Last = eCamCtrlIsPtz + SECTIONINDEX3,

	  // [camctrl4] only in VS2403
	eCamCtrl4PresetName = 641 + SECTIONINDEX4,
	eCamCtrl4PresetPan = 642 + SECTIONINDEX4,
	eCamCtrl4PresetTilt = 643 + SECTIONINDEX4,
	eCamCtrl4PatrolName = 644 + SECTIONINDEX4,
    eCamCtrl4AutoSpeed = 645 + SECTIONINDEX4,
    eCamCtrl4ZoomSpeed = 646 + SECTIONINDEX4,		
    eCamCtrl4FocusSpeed	= 647 + SECTIONINDEX4,
    eCamCtrl4PanSpeed = 648 + SECTIONINDEX4,
    eCamCtrl4TiltSpeed = 649 + SECTIONINDEX4,
    eCamCtrl4AxisX = 650 + SECTIONINDEX4,
    eCamCtrl4AxisY = 651 + SECTIONINDEX4,
    eCamCtrl4Dwelling = 652 + SECTIONINDEX4,
    eCamCtrl4OsdZoom = 653 + SECTIONINDEX4,
    eCamCtrl4HomeX = 654 + SECTIONINDEX4,	
    eCamCtrl4HomeY = 655 + SECTIONINDEX4,
    eCamCtrl4DefaultHome = 656 + SECTIONINDEX4,
    eCamCtrl4Drivers = 657 + SECTIONINDEX4,
    eCamCtrl4Serial = 658 + SECTIONINDEX4,	
    eCamCtrl4CamerAid = 659 + SECTIONINDEX4,
    eCamCtrl4IsPtz = 660 + SECTIONINDEX4,
    eCamCtrl4Last = eCamCtrlIsPtz + SECTIONINDEX4,


	//[multicast]			//steven add 2006/03/30
	eMulticastIPAddress = 690,
	eMulticastVideoPort = 691,
	eMulticastAudioPort = 692,
	eMulticastTTL		= 693,
	eMulticastEnablem	= 694,
	eMulticastLast		= eMulticastEnablem,

	//[upnpfor]
	eUpnpForEnable	= 720,
	eUpnpForLast	= eUpnpForEnable,

	//[mobile] for psedo-dualstream
	eMobileAccessname = 750,
	eMobileResolution = 751,
	eMobileMaxframe = 752,
	eMobileKeyinterval = 753,
	eMobileBitrate = 754,
	eMobileAmrbitrate = 755,
	eMobileLast = eMobileAmrbitrate,

	//[ccd] for 6k series
	eCCDAutoIris = 780,
	eCCDIrisLevel = 781,
	eCCDAES = 782,
	eCCDWhiteBalanceMode = 782,
	eCCDAutoWhiteBalance = 783,
	eCCDWhiteBalanceCtrl = 784,
	eCCDLowLuxMode = 785,
	eCCDOBWLowLuxMode = 786,
	eCCDLast = eCCDOBWLowLuxMode,
} ESYSTEM_INFO_ID;

typedef struct
{
	LPCTSTR	lpszHost;
	LPCTSTR	lpszUserName;
	LPCTSTR	lpszPassword;
	WORD	wPort;
}TSERVERUTL_PROXY_INFO;

typedef enum
{
	eResetIPAfterUpgrade = 1,
    ePingTimeInUpgraging = 2,
	eProxyInfo			 = 3		//proxy information, dwParam1 is the pointer to TSERVERUTL_PROXY_INFO
} TServerUtl_ExtraOptions;

//--------------------------------------------------------------------------------
// DATA STRUCTURE
//--------------------------------------------------------------------------------
typedef void (__stdcall *SERVUTL_STATUS_CALLBACK)
(     
    HANDLE	hDevice,
	DWORD	dwContext,
	DWORD	dwStatus,
	DWORD	dwParam1,
	DWORD	dwParam2
);

typedef struct
{
	LPCTSTR	lpszHost;
	LPCTSTR	lpszUserName;
	LPCTSTR	lpszPassword;
	LPCTSTR lpszServerType;	// could be null or empty if you don't know
							// but for known server, the call will be finished faster
	// need to enable user to assign port for HTTP and FTP
	DWORD	dwHttpPort;	// 0 means to use default 80
	DWORD	dwFtpPort;	// 0 means to use default 21
						// the module will get ftp port from page in network page of server
						// by http protocol if ftp port is 0, so users could enhance 
						// the performance by assign this port number if you know 
						// exact the port
	DWORD	dwTimeout;	// timeout value in milliseconds
	SERVUTL_STATUS_CALLBACK pfStatusCallback;
	DWORD	dwContext;
	BOOL	bForceNotCachePwd;
} TSERVERUTL_DEV_PROPERTY, *PTSERVERUTL_DEV_PROPERTY;

typedef struct
{
	BOOL		bSetHostName;
	BOOL		bSetAdminPass;
	TCHAR		szHostName[MAX_HOST_NAME_LEN + 1];
	TCHAR		szDate[MAX_DATE_LEN + 1];
	TCHAR		szTime[MAX_TIME_LEN + 1];
	TCHAR		szAdminPass[MAX_PASS_LEN + 1];
} TSERVERUTL_SET_PARAM_INFO;

typedef struct
{
	TCHAR		szFirmVersion[MAX_VERSION_LEN + 1];
	TCHAR		szSupportScriptVersion[MAX_VERSION_LEN + 1];
	TCHAR		szWebVersion[MAX_WEB_VERSION + 1];
	TCHAR		szScriptVersion[MAX_VERSION_LEN + 1];
	TCHAR		szViewerPluginVersion[MAX_VERSION_LEN + 1];
	TCHAR		szUpgragePluginVersion[MAX_VERSION_LEN + 1];
	TCHAR		szSupportedModel[MAX_MODEL_LEN + 1];
	TCHAR		szLanguage[MAX_LANGUAGE_LEN + 1];
	BOOL		bOldModel;
	int			iFirmPackFileType;
} TSERVERUTL_PACK_VERSION_INFO;

typedef struct
{
	TCHAR						szFileName[MAX_PATH];
	SERVUTL_STATUS_CALLBACK		pfUpgradeCB;
	DWORD						dwContext;
} TSERVERUTL_UPG_INFO;

typedef struct
{
	long	lTotalSize;
	long	lCurSentSize;
	long	lCurFileSize;
	long	lCurFileSentSize;
	TCHAR	szCurFileName[MAX_PATH];
} TSERVERUTL_UPGSEND_INFO;

#define	CONST_MAX_WINDOWNAME		15

typedef struct 
{
	TCHAR	szName[CONST_MAX_WINDOWNAME];
	int		nX;
	int		nY;
	int		nW;
	int		nH;
	int		nPercent;
	int		nSensitivity;
	BOOL	bWindowEnabled;	// false means the window is not shown when using plugin
	BOOL	bFullSize;	// if this is set, nX, nY, nW, nH is all 0 and the size of the
						// motion detection window is the same as the video size
} TSERVERUTL_MODETECT_INFO;


// #####  API FUNCTION
#ifdef __cplusplus
extern "C" {
#endif
SCODE DLLAPI ServerUtl_Initial(HANDLE *phObject,DWORD dwVersion);
SCODE DLLAPI ServerUtl_InitialEx(HANDLE *phObject, DWORD dwMaxDevice, DWORD dwFlag, DWORD dwVersion);
SCODE DLLAPI ServerUtl_Release(HANDLE *phObject);
SCODE DLLAPI ServerUtl_CreateDevice(HANDLE hObject,HANDLE *phDevice);
SCODE DLLAPI ServerUtl_CreateDeviceEx(HANDLE hObject, HANDLE *phDevice, LPCTSTR pszProxy, LPCTSTR pszByPass);

SCODE DLLAPI ServerUtl_DeleteDevice(HANDLE hDevice);
SCODE DLLAPI ServerUtl_SetDeviceProperty(HANDLE hDevice, TSERVERUTL_DEV_PROPERTY tDevProperty, BOOL bNonBlock);
SCODE DLLAPI ServerUtl_SetDevicePropertyLite(HANDLE hDevice, TSERVERUTL_DEV_PROPERTY tDevProperty);
SCODE DLLAPI ServerUtl_GetDeviceModel(HANDLE hDevice, LPTSTR lpszModelName);
SCODE DLLAPI ServerUtl_OpenDevice(HANDLE hDevice, LPTSTR lpszModelName);    

SCODE DLLAPI ServerUtl_UpdateLocalConfig(HANDLE hDevice, BOOL bNonBlock);
SCODE DLLAPI ServerUtl_UpdateRemoteConfig(HANDLE hDevice, BOOL bNonBlock);
SCODE DLLAPI ServerUtl_GetSysInfo(HANDLE hDevice, LPTSTR lpszValue, LPTSTR lpszSection, LPTSTR lpszKey, int iIndex);//=0);
SCODE DLLAPI ServerUtl_SetSysInfo(HANDLE hDevice, LPTSTR lpszValue, LPTSTR lpszSection, LPTSTR lpszKey, int iIndex);//=0);

SCODE DLLAPI ServerUtl_GetSysInfoSecEntry(HANDLE hDevice, DWORD eID, LPTSTR lpszSection, LPTSTR lpszKey);
SCODE DLLAPI ServerUtl_GetSysInfoByID(HANDLE hDevice, LPTSTR lpszValue, DWORD eID, int iIndex);
SCODE DLLAPI ServerUtl_SetSysInfoByID(HANDLE hDevice, LPTSTR lpszValue, DWORD eID, int iIndex);

SCODE DLLAPI ServerUtl_SendPTZCommand(HANDLE hDevice, int iCam, PTZCOMMANDS ptzCommand, LPTSTR lpBuffer, BOOL bNonBlock);//=NULL); 
SCODE DLLAPI ServerUtl_SendPTZCommandEx(HANDLE hDevice, int iCam, PTZCOMMANDS ptzCommand, LPTSTR lpBuffer, int iIndex, BOOL bNonBlock);//=NULL); 
SCODE DLLAPI ServerUtl_MoveCamera(HANDLE hDevice, int iPort, DWORD dwX, DWORD dwY, BOOL bNonBlock);

SCODE DLLAPI ServerUtl_GetDIStatus(HANDLE hDevice, DIDOSTATUS *DI1, DIDOSTATUS *DI2, 
								   DIDOSTATUS *DI3, DIDOSTATUS *DI4, BOOL bNonBlock);
SCODE DLLAPI ServerUtl_SetDOLevel(HANDLE hDevice, DIDOSTATUS DO1, DIDOSTATUS DO2, BOOL bNonBlock);

SCODE DLLAPI ServerUtl_UartWrite(HANDLE hDevice, int iPort, LPTSTR lpszCommand, BOOL bNonBlock);
SCODE DLLAPI ServerUtl_UartRead(HANDLE hDevice, int iPort, LPTSTR lpszRespone, DWORD dwReadBytes, 
								BOOL bFlush, DWORD dwWaitTime, BOOL bNonBlock);
//bruce add UartReadAFWrite() 20061018
SCODE DLLAPI ServerUtl_UartReadAFWrite(HANDLE hDevice, int iPort, LPTSTR lpszCommand, LPTSTR lpszRespone, DWORD dwReadBytes, BOOL bFlush, DWORD dwWaitTime, BOOL bNonBlock);


SCODE DLLAPI ServerUtl_GetVersionInfo(BYTE *pbyMajor, BYTE *pbyMinor, BYTE *pbyBuild, BYTE *pbyRevision);

// new for 1.1.0.0 to do http get or post (no response data is retrieved)
SCODE DLLAPI ServerUtl_HttpOperation(HANDLE hDevice, LPCTSTR lpszUrlCommand, BOOL bPost, BOOL bNonBlock);

// new for 4.2.0.0 do http get or post and read the returned data
// pdwBuffLen is a in/out parameter
SCODE DLLAPI ServerUtl_HttpOperationRead(HANDLE hDevice, LPCTSTR lpszUrlCommand, BYTE *pbyReadBuff, 
										 DWORD *pdwBuffLen, BOOL bPost, BOOL bNonBlock);

// new for 4.2.0.0 this function could be called by another thread if the operation is blocking
// and doesn't return for a while, in async mode, this function has no effect
SCODE DLLAPI ServerUtl_AbortNetworkOper(HANDLE hDevice);

// new for 4.2.0.0 this function will not set server model, so only http operation is capable of calling
SCODE DLLAPI ServerUtl_SetDevicePropertyHttp(HANDLE hDevice, TSERVERUTL_DEV_PROPERTY tDevProperty);

SCODE DLLAPI ServerUtl_SetParam(HANDLE hDevice, TSERVERUTL_SET_PARAM_INFO *ptParamInfo, BOOL bNonBlock);
SCODE DLLAPI ServerUtl_GetFirmVersionInfo(LPCTSTR pszFileName, TSERVERUTL_PACK_VERSION_INFO *ptPackInfo);
SCODE DLLAPI ServerUtl_Upgrade(HANDLE hDevice, TSERVERUTL_UPG_INFO *ptUpgInfo);
SCODE DLLAPI ServerUtl_AbortUpgrade(HANDLE hDevice);

// this function is only available if the server model is known, that is, the SetDeviceProperty
// or SetDevicePropertyLite is called
SCODE DLLAPI ServerUtl_GetMotionDetectionInfo(HANDLE hDevice, TSERVERUTL_MODETECT_INFO *ptMDInfo, DWORD *pdwWinNum, BOOL bSyncSysInfo, BOOL bNonBlock);
SCODE DLLAPI ServerUtl_SetMotionDetectionInfo(HANDLE hDevice, TSERVERUTL_MODETECT_INFO *ptMDInfo, BOOL bSyncSysInfo, BOOL bNonBlock);
//new 
SCODE DLLAPI ServerUtl_SetExtraOptions(HANDLE hDevice,  DWORD dwOption, DWORD dwParam1, DWORD dwParam2);
SCODE DLLAPI ServerUtl_SetDOLevelEx(HANDLE hDevice, DIDOSTATUS* pDIDOSTATUS, int iDIDONumber, BOOL bNonBlock);


#ifdef __cplusplus
}
#endif
#endif // _ServerUtl_H
