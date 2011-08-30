#include "Solo6010.h"

#include "SupportFunc.h"
#include "Solo6010_VideoIn.h"
#include "Solo6010_GPIO.h"
#include "Solo6010_MP4Enc.h"
#include "Solo6010_TW2815.h"
#include "Solo6010_P2M.h"
#include "Solo6010_I2C.h"
#include "Solo6010_Display.h"
#include "Solo6010_SAA7128.h"
#include "Solo6010_G723.h"
#include "Solo6010_MP4Dec.h"
#include "Solo6010_UART.h"


void S6010_InitClock (DEVICE_EXTENSION *pdx);
void S6010_CheckSDRAM (DEVICE_EXTENSION *pdx);

static void SyncExRegRead (DEVICE_EXTENSION *pdx);
static void SyncExRegWrite (DEVICE_EXTENSION *pdx);


#if __NUM_SOLO_CHIPS > 1
	void SyncExRegRead (DEVICE_EXTENSION *pdx)
	{
		if (glIAboutRegIO.bufIdxOldAccS6010[pdx->iS6010.idxID_RegIO] == pdx->iS6010.idxID)
		{
			glIAboutRegIO.numTmpForRegAcc = READ_REGISTER_ULONG((volatile ULONG *)(glIAboutRegIO.bufAddrOtherBaseReg[pdx->iS6010.idxID]));
		}
		pdx->valRegIO = PLX_REG_READ(pdx, pdx->addrRegIO);
		glIAboutRegIO.bufIdxOldAccS6010[pdx->iS6010.idxID_RegIO] = pdx->iS6010.idxID;
	}
	void SyncExRegWrite (DEVICE_EXTENSION *pdx)
	{
		if (glIAboutRegIO.bufIdxOldAccS6010[pdx->iS6010.idxID_RegIO] == pdx->iS6010.idxID)
		{
			glIAboutRegIO.numTmpForRegAcc = READ_REGISTER_ULONG((volatile ULONG *)(glIAboutRegIO.bufAddrOtherBaseReg[pdx->iS6010.idxID]));
		}
		PLX_REG_WRITE(pdx, pdx->addrRegIO, pdx->valRegIO);
		glIAboutRegIO.bufIdxOldAccS6010[pdx->iS6010.idxID_RegIO] = pdx->iS6010.idxID;
	}
	ULONG S6010_RegRead (DEVICE_EXTENSION *pdx, ULONG reg)
	{
		KIRQL oldIrql;
		U32 valRead;

		KeAcquireSpinLock (&glIAboutRegIO.bufKSPLockRegIO[pdx->iS6010.idxID_RegIO], &oldIrql);

		pdx->addrRegIO = reg;
		KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)SyncExRegRead, (PVOID)pdx);
		valRead = pdx->valRegIO;

		KeReleaseSpinLock (&glIAboutRegIO.bufKSPLockRegIO[pdx->iS6010.idxID_RegIO], oldIrql);

		return valRead;
	}
	void S6010_RegWrite (DEVICE_EXTENSION *pdx, ULONG reg, ULONG value)
	{
		KIRQL oldIrql;

		KeAcquireSpinLock (&glIAboutRegIO.bufKSPLockRegIO[pdx->iS6010.idxID_RegIO], &oldIrql);

		pdx->addrRegIO = reg;
		pdx->valRegIO = value;
		KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)SyncExRegWrite, (PVOID)pdx);

		KeReleaseSpinLock (&glIAboutRegIO.bufKSPLockRegIO[pdx->iS6010.idxID_RegIO], oldIrql);
	}
	ULONG S6010_RegRead_DPC (DEVICE_EXTENSION *pdx, ULONG reg)
	{
		U32 valRead;

		KeAcquireSpinLockAtDpcLevel (&glIAboutRegIO.bufKSPLockRegIO[pdx->iS6010.idxID_RegIO]);

		pdx->addrRegIO = reg;
		KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)SyncExRegRead, (PVOID)pdx);
		valRead = pdx->valRegIO;

		KeReleaseSpinLockFromDpcLevel (&glIAboutRegIO.bufKSPLockRegIO[pdx->iS6010.idxID_RegIO]);

		return valRead;
	}
	void S6010_RegWrite_DPC (DEVICE_EXTENSION *pdx, ULONG reg, ULONG value)
	{
		KeAcquireSpinLockAtDpcLevel (&glIAboutRegIO.bufKSPLockRegIO[pdx->iS6010.idxID_RegIO]);

		pdx->addrRegIO = reg;
		pdx->valRegIO = value;
		KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)SyncExRegWrite, (PVOID)pdx);

		KeReleaseSpinLockFromDpcLevel (&glIAboutRegIO.bufKSPLockRegIO[pdx->iS6010.idxID_RegIO]);
	}
#else
	void SyncExRegRead (DEVICE_EXTENSION *pdx)
	{
		pdx->valRegIO = PLX_REG_READ(pdx, pdx->addrRegIO);
		KeStallExecutionProcessor (1);
	}
	void SyncExRegWrite (DEVICE_EXTENSION *pdx)
	{
		PLX_REG_WRITE(pdx, pdx->addrRegIO, pdx->valRegIO);
		KeStallExecutionProcessor (1);
		pdx->valRegIO = PLX_REG_READ(pdx, pdx->addrRegIO);
		KeStallExecutionProcessor (1);
	}
	ULONG S6010_RegRead (DEVICE_EXTENSION *pdx, ULONG reg)
	{
		KIRQL oldIrql;
		U32 valRead;

		KeAcquireSpinLock (&pdx->bufKSpLock[K_SPLOCK_REG_IO], &oldIrql);

		pdx->addrRegIO = reg;
		KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)SyncExRegRead, (PVOID)pdx);
		valRead = pdx->valRegIO;

		KeReleaseSpinLock (&pdx->bufKSpLock[K_SPLOCK_REG_IO], oldIrql);

		return valRead;
	}
	ULONG S6010_RegWrite (DEVICE_EXTENSION *pdx, ULONG reg, ULONG value)
	{
		KIRQL oldIrql;
		ULONG valueWritten;

		KeAcquireSpinLock (&pdx->bufKSpLock[K_SPLOCK_REG_IO], &oldIrql);

		pdx->addrRegIO = reg;
		pdx->valRegIO = value;
		KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)SyncExRegWrite, (PVOID)pdx);
		valueWritten = pdx->valRegIO;

		KeReleaseSpinLock (&pdx->bufKSpLock[K_SPLOCK_REG_IO], oldIrql);

		return valueWritten;
	}
	ULONG S6010_RegRead_DPC (DEVICE_EXTENSION *pdx, ULONG reg)
	{
		U32 valRead;

		KeAcquireSpinLockAtDpcLevel (&pdx->bufKSpLock[K_SPLOCK_REG_IO]);

		pdx->addrRegIO = reg;
		KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)SyncExRegRead, (PVOID)pdx);
		valRead = pdx->valRegIO;

		KeReleaseSpinLockFromDpcLevel (&pdx->bufKSpLock[K_SPLOCK_REG_IO]);

		return valRead;
	}
	void S6010_RegWrite_DPC (DEVICE_EXTENSION *pdx, ULONG reg, ULONG value)
	{
		KeAcquireSpinLockAtDpcLevel (&pdx->bufKSpLock[K_SPLOCK_REG_IO]);

		pdx->addrRegIO = reg;
		pdx->valRegIO = value;
		KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)SyncExRegWrite, (PVOID)pdx);

		KeReleaseSpinLockFromDpcLevel (&pdx->bufKSpLock[K_SPLOCK_REG_IO]);
	}
#endif

BOOLEAN S6010_DisableAllInterrupt (DEVICE_EXTENSION *pdx)
{
	if (pdx->pInterruptObject == NULL)
	{
		PLX_REG_WRITE(pdx, REG_INTR_ENABLE, 0);
		pdx->valRegIO = PLX_REG_READ(pdx, REG_INTR_ENABLE);
	}
	else
	{
		S6010_RegWrite (pdx, REG_INTR_ENABLE, 0);
	}
	pdx->curIntStat = 0;

	return TRUE;
}

void S6010_EnableInterrupt (DEVICE_EXTENSION *pdx, ULONG nMask)
{
//	KdPrint((DBG_NAME "EnInt:%08x ->", pdx->curIntStat));

	pdx->curIntStat |= nMask;
	S6010_RegWrite (pdx, REG_INTR_ENABLE, pdx->curIntStat);

//	KdPrint((DBG_NAME " %08x\n", pdx->curIntStat));
}

void S6010_DisableInterrupt (DEVICE_EXTENSION *pdx, ULONG nMask)
{
//	KdPrint((DBG_NAME "DisInt:%08x ->", pdx->curIntStat));

	pdx->curIntStat &= ~nMask;
	S6010_RegWrite (pdx, REG_INTR_ENABLE, pdx->curIntStat);

//	KdPrint((DBG_NAME " %08x\n", pdx->curIntStat));
}

void S6010_InitInfoS6010 (DEVICE_EXTENSION *pdx, INFO_S6010_INIT *pInfoInit)
{
	pdx->iS6010.typeVideo = pInfoInit->typeVideo;

	pdx->iS6010.szVideoH = pInfoInit->szVideoH;
	pdx->iS6010.szVideoV = pInfoInit->szVideoV;

	pdx->iS6010.posVOutH = pInfoInit->posVOutH;
	pdx->iS6010.posVOutV = pInfoInit->posVOutV;
	pdx->iS6010.posVInH = pInfoInit->posVInH;
	pdx->iS6010.posVInV = pInfoInit->posVInV;

	pdx->iS6010.curStatGPIO_Out = 0;
}

int S6010_Init (DEVICE_EXTENSION *pdx)
{
	KdPrint((DBG_NAME "S6010_Init (Build %s, %s)[S]\n", __DATE__, __TIME__));
	KdPrint((DBG_NAME "Total Used SDRAM Size is 0x%08x/0x%08x Byte.\n", VMEM_TOTAL_USED, SZ_SOLO6010_SDRAM));

	if (VMEM_TOTAL_USED > SZ_SOLO6010_SDRAM)
	{
		KdPrint((DBG_NAME "SDRAM Memory Map Error.\nUsed SDRAM size exceeds Total SDRAM size. (%08x > %08x)\n", VMEM_TOTAL_USED, SZ_SOLO6010_SDRAM));
		return -1;
	}
	
	S6010_EnableInterrupt (pdx, INTR_PCI_ERR);

	if (S6010_InitP2M (pdx) == -1)
	{
		KdPrint((DBG_NAME "P2M Init Error\n"));
		return -1;
	}
	else
	{
		KdPrint((DBG_NAME "P2M Init Ok\n"));
	}
	S6010_InitClock (pdx);
	S6010_InitVideoIn (pdx);
	S6010_InitDisplay (pdx);
	S6010_InitGPIO (pdx);
	S6010_GPIO_A_Mode (pdx, GPIO_PORT(8) |GPIO_PORT(9) |GPIO_PORT(10) |GPIO_PORT(11)
		|GPIO_PORT(12) |GPIO_PORT(13) |GPIO_PORT(14) |GPIO_PORT(15), 3);	// for UART
	S6010_InitUART (pdx);
	
	// video reset [S]
	S6010_GPIO_A_Mode (pdx, GPIO_PORT(4) |GPIO_PORT(5), 1);
	S6010_GPIO_Clear (pdx, GPIO_PORT(4) |GPIO_PORT(5));
	KeStallExecutionProcessor (100);
#ifdef __RESET_SAA7128__
	S6010_GPIO_Set (pdx, GPIO_ALL_PORT, GPIO_PORT(4));
#else
	S6010_GPIO_Set (pdx, GPIO_ALL_PORT, GPIO_PORT(4) |GPIO_PORT(5));
#endif
	KeStallExecutionProcessor (100);
	// video reset [E]

#ifdef __ONLY_FIRST_SOLO_CTRL_I2C__
	if (pdx->iS6010.idxID == 0)
	{
#endif

	// I2C Reset[S]
	S6010_GPIO_A_Mode (pdx, GPIO_PORT(0) |GPIO_PORT(1) |GPIO_PORT(2) |GPIO_PORT(3), 2);
	S6010_InitI2C (pdx);
	KeStallExecutionProcessor (100);
	// I2C Reset[E]

	// TW2815 Setup[S]
#if __NUM_TW2815_CHIPS == 1
	KdPrint((DBG_NAME "TW2815 Setup 0\n"));
	S6010_SetupTW2815 (pdx, TW2815_A_IIC_ID, TW2815_A_IIC_CHANNEL);
#elif __NUM_TW2815_CHIPS == 2
	KdPrint((DBG_NAME "TW2815 Setup 0\n"));
	S6010_SetupTW2815 (pdx, TW2815_A_IIC_ID, TW2815_A_IIC_CHANNEL);
	KdPrint((DBG_NAME "TW2815 Setup 1\n"));
	S6010_SetupTW2815 (pdx, TW2815_B_IIC_ID, TW2815_B_IIC_CHANNEL);
#elif __NUM_TW2815_CHIPS == 3
	KdPrint((DBG_NAME "TW2815 Setup 0\n"));
	S6010_SetupTW2815 (pdx, TW2815_A_IIC_ID, TW2815_A_IIC_CHANNEL);
	KdPrint((DBG_NAME "TW2815 Setup 1\n"));
	S6010_SetupTW2815 (pdx, TW2815_B_IIC_ID, TW2815_B_IIC_CHANNEL);
	KdPrint((DBG_NAME "TW2815 Setup 2\n"));
	S6010_SetupTW2815 (pdx, TW2815_C_IIC_ID, TW2815_C_IIC_CHANNEL);
#elif __NUM_TW2815_CHIPS == 4
	KdPrint((DBG_NAME "TW2815 Setup 0\n"));
	S6010_SetupTW2815 (pdx, TW2815_A_IIC_ID, TW2815_A_IIC_CHANNEL);
	KdPrint((DBG_NAME "TW2815 Setup 1\n"));
	S6010_SetupTW2815 (pdx, TW2815_B_IIC_ID, TW2815_B_IIC_CHANNEL);
	KdPrint((DBG_NAME "TW2815 Setup 2\n"));
	S6010_SetupTW2815 (pdx, TW2815_C_IIC_ID, TW2815_C_IIC_CHANNEL);
	KdPrint((DBG_NAME "TW2815 Setup 3\n"));
	S6010_SetupTW2815 (pdx, TW2815_D_IIC_ID, TW2815_D_IIC_CHANNEL);
#endif
	// TW2815 Setup[E]

#ifdef __USE_SAA7128__
	S6010_SetupSAA7128 (pdx, SAA7128_IIC_ID, SAA7128_IIC_CHANNEL);	// SAA7128 Setup
#endif

#ifdef __ONLY_FIRST_SOLO_CTRL_I2C__
	}
#endif

	S6010_InitMP4Enc (pdx);
	S6010_InitMP4Dec (pdx);
	S6010_InitG723Coder (pdx);

	KdPrint((DBG_NAME "S6010_Init [E]\n"));

	return 0;
}

void S6010_Remove (DEVICE_EXTENSION *pdx)
{
	KdPrint((DBG_NAME "S6010_Remove [S]\n"));

	// Mute Audio Out[S]
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
	// Mute Audio Out[E]

/*	// Reset TW2815 & SAA7128[S]
	S6010_GPIO_A_Mode (pdx, GPIO_PORT(4) |GPIO_PORT(5), 1);
	S6010_GPIO_Clear (pdx, GPIO_PORT(4) |GPIO_PORT(5));
*/	// Reset TW2815 & SAA7128[E]
	// Instead of resetting SAA7128, turn off Display[S]
	S6010_RegWrite (pdx, REG_VO_DISP_CTRL, VO_DISP_ERASE_COUNT(8) |VO_DISP_BASE((VMEM_DISP_WR_BASE >>16) &0xffff));
	// Instead of resetting SAA7128, turn off Display[E]

	S6010_DestroyG723Coder (pdx);
	S6010_DestroyMP4Dec (pdx);
	S6010_DestroyMP4Enc (pdx);
	S6010_DestroyI2C (pdx);
	S6010_DestroyGPIO (pdx);

	S6010_DisableAllInterrupt (pdx);

	KdPrint((DBG_NAME "S6010_Remove [E]\n"));
}

#define VIN_CLK_DELAY	3
void S6010_InitClock (DEVICE_EXTENSION *pdx)
{
	unsigned int sys_clk = 0;

/*	sys_clk = SYS_CONFIG_INPUTDIV(14) |SYS_CONFIG_FEEDBACKDIV(126)
		|SYS_CONFIG_OUTDIV(3);	// 108MHz (54MHz input)
	KdPrint(("SYS_CLK: 108 MHz\n"));
*/

	sys_clk = SYS_CONFIG_INPUTDIV(25) |SYS_CONFIG_FEEDBACKDIV((CLOCK_INPUT *2) -2)
		|SYS_CONFIG_OUTDIV(3);
	KdPrint(("SYS_CLK: %d MHz\n", CLOCK_INPUT));

	sys_clk |= SYS_CONFIG_SDRAM64BIT;

	S6010_RegWrite (pdx, REG_SYS_CONFIG, sys_clk);

#ifdef __INSERT_SDRAM_CHECK__
	S6010_CheckSDRAM (pdx);
#else
	S6010_RegWrite (pdx, REG_DMA_CTRL, 0x000001a5);		// 108MHz
#endif

	S6010_RegWrite (pdx, REG_SYS_VCLK, SYS_CONFIG_VOUT_CLK_SELECT(2)
		|SYS_CONFIG_VIN1415_DELAY(VIN_CLK_DELAY)
		|SYS_CONFIG_VIN1213_DELAY(VIN_CLK_DELAY)
		|SYS_CONFIG_VIN1011_DELAY(VIN_CLK_DELAY)
		|SYS_CONFIG_VIN0809_DELAY(VIN_CLK_DELAY)
		|SYS_CONFIG_VIN0607_DELAY(VIN_CLK_DELAY)
		|SYS_CONFIG_VIN0405_DELAY(VIN_CLK_DELAY)
		|SYS_CONFIG_VIN0203_DELAY(VIN_CLK_DELAY)
		|SYS_CONFIG_VIN0001_DELAY(VIN_CLK_DELAY)
		);

	S6010_RegWrite (pdx, 0x0be0, CLOCK_INPUT -1);	// Timer init (Clk -1)
	S6010_RegWrite (pdx, 0x0008, 1 <<8);	// Bug fix for HW Playback 9-Split mode
}

void S6010_CheckSDRAM (DEVICE_EXTENSION *pdx)
{
	int FDMA_ReadLatency, FDMA_Read_Data_Clock_Inversion, FDMA_Read_Data_Selection, FDMA_Read_Data_Strobe_Selection, SDRAM_Clock_Inversion,
		SDRAM_Size, SDRAM_Refresh_Cycle, i, bPassTest;
	ULONG valPassTest;

	bPassTest = 0;

	for (SDRAM_Refresh_Cycle=0x8; SDRAM_Refresh_Cycle<=0x20; SDRAM_Refresh_Cycle++)
	for (SDRAM_Size=0; SDRAM_Size<=3; SDRAM_Size++)
	for (SDRAM_Clock_Inversion=0; SDRAM_Clock_Inversion<=1; SDRAM_Clock_Inversion++)
	for (FDMA_Read_Data_Strobe_Selection=0; FDMA_Read_Data_Strobe_Selection<=1; FDMA_Read_Data_Strobe_Selection++)
	for (FDMA_Read_Data_Selection=0; FDMA_Read_Data_Selection<=1; FDMA_Read_Data_Selection++)
	for (FDMA_Read_Data_Clock_Inversion=0; FDMA_Read_Data_Clock_Inversion<=1; FDMA_Read_Data_Clock_Inversion++)
	for (FDMA_ReadLatency=0; FDMA_ReadLatency<=3; FDMA_ReadLatency++)
	{
		S6010_RegWrite (pdx, REG_DMA_CTRL, (SDRAM_Refresh_Cycle <<8) +(SDRAM_Size <<6) +(SDRAM_Clock_Inversion <<5) +(FDMA_Read_Data_Strobe_Selection <<4)
			+(FDMA_Read_Data_Selection <<3) +(FDMA_Read_Data_Clock_Inversion <<2) +(FDMA_ReadLatency <<0));
		KdPrint(("CHK_SDRAM: [15:8]:%02x [7:6]:%d [5]:%d [4]:%d [3]:%d [2]:%d [1:0]:%d => ", SDRAM_Refresh_Cycle, SDRAM_Size, SDRAM_Clock_Inversion, FDMA_Read_Data_Strobe_Selection,
			FDMA_Read_Data_Selection, FDMA_Read_Data_Clock_Inversion, FDMA_ReadLatency));

		for (i=0; i<64; i++)
		{
			if (S6010_TestP2M (pdx, VMEM_OSG_BASE +(i %32) *SZ_P2M_BUF_TEST) != 0)
			{
				break;
			}
		}
		if (i == 64)
		{
			KdPrint(("OK\n"));

			if (bPassTest == 0)
			{
				valPassTest = (SDRAM_Refresh_Cycle <<8) +(SDRAM_Size <<6) +(SDRAM_Clock_Inversion <<5) +(FDMA_Read_Data_Strobe_Selection <<4)
					+(FDMA_Read_Data_Selection <<3) +(FDMA_Read_Data_Clock_Inversion <<2) +(FDMA_ReadLatency <<0);
				bPassTest = 1;
			}
		}
		else
		{
			KdPrint(("FAIL\n"));
		}
	}
}
