#ifndef __SOLO6010_G723_H__
#define __SOLO6010_G723_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif


int S6010_InitG723Coder (DEVICE_EXTENSION *pdx);
void S6010_DestroyG723Coder (DEVICE_EXTENSION *pdx);

void S6010_SetInfoG723Rec (DEVICE_EXTENSION *pdx, INFO_REC *pInfoRec);
void S6010_StartG723Enc (DEVICE_EXTENSION *pdx);
void S6010_EndG723Enc (DEVICE_EXTENSION *pdx);

void S6010_OpenG723Dec (DEVICE_EXTENSION *pdx, int idxSamplingRate);
void S6010_StartG723Dec (DEVICE_EXTENSION *pdx);
void S6010_EndG723Dec (DEVICE_EXTENSION *pdx);


#ifdef __cplusplus
}
#endif

#endif
