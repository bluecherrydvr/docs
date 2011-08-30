#include "Solo6010_G723.h"
#include "Solo6010_I2C.h"
#include "Solo6010_P2M.h"


#define G723_INTR_ORDER		0
#define G723_FDMA_PAGES		32

#define G723_MAX_ITEM		120

#define OUTMODE_MASK		0x300
#define G723_DEC_CHANNEL	17

#define SAMPLERATE			8000
#define BITRATE				25


static void SetSamplingRateTW2815 (DEVICE_EXTENSION *pdx, int idxSamplingRate);


int S6010_InitG723Coder (DEVICE_EXTENSION *pdx)
{
	int clk_div;

	clk_div = (CLOCK_INPUT *1000000) /(SAMPLERATE *(BITRATE *2) *2);

	S6010_RegWrite (pdx, REG_AUDIO_SAMPLE,
		AUDIO_BITRATE(BITRATE) |
		AUDIO_CLK_DIV(clk_div));

	S6010_RegWrite (pdx, REG_AUDIO_FDMA_INTR,
		AUDIO_FDMA_INTERVAL(1) |
		AUDIO_INTR_ORDER(G723_INTR_ORDER) |
		AUDIO_FDMA_BASE((VMEM_AUDIO_BASE >>16) &0xffff));

	S6010_RegWrite (pdx, REG_AUDIO_CONTROL,
		AUDIO_ENABLE |
		AUDIO_I2S_MODE |
		AUDIO_I2S_MULTI(3) |
		AUDIO_MODE(OUTMODE_MASK));

	return 0;
}

void S6010_DestroyG723Coder (DEVICE_EXTENSION *pdx)
{
	S6010_EndG723Enc (pdx);
	S6010_EndG723Dec (pdx);

	S6010_RegWrite (pdx, REG_AUDIO_CONTROL, 0);
}

void S6010_SetInfoG723Rec (DEVICE_EXTENSION *pdx, INFO_REC *pInfoRec)
{
	SetSamplingRateTW2815 (pdx, pInfoRec->iG723.idxSampRate);
}

void S6010_StartG723Enc (DEVICE_EXTENSION *pdx)
{
	if (pdx->iS6010.statG723InQueue.bEncOn == 1)
	{
		return;
	}

	pdx->pIShare->iG723Buf.addrStart = 0;
	pdx->pIShare->iG723Buf.addrEnd = 0;

	pdx->iS6010.statG723InQueue.posEncDrv = S6010_RegRead (pdx, REG_AUDIO_STA) &0x1f;
	pdx->iS6010.statG723InQueue.bEncOn = 1;

	if (pdx->iS6010.statG723InQueue.bDecOn == 0)
	{
		S6010_EnableInterrupt (pdx, INTR_G723);
	}
}

void S6010_EndG723Enc (DEVICE_EXTENSION *pdx)
{
	if (pdx->iS6010.statG723InQueue.bEncOn == 0)
	{
		return;
	}

	pdx->iS6010.statG723InQueue.bEncOn = 0;

	if (pdx->iS6010.statG723InQueue.bDecOn == 0)
	{
		S6010_DisableInterrupt (pdx, INTR_G723);
	}
}

void S6010_OpenG723Dec (DEVICE_EXTENSION *pdx, int idxSamplingRate)
{
	pdx->pIShare->iG723DecStat.pBufG723Dec = pdx->pIShare->pBufG723Dec;
	pdx->pIShare->iG723DecStat.szBufG723Dec = pdx->pIShare->iG723DecBuf.szBuf;

	pdx->pIShare->iG723DecStat.idxQDrvWrite = 0;
	pdx->pIShare->iG723DecStat.idxQAppWrite = 0;

	if (pdx->iS6010.statG723InQueue.bEncOn == 0)
	{
		SetSamplingRateTW2815 (pdx, idxSamplingRate);
	}
}

void S6010_StartG723Dec (DEVICE_EXTENSION *pdx)
{
	int curQPos;

	if (pdx->iS6010.statG723InQueue.bDecOn == 1)
	{
		return;
	}

	#define NUM_START_OFFSET	1

#if __NUM_TW2815_CHIPS == 1
	S6010_I2C_Write (pdx, 0, TW2815_A_IIC_ID, 0x71, 0x10);
#endif
#if __NUM_TW2815_CHIPS == 2
	S6010_I2C_Write (pdx, 0, TW2815_B_IIC_ID, 0x71, 0x10);
#endif
#if __NUM_TW2815_CHIPS == 3
	S6010_I2C_Write (pdx, 0, TW2815_C_IIC_ID, 0x71, 0x10);
#endif
#if __NUM_TW2815_CHIPS == 4
	S6010_I2C_Write (pdx, 0, TW2815_D_IIC_ID, 0x71, 0x10);
#endif

	curQPos = S6010_RegRead (pdx, REG_AUDIO_STA) &0x1f;

	// CAUTION: This function reads G.723 data from buffer start position.
	if (curQPos <= 1)
	{
		S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE,
			pdx->bufInfoP2M[P2M_BUF_G723_DEC].nPhyAddr.LowPart +0,
			VMEM_AUDIO_BASE +SZ_G723_ENC_QUEUE +((0 +NUM_START_OFFSET) *SZ_G723_1_ITEM),
			SZ_G723_ENC_QUEUE -((0 +NUM_START_OFFSET +1) *SZ_G723_1_ITEM),
			P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
	}
	else
	{
		S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE,
			pdx->bufInfoP2M[P2M_BUF_G723_DEC].nPhyAddr.LowPart +0,
			VMEM_AUDIO_BASE +SZ_G723_ENC_QUEUE +((curQPos +NUM_START_OFFSET) *SZ_G723_1_ITEM),
			SZ_G723_ENC_QUEUE -((curQPos +NUM_START_OFFSET) *SZ_G723_1_ITEM),
			P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
		
		S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE,
			pdx->bufInfoP2M[P2M_BUF_G723_DEC].nPhyAddr.LowPart +SZ_G723_ENC_QUEUE -((curQPos +NUM_START_OFFSET) *SZ_G723_1_ITEM),
			VMEM_AUDIO_BASE +SZ_G723_ENC_QUEUE +0,
			(curQPos -1) *SZ_G723_1_ITEM,
			P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
	}

	pdx->pIShare->iG723DecStat.idxQDrvWrite += NUM_G723_ENC_QUEUE -NUM_START_OFFSET -1;	// Because "idxQDrvWrite" is 0.

	pdx->iS6010.statG723InQueue.posDecDrv = (curQPos +NUM_G723_ENC_QUEUE -1) %NUM_G723_ENC_QUEUE;
	pdx->iS6010.statG723InQueue.bDecOn = 1;

	if (pdx->iS6010.statG723InQueue.bEncOn == 0)
	{
		S6010_EnableInterrupt (pdx, INTR_G723);
	}
}

void S6010_EndG723Dec (DEVICE_EXTENSION *pdx)
{
	if (pdx->iS6010.statG723InQueue.bDecOn == 0)
	{
		return;
	}

#if __NUM_TW2815_CHIPS == 1
	S6010_I2C_Write (pdx, 0, TW2815_A_IIC_ID, 0x71, 0x11);
#endif
#if __NUM_TW2815_CHIPS == 2
	S6010_I2C_Write (pdx, 0, TW2815_B_IIC_ID, 0x71, 0x11);
#endif
#if __NUM_TW2815_CHIPS == 3
	S6010_I2C_Write (pdx, 0, TW2815_C_IIC_ID, 0x71, 0x11);
#endif
#if __NUM_TW2815_CHIPS == 4
	S6010_I2C_Write (pdx, 0, TW2815_D_IIC_ID, 0x71, 0x11);
#endif
	
	memset (pdx->bufInfoP2M[P2M_BUF_G723_DEC].bufP2M, 0, SZ_G723_ENC_QUEUE);
	S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE, pdx->bufInfoP2M[P2M_BUF_G723_DEC].nPhyAddr.LowPart,
		VMEM_AUDIO_BASE +SZ_G723_ENC_QUEUE, SZ_G723_ENC_QUEUE, P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);

	pdx->iS6010.statG723InQueue.bDecOn = 0;

	if (pdx->iS6010.statG723InQueue.bEncOn == 0)
	{
		S6010_DisableInterrupt (pdx, INTR_G723);
	}
}

void SetSamplingRateTW2815 (DEVICE_EXTENSION *pdx, int idxSamplingRate)
{
	int nCurVal;

	if (idxSamplingRate == G723_SAMPLING_RATE_8K)
	{
		nCurVal = S6010_I2C_Read (pdx, 0, TW2815_A_IIC_ID, TW2815_A_IIC_ID +1, 0x62) &~(1 <<2);
		S6010_I2C_Write (pdx, 0, TW2815_A_IIC_ID, 0x62, nCurVal);

		nCurVal = S6010_I2C_Read (pdx, 0, TW2815_B_IIC_ID, TW2815_B_IIC_ID +1, 0x62) &~(1 <<2);
		S6010_I2C_Write (pdx, 0, TW2815_B_IIC_ID, 0x62, nCurVal);

		nCurVal = S6010_I2C_Read (pdx, 0, TW2815_C_IIC_ID, TW2815_C_IIC_ID +1, 0x62) &~(1 <<2);
		S6010_I2C_Write (pdx, 0, TW2815_C_IIC_ID, 0x62, nCurVal);

		nCurVal = S6010_I2C_Read (pdx, 0, TW2815_D_IIC_ID, TW2815_D_IIC_ID +1, 0x62) &~(1 <<2);
		S6010_I2C_Write (pdx, 0, TW2815_D_IIC_ID, 0x62, nCurVal);
	}
	else if (idxSamplingRate == G723_SAMPLING_RATE_16K)
	{
		nCurVal = S6010_I2C_Read (pdx, 0, TW2815_A_IIC_ID, TW2815_A_IIC_ID +1, 0x62) |(1 <<2);
		S6010_I2C_Write (pdx, 0, TW2815_A_IIC_ID, 0x62, nCurVal);

		nCurVal = S6010_I2C_Read (pdx, 0, TW2815_B_IIC_ID, TW2815_B_IIC_ID +1, 0x62) |(1 <<2);
		S6010_I2C_Write (pdx, 0, TW2815_B_IIC_ID, 0x62, nCurVal);

		nCurVal = S6010_I2C_Read (pdx, 0, TW2815_C_IIC_ID, TW2815_C_IIC_ID +1, 0x62) |(1 <<2);
		S6010_I2C_Write (pdx, 0, TW2815_C_IIC_ID, 0x62, nCurVal);

		nCurVal = S6010_I2C_Read (pdx, 0, TW2815_D_IIC_ID, TW2815_D_IIC_ID +1, 0x62) |(1 <<2);
		S6010_I2C_Write (pdx, 0, TW2815_D_IIC_ID, 0x62, nCurVal);
	}
}

