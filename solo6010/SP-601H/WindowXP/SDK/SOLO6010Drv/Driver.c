#include <stdio.h>
#include "Driver.h"
#include "Dispatch.h"
#include "Interrupts.h"
#include "SupportFunc.h"
#include "PlugPlay.h"
#include "Power.h"

#include "Solo6010.h"
#include "Solo6010_P2M.h"
#include "Solo6010_I2C.h"
#include "Solo6010_UART.h"

#if defined(ALLOC_PRAGMA) && !DBG
	#pragma alloc_text(INIT, DriverEntry)
#endif


#if __NUM_SOLO_CHIPS > 1
int glNumS6010 = 0;
INFO_GL_ABOUT_REG_IO glIAboutRegIO;

typedef struct _INFO_GL_SET_UP_INT
{
	KSPIN_LOCK kSPLockForISR;

	DEVICE_EXTENSION *bufPdx[NUM_MAX_S6010];
	KIRQL numMaxIrqL;
	U32 bufVector[NUM_MAX_S6010];
	KIRQL bufIrqL[NUM_MAX_S6010];
	KINTERRUPT_MODE bufMode[NUM_MAX_S6010];
	KAFFINITY bufAffinity[NUM_MAX_S6010];
} INFO_GL_SET_UP_INT;

INFO_GL_SET_UP_INT glISetUpInt;
#endif


/******************************************************************************
 *
 * Function   :  DriverEntry
 *
 * Description:  Entry point for the driver.
 *
 ******************************************************************************/
NTSTATUS
DriverEntry(
	PDRIVER_OBJECT	pDriverObject,		// pointer that describe driver itself
	PUNICODE_STRING pRegistryPath		// registry path
	)
{
	KdPrint(("\n<========================================================>\n"));
	KdPrint((DBG_NAME "WDM v%x.%x Device Driver - built on %s %s\n",
			 WDM_MAJORVERSION, WDM_MINORVERSION,
			 __DATE__, __TIME__));

	KdPrint((DBG_NAME "Device Registry path = \"%ws\"\n", pRegistryPath->Buffer));

	// Fill in the appropriate dispatch handlers
	pDriverObject->DriverUnload 						= DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] 		= DispatchCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE]			= DispatchClose;
	pDriverObject->MajorFunction[IRP_MJ_READ]			= DispatchRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE]			= DispatchWrite;
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP]		= DispatchCleanup;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoControl;
	pDriverObject->MajorFunction[IRP_MJ_PNP]			= DispatchPnp;
	pDriverObject->MajorFunction[IRP_MJ_POWER]			= DispatchPower;
	pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = DispatchSystemControl;
	pDriverObject->DriverExtension->AddDevice			= AddDevice;

	return STATUS_SUCCESS;
}


/******************************************************************************
 *
 * Function   :  DriverUnload
 *
 * Description:  Unload the driver.
 *
 ******************************************************************************/
VOID
DriverUnload(
	PDRIVER_OBJECT pDriverObject
	)
{
	KdPrint(("\n"));
	KdPrint((DBG_NAME "Unloading Driver...\n"));

	KdPrint((DBG_NAME "Driver unloaded\n"));
}




/******************************************************************************
 *
 * Function   :  AddDevice
 *
 * Description:  Add a new functional device object for a physical one
 *
 ******************************************************************************/
NTSTATUS
AddDevice(
	PDRIVER_OBJECT pDriverObject,	
	PDEVICE_OBJECT pdo
	)
{
	U8				  i;
	WCHAR			  DeviceNameBuffer[MAX_NAME_LENGTH];
	WCHAR			  DeviceLinkNameBuffer[MAX_NAME_LENGTH];
	NTSTATUS		  status;
	POWER_STATE 	  powerState;
	UNICODE_STRING	  DeviceName_Unicode;
	UNICODE_STRING	  DeviceLinkName_Unicode;
	PDEVICE_OBJECT	  fdo;
	DEVICE_EXTENSION *pdx;
	PHYSICAL_ADDRESS nAddrLow, nAddrHigh;


	// Build a device name and attempt to create it
	for (i=0; i < 10; i++)
	{
		swprintf(DeviceNameBuffer, L"\\Device\\" SOLO6010_DRIVER_NAME_UNICODE L"-%d", i);

		RtlInitUnicodeString(
			&DeviceName_Unicode,
			DeviceNameBuffer
			);

		// Create the device object
		status = IoCreateDevice(
					 pDriverObject,
					 sizeof(DEVICE_EXTENSION),
					 &DeviceName_Unicode,
					 FILE_DEVICE_UNKNOWN,
					 0,
					 FALSE, 		   // Shared by applications
					 &fdo
					 );

		// IoCreateDevice() will fail if the same object already exists
		if (NT_SUCCESS(status))
		{
			break;
		}
	}


	// Check if the creation succeeded
	if ( !NT_SUCCESS(status) )
	{
		KdPrint((DBG_NAME "ERROR - Unable to create Device\n"));
		return status;
	}

	KdPrint((DBG_NAME "Created Device (%ws)...\n", DeviceNameBuffer));

	// Link a Win32 name for user applications
	swprintf(DeviceLinkNameBuffer, L"\\DosDevices\\" SOLO6010_DRIVER_NAME_UNICODE L"-%d", i);

	RtlInitUnicodeString(
		&DeviceLinkName_Unicode,
		DeviceLinkNameBuffer
		);

	KdPrint((DBG_NAME "Creating Win32 symbolic link (%ws)...\n", DeviceLinkNameBuffer));

	status = IoCreateSymbolicLink(
				 &DeviceLinkName_Unicode,
				 &DeviceName_Unicode
				 );

	if ( !NT_SUCCESS(status) )
	{
		KdPrint((DBG_NAME "WARNING - Unable to create symbolic link for Win32 Apps\n"));
		swprintf(DeviceLinkNameBuffer, L"");
	}

	// Initialize the device extension
	pdx = fdo->DeviceExtension;

	RtlZeroMemory(
		pdx,
		sizeof(DEVICE_EXTENSION)
		);

	pdx->pDeviceObject		   = fdo;
	pdx->pPhysicalDeviceObject = pdo;
	pdx->usage				   = 1; 		 // Locked until RemoveDevice

	sprintf(
		pdx->Device.SerialNumber,
		SOLO6010_DRIVER_NAME "-%d",
		i
		);

	wcscpy(
		pdx->DeviceLinkNameBuffer,
		DeviceLinkNameBuffer
		);

	// Initialize PCI BAR variables
	for (i = 0 ; i < PCI_NUM_BARS; i++)
	{
		pdx->PciBar[i].pVa				 = (U32*)-1;
		pdx->PciBar[i].Physical.QuadPart = -1;
		pdx->PciBar[i].Size 			 = 0;
		pdx->PciBar[i].pMdl 			 = NULL;
	}

	for (i=0; i<NUM_FAST_MUTEX; i++)	// Fast mutex init
	{
		ExInitializeFastMutex (&pdx->bufFMutex[i]);
	}

	for (i=0; i<NUM_K_SPLOCK; i++)
	{
		KeInitializeSpinLock (&pdx->bufKSpLock[i]);
	}

	// Initialize the interrupt DPC
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_MP4_ENC]),
		Dpc_MP4_ENC,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_MP4_DEC]),
		Dpc_MP4_DEC,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_G723_ENC_DEC]),
		Dpc_G723_ENC_DEC,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_LIVE_REFRESH]),
		Dpc_LIVE_REFRESH,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_V_MOTION]),
		Dpc_V_MOTION,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_P2M_0]),
		Dpc_P2M_0,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_P2M_1]),
		Dpc_P2M_1,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_P2M_2]),
		Dpc_P2M_2,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_P2M_3]),
		Dpc_P2M_3,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_I2C]),
		Dpc_I2C,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_GPIO_INT]),
		Dpc_GPIO_Int,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_UART_0]),
		Dpc_UART_0,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_UART_1]),
		Dpc_UART_1,
		pdx
		);
	KeInitializeDpc(
		&(pdx->bufFuncDPC[IDX_DPC_PCI_ERROR]),
		Dpc_PCI_ERROR,
		pdx
		);

	KeInitializeEvent(
		&pdx->evRemove,
		NotificationEvent,
		FALSE
		);

	/*
	 *	Since we must pass PNP requests down to the next device object in the
	 *	chain (namely the physical device object created by the bus enumerator),
	 *	we have to remember what that device is. That's why we defined the
	 *	LowerDeviceObject member in our device extension.
	 */
	pdx->pLowerDeviceObject =
		IoAttachDeviceToDeviceStack(
			fdo,
			pdo
			);

	KdPrint((DBG_NAME "Attached device to stack\n"
					  " 		 Lower      DevObj : 0x%08x\n"
					  " 		 Functional DevObj : 0x%08x\n"
					  " 		 Physical   DevObj : 0x%08x\n",
			 pdx->pLowerDeviceObject, fdo, pdo));

	// Notify the power manager of the initial power state
	pdx->Power			   = PowerDeviceD0; 	 // Start device in full power state
	powerState.DeviceState = PowerDeviceD0;
	PoSetPowerState(
		fdo,
		DevicePowerState,
		powerState
		);

	// Indicate the I/O Manager buffer management method
	fdo->Flags |= DO_BUFFERED_IO;

	// Only one thread can open device
	fdo->Flags |= DO_EXCLUSIVE;

	// Manually clear the Device Initialzing flag
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;

	// SOLO6100 Specific Init[S]

	// Share Data Buffer Create[S]
	pdx->pIShare = ExAllocatePoolWithTag (NonPagedPoolCacheAligned, sizeof(INFO_SHARE_DATA), 'INSD');
	if (pdx->pIShare == NULL)
 	{
 		KdPrint((DBG_NAME "Error in 'pIShare' Allocation"));
 	}
	else
	{
		pdx->pMDL_iShare = IoAllocateMdl (pdx->pIShare,
			sizeof(INFO_SHARE_DATA), FALSE, FALSE, NULL);

		if (pdx->pMDL_iShare == NULL)
		{
			ExFreePool (pdx->pIShare);
			pdx->pIShare = NULL;
		}
		else
		{
			MmBuildMdlForNonPagedPool (pdx->pMDL_iShare);
		}
	}
	// Share Data Buffer Create[E]

	// Sharable P2M Buffers Create[S]
	pdx->bufInfoP2M[P2M_BUF_MP4_ENC].szBufP2M = VMEM_MPEG_CODE_SIZE;
	pdx->bufInfoP2M[P2M_BUF_JPEG_ENC].szBufP2M = VMEM_JPEG_CODE_SIZE;
	pdx->bufInfoP2M[P2M_BUF_G723_ENC].szBufP2M = SZ_G723_COMMON_BUF;
	pdx->bufInfoP2M[P2M_BUF_MP4_DEC].szBufP2M = VMEM_DECODE_CODE_SIZE;
	pdx->bufInfoP2M[P2M_BUF_G723_DEC].szBufP2M = SZ_G723_DEC_BUFFER;
	pdx->bufInfoP2M[P2M_BUF_ENC_OSD].szBufP2M = 0x10000;
	pdx->bufInfoP2M[P2M_BUF_VID_OSG].szBufP2M = MAX_SZ_REC_IMAGE_H *2 *MAX_SZ_REC_IMAGE_V;
	pdx->bufInfoP2M[P2M_BUF_CAM_BLK_DET].szBufP2M = SZ_BUF_V_MOTION_TH_IN_BYTE;
	pdx->bufInfoP2M[P2M_BUF_ADV_MOTION].szBufP2M = SZ_BUF_ADV_MOTION_DATA;
	pdx->bufInfoP2M[P2M_BUF_SHR_MEM_FOR_LIVE].szBufP2M = SZ_DDRAW_SHR_MEM_FOR_LIVE;
	pdx->bufInfoP2M[P2M_BUF_TEMP].szBufP2M = PAGE_SIZE;
	pdx->bufInfoP2M[P2M_BUF_TEST].szBufP2M = SZ_P2M_BUF_TEST;
	for (i=0; i<NUM_P2M_BUF; i++)
	{
		if (pdx->bufInfoP2M[i].szBufP2M == 0)
		{
			pdx->bufInfoP2M[i].bufP2M = NULL;
			pdx->bufInfoP2M[i].nPhyAddr.HighPart = 0;
			pdx->bufInfoP2M[i].nPhyAddr.LowPart = 0;
			continue;
		}

		nAddrLow.HighPart = 0;
		nAddrLow.LowPart = 0;
		nAddrHigh.HighPart = 0;
		nAddrHigh.LowPart = 0xffffffff -pdx->bufInfoP2M[i].szBufP2M +1;
		pdx->bufInfoP2M[i].bufP2M = MmAllocateContiguousMemorySpecifyCache (pdx->bufInfoP2M[i].szBufP2M, nAddrLow, nAddrHigh, nAddrLow, MmNonCached);

		if (pdx->bufInfoP2M[i].bufP2M == NULL)
		{
			KdPrint((DBG_NAME "Error in %dth P2M Buffer Allocation\n", i));

			pdx->bufInfoP2M[i].nPhyAddr.HighPart = 0;
			pdx->bufInfoP2M[i].nPhyAddr.LowPart = 0;
		}
		else
		{
 			KdPrint((DBG_NAME "%dth P2M Buffer Allocation OK : %08x\n", i,
				pdx->bufInfoP2M[i].szBufP2M));

			pdx->bufInfoP2M[i].nPhyAddr = MmGetPhysicalAddress (pdx->bufInfoP2M[i].bufP2M);

			pdx->bufInfoP2M[i].pMDL = IoAllocateMdl (pdx->bufInfoP2M[i].bufP2M, pdx->bufInfoP2M[i].szBufP2M, FALSE, FALSE, NULL);

			if (pdx->bufInfoP2M[i].pMDL == NULL)
			{
				MmFreeContiguousMemorySpecifyCache (pdx->bufInfoP2M[i].bufP2M, pdx->bufInfoP2M[i].szBufP2M, MmNonCached);

				pdx->bufInfoP2M[i].bufP2M = NULL;
			}
			else
			{
				MmBuildMdlForNonPagedPool (pdx->bufInfoP2M[i].pMDL);
			}
		}
	}
	// Sharable P2M Buffers Create[E]

	// Temporary Video Motion Threshold Buffer Create[S]
	nAddrLow.HighPart = 0;
	nAddrLow.LowPart = 0;
	nAddrHigh.HighPart = 0;
	nAddrHigh.LowPart = 0xffffffff -SZ_BUF_V_MOTION_TH_IN_BYTE +1;
	pdx->iS6010.bufMotionTh = MmAllocateContiguousMemorySpecifyCache (
		SZ_BUF_V_MOTION_TH_IN_BYTE, nAddrLow, nAddrHigh, nAddrLow, MmCached);
	if (pdx->iS6010.bufMotionTh == NULL)
	{
		KdPrint((DBG_NAME "Error in Video Motion Threshold Buffer Create\n"));
	}
	pdx->iS6010.phyAddrBufMotionTh = MmGetPhysicalAddress (pdx->iS6010.bufMotionTh);
	// Temporary Video Motion Threshold Buffer Create[E]

	// SOLO6010 Specific Init[E]

	return STATUS_SUCCESS;
}




/******************************************************************************
 *
 * Function   :  RemoveDevice
 *
 * Description:  Remove a functional device object
 *
 ******************************************************************************/
VOID
RemoveDevice(
	PDEVICE_OBJECT fdo
	)
{
	NTSTATUS		   status;
	UNICODE_STRING	   DeviceLinkName_Unicode;
	DEVICE_EXTENSION  *pdx;
	int i;


	pdx = fdo->DeviceExtension;

	// Remove Win32 link name
	if (wcslen(pdx->DeviceLinkNameBuffer) != 0)
	{
		KdPrint((DBG_NAME "Removing Win32 link (%ws)\n",
				 pdx->DeviceLinkNameBuffer));

		RtlInitUnicodeString(
			&DeviceLinkName_Unicode,
			pdx->DeviceLinkNameBuffer
			);

		status = IoDeleteSymbolicLink(
					 &DeviceLinkName_Unicode
					 );
		if ( !NT_SUCCESS(status) )
			KdPrint((DBG_NAME "WARNING - Unable to remove Win32 link\n"));
	}

	// Detach device from the device object stack
	if (pdx->pLowerDeviceObject)
	{
		IoDetachDevice (pdx->pLowerDeviceObject);
	}

	// Shared Buffer Free[S]...
	ExFreePool (pdx->pIShare);
	ExFreePool (pdx->iS6010.bufMotionTh);

	for (i=0; i<NUM_P2M_BUF; i++)
	{
		if (pdx->bufInfoP2M[i].szBufP2M != 0)
		{
			MmFreeContiguousMemorySpecifyCache (pdx->bufInfoP2M[i].bufP2M,
				pdx->bufInfoP2M[i].szBufP2M, MmNonCached);
		}
	}
	// Shared Buffer Free[E]...

	KdPrint((DBG_NAME "Deleting device object...\n"));

	// Delete the functional device object
	IoDeleteDevice (fdo);
}

void Sync_ResetAllGOP (DEVICE_EXTENSION *pdx)
{
	int i;
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (pdx->iRec.iMP4[i].bSet == 1 && pdx->bufMP4_GOP[i] == 0)
		{
			pdx->bufMP4_GOP[i] = pdx->iRec.iMP4[i].nGOP;
			S6010_RegWrite_ISR(REG_VE_CH_GOP(i), 1);
				// REG_VE_CH_GOP() macro can be used in "Real Channel" and Virtual Channel" both.
				// Because Real Channel GOP address is from 0x0780, and Virtual Channel GOP address is from 0x07C0.
		}
	}
}

#define MACRO_RESET_GOP	\
	KeSynchronizeExecution (pdx->pInterruptObject, (PKSYNCHRONIZE_ROUTINE)Sync_ResetAllGOP, (PVOID)pdx);\
	memset (bufFlagNeedFrmI, 1, NUM_VID_TOTAL_ENC_CH)

void KE_VID_ENC_DATA_READ_THREAD (PVOID pParam)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pParam;
	REC_BUF_STAT *pIMP4Buf, *pIJPEGBuf;
	HEADER_MP4_FRM *pHdrMP4Frm;
	U32 addrTmpExtStart, addrTmpExtEnd, addrTmpPCStart, addrTmpPCEnd, phyAddrPC, szCopy, bSetNotifyEvent;
	unsigned int i, numP2MCopy, bufAddrPC[3], bufAddrS6010[3], bufSzCopy[3];
	int oldPosIndexHW, oldPosIndexDrv, tmpPosIndexDrv, nRetCode;
	U8 bufFlagNeedFrmI[NUM_VID_TOTAL_ENC_CH];
	STAT_VIDEO_ENC_ITEM *pVidEncStatBufIndex;
	PKTHREAD pkTh;
	KPRIORITY nKPOld;

	KdPrint((DBG_NAME " KE_VID_ENC_DATA_READ_THREAD START\n"));

	pkTh = KeGetCurrentThread ();
	nKPOld = KeSetPriorityThread (pkTh, HIGH_PRIORITY);
	KdPrint((DBG_NAME "KE_VID_ENC_DATA_READ_THREAD Thread Proirity Change %d -> %d", nKPOld, HIGH_PRIORITY));

	bSetNotifyEvent = 0;

	memset (bufFlagNeedFrmI, 0, NUM_VID_TOTAL_ENC_CH);

	pIMP4Buf = &pdx->pIShare->iMP4Buf;
	pIJPEGBuf = &pdx->pIShare->iJPEGBuf;
	pVidEncStatBufIndex = &pdx->statVidEnc.bufIndex[0];
	
	while (1)
	{
		KeWaitForSingleObject (&pdx->bufEvent[IDX_EVT_TH_VID_ENC_DATA_READ],
			Executive, KernelMode, FALSE, NULL);
		KeClearEvent (&pdx->bufEvent[IDX_EVT_TH_VID_ENC_DATA_READ]);

		if (pdx->bufBEndThread[IDX_THREAD_VID_ENC_DATA_READ] == 1)
		{
			break;
		}

		oldPosIndexHW = pdx->statVidEnc.posIndexHW;
		oldPosIndexDrv = pdx->statVidEnc.posIndexDrv;

		// MPEG4 Enc Code Read[S]
		tmpPosIndexDrv = oldPosIndexDrv;

		while (tmpPosIndexDrv != oldPosIndexHW)
		{
			addrTmpExtStart = pVidEncStatBufIndex[tmpPosIndexDrv].posMP4;

			// Check GOP reset[S]
			if (bufFlagNeedFrmI[pVidEncStatBufIndex[tmpPosIndexDrv].idxCh] == 1)
			{
				if (pVidEncStatBufIndex[tmpPosIndexDrv].typeVOP == 0x00)
				{
					bufFlagNeedFrmI[pVidEncStatBufIndex[tmpPosIndexDrv].idxCh] = 0;
				}
				else
				{
					goto ONLY_GET_FRM_HDR;
				}
			}
			// Check GOP reset[E]

			// Check SOLO6010 SDRAM overlap error[S]
			if (tmpPosIndexDrv != pdx->statVidEnc.posIndexHW_Minus1 &&
				(addrTmpExtStart +VMEM_MPEG_CODE_SIZE -pVidEncStatBufIndex[pdx->statVidEnc.posIndexHW_Minus1].posMP4) %VMEM_MPEG_CODE_SIZE < SDRAM_OVERLAP_MARGINE)
			{
#ifdef __PRINT_CRITICAL_LOG_MSG__
				KdPrint((" MP4_SDRAM_OVF\n"));
#endif

				MACRO_RESET_GOP;
				oldPosIndexHW = pdx->statVidEnc.posIndexHW;
				pIMP4Buf->flagError |= REC_BUF_STAT_FLAG_SDRAM_OVF;
				goto END_VID_ENC_DATA_READ;
			}
			// Check SOLO6010 SDRAM overlap error[E]

			if (pVidEncStatBufIndex[tmpPosIndexDrv].typeVOP != 2)
			{
				addrTmpExtEnd = (addrTmpExtStart +pVidEncStatBufIndex[tmpPosIndexDrv].szMP4) %VMEM_MPEG_CODE_SIZE;
				addrTmpPCStart = pIMP4Buf->addrStart;
				szCopy = (addrTmpExtEnd +VMEM_MPEG_CODE_SIZE -addrTmpExtStart) %VMEM_MPEG_CODE_SIZE;

				// Check shared buffer overflow[S]
				if ((addrTmpPCStart != pIMP4Buf->addrEnd) &&
					((addrTmpPCStart +pIMP4Buf->szBuf -pIMP4Buf->addrEnd) %pIMP4Buf->szBuf) <= szCopy)
				{
#ifdef __PRINT_CRITICAL_LOG_MSG__
					KdPrint((" MP4_SHR_MEM_OVF\n"));
#endif

					MACRO_RESET_GOP;
					while (tmpPosIndexDrv != oldPosIndexHW)
					{
						S6010_P2M_Copy (pdx, P2M_CH_0, P2M_READ, pdx->bufInfoP2M[P2M_BUF_TEMP].nPhyAddr.LowPart,
							VMEM_MPEG_CODE_BASE +pVidEncStatBufIndex[tmpPosIndexDrv].posMP4, 32, P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_CODE_DATA);

						pHdrMP4Frm = (HEADER_MP4_FRM *)pdx->bufInfoP2M[P2M_BUF_TEMP].bufP2M;
						pVidEncStatBufIndex[tmpPosIndexDrv].timeSec = GET_SEC_PT(pHdrMP4Frm);
						pVidEncStatBufIndex[tmpPosIndexDrv].timeUSec = GET_USEC_PT(pHdrMP4Frm);

						tmpPosIndexDrv = (tmpPosIndexDrv +1) %NUM_STAT_VIDEO_ENC_ITEM;
					}
					pIMP4Buf->flagError |= REC_BUF_STAT_FLAG_SHR_MEM_OVF;
					bSetNotifyEvent = 1;
					break;
				}
				// Check shared buffer overflow[E]

				phyAddrPC = pdx->bufInfoP2M[P2M_BUF_MP4_ENC].nPhyAddr.LowPart;

				// Ext -> PC [S]
				// Ext : Copy data from START to END
				// PC  : Copy data from END to END +szCopy
				if (addrTmpExtEnd < addrTmpExtStart)	// EXT_BUF|--END---START--|
				{
					if (pIMP4Buf->addrEnd +szCopy > pIMP4Buf->szBuf)
					{
						if (VMEM_MPEG_CODE_SIZE -addrTmpExtStart > pIMP4Buf->szBuf -pIMP4Buf->addrEnd)
						{	// EXT_BUF|--END---START----| ... PC_BUF|-----END---START-|
							numP2MCopy = 3;

							bufSzCopy[0] = pIMP4Buf->szBuf -pIMP4Buf->addrEnd;
							bufSzCopy[1] = VMEM_MPEG_CODE_SIZE -addrTmpExtStart -bufSzCopy[0];
							bufSzCopy[2] = addrTmpExtEnd;

							bufAddrPC[0] = phyAddrPC +pIMP4Buf->addrEnd;
							bufAddrPC[1] = phyAddrPC +0;
							bufAddrPC[2] = phyAddrPC +bufSzCopy[1];

							bufAddrS6010[0] = VMEM_MPEG_CODE_BASE +addrTmpExtStart;
							bufAddrS6010[1] = VMEM_MPEG_CODE_BASE +addrTmpExtStart +bufSzCopy[0];
							bufAddrS6010[2] = VMEM_MPEG_CODE_BASE +0;
						}
						else
						{	// EXT_BUF|-----END---START-| ... PC_BUF|--END---START----|
							numP2MCopy = 3;

							bufSzCopy[0] = VMEM_MPEG_CODE_SIZE -addrTmpExtStart;
							bufSzCopy[1] = pIMP4Buf->szBuf -pIMP4Buf->addrEnd -bufSzCopy[0];
							bufSzCopy[2] = pIMP4Buf->addrEnd +szCopy -pIMP4Buf->szBuf;

							bufAddrPC[0] = phyAddrPC +pIMP4Buf->addrEnd;
							bufAddrPC[1] = phyAddrPC +pIMP4Buf->addrEnd +bufSzCopy[0];
							bufAddrPC[2] = phyAddrPC +0;

							bufAddrS6010[0] = VMEM_MPEG_CODE_BASE +addrTmpExtStart;
							bufAddrS6010[1] = VMEM_MPEG_CODE_BASE +0;
							bufAddrS6010[2] = VMEM_MPEG_CODE_BASE +bufSzCopy[1];
						}
					}
					else
					{
						numP2MCopy = 2;

						bufSzCopy[0] = VMEM_MPEG_CODE_SIZE -addrTmpExtStart;
						bufSzCopy[1] = addrTmpExtEnd;

						bufAddrPC[0] = phyAddrPC +pIMP4Buf->addrEnd;
						bufAddrPC[1] = phyAddrPC +pIMP4Buf->addrEnd +bufSzCopy[0];

						bufAddrS6010[0] = VMEM_MPEG_CODE_BASE +addrTmpExtStart;
						bufAddrS6010[1] = VMEM_MPEG_CODE_BASE +0;
					}
				}
				else	// EXT_BUF|--START---END--|
				{
					if (pIMP4Buf->addrEnd +szCopy > pIMP4Buf->szBuf)
					{
						numP2MCopy = 2;

						bufSzCopy[0] = pIMP4Buf->szBuf -pIMP4Buf->addrEnd;
						bufSzCopy[1] = pIMP4Buf->addrEnd +szCopy -pIMP4Buf->szBuf;

						bufAddrPC[0] = phyAddrPC +pIMP4Buf->addrEnd;
						bufAddrPC[1] = phyAddrPC +0;

						bufAddrS6010[0] = VMEM_MPEG_CODE_BASE +addrTmpExtStart;
						bufAddrS6010[1] = VMEM_MPEG_CODE_BASE +addrTmpExtStart +bufSzCopy[0];
					}
					else
					{
						numP2MCopy = 1;

						bufSzCopy[0] = szCopy;

						bufAddrPC[0] = phyAddrPC +pIMP4Buf->addrEnd;

						bufAddrS6010[0] = VMEM_MPEG_CODE_BASE +addrTmpExtStart;
					}
				}

				for (i=0; i<numP2MCopy; i++)
				{
					if (bufSzCopy[i] == 0)
					{
						continue;
					}
					nRetCode = S6010_P2M_Copy (pdx, P2M_CH_0, P2M_READ, bufAddrPC[i], bufAddrS6010[i], bufSzCopy[i], P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_CODE_DATA);
					if (nRetCode != P2M_FUNC_RET_OK)
					{
						// Perment P2M Error -> Need All SDRAM Reset[S]
						KdPrint((" MP4_PERM_P2M_ERR: PC:%08x, S6010:%08x, SZ:%d\n", bufAddrPC[i], bufAddrS6010[i], bufSzCopy[i]));

						MACRO_RESET_GOP;
						oldPosIndexHW = pdx->statVidEnc.posIndexHW;
						pIMP4Buf->flagError |= REC_BUF_STAT_FLAG_P2M_ERROR;
						goto END_VID_ENC_DATA_READ;
						// Perment P2M Error -> Need All SDRAM Reset[E]
					}
				}
				// Ext -> PC [E]

				pHdrMP4Frm = (HEADER_MP4_FRM *)&pdx->bufInfoP2M[P2M_BUF_MP4_ENC].bufP2M[pIMP4Buf->addrEnd];
				pIMP4Buf->addrEnd = (pIMP4Buf->addrEnd +szCopy) %pIMP4Buf->szBuf;
			}
			else
			{
ONLY_GET_FRM_HDR:
				phyAddrPC = pdx->bufInfoP2M[P2M_BUF_TEMP].nPhyAddr.LowPart;
				S6010_P2M_Copy (pdx, P2M_CH_0, P2M_READ, phyAddrPC, VMEM_MPEG_CODE_BASE +addrTmpExtStart, 32,
					P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);

				pHdrMP4Frm = (HEADER_MP4_FRM *)pdx->bufInfoP2M[P2M_BUF_TEMP].bufP2M;
			}

			pVidEncStatBufIndex[tmpPosIndexDrv].timeSec = GET_SEC_PT(pHdrMP4Frm);
			pVidEncStatBufIndex[tmpPosIndexDrv].timeUSec = GET_USEC_PT(pHdrMP4Frm);

			tmpPosIndexDrv = (tmpPosIndexDrv +1) %NUM_STAT_VIDEO_ENC_ITEM;
		}
		// MPEG4 Enc Code Read[E]

		// Check Notify Event[S]
#ifndef __NOTIFY_EVENT_ON_EVERY_FRM_MP4__
		if ((pIMP4Buf->addrEnd -addrTmpPCStart +pIMP4Buf->szBuf) %pIMP4Buf->szBuf > (pIMP4Buf->szBuf >>3) ||
			bSetNotifyEvent == 1)
#endif
		{
			if (pdx->iS6010.bufPKEvt[COMMON_EVT_MP4] != NULL)
			{
				KeSetEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_MP4], 0, FALSE);
			}
			bSetNotifyEvent = 0;
		}
		// Check Notify Event[E]

		// JPEG Enc Code Read[S]
		tmpPosIndexDrv = oldPosIndexDrv;

		while (tmpPosIndexDrv != oldPosIndexHW)
		{
			if (!(pdx->iS6010.maskEncJPEG &(1 <<pVidEncStatBufIndex[tmpPosIndexDrv].idxCh)))
			{	// JPEG encoding channel check
				tmpPosIndexDrv = (tmpPosIndexDrv +1) %NUM_STAT_VIDEO_ENC_ITEM;
				continue;
			}

			addrTmpExtStart = pVidEncStatBufIndex[tmpPosIndexDrv].posJPEG;

			addrTmpExtEnd = (addrTmpExtStart +pVidEncStatBufIndex[tmpPosIndexDrv].szJPEG) %VMEM_JPEG_CODE_SIZE;
			addrTmpPCStart = pIJPEGBuf->addrStart;
			addrTmpPCEnd = pIJPEGBuf->addrEnd;
			szCopy = (addrTmpExtEnd +VMEM_JPEG_CODE_SIZE -addrTmpExtStart) %VMEM_JPEG_CODE_SIZE;

			if ((addrTmpPCStart != addrTmpPCEnd) &&
				((addrTmpPCStart +pIJPEGBuf->szBuf -addrTmpPCEnd) %pIJPEGBuf->szBuf) <= szCopy +sizeof(STAT_VIDEO_ENC_ITEM))
			{	// Shared buffer overflow
#ifdef __PRINT_CRITICAL_LOG_MSG__
				KdPrint((" JPEG_SHR_BUF_OVF\n"));
#endif

				pIJPEGBuf->flagError |= REC_BUF_STAT_FLAG_SHR_MEM_OVF;
				bSetNotifyEvent = 1;
				break;
			}

			phyAddrPC = pdx->bufInfoP2M[P2M_BUF_JPEG_ENC].nPhyAddr.LowPart;

			// Header copy[S]
			memcpy (&pdx->bufInfoP2M[P2M_BUF_JPEG_ENC].bufP2M[addrTmpPCEnd], &pVidEncStatBufIndex[tmpPosIndexDrv], sizeof(STAT_VIDEO_ENC_ITEM));
			addrTmpPCEnd = (addrTmpPCEnd +sizeof(STAT_VIDEO_ENC_ITEM)) %pIJPEGBuf->szBuf;
			// Header copy[E]
			
			// Ext -> PC [S]
			// Ext : Copy data from START to END
			// PC  : Copy data from END to END +szCopy
			if (addrTmpExtEnd < addrTmpExtStart)	// EXT_BUF|--END---START--|
			{
				if (addrTmpPCEnd +szCopy > pIJPEGBuf->szBuf)
				{
					if (VMEM_JPEG_CODE_SIZE -addrTmpExtStart > pIJPEGBuf->szBuf -addrTmpPCEnd)
					{	// EXT_BUF|--END---START----| ... PC_BUF|-----END---START-|
						numP2MCopy = 3;

						bufSzCopy[0] = pIJPEGBuf->szBuf -addrTmpPCEnd;
						bufSzCopy[1] = VMEM_JPEG_CODE_SIZE -addrTmpExtStart -bufSzCopy[0];
						bufSzCopy[2] = addrTmpExtEnd;

						bufAddrPC[0] = phyAddrPC +addrTmpPCEnd;
						bufAddrPC[1] = phyAddrPC +0;
						bufAddrPC[2] = phyAddrPC +bufSzCopy[1];

						bufAddrS6010[0] = VMEM_JPEG_CODE_BASE +addrTmpExtStart;
						bufAddrS6010[1] = VMEM_JPEG_CODE_BASE +addrTmpExtStart +bufSzCopy[0];
						bufAddrS6010[2] = VMEM_JPEG_CODE_BASE +0;
					}
					else
					{	// EXT_BUF|-----END---START-| ... PC_BUF|--END---START----|
						numP2MCopy = 3;

						bufSzCopy[0] = VMEM_JPEG_CODE_SIZE -addrTmpExtStart;
						bufSzCopy[1] = pIJPEGBuf->szBuf -addrTmpPCEnd -bufSzCopy[0];
						bufSzCopy[2] = addrTmpPCEnd +szCopy -pIJPEGBuf->szBuf;

						bufAddrPC[0] = phyAddrPC +addrTmpPCEnd;
						bufAddrPC[1] = phyAddrPC +addrTmpPCEnd +bufSzCopy[0];
						bufAddrPC[2] = phyAddrPC +0;

						bufAddrS6010[0] = VMEM_JPEG_CODE_BASE +addrTmpExtStart;
						bufAddrS6010[1] = VMEM_JPEG_CODE_BASE +0;
						bufAddrS6010[2] = VMEM_JPEG_CODE_BASE +bufSzCopy[1];
					}
				}
				else
				{
					numP2MCopy = 2;

					bufSzCopy[0] = VMEM_JPEG_CODE_SIZE -addrTmpExtStart;
					bufSzCopy[1] = addrTmpExtEnd;

					bufAddrPC[0] = phyAddrPC +addrTmpPCEnd;
					bufAddrPC[1] = phyAddrPC +addrTmpPCEnd +bufSzCopy[0];

					bufAddrS6010[0] = VMEM_JPEG_CODE_BASE +addrTmpExtStart;
					bufAddrS6010[1] = VMEM_JPEG_CODE_BASE +0;
				}
			}
			else	// EXT_BUF|--START---END--|
			{
				if (addrTmpPCEnd +szCopy > pIJPEGBuf->szBuf)
				{
					numP2MCopy = 2;

					bufSzCopy[0] = pIJPEGBuf->szBuf -addrTmpPCEnd;
					bufSzCopy[1] = addrTmpPCEnd +szCopy -pIJPEGBuf->szBuf;

					bufAddrPC[0] = phyAddrPC +addrTmpPCEnd;
					bufAddrPC[1] = phyAddrPC +0;

					bufAddrS6010[0] = VMEM_JPEG_CODE_BASE +addrTmpExtStart;
					bufAddrS6010[1] = VMEM_JPEG_CODE_BASE +addrTmpExtStart +bufSzCopy[0];
				}
				else
				{
					numP2MCopy = 1;

					bufSzCopy[0] = szCopy;

					bufAddrPC[0] = phyAddrPC +addrTmpPCEnd;

					bufAddrS6010[0] = VMEM_JPEG_CODE_BASE +addrTmpExtStart;
				}
			}

			for (i=0; i<numP2MCopy; i++)
			{
				if (bufSzCopy[i] == 0)
				{
					continue;
				}
				nRetCode = S6010_P2M_Copy (pdx, P2M_CH_0, P2M_READ, bufAddrPC[i], bufAddrS6010[i], bufSzCopy[i], P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_CODE_DATA);
				if (nRetCode != P2M_FUNC_RET_OK)
				{
					// Perment P2M Error -> Need All SDRAM Reset[S]
					KdPrint((" JPEG_PERM_P2M_ERR: PC:%08x, S6010:%08x, SZ:%d\n", bufAddrPC[i], bufAddrS6010[i], bufSzCopy[i]));

					MACRO_RESET_GOP;
					oldPosIndexHW = pdx->statVidEnc.posIndexHW;
					pIJPEGBuf->flagError |= REC_BUF_STAT_FLAG_P2M_ERROR;
					goto END_VID_ENC_DATA_READ;
					// Perment P2M Error -> Need All SDRAM Reset[E]
				}
			}
			// Ext -> PC [E]

			pIJPEGBuf->addrEnd = (addrTmpPCEnd +szCopy) %pIJPEGBuf->szBuf;

			tmpPosIndexDrv = (tmpPosIndexDrv +1) %NUM_STAT_VIDEO_ENC_ITEM;
		}
		// JPEG Enc Code Read[E]

		// Check Notify Event[S]
#ifndef __NOTIFY_EVENT_ON_EVERY_FRM_JPEG__
		if ((pIJPEGBuf->addrEnd -addrTmpPCStart +pIJPEGBuf->szBuf) %pIJPEGBuf->szBuf > (pIJPEGBuf->szBuf >>3) ||
			bSetNotifyEvent == 1)
#endif
		{
			if (pdx->iS6010.bufPKEvt[COMMON_EVT_JPEG] != NULL)
			{
				KeSetEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_JPEG], 0, FALSE);
			}

			bSetNotifyEvent = 0;
		}
		// Check Notify Event[E]

END_VID_ENC_DATA_READ:
		pdx->statVidEnc.posIndexDrv = oldPosIndexHW;
	}

	KdPrint((DBG_NAME " KE_VID_ENC_DATA_READ_THREAD END\n"));
	pdx->bufBEndThread[IDX_THREAD_VID_ENC_DATA_READ] = 0;

	PsTerminateSystemThread (STATUS_SUCCESS);
}

void KE_G723_ENC_READ_DEC_WRITE_THREAD (PVOID pParam)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pParam;
	U32 posCurQueueS6010, posCurQueueDrv, addrExtG723Start, addrExtG723End, szCopy, addrTmpPCStart, i,
		numP2MCopy, bufAddrPC[3], bufAddrS6010[3], bufSzCopy[3], phyAddrPC;
	int nRetCode;
	REC_BUF_STAT *pIG723Buf;
	
	KdPrint((DBG_NAME " KE_G723_ENC_READ_DEC_WRITE_THREAD START\n"));
	
	pIG723Buf = &pdx->pIShare->iG723Buf;
	while (1)
	{
		KeWaitForSingleObject (&pdx->bufEvent[IDX_EVT_TH_G723_ENC_READ_DEC_WRITE],
			Executive, KernelMode, FALSE, NULL);
		KeClearEvent (&pdx->bufEvent[IDX_EVT_TH_G723_ENC_READ_DEC_WRITE]);

		if (pdx->bufBEndThread[IDX_THREAD_G723_ENC_DATA_READ] == 1)
		{
			break;
		}

		if (pdx->iS6010.statG723InQueue.bEncOn == 1)
		{
			posCurQueueS6010 = pdx->iS6010.statG723InQueue.posSOLO6010;
			posCurQueueDrv = pdx->iS6010.statG723InQueue.posEncDrv;

			addrTmpPCStart = pIG723Buf->addrStart;
			addrExtG723Start = posCurQueueDrv *SZ_G723_1_ITEM;
			addrExtG723End = posCurQueueS6010 *SZ_G723_1_ITEM;
			szCopy = (addrExtG723End -addrExtG723Start +SZ_G723_ENC_QUEUE) %SZ_G723_ENC_QUEUE;

			// Check shared buffer overflow[S]
			if ((addrTmpPCStart != pIG723Buf->addrEnd) &&
				((addrTmpPCStart +pIG723Buf->szBuf -pIG723Buf->addrEnd) %pIG723Buf->szBuf) <= szCopy)
			{
				szCopy = 0;
				pIG723Buf->flagError |= REC_BUF_STAT_FLAG_SHR_MEM_OVF;
				goto CHECK_BUFFER_G723_ENC_DATA_READ;
			}
			// Check shared buffer overflow[E]

			phyAddrPC = pdx->bufInfoP2M[P2M_BUF_G723_ENC].nPhyAddr.LowPart;

			if (posCurQueueS6010 < posCurQueueDrv)	// G723_QUEUE|--CUR---OLD--|
			{
				if (pIG723Buf->addrEnd +szCopy > pIG723Buf->szBuf)
				{
					if (SZ_G723_ENC_QUEUE -addrExtG723Start > pIG723Buf->szBuf -pIG723Buf->addrEnd)
					{	// G723_QUEUE|--END---START----| ... PC_BUF|-----END---START-|
						numP2MCopy = 3;

						bufSzCopy[0] = pIG723Buf->szBuf -pIG723Buf->addrEnd;
						bufSzCopy[1] = SZ_G723_ENC_QUEUE -addrExtG723Start -bufSzCopy[0];
						bufSzCopy[2] = addrExtG723End;

						bufAddrPC[0] = phyAddrPC +pIG723Buf->addrEnd;
						bufAddrPC[1] = phyAddrPC +0;
						bufAddrPC[2] = phyAddrPC +bufSzCopy[1];

						bufAddrS6010[0] = VMEM_AUDIO_BASE +addrExtG723Start;
						bufAddrS6010[1] = VMEM_AUDIO_BASE +addrExtG723Start +bufSzCopy[0];
						bufAddrS6010[2] = VMEM_AUDIO_BASE +0;
					}
					else
					{	// G723_QUEUE|-----END---START-| ... PC_BUF|--END---START----|
						numP2MCopy = 3;

						bufSzCopy[0] = SZ_G723_ENC_QUEUE -addrExtG723Start;
						bufSzCopy[1] = pIG723Buf->szBuf -pIG723Buf->addrEnd -bufSzCopy[0];
						bufSzCopy[2] = pIG723Buf->addrEnd +szCopy -pIG723Buf->szBuf;

						bufAddrPC[0] = phyAddrPC +pIG723Buf->addrEnd;
						bufAddrPC[1] = phyAddrPC +pIG723Buf->addrEnd +bufSzCopy[0];
						bufAddrPC[2] = phyAddrPC +0;

						bufAddrS6010[0] = VMEM_AUDIO_BASE +addrExtG723Start;
						bufAddrS6010[1] = VMEM_AUDIO_BASE +0;
						bufAddrS6010[2] = VMEM_AUDIO_BASE +bufSzCopy[1];
					}
				}
				else
				{
					numP2MCopy = 2;

					bufSzCopy[0] = SZ_G723_ENC_QUEUE -addrExtG723Start;
					bufSzCopy[1] = addrExtG723End;

					bufAddrPC[0] = phyAddrPC +pIG723Buf->addrEnd;
					bufAddrPC[1] = phyAddrPC +pIG723Buf->addrEnd +bufSzCopy[0];

					bufAddrS6010[0] = VMEM_AUDIO_BASE +addrExtG723Start;
					bufAddrS6010[1] = VMEM_AUDIO_BASE +0;
				}
			}
			else	// G723_QUEUE|--OLD---CUR--|
			{
				if (pIG723Buf->addrEnd +szCopy > pIG723Buf->szBuf)
				{
					numP2MCopy = 2;

					bufSzCopy[0] = pIG723Buf->szBuf -pIG723Buf->addrEnd;
					bufSzCopy[1] = pIG723Buf->addrEnd +szCopy -pIG723Buf->szBuf;

					bufAddrPC[0] = phyAddrPC +pIG723Buf->addrEnd;
					bufAddrPC[1] = phyAddrPC +0;

					bufAddrS6010[0] = VMEM_AUDIO_BASE +addrExtG723Start;
					bufAddrS6010[1] = VMEM_AUDIO_BASE +addrExtG723Start +bufSzCopy[0];
				}
				else
				{
					numP2MCopy = 1;

					bufSzCopy[0] = szCopy;

					bufAddrPC[0] = phyAddrPC +pIG723Buf->addrEnd;

					bufAddrS6010[0] = VMEM_AUDIO_BASE +addrExtG723Start;
				}
			}
			
			for (i=0; i<numP2MCopy; i++)
			{
				if (bufSzCopy[i] == 0)
				{
					continue;
				}
				nRetCode = S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_READ, bufAddrPC[i], bufAddrS6010[i], bufSzCopy[i], P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_CODE_DATA);
				if (nRetCode != P2M_FUNC_RET_OK)
				{
					// Perment P2M Error -> Need All SDRAM Reset[S]
					KdPrint((" G723_PERM_P2M_ERR: PC:%08x, S6010:%08x, SZ:%d\n", bufAddrPC[i], bufAddrS6010[i], bufSzCopy[i]));

					pIG723Buf->flagError |= REC_BUF_STAT_FLAG_P2M_ERROR;
					goto CHECK_BUFFER_G723_ENC_DATA_READ;
					// Perment P2M Error -> Need All SDRAM Reset[E]
				}
			}
			// Ext -> PC [E]

			// Insert time information[S]
			for (i=0; i<szCopy; i+=SZ_G723_1_ITEM)
			{
				memcpy (&pdx->bufInfoP2M[P2M_BUF_G723_ENC].bufP2M[(pIG723Buf->addrEnd +i +SZ_G723_1_FRM *NUM_G723_CH) %pIG723Buf->szBuf],
					pdx->iS6010.statG723InQueue.bufTime[(posCurQueueDrv +i) %NUM_G723_ENC_QUEUE], sizeof(U32) *2);
			}
			// Insert time information[E]

CHECK_BUFFER_G723_ENC_DATA_READ:
			pdx->iS6010.statG723InQueue.posEncDrv = posCurQueueS6010;
			pIG723Buf->addrEnd = (pIG723Buf->addrEnd +szCopy) %pIG723Buf->szBuf;

			// Check Notify Event[S]
			if ((pIG723Buf->addrEnd -pIG723Buf->addrStart +pIG723Buf->szBuf) %pIG723Buf->szBuf > pIG723Buf->szBuf /4)
			{
				if (pdx->iS6010.bufPKEvt[COMMON_EVT_G723] != NULL)
				{
					KeSetEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_G723], 0, FALSE);
				}
			}
			// Check Notify Event[E]
		}

		if (pdx->iS6010.statG723InQueue.bDecOn == 1)
		{
			int numQCopy;
			
			numQCopy = (pdx->iS6010.statG723InQueue.posSOLO6010 -pdx->iS6010.statG723InQueue.posDecDrv -1 +NUM_G723_ENC_QUEUE) %NUM_G723_ENC_QUEUE;
			if (numQCopy > (NUM_G723_ENC_QUEUE >>3))	// need buffering
			{
				if (pdx->iS6010.statG723InQueue.posDecDrv +numQCopy > NUM_G723_ENC_QUEUE)
				{
					numQCopy = NUM_G723_ENC_QUEUE -pdx->iS6010.statG723InQueue.posDecDrv;
				}
				if (pdx->pIShare->iG723DecStat.idxQDrvWrite +numQCopy > SZ_G723_DEC_QUEUE_ITEM)
				{
					numQCopy = SZ_G723_DEC_QUEUE_ITEM -pdx->pIShare->iG723DecStat.idxQDrvWrite;
				}
				S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_WRITE,
					pdx->bufInfoP2M[P2M_BUF_G723_DEC].nPhyAddr.LowPart +(pdx->pIShare->iG723DecStat.idxQDrvWrite *SZ_G723_1_ITEM),
					VMEM_AUDIO_BASE +SZ_G723_ENC_QUEUE +(pdx->iS6010.statG723InQueue.posDecDrv *SZ_G723_1_ITEM),
					numQCopy *SZ_G723_1_ITEM,
					P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);

				pdx->iS6010.statG723InQueue.posDecDrv = (pdx->iS6010.statG723InQueue.posDecDrv +numQCopy) %NUM_G723_ENC_QUEUE;
				pdx->pIShare->iG723DecStat.idxQDrvWrite = (pdx->pIShare->iG723DecStat.idxQDrvWrite +numQCopy) %SZ_G723_DEC_QUEUE_ITEM;
			}
		}
	}

	KdPrint((DBG_NAME " KE_G723_ENC_READ_DEC_WRITE_THREAD END\n"));
	pdx->bufBEndThread[IDX_THREAD_G723_ENC_DATA_READ] = 0;

	PsTerminateSystemThread (STATUS_SUCCESS);
}

void KE_LIVE_REFRESH_THREAD (PVOID pParam)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pParam;
	int i, idxCurDDrawBuf, idxCurLivePage, bTopField,
		offsetBasePC, offsetBaseExt,
		numDisp, bufCurDecPage[NUM_LIVE_DISP_WND],
		idxScale, numDiv, numMulPos, numMulSz, idxPos, szCellH, szCellV, szH, szV, posH, posV,
		idxTmpPage, numCopyLine;
	ULONG nRegVal;
	CUR_STAT_DDRAW_BUF *pCurDDrawStat;
	static int cntLiveUpdate = 0;
	PKTHREAD pkTh;
	KPRIORITY nKPOld;

	KdPrint((DBG_NAME " KE_LIVE_REFRESH_THREAD START\n"));

	pkTh = KeGetCurrentThread ();
	nKPOld = KeSetPriorityThread (pkTh, HIGH_PRIORITY);
	KdPrint((DBG_NAME "KE_VID_ENC_DATA_READ_THREAD Thread Proirity Change %d -> %d", nKPOld, HIGH_PRIORITY));
	
	pCurDDrawStat = &pdx->pIShare->curStatDDrawBuf;
	while (1)
	{
		KeWaitForSingleObject (&pdx->bufEvent[IDX_EVT_TH_LIVE_REFRESH],
			Executive, KernelMode, FALSE, NULL);
		KeClearEvent (&pdx->bufEvent[IDX_EVT_TH_LIVE_REFRESH]);

		// Adv Motion Data Get[S]
		if (pdx->iS6010.bGetAdvMotionData == 1)
		{
			cntLiveUpdate++;

			if (cntLiveUpdate >= 4)
			{
				S6010_P2M_Copy_Sync (pdx, P2M_CH_3, P2M_READ, pdx->bufInfoP2M[P2M_BUF_ADV_MOTION].nPhyAddr.LowPart,
					VMEM_MOTION_BASE +SZ_BUF_V_MOTION_TH_IN_BYTE *2, SZ_BUF_ADV_MOTION_DATA, P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
				
				if (pdx->iS6010.bufPKEvt[COMMON_EVT_ADV_MOTION_DATA] != NULL)
				{
					KeSetEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_ADV_MOTION_DATA], 0, FALSE);
				}

				cntLiveUpdate = 0;
			}
		}
		// Adv Motion Data Get[E]

		if (pdx->bufBEndThread[IDX_THREAD_LIVE_REFRESH] == 1)
		{
			break;
		}

		if (pdx->iDDraw.bLiveUpdateOn == 1)
		{
			idxCurLivePage = S6010_RegRead (pdx, REG_VI_STATUS0);
			bTopField = (idxCurLivePage >>3) &0x1;

			if (pdx->iDDraw.bField == 1)
			{
				if (bTopField == 0)
				{
#ifdef __USE_SINGLE_DISP_PAGE__
					idxCurLivePage = 0;
#else
					idxCurLivePage = ((idxCurLivePage &0x03) +0) %4;
#endif
				}
				else
				{
#ifdef __USE_SINGLE_DISP_PAGE__
					idxCurLivePage = 0;
#else
					idxCurLivePage = ((idxCurLivePage &0x03) +3) %4;
#endif
				}

				if (bTopField == 0)
				{
					offsetBasePC = 0;
					offsetBaseExt = 0;
				}
				else
				{
					offsetBasePC = pdx->iDDraw.nPitch;
					offsetBaseExt = 2048;
				}
				numCopyLine = pdx->iDDraw.szV >>1;
			}
			else
			{
				if (bTopField == 0)
				{
					continue;
				}

#ifdef __USE_SINGLE_DISP_PAGE__
				idxCurLivePage = 0;
#else
				idxCurLivePage = ((idxCurLivePage &0x03) +3) %4;
#endif
				numCopyLine = pdx->iDDraw.szV;
			}

			if (pCurDDrawStat->numBuf <= 1)
			{	// 1 common surface
				if (pCurDDrawStat->bAccSurf == 1)
				{
					continue;
				}
				pCurDDrawStat->bAccSurf = 1;
				idxCurDDrawBuf = 0;
			}
			else
			{	// N common surface
				if ((pCurDDrawStat->posDrv +1) %pCurDDrawStat->numBuf == pCurDDrawStat->posApp)
				{
					if (pdx->iDDraw.bFlip == 1)
					{
						KeSetEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_OVERLAY_FLIP], 0, FALSE);
					}
					continue;
				}

				idxCurDDrawBuf = pCurDDrawStat->posDrv;
			}

			ExAcquireFastMutex (&pdx->bufFMutex[FMUX_INFO_LIVE]);
			if (pdx->iLive.flagLiveOrDec == 0)
			{	// Live Only
				if (pdx->iDDraw.bField == 1)
				{
					S6010_P2M_Copy_Adv (pdx, P2M_CH_1, P2M_READ,
						pdx->iDDraw.bufPhyAddr[idxCurDDrawBuf].LowPart +offsetBasePC,
						idxCurLivePage *pdx->iDDraw.szLiveExt1Pic +offsetBaseExt,
						pdx->iDDraw.sz1LineCopy, numCopyLine -1,
						pdx->iDDraw.nPitch *2, 4096,
						P2M_ERROR_RETRY_LIVE, P2M_TIMEOUT_NORM);
				}
				else
				{
					if (pdx->iDDraw.bUse1FieldInFrmMode == 0)
					{
						S6010_P2M_Copy_Adv (pdx, P2M_CH_1, P2M_READ,
							pdx->iDDraw.bufPhyAddr[idxCurDDrawBuf].LowPart,
							idxCurLivePage *pdx->iDDraw.szLiveExt1Pic,
							pdx->iDDraw.sz1LineCopy, numCopyLine -1,
							pdx->iDDraw.nPitch, 2048,
							P2M_ERROR_RETRY_LIVE, P2M_TIMEOUT_NORM);
					}
					else
					{
						S6010_P2M_Copy_Adv (pdx, P2M_CH_1, P2M_READ,
							pdx->iDDraw.bufPhyAddr[idxCurDDrawBuf].LowPart,
							idxCurLivePage *pdx->iDDraw.szLiveExt1Pic,
							pdx->iDDraw.sz1LineCopy, (numCopyLine >>1) -1,
							pdx->iDDraw.nPitch, 4096,
							P2M_ERROR_RETRY_LIVE, P2M_TIMEOUT_NORM);
					}
				}
			}
			else
			{	// Live +Dec
				numDisp = GL_NUM_DISP_FROM_DISP_MODE[pdx->iLive.idxSplitMode];
				for (i=0; i<4; i++)
				{
					nRegVal = S6010_RegRead (pdx, REG_MP4D_WR_PAGE_1_TO_4 +4 *i);
					bufCurDecPage[i *4 +0] = (nRegVal >>0) &0x3;
					bufCurDecPage[i *4 +1] = (nRegVal >>8) &0x3;
					bufCurDecPage[i *4 +2] = (nRegVal >>16) &0x3;
					bufCurDecPage[i *4 +3] = (nRegVal >>24) &0x3;
				}

				numDisp = GL_NUM_DISP_FROM_DISP_MODE[pdx->iLive.idxSplitMode];
				numDiv = GL_NUM_DIV_FROM_DISP_MODE[pdx->iLive.idxSplitMode];
				
				szCellH = (pdx->iS6010.szVideoH /numDiv) /4 *4;
				szCellV = (numCopyLine /numDiv) /4 *4;

				for (i=0; i<numDisp; i++)
				{
					numMulPos = GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND[pdx->iLive.idxSplitMode][i];
					numMulSz = GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND[pdx->iLive.idxSplitMode][i];
					idxPos = GL_IDX_POS_FROM_DISP_MODE_IDX_WND[pdx->iLive.idxSplitMode][i];
					idxScale = GL_IDX_SCALE_FROM_DISP_MODE_IDX_WND[pdx->iLive.idxSplitMode][i];

					posH = szCellH *numMulPos *(idxPos %numDiv);
					posV = szCellV *numMulPos *(idxPos /numDiv);
					szH = szCellH *numMulSz;
					szV = szCellV *numMulSz;

					if (((pdx->iLive.flagLiveOrDec >>i) &0x01) == 0x01)
					{
						idxTmpPage = bufCurDecPage[i];
					}
					else
					{
						idxTmpPage = idxCurLivePage;
					}

					if (pdx->iDDraw.bField == 1)
					{
						S6010_P2M_Copy_Adv (pdx, P2M_CH_1, P2M_READ,
							pdx->iDDraw.bufPhyAddr[idxCurDDrawBuf].LowPart +offsetBasePC +posV *(pdx->iDDraw.nPitch *2) +posH *2,
							idxTmpPage *pdx->iDDraw.szLiveExt1Pic +offsetBaseExt +posV *(4096) +posH *2,
							szH *2,
							szV -1,
							pdx->iDDraw.nPitch *2, 4096,
							P2M_ERROR_RETRY_LIVE, P2M_TIMEOUT_NORM);
					}
					else
					{
						if (pdx->iDDraw.bUse1FieldInFrmMode == 0)
						{
							S6010_P2M_Copy_Adv (pdx, P2M_CH_1, P2M_READ,
								pdx->iDDraw.bufPhyAddr[idxCurDDrawBuf].LowPart +posV *pdx->iDDraw.nPitch +posH *2,
								idxTmpPage *pdx->iDDraw.szLiveExt1Pic +posV *(2048) +posH *2,
								szH *2,
								szV -1,
								pdx->iDDraw.nPitch, 2048,
								P2M_ERROR_RETRY_LIVE, P2M_TIMEOUT_NORM);
						}
						else
						{
							S6010_P2M_Copy_Adv (pdx, P2M_CH_1, P2M_READ,
								pdx->iDDraw.bufPhyAddr[idxCurDDrawBuf].LowPart +posV /2 *pdx->iDDraw.nPitch +posH *2,
								idxTmpPage *pdx->iDDraw.szLiveExt1Pic +posV *(2048) +posH *2,
								szH *2,
								(szV >>1) -1,
								pdx->iDDraw.nPitch, 4096,
								P2M_ERROR_RETRY_LIVE, P2M_TIMEOUT_NORM);
						}
					}
				}
			}
			ExReleaseFastMutex (&pdx->bufFMutex[FMUX_INFO_LIVE]);
			
			pCurDDrawStat->bAccSurf = 0;

			if (pdx->iDDraw.bFlip == 1)
			{
				if (pdx->iDDraw.bField == 1)
				{
					if (bTopField == 1)
					{
						pCurDDrawStat->bufIdxField[idxCurDDrawBuf] = LIVE_BUF_FIELD_ODD;
					}
					else
					{
						pCurDDrawStat->bufIdxField[idxCurDDrawBuf] = LIVE_BUF_FIELD_EVEN;
					}
				}
				else
				{
					pCurDDrawStat->bufIdxField[idxCurDDrawBuf] = LIVE_BUF_STAT_FRAME;
				}
				pCurDDrawStat->posDrv = (pCurDDrawStat->posDrv +1) %pCurDDrawStat->numBuf;

				KeSetEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_OVERLAY_FLIP], 0, FALSE);
			}
		}
	}

	KdPrint((DBG_NAME " KE_LIVE_REFRESH_THREAD END\n"));
	pdx->bufBEndThread[IDX_THREAD_LIVE_REFRESH] = 0;

	PsTerminateSystemThread (STATUS_SUCCESS);
}

void KE_TEST_THREAD (PVOID pParam)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pParam;
#ifdef __INSERT_P2M_TEST__
	int cntTest;

	cntTest = 0;
#endif

	KdPrint((DBG_NAME " KE_TEST_THREAD START\n"));
	
	while (1)
	{
		if (pdx->bufBEndThread[IDX_THREAD_TEST] == 1)
		{
			break;
		}

#ifdef __INSERT_P2M_TEST__
		if (pdx->iDDraw.bLiveUpdateOn == 1)
		{
			S6010_TestP2M (pdx, VMEM_TOTAL_USED);

			cntTest++;
			if (cntTest > 300)
			{
				cntTest = 0;
				KdPrint((DBG_NAME " ING\n"));
			}
		}
		else
		{
			S6010_Sleep_1S ();
		}
#else
		S6010_Sleep_1S ();
#endif
	}

	KdPrint((DBG_NAME " KE_TEST_THREAD END\n"));
	pdx->bufBEndThread[IDX_THREAD_TEST] = 0;

	PsTerminateSystemThread (STATUS_SUCCESS);
}

void KE_MP4_DEC_FILL_BUF_THREAD (PVOID pParam)
{
	DEVICE_EXTENSION *pdx = (DEVICE_EXTENSION *)pParam;

	HEADER_MP4_DEC_FRM *pHdrMP4DecFrm;
	INFO_MP4_DEC_STAT *pIMP4DecStat;
	int i, idxQDrvWrite, idxQAppWrite, numCopyFrm, idxQCurCopy, idxWnd;
	unsigned int posCopyStart, posCopyEnd, posTmpCopyStart, posTmpCopyEnd;
	
	KdPrint((DBG_NAME " KE_MP4_DEC_FILL_BUF_THREAD START\n"));
	
	while (1)
	{
		KeWaitForSingleObject (&pdx->bufEvent[IDX_EVT_TH_MP4_DEC_FILL_BUF],
			Executive, KernelMode, FALSE, NULL);

		if (pdx->bufBEndThread[IDX_THREAD_MP4_DEC_FILL_BUF] == 1)
		{
			break;
		}

		if (pdx->iS6010.bufPKEvt[COMMON_EVT_MP4D_COPY_DATA] == NULL)
		{
			S6010_Sleep_10MS ();
			continue;
		}

		KeWaitForSingleObject (pdx->iS6010.bufPKEvt[COMMON_EVT_MP4D_COPY_DATA],
			Executive, KernelMode, FALSE, NULL);
		KeClearEvent (pdx->iS6010.bufPKEvt[COMMON_EVT_MP4D_COPY_DATA]);

		pIMP4DecStat = &pdx->pIShare->iMP4DecStat;

		idxQDrvWrite = pIMP4DecStat->idxQDrvWrite;
		idxQAppWrite = pIMP4DecStat->idxQAppWrite;

		if (idxQDrvWrite != idxQAppWrite)
		{
			numCopyFrm = (idxQAppWrite -idxQDrvWrite +SZ_MP4_DEC_QUEUE_ITEM) &(SZ_MP4_DEC_QUEUE_ITEM -1);

			for (i=0; i<numCopyFrm; i++)
			{
				idxQCurCopy = (idxQDrvWrite +i) &(SZ_MP4_DEC_QUEUE_ITEM -1);
				idxWnd = pIMP4DecStat->iQueue[idxQCurCopy].idxWnd;
				pHdrMP4DecFrm = (HEADER_MP4_DEC_FRM *)&pdx->bufInfoP2M[P2M_BUF_MP4_DEC].bufP2M[pIMP4DecStat->iQueue[idxQCurCopy].posFrm];

				pHdrMP4DecFrm->nStat[VD_IDX_0] &= ~(VD_IDX_START_AT_SYNC |VD_IDX_DECODE_ONLY |(0xf <<24));
				pHdrMP4DecFrm->nStat[VD_IDX_0] |= (idxWnd <<24);			// Source Channel ID

				pHdrMP4DecFrm->nStat[VD_IDX_1] &= ~(0xfff <<16);
				pHdrMP4DecFrm->nStat[VD_IDX_1] |= VD_IDX_WINDOW(idxWnd);	// Target Window ID
				pHdrMP4DecFrm->nStat[VD_IDX_1] |= (pIMP4DecStat->iQueue[idxQCurCopy].bFrameIntp <<16);		// Frame Interpolation On

				pHdrMP4DecFrm->nStat[VD_IDX_2] = VD_IDX_REF_BASE(MEM_DEC_REF_BASE +MEM_DEC_REF_SIZE	*idxWnd);

				switch (pIMP4DecStat->iQueue[idxQCurCopy].idxDecMode)
				{
				case MP4D_IDX_DEC_ONLY:
					pIMP4DecStat->bufDecPage[idxWnd] = (pIMP4DecStat->bufDispPage[idxWnd] +3) &0x3;
					break;
				case MP4D_IDX_DEC_AND_DISP:
					pIMP4DecStat->bufDecPage[idxWnd] = (pIMP4DecStat->bufDecPage[idxWnd] +1) &0x3;
					pIMP4DecStat->bufDispPage[idxWnd] = pIMP4DecStat->bufDecPage[idxWnd];
					break;
				}
				pHdrMP4DecFrm->nStat[VD_IDX_4] = VD_IDX_DEC_WR_PAGE(pIMP4DecStat->bufDecPage[idxWnd]) +VD_IDX_DISP_RD_PAGE(pIMP4DecStat->bufDispPage[idxWnd]);

				pIMP4DecStat->iQueue[idxQCurCopy].bufMP4DecHdr[0] = pHdrMP4DecFrm->nStat[VD_IDX_0];
				pIMP4DecStat->iQueue[idxQCurCopy].bufMP4DecHdr[1] = pHdrMP4DecFrm->nStat[VD_IDX_1];
				pIMP4DecStat->iQueue[idxQCurCopy].bufMP4DecHdr[2] = pHdrMP4DecFrm->nStat[VD_IDX_2];
				pIMP4DecStat->iQueue[idxQCurCopy].bufMP4DecHdr[3] = pHdrMP4DecFrm->nStat[VD_IDX_4];
			}

			posCopyStart = pIMP4DecStat->iQueue[idxQDrvWrite].posFrm;
			posCopyEnd = pIMP4DecStat->iQueue[idxQCurCopy].posFrm +pIMP4DecStat->iQueue[idxQCurCopy].szFrm;
			if (posCopyEnd < posCopyStart)
			{
				posCopyEnd += VMEM_DECODE_CODE_SIZE;
			}

			for (; posCopyStart+0x20000<posCopyEnd; posCopyStart+=0x20000)
			{
				posTmpCopyStart = posCopyStart %VMEM_DECODE_CODE_SIZE;
				posTmpCopyEnd = (posCopyStart +0x20000) %VMEM_DECODE_CODE_SIZE;
				if (posTmpCopyEnd < posTmpCopyStart)
				{
					S6010_P2M_Copy (pdx, P2M_CH_2, P2M_WRITE,
						pdx->bufInfoP2M[P2M_BUF_MP4_DEC].nPhyAddr.LowPart +posTmpCopyStart,
						VMEM_DECODE_BASE +posTmpCopyStart,
						VMEM_DECODE_CODE_SIZE -posTmpCopyStart,
						P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);

					S6010_P2M_Copy (pdx, P2M_CH_2, P2M_WRITE,
						pdx->bufInfoP2M[P2M_BUF_MP4_DEC].nPhyAddr.LowPart +0,
						VMEM_DECODE_BASE +0,
						posTmpCopyEnd,
						P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
				}
				else
				{
					S6010_P2M_Copy (pdx, P2M_CH_2, P2M_WRITE,
						pdx->bufInfoP2M[P2M_BUF_MP4_DEC].nPhyAddr.LowPart +posTmpCopyStart,
						VMEM_DECODE_BASE +posTmpCopyStart,
						posTmpCopyEnd -posTmpCopyStart,
						P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
				}
			}

			posTmpCopyStart = posCopyStart %VMEM_DECODE_CODE_SIZE;
			posTmpCopyEnd = posCopyEnd %VMEM_DECODE_CODE_SIZE;
			if (posTmpCopyEnd < posTmpCopyStart)
			{
				S6010_P2M_Copy (pdx, P2M_CH_2, P2M_WRITE,
					pdx->bufInfoP2M[P2M_BUF_MP4_DEC].nPhyAddr.LowPart +posTmpCopyStart,
					VMEM_DECODE_BASE +posTmpCopyStart,
					VMEM_DECODE_CODE_SIZE -posTmpCopyStart,
					P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);

				S6010_P2M_Copy (pdx, P2M_CH_2, P2M_WRITE,
					pdx->bufInfoP2M[P2M_BUF_MP4_DEC].nPhyAddr.LowPart +0,
					VMEM_DECODE_BASE +0,
					posTmpCopyEnd,
					P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
			}
			else
			{
				S6010_P2M_Copy (pdx, P2M_CH_2, P2M_WRITE,
					pdx->bufInfoP2M[P2M_BUF_MP4_DEC].nPhyAddr.LowPart +posTmpCopyStart,
					VMEM_DECODE_BASE +posTmpCopyStart,
					posTmpCopyEnd -posTmpCopyStart,
					P2M_ERROR_RETRY_NORM, P2M_TIMEOUT_NORM);
			}

			pIMP4DecStat->idxQDrvWrite = idxQAppWrite;
		}
	}

	KdPrint((DBG_NAME " KE_MP4_DEC_FILL_BUF_THREAD END\n"));
	pdx->bufBEndThread[IDX_THREAD_MP4_DEC_FILL_BUF] = 0;

	PsTerminateSystemThread (STATUS_SUCCESS);
}

/******************************************************************************
 *
 * Function   :  StartDevice
 *
 * Description:  Start a device
 *
 ******************************************************************************/
NTSTATUS
StartDevice(
	PDEVICE_OBJECT			  fdo,
	PCM_PARTIAL_RESOURCE_LIST ResourceListRaw,
	PCM_PARTIAL_RESOURCE_LIST ResourceList
	)
{
	U32								 i;
	U8								 BarIndex;
	U32 							 vector;
	KIRQL							 IrqL;
	BOOLEAN 						 intPresent;
	NTSTATUS						 status;
	KAFFINITY						 affinity;
	KINTERRUPT_MODE 				 mode;
	DEVICE_EXTENSION				*pdx;
	PCI_COMMON_CONFIG				 pciRegs;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR  ResourceRaw;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR  Resource;
	PKTHREAD thread;

	ULONG BusNumber, propertyAddress, length;
	USHORT FunctionNumber, DeviceNumber;

	pdx 		= fdo->DeviceExtension;
	intPresent	= FALSE;
	ResourceRaw = ResourceListRaw->PartialDescriptors;
	Resource	= ResourceList->PartialDescriptors;

	// Get the PCI base addresses
	PciConfigRegisterBufferRead (fdo, &pciRegs, 0, sizeof(PCI_COMMON_CONFIG));

/*	// Write 0x20, 0x80 to address 0x0c, 0x0d in PCI configuration register[S]
	i = 0x8020;
	PciConfigRegisterBufferWrite (fdo, &i, 0x0c, 2);
	// Write 0x20, 0x80 to address 0x0c, 0x0d in PCI configuration register[E]
*/	
	// Write 00 to address 0x40, 0x41 in PCI configuration register[S]
	i = 0;
	PciConfigRegisterBufferWrite (fdo, &i, 0x40, 2);
	// Write 00 to address 0x40, 0x41 in PCI configuration register[E]

	// Get PCI configuration register address[S]
	// Get the BusNumber. Please read the warning to follow.
	IoGetDeviceProperty (pdx->pPhysicalDeviceObject, DevicePropertyBusNumber, sizeof(ULONG), (PVOID)&BusNumber, &length);

	// Get the DevicePropertyAddress
	IoGetDeviceProperty (pdx->pPhysicalDeviceObject, DevicePropertyAddress, sizeof(ULONG), (PVOID)&propertyAddress, &length);

	// For PCI, the DevicePropertyAddress has device number 
	// in the high word and the function number in the low word. 
	FunctionNumber = (USHORT)((propertyAddress) &0x0000ffff);
	DeviceNumber = (USHORT)(((propertyAddress) >>16) &0x0000ffff);

	KdPrint((DBG_NAME "BusNumber: %d, DeviceNumber: %d, FunctionNumber: %d\n", BusNumber, DeviceNumber, FunctionNumber));
	// Get PCI configuration register address[E]

	KdPrint((DBG_NAME "Resource list contains %d descriptors\n",
			 ResourceListRaw->Count));

	for (i = 0; i < ResourceListRaw->Count; ++i, ++Resource, ++ResourceRaw)
	{
		KdPrint(("			  Resource %02d\n", i));

		switch (ResourceRaw->Type)
		{
			case CmResourceTypeInterrupt:
				intPresent = TRUE;
				IrqL	   = (KIRQL) Resource->u.Interrupt.Level;
				vector	   = Resource->u.Interrupt.Vector;
				affinity   = Resource->u.Interrupt.Affinity;

				KdPrint(("				Type	: Interrupt\n"
						 "				Vector	: 0x%02x		(Translated = 0x%02x)\n"
						 "				IRQL	: 0x%02x		(Translated = 0x%02x)\n"
						 "				Affinity: 0x%08x  (Translated = 0x%08x)\n",
						 ResourceRaw->u.Interrupt.Vector, vector,
						 ResourceRaw->u.Interrupt.Level, IrqL,
						 ResourceRaw->u.Interrupt.Affinity, affinity));

				if (ResourceRaw->Flags == CM_RESOURCE_INTERRUPT_LATCHED)
				{
					mode = Latched;
					KdPrint(("				Mode	: Latched\n"));
				}
				else
				{
					mode = LevelSensitive;
					KdPrint(("				Mode	: Level Sensitive\n"));
				}
				break;

			case CmResourceTypePort:
				KdPrint(("				Type	 : I/O Port\n"
						 "				Address  : 0x%08x  (Translated = 0x%08x)\n"
						 "				Size	 : 0x%08x  ",
						 ResourceRaw->u.Port.Start.LowPart,
						 Resource->u.Port.Start.LowPart,
						 ResourceRaw->u.Port.Length));

				if (ResourceRaw->u.Port.Length >= (1 << 10))
					KdPrint(("(%d Kb)\n",
							 ResourceRaw->u.Port.Length >> 10));
				else
					KdPrint(("(%d Bytes)\n",
							 ResourceRaw->u.Port.Length));

				BarIndex = GetBarIndex(
							   ResourceRaw->u.Port.Start,
							   &pciRegs
							   );

				if (BarIndex != (U8)-1)
				{
					pdx->PciBar[BarIndex].Physical = ResourceRaw->u.Port.Start;
					pdx->PciBar[BarIndex].Size	   = ResourceRaw->u.Port.Length;

					// Check if we need to map (usually on RISC platforms)
					if (ResourceRaw->Flags & CM_RESOURCE_PORT_MEMORY)
					{
						pdx->PciBar[BarIndex].IsIoMapped = FALSE;

						KdPrint(("				Kernel VA: "));

						status = S6010_PciBarSpaceMap(
									 pdx,
									 BarIndex
									 );

						if ( NT_SUCCESS(status) )
						{
							KdPrint(("0x%08x\n", pdx->PciBar[BarIndex].pVa));
						}
						else
						{
							KdPrint(("ERROR - Unable to map 0x%08x ==> Kernel VA\n",
									 ResourceRaw->u.Port.Start.LowPart));
						}
					}
					else
					{
						pdx->PciBar[BarIndex].pVa			 = (U32*)(U32)-1;
						pdx->PciBar[BarIndex].IsUserMappable = FALSE;
						pdx->PciBar[BarIndex].IsIoMapped	 = TRUE;
					}
				}
				break;

			case CmResourceTypeMemory:
				KdPrint(("				Type	 : Memory Space\n"
						 "				Address  : 0x%08x  (Translated = 0x%08x)\n"
						 "				Size	 : 0x%08x  ",
						 ResourceRaw->u.Memory.Start.LowPart,
						 Resource->u.Memory.Start.LowPart,
						 ResourceRaw->u.Memory.Length));

				if (ResourceRaw->u.Memory.Length >= (1 << 10))
					KdPrint(("(%d Kb)\n",
							 ResourceRaw->u.Memory.Length >> 10));
				else
					KdPrint(("(%d Bytes)\n",
							 ResourceRaw->u.Memory.Length));

				BarIndex = GetBarIndex(
							   ResourceRaw->u.Memory.Start,
							   &pciRegs
							   );

				if (BarIndex != (U8)-1)
				{
					KdPrint(("				Kernel VA: "));

					// Record resources
					pdx->PciBar[BarIndex].Physical	  = ResourceRaw->u.Memory.Start;
					pdx->PciBar[BarIndex].Size		  = ResourceRaw->u.Memory.Length;
					pdx->PciBar[BarIndex].IsIoMapped  = FALSE;

					status = S6010_PciBarSpaceMap(
								 pdx,
								 BarIndex
								 );

					if ( NT_SUCCESS(status) )
					{
						KdPrint(("0x%08x\n", pdx->PciBar[BarIndex].pVa));
					}
					else
					{
						KdPrint(("ERROR - Unable to map 0x%08x ==> Kernel VA\n",
								 ResourceRaw->u.Memory.Start.LowPart));

						// PCI BAR 0 is required for register access
						if (BarIndex == 0)
						{
							KdPrint((DBG_NAME "ERROR - BAR 0 mapping is required\n"));
							return STATUS_INSUFFICIENT_RESOURCES;
						}
					}
				}
				break;

			case CmResourceTypeNull:
				KdPrint(("				Type: Null (unsupported)\n"));
				break;

			case CmResourceTypeDma:
				KdPrint(("				Type: DMA (unsupported)\n"));
				break;

			case CmResourceTypeDeviceSpecific:
				KdPrint(("				Type: Device Specific (unsupported)\n"));
				break;

			case CmResourceTypeBusNumber:
				KdPrint(("				Type: Bus Number (unsupported)\n"));
				break;

			// NonArbitrated & ConfigData are currently #defined as the same number
			case CmResourceTypeConfigData:
				KdPrint(("				Type: Non-Arbitrated or Config Data (unsupported)\n"));
				break;

			case CmResourceTypeDevicePrivate:
				KdPrint(("				Type: Device Private Data (unsupported)\n"));
				break;

			case CmResourceTypePcCardConfig:
				KdPrint(("				Type: PC Card Configuration (unsupported)\n"));
				break;
			case CmResourceTypeMfCardConfig:
				KdPrint(("			    Type: Multi-function Card Configuration (unsupported)\n"));
				break;
			default:
				KdPrint(("				Type: ?Unknown Resource Type?\n"));
				break;
		}
	}

	// Make sure BAR 0 exists or the device can't be started
	if ((U32)(pdx->PciBar[0].pVa) == (U32)-1)
	{
		KdPrint((DBG_NAME "ERROR - BAR 0 address not configured, unable to load driver\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

#if __NUM_SOLO_CHIPS > 1
	if (glNumS6010 == 0)
	{
		for (i=0; i<NUM_MAX_S6010; i++)
		{
			glIAboutRegIO.bufIdxOldAccS6010[i] = -1;
			KeInitializeSpinLock (&glIAboutRegIO.bufKSPLockRegIO[i]);
		}

		KeInitializeSpinLock (&glISetUpInt.kSPLockForISR);
		glISetUpInt.numMaxIrqL = 0;
	}

	pdx->iS6010.idxID = glNumS6010;
	glNumS6010++;

	glIAboutRegIO.bufAddrPhyBaseAddr[pdx->iS6010.idxID] = pdx->PciBar[0].Physical;
	KdPrint((DBG_NAME "NumCreate %d / %d [%08x:%08x]\n", pdx->iS6010.idxID, __NUM_SOLO_CHIPS,
		(U32)pdx->PciBar[0].Physical.HighPart, (U32)pdx->PciBar[0].Physical.LowPart));

	if (intPresent)
	{
		S6010_DisableAllInterrupt (pdx);

		if (IrqL > glISetUpInt.numMaxIrqL)
		{
			glISetUpInt.numMaxIrqL = IrqL;
		}

		glISetUpInt.bufPdx[pdx->iS6010.idxID] = pdx;
		glISetUpInt.bufVector[pdx->iS6010.idxID] = vector;
		glISetUpInt.bufIrqL[pdx->iS6010.idxID] = IrqL;
		glISetUpInt.bufMode[pdx->iS6010.idxID] = mode;
		glISetUpInt.bufAffinity[pdx->iS6010.idxID] = affinity;

		if (glNumS6010 == __NUM_SOLO_CHIPS)
		{
			for (i=0; i<__NUM_SOLO_CHIPS; i++)
			{
				status = IoConnectInterrupt (
							 &glISetUpInt.bufPdx[i]->pInterruptObject,
							 OnInterrupt,
							 glISetUpInt.bufPdx[i],
							 &glISetUpInt.kSPLockForISR,
							 glISetUpInt.bufVector[i],
							 glISetUpInt.bufIrqL[i],
							 glISetUpInt.numMaxIrqL,
							 glISetUpInt.bufMode[i],
							 TRUE,
							 glISetUpInt.bufAffinity[i],
							 FALSE
							 );

				if ( !NT_SUCCESS(status) )
				{
					KdPrint((DBG_NAME "ERROR - IoConnectInterrupt() failed, status = 0x%08x\n", status));

					glISetUpInt.bufPdx[i]->pInterruptObject = NULL;
				}
				else
				{
					KdPrint((DBG_NAME "Connected to interrupt vector\n"));
				}
			}
		}
	}
	else
	{
		KdPrint((DBG_NAME "No interrupt found\n"));
		pdx->pInterruptObject = NULL;
	}
#else
	if (intPresent)
	{
		// Disable the PCI interrupt
		S6010_DisableAllInterrupt (pdx);

		status = IoConnectInterrupt(
					 &pdx->pInterruptObject,
					 OnInterrupt,
					 pdx,
					 NULL,
					 vector,
					 IrqL,
					 IrqL,
					 mode,
					 TRUE,
					 affinity,
					 FALSE
					 );

		if ( !NT_SUCCESS(status) )
		{
			KdPrint((DBG_NAME "ERROR - IoConnectInterrupt() failed, status = 0x%08x\n", status));

			pdx->pInterruptObject = NULL;
		}
		else
		{
			KdPrint((DBG_NAME "Connected to interrupt vector\n"));
		}
	}
	else
	{
		KdPrint((DBG_NAME "No interrupt found\n"));
		pdx->pInterruptObject = NULL;
	}
#endif


	// Record the Vendor and Device ID
	pdx->Device.VendorId = pciRegs.VendorID;
	pdx->Device.DeviceId = pciRegs.DeviceID;

	// Get the bus number and the slot number
	status = GetBusSlotNumber(
				 pdx->pPhysicalDeviceObject,
				 pdx
				 );
	if (!NT_SUCCESS(status))
	{
		KdPrint((DBG_NAME "WARNING - Unable to get bus and slot number\n"));
	}

	// SOLO6010 init[S]
	for (i=0; i<NUM_EVENT; i++)	// Event Initialize
	{
		KeInitializeEvent (&(pdx->bufEvent[i]), NotificationEvent, FALSE);
	}

	for (i=0; i<NUM_THREAD; i++)
	{
		pdx->bufBEndThread[i] = 0;
	}
	PsCreateSystemThread (&thread, THREAD_ALL_ACCESS, NULL, NULL, NULL,	KE_VID_ENC_DATA_READ_THREAD, pdx);
	PsCreateSystemThread (&thread, THREAD_ALL_ACCESS, NULL, NULL, NULL,	KE_G723_ENC_READ_DEC_WRITE_THREAD, pdx);
	PsCreateSystemThread (&thread, THREAD_ALL_ACCESS, NULL, NULL, NULL,	KE_LIVE_REFRESH_THREAD, pdx);
	PsCreateSystemThread (&thread, THREAD_ALL_ACCESS, NULL, NULL, NULL,	KE_TEST_THREAD, pdx);
	PsCreateSystemThread (&thread, THREAD_ALL_ACCESS, NULL, NULL, NULL,	KE_MP4_DEC_FILL_BUF_THREAD, pdx);
	PsCreateSystemThread (&thread, THREAD_ALL_ACCESS, NULL, NULL, NULL,	KE_UART_0_THREAD, pdx);
	PsCreateSystemThread (&thread, THREAD_ALL_ACCESS, NULL, NULL, NULL,	KE_UART_1_THREAD, pdx);

	// Fix JPEG enc flag(for preventing JPEG HW bug)
	if (VMEM_JPEG_CODE_SIZE != 0)
	{
		KeStallExecutionProcessor (5);
		PLX_REG_WRITE(pdx, REG_VE_JPEG_CTRL, 0xffffffff);	// JPEG On
		KeStallExecutionProcessor (5);
	}
	else
	{
		KeStallExecutionProcessor (5);
		PLX_REG_WRITE(pdx, REG_VE_JPEG_CTRL, 0);	// JPEG Off
		KeStallExecutionProcessor (5);
	}

	KdPrint((DBG_NAME "SOLO6010 Init OK\n"));
	// SOLO6010 init[E]

	return STATUS_SUCCESS;
}

/******************************************************************************
 *
 * Function   :  StopDevice
 *
 * Description:  Stop a device
 *
 ******************************************************************************/
VOID StopDevice (PDEVICE_OBJECT fdo)
{
	DEVICE_EXTENSION *pdx;
	int i;

	pdx = fdo->DeviceExtension;

	for (i=0; i<NUM_THREAD; i++)
	{
		pdx->bufBEndThread[i] = 1;
	}
	for (i=IDX_EVT_TH_VID_ENC_DATA_READ; i<=IDX_EVT_TH_UART_1; i++)
	{
		KeSetEvent (&pdx->bufEvent[i], EVENT_INCREMENT, FALSE);
	}
	for (i=0; i<NUM_THREAD; i++)
	{
		while (1)
		{
			if (pdx->bufBEndThread[i] == 0)
			{
				break;
			}
			S6010_Sleep_1MS ();
		}
	}

	// Free all interrupt resources
	if (pdx->pInterruptObject != NULL)
	{
		IoDisconnectInterrupt (pdx->pInterruptObject);

		pdx->pInterruptObject = NULL;
	}

#if __NUM_SOLO_CHIPS > 1
	glNumS6010--;
	pdx->iS6010.idxID = 0;

	KdPrint((DBG_NAME "NumCreate Decrease: %d\n", glNumS6010));
#endif

	S6010_PciBarResourcesUnmap (pdx);
}




/******************************************************************************
 *
 * Function   :  LockDevice
 *
 * Description:  Lock a device for operation, return FALSE if device is being
 *				 removed.
 *
 ******************************************************************************/
BOOLEAN LockDevice (DEVICE_EXTENSION *pdx)
{
	S32 usage;


	// Increment use count on our device object
	usage = InterlockedIncrement(
				&pdx->usage
				);

	KdPrint(("\n"
			 DBG_NAME "LOCKING..... (usage = %d)\n", usage));

	/*
	   If device is about to be removed, restore the use count and return FALSE.
	   If a race exists with HandleRemoveDevice (maybe running on another CPU),
	   the sequence we've followed is guaranteed to avoid a mistaken deletion of
	   the device object. If we test "removing" after HandleRemoveDevice sets
	   it, we'll restore the use count and return FALSE. In the meantime, if
	   HandleRemoveDevice decremented count to 0 before we did our increment,
	   its thread will have set the remove event. Otherwise, we'll decrement to
	   0 and set the event. Either way, HandleRemoveDevice will wake up to
	   finish removing the device, and we'll return FALSE to our caller.

	   If, on the other hand, we test "removing" before HandleRemoveDevice sets
	   it, we'll have already incremented the use count past 1 and will return
	   TRUE. Our caller will eventually call UnlockDevice, which will decrement
	   the use count and might set the event HandleRemoveDevice is waiting on at
	   that point.
	*/
	if (pdx->removing)
	{
		// Removing device
		if (InterlockedDecrement(&pdx->usage) == 0)
		{
			KeSetEvent(
				&pdx->evRemove,
				0,
				FALSE
				);
		}

		return FALSE;
	}

	return TRUE;
}




/******************************************************************************
 *
 * Function   :  UnlockDevice
 *
 * Description:  Unlock a device.
 *
 ******************************************************************************/
VOID
UnlockDevice(
	DEVICE_EXTENSION *pdx
	)
{
	S32 usage;


	usage = InterlockedDecrement(
				&pdx->usage
				);

	KdPrint((DBG_NAME "UNLOCKING... (usage = %d)\n", usage));

	if (usage == 0)
	{
		// Removing device
		KeSetEvent(
			&pdx->evRemove,
			0,
			FALSE
			);
	}
}
