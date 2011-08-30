#ifndef __SOLO6010_MP4_ENC_H__
#define __SOLO6010_MP4_ENC_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	IDX_MODE_GOPRST_IDLE,
	IDX_MODE_GOPRST_SET_VALUE,
	IDX_MODE_GOPRST_EXEC,
};	// GOP Reset Mode

int S6010_InitMP4Enc (DEVICE_EXTENSION *pdx);
void S6010_DestroyMP4Enc (DEVICE_EXTENSION *pdx);

void S6010_SetInfoMP4JPEGRec (DEVICE_EXTENSION *pdx, INFO_REC *pInfoRec);
void S6010_StartMP4JPEGEnc (DEVICE_EXTENSION *pdx);
void S6010_EndMP4JPEGEnc (DEVICE_EXTENSION *pdx);
void S6010_ChangeInfoMP4JPEGRec (DEVICE_EXTENSION *pdx, INFO_REC *pInfoRec);

void S6010_SetEncoderOSD_Prop (DEVICE_EXTENSION *pdx, INFO_ENCODER_OSD *pInfoEncOSD);
void S6010_GetEncoderOSD (DEVICE_EXTENSION *pdx, INFO_ENCODER_OSD *pInfoEncOSD);
void S6010_SetEncoderOSD (DEVICE_EXTENSION *pdx, INFO_ENCODER_OSD *pInfoEncOSD);


#ifdef __cplusplus
}
#endif

#endif
