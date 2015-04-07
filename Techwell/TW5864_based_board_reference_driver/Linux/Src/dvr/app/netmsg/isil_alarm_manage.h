#ifndef __ISIL_ALARM_MANAGE_H__
#define __ISIL_ALARM_MANAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_messagestruct.h"

enum eALARM_CLASS
{
    ALARM_IN = 0,
    ALARM_OUT,
    ALARM_LINKAGE,
    ALARM_STATUS,
    ALARM_MAX,
};



extern S32 ISIL_GetAlarmCfg(U32 iChannel, S32 iAlarmClass, S32 iAlarmType, S32 iLinkageType,
                        S32 iCfgSize, void *pAlarmCfg);

extern S32 ISIL_SaveAlarmCfg(U32 iChannel, S32 iAlarmClass, S32 iAlarmType, S32 iLinkageType,
                        S32 iCfgSize, void *pAlarmCfg);

extern S32 ISIL_AlarmLinkageCfgHandle(U32 Channel, S32 iAlarmType, U32 lLinkageType,
                U32 lIoFlag, S32 iLinkageSize, void* pLinkage);
extern S32 ISIL_GetPcEventSize(U32 iChannel,S32 iAlarmType);

extern S32 ISIL_HandleOutCtrlLinkage(void* pLinkageInfo);

extern S32 ISIL_HandlePtzLinkage(void* pLinkageInfo);
extern S32 ISIL_SetAlarmInParameter(S32 gpio_no, S32 val);

#ifdef __cplusplus
}
#endif
#endif
