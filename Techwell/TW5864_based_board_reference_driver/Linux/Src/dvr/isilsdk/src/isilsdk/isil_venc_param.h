#ifndef  _ISIL_VENC_PARAM_H
#define _ISIL_VENC_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif


#include "isil_media_config.h"
#include "isil_encoder.h"
#include "isil_vi.h"
#include "isil_codec_chip_ctl.h"



typedef enum{

    eVENCSETPARAMS = 0xf0000001,

}EVENCPARAMTYPE;


typedef struct
{
     enum ECHANFUNCPRO streamType; // main/assistant stream ;
     unsigned char channel_enable[2];
     unsigned char channel_open[2];
     VENC_H264_CFG venc_h264cfg[2];
	 VENC_H264_FEATURE venc_h264feature[2];
	 VENC_H264_RC venc_h264rc[2];
}VENC_H264_INFO_A;

typedef struct  {
    unsigned short chip_id;
    unsigned short chn;
    VENC_H264_INFO_A venc_h264_inf;
    int result;
}VENC_H264_PARAM;

typedef struct {
 unsigned short chip_id;
 unsigned short chn;
 enum ECHANFUNCPRO streamType;
 VENC_H264_CFG cfg;
}VENC_H264_CFG_PARM;

typedef struct {
 unsigned short chip_id;
 unsigned short chn;
 enum ECHANFUNCPRO streamType;
 VENC_H264_FEATURE cfg;
}VENC_H264_FEATURE_PARM;

typedef struct {
 unsigned short chip_id;
 unsigned short chn;
 enum ECHANFUNCPRO streamType;
 VENC_H264_RC cfg;

}VENC_H264_RC_PARM;

typedef struct {
 unsigned short chip_id;
 unsigned short chn_id;
 //unsigned short videoStandard;
 //CODEC_CHIP_VIDEO_STD videoStandard;
 VI_CH_VIDEOTYPE videoStandard;
}VENC_H264_VIDEO_STANDARD_PARM;


typedef struct {
    unsigned short chip_id;
    unsigned short chn;
    enum ECHANFUNCPRO streamType;
    VI_CH_CFG cfg;
}VENC_H264_VI_CFG_PARM;

typedef struct {
    unsigned short chip_id;
    unsigned short chn;
    enum ECHANFUNCPRO streamType;
    VENC_AUD_CFG cfg;
}VENC_AUDIO_CFG_PARM;

typedef struct {
    unsigned short chip_id;
    unsigned short chn;
    enum ECHANFUNCPRO streamType;
    VENC_MJPG_CFG cfg;
}VENC_MJPEG_CFG_PARM;

typedef struct
{
    unsigned int chip_id;
    VD_CONFIG_REALTIME vd;
}VENC_VD_CONFIG_REALTIME;


typedef struct {
    unsigned short chip_id;
    unsigned short chn;
    enum ECHANFUNCPRO streamType;
    ISIL_OSD_CFG cfg;
}ISIL_OSD_CFG_PARM;


extern int isil_set_single_venc_param_cb(void *arg);

extern int isil_set_venc_params(VENC_H264_PARAM *venc_h264_param_array);

extern int isil_set_venc_h264_cfg_parm(VENC_H264_CFG_PARM *venc_h264_cfg);

extern int isil_set_venc_h264_feature_parm(VENC_H264_FEATURE_PARM *venc_h264_feature);

extern int isil_set_venc_h264_rc_parm(VENC_H264_RC_PARM *venc_h264_rc);

extern int isil_set_venc_video_standard_parm(VENC_H264_VIDEO_STANDARD_PARM *video_std);

extern int isil_set_venc_vi_cfg_parm(VENC_H264_VI_CFG_PARM *vi_cfg);

extern int isil_set_venc_audio_cfg_parm(VENC_AUDIO_CFG_PARM *audio_cfg);

extern int isil_set_venc_mjpeg_cfg_parm(VENC_MJPEG_CFG_PARM *mjpeg_cfg);

extern int isil_venc_mjpeg_startcap(VENC_MJPEG_CFG_PARM *mjpeg_cfg);

extern int isil_venc_mjpeg_stopcap(VENC_MJPEG_CFG_PARM *mjpeg_cfg);
extern int isil_set_osd_cfg(ISIL_OSD_CFG_PARM *mjpeg_cfg);
extern int isil_clac_set_vd(unsigned int chipID, VD_CONFIG_REALTIME* cfg);

extern int isil_venc_set_vd_config_realtime(VENC_VD_CONFIG_REALTIME *vd_cfg);
#ifdef __cplusplus
}
#endif

#endif


