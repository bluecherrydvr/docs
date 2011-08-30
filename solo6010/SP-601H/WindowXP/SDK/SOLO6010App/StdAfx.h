// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8D653CAC_3914_46C8_8409_C35128607FAB__INCLUDED_)
#define AFX_STDAFX_H__8D653CAC_3914_46C8_8409_C35128607FAB__INCLUDED_

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

#include <afxsock.h>		// MFC socket extensions
#include "Xdefine.h"
#include "SOLO6010.h"

#define STR_APP_TITLE	"SOLO6010 Application"
#define STR_G_INFO_FILE_NAME	"config.slc"

#define WM_DOO_INDEX_BUTTON_CLICK	(WM_USER +30)
#define WM_DOO_LIVE_DISP_WND_CLICK	(WM_USER +36)
#define WM_DOO_APP_DLG_CTRL			(WM_USER +37)
#define WM_DOO_V_MOTION_EVENT		(WM_USER +38)
#define WM_DOO_CAM_BLK_DET_ALARM	(WM_USER +39)
#define WM_DOO_ADV_MOT_DET_ALARM	(WM_USER +40)

//#define __S6010APP_REC_NO_WRITE__

enum
{
	IDX_APP_MSG_ON_BUT_LIVE,
	IDX_APP_MSG_ON_BUT_PLAYER,
	IDX_APP_MSG_ON_BUT_RECORD,
	NUM_APP_MSG
};

enum
{
	IDX_SETM_MODE_MOSAIC,
	IDX_SETM_MODE_V_MOTION,
	IDX_SETM_MODE_ADV_MOT_DET,
};

enum
{
	IDX_VM_TH_0,
	IDX_VM_TH_1,
	IDX_VM_TH_2,
	IDX_VM_TH_3,
	IDX_VM_TH_4,
	IDX_VM_TH_5,
	IDX_VM_TH_6,
	IDX_VM_TH_7,
	IDX_VM_TH_8,
	NUM_VM_TH
};

struct INFO_APP_GLOBAL
{
	// General[S]
	BYTE bOpenS6010;
	BYTE bInitS6010;

	BYTE typeVideo;
	SIZE szVideo;
	
	CHAR strRecDir[MAX_PATH];
	// General[E]

	// Main Status[S]
	BYTE bOnLive;
	BYTE bOnRec;
	// Main Status[E]

	// Live[S]
	BYTE idxLiveSelWnd;
	BYTE bLiveDispCh;			// Whether show channel info or not
	BYTE bSetMosaicVM;			// whether show live control dlg. or mosaic & video-motion setting dlg.
	BYTE idxModeSetMosaic;		// whether now setting mosaic or video-motion or camera block detection or advanced motion detection
	BYTE bAlwaysTop;

	BYTE idxVCard;
	BYTE modeLiveDisp;
	BYTE modeDeinterlacing;
	COLORREF clrOverlay;
	BYTE numOverSurf;
	// Live[E]

	// Mosaic[S]
	BYTE MOSAIC_bufBSet[NUM_LIVE_DISP_WND];
	RECT MOSAIC_bufRect[NUM_LIVE_DISP_WND];
	WORD MOSAIC_nStrength;
	// Mosaic[E]

	// Video Motion[S]
	BYTE VM_bufBUse[NUM_LIVE_DISP_WND];
	WORD VM_bbufTh[NUM_LIVE_DISP_WND][SZ_BUF_V_MOTION_TH_IN_USHORT];
	WORD VM_minCntTh;
	BYTE VM_bShowGrid;
	
	BYTE VM_bExecThSetting;
	RECT VM_rcCurSelTh;				// currently dragging rectangle

	BYTE VM_clrModeBorderY;
	BYTE VM_clrModeBorderCb;
	BYTE VM_clrModeBorderCr;
	BYTE VM_clrModeBarY;
	BYTE VM_clrModeBarCb;
	BYTE VM_clrModeBarCr;
	BYTE VM_clrValBorderY;
	BYTE VM_clrValBorderCb;
	BYTE VM_clrValBorderCr;
	BYTE VM_clrValBarY;
	BYTE VM_clrValBarCb;
	BYTE VM_clrValBarCr;
	// Video Motion[E]

	// Camera Block Detection[S]
	BYTE CBD_bufBUseCBD[NUM_LIVE_DISP_WND];
	BYTE CBD_bufNumAlarmLevel[NUM_LIVE_DISP_WND];
	DWORD CBD_timeCheckInterval;
	// Camera Block Detection[E]

	// Advanced Motion Detection[S]
	BYTE ADVMD_bufBUse[NUM_LIVE_DISP_WND];
	BYTE ADVMD_bufNumMotionCheck[NUM_LIVE_DISP_WND];
	
	BYTE ADVMD_bbufMotDataDist[NUM_LIVE_DISP_WND][MAX_ADV_MOT_CHECK_ITEM];
	WORD ADVMD_bbbufMotionTh[NUM_LIVE_DISP_WND][MAX_ADV_MOT_CHECK_ITEM][SZ_BUF_V_MOTION_TH_IN_USHORT];
	WORD ADVMD_bufCntMinDet[NUM_LIVE_DISP_WND];
	
	BYTE ADVMD_bShowGrid;
	BYTE ADVMD_bExecThSetting;
	BYTE ADVMD_idxCurSelCheckItem;
	RECT ADVMD_rcCurSelTh;				// currently dragging rectangle
	// Advanced Motion Detection[E]

	// Video Matrix[S]
	BYTE bufIdxCamToWndCh[NUM_LIVE_DISP_WND];
	// Video Matrix[E]

	// Record[S]
	SETTING_REC setRec;
	// Record[E]

	// Video Loss[S]
	DWORD VL_checkInterval;			// msec
	// Video Loss[E]

	// SW Player[S]
	BOOL bUseMP4Flt_Deblk;
	BOOL bUseMP4Flt_Dering;

	BOOL bUseImgFlt_Deint;
	BOOL bUseImgFlt_UpSc;
	BOOL bUseImgFlt_DnSc;
	// SW Player[E]

	// Encoder OSD[S]
	BOOL ENC_OSD_bShowCh;
	BOOL ENC_OSD_bShowDate;
	BOOL ENC_OSD_bShowTime;

	BYTE ENC_OSD_bUseShadowText;
	BYTE ENC_OSD_colorY;
	BYTE ENC_OSD_colorU;
	BYTE ENC_OSD_colorV;

	BYTE ENC_OSD_offsetH;
	BYTE ENC_OSD_offsetV;
	// Encoder OSD[E]

	// GPIO[S]
	BYTE GPIO_bufMode[NUM_GPIO_ALL_PORT];
	BYTE GPIO_bufIntCfg[NUM_GPIO_ALL_PORT];
	// GPIO[E]
};

class CSOLO6010;
class CSOLO6010AppDlg;
class CWndLiveDisp;
class CDlgCtrlLive;
class CRecManager; 
class CHWDecManager; 

extern INFO_APP_GLOBAL glBufGInfo[NUM_MAX_S6010];
extern INFO_APP_GLOBAL *glPCurGInfo;
extern CSOLO6010AppDlg *glPDlgApp;
extern CWndLiveDisp *glPWndLiveDisp;
extern CSOLO6010 *glPCurSelS6010;
extern CRecManager *glPCurRecManager;
extern CHWDecManager *glPCurHWDecManager;

extern CDlgCtrlLive *glPDlgCtrlLive;

BOOL MiscBrowseForFolder (CString *pStrFolder, CString strTitle, HWND hWndOwner);
void SetFastMemcpy ();
extern void (*FastMemcpy) (BYTE *pDest, BYTE *pSrc, int szCopy);
DWORD GetHexValueFromHexString (LPCTSTR strHex);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8D653CAC_3914_46C8_8409_C35128607FAB__INCLUDED_)
