// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#define MAX_SUPPORT_BOARD		4
#define MAX_BOARD_CHANNEL		16
#define MAX_RELAYS_SENSORS		8			// One Board.
#define MAX_SUPPORT_CHANNEL     64 

#define MAX_REGION_PER_LAYER		SDVR_REGION_MAP_D1_WIDTH*SDVR_REGION_MAP_LINE_PAL		// 720*576/(16*16)

#define SENSOR_EDGE_TRIGGER     0
#define SENSOR_LEVEL_SENSITIVE   1

#define  MAX_VDO_WIDTH 720
#define  MAX_VDO_HEIGHT 576
#define  MAX_VOD_BUFFER_SIZE MAX_VDO_WIDTH*MAX_VDO_HEIGHT*2

#define  WM_CREATE_BOARD_SETTING_DLG	WM_USER+700
#define  WM_CREATE_CHANNEL_SETTING_DLG	WM_USER+701
#define  WM_MODIFY_CHANNEL_SETTING_DLG WM_USER+702
#define  WM_CREATE_REGION_SETTING_DLG	WM_USER+703
#define  WM_MD_MODIFY_REGION			WM_USER+704
#define  WM_MD_ENABLE					WM_USER+705
#define  WM_CREATE_RS_DLG					WM_USER+706	// Create Relays and Sensor Dialog.
#define  WM_CONFIG_SENSORS				WM_USER+707
#define  WM_RELAY_TRIGGER					WM_USER+708
#define  WM_GET_PROTECT_ID				WM_USER+709


// PTZ
#define WM_PTZ_ACTION						WM_USER+710
#define WM_PTZ_ACTION_STOP				WM_USER+711		// The demo use Pelco-D PTZ, should send stop action to stop normal action.


typedef enum
{
	PTZ_INIT = 0,
	PTZ_UNINIT,
	PTZ_UP,
	PTZ_LEFT,
	PTZ_RIGHT,
	PTZ_DOWN,
	PTZ_FOCUS_FAR,
	PTZ_FOCUS_NEAR,
	PTZ_ZOOM_IN,
	PTZ_ZOOM_OUT,
	PTZ_IRIS_LARGE,
	PTZ_IRIS_SMALL,
	PTZ_ACTION_STOP
}PTZ_ACTION;


typedef struct _PELCO_CAMERA_INFO
{
	unsigned char btAddressOfCamera;
	unsigned char btPanDegree;
	unsigned char btTiltDegree;
	unsigned char PresetNum;
	unsigned char Group;
	unsigned char PresetWaitTime;
}PELCO_CAMERA_INFO,*PPELCO_CAMERA_INFO;


//========PELCO CAMERA Control Byte====
#define PELCO_FOCUS_NEAR      0x0001         
#define PELCO_FOCUS_FAR       0x8000		
#define PELCO_ZOOM_OUT        0x4000	
#define PELCO_ZOOM_IN         0x2000
#define PELCO_TILT_DOWN       0x1000
#define PELCO_TILT_UP         0x0800
#define PELCO_PAN_LEFT        0x0400
#define PELCO_PAN_RIGHT       0x0200
#define PELCO_AUTO_FOCUS      0x2B00
#define PELCO_SET_PRESET      0x0300
#define PELCO_PRESET_POS      0x0700
#define PELCO_CLEAR_PRESET    0x0500
#define PELCO_START_PRESET    0x0700
#define PELCO_STOP_PRESET     0x0700
#define PELCO_GROUP     			0x0700
#define PELCO_PRESET_PAUSE_TIME  0x0300    
#define PELCO_IRIS_LARGE      0x0002						
#define PELCO_IRIS_SMALL	  0x0004						
#define PELCO_STOP_ALL_ACTION 0x0000
//===============Auto Pan Pause Time=====================
#define PELCO_PAUSE_1S            0x46
#define PELCO_PAUSE_5S            0x47
#define PELCO_PAUSE_10S            0x48
#define PELCO_PAUSE_30S            0x49
//==Use Pan Speed that simulate Pan Degree===
#define PELCO_PAN_STOP             0x00
#define PELCO_PAN_MIN              0x08									
#define PELCO_PAN_15_DEGREE        0x10										
#define PELCO_PAN_22_DEGREE        0x18										
#define PELCO_PAN_45_DEGREE        0x20																
#define PELCO_PAN_90_DEGREE        0x28										
#define PELCO_PAN_180_DEGREE       0x30										
#define PELCO_PAN_360_DEGREE       0x38											
#define PELCO_PAN_MAX              0x3F

//=====================================
//==Use Tile Speed that simulate Tile Degree==
#define PELCO_TILT_STOP             0x00
#define PELCO_TILT_MIN              0x08										
#define PELCO_TILT_15_DEGREE        0x10										
#define PELCO_TILT_22_DEGREE        0x18										
#define PELCO_TILT_45_DEGREE        0x20																				
#define PELCO_TILT_90_DEGREE        0x28										
#define PELCO_TILT_180_DEGREE       0x30										
#define PELCO_TILT_360_DEGREE       0x38											
#define PELCO_TILT_MAX              0x3F
//=====================================
#define PELCO_AUTO_PAN_START       0X46		
#define PELCO_AUTO_PAN_STOP        0X60
#define PELCO_AUTO_SCAN            0x63
#define PELCO_GROUP1     			  0x47
#define PELCO_GROUP2     			  0x48
#define PELCO_GROUP3     			  0x49
#define PELCO_GROUP4     			  0x4A



typedef enum 
{
	CHANNEL_VDO_DATA_TYPE_RAW,
	CHANNEL_VDO_DATA_TYPE_ENC,
	CHANNEL_VDO_DATA_TYPE_DEC,
	MAX_CHANNEL_SUPPOR_VDO_DATA_TYPE
} CHANNEL_VDO_DATA_TYPE;

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


#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


