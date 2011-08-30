#ifndef __SOLO6010_H__
#define __SOLO6010_H__

#include "DriverDefs.h"
#include "Solo6010_DrvGlobal.h"

#ifdef __cplusplus
extern "C" {
#endif


void S6010_InitInfoS6010 (DEVICE_EXTENSION *pdx, INFO_S6010_INIT *pInfoInit);
int S6010_Init (DEVICE_EXTENSION *pdx);
void S6010_Remove (DEVICE_EXTENSION *pdx);

#if __NUM_SOLO_CHIPS > 1
	ULONG S6010_RegRead (DEVICE_EXTENSION *pdx, ULONG reg);
	void S6010_RegWrite (DEVICE_EXTENSION *pdx, ULONG reg, ULONG value);

	ULONG S6010_RegRead_DPC (DEVICE_EXTENSION *pdx, ULONG reg);
	void S6010_RegWrite_DPC (DEVICE_EXTENSION *pdx, ULONG reg, ULONG value);

	#define S6010_RegRead_ISR(nAddr, nRcvVar)	\
		if (glIAboutRegIO.bufIdxOldAccS6010[pdx->iS6010.idxID_RegIO] == pdx->iS6010.idxID)	\
		{	\
			glIAboutRegIO.numTmpForRegAcc = READ_REGISTER_ULONG((volatile ULONG *)(glIAboutRegIO.bufAddrOtherBaseReg[pdx->iS6010.idxID]));	\
		}	\
		nRcvVar = PLX_REG_READ(pdx, (nAddr));	\
		glIAboutRegIO.bufIdxOldAccS6010[pdx->iS6010.idxID_RegIO] = pdx->iS6010.idxID;
	#define S6010_RegWrite_ISR(nAddr, nValue)	\
		if (glIAboutRegIO.bufIdxOldAccS6010[pdx->iS6010.idxID_RegIO] == pdx->iS6010.idxID)	\
		{	\
			glIAboutRegIO.numTmpForRegAcc = READ_REGISTER_ULONG((volatile ULONG *)(glIAboutRegIO.bufAddrOtherBaseReg[pdx->iS6010.idxID]));	\
		}	\
		PLX_REG_WRITE(pdx, (nAddr), (nValue));	\
		glIAboutRegIO.bufIdxOldAccS6010[pdx->iS6010.idxID_RegIO] = pdx->iS6010.idxID;
#else
	ULONG S6010_RegRead (DEVICE_EXTENSION *pdx, ULONG reg);
	ULONG S6010_RegWrite (DEVICE_EXTENSION *pdx, ULONG reg, ULONG value);

	ULONG S6010_RegRead_DPC (DEVICE_EXTENSION *pdx, ULONG reg);
	void S6010_RegWrite_DPC (DEVICE_EXTENSION *pdx, ULONG reg, ULONG value);

	#define S6010_RegRead_ISR(nAddr, nRcvVar)	nRcvVar = PLX_REG_READ(pdx, (nAddr));	\
												KeStallExecutionProcessor (1)
	#define S6010_RegWrite_ISR(nAddr, nValue)	PLX_REG_WRITE(pdx, (nAddr), (nValue));	\
												KeStallExecutionProcessor (1);			\
												PLX_REG_READ(pdx, (nAddr));				\
												KeStallExecutionProcessor (1)
#endif


BOOLEAN S6010_DisableAllInterrupt (DEVICE_EXTENSION *pdx);
void S6010_EnableInterrupt (DEVICE_EXTENSION *pdx, ULONG nMask);
void S6010_DisableInterrupt (DEVICE_EXTENSION *pdx, ULONG nMask);


#ifdef __cplusplus
}
#endif

#endif
