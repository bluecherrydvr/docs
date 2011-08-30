#include "DriverDefs.h"
#include "Interrupts.h"
#include "SupportFunc.h"

#include "Solo6010.h"


BOOLEAN OnInterrupt (PKINTERRUPT pInterrupt, PVOID pParam)
{
	U32 nIntrStat, nTmp;
	U32 idxCurQ, idxNextQ, idxTmpQ, regValCurQ_MP4, regValTmpQ_MP4, regValNextQ_MP4, regValCurQ_JPEG, regValTmpQ_JPEG, regValNextQ_JPEG;
	int idxCh, typeVOP, posMP4, szMP4, posJPEG, szJPEG, szMP4_Cur, bCurFrm;
	int szFrmMean, idxSzFrm, idxBufLevel;
	STAT_VIDEO_ENC_ITEM *pVideoEncIndex;
	DEVICE_EXTENSION *pdx;

	pdx = (DEVICE_EXTENSION *)pParam;

	S6010_RegRead_ISR(REG_INTR_STAT, nIntrStat);

	if (nIntrStat &INTR_ENCODER)
	{
		S6010_RegRead_ISR(REG_VE_STATE(11), idxCurQ);		// Get current queue position

		S6010_RegRead_ISR(REG_VE_MPEG4_QUE(idxCurQ), regValCurQ_MP4);
		S6010_RegRead_ISR(REG_VE_JPEG_QUE(idxCurQ), regValCurQ_JPEG);
		
		S6010_RegRead_ISR(REG_VE_STATE(10), szMP4_Cur);
		szMP4_Cur = SZ_FRM_FROM_VE_STAT_10(szMP4_Cur);		// Get MPEG4 code size

		idxTmpQ = pdx->statVidEnc.idxLastQ;
		regValNextQ_MP4 = 0xffffffff;

V_ISR_LOOP_START:

		if (idxTmpQ == idxCurQ)
		{
			goto END_MP4_ISR;
		}

		idxTmpQ = (idxTmpQ +1) &0xf;
		idxNextQ = (idxTmpQ +1) &0xf;

		if (idxTmpQ == idxCurQ)	// Current frame
		{
			regValTmpQ_MP4 = regValCurQ_MP4;
			regValTmpQ_JPEG = regValCurQ_JPEG;

			bCurFrm = 1;
		}
		else if (regValNextQ_MP4 == 0xffffffff)	// Missed frame, 1st
		{
			S6010_RegRead_ISR(REG_VE_MPEG4_QUE(idxTmpQ), regValTmpQ_MP4);
			S6010_RegRead_ISR(REG_VE_JPEG_QUE(idxTmpQ), regValTmpQ_JPEG);

			bCurFrm = 0;
		}
		else	// Missed frame
		{
			regValTmpQ_MP4 = regValNextQ_MP4;
			regValTmpQ_JPEG = regValNextQ_JPEG;

			bCurFrm = 0;
		}

		S6010_RegRead_ISR(REG_VE_MPEG4_QUE(idxNextQ), regValNextQ_MP4);
		S6010_RegRead_ISR(REG_VE_JPEG_QUE(idxNextQ), regValNextQ_JPEG);

		
		idxCh = (U8)((regValTmpQ_MP4 >>24) &0x1f);			// Get channel
		typeVOP = (U8)((regValTmpQ_MP4 >>29) &0x03);		// Get VOP type

		posMP4 = (regValTmpQ_MP4 &0x00ffffff);
		szMP4 = ((regValNextQ_MP4 &0x00ffffff) +VMEM_MPEG_CODE_SIZE -posMP4) %VMEM_MPEG_CODE_SIZE;

		posJPEG = (regValTmpQ_JPEG &0x00ffffff);
		szJPEG = ((regValNextQ_JPEG &0x00ffffff) +VMEM_JPEG_CODE_SIZE -posJPEG) %VMEM_JPEG_CODE_SIZE;

		// MPEG4 GOP reset[S]
		if (pdx->bufMP4_GOP[idxCh] != 0)
		{
			if (typeVOP == 0x00)
			{
				// set real GOP[S]
				S6010_RegWrite_ISR(REG_VE_CH_GOP(idxCh), pdx->bufMP4_GOP[idxCh]);
					// REG_VE_CH_GOP() macro can be used in "Real Channel" and Virtual Channel" both.
					// Because Real Channel GOP address is from 0x0780, and Virtual Channel GOP address is from 0x07C0.
				// set real GOP[E]

				pdx->bufMP4_GOP[idxCh] = 0;
			}
			else
			{
				typeVOP = 2;
				goto END_CBR_SETTING;
			}
		}
		// MPEG4 GOP reset[E]

		if (posMP4 +szMP4 > VMEM_MPEG_CODE_SIZE)
		{
			if (bCurFrm != 1 || szMP4 != szMP4_Cur)	// MPEG4 EXT. BUFFER OVERLAP BUG OCCURED
			{
				// GOP Reset[S]
				if (pdx->bufMP4_GOP[idxCh] == 0)
				{
					pdx->bufMP4_GOP[idxCh] = pdx->iRec.iMP4[idxCh].nGOP;
					S6010_RegWrite_ISR(REG_VE_CH_GOP(idxCh), 1);
						// REG_VE_CH_GOP() macro can be used in "Real Channel" and Virtual Channel" both.
						// Because Real Channel GOP address is from 0x0780, and Virtual Channel GOP address is from 0x07C0.
				}
				// GOP Reset[E]

				typeVOP = 2;
				goto END_CBR_SETTING;
			}
		}

		// CBR Settings[S]
		if (pdx->infoMP4_CBR[idxCh].bUseCBR == 1)	// CBR
		{
			// Subtract "mean of frame size" from "cur virtual decoder buffer size"[S]
			szFrmMean = pdx->infoMP4_CBR[idxCh].bufSzFrmMean[typeVOP];
			pdx->infoMP4_CBR[idxCh].curSzVirtualDecBuf -= szFrmMean;
			// Subtract "mean of frame size" from "cur virtual decoder buffer size"[E]

			// Virtual Dec Buffer Underflow Control[S]
			if (pdx->infoMP4_CBR[idxCh].curSzVirtualDecBuf < -pdx->infoMP4_CBR[idxCh].orgSzVirtualDecBuf)
			{
				pdx->infoMP4_CBR[idxCh].curSzVirtualDecBuf = -pdx->infoMP4_CBR[idxCh].orgSzVirtualDecBuf;
			}
			// Virtual Dec Buffer Underflow Control[E]
			
			pdx->infoMP4_CBR[idxCh].curSzVirtualDecBuf += szMP4;	// Add "encoded frame size"
			
			// Virtual Dec Buffer Overflow Control[S]
			if (pdx->infoMP4_CBR[idxCh].curSzVirtualDecBuf >= 2 *pdx->infoMP4_CBR[idxCh].orgSzVirtualDecBuf)
			{
				pdx->infoMP4_CBR[idxCh].curSzVirtualDecBuf = 2 *pdx->infoMP4_CBR[idxCh].orgSzVirtualDecBuf -1;
			}
			// Virtual Dec Buffer Overflow Control[E]

			// Cur Frame Sz Check (Bigger than x4 -> QP -= 1, Smaller than /4 -> QP += 1) [S]
			if (szMP4 > (szFrmMean <<2))
			{
				idxSzFrm = CUR_FRM_SZ_BIG;
			}
			else if (szMP4 < (szFrmMean >>2))
			{
				idxSzFrm = CUR_FRM_SZ_SMALL;
			}
			else
			{
				idxSzFrm = CUR_FRM_SZ_NORMAL;
			}
			// Cur Frame Sz Check (Bigger than x4 -> QP -= 1, Smaller than /4 -> QP += 1) [E]
			
			idxBufLevel = (pdx->infoMP4_CBR[idxCh].curSzVirtualDecBuf *10 /pdx->infoMP4_CBR[idxCh].orgSzVirtualDecBuf) +10;

			if (BUF_RATE_TO_QP_DIFF[idxSzFrm][idxBufLevel] == 0)
			{
				goto END_CBR_SETTING;
			}
			pdx->infoMP4_CBR[idxCh].nCurQP = glPBufQP_Saturation[pdx->infoMP4_CBR[idxCh].nCurQP +BUF_RATE_TO_QP_DIFF[idxSzFrm][idxBufLevel]];

			S6010_RegWrite_ISR(REG_VE_CH_QP(idxCh), pdx->infoMP4_CBR[idxCh].nCurQP);
				// REG_VE_CH_QP() macro can be used in "Real Channel" and Virtual Channel" both.
				// Because Real Channel QP address is from 0x0780, and Virtual Channel QP address is from 0x07C0.
		}
		// CBR Settings[E]
END_CBR_SETTING:
		pVideoEncIndex = &pdx->statVidEnc.bufIndex[pdx->statVidEnc.posIndexHW];
		pVideoEncIndex->idxCh = idxCh;
		pVideoEncIndex->typeVOP = typeVOP;
		pVideoEncIndex->posMP4 = posMP4;
		pVideoEncIndex->szMP4 = szMP4;
		pVideoEncIndex->posJPEG = posJPEG;
		pVideoEncIndex->szJPEG = szJPEG;

		pdx->statVidEnc.posIndexHW_Minus1 = pdx->statVidEnc.posIndexHW;
		pdx->statVidEnc.posIndexHW = (pdx->statVidEnc.posIndexHW +1) %NUM_STAT_VIDEO_ENC_ITEM;

		goto V_ISR_LOOP_START;

END_MP4_ISR:
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_MP4_ENC], (VOID *)NULL, (VOID *)NULL);

		pdx->statVidEnc.idxLastQ = idxCurQ;
	}

	if (nIntrStat &INTR_DECODER)
	{
		U32 decStat;
		S6010_RegRead_ISR(REG_VD_STATUS0, decStat);

		if (decStat &VD_STATUS0_INTR_ERR)
		{
			KdPrint(("MP4_DEC_ERR\n"));
		}

		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_MP4_DEC], (VOID *)decStat, (VOID *)NULL);
	}
	
	if (nIntrStat &INTR_G723)
	{
		U32 posCurQueue;
		S6010_RegRead_ISR(REG_AUDIO_STA, posCurQueue);
		posCurQueue &= 0x1f;
		S6010_RegRead_ISR(REG_TIMER_SEC, pdx->iS6010.statG723InQueue.bufTime[posCurQueue][0]);
		S6010_RegRead_ISR(REG_TIMER_USEC, pdx->iS6010.statG723InQueue.bufTime[posCurQueue][1]);
		pdx->iS6010.statG723InQueue.posSOLO6010 = posCurQueue;

		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_G723_ENC_DEC], (VOID *)NULL, (VOID *)NULL);
	}
	
	if (nIntrStat &INTR_UART0)
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_UART_0], (VOID *)NULL, (VOID *)NULL);
	}
	
	if (nIntrStat &INTR_UART1)
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_UART_1], (VOID *)NULL, (VOID *)NULL);
	}
	
	if (nIntrStat &INTR_I2C)
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_I2C], (VOID *)NULL, (VOID *)NULL);
	}
	
/*	if (nIntrStat &INTR_SPI)
	{
	}
	
	if (nIntrStat &INTR_PS2_0)
	{
	}
	
	if (nIntrStat &INTR_PS2_1)
	{
	}
*/	
	if (nIntrStat &INTR_PCI_ERR)
	{
		S6010_RegRead_ISR(0x70, nTmp);
		KdPrint(("PCI_ERR:%08x\n", nTmp));

		if (nTmp &0x01)
		{
#ifdef __USE_ONLY_1_P2M_CHANNEL__
			pdx->statP2M[P2M_CH_0].bErrStop = 1;
			S6010_RegWrite_ISR(REG_P2M_CONTROL(P2M_CH_0), 0);
#else
			pdx->statP2M[P2M_CH_0].bErrStop = 1;
			S6010_RegWrite_ISR(REG_P2M_CONTROL(P2M_CH_0), 0);
			pdx->statP2M[P2M_CH_1].bErrStop = 1;
			S6010_RegWrite_ISR(REG_P2M_CONTROL(P2M_CH_1), 0);
			pdx->statP2M[P2M_CH_2].bErrStop = 1;
			S6010_RegWrite_ISR(REG_P2M_CONTROL(P2M_CH_2), 0);
			pdx->statP2M[P2M_CH_3].bErrStop = 1;
			S6010_RegWrite_ISR(REG_P2M_CONTROL(P2M_CH_3), 0);
#endif

			KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_PCI_ERROR], (VOID *)NULL, (VOID *)NULL);
		}
	}
/*	
	if (nIntrStat &INTR_ATA_DIR)
	{
	}
	
	if (nIntrStat &INTR_ATA_CMD)
	{
	}
*/	
	if (nIntrStat &INTR_MOTION)
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_V_MOTION], (VOID *)NULL, (VOID *)NULL);
	}
	
	if (nIntrStat &INTR_VIDEO_IN)
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_LIVE_REFRESH], (VOID *)NULL, (VOID *)NULL);
	}
/*	
	if (nIntrStat &INTR_VIDEO_LOSS)
	{
	}*/
	
	if (nIntrStat &INTR_GPIO)
	{
		S6010_RegRead_ISR(REG_GPIO_INT_ACK_STA, nTmp);

		if (nTmp != 0)
		{
			pdx->pIShare->iGPIO_IntStat.maskIntStat |= nTmp;
			KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_GPIO_INT], (VOID *)NULL, (VOID *)NULL);
		}

//		KdPrint(("INTR_GPIO:%08x\n", nTmp));
		S6010_RegWrite_ISR(REG_GPIO_INT_ACK_STA, nTmp);
	}
	
	if (nIntrStat &INTR_P2M_0)	// P2M_CH_0
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_P2M_0], (VOID *)NULL, (VOID *)NULL);
	}

	if (nIntrStat &INTR_P2M_1)	// P2M_CH_1
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_P2M_1], (VOID *)NULL, (VOID *)NULL);
	}

	if (nIntrStat &INTR_P2M_2)	// P2M_CH_2
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_P2M_2], (VOID *)NULL, (VOID *)NULL);
	}

	if (nIntrStat &INTR_P2M_3)	// P2M_CH_4
	{
		KeInsertQueueDpc (&pdx->bufFuncDPC[IDX_DPC_P2M_3], (VOID *)NULL, (VOID *)NULL);
	}

	if (nIntrStat)
	{
		S6010_RegWrite_ISR(REG_INTR_STAT, nIntrStat);

		return TRUE;
	}

    // If we reach here, then the interrupt is not ours
    return FALSE;
}

VOID Dpc_MP4_ENC (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_TH_VID_ENC_DATA_READ], EVENT_INCREMENT, FALSE);
}

VOID Dpc_MP4_DEC (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pContext;
	INFO_MP4_DEC_QUEUE_ITEM *pQueue;

	if (pdx->pIShare->iMP4DecStat.reqPauseDecoding == 1)
	{
		pdx->pIShare->iMP4DecStat.idxQDecNeed = pdx->pIShare->iMP4DecStat.idxQDecOK;
		if (((U32)pArg1) &VD_STATUS0_INTR_EMPTY)
		{
			pdx->pIShare->iMP4DecStat.reqPauseDecoding = 0;
		}
	}

	if (pdx->pIShare->iMP4DecStat.idxQDecOK != pdx->pIShare->iMP4DecStat.idxQDecNeed)
	{
		pQueue = &pdx->pIShare->iMP4DecStat.iQueue[pdx->pIShare->iMP4DecStat.idxQDecOK];

		S6010_RegWrite_DPC (pdx, REG_VD_IDX0, pQueue->bufMP4DecHdr[0]);
		S6010_RegWrite_DPC (pdx, REG_VD_IDX1, pQueue->bufMP4DecHdr[1]);
		S6010_RegWrite_DPC (pdx, REG_VD_IDX2, pQueue->bufMP4DecHdr[2]);
		S6010_RegWrite_DPC (pdx, REG_VD_IDX4, pQueue->bufMP4DecHdr[3]);

		S6010_RegWrite_DPC (pdx, REG_VD_CODE_ADR, VMEM_DECODE_BASE +pQueue->posFrm +64);
		S6010_RegWrite_DPC (pdx, REG_VD_CTRL, VD_OPER_ON |VD_MAX_ITEM(1));

		pdx->pIShare->iMP4DecStat.idxCurFrm = pQueue->idxFrm;
		pdx->pIShare->iMP4DecStat.idxQDecOK = (pdx->pIShare->iMP4DecStat.idxQDecOK +1) &(SZ_MP4_DEC_QUEUE_ITEM -1);
	}
}

VOID Dpc_G723_ENC_DEC(PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_TH_G723_ENC_READ_DEC_WRITE], EVENT_INCREMENT, FALSE);
}

VOID Dpc_LIVE_REFRESH (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pContext;

	if (pdx->iDDraw.bLiveUpdateOn == 1)
	{
		KeSetEvent (&pdx->bufEvent[IDX_EVT_TH_LIVE_REFRESH], EVENT_INCREMENT, FALSE);
	}
}

VOID Dpc_V_MOTION (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pContext;

	pdx->pIShare->statCurVMotion = S6010_RegRead_DPC (pdx, REG_VI_MOT_STATUS);
	S6010_RegWrite_DPC (pdx, REG_VI_MOT_CLEAR, 0xffff);
	if (pdx->iS6010.bufPKEvt[COMMON_EVT_V_MOTION] != NULL)
	{
		KeSetEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_V_MOTION], 0, FALSE);
	}
}

VOID Dpc_P2M_0 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_0], EVENT_INCREMENT, FALSE);
}

VOID Dpc_P2M_1 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_1], EVENT_INCREMENT, FALSE);
}

VOID Dpc_P2M_2 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_2], EVENT_INCREMENT, FALSE);
}

VOID Dpc_P2M_3 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_3], EVENT_INCREMENT, FALSE);
}

VOID Dpc_I2C (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_IIC], EVENT_INCREMENT, FALSE);
}

VOID Dpc_GPIO_Int (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pContext;

	if (pdx->iS6010.bufPKEvt[COMMON_EVT_GPIO_INT_ALARM] != NULL)
	{
		KeSetEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_GPIO_INT_ALARM], 0, FALSE);
	}
}

VOID Dpc_UART_0 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_TH_UART_0], EVENT_INCREMENT, FALSE);
}

VOID Dpc_UART_1 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_TH_UART_1], EVENT_INCREMENT, FALSE);
}

VOID Dpc_PCI_ERROR (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2)
{
#ifdef __USE_ONLY_1_P2M_CHANNEL__
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_0], EVENT_INCREMENT, FALSE);
#else
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_0], EVENT_INCREMENT, FALSE);
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_1], EVENT_INCREMENT, FALSE);
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_2], EVENT_INCREMENT, FALSE);
	KeSetEvent (&((DEVICE_EXTENSION *)pContext)->bufEvent[IDX_EVT_P2M_3], EVENT_INCREMENT, FALSE);
#endif
}

