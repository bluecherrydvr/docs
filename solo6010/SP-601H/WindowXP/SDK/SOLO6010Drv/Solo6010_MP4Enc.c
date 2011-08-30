#include "Solo6010_MP4Enc.h"
#include "Solo6010_P2M.h"


char BUF_QP_SATURATION[MP4E_MAX_QP *3];
char *glPBufQP_Saturation;


void InitCapture (DEVICE_EXTENSION *pdx);
void InitMP4Enc (DEVICE_EXTENSION *pdx);
void InitEncoderOSD (DEVICE_EXTENSION *pdx);
void GetSetEncoderOSD (DEVICE_EXTENSION *pdx, INFO_ENCODER_OSD *pInfoEncOSD, int bGet);

int S6010_InitMP4Enc (DEVICE_EXTENSION *pdx)
{
	int i;

	InitCapture (pdx);
	InitMP4Enc (pdx);
	InitEncoderOSD (pdx);

	for (i=0; i<MP4E_MAX_QP *3; i++)
	{
		if (i < MP4E_MAX_QP +MP4E_MIN_QP)
		{
			BUF_QP_SATURATION[i] = MP4E_MIN_QP;
		}
		else if (i >= MP4E_MAX_QP +MP4E_MAX_QP)
		{
			BUF_QP_SATURATION[i] = MP4E_MAX_QP;
		}
		else
		{
			BUF_QP_SATURATION[i] = i -MP4E_MAX_QP;
		}
	}
	glPBufQP_Saturation = &BUF_QP_SATURATION[MP4E_MAX_QP];

	return 0;
}

void S6010_DestroyMP4Enc (DEVICE_EXTENSION *pdx)
{
	int i;

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, REG_VE_CH_INTL(i), 0);
		S6010_RegWrite (pdx, REG_CAP_CH_SCALE(i), 0);
		S6010_RegWrite (pdx, REG_CAP_CH_INTV(i), 0);

		S6010_RegWrite (pdx, REG_VE_CH_QP(i), 0);
		S6010_RegWrite (pdx, REG_VE_CH_GOP(i), 0);
	}
	for (i=0; i<NUM_VID_VIRT_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, REG_CAP_CH_COMP_ENA_E(i), 0);
		S6010_RegWrite (pdx, REG_CAP_CH_INTV_E(i), 0);

		S6010_RegWrite (pdx, REG_VE_CH_QP_E(i), 0);
		S6010_RegWrite (pdx, REG_VE_CH_GOP_E(i), 0);
	}
}

void S6010_SetInfoMP4JPEGRec (DEVICE_EXTENSION *pdx, INFO_REC *pInfoRec)
{
	int i, nQP, bD1HD1, bCIF, numBW;
	INFO_MP4_REC *pInfoMP4;
	INFO_JPEG_REC *pInfoJPEG;
	float nFPS;
	unsigned long flagQIndex;
	unsigned char bufNumChFromInterval[256];

	memset (bufNumChFromInterval, 0, 256);

	// MPEG Enc[S]
	bD1HD1 = 0;
	bCIF = 0;

	memcpy (&pdx->iRec.iMP4[0], &pInfoRec->iMP4[0], sizeof(INFO_MP4_REC) *NUM_VID_TOTAL_ENC_CH);

	pInfoMP4 = &pdx->iRec.iMP4[NUM_VID_TOTAL_ENC_CH -1];
//	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	for (i=NUM_VID_TOTAL_ENC_CH -1; i>=0; i--)
	{
		if (pInfoMP4->bSet == 1)
		{
			if (pInfoMP4->bUseCBR == 1 && pInfoMP4->nQP < MP4E_MIN_QP)
			{
				nQP = MP4E_MIN_QP;
			}
			else
			{
				nQP = pInfoMP4->nQP;
			}

			if (pInfoMP4->nScale == 1 || pInfoMP4->nScale == 9)
			{
				bD1HD1 = 1;
			}
			if (pInfoMP4->nScale == 2)
			{
				bCIF = 1;
			}

			bufNumChFromInterval[pInfoMP4->nInterval]++;
			
			if (i < NUM_VID_REAL_ENC_CH)	// Real Channel
			{
				S6010_RegWrite (pdx, REG_CAP_CH_INTV(i), pInfoMP4->nInterval);
				S6010_RegWrite (pdx, REG_VE_CH_QP(i), nQP);
				S6010_RegWrite (pdx, REG_VE_CH_GOP(i), 1);
				pdx->bufMP4_GOP[i] = pInfoMP4->nGOP;
				S6010_RegWrite (pdx, REG_VE_CH_INTL(i), (pInfoMP4->bIBasedPred <<1) +((pInfoMP4->nScale >>3) &1));
				S6010_RegWrite (pdx, REG_VE_CH_MOT(i), pInfoMP4->nMotionTh);
				S6010_RegWrite (pdx, REG_CAP_CH_SCALE(i), pInfoMP4->nScale);
			}
			else	// Virtual Channel
			{
				S6010_RegWrite (pdx, REG_CAP_CH_INTV_E((i -NUM_VID_REAL_ENC_CH)), pInfoMP4->nInterval);
				S6010_RegWrite (pdx, REG_VE_CH_QP_E((i -NUM_VID_REAL_ENC_CH)), nQP);
				S6010_RegWrite (pdx, REG_VE_CH_GOP_E((i -NUM_VID_REAL_ENC_CH)), 1);
				pdx->bufMP4_GOP[i] = pInfoMP4->nGOP;
				S6010_RegWrite (pdx, REG_CAP_CH_COMP_ENA_E((i -NUM_VID_REAL_ENC_CH)), 1);
			}
			
			if (pInfoMP4->bUseCBR == 1)
			{
				if (pInfoMP4->nScale < 8)
				{
					if (pInfoMP4->nInterval == 0)
					{
						nFPS = 60;
					}
					else
					{
						nFPS = (float)30. /pInfoMP4->nInterval;
					}
				}
				else
				{
					nFPS = (float)30. /(pInfoMP4->nInterval +1);
				}

				pdx->infoMP4_CBR[i].bUseCBR = 1;
				pdx->infoMP4_CBR[i].orgSzVirtualDecBuf = pInfoMP4->szVirtualDecBuf;
				pdx->infoMP4_CBR[i].curSzVirtualDecBuf = pInfoMP4->szVirtualDecBuf /2;
				pdx->infoMP4_CBR[i].nCurQP = pInfoMP4->nQP;
				
				pdx->infoMP4_CBR[i].bufSzFrmMean[0] = (unsigned int)(pInfoMP4->nBitrate *pInfoMP4->nGOP *2 /nFPS /(pInfoMP4->nGOP +1));
				pdx->infoMP4_CBR[i].bufSzFrmMean[1] = pdx->infoMP4_CBR[i].bufSzFrmMean[0] /2;
				pdx->infoMP4_CBR[i].bufSzFrmMean[2] = pdx->infoMP4_CBR[i].bufSzFrmMean[1];
			}
			else
			{
				pdx->infoMP4_CBR[i].bUseCBR = 0;
			}
		}
		else
		{
			pdx->bufMP4_GOP[i] = 0;
			if (i < NUM_VID_REAL_ENC_CH)	// Real Channel
			{
				S6010_RegWrite (pdx, REG_CAP_CH_SCALE(i), 0);
			}
			else	// Virtual Channel
			{
				S6010_RegWrite (pdx, REG_CAP_CH_COMP_ENA_E((i -NUM_VID_REAL_ENC_CH)), 0);
			}
		}

		pInfoMP4--;
	}
	// MPEG Enc[E]

	// JPEG Enc[S]
	memcpy (&pdx->iRec.iJPEG[0], &pInfoRec->iJPEG[0], sizeof(INFO_JPEG_REC) *NUM_VID_TOTAL_ENC_CH);

	pInfoJPEG = &pdx->iRec.iJPEG[0];
	pdx->iS6010.maskEncJPEG = 0;
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		pdx->iS6010.maskEncJPEG |= (pInfoJPEG->bRec <<i);
		pInfoJPEG++;
	}

	flagQIndex = 0;
	for (i=NUM_VID_REAL_ENC_CH -1; i>=0; i--)
	{
		flagQIndex <<= 2;
		flagQIndex |= (pdx->iRec.iJPEG[i].idxQP) &0x03;
	}
	S6010_RegWrite (pdx, REG_VE_JPEG_QP_CH_L, flagQIndex);

	flagQIndex = 0;
	for (i=NUM_VID_VIRT_ENC_CH -1; i>=0; i--)
	{
		flagQIndex <<= 2;
		flagQIndex |= (pdx->iRec.iJPEG[NUM_VID_REAL_ENC_CH +i].idxQP) &0x03;
	}
	S6010_RegWrite (pdx, REG_VE_JPEG_QP_CH_H, flagQIndex);
	// JPEG Enc[E]

	// Set Capture Max Bandwidth[S]
	numBW = 32;
	
	if ((bD1HD1 &bCIF) == 1)
	{
		numBW = 0;
		if (bufNumChFromInterval[0] != 0)
		{
			bufNumChFromInterval[1] += bufNumChFromInterval[0];
			bufNumChFromInterval[0] = 0;
		}
		for (i=1; i<256; i++)
		{
			if (bufNumChFromInterval[i] != 0)
			{
				numBW += ((bufNumChFromInterval[i] +i -1) /i) *4;
			}
		}
		numBW++;

		if (numBW < 32)
		{
			numBW = 32;
		}
	}

	S6010_RegWrite (pdx, REG_CAP_BTW, CAP_PROG_BANDWIDTH(2) |CAP_MAX_BANDWIDTH(numBW));
	KdPrint((DBG_NAME " CAP_MAX_BW: %d\n", numBW));
	// Set Capture Max Bandwidth[E]
}

void S6010_StartMP4JPEGEnc (DEVICE_EXTENSION *pdx)
{
	// Shared Buffer Offset[S]
	pdx->pIShare->iMP4Buf.addrStart = 0;
	pdx->pIShare->iMP4Buf.addrEnd = 0;

	pdx->pIShare->iJPEGBuf.addrStart = 0;
	pdx->pIShare->iJPEGBuf.addrEnd = 0;
	// Shared Buffer Offset[E]

	pdx->statVidEnc.idxLastQ = S6010_RegRead (pdx, REG_VE_STATE(11));
	pdx->statVidEnc.posIndexHW = 0;
	pdx->statVidEnc.posIndexHW_Minus1 = 0;
	pdx->statVidEnc.posIndexDrv = 0;

	S6010_EnableInterrupt (pdx, INTR_ENCODER);
}

void S6010_EndMP4JPEGEnc (DEVICE_EXTENSION *pdx)
{
	int i;
	
	S6010_DisableInterrupt (pdx, INTR_ENCODER);

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, REG_VE_CH_GOP(i), 1);
		S6010_RegWrite (pdx, REG_VE_CH_GOP_E(i), 1);
	}

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, REG_CAP_CH_SCALE(i), 0);
		S6010_RegWrite (pdx, REG_CAP_CH_COMP_ENA_E(i), 0);
	}
}

void S6010_ChangeInfoMP4JPEGRec (DEVICE_EXTENSION *pdx, INFO_REC *pInfoRec)
{
	int i, nQP;
	INFO_MP4_REC *pOldInfoMP4, *pCurInfoMP4;
	float nFPS;

	// MPEG Enc[S]
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		pCurInfoMP4 = &pInfoRec->iMP4[i];
		if (pCurInfoMP4->bSet >= MP4E_ADD_VAL_FOR_CHANGE_REC_SETTINGS)
		{
			pOldInfoMP4 = &pdx->iRec.iMP4[i];

			pCurInfoMP4->bSet -= MP4E_ADD_VAL_FOR_CHANGE_REC_SETTINGS;

			if (pCurInfoMP4->bSet == 1)
			{
				if (pCurInfoMP4->bUseCBR == 1 && pCurInfoMP4->nQP < MP4E_MIN_QP)
				{
					nQP = MP4E_MIN_QP;
				}
				else
				{
					nQP = pCurInfoMP4->nQP;
				}
				
				if (i < NUM_VID_REAL_ENC_CH)	// Real Channel
				{
					if (pOldInfoMP4->bSet == 1 && pOldInfoMP4->nScale != pCurInfoMP4->nScale)
					{
						S6010_RegWrite (pdx, REG_CAP_CH_SCALE(i), 0);
						pOldInfoMP4->bSet = 0;

						S6010_Sleep_1MS ();
					}

					S6010_RegWrite (pdx, REG_CAP_CH_INTV(i), pCurInfoMP4->nInterval);
					S6010_RegWrite (pdx, REG_VE_CH_QP(i), nQP);
					S6010_RegWrite (pdx, REG_VE_CH_GOP(i), 1);
					pdx->bufMP4_GOP[i] = pCurInfoMP4->nGOP;
					S6010_RegWrite (pdx, REG_VE_CH_MOT(i), pCurInfoMP4->nMotionTh);

					if (pOldInfoMP4->bSet == 0)
					{
						S6010_RegWrite (pdx, REG_VE_CH_INTL(i), (pCurInfoMP4->bIBasedPred <<1) +((pCurInfoMP4->nScale >>3) &1));
						S6010_RegWrite (pdx, REG_CAP_CH_SCALE(i), pCurInfoMP4->nScale);
					}
				}
				else	// Virtual Channel
				{
					S6010_RegWrite (pdx, REG_CAP_CH_INTV_E((i -NUM_VID_REAL_ENC_CH)), pCurInfoMP4->nInterval);
					S6010_RegWrite (pdx, REG_VE_CH_QP_E((i -NUM_VID_REAL_ENC_CH)), nQP);
					S6010_RegWrite (pdx, REG_VE_CH_GOP_E((i -NUM_VID_REAL_ENC_CH)), 1);
					pdx->bufMP4_GOP[i] = pCurInfoMP4->nGOP;
					S6010_RegWrite (pdx, REG_CAP_CH_COMP_ENA_E((i -NUM_VID_REAL_ENC_CH)), 1);
				}
				
				if (pCurInfoMP4->bUseCBR == 1)
				{
					if (pCurInfoMP4->nScale < 8)
					{
						if (pCurInfoMP4->nInterval == 0)
						{
							nFPS = 60;
						}
						else
						{
							nFPS = (float)30. /pCurInfoMP4->nInterval;
						}
					}
					else
					{
						nFPS = (float)30. /(pCurInfoMP4->nInterval +1);
					}

					pdx->infoMP4_CBR[i].bUseCBR = 1;
					pdx->infoMP4_CBR[i].orgSzVirtualDecBuf = pCurInfoMP4->szVirtualDecBuf;
					pdx->infoMP4_CBR[i].curSzVirtualDecBuf = pCurInfoMP4->szVirtualDecBuf /2;
					pdx->infoMP4_CBR[i].nCurQP = pCurInfoMP4->nQP;
					
					pdx->infoMP4_CBR[i].bufSzFrmMean[0] = (unsigned int)(pCurInfoMP4->nBitrate *pCurInfoMP4->nGOP *2 /nFPS /(pCurInfoMP4->nGOP +1));
					pdx->infoMP4_CBR[i].bufSzFrmMean[1] = pdx->infoMP4_CBR[i].bufSzFrmMean[0] /2;
					pdx->infoMP4_CBR[i].bufSzFrmMean[2] = pdx->infoMP4_CBR[i].bufSzFrmMean[1];
				}
				else
				{
					pdx->infoMP4_CBR[i].bUseCBR = 0;
				}
			}
			else
			{
				pdx->bufMP4_GOP[i] = 0;
				if (i < NUM_VID_REAL_ENC_CH)	// Real Channel
				{
					S6010_RegWrite (pdx, REG_CAP_CH_SCALE(i), 0);
				}
				else	// Virtual Channel
				{
					S6010_RegWrite (pdx, REG_CAP_CH_COMP_ENA_E((i -NUM_VID_REAL_ENC_CH)), 0);
				}
			}

			memcpy (&pdx->iRec.iMP4[i], &pInfoRec->iMP4[i], sizeof(INFO_MP4_REC));
		}
	}
	// MPEG Enc[E]
}

void S6010_SetEncoderOSD_Prop (DEVICE_EXTENSION *pdx, INFO_ENCODER_OSD *pInfoEncOSD)
{
	if (pInfoEncOSD->bUseShadowText == 1)
	{
		S6010_RegWrite (pdx, REG_VE_OSD_OPT, (1 <<15) |(1 <<14) |((pInfoEncOSD->offsetH &0x7f) <<7) |(pInfoEncOSD->offsetV &0x7f));
	}
	else
	{
		S6010_RegWrite (pdx, REG_VE_OSD_OPT, ((pInfoEncOSD->offsetH &0x7f) <<7) |(pInfoEncOSD->offsetV &0x7f));
	}
	S6010_RegWrite (pdx, REG_VE_OSD_CLR, (pInfoEncOSD->clrY <<16) |(pInfoEncOSD->clrU <<8) |pInfoEncOSD->clrV);
}

void S6010_GetEncoderOSD (DEVICE_EXTENSION *pdx, INFO_ENCODER_OSD *pInfoEncOSD)
{
	GetSetEncoderOSD (pdx, pInfoEncOSD, 1);
}

void S6010_SetEncoderOSD (DEVICE_EXTENSION *pdx, INFO_ENCODER_OSD *pInfoEncOSD)
{
	GetSetEncoderOSD (pdx, pInfoEncOSD, 0);
}

#define VI_PROG_HSIZE		(1280 -16)
#define VI_PROG_VSIZE		(1024 -16)
#define COMP_DCT_INTV		9
#define COMP_MAX_BANDWIDTH	32		// D1 4channel (D1 1channel == 4)
void InitCapture (DEVICE_EXTENSION *pdx)
{
	unsigned long height;
	unsigned long width;

	S6010_RegWrite (pdx, REG_CAP_BASE,
		CAP_MAX_PAGE((MEM_COMP_WR_TOTAL -MEM_COMP_WR_SIZE) >>16) |CAP_BASE_ADDR((VMEM_COMP_WR_BASE >>16) &0xffff));
	S6010_RegWrite (pdx, REG_CAP_BTW,
		CAP_PROG_BANDWIDTH(2) |CAP_MAX_BANDWIDTH(COMP_MAX_BANDWIDTH));

	// set scale 1, 9 dimension
	width = pdx->iS6010.szVideoH;
	height = pdx->iS6010.szVideoV;
	S6010_RegWrite (pdx, REG_DIM_SCALE1,
		DIM_H_MB_NUM(width /16) | DIM_V_MB_NUM_FRAME(height /8) | DIM_V_MB_NUM_FIELD(height /16));

	// set scale 2, 10 dimension
	width = pdx->iS6010.szVideoH /2;
	height = pdx->iS6010.szVideoV /1;
	S6010_RegWrite (pdx, REG_DIM_SCALE2,
		DIM_H_MB_NUM(width /16) | DIM_V_MB_NUM_FRAME(height /8) |DIM_V_MB_NUM_FIELD(height /16));

	// set scale 3, 11 dimension
	width = pdx->iS6010.szVideoH /2;
	height = pdx->iS6010.szVideoV /2;
	S6010_RegWrite (pdx, REG_DIM_SCALE3,
		DIM_H_MB_NUM(width /16) | DIM_V_MB_NUM_FRAME(height /8) |DIM_V_MB_NUM_FIELD(height /16));

	// set scale 4, 12 dimension
	width = pdx->iS6010.szVideoH /3;
	height = pdx->iS6010.szVideoV /3;
	S6010_RegWrite (pdx, REG_DIM_SCALE4,
		DIM_H_MB_NUM(width /16) | DIM_V_MB_NUM_FRAME(height /8) |DIM_V_MB_NUM_FIELD(height /16));

	// set scale 5, 13 dimension
	width = pdx->iS6010.szVideoH /4;
	height = pdx->iS6010.szVideoV /2;
	S6010_RegWrite (pdx, REG_DIM_SCALE5,
		DIM_H_MB_NUM(width /16) | DIM_V_MB_NUM_FRAME(height /8) |DIM_V_MB_NUM_FIELD(height /16));

	// progressive
	width = VI_PROG_HSIZE;
	height = VI_PROG_VSIZE;
	S6010_RegWrite (pdx, REG_DIM_PROG,
		DIM_H_MB_NUM(width /16) | DIM_V_MB_NUM_FRAME(height /16 *1) |DIM_V_MB_NUM_FIELD(height /16));
}

void InitMP4Enc (DEVICE_EXTENSION *pdx)
{
	int i;

	// Set code base address
	S6010_RegWrite (pdx, REG_VE_CFG0,
		VE_INTR_CTRL(0) |
		VE_BLOCK_SIZE(VMEM_MPEG_CODE_SIZE >>16) |
		VE_BLOCK_BASE(VMEM_MPEG_CODE_BASE >>16));

	S6010_RegWrite (pdx, REG_VE_CFG1,
		VE_INSERT_INDEX |
		VE_MOTION_MODE(0) |
		VE_BYTE_ALIGN(2));

	S6010_RegWrite (pdx, REG_VE_WMRK_POLY, 0);
	S6010_RegWrite (pdx, REG_VE_WMRK_INIT_KEY, 0);
	S6010_RegWrite (pdx, REG_VE_WMRK_STRL, 0);
	S6010_RegWrite (pdx, REG_VE_ENCRYP_POLY, 0);
	S6010_RegWrite (pdx, REG_VE_ENCRYP_INIT, 0);

	S6010_RegWrite (pdx, REG_VE_ATTR,
		VE_LITTLE_ENDIAN |
		COMP_ATTR_FCODE(1) |
		COMP_TIME_INC(0) |
		COMP_TIME_WIDTH(15) |
		DCT_INTERVAL(COMP_DCT_INTV));

	// Set reference base address[S]
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, REG_VE_CH_REF_BASE(i), (MEM_ENC_REF_BASE +(i *MEM_ENC_REF_SIZE)) >>16);
	}

	for (i=0; i<NUM_VID_VIRT_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, REG_VE_CH_REF_BASE_E(i), (MEM_EXT_ENC_REF_BASE +(i *MEM_EXT_ENC_REF_SIZE)) >>16);
	}
	// Set reference base address[E]

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, REG_VE_CH_MOT(i), 0);
	}

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, REG_VE_CH_GOP(i), 1);
		S6010_RegWrite (pdx, REG_VE_CH_GOP_E(i), 1);

		S6010_RegWrite (pdx, REG_CAP_CH_SCALE(i), 0);
		S6010_RegWrite (pdx, REG_CAP_CH_COMP_ENA_E(i), 0);
	}

	// JPEG[S]
	S6010_RegWrite (pdx, REG_VE_JPEG_QP_TBL, (2 <<24) |(2 <<16) |(2 <<8) |(2 <<0));	// QP index fix (All Low)
	S6010_RegWrite (pdx, REG_VE_JPEG_QP_CH_L, 0);
	S6010_RegWrite (pdx, REG_VE_JPEG_QP_CH_H, 0);
	S6010_RegWrite (pdx, REG_VE_JPEG_CFG, ((VMEM_JPEG_CODE_SIZE >>16) <<16) |(VMEM_JPEG_CODE_BASE >>16));
	// JPEG[E]
}

void InitEncoderOSD (DEVICE_EXTENSION *pdx)
{
	int i;
	
	S6010_RegWrite (pdx, REG_VE_OSD_CH, 0xffffffff);	// Enable encoder OSD
//	S6010_RegWrite (pdx, REG_VE_OSD_CH, 0);				// Disable encoder OSD

	S6010_RegWrite (pdx, REG_VE_OSD_BASE, VMEM_COMP_OSD_BASE >>16);
	S6010_RegWrite (pdx, REG_VE_OSD_CLR, (0xf0 <<16) |(0x80 <<8) |0x80);
//	S6010_RegWrite (pdx, REG_VE_OSD_OPT, (1 <<15) |(1 <<14));
	S6010_RegWrite (pdx, REG_VE_OSD_OPT, 0);

	memset (pdx->bufInfoP2M[P2M_BUF_ENC_OSD].bufP2M, 0, 0x10000);
	for (i=0; i<MEM_COMP_OSD_SIZE; i+=0x10000)
	{
		S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE, pdx->bufInfoP2M[P2M_BUF_ENC_OSD].nPhyAddr.LowPart,
			VMEM_COMP_OSD_BASE +i, 0x10000, P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
	}
}

void GetSetEncoderOSD (DEVICE_EXTENSION *pdx, INFO_ENCODER_OSD *pInfoEncOSD, int bGet)
{
	int idxOp, szCopy;

	if (bGet == 1)
	{
		idxOp = P2M_READ;
	}
	else
	{
		idxOp = P2M_WRITE;
	}

	szCopy = 2048 *pInfoEncOSD->szV;
	if (szCopy <= 0)
	{
		return;
	}
	if (szCopy > 0x10000)
	{
		szCopy = 0x10000;
	}
	S6010_P2M_Copy_Sync (pdx, P2M_CH_3, idxOp, pInfoEncOSD->phyAddrBufOSD,
		VMEM_COMP_OSD_BASE +0x10000 *pInfoEncOSD->idxCh +2048 *pInfoEncOSD->posY, szCopy,
		P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
}

