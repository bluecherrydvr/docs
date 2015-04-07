#ifndef _ISIL_VENC_AUDIO_H_

#define _ISIL_VENC_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_codec_common.h"
#include "isil_encoder.h"


/*******************************API****************************************************/
extern int ISIL_VENC_AUD_CreateCh(CODEC_CHANNEL* pChan, CODEC_HANDLE* pChipHandle, char* pChDir);
extern int ISIL_VENC_AUD_ReleaseCh(CODEC_CHANNEL* pChan, CODEC_HANDLE* pChipHandle);

extern int ISIL_VENC_AUD_Enable(CODEC_CHANNEL* pChan);//start encoding
extern int ISIL_VENC_AUD_Disable(CODEC_CHANNEL* pChan);

extern int ISIL_VENC_AUD_SetChCfg(CODEC_CHANNEL* pChan, VENC_AUD_CFG* stpVencAUDCfg);
extern int ISIL_VENC_AUD_GetChCfg(CODEC_CHANNEL* pChan, VENC_AUD_CFG* stpVencAUDCfg);


extern int ISIL_VENC_AUD_GetData(CODEC_CHANNEL* pChan, CODEC_STREAM_DATA *stpStream, DATA_ACCESS_MODE eReadway, IO_BLK_TYPE eBlk);

extern int ISIL_VENC_AUD_GetChHandle(CODEC_CHANNEL* pChan, CODEC_HANDLE *pHandle);

extern int ISIL_VENC_AUD_DiscardFrame(CODEC_CHANNEL* pChan);

extern int ISIL_VENC_AUD_Flush(CODEC_CHANNEL* pChan);

#ifdef __cplusplus
}
#endif
#endif






