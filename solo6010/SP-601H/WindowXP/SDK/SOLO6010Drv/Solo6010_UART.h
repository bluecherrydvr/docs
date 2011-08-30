#ifndef __SOLO6010_UART_H__
#define __SOLO6010_UART_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif

int S6010_InitUART (DEVICE_EXTENSION *pdx);
void S6010_ResetUART (DEVICE_EXTENSION *pdx, int idxUART_Port);
void S6010_DestroyUART (DEVICE_EXTENSION *pdx);

void S6010_UART_Setup (DEVICE_EXTENSION *pdx, INFO_UART_SETUP *pI_UART_Setup);
void S6010_UART_Read (DEVICE_EXTENSION *pdx, int idxUART_Port, int numRead, U8 *pBufRead, int *pSzRead);
void S6010_UART_Write (DEVICE_EXTENSION *pdx, int idxUART_Port, int numWrite, U8 *pBufWrite, int *pSzWritten);

void KE_UART_0_THREAD (PVOID pParam);
void KE_UART_1_THREAD (PVOID pParam);


#ifdef __cplusplus
}
#endif

#endif
