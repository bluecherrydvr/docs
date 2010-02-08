#pragma once

#define CMD_HEADER_SIZE				12

#define NET_REQ_TOKEN_UNKNOWN   	0x00000000
#define NET_REQ_TOKEN_CONTROL   	0x00000001
#define NET_REQ_TOKEN_AUDIO     	0x00000002
//////////////////////////////////////////////
#define NET_REQ_BANDWIDTH			0x00000000                 
#define NET_REQ_REGISTER			0x00000001                 
#define NET_REQ_MAGICNOCHECK		0x00000002                 
#define NET_REQ_LIVECHECK			0x00000003                 
#define NET_REQ_BRIGHTNESS			0x00000004                 
#define NET_REQ_CONTRAST			0x00000005                 
#define NET_REQ_SATURATION			0x00000006                 
#define NET_REQ_HUE			    	0x00000007                 
#define NET_REQ_FPS					0x00000008                 
#define NET_REQ_BITRATE				0x00000009                 
#define NET_REQ_RESOLUTION			0x0000000a                 
#define NET_REQ_RS232				0x0000000b                 
#define NET_REQ_DI					0x0000000c
#define NET_REQ_DO					0x0000000d
#define NET_REQ_MD_RANGESET			0x0000000e
#define NET_REQ_MD_SENSET			0x0000000f
#define NET_REQ_RS232_SET			0x00000011                 
#define NET_REQ_UNREGISTER			0x00000012
#define NET_REQ_DIOSTATUS			0x00000014                 
#define NET_REQ_MOTIONINFO			0x00000015
#define NET_REQ_ONLINEUSER			0x00000016
#define NET_REQ_SERVERVERSION		0x00000017
#define NET_REQ_SCFG				0x00000020
#define NET_REQ_VIDEOSTART			0x00000021
#define NET_REQ_VIDEORUNNING		0x00000022
#define NET_REQ_VIDEOPAUSE			0x00000023

#define NET_REQ_CONTROL_LIVE		0x00000030
#define NET_REQ_CONTROL_EXIT		0x00000031
#define NET_REQ_CONTROL_DO			0x00000032
#define NET_REQ_CONTROL_DIOSTATUS	0x00000033
#define NET_REQ_CONTROL_SERIAL  	0x00000036
#define NET_REQ_AUDIO_PLAY  	    0x00000037
#define NET_REQ_CONTROL_VLOSS  	    0x00000038
#define NET_REQ_CONTROL_MD  	    0x00000039
#define NET_REQ_CONTROL_NAME        0x00000040

#define  NET_REQ_VARIABLE_FPS	0xA0000051
#define  S_ENGINE_REQ_PLAY		0x11111111
#define  PLY_REQ_SET_POSITION   0xBAC00001
#define  PLY_REQ_SET_TIMECODE   0xBAC00002

#define  NET_REQ_URL_COMMAND		0xA0000001
#define	 NET_CONTROL_ONLY_NETWORK_LOSS	0xA0010001

#define NET_RSP_BANDWIDTH		0x00010000
#define NET_RSP_REGISTER		0x00010001
#define NET_RSP_MAGICNOCHECK	0x00010002
#define NET_RSP_LIVECHECK		0x00010003
#define NET_RSP_BRIGHTNESS		0x00010004
#define NET_RSP_RESOLUTION		0x0001000a
#define NET_RSP_DI				0x0001000c
#define NET_RSP_MD				0x0001000e
#define NET_RSP_RS232_RECV		0x00010010
#define NET_RSP_VIDEOLOST		0x00010013
#define NET_RSP_DIOSTATUS		0x00010014                 
#define NET_RSP_MOTIONINFO		0x00010015
#define NET_RSP_ONLINEUSER		0x00010016
#define NET_RSP_SERVERVERSION	0x00010017
#define NET_REQ_SAVE_REBOOT		0x00000018 //*

#define NET_RSP_SCFG			0x00010020
#define NET_RSP_VIDEOSTART		0x00010021
#define NET_RSP_VIDEORUNNING    0x00010022

#define NET_RSP_CONTROL_DI			0x00010034
#define NET_RSP_CONTROL_DIOSTATUS	0x00010033
#define NET_RSP_CONTROL_SERIAL  	0x00010035
#define NET_RSP_AUDIO_PLAY  	    0x00010037
#define NET_RSP_CONTROL_VLOSS  	    0x00010038
#define NET_RSP_CONTROL_MD  	    0x00010039
#define NET_RSP_CONTROL_NAME        0x00010040

// TCP 2.0 Register Return Code
#define NET_RSP_TOKEN_SUCCESS       0x00000000
#define NET_RSP_ACCOUNT_ERROR		0x00000001
#define NET_RSP_TOKEN_UNKNOWN		0x00000002
#define NET_RSP_TOKEN_CONTROL_BUSY  0x00000003
#define NET_RSP_TOKEN_AUDIO_BUSY	0x00000004

////////////////////////////////////////
//----> ipcmrSetStatusCallBack Status///
#define NET_CB_STS_LiveDisconnect			1
#define NET_CB_STS_ResolutionChange		    2
#define NET_CB_STS_VideoLost				3
#define NET_CB_STS_VideoLostRecovery		4
#define NET_CB_STS_FpsModeChange		    5
#define NET_CB_STS_FpsChange				6
#define	NET_CB_STS_VideoSocketClosed        7
#define NET_CB_STS_AlmRecordFail    		8
#define	NET_CB_STS_RecordFail               9

//----> ipcmrSetServerConfig Mask Definition
//System Config Mask Define
#define NET_SCFG_BRI	0x00000001
#define NET_SCFG_CON	0x00000002
#define NET_SCFG_SAT	0x00000004
#define NET_SCFG_HUE 	0x00000008
#define NET_SCFG_FPS	0x00000010
#define NET_SCFG_BRT 	0x00000020
#define NET_SCFG_RES 	0x00000040


//----> For ipcmrSetRS232 x81 Setting
//RS232 Setting
#define NET_N81			0x00		// N,    8, 1
#define NET_O81			0x08		// Odd,  8, 1
#define NET_E81			0x18		// Even, 8, 1

//----> For ipcmrSetRS232 baudrate Setting
//RS232 BaudRate Setting
#define NET_1200BPS		0X00
#define NET_2400BPS		0X01
#define NET_4800BPS		0X02
#define NET_9600BPS		0X03
#define NET_19200BPS	0X04
#define NET_38400BPS	0X05
#define NET_57600BPS	0X06
#define NET_115200BPS	0X07
#define NET_230400BPS	0X08


#define NET_TVNTSC			0
#define NET_TVPAL			1
#define NET_COMPOSITE		0
#define NET_SVIDEO			1
#define NET_LANDISABLE		0
#define NET_LANUNICAST		1
#define NET_LANMULTICAST	2
#define NET_WANDISABLE		0
#define NET_WANUNICAST		1
