// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__13D827CA_8F1C_4287_8AD3_1B442360D23D__INCLUDED_)
#define AFX_STDAFX_H__13D827CA_8F1C_4287_8AD3_1B442360D23D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "Xdefine.h"
#include <afxmt.h>


// NT,PAL 인 경우에 각각 처음 스트림 사이즈 초기화 해 줘야 한다...
// 스크린 사이즈 피치가 디코딩 이미지 사이즈보다 작으면
// 디인터레이스에서 망가진다. 이후 디스플레이에서도 망가지는 것 같다...

// Resize 넣고, Zoom 넣기 (메인 컨트롤 윈도우의 서브로 구현)
// 전부 그런 방식으로 변경 (플레이에는 필요한 컨트롤만 넣기)
// 컨트롤 없애고 키보드로도 작동 가능하도록 하기 (데모용)...
// 이걸 베이스로 리모트뷰어도 만들자...

#define STR_APPLICATION_TITLE	"S6010 Stream Tester"

#define MAX_REAL_CHANNEL	16
#define MAX_VIRT_CHANNEL	16
#define MAX_ENC_CHANNEL	(MAX_REAL_CHANNEL +MAX_VIRT_CHANNEL)
#define MAX_DISP_WND	36
#define SZ_MAX_MOTION_BUF	256

#define WM_DOO_INDEX_BUTTON_CLICK	(WM_USER +30)

#define WM_DOO_DISP_WND_L_CLICK	(WM_USER +40)
#define WM_DOO_DISP_WND_L_DBLCLK	(WM_USER +41)
#define WM_DOO_DISP_WND_MOUSE_DRAG	(WM_USER +42)
#define WM_DOO_CTRL_DLG_CHANGE	(WM_USER +43)
#define WM_DOO_UPDATE_FILE_PATH	(WM_USER +44)
#define WM_DOO_UPDATE_G723_SLIDER_POS	(WM_USER +45)

#define WM_DOO_UPDATE_OVERLAY	(WM_USER +50)

#define POS_DISP_WND_X	0
#define POS_DISP_WND_Y	0
#define SZ_INIT_DISP_WND_NTSC_H	VIDEO_IN_H_SIZE
#define SZ_INIT_DISP_WND_NTSC_V	480
#define SZ_INIT_DISP_WND_PAL_H	VIDEO_IN_H_SIZE
#define SZ_INIT_DISP_WND_PAL_V	576

#define ID_S6010_DISP_WND	5000

enum	// Control...
{
	IDX_BUT_OPEN_STREAM_FILE,

	IDX_BUT_DISP_1,
	IDX_BUT_DISP_4,
	IDX_BUT_DISP_9,
	IDX_BUT_DISP_16,
	IDX_BUT_DISP_V1,
	IDX_BUT_DISP_V4,
	IDX_BUT_DISP_V9,
	IDX_BUT_DISP_V16,
	IDX_BUT_DISP_32,
	IDX_BUT_DISP_36,

	IDX_CBUT_CH_1,
	IDX_CBUT_CH_2,
	IDX_CBUT_CH_3,
	IDX_CBUT_CH_4,
	IDX_CBUT_CH_5,
	IDX_CBUT_CH_6,
	IDX_CBUT_CH_7,
	IDX_CBUT_CH_8,
	IDX_CBUT_CH_9,
	IDX_CBUT_CH_10,
	IDX_CBUT_CH_11,
	IDX_CBUT_CH_12,
	IDX_CBUT_CH_13,
	IDX_CBUT_CH_14,
	IDX_CBUT_CH_15,
	IDX_CBUT_CH_16,

	IDX_CBUT_CH_V1,
	IDX_CBUT_CH_V2,
	IDX_CBUT_CH_V3,
	IDX_CBUT_CH_V4,
	IDX_CBUT_CH_V5,
	IDX_CBUT_CH_V6,
	IDX_CBUT_CH_V7,
	IDX_CBUT_CH_V8,
	IDX_CBUT_CH_V9,
	IDX_CBUT_CH_V10,
	IDX_CBUT_CH_V11,
	IDX_CBUT_CH_V12,
	IDX_CBUT_CH_V13,
	IDX_CBUT_CH_V14,
	IDX_CBUT_CH_V15,
	IDX_CBUT_CH_V16,

	IDX_BUT_SETTING_GENERAL,
	IDX_BUT_MINIMIZE,

	IDX_BUT_PLAYER_PLAY,	// '1'...
	IDX_BUT_PLAYER_BACK_GOP,
	IDX_BUT_PLAYER_PAUSE,	// '2'...
	IDX_BUT_PLAYER_FW_ONE_FRM,
	IDX_BUT_PLAYER_FW_GOP,
	IDX_BUT_PLAYER_STOP,	// '3'...

	IDX_BUT_PLAYER_PLAY_ALL,	// 'Q'...
	IDX_BUT_PLAYER_BACK_GOP_ALL,
	IDX_BUT_PLAYER_PAUSE_ALL,	// 'W'...
	IDX_BUT_PLAYER_FW_ONE_FRM_ALL,
	IDX_BUT_PLAYER_FW_GOP_ALL,
	IDX_BUT_PLAYER_STOP_ALL,	// 'E'...

	IDX_CBUT_MISC_DEBLOCKING,
	IDX_CBUT_MISC_DERINGING,
	IDX_CBUT_MISC_DEINTERLACE,
	IDX_CBUT_MISC_MOTION_INFO,
	IDX_CBUT_MISC_CHANNEL_INFO,
	IDX_CBUT_MISC_WM_INFO,
	IDX_BUT_MISC_AVI_SAVE,
	IDX_BUT_MISC_SNAPSHOT,
	IDX_CBUT_MISC_I_BASED_PRED,

	IDX_CBUT_ZOOM_H_X1,
	IDX_CBUT_ZOOM_H_X2,
	IDX_CBUT_ZOOM_H_X3,
	IDX_CBUT_ZOOM_H_FIT,
	IDX_CBUT_ZOOM_V_X1,
	IDX_CBUT_ZOOM_V_X2,
	IDX_CBUT_ZOOM_V_X3,
	IDX_CBUT_ZOOM_V_FIT,

	IDX_BUT_SZ_DISP_X1,
	IDX_BUT_SZ_DISP_X2,
	IDX_BUT_SZ_DISP_X3,
	IDX_BUT_SZ_DISP_FULL,

	NUM_ALL_BUTTONS
};

enum
{
	IDX_DISP_MODE_1,
	IDX_DISP_MODE_4,
	IDX_DISP_MODE_9,
	IDX_DISP_MODE_16,
	IDX_DISP_MODE_V1,
	IDX_DISP_MODE_V4,
	IDX_DISP_MODE_V9,
	IDX_DISP_MODE_V16,
	IDX_DISP_MODE_32,
	IDX_DISP_MODE_36,
	NUM_DISP
};

const int GL_DISP_MODE_TO_NUM_DISP[NUM_DISP] = 
{
	1, 4, 9, 16, 1, 4, 9, 16, 32, 36
};
const int GL_DISP_MODE_TO_NUM_DIV[NUM_DISP] = 
{
	1, 2, 3, 4, 1, 2, 3, 4, 0, 6
};

enum
{
	IDX_CH_1,
	IDX_CH_2,
	IDX_CH_3,
	IDX_CH_4,
	IDX_CH_5,
	IDX_CH_6,
	IDX_CH_7,
	IDX_CH_8,
	IDX_CH_9,
	IDX_CH_10,
	IDX_CH_11,
	IDX_CH_12,
	IDX_CH_13,
	IDX_CH_14,
	IDX_CH_15,
	IDX_CH_16,
	NUM_CHANNEL
};

enum
{
	IDX_ZOOM_HOR,
	IDX_ZOOM_VER,
	NUM_ZOOM,
};

enum
{
	IDX_ZOOM_X1,
	IDX_ZOOM_X2,
	IDX_ZOOM_X3,
	IDX_ZOOM_FIT,
};

enum
{
	IDX_DISP_SZ_X1,
	IDX_DISP_SZ_X2,
	IDX_DISP_SZ_X3,
	IDX_DISP_SZ_FULL,
};

enum
{
	IDX_VIDEO_NTSC,
	IDX_VIDEO_PAL,
	NUM_VIDEO_MODE
};

#define MAX_NUM_SCALE	16
#define PLAYER_TIMER_RES	1
#define SZ_MAX_MP4_STREAM	(1024 *1024)
#define SZ_MAX_DISP_BUF	(VIDEO_IN_H_SIZE *576 *4)


class CDlgPlayerBase;
class CWndS6010Disp;
class CDlgMainControl;
class CInfoS6010StreamIdx;

extern CDlgPlayerBase *glPDlgPB;
extern CWndS6010Disp *glPWndDisp;
extern CDlgMainControl *glPDlgMC;
extern CInfoS6010StreamIdx *glInfoStreamIdx;

extern BYTE glBufStrh[30];

void MakeMP4SeqHdr (int szH, int szV, int bInterlace);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__13D827CA_8F1C_4287_8AD3_1B442360D23D__INCLUDED_)
