#ifndef __DRIVER_DEFS_H
#define __DRIVER_DEFS_H

#include "Solo6010_DrvGlobal.h"

#pragma warning( disable : 4090 )
#pragma warning( disable : 4024 )

#ifdef __cplusplus
extern "C" {
#endif


// Definitions
#define MAX_NAME_LENGTH	0x20  // Max length of registered device name
#define PCI_NUM_BARS	7     // Total number of PCI BAR registers

#define PLX_REG_READ(pdx, offset)                          \
    READ_REGISTER_ULONG(                                   \
        (volatile ULONG *)(((U32)(pdx)->PciBar[0].pVa) + (offset))  \
        )

#define PLX_REG_WRITE(pdx, offset, value)                  \
    WRITE_REGISTER_ULONG(                                  \
        (volatile ULONG *)(((U32)(pdx)->PciBar[0].pVa) + (offset)), \
        value                                              \
        )

// PCI BAR Space information
typedef struct _PCI_BAR_INFO
{
    U32              *pVa;                        // BAR Kernel Addresses
    PHYSICAL_ADDRESS  Physical;                   // BAR Physical Addresses
    U32               Size;                       // BAR size
    BOOLEAN           IsIoMapped;                 // Memory or I/O mapped?
    BOOLEAN           IsUserMappable;             // Can this BAR be mapped?
    PMDL              pMdl;                       // MDL for the BAR space
} PCI_BAR_INFO;

// Device Extension structure
typedef struct _DEVICE_EXTENSION
{
    PDEVICE_OBJECT      pDeviceObject;            // Device object this extension belongs to
    PDEVICE_OBJECT      pLowerDeviceObject;
    PDEVICE_OBJECT      pPhysicalDeviceObject;
    S32                 usage;
    KEVENT              evRemove;
    BOOLEAN             removing;
    DEVICE_POWER_STATE  Power;

    // Device information
    DEVICE_LOCATION     Device;
    PCI_BAR_INFO        PciBar[PCI_NUM_BARS];

    // Interrupt handling variables
    PKINTERRUPT         pInterruptObject;
    KDPC bufFuncDPC[NUM_DPC];

    NTSTATUS            NTStatus;
    WCHAR               DeviceLinkNameBuffer[MAX_NAME_LENGTH];

	// doo add[S]
	U8 bufBEndThread[NUM_THREAD];
	KEVENT bufEvent[NUM_EVENT];
    FAST_MUTEX bufFMutex[NUM_FAST_MUTEX];
	KSPIN_LOCK bufKSpLock[NUM_K_SPLOCK];


	INFO_S6010 iS6010;
	INFO_DDRAW iDDraw;

	INFO_LIVE iLive;
	INFO_REC iRec;
	INFO_UART_BUF bufI_UART[NUM_UART_PORT];

	INFO_P2M_BUF bufInfoP2M[NUM_P2M_BUF];

	STAT_VIDEO_ENC statVidEnc;
	INFO_CBR infoMP4_CBR[NUM_VID_TOTAL_ENC_CH];
	U8 bufMP4_GOP[NUM_VID_TOTAL_ENC_CH];	// for GOP reset when record start

	U32 addrRegIO;
	U32 valRegIO;

	U32 curIntStat;

	INFO_SHARE_DATA *pIShare;
	PMDL pMDL_iShare;

	STAT_P2M statP2M[NUM_P2M];
	// doo add[E]

} DEVICE_EXTENSION;


typedef struct _POWER_INFO
{
    DEVICE_EXTENSION *pdx;
    DEVICE_POWER_STATE state;
} POWER_INFO;


NTKERNELAPI PVOID MmAllocateContiguousMemory (IN ULONG NumberOfBytes,
	IN PHYSICAL_ADDRESS HighestAcceptableAddress);
NTKERNELAPI VOID MmFreeContiguousMemory (IN PVOID BaseAddress);

NTKERNELAPI PVOID MmAllocateContiguousMemorySpecifyCache (IN SIZE_T NumberOfBytes,
	IN PHYSICAL_ADDRESS LowestAcceptableAddress,
	IN PHYSICAL_ADDRESS HighestAcceptableAddress,
	IN PHYSICAL_ADDRESS BoundaryAddressMultiple OPTIONAL,
	IN MEMORY_CACHING_TYPE CacheType);
NTKERNELAPI VOID MmFreeContiguousMemorySpecifyCache (IN PVOID BaseAddress,
	IN SIZE_T NumberOfBytes,
	IN MEMORY_CACHING_TYPE CacheType);

NTKERNELAPI PHYSICAL_ADDRESS MmGetPhysicalAddress (IN PVOID BaseAddress);
NTKERNELAPI PVOID MmMapLockedPagesSpecifyCache (IN PMDL MemoryDescriptorList,
	IN KPROCESSOR_MODE AccessMode,
	IN MEMORY_CACHING_TYPE CacheType,
	IN PVOID BaseAddress,
	IN ULONG BugCheckOnFailure,
	IN MM_PAGE_PRIORITY Priority);

#ifdef __cplusplus
}
#endif

#endif
