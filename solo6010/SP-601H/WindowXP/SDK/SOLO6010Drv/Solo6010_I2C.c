#include "Solo6010_I2C.h"


int S6010_I2C_WaitTransfer (DEVICE_EXTENSION *pdx);


void S6010_InitI2C (DEVICE_EXTENSION *pdx)
{
	S6010_RegWrite (pdx, REG_IIC_CFG, IIC_PRESCALE(6) |IIC_ENABLE);
	S6010_EnableInterrupt (pdx, INTR_I2C);
}

void S6010_DestroyI2C (DEVICE_EXTENSION *pdx)
{
	S6010_DisableInterrupt (pdx, INTR_I2C);
}

int S6010_I2C_Write (DEVICE_EXTENSION *pdx, int nChannel,
	int nAddrSlave, int nAddrSub, int nData)
{
	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_IO_I2C]);

	// write slave address
	S6010_RegWrite (pdx, REG_IIC_TXD, nAddrSlave <<0);
	S6010_RegWrite (pdx, REG_IIC_CTRL, IIC_CH(nChannel) |IIC_WRITE |IIC_START);
	if (S6010_I2C_WaitTransfer (pdx))
	{
		KdPrint((DBG_NAME "E:I2C W0\n"));
		goto ERROR_EXIT;
	}

	// write sub address
	S6010_RegWrite (pdx, REG_IIC_TXD, nAddrSub <<0);
	S6010_RegWrite (pdx, REG_IIC_CTRL, IIC_CH(nChannel) |IIC_WRITE);
	if (S6010_I2C_WaitTransfer (pdx))
	{
		KdPrint((DBG_NAME "E:I2C W1\n"));
		goto ERROR_EXIT;
	}

	// write data
	S6010_RegWrite (pdx, REG_IIC_TXD, nData <<0);
	S6010_RegWrite (pdx, REG_IIC_CTRL, IIC_CH(nChannel) |IIC_WRITE |IIC_STOP);
	if (S6010_I2C_WaitTransfer (pdx))
	{
		KdPrint((DBG_NAME "E:I2C W2\n"));
		goto ERROR_EXIT;
	}

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_IO_I2C]);
	return 0;

ERROR_EXIT:
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_IO_I2C]);
	return -1;
}

int S6010_I2C_Read (DEVICE_EXTENSION *pdx, int nChannel,
	int nAddrSlave, int nAddrSlaveRead, int nAddrSub)
{
	unsigned int data;

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_IO_I2C]);

	// read operation
	S6010_RegWrite (pdx, REG_IIC_TXD, nAddrSlave);
	S6010_RegWrite (pdx, REG_IIC_CTRL, IIC_CH(nChannel) |IIC_WRITE |IIC_START);
	if (S6010_I2C_WaitTransfer (pdx))
	{
		KdPrint((DBG_NAME "E:I2C R0\n"));
		goto ERROR_EXIT;
	}

	// write sub address
	S6010_RegWrite (pdx, REG_IIC_TXD, nAddrSub);
	S6010_RegWrite (pdx, REG_IIC_CTRL, IIC_CH(nChannel) |IIC_WRITE);
	if (S6010_I2C_WaitTransfer (pdx))
	{
		KdPrint((DBG_NAME "E:I2C R1\n"));
		goto ERROR_EXIT;
	}

	// drive salve read address
	S6010_RegWrite (pdx, REG_IIC_TXD, nAddrSlaveRead);
	S6010_RegWrite (pdx, REG_IIC_CTRL, IIC_CH(nChannel) |IIC_WRITE |IIC_START);
	if (S6010_I2C_WaitTransfer (pdx))
	{
		KdPrint((DBG_NAME "E:I2C R2\n"));
		goto ERROR_EXIT;
	}

	// read data from slave
	S6010_RegWrite (pdx, REG_IIC_CTRL, IIC_CH(nChannel) |IIC_READ |IIC_STOP |IIC_ACK_EN);
	if (S6010_I2C_WaitTransfer (pdx))
	{
		KdPrint((DBG_NAME "E: I2C R3\n"));
		goto ERROR_EXIT;
	}

	// check read data
	data = 0xff &S6010_RegRead (pdx, REG_IIC_RXD);

	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_IO_I2C]);
	return data;

ERROR_EXIT:
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_IO_I2C]);
	return -1;
}

int S6010_I2C_WaitTransfer (DEVICE_EXTENSION *pdx)
{
	LARGE_INTEGER nTimeout;
	NTSTATUS nRetVal;
	ULONG nStat;

#if DBG
	nTimeout.QuadPart = -5000000;	// 500 ms...
#else
	nTimeout.QuadPart = -1000000;	// 100 ms...
#endif

	nRetVal = KeWaitForSingleObject (&pdx->bufEvent[IDX_EVT_IIC],
		Executive, KernelMode, FALSE, &nTimeout);
	KeClearEvent (&pdx->bufEvent[IDX_EVT_IIC]);
	
	nStat = S6010_RegRead (pdx, REG_IIC_CTRL);

	if (!(IIC_STAT_TRNS &nStat))
		return 0;

	if (nRetVal == STATUS_TIMEOUT)
	{
		KdPrint(("E:I2C_WT_TIMEOUT\n"));
	}

	return -1;
}

