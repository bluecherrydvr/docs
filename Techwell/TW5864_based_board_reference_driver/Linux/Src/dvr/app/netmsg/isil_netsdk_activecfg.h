#ifndef __ISIL_NETSDK_ACTIVE_CFG_H__
#define __ISIL_NETSDK_ACTIVE_CFG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "isil_netsdk_netmsg.h"


S32 ISIL_NetSDK_ActiveEncodeParameter(S32 channel, S32 netfd, void*buff);

S32 ISIL_NetSDK_ActiveMediaSdkEncodeParameter(S32 channel, S32 netfd, void*buff);
S32 ISIL_NetSDK_ActiveMediaSdkMjpeg(S32 channel, S32 netfd, void*buff);
S32 ISIL_NetSDK_ActiveMediaSdkAudio(S32 channel, S32 netfd, void*buff);

S32 ISIL_NetSDK_ActiveMediaSdkNotRT(S32 channel, S32 netfd, void*buff);

S32 ISIL_NetSDK_ActiveMediaSdkAD(S32 channel, S32 netfd, void*buff);
S32 ISIL_NetSDK_ActiveFileSearch(S32 iFd, ISIL_SEARCH_EVNET* pEvnt);

S32 ISIL_NetSDK_ActiveFileOpen(S08* filename);

S32 ISIL_NetSDK_ActiveFileClose(S08* filename);

S32 ISIL_NetSDK_StartFileRec(S32 iChn);

S32 ISIL_NetSDK_StopFileRec(S32 iChn);


S32 ISIL_NetSDK_SetSubWindows(void *buff);
S32 ISIL_NetSDK_BindFileWindows(S32 fd,void *buff);

S32 ISIL_NetSDK_SetPlayMod(void *buff);

S32 ISIL_NetSDK_SetAVSync(void *buff);

S32 ISIL_NetSDK_SetDisplayPortMode(void *buff);

S32 ISIL_NetSDK_TestW(S32 chip, void *buff);


S32 ISIL_NetSDK_TestR(S32 chip, U32* sdsize, void* cmd,void *buff);

S32 ISIL_NetSDK_SetVideoStandard(char stdandard);

S32 ISIL_NetSDK_ActiveAlarmType(S32 chn, int type);

S32 ISIL_NetSDK_ClearAlarmType(S32 chn, int type);

S32 ISIL_NetSDK_ActiveMediaSdkOsd(S32 channel, S32 netfd, void*buff);

#ifdef __cplusplus
}
#endif
#endif

