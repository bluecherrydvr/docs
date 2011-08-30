#include <Wdm.h>
#include "Dispatch.h"
#include "Driver.h"
#include "PlugPlay.h"
#include "SupportFunc.h"


/**************************************************************
 * The following IRP minor code is erroneously not included
 * in the Win2000 DDK "wdm.h" file.  It is, however, included
 * in the "ntddk.h" file.  Until this is fixed by Microsoft,
 * the definition is provided here so the driver will build.
 *************************************************************/
#if !defined(IRP_MN_QUERY_LEGACY_BUS_INFORMATION)
    #define IRP_MN_QUERY_LEGACY_BUS_INFORMATION        0x18
#endif




/******************************************************************************
 *
 * Function   :  DispatchPnp
 *
 * Description:  Handle PnP requests
 *
 ******************************************************************************/
NTSTATUS
DispatchPnp(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    BOOLEAN            Unlock;
    NTSTATUS           status;
    PIO_STACK_LOCATION stack;


    if (!LockDevice(fdo->DeviceExtension))
    {
        return S6010_CompleteIrpWithInformation(
                   pIrp,
                   STATUS_DELETE_PENDING,
                   0
                   );
    }

    KdPrint((DBG_NAME "Received PNP Message (IRP=0x%08x) ==> ", pIrp));

    Unlock = TRUE;

    stack = IoGetCurrentIrpStackLocation(
                pIrp
                );

    // Check minor code
    switch (stack->MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            KdPrint(("IRP_MN_START_DEVICE\n"));
            status = HandleStartDevice(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_STOP_DEVICE:
            KdPrint(("IRP_MN_STOP_DEVICE\n"));
            status = HandleStopDevice(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_REMOVE_DEVICE:
            KdPrint(("IRP_MN_REMOVE_DEVICE\n"));
            Unlock = FALSE;
            status = HandleRemoveDevice(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_REMOVE_DEVICE:
            KdPrint(("IRP_MN_QUERY_REMOVE_DEVICE\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
            KdPrint(("IRP_MN_CANCEL_REMOVE_DEVICE\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_STOP_DEVICE:
            KdPrint(("IRP_MN_QUERY_STOP_DEVICE\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_CANCEL_STOP_DEVICE:
            KdPrint(("IRP_MN_CANCEL_STOP_DEVICE\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            KdPrint(("IRP_MN_QUERY_DEVICE_RELATIONS\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_INTERFACE:
            KdPrint(("IRP_MN_QUERY_INTERFACE\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_CAPABILITIES:
            KdPrint(("IRP_MN_QUERY_CAPABILITIES\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_RESOURCES:
            KdPrint(("IRP_MN_QUERY_RESOURCES\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            KdPrint(("IRP_MN_QUERY_RESOURCE_REQUIREMENTS\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_DEVICE_TEXT:
            KdPrint(("IRP_MN_QUERY_DEVICE_TEXT\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            KdPrint(("IRP_MN_FILTER_RESOURCE_REQUIREMENTS\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_READ_CONFIG:
            KdPrint(("IRP_MN_READ_CONFIG\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_WRITE_CONFIG:
            KdPrint(("IRP_MN_WRITE_CONFIG\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_EJECT:
            KdPrint(("IRP_MN_EJECT\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_SET_LOCK:
            KdPrint(("IRP_MN_SET_LOCK\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_ID:
            KdPrint(("IRP_MN_QUERY_ID\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            KdPrint(("IRP_MN_QUERY_PNP_DEVICE_STATE\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_BUS_INFORMATION:
            KdPrint(("IRP_MN_QUERY_BUS_INFORMATION\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            KdPrint(("IRP_MN_DEVICE_USAGE_NOTIFICATION\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_SURPRISE_REMOVAL:
            KdPrint(("IRP_MN_SURPRISE_REMOVAL\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
            KdPrint(("IRP_MN_QUERY_LEGACY_BUS_INFORMATION\n"));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;


        default:
            KdPrint(("Unsupported IRP_MN_Xxx (0x%08x)\n", stack->MinorFunction));
            status = DefaultPnpHandler(
                         fdo,
                         pIrp
                         );
            break;
    }

    if (Unlock == TRUE)
    {
        UnlockDevice(
            fdo->DeviceExtension
            );
    }

    return status;
}




/******************************************************************************
 *
 * Function   :  DefaultPnpHandler
 *
 * Description:  Handle standard PnP requests
 *
 ******************************************************************************/
NTSTATUS
DefaultPnpHandler(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    IoSkipCurrentIrpStackLocation(
        pIrp
        );

    return IoCallDriver(
               ((DEVICE_EXTENSION *)fdo->DeviceExtension)->pLowerDeviceObject,
               pIrp
               );
}




/******************************************************************************
 *
 * Function   :  HandleRemoveDevice
 *
 * Description:  Handle the IRP_MN_REMOVE_DEVICE PnP request
 *
 ******************************************************************************/
NTSTATUS
HandleRemoveDevice(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    NTSTATUS          status;
    DEVICE_EXTENSION *pdx;


    // Wait for any pending I/O operations to complete

    pdx           = fdo->DeviceExtension;
    pdx->removing = TRUE;

    UnlockDevice(pdx);            // once for LockDevice at start of dispatch
    UnlockDevice(pdx);            // once for initialization during AddDevice

    KeWaitForSingleObject(
        &pdx->evRemove,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    // Do any processing required for *us* to remove the device. This
    // would include completing any outstanding requests, etc.

    StopDevice(
        fdo
        );

    // Let lower-level drivers handle this request & ignore the result
    status = DefaultPnpHandler(
                 fdo,
                 pIrp
                 );

    // Remove the device object
    RemoveDevice(
        fdo
        );

    return status;
}




/******************************************************************************
 *
 * Function   :  HandleStartDevice
 *
 * Description:  Handle the IRP_MN_START_DEVICE PnP request
 *
 ******************************************************************************/
NTSTATUS
HandleStartDevice(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    NTSTATUS           status;
    PIO_STACK_LOCATION stack;


    /*
        First let all lower-level drivers handle this request. In this
        particular sample, the only lower-level driver should be the physical
        device created by the bus driver, but there could theoretically be any
        number of intervening bus filter devices. Those drivers may need to do
        some setup at this point in time before they'll be ready to handle
        non-PnP IRP's.
    */

    status = ForwardAndWait(
                 fdo,
                 pIrp
                 );
    if (!NT_SUCCESS(status))
    {
        return S6010_CompleteIrp(
                   pIrp,
                   status
                   );
    }

    stack = IoGetCurrentIrpStackLocation(
                pIrp
                );

    // Assign resources to the devices
    status = StartDevice(
                 fdo,
                 &stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList,
                 &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList
                 );

    return S6010_CompleteIrpWithInformation(
               pIrp,
               status,
               0
               );
}




/******************************************************************************
 *
 * Function   :  HandleStopDevice
 *
 * Description:  Handle the IRP_MN_STOP_DEVICE PnP request
 *
 ******************************************************************************/
NTSTATUS
HandleStopDevice(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    NTSTATUS status;


    status = DefaultPnpHandler(
                 fdo,
                 pIrp
                 );

    StopDevice(
        fdo
        );

    return status;
}
