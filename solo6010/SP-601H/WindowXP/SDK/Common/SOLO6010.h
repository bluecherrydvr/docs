#pragma once

#include <windows.h>
#include "Solo6010_Global.h"
#include "Solo6010_InternalDef.h"
#include "..\\SOLO6010App\\Xdefine.h"

#pragma pack(push, 4)

// Record callback[S]
typedef void (CALLBACK *MP4_CAPTURE_PROC) (void *pContext, BYTE *bufMP4, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError);
typedef void (CALLBACK *JPEG_CAPTURE_PROC) (void *pContext, BYTE *bufJPEG, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError);
typedef void (CALLBACK *G723_CAPTURE_PROC) (void *pContext, int idxCh, BYTE *bufG723, DWORD szBuf, DWORD flagError);
// Record callback[S]

// Record MP4[S]
#define MP4_IMG_QUALITY_BEST	0
#define MP4_IMG_QUALITY_WORST	(MP4E_MAX_QP -MP4E_MIN_QP)	// 29
	// The range of image quality is from 0(best) to 29(worst)

enum	// Record image size
{
	IMG_SZ_704X240_704X288,
	IMG_SZ_352X240_352X288,
	IMG_SZ_176X112_176X144,
	IMG_SZ_704X480_704X576,
	NUM_IMG_SZ
};
const USHORT IMG_SZ_H_FROM_INDEX_IMG_SZ[NUM_VIDEO_TYPE][NUM_IMG_SZ] =
{
	VIDEO_IN_H_SIZE, VIDEO_IN_H_SIZE /2, VIDEO_IN_H_SIZE /4, VIDEO_IN_H_SIZE, VIDEO_IN_H_SIZE, VIDEO_IN_H_SIZE /2, VIDEO_IN_H_SIZE /4, VIDEO_IN_H_SIZE
};
const USHORT IMG_SZ_V_FROM_INDEX_IMG_SZ[NUM_VIDEO_TYPE][NUM_IMG_SZ] =
{
	240, 240, 112, 480, 288, 288, 144, 576
};
const USHORT IS_INTERLACE_FROM_INDEX_IMG_SZ[NUM_IMG_SZ] =
{
	0, 0, 0, 1
};

// FPS calculation formula[S]
//
// NTSC
// Field : 60                  if interval = 0
//       : 30 /interval        else
// Frame : 30 /(interval +1)
//
// PAL
// Field : 50                  if interval = 0
//       : 25 /interval        else
// Frame : 25 /(interval +1)
//
// FPS calculation formula[E]

enum	// Frame/sec index
{
	FPS_60,		// FPS_50		// only available with image size 704x240, 352x240, 352x120
	FPS_30,		// FPS_25
	FPS_15,		// FPS_12_5
	FPS_10,		// FPS_8_3
	FPS_7_5,	// FPS_6_25
	FPS_6,		// FPS_5
	FPS_5,		// FPS_4_2
	FPS_3,		// FPS_3_1
	FPS_2,		// FPS_2
	FPS_1,		// FPS_1
	FPS_0_5,	// FPS_0_5
	FPS_0_25,	// FPS_0_25
	FPS_0_2,	// FPS_0_2
	FPS_0_1,	// FPS_0_1		// only available with image size 704x480
	NUM_FPS
};
const char *const GL_STR_FPS[NUM_VIDEO_TYPE][NUM_FPS] = {
	"60", "30",   "15",  "10",  "7.5", "6",   "5",   "3", "2", "1", "0.5", "0.25", "0.2", "0.1",
	"50", "25", "12.5", "8.3", "6.25", "5", "4.2", "3.1", "2", "1", "0.5", "0.25", "0.2", "0.1",
};

struct SETTING_MP4_REC
{
	BYTE bSet;				// Record(1) or not(0)
	BYTE idxImgSz;			// Record image size
	BYTE nKeyFrmInterval;	// Interval between key(I) frame
	BYTE nQuality;			// Image quality
	BYTE bIBasedPred;		// Key(I) frame based prediction
	BYTE bUseCBR;			// CBR(1) or VBR(0)
	BYTE idxFPS;			// Frame/sec index
	WORD nMotionTh;			// Record motion detection threshold
	
	DWORD nBitrate;			// Bitrate (bit/sec)
	DWORD szVirtualDecBuf;	// Virtual decoder buffer size (byte)
};
// Record MP4[E]

// Record JPEG[S]
struct SETTING_JPEG_REC
{
	BYTE bRec;
};
// Record JPEG[E]

// Record G723[S]
struct SETTING_G723_REC
{
	BYTE idxSampRate;
	BYTE bufBEnc[NUM_G723_CH];	// When recording, even channel recording property is applied to odd channel.
								// When playing, all channel have own property.
};
// Record G723[E]

// Record general[S]
struct SETTING_REC
{
	SETTING_MP4_REC setMP4[NUM_VID_TOTAL_ENC_CH];
	SETTING_JPEG_REC setJPEG[NUM_VID_TOTAL_ENC_CH];
	SETTING_G723_REC setG723;

	BYTE modeRecMP4;			// write raw data or write by channel
	BYTE modeRecJPEG;			// ..
	BYTE modeRecG723;			// ..
	BYTE nNotifyBufLevelMP4;	// 0[notify every interrupt] to 100[notify when buffer full]
	BYTE nNotifyBufLevelJPEG;	// ..
	BYTE nNotifyBufLevelG723;	// ..
};

enum
{
	REC_DATA_RCV_BY_FRAME,
	REC_DATA_RCV_BY_RAW_DATA,
	NUM_REC_DATA_RCV_MODE
};	// Recorded data receive mode
	// REC_DATA_RCV_BY_FRAME    : rec data is received by frame
	// REC_DATA_RCV_BY_RAW_DATA : rec data is received by raw data
	// Frame : [Frame Header] + [Stream Data]
	// Raw Data : [Frame Header] + [Stream Data] + [Frame Header] + [Stream Data] + ...
	//            This buffer is ring buffer. You must handle buffer overlap manually.
	//            REC_DATA_RCV_BY_RAW_DATA mode is more difficult than REC_DATA_RCV_BY_FRAME mode,
	//            but more efficient.

#define DEFAULT_REC_BUF_NOTIFY_LEVEL	25
// Record general[E]

// Live[S]
enum
{
	VCARD_INTEL_ONBOARD,			// Intel Onboard Video Card
	VCARD_NVIDIA,					// NVidia Video Card
	VCARD_ATI,						// ATI Video Card
	VCARD_OTHER,					// Other Video Card
	NUM_VCARD
};

#define SZ_OVERLAY_WIDTH		(704 +64)
#define NUM_DEF_FLIP_OVER_60FPS_SURF	3	// Triple buffering
#define NUM_DEF_FLIP_OVER_30FPS_SURF	2	// Double buffering
enum
{
	LIVE_DISP_1_OVERLAY_60_FI,
	LIVE_DISP_1_OVERLAY_30_FR,
	LIVE_DISP_N_OVERLAY_60_FI_FLIP,
	LIVE_DISP_N_OVERLAY_30_FR_FLIP,
	LIVE_DISP_SYS_TO_OVERLAY_60_FI_FLIP,
	LIVE_DISP_SYS_TO_OVERLAY_30_FR_FLIP,
	LIVE_DISP_N_OVERLAY_30_FI_FLIP,
	NUM_LIVE_DISP_MODE
};
enum
{
	LIVE_DI_BOB,
	LIVE_DI_WEAVE,
	NUM_LIVE_DI_MODE
};

typedef void (CALLBACK *OVERLAY_FLIP_PROC) (void *pContext, int idxBuf, int numBuf, int idxField);
// Live[E]

// Video Motion[S]
#define VMOTION_NOT_CHANGE_PROP	0xff

typedef void (CALLBACK *V_MOTION_ALARM_PROC) (void *pContext, unsigned long flagCurVMotion);
// Video Motion[E]

// Camera block detection[S]
typedef void (CALLBACK *CAM_BLK_DET_ALARM_PROC) (void *pContext, unsigned long flagChBlockDetected);
// Camera block detection[E]

// Advanced motion detection[S]
typedef void (CALLBACK *ADV_MOT_DET_ALARM_PROC) (void *pContext, unsigned long flagAdvMotDet, unsigned char *bufMotionMap);
// Advanced motion detection[E]

// MPEG4 HW Decode[S]
// MPEG4 HW Decode[E]

// GPIO[S]
enum
{
	GPIO_MODE_IN_OR_INT,
	GPIO_MODE_OUT,
	GPIO_MODE_I2C,
	GPIO_MODE_ETC,
	NUM_GPIO_MODE
};

// SoftLogic RDK initial value (GPIO[0]-[3]: I2C, GPIO[4]-[5]: Video Reset)
#define GPIO_INIT_MODE_IN	0xffff00c0
#define GPIO_INIT_MODE_OUT	0x00000030
#define GPIO_INIT_MODE_I2C	0x0000000f
#define GPIO_INIT_MODE_ETC	0x0000ff00

typedef void (CALLBACK *GPIO_INT_ALARM_PROC) (void *pContext, unsigned long maskPortIntStat);
// GPIO[E]

#pragma pack(pop, 4)

class CSOLO6010  
{
public:
	// Constructor, destructor[S]
	CSOLO6010(int idxS6010);
	virtual ~CSOLO6010();
	// Constructor, destructor[E]

	// General[S]
	static int GENERAL_GetNumOfS6010 ();

	BOOL GENERAL_Open ();
	void GENERAL_Close ();
	BOOL GENERAL_Init (BYTE typeVideo);
	__inline int GENERAL_GetID ()		{ return m_idxS6010; }

	static UINT AllocateTimerRes ();
	static void ReleaseTimerRes ();
	// General[E]

	// Register IO[S]
	DWORD REGISTER_Get (DWORD nAddr);
	void REGISTER_Set (DWORD nAddr, DWORD nData);
	// Register IO[E]

	// Record[S]
	BOOL REC_MP4_RealChSetProp (BYTE idxCh, BYTE bRec, BYTE idxImgSz, BYTE nKeyFrmInterval,
		BYTE nQuality, BYTE bIBasedPred, BYTE idxFPS, WORD nMotionTh,
		BYTE bUseCBR, DWORD nBitRate, DWORD szVirtualDecBuf);
	void REC_MP4_RealChGetProp (BYTE idxCh, BYTE &bRec, BYTE &idxImgSz, BYTE &nKeyFrmInterval,
		BYTE &nQuality, BYTE &bIBasedPred, BYTE &idxFPS, WORD &nMotionTh,
		BYTE &bUseCBR, DWORD &nBitRate, DWORD &szVirtualDecBuf);
	BOOL REC_MP4_VirtChSetProp (BYTE idxCh,	BYTE bRec, BYTE nKeyFrmInterval,
		BYTE nQuality, BYTE idxFPS, BYTE bUseCBR, DWORD nBitRate, DWORD szVirtualDecBuf);
	void REC_MP4_VirtChGetProp (BYTE idxCh, BYTE &bRec, BYTE &idxImgSz, BYTE &nKeyFrmInterval,
		BYTE &nQuality, BYTE &bIBasedPred, BYTE &idxFPS, WORD &nMotionTh,
		BYTE &bUseCBR, DWORD &nBitRate, DWORD &szVirtualDecBuf);
	__inline BOOL REC_MP4_IsBufferFlushed () { return m_bRecMP4_BufferFlushed; }

	BOOL REC_JPEG_RealChSetProp (BYTE idxCh, BYTE bRec, BYTE idxQuality);
	void REC_JPEG_RealChGetProp (BYTE idxCh, BYTE &bRec, BYTE &idxQuality);
	BOOL REC_JPEG_VirtChSetProp (BYTE idxCh, BYTE bRec, BYTE idxQuality);
	void REC_JPEG_VirtChGetProp (BYTE idxCh, BYTE &bRec, BYTE &idxQuality);
	__inline BOOL REC_JPEG_IsBufferFlushed () { return m_bRecJPEG_BufferFlushed; }

	void REC_G723_SetProp (BYTE idxChPair, BYTE bRec, BYTE idxSamplingRate);
	void REC_G723_GetProp (BYTE idxChPair, BYTE &bRec, BYTE &idxSamplingRate);
	__inline BOOL REC_G723_IsBufferFlushed () { return m_bRecG723_BufferFlushed; }

	void REC_SetRecDataRcvMode (BYTE modeMP4, BYTE modeJPEG, BYTE modeG723);
	void REC_GetRecDataRcvMode (BYTE &modeMP4, BYTE &modeJPEG, BYTE &modeG723);
	void REC_SetNotifyBufferLevel (BYTE levelMP4, BYTE levelJPEG, BYTE levelG723);
	void REC_GetNotifyBufferLevel (BYTE &levelMP4, BYTE &levelJPEG, BYTE &levelG723);
	BOOL REC_Start ();
	BOOL REC_End ();

	void REC_MP4_RegisterCallback (MP4_CAPTURE_PROC cbFnMP4Capture, void *pContext);
	void REC_MP4_UnregisterCallback ();
	void REC_JPEG_RegisterCallback (JPEG_CAPTURE_PROC cbFnJPEGCapture, void *pContext);
	void REC_JPEG_UnregisterCallback ();
	void REC_G723_RegisterCallback (G723_CAPTURE_PROC cbFnG723Capture, void *pContext);
	void REC_G723_UnregisterCallback ();

	BOOL IsValidInfoRec (SETTING_REC *pSetRec);
	DWORD GetNumMP4EncCRU (SETTING_MP4_REC *pInfoMP4Enc);

	BOOL REC_MP4_RealChChangePropOnRec (BYTE idxCh, BYTE bRec, BYTE idxImgSz, BYTE nKeyFrmInterval,
		BYTE nQuality, BYTE bIBasedPred, BYTE idxFPS, BYTE bUseCBR, DWORD nBitRate, DWORD szVirtualDecBuf);
	// Record[E]

	// Live display & control[S]
	BOOL LIVE_Start (INFO_DDRAW_SURF *pIDDrawSurf);
	BOOL LIVE_End ();
	void LIVE_SetProp ();
	void LIVE_RegisterCallback (OVERLAY_FLIP_PROC cbFnFlipOverlay, void *pContext);
	void LIVE_UnregisterCallback ();
	
	void LIVE_Init ();
	void LIVE_SetSplitMode (int idxSplitMode);
	__inline BYTE LIVE_GetSplitMode ()					{ return m_iLive.idxSplitMode; }
	void LIVE_SetWndLiveOrDec (int idxWnd, int bLive);
	BOOL LIVE_IsWndLiveOrDec (int idxWnd);				// TRUE if live, FALSE if MP4 decoding
	void LIVE_ChangeLiveCh (int idxSelWnd, int idxCh);
	void LIVE_ChangeDecCh (int idxSelWnd, int idxCh);
	__inline BYTE LIVE_GetChFromIdxWnd (int idxWnd)		{ return m_iLive.bufIdxCh[idxWnd]; }
	__inline BYTE LIVE_GetIdxWndFromDecCh (BYTE idxCh)	{ return m_iLive.bufIdxWndFromDecCh[idxCh]; }
	__inline BYTE LIVE_GetDecChFromIdxWnd (BYTE idxWnd)	{ return m_iLive.bufIdxDecCh[idxWnd]; }
	// Live display & control[E]

	// Color control[S]
	void COLOR_SetHue (BYTE idxCh, BYTE nHue);
	void COLOR_SetSaturation (BYTE idxCh, BYTE nSaturation);
	void COLOR_SetContrast (BYTE idxCh, BYTE nContrast);
	void COLOR_SetBrightness (BYTE idxCh, BYTE nBrightness);
	void COLOR_Get (BYTE idxCh, BYTE &nHue, BYTE &nSaturation, BYTE &nContrast, BYTE &nBrightness);
	// Color control[E]

	// Mosaic control[S]
	void MOSAIC_SetOnOffAndRect (BYTE idxCh, BYTE bSet, RECT *pRcMosaic);
	void MOSAIC_GetOnOffAndRect (BYTE idxCh, BYTE *pBSet, RECT *pRcMosaic);
	void MOSAIC_SetStrength (DWORD nStrength);
	DWORD MOSAIC_GetStrength ();

	void MOSAIC_SetInfo (INFO_MOSAIC *pInfoMosaic);
	// Mosaic control[E]

	// Video motion control[S]
	void VMOTION_SetOnOff (BYTE idxCh, BYTE bOn);
	__inline BYTE VMOTION_GetOnOff (BYTE idxCh)			{ return m_iVMotion.bufBUseMotion[idxCh]; }
	void VMOTION_SetThreshold (BYTE idxCh, WORD *pBufThreshold);
	__inline WORD *VMOTION_GetThreshold (BYTE idxCh)	{ return m_iVMotion.bbufMotionTh[idxCh]; }
	void VMOTION_SetMinDetectThreshold (WORD minDetThreshold);
	__inline WORD VMOTION_GetMinDetectThreshold ()		{ return m_iVMotion.cntMinDet; }
	void VMOTION_SetGridOnOff (BYTE bOn);
	__inline BYTE VMOTION_GetGridOnOff ()				{ return m_iVMotion.bShowGrid; }
	void VMOTION_SetBorderColor (BYTE idxColModeY, BYTE valY, BYTE idxColModeCb, BYTE valCb, BYTE idxColModeCr, BYTE valCr);
	void VMOTION_GetBorderColor (BYTE &idxColModeY, BYTE &valY, BYTE &idxColModeCb, BYTE &valCb, BYTE &idxColModeCr, BYTE &valCr);
	void VMOTION_SetBarColor (BYTE idxColModeY, BYTE valY, BYTE idxColModeCb, BYTE valCb, BYTE idxColModeCr, BYTE valCr);
	void VMOTION_GetBarColor (BYTE &idxColModeY, BYTE &valY, BYTE &idxColModeCb, BYTE &valCb, BYTE &idxColModeCr, BYTE &valCr);
	BYTE *VMOTION_GetCurMotionMap ();	// 512Byte per channel, Total 16channel, 8192Byte

	void VMOTION_RegisterAlarmCallback (V_MOTION_ALARM_PROC cbFnVMotionAlarm, void *pContext);
	void VMOTION_UnregisterAlarmCallback ();
	// Video motion control[E]

	// Camera block detection[S]
	void CAM_BLK_DET_SetOnOff (BYTE idxCh, BYTE bOn);
	__inline BYTE CAM_BLK_DET_GetOnOff (BYTE idxCh)		{ return m_iCamBlkDet.bufBUseDetection[idxCh]; }
	void CAM_BLK_DET_SetAlarmLevel (BYTE idxCh, BYTE nAlarmLevel);
	void CAM_BLK_DET_SetCheckInterval (DWORD nCheckInterval);
	void CAM_BLK_DET_RegisterAlarmCallback (CAM_BLK_DET_ALARM_PROC cbFnCamBlkDetAlarm, void *pContext);
	void CAM_BLK_DET_UnregisterAlarmCallback ();
	// Camera block detection[E]

	// Advanced motion detection[S]
	void ADV_MOT_SetOnOff (BYTE idxCh, BYTE bOn);
	void ADV_MOT_SetNumMotChkItem (BYTE idxCh, BYTE numMotChkItem);
	void ADV_MOT_SetMotChkDist (BYTE idxCh, BYTE idxChkItem, BYTE numDist);
	void ADV_MOT_SetThreshold (BYTE idxCh, BYTE idxChkItem, WORD *pBufThreshold);
	void ADV_MOT_SetMinDetectThreshold (BYTE idxCh, WORD minDetThreshold);

	void ADV_MOT_RegisterAlarmCallback (ADV_MOT_DET_ALARM_PROC cbFnAdvMotDetAlarm, void *pContext);
	void ADV_MOT_UnregisterAlarmCallback ();
	void ADV_MOT_CheckAdvMotion (DWORD &flagAdvMotDet, BYTE *bufMotionMap);
	// Advanced motion detection[E]

	// Video matrix control[S]
	void VMATRIX_Set (BYTE *bufCamToCh);
	void VMATRIX_Get (BYTE *bufCamToCh);

	void VMATRIX_SetInfo (INFO_VIDEO_MATRIX *pInfoVMatrix);
	// Video matrix control[E]

	// HW MP4 decoding[S]
	INFO_MP4_DEC_STAT *MP4D_Open ();
	void MP4D_Close ();

	BYTE *MP4D_GetFrameBuffer (DWORD &posStart, DWORD &posEnd, DWORD &szBufFrm, DWORD &numRemainQ);
	int MP4D_UpdateFrameBufferStat (int numFrm, DWORD *bufPosFrm, DWORD *bufIdxFrm, DWORD *bufTimeMS, BYTE *bufIdxDecMode, BYTE *bufBFrmIntp);
	void MP4D_DecodeNFrm (int numDec);
	void MP4D_PauseDecoding ();
	// HW MP4 decoding[E]

	// HW G723 decoding[S]
	INFO_G723_DEC_STAT *G723D_Open (int idxSamplingRate);
	void G723D_Close ();
	void G723D_Play ();
	void G723D_Stop ();
	// HW G723 decoding[E]

	// GPIO[S]
	__inline void GPIO_SetMode (BYTE idxMode, DWORD maskPort)			{ GPIO_Operation (idxMode, maskPort); }
	__inline void GPIO_Write (DWORD maskPort, DWORD nData)				{ GPIO_Operation (GPIO_OP_WRITE, maskPort, nData); }
	__inline DWORD GPIO_Read ()											{ return GPIO_Operation (GPIO_OP_READ); }
	void GPIO_EnableInterrupt (DWORD maskPort, DWORD intCfg);

	unsigned long GPIO_Operation (BYTE idxOp, DWORD maskPort = 0, DWORD nData = 0);
	void GPIO_RegisterIntAlarmCallback (GPIO_INT_ALARM_PROC cbFnGPIO_IntAlarm, void *pContext);
	void GPIO_UnregisterIntAlarmCallback ();
	// GPIO[E]

	// Video Loss[S]
	DWORD VLOSS_GetState ();		// Bit0: Cam1, Bit1: Cam12, ..., Bit15: Cam116 (1:Loss, 0:OK)(Not Channel, Camera)
	// Video Loss[E]

	// Encoder OSD[S]
	void ENC_OSD_Init (BOOL bUseShadowText, BYTE colorY, BYTE colorU, BYTE colorV, BYTE offsetH, BYTE offsetV);
	void ENC_OSD_Clear (int idxCh);
	void ENC_OSD_Set (int idxCh, const char*strOSD, DWORD lenStrOSD, int idxStartLine);
	// Encoder OSD[E]

	// Video OSG[S]
	// Video OSG[E]

	// UART[S]
	void UART_Setup (int idxPort, int idxBaudRate, int idxDataSize, int idxStopBit, int idxParity);
	void UART_Read (int idxPort, BYTE *pBufRead, int szBufRead, int *pSzActualRead);
	void UART_Write (int idxPort, BYTE *pBufWrite, int szBufWrite, int *pSzActualWritten);
	// UART[E]

	// I2C I/O[S]
	int I2C_Read (int nChannel, int nAddrSlave, int nAddrSub);
	int I2C_Write (int nChannel, int nAddrSlave, int nAddrSub, int nData);
	// I2C I/O[S]

	// ETC[S]
	__inline int GetVInOffset_H ()	{ return m_iS6010Init.posVInH; }
	__inline int GetVInOffset_V ()	{ return m_iS6010Init.posVInV; }
	// ETC[S]

private:
	// General[S]
	int m_idxS6010;
	HANDLE m_hS6010;

	INFO_S6010_INIT m_iS6010Init;

	HANDLE m_bufHThread[S6010_NUM_THREAD];
	volatile BOOL m_bufBEndTh[S6010_NUM_THREAD];
	HANDLE m_bufHEvt[S6010_NUM_EVENT];
//	CRITICAL_SECTION m_bufCrt[S6010_NUM_CRT];

	INFO_SHARE_DATA *m_pIShare;

	volatile BOOL m_bInit;
	BOOL m_bLiveOn;
	BOOL m_bRecOn;

	LPVOID m_bufCbProc[S6010_NUM_CALLBACK];
	LPVOID m_bufCbContext[S6010_NUM_CALLBACK];
	// General[E]

	// Record[S]
	INFO_REC m_iRec;
	BYTE m_recModeMP4;
	BYTE m_recModeJPEG;
	BYTE m_recModeG723;
	volatile BOOL m_bRecMP4_BufferFlushed;
	volatile BOOL m_bRecJPEG_BufferFlushed;
	volatile BOOL m_bRecG723_BufferFlushed;
	// Record[S]

	// Live display & control[S]
	INFO_LIVE m_iLive;
	// Live display & control[S]

	// Mosaic control[S]
	INFO_MOSAIC m_iMosaic;
	// Mosaic control[E]
	
	// Video motion control[S]
	INFO_V_MOTION m_iVMotion;
	// Video motion control[E]

	// Camera block detection[S]
	INFO_CAM_BLK_DETECTION m_iCamBlkDet;
	// Camera block detection[E]

	// Advanced motion detection[S]
	INFO_ADV_MOT_DETECTION m_iAdvMotDet;
	// Advanced motion detection[E]

	// Video matrix control[S]
	INFO_VIDEO_MATRIX m_iVMatrix;
	// Video matrix control[E]

	// HW MP4 decoding[S]
	BOOL m_bMP4DecOn;
	// HW MP4 decoding[E]

	// HW G723 decoding[S]
	// HW G723 decoding[E]

	// Multimedia timer support[S]
	static LONG m_cntTimerUsed;
	static UINT m_nMMTimerRes;		// Multimedia timer Resolution
	static CRITICAL_SECTION m_crtAccMMTimerCnt;
	// Multimedia timer support[E]

	// Friend function[S]
	friend UINT WINAPI ThFn_MP4DataGet (LPVOID pParam);
	friend UINT WINAPI ThFn_JPEGDataGet (LPVOID pParam);
	friend UINT WINAPI ThFn_G723DataGet (LPVOID pParam);
	friend UINT WINAPI ThFn_OverlayFlip (LPVOID pParam);
	friend UINT WINAPI ThFn_VMotion (LPVOID pParam);
	friend UINT WINAPI ThFn_CheckCamBlkDet (LPVOID pParam);
	friend UINT WINAPI ThFn_GPIO_IntAlarm (LPVOID pParam);
	friend UINT WINAPI ThFn_AdvMotionData (LPVOID pParam);
	// Friend function[E]
};
