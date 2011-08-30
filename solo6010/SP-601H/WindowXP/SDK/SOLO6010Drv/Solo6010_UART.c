#include "Solo6010_UART.h"


int UART_Proc (DEVICE_EXTENSION *pdx, int idxUART);


int S6010_InitUART (DEVICE_EXTENSION *pdx)
{
	U32 numDiv = (CLOCK_INPUT *1000 /3375) -1;

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_UART_0]);

	pdx->bufI_UART[UART_PORT_0].bufPosApp[UART_BUF_RX] = 0;
	pdx->bufI_UART[UART_PORT_0].bufPosApp[UART_BUF_TX] = 0;
	pdx->bufI_UART[UART_PORT_0].bufPosDrv[UART_BUF_RX] = 0;
	pdx->bufI_UART[UART_PORT_0].bufPosDrv[UART_BUF_TX] = 0;

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0]);

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_UART_1]);

	pdx->bufI_UART[UART_PORT_1].bufPosApp[UART_BUF_RX] = 0;
	pdx->bufI_UART[UART_PORT_1].bufPosApp[UART_BUF_TX] = 0;
	pdx->bufI_UART[UART_PORT_1].bufPosDrv[UART_BUF_RX] = 0;
	pdx->bufI_UART[UART_PORT_1].bufPosDrv[UART_BUF_TX] = 0;

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_1]);

	S6010_RegWrite (pdx, REG_UART_CONTROL(UART_PORT_0), 0xff000000);
	S6010_RegWrite (pdx, REG_UART_CONTROL(UART_PORT_0),
		UART_CLK_DIV(numDiv) |INTR_ERR_EN |INTR_RX_EN |INTR_TX_EN |RX_EN |TX_EN
		|BAUDRATE_9600 |UART_DATA_BIT_8 |UART_STOP_BIT_1 |UART_PARITY_NONE);

	S6010_RegWrite (pdx, REG_UART_CONTROL(UART_PORT_1), 0xff000000);
	S6010_RegWrite (pdx, REG_UART_CONTROL(UART_PORT_1),
		UART_CLK_DIV(numDiv) |INTR_ERR_EN |INTR_RX_EN |INTR_TX_EN |RX_EN |TX_EN
		|BAUDRATE_9600 |UART_DATA_BIT_8 |UART_STOP_BIT_1 |UART_PARITY_NONE);

	S6010_EnableInterrupt (pdx, INTR_UART0);
	S6010_EnableInterrupt (pdx, INTR_UART1);

	return 0;
}

void S6010_ResetUART (DEVICE_EXTENSION *pdx, int idxUART_Port)
{
	U32 nRegVal = S6010_RegRead (pdx, REG_UART_CONTROL(idxUART_Port));

	if (idxUART_Port == UART_PORT_0)
	{
		S6010_DisableInterrupt (pdx, INTR_UART0);
	}
	else
	{
		S6010_DisableInterrupt (pdx, INTR_UART1);
	}

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART_Port]);

	pdx->bufI_UART[idxUART_Port].bufPosApp[UART_BUF_RX] = 0;
	pdx->bufI_UART[idxUART_Port].bufPosApp[UART_BUF_TX] = 0;
	pdx->bufI_UART[idxUART_Port].bufPosDrv[UART_BUF_RX] = 0;
	pdx->bufI_UART[idxUART_Port].bufPosDrv[UART_BUF_TX] = 0;

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART_Port]);

	S6010_RegWrite (pdx, REG_UART_CONTROL(idxUART_Port), nRegVal &0xff000000);
	S6010_RegWrite (pdx, REG_UART_CONTROL(idxUART_Port), nRegVal);

	if (idxUART_Port == UART_PORT_0)
	{
		S6010_EnableInterrupt (pdx, INTR_UART0);
	}
	else
	{
		S6010_EnableInterrupt (pdx, INTR_UART1);
	}
}

void S6010_DestroyUART (DEVICE_EXTENSION *pdx)
{
	U32 nRegVal = S6010_RegRead (pdx, REG_UART_CONTROL(UART_PORT_0));

	S6010_DisableInterrupt (pdx, INTR_UART0);
	S6010_DisableInterrupt (pdx, INTR_UART1);

	S6010_RegWrite (pdx, REG_UART_CONTROL(UART_PORT_0), nRegVal &0xff000000);
	S6010_RegWrite (pdx, REG_UART_CONTROL(UART_PORT_1), nRegVal &0xff000000);
}

void KE_UART_0_THREAD (PVOID pParam)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pParam;
	U32 nRet;
	
	KdPrint((DBG_NAME " KE_UART_0_THREAD START\n"));
	
	while (1)
	{
		KeWaitForSingleObject (&pdx->bufEvent[IDX_EVT_TH_UART_0], Executive, KernelMode, FALSE, NULL);
		KeClearEvent (&pdx->bufEvent[IDX_EVT_TH_UART_0]);

		if (pdx->bufBEndThread[IDX_THREAD_UART_0] == 1)
		{
			break;
		}

		while (1)
		{
			nRet = UART_Proc (pdx, UART_PORT_0);
			
			if (nRet == 0)
			{	// All UART job is completed.
				break;
			}
			else if (nRet == 1)
			{	// Rx or Tx job is remained.
			}
			else
			{	// Tx buffer is full. (Need some delay)
				S6010_Sleep_10MS ();
			}
		}
	}

	KdPrint((DBG_NAME " KE_UART_0_THREAD END\n"));
	pdx->bufBEndThread[IDX_THREAD_UART_0] = 0;

	PsTerminateSystemThread (STATUS_SUCCESS);
}

void KE_UART_1_THREAD (PVOID pParam)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pParam;
	U32 nRet;
	
	KdPrint((DBG_NAME " KE_UART_1_THREAD START\n"));
	
	while (1)
	{
		KeWaitForSingleObject (&pdx->bufEvent[IDX_EVT_TH_UART_1], Executive, KernelMode, FALSE, NULL);
		KeClearEvent (&pdx->bufEvent[IDX_EVT_TH_UART_1]);

		if (pdx->bufBEndThread[IDX_THREAD_UART_1] == 1)
		{
			break;
		}

		while (1)
		{
			nRet = UART_Proc (pdx, UART_PORT_1);
			
			if (nRet == 0)
			{	// All UART job is completed.
				break;
			}
			else if (nRet == 1)
			{	// Rx or Tx job is remained.
			}
			else
			{	// Tx buffer is full. (Need some delay)
				S6010_Sleep_10MS ();
			}
		}
	}

	KdPrint((DBG_NAME " KE_UART_1_THREAD END\n"));
	pdx->bufBEndThread[IDX_THREAD_UART_1] = 0;

	PsTerminateSystemThread (STATUS_SUCCESS);
}

int UART_Proc (DEVICE_EXTENSION *pdx, int idxUART)
{
	int bNeedMoreJobToDo, posNext;
	U32 statUART;
	U8 nTmp;
	INFO_UART_BUF *pI_UART = &pdx->bufI_UART[idxUART];

	bNeedMoreJobToDo = 0;

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART]);

	statUART = S6010_RegRead (pdx, REG_UART_STATUS(idxUART));

	// Check error[S]
	if ((statUART &(UART_OVERRUN |UART_FRAME_ERR |UART_PARITY_ERR)) != 0)
	{
		ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART]);
		S6010_ResetUART (pdx, idxUART);
		return 0;
	}
	// Check error[E]

	// Check Rx[S]
	if (UART_RX_BUFF_CNT(statUART) != 0)
	{
		bNeedMoreJobToDo = 1;

		posNext = (pI_UART->bufPosDrv[UART_BUF_RX] +1) %SZ_UART_DATA_BUF;
		nTmp = (U8)(S6010_RegRead (pdx, REG_UART_RX_DATA(idxUART)) &0xff);
		if (posNext != pI_UART->bufPosApp[UART_BUF_RX])
		{
			pI_UART->bufData[UART_BUF_RX][pI_UART->bufPosDrv[UART_BUF_RX]] = nTmp;
			pI_UART->bufPosDrv[UART_BUF_RX] = posNext;
		}
		S6010_RegWrite (pdx, REG_UART_RX_DATA(idxUART), UART_RX_DATA_POP);
		S6010_RegWrite (pdx, REG_UART_RX_DATA(idxUART), 0);
	}
	// Check Rx[E]

	// Check Tx[S]
	if (pI_UART->bufPosDrv[UART_BUF_TX] != pI_UART->bufPosApp[UART_BUF_TX])	// Check whether Tx data remains or not.
	{
		statUART = S6010_RegRead (pdx, REG_UART_STATUS(idxUART));
		if (UART_TX_BUFF_CNT(statUART) == 0)
		{
			if (bNeedMoreJobToDo != 1)
			{	// Only Tx job remain and Tx buffer is now full.
				bNeedMoreJobToDo = 2;
			}
		}
		else
		{
			bNeedMoreJobToDo = 1;

			nTmp = pI_UART->bufData[UART_BUF_TX][pI_UART->bufPosDrv[UART_BUF_TX]];
			S6010_RegWrite (pdx, REG_UART_TX_DATA(idxUART), UART_TX_DATA_PUSH |nTmp);
			S6010_RegWrite (pdx, REG_UART_TX_DATA(idxUART), nTmp);

			pI_UART->bufPosDrv[UART_BUF_TX] = (pI_UART->bufPosDrv[UART_BUF_TX] +1) %SZ_UART_DATA_BUF;
		}
	}
	// Check Tx[E]

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART]);

	return bNeedMoreJobToDo;
}

void S6010_UART_Setup (DEVICE_EXTENSION *pdx, INFO_UART_SETUP *pI_UART_Setup)
{
	U32 valReg = (CLOCK_INPUT *1000 /3375) -1;
	valReg = UART_CLK_DIV(valReg);

	if (pI_UART_Setup->idxPort == UART_PORT_0)
	{
		S6010_DisableInterrupt (pdx, INTR_UART0);
	}
	else
	{
		S6010_DisableInterrupt (pdx, INTR_UART1);
	}

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_UART_0 +pI_UART_Setup->idxPort]);

	pdx->bufI_UART[pI_UART_Setup->idxPort].bufPosApp[UART_BUF_RX] = 0;
	pdx->bufI_UART[pI_UART_Setup->idxPort].bufPosApp[UART_BUF_TX] = 0;
	pdx->bufI_UART[pI_UART_Setup->idxPort].bufPosDrv[UART_BUF_RX] = 0;
	pdx->bufI_UART[pI_UART_Setup->idxPort].bufPosDrv[UART_BUF_TX] = 0;

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0 +pI_UART_Setup->idxPort]);

	S6010_RegWrite (pdx, REG_UART_CONTROL(pI_UART_Setup->idxPort), 0xff000000);

	valReg |= INTR_ERR_EN |INTR_RX_EN |INTR_TX_EN |RX_EN |TX_EN;

	switch (pI_UART_Setup->idxBaudRate)
	{
	case IDX_UART_BAUD_RATE_300:
		valReg |= BAUDRATE_300;
		break;
	case IDX_UART_BAUD_RATE_1200:
		valReg |= BAUDRATE_1200;
		break;
	case IDX_UART_BAUD_RATE_2400:
		valReg |= BAUDRATE_2400;
		break;
	case IDX_UART_BAUD_RATE_4800:
		valReg |= BAUDRATE_4800;
		break;
	case IDX_UART_BAUD_RATE_9600:
		valReg |= BAUDRATE_9600;
		break;
	case IDX_UART_BAUD_RATE_19200:
		valReg |= BAUDRATE_19200;
		break;
	case IDX_UART_BAUD_RATE_38400:
		valReg |= BAUDRATE_38400;
		break;
	case IDX_UART_BAUD_RATE_57600:
		valReg |= BAUDRATE_57600;
		break;
	case IDX_UART_BAUD_RATE_115200:
		valReg |= BAUDRATE_115200;
		break;
	case IDX_UART_BAUD_RATE_230400:
		valReg |= BAUDRATE_230400;
		break;
	default:
		valReg |= BAUDRATE_9600;
		break;
	}

	switch (pI_UART_Setup->idxDataSize)
	{
	case IDX_UART_DATA_SZ_5_BIT:
		valReg |= UART_DATA_BIT_5;
		break;
	case IDX_UART_DATA_SZ_6_BIT:
		valReg |= UART_DATA_BIT_6;
		break;
	case IDX_UART_DATA_SZ_7_BIT:
		valReg |= UART_DATA_BIT_7;
		break;
	case IDX_UART_DATA_SZ_8_BIT:
		valReg |= UART_DATA_BIT_8;
		break;
	default:
		valReg |= UART_DATA_BIT_8;
		break;
	}

	switch (pI_UART_Setup->idxStopBit)
	{
	case IDX_UART_STOP_BIT_1:
		valReg |= UART_STOP_BIT_1;
		break;
	case IDX_UART_STOP_BIT_2:
		valReg |= UART_STOP_BIT_2;
		break;
	case IDX_UART_STOP_BIT_1_5:
		valReg |= UART_STOP_BIT_1_5;
		break;
	default:
		valReg |= UART_STOP_BIT_1;
		break;
	}

	switch (pI_UART_Setup->idxParity)
	{
	case IDX_UART_PARITY_NONE:
		valReg |= UART_PARITY_NONE;
		break;
	case IDX_UART_PARITY_EVEN:
		valReg |= UART_PARITY_EVEN;
		break;
	case IDX_UART_PARITY_ODD:
		valReg |= UART_PARITY_ODD;
		break;
	default:
		valReg |= UART_PARITY_NONE;
		break;
	}

	if (pI_UART_Setup->bEnableCTS_RTS)
	{
		valReg |= MODEM_CTRL_EN;
	}

	if (pI_UART_Setup->bEnableDataDropWhenParityError)
	{
		valReg |= PARITY_ERROR_DROP;
	}

	S6010_RegWrite (pdx, REG_UART_CONTROL(pI_UART_Setup->idxPort), valReg);

	if (pI_UART_Setup->idxPort == UART_PORT_0)
	{
		S6010_EnableInterrupt (pdx, INTR_UART0);
	}
	else
	{
		S6010_EnableInterrupt (pdx, INTR_UART1);
	}
}

void S6010_UART_Read (DEVICE_EXTENSION *pdx, int idxUART_Port, int numRead, U8 *pBufRead, int *pSzRead)
{
	INFO_UART_BUF *pI_UART = &pdx->bufI_UART[idxUART_Port];

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART_Port]);

	if ((pI_UART->bufPosDrv[UART_BUF_RX] -pI_UART->bufPosApp[UART_BUF_RX] +SZ_UART_DATA_BUF) %SZ_UART_DATA_BUF < numRead)
	{
		*pSzRead = (pI_UART->bufPosDrv[UART_BUF_RX] -pI_UART->bufPosApp[UART_BUF_RX] +SZ_UART_DATA_BUF) %SZ_UART_DATA_BUF;
	}
	else
	{
		*pSzRead = numRead;
	}

	if (*pSzRead == 0)
	{
		ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART_Port]);

		return;
	}

	if (pI_UART->bufPosApp[UART_BUF_RX] +*pSzRead > SZ_UART_DATA_BUF)
	{
		memcpy (&pBufRead[0], &pI_UART->bufData[UART_BUF_RX][pI_UART->bufPosApp[UART_BUF_RX]],
			SZ_UART_DATA_BUF -pI_UART->bufPosApp[UART_BUF_RX]);
		memcpy (&pBufRead[SZ_UART_DATA_BUF -pI_UART->bufPosApp[UART_BUF_RX]], &pI_UART->bufData[UART_BUF_RX][0],
			pI_UART->bufPosApp[UART_BUF_RX] +*pSzRead -SZ_UART_DATA_BUF);
	}
	else
	{
		memcpy (&pBufRead[0], &pI_UART->bufData[UART_BUF_RX][pI_UART->bufPosApp[UART_BUF_RX]], *pSzRead);
	}
	pI_UART->bufPosApp[UART_BUF_RX] = (pI_UART->bufPosApp[UART_BUF_RX] +*pSzRead) %SZ_UART_DATA_BUF;

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART_Port]);
}

void S6010_UART_Write (DEVICE_EXTENSION *pdx, int idxUART_Port, int numWrite, U8 *pBufWrite, int *pSzWritten)
{
	INFO_UART_BUF *pI_UART = &pdx->bufI_UART[idxUART_Port];

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART_Port]);

	if ((pI_UART->bufPosDrv[UART_BUF_TX] -pI_UART->bufPosApp[UART_BUF_TX] -1 +SZ_UART_DATA_BUF) %SZ_UART_DATA_BUF < numWrite)
	{
		*pSzWritten = (pI_UART->bufPosDrv[UART_BUF_TX] -pI_UART->bufPosApp[UART_BUF_TX] -1 +SZ_UART_DATA_BUF) %SZ_UART_DATA_BUF;
	}
	else
	{
		*pSzWritten = numWrite;
	}

	if (*pSzWritten == 0)
	{
		ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART_Port]);

		return;
	}

	if (pI_UART->bufPosDrv[UART_BUF_TX] +*pSzWritten > SZ_UART_DATA_BUF)
	{
		memcpy (&pI_UART->bufData[UART_BUF_TX][pI_UART->bufPosDrv[UART_BUF_TX]], &pBufWrite[0],
			SZ_UART_DATA_BUF -pI_UART->bufPosDrv[UART_BUF_TX]);
		memcpy (&pI_UART->bufData[UART_BUF_TX][0], &pBufWrite[SZ_UART_DATA_BUF -pI_UART->bufPosDrv[UART_BUF_TX]],
			pI_UART->bufPosDrv[UART_BUF_TX] +*pSzWritten -SZ_UART_DATA_BUF);
	}
	else
	{
		memcpy (&pI_UART->bufData[UART_BUF_TX][pI_UART->bufPosDrv[UART_BUF_TX]], &pBufWrite[0], *pSzWritten);
	}
	pI_UART->bufPosApp[UART_BUF_TX] = (pI_UART->bufPosApp[UART_BUF_TX] +*pSzWritten) %SZ_UART_DATA_BUF;

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_UART_0 +idxUART_Port]);

	KeSetEvent (&pdx->bufEvent[IDX_EVT_TH_UART_0 +idxUART_Port], EVENT_INCREMENT, FALSE);
}


