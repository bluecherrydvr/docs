#include "Solo6010_P2M.h"


int S6010_InitP2M (DEVICE_EXTENSION *pdx)
{
	int i, j;

	for(i=0; i<NUM_P2M; i++)
	{
		S6010_RegWrite (pdx, REG_P2M_CONTROL(i), 0);
		S6010_RegWrite (pdx, REG_P2M_CONFIG(i), P2M_DMA_INTERVAL(0) |P2M_PCI_MASTER_MODE);

		S6010_EnableInterrupt (pdx, INTR_P2M(i));
	}

#ifndef __INSERT_SDRAM_CHECK__
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			if (S6010_TestP2M (pdx, VMEM_OSG_BASE +j *SZ_P2M_BUF_TEST) != 0)
			{
				return -1;
			}
		}
	}
#endif

	return 0;
}

int S6010_TestP2M (DEVICE_EXTENSION *pdx, unsigned int addrTest)
{
	int i, numErr;

	numErr = 0;
	for (i=0; i<SZ_P2M_BUF_TEST; i++)
	{
		pdx->bufInfoP2M[P2M_BUF_TEST].bufP2M[i] = i %0xff;
	}

	S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE,
		pdx->bufInfoP2M[P2M_BUF_TEST].nPhyAddr.LowPart,
		addrTest, SZ_P2M_BUF_TEST, 0, P2M_TIMEOUT_NORM);

	memset (pdx->bufInfoP2M[P2M_BUF_TEST].bufP2M, 0, SZ_P2M_BUF_TEST);

	S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_READ,
		pdx->bufInfoP2M[P2M_BUF_TEST].nPhyAddr.LowPart,
		addrTest, SZ_P2M_BUF_TEST, 0, P2M_TIMEOUT_NORM);

	for (i=0; i<SZ_P2M_BUF_TEST; i++)
	{
		if (pdx->bufInfoP2M[P2M_BUF_TEST].bufP2M[i] != i %0xff)
		{
#ifndef __INSERT_SDRAM_CHECK__
			if (numErr == 0)
			{
				KdPrint((DBG_NAME " P2M Test Fail: %02x != %02x (Addr:%08x)", pdx->bufInfoP2M[P2M_BUF_TEST].bufP2M[i], i %0xff, addrTest +i));
			}
#endif
			numErr++;
#ifdef __INSERT_SDRAM_CHECK__
			break;
#endif
		}
	}
	if (numErr > 0)
	{
#ifndef __INSERT_SDRAM_CHECK__
		KdPrint((DBG_NAME " NumError:%d\n", numErr));
#endif
		return -1;
	}

	return 0;
}

#if __NUM_SOLO_CHIPS > 1
#define P2M_WRITE_REG(addr, data)	\
	S6010_RegWrite (pdx, addr, data);	\
	nTmp = S6010_RegRead (pdx, addr);
#else
#define P2M_WRITE_REG(addr, data)	\
	nTmp = S6010_RegWrite (pdx, addr, data);
#endif

#ifdef __USE_ONLY_1_P2M_CHANNEL__

int S6010_P2M_Copy (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy,
	unsigned int numErrorRetry, unsigned int numTimeoutMS)
{
	LARGE_INTEGER nTimeout;
	NTSTATUS ntStatus;
	unsigned int cntRetry, cntRegSet;
	U32 reg_PCI_ERR, nTmp;
#if DBG
	U32 reg_P2M_CTRL;
#endif

	nTimeout.QuadPart = -((int)numTimeoutMS *10000);
	
	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);
	cntRetry = 0;

START_P2M:

	cntRegSet = 0;
RESET_P2M_CTRL:
	P2M_WRITE_REG(REG_P2M_CONTROL(P2M_CH_0), 0);
	if (nTmp != 0)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2M_REG_ACC_FAIL: RESET_P2M_CTRL : ORG:%02x REAL:%02x\n",
				0, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto RESET_P2M_CTRL;
		}
	}

	cntRegSet = 0;
SET_TARGET_ADDR:
	P2M_WRITE_REG(REG_P2M_TAR_ADR(P2M_CH_0), nAddrPC);
	if (nTmp != nAddrPC)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2M_REG_ACC_FAIL: REG_P2M_TAR_ADR() : ORG:%08x REAL:%08x\n",
				nAddrPC, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_TARGET_ADDR;
		}
	}

	cntRegSet = 0;
SET_EXT_ADDR:
	P2M_WRITE_REG(REG_P2M_EXT_ADR(P2M_CH_0), nAddrExt);
	if (nTmp != nAddrExt)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2M_REG_ACC_FAIL: REG_P2M_EXT_ADR() : ORG:%08x REAL:%08x\n",
				nAddrExt, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_EXT_ADDR;
		}
	}

	cntRegSet = 0;
SET_P2M_CFG:
	P2M_WRITE_REG(REG_P2M_EXT_CFG(P2M_CH_0), P2M_EXT_INC(0 >>2) |P2M_COPY_SIZE(szCopy >>2));
	if (nTmp != (P2M_EXT_INC(0 >>2) |P2M_COPY_SIZE(szCopy >>2)))
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2M_REG_ACC_FAIL: REG_P2M_EXT_CFG() : ORG:%08x REAL:%08x\n\n",
				(P2M_EXT_INC(0 >>2) |P2M_COPY_SIZE(szCopy >>2)), nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_P2M_CFG;
		}
	}

	cntRegSet = 0;
SET_P2M_CTRL:
	P2M_WRITE_REG(REG_P2M_CONTROL(P2M_CH_0), P2M_PCI_INC(0 >>2) |P2M_REPEAT(0) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON);
	if (nTmp != (P2M_PCI_INC(0 >>2) |P2M_REPEAT(0) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON))
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2M_REG_ACC_FAIL: REG_P2M_CONTROL() : ORG:%08x REAL:%08x\n",
				(P2M_PCI_INC(0 >>2) |P2M_REPEAT(0) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON), nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_P2M_CTRL;
		}
	}

WAIT_P2M:
	ntStatus = KeWaitForSingleObject (&pdx->bufEvent[P2M_CH_0], Executive, KernelMode, FALSE, &nTimeout);
	KeClearEvent (&pdx->bufEvent[P2M_CH_0]);

	if (ntStatus == STATUS_SUCCESS && pdx->statP2M[P2M_CH_0].bErrStop == 0)
	{
		goto END_P2M;
	}

	cntRetry++;
	if (cntRetry > numErrorRetry)
	{
		goto FAIL_EXIT;
	}

	if (pdx->statP2M[P2M_CH_0].bErrStop == 0)
	{
		reg_PCI_ERR = S6010_RegRead (pdx, REG_PCI_ERROR);
		if (reg_PCI_ERR &0x10)
		{	// P2M is now running
			goto WAIT_P2M;
		}
	}
	else
	{
		pdx->statP2M[P2M_CH_0].bErrStop = 0;
	}

#if DBG
	if (ntStatus == STATUS_TIMEOUT)
	{
		reg_P2M_CTRL = S6010_RegRead (pdx, REG_P2M_CONTROL(P2M_CH_0));
		KdPrint((" P2M_Timeout: CTRL:%08x, PCIE:%08x, PC:%08x, SDRAM:%08x, SZ:%d\n", reg_P2M_CTRL, reg_PCI_ERR, nAddrPC, nAddrExt, szCopy));
	}
	else
	{
		KdPrint((" P2M_ERROR\n"));
	}
#endif

	goto START_P2M;

END_P2M:
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);
	return P2M_FUNC_RET_OK;

FAIL_EXIT:
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);
	return P2M_FUNC_RET_ERROR;
}

int S6010_P2M_Copy_Adv (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy, unsigned int numRepeat, unsigned int nAddrIncPC, unsigned int nAddrIncExt,
	unsigned int numErrorRetry, unsigned int numTimeoutMS)
{
	LARGE_INTEGER nTimeout;
	NTSTATUS ntStatus;
	unsigned int cntRetry, cntRegSet;
	U32 reg_PCI_ERR, nTmp;
#if DBG
	U32 reg_P2M_CTRL;
#endif

	nTimeout.QuadPart = -((int)numTimeoutMS *10000);

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);
	cntRetry = 0;

START_P2M:

	cntRegSet = 0;
RESET_P2M_CTRL:
	P2M_WRITE_REG(REG_P2M_CONTROL(P2M_CH_0), 0);
	if (nTmp != 0)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2MA_REG_ACC_FAIL: RESET_P2M_CTRL : ORG:%02x REAL:%02x\n",
				0, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto RESET_P2M_CTRL;
		}
	}

	cntRegSet = 0;
SET_TARGET_ADDR:
	P2M_WRITE_REG(REG_P2M_TAR_ADR(P2M_CH_0), nAddrPC);
	if (nTmp != nAddrPC)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2MA_REG_ACC_FAIL: REG_P2M_TAR_ADR() : ORG:%08x REAL:%08x\n",
				nAddrPC, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_TARGET_ADDR;
		}
	}

	cntRegSet = 0;
SET_EXT_ADDR:
	P2M_WRITE_REG(REG_P2M_EXT_ADR(P2M_CH_0), nAddrExt);
	if (nTmp != nAddrExt)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2MA_REG_ACC_FAIL: REG_P2M_EXT_ADR() : ORG:%08x REAL:%08x\n",
				nAddrExt, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_EXT_ADDR;
		}
	}

	cntRegSet = 0;
SET_P2M_CFG:
	P2M_WRITE_REG(REG_P2M_EXT_CFG(P2M_CH_0), P2M_EXT_INC(nAddrIncExt >>2) |P2M_COPY_SIZE(szCopy >>2));
	if (nTmp != (P2M_EXT_INC(nAddrIncExt >>2) |P2M_COPY_SIZE(szCopy >>2)))
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2MA_REG_ACC_FAIL: REG_P2M_EXT_CFG() : ORG:%08x REAL:%08x\n\n",
				(P2M_EXT_INC(nAddrIncExt >>2) |P2M_COPY_SIZE(szCopy >>2)), nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_P2M_CFG;
		}
	}

	cntRegSet = 0;
SET_P2M_CTRL:
	P2M_WRITE_REG(REG_P2M_CONTROL(P2M_CH_0), P2M_PCI_INC(nAddrIncPC >>2) |P2M_REPEAT(numRepeat) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON);
	if (nTmp != (P2M_PCI_INC(nAddrIncPC >>2) |P2M_REPEAT(numRepeat) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON))
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" P2MA_REG_ACC_FAIL: REG_P2M_CONTROL() : ORG:%08x REAL:%08x\n",
				(P2M_PCI_INC(nAddrIncPC >>2) |P2M_REPEAT(numRepeat) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON), nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_P2M_CTRL;
		}
	}

WAIT_P2M:
	ntStatus = KeWaitForSingleObject (&pdx->bufEvent[P2M_CH_0], Executive, KernelMode, FALSE, &nTimeout);
	KeClearEvent (&pdx->bufEvent[P2M_CH_0]);

	if (ntStatus == STATUS_SUCCESS && pdx->statP2M[P2M_CH_0].bErrStop == 0)
	{
		goto END_P2M;
	}

	cntRetry++;
	if (cntRetry > numErrorRetry)
	{
		goto FAIL_EXIT;
	}

	if (pdx->statP2M[P2M_CH_0].bErrStop == 0)
	{
		reg_PCI_ERR = S6010_RegRead (pdx, REG_PCI_ERROR);
		if (reg_PCI_ERR &0x10)
		{	// P2M is now running
			goto WAIT_P2M;
		}
	}
	else
	{
		pdx->statP2M[P2M_CH_0].bErrStop = 0;
	}

#if DBG
	if (ntStatus == STATUS_TIMEOUT)
	{
		reg_P2M_CTRL = S6010_RegRead (pdx, REG_P2M_CONTROL(P2M_CH_0));
		KdPrint((" P2MA_Timeout: CTRL:%08x, PCIE:%08x, PC:%08x, SDRAM:%08x, SZ:%d, RPT:%d\n", reg_P2M_CTRL, reg_PCI_ERR, nAddrPC, nAddrExt, szCopy, numRepeat));
	}
	else
	{
		KdPrint((" P2M_ERROR\n"));
	}
#endif

	goto START_P2M;

END_P2M:
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);
	return P2M_FUNC_RET_OK;

FAIL_EXIT:
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);
	return P2M_FUNC_RET_ERROR;
}

#else

int S6010_P2M_Copy (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy,
	unsigned int numErrorRetry, unsigned int numTimeoutMS)
{
	LARGE_INTEGER nTimeout;
	NTSTATUS ntStatus;
	unsigned int cntRetry, cntRegSet;
	U32 reg_PCI_ERR, nTmp;
#if DBG
	U32 reg_P2M_CTRL;
#endif

	nTimeout.QuadPart = -((int)numTimeoutMS *10000);
	
	cntRetry = 0;

START_P2M:

	cntRegSet = 0;
RESET_P2M_CTRL:
	P2M_WRITE_REG(REG_P2M_CONTROL(idxP2M), 0);
	if (nTmp != 0)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2M_REG_ACC_FAIL: RESET_P2M_CTRL : ORG:%02x REAL:%02x\n",
				idxP2M, 0, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto RESET_P2M_CTRL;
		}
	}

	cntRegSet = 0;
SET_TARGET_ADDR:
	P2M_WRITE_REG(REG_P2M_TAR_ADR(idxP2M), nAddrPC);
	if (nTmp != nAddrPC)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2M_REG_ACC_FAIL: REG_P2M_TAR_ADR() : ORG:%08x REAL:%08x\n",
				idxP2M, nAddrPC, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_TARGET_ADDR;
		}
	}

	cntRegSet = 0;
SET_EXT_ADDR:
	P2M_WRITE_REG(REG_P2M_EXT_ADR(idxP2M), nAddrExt);
	if (nTmp != nAddrExt)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2M_REG_ACC_FAIL: REG_P2M_EXT_ADR() : ORG:%08x REAL:%08x\n",
				idxP2M, nAddrExt, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_EXT_ADDR;
		}
	}

	cntRegSet = 0;
SET_P2M_CFG:
	P2M_WRITE_REG(REG_P2M_EXT_CFG(idxP2M), P2M_EXT_INC(0 >>2) |P2M_COPY_SIZE(szCopy >>2));
	if (nTmp != (P2M_EXT_INC(0 >>2) |P2M_COPY_SIZE(szCopy >>2)))
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2M_REG_ACC_FAIL: REG_P2M_EXT_CFG() : ORG:%08x REAL:%08x\n\n",
				idxP2M, (P2M_EXT_INC(0 >>2) |P2M_COPY_SIZE(szCopy >>2)), nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_P2M_CFG;
		}
	}

	cntRegSet = 0;
SET_P2M_CTRL:
	P2M_WRITE_REG(REG_P2M_CONTROL(idxP2M), P2M_PCI_INC(0 >>2) |P2M_REPEAT(0) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON);
	if (nTmp != (P2M_PCI_INC(0 >>2) |P2M_REPEAT(0) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON))
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2M_REG_ACC_FAIL: REG_P2M_CONTROL() : ORG:%08x REAL:%08x\n",
				idxP2M, (P2M_PCI_INC(0 >>2) |P2M_REPEAT(0) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON), nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_P2M_CTRL;
		}
	}

WAIT_P2M:
	ntStatus = KeWaitForSingleObject (&pdx->bufEvent[idxP2M], Executive, KernelMode, FALSE, &nTimeout);
	KeClearEvent (&pdx->bufEvent[idxP2M]);

	if (ntStatus == STATUS_SUCCESS && pdx->statP2M[idxP2M].bErrStop == 0)
	{
		goto END_P2M;
	}

	cntRetry++;
	if (cntRetry > numErrorRetry)
	{
		goto FAIL_EXIT;
	}

	if (pdx->statP2M[idxP2M].bErrStop == 0)
	{
		reg_PCI_ERR = S6010_RegRead (pdx, REG_PCI_ERROR);
		if (reg_PCI_ERR &0x10)
		{	// P2M is now running
			goto WAIT_P2M;
		}
	}
	else
	{
		pdx->statP2M[idxP2M].bErrStop = 0;
	}

#if DBG
	if (ntStatus == STATUS_TIMEOUT)
	{
		reg_P2M_CTRL = S6010_RegRead (pdx, REG_P2M_CONTROL(idxP2M));
		KdPrint((" [%d] P2M_Timeout: CTRL:%08x, PCIE:%08x, PC:%08x, SDRAM:%08x, SZ:%d\n", idxP2M, reg_P2M_CTRL, reg_PCI_ERR, nAddrPC, nAddrExt, szCopy));
	}
	else
	{
		KdPrint((" [%d] P2M_ERROR\n", idxP2M));
	}
#endif

	goto START_P2M;

END_P2M:
	return P2M_FUNC_RET_OK;

FAIL_EXIT:
	return P2M_FUNC_RET_ERROR;
}

int S6010_P2M_Copy_Adv (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy, unsigned int numRepeat, unsigned int nAddrIncPC, unsigned int nAddrIncExt,
	unsigned int numErrorRetry, unsigned int numTimeoutMS)
{
	LARGE_INTEGER nTimeout;
	NTSTATUS ntStatus;
	unsigned int cntRetry, cntRegSet;
	U32 reg_PCI_ERR, nTmp;
#if DBG
	U32 reg_P2M_CTRL;
#endif

	nTimeout.QuadPart = -((int)numTimeoutMS *10000);

	cntRetry = 0;

START_P2M:

	cntRegSet = 0;
RESET_P2M_CTRL:
	P2M_WRITE_REG(REG_P2M_CONTROL(idxP2M), 0);
	if (nTmp != 0)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2MA_REG_ACC_FAIL: RESET_P2M_CTRL : ORG:%02x REAL:%02x\n",
				idxP2M, 0, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto RESET_P2M_CTRL;
		}
	}

	cntRegSet = 0;
SET_TARGET_ADDR:
	P2M_WRITE_REG(REG_P2M_TAR_ADR(idxP2M), nAddrPC);
	if (nTmp != nAddrPC)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2MA_REG_ACC_FAIL: REG_P2M_TAR_ADR() : ORG:%08x REAL:%08x\n",
				idxP2M, nAddrPC, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_TARGET_ADDR;
		}
	}

	cntRegSet = 0;
SET_EXT_ADDR:
	P2M_WRITE_REG(REG_P2M_EXT_ADR(idxP2M), nAddrExt);
	if (nTmp != nAddrExt)
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2MA_REG_ACC_FAIL: REG_P2M_EXT_ADR() : ORG:%08x REAL:%08x\n",
				idxP2M, nAddrExt, nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_EXT_ADDR;
		}
	}

	cntRegSet = 0;
SET_P2M_CFG:
	P2M_WRITE_REG(REG_P2M_EXT_CFG(idxP2M), P2M_EXT_INC(nAddrIncExt >>2) |P2M_COPY_SIZE(szCopy >>2));
	if (nTmp != (P2M_EXT_INC(nAddrIncExt >>2) |P2M_COPY_SIZE(szCopy >>2)))
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2MA_REG_ACC_FAIL: REG_P2M_EXT_CFG() : ORG:%08x REAL:%08x\n\n",
				idxP2M, (P2M_EXT_INC(nAddrIncExt >>2) |P2M_COPY_SIZE(szCopy >>2)), nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_P2M_CFG;
		}
	}

	cntRegSet = 0;
SET_P2M_CTRL:
	P2M_WRITE_REG(REG_P2M_CONTROL(idxP2M), P2M_PCI_INC(nAddrIncPC >>2) |P2M_REPEAT(numRepeat) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON);
	if (nTmp != (P2M_PCI_INC(nAddrIncPC >>2) |P2M_REPEAT(numRepeat) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON))
	{
		cntRegSet++;
		if (cntRegSet > 3)
		{
			KdPrint((" [%d] P2MA_REG_ACC_FAIL: REG_P2M_CONTROL() : ORG:%08x REAL:%08x\n",
				idxP2M, (P2M_PCI_INC(nAddrIncPC >>2) |P2M_REPEAT(numRepeat) |P2M_BURST_SIZE(P2M_BURST_256) |(idxOP) |P2M_TRANS_ON), nTmp));
			goto FAIL_EXIT;
		}
		else
		{
			goto SET_P2M_CTRL;
		}
	}

WAIT_P2M:
	ntStatus = KeWaitForSingleObject (&pdx->bufEvent[idxP2M], Executive, KernelMode, FALSE, &nTimeout);
	KeClearEvent (&pdx->bufEvent[idxP2M]);

	if (ntStatus == STATUS_SUCCESS && pdx->statP2M[idxP2M].bErrStop == 0)
	{
		goto END_P2M;
	}

	cntRetry++;
	if (cntRetry > numErrorRetry)
	{
		goto FAIL_EXIT;
	}

	if (pdx->statP2M[idxP2M].bErrStop == 0)
	{
		reg_PCI_ERR = S6010_RegRead (pdx, REG_PCI_ERROR);
		if (reg_PCI_ERR &0x10)
		{	// P2M is now running
			goto WAIT_P2M;
		}
	}
	else
	{
		pdx->statP2M[idxP2M].bErrStop = 0;
	}

#if DBG
	if (ntStatus == STATUS_TIMEOUT)
	{
		reg_P2M_CTRL = S6010_RegRead (pdx, REG_P2M_CONTROL(idxP2M));
		KdPrint((" [%d] P2MA_Timeout: CTRL:%08x, PCIE:%08x, PC:%08x, SDRAM:%08x, SZ:%d, RPT:%d\n", idxP2M, reg_P2M_CTRL, reg_PCI_ERR, nAddrPC, nAddrExt, szCopy, numRepeat));
	}
	else
	{
		KdPrint((" [%d] P2M_ERROR\n", idxP2M));
	}
#endif

	goto START_P2M;

END_P2M:
	return P2M_FUNC_RET_OK;

FAIL_EXIT:
	return P2M_FUNC_RET_ERROR;
}

int S6010_P2M_Copy_Sync (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy,
	unsigned int numErrorRetry, unsigned int numTimeoutMS)
{
	int nRet;

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);
	nRet = S6010_P2M_Copy (pdx, idxP2M, idxOP, nAddrPC, nAddrExt, szCopy, numErrorRetry, numTimeoutMS);
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);

	return nRet;
}

int S6010_P2M_Copy_Adv_Sync (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy, unsigned int numRepeat, unsigned int nAddrIncPC, unsigned int nAddrIncExt,
	unsigned int numErrorRetry, unsigned int numTimeoutMS)
{
	int nRet;

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);
	nRet = S6010_P2M_Copy_Adv (pdx, idxP2M, idxOP, nAddrPC, nAddrExt, szCopy, numRepeat, nAddrIncPC, nAddrIncExt, numErrorRetry, numTimeoutMS);
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_P2M_IO]);

	return nRet;
}

#endif