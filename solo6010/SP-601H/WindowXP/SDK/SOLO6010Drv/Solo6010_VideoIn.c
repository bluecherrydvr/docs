#include "Solo6010_VideoIn.h"
#include "Solo6010_P2M.h"


#define PROGRESSIVE_HSIZE		1280
#define PROGRESSIVE_VSIZE		1024


static int InitVideoInConfig (DEVICE_EXTENSION *pdx);
static int InitVideoInMotion (DEVICE_EXTENSION *pdx);


int S6010_InitVideoIn (DEVICE_EXTENSION *pdx)
{
	InitVideoInConfig (pdx);
	InitVideoInMotion (pdx);

	return 0;
}

static int InitVideoInConfig (DEVICE_EXTENSION *pdx)
{
	S6010_RegWrite (pdx, REG_VI_CH_ENA, 0xffff);
	S6010_RegWrite (pdx, REG_VI_CH_FORMAT, VI_FD_SEL_MASK(0) |VI_PROG_MASK(0));
//	S6010_RegWrite (pdx, REG_VI_FMT_CFG, VI_FMT_CHECK_HCOUNT |VI_FMT_CHECK_VCOUNT);
	S6010_RegWrite (pdx, REG_VI_FMT_CFG, VI_FMT_CHECK_HCOUNT);

	S6010_RegWrite (pdx, REG_VI_ACT_I_P,
		//(1<<30)	|
		VI_H_START(pdx->iS6010.posVInH) |
		VI_V_START(pdx->iS6010.posVInV) |
		VI_V_STOP(pdx->iS6010.posVInV +pdx->iS6010.szVideoV));

	S6010_RegWrite (pdx, REG_VI_ACT_I_S,
		//(1<<30)	|
		VI_H_START(pdx->iS6010.posVOutH) |
		VI_V_START(pdx->iS6010.posVOutV) |
		VI_V_STOP(pdx->iS6010.posVOutV +pdx->iS6010.szVideoV));

	S6010_RegWrite (pdx, REG_VI_ACT_P,
		VI_H_START(0) |
		VI_V_START(1) |
		VI_V_STOP(PROGRESSIVE_VSIZE));

	// PB live data reader
	if(pdx->iS6010.typeVideo == VIDEO_TYPE_NTSC)
	{
		S6010_RegWrite (pdx, VI_PB_CONFIG, VI_PB_USER_MODE);
		S6010_RegWrite (pdx, VI_PB_RANGE_HV, VI_PB_HSIZE(858) |VI_PB_VSIZE(246));
		S6010_RegWrite (pdx, VI_PB_ACT_H, VI_PB_HSTART(16) |VI_PB_HSTOP(16 +720));
		S6010_RegWrite (pdx, VI_PB_ACT_V, VI_PB_VSTART(4) |VI_PB_VSTOP(4 +240));
	}
	else
	{
		S6010_RegWrite (pdx, VI_PB_CONFIG, VI_PB_USER_MODE |VI_PB_PAL);
		S6010_RegWrite (pdx, VI_PB_RANGE_HV, VI_PB_HSIZE(864) |VI_PB_VSIZE(294));
		S6010_RegWrite (pdx, VI_PB_ACT_H, VI_PB_HSTART(16) |VI_PB_HSTOP(16 +720));
		S6010_RegWrite (pdx, VI_PB_ACT_V, VI_PB_VSTART(4) |VI_PB_VSTOP(4 +288));
	}

	S6010_RegWrite (pdx, REG_VI_PAGE_SW,
							 (0<<16)		//	FI for sync
							|(0<<15)		//	cal in FI inv
							|(0<<14)		//	cal in FI inv
							|(0<<13)		//	cal out in FI inv
							|(0<<11)		//	1,1			//	I,O
							|(0<<9)			//	1,0
							|(0<<7)			//	0,1
							|(0<<5)			//	0,0
							|(0<<3)			//	pip add
							|(2<<0)			//	normal add
							);

	// input channel remapping
	S6010_RegWrite (pdx, REG_VI_CH_SWITCH_0,  (5<<25) |  (4<<20) |  (3<<15) |  (2<<10) |  (1<<5) |  (0<<0));
	S6010_RegWrite (pdx, REG_VI_CH_SWITCH_1, (11<<25) | (10<<20) |  (9<<15) |  (8<<10) |  (7<<5) |  (6<<0));
	S6010_RegWrite (pdx, REG_VI_CH_SWITCH_2,            (16<<20) | (15<<15) | (14<<10) | (13<<5) | (12<<0));

	return 0;
}

static int InitVideoInMotion (DEVICE_EXTENSION *pdx)
{
	int i;

	memset (pdx->iS6010.bufMotionTh, 0, sizeof(unsigned short) *SZ_BUF_V_MOTION_TH_IN_USHORT);

	// Clear motion flag area [S]
	S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE, pdx->iS6010.phyAddrBufMotionTh.LowPart,
		VMEM_MOTION_BASE, SZ_BUF_V_MOTION_TH_IN_BYTE, P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
	S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE, pdx->iS6010.phyAddrBufMotionTh.LowPart,
		VMEM_MOTION_BASE +SZ_BUF_V_MOTION_TH_IN_BYTE, SZ_BUF_V_MOTION_TH_IN_BYTE, P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
	// Clear motion flag area [E]

	// Clear working cache [S]
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE, pdx->iS6010.phyAddrBufMotionTh.LowPart,
			VMEM_MOTION_BASE +SZ_BUF_V_MOTION_TH_IN_BYTE *2 +(i *(SZ_BUF_V_MOTION_TH_IN_BYTE *2)) +SZ_BUF_V_MOTION_TH_IN_BYTE, SZ_BUF_V_MOTION_TH_IN_BYTE,
			P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
	}
	// Clear working cache [E]

	for(i=0; i<SZ_BUF_V_MOTION_TH_IN_USHORT; i++)
	{
		pdx->iS6010.bufMotionTh[i] = NUM_DEF_V_MOTION_TH;
	}

	// Set default threshold table [S]
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE, pdx->iS6010.phyAddrBufMotionTh.LowPart,
			VMEM_MOTION_BASE +SZ_BUF_V_MOTION_TH_IN_BYTE *2 +(i *(SZ_BUF_V_MOTION_TH_IN_BYTE *2)), SZ_BUF_V_MOTION_TH_IN_BYTE,
			P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
	}
	// Set default threshold table [E]

	S6010_RegWrite (pdx, REG_VI_MOT_ADR, VI_MOTION_EN(0xffff) |(VMEM_MOTION_BASE >>16));
	S6010_RegWrite (pdx, REG_VI_MOT_CTRL, ((pdx->iS6010.szVideoH /16) <<16) |NUM_DEF_MIN_DET_CNT);

	S6010_RegWrite (pdx, REG_VI_MOTION_BORDER, 0);
	S6010_RegWrite (pdx, REG_VI_MOTION_BAR, 0);

	return 0;
}

int S6010_SetInfoMosaic (DEVICE_EXTENSION *pdx, INFO_MOSAIC *pInfoMosaic)
{
	int i;
	unsigned long nTmp;

	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		if (pInfoMosaic->bufBSetMosaic[i] == 1)
		{
			S6010_RegWrite (pdx, REG_VI_MOSAIC(i),
				VI_MOSAIC_SX(pInfoMosaic->bufPosHStart[i] /8) |
				VI_MOSAIC_EX((pInfoMosaic->bufPosHEnd[i] +7) /8) |
				VI_MOSAIC_SY(pInfoMosaic->bufPosVStart[i] /8) |
				VI_MOSAIC_EY((pInfoMosaic->bufPosVEnd[i] +7) /8));
		}
		else
		{
			S6010_RegWrite (pdx, REG_VI_MOSAIC(i), 0);
		}
	}

	nTmp = S6010_RegRead (pdx, REG_VI_FMT_CFG);
	nTmp = (nTmp &(~(0xf <<22))) |(pInfoMosaic->nStrength <<22);
	S6010_RegWrite (pdx, REG_VI_FMT_CFG, nTmp);

	return 0;
}

int S6010_SetInfoVMotion (DEVICE_EXTENSION *pdx, INFO_V_MOTION *pInfoVMotion)
{
	int i, nTmp;

	nTmp = 0;
	for (i=NUM_LIVE_DISP_WND -1; i>=0; i--)
	{
		nTmp = (nTmp <<1) |pInfoVMotion->bufBUseMotion[i];
	}

	S6010_RegWrite (pdx, REG_VI_MOT_ADR, VI_MOTION_EN(nTmp) |(VMEM_MOTION_BASE >>16));

	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		if (pInfoVMotion->bufBUseMotion[i] == 1 &&
			pInfoVMotion->bufBSetMotionTh[i] == 1)
		{
			memcpy (pdx->iS6010.bufMotionTh, pInfoVMotion->bbufMotionTh[i], SZ_BUF_V_MOTION_TH_IN_BYTE);

			S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE, pdx->iS6010.phyAddrBufMotionTh.LowPart,
				VMEM_MOTION_BASE +SZ_BUF_V_MOTION_TH_IN_BYTE *2 +(i *(SZ_BUF_V_MOTION_TH_IN_BYTE *2)), SZ_BUF_V_MOTION_TH_IN_BYTE,
				P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
		}
	}

	if (pInfoVMotion->bSetCntMinDet == 1)
	{
		S6010_RegWrite (pdx, REG_VI_MOT_CTRL,
			((pdx->iS6010.szVideoH /16) <<16) |pInfoVMotion->cntMinDet);
	}

	if (pInfoVMotion->bSetShowGrid == 1)
	{
		if (pInfoVMotion->bShowGrid == 1)
		{
			S6010_RegWrite (pdx, REG_VI_MOTION_BORDER,
					((pInfoVMotion->bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y] &0x3) <<28) |
					((pInfoVMotion->bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB] &0x3) <<26) |
					((pInfoVMotion->bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR] &0x3) <<24) |
					(((pInfoVMotion->bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y] >>1) &0xff) <<16) |
					((pInfoVMotion->bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB] &0xff) <<8) |
					((pInfoVMotion->bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR] &0xff) <<0));

			S6010_RegWrite (pdx, REG_VI_MOTION_BAR,
					((pInfoVMotion->bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y] &0x3) <<28) |
					((pInfoVMotion->bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB] &0x3) <<26) |
					((pInfoVMotion->bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR] &0x3) <<24) |
					(((pInfoVMotion->bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y] >>1) &0xff) <<16) |
					((pInfoVMotion->bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB] &0xff) <<8) |
					((pInfoVMotion->bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR] &0xff) <<0));
		}
		else
		{
			S6010_RegWrite (pdx, REG_VI_MOTION_BORDER, 0);
			S6010_RegWrite (pdx, REG_VI_MOTION_BAR, 0);
		}
	}

	if (pInfoVMotion->bGetCurMotionMap == 1)
	{
		S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_READ, pdx->iS6010.phyAddrBufMotionTh.LowPart,
			VMEM_MOTION_BASE, SZ_BUF_V_MOTION_TH_IN_BYTE, P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);

		memcpy (pInfoVMotion->bufCurMotionMap, pdx->iS6010.bufMotionTh, SZ_BUF_V_MOTION_TH_IN_BYTE);
	}

	if (nTmp != 0)
	{
		S6010_EnableInterrupt (pdx, INTR_MOTION);
	}

	return 0;
}

int S6010_SetInfoCamToCh (DEVICE_EXTENSION *pdx, INFO_VIDEO_MATRIX *pInfoVMatrix)
{
	S6010_RegWrite (pdx, REG_VI_CH_SWITCH_0,
		((pInfoVMatrix->bufIdxCamera[0] &0xf) <<0) |
		((pInfoVMatrix->bufIdxCamera[1] &0xf) <<5) |
		((pInfoVMatrix->bufIdxCamera[2] &0xf) <<10) |
		((pInfoVMatrix->bufIdxCamera[3] &0xf) <<15) |
		((pInfoVMatrix->bufIdxCamera[4] &0xf) <<20) |
		((pInfoVMatrix->bufIdxCamera[5] &0xf) <<25));
	S6010_RegWrite (pdx, REG_VI_CH_SWITCH_1,
		((pInfoVMatrix->bufIdxCamera[6] &0xf) <<0) |
		((pInfoVMatrix->bufIdxCamera[7] &0xf) <<5) |
		((pInfoVMatrix->bufIdxCamera[8] &0xf) <<10) |
		((pInfoVMatrix->bufIdxCamera[9] &0xf) <<15) |
		((pInfoVMatrix->bufIdxCamera[10] &0xf) <<20) |
		((pInfoVMatrix->bufIdxCamera[11] &0xf) <<25));
	S6010_RegWrite (pdx, REG_VI_CH_SWITCH_2,
		((pInfoVMatrix->bufIdxCamera[12] &0xf) <<0) |
		((pInfoVMatrix->bufIdxCamera[13] &0xf) <<5) |
		((pInfoVMatrix->bufIdxCamera[14] &0xf) <<10) |
		((pInfoVMatrix->bufIdxCamera[15] &0xf) <<15) |
		(16 <<20));

	return 0;
}

int S6010_GetCamBlkDetState (DEVICE_EXTENSION *pdx, INFO_CAM_BLK_DETECTION *pInfoCamBlkDet)
{
	S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_READ, pdx->bufInfoP2M[P2M_BUF_CAM_BLK_DET].nPhyAddr.LowPart,
		VMEM_MOTION_BASE +SZ_BUF_V_MOTION_TH_IN_BYTE *2 +(pInfoCamBlkDet->idxCh *(SZ_BUF_V_MOTION_TH_IN_BYTE *2)) +SZ_BUF_V_MOTION_TH_IN_BYTE,
		SZ_BUF_V_MOTION_TH_IN_BYTE, P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);

	return 0;
}
int S6010_StartAdvMotionDataGet (DEVICE_EXTENSION *pdx)
{
	pdx->iS6010.bGetAdvMotionData = 1;

	return 0;
}

int S6010_EndAdvMotionDataGet (DEVICE_EXTENSION *pdx)
{
	pdx->iS6010.bGetAdvMotionData = 0;

	return 0;
}

