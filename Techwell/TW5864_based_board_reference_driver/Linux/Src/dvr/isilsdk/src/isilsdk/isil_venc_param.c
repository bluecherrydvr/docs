#include <stdio.h>
#include <stdlib.h>

#include "isil_venc_param.h"

#include "isil_msg.h"

#include "isil_media_config.h"

#include "isil_venc_audio.h"

#include "isil_venc_mjpg.h"

#include "isil_private_func.h"




int isil_set_single_venc_param_cb(void *arg)
{

    int ret, i;
    enum ECHANFUNCPRO eStreamType;

    VENC_H264_INFO_A *pEncodeInfo;
    VENC_H264_PARAM *h264_param = (VENC_H264_PARAM *)arg;

    pEncodeInfo = &h264_param->venc_h264_inf;

    eStreamType = pEncodeInfo->streamType;

    if( !h264_param ) {
        return -1;
    }

    if(eStreamType >= IS_H264_SUB_STREAM) {
        return -1;
    }

    if(eStreamType == IS_H264_MAIN_STREAM) {
        i = 0;
    }
    else{
        i = 1;
    }

    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(h264_param->chip_id,
                                                   h264_param->chn ,
                                                   CHAN_IS_ENC,
                                                   eStreamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_param failed .\n");
        return -1;
    }



    if(pEncodeInfo->channel_enable[i]){
        ret = ISIL_CODEC_ENC_EnableCh(&sin_chn_mgt->codec_channel_t);
        if( ret !=  CODEC_ERR_OK) {
        }
    } else{
        ret = ISIL_CODEC_ENC_DisableCh(&sin_chn_mgt->codec_channel_t);
        if( ret !=  CODEC_ERR_OK) {

        }
    }

    pEncodeInfo->venc_h264cfg[i].change_mask_flag =0xffffffff;
	ret = ISIL_CODEC_ENC_SetCfg(&sin_chn_mgt->codec_channel_t, (void *)&pEncodeInfo->venc_h264cfg[i]);
    if(ret < 0)
        return -1;
	pEncodeInfo->venc_h264feature[i].change_mask_flag = 0xffffffff;
    ret = ISIL_CODEC_ENC_SetFeature(&sin_chn_mgt->codec_channel_t, (void *)&pEncodeInfo->venc_h264feature[i]);
    if(ret < 0)
        return -1;
	pEncodeInfo->venc_h264rc[i].change_mask_flag =0xffffffff;
    ret = ISIL_CODEC_ENC_SetRC(&sin_chn_mgt->codec_channel_t, (void *)&pEncodeInfo->venc_h264rc[i]);
    if(ret < 0)
        return -1;

    return 0;
}



int isil_set_venc_h264_cfg(VENC_H264_CFG_PARM *h264_cfg)
{

    int ret;

    VENC_H264_CFG *h264_cfg_ptr;

    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(h264_cfg->chip_id,
                                                   h264_cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   h264_cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_h264_cfg failed .\n");
        return -1;
    }

    h264_cfg_ptr = &h264_cfg->cfg;

	ret = ISIL_CODEC_ENC_SetCfg(&sin_chn_mgt->codec_channel_t, h264_cfg_ptr);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_venc_h264_cfg set parm fail .\n");
        return -1;
    }
    return 0;
}

int isil_set_venc_h264_feature(VENC_H264_FEATURE_PARM *cfg)
{

    int ret;

    VENC_H264_FEATURE *h264_feature_ptr;
    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

    if(NULL == cfg)
    {
        fprintf(stderr,"isil_set_venc_h264_feature parm is null .\n");
        return -1;
    }
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(cfg->chip_id,
                                                   cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_h264_feature failed .\n");
        return -1;
    }

    h264_feature_ptr = &cfg->cfg;

	ret = ISIL_CODEC_ENC_SetFeature(&sin_chn_mgt->codec_channel_t, h264_feature_ptr);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_venc_h264_feature set parm fail .\n");
        return -1;
    }
    return 0;
}


int isil_set_venc_h264_rc(VENC_H264_RC_PARM *cfg)
{

    int ret;

    VENC_H264_RC* h264_rc_ptr;
    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

    if(NULL == cfg)
    {
        fprintf(stderr,"isil_set_venc_h264_feature parm is null .\n");
        return -1;
    }
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(cfg->chip_id,
                                                   cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_h264_rc failed .\n");
        return -1;
    }

    h264_rc_ptr = &cfg->cfg;

	ret = ISIL_CODEC_ENC_SetRC(&sin_chn_mgt->codec_channel_t, h264_rc_ptr);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_venc_h264_rc set parm fail .\n");
        return -1;
    }

    return 0;
}

int isil_set_venc_h264_vi_cfg(VENC_H264_VI_CFG_PARM *cfg)
{

    int ret;

    VI_CH_CFG* h264_vi_cfg_ptr;
    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

    if(NULL == cfg)
    {
        fprintf(stderr,"isil_set_venc_h264_vi_cfg parm is null .\n");
        return -1;
    }
    /*
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(cfg->chip_id,
                                                   cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_h264_vi_cfg failed .\n");
        return -1;
    }
    */

    h264_vi_cfg_ptr = &cfg->cfg;

	ret = ISIL_VI_SetCfg(cfg->chip_id, cfg->chn, h264_vi_cfg_ptr);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_venc_h264_vi_cfg set parm fail .\n");
        return -1;
    }

    return 0;
}

int isil_set_venc_audio_cfg(VENC_AUDIO_CFG_PARM *cfg)
{
    int ret;
    VENC_AUD_CFG* audio_cfg_ptr;
    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

    if(NULL == cfg)
    {
        fprintf(stderr,"isil_set_venc_audio_cfg parm is null .\n");
        return -1;
    }
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(cfg->chip_id,
                                                   cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_audio_cfg failed .\n");
        return -1;
    }

    audio_cfg_ptr = &cfg->cfg;

    ret = ISIL_VENC_AUD_SetChCfg(&sin_chn_mgt->codec_channel_t, audio_cfg_ptr);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_venc_audio_cfg set parm fail .\n");
        return -1;
    }

    return 0;
}

int isil_set_venc_mjpeg_cfg(VENC_MJPEG_CFG_PARM *cfg)
{
    int ret;
    VENC_MJPG_CFG* cfg_ptr;
    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

    if(NULL == cfg)
    {
        fprintf(stderr,"isil_set_venc_mjpeg_cfg parm is null .\n");
        return -1;
    }
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(cfg->chip_id,
                                                   cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_mjpeg_cfg failed .\n");
        return -1;
    }

    cfg_ptr = &cfg->cfg;

    ret = ISIL_VENC_MJPG_SetChCfg(&sin_chn_mgt->codec_channel_t, cfg_ptr);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_venc_mjpeg_cfg set parm fail .\n");
        return -1;
    }

    return 0;
}


int isil_set_venc_mjpeg_startcap(VENC_MJPEG_CFG_PARM *cfg)
{
    int ret;
    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

    if(NULL == cfg)
    {
        fprintf(stderr,"isil_set_venc_mjpeg_startcap parm is null .\n");
        return -1;
    }
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(cfg->chip_id,
                                                   cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_mjpeg_startcap failed .\n");
        return -1;
    }

    ret = ISIL_VENC_MJPG_StartCap(&sin_chn_mgt->codec_channel_t, cfg->cfg.i_capture_type);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_venc_mjpeg_startcap start cap fail .\n");
        return -1;
    }

    return 0;
}

int isil_set_venc_mjpeg_stopcap(VENC_MJPEG_CFG_PARM *cfg)
{
    int ret;
    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

    if(NULL == cfg)
    {
        fprintf(stderr,"isil_set_venc_mjpeg_stopcap parm is null .\n");
        return -1;
    }
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(cfg->chip_id,
                                                   cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_venc_mjpeg_stopcap failed .\n");
        return -1;
    }

    ret = ISIL_VENC_MJPG_StopCap(&sin_chn_mgt->codec_channel_t, cfg->cfg.i_capture_type);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_venc_mjpeg_stopcap start cap fail .\n");
        return -1;
    }

    return 0;
}

int isil_set_osd_parm(ISIL_OSD_CFG_PARM *cfg)
{
    int ret;
    S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

    if(NULL == cfg)
    {
        fprintf(stderr,"isil_set_osd_parm parm is null .\n");
        return -1;
    }
    sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(cfg->chip_id,
                                                   cfg->chn ,
                                                   CHAN_IS_ENC,
                                                   cfg->streamType);
    if( !sin_chn_mgt ) {
        fprintf(stderr,"isil_set_osd_parm failed .\n");
        return -1;
    }
    fprintf(stderr, "%s :osd na %d nx %d ny %d ta %d tx %d ty %d sh1a %d sh1x %d sh1y %d sh2a %d sh2x %d sh2y %d\n",
            __FUNCTION__, cfg->cfg.name_attrib, cfg->cfg.name_pos_x, cfg->cfg.name_pos_y,
            cfg->cfg.time_attrib, cfg->cfg.time_pos_x, cfg->cfg.time_pos_y,
            cfg->cfg.shelter1_attrib, cfg->cfg.shelter1_pos_x, cfg->cfg.shelter1_pos_y,
            cfg->cfg.shelter2_attrib, cfg->cfg.shelter2_pos_x, cfg->cfg.shelter2_pos_y);
    ret = ISIL_CODEC_ENC_SetOSDCfg(&sin_chn_mgt->codec_channel_t, (void*)&cfg->cfg);
    if(ret < 0)
    {
        fprintf(stderr,"isil_set_osd_parm fail .\n");
        return -1;
    }

    return 0;
}

int isil_clac_set_vd(unsigned int chipID, VD_CONFIG_REALTIME* cfg)
{
	int ret;
	if(NULL == cfg)
	{
		fprintf(stderr,"%s parm is null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_CalcSetVd(chipID, cfg);
	if(ret < 0)
	{
		fprintf(stderr,"%s set parm fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

int isil_set_venc_h264_cfg_cb(void *arg)
{

    int ret;
    VENC_H264_CFG_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_h264_cfg_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_H264_CFG_PARM*)arg;
    ret = isil_set_venc_h264_cfg(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_h264_cfg_cb failed .\n");
        return -1;
    }
    return 0;
}

int isil_set_venc_h264_feature_cb(void *arg)
{

    int ret;
    VENC_H264_FEATURE_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_h264_feature_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_H264_FEATURE_PARM*)arg;
    ret = isil_set_venc_h264_feature(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_h264_feature_cb failed .\n");
        return -1;
    }
    return 0;
}

int isil_set_venc_h264_rc_cb(void *arg)
{

    int ret;
    VENC_H264_RC_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_h264_rc_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_H264_RC_PARM*)arg;
    ret = isil_set_venc_h264_rc(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_h264_rc_cb failed .\n");
        return -1;
    }
    return 0;
}

int isil_set_venc_video_standard_cb(void *arg)
{

    int ret;
    VENC_H264_VIDEO_STANDARD_PARM* ptr;
    //CODEC_CHIP_VIDEO_STD VideoStd;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_video_standard_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_H264_VIDEO_STANDARD_PARM*)arg;

    //VideoStd.e_video_standard = ptr->videoStandard;
    //ret = ISIL_CODEC_CTL_SetVideoSTD(ptr->chip_id, &ptr->videoStandard);
    ret = ISIL_VI_SetVideoType(ptr->chip_id, ptr->chn_id, &ptr->videoStandard);

    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_video_standard_cb failed .\n");
        return -1;
    }
    return 0;
}

int isil_set_venc_h264_vi_cfg_cb(void *arg)
{

    int ret ;
    VENC_H264_VI_CFG_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_h264_rc_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_H264_VI_CFG_PARM*)arg;
    ret = isil_set_venc_h264_vi_cfg(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_h264_rc_cb failed .\n");
        return -1;
    }
    return 0;
}


int isil_set_venc_audio_cfg_cb(void *arg)
{
    int ret;
    VENC_AUDIO_CFG_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_h264_rc_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_AUDIO_CFG_PARM*)arg;
    ret = isil_set_venc_audio_cfg(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_h264_rc_cb failed .\n");
        return -1;
    }
    return 0;
}

int isil_set_venc_mjpeg_cfg_cb(void *arg)
{
    int ret;
    VENC_MJPEG_CFG_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_h264_rc_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_MJPEG_CFG_PARM*)arg;
    ret = isil_set_venc_mjpeg_cfg(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_h264_rc_cb failed .\n");
        return -1;
    }
    return 0;
}

int isil_set_venc_mjpeg_startcap_cb(void *arg)
{
    int ret;
    VENC_MJPEG_CFG_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_h264_rc_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_MJPEG_CFG_PARM*)arg;
    ret = isil_set_venc_mjpeg_startcap(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_h264_rc_cb failed .\n");
        return -1;
    }
    return 0;
}

int isil_set_venc_mjpeg_stopcap_cb(void *arg)
{
    int ret;
    VENC_MJPEG_CFG_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_h264_rc_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_MJPEG_CFG_PARM*)arg;
    ret = isil_set_venc_mjpeg_stopcap(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_h264_rc_cb failed .\n");
        return -1;
    }
    return 0;
}
int isil_set_venc_vd_config_realtime_cb(void *arg)
{
    int ret;
    VENC_VD_CONFIG_REALTIME* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_venc_vd_config_realtime_cb parm is null .\n");
        return -1;
    }
    ptr = (VENC_VD_CONFIG_REALTIME*)arg;
    ret = ISIL_CODEC_CTL_CalcSetVd(ptr->chip_id, &ptr->vd);
    if(ret < 0) {
        fprintf(stderr,"isil_set_venc_vd_config_realtime_cb failed .\n");
        return -1;
    }
    return 0;
}


int isil_set_osd_parm_cb(void *arg)
{
    int ret;
    ISIL_OSD_CFG_PARM* ptr;

    if(NULL == arg)
    {
        fprintf(stderr,"isil_set_osd_parm_cb parm is null .\n");
        return -1;
    }
    ptr = (ISIL_OSD_CFG_PARM*)arg;
    ret = isil_set_osd_parm(ptr);
    if(ret < 0) {
        fprintf(stderr,"isil_set_osd_parm_cb failed .\n");
        return -1;
    }
    return 0;
}




int isil_set_venc_params(VENC_H264_PARAM *venc_h264_param_array)
{
    return isil_msg_set_param((void *)venc_h264_param_array ,isil_set_single_venc_param_cb ,0 );
}


int isil_set_venc_h264_cfg_parm(VENC_H264_CFG_PARM *venc_h264_cfg)
{
    return isil_msg_set_param((void *)venc_h264_cfg ,isil_set_venc_h264_cfg_cb ,0 );
}

int isil_set_venc_h264_feature_parm(VENC_H264_FEATURE_PARM *venc_h264_feature)
{
    return isil_msg_set_param((void *)venc_h264_feature ,isil_set_venc_h264_feature_cb ,0 );
}

int isil_set_venc_h264_rc_parm(VENC_H264_RC_PARM *venc_h264_rc)
{
    return isil_msg_set_param((void *)venc_h264_rc ,isil_set_venc_h264_rc_cb ,0 );
}

int isil_set_venc_video_standard_parm(VENC_H264_VIDEO_STANDARD_PARM *video_std)
{
    return isil_msg_set_param((void *)video_std ,isil_set_venc_video_standard_cb ,0 );
}

int isil_set_venc_vi_cfg_parm(VENC_H264_VI_CFG_PARM *vi_cfg)
{
    return isil_msg_set_param((void*)vi_cfg, isil_set_venc_h264_vi_cfg_cb, 0);
}

int isil_set_venc_audio_cfg_parm(VENC_AUDIO_CFG_PARM *audio_cfg)
{

    return isil_msg_set_param((void*)audio_cfg, isil_set_venc_audio_cfg_cb, 0);
}

int isil_set_venc_mjpeg_cfg_parm(VENC_MJPEG_CFG_PARM *mjpeg_cfg)
{
    return isil_msg_set_param((void*)mjpeg_cfg, isil_set_venc_mjpeg_cfg_cb, 0);
}

int isil_venc_mjpeg_startcap(VENC_MJPEG_CFG_PARM *mjpeg_cfg)
{

    return isil_msg_set_param((void*)mjpeg_cfg, isil_set_venc_mjpeg_startcap_cb, 0);
}

int isil_venc_mjpeg_stopcap(VENC_MJPEG_CFG_PARM *mjpeg_cfg)
{

    return isil_msg_set_param((void*)mjpeg_cfg, isil_set_venc_mjpeg_stopcap_cb, 0);
}

int isil_venc_set_vd_config_realtime(VENC_VD_CONFIG_REALTIME *vd_cfg)
{

    return isil_msg_set_param((void*)vd_cfg, isil_set_venc_vd_config_realtime_cb, 0);
}

int isil_set_osd_cfg(ISIL_OSD_CFG_PARM *osdcfg)
{
	return isil_msg_set_param((void*)osdcfg, isil_set_osd_parm_cb, 0);
}

