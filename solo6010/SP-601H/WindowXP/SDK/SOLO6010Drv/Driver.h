#ifndef __DRIVER_H
#define __DRIVER_H

#include "DriverDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************
*               Functions
**********************************************/
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT  pDriverObject,
    PUNICODE_STRING pRegistryPath
    );

VOID
DriverUnload(
    PDRIVER_OBJECT pDriverObject
    );

#if defined(WDM_DRIVER)
NTSTATUS
AddDevice(
    PDRIVER_OBJECT pDriverObject,
    PDEVICE_OBJECT pdo
    );
#else
NTSTATUS
AddDevice(
    PDRIVER_OBJECT   pDriverObject,
    DEVICE_LOCATION *pDeviceInfo
    );
#endif

VOID
RemoveDevice(
    PDEVICE_OBJECT fdo
    );

NTSTATUS
StartDevice(
    PDEVICE_OBJECT            fdo,
    PCM_PARTIAL_RESOURCE_LIST ResourceListRaw,
    PCM_PARTIAL_RESOURCE_LIST ResourceList
    );

VOID
StopDevice(
    PDEVICE_OBJECT fdo
    );

BOOLEAN
LockDevice(
    DEVICE_EXTENSION *pdx
    );

VOID
UnlockDevice(
    DEVICE_EXTENSION *pdx
    );

#ifdef __cplusplus
}
#endif

#endif
