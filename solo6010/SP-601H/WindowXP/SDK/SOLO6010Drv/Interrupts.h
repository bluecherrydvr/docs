#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

#include "TypesDef.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN OnInterrupt (PKINTERRUPT pInterrupt, PVOID pdx);
VOID Dpc_MP4_ENC (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_MP4_DEC (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_G723_ENC_DEC (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_LIVE_REFRESH (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_V_MOTION (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_P2M_0 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_P2M_1 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_P2M_2 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_P2M_3 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_I2C (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_GPIO_Int (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_UART_0 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_UART_1 (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);
VOID Dpc_PCI_ERROR (PKDPC pDpc, PVOID pContext, PVOID pArg1, PVOID pArg2);

#ifdef __cplusplus
}
#endif

#endif
