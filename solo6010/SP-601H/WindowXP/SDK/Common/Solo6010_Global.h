#ifndef __SOLO6010_GLOBAL_H
#define __SOLO6010_GLOBAL_H

#pragma pack(push, 4)

//#define __APP_DEBUG_MODE__
//#define __JPEG_OPT_FOR_DECODING__

#define VIDEO_IN_H_SIZE			704	// Change this variable to the new horizontal image size

// Global Definition[S]
#define SOLO6010_DRIVER_NAME				"SOLO6010"
#define SOLO6010_DRIVER_NAME_UNICODE		L"SOLO6010"
#define SOLO6010_DEFAULT_ID 				L"SOLO6010"
#define SOLO6010_DEFAULT_DEV_VEN_ID 		0x94136010
#define DBG_NAME							SOLO6010_DRIVER_NAME ": "

#define NUM_MAX_S6010			6

enum
{
	COMMON_EVT_MP4,
	COMMON_EVT_JPEG,
	COMMON_EVT_G723,
	COMMON_EVT_OVERLAY_FLIP,
	COMMON_EVT_V_MOTION,
	COMMON_EVT_MP4D_COPY_DATA,
	COMMON_EVT_GPIO_INT_ALARM,
	COMMON_EVT_ADV_MOTION_DATA,
	NUM_COMMON_EVT
};

enum
{
	VIDEO_TYPE_NTSC,
	VIDEO_TYPE_PAL,
	NUM_VIDEO_TYPE
};

#define MAX_SZ_REC_IMAGE_H		VIDEO_IN_H_SIZE
#define MAX_SZ_REC_IMAGE_V		576

//// About MP4 Core Resource Use[S]
#define NUM_CRU_176_112			1
#define NUM_CRU_352_240			4
#define NUM_CRU_704_240			8
#define NUM_CRU_704_480			16
#define MAX_NUM_CRU_MP4_ENC		(NUM_CRU_704_240 *60 *4)
#define MAX_NUM_CRU_MP4_DEC		(NUM_CRU_704_240 *60 *2)
//// About MP4 Core Resource Use[E]
// Global Definition[E]


// Register I/O[S]
#define ADDR_FIRST_REG			0x00000000
#define ADDR_LAST_REG			0x00000d14

typedef struct _INFO_REG_RW
{
	unsigned long nAddr;
	unsigned long nVal;
} INFO_REG_RW;
// Register I/O[E]


// Video Record[S]
#define NUM_VID_REAL_ENC_CH		16
#define NUM_VID_VIRT_ENC_CH		16
#define NUM_VID_TOTAL_ENC_CH	32

//// MPEG4 Record[S]

#define MP4E_MAX_QP				31
#define MP4E_MIN_QP				3

#define MP4E_ADD_VAL_FOR_CHANGE_REC_SETTINGS	0x10

typedef struct _INFO_MP4_REC
{
	unsigned char bSet;		// Change Settings(1) or Leave Unchanged(0)
	unsigned char nScale;	// ABBB : A:IsInterlace?, BBB:Scale
	unsigned char nGOP;
	unsigned char nQP;
	unsigned char bIBasedPred;
	unsigned char bUseCBR;
	unsigned short nInterval;
	unsigned short nMotionTh;
	
	unsigned long nBitrate;
	unsigned long szVirtualDecBuf;
} INFO_MP4_REC;
//// MPEG4 Record[E]

//// JPEG Record[S]
enum
{
	JPEG_IMG_QUALITY_LOW,
	JPEG_IMG_QUALITY_MEDIUM,
	JPEG_IMG_QUALITY_HIGH,
	NUM_JPEG_IMG_QUALITY
};

typedef struct _INFO_JPEG_REC
{
	unsigned char bRec;
	unsigned char idxQP;
} INFO_JPEG_REC;
//// JPEG Record[E]

// Video Record[E]


// G723 Record[S]
#define NUM_G723_CH						20
#define NUM_G723_CH_PAIR				10
#define SZ_G723_1_FRM					48
#define NUM_SAMPLE_G723_1_FRM_DEC		(SZ_G723_1_FRM *8 /3)
#define SZ_BUF_IN_BYTE_G723_1_FRM_DEC	(NUM_SAMPLE_G723_1_FRM_DEC *2)
#define SZ_G723_1_ITEM					1024	// 1 line size of G723 Ext Ring buf

#define SZ_G723_TIME_INFO				8		// Time information

enum
{
	G723_SAMPLING_RATE_8K,
	G723_SAMPLING_RATE_16K,
	NUM_G723_SR
};

// ------------------------------------------------------------------------
// G723 Buffer Structure
// 1 Interrupt => All 20 Channel Encoding & Decoding Data is made
// (1024 *2 byte (> 480 *20 *2))
// G723 Ring Buffer Size = 32
//
// G723 Buf = [32 *1024](Encode Ring Buffer) +[32 *1024](Decode Ring Buffer)
// ------------------------------------------------------------------------
typedef struct _INFO_G723_REC
{
	unsigned char idxSampRate;
	unsigned char bufBEncChPair[NUM_G723_CH_PAIR];	// [Ch0, Ch1], [Ch2, Ch3], ..., [Ch18, Ch19]
} INFO_G723_REC;
// G723 Record[E]


// Record Control[S]
// ------------------------------------------------------------------------
// SOLO6010 Chip-Driver-Application MPEG4 Rec Data Memory Management
//
// SOLO6010 : Write Data from [Ext. End] with [Size Frame] and
//            Modify [Ext. End] to [Ext. End] +[Size Frame]
//
// Driver   : Write S6010 Rec Data to Drv-App Common Buffer
//              - S6010 Rec Data = From [Ext. Start] to [Ext. End]
//              - Drv-App Common Buffer = From [PC End] with [Size S6010 Rec Data]
//            Modify [Ext. Start] to [Ext. End]
//            Modify [PC End] to [PC End] +[Size S6010 Rec Data]
//
// App      : Write Drv-App Common Buffer data (From [PC Start] to [PC End]) to File
//            Modify [PC Start] to [PC End]
// ------------------------------------------------------------------------
typedef struct _INFO_REC
{
	INFO_MP4_REC iMP4[NUM_VID_TOTAL_ENC_CH];
	INFO_JPEG_REC iJPEG[NUM_VID_TOTAL_ENC_CH];
	INFO_G723_REC iG723;

	unsigned char nNotifyBufLevelMP4;
	unsigned char nNotifyBufLevelJPEG;
	unsigned char nNotifyBufLevelG723;
} INFO_REC;

#define REC_BUF_STAT_FLAG_SDRAM_OVF			(1 <<0)
#define REC_BUF_STAT_FLAG_SHR_MEM_OVF		(1 <<1)
#define REC_BUF_STAT_FLAG_P2M_ERROR			(1 <<2)
typedef struct _REC_BUF_STAT
{
	volatile unsigned long addrStart;	// App:Write, Drv:Read,  Init
	volatile unsigned long addrEnd;		// App:Read,  Drv:Write, Init
	unsigned long szBuf;

	volatile unsigned int flagError;
} REC_BUF_STAT;

#define NUM_STAT_VIDEO_ENC_ITEM	512
typedef struct _STAT_VIDEO_ENC_ITEM
{
	int idxCh;
	int typeVOP;			// 0:I, 1:P, 2:ERROR

	int posMP4;
	int szMP4;
	int posJPEG;
	int szJPEG;

	unsigned long timeSec;
	unsigned long timeUSec;
} STAT_VIDEO_ENC_ITEM;

typedef struct _STAT_VIDEO_ENC
{
	int idxLastQ;

	int posIndexHW;
	int posIndexHW_Minus1;
	int posIndexDrv;

	STAT_VIDEO_ENC_ITEM bufIndex[NUM_STAT_VIDEO_ENC_ITEM];
} STAT_VIDEO_ENC;

//// MPEG4 Record Control[S]
enum
{
	VE_STATUS_0,
	VE_STATUS_1,
	VE_STATUS_2,
	VE_STATUS_3,
	VE_STATUS_4,
	VE_STATUS_5,
	VE_STATUS_6,
	VE_STATUS_7,
	VE_STATUS_8,
	VE_STATUS_9,
	VE_STATUS_10,
	VE_STATUS_11,
	NUM_ITEM_MP4_FRM_HEADER
};
typedef struct _HEADER_MP4_FRM
{
	unsigned long nStat[NUM_ITEM_MP4_FRM_HEADER];
		// Refer to Solo6010 Data Sheet "3-23. MPEG4 Video Encoder Status Registers"
		// Register from 0x0640 (VE_STATUS_0) to 0x066C (VE_STATUS_11)
	unsigned int nDummy[4];
} HEADER_MP4_FRM;
#define GET_CH(hdrFrm)				((hdrFrm).nStat[VE_STATUS_9] &0x1f)
#define GET_MP4_CODE_SZ(hdrFrm)		(((hdrFrm).nStat[VE_STATUS_10] +0x20) &0xfffe0)
#define GET_SEC(hdrFrm)				((hdrFrm).nStat[VE_STATUS_5])
#define GET_USEC(hdrFrm)			((hdrFrm).nStat[VE_STATUS_6])
#define IS_KEY_FRM(hdrFrm)			((((hdrFrm).nStat[VE_STATUS_0] >>22) &0x01) == 0 ? 1 : 0)
#define GET_SCALE(hdrFrm)			(((hdrFrm).nStat[VE_STATUS_1] >>28) &0xf)

#define GET_CH_PT(pHdrFrm)			((pHdrFrm)->nStat[VE_STATUS_9] &0x1f)
#define GET_MP4_CODE_SZ_PT(pHdrFrm)	(((pHdrFrm)->nStat[VE_STATUS_10] +0x20) &0xfffe0)
#define GET_SEC_PT(pHdrFrm)			((pHdrFrm)->nStat[VE_STATUS_5])
#define GET_USEC_PT(pHdrFrm)		((pHdrFrm)->nStat[VE_STATUS_6])
#define IS_KEY_FRM_PT(pHdrFrm)		((((pHdrFrm)->nStat[VE_STATUS_0] >>22) &0x01) == 0 ? 1 : 0)
#define GET_SCALE_PT(pHdrFrm)		(((pHdrFrm)->nStat[VE_STATUS_1] >>28) &0xf)

#define SZ_FRM_FROM_VE_STAT_10(nRegVal)	(sizeof(HEADER_MP4_FRM) +((nRegVal +0x20) &0xfffe0))
//// MPEG4 Record Control[E]

//// JPEG Record Control[S]
#define HEADER_JPEG_FRM				STAT_VIDEO_ENC_ITEM
//// JPEG Record Control[E]
// Record Control[E]


// MPEG4 HW Decode[S]
// Application              : Write MPEG4 encoded frame data to shared memory
// Driver(Buffering Thread) : Modify MPEG4 encoding frame header into decoding frame header and write data to EXT memory
// Driver(Decoder Thread)   : Check "idxDecNeed" and run encoder
enum
{
	VD_IDX_0,
	VD_IDX_1,
	VD_IDX_2,
	VD_IDX_3,
	VD_IDX_4,
	VD_IDX_5,
	NUM_ITEM_MP4_DEC_FRM_HEADER
};
typedef struct _HEADER_MP4_DEC_FRM
{
	unsigned long nStat[NUM_ITEM_MP4_DEC_FRM_HEADER];
		// Refer to Solo6010 Data Sheet "3-28. MPEG4 Video Decoder Control Registers"
		// Register from 0x0930 (VD_IDX_0) to 0x0944 (VD_IDX_5)
	unsigned int nDummy[10];
} HEADER_MP4_DEC_FRM;

#define SZ_MP4_DEC_QUEUE_ITEM	512	// MUST BE SQUARE OF 2

enum
{
	MP4D_IDX_DEC_ONLY,
	MP4D_IDX_DEC_AND_DISP,
	NUM_MP4D_DEC_MODE
};

typedef struct _INFO_MP4_DEC_QUEUE_ITEM
{
	unsigned int posFrm;
	unsigned int szFrm;
	unsigned int idxFrm;
	unsigned int timeMS;
	
	unsigned char idxWnd;
	unsigned char idxDecMode;
	unsigned char bFrameIntp;

	unsigned long bufMP4DecHdr[4];	// VD_IDX_0, VD_IDX_1, VD_IDX_2, VD_IDX_4
} INFO_MP4_DEC_QUEUE_ITEM;

typedef struct _INFO_MP4_DEC_STAT
{
	unsigned char *pBufMP4Dec;
	unsigned int szBufMP4Dec;

	volatile int idxQDecOK;		// Driver Set, Current status of decoder position
	volatile int idxQDecNeed;	// Application Set, The position that decoder need to decode

	volatile int idxQDrvWrite;	// The position that driver write MPEG4 frame data to EXT memory
	volatile int idxQAppWrite;	// The position that application write MPEG4 frame data to shared memory
								// [idxQDecOK : idxQDecNeed : idxQDrvWrite : idxQAppWrite]

	int bufDecPage[NUM_VID_REAL_ENC_CH];
	int bufDispPage[NUM_VID_REAL_ENC_CH];

	unsigned int idxCurFrm;
	unsigned int idxBufferingNextFrm;

	volatile int reqPauseDecoding;

	INFO_MP4_DEC_QUEUE_ITEM iQueue[SZ_MP4_DEC_QUEUE_ITEM];
} INFO_MP4_DEC_STAT;
// MPEG4 HW Decode[E]


// G.723 HW Decode[S]
// 1. Fill G.723 data to buffer from start position.
// 2. Call "S6010_StartG723Dec ()" function. (This function read G.723 data from buffer start postion.)
// 3. Check buffer status and fill buffer in application.
// 4. Goto 3.
//
#define IDX_G723_DEC_CH			17
#define SZ_G723_DEC_QUEUE_ITEM	(8 *32)		// 32 == "NUM_G723_ENC_QUEUE"
#define SZ_G723_DEC_BUFFER		(SZ_G723_DEC_QUEUE_ITEM *SZ_G723_1_ITEM)

typedef struct _INFO_G723_DEC_STAT
{
	unsigned char *pBufG723Dec;
	unsigned int szBufG723Dec;

	volatile int idxQDrvWrite;
	volatile int idxQAppWrite;

	unsigned int idxCurFrm;

	unsigned int idxSamplingRate;
} INFO_G723_DEC_STAT;
// G.723 HW Decode[E]


// Live[S]
enum
{
	LIVE_SCALE_OFF,		// 0
	LIVE_SCALE_1_1,		// 1
	LIVE_SCALE_2_1,		// 2
	LIVE_SCALE_2_2,		// 3
	LIVE_SCALE_3_3,		// 4
	LIVE_SCALE_4_4,		// 5
	LIVE_SCALE_2X3_2X3,	// 6
	LIVE_SCALE_3X4_3X4,	// 7
	NUM_LIVE_SCALE
};
enum
{
	LIVE_SPLIT_MODE_1,	// 1
	LIVE_SPLIT_MODE_4,	// 4
	LIVE_SPLIT_MODE_6,	// L1 +S5
	LIVE_SPLIT_MODE_7,	// L3 +S4
	LIVE_SPLIT_MODE_8,	// L1 +S7
	LIVE_SPLIT_MODE_9,	// 9
	LIVE_SPLIT_MODE_10,	// L2 +S8
	LIVE_SPLIT_MODE_13,	// L1 +S12
	LIVE_SPLIT_MODE_16,	// 16
	NUM_LIVE_SPLIT_MODE
};	// L:Large, S:Small
	// Window position is raster scan order.
#define NUM_LIVE_DISP_WND	16

static const unsigned char GL_NUM_DISP_FROM_DISP_MODE[NUM_LIVE_SPLIT_MODE] = 
{
	1, 4, 6, 7, 8, 9, 10, 13, 16
};
static const unsigned char GL_NUM_DIV_FROM_DISP_MODE[NUM_LIVE_SPLIT_MODE] = 
{
	1, 2, 3, 4, 4, 3, 4, 4, 4
};

static const unsigned char GL_IDX_SCALE_FROM_DISP_MODE_IDX_WND
[NUM_LIVE_SPLIT_MODE][NUM_LIVE_DISP_WND] = 
{
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 1
	3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 4
	6, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 6 (L1 +S5)
	3, 3, 3, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 7 (L3 +S4)
	7, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0,	// 8 (L1 +S7)
	4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0,	// 9
	3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0,	// 10 (L2 +S8)
//	3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0,	// 13 (L1 +S12)	// L1 upper left
	5, 5, 5, 5, 5, 3, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0,	// 13 (L1 +S12)	// L1 center
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,	// 16
};
static const unsigned char GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND
[NUM_LIVE_SPLIT_MODE][NUM_LIVE_DISP_WND] = 
{
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 1
	1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 4
	2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 6 (L1 +S5)
	2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 7 (L3 +S4)
	3, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,	// 8 (L1 +S7)
	1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,	// 9
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,	// 10 (L2 +S8)
//	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,	// 13 (L1 +S12)	// L1 upper left
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,	// 13 (L1 +S12)	// L1 center
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	// 16
};
static const unsigned char GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND
[NUM_LIVE_SPLIT_MODE][NUM_LIVE_DISP_WND] = 
{
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 1
	1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 4
	2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 6 (L1 +S5)
	2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 7 (L3 +S4)
	3, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,	// 8 (L1 +S7)
	1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,	// 9
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,	// 10 (L2 +S8)
//	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,	// 13 (L1 +S12)	// L1 upper left
	1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,	// 13 (L1 +S12)	// L1 center
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	// 16
};
static const unsigned char GL_IDX_POS_FROM_DISP_MODE_IDX_WND
[NUM_LIVE_SPLIT_MODE][NUM_LIVE_DISP_WND] = 
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			// 1
	0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			// 4
	0, 2, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			// 6 (L1 +S5)
	0, 1, 4, 10, 11, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 7 (L3 +S4)
	0, 3, 7, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0,	// 8 (L1 +S7)
	0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0,			// 9
	0, 1, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0,	// 10 (L2 +S8)
//	0, 2, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, 0,	// 13 (L1 +S12)	// L1 upper left
	0, 1, 2, 3, 4, 5, 7, 8, 11, 12, 13, 14, 15, 0, 0, 0,	// 13 (L1 +S12)	// L1 center
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,	// 16
};

typedef struct _INFO_LIVE
{
	unsigned char idxSplitMode;
	unsigned char bFreezeChange;	// Freeze live when mode change
	unsigned char bufIdxCh[NUM_LIVE_DISP_WND];
	unsigned short flagLiveOrDec;	// Is live disp. wnd or MP4 dec wnd? Bit15[Ch15 ... Ch0]Bit0
									// 0:Live, 1:Dec
	unsigned char bufIdxDecCh[NUM_LIVE_DISP_WND];
	unsigned char bufIdxWndFromDecCh[NUM_VID_TOTAL_ENC_CH];	// decoding channel -> window
} INFO_LIVE;
#define INVALID_CH_IDX	0xff
#define INVALID_WND_IDX	0xff

#define MAX_DDRAW_SURF	6
#define SZ_DDRAW_SHR_MEM_FOR_LIVE	(2048 *576 *3)
typedef struct _INFO_DDRAW_SURF
{
	int bField;
	int bFlip;
	int numSurf;
	int nPitch;
	unsigned short szH;
	unsigned short szV;
	PVOID lpSurf[MAX_DDRAW_SURF];

	int bUse2StepCopyMode;		// 1: Copy live data to shared continuous memory (in Drv), 2: Copy shared memory to overlay (in App)
	int bUse1FieldInFrmMode;
} INFO_DDRAW_SURF;

enum
{
	LIVE_BUF_FIELD_EVEN,
	LIVE_BUF_FIELD_ODD,
	LIVE_BUF_STAT_FRAME,
};
typedef struct _CUR_STAT_DDRAW_BUF
{
	volatile int bAccSurf;
	volatile int posDrv;		// Write:Drv, Read:App
	volatile int posApp;		// Write:App, Read:Drv

	int numBuf;
	int bufIdxField[MAX_DDRAW_SURF];
} CUR_STAT_DDRAW_BUF;
// Live[E]


// Color[S]
typedef struct _INFO_COLOR
{
	unsigned char idxCh;			// Channel Index
	unsigned char bAccHue;			// Get or Set Hue?
	unsigned char nHue;				// Hue value
	unsigned char bAccSaturation;	// Get or Set Saturation?
	unsigned char nSaturation;		// Saturation value
	unsigned char bAccContrast;		// Get or Set Contrast?
	unsigned char nContrast;		// Contrast value
	unsigned char bAccBrightness;	// Get or Set Brightness?
	unsigned char nBrightness;		// Brightness value
} INFO_COLOR;
// Color[E]


// Mosaic[S]
#define MIN_MOSAIC_STRENGTH				0
#define MAX_MOSAIC_STRENGTH				15
typedef struct _INFO_MOSAIC
{
	unsigned char bufBSetMosaic[NUM_LIVE_DISP_WND];

	unsigned short bufPosHStart[NUM_LIVE_DISP_WND];
	unsigned short bufPosHEnd[NUM_LIVE_DISP_WND];
	unsigned short bufPosVStart[NUM_LIVE_DISP_WND];
	unsigned short bufPosVEnd[NUM_LIVE_DISP_WND];

	unsigned int nStrength;
} INFO_MOSAIC;
// Mosaic[E]


// Video Motion[S]
#define NUM_V_MOTION_TH_MOST_SENSITIVE	0
#define NUM_V_MOTION_TH_LEAST_SENSITIVE	0xffff
#define NUM_DEF_V_MOTION_TH				0x300
#define NUM_DEF_MIN_DET_CNT				150

#define SZ_BUF_V_MOTION_TH_IN_BYTE		(8 <<10)
#define SZ_BUF_V_MOTION_TH_IN_USHORT	(4 <<10)

#define SZ_BUF_ADV_MOTION_DATA			(SZ_BUF_V_MOTION_TH_IN_BYTE *2 *16)

enum
{
	IDX_VM_GRID_ITEM_BORDER,
	IDX_VM_GRID_ITEM_BAR,
	IDX_VM_GRID_ITEM,
};

enum
{
	IDX_VM_CLR_Y,	// Range of Y is from 0 to 127. Y[0:127]
	IDX_VM_CLR_CB,
	IDX_VM_CLR_CR,
	NUM_VM_CLR
};

enum
{
	IDX_VM_GRID_CLR_CLEAR,
	IDX_VM_GRID_CLR_ADD,
	IDX_VM_GRID_CLR_SET,
	NUM_VM_GRID_CLR
};

// ------------------------------------------------------------------------
// Motion Threshold Change Step
// 1. Set bbufMotionTh[][] to desirable Motion Threshold
// 2. Set bufBSetMotionTh[] to 1
// 3. Call SetVMotionProp function
// 4. Set bufBSetMotionTh[] to 0 (IMPORTANT : for not use S6010 DMA when call SetVMotionProp next time)
// ------------------------------------------------------------------------
typedef struct _INFO_V_MOTION
{
	unsigned char bufBUseMotion[NUM_LIVE_DISP_WND];

	unsigned char bufBSetMotionTh[NUM_LIVE_DISP_WND];
	unsigned short bbufMotionTh[NUM_LIVE_DISP_WND][SZ_BUF_V_MOTION_TH_IN_USHORT];

	unsigned char bSetCntMinDet;
	unsigned short cntMinDet;

	unsigned char bSetShowGrid;
	unsigned char bShowGrid;
	unsigned char bufModeColorSet[IDX_VM_GRID_ITEM][NUM_VM_CLR];	// use IDX_VM_GRID_CLR_XXX
	unsigned char bufColor[IDX_VM_GRID_ITEM][NUM_VM_CLR];

	unsigned char bGetCurMotionMap;
	unsigned char bufCurMotionMap[SZ_BUF_V_MOTION_TH_IN_BYTE];
} INFO_V_MOTION;
// Video Motion[E]


// Camera Block Detection[S]
typedef struct _INFO_CAM_BLK_DETECTION
{
	unsigned char bufBUseDetection[NUM_LIVE_DISP_WND];

	unsigned char bufAlarmLevel[NUM_LIVE_DISP_WND];
	unsigned long timeCheckInterval;

	unsigned char idxCh;
} INFO_CAM_BLK_DETECTION;
// Camera Block Detection[E]


// Advanced Motion Detection[S]
#define MAX_ADV_MOT_CHECK_ITEM			4
#define MAX_MOT_DATA_DISTANCE			16

#define INTERVAL_ADV_MOT_CHECK			500

typedef struct _INFO_ADV_MOT_DETECTION
{
	unsigned char bUseAdvMotion;
	unsigned char bInitOK;
	unsigned char cntNumGetData;

	unsigned char bufBUseMotion[NUM_LIVE_DISP_WND];
	unsigned char bufNumMotionCheck[NUM_LIVE_DISP_WND];

	unsigned char bbufMotDataDist[NUM_LIVE_DISP_WND][MAX_ADV_MOT_CHECK_ITEM];
	unsigned short bbbufMotionTh[NUM_LIVE_DISP_WND][MAX_ADV_MOT_CHECK_ITEM][SZ_BUF_V_MOTION_TH_IN_USHORT];
	unsigned short bbufCntMinDet[NUM_LIVE_DISP_WND];

	unsigned short bbufCurWCData[MAX_MOT_DATA_DISTANCE][SZ_BUF_ADV_MOTION_DATA /2];
	unsigned char curWCDataPos;
} INFO_ADV_MOT_DETECTION;
// Advanced Motion Detection[E]


// Video Matrix[S]
typedef struct _INFO_VIDEO_MATRIX
{
	unsigned char bufIdxCamera[NUM_LIVE_DISP_WND];
} INFO_VIDEO_MATRIX;
// Video Matrix[E]


// Video Loss[S]
#define VLOSS_CH(n)	(1 <<(n))
typedef struct _INFO_VLOSS
{
	unsigned long maskVLoss;
} INFO_VLOSS;
// Video Loss[E]


// GPIO[S]
// ------------------------------------------------------------------------
// GPIO[0]-[15]  : Input, Output, I2C, ETC
// GPIO[16]-[31] : Input, Output
// GPIO[0]-[5] is pre-defined in SoftLogic RDK board. (Video Reset, I2C)
// ------------------------------------------------------------------------
#define GPIO_PORT(x)		(1 <<(x))
#define GPIO_PORT_A			0x0000ffff
#define GPIO_PORT_B			0xffff0000
#define GPIO_ALL_PORT		0xffffffff
#define NUM_GPIO_PORT_A		16	// GPIO[0]-[15]
#define NUM_GPIO_PORT_B		16	// GPIO[16]-[31]
#define NUM_GPIO_ALL_PORT	32

enum
{
	GPIO_OP_SET_MODE_IN,
	GPIO_OP_SET_MODE_OUT,
	GPIO_OP_SET_MODE_I2C,	// for GPIO[0]-[15]
	GPIO_OP_SET_MODE_ETC,	// for GPIO[0]-[15]
	GPIO_OP_WRITE,
	GPIO_OP_READ,
	GPIO_OP_SET_ENABLE_INT,
	GPIO_OP_SET_DISABLE_INT,
	NUM_GPIO_OP
};

enum
{
	GPIO_INT_CFG_RISING_EDGE,
	GPIO_INT_CFG_FALLING_EDGE,
	GPIO_INT_CFG_BOTH_EDGE,
	GPIO_INT_CFG_LOW_LEVEL,
	GPIO_INT_CFG_DISABLE_INTERRUPT,
	NUM_GPIO_INT_CFG
};	// This value is inserted in "nData" variable when "GPIO_OP_SET_INT_PORT" operation.

typedef struct _INFO_GPIO
{
	unsigned long idxOp;
	unsigned long maskPort;
	unsigned long nData;
} INFO_GPIO;

typedef struct _INFO_GPIO_INT_STAT
{
	unsigned long maskIntStat;
} INFO_GPIO_INT_STAT;
// GPIO[E]


// Encoder OSD, Video OSG[S]
typedef struct _INFO_ENCODER_OSD
{
	unsigned char idxCh;
	unsigned char posY;		// 16 pixel resolution
	unsigned char szV;		// 16 pixel resolution

	unsigned char bUseShadowText;
	unsigned char clrY;
	unsigned char clrU;
	unsigned char clrV;

	unsigned char offsetH;
	unsigned char offsetV;

	unsigned long phyAddrBufOSD;
	unsigned long szBuf;
} INFO_ENCODER_OSD;

typedef struct _INFO_VIDEO_OSG
{
	unsigned short posX;
	unsigned short posY;
	unsigned short szH;
	unsigned short szV;

	unsigned long szBuf;
} INFO_VIDEO_OSG;
// Encoder OSD, Video OSG[E]


// UART[S]
enum
{
	UART_PORT_0,
	UART_PORT_1,
	NUM_UART_PORT
};

enum
{
	UART_OP_INIT,
	UART_OP_READ,
	UART_OP_WRITE,
	NUM_UART_OP
};

enum
{
	IDX_UART_BAUD_RATE_300,
	IDX_UART_BAUD_RATE_1200,
	IDX_UART_BAUD_RATE_2400,
	IDX_UART_BAUD_RATE_4800,
	IDX_UART_BAUD_RATE_9600,
	IDX_UART_BAUD_RATE_19200,
	IDX_UART_BAUD_RATE_38400,
	IDX_UART_BAUD_RATE_57600,
	IDX_UART_BAUD_RATE_115200,
	IDX_UART_BAUD_RATE_230400,
	NUM_UART_BAUD_RATE
};

enum
{
	IDX_UART_DATA_SZ_5_BIT,
	IDX_UART_DATA_SZ_6_BIT,
	IDX_UART_DATA_SZ_7_BIT,
	IDX_UART_DATA_SZ_8_BIT,
	NUM_UART_DATA_SZ
};

enum
{
	IDX_UART_STOP_BIT_1,
	IDX_UART_STOP_BIT_2,
	IDX_UART_STOP_BIT_1_5,
	NUM_UART_STOP_BIT
};

enum
{
	IDX_UART_PARITY_NONE,
	IDX_UART_PARITY_EVEN,
	IDX_UART_PARITY_ODD,
	NUM_UART_PARITY
};

#define SZ_UART_DATA_BUF		2048
enum
{
	UART_BUF_RX,
	UART_BUF_TX,
	NUM_UART_BUF
};

typedef struct _INFO_UART_BUF
{
	unsigned char bufData[NUM_UART_BUF][SZ_UART_DATA_BUF];
	int bufPosDrv[NUM_UART_BUF];
	int bufPosApp[NUM_UART_BUF];
} INFO_UART_BUF;

typedef struct _INFO_UART_SETUP
{
	unsigned char idxPort;

	unsigned char idxBaudRate;
	unsigned char idxDataSize;
	unsigned char idxStopBit;
	unsigned char idxParity;

	unsigned char bEnableCTS_RTS;
	unsigned char bEnableDataDropWhenParityError;
} INFO_UART_SETUP;

typedef struct _INFO_UART_RW
{
	unsigned char idxPort;

	int szBufData;
	int szActualRW;
	unsigned char *pBufData;
} INFO_UART_RW;
// UART[E]


// I2C I/O[S]
typedef struct _INFO_I2C_RW
{
	unsigned long nChannel;
	unsigned long nAddrSlave;
	unsigned long nAddrSub;
	unsigned long nData;
} INFO_I2C_RW;
// I2C I/O[E]


// General[S]
typedef struct _INFO_SHARE_DATA
{
	unsigned char *pBufMP4Rec;
	unsigned char *pBufJPEGRec;
	unsigned char *pBufG723Rec;
	unsigned char *pBufMP4Dec;
	unsigned char *pBufG723Dec;
	unsigned char *pBufEncOSD;
	unsigned char *pBufVidOSG;
	unsigned char *pBufCamBlkDet;
	unsigned char *pBufAdvVM;
	unsigned char *pBufLiveImg;

	REC_BUF_STAT iMP4Buf;
	REC_BUF_STAT iJPEGBuf;
	REC_BUF_STAT iG723Buf;
	REC_BUF_STAT iMP4DecBuf;
	REC_BUF_STAT iG723DecBuf;
	unsigned long szBufLiveImg;

	volatile unsigned long statCurVMotion;
	INFO_MP4_DEC_STAT iMP4DecStat;
	INFO_G723_DEC_STAT iG723DecStat;
	CUR_STAT_DDRAW_BUF curStatDDrawBuf;
	INFO_ENCODER_OSD iEncOSD;
	INFO_VIDEO_OSG iVidOSG;
	INFO_GPIO_INT_STAT iGPIO_IntStat;
} INFO_SHARE_DATA;

typedef struct _INFO_S6010_INIT
{
	unsigned char idxS6010;
	unsigned char typeVideo;

	unsigned short szVideoH;
	unsigned short szVideoV;

	unsigned short posVOutH;
	unsigned short posVOutV;
	unsigned short posVInH;
	unsigned short posVInV;

	INFO_SHARE_DATA *pIShare;	// Drv & App Common Buffer
								// Initialized when S6010_IOCTL_OPEN_S6010
								// Destroy when CloseHandle ()

	HANDLE bufHEvt[NUM_COMMON_EVT];
	unsigned char offsetEncOSD;
} INFO_S6010_INIT;
// General[E]

#pragma pack(pop, 4)

#endif
