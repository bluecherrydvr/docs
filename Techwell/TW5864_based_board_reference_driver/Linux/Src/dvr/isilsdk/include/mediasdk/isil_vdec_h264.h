#ifndef _ISIL_CODEC_VDEC_H_

#define _ISIL_CODEC_VDEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_codec_common.h"
#include "isil_decoder.h"


/*******************************API****************************************************/
extern int ISIL_VDEC_H264_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir);
extern int ISIL_VDEC_H264_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle);

extern int ISIL_VDEC_H264_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle);

extern int ISIL_VDEC_H264_Enable(CODEC_CHANNEL *pChan);//start encoding
extern int ISIL_VDEC_H264_Disable(CODEC_CHANNEL *pChan);

extern int ISIL_VDEC_H264_SetChCfg(CODEC_CHANNEL *pChan, VDEC_H264_CFG* stpVdecH264Cfg);
extern int ISIL_VDEC_H264_GetChCfg(CODEC_CHANNEL *pChan, VDEC_H264_CFG* stpVdecH264Cfg);

extern int ISIL_VDEC_H264_SendData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *pstStream);

extern int ISIL_VDEC_H264_Flush(CODEC_CHANNEL *pChan);

//set playback strategy
extern int ISIL_VDEC_H264_SetStrategy(CODEC_CHANNEL *pChan, VDEC_CH_STRATEGY *pStrategy);

//set playback mode
extern int ISIL_VDEC_H264_SetPBMode(CODEC_CHANNEL *pChan, VDEC_CH_MODE *pMode);


//set request-frame command timeout interval
extern int ISIL_VDEC_H264_SetTimeout(CODEC_CHANNEL *pChan, VDEC_CH_TIMEOUT *pTimeout); 

extern int ISIL_VDEC_H264_Readydie(CODEC_CHANNEL *pChan);

extern int ISIL_VDEC_H264_SetStreamType(CODEC_CHANNEL *pDecChan, VDEC_CH_HEADER* pHeader);

#ifdef __cplusplus
}
#endif
#endif
