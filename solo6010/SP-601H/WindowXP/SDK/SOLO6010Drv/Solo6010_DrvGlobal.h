#ifndef __SOLO6010_DRV_GLOBAL_H__
#define __SOLO6010_DRV_GLOBAL_H__

#include "TypesDef.h"
#include "Solo6010_Global.h"
#include "solo6010_register.h"


// Global Definition for Driver[S]
#define __NUM_SOLO_CHIPS	1
	// WARNING : THIS IS VERY IMPORTANT PARAMETER. THIS PARAMETER MUST BE SET CORRECTLY.
	//
	// The number of installed SOLO6010 in your PC (entire system).
	// 
	// If the number of installed SOLO6010 in your board is 2 and the number of your board installed in your PC is 1, then __NUM_SOLO_CHIPS is 2.
	// If the number of installed SOLO6010 in your board is 2 and the number of your board installed in your PC is 2, then __NUM_SOLO_CHIPS is 4.

#define __NUM_TW2815_CHIPS	4
	// The number of TW2815 video decoder connected to 1 SOLO6010 chip.

#define __USE_SAA7128__
	// whether use SAA7128 video encoder or not

//#define __RESET_SAA7128__
	// If this statement is defined, then the reset pin of SAA7128 is always low.

#define __USE_ONLY_1_P2M_CHANNEL__
	// Use only 1 P2M(DMA) channel for P2M(DMA) operation.
	// Using 1 P2M channel is more stable than using 4 P2M channel, but slower.

//#define __ONLY_FIRST_SOLO_CTRL_I2C__
	// When multiple SOLO6010 is installed in your board and only first SOLO6010 I2C pin is enabled define this statement.

//#define __INSERT_P2M_TEST__
	// Execute P2M test always. This is for debugging. You must not define this statement at release version.

//#define __USE_SINGLE_DISP_PAGE__
	// If you have problem in live preview on PC monitor while recording, try to define this statement.

//#define __NOTIFY_EVENT_ON_EVERY_FRM_MP4__
	// The driver notifies MPEG4 record event to application on every encoded frame.

//#define __NOTIFY_EVENT_ON_EVERY_FRM_JPEG__
	// The driver notifies JPEG record event to application on every encoded frame.

//#define __INSERT_SDRAM_CHECK__
	// Insert the SDRAM availability check function.

//#define __PRINT_CRITICAL_LOG_MSG__
	// Print critical log message that can be received by DbgView.exe or WinDbg.exe.
// Global Definition for Driver[E]


#define SZ_SOLO6010_SDRAM			0x08000000						// 128MB

// SOLO6010 SDRAM Map[S] (Full function of SOLO6010)
#define VMEM_DISP_WR_BASE			0x00000000
#define 	MEM_DISP_SIZE			0x00480000						// 0x00120000 *4 page

#define VMEM_OSG_BASE				(VMEM_DISP_WR_BASE +MEM_DISP_SIZE)
#define		MEM_OSG_SIZE			0x00120000						// PAL size

#define VMEM_COMP_OSD_BASE			(VMEM_OSG_BASE +MEM_OSG_SIZE)
#define		MEM_COMP_OSD_SIZE		0x00200000						// 64KB *Num of Total Video Channel

#define VMEM_MOTION_BASE			(VMEM_COMP_OSD_BASE +MEM_COMP_OSD_SIZE)
#define		MEM_MOTION_SIZE			0x00080000						// 512KB

#define	VMEM_AUDIO_BASE				(VMEM_MOTION_BASE +MEM_MOTION_SIZE)
#define		MEM_AUDIO_SIZE			0x00010000

#define VMEM_COMP_WR_BASE			(VMEM_AUDIO_BASE +MEM_AUDIO_SIZE)
#define		MEM_COMP_WR_SIZE		0x00120000						// 18 *64KB = 1,152KB
#define		MEM_COMP_WR_TOTAL		(MEM_COMP_WR_SIZE *(16 +1))		// Num of Real Video Channel +1

#define MEM_ENC_REF_BASE			(VMEM_COMP_WR_BASE +MEM_COMP_WR_TOTAL)
#define		MEM_ENC_REF_SIZE		0x00140000						// 20 *64KB = 1,280KB
#define		MEM_ENC_REF_TOTAL		(MEM_ENC_REF_SIZE *16)			// Num of Real Video Channel

#define MEM_EXT_ENC_REF_BASE		(MEM_ENC_REF_BASE +MEM_ENC_REF_TOTAL)
#define		MEM_EXT_ENC_REF_SIZE	0x00140000						// 20 *64KB = 1,280KB
#define		MEM_EXT_ENC_REF_TOTAL	(MEM_EXT_ENC_REF_SIZE *16)		// Num of Real Video Channel

#define MEM_DEC_REF_BASE			(MEM_EXT_ENC_REF_BASE +MEM_EXT_ENC_REF_TOTAL)
#define		MEM_DEC_REF_SIZE		0x00140000						// 20 *64KB = 1,280KB
#define		MEM_DEC_REF_TOTAL		(MEM_DEC_REF_SIZE *16)			// Num of Real Video Channel

#define MEM_DEC_TO_LIVE				(MEM_DEC_REF_BASE +MEM_DEC_REF_TOTAL)
#define		MEM_DEC_TO_LIVE_SIZE	0x00240000						// 0x00120000 *2

#define VMEM_MPEG_CODE_BASE			(MEM_DEC_TO_LIVE +MEM_DEC_TO_LIVE_SIZE)
#define		VMEM_MPEG_CODE_SIZE		0x00800000
#define VMEM_JPEG_CODE_BASE			(VMEM_MPEG_CODE_BASE +VMEM_MPEG_CODE_SIZE)
#define		VMEM_JPEG_CODE_SIZE		0x00800000
#define VMEM_DECODE_BASE			(VMEM_JPEG_CODE_BASE +VMEM_JPEG_CODE_SIZE)
#define		VMEM_DECODE_CODE_SIZE	0x00800000
#define VMEM_TOTAL_USED				(VMEM_DECODE_BASE +VMEM_DECODE_CODE_SIZE)
// SOLO6010 SDRAM Map[E] (Full function of SOLO6010)
// Use "MemoryMapMaker.exe" to get other SDRAM settings.


// Main[S]
#define CLOCK_INPUT					104
// Main[E]


enum
{
	IDX_THREAD_VID_ENC_DATA_READ,
	IDX_THREAD_G723_ENC_DATA_READ,
	IDX_THREAD_LIVE_REFRESH,
	IDX_THREAD_MP4_DEC_FILL_BUF,	// Fill MP4 Dec Buffer
	IDX_THREAD_TEST,
	IDX_THREAD_UART_0,
	IDX_THREAD_UART_1,
	NUM_THREAD
};

enum
{
	IDX_EVT_P2M_0,
	IDX_EVT_P2M_1,
	IDX_EVT_P2M_2,
	IDX_EVT_P2M_3,

	IDX_EVT_IIC,

	IDX_EVT_TH_VID_ENC_DATA_READ,
	IDX_EVT_TH_G723_ENC_READ_DEC_WRITE,
	IDX_EVT_TH_LIVE_REFRESH,
	IDX_EVT_TH_MP4_DEC_FILL_BUF,
	IDX_EVT_TH_UART_0,
	IDX_EVT_TH_UART_1,
	NUM_EVENT
};

enum
{
	IDX_DPC_MP4_ENC,
	IDX_DPC_MP4_DEC,
	IDX_DPC_G723_ENC_DEC,
	IDX_DPC_LIVE_REFRESH,
	IDX_DPC_V_MOTION,
	IDX_DPC_P2M_0,
	IDX_DPC_P2M_1,
	IDX_DPC_P2M_2,
	IDX_DPC_P2M_3,
	IDX_DPC_I2C,
	IDX_DPC_GPIO_INT,
	IDX_DPC_UART_0,
	IDX_DPC_UART_1,
	IDX_DPC_PCI_ERROR,
	NUM_DPC
};

enum
{
	FMUX_DISPATCH,
	FMUX_IO_I2C,
	FMUX_INFO_LIVE,
	FMUX_UART_0,
	FMUX_UART_1,
	FMUX_P2M_IO,
	NUM_FAST_MUTEX
};

enum
{
	K_SPLOCK_REG_IO,
	NUM_K_SPLOCK
};


// Video In/Out[S]
#define TW2815_A_IIC_ID			0x50
#define TW2815_A_IIC_CHANNEL	0
#define TW2815_B_IIC_ID			0x52
#define TW2815_B_IIC_CHANNEL	0
#define TW2815_C_IIC_ID			0x54
#define TW2815_C_IIC_CHANNEL	0
#define TW2815_D_IIC_ID			0x56
#define TW2815_D_IIC_CHANNEL	0

#define SAA7128_IIC_ID			0x8c
#define SAA7128_IIC_CHANNEL		1
// Video In/Out[E]


// MPEG4 & JPEG Enc[S]
#define NUM_MP4_ENC_QUEUE		16
#define NUM_JPEG_ENC_QUEUE		16

#define OSD_BUF_SIZE			0x10000
#define SDRAM_OVERLAP_MARGINE	(1 *1024 *1024)
// MPEG4 & JPEG Enc[E]


// G723 Enc[S]
#define SZ_G723_COMMON_BUF		(512 *SZ_G723_1_ITEM)
#define NUM_G723_ENC_QUEUE		32
#define SZ_G723_ENC_QUEUE		(NUM_G723_ENC_QUEUE *SZ_G723_1_ITEM)
// G723 Enc[E]


// CBR[S]
enum
{
	CUR_FRM_SZ_NORMAL,
	CUR_FRM_SZ_BIG,
	CUR_FRM_SZ_SMALL,
	NUM_FRM_SZ
};

static const char BUF_RATE_TO_QP_DIFF[NUM_FRM_SZ][30] = 
{
	-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,   -4, -3, -2, -1,  0,  0, 1, 2, 3, 4,    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,   -4, -3, -2, -1,  1,  1, 1, 2, 3, 4,    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,   -4, -3, -2, -1, -1, -1, 1, 2, 3, 4,    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
};
extern char BUF_QP_SATURATION[MP4E_MAX_QP *3];
extern char *glPBufQP_Saturation;

typedef struct _INFO_CBR
{
	unsigned char bUseCBR;
	unsigned int bufSzFrmMean[3];
	int curSzVirtualDecBuf;
	int orgSzVirtualDecBuf;
	char nCurQP;
} INFO_CBR;
// CBR[E]


// P2M[S]
enum
{
	P2M_CH_0,
	P2M_CH_1,
	P2M_CH_2,
	P2M_CH_3,
	NUM_P2M
};

enum
{
	P2M_BUF_MP4_ENC,
	P2M_BUF_JPEG_ENC,
	P2M_BUF_G723_ENC,
	P2M_BUF_MP4_DEC,
	P2M_BUF_G723_DEC,
	P2M_BUF_ENC_OSD,
	P2M_BUF_VID_OSG,

	P2M_BUF_CAM_BLK_DET,
	P2M_BUF_ADV_MOTION,
	P2M_BUF_SHR_MEM_FOR_LIVE,
	P2M_BUF_TEMP,
	P2M_BUF_TEST,
	NUM_P2M_BUF
};
#ifdef __INSERT_P2M_TEST__
#define SZ_P2M_BUF_TEST			(512 *1024)
#else
#define SZ_P2M_BUF_TEST			PAGE_SIZE
#endif

typedef struct _INFO_P2M_BUF
{
	U8 *bufP2M;
	U32 szBufP2M;
	PHYSICAL_ADDRESS nPhyAddr;
	PMDL pMDL;
	PVOID pLockedBuf;
} INFO_P2M_BUF;

typedef struct _STAT_P2M
{
	volatile int bErrStop;
} STAT_P2M;
// P2M[E]


// G723 Queue Status[S]
typedef struct _STAT_G723_IN_QUEUE
{
	volatile U32 posSOLO6010;

	volatile U32 posEncDrv;
	volatile U32 posDecDrv;

	U32 bEncOn;
	U32 bDecOn;

	U32 bufTime[NUM_G723_ENC_QUEUE][2];		// 0:Sec, 1:USec
} STAT_G723_IN_QUEUE;
// G723 Queue Status[E]


// SOLO6010 Info[S]
typedef struct _INFO_S6010
{
	U32 idxID;
#if __NUM_SOLO_CHIPS > 1
	U32 idxID_RegIO;
#endif
	
	U8 bOpen;
	U8 bInit;
	U8 bLockBuffer;
	U8 typeVideo;

	U16 szVideoH;
	U16 szVideoV;

	U16 posVInH;
	U16 posVInV;

	U16 posVOutH;
	U16 posVOutV;

	ULONG curStatGPIO_Out;

	PVOID pLockedInfoShareData;

	PKEVENT bufPKEvt[NUM_COMMON_EVT];

	U32 maskEncJPEG;

	// V-Motion[S]
	U16 *bufMotionTh;
	PHYSICAL_ADDRESS phyAddrBufMotionTh;
	// V-Motion[E]

	// G723 recording[S]
	STAT_G723_IN_QUEUE statG723InQueue;
	// G723 recording[E]

	// Adv Motion[S]
	U8 bGetAdvMotionData;
	// Adv Motion[E]
} INFO_S6010;
// SOLO6010 Info[E]


// DDraw Info[S]
typedef struct _INFO_DDRAW
{
	volatile int bLiveUpdateOn;
	int bField;
	int bFlip;
	int numFSkip;

	PMDL bufPMdl[MAX_DDRAW_SURF];
	PHYSICAL_ADDRESS bufPhyAddr[MAX_DDRAW_SURF];

	unsigned long sz1LineCopy;
	unsigned long nPitch;
	unsigned long szH;
	unsigned long szV;

	unsigned long szLiveExt1Pic;
	int bUse1FieldInFrmMode;
} INFO_DDRAW;
// DDraw Info[E]


#if __NUM_SOLO_CHIPS > 1
//Global variables for preventing PCI error[S]
typedef struct _INFO_GL_ABOUT_REG_IO
{
	U32 bufIdxOldAccS6010[NUM_MAX_S6010];
	PHYSICAL_ADDRESS bufAddrPhyBaseAddr[NUM_MAX_S6010];
	KSPIN_LOCK bufKSPLockRegIO[NUM_MAX_S6010];

	U32 bufAddrOtherBaseReg[NUM_MAX_S6010];

	U32 numTmpForRegAcc;
} INFO_GL_ABOUT_REG_IO;

extern int glNumS6010;
extern INFO_GL_ABOUT_REG_IO glIAboutRegIO;
//Global variables for preventing PCI error[E]
#endif


#endif