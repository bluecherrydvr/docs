#ifndef __PLUGPLAY_H
#define __PLUGPLAY_H

#include "TypesDef.h"


#ifdef __cplusplus
extern "C" {
#endif



/**********************************************
*               Functions
**********************************************/
NTSTATUS
DispatchPnp(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS
DefaultPnpHandler(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS
HandleRemoveDevice(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS
HandleStartDevice(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );

NTSTATUS
HandleStopDevice(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    );



#ifdef __cplusplus
}
#endif

#endif
