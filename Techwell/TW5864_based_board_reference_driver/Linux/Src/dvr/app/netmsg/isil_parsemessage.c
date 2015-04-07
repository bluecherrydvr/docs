/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_handlemessages.c
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-20
	Description:handle message: receive message, send message, parse message
	Other:
	Function list:
	History:

*******************************************************************
*/
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <semaphore.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include "isil_channel_map.h"

#include "isil_parsemessage.h"
#include "isil_debug.h"
#include "isil_usr_manage.h"
#include "isil_dev_manage.h"
#include "isil_net_manage.h"
#include "isil_video_manage.h"
#include "isil_alarm_manage.h"
#include "isil_stream_manage.h"
#include "isil_common.h"

#include "config.h"
#include "net_config.h"
#include "isil_cfg_file_manage.h"
#include "isil_searchdev.h"
#include "isil_netsdk_netmsg.h"
#include "isil_endian_trans.h"
#include "default_cfg_value.h"
#include "isil_ap_mediasdk.h"
#include "isil_data_stream.h"
#include "isil_msg_reg_cb.h"
#include "isil_fm_play.h"
#include "isil_endian_trans.h"
#include "isil_channel_map.h"
#include "isil_netsdk_cfgfile.h"

/*
*	Function define
*/
S32 ISIL_APP_SetVD(G_STREAM_CFG* pCfg);

/*传输的net ctl ID号，到了最大就清0*/
static unsigned short nctl_trans_id = 0;

static inline void add_nctl_trans_id(void)
{
	if(nctl_trans_id != 65535){
		nctl_trans_id++;
	}
	else
		nctl_trans_id= 0;
}

/*超时*/
#define NCTL_DEFAULT_MAX_TIMEOUT (3)
static S32 ISIL_SetMDActive(S32 channel, ISIL_VIDEO_MOTION_CFG_S* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set motion");
    S32 ret;
    G_VIDEO_MOTION_CFG_S sVideoMotion;
    sVideoMotion.channel = channel;

    memcpy(&sVideoMotion.sMotionCfg, pCfg, sizeof(ISIL_VIDEO_MOTION_CFG_S));

    ret = ISIL_SaveVideoCfg(channel, VIDEO_MOTION,
        sizeof(G_VIDEO_MOTION_CFG_S), &sVideoMotion);
    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    ret = ISIL_SetAlarmActive(channel, ISIL_ALARM_TYPE_VIDEO_MOTION,
        &sVideoMotion);
    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    return ISIL_SUCCESS;
}

static S32 ISIL_SetLostActive(S32 channel, ISIL_VIDEO_LOST_CFG_S* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set lost");
    S32 ret;
    G_VIDEO_LOST_CFG_S sVideoLost;
    sVideoLost.channel = channel;
    memcpy(&sVideoLost.sLostCfg, pCfg, sizeof(ISIL_VIDEO_LOST_CFG_S));

    ret = ISIL_SaveVideoCfg(channel, VIDEO_LOST,
        sizeof(G_VIDEO_LOST_CFG_S), &sVideoLost);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    ret = ISIL_SetAlarmActive(channel, ISIL_ALARM_TYPE_VIDEO_LOST, &sVideoLost);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;
    return ISIL_SUCCESS;
}

static S32 ISIL_SetShelterActive(S32 channel, ISIL_VIDEO_SHELTER_CFG_S* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set shelter");
    S32 ret;
    G_VIDEO_SHELTER_CFG_S stVideoShelter;
    stVideoShelter.channel = channel;
    memcpy(&stVideoShelter.sShelterCfg, pCfg, sizeof(ISIL_VIDEO_SHELTER_CFG_S));
    ret = ISIL_SaveVideoCfg(channel, VIDEO_SHELTER,
        sizeof(G_VIDEO_SHELTER_CFG_S), &stVideoShelter);
    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    ret = ISIL_SetAlarmActive(channel, ISIL_ALARM_TYPE_VIDEO_SHELTER,
        &stVideoShelter);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    return ISIL_SUCCESS;
}
static S32 ISIL_SetMaskActive(S32 iConnectfd, S32 channel, ISIL_VIDEO_MASK_CFG_S* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set mask");
    S32 ret;
    G_VIDEO_MASK_CFG_S stVideoMask;
    stVideoMask.channel = channel;
    memcpy(&stVideoMask.sMaskCfg, pCfg, sizeof(ISIL_VIDEO_MASK_CFG_S));
    ret = ISIL_SaveVideoCfg(channel, VIDEO_MASK,
        sizeof(G_VIDEO_MASK_CFG_S), &stVideoMask);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;
/*
    ret = ISIL_SendMsgToOther(ALARM_MOD, G_MSG_SET_VIDEOMASKCFG,
				sizeof(G_VIDEO_MASK_CFG_S),  iConnectfd,  ISIL_PC_MSG,
				&stVideoMask);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;
*/

    return ISIL_SUCCESS;
}
static S32 ISIL_SetOverlayActive(S32 iConnectfd, S32 channel, ISIL_OVERLAY_CFG_S* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set overlay");
    S32 ret;
    ISIL_OSD_CFG cfg;
    CHIP_CHN_PHY_ID id;

    memset(&cfg, 0x00, sizeof(ISIL_OSD_CFG));
    #ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &id);
    #else
    ISIL_GetChipChnIDByLogicID(channel,&id);
    #endif

    /*设置OSD*/
    /*
     * memcpy(&stOSDInfo.sOverlayerCfg, pCfg, sizeof(ISIL_OVERLAY_CFG_S));
    ret = ISIL_SendMsgToOther(DRV_MOD, G_MSG_SET_VIDEOOVERLAYCFG,
                sizeof(G_OVERLAY_CFG_S),  iConnectfd, ISIL_PC_MSG,&stOSDInfo);
    if(ISIL_FAIL == ret)
        return ISIL_FAIL;
    */
    fprintf(stderr, "%s, ch name %s x %d y %d, time x %d y %d\n",
    		__FUNCTION__, pCfg->sChannelName, pCfg->nNameTopLeftX, pCfg->nNameTopLeftY,
    		pCfg->nTimeTopLeftX, pCfg->nTimeTopLeftY);
    cfg.channel = channel;

    if(pCfg->cShowName)
    {
        memcpy(cfg.name, pCfg->sChannelName, ISIL_NAME_LEN);
        cfg.name_pos_x = pCfg->nNameTopLeftX;
        cfg.name_pos_y = pCfg->nNameTopLeftY;
        cfg.name_attrib = pCfg->cShowName;
    }
    
    if(pCfg->cShowTime)
    {
        cfg.time_pos_x = pCfg->nTimeTopLeftX;
        cfg.time_pos_y = pCfg->nTimeTopLeftY;
        cfg.time_attrib = pCfg->cShowTime;
    }
    
    if(pCfg->stText[0].cShowString)
    {
        memcpy(cfg.subtitle1, pCfg->stText[0].sText, SUB_LEN);
        cfg.subtitle1_pos_x = pCfg->stText[0].nTextTopLeftX;
        cfg.subtitle1_pos_y = pCfg->stText[0].nTextTopLeftY;
        cfg.subtitle1_attrib = pCfg->stText[0].cShowString;
    }
	
    if(pCfg->stText[1].cShowString)
    {
        memcpy(cfg.subtitle2, pCfg->stText[1].sText, SUB_LEN);
        cfg.subtitle2_pos_x = pCfg->stText[1].nTextTopLeftX;
        cfg.subtitle2_pos_y = pCfg->stText[1].nTextTopLeftY;
        cfg.subtitle2_attrib = pCfg->stText[1].cShowString;
    }
	
    if(pCfg->stText[2].cShowString)
    {
        memcpy(cfg.subtitle3, pCfg->stText[2].sText, SUB_LEN);
        cfg.subtitle3_pos_x = pCfg->stText[2].nTextTopLeftX;
    	cfg.subtitle3_pos_y = pCfg->stText[2].nTextTopLeftY;
        cfg.subtitle3_attrib = pCfg->stText[2].cShowString;
    }
	
    if(pCfg->stText[3].cShowString)
    {
        memcpy(cfg.subtitle4, pCfg->stText[3].sText, SUB_LEN);
        cfg.subtitle4_pos_x = pCfg->stText[3].nTextTopLeftX;
        cfg.subtitle4_pos_y = pCfg->stText[3].nTextTopLeftY;
        cfg.subtitle4_attrib = pCfg->stText[3].cShowString;
    }

    ret = ISIL_MediaSDK_SetOsdCfg(id.chipid, id.chanid,
    		&cfg,  IS_H264_MAIN_STREAM);
    ret = ISIL_MediaSDK_SetOsdCfg(id.chipid, id.chanid,
    		&cfg,  IS_H264_SUB_STREAM);
    return ret;
}
static S32 ISIL_SetPTZActive(S32 channel, void* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set ptz");

    return ISIL_SUCCESS;
}
static S32 ISIL_SetPTZPresetActive(S32 channel, void* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set ptz preset");

    return ISIL_SUCCESS;
}


static void ISIL_SetVideoStander(S32 stander)
{
    S32 i;
    G_VIDEO_SIGNAL_CFG_S sVideoSignal;
    memset(&sVideoSignal, 0x00, sizeof(G_VIDEO_SIGNAL_CFG_S));
    #ifndef ISIL_USE_SYS_CFG_FILE
    for(i = 0; i < CHANNEL_NUMBER; i++)
    #else
    for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
    #endif
    {
        sVideoSignal.channel = i;
        sVideoSignal.cmdType = ISIL_PC_SET_PARAM;
        sVideoSignal.sVideoSignalCfg.cVideoStandard = stander;
        /*
        ISIL_SendMsgToOther(DRV_MOD, G_MSG_SET_VIDEOSIGNALCFG,
            sizeof(G_VIDEO_SIGNAL_CFG_S),  -1, ISIL_PC_MSG,&sVideoSignal);
        */

    }
    return;
}

static S32 ISIL_SetSignActive(S32 iConnectfd, S32 channel, ISIL_VIDEO_SIGNAL_CFG_S * pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set sign");
    S32 ret;
    CHIP_CHN_PHY_ID id;
    G_VIDEO_SIGNAL_CFG_S sVideoSignal;



    memset(&sVideoSignal, 0x00, sizeof(G_VIDEO_SIGNAL_CFG_S));
    sVideoSignal.channel = channel;
    sVideoSignal.cmdType = ISIL_PC_SET_PARAM;
	memcpy(&sVideoSignal.sVideoSignalCfg, pCfg, sizeof(ISIL_VIDEO_SIGNAL_CFG_S));

    #ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &id);
    #else
    ISIL_GetChipChnIDByLogicID(channel,&id);
    #endif
    ISIL_TRACE(BUG_NORMAL, "set SIGNAL: %d %d %d %d",
    		pCfg->cBrightness,
    		pCfg->cContrast,
    		pCfg->cHue,
    		pCfg->cSaturation);
    ret = ISIL_MediaSDK_SetVideoPara(id.chipid, id.chanid,
        pCfg->cBrightness, pCfg->cContrast, pCfg->cSaturation, pCfg->cHue, 0);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

	ret = ISIL_SaveVideoCfg(channel, VIDEO_SIGNAL,
	            sizeof(G_VIDEO_SIGNAL_CFG_S), &sVideoSignal);

    return ISIL_SUCCESS;
}

void ISIL_WinToLocalH264CfgTrans(VENC_H264_CFG* local, ISIL_ENCODE_INFO_S* win)
{
    U32 w, h;
    if(NULL == local || NULL == win)
        return;
    local->i_bps = win->nBitRate*1000;
    local->i_fps = win->cFPS;
    local->i_gop_value = win->nGOPIntervals;
    local->i_I_P_stride = win->nIFrameIntervals;
    local->i_P_B_stride = win->nBPFrameInterval;
    local->change_mask_flag = 0xffffffff;

    ISIL_ImageSizeEnumToDigit(win->cImageSize,
        &w,//&(local->i_logic_video_width_mb_size),
        &h,//&(local->i_logic_video_height_mb_size),
        win->nHeightMBSize, win->nWidthMBSize);
    local->i_logic_video_width_mb_size = w;
    local->i_logic_video_height_mb_size = h;
    return;
}

void ISIL_WinToLocalRCCfgTrans(VENC_H264_RC* local, ISIL_ENCODE_INFO_S* win)
{
    if(NULL == local || NULL == win)
        return;
    local->e_image_priority = win->cBitMode;
    local->e_rc_type = win->cRCType;
    local->i_qpb = win->cPQP;
    local->i_qpi = win->cIQP;
    local->i_qpp = win->cPQP;
    return;
}

void ISIL_WinToLocalMjpegCfgTrans(VENC_MJPG_CFG* local, ISIL_MJPEG_CFG* win)
{
    U32 w, h;
    if(NULL == local || NULL == win)
        return;
    local->e_image_level = MJPEG_IMAGE_LEVEL_0;
    //local->i_capture_type = TW_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER;
    local->i_capture_frame_number = 1;
    local->i_capture_frame_stride = 1000/win->cFPS;
    ISIL_ImageSizeEnumToDigit(win->cImageSize,
        &w,
        &h,
        win->nHeightMBSize, win->nWidthMBSize);

        local->i_image_width_mb_size = w;
        local->i_image_height_mb_size = h;
}

void ISIL_WinToLocalAudioCfgTrans(VENC_AUD_CFG* local, ISIL_ENCODE_INFO_S* win, S32 ch)
{
     if(NULL == local || NULL == win)
        return;
     local->channel = ch;
     local->e_audio_type = win->cAudioType;
     local->i_bit_wide = win->cAudioBitAlign;
     local->i_sample_rate = win->cAudioSampleRate;
     return;
}


void ISIL_WinToLocalEncCfgTrans(VENC_H264_INFO* local, G_STREAM_CFG* win, VENC_H264_FEATURE* feature)
{
    if(NULL == local || NULL == win || NULL == feature)
        return;

    ISIL_WinToLocalH264CfgTrans(&local->venc_h264cfg[0],
        &(win->sStreamCfg.stStreamEncode.stStreamEncode));
    ISIL_WinToLocalH264CfgTrans(&local->venc_h264cfg[1],
        &(win->sStreamCfg.stStreamEncode.stStreamEncode2));

    ISIL_WinToLocalRCCfgTrans(&(local->venc_h264rc[0]),
        &(win->sStreamCfg.stStreamEncode.stStreamEncode));
    ISIL_WinToLocalRCCfgTrans(&(local->venc_h264rc[1]),
        &(win->sStreamCfg.stStreamEncode.stStreamEncode));

    memcpy(&local->venc_h264feature[0], feature, sizeof(VENC_H264_FEATURE));
    feature++;
    memcpy(&local->venc_h264feature[1], feature, sizeof(VENC_H264_FEATURE));
}

S32 ISIL_GetEncodeParmByType(S32 ch, S32 type, S32 streamType, void* buff, int len)
{
    S32 ret;
    VENC_H264_INFO* sEncodeH264;
    VENC_AUDIO_INFO* audioCfg;
    VENC_MJPG_INFO* mjpeg;
    VENC_MJPG_INFO localcfg[CHANNEL_NUMBER];
    G_STREAM_CFG cfg;
    VENC_H264_FEATURE feature;
    if(NULL == buff)
        return ISIL_FAIL;
    memset(&cfg, 0x00, sizeof(G_STREAM_CFG));
    memset(&feature, 0x00, sizeof(VENC_H264_FEATURE));
    memset(localcfg, 0x00, sizeof(localcfg));
    cfg.channel = ch;
    ret = ISIL_GetStreamCfg(&cfg);
    if(ret < 0)
        return ISIL_FAIL;

    ret = ISIL_GetH264Cfg(ch, &feature, sizeof(VENC_H264_FEATURE));
    if(ret < 0)
        return ISIL_FAIL;
    ret = ISIL_GetAllChMjpegCfg(-1, localcfg);
    if(ret < 0)
        return ISIL_FAIL;

    switch(type)
    {
        case ISIL_ENC_H264_CFG:
        {
            sEncodeH264 = (VENC_H264_INFO*)buff;
            sEncodeH264->channel_enable[0] = 1;
            sEncodeH264->channel_enable[1] = 1;
            ISIL_WinToLocalEncCfgTrans(sEncodeH264, &cfg, &feature);
        }
        break;
        case ISIL_ENC_MJPG_CFG:
        {
            mjpeg = (VENC_MJPG_INFO*)buff;
            mjpeg->channel_enable[0]=1;
            mjpeg->channel_enable[1]=1;
            ISIL_WinToLocalMjpegCfgTrans(&mjpeg->venc_mjpg_cfg[0], &cfg.sStreamCfg.stMJPEGCfg);
            mjpeg->venc_mjpg_cfg[0].i_capture_type = localcfg[ch].venc_mjpg_cfg[0].i_capture_type;
        }
        break;
        case ISIL_ENC_AUDIO_CFG:
        {
            audioCfg = (VENC_AUDIO_INFO*)buff;
            audioCfg->channel_enable= 1;
            ISIL_WinToLocalAudioCfgTrans((VENC_AUD_CFG* )audioCfg, (ISIL_ENCODE_INFO_S *)(&cfg.sStreamCfg.stStreamEncode), ch);
        }
        break;

        default:
            break;
    }

    return ISIL_SUCCESS;
}


static void ISIL_TransValue_W2E(S32 chipid, ISIL_ENCODE_INFO_S* in, ISIL_ENCODE_INFO_S* out)
{
    enum ISIL_VIDEO_STANDARD chipstd;
    if(NULL == in || NULL == out)
        return;
    printf("enter %s\n", __FUNCTION__);

    memcpy(out, in, sizeof(ISIL_ENCODE_INFO_S));
    ISIL_MediaSDK_GetVideoStandard(chipid, &chipstd);
    switch(in->cAudioType)
    {
        case ISIL_AUDIO_PCM_W:
        {
            out->cAudioType = ISIL_AUDIO_PCM_E;
        }
        break;
	    case ISIL_AUDIO_ALAW_W:
        {
            out->cAudioType = ISIL_AUDIO_ALAW_E;
        }
        break;
	    case ISIL_AUDIO_ULAW_W:
        {
            out->cAudioType = ISIL_AUDIO_ULAW_E;
        }
        break;
	    case ISIL_AUDIO_ADPCM_W:
        {
            out->cAudioType = ISIL_AUDIO_ADPCM_E;
        }
        break;
        default:
        break;
    }
    switch(in->cBitMode)
    {
	    case ISIL_STREAM_BITRATE_MODE_FLUX_W:
        {
            out->cBitMode = ISIL_STREAM_BITRATE_RC_IMAGE_SMOOTH_FIRST;
        }
        break;

        case ISIL_STREAM_BITRATE_MODE_QUALITY_W:
        {
            out->cBitMode = ISIL_STREAM_BITRATE_RC_IMAGE_QUALITY_FIRST;
        }
        break;

        case ISIL_STREAM_BITRATE_MODE_DEFAULT_W:
        {}
        break;
        default:
            break;
    }

    switch(in->cImageSize)
    {
        case CIF_VIDEO_IMG:
        {
            if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                out->nWidthMBSize = WIDTH_FRAME_CIF_PAL >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_CIF_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                out->nWidthMBSize = WIDTH_FRAME_CIF_NTSC >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_CIF_NTSC >>4;
            }

            out->cImageSize = ISIL_VIDEO_SIZE_CIF_E;
        }
        break;

        case D1_VIDEO_IMG:
        {
            if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                out->nWidthMBSize = WIDTH_FRAME_D1_PAL >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_D1_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                out->nWidthMBSize = WIDTH_FRAME_D1_NTSC >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_D1_NTSC >>4;
            }
            out->cImageSize = ISIL_VIDEO_SIZE_D1_E;
        }
        break;
        case FOUR_CIF_VIDEO_IMG:
        {
            if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                out->nWidthMBSize = WIDTH_FRAME_4CIF_PAL >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_4CIF_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                out->nWidthMBSize = WIDTH_FRAME_4CIF_NTSC >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_4CIF_NTSC >>4;
            }
            out->cImageSize = ISIL_VIDEO_SIZE_4CIF_E;
        }
        break;
        case QCIF_VIDEO_IMG:
        {
            if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                out->nWidthMBSize = WIDTH_FRAME_QCIF_PAL >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_QCIF_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                out->nWidthMBSize = WIDTH_FRAME_QCIF_NTSC >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_QCIF_NTSC >>4;
            }
            out->cImageSize = ISIL_VIDEO_SIZE_QCIF_E;
        }
        break;
        case HALF_D1_VIDEO_IMG:
        case TWO_CIF_VIDEO_IMG:
        {
            if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                out->nWidthMBSize = WIDTH_FRAME_HALF_D1_PAL >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_HALF_D1_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                out->nWidthMBSize = WIDTH_FRAME_HALF_D1_NTSC >> 4;
                out->nHeightMBSize = HEIGHT_FRAME_HALF_D1_NTSC >>4;
            }
            out->cImageSize = ISIL_VIDEO_SIZE_HALF_D1_E;
        }
        break;
        default:
        break;
    }
    switch(in->cRCType)
    {
        case ISIL_STREAM_BITRATE_TYPE_CBR_W:
        {
            out->cRCType = ISIL_STREAM_BITRATE_H264_CBR;
        }
        break;
	    case ISIL_STREAM_BITRATE_TYPE_VBR_W:
        {
            out->cRCType = ISIL_STREAM_BITRATE_H264_VBR;
        }
        break;
        case ISIL_STREAM_BITRATE_TYPE_CQP_W:
        {
            out->cRCType = ISIL_STREAM_BITRATE_H264_NO_RC;
        }
        break;
        default:
            break;
    }

}

static void ISIL_SetEncodeActive(S32 iConnectfd, S32 channel, ISIL_ENCODE_CFG_S* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set encode");


    S32 ret;
    CHIP_CHN_PHY_ID id;
    ISIL_BITRATE_VALUE bitrate;
    ISIL_ENCODE_INFO_S out;

    #ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &id);
    #else
    ISIL_GetChipChnIDByLogicID(channel,&id);
    #endif

    /*设置主码流*/
    printf("encode main parm: audio type %d,bitmod %d, chnpri %d,fps %d,imgQ %d,imgsize %d,RCtype %d,strm type %d,bitrate %d, bp-i %d,i-i %d\n",
        pCfg->stStreamEncode.cAudioType,
        pCfg->stStreamEncode.cBitMode,
        pCfg->stStreamEncode.cChannelPri,
        pCfg->stStreamEncode.cFPS,
        pCfg->stStreamEncode.cImageQuality,
        pCfg->stStreamEncode.cImageSize,
        pCfg->stStreamEncode.cRCType,
        pCfg->stStreamEncode.cStreamType,
        pCfg->stStreamEncode.nBitRate,
        pCfg->stStreamEncode.nBPFrameInterval,
        pCfg->stStreamEncode.nIFrameIntervals
    );

    /*将windows的数值转换为嵌入式内部的数值*/
    memset(&out, 0x00, sizeof(ISIL_ENCODE_INFO_S));
    ISIL_TransValue_W2E(id.chipid, &pCfg->stStreamEncode, &out);

    bitrate.tQP.cIQP = out.cIQP;
    bitrate.tQP.cBQP = out.cBQP;
    bitrate.tQP.cPQP = out.cPQP;

    ISIL_MediaSDK_SetAudioType(id.chipid, id.chanid, out.cAudioType);

    ISIL_MediaSDK_SetVideoFlip(id.chipid, id.chanid, 0, 0);

    ISIL_MediaSDK_SetBitrateControlType(id.chipid, id.chanid,
        out.cRCType, bitrate, IS_H264_MAIN_STREAM);

    ISIL_MediaSDK_SetupBitrateControl(id.chipid, id.chanid,
        out.nBitRate*1000, IS_H264_MAIN_STREAM);

    ISIL_MediaSDK_SetBitrateControlMode(id.chipid, id.chanid,
        out.cBitMode, IS_H264_MAIN_STREAM);

    ISIL_MediaSDK_SetIBPMode(id.chipid, id.chanid,
        out.nIFrameIntervals, 0, 0,
        out.cFPS, IS_H264_MAIN_STREAM);

    ISIL_MediaSDK_SetEncoderVideoSize(id.chipid, id.chanid,
        out.cImageSize, IS_H264_MAIN_STREAM);

    /*设置子码流*/
    printf("encode sub parm: audio type %d,bitmod %d, chnpri %d,fps %d,imgQ %d,imgsize %d,RCtype %d,strm type %d,bitrate %d, bp-i %d,i-i %d\n",
        pCfg->stStreamEncode2.cAudioType,
        pCfg->stStreamEncode2.cBitMode,
        pCfg->stStreamEncode2.cChannelPri,
        pCfg->stStreamEncode2.cFPS,
        pCfg->stStreamEncode2.cImageQuality,
        pCfg->stStreamEncode2.cImageSize,
        pCfg->stStreamEncode2.cRCType,
        pCfg->stStreamEncode2.cStreamType,
        pCfg->stStreamEncode2.nBitRate,
        pCfg->stStreamEncode2.nBPFrameInterval,
        pCfg->stStreamEncode2.nIFrameIntervals
    );

    /*将windows的数值转换为嵌入式内部的数值*/
    memset(&out, 0x00, sizeof(ISIL_ENCODE_INFO_S));
    ISIL_TransValue_W2E(id.chipid, &pCfg->stStreamEncode2, &out);

    //ISIL_MediaSDK_SetAudioType(id.chipid, id.chanid, out.cAudioType);

    ISIL_MediaSDK_SetVideoFlip(id.chipid, id.chanid, 0, 0);

    ISIL_MediaSDK_SetBitrateControlType(id.chipid, id.chanid,
        out.cRCType, bitrate, IS_H264_SUB_STREAM);

    ISIL_MediaSDK_SetBitrateControlMode(id.chipid, id.chanid,
        out.cBitMode, IS_H264_SUB_STREAM);

    ISIL_MediaSDK_SetupBitrateControl(id.chipid, id.chanid,
        out.nBitRate*1000, IS_H264_SUB_STREAM);

    ISIL_MediaSDK_SetIBPMode(id.chipid, id.chanid,
        out.nIFrameIntervals, 0, 0,
        out.cFPS, IS_H264_SUB_STREAM);

    ISIL_MediaSDK_SetEncoderVideoSize(id.chipid, id.chanid,
        out.cImageSize, IS_H264_SUB_STREAM);

    return ;
}

static S32 ISIL_SetAlarmInActive(S32 iConnectfd, S32 channel, ISIL_ALARMIN_CFG_S* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set alarm in");

    S32 ret;
    G_ALARMIN_CFG_S gAlarmIn;

    ret = ISIL_SaveAlarmCfg(channel, ALARM_IN, ISIL_ALARM_TYPE_VIDEO_MAX,
            ISIL_LINKAGE_TYPE_MAX,sizeof(ISIL_ALARMIN_CFG_S),
            pCfg);
    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    gAlarmIn.channel = channel;
    memcpy(&gAlarmIn.sAlarmInCfg, pCfg, sizeof(ISIL_ALARMIN_CFG_S));
    ret = ISIL_SetAlarmActive(channel, ISIL_ALARM_TYPE_ALARMIN, &gAlarmIn);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    return ISIL_SUCCESS;
}

static S32 ISIL_SetOutCtrlActive(S32 iConnectfd, S32 channel, ISIL_OUTCTRL_CTRL* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set alarm in");
    S32 ret;
    G_OUTCTRL_CFG_S  gAlarmOutctrl;
    ret = ISIL_SaveAlarmCfg(channel, ALARM_OUT, ISIL_ALARM_TYPE_VIDEO_MAX,
            ISIL_LINKAGE_TYPE_MAX,sizeof(ISIL_OUTCTRL_CTRL),pCfg);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    gAlarmOutctrl.channel = channel;
    memcpy(&gAlarmOutctrl.sOutCtlCfg, pCfg, sizeof(ISIL_OUTCTRL_CTRL));
    ret = ISIL_SetAlarmActive(channel, ISIL_ALARM_TYPE_OUTCTRL, &gAlarmOutctrl);

    if(ISIL_FAIL == ret)
        return ISIL_FAIL;

    return ISIL_SUCCESS;
}

static S32 ISIL_SetH264Feature(S32 iConnectfd, S32 channel, VENC_H264_FEATURE* pCfg)
{
    ISIL_TRACE(BUG_NORMAL, "set h264 feature");
  
    S32 ret;
    VENC_H264_FEATURE* pTmp;
    CHIP_CHN_PHY_ID phy_id_ptr;

    pTmp = pCfg;
    #ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &phy_id_ptr);
    #else
    ISIL_GetChipChnIDByLogicID(channel,&phy_id_ptr);
    #endif
    /*设置主码流*/
    //ret = isil_set_venc_h264_feature_parm(&venc_h264_feature);
    ret = ISIL_MediaSDK_SetH264Feature(phy_id_ptr.chipid, phy_id_ptr.chanid,
        pTmp, IS_H264_MAIN_STREAM);
    if(ISIL_FAIL == ret)
    {
        printf("set main stream feature fail\n");
        return ISIL_FAIL;
    }

    /*设置子码流*/
    pTmp++;
    //ret = isil_set_venc_h264_feature_parm(&venc_h264_feature);
    ret = ISIL_MediaSDK_SetH264Feature(phy_id_ptr.chipid, phy_id_ptr.chanid,
        pTmp, IS_H264_SUB_STREAM);
    if(ISIL_FAIL == ret)
    {
        printf("set sub stream feature fail\n");
        return ISIL_FAIL;
    }
    ret = ISIL_SaveH264Cfg(channel, pCfg, sizeof(VENC_H264_FEATURE)*2);
    if(ISIL_FAIL == ret)
    {
        printf("save stream feature fail\n");
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}


/*Build ACK struct, Send ack data*/

/********************************************************
 *Function: ISIL_SendAckData
 *Description: Build ACK struct, Send ack data
 *Input:
 *	S32 fd : connected fd
 *	void data: ack data pointer
 *	S32 iMsgNum: message number
 *  void* net_addr:结构为sockaddr_in的网络地址
 *output:
 *       No
 *return:
 *        not -1: Success; -1: Fail
 *******************************************************/
S32 ISIL_SendAckData(S32 sockfd, ISIL_ACK_DATA_S* ackData, S32 msg_num,
        void* net_addr, U32 uiChanneNo, S32 where)
{
	int ret;
    struct msghdr msg;
	ISIL_NET_IE_ACK_S sIEHeader;
	ISIL_MSG_HERADE_S sMSGHeader;
    S08 *pUsr = NULL;
	struct iovec iov[3];

	if(NULL == ackData)
	{
		ISIL_TRACE(BUG_ERROR, "Send ack data is null\n");
		return ISIL_FAIL;
	}

    if(NULL == net_addr)
	{
	    msg.msg_name = NULL;
        msg.msg_namelen = 0;
	}
	else
	{
        msg.msg_name = (void*)(net_addr);
        msg.msg_namelen = sizeof(struct sockaddr_in);
	}
    msg.msg_control = NULL ;
    msg.msg_controllen = 0 ;
    msg.msg_flags = 0 ;

	memset(&sMSGHeader, 0x00,sizeof(ISIL_MSG_HERADE_S));
	memset(&sIEHeader, 0x00,sizeof(ISIL_NET_IE_ACK_S));

    if(ISIL_LOCAL_MSG == where)
    {
#ifdef LOCAL_GUI_ON_BOARD
        sMSGHeader.nMsgTag = (MSG_TAG);
        sMSGHeader.nMsgNum = (msg_num);
#else
        sMSGHeader.nMsgTag = _swab16(MSG_TAG);
        sMSGHeader.nMsgNum = _swab16(msg_num);
#endif
    }
    else
    {
        sMSGHeader.nMsgTag = _swab16(MSG_TAG);
        sMSGHeader.nMsgNum = _swab16(msg_num);
    }

	switch(ackData->nIEType)
	{
		case IE_TYPE_ACK_LOGIN:
        {
            if(ISIL_LOCAL_MSG == where)
            {
#ifdef LOCAL_GUI_ON_BOARD
                sMSGHeader.nMsgType = (MSG_TYPE_ACK_LOGIN);
#else
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_LOGIN);
#endif
            }
            else
            {
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_LOGIN);
            }
        }

		break;
		case IE_TYPE_ACK_REALPLAY:
        {
            if(ISIL_LOCAL_MSG == where)
            {
#ifdef LOCAL_GUI_ON_BOARD
                sMSGHeader.nMsgType = (MSG_TYPE_ACK_REALPLAY);
#else
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_REALPLAY);
#endif
            }
            else
            {
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_REALPLAY);
            }
        }
		break;
		case IE_TYPE_ACK_TALK:
        {
            if(ISIL_LOCAL_MSG == where)
            {
#ifdef LOCAL_GUI_ON_BOARD
                sMSGHeader.nMsgType = (MSG_TYPE_ACK_TALK);
#else
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_TALK);
#endif
            }
            else
            {
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_TALK);
            }
        }
		break;
		case IE_TYPE_ACK_BROADCAST:
        {
            if(ISIL_LOCAL_MSG == where)
            {
#ifdef LOCAL_GUI_ON_BOARD
                sMSGHeader.nMsgType = (MSG_TYPE_ACK_BROADCAST);
#else
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_BROADCAST);
#endif
            }
            else
            {
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_BROADCAST);
            }
        }
		break;
		case IE_TYPE_ACK_AUTOSEARCH:
        {
            if(ISIL_LOCAL_MSG == where)
            {
#ifdef LOCAL_GUI_ON_BOARD
                sMSGHeader.nMsgType = (MSG_TYPE_ACK_AUTOSEARCH);
#else
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_AUTOSEARCH);
#endif
            }
            else
            {
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_ACK_AUTOSEARCH);
            }
		}

		break;
		case IE_TYPE_FILE_OPEN+1:
        {
            if(ISIL_LOCAL_MSG == where)
            {
#ifdef LOCAL_GUI_ON_BOARD
                sMSGHeader.nMsgType = (MSG_TYPE_FILE+1);
#else
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_FILE+1);
#endif
            }
            else
            {
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_FILE+1);
            }
		}

		break;

		default:
        {
            if(ISIL_LOCAL_MSG == where)
            {
#ifdef LOCAL_GUI_ON_BOARD
                sMSGHeader.nMsgType = (MSG_TYPE_DEVTOPC_RET);
#else
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_DEVTOPC_RET);
#endif
            }
            else
            {
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_DEVTOPC_RET);
            }
		}
		break;
	}
    if(ISIL_LOCAL_MSG == where)
    {
#ifdef LOCAL_GUI_ON_BOARD
        sIEHeader.nAckType = (ackData->nIEType);
        sIEHeader.nAckLength = (ackData->iLen);
        sIEHeader.iAckFlag = (ackData->iAckFlag);
        sIEHeader.lReserve = (uiChanneNo);
#else
        sIEHeader.nAckType = _swab16(ackData->nIEType);
        sIEHeader.nAckLength = _swab16(ackData->iLen);
        sIEHeader.iAckFlag = _swab32(ackData->iAckFlag);
        sIEHeader.lReserve = _swab32(uiChanneNo);
#endif
    }
    else
    {
        sIEHeader.nAckType = _swab16(ackData->nIEType);
    	sIEHeader.nAckLength = _swab16(ackData->iLen);
    	sIEHeader.iAckFlag = _swab32(ackData->iAckFlag);
    	sIEHeader.lReserve = _swab32(uiChanneNo);
    }


	ISIL_TRACE(BUG_NORMAL, "[%s] fd %d,ie type 0x%x, len %d, flag %d",\
        __FUNCTION__, sockfd, ackData->nIEType,\
		ackData->iLen, ackData->iAckFlag);
    if(ISIL_LOCAL_MSG == where)
    {
        #ifdef LOCAL_GUI_ON_BOARD
            sMSGHeader.lMsgLen = (sizeof(ISIL_NET_IE_ACK_S) + ackData->iLen);
        #else
            sMSGHeader.lMsgLen = _swab32(sizeof(ISIL_NET_IE_ACK_S) + ackData->iLen);
        #endif
    }
    else
        sMSGHeader.lMsgLen = _swab32(sizeof(ISIL_NET_IE_ACK_S) + ackData->iLen);

	sMSGHeader.lMsgCrc = 0;
    if(ISIL_LOCAL_MSG == where)
    {
#ifdef LOCAL_GUI_ON_BOARD
        sMSGHeader.lMsgCrc = (crc32(0, (U08*)&sMSGHeader, sizeof(ISIL_MSG_HERADE_S) - sizeof(sMSGHeader.lMsgCrc)));
#else
        sMSGHeader.lMsgCrc = _swab32(crc32(0, (U08*)&sMSGHeader, sizeof(ISIL_MSG_HERADE_S) - sizeof(sMSGHeader.lMsgCrc)));
#endif
    }
    else
	    sMSGHeader.lMsgCrc = _swab32(crc32(0, (U08*)&sMSGHeader, sizeof(ISIL_MSG_HERADE_S) - sizeof(sMSGHeader.lMsgCrc)));

	iov[0].iov_base = &sMSGHeader;
	iov[0].iov_len = sizeof(ISIL_MSG_HERADE_S);
	iov[1].iov_base = &sIEHeader;
	iov[1].iov_len = sizeof(ISIL_NET_IE_ACK_S);

	iov[2].iov_base = ackData->data;
	iov[2].iov_len = ackData->iLen;//sIEHeader.nAckLength;
	msg.msg_iov = iov;
    msg.msg_iovlen = 3;
    ret = sendmsg(sockfd, &msg, 0);
	//ret = writev(sockfd, iov, 3);
	if((ret < 0) && (errno != EINTR)){
		fprintf(stderr,"Send failed.\n");
		return ISIL_FAIL;
	}
	return ret;
}

/*设备发给客户端的请求*/
static S32 ISIL_SendAskData(S32 sockfd, ISIL_ACK_DATA_S* ackData, S32 msg_num, S32 where)
{
	int ret;
	ISIL_NET_IE_ASK_S sIEHeader;
	ISIL_MSG_HERADE_S sMSGHeader;
	struct iovec iov[3];

	if(NULL == ackData)
	{
		ISIL_TRACE(BUG_ERROR, "Send ack data is null\n");
		return ISIL_FAIL;
	}

	memset(&sMSGHeader, 0x00,sizeof(ISIL_MSG_HERADE_S));
	memset(&sIEHeader, 0x00,sizeof(ISIL_NET_IE_ASK_S));
    if(ISIL_LOCAL_MSG == where)
    {

#ifdef LOCAL_GUI_ON_BOARD
            sMSGHeader.nMsgTag = (MSG_TAG);
            sMSGHeader.nMsgNum = (msg_num);
#else
            sMSGHeader.nMsgTag = _swab16(MSG_TAG);
            sMSGHeader.nMsgNum = _swab16(msg_num);
#endif

    }
    else
    {
        sMSGHeader.nMsgTag = _swab16(MSG_TAG);
        sMSGHeader.nMsgNum = _swab16(msg_num);
    }

	switch(ackData->nIEType)
	{

		case IE_TYPE_FILE_SEARCH:
        case IE_TYPE_DEVTOC_ALARMSTATUS:
        case IE_TYPE_DEVTOC_PARAMCHANGED:
        if(ISIL_LOCAL_MSG == where)
        {

#ifdef LOCAL_GUI_ON_BOARD
                sMSGHeader.nMsgType = (MSG_TYPE_DEVTOPC_REQ);
#else
                sMSGHeader.nMsgType = _swab16(MSG_TYPE_DEVTOPC_REQ);
#endif
        }
        else
        {
            sMSGHeader.nMsgType = _swab16(MSG_TYPE_DEVTOPC_REQ);
        }
		break;
		default:
			sMSGHeader.nMsgType = -1;
		break;
	}
    sMSGHeader.lMsgCrc = 0;
    if(ISIL_LOCAL_MSG == where)
    {
        #ifdef LOCAL_GUI_ON_BOARD
        sIEHeader.nAskType = (ackData->nIEType);
    	sIEHeader.nAskLength = (ackData->iLen);

        sMSGHeader.lMsgLen = (sizeof(ISIL_NET_IE_ASK_S) + ackData->iLen);
        sMSGHeader.lMsgCrc = (crc32(0, (U08*)&sMSGHeader, sizeof(ISIL_MSG_HERADE_S) -
            sizeof(sMSGHeader.lMsgCrc)));
        #else
    	sIEHeader.nAskType = _swab16(ackData->nIEType);
    	sIEHeader.nAskLength = _swab16(ackData->iLen);

        sMSGHeader.lMsgLen = _swab32(sizeof(ISIL_NET_IE_ASK_S) + ackData->iLen);
        sMSGHeader.lMsgCrc = _swab32(crc32(0, (U08*)&sMSGHeader, sizeof(ISIL_MSG_HERADE_S) -
            sizeof(sMSGHeader.lMsgCrc)));
        #endif
    }
    else
    {
    	sIEHeader.nAskType = _swab16(ackData->nIEType);
    	sIEHeader.nAskLength = _swab16(ackData->iLen);

        sMSGHeader.lMsgLen = _swab32(sizeof(ISIL_NET_IE_ASK_S) + ackData->iLen);
        sMSGHeader.lMsgCrc = _swab32(crc32(0, (U08*)&sMSGHeader, sizeof(ISIL_MSG_HERADE_S) -
            sizeof(sMSGHeader.lMsgCrc)));
    }
	ISIL_TRACE(BUG_NORMAL, "[%s] fd %d,ie type 0x%x, len %d",\
        __FUNCTION__, sockfd, sIEHeader.nAskType,\
		sIEHeader.nAskLength);

	iov[0].iov_base = &sMSGHeader;
	iov[0].iov_len = sizeof(ISIL_MSG_HERADE_S);
	iov[1].iov_base = &sIEHeader;
	iov[1].iov_len = sizeof(ISIL_NET_IE_ASK_S);

	iov[2].iov_base = ackData->data;
	iov[2].iov_len = ackData->iLen;//sIEHeader.nAskLength;
	ret = writev(sockfd, iov, 3);
	if((ret < 0) && (errno != EINTR)){
		fprintf(stderr,"Send failed.\n");
		return ISIL_FAIL;
	}

	fprintf(stderr,"ISIL_SendAskData ---> have send msg %d.\n", sIEHeader.nAskType);
	return ret;
}



/********************************************************
 *Function: ISIL_SendAckData
 *Description: 构建回给主机的数据结构值
 *Input:
 *	ISIL_ACK_DATA_S *ackdata : 需要赋值的数据结构指针
 *	void *data: ack data pointer
 *	S32 len: message length
 *	S32 type:返回主机的消息类型
 *	S32 ackfalg:错误码
 *output:
 *       No
 *return:
 *        not -1: Success; -1: Fail
 *******************************************************/
void ISIL_BuildAckData(ISIL_ACK_DATA_S *ackdata, void * data, S32 len,
									S32 type, S32 ackfalg)
{
	if(NULL == ackdata)
	{
		ISIL_TRACE(BUG_ERROR, "Ack data pointer is null\n");
		return;
	}
	ackdata->data = data;
	ackdata->nIEType = type;
	ackdata->iLen = len;
	ackdata->iAckFlag = ackfalg;
	return;
}

static S32 ISIL_SendAlarmToPCByFd(void* arg1, void* arg2)
{
    return 0;
}

S32 ISIL_SendAlarmToPC(S32 iChannel, S32 iAlarmType, void* service)
{
    return 0;
}

/*处理搜索事件*/

#define ISIL_SEARCH_EVNT_BUFF_SIZE (4*1024)

S32 ISIL_HandleSearchEvnt(S32 iFd, ISIL_SEARCH_EVNET* pEvnt, S32 iMsgNum, S32 MsgWhere)
{
    #if 1
    S32 iMsgLen;
    S32 iInfotype;
    S32 iRet;
    S32 iChannelCnt = 0;
    S32 iServiceID;
    ISIL_ACK_DATA_S ackdata;
    MSG_FILE_DEC_REG_T *filedec;
    char buff[ISIL_SEARCH_EVNT_BUFF_SIZE];

    if(NULL == pEvnt)
    {
        return ISIL_FAIL;
    }

    ISIL_TRACE(BUG_NORMAL, "[%s] command %d", __FUNCTION__, pEvnt->cCommand);
    /*
     * if(ISIL_PC_MSG == MsgWhere)
     * 	iServiceID = PLAY_MOD;
     * else if(ISIL_LOCAL_MSG == MsgWhere)
     * 	iServiceID = LOC_PB_MOD;
     */


    switch(pEvnt->cCommand)
    {
        case ISIL_SEARCH_STOP:
        {
        }
        break;

        case ISIL_SEARCH_FILE:
        {
            MSG_FM_SEARCH stFMSearch;
            memset(&stFMSearch, 0x0, sizeof(MSG_FM_SEARCH));
            iMsgLen = sizeof(MSG_FM_SEARCH);
            stFMSearch.ucChannel = pEvnt->cChannelNum;
            /*类型暂且这样操作，TODO*/
            stFMSearch.ucType = pEvnt->cEventType;
            //stFMSearch.ucType = pEvnt->cType;
            #ifndef LOCAL_GUI_ON_BOARD
            ISIL_TransTime(&pEvnt->stStart);
            ISIL_TransTime(&pEvnt->stEnd);
            #endif
            memcpy(&stFMSearch.stStart, &pEvnt->stStart, sizeof(ISIL_TIME));
            memcpy(&stFMSearch.stEnd, &pEvnt->stEnd, sizeof(ISIL_TIME));

            /*
             * iInfotype = MSG_TYPE_FM_SEARCH;
             * iRet = ISIL_SendMsgToOther(iServiceID, iInfotype, iMsgLen,
             * iFd, ISIL_PC_MSG,&stFMSearch);
             */
            filedec = get_glb_file_dec_reg_ptr();
            if(filedec){

            	MSG_FM_SEARCH_RESULT *pSrhRst = NULL;

            	iRet = filedec->fm_remote_search((void*)&stFMSearch, (void*)buff);
				pSrhRst = (MSG_FM_SEARCH_RESULT*)buff;
				pSrhRst->ResultEntry->lFlag = _swab32(pSrhRst->ResultEntry->lFlag);
				ISIL_BuildAckData(&ackdata, pSrhRst->ResultEntry,
						iRet - sizeof(MSG_FM_SEARCH_RESULT),
					IE_TYPE_FILE_SEARCH, 0);
				ISIL_SendAskData(iFd, &ackdata, 0, MsgWhere);
            }
        }
        break;

        case ISIL_SEARCH_DEV:
        {}
        break;
        case ISIL_SEARCH_CHANNEL:
        {
            S32 iLoop;
            ISIL_LOG_DEVICE channelInfo;
            iChannelCnt = ISIL_GetChCnt();
            channelInfo.lFlag = 0x0FFEABCD;
            for(iLoop = 0; iLoop < iChannelCnt; iLoop++)
            {
                memset(&channelInfo, 0, sizeof(ISIL_LOG_DEVICE));
                memset(&ackdata, 0, sizeof(ISIL_ACK_DATA_S));
                channelInfo.cChannelNum = iLoop;
                ISIL_GetChannelInfo(iLoop, channelInfo.szChannelName);
                ISIL_TRACE(BUG_NORMAL, "[%s] channel num %d, name %s, msgnum %d",
                    __FUNCTION__, channelInfo.cChannelNum,
                    channelInfo.szChannelName, iMsgNum);

                ISIL_BuildAckData(&ackdata, &channelInfo, sizeof(ISIL_LOG_DEVICE),
						        IE_TYPE_FILE_SEARCH, 0);
                ISIL_SendAskData(iFd, &ackdata, iMsgNum, MsgWhere);

            }
            return ISIL_SUCCESS;
        }
        break;
    }
    return iRet;
    #else
    //MSG_FILE_DEC_REG_T *get_glb_file_dec_reg_ptr( void )
    return 0;
    #endif
}


void ISIL_GetPlaybackFileInfo(S32 iConnectFd, S32 MsgSrc, ISIL_NETFILE_OPEN * pFileOpen, void * arg)
{
    S32 iRet;
    S32 fd;
    ISIL_ACK_DATA_S ackdata;
    MSG_FILE_DEC_REG_T *filedec;

    /*根据ISIL_NETFILE_OPEN,发送消息给FM*/

    if(pFileOpen)
    {
        MSG_FM_REMOTE_OPEN stFMRmotOpen;
        MSG_FM_REMOTE_FILEINFO stOpenInfo;
        //ISIL_TransFileOpen(pFileOpen);
        memset(&stFMRmotOpen, 0x0, sizeof(MSG_FM_REMOTE_OPEN));
        stFMRmotOpen.lFileCount = pFileOpen->nFileCount;
		stFMRmotOpen.reserve = 0;
		stFMRmotOpen.callback = NULL;
		memcpy(stFMRmotOpen.cFileNames, pFileOpen->szFileNames, sizeof(stFMRmotOpen.cFileNames));

        filedec = get_glb_file_dec_reg_ptr();
        iRet = filedec->fm_remote_open_file(&stFMRmotOpen, &stOpenInfo);
        fd = stOpenInfo.ulFileFd;
        if(MsgSrc == ISIL_PC_MSG)
			ISIL_BuildAckData(&ackdata, &stOpenInfo,
				sizeof(ISIL_FILE_BASE_INFO), IE_TYPE_FILE_OPEN + 1, iRet);
		else
		{

#ifdef LOCAL_GUI_ON_BOARD
			stOpenInfo.ulFileFd = (stOpenInfo.ulFileFd);
			//ISIL_TransFileBaseInfo((ISIL_FILE_BASE_INFO *) pMSG->payload);
#else
			stOpenInfo.ulFileFd = _swab32(stOpenInfo.ulFileFd);
			ISIL_TransFileBaseInfo((ISIL_FILE_BASE_INFO *) &stOpenInfo);

#endif
			ISIL_BuildAckData(&ackdata, &stOpenInfo,
				sizeof(MSG_FM_REMOTE_FILEINFO), ISIL_OPEN_FILE, iRet);
		}

		printf("%s filefd %ld\n",__FUNCTION__, stOpenInfo.ulFileFd);
		ISIL_SendAckData(iConnectFd, &ackdata, 0, NULL, fd, MsgSrc);
    }
    return;
}

S32 ISIL_SetPlaybackAction(S32 iConnectFd, S32 iActionType, S32 MsgSrc,
                                    NET_FILE_COMMAND* pActionParm, U32 filefd)
{
	S32 iRet;
    MSG_FILE_DEC_REG_T *filedec;
    MSG_FM_REMOTE_OPERATOR fileopt;

    printf("%s filefd = %d, iActionType: 0x%x\n",__FUNCTION__, filefd, iActionType);
    filedec = get_glb_file_dec_reg_ptr();

    memcpy(&fileopt, pActionParm, sizeof(NET_FILE_COMMAND));
    fileopt.uiPlayFd = filefd;

    if(pActionParm && filedec)
    {
        switch(iActionType)
        {
            case IE_TYPE_FILE_GET_FILEINFO:
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_GET_FILEINFO;
            break;
            case IE_TYPE_FILE_GET_POSITION:
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_GET_CURTIME;
            break;
            case IE_TYPE_FILE_SET_POSITION:
                printf("[%s] lTimePosition: %ld\n",__FUNCTION__, pActionParm->lTimePosition);
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_SET_CURTIME;
            break;
            case IE_TYPE_FILE_SET_PERCENT:
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_SEEK;
            break;
            case IE_TYPE_FILE_SET_LOOP:
            {
                printf("[%s] loop mode: %d\n",__FUNCTION__, pActionParm->cIsLoop);
                  //iFMMsgtype = MSG_TYPE_FM_REMOTE_SET_LOOP;
            }
            break;
            case IE_TYPE_FILE_SET_READ_ORDER:
            {
                printf("[%s] order mode: %d\n", __FUNCTION__, pActionParm->cIsOrder);
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_SET_DIRECTION;
            }
            break;
            case IE_TYPE_FILE_SET_READ_KEYFRAME:
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_SET_KEYFRAME;
            break;
            case IE_TYPE_FILE_GET_FRAMES:
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_GET_FRAME;
            	iRet = filedec->fm_remote_get_frame(filefd, (void*)&fileopt);
            break;

            case IE_TYPE_FILE_CLOSE:
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_CLOSE;
            	iRet = filedec->fm_remote_close(filefd, (void*)&fileopt);
            break;
            case ISIL_STOP_FILE_PLAY:
                //iFMMsgtype = MSG_TYPE_FM_REMOTE_STOP;
                //fprintf(stderr, "-----net stop:%d,%x---\n", iFMMsgtype,iFMMsgtype);
            	iRet = filedec->fm_remote_stop(filefd, (void*)&fileopt);
            break;
            default:
            break;

        }
    }
    return iRet;

}


static U32 ISIL_GetLinkageCfgSize(U32 lLinkageType, U32 lAlarmType, U32 channel)
{
    U32 lCnt = 0;
	U32 LinkageSize = 0;
    switch(lLinkageType)
    {
        case ISIL_LINKAGE_TYPE_PC_EVENT:
        {
            LinkageSize = ISIL_GetPcEventSize(channel, lAlarmType);
        }
        break;

        case ISIL_LINKAGE_TYPE_OUTCTRL:
        {

            lCnt = ISIL_GetOutCtlCnt();
            LinkageSize = lCnt*sizeof(ISIL_LINKAGE_OUTCTRL_CFG);
        }
        break;

        case ISIL_LINKAGE_TYPE_PIC_SHOOT:
        {
            lCnt = ISIL_GetChCnt();
            LinkageSize = lCnt*sizeof(ISIL_LINKAGE_SHOOT_CFG_S);
        }
        break;

        case ISIL_LINKAGE_TYPE_PTZ:
        {
            lCnt = ISIL_GetChCnt();
            LinkageSize = lCnt*sizeof(ISIL_LINKAGE_SHOOT_CFG_S);
        }
        break;

        case ISIL_LINKAGE_TYPE_STREAM_RECORD:
        {
            lCnt = ISIL_GetChCnt();
            LinkageSize = lCnt*sizeof(ISIL_LINKAGE_RECORD_CFG_S);
        }
        break;

        default:
             LinkageSize = 0;
        break;
    }
    return LinkageSize;

}


static S32 ISIL_SetPlayNetInfo(S32 channel, S32 playtype,
                                ISIL_USR_CFG_S* sUsrInf,
                                ISIL_ASK_NETINFO_S* psNetLinkType,
                                ISIL_NET_INFO_DATA_S *sNetInfoData)
{
    S32 port;
    if(NULL == sUsrInf || NULL == psNetLinkType || NULL == sNetInfoData)
        return ISIL_FAIL;

    ISIL_TRACE(BUG_NORMAL, "link type %d", psNetLinkType->nLinkProtocol);

    sNetInfoData->nChannel = channel;
    sNetInfoData->lIPAddr = ISIL_GetLocalNetIP();

    memcpy(sNetInfoData->szUserName, sUsrInf->sName, ISIL_NAME_LEN);
    memcpy(sNetInfoData->szPassword, sUsrInf->sPassword, ISIL_PASSWORD_LEN);

    if(playtype == IE_TYPE_GET_REALPLAYINFO)
    {
        if(ISIL_LINKTYEP_TCP == psNetLinkType->nLinkProtocol)
            sNetInfoData->nPort = REALPLAYLISTENPORT;
     
        else if(ISIL_LINKTYEP_UDP == psNetLinkType->nLinkProtocol)
        {
            net_msg_reg_func *net_msg_reg_ptr = get_glb_net_msg_reg_t();



#if 0
            if(net_msg_reg_ptr->set_udp_task_cb2) {
                
                int ret;
                ret = net_msg_reg_ptr->set_udp_task_cb2(sNetInfoData->lIPAddr);
                if( ret < 0 ) {
                    fprintf(stderr,"set_udp_task_cb failed.\n");
                    return ISIL_FAIL;
                }

                sNetInfoData->nPort = ret;
                
            }

#endif

            if(net_msg_reg_ptr->set_udp_task_cb) {
                
                int ret;
                ret = net_msg_reg_ptr->set_udp_task_cb();
                if( ret < 0 ) {
                    fprintf(stderr,"set_udp_task_cb failed.\n");
                    return ISIL_FAIL;
                }

                sNetInfoData->nPort = ret;
                
            }
            
        }
        
        #if 0
        else if(ISIL_LINKTYEP_BROADCAST == psNetLinkType->nLinkProtocol)
        {
            SESSION* chan_sess = NULL;
            sNetInfoData.nPort = ISIL_UDP_MULTICAST_PORT;
            sNetInfoData.lIPAddr = inet_addr(ISIL_UDP_MULTICAST_IP);
            udp_init_session(ISIL_UDP_MULTICAST_IP,
                ISIL_UDP_MULTICAST_PORT,NET_DATA_MOD, 1, channel);
        }
        #endif


    }
    else if(playtype == IE_TYPE_GET_PLAYBACKINFO)
    {
        /*用其他端口*/
        sNetInfoData->nPort = REALPLAYLISTENPORT;
    }
    printf("%s ip %x ch %x protocol %x port %x\n",__FUNCTION__,sNetInfoData->lIPAddr,
            sNetInfoData->nChannel,sNetInfoData->nLinkProtocol,
            sNetInfoData->nPort);
    return ISIL_SUCCESS;

}

/********************************************************
 *Function: ISIL_ParseClientReqCfg
 *Description: 解析IE
 *Input:
 *  U32 uiMsgType:消息类型
 *	S08 *data: 从网络上获得的数据
 *	S32 len: message length
 *	S32 iConnectfd:Sock字
 *	S32 iMsgNum:消息序列号
 *output:
 *       No
 *return:
 *        not -1: Success; -1: Fail
 *******************************************************/

S32  ISIL_ClientReqCfg(S08 *data, S32 iConnectfd, void * arg)
{
	ISIL_ACK_DATA_S ackdata;
	ISIL_USR_CFG_S sUsrInf;
	S32 ret = 0;
	U32 channel;
    U32 uiUsrID = 0;
    U32 uiDataSize = 0;
    U16 askLen;
    U16 asktype;

    ISIL_NET_IE_ASK_S *ieheader = NULL;
	S08 *ieheader_payload = NULL;
    S08 *pUsr = NULL;
    S08 *pTemp = NULL;
    ISIL_COM_CFG_S* sComCfg = NULL;


    memset(&sUsrInf, 0, sizeof(ISIL_USR_CFG_S));
    #if 0
    ret = ISIL_GetUsrInfoByID(uiUsrID, &sUsrInf);
	if(ret < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] Get usr info fail", __FUNCTION__);
		return ISIL_FAIL;
	}
    #endif
    ieheader = (ISIL_NET_IE_ASK_S*)data;
	memset(&ackdata, 0x00,sizeof(ISIL_ACK_DATA_S));
	ieheader = (ISIL_NET_IE_ASK_S*)ieheader;
	ieheader_payload = (S08*)(ieheader + 1);
    askLen = _swab16(ieheader->nAskLength);
	channel = _swab32(ieheader->lReserve);
    asktype = _swab16(ieheader->nAskType);
	ISIL_TRACE(BUG_NORMAL, "[%s] Get ie type 0x%x, USRID %d, channel no %d, ie playloadlen %d",
        __FUNCTION__, _swab16(ieheader->nAskType), uiUsrID, channel, askLen);

    switch(asktype)
	{
        case IE_TYPE_GET_USERCFG:
        {
            if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;

            ret = ISIL_UsrLevelMatch(uiUsrID, RIGHT_USERCFG);
            if(ret == ISIL_FALSE)
            {
                return ISIL_FAIL;
            }

            uiDataSize = ISIL_GetAllUsrInfoSize();
            if(uiDataSize > 0)
            {
                pUsr = calloc(sizeof(S08), uiDataSize);
                if(NULL != pUsr)
                {
                    ret = ISIL_GetAllUserInfo(pUsr, uiDataSize, 0);
                    pTemp = (S08*)pUsr;
                }
                else
                {
                    uiDataSize = 0;
                }
            }
        }
        break;

        case IE_TYPE_SET_USERCFG:
        {
            if(askLen == IE_NO_PLAYLOAD)
				return ISIL_FAIL;

            ret = ISIL_SaveAllUserInfo((S08*)ieheader_payload, askLen, 0);
        }
        break;

        case IE_TYPE_GET_USERGROUPCFG:
        {
            if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
            uiDataSize = ISIL_GetAllUsrGrpInfoSize();

            if(uiDataSize > 0)
            {
                pUsr = calloc(sizeof(S08), uiDataSize);
                if(NULL != pUsr)
                {
                    ret = ISIL_GetAllUserInfo(pUsr, uiDataSize, 1);
                    pTemp = (S08*)pUsr;
                }
                else
                {
                    uiDataSize = 0;
                }
            }
        }
        break;

        case IE_TYPE_SET_USERGROUPCFG:
        {
            if(askLen == IE_NO_PLAYLOAD)
				return ISIL_FAIL;

            ret = ISIL_SaveAllUserInfo((S08*)ieheader_payload, askLen, 1);
        }
        break;


		case IE_TYPE_GET_DEVICEBASECFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
			ISIL_DEV_BASE_CFG_S sDevBaseCfg;
            uiDataSize = sizeof(ISIL_DEV_BASE_CFG_S);

			memset(&sDevBaseCfg,0x00, uiDataSize);
			ret = ISIL_GetDevBaseInfo(&sDevBaseCfg);
            ISIL_TransDevBaseInfo(&sDevBaseCfg);
            pTemp = (S08*)&sDevBaseCfg;
            uiDataSize = (ret < 0)?0:uiDataSize;
		}
		break;
		case IE_TYPE_SET_DEVICEBASECFG:
		{
			if(askLen != sizeof(ISIL_DEV_BASE_CFG_S))
				return ISIL_FAIL;
            ISIL_DEV_BASE_CFG_S * cfg = (ISIL_DEV_BASE_CFG_S *)ieheader_payload;
            /*设置制式*/
            ISIL_SetVideoStander(cfg->cStandard);
            ISIL_TransDevBaseInfo(cfg);
			ret = ISIL_SetDevBaseInfo(cfg);
		}
		break;
		case IE_TYPE_GET_DEVICECFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;

			ISIL_DEV_CFG_S sDevCfg;
            uiDataSize = sizeof(ISIL_DEV_CFG_S);
			memset(&sDevCfg, 0, uiDataSize);
			ret = ISIL_GetDevInfo(&sDevCfg);
			pTemp = (S08*)&sDevCfg;
            uiDataSize = (ret < 0)?0:uiDataSize;
		}
		break;

		case IE_TYPE_SET_DEVICECFG:
		{
			if(askLen != sizeof(ISIL_DEV_CFG_S))
				return ISIL_FAIL;
			ret = ISIL_SetDevInfo((ISIL_DEV_CFG_S *)ieheader_payload);
		}
		break;

		case IE_TYPE_GET_NETCFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
			ISIL_NET_CFG_S sNetCfg;
            uiDataSize = sizeof(ISIL_NET_CFG_S);
			memset(&sNetCfg, 0x00, uiDataSize);
			ret = ISIL_GetNetworkConfig(&sNetCfg);
            /*转换结构体成员字节序*/
            ISIL_TransNetworkConfig(&sNetCfg);
            pTemp = (S08*)&sNetCfg;
            uiDataSize = (ret < 0)?0:uiDataSize;
		}
		break;

		case IE_TYPE_SET_NETCFG:
		{
			if(askLen != sizeof(ISIL_NET_CFG_S))
				return ISIL_FAIL;

            ISIL_BuildAckData(&ackdata, NULL, IE_NO_PLAYLOAD,
					IE_TYPE_SET_NETCFG + 1, 0);
            ISIL_SendAckData(iConnectfd, &ackdata, 1, NULL, 0, ISIL_PC_MSG);
            /*转换网络字节序*/
            ISIL_TransNetworkConfig((ISIL_NET_CFG_S*)ieheader_payload);
			ret = ISIL_SetNetworkConfig((ISIL_NET_CFG_S*)ieheader_payload);
            return ISIL_SUCCESS;
		}
		break;

		case IE_TYPE_GET_COMMCFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
            S32 COMCnt;
            COMCnt = ISIL_GetComRS232Cnt() + ISIL_GetComRS485Cnt();
            sComCfg = calloc(sizeof(ISIL_COM_CFG_S), COMCnt);

			//ret = ISIL_GetCOMCfg(sComCfg, COMCnt);
			ret = 2;

			uiDataSize = (ret < 0)?0:(sizeof(ISIL_COM_CFG_S)*COMCnt);
			pTemp = (S08*)sComCfg;

		}
		break;

		case IE_TYPE_SET_COMMCFG:
		{
            S32 COMCnt;
            COMCnt = ISIL_GetComRS232Cnt() + ISIL_GetComRS485Cnt();

			if(askLen != (sizeof(ISIL_COM_CFG_S)) *COMCnt)
				return ISIL_FAIL;

			//ret = ISIL_SetCOMCfg((ISIL_COM_CFG_S*)ieheader_payload, COMCnt);
		}
		break;

		case IE_TYPE_SET_COMM_SENDDATA:
		break;

		case IE_TYPE_SET_COMM_STARTCAPDATA:
		break;

		case IE_TYPE_SET_COMM_STOPCAPDATA:
		break;

		case IE_TYPE_GET_PTZCFG:
        {
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
			ISIL_PTZ_CFG_S sPTZCfg;
            uiDataSize = sizeof(ISIL_PTZ_CFG_S);
			memset(&sPTZCfg, 0x00,sizeof(ISIL_PTZ_CFG_S));

			ret = ISIL_GetPTZInfo(&sPTZCfg, channel);
            pTemp = (S08*)&sPTZCfg;
			uiDataSize = (ret < 0)?0:uiDataSize;
		}
		break;
		case IE_TYPE_SET_PTZCFG:
		{
			if(askLen != sizeof(ISIL_PTZ_CFG_S))
				return ISIL_FAIL;
            ISIL_PTZ_CFG_S *spPtzCfg;
			spPtzCfg = (ISIL_PTZ_CFG_S*)ieheader_payload;
            /*TODO*/
            strcpy(spPtzCfg->sName, "PELCO_D.COD");
			ret = ISIL_SetPTZInfo((ISIL_PTZ_CFG_S*)ieheader_payload, channel);
		}
		break;

		case IE_TYPE_GET_PTZPROTOCOL:
        {
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
			ISIL_PTZ_PROTOCOL_S sPTZProtocol;

            uiDataSize = sizeof(ISIL_PTZ_PROTOCOL_S);
			memset(&sPTZProtocol, 0x00, uiDataSize);
			ret = ISIL_GetPTZProtocol(channel, &sPTZProtocol);

			pTemp = (ret < 0)? NULL:((S08*)&sPTZProtocol);
            uiDataSize  = (ret < 0)?0:uiDataSize;
		}
		break;

		case IE_TYPE_GET_PTZPRESETCFG:
		break;
		case IE_TYPE_SET_PTZPRESETCFG:
		break;

		case IE_TYPE_SET_PTZCTRL:
        break;

		case IE_TYPE_GET_VIDEOSIGNALCFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;

			G_VIDEO_SIGNAL_CFG_S gpsVideoSignal;
            uiDataSize = sizeof(G_VIDEO_SIGNAL_CFG_S);
			ret = ISIL_GetVideoCfg(channel, VIDEO_SIGNAL, uiDataSize, &gpsVideoSignal);

            pTemp = (S08*)(&gpsVideoSignal.sVideoSignalCfg);
            uiDataSize = (ret < 0)?0:(sizeof(ISIL_VIDEO_SIGNAL_CFG_S));
		}
		break;

		case IE_TYPE_SET_VIDEOSIGNALCFG:
		{
			if(askLen != sizeof(ISIL_VIDEO_SIGNAL_CFG_S))
				return ISIL_FAIL;

            ret = ISIL_SetSignActive(iConnectfd, channel,
                    (ISIL_VIDEO_SIGNAL_CFG_S*)ieheader_payload);

		}
		break;

		case IE_TYPE_GET_VIDEOMOTIONCFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;

			G_VIDEO_MOTION_CFG_S gpsVideoMotion;

            uiDataSize = sizeof(G_VIDEO_MOTION_CFG_S);
			ret = ISIL_GetVideoCfg(channel, VIDEO_MOTION, uiDataSize, &gpsVideoMotion);
            ISIL_TransMotionCfg(&gpsVideoMotion.sMotionCfg);
            pTemp = (S08*)(&gpsVideoMotion.sMotionCfg);
            uiDataSize = (ret < 0)?0:(sizeof(ISIL_VIDEO_MOTION_CFG_S));
		}
		break;

		case IE_TYPE_SET_VIDEOMOTIONCFG:
		{
			if(askLen != sizeof(ISIL_VIDEO_MOTION_CFG_S))
				return ISIL_FAIL;

            ISIL_TransMotionCfg((ISIL_VIDEO_MOTION_CFG_S*)ieheader_payload);
            ret = ISIL_SetMDActive(channel, (ISIL_VIDEO_MOTION_CFG_S*)ieheader_payload);
		}
		break;

		case IE_TYPE_GET_VIDEOLOSTCFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;

			G_VIDEO_LOST_CFG_S gpsVideoLost;
            uiDataSize = sizeof(G_VIDEO_LOST_CFG_S);
			ret = ISIL_GetVideoCfg(channel, VIDEO_LOST, uiDataSize, &gpsVideoLost);
            ISIL_TransLostCfg(&gpsVideoLost.sLostCfg);
            pTemp = (S08*)(&gpsVideoLost.sLostCfg);
            uiDataSize = (ret < 0)?0:(sizeof(ISIL_VIDEO_LOST_CFG_S));
		}
		break;

		case IE_TYPE_SET_VIDEOLOSTCFG:
		{
			if(askLen != sizeof(ISIL_VIDEO_LOST_CFG_S))
				return ISIL_FAIL;
            ISIL_TransLostCfg((ISIL_VIDEO_LOST_CFG_S*)ieheader_payload);
            ret = ISIL_SetLostActive(channel, (ISIL_VIDEO_LOST_CFG_S*)ieheader_payload);
		}
		break;

		case IE_TYPE_GET_VIDEOMASKCFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
			G_VIDEO_MASK_CFG_S gpsVideoMask;

            uiDataSize = sizeof(G_VIDEO_MASK_CFG_S);
			ret = ISIL_GetVideoCfg(channel, VIDEO_MASK, uiDataSize, &gpsVideoMask);
            ISIL_TransMaskCfg(&gpsVideoMask.sMaskCfg);
            pTemp = (S08*)(&gpsVideoMask.sMaskCfg);
            uiDataSize = (ret < 0)?0:(sizeof(ISIL_VIDEO_MASK_CFG_S));
		}
		break;

		case IE_TYPE_SET_VIDEOMASKCFG:
		{
			if(askLen != sizeof(ISIL_VIDEO_MASK_CFG_S))
				return ISIL_FAIL;
            ISIL_TransMaskCfg((ISIL_VIDEO_MASK_CFG_S*)ieheader_payload);
            ret = ISIL_SetMaskActive(iConnectfd, channel,
                        (ISIL_VIDEO_MASK_CFG_S*)ieheader_payload);
		}
		break;

		case IE_TYPE_GET_VIDEOSHELTERCFG:
		{
			if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
			G_VIDEO_SHELTER_CFG_S gpsVideoShelter;
            uiDataSize = sizeof(G_VIDEO_SHELTER_CFG_S);
			ret = ISIL_GetVideoCfg(channel, VIDEO_LOST, uiDataSize, &gpsVideoShelter);
            ISIL_TransShelterCfg(&gpsVideoShelter.sShelterCfg);
            pTemp = (S08*)(&gpsVideoShelter.sShelterCfg);
            uiDataSize = (ret < 0)?0:(sizeof(G_VIDEO_SHELTER_CFG_S));
		}
		break;


		case IE_TYPE_SET_VIDEOSHELTERCFG:
		{
			if(askLen != sizeof(ISIL_VIDEO_SHELTER_CFG_S))
			{
				return ISIL_FAIL;
			}
            ISIL_TransShelterCfg((ISIL_VIDEO_SHELTER_CFG_S*)ieheader_payload);
            ret = ISIL_SetShelterActive(channel,
                (ISIL_VIDEO_SHELTER_CFG_S*)ieheader_payload);
		}
		break;

		case IE_TYPE_GET_ALARMINCFG:
		{
            if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;

            ISIL_ALARMIN_CFG_S sAlarmInCfg;
            uiDataSize = sizeof(ISIL_ALARMIN_CFG_S);
            memset(&sAlarmInCfg, 0 , uiDataSize);
            ret = ISIL_GetAlarmCfg(channel, ALARM_IN, ISIL_ALARM_TYPE_VIDEO_MAX,
                    ISIL_LINKAGE_TYPE_MAX,uiDataSize, &sAlarmInCfg);

            pTemp = (S08*)&sAlarmInCfg;
            uiDataSize = (ret < 0)?0:uiDataSize;
		}
		break;

		case IE_TYPE_SET_ALARMINCFG:
        {
            if(askLen != sizeof(ISIL_ALARMIN_CFG_S))
                return ISIL_FAIL;

            ret = ISIL_SetAlarmInActive(iConnectfd, channel,
                    (ISIL_ALARMIN_CFG_S*)ieheader_payload);
        }
		break;

		case IE_TYPE_GET_OUTCTRLCTRL:
		{
            if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
            ISIL_OUTCTRL_CTRL sOutCtlCfg;

            uiDataSize = sizeof(ISIL_OUTCTRL_CTRL);
            memset(&sOutCtlCfg, 0 , uiDataSize);
            ret = ISIL_GetAlarmCfg(channel, ALARM_OUT, ISIL_ALARM_TYPE_VIDEO_MAX,
                    ISIL_LINKAGE_TYPE_MAX,uiDataSize, &sOutCtlCfg);

            pTemp = (S08*)&sOutCtlCfg;
            uiDataSize = (ret < 0)?0:uiDataSize;
		}
		break;

		case IE_TYPE_SET_OUTCTRLCTRL:
		{
		     if(askLen != sizeof(ISIL_OUTCTRL_CTRL))
                return ISIL_FAIL;

            ret = ISIL_SetOutCtrlActive(iConnectfd, channel,
                (ISIL_OUTCTRL_CTRL*)ieheader_payload);
        }
		break;

        case IE_TYPE_GET_OUTCTRLCFG:
        {
            if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
            ISIL_OUTCTRL_CFG_S outctl;
            uiDataSize = sizeof(ISIL_OUTCTRL_CFG_S);
            memset(&outctl, 0, uiDataSize);
            pTemp = (S08*)&outctl;
        }
        break;


        case IE_TYPE_SET_OUTCTRLCFG:
        {
            ret = 0;
        }
        break;

		case IE_TYPE_GET_ALARMLINKAGECFG:
		{
            if(askLen != 8)/*报警类型+联动类型*/
				return ISIL_FAIL;
            U32 * pIETemp = NULL;
            U32 alarmtype;
            U32 linkagetype;
            S08 cTempBuf[ISIL_MAX_MSG_LEN];

            pIETemp = (U32*)ieheader_payload;
            alarmtype = (*pIETemp);
            linkagetype = (*(pIETemp+1));
            ret = ISIL_AlarmLinkageCfgHandle(channel, _swab32(alarmtype),
                            _swab32(linkagetype), ISIL_GET_PARAMETER,
                            ISIL_MAX_MSG_LEN, cTempBuf);

            uiDataSize = ISIL_GetLinkageCfgSize(_swab32(alarmtype),
                _swab32(linkagetype), channel);
            pTemp = cTempBuf;
        }
		break;

		case IE_TYPE_SET_ALARMLINKAGECFG:
		{
            U32 *pIETemp = (U32*)ieheader_payload;
            U32 alarmtype = (*pIETemp);
            U32 linkagetype = (*(pIETemp+1));
            ret = ISIL_AlarmLinkageCfgHandle(channel, _swab32(alarmtype),
                _swab32(linkagetype), ISIL_SET_PARAMETER,
                (S32)(askLen- 8),(pIETemp + 2));
		}
		break;

		case IE_TYPE_GET_ALARMSTATUS:
		{
		}
		break;

		case IE_TYPE_SET_ALARMSTATUS:
		{}
		break;

		case IE_TYPE_GET_ENCODECFG:
		{
        }
		break;

		case IE_TYPE_SET_ENCODECFG:
		{}
		break;

		case IE_TYPE_GET_STREAMCFG:
		{
            if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;

            G_STREAM_CFG sStreamCfg;

            uiDataSize = sizeof(G_STREAM_CFG);
            memset(&sStreamCfg, 0 , uiDataSize);
            sStreamCfg.channel = channel;
            ret =  ISIL_GetStreamCfg(&sStreamCfg);

            ISIL_TransStreamCfg(&sStreamCfg.sStreamCfg);
            pTemp = (S08*)(&sStreamCfg.sStreamCfg);
            uiDataSize = (ret < 0)?0:(sizeof(ISIL_STREAM_CFG_S));
        }
		break;

		case IE_TYPE_SET_STREAMCFG:
		{
			if(askLen != sizeof(ISIL_STREAM_CFG_S))
			{
				return ISIL_FAIL;
			}
			printf("stream size %d\n", askLen);
			G_STREAM_CFG gsStremCfg;
			gsStremCfg.channel = channel;
            ISIL_TransStreamCfg((ISIL_STREAM_CFG_S *) ieheader_payload);
			memcpy(&gsStremCfg.sStreamCfg, (ISIL_STREAM_CFG_S*)ieheader_payload,
				sizeof(ISIL_STREAM_CFG_S));

			ret = ISIL_SetStreamCfg(iConnectfd, &gsStremCfg);
		}
		break;

        case IE_TYPE_GET_REALPLAYINFO:
        case IE_TYPE_GET_PLAYBACKINFO:
        {
            if(askLen != sizeof(ISIL_ASK_NETINFO_S))
                return ISIL_FAIL;
            ISIL_NET_INFO_DATA_S sNetInfoData;

            memset(&sNetInfoData, 0x00, sizeof(ISIL_NET_INFO_DATA_S));
            ISIL_TransPlayNetInterface((ISIL_ASK_NETINFO_S*)ieheader_payload);

            ISIL_SetPlayNetInfo(channel, asktype, &sUsrInf,
                (ISIL_ASK_NETINFO_S*)ieheader_payload, &sNetInfoData);

            ISIL_TransPlayNetInfo(&sNetInfoData);
            ISIL_BuildAckData(&ackdata, &sNetInfoData, sizeof(ISIL_NET_INFO_DATA_S),
                                IE_TYPE_GET_REALPLAYINFO + 1, 0);
            ISIL_SendAckData(iConnectfd, &ackdata, 1,
	            /*&session->io_info_t.io_info_union.net_info.sa*/NULL, 0, ISIL_PC_MSG);
            ISIL_SetVDByOpenChn(channel);
           	sleep(1);
            return ISIL_SUCCESS;
        }
        break;

        case IE_TYPE_GET_TALKINFO:
        break;

        case IE_TYPE_GET_BROADCASTINFO:
        break;


        case IE_TYPE_SET_MANUALSTARTRECORD:
        {
            if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;

       
            MSG_FM_START_REC stStartRec;
            stStartRec.lChannel = channel;
            stStartRec.ucRecType = 0x4;//REC_LOG_TYPE_MANUAL_REC;
            stStartRec.u8Stream = REC_ONLY_AUDIO|REC_MAIN_VIDEO;
            
            net_msg_reg_func *net_msg_reg_ptr =get_glb_net_msg_reg_t();

            if( !net_msg_reg_ptr->start_record_file_cb ){
                return ISIL_FAIL;
            }

            ret = net_msg_reg_ptr->start_record_file_cb( (void *)&stStartRec);
            if(ret < 0){
                fprintf(stderr,"start rec file err .\n");
                return ISIL_FAIL;
            }

        }
        break;
        case IE_TYPE_SET_MANUALSTOPRECORD:
        {
            if(askLen != IE_NO_PLAYLOAD)
				return ISIL_FAIL;
            
            MSG_FM_STOP_REC stStopRec;
            stStopRec.lChannel = channel;

            net_msg_reg_func *net_msg_reg_ptr =get_glb_net_msg_reg_t();

            if( !net_msg_reg_ptr->stop_record_file_cb ){
                return ISIL_FAIL;
            }

            ret = net_msg_reg_ptr->stop_record_file_cb( stStopRec.lChannel );
            if(ret < 0){
                fprintf(stderr,"stop record file  err .\n");
                return ISIL_FAIL;
            }
            
            
        }
        break;

        case IE_TYPE_FILE_SEARCH:
        {
            ISIL_BuildAckData(&ackdata, NULL, IE_NO_PLAYLOAD,
					IE_TYPE_FILE_SEARCH + 1, 0);

			ISIL_SendAckData(iConnectfd, &ackdata, 1, NULL, 0, ISIL_PC_MSG);

            ISIL_HandleSearchEvnt(iConnectfd, (ISIL_SEARCH_EVNET*)ieheader_payload, 1, ISIL_PC_MSG);
            return ISIL_SUCCESS;
        }
        break;
        case (IE_TYPE_FILE_SEARCH + 1):
        {
            return ISIL_SUCCESS;
        }
        break;
        case IE_TYPE_FILE_OPEN:
        {
            DEBUG_FUNCTION();
            /*获得打开文件的相关信息*/
            ISIL_TransFileOpen((ISIL_NETFILE_OPEN*)ieheader_payload);
		    ISIL_GetPlaybackFileInfo(iConnectfd, ISIL_PC_MSG,
                (ISIL_NETFILE_OPEN*)ieheader_payload, arg);
		    return ISIL_SUCCESS;
        }
        break;
        /*文件操作命令解析*/
        case IE_TYPE_FILE_SET_LOOP:
        case IE_TYPE_FILE_SET_READ_ORDER:
        case IE_TYPE_FILE_GET_FRAMES:
        case IE_TYPE_FILE_GET_FILEINFO:
        case IE_TYPE_FILE_GET_POSITION:
        case IE_TYPE_FILE_SET_POSITION:
        case IE_TYPE_FILE_SET_PERCENT:
        case IE_TYPE_FILE_SET_READ_KEYFRAME:
        {
           ISIL_TransFileCmmd((NET_FILE_COMMAND *)ieheader_payload);
           ISIL_SetPlaybackAction(iConnectfd, asktype, ISIL_PC_MSG,
                        (NET_FILE_COMMAND*)ieheader_payload, channel);
           ret = 0;
        }
        break;

        /*mjpeg*/
        case IE_TYPE_GET_MJPEGCFG:
        {
            ISIL_MJPEG_CFG mjpeg;
            uiDataSize = sizeof(ISIL_MJPEG_CFG);
        }
        break;
        case IE_TYPE_SET_MJPEGCFG:
            break;
        case IE_TYPE_GET_H264FEATURECFG:
        {
            VENC_H264_FEATURE pccfg[2];
            pTemp = (S08*)(pccfg);
            uiDataSize = sizeof(pccfg);
            ret = ISIL_GetH264Cfg(channel, pccfg, uiDataSize);
            ISIL_Trans264Feature(&pccfg[0]);
            ISIL_Trans264Feature(&pccfg[1]);
        }
        break;

        case IE_TYPE_SET_H264FEATURECFG:
        {
            //VENC_PC_H264_FEATURE* cfg;
            //cfg = (VENC_PC_H264_FEATURE*)ieheader_payload);
            VENC_H264_FEATURE* cfg, *tmp;
            cfg = tmp = (VENC_H264_FEATURE*)ieheader_payload;
            printf("============%s %d\n", __FUNCTION__, cfg->i_mb_delay_value);
            ISIL_Trans264Feature(tmp);
            tmp++;
            ISIL_Trans264Feature(tmp);
            ret = ISIL_SetH264Feature(iConnectfd, channel, cfg);
        }
        break;

        /*供IDE/USB专用*/
        case IE_TYPE_FILE_AUTO_START_REC:
        {
            ISIL_NODE_PATH* path;
            path = (ISIL_NODE_PATH*)ieheader_payload;
            printf("dev path %s, mnt path %s\n", path->dev_path,
                path->mount_point);

        }
        break;

		default:
		break;
	}
    ISIL_BuildAckData(&ackdata, pTemp, uiDataSize,
                asktype + 1, (ret < 0)?ISIL_ERR_FAILURE:0);
	//ISIL_SendAckData(iConnectfd, &ackdata, 1,
	    //&session->io_info_t.io_info_union.net_info.sa, 0, ISIL_PC_MSG);

    ISIL_SendAckData(iConnectfd, &ackdata, 1,
	    NULL, 0, ISIL_PC_MSG);
	if(NULL != pUsr)
	{
	    free(pUsr);
        pUsr = NULL;
	}
    if(NULL != sComCfg)
    {
        free(sComCfg);
        sComCfg = NULL;
    }
	return ISIL_SUCCESS;

}

void ISIL_TransRealPlay(ISIL_NET_REALPLAY_S* cfg)
{
    if(NULL == cfg)
        return;
    cfg->lDeviceID = _swab32(cfg->lDeviceID);
    cfg->nLinkType = _swab16(cfg->nLinkType);
}

/*解析自动搜索设备事件*/
static int ISIL_ParseAutoSearchDev(S08 *data, S32 iConnectfd, void * arg)
{
    ISIL_NET_IE_ASK_S *ieheader;
    S08 *ieheader_payload;
    ISIL_ACK_DATA_S ackdata;
    S32 ret = 0;
    S32 channel;
    ISIL_RET_SEARCH_DEVICE_INFO_S stDevInfo;

    if(NULL == arg)
    {
        ISIL_TRACE(BUG_ERROR, "[%s] arg null", __FUNCTION__);
        return ISIL_FAIL;
    }
    memset(&stDevInfo, 0, sizeof(ISIL_RET_SEARCH_DEVICE_INFO_S));
    ieheader = (ISIL_NET_IE_ASK_S*)data;

    memset(&ackdata, 0x00,sizeof(ISIL_ACK_DATA_S));

    ieheader = (ISIL_NET_IE_ASK_S*)((char*)ieheader);
    ieheader_payload = (S08*)(ieheader + 1);
    channel = _swab32(ieheader->lReserve);

    if(_swab16(ieheader->nAskLength) != 0)
    {
        return ISIL_FAIL;
    }
    ISIL_SearchDevInfo(&stDevInfo);
    ISIL_BuildAckData(&ackdata, &stDevInfo, sizeof(stDevInfo),
			IE_TYPE_ACK_AUTOSEARCH, 0);

    ISIL_SendAckData(iConnectfd, &ackdata, 1,
        NULL/*&(((SESSION*)arg)->io_info_t.io_info_union.net_info.sa)*/, 0,ISIL_PC_MSG);
    return ISIL_SUCCESS;
}

int ISIL_ParseLogin(S08 *data, S32 iConnectfd)
{
    ISIL_NET_IE_ASK_S *ieheader;
    S08 *ieheader_payload;
    ISIL_ACK_DATA_S ackdata;
    S32 ret = 0;
    ISIL_USR_CFG_S sUsrInf;
	ISIL_USR_LOGIN_S  *usrinfo;


    ieheader = (ISIL_NET_IE_ASK_S*)data;

    memset(&ackdata, 0x00,sizeof(ISIL_ACK_DATA_S));

    ieheader = (ISIL_NET_IE_ASK_S*)((char*)ieheader);
    ieheader_payload = (S08*)(ieheader + 1);

	if(_swab16(ieheader->nAskLength) !=sizeof(ISIL_USR_LOGIN_S))
		return ISIL_FAIL;

	usrinfo = (ISIL_USR_LOGIN_S *)ieheader_payload;
	ISIL_TRACE(BUG_NORMAL, "login, name:%s, pswd:%s", usrinfo->sName, usrinfo->sPassword);
	ret = ISIL_VerifyUsrInfo(usrinfo);

	if(ret == ISIL_SUCCESS)
	{
		ret = ISIL_GetUsrInfoByName(usrinfo->sName, &sUsrInf);
		ISIL_BuildAckData(&ackdata, sUsrInf.cLevel, sizeof(sUsrInf.cLevel),
			IE_TYPE_ACK_LOGIN, 0);
	}
	else
	{
		ISIL_BuildAckData(&ackdata, NULL, IE_NO_PLAYLOAD,
			IE_TYPE_ACK_LOGIN, ISIL_ERR_FAILURE);
	}

    ISIL_SendAckData(iConnectfd, &ackdata, 1,
        NULL, 0,ISIL_PC_MSG);

    return ISIL_SUCCESS;
}



/*将通道的告警设置为活动状态*/
S32 ISIL_SetAlarmActive(S32 iChannel, S32 iVideoAlarmType, void *pAlarmCfg)
{
    return ISIL_SUCCESS;
}

static S32 GetMjpegSizeFrom_Win(S32 size_w){
    S32 size_e = -1;
    switch(size_w)
    {
        case CIF_VIDEO_IMG:
            size_e = ISIL_VIDEO_SIZE_CIF;
        break;
        case QCIF_VIDEO_IMG:
            size_e = ISIL_VIDEO_SIZE_QCIF;
        break;
        case TWO_CIF_VIDEO_IMG:
            size_e = ISIL_VIDEO_SIZE_2CIF;
        break;
        case D1_VIDEO_IMG:
            size_e = ISIL_VIDEO_SIZE_D1;
        break;
        
        default:
            size_e = ISIL_VIDEO_SIZE_USER_E;
            break;
    }
    return size_e;
}
S32 ISIL_SetMjpegCfg(S32 iConnectfd, S32 channel, ISIL_MJPEG_CFG* pCfg)
{
    S32 size_e;
    CHIP_CHN_PHY_ID phy_id_ptr;

    
    #ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &phy_id_ptr);
    #else
    ISIL_GetChipChnIDByLogicID(channel,&phy_id_ptr);
    #endif

    size_e = GetMjpegSizeFrom_Win(pCfg->cImageSize);

    fprintf(stderr, "%s mjpeg size %d fps %d enable %d\n", __FUNCTION__, size_e, pCfg->cFPS, pCfg->cEnable);
    ISIL_MediaSDK_SetMJpegFormat(phy_id_ptr.chipid, phy_id_ptr.chanid, (ISIL_VIDEO_SIZE_TYPE)size_e);
    ISIL_MediaSDK_SetMJpegFrameRate(phy_id_ptr.chipid, phy_id_ptr.chanid,pCfg->cFPS);
    /*
    if(pCfg->cEnable)
    {
        ISIL_MediaSDK_StartMJpegCapture(phy_id_ptr.chipid, phy_id_ptr.chanid);
    }
    else{
        ISIL_MediaSDK_StopMJpegCapture(phy_id_ptr.chipid, phy_id_ptr.chanid);
    } 
    */ 
    return ISIL_SUCCESS;
}
  
/*根据配置文件设置Stream结构的相关选项*/
void ISIL_SetChannelParameterByStreamCfg(S32 iConnectfd, G_STREAM_CFG* pCfg)
{
    S32 channel;

    if(NULL == pCfg)
    {
        return;
    }

    channel = pCfg->channel;
    if(pCfg->sStreamCfg.cMotionEnable)
    {
        ISIL_SetMDActive(channel, &pCfg->sStreamCfg.stVideoMotion);
    }
    if(pCfg->sStreamCfg.cLostEnable)
    {
        ISIL_SetLostActive(channel, &pCfg->sStreamCfg.stVideoLost);
    }
    if(pCfg->sStreamCfg.cShelterEnable)
    {
        ISIL_SetShelterActive(channel, &pCfg->sStreamCfg.stVideoShelter);
    }
    if(pCfg->sStreamCfg.cMaskEnable)
    {
        ISIL_SetMaskActive(iConnectfd, channel, &pCfg->sStreamCfg.stVideoMask);
    }
    if(pCfg->sStreamCfg.cOverlayEnable)
    {
        ISIL_SetOverlayActive(iConnectfd, channel, &pCfg->sStreamCfg.stVideoOverlay);
    }
    if(pCfg->sStreamCfg.cPTZEnable)
    {
        ISIL_SetPTZActive(channel, &pCfg->sStreamCfg.stPTZCfg);
    }
    if(pCfg->sStreamCfg.cPTZPresetEnable)
    {
        ISIL_SetPTZPresetActive(channel, &pCfg->sStreamCfg.stPTZPrsetCfg);
    }
    if(pCfg->sStreamCfg.cSignalEnable)
    {
        ISIL_SetSignActive(iConnectfd, channel, &pCfg->sStreamCfg.stVideoSignal);
    }
    if(pCfg->sStreamCfg.cEncodeEnable)
    {
        ISIL_SetEncodeActive(iConnectfd, channel, &pCfg->sStreamCfg.stStreamEncode);
    }
    ISIL_SetMjpegCfg(iConnectfd, channel, &pCfg->sStreamCfg.stMJPEGCfg);
    return;
}

void ISIL_ImageSizeEnumToDigit(U32 type, U32* w1, U32* h1, U32 w2, U32 h2)
{
    switch(type)
    {
        case CIF_VIDEO_IMG:
            *w1 = 352 >> 4;
            *h1 = 288 >> 4;
            break;

        case D1_VIDEO_IMG:
        case FOUR_CIF_VIDEO_IMG:
            *w1 = 704 >> 4;
            *h1 = 576 >> 4;
            break;
        case QCIF_VIDEO_IMG:
            *w1 = 352 >> 5;
            *h1= 288 >> 5;
            break;
         case HALF_D1_VIDEO_IMG:
            case TWO_CIF_VIDEO_IMG:
            *w1 = 704 >> 4;
            *h1= 288 >> 4;
            break;
        default:
            *w1 = w2;
            *h1 = h2;
            break;
   }
   printf("%s *w= %d, *h = %d\n", __FUNCTION__, *w1, *h1);
    return;
}

S32 ISIL_APP_SetVD(G_STREAM_CFG* pCfg)
{
    S32 ret;
    S32 i=0, j;
    U32 chip_nums;
    G_STREAM_CFG tmp;
    VD_CONFIG_REALTIME Realtime;
    ISIL_ENCODE_INFO_S out;
    S32 sdcnt, hdcnt, allcnt;

    if(NULL == pCfg)
    {
    	fprintf(stderr, "%s parm is null\n", __FUNCTION__);
    	return ISIL_FAIL;
    }
    
    CHIP_ENC_STATUS_INF chip_enc_status;
   
    chip_nums = isil_get_chips_count();
    memset(&chip_enc_status, 0x00, sizeof(CHIP_ENC_STATUS_INF));
    memset(&Realtime, 0x00, sizeof(VD_CONFIG_REALTIME));
    
    Realtime.video_std = 0;
    Realtime.drop_frame = 0;

    for(j = 0; j < chip_nums; j++)
    {
        #ifdef ISIL_USE_SYS_CFG_FILE
        allcnt = ISIL_GetChnCntPerChip((unsigned int *)&sdcnt, (unsigned int *)&hdcnt, j);
        if(allcnt < 0)
        {
            continue;
        }
        #endif

        for(i = 0; i < PER_CHIP_MAX_CHAN_NUM; i++)
        {
            Realtime.chan[i] = i;
            /*将windows的数值转换为嵌入式内部的数值*/
            #ifdef ISIL_USE_SYS_CFG_FILE
            if(i < sdcnt)
            #endif
            {
                memset(&out, 0x00, sizeof(ISIL_ENCODE_INFO_S));
                #ifndef ISIL_USE_SYS_CFG_FILE
                tmp.channel = j*PER_CHIP_MAX_CHAN_NUM + i;
                #else
                tmp.channel = j*allcnt + i;
                #endif
                ISIL_GetStreamCfg(&tmp);
                ISIL_TransValue_W2E(j, &tmp.sStreamCfg.stStreamEncode.stStreamEncode, &out);
                
                
                {    
                    Realtime.frame_rate[i] = 
                        (tmp.sStreamCfg.stStreamEncode.stStreamEncode.cFPS \
                         > tmp.sStreamCfg.stStreamEncode.stStreamEncode2.cFPS)?\
                        tmp.sStreamCfg.stStreamEncode.stStreamEncode.cFPS : \
                        tmp.sStreamCfg.stStreamEncode.stStreamEncode2.cFPS;
                    Realtime.interleave[i] = 0;
                    Realtime.video_size[i] = out.cImageSize;
                }
                fprintf(stderr, "%s chip %d chan[%d] main status %d sub status %d, fps %d, videosize %d\n", __FUNCTION__, j, i, 
                     chip_enc_status.h264_main_stream_status_arr[i],
                     chip_enc_status.h264_sub_stream_status_arr[i], Realtime.frame_rate[i], Realtime.video_size[i]); 
                
            }
        }
        
        ret = ISIL_MediaSDK_ClacSetVd(j, &Realtime);
    }
    
    return ret;
}

S32 ISIL_SetVDByOpenChn(S32 chn)
{
    return 0;
    S32 ret, sdcnt, hdcnt, allcnt;
    S32 i=0, j =0;
    G_STREAM_CFG tmp;
    VD_CONFIG_REALTIME Realtime;
    CHIP_ENC_STATUS_INF chip_enc_status;
    ISIL_ENCODE_INFO_S out;

    memset(&chip_enc_status, 0x00, sizeof(CHIP_ENC_STATUS_INF));
    memset(&Realtime, 0x00, sizeof(VD_CONFIG_REALTIME));

    Realtime.video_std = 0;
    Realtime.drop_frame = 0;
    U32 chip_nums;

    chip_nums = isil_get_chips_count();
    for(j = 0; j < chip_nums; j++)
    {
        ret = get_enc_chan_status_by_chip(j, &chip_enc_status);
        if(ret < 0){
            fprintf(stderr, "get channel status fail\n");
            return ret;
        }
        #ifdef ISIL_USE_SYS_CFG_FILE
        allcnt = ISIL_GetChnCntPerChip((unsigned int *)&sdcnt, (unsigned int *)&hdcnt, j);
        if(allcnt < 0)
        {
            continue;
        }
        #endif
        for(i = 0; i < PER_CHIP_MAX_CHAN_NUM; i++)
        {
            Realtime.chan[i] = i;
            /*将windows的数值转换为嵌入式内部的数值*/
            if(i < sdcnt)
            {
                memset(&out, 0x00, sizeof(ISIL_ENCODE_INFO_S));
                #ifndef ISIL_USE_SYS_CFG_FILE
                tmp.channel = j*PER_CHIP_MAX_CHAN_NUM + i;
                #else
                tmp.channel = j*allcnt + i;
                #endif
                //tmp.channel = j*PER_CHIP_MAX_CHAN_NUM + i;
    
                ISIL_GetStreamCfg(&tmp);
                ISIL_TransValue_W2E(j,&tmp.sStreamCfg.stStreamEncode.stStreamEncode, &out);
                fprintf(stderr, "%s chan[%d] main status %d sub status %d\n", __FUNCTION__, i, 
                     chip_enc_status.h264_main_stream_status_arr[i],
                     chip_enc_status.h264_sub_stream_status_arr[i]); 
            
                if(chip_enc_status.h264_main_stream_status_arr[i] == 1 || 
                   chip_enc_status.h264_sub_stream_status_arr[i] == 1 ||
                   #ifndef ISIL_USE_SYS_CFG_FILE
                   i == chn%PER_CHIP_MAX_CHAN_NUM)
                   #else
                   i == chn%allcnt)
                   #endif
                {    
                    Realtime.frame_rate[i] = 
                        (tmp.sStreamCfg.stStreamEncode.stStreamEncode.cFPS \
                         > tmp.sStreamCfg.stStreamEncode.stStreamEncode2.cFPS)?\
                        tmp.sStreamCfg.stStreamEncode.stStreamEncode.cFPS : \
                        tmp.sStreamCfg.stStreamEncode.stStreamEncode2.cFPS;
                    Realtime.interleave[i] = 0;
                    Realtime.video_size[i] = out.cImageSize;
                }
                fprintf(stderr, "%s chip %d chan[%d] main status %d sub status %d, fps %d\n", __FUNCTION__, j, i, 
                     chip_enc_status.h264_main_stream_status_arr[i],
                     chip_enc_status.h264_sub_stream_status_arr[i], Realtime.frame_rate[i]); 
            }
        }
        
        ret = ISIL_MediaSDK_ClacSetVd(j, &Realtime);
        /*
        tmp.channel = chn;
        ISIL_GetStreamCfg(&tmp);
        ISIL_SetEncodeActive(-1, tmp.channel, &tmp.sStreamCfg.stStreamEncode);
        */
    }
    return ret;
}

S32 ISIL_SetDefaultVD16Cif()
{
    S32 ret = -1;
    S32 i=0, j =0;
    U32 chip_nums;
    S32 sdcnt, hdcnt, allcnt;
    VD_CONFIG_REALTIME Realtime;
    memset(&Realtime, 0x00, sizeof(VD_CONFIG_REALTIME));

    Realtime.video_std = 0;
    Realtime.drop_frame = 0;
    

    chip_nums = isil_get_chips_count();
    for(j = 0; j < chip_nums; j++)
    {
        #ifdef ISIL_USE_SYS_CFG_FILE
        allcnt = ISIL_GetChnCntPerChip((unsigned int *)&sdcnt, (unsigned int *)&hdcnt, j);
        if(allcnt < 0)
        {
            continue;
        }
        #endif
        for(i = 0; i < PER_CHIP_MAX_CHAN_NUM; i++)
        {
            /*将windows的数值转换为嵌入式内部的数值*/ 
            #ifdef ISIL_USE_SYS_CFG_FILE
            if(i < sdcnt)
            #endif
            {
                Realtime.chan[i] = i;
                
                Realtime.frame_rate[i] = 25;
                Realtime.interleave[i] = 0;
                Realtime.video_size[i] = ISIL_VIDEO_SIZE_CIF_E;
            }
        }
        
        ret = ISIL_MediaSDK_ClacSetVd(j, &Realtime);   
    }
    return ret;
}

S32 ISIL_SetDefaultVD4D1(){
    S32  sdcnt, hdcnt, allcnt ,ret = -1;
    S32 i=0, j =0;
    U32 chip_nums;

    VD_CONFIG_REALTIME Realtime;
    memset(&Realtime, 0x00, sizeof(VD_CONFIG_REALTIME));

    Realtime.video_std = 0;
    Realtime.drop_frame = 0;
    

    chip_nums = isil_get_chips_count();
    for(j = 0; j < chip_nums; j++)
    {
        #ifdef ISIL_USE_SYS_CFG_FILE
        allcnt = ISIL_GetChnCntPerChip((unsigned int *)&sdcnt, (unsigned int *)&hdcnt, j);
        if(allcnt < 0)
        {
            continue;
        }
        #endif
        for(i = 0; i < PER_CHIP_MAX_CHAN_NUM; i++)
        {
            /*将windows的数值转换为嵌入式内部的数值*/    
            #ifdef ISIL_USE_SYS_CFG_FILE
            if(i < sdcnt)
            #endif
            {
                Realtime.chan[i] = i;
                Realtime.frame_rate[i] = 25;
                Realtime.interleave[i] = 0;
                Realtime.video_size[i] = ISIL_VIDEO_SIZE_D1_E;
            }
        }
        ret = ISIL_MediaSDK_ClacSetVd(j, &Realtime);   
    }
    return ret;
}

S32 ISIL_SetStreamCfg(S32 iConnectfd, G_STREAM_CFG* pCfg)
{
	S32 iRet;
    if(NULL == pCfg)
	{
		return ISIL_FAIL;
	}

    iRet = ISIL_SaveStreamCfg(pCfg);
    if(iRet < 0)
        return ISIL_FAIL;
    /*查看配置，是否设置VD*/

    iRet = ISIL_APP_SetVD(pCfg);
    if(iRet < 0)
    {
    	fprintf(stderr, "ISIL_SetStreamCfg set vd fail\n");
    }

	ISIL_SetChannelParameterByStreamCfg(iConnectfd, pCfg);

	return ISIL_SUCCESS;
}


S32 ISIL_HandleRecLinkage(void* pLinkageInfo)
{
    return ISIL_SUCCESS;
}

void ISIL_HandleLinkage(U32 lLinkageType, void* pLinkageInfo)
{
    printf("%s linkage type %d", __FUNCTION__, lLinkageType);
    switch(lLinkageType)
    {
        case ISIL_LINKAGE_TYPE_OUTCTRL:
        {
            ISIL_HandleOutCtrlLinkage(pLinkageInfo);
        }
        break;

        case ISIL_LINKAGE_TYPE_PTZ:
        {
            ISIL_HandlePtzLinkage(pLinkageInfo);
        }
        break;

        case ISIL_LINKAGE_TYPE_STREAM_RECORD:
        {
            ISIL_HandleRecLinkage(pLinkageInfo);
        }
        break;

        case ISIL_LINKAGE_TYPE_PIC_SHOOT:
        {
            ISIL_HandleShootLinkage(pLinkageInfo);
        }

        break;
        default:
        break;
    }
}


S32 ISIL_SetLinkageActive(U32 Channel, U32 AlarmType)
{
    S32 iRet;
    U32 ChCnt;
    U32 OutCtlCnt;
    U32 iLinkageType;
    S08 cTempBuf[ISIL_MAX_MSG_LEN];

    /*遍历联动类型*/
    for(iLinkageType = ISIL_LINKAGE_TYPE_OUTCTRL;
        iLinkageType < ISIL_LINKAGE_TYPE_MAX; iLinkageType++)
    {
        /*先读出配置文件*/

        memset(cTempBuf, 0, ISIL_MAX_MSG_LEN);
        ISIL_AlarmLinkageCfgHandle(Channel, AlarmType,
                                    iLinkageType, ISIL_GET_PARAMETER,
                                    ISIL_MAX_MSG_LEN, cTempBuf);
        /*根据配置文件，进行设置联动*/
        ISIL_HandleLinkage(iLinkageType, cTempBuf);

    }
    return ISIL_SUCCESS;
}


S32 ISIL_HandleShootLinkage(void* pLinkageInfo)

{
    return ISIL_SUCCESS;
}

S32 ISIL_InitCfgFile()
{
    ISIL_MkConfigDir();
    ISIL_NetSDKCfgDir();
    ISIL_InitUsrCfgFile();
	return ISIL_SUCCESS;
}


