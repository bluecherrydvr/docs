#ifndef __ISIL_STREAM_MANAGE_H__
#define __ISIL_STREAM_MANAGE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "isil_messagestruct.h"

#define ISIL_STREAM_CFG_FILE "/flash/config/stream/encode_cfg_"
#define ISIL_CHANNEL_INFO    "/flash/config/stream/channel_info_"
#define ISIL_CH_H264_FEATURE "/flash/config/stream/h264_"

extern S32 ISIL_GetStreamCfg(G_STREAM_CFG* pCfg);
extern S32 ISIL_SaveStreamCfg(G_STREAM_CFG* pCfg);

extern void ISIL_GetChannelInfo(S32 channelNum, S08 *pChannelName);
extern S32 ISIL_SaveH264Cfg(S32 ch, void* pCfg, S32 len);

extern S32 ISIL_GetH264Cfg(S32 ch, void* pCfg, S32 len);

#ifdef __cplusplus
}
#endif
#endif

