#ifndef _ISIL_VENC_H264_H_

#define _ISIL_VENC_H264_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_codec_common.h"
#include "isil_encoder.h"

/********************************************API************************************************/
extern int ISIL_VENC_H264_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir);
extern int ISIL_VENC_H264_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle);

extern int ISIL_VENC_H264_Enable(CODEC_CHANNEL *pChan);//start encoding
extern int ISIL_VENC_H264_Disable(CODEC_CHANNEL *pChan);

extern int ISIL_VENC_H264_SetChCfg(CODEC_CHANNEL *pChan, VENC_H264_CFG* stpVencH264Cfg);
extern int ISIL_VENC_H264_GetChCfg(CODEC_CHANNEL *pChan, VENC_H264_CFG* stpVencH264Cfg);

extern int ISIL_VENC_H264_SetChFeature(CODEC_CHANNEL *pChan, VENC_H264_FEATURE* stpVencH264Feature);
extern int ISIL_VENC_H264_GetChFeature(CODEC_CHANNEL *pChan, VENC_H264_FEATURE* stpVencH264Feature);

extern int ISIL_VENC_H264_GetChMapInfo(CODEC_CHANNEL *pChan, VENC_H264_CHAN_MAP_INFO* stpVencH264Map);

extern int ISIL_VENC_H264_SetChRC(CODEC_CHANNEL *pChan, VENC_H264_RC* stpVencH264RC);
extern int ISIL_VENC_H264_GetChRC(CODEC_CHANNEL *pChan, VENC_H264_RC* stpVencH264RC);

extern int ISIL_VENC_H264_SetChRCParam(CODEC_CHANNEL *pChan, VENC_H264_RC_PARAM* stpVencH264RCParam);
extern int ISIL_VENC_H264_GetChRCParam(CODEC_CHANNEL *pChan, VENC_H264_RC_PARAM* stpVencH264RCParam);


extern int ISIL_VENC_H264_SendUserData(CODEC_CHANNEL *pChan, char *pUsrData, unsigned int u32UsrDataLen);

extern int ISIL_VENC_H264_GetData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *stpStream, DATA_ACCESS_MODE eReadway, IO_BLK_TYPE eBlk);
           
extern int ISIL_VENC_H264_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle);

extern int ISIL_VENC_H264_DiscardFrame(CODEC_CHANNEL *pChan);


extern int ISIL_VENC_H264_SetOSDCfg(CODEC_CHANNEL *pChan, ISIL_OSD_CFG* pOSDCfg);
extern int ISIL_VENC_H264_GetOSDCfg(CODEC_CHANNEL *pChan, ISIL_OSD_CFG* pOSDCfg);


extern int ISIL_VENC_H264_Flush(CODEC_CHANNEL *pChan);

#ifdef __cplusplus
    }
#endif

#endif
