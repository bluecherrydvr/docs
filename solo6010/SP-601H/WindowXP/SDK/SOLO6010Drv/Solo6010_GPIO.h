#ifndef __SOLO6010_GPIO_H__
#define __SOLO6010_GPIO_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif


int S6010_InitGPIO (DEVICE_EXTENSION *pdx);
void S6010_DestroyGPIO (DEVICE_EXTENSION *pdx);

void S6010_GPIO_Operation (DEVICE_EXTENSION *pdx, INFO_GPIO *pIGPIO);

void S6010_GPIO_A_Mode (DEVICE_EXTENSION *pdx, ULONG port_mask, ULONG mode);
void S6010_GPIO_B_Mode (DEVICE_EXTENSION *pdx, ULONG port_mask, ULONG mode);
ULONG S6010_GPIO_Get (DEVICE_EXTENSION *pdx);
void S6010_GPIO_Set (DEVICE_EXTENSION *pdx, ULONG maskPort, ULONG nValue);
void S6010_GPIO_Clear (DEVICE_EXTENSION *pdx, unsigned int nValue);


#ifdef __cplusplus
}
#endif

#endif
