#ifndef __SOLO6010_MP4_DEC_H__
#define __SOLO6010_MP4_DEC_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif


int S6010_InitMP4Dec (DEVICE_EXTENSION *pdx);
void S6010_DestroyMP4Dec (DEVICE_EXTENSION *pdx);
void S6010_OpenMP4Dec (DEVICE_EXTENSION *pdx);
void S6010_CloseMP4Dec (DEVICE_EXTENSION *pdx);


#ifdef __cplusplus
}
#endif

#endif
