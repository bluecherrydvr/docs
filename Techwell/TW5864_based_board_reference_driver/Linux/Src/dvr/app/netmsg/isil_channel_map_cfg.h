#ifndef __ISIL_CHANNEL_MAP_CFG_H__
#define __ISIL_CHANNEL_MAP_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_messagestruct.h"
S32 ISIL_SetLocGuiChnMapCfg(S32 channel,void *buff);
S32 ISIL_GetLocGuiChnMapCfg(S32 channel,void *buff);


#ifdef __cplusplus
}
#endif
#endif
