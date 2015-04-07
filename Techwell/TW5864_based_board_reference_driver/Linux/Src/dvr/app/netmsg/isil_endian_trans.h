#ifndef _ISIL_ENDIAN_TRANS_H_
#define _ISIL_ENDIAN_TRANS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "isil_netsdk_netmsg.h"
#include "isil_chan_chip_map.h"

void ISIL_TransPlayNetInterface(ISIL_ASK_NETINFO_S* psNetLinkType);

void ISIL_TransPlayNetInfo(ISIL_NET_INFO_DATA_S* cfg);

void ISIL_TransDevBaseInfo(ISIL_DEV_BASE_CFG_S *sDevInfo);

void ISIL_TransNetworkConfig(ISIL_NET_CFG_S *param);

void ISIL_TransRect(U32 *rect, S32 size);

void ISIL_TransMotionCfg(ISIL_VIDEO_MOTION_CFG_S* cfg);


void ISIL_TransLostCfg(ISIL_VIDEO_LOST_CFG_S* cfg);

void ISIL_TransMaskCfg(ISIL_VIDEO_MASK_CFG_S* cfg);

void ISIL_TransShelterCfg(ISIL_VIDEO_SHELTER_CFG_S* cfg);

void ISIL_TransStreamCfg(ISIL_STREAM_CFG_S* cfg);
void ISIL_LOCAL_TransSysAttrCfg(ISIL_DEV_SYSTEM_ATTR_CFG* cfg);

void ISIL_LOCAL_Trans_VENC_MJPG_CFG(VENC_MJPG_CFG* cfg);
void ISIL_LOCAL_Trans_VENC_H264_FEATURE(VENC_H264_FEATURE* cfg);
void ISIL_LOCAL_Trans_VENC_H264_RC(VENC_H264_RC* cfg);

void ISIL_LOCAL_Trans_VENC_H264_CFG(VENC_H264_CFG* cfg);

void ISIL_LOCAL_Trans_VENC_AUD_CFG(VENC_AUD_CFG* cfg);

void ISIL_LOCAL_Trans_VENC_H264_INFO(VENC_H264_INFO* cfg);


void ISIL_LOCAL_Trans_VENC_MJPG_INFO(VENC_MJPG_INFO* cfg);

void ISIL_LOCAL_Trans_VENC_AUDIO_INFO(VENC_AUDIO_INFO* cfg);

void ISIL_LOCAL_Trans_NotRT(ENC_CTL_VD_INFO* cfg);

void ISIL_LOCAL_Trans_AD(VI_ANALOG_INFO* cfg);
void ISIL_Trans264Feature(VENC_H264_FEATURE* cfg);
void ISIL_TransSubWindows(ISIL_SUB_WINDOWS* cfg);

void ISIL_TransFileBindWin(ISIL_SUB_WINDOWS_BIND* cfg);
void ISIL_TransDisplayPortMode(ISIL_DIAPLAY_PORT_MODE* cfg);
void ISIL_LOCAL_TransChanelCfg(ISIL_DEV_CHANNEL_CFG *cfg);
void ISIL_TransTestReg(ISIL_TEST_REG_INFO* cfg);

void ISIL_TransTestMPB(ISIL_TEST_MPB_INFO* cfg);

void ISIL_TransTestI2C(ISIL_TEST_I2C_INFO* cfg);

void ISIL_TransTestDDR(ISIL_TEST_DDR_INFO* cfg);

void ISIL_TransTestGPIO(ISIL_TEST_GPIO_INFO* cfg);

void ISIL_TransAlarmType(unsigned int* type);
void ISIL_TransFileOpen(ISIL_NETFILE_OPEN * pFileOpen);
void ISIL_TransFileCmmd(NET_FILE_COMMAND *cmd);
void ISIL_TransLocalOSDCfg(ISIL_OSD_CFG* cfg);
void TransMediaSdkOsdByCh(S32 channel,void *buff);

extern void ISIL_TransChipsChanPriMap(SYS_CHIPS_CHAN_PRI_MAP_T *pri_map);

extern void ISIL_TransTime(ISIL_TIME* time);

extern void ISIL_TransFileBaseInfo(ISIL_FILE_BASE_INFO *info);


extern void debug_venc_h264_feature(VENC_H264_FEATURE* cfg);

extern void debug_venc_h264_cfg(VENC_H264_CFG* cfg);

extern void debug_venc_h264_rc(VENC_H264_RC* cfg);
#ifdef __cplusplus
}
#endif
#endif



