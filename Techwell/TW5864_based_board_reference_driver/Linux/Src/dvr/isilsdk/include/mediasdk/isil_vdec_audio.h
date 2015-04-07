#ifndef _ISIL_VDEC_AUDIO_H_

#define _ISIL_VDEC_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_codec_common.h"
#include "isil_decoder.h"


/*******************************API****************************************************/
extern int ISIL_VDEC_AUD_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir);
extern int ISIL_VDEC_AUD_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle);

extern int ISIL_VDEC_AUD_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle);

extern int ISIL_VDEC_AUD_Enable(CODEC_CHANNEL *pChan);//start encoding
extern int ISIL_VDEC_AUD_Disable(CODEC_CHANNEL *pChan);

extern int ISIL_VDEC_AUD_SetChCfg(CODEC_CHANNEL *pChan, VDEC_AUD_CFG* stpVdecAudCfg);
extern int ISIL_VDEC_AUD_GetChCfg(CODEC_CHANNEL *pChan, VDEC_AUD_CFG* stpVdecAudCfg);

extern int ISIL_VDEC_AUD_SendData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *stpStream);

extern int ISIL_VDEC_AUD_Flush(CODEC_CHANNEL *pChan);

//set playback strategy
extern int ISIL_VDEC_AUD_SetStrategy(CODEC_CHANNEL *pChan, VDEC_CH_STRATEGY *pStrategy);

//set playback mode
extern int ISIL_VDEC_AUD_SetPBMode(CODEC_CHANNEL *pChan, VDEC_CH_MODE *pMode);

//set request-frame command timeout interval
extern int ISIL_VDEC_AUD_SetTimeout(CODEC_CHANNEL *pChan, VDEC_CH_TIMEOUT *pTimeout); 

extern int ISIL_VDEC_AUD_SetStreamType(CODEC_CHANNEL *pDecChan, VDEC_CH_HEADER* pHeader);

extern int ISIL_VDEC_AUD_Readydie(CODEC_CHANNEL *pChan);

#ifdef __cplusplus
}
#endif
#endif
