#include "Solo6010_MP4Dec.h"


int S6010_InitMP4Dec (DEVICE_EXTENSION *pdx)
{
	S6010_RegWrite (pdx, REG_VD_CFG0, 
		VD_CFG_BUSY_WAIT_CODE |VD_CFG_BUSY_WAIT_REF |VD_CFG_BUSY_WAIT_RES |VD_CFG_BUSY_WAIT_MS
		|VD_CFG_USER_PAGE_CTRL |VD_CFG_SINGLE_MODE
		|VD_CFG_LITTLE_ENDIAN |VD_CFG_ERR_LOCK |VD_CFG_ERR_INT_ENA |VD_CFG_TIME_WIDTH(15) |VD_CFG_DCT_INTERVAL(18));

	S6010_RegWrite (pdx, REG_VD_DEINTERLACE,
		VD_DEINTERLACE_THRESHOLD(100) |VD_DEINTERLACE_EDGE_VALUE(5));

	S6010_RegWrite (pdx, REG_VD_CFG1, MEM_DEC_TO_LIVE >>16);

	return 0;
}

void S6010_DestroyMP4Dec (DEVICE_EXTENSION *pdx)
{
	S6010_CloseMP4Dec (pdx);
}

void S6010_OpenMP4Dec (DEVICE_EXTENSION *pdx)
{
	memset (&pdx->pIShare->iMP4DecStat, 0, sizeof(INFO_MP4_DEC_STAT));

	pdx->pIShare->iMP4DecStat.pBufMP4Dec = pdx->pIShare->pBufMP4Dec;
	pdx->pIShare->iMP4DecStat.szBufMP4Dec = pdx->pIShare->iMP4DecBuf.szBuf;

	S6010_EnableInterrupt (pdx, INTR_DECODER);
	S6010_RegWrite (pdx, REG_VD_CTRL, VD_OPER_ON);

	KeSetEvent (&pdx->bufEvent[IDX_EVT_TH_MP4_DEC_FILL_BUF], EVENT_INCREMENT, FALSE);
}

void S6010_CloseMP4Dec (DEVICE_EXTENSION *pdx)
{
	KeClearEvent (&pdx->bufEvent[IDX_EVT_TH_MP4_DEC_FILL_BUF]);
	
	S6010_RegWrite (pdx, REG_VD_CTRL, 0);
	S6010_DisableInterrupt (pdx, INTR_DECODER);
}
