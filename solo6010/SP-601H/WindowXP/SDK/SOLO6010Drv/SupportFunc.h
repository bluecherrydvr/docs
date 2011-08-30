#ifndef __SUPPORT_FUNC_H
#define __SUPPORT_FUNC_H

#include "DriverDefs.h"

#define MIN_SIZE_MAPPING_BAR_0	(REG_AUDIO_STA +4)	// Min Size of BAR0 required mapping
#define CFG_BAR0	0x010
#define MAX_PCI_DEV	32

#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS S6010_CompleteIrpWithInformation (PIRP pIrp, NTSTATUS status, unsigned long Info);
void S6010_Sleep_1US ();
void S6010_Sleep_1MS ();
void S6010_Sleep_10MS ();
void S6010_Sleep_50MS ();
void S6010_Sleep_100MS ();
void S6010_Sleep_1S ();
__inline void S6010_Sleep_100NS_Base (signed long time);
NTSTATUS S6010_CompleteIrp (PIRP pIrp, NTSTATUS status);									  
NTSTATUS OnRequestComplete (PDEVICE_OBJECT fdo, PIRP pIrp, PKEVENT pKEvent);
NTSTATUS PciConfigRegisterBufferRead (PDEVICE_OBJECT  fdo, VOID *buffer,
	unsigned short offset, unsigned long length);
NTSTATUS PciConfigRegisterBufferWrite (PDEVICE_OBJECT  fdo, VOID *buffer,
	unsigned short offset, unsigned long length);
unsigned char GetBarIndex (PHYSICAL_ADDRESS address, PCI_COMMON_CONFIG *pciRegs);
NTSTATUS S6010_PciBarSpaceMap (DEVICE_EXTENSION *pdx, unsigned char BarIndex);
VOID S6010_PciBarResourcesUnmap (DEVICE_EXTENSION *pdx);
NTSTATUS GetBusSlotNumber (PDEVICE_OBJECT pdo, DEVICE_EXTENSION *pdx);
NTSTATUS ForwardAndWait (PDEVICE_OBJECT fdo, PIRP pIrp);

#ifdef __cplusplus
}
#endif

#endif
