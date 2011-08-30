#include <Wdm.h>
#include "Driver.h"
#include "Power.h"
#include "SupportFunc.h"




/******************************************************************************
 *
 * Function   :  DispatchPower
 *
 * Description:  Handles power requests
 *
 ******************************************************************************/
NTSTATUS
DispatchPower(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    NTSTATUS           status;
    PIO_STACK_LOCATION stack;


    LockDevice(
        fdo->DeviceExtension
        );

    KdPrint((DBG_NAME "Received POWER Message (IRP=0x%08x) ==> ", pIrp));

    stack = IoGetCurrentIrpStackLocation(
                pIrp
                );

    switch (stack->MinorFunction)
    {
        case IRP_MN_WAIT_WAKE:
            KdPrint(("IRP_MN_WAIT_WAKE\n"));
            status = DefaultPowerHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_POWER_SEQUENCE:
            KdPrint(("IRP_MN_POWER_SEQUENCE\n"));
            status = DefaultPowerHandler(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_SET_POWER:
            KdPrint(("IRP_MN_SET_POWER\n"));
            if (stack->Parameters.Power.Type == SystemPowerState)
            {
                KdPrint((DBG_NAME "SystemContext = 0x%08x  SystemPowerState = 0x%08x\n",
                         stack->Parameters.Power.SystemContext,
                         stack->Parameters.Power.State.SystemState));
            }
            else
            {
                KdPrint((DBG_NAME "SystemContext = 0x%08x  DevicePowerState = 0x%08x\n",
                         stack->Parameters.Power.SystemContext,
                         stack->Parameters.Power.State.DeviceState));
            }
            status = HandleSetPower(
                         fdo,
                         pIrp
                         );
            break;

        case IRP_MN_QUERY_POWER:
            KdPrint(("IRP_MN_QUERY_POWER\n"));
            KdPrint((DBG_NAME "SystemContext = 0x%08x",
                     stack->Parameters.Power.SystemContext));
            status = HandleQueryPower(
                         fdo,
                         pIrp
                         );
            break;

        default:
            KdPrint(("Unsupported IRP_MN_Xxx (0x%08x)\n", stack->MinorFunction));
            status = DefaultPowerHandler(
                         fdo,
                         pIrp
                         );
            break;
    }

    UnlockDevice(
        fdo->DeviceExtension
        );

    return status;
}




/******************************************************************************
 *
 * Function   :  DefaultPowerHandler
 *
 * Description:  Handle defaults power requests
 *
 ******************************************************************************/
NTSTATUS
DefaultPowerHandler(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    PoStartNextPowerIrp(           // Must be done while we own the IRP
        pIrp
        );

    IoSkipCurrentIrpStackLocation(
        pIrp
        );

    return PoCallDriver(
               ((DEVICE_EXTENSION *)fdo->DeviceExtension)->pLowerDeviceObject,
               pIrp
               );
}




/******************************************************************************
 *
 * Function   :  HandleSetPower
 *
 * Description:  Handles IRP_MN_SET_POWER and IRP_MN_QUERY_POWER requests
 *
 ******************************************************************************/
NTSTATUS
HandleSetPower(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    U32                 context;
    NTSTATUS            status;
    POWER_STATE         state;
    POWER_STATE_TYPE    type;
    DEVICE_EXTENSION   *pdx;
    PIO_STACK_LOCATION  stack;
    DEVICE_POWER_STATE  devstate;


    pdx     = fdo->DeviceExtension;
    stack   = IoGetCurrentIrpStackLocation(pIrp);
    context = stack->Parameters.Power.SystemContext;
    type    = stack->Parameters.Power.Type;
    state   = stack->Parameters.Power.State;

    /* If this IRP pertains to a system state, we want to put ourselves into
     * a corresponding device state. The rule here is very simple: if the
     * system is below SystemWorking, me must go to D3. If the system is
     * in SystemWorking, we're allowed to pick our own state (so we pick
     * D0). We can't just change our power state now, though: we must send
     * ourselves a separate POWER IRP with the selected device state.
     */
    if (type == SystemPowerState)
    {
        if (state.SystemState <= PowerSystemWorking)
            devstate = PowerDeviceD0;
        else
            devstate = PowerDeviceD3;
    }
    else
        devstate = state.DeviceState;

    // If we're adding power, first pass the IRP down. Queue the
    // requisite device IRP in the completion routine after the
    // lower layers have restored power.

    if (devstate < pdx->Power)
    {
        // Adding more power
        IoCopyCurrentIrpStackLocationToNext(
            pIrp
            );

        IoSetCompletionRoutine(
            pIrp,
            (PIO_COMPLETION_ROUTINE) OnFinishPowerUp,
            NULL,
            TRUE,
            TRUE,
            TRUE
            );

        return PoCallDriver(
                   pdx->pLowerDeviceObject,
                   pIrp
                   );
    }
    else if (devstate > pdx->Power)
    {
        /* If we're removing power, first do the device specific part.
         * Then send the request down the stack. In the case of a system
         * power request, send ourselves a device power IRP to power down first.
         * In the case of a device power request, use PoSetPowerState to tell
         * the Power Manager what we're about to do and then actually depower
         * the hardware.
         */

        // Removing power
        if (type == SystemPowerState)
        {
            // Change in system state
            status = SendDeviceSetPower(
                         fdo,
                         devstate,
                         context
                         );
            if (!NT_SUCCESS(status))
            {
                // Couldn't set device power state
                PoStartNextPowerIrp(
                    pIrp
                    );

                return S6010_CompleteIrpWithInformation(
                           pIrp,
                           status,
                           0
                           );
            }
        }
        else
        {
            // Change in device state
            PoSetPowerState(
                fdo,
                type,
                state
                ); // before we power down

            SetPowerState(
                fdo,
                devstate
                );
        }

        // Pass request down
        return DefaultPowerHandler(
                   fdo,
                   pIrp
                   );
    }

    // Pass request down
    return DefaultPowerHandler(
               fdo,
               pIrp
               );
}




/******************************************************************************
 *
 * Function   :  HandleQueryPower
 *
 * Description:  Handle the IRP_MN_QUERY_POWER power IRP.
 *
 ******************************************************************************/
NTSTATUS
HandleQueryPower(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp
    )
{
    U32                 context;
    POWER_STATE         state;
    POWER_STATE_TYPE    type;
    DEVICE_EXTENSION   *pdx;
    PIO_STACK_LOCATION  stack;
    DEVICE_POWER_STATE  devstate;


    pdx     = fdo->DeviceExtension;
    stack   = IoGetCurrentIrpStackLocation(pIrp);
    context = stack->Parameters.Power.SystemContext;
    type    = stack->Parameters.Power.Type;
    state   = stack->Parameters.Power.State;

    if (type == SystemPowerState)
    {
        if (state.SystemState <= PowerSystemWorking)
            devstate = PowerDeviceD0;
        else
            devstate = PowerDeviceD3;
    }
    else
        devstate = state.DeviceState;

    if (devstate - PowerDeviceD0 + D0 != D0)
    {
        // Unsupported power level, we must fail the IRP
        KdPrint((DBG_NAME "WARNING - HandleQueryPower() unsupported power level\n"));
        return S6010_CompleteIrpWithInformation(
                   pIrp,
                   STATUS_NOT_IMPLEMENTED,
                   0
                   );
    }

    // Pass request down
    return DefaultPowerHandler(
               fdo,
               pIrp
               );
}




/******************************************************************************
 *
 * Function   :  OnFinishPowerUp
 *
 * Description:  Completion routine for IRP_MN_SET_POWER
 *
 ******************************************************************************/
NTSTATUS
OnFinishPowerUp(
    PDEVICE_OBJECT fdo,
    PIRP           pIrp,
    PVOID          junk
    )
{
    NTSTATUS           status;
    POWER_STATE        state;
    POWER_STATE_TYPE   type;
    PIO_STACK_LOCATION stack;
    DEVICE_POWER_STATE devstate;


    KdPrint((DBG_NAME "in OnFinishPowerUp\n"));

    if (pIrp->PendingReturned)
        IoMarkIrpPending(pIrp);    // lower-level pended this IRP

    status = pIrp->IoStatus.Status;
    if (!NT_SUCCESS(status))
    {
        KdPrint((DBG_NAME "OnFinishPowerUp: IRP failed\n"));
        return status;
    }

    stack = IoGetCurrentIrpStackLocation(pIrp);
    type  = stack->Parameters.Power.Type;
    state = stack->Parameters.Power.State;

    /* If we just handled a request to restore power to the system, we
     * want to send ourselves a device power IRP. But don't do it unless
     * there's actually a request pending right now (we may immediately get
     * another system IRP to go to a different sleep state than we were in
     * to start with).
     */
    if (type == SystemPowerState)
    {
        // Restoring power to the system
        if (state.SystemState <= PowerSystemWorking)
            devstate = PowerDeviceD0;
        else
            devstate = PowerDeviceD3;

        status = SendDeviceSetPower(
                     fdo,
                     devstate,
                     stack->Parameters.Power.SystemContext
                     );
    }
    else
    {
        /* For a device re-power request, first restore power to the actual
         * device and then call PoSetPowerState to tell the Power Manager we
         * did so.
         */

        // Restoring power to the device
        SetPowerState(
            fdo,
            state.DeviceState
            );
        PoSetPowerState(
            fdo,
            type,
            state
            ); // after we power up
    }

    PoStartNextPowerIrp(pIrp);

    return status;
}




/******************************************************************************
 *
 * Function   :  OnPowerRequestComplete
 *
 * Description:  Completion routine for SendSelfSetPowerRequest
 *
 ******************************************************************************/
VOID
OnPowerRequestComplete(
    PDEVICE_OBJECT   DeviceObject,
    U8               MinorFunction,
    POWER_STATE      PowerState,
    PVOID            context,
    PIO_STATUS_BLOCK ioStatus
    )
{
    KdPrint((DBG_NAME "in OnPowerRequestComplete\n"));

    // Set event
    if ((PKEVENT)context != NULL)
    {
        KeSetEvent(
            (PKEVENT)context,
            1,
            FALSE
            );
    }
}




/******************************************************************************
 *
 * Function   :  SendDeviceSetPower
 *
 * Description:  Send ourselves an IRP_MN_SET_POWER request
 *
 ******************************************************************************/
NTSTATUS
SendDeviceSetPower(
    PDEVICE_OBJECT     fdo,
    DEVICE_POWER_STATE state,
    U32                context
    )
{
    PIRP               pIrp;
    KEVENT             event;
    NTSTATUS           status;
    PIO_STACK_LOCATION stack;


    KdPrint((DBG_NAME "in SendDeviceSetPower\n"));

    // Skip request if we are already at the desired Power level
    if (state == ((DEVICE_EXTENSION *)fdo->DeviceExtension)->Power)
        return STATUS_SUCCESS;

    pIrp = IoAllocateIrp(
               fdo->StackSize,
               FALSE
               );
    if (pIrp == NULL)
    {
        KdPrint((DBG_NAME "ERROR - Unable to allocate IRP for Power request\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    stack                                     = IoGetNextIrpStackLocation(pIrp);
    stack->MajorFunction                      = IRP_MJ_POWER;
    stack->MinorFunction                      = IRP_MN_SET_POWER;
    stack->Parameters.Power.SystemContext     = context;
    stack->Parameters.Power.Type              = DevicePowerState;
    stack->Parameters.Power.State.DeviceState = state;

    KeInitializeEvent(
        &event,
        NotificationEvent,
        FALSE
        );

    IoSetCompletionRoutine(
        pIrp,
        (PIO_COMPLETION_ROUTINE) OnRequestComplete,
        (PVOID) &event,
        TRUE,
        TRUE,
        TRUE
        );

    status = PoCallDriver(
                 fdo,
                 pIrp
                 );
    if (status == STATUS_PENDING)
    {
        // Wait for completion
        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = pIrp->IoStatus.Status;
    }

    IoFreeIrp(pIrp);

    return status;
}




/******************************************************************************
 *
 * Function   :  SetPowerState
 *
 * Description:  Set the new power state
 *
 ******************************************************************************/
VOID
SetPowerState(
    PDEVICE_OBJECT     fdo,
    DEVICE_POWER_STATE state
    )
{
    POWER_INFO  pPowerInfo;


    pPowerInfo.pdx   = fdo->DeviceExtension;
    pPowerInfo.state = state;

    EmpowerDevice(
        &pPowerInfo
        );
}




/******************************************************************************
 *
 * Function   :  EmpowerDevice
 *
 * Description:  Empower device
 *
 ******************************************************************************/
BOOLEAN
EmpowerDevice(
    POWER_INFO *pPowerInfo
    )
{
    // If already in the requested state, do nothing
    if (pPowerInfo->state == pPowerInfo->pdx->Power)
        return TRUE;

    KdPrint((DBG_NAME "Putting device into state ==> "));

    switch (pPowerInfo->state)
    {
        case D0Uninitialized:
            KdPrint(("D0Uninitialized\n"));
            break;

        case D0:
            KdPrint(("D0\n"));
            break;

        case D1:
            KdPrint(("D1\n"));
            break;

        case D2:
            KdPrint(("D2\n"));
            break;

        case D3Hot:
            KdPrint(("D3Hot\n"));
            break;

        case D3Cold:
            KdPrint(("D3Cold\n"));
            break;
    }

    /*
        A real device would interpret the new state relative to the old state
        and save (or restore) some context information, send control information
        to the device, etc. This fake device just records the power state in the
        device extension.
    */

    // Record the new power state
    pPowerInfo->pdx->Power = pPowerInfo->state;

    return TRUE;
}




#if 0
/******************************************************************************
 *
 * Function   :  SendSelfSetPowerRequest
 *
 * Description:  Send ourselves an IRP_MN_SET_POWER request.
 *               This is the standard method recommended by the WDM DDK. This
 *               does not seem to work yet, therefore this code is kept
 *               here in the #if 0 and has been replaced by older method.
 *
 ******************************************************************************/
NTSTATUS
SendSelfSetPowerRequest(
    PDEVICE_OBJECT     fdo,
    DEVICE_POWER_STATE state
    )
{
    KEVENT            event;
    NTSTATUS          status;
    NTSTATUS          waitStatus;
    POWER_STATE       poState;
    DEVICE_EXTENSION *pdx;


    KdPrint((DBG_NAME "in SendSelfSetPowerRequest()\n"));

    pdx = fdo->DeviceExtension;

    // Check if we are already at requested Power Level
    if (state == pdx->Power)
        return STATUS_SUCCESS;

    KeInitializeEvent(
        &event,
        NotificationEvent,
        FALSE
        );

    poState.DeviceState = state;

    // actually request the Irp
    status = PoRequestPowerIrp(
                 pdx->PhysicalDeviceObject,
                 IRP_MN_SET_POWER,
                 poState,
                 OnPowerRequestComplete,
                 &event,
                 NULL
                 );

    if (status == STATUS_PENDING)
    {
        // status pending is the return code we wanted
        waitStatus = KeWaitForSingleObject(
                         &event,
                         Suspended,
                         KernelMode,
                         FALSE,
                         NULL
                         );

        status = STATUS_SUCCESS;
    }
    else
    {
        // The return status was not STATUS_PENDING;
        // any other codes must be considered in error here;
        // i.e., it is not possible to get a STATUS_SUCCESS
        // or any other non-error return from this call;
        KdPrint((DBG_NAME "PoRequestPowerIrp failed\n"));
    }

    return status;
}


#else


/******************************************************************************
 *
 * Function   :  SendSelfSetPowerRequest
 *
 * Description:  Change the power level of the device. Note that this is not
 *               the way Microsoft recommends.
 *
 ******************************************************************************/
NTSTATUS
SendSelfSetPowerRequest(
    PDEVICE_OBJECT     fdo,
    DEVICE_POWER_STATE state
    )
{
    POWER_STATE       power;
    DEVICE_EXTENSION *pdx;


    KdPrint((DBG_NAME "in SendSelfSetPowerRequest\n"));

    // Get the device extension
    if (fdo->DeviceExtension == NULL)
    {
        KdPrint((DBG_NAME "ERROR - NULL extension\n"));
        return STATUS_NOT_SUPPORTED;
    }

    pdx = fdo->DeviceExtension;

    // Compare new and old power states
    if (state == pdx->Power)
        return STATUS_SUCCESS;

    // PoSetPowerState needs a POWER_STATE instead of a DEVICE_POWER_STATE
    power.DeviceState = state;

    if (state > pdx->Power)
    {
        // Removing power, process first
        PoSetPowerState(
            fdo,
            DevicePowerState,
            power
            );

        SetPowerState(
            fdo,
            state
            );
    }

    /*
        Normally, here we would create an IRP and send it down to the PCI bus
        driver. However, this crashes Win98. So, we do not do it. Note that
        the consequence of that is that the PCI bus driver does not know that
        we are changing the power level of the device.
    */

    if (state < pdx->Power)
    {
        // adding power, process request after
        SetPowerState(
            fdo,
            state
            );

        PoSetPowerState(
            fdo,
            DevicePowerState,
            power
            );
    }

    return STATUS_SUCCESS;
}
#endif
