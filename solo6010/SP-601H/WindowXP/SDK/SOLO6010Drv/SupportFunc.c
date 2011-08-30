#include "SupportFunc.h"
#include <wdmguid.h>


/* ms to 100-ns
     *     1 ns  = 10 ^ -9 sec
     *   100 ns  = 10 ^ -7 sec (1 timer interval)
     *     1 ms  = (1 timer interval) * 10000
     *    10 ms  = (1 timer interval) * 100000
     *   100 ms  = (1 timer interval) * 1000000
     *  1000 ms  = (1 timer interval) * 10000000
*/
__inline VOID S6010_Sleep_100NS_Base (signed long time)
{
    LARGE_INTEGER liTime;

    // Negative value means relative time, not absolute
    liTime = RtlConvertLongToLargeInteger(-time);

    KeDelayExecutionThread(
        KernelMode,
        FALSE,
        &liTime
        );
}

void S6010_Sleep_1US ()
{
	S6010_Sleep_100NS_Base (10);
}

void S6010_Sleep_1MS ()
{
	S6010_Sleep_100NS_Base (10000);
}

void S6010_Sleep_10MS ()
{
	S6010_Sleep_100NS_Base (100000);
}

void S6010_Sleep_50MS ()
{
	S6010_Sleep_100NS_Base (500000);
}

void S6010_Sleep_100MS ()
{
	S6010_Sleep_100NS_Base (1000000);
}

void S6010_Sleep_1S ()
{
	S6010_Sleep_100NS_Base (10000000);
}

NTSTATUS S6010_CompleteIrp (PIRP pIrp, NTSTATUS status)										  
{
    pIrp->IoStatus.Status = status;

    IoCompleteRequest(
        pIrp,
        IO_NO_INCREMENT
        );
/*
    if (status == STATUS_CANCELLED)
        KdPrint((".Cancel IRP (0x%08x)\n", pIrp));
    else
        KdPrint((".Ok IRP (0x%08x)\n", pIrp));
*/
    return status;
}

NTSTATUS S6010_CompleteIrpWithInformation (PIRP pIrp, NTSTATUS status, unsigned long Info)
{
    pIrp->IoStatus.Status      = status;
    pIrp->IoStatus.Information = Info;

    IoCompleteRequest(
        pIrp,
        IO_NO_INCREMENT
        );

    if (status == STATUS_CANCELLED)
        KdPrint((".Cancelled IRP (0x%08x)\n", pIrp));
    else
        KdPrint((".Completed IRP (0x%08x)\n", pIrp));

    return status;
}

NTSTATUS OnRequestComplete (PDEVICE_OBJECT fdo, PIRP pIrp, PKEVENT pKEvent)
{
    KeSetEvent(
       pKEvent,
       0,
       FALSE
       );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS PciConfigRegisterBufferRead (PDEVICE_OBJECT  fdo, VOID *buffer,
	unsigned short offset, unsigned long length)
{
    PIRP               pIrp;
    KEVENT             event;
    NTSTATUS           status;
    PIO_STACK_LOCATION stack;


    /*
        Although it is not documented in the WDM DDK, it would appear that the
        correct way to access a PCI configuration register in Windows 98 is
        to send down a PnP packet to the bus driver.
        The bus driver, provided by Microsoft, will perform the actual PCI
        configuration access and return the value.
    */

    pIrp = IoAllocateIrp(
               fdo->StackSize,
               FALSE
               );
    if (pIrp == NULL)
    {
        KdPrint((DBG_NAME "ERROR - ConfigRegisterBufferRead() unable to allocate IRP\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Initialize kernel event
    KeInitializeEvent(
        &event,
        NotificationEvent,
        FALSE
        );

    stack =
        IoGetNextIrpStackLocation(
            pIrp
            );

    // Fill the IRP
    pIrp->IoStatus.Status                        = STATUS_NOT_SUPPORTED;
    stack->MajorFunction                         = IRP_MJ_PNP;
    stack->MinorFunction                         = IRP_MN_READ_CONFIG;
    stack->Parameters.ReadWriteConfig.WhichSpace = 0;
    stack->Parameters.ReadWriteConfig.Buffer     = buffer;
    stack->Parameters.ReadWriteConfig.Offset     = offset;
    stack->Parameters.ReadWriteConfig.Length     = length;

    IoSetCompletionRoutine(
        pIrp,
        (PIO_COMPLETION_ROUTINE)OnRequestComplete,
        (PVOID)&event,
        TRUE,
        TRUE,
        TRUE
        );

    // Send the packet
    status =
        IoCallDriver(
            ((DEVICE_EXTENSION *) fdo->DeviceExtension)->pLowerDeviceObject,
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

    // Release the IRP
    IoFreeIrp(
        pIrp
        );

    return status;
}

NTSTATUS PciConfigRegisterBufferWrite (PDEVICE_OBJECT  fdo, VOID *buffer,
	unsigned short offset, unsigned long length)
{
    PIRP               pIrp;
    KEVENT             event;
    NTSTATUS           status;
    PIO_STACK_LOCATION stack;


    /*
        Although it is not documented in the WDM DDK, it would appear that the
        correct way to access a PCI configuration register in Windows 98 is
        to send down a PnP packet to the bus driver.
        The bus driver, provided by Microsoft, will perform the actual PCI
        configuration access and return the value.
    */

    pIrp = IoAllocateIrp(
               fdo->StackSize,
               FALSE
               );
    if (pIrp == NULL)
    {
        KdPrint((DBG_NAME "ERROR - ConfigRegisterBufferRead() unable to allocate IRP\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Initialize kernel event
    KeInitializeEvent(
        &event,
        NotificationEvent,
        FALSE
        );

    stack =
        IoGetNextIrpStackLocation(
            pIrp
            );

    // Fill the IRP
    pIrp->IoStatus.Status                        = STATUS_NOT_SUPPORTED;
    stack->MajorFunction                         = IRP_MJ_PNP;
    stack->MinorFunction                         = IRP_MN_WRITE_CONFIG;
    stack->Parameters.ReadWriteConfig.WhichSpace = 0;
    stack->Parameters.ReadWriteConfig.Buffer     = buffer;
    stack->Parameters.ReadWriteConfig.Offset     = offset;
    stack->Parameters.ReadWriteConfig.Length     = length;

    IoSetCompletionRoutine(
        pIrp,
        (PIO_COMPLETION_ROUTINE)OnRequestComplete,
        (PVOID)&event,
        TRUE,
        TRUE,
        TRUE
        );

    // Send the packet
    status =
        IoCallDriver(
            ((DEVICE_EXTENSION *) fdo->DeviceExtension)->pLowerDeviceObject,
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

    // Release the IRP
    IoFreeIrp(
        pIrp
        );

    return status;
}

unsigned char GetBarIndex (PHYSICAL_ADDRESS address, PCI_COMMON_CONFIG *pciRegs)
{
    unsigned char i;
    unsigned long compareAddress;


    // Compare the physical address with each BAR
    for (i = 0; i < PCI_NUM_BARS; i++)
    {
        if (i == 6)
        {
            // Check if Expansion ROM BAR
            compareAddress = (pciRegs->u.type0.ROMBaseAddress & ~0x1);
        }
        else
        {
            if (pciRegs->u.type0.BaseAddresses[i] & 0x1)
                compareAddress = pciRegs->u.type0.BaseAddresses[i] & 0xFFFFFFFC;
            else
                compareAddress = pciRegs->u.type0.BaseAddresses[i] & 0xFFFFFFF0;
        }

        if (address.LowPart == compareAddress)
            return i;
    }

    // Unable to find the BAR index
    KdPrint((DBG_NAME "ERROR - GetBarIndex() unable to match BAR value (0x%08x)\n",
             address.LowPart));

    return (unsigned char)-1;
}

NTSTATUS S6010_PciBarSpaceMap (DEVICE_EXTENSION *pdx, unsigned char BarIndex)
{
    // Map into Kernel Virtual Space
    pdx->PciBar[BarIndex].pVa =
        MmMapIoSpace(
            pdx->PciBar[BarIndex].Physical,
            pdx->PciBar[BarIndex].Size,
            FALSE
            );

    if (pdx->PciBar[BarIndex].pVa == NULL)
    {
        /******************************************************************
         * In Windows, sometimes the BAR registers do not get mapped due to
         * insufficient page table entries.  Since the driver requires BAR0
         * for register access, we must try to map at least the registers.
         * If this fails, then the driver cannot load.
         *****************************************************************/
        if (BarIndex == 0)
        {
            // Attempt a minimum mapping
            pdx->PciBar[BarIndex].pVa =
                MmMapIoSpace(
                    pdx->PciBar[BarIndex].Physical,
                    MIN_SIZE_MAPPING_BAR_0,
                    FALSE
                    );

            if (pdx->PciBar[BarIndex].pVa == NULL)
            {
                pdx->PciBar[BarIndex].pVa = (U32*)-1;
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                pdx->PciBar[BarIndex].Size = MIN_SIZE_MAPPING_BAR_0;
            }
        }
        else
        {
            pdx->PciBar[BarIndex].pVa = (U32*)-1;
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    pdx->PciBar[BarIndex].IsUserMappable = TRUE;

    return STATUS_SUCCESS;
}

VOID S6010_PciBarResourcesUnmap (DEVICE_EXTENSION *pdx)
{
    unsigned char i;


    // Go through all the BARS
    for (i = 0; i < PCI_NUM_BARS; i++)
    {
        // Unmap the space from Kernel and User spaces if previously mapped
        if (pdx->PciBar[i].IsUserMappable)
        {
            KdPrint((DBG_NAME "Unmapping BAR %d...\n", i));

            // Record that the local space is no longer mappable
            pdx->PciBar[i].IsUserMappable = 0;

            // Release the MDL describing the user space map
            if (pdx->PciBar[i].pMdl != (PMDL)NULL)
            {
                IoFreeMdl(
                    pdx->PciBar[i].pMdl
                    );
                pdx->PciBar[i].pMdl = NULL;
            }

            // Unmap from kernel space
            if ((pdx->PciBar[i].pVa != (unsigned long *)(unsigned long)-1) &&
                (pdx->PciBar[i].pVa != (unsigned long *)NULL))
            {
                MmUnmapIoSpace(
                    pdx->PciBar[i].pVa,
                    pdx->PciBar[i].Size
                    );
                pdx->PciBar[i].pVa  = (unsigned long *)(unsigned long)-1;
                pdx->PciBar[i].Size = 0;
            }
        }
    }
}

NTSTATUS GetBusSlotNumber (PDEVICE_OBJECT pdo, DEVICE_EXTENSION *pdx)
{
    unsigned char       SlotNumber;
    unsigned long      ResultLength;
    unsigned long      PropertyBuffer;
    unsigned long      RegisterSave;
    unsigned long      RegisterToCompare;
    KIRQL    IrqlSave;
    NTSTATUS status;


    ResultLength = 0;

    // Get the bus number first
    status = IoGetDeviceProperty(
                 pdo,
                 DevicePropertyBusNumber,
                 sizeof(U32),
                 &PropertyBuffer,
                 &ResultLength
                 );

    if ( !NT_SUCCESS(status) )
    {
        KdPrint((DBG_NAME "ERROR - IoGetDeviceProperty() unable to get bus number, code = %Xh\n", status));
        return status;
    }

    // Verify that the function did write back a U32
    if (ResultLength != sizeof(unsigned long))
    {
        KdPrint((DBG_NAME "ERROR - IoGetDeviceProperty() invalid ResultLength (%d)\n", ResultLength));
        return STATUS_UNSUCCESSFUL;
    }

    // Store the Bus Number
    pdx->Device.BusNumber = PropertyBuffer;


    /**************************************************************************
    * Now attempt to get the device location, which should include the slot number.
    *
    * Note: The IoGetDeviceProperty() function is not implemented correctly in
    *       Windows 98.  The code has been left here in case this is fixed in a
    *       future release.  If the IoGetDeviceProperty() function is unsuccessful,
    *       a workaround is provided to retrieve the correct slot number.
    ***************************************************************************/

    ResultLength = 0;

    status = IoGetDeviceProperty(
                 pdo,
                 DevicePropertyLocationInformation,
                 sizeof(unsigned long),
                 &PropertyBuffer,
                 &ResultLength
                 );

    if ( NT_SUCCESS(status) )
    {
        if (ResultLength != sizeof(unsigned long))
        {
            KdPrint((DBG_NAME "ERROR - IoGetDeviceProperty() invalid ResultLength (%d)\n", ResultLength));
            return STATUS_UNSUCCESSFUL;
        }

        pdx->Device.SlotNumber = PropertyBuffer;

        return status;
    }


    /****************************************************************
    * We were unable to get the slot number, so another method is
    * required.  Our workaround is to scan the PCI bus and find the
    * device whose BAR0 matches the current device.
    ****************************************************************/

    KdPrint((DBG_NAME "WARNING - IoGetDeviceProperty() unable to get slot number, code = "));

    switch (status)
    {
        case STATUS_BUFFER_TOO_SMALL:
            KdPrint(("STATUS_BUFFER_TOO_SMALL (req=%d bytes)\n", ResultLength));
            break;

        case STATUS_NOT_FOUND:
            KdPrint(("STATUS_NOT_FOUND\n"));
            break;

        default:
            KdPrint(("%08xh\n", status));
    }

    KdPrint((DBG_NAME "NOTE: Implementing workaround to get Slot number\n"));
        
    status = PciConfigRegisterBufferRead(
                 pdx->pDeviceObject,
                 &RegisterToCompare,
                 CFG_BAR0,
                 sizeof(unsigned long)
                 );

    if ( !NT_SUCCESS(status) )
    {
        KdPrint((DBG_NAME "ERROR - Unable to implement workaround for getting Slot number\n"));
        return status;
    }

    /***************************************************************
     * Scanning the PCI bus involves using  I/O addresses 0xcf8 and 
     * 0xcfc. These addresses must be used together and no other
     * process must interrupt, so we must raise the IRQL.
     **************************************************************/
    KeRaiseIrql(
        DISPATCH_LEVEL,
        &IrqlSave
        );

    // Save the content of the command register
    RegisterSave = READ_PORT_ULONG(
                       (ULONG*)0xcf8
                       );

    // Scan the PCI bus to find our device
    for (SlotNumber = 0; SlotNumber < MAX_PCI_DEV; SlotNumber++)
    {
        // Issue PCI Config write command
        WRITE_PORT_ULONG(
            (ULONG*)0xcf8,
            (1 << 31) | (pdx->Device.BusNumber << 16) | (SlotNumber << 11) | 0x10
            );

        // Get the Result
        if (READ_PORT_ULONG((ULONG*)0xcfc) == RegisterToCompare)
        {
            pdx->Device.SlotNumber = SlotNumber;
            KdPrint((DBG_NAME "Workaround successful, Slot number = %02Xh\n", SlotNumber));
            break;
        }
    }

    // Restore saved command register
    WRITE_PORT_ULONG(
        (ULONG*)0xcf8,
        RegisterSave
        );

    // Restore IRQL
    KeLowerIrql(
        IrqlSave
        );

    return STATUS_SUCCESS;
}

NTSTATUS ForwardAndWait (PDEVICE_OBJECT fdo, PIRP pIrp)
{
    KEVENT    event;
    NTSTATUS  status;


    /* 
       Initialize a kernel event object to use in waiting for the lower-level
       driver to finish processing the object. It's a little known fact that the
       kernel stack *can* be paged, but only while someone is waiting in user 
       mode for an event to finish. Since neither we nor a completion routine 
       can be in the forbidden state, it's okay to put the event object on the 
       stack.
    */
    KeInitializeEvent(
        &event,
        NotificationEvent,
        FALSE
        );

    IoCopyCurrentIrpStackLocationToNext(
        pIrp
        );

    IoSetCompletionRoutine(
        pIrp,
        (PIO_COMPLETION_ROUTINE) OnRequestComplete,
        (PVOID) &event,
        TRUE,
        TRUE,
        TRUE
        );

    status = IoCallDriver(
                 ((DEVICE_EXTENSION *) fdo->DeviceExtension)->pLowerDeviceObject,
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

        return pIrp->IoStatus.Status;
    }

    return status;
}

