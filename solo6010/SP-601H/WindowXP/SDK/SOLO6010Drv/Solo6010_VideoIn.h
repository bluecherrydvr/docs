#ifndef __SOLO6010_VIDEO_IN_H__
#define __SOLO6010_VIDEO_IN_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif


int S6010_InitVideoIn (DEVICE_EXTENSION *pdx);

int S6010_SetInfoMosaic (DEVICE_EXTENSION *pdx, INFO_MOSAIC *pInfoMosaic);

int S6010_SetInfoVMotion (DEVICE_EXTENSION *pdx, INFO_V_MOTION *pInfoVMotion);

int S6010_SetInfoCamToCh (DEVICE_EXTENSION *pdx, INFO_VIDEO_MATRIX *pInfoVMatrix);

int S6010_GetCamBlkDetState (DEVICE_EXTENSION *pdx, INFO_CAM_BLK_DETECTION *pInfoCamBlkDet);

int S6010_StartAdvMotionDataGet (DEVICE_EXTENSION *pdx);
int S6010_EndAdvMotionDataGet (DEVICE_EXTENSION *pdx);

#ifdef __cplusplus
}
#endif

#endif
