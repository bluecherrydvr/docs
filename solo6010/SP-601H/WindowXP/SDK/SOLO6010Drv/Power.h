#ifndef __POWER_H
#define __POWER_H

#include "DriverDefs.h"

#ifdef __cplusplus
extern "C" {
#endif



/**********************************************
*               Functions
**********************************************/
NTSTATUS
DispatchPower(
    PDEVICE_OBJECT fdo,
    PIRP           Irp
    );

NTSTATUS 
DefaultPowerHandler(
    PDEVICE_OBJECT fdo,
    PIRP           Irp
    );

NTSTATUS 
HandleSetPower(
    PDEVICE_OBJECT fdo,
    PIRP           Irp
    );

NTSTATUS 
HandleQueryPower(
    PDEVICE_OBJECT fdo,
    PIRP           Irp
    );

NTSTATUS 
OnFinishPowerUp(
    PDEVICE_OBJECT fdo,
    PIRP           Irp,
    PVOID          junk
    );

VOID 
OnPowerRequestComplete(
    PDEVICE_OBJECT   DeviceObject,
    U8               MinorFunction,
    POWER_STATE      PowerState, 
    PVOID            context,
    PIO_STATUS_BLOCK ioStatus
    );

NTSTATUS
SendDeviceSetPower(
    PDEVICE_OBJECT     fdo,
    DEVICE_POWER_STATE state,
    U32                context
    );

VOID        
SetPowerState(
    PDEVICE_OBJECT     fdo, 
    DEVICE_POWER_STATE state
    );

BOOLEAN 
EmpowerDevice(
    POWER_INFO *pPowerInfo
    );

NTSTATUS
SendSelfSetPowerRequest(
    PDEVICE_OBJECT     fdo,
    DEVICE_POWER_STATE state
    );



#ifdef __cplusplus
}
#endif

#endif
