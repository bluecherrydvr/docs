
#include "isil_endian_trans.h"
#include "isil_channel_map.h"


void ISIL_TransTime(ISIL_TIME* time)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == time)
        return;
    time->nYear = _swab16(time->nYear);
    return;
}
void ISIL_TransPlayNetInterface(ISIL_ASK_NETINFO_S* psNetLinkType)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == psNetLinkType)
        return;
    psNetLinkType->nLinkProtocol = _swab16(psNetLinkType->nLinkProtocol);
}
void ISIL_TransPlayNetInfo(ISIL_NET_INFO_DATA_S* cfg)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == cfg)
        return;
    cfg->nChannel = _swab16(cfg->nChannel);
    cfg->nLinkProtocol = _swab16(cfg->nLinkProtocol);
    cfg->nPort = _swab16(cfg->nPort);
}

void ISIL_TransDevBaseInfo(ISIL_DEV_BASE_CFG_S *sDevInfo)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    U32 tmpi;
    U16 tmps;

    if(NULL == sDevInfo)
        return;
   /* printf("%d %d %d %d %d %d %d %d %d\n",
        sDevInfo->lDeviceClass,
        sDevInfo->lDeviceID,
        sDevInfo->lDeviceType,
        sDevInfo->lDSPSoftwareBuildDate,
        sDevInfo->lDSPSoftwareVersion,
        sDevInfo->lHardwareVersion,
        sDevInfo->lPanelVersion,
        sDevInfo->lSoftwareBuildDate,
        sDevInfo->lSoftwareVersion);
    */
    sDevInfo->lDeviceClass = _swab32(sDevInfo->lDeviceClass);
    sDevInfo->lDeviceID = _swab32(sDevInfo->lDeviceID);
    sDevInfo->lDeviceType = _swab32(sDevInfo->lDeviceID);
    sDevInfo->lDSPSoftwareBuildDate = _swab32(sDevInfo->lDSPSoftwareBuildDate);
    sDevInfo->lDSPSoftwareVersion = _swab32(sDevInfo->lDSPSoftwareVersion);
    sDevInfo->lHardwareVersion = _swab32(sDevInfo->lHardwareVersion);
    sDevInfo->lPanelVersion = _swab32(sDevInfo->lPanelVersion);
    sDevInfo->lSoftwareBuildDate = _swab32(sDevInfo->lSoftwareBuildDate);
    sDevInfo->lSoftwareVersion = _swab32(sDevInfo->lSoftwareVersion);
    sDevInfo->stSysTime.nYear = _swab16(sDevInfo->stSysTime.nYear);

    /*printf("%d %d %d %d %d %d %d %d %d\n",
        sDevInfo->lDeviceClass,
        sDevInfo->lDeviceID,
        sDevInfo->lDeviceType,
        sDevInfo->lDSPSoftwareBuildDate,
        sDevInfo->lDSPSoftwareVersion,
        sDevInfo->lHardwareVersion,
        sDevInfo->lPanelVersion,
        sDevInfo->lSoftwareBuildDate,
        sDevInfo->lSoftwareVersion);
        */
    return;
}

void ISIL_TransNetworkConfig(ISIL_NET_CFG_S *param)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == param)
        return;

    //param->lGateWay = _swab32(param->lGateWay);
    //param->lIPAddr = _swab32(param->lIPAddr);
    //param->lMultiCastIP = _swab32(param->lMultiCastIP);
    //param->lNetMask = _swab32(param->lNetMask);
    //param->lPppoeIPAddr = _swab32(param->lPppoeIPAddr);
    param->lPppoeTimesl = _swab32(param->lPppoeTimesl);
    param->lSize = _swab32(param->lSize);

    param->nDataPort = _swab16(param->nDataPort);
    param->nDdnsMapDataPort = _swab16(param->nDdnsMapDataPort);
    param->nDdnsMapWebPort = _swab16(param->nDdnsMapWebPort);
    param->nDdnsPortNo = _swab16(param->nDdnsPortNo);
    param->nMultiCastPort = _swab16(param->nMultiCastPort);
    param->nNetInterface = _swab16(param->nNetInterface);
    param->nWebPort = _swab16(param->nWebPort);

    return;
}

void ISIL_TransRect(U32 *rect, S32 size)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    S32 i;
    for(i = 0; i < size; i++)
    {
        rect[i] = _swab32(rect[i]);
    }
    return;
}

void ISIL_TransMotionCfg(ISIL_VIDEO_MOTION_CFG_S* cfg)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == cfg)
    {
        return;
    }
    cfg->lAutoClearTime = _swab16(cfg->lAutoClearTime);
    ISIL_TransRect(cfg->lMotionRect, 24);
    return;
}


void ISIL_TransLostCfg(ISIL_VIDEO_LOST_CFG_S* cfg)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == cfg)
    {
        return;
    }
    cfg->nAutoClearTime = _swab16(cfg->nAutoClearTime);
    return;
}


void ISIL_TransMaskCfg(ISIL_VIDEO_MASK_CFG_S* cfg)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == cfg)
    {
        return;
    }
    ISIL_TransRect(cfg->lMaskArea, 24);
}

void ISIL_TransShelterCfg(ISIL_VIDEO_SHELTER_CFG_S* cfg)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == cfg)
    {
        return;
    }
    cfg->lAutoClearTime = _swab16(cfg->lAutoClearTime);
    ISIL_TransRect(cfg->lShelterRect, 24);
}

void ISIL_TransEncodeInfo(ISIL_ENCODE_INFO_S* cfg)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == cfg)
        return;
    cfg->nBitRate = _swab16(cfg->nBitRate);
    cfg->nBPFrameInterval = _swab16(cfg->nBPFrameInterval);
    cfg->nIFrameIntervals = _swab16(cfg->nIFrameIntervals);
    cfg->nGOPIntervals = _swab16(cfg->nGOPIntervals);
    cfg->nHeightMBSize = _swab16(cfg->nHeightMBSize);
    cfg->nWidthMBSize = _swab16(cfg->nWidthMBSize);
    return;
}

void ISIL_TransEncodeCfg(ISIL_ENCODE_CFG_S* cfg)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    if(NULL == cfg)
        return;
    ISIL_TransEncodeInfo(&cfg->stStreamEncode);
    ISIL_TransEncodeInfo(&cfg->stStreamEncode2);
    return;
}
void ISIL_TransOSDCfg(ISIL_OVERLAY_CFG_S* cfg)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
	S32 i;
    if(NULL == cfg)
        return;
    cfg->nNameTopLeftX = _swab16(cfg->nNameTopLeftX);
    cfg->nNameTopLeftY = _swab16(cfg->nNameTopLeftY);
    cfg->nTimeTopLeftX = _swab16(cfg->nTimeTopLeftX);
    cfg->nTimeTopLeftY = _swab16(cfg->nTimeTopLeftY);

    for(i = 0; i < 4; i++)
    {
    	cfg->stText[i].nTextTopLeftX = _swab16(cfg->stText[i].nTextTopLeftX);
    	cfg->stText[i].nTextTopLeftY = _swab16(cfg->stText[i].nTextTopLeftY);
    }
    return;
}

void ISIL_TransLocalOSDCfg(ISIL_OSD_CFG* cfg)
{

    if(NULL == cfg)
        return;
    
    
    cfg->channel = _swab32(cfg->channel);
    cfg->name_attrib = _swab32(cfg->name_attrib);
    
    cfg->name_pos_x = _swab16(cfg->name_pos_x);
   
    cfg->name_pos_y = _swab16(cfg->name_pos_y);

    cfg->time_attrib = _swab32(cfg->time_attrib);
    cfg->time_pos_x = _swab16(cfg->time_pos_x);
    cfg->time_pos_y = _swab16(cfg->time_pos_y);

    cfg->shelter1_attrib = _swab32(cfg->shelter1_attrib);
    cfg->shelter1_pos_x = _swab16(cfg->shelter1_pos_x);
    cfg->shelter1_pos_y = _swab16(cfg->shelter1_pos_y);

    cfg->shelter2_attrib = _swab32(cfg->shelter2_attrib);
    cfg->shelter2_pos_x = _swab16(cfg->shelter2_pos_x);
    cfg->shelter2_pos_y = _swab16(cfg->shelter2_pos_y);

    cfg->subtitle1_attrib = _swab32(cfg->subtitle1_attrib);
    cfg->subtitle1_pos_x = _swab16(cfg->subtitle1_pos_x);
    cfg->subtitle1_pos_y = _swab16(cfg->subtitle1_pos_y);

    cfg->subtitle2_attrib = _swab32(cfg->subtitle2_attrib);
    cfg->subtitle2_pos_x = _swab16(cfg->subtitle2_pos_x);
    cfg->subtitle2_pos_y = _swab16(cfg->subtitle2_pos_y);

    cfg->subtitle3_attrib = _swab32(cfg->subtitle3_attrib);
    cfg->subtitle3_pos_x = _swab16(cfg->subtitle3_pos_x);
    cfg->subtitle3_pos_y = _swab16(cfg->subtitle3_pos_y);

    cfg->subtitle4_attrib = _swab32(cfg->subtitle4_attrib);
    cfg->subtitle4_pos_x = _swab16(cfg->subtitle4_pos_x);
    cfg->subtitle4_pos_y = _swab16(cfg->subtitle4_pos_y);

    return;
}

void TransMediaSdkOsdByCh(S32 channel,void *buff)
{
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif

    ISIL_OSD_CFG* cfg;
    S32 i;
    
    if(NULL == buff)
    {
        return;
    }

    cfg = (ISIL_OSD_CFG*)buff;
    if(channel < 0) /*所有通道*/
    {
        #ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
        #else
        for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
        #endif
        {
            /*进行结构体转换*/
            ISIL_TransLocalOSDCfg(cfg);
            cfg++;
        }
    }
    else /*单独通道*/
    {
        ISIL_TransLocalOSDCfg(cfg);
    }
    return;
}

void ISIL_TransStreamCfg(ISIL_STREAM_CFG_S* cfg)
{
    if(NULL == cfg)
        return;
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    ISIL_TransEncodeCfg(&cfg->stStreamEncode);
    //ISIL_TransCommCfg(&cfg->stComCfg);
    //(&cfg->stRecord);
    //(cfg->stSchRecord);
    ISIL_TransLostCfg(&cfg->stVideoLost);
    ISIL_TransMaskCfg(&cfg->stVideoMask);
    ISIL_TransMotionCfg(&cfg->stVideoMotion);
    //cfg->stVideoOverlay;
    ISIL_TransOSDCfg(&cfg->stVideoOverlay);
    ISIL_TransShelterCfg(&cfg->stVideoShelter);

    return;
}

void ISIL_TransSearchEvent(ISIL_SEARCH_EVNET* cfg)
{
    if(NULL == cfg)
    {
        return;
    }
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    cfg->lFlag = _swab32(cfg->lFlag);
    ISIL_TransTime(&cfg->stStart);
    ISIL_TransTime(&cfg->stEnd);
    return;
}

void ISIL_TransFileOpen(ISIL_NETFILE_OPEN * pFileOpen)
{
    if(NULL == pFileOpen)
        return;

#ifdef LOCAL_GUI_ON_BOARD
            return;
#endif
    pFileOpen->lDeviceID = _swab32(pFileOpen->lDeviceID);
    pFileOpen->nFileCount = _swab16(pFileOpen->nFileCount);
    pFileOpen->nLinkType = _swab16(pFileOpen->nLinkType);
    return;
}
void ISIL_TransFileBaseInfo(ISIL_FILE_BASE_INFO *info)
{
    if(NULL == info)
        return;
     #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    info->lBeginTimeStamp = _swab32(info->lBeginTimeStamp);
    info->lEndTimeStamp = _swab32(info->lEndTimeStamp);
    info->lFileSize = _swab32(info->lFileSize);
    info->lMaxKeyFrameInterval = _swab32(info->lMaxKeyFrameInterval);
    info->lTimeLength = _swab32(info->lTimeLength);
    return;
}
void ISIL_TransFileCmmd(NET_FILE_COMMAND *cmd)
{
    if(NULL == cmd)
        return;
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    cmd->lFilePosition = _swab32(cmd->lFilePosition);
    cmd->lTimePosition = _swab32(cmd->lTimePosition);
    cmd->xReadParam.lReadDataID = _swab32(cmd->xReadParam.lReadDataID);
    cmd->xReadParam.lReadLength = _swab32(cmd->xReadParam.lReadLength);
    cmd->xFrameParam.lFrameCount = _swab32(cmd->xFrameParam.lFrameCount);
    cmd->xFrameParam.lFrameDelay = _swab32(cmd->xFrameParam.lFrameDelay);
    return;
}

/*local gui*/
void ISIL_LOCAL_TransVersionInfo(ISIL_VERSION_INFO* cfg)
{
    if(NULL == cfg)
        return;
    #ifdef LOCAL_GUI_ON_BOARD
        return;
    #endif
    cfg->dwDspSoftwareBuildDate = _swab32(cfg->dwDspSoftwareBuildDate);
    cfg->dwDspSoftwareVersion = _swab32(cfg->dwDspSoftwareVersion);
    cfg->dwHardwareDate = _swab32(cfg->dwHardwareDate);
    cfg->dwHardwareVersion = _swab32(cfg->dwHardwareVersion);
    cfg->dwPanelSoftwareBuildDate = _swab32(cfg->dwPanelSoftwareBuildDate);
    cfg->dwPanelVersion = _swab32(cfg->dwPanelVersion);
    cfg->dwSoftwareBuildDate = _swab32(cfg->dwSoftwareBuildDate);
    cfg->dwSoftwareVersion = _swab32(cfg->dwSoftwareVersion);
    cfg->dwWebBuildDate = _swab32(cfg->dwWebBuildDate);
    cfg->dwWebVersion = _swab32(cfg->dwWebVersion);
    return;
}
void ISIL_LOCAL_TransDspEncodeCap(ISIL_DSP_ENCODECAP* cfg)
{
    U32 i;
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->dwEncodeModeMask = _swab32(cfg->dwEncodeModeMask);
    cfg->dwImageSizeMask = _swab32(cfg->dwImageSizeMask);
    for(i = 0; i < 8; i++)
        cfg->dwImageSizeMask_Assi[i] = _swab32(cfg->dwImageSizeMask_Assi[i]);
    cfg->dwMaxEncodePower = _swab32(cfg->dwMaxEncodePower);
    cfg->dwStreamCap = _swab32(cfg->dwStreamCap);
    cfg->dwVideoStandardMask = _swab32(cfg->dwVideoStandardMask);
    cfg->wChannelMaxSetSync = _swab32(cfg->wChannelMaxSetSync);
    cfg->wMaxSupportChannel = _swab32(cfg->wMaxSupportChannel);
}


void ISIL_LOCAL_TransSysAttrCfg(ISIL_DEV_SYSTEM_ATTR_CFG* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->dwSize = _swab32(cfg->dwSize);
    ISIL_LOCAL_TransVersionInfo(&cfg->stVersion);
    ISIL_LOCAL_TransDspEncodeCap(&cfg->stDspEncodeCap);
    cfg->wDevNo = _swab16(cfg->wDevNo);
    return;
}

void ISIL_LOCAL_TransVideoEncOpt(ISIL_VIDEOENC_OPT* opt)
{
    if(NULL == opt)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    opt->nChannels = _swab16(opt->nChannels);
    opt->nSamplesPerSec = _swab32(opt->nSamplesPerSec);
    opt->wBitsPerSample = _swab16(opt->wBitsPerSample);
    opt->wLimitStream = _swab16(opt->wLimitStream);
    return;
}

void ISIL_LOCAL_TransTimeSect(ISIL_TSECT* sect)
{
    if(NULL == sect)
        return;
#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif

    sect->bEnable = _swab32(sect->bEnable);
    sect->iBeginHour = _swab32(sect->iBeginHour);
    sect->iBeginMin = _swab32(sect->iBeginMin);
    sect->iBeginSec = _swab32(sect->iBeginSec);
    sect->iEndHour = _swab32(sect->iEndHour);
    sect->iEndMin = _swab32(sect->iEndMin);
    sect->iEndSec = _swab32(sect->iEndSec);
    return;
}

void ISIL_LOCAL_TransColorCfg(ISIL_COLOR_CFG* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    ISIL_LOCAL_TransTimeSect(&cfg->stSect);
    return;
}

void ISIL_LOCAL_TransRect(ISIL_RECT* rect)
{
    if(NULL == rect)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    rect->bottom = _swab32(rect->bottom);
    rect->left = _swab32(rect->left);
    rect->right = _swab32(rect->right);
    rect->top = _swab32(rect->top);
    return;
}

void ISIL_LOCAL_TransEncodeWidget(ISIL_ENCODE_WIDGET *cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->rgbaFrontground = _swab32(cfg->rgbaFrontground);
    cfg->rgbaBackground = _swab32(cfg->rgbaBackground);
    ISIL_LOCAL_TransRect(&cfg->rcRect);
    return;
}


void ISIL_LOCAL_TransChanelCfg(ISIL_DEV_CHANNEL_CFG *cfg)
{
    U32 i;
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->dwSize = _swab16(cfg->dwSize);
    for(i = 0; i < ISIL_REC_TYPE_NUM; i++)
        ISIL_LOCAL_TransVideoEncOpt(&cfg->stMainVideoEncOpt[i]);
    for(i = 0; i < ISIL_N_ENCODE_AUX; i++)
        ISIL_LOCAL_TransVideoEncOpt(&cfg->stAssiVideoEncOpt[i]);
    for(i = 0; i < ISIL_N_COL_TSECT; i++)
        ISIL_LOCAL_TransColorCfg(&cfg->stColorCfg[i]);
    ISIL_LOCAL_TransEncodeWidget(&cfg->stTimeOSD);
    ISIL_LOCAL_TransEncodeWidget(&cfg->stChannelOSD);
    for(i = 0; i < ISIL_N_COVERS; i++)
        ISIL_LOCAL_TransEncodeWidget(&cfg->stBlindCover[i]);
    return;
}

/*for media sdk*/
void ISIL_LOCAL_Trans_VENC_MJPG_CFG(VENC_MJPG_CFG* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->change_mask_flag = _swab32(cfg->change_mask_flag);
    cfg->e_image_level = _swab32(cfg->e_image_level);
    cfg->i_capture_frame_number = _swab32(cfg->i_capture_frame_number);
    cfg->i_capture_frame_stride = _swab32(cfg->i_capture_frame_stride);
    //printf(" mjpg swab32 before: 0x%x\n", cfg->i_capture_type);
    cfg->i_capture_type = _swab32(cfg->i_capture_type);
    //printf(" mjpg swab32 after: 0x%x\n", cfg->i_capture_type);
    cfg->i_image_height_mb_size = _swab32(cfg->i_image_height_mb_size);
    cfg->i_image_width_mb_size = _swab32(cfg->i_image_width_mb_size);
    return;
}
void ISIL_LOCAL_Trans_VENC_H264_FEATURE(VENC_H264_FEATURE* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->change_mask_flag = _swab32(cfg->change_mask_flag);
    cfg->i_mb_delay_value = _swab32(cfg->i_mb_delay_value);
    return;
}


void debug_venc_h264_feature(VENC_H264_FEATURE* cfg)
{
    if(cfg){
        fprintf( stderr,"b_enable_deinterlace:%d, "
                        "b_enable_half_pixel:%d, "
                        "b_enable_I_4X4:%d, "
                        "b_enable_quarter_pixel:%d, "
                        "b_enable_skip:%d, "
                        "change_mask_flag:%d, "
                        "i_mb_delay_value:%d .\n",
                 cfg->b_enable_deinterlace,
                 cfg->b_enable_half_pixel,
                 cfg->b_enable_I_4X4,
                 cfg->b_enable_quarter_pixel,
                 cfg->b_enable_skip,
                 cfg->change_mask_flag,
                 cfg->i_mb_delay_value);
    }
}

void ISIL_LOCAL_Trans_VENC_H264_RC(VENC_H264_RC* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->change_mask_flag = _swab32(cfg->change_mask_flag);
    cfg->e_image_priority = _swab32(cfg->e_image_priority);
    cfg->e_rc_type = _swab32(cfg->e_rc_type);
    cfg->i_qpi = _swab32(cfg->i_qpi);
    cfg->i_qpb = _swab32(cfg->i_qpb);
    cfg->i_qpp = _swab32(cfg->i_qpp);
    return;
}


void debug_venc_h264_rc(VENC_H264_RC* cfg)
{
    if(cfg){
        fprintf( stderr,"change_mask_flag:%d, "
                        "e_image_priority:%d, "
                        "e_rc_type:%d, "
                        "i_qpb:%d, "
                        "i_qpi:%d, "
                        "i_qpp:%d .\n",
                 cfg->change_mask_flag,
                 cfg->e_image_priority,
                 cfg->e_rc_type,
                 cfg->i_qpb,
                 cfg->i_qpi,
                 cfg->i_qpp);
    }
}

void ISIL_LOCAL_Trans_VENC_H264_CFG(VENC_H264_CFG* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->change_mask_flag = _swab32(cfg->change_mask_flag);
    cfg->b_force_I_flag = _swab32(cfg->b_force_I_flag);
    cfg->i_bps = _swab32(cfg->i_bps);
    cfg->i_fps = _swab32(cfg->i_fps);
    cfg->i_gop_value = _swab32(cfg->i_gop_value);
    cfg->i_I_P_stride = _swab32(cfg->i_I_P_stride);
    cfg->i_logic_video_height_mb_size = _swab16(cfg->i_logic_video_height_mb_size);
    cfg->i_logic_video_width_mb_size = _swab16(cfg->i_logic_video_width_mb_size);
    cfg->i_P_B_stride = _swab32(cfg->i_P_B_stride);
    return;
}

void debug_venc_h264_cfg(VENC_H264_CFG* cfg)
{
    if(cfg){
        fprintf( stderr,"b_force_I_flag:%d, "
                        "change_mask_flag:%d, "
                        "i_bps:%d, "
                        "i_fps:%d, "
                        "i_gop_value:%d, "
                        "i_I_P_stride:%d, "
                        "i_logic_video_height_mb_size:%d "
                        "i_logic_video_width_mb_size:%d "
                        "i_P_B_stride:%d .\n",
                 cfg->b_force_I_flag,
                 cfg->change_mask_flag,
                 cfg->i_bps,
                 cfg->i_fps,
                 cfg->i_gop_value,
                 cfg->i_I_P_stride,
                 cfg->i_logic_video_height_mb_size,
                 cfg->i_logic_video_width_mb_size,
                 cfg->i_P_B_stride);
    }
}

void ISIL_LOCAL_Trans_VENC_AUD_CFG(VENC_AUD_CFG* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->change_mask_flag = _swab32(cfg->change_mask_flag);
    cfg->channel = _swab32(cfg->channel);
    cfg->e_audio_type = _swab16(cfg->e_audio_type);
    return;
}

void ISIL_LOCAL_Trans_VENC_H264_INFO(VENC_H264_INFO* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    ISIL_LOCAL_Trans_VENC_H264_CFG(&cfg->venc_h264cfg[0]);
    ISIL_LOCAL_Trans_VENC_H264_CFG(&cfg->venc_h264cfg[1]);
    
    debug_venc_h264_cfg(&cfg->venc_h264cfg[0]);
    debug_venc_h264_cfg(&cfg->venc_h264cfg[1]);

    ISIL_LOCAL_Trans_VENC_H264_FEATURE(&cfg->venc_h264feature[0]);
    ISIL_LOCAL_Trans_VENC_H264_FEATURE(&cfg->venc_h264feature[1]);

    debug_venc_h264_feature(&cfg->venc_h264feature[0]);
    debug_venc_h264_feature(&cfg->venc_h264feature[1]);

    ISIL_LOCAL_Trans_VENC_H264_RC(&cfg->venc_h264rc[0]);
    ISIL_LOCAL_Trans_VENC_H264_RC(&cfg->venc_h264rc[1]);

    debug_venc_h264_rc(&cfg->venc_h264rc[0]);
    debug_venc_h264_rc(&cfg->venc_h264rc[1]);



    return;
}


void ISIL_LOCAL_Trans_VENC_MJPG_INFO(VENC_MJPG_INFO* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    ISIL_LOCAL_Trans_VENC_MJPG_CFG(&cfg->venc_mjpg_cfg[0]);
    ISIL_LOCAL_Trans_VENC_MJPG_CFG(&cfg->venc_mjpg_cfg[1]);
    return;
}

void ISIL_LOCAL_Trans_VENC_AUDIO_INFO(VENC_AUDIO_INFO* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    ISIL_LOCAL_Trans_VENC_AUD_CFG(&cfg->venc_audio_cfg);
    return;
}

void ISIL_LOCAL_Trans_NotRT(ENC_CTL_VD_INFO* cfg)
{
    int i;
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
//    cfg->vdmode.eVdmode = _swab32(cfg->vdmode.eVdmode);
    cfg->vdcfg.i_vd_chan_number = _swab32(cfg->vdcfg.i_vd_chan_number);
    for(i = 0; i < ENC_MAXCH; i++)
    {
        cfg->vdcfg.vd_param[i].change_mask_flag =
            _swab32(cfg->vdcfg.vd_param[i].change_mask_flag);

        cfg->vdcfg.vd_param[i].i_phy_video_height_mb_size =
            _swab16(cfg->vdcfg.vd_param[i].i_phy_video_height_mb_size);

        cfg->vdcfg.vd_param[i].i_phy_video_width_mb_size =
            _swab16(cfg->vdcfg.vd_param[i].i_phy_video_width_mb_size);

        cfg->vdcfg.vd_param[i].hor_reverse=
            _swab32(cfg->vdcfg.vd_param[i].hor_reverse);

        cfg->vdcfg.vd_param[i].ver_reverse =
            _swab32(cfg->vdcfg.vd_param[i].ver_reverse);
    }
    return;
}

void ISIL_LOCAL_Trans_AD(VI_ANALOG_INFO* cfg)
{
    int i;
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->change_mask_flag = _swab32(cfg->change_mask_flag);
    cfg->vichcfg.u32Brightness = _swab32(cfg->vichcfg.u32Brightness);
    cfg->vichcfg.u32Channel = _swab32(cfg->vichcfg.u32Channel);
    cfg->vichcfg.u32ChipID = _swab32(cfg->vichcfg.u32ChipID);
    cfg->vichcfg.u32Contrast = _swab32(cfg->vichcfg.u32Contrast);
    cfg->vichcfg.u32Hue = _swab32(cfg->vichcfg.u32Hue);
    cfg->vichcfg.u32Mask = _swab32(cfg->vichcfg.u32Mask);
    cfg->vichcfg.u32Saturation = _swab32(cfg->vichcfg.u32Saturation);
    cfg->vichcfg.u32Sharpness = _swab32(cfg->vichcfg.u32Sharpness);

    cfg->videostd.eVideoType = _swab32(cfg->videostd.eVideoType);
    cfg->videostd.u32Channel = _swab32(cfg->videostd.u32Channel);
    cfg->videostd.u32ChipID = _swab32(cfg->videostd.u32ChipID);
    return;
}
void ISIL_Trans264Feature(VENC_H264_FEATURE* cfg)
{
    int i;
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->change_mask_flag = _swab32(cfg->change_mask_flag);
    cfg->i_mb_delay_value = _swab32(cfg->i_mb_delay_value);
    return;
}

static void _TransSubWindows(ISIL_LOCCALGUI_RECT* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
            return;
#endif
    cfg->iSubWin_x = _swab32(cfg->iSubWin_x);
    cfg->iSubWin_y = _swab32(cfg->iSubWin_y);
    cfg->iSubWin_XSize = _swab32(cfg->iSubWin_XSize);
    cfg->iSubWin_YSize = _swab32(cfg->iSubWin_YSize);
    return;
}

void ISIL_TransSubWindows(ISIL_SUB_WINDOWS* cfg)
{
    int i;
    int winCnt;
    ISIL_LOCCALGUI_RECT* tmp;
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif

    tmp = cfg->stSubWindows;
    winCnt = cfg->cSubWindowsNum;
    for(i = 0; i < winCnt; i++)
    {
        _TransSubWindows(tmp);
        printf("get ch %d, x=%d, y=%d,w=%d ,h=%d\n", i,
            tmp->iSubWin_x,tmp->iSubWin_y,tmp->iSubWin_XSize, tmp->iSubWin_YSize);
        tmp++;//= sizeof(ISIL_LOCCALGUI_RECT);

    }
    return;
}

void ISIL_TransFileBindWin(ISIL_SUB_WINDOWS_BIND* cfg)
{
    ISIL_LOCCALGUI_RECT* tmp;
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif

    tmp = cfg->stSubInfo.stSubWindows;
    _TransSubWindows(tmp);
    cfg->emViType = _swab32(cfg->emViType);
    cfg->lChipId = _swab32(cfg->lChipId);
    cfg->lDisplayId = _swab32(cfg->lDisplayId);
    cfg->lVi_chId = _swab32(cfg->lVi_chId);

    return;
}
void ISIL_TransDisplayPortMode(ISIL_DIAPLAY_PORT_MODE* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->i_port = _swab32(cfg->i_port);
    cfg->e_mode = _swab32(cfg->e_mode);
    cfg->e_interleave = _swab32(cfg->e_interleave);
    return;
}

void ISIL_TransTestReg(ISIL_TEST_REG_INFO* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->type = _swab32(cfg->type);
    cfg->len = _swab32(cfg->len);
    cfg->reg.count = _swab32(cfg->reg.count);
    cfg->reg.startaddr = _swab32(cfg->reg.startaddr);
    return;
}

void ISIL_TransTestMPB(ISIL_TEST_MPB_INFO* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->type = _swab32(cfg->type);
    cfg->len = _swab32(cfg->len);
    cfg->mpb.count = _swab32(cfg->mpb.count);
    cfg->mpb.startaddr = _swab32(cfg->mpb.startaddr);
    return;
}

void ISIL_TransTestI2C(ISIL_TEST_I2C_INFO* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->type = _swab32(cfg->type);
    cfg->len = _swab32(cfg->len);
    cfg->i2c.count = _swab32(cfg->i2c.count);
    cfg->i2c.busaddr = _swab32(cfg->i2c.busaddr);
    cfg->i2c.offset = _swab32(cfg->i2c.offset);
    return;
}

void ISIL_TransTestDDR(ISIL_TEST_DDR_INFO* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->type = _swab32(cfg->type);
    cfg->len = _swab32(cfg->len);
    cfg->ddr.count = _swab32(cfg->ddr.count);
    cfg->ddr.startaddr = _swab32(cfg->ddr.startaddr);
    return;
}

void ISIL_TransTestGPIO(ISIL_TEST_GPIO_INFO* cfg)
{
    if(NULL == cfg)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    cfg->type = _swab32(cfg->type);
    cfg->gpio.addr = _swab32(cfg->gpio.addr);
    cfg->gpio.mode = _swab32(cfg->gpio.mode);
    cfg->gpio.value = _swab32(cfg->gpio.value);
    return;
}

void ISIL_TransAlarmType(unsigned int* type)
{
    if(NULL == type)
        return;

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    *type = _swab32(*type);
    return;
}

void ISIL_TransChipsChanPriMap(SYS_CHIPS_CHAN_PRI_MAP_T *pri_map)
{
    int i;
    CHIP_CHAN_PRI_MAP_T *chip_map;
    if( !pri_map ){
        return;
    }

#ifdef LOCAL_GUI_ON_BOARD
        return;
#endif
    i = pri_map->chip_nums;
    pri_map->chip_nums = _swab32(pri_map->chip_nums);
    
    for( i = 0 ; i < pri_map->chip_nums ; ++i){
      chip_map =  &pri_map->chip_chan_pri_map_arr[i]; 
      chip_map->chip_id = _swab32(chip_map->chip_id);
      chip_map->valid_chan_nums = _swab32(chip_map->valid_chan_nums);

    }

    return;

}
