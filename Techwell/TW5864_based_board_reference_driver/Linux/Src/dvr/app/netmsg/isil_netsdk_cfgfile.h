#ifndef __ISIL_NET_SDK_CFG_FILE_H__
#define __ISIL_NET_SDK_CFG_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_cfg_file_manage.h"
#include "isil_netsdk_netmsg.h"
#include "isil_endian_trans.h"


extern S32 ISIL_GetDeviceInfo(void *buff);
extern S32 ISIL_SetDeviceInfo(void *buff);
extern S32 ISIL_GetSysAttrCfg(void *buff);
extern S32 ISIL_SetSysAttrCfg(void *buff);
extern S32 ISIL_GetNetCfg(void *buff);
extern S32 ISIL_SetNetCfg(void *buff);
extern S32 ISIL_GetChannelCfg(S32 channel,void *buff);
extern S32 ISIL_SetChannelCfg(S32 channel,void *buff);
extern S32 ISIL_GetRecordCfg(S32 channel,void *buff);
extern S32 ISIL_SetRecordCfg(S32 channel,void *buff);
extern S32 ISIL_GetNetSDKComCfg(void *buff);
extern S32 ISIL_SetNetSDKComCfg(void *buff);
extern S32 ISIL_SetSnapPictureCfg(S32 channel,void *buff);
extern S32 ISIL_GetSnapPictureCfg(S32 channel,void *buff);
extern S32 ISIL_GetMediaSdkEncode(S32 channel,void *buff);
extern S32 ISIL_SetMediaSdkEncode(S32 channel,void *buff);
extern S32 ISIL_GetAllChMjpegCfg(S32 channel,void *buff);
extern S32 ISIL_GetMediaSdkMjpeg(S32 channel,void *buff);
extern S32 ISIL_SetMediaSdkMjpeg(S32 channel,void *buff);
extern S32 ISIL_GetMediaSdkAudio(S32 channel, void *buff);
extern S32 ISIL_SetMediaSdkAudio(S32 channel,void *buff);
extern S32 ISIL_GetMediaSdkNoRT(void *buff);
extern S32 ISIL_SetMediaSdkNoRT(S32 channel,void *buff);
extern S32 ISIL_SetMediaSdkAD(S32 channel,void *buff);
extern S32 ISIL_GetMediaSdkAD(S32 channel,void *buff);
extern S32 ISIL_SaveMediaSdkADDefault();
extern S32 ISIL_GetMediaSdkEncodeDefault(VENC_H264_INFO* tmp);
extern S32 ISIL_SaveMediaSdkEncodeDefault();
extern S32 ISIL_GetLocGuiRecordCfg(S32 channel,void *buff);
extern S32 ISIL_SetLocGuiRecordCfg(S32 channel,void *buff);
extern S32 ISIL_GetLocGuiAlarmTypeCfg(void *buff);
extern  S32 ISIL_SetLocGuiAlarmTypeCfg(S32 channel,void *buff);
extern S32 ISIL_SetMediaSdkOsd(S32 channel,void *buff);
extern S32 ISIL_GetMediaSdkOsd(S32 channel,void *buff);
#ifdef __cplusplus
}
#endif
#endif

