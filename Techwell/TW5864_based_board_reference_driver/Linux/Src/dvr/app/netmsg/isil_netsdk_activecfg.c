
#include "isil_netsdk_activecfg.h"
#include "isil_ap_mediasdk.h"
//#include "isil_fm_msg.h"
#include "isil_msg_reg_cb.h"
#include "isil_endian_trans.h"
#include "isil_parsemessage.h"
#include "isil_netsdk_cfgfile.h"
#include "isil_channel_map.h"

static void EncoderParameterTransform(LPISIL_DEV_CHANNEL_CFG client, ISIL_ENCODE_CFG_EX* local)
{
    if(NULL == client || NULL == local)
        return;

    local->cEncodeEnable = client->stMainVideoEncOpt[0].byVideoEnable;
    local->cEncode2Enable = client->stAssiVideoEncOpt[0].byVideoEnable;

    /*主码流设置*/
    local->stStreamEncode.cFPS =
        client->stMainVideoEncOpt[0].byFramesPerSec;

    local->stStreamEncode.cImageQuality =
        client->stMainVideoEncOpt[0].byImageQlty;

    local->stStreamEncode.cImageSize =
        client->stMainVideoEncOpt[0].byImageSize;

    local->stStreamEncode.nBitRate =
        client->stMainVideoEncOpt[0].byBitRateControl;

    local->stStreamEncode.nIFrameIntervals =
        client->stMainVideoEncOpt[0].bIFrameInterval;

    /*子码流设置*/
    local->stStreamEncode2.cFPS =
        client->stAssiVideoEncOpt[0].byFramesPerSec;

    local->stStreamEncode2.cImageQuality =
        client->stAssiVideoEncOpt[0].byImageQlty;

    local->stStreamEncode2.cImageSize =
        client->stAssiVideoEncOpt[0].byImageSize;

    local->stStreamEncode.nBitRate =
        client->stMainVideoEncOpt[0].byBitRateControl;

    local->stStreamEncode2.nIFrameIntervals =
        client->stAssiVideoEncOpt[0].bIFrameInterval;
    return;
}

S32 ISIL_NetSDK_ActiveEncodeParameter(S32 channel, S32 netfd, void*buff)
{
    S32 ret;
    S32 i;
    LPISIL_DEV_CHANNEL_CFG cfg;
    G_ENCODE_CFG_S encodeparm;
    if(NULL == buff)
        return ISIL_FAIL;

    /*这个地方需要重新做*/
    cfg = (LPISIL_DEV_CHANNEL_CFG)buff;
    if(channel < 0) /*所有通道*/
    {
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
        for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
        #endif
        {
            /*进行结构体转换*/
            memset(&encodeparm, 0, sizeof(G_ENCODE_CFG_S));
            if(cfg->stMainVideoEncOpt[0].byVideoEnable ||
                cfg->stAssiVideoEncOpt[0].byVideoEnable)
            {
                printf("%s need set ch %d encode parm, fps %d\n", __FUNCTION__, i, cfg->stMainVideoEncOpt[0].byFramesPerSec);
                encodeparm.channel = i;
                EncoderParameterTransform(cfg, &encodeparm.sEncodeCfg);
                /*ISIL_SendMsgToOther(DRV_MOD, G_MSG_SET_ENCODECFG,
                    sizeof(G_ENCODE_CFG_S),  netfd, ISIL_LOCAL_MSG,ISIL_LOCAL_MSG,
                    &encodeparm);*/
            }
            cfg++;
        }
    }
    else /*单独通道*/
    {
        memset(&encodeparm, 0, sizeof(G_ENCODE_CFG_S));
        encodeparm.channel = channel;
        EncoderParameterTransform(cfg, &encodeparm.sEncodeCfg);
        printf("%s, %d, %d\n", __FUNCTION__, encodeparm.channel, encodeparm.sEncodeCfg.stStreamEncode.cFPS);
        /*ISIL_SendMsgToOther(DRV_MOD, G_MSG_SET_ENCODECFG,
            sizeof(G_ENCODE_CFG_S),  netfd,  ISIL_LOCAL_MSG,
            &encodeparm);
            */
    }
    return ISIL_SUCCESS;
}

static void ISIL_NetSDK_SetEncodeCfgParm(S32 channel, VENC_H264_INFO* cfg)
{
    ISIL_BITRATE_VALUE qp;
    CHIP_CHN_PHY_ID id;
	#ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &id);
	#else
    ISIL_GetChipChnIDByLogicID(channel,&id);
    #endif

    ISIL_MediaSDK_SetEncoderVideoSizeByWH(id.chipid, id.chanid,
        cfg->venc_h264cfg[0].i_logic_video_width_mb_size,
        cfg->venc_h264cfg[0].i_logic_video_height_mb_size, IS_H264_MAIN_STREAM);

    //if(cfg->venc_h264rc[0].e_rc_type == ISIL_STREAM_BITRATE_H264_VBR)
    qp.tQP.cBQP =  cfg->venc_h264rc[0].i_qpb;
    qp.tQP.cIQP = cfg->venc_h264rc[0].i_qpi;
    qp.tQP.cPQP = cfg->venc_h264rc[0].i_qpp;

    ISIL_MediaSDK_SetBitrateControlType(id.chipid, id.chanid,
        cfg->venc_h264rc[0].e_rc_type, qp,
        IS_H264_MAIN_STREAM);

    ISIL_MediaSDK_SetBitrateControlMode(id.chipid, id.chanid,
        cfg->venc_h264rc[0].e_image_priority, IS_H264_MAIN_STREAM);
    if(cfg->venc_h264cfg[0].b_force_I_flag)
        ISIL_MediaSDK_ForceIFrameEncode(id.chipid, id.chanid,
            IS_H264_MAIN_STREAM);

    fprintf(stderr, "%s ip=%d,pb=%d\n", __FUNCTION__,
    		cfg->venc_h264cfg[0].i_I_P_stride,
    		cfg->venc_h264cfg[0].i_P_B_stride);
    ISIL_MediaSDK_SetIBPMode(id.chipid, id.chanid,
        cfg->venc_h264cfg[0].i_I_P_stride,
        cfg->venc_h264cfg[0].i_P_B_stride,
        cfg->venc_h264cfg[0].i_P_B_stride,
        cfg->venc_h264cfg[0].i_fps,
        IS_H264_MAIN_STREAM);

   ISIL_MediaSDK_SetEncoderVideoSizeByWH(id.chipid, id.chanid,
        cfg->venc_h264cfg[1].i_logic_video_width_mb_size,
        cfg->venc_h264cfg[1].i_logic_video_height_mb_size, IS_H264_SUB_STREAM);
   //qp.lImageQuality =  cfg->venc_h264rc[1].i_default_qp;
   qp.tQP.cBQP =  cfg->venc_h264rc[1].i_qpb;
   qp.tQP.cIQP = cfg->venc_h264rc[1].i_qpi;
   qp.tQP.cPQP = cfg->venc_h264rc[1].i_qpp;

   ISIL_MediaSDK_SetBitrateControlType(id.chipid, id.chanid,
        cfg->venc_h264rc[1].e_rc_type, qp,
        IS_H264_MAIN_STREAM);

    ISIL_MediaSDK_SetBitrateControlMode(id.chipid, id.chanid,
        cfg->venc_h264rc[1].e_image_priority, IS_H264_SUB_STREAM);
    if(cfg->venc_h264cfg[1].b_force_I_flag)
        ISIL_MediaSDK_ForceIFrameEncode(id.chipid, id.chanid,
            IS_H264_SUB_STREAM);

    ISIL_MediaSDK_SetIBPMode(id.chipid, id.chanid,
        cfg->venc_h264cfg[1].i_I_P_stride,cfg->venc_h264cfg[0].i_P_B_stride,
        cfg->venc_h264cfg[1].i_P_B_stride,
        cfg->venc_h264cfg[1].i_fps, IS_H264_SUB_STREAM);

    ISIL_MediaSDK_SetH264Feature(id.chipid, id.chanid,
            &cfg->venc_h264feature[0],  IS_H264_MAIN_STREAM);
    ISIL_MediaSDK_SetH264Feature(id.chipid, id.chanid,
                &cfg->venc_h264feature[1],  IS_H264_SUB_STREAM);
    return;
}

static int ISIL_NetSDK_SetEncodeCfgParmAll(S32 channel, VENC_H264_INFO* cfg)
{
    CHIP_CHN_PHY_ID id;


	#ifndef ISIL_USE_SYS_CFG_FILE
    if(get_chip_chn_id_by_logic_id(channel, &id) < 0){
    	return -1;
    }
	#else
    if(ISIL_GetChipChnIDByLogicID(channel,&id) < 0){
    	return -1;
    }
    #endif

    if(ISIL_MediaSDK_SetH264CfgAll(id.chipid, id.chanid, &cfg->venc_h264cfg[0], IS_H264_MAIN_STREAM) < 0){
    	return -1;
    }

    if(ISIL_MediaSDK_SetH264CfgAll(id.chipid, id.chanid, &cfg->venc_h264cfg[1], IS_H264_SUB_STREAM) < 0){
    	return -1;
    }

    if(ISIL_MediaSDK_SetRcAll(id.chipid, id.chanid, &cfg->venc_h264rc[0], IS_H264_MAIN_STREAM) < 0){
    	return -1;
    }

    if(ISIL_MediaSDK_SetRcAll(id.chipid, id.chanid, &cfg->venc_h264rc[1], IS_H264_SUB_STREAM) < 0){
    	return -1;
    }

    if(ISIL_MediaSDK_SetH264Feature(id.chipid, id.chanid,
                &cfg->venc_h264feature[0],  IS_H264_MAIN_STREAM) < 0){
    	return -1;
    }

    if(ISIL_MediaSDK_SetH264Feature(id.chipid, id.chanid,
                    &cfg->venc_h264feature[1],  IS_H264_SUB_STREAM) < 0){
    	return -1;
    }

    return 0;
}

#if 0
S32 ISIL_APP_LocalSetVD(VENC_H264_INFO* pCfg)
{
    S32 ret;
    S32 i=0;
    VD_CONFIG_REALTIME Realtime;

    if(NULL == pCfg)
    {
    	fprintf(stderr, "%s parm is null\n", __FUNCTION__);
    	return ISIL_FAIL;
    }


    Realtime.video_std = 0;
    Realtime.drop_frame = 0;
    for(i = 0; i < 16; i++)
	{
    	/*将windows的数值转换为嵌入式内部的数值*/
    	if(i == pCfg->channel)
		{
			Realtime.chan[i] = i;
			Realtime.frame_rate[i] = pCfg->venc_h264cfg[0].i_fps;
			Realtime.interleave[i] = 2;
			Realtime.video_size[i] = out.cImageSize;
		}
		else
		{
			tmp.channel = i;
			ISIL_GetStreamCfg(&tmp);
			Realtime.chan[i] = i;
			Realtime.frame_rate[i] = tmp.sStreamCfg.stStreamEncode.stStreamEncode.cFPS;
			Realtime.interleave[i] = 2;
			Realtime.video_size[i] = out.cImageSize;

		}
	}
    ret = ISIL_MediaSDK_ClacSetVd(0, &Realtime);
    return ret;
}
#endif

S32 ISIL_NetSDK_ActiveMediaSdkEncodeParameter(S32 channel, S32 netfd, void*buff)
{
    S32 ret = 0;
    S32 i;
    VENC_H264_INFO* cfg;
    CHIP_CHN_PHY_ID id;
    ISIL_BITRATE_VALUE qp;

    if(NULL == buff)
        return ISIL_FAIL;

    cfg = (VENC_H264_INFO*)buff;

    //fprintf(stderr, "----%s, %d, %d------\n", __FUNCTION__, __LINE__, channel);
    if(channel < 0) /*锟斤拷锟斤拷通锟斤拷*/
    {
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
        for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
        #endif
        {
            //ISIL_NetSDK_SetEncodeCfgParm(i, cfg);
        	ret = ISIL_NetSDK_SetEncodeCfgParmAll(i, cfg);
        	if(ret < 0){
        		break;
        	}
            printf("ISIL_NetSDK_ActiveMediaSdkEncodeParameter: w %d h%d, ipstr %d\n",
            		cfg->venc_h264cfg[0].i_logic_video_width_mb_size,
            		cfg->venc_h264cfg[0].i_logic_video_height_mb_size,
            		cfg->venc_h264cfg[0].i_I_P_stride);
            cfg++;
        }
    }
    else /*锟斤拷锟斤拷通锟斤拷*/
    {
        //ISIL_NetSDK_SetEncodeCfgParm(channel, cfg);
    	ret = ISIL_NetSDK_SetEncodeCfgParmAll(i, cfg);
    }

    return ret;
}

static S32 ISIL_NetSDK_SetMediaSdkMjpegParm(S32 channel, VENC_MJPG_CFG* cfg)
{
    S32 ret;
    CHIP_CHN_PHY_ID id;

    if(NULL == cfg)
        return ISIL_FAIL;
	#ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &id);
	#else
    ret = ISIL_GetChipChnIDByLogicID(channel,&id);
    #endif

    ret = ISIL_MediaSDK_SetMJpegCfgParm(id.chipid, id.chanid, cfg);
    return ret;
}

static S32 ISIL_NetSDK_SetMediaSdkAudioParm(S32 channel, VENC_AUD_CFG* cfg)
{
    S32 ret;
    CHIP_CHN_PHY_ID id;

    if(NULL == cfg)
        return ISIL_FAIL;
	#ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &id);
	#else
    ret = ISIL_GetChipChnIDByLogicID(channel,&id);
    #endif

    ret = ISIL_MediaSDK_SetAudioCfgAll(id.chipid, id.chanid, cfg);
    return ret;
}
/*active mediasdk mjpeg*/
S32 ISIL_NetSDK_ActiveMediaSdkMjpeg(S32 channel, S32 netfd, void*buff)
{
    S32 ret;
    S32 i;
    S32 chCnt;
    VENC_MJPG_INFO* cfg;

    if(NULL == buff)
        return ISIL_FAIL;

    cfg = (VENC_MJPG_INFO*)buff;
    if(channel < 0) /*所有通道*/
    {
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
        for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
        #endif
        {
            ret = ISIL_NetSDK_SetMediaSdkMjpegParm(i, &cfg->venc_mjpg_cfg[0]);
        }
        cfg++;
    }
    else
    {
        ret = ISIL_NetSDK_SetMediaSdkMjpegParm(channel, &cfg->venc_mjpg_cfg[0]);
    }


    return ret;
}

#if 0
S32 ISIL_NetSDK_ActiveMediaSdkAudio(S32 channel, S32 netfd, void*buff)
{
    S32 ret;
    VENC_AUDIO_INFO* cfg;
    CHIP_CHN_PHY_ID id;

    if(NULL == buff)
        return ISIL_FAIL;
	#ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &id);
	#else
    ISIL_GetChipChnIDByLogicID(channel,&id);
    #endif

    cfg = (VENC_AUDIO_INFO*)buff;
    /*
    ret = ISIL_MediaSDK_SetAudioType(id.chipid, id.chanid,
            cfg->venc_audio_cfg.e_audio_type);
    if(ret < 0)
        return ISIL_FAIL;
    ret = ISIL_MediaSDK_SetAudioParm(id.chipid, id.chanid,
        cfg->venc_audio_cfg.i_bit_wide, cfg->venc_audio_cfg.i_sample_rate);
    if(ret < 0)
        return ISIL_FAIL;
        */

	ret = ISIL_MediaSDK_SetAudioCfgAll(id.chipid, id.chanid, &cfg->venc_audio_cfg);
	if(ret < 0)
		return ISIL_FAIL;
	return ISIL_SUCCESS;
}
#else
S32 ISIL_NetSDK_ActiveMediaSdkAudio(S32 channel, S32 netfd, void*buff)
{
    S32 ret;
    S32 i;
    S32 chCnt;
    VENC_AUDIO_INFO* cfg;

    if(NULL == buff)
        return ISIL_FAIL;

    cfg = (VENC_AUDIO_INFO*)buff;
    if(channel < 0) /*锟斤拷锟斤拷通锟斤拷*/
    {
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
        for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
        #endif
        {
            ret = ISIL_NetSDK_SetMediaSdkAudioParm(i, &cfg->venc_audio_cfg);
        }
        cfg++;
    }
    else
    {
        ret = ISIL_NetSDK_SetMediaSdkAudioParm(channel, &cfg->venc_audio_cfg);
    }


    return ret;
}
#endif

S32 ISIL_NetSDK_ActiveMediaSdkNotRT(S32 channel, S32 netfd, void*buff)
{
    #if 0
    S32 ret;
    S32 i;
    ENC_CTL_VD_INFO* cfg;
    G_LOCAL_VD_CFG_S parm;
    if(NULL == buff)
        return ISIL_FAIL;

    cfg = (VENC_AUDIO_INFO*)buff;

    memset(&parm, 0, sizeof(G_LOCAL_VD_CFG_S));
    parm.channel = channel;
    parm.parmType = ISIL_LOCAL_NOT_RT_ENC;
    memcpy(&parm.sVDCfg, cfg, sizeof(ENC_CTL_VD_INFO));
    ISIL_SendMsgToOther(DRV_MOD, G_MSG_SET_ENCODECFG,
            sizeof(G_LOCAL_VD_CFG_S),  netfd, ISIL_LOCAL_MSG,
            &parm);

    #endif

    return ISIL_SUCCESS;
}

static S32 ISIL_NetSDK_SetMediaSdkAD(S32 channel, VI_ANALOG_INFO* cfg)
{
    S32 ret;
     CHIP_CHN_PHY_ID id;

    if(NULL == cfg)
        return ISIL_FAIL;
	#ifndef ISIL_USE_SYS_CFG_FILE
    get_chip_chn_id_by_logic_id(channel, &id);
	#else
    ISIL_GetChipChnIDByLogicID(channel,&id);
    #endif
    /*
     * ret = ISIL_MediaSDK_SetVideoStandard(id.chipid, cfg->videostd.eVideoType);
    if(ret < 0)
        return ISIL_FAIL;
     */

    ret = ISIL_MediaSDK_SetVideoPara(id.chipid, id.chanid,
        cfg->vichcfg.u32Brightness, cfg->vichcfg.u32Contrast,
        cfg->vichcfg.u32Saturation, cfg->vichcfg.u32Hue, cfg->vichcfg.u32Sharpness);
    return ret;
}
S32 ISIL_NetSDK_ActiveMediaSdkAD(S32 channel, S32 netfd, void*buff)
{
    S32 ret;
    S32 i;
    VI_ANALOG_INFO* cfg;
    if(NULL == buff)
        return ISIL_FAIL;

    cfg = (VI_ANALOG_INFO*)buff;

    if(channel < 0) /*所有通道*/
    {
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
        for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
        #endif
        {
            ret = ISIL_NetSDK_SetMediaSdkAD(i, cfg);
            if(ret < 0)
                return ret;
            cfg++;
        }
    }
    else /*单独通道*/
    {
       ret = ISIL_NetSDK_SetMediaSdkAD(channel, cfg);
    }

    return ret;
}



S32 ISIL_NetSDK_ActiveFileSearch(S32 iFd, ISIL_SEARCH_EVNET* pEvnt)
{

   S32 iRet;
   if(NULL == pEvnt)
    return ISIL_FAIL;
   iRet = ISIL_HandleSearchEvnt(iFd, pEvnt, 0, ISIL_LOCAL_MSG);
   return iRet;
}

S32 ISIL_NetSDK_ActiveFileOpen(S08* filename)
{

    return ISIL_SUCCESS;
}

S32 ISIL_NetSDK_ActiveFileClose(S08* filename)
{
    return ISIL_SUCCESS;
}

S32 ISIL_NetSDK_StartFileRec(S32 iChn)
{
    #if 0
    stStartRec.lChannel = channel;
    stStartRec.ucRecType = REC_LOG_TYPE_MANUAL_REC;
    ret = ISIL_SendMsgToOther(DISK_MOD, MSG_TYPE_FM_START_REC,
        sizeof(MSG_FM_START_REC), iConnectfd, ISIL_PC_MSG,&stStartRec);
    #endif
    return ISIL_SUCCESS;

}

S32 ISIL_NetSDK_StopFileRec(S32 iChn)
{
    #if 0
    stStartRec.lChannel = channel;
    stStartRec.ucRecType = REC_LOG_TYPE_MANUAL_REC;
    ret = ISIL_SendMsgToOther(DISK_MOD, MSG_TYPE_FM_START_REC,
        sizeof(MSG_FM_START_REC), iConnectfd, ISIL_PC_MSG,&stStartRec);
    #endif
    return ISIL_SUCCESS;
}


S32 ISIL_NetSDK_SetSubWindows(void *buff)
{
    #if 0
    S32 iRet;
    ISIL_SUB_WINDOWS *win;
    S32 len;
    if(NULL == buff)
        return ISIL_FAIL;
    win = (ISIL_SUB_WINDOWS*)buff;
    len = sizeof(ISIL_SUB_WINDOWS)+win->cSubWindowsNum*sizeof(ISIL_LOCCALGUI_RECT);
    iRet= ISIL_SendMsgToOther(LOC_PB_MOD, G_MSG_SET_SUB_WINDOWS,
        len, -1, ISIL_LOCAL_MSG,win);

    return iRet;
    #endif
    return ISIL_SUCCESS;
}
S32 ISIL_NetSDK_BindFileWindows(S32 fd, void *buff)
{
    S32 ret;
    MSG_PREVIEW_DEC_REG_T *ptr = NULL;
    ISIL_SUB_WINDOWS_BIND *win;

	if(NULL == buff)
	   return ISIL_FAIL;
    win = (ISIL_SUB_WINDOWS_BIND*)buff;

    ptr = get_glb_preview_dec_reg_t();
    if(NULL == ptr)
    {
    	printf("ISIL_NetSDK_BindFileWindows get cb null\n");
    	return ISIL_FAIL;
    }
	//fprintf(stderr, "%s subwx %d subwy %d\n", __FUNCTION__, win->stSubInfo.stSubWindows[0].iSubWin_x, win->stSubInfo.stSubWindows[0].iSubWin_y);
    ret = ptr->dec_preview_start(win->lChipId, win);
    return ret;
}

S32 ISIL_NetSDK_SetPlayMod(void *buff)
{
    S32 iRet;
	MSG_FILE_DEC_REG_T *filedec;
    G_ISIL_PLAY_MOD* playMod;

    if(NULL == buff)
        return ISIL_FAIL;

    playMod = (G_ISIL_PLAY_MOD*)buff;
	filedec = get_glb_file_dec_reg_ptr();
    iRet= filedec->set_play_mod_param(playMod->fd, (void*)playMod);
    return iRet;
}

S32 ISIL_NetSDK_SetAVSync(void *buff)
{
    S32 iRet;
    G_FILE_AV_SYNC* AVSync;
    MSG_FILE_DEC_REG_T *filedec;

    if(NULL == buff)
        return ISIL_FAIL;

    filedec = get_glb_file_dec_reg_ptr();


    AVSync = (G_FILE_AV_SYNC*)buff;
    iRet= filedec->fm_set_av_sync(AVSync->fd, AVSync);
    return iRet;
}

S32 ISIL_NetSDK_SetDisplayPortMode(void *buff)
{
	S32 iRet;
	ISIL_DIAPLAY_PORT_MODE* portMode;
	MSG_FILE_DEC_REG_T *filedec;

	if(buff == NULL)
	{
		return ISIL_FAIL;
	}

	filedec = get_glb_file_dec_reg_ptr();
    portMode = (ISIL_DIAPLAY_PORT_MODE*)buff;
    iRet= filedec->fm_dec_set_diaplay_port_mode(0, portMode);

	return iRet;
}

S32 ISIL_NetSDK_TestW(S32 chip, void *buff)
{
	S32 iRet;
	S32* tmp;
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}

	tmp = (S32*)buff;

#ifdef LOCAL_GUI_ON_BOARD
	printf("%s, test type %d\n", __FUNCTION__, *tmp);
    switch(*tmp)
#else
    printf("%s, test type %d\n", __FUNCTION__, _swab32(*tmp));
    switch(_swab32(*tmp))
#endif
	{
		case ISIL_TEST_REG:
		{
			ISIL_TEST_REG_INFO* info;
			info = (ISIL_TEST_REG_INFO *)buff;
			ISIL_TransTestReg((ISIL_TEST_REG_INFO *)info);
			printf("test reg, addrr 0x%x, count 0x%x\n", info->reg.startaddr, info->reg.count);
			iRet = ISIL_MediaSDK_WriteReg(chip, info->reg.startaddr, (unsigned long*)info->buffer, info->reg.count);
		}
		break;
		case ISIL_TEST_DDR:{
			ISIL_TEST_DDR_INFO* info;
			info = (ISIL_TEST_DDR_INFO *)buff;
			ISIL_TransTestDDR((ISIL_TEST_DDR_INFO*)info);
			iRet = ISIL_MediaSDK_WriteDDR(chip, info->ddr.startaddr, (unsigned long*)info->buffer, info->ddr.count);
		}
		break;

		case ISIL_TEST_MPB:{
			ISIL_TEST_MPB_INFO* info;
			info = (ISIL_TEST_MPB_INFO *)buff;
			ISIL_TransTestMPB((ISIL_TEST_MPB_INFO*) info);
			iRet = ISIL_MediaSDK_WriteMPB(chip, info->mpb.startaddr, info->buffer, info->mpb.count);
		}
		break;
		case ISIL_TEST_I2C:{
			ISIL_TEST_I2C_INFO* info;
			info = (ISIL_TEST_I2C_INFO *)buff;

			ISIL_TransTestI2C((ISIL_TEST_I2C_INFO*) info);
			iRet = ISIL_MediaSDK_WriteI2C(chip, info->i2c.busaddr, info->i2c.offset, info->buffer, info->i2c.count, 1);
		}
		break;
		case ISIL_TEST_GPIO:{
			ISIL_TEST_GPIO_INFO* info;
			info = (ISIL_TEST_GPIO_INFO *)buff;
			ISIL_TransTestGPIO((ISIL_TEST_GPIO_INFO*) info);

		}
		break;

	}

	return iRet;
}


S32 ISIL_NetSDK_TestR(S32 chip, U32* sdsize, void* cmd,void *buff)
{
	S32 iRet;
	S32* tmp;
	if(NULL == buff && NULL == cmd)
	{
		return ISIL_FAIL;
	}

	tmp = (S32*)cmd;
#ifdef LOCAL_GUI_ON_BOARD
	printf("%s, test type %d\n", __FUNCTION__, *tmp);
    switch(*tmp)
#else
    printf("%s, test type %d\n", __FUNCTION__, _swab32(*tmp));
    switch(_swab32(*tmp))
#endif
	{
		case ISIL_TEST_REG:
		{
			ISIL_TEST_REG_INFO* info;
			ISIL_TEST_REG_INFO* sdbuff;
			sdbuff = (ISIL_TEST_REG_INFO*)buff;
			info = (ISIL_TEST_REG_INFO *)cmd;

			ISIL_TransTestReg((ISIL_TEST_REG_INFO *)cmd);
			printf("test reg, addrr 0x%x, count 0x%x\n", info->reg.startaddr, info->reg.count);
			iRet = ISIL_MediaSDK_ReadReg(chip, info->reg.startaddr, sdbuff->buffer, info->reg.count);
			*sdsize = sizeof(ISIL_TEST_REG_INFO) + info->len;
		}
		break;
		case ISIL_TEST_DDR:{
			ISIL_TEST_DDR_INFO* info;
			ISIL_TEST_DDR_INFO* sdbuff;
			info = (ISIL_TEST_DDR_INFO *)cmd;
			sdbuff = (ISIL_TEST_DDR_INFO *)buff;
			ISIL_TransTestDDR((ISIL_TEST_DDR_INFO*)cmd);
			iRet = ISIL_MediaSDK_ReadDDR(chip, info->ddr.startaddr, sdbuff->buffer, info->ddr.count);
			*sdsize = sizeof(ISIL_TEST_DDR_INFO) + info->len;
		}
		break;

		case ISIL_TEST_MPB:{
			ISIL_TEST_MPB_INFO* info;
			ISIL_TEST_MPB_INFO* sdbuff;
			info = (ISIL_TEST_MPB_INFO *)cmd;
			sdbuff = (ISIL_TEST_MPB_INFO *)buff;

			ISIL_TransTestMPB((ISIL_TEST_MPB_INFO*) cmd);
			iRet = ISIL_MediaSDK_ReadMPB(chip, info->mpb.startaddr, sdbuff->buffer, info->mpb.count);
			*sdsize = sizeof(ISIL_TEST_MPB_INFO) + info->len;
		}
		break;
		case ISIL_TEST_I2C:{
			ISIL_TEST_I2C_INFO* info;
			ISIL_TEST_I2C_INFO* sdbuff;
			info = (ISIL_TEST_I2C_INFO *)cmd;
			sdbuff = (ISIL_TEST_I2C_INFO *)buff;

			ISIL_TransTestI2C((ISIL_TEST_I2C_INFO*) cmd);
			iRet = ISIL_MediaSDK_ReadI2C(chip, info->i2c.busaddr, info->i2c.offset, sdbuff->buffer, info->i2c.count, 1);
			*sdsize = sizeof(ISIL_TEST_I2C_INFO) + info->len;
		}
		break;
		case ISIL_TEST_GPIO:{
			ISIL_TEST_GPIO_INFO* info;
			ISIL_TEST_GPIO_INFO* sdbuff;

			info = (ISIL_TEST_GPIO_INFO *)cmd;
			sdbuff = (ISIL_TEST_GPIO_INFO *)buff;

			ISIL_TransTestGPIO((ISIL_TEST_GPIO_INFO*) cmd);
		}
		break;

	}

	return iRet;
}

S32 ISIL_NetSDK_SetVideoStandard(char stdandard)
{
    #if 0
	S32 iRet;
	struct ISIL_display_video_standard std;

    std.e_video_standard = stdandard;
    iRet= ISIL_SendMsgToOther(LOC_PB_MOD, G_MSG_VIDEO_STANDARD_CFG,
        sizeof(struct ISIL_display_video_standard), -1, ISIL_LOCAL_MSG, &std);

	return iRet;
    #endif
	return 0;
}

S32 ISIL_NetSDK_ActiveAlarmType(S32 chn, int type)
{
	S32 iRet;
	CHIP_CHN_PHY_ID id;
	#ifndef ISIL_USE_SYS_CFG_FILE
	get_chip_chn_id_by_logic_id(chn, &id);
	#else
    ISIL_GetChipChnIDByLogicID(chn,&id);
    #endif
	iRet = ISIL_MediaSDK_SetChannelAlarmType(id.chipid, id.chanid, type);
	return iRet;
}

S32 ISIL_NetSDK_ClearAlarmType(S32 chn, int type)
{
	S32 iRet;
	CHIP_CHN_PHY_ID id;
	#ifndef ISIL_USE_SYS_CFG_FILE
	get_chip_chn_id_by_logic_id(chn, &id);
	#else
    ISIL_GetChipChnIDByLogicID(chn,&id);
    #endif
	iRet = ISIL_MediaSDK_ClearChannelAlarmType(id.chipid, id.chanid, type);
	return iRet;
}


static void Debug_osd_value(S32 chn, ISIL_OSD_CFG* cfg)
{
	if(NULL == cfg)
	{
		return;
	}
	fprintf(stderr, "OSD: ch %d na %d nx %d ny %d ta %d tx %d ty %d sha %d shx %d shy %d\n",
			chn, cfg->name_attrib, cfg->name_pos_x, cfg->name_pos_y,
			cfg->time_attrib, cfg->time_pos_x, cfg->time_pos_y, cfg->shelter1_attrib,
			cfg->shelter1_pos_x, cfg->shelter1_pos_y);
}
S32 ISIL_NetSDK_ActiveMediaSdkOsd(S32 channel, S32 netfd, void*buff)
{
    S32 ret;
    S32 i;
    S32 chCnt;
    ISIL_OSD_CFG* cfg;
	CHIP_CHN_PHY_ID id;
    if(NULL == buff)
        return ISIL_FAIL;

    cfg = (ISIL_OSD_CFG*)buff;

    if(channel < 0) /*所有通道*/
    {
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
        for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
        #endif
        {
			#ifndef ISIL_USE_SYS_CFG_FILE
			get_chip_chn_id_by_logic_id(i, &id);
			#else
			ISIL_GetChipChnIDByLogicID(i,&id);
			#endif
			Debug_osd_value(i,cfg);
            ret = ISIL_MediaSDK_SetOsdCfg(id.chipid, id.chanid,
										  cfg,  IS_H264_MAIN_STREAM);
			ret = ISIL_MediaSDK_SetOsdCfg(id.chipid, id.chanid,
										  cfg,  IS_H264_SUB_STREAM);
			cfg++;
        }
		
    }
    else
    {
		#ifndef ISIL_USE_SYS_CFG_FILE
		get_chip_chn_id_by_logic_id(channel, &id);
		#else
		ISIL_GetChipChnIDByLogicID(channel,&id);
		#endif
		Debug_osd_value(channel,cfg);
        ret = ISIL_MediaSDK_SetOsdCfg(id.chipid, id.chanid,
										  cfg,  IS_H264_MAIN_STREAM);
		ret = ISIL_MediaSDK_SetOsdCfg(id.chipid, id.chanid,
										  cfg,  IS_H264_SUB_STREAM);
    }


    return ret;
}
