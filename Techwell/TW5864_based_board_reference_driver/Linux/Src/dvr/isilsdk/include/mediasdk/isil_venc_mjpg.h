#ifndef _ISIL_VENC_MJPG_H_

#define _ISIL_VENC_MJPG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_codec_common.h"
#include "isil_encoder.h"

/**************************************************API*******************************************/
extern int ISIL_VENC_MJPG_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir);
extern int ISIL_VENC_MJPG_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle);

extern int ISIL_VENC_MJPG_Enable(CODEC_CHANNEL *pChan);//start encoding
extern int ISIL_VENC_MJPG_Disable(CODEC_CHANNEL *pChan);

extern int ISIL_VENC_MJPG_SetChCfg(CODEC_CHANNEL *pChan, VENC_MJPG_CFG* stpVencMjpgCfg);
extern int ISIL_VENC_MJPG_GetChCfg(CODEC_CHANNEL *pChan, VENC_MJPG_CFG* stpVencMjpgCfg);


extern int ISIL_VENC_MJPG_GetData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *stpStream, DATA_ACCESS_MODE eReadway, IO_BLK_TYPE eBlk);

extern int ISIL_VENC_MJPG_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle);

extern int ISIL_VENC_MJPG_StartCap(CODEC_CHANNEL *pEncChan, unsigned int u32Type);
extern int ISIL_VENC_MJPG_StopCap(CODEC_CHANNEL *pEncChan, unsigned int u32Type);

extern int ISIL_VENC_MJPG_DiscardFrame(CODEC_CHANNEL *pChan);

extern int ISIL_VENC_MJPG_Flush(CODEC_CHANNEL *pChan);

#ifdef __cplusplus
    }
#endif

#endif
