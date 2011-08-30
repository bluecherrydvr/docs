#ifndef __DISPATCH_H
#define __DISPATCH_H

#include "DriverDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************
*               Functions
**********************************************/
NTSTATUS
DispatchCreate(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS
DispatchCleanup(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS 
DispatchClose(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS 
DispatchRead(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS 
DispatchWrite(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS
DispatchSystemControl(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS
DispatchIoControl(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );




#ifdef __cplusplus
}
#endif

#endif
