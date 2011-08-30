#ifndef __SOLO6010_TW2815_H__
#define __SOLO6010_TW2815_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif


int S6010_SetupTW2815 (DEVICE_EXTENSION *pdx, int nIDW, int nChannel);
int S6010_GetColor (DEVICE_EXTENSION *pdx, INFO_COLOR *pInfoColor);
int S6010_SetColor (DEVICE_EXTENSION *pdx, INFO_COLOR *pInfoColor);
int S6010_GetVLoss (DEVICE_EXTENSION *pdx, INFO_VLOSS *pInfoVLoss);


#ifdef __cplusplus
}
#endif

#endif
