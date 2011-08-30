#ifndef __SOLO6010_DISPLAY_H__
#define __SOLO6010_DISPLAY_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif


int S6010_InitDisplay (DEVICE_EXTENSION *pdx);
void S6010_SetInfoLive (DEVICE_EXTENSION *pdx, INFO_LIVE *pInfoLive);


#ifdef __cplusplus
}
#endif

#endif
