#include "isil_ap_mediasdk.h"
#include "isil_test_sdk.h"
#include "isil_media_config.h"
#include "default_cfg_value.h"


/*ChipSDK Base*/
void ISIL_MediaSDK_GetSdkVersion()
{}

/*ChipSDK Chip*/
ISILERR_CODE  ISIL_MediaSDK_SetAutoResetTime(unsigned int nChipIndex, unsigned int nTime)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetVideoStandard(unsigned int nChipIndex,
											 unsigned int nChnId,
											enum CHIP_VI_STANDARD eVideoStandard)
{
    int ret;
    VENC_H264_VIDEO_STANDARD_PARM VideoStd;

    VideoStd.chip_id = nChipIndex;
	VideoStd.chn_id = nChnId;
	VideoStd.videoStandard.eVideoType = eVideoStandard;
	VideoStd.videoStandard.u32ChipID = nChipIndex;
	VideoStd.videoStandard.u32Channel = nChnId;
    ret = isil_set_venc_video_standard_parm(&VideoStd);
    if(ret < 0)
        return ISIL_ERR_FAILURE;
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_GetVideoStandard(unsigned int nChipIndex,
    enum ISIL_VIDEO_STANDARD *lpeVideoStandard)
{
    int ret;
	CODEC_CHIP_VIDEO_STD VideoStd;

   /* 
	 VENC_H264_VIDEO_STANDARD_PARM VideoStd;

    VideoStd.chip_id = nChipIndex;
    
    ret = isil_set_venc_video_standard_parm(&VideoStd);
    if(ret < 0)
        return ISIL_ERR_FAILURE;
    */
	VideoStd.change_mask_flag = -1;
	ret = ISIL_CODEC_CTL_GetVideoSTD(nChipIndex, &VideoStd);
	if(ret < 0){
		fprintf(stderr, "ISIL_CODEC_CTL_GetVideoSTD err\n");
		return ISIL_ERR_FAILURE;
	}
	*lpeVideoStandard = VideoStd.e_video_standard;
	fprintf(stderr, "%s std %d\n", __FUNCTION__, *lpeVideoStandard);
    return ISIL_ERR_SUCCESS;
}



/*ChipSDK MV*/
ISILERR_CODE  ISIL_MediaSDK_StartMVInfo(unsigned int nChipId, unsigned int nChannel)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_StopMVInfo(unsigned int nChipId, unsigned int nChannel)
{
    return ISIL_ERR_SUCCESS;
}

/*ChipSDK MV Flag*/
ISILERR_CODE  ISIL_MediaSDK_StartMVFlag(unsigned int nChipId, unsigned int nChannel)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_StopMVFlag(unsigned int nChipId, unsigned int nChannel)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetNoMVFlagScope(unsigned int nChipId, unsigned int nChannel,
    ISIL_MediaSDK_RECT *pRectList,int nRectSize)
{
    return ISIL_ERR_SUCCESS;
}

/*
*ppPicBuf: the memeroy is managed by SDK
*nPicType: 0-bmp,1-jpg
*/
ISILERR_CODE  ISIL_MediaSDK_PreviewCapPicture(unsigned int nChipId, unsigned int nChannel,
    void **ppPicBuf,long lPicSize,int nPicType)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_StartCapPreview(unsigned int nChipId, unsigned int nChannel,
    ISIL_VIDEO_SIZE_TYPE eVideoSize,int nFrameRate)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_StopCapPreview(unsigned int nChipId, unsigned int nChannel)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_GetPreviewData(unsigned int nChipId, unsigned int nChannel,
    ISIL_PREVIEW_DATA *pData)
{
    return ISIL_ERR_SUCCESS;
}

/*ChipSDK OSD*/
ISILERR_CODE  ISIL_MediaSDK_SetOsd(unsigned int nChipId, unsigned int nChannel,
    unsigned int bEnable)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetLogo(unsigned int nChipId, unsigned int nChannel,
    int x,int y,int w,int h,unsigned char *yuv)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_StopLogo(unsigned int nChipId, unsigned int nChannel)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetOsdDisplayMode(unsigned int nChipId, unsigned int nChannel,
    int nBrightness,unsigned int  bTranslucent,int parameter,
    unsigned int  *Format1,unsigned int *Format2)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetLogoDisplayMode(unsigned int nChipId, unsigned int nChannel,
    COLORREF ColorKey,unsigned int bTranslucent,int nTwinkleInterval)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetOsdDisplayModeEx(unsigned int nChipId, unsigned int nChannel,
    int color,unsigned int Translucent,int param,int nLineCount,unsigned int **Format)
{
    return ISIL_ERR_SUCCESS;
}

#if 0
ISILERR_CODE  ISIL_MediaSDK_LoadFont(char *pFontFile,int nFontSize,HANDLE &hFont);
ISILERR_CODE  ISIL_MediaSDK_ReleaseFont(HANDLE hFont);
//when hFont == NULL,it will be selected default font
ISILERR_CODE  ISIL_MediaSDK_SelectFont(ISIL_VIDEO_SIZE_TYPE eSize,HANDLE hFont=NULL);
ISILERR_CODE  ISIL_MediaSDK_SelectFont2(HANDLE hChannel,HANDLE hFont=NULL);
#endif

/*ChipSDK MJPEG*/
ISILERR_CODE  ISIL_MediaSDK_StartMJpegCapture(unsigned int nChipId, unsigned int nChannel)
{
    int ret;
    VENC_MJPEG_CFG_PARM mjpeg_cfg;

    mjpeg_cfg.chip_id = nChipId;
    mjpeg_cfg.chn = nChannel;
    mjpeg_cfg.streamType = IS_MJPEG_STREAM;
    mjpeg_cfg.cfg.i_capture_type = ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER;

    ret = isil_venc_mjpeg_startcap(&mjpeg_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;
    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_StopMJpegCapture(unsigned int nChipId, unsigned int nChannel)
{
    int ret;
    VENC_MJPEG_CFG_PARM mjpeg_cfg;

    mjpeg_cfg.chip_id = nChipId;
    mjpeg_cfg.chn = nChannel;
    mjpeg_cfg.streamType = IS_MJPEG_STREAM;
    mjpeg_cfg.cfg.i_capture_type = ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER;

    ret = isil_venc_mjpeg_stopcap(&mjpeg_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;
    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_SetMJpegFormat(unsigned int nChipId, unsigned int nChannel,
    ISIL_VIDEO_SIZE_TYPE ePicSize)
{
    int ret;
    VENC_MJPEG_CFG_PARM mjpeg_cfg;
	enum ISIL_VIDEO_STANDARD chipstd;
   ISIL_MediaSDK_GetVideoStandard(nChipId, &chipstd);

    mjpeg_cfg.chip_id = nChipId;
    mjpeg_cfg.chn = nChannel;
    mjpeg_cfg.streamType = IS_MJPEG_STREAM;
    switch(ePicSize) {
        case ISIL_VIDEO_SIZE_CIF:{
			if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_CIF_PAL >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_CIF_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_CIF_NTSC >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_CIF_NTSC >>4;
            }
		}
            break;

        case ISIL_VIDEO_SIZE_D1:{
			if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_D1_PAL >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_D1_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_D1_NTSC >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_D1_NTSC >>4;
            }
		}
		break;

		case ISIL_VIDEO_SIZE_4CIF:{
			if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_4CIF_PAL >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_4CIF_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_4CIF_NTSC >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_4CIF_NTSC >>4;
            }
		}
		break;
        case ISIL_VIDEO_SIZE_QCIF:{
			if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_QCIF_PAL >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_QCIF_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_QCIF_NTSC >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_QCIF_NTSC >>4;
            }
		}
        break;
        case ISIL_VIDEO_SIZE_HALF_D1:
        case ISIL_VIDEO_SIZE_2CIF:{
			if(chipstd == ISIL_VIDEO_STANDARD_PAL)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_HALF_D1_PAL >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_HALF_D1_PAL >>4;
            }
            else if(chipstd == ISIL_VIDEO_STANDARD_NTSC)
            {
                mjpeg_cfg.cfg.i_image_width_mb_size = WIDTH_FRAME_HALF_D1_NTSC >> 4;
                mjpeg_cfg.cfg.i_image_height_mb_size = HEIGHT_FRAME_HALF_D1_NTSC >>4;
            }
		}
        break;

        default:
        break;
    }
    mjpeg_cfg.cfg.change_mask_flag = ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_WIDTH_MASK |
        ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_HEIGHT_MASK;
    ret = isil_set_venc_mjpeg_cfg_parm(&mjpeg_cfg);

    if(ret < 0)
        return ISIL_ERR_FAILURE;
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetMJpegFrameRate(unsigned int nChipId, unsigned int nChannel,
    int nFps)
{
    int ret;
    VENC_MJPEG_CFG_PARM mjpeg_cfg;

    mjpeg_cfg.chip_id = nChipId;
    mjpeg_cfg.chn = nChannel;
    mjpeg_cfg.streamType = IS_MJPEG_STREAM;
    mjpeg_cfg.cfg.i_capture_frame_number = 1;
    if(nFps > 0)
        mjpeg_cfg.cfg.i_capture_frame_stride = 1000/nFps;
    else
        mjpeg_cfg.cfg.i_capture_frame_stride = 1000;

    mjpeg_cfg.cfg.change_mask_flag =
        ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_NUMBER_MASK |
        ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_STRIDE_MASK;

    ret = isil_set_venc_mjpeg_cfg_parm(&mjpeg_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetMJpegQP(unsigned int nChipId, unsigned int nChannel, int nQP)      //nQP: good to bad[10,51]
{
    int ret;
    VENC_MJPEG_CFG_PARM mjpeg_cfg;

    mjpeg_cfg.chip_id = nChipId;
    mjpeg_cfg.chn = nChannel;
    mjpeg_cfg.streamType = IS_MJPEG_STREAM;
    mjpeg_cfg.cfg.e_image_level = nQP;

    mjpeg_cfg.cfg.change_mask_flag =
        ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_LEVEL_MASK;

    ret = isil_set_venc_mjpeg_cfg_parm(&mjpeg_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetMJpegCfgParm(unsigned int nChipId, unsigned int nChannel,
    VENC_MJPG_CFG* cfg)
{
    int ret;
    VENC_MJPEG_CFG_PARM mjpeg_cfg;

    mjpeg_cfg.chip_id = nChipId;
    mjpeg_cfg.chn = nChannel;
    mjpeg_cfg.streamType = IS_MJPEG_STREAM;
    memcpy(&mjpeg_cfg.cfg, cfg, sizeof(VENC_MJPG_CFG));

    mjpeg_cfg.cfg.change_mask_flag = -1;

    ret = isil_set_venc_mjpeg_cfg_parm(&mjpeg_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}



ISILERR_CODE  ISIL_MediaSDK_SetAudioType(unsigned int nChipId, unsigned int nChannel,
    AUDIO_TYPE_E eAudioType)
{
    int ret;
    VENC_AUDIO_CFG_PARM audioCfg;

    memset(&audioCfg, 0x00, sizeof(VENC_AUDIO_CFG_PARM));

    audioCfg.chip_id = nChipId;
    audioCfg.chn = nChannel;
    audioCfg.streamType = IS_AUDIO_STREAM;
    audioCfg.cfg.channel = nChannel;
    audioCfg.cfg.e_audio_type = eAudioType;
    audioCfg.cfg.change_mask_flag =
        ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_ENCODE_TYPE_MASK;
    fprintf(stderr,"%s channel %d, audio type %d\n", __FUNCTION__, nChannel, eAudioType);
    ret = isil_set_venc_audio_cfg_parm(&audioCfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetAudioParm(unsigned int nChipId, unsigned int nChannel,
    unsigned int bit_wide, unsigned int sample_rate)
{
    int ret;
    VENC_AUDIO_CFG_PARM audioCfg;

    memset(&audioCfg, 0x00, sizeof(VENC_AUDIO_CFG_PARM));

    audioCfg.chip_id = nChipId;
    audioCfg.chn = nChannel;
    audioCfg.streamType = IS_AUDIO_STREAM;
    audioCfg.cfg.channel = nChannel;
    audioCfg.cfg.i_bit_wide = bit_wide;
    audioCfg.cfg.i_sample_rate = sample_rate;
    audioCfg.cfg.change_mask_flag =
        ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_BIT_WIDE_MASK |
        ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_SAMPLE_RATE_MASK;
    fprintf(stderr,"%s channel %d, bit_wide %d, sample_rate %d\n", __FUNCTION__,
        nChannel, bit_wide, sample_rate);
    ret = isil_set_venc_audio_cfg_parm(&audioCfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_SetVideoFlip(unsigned int nChipId, unsigned int nChannel,
    unsigned int bHFlip,unsigned int bVFlip)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetVideoPara(unsigned int nChipId, unsigned int nChannel,
    int  nBrightness, int  nContrast, int  nSaturation, int  nHue, int nSharpness)
{
    int ret;
    VENC_H264_VI_CFG_PARM ViCfg;


    memset(&ViCfg, 0x00, sizeof(VENC_H264_VI_CFG_PARM));

    ViCfg.chip_id = nChipId;
    ViCfg.chn = nChannel;
    ViCfg.streamType = IS_H264_MAIN_STREAM;

    ViCfg.cfg.u32ChipID = nChipId;
    ViCfg.cfg.u32Channel = nChannel;
    ViCfg.cfg.u32Brightness = nBrightness;
    ViCfg.cfg.u32Contrast = nContrast;
    ViCfg.cfg.u32Saturation = nSaturation;
    ViCfg.cfg.u32Hue = nHue;
    ViCfg.cfg.u32Mask = MASK_VI_ALL;
    fprintf(stderr,"%s channel %d, nBrightness %d nContrast %d nSaturation %d nHue %d\n",
        __FUNCTION__, nChannel, nBrightness,nContrast,nSaturation, nHue);
    ret = isil_set_venc_vi_cfg_parm(&ViCfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_GetVideoPara(unsigned int nChipId, unsigned int nChannel,
    int *lpBrightness,int *lpContrast,int *lpSaturation,int *lpHue, int *lpSharpness)
{
    /*
    int ret;
    VI_CH_CFG ViCfg;

    memset(&ViCfg, 0x00, sizeof(VI_CH_CFG));

    ret = ISIL_VI_GetCfg(nChipId, nChannel, &ViCfg);
    if(ret < 0)
       return ISIL_ERR_FAILURE;
    *lpBrightness = ViCfg.u32Brightness;
    *lpContrast = ViCfg.u32Contrast;
    *lpSaturation = ViCfg.u32Saturation;
    *lpHue = ViCfg.u32Hue;
    */
    return ISIL_ERR_SUCCESS;
}


/*
*if eType==ISIL_STREAM_BITRATE_TYPE_CBR, tValue invalid£¬
    call ISIL_MediaSDK_SetupBitrateControl to set BPS
*if eType==ISIL_STREAM_BITRATE_TYPE_VBR, tValue.lImageQuality£ºbad to good[0,5]£¬
    call ISIL_MediaSDK_SetupBitrateControl to set BPS
*if eType==ISIL_STREAM_BITRATE_TYPE_CQP, tValue.tQP£º   cIQP,cPQP,cBQP [0,51]
*/
ISILERR_CODE  ISIL_MediaSDK_SetBitrateControlType(unsigned int nChipId, unsigned int nChannel,
    ISIL_STREAM_BITRATE_TYPE eType, ISIL_BITRATE_VALUE tValue, unsigned int bSub)
{
    int ret;
    VENC_H264_RC_PARM rcCfg;


    memset(&rcCfg, 0x00, sizeof(VENC_H264_RC_PARM));

    rcCfg.chip_id = nChipId;
    rcCfg.chn = nChannel;
    rcCfg.streamType = (enum ECHANFUNCPRO)bSub;
    rcCfg.cfg.e_rc_type = eType;
    rcCfg.cfg.change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK;

    rcCfg.cfg.i_qpb = tValue.tQP.cBQP;
    rcCfg.cfg.change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK;

    rcCfg.cfg.i_qpi = tValue.tQP.cIQP;
    rcCfg.cfg.change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK;

    rcCfg.cfg.i_qpp = tValue.tQP.cPQP;
    rcCfg.cfg.change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK;
    //rcCfg.cfg.i_default_qp = (int)tValue;


    fprintf(stderr,"%s channel %d, streamType %d, e_rc_type %d, qpb: %d,qpi: %d,qpp: %d\n",
    		__FUNCTION__, nChannel, bSub, eType, rcCfg.cfg.i_qpb, rcCfg.cfg.i_qpi, rcCfg.cfg.i_qpp);
    ret = isil_set_venc_h264_rc_parm(&rcCfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_SetBitrateControlMode(unsigned int nChipId, unsigned int nChannel,
    ISIL_STREAM_BITRATE_MODE eMode, unsigned int bSub)
{
    int ret;
    VENC_H264_RC_PARM rcCfg;


    memset(&rcCfg, 0x00, sizeof(VENC_H264_RC_PARM));

    rcCfg.chip_id = nChipId;
    rcCfg.chn = nChannel;
    rcCfg.streamType = (enum ECHANFUNCPRO)bSub;
    rcCfg.cfg.e_image_priority = eMode;
    rcCfg.cfg.change_mask_flag = ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK;

    fprintf(stderr,"%s channel %d, streamType %d, e_image_priority %d\n",
        __FUNCTION__, nChannel, bSub, eMode);
    ret = isil_set_venc_h264_rc_parm(&rcCfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_SetupBitrateControl(unsigned int nChipId, unsigned int nChannel,
    unsigned long lMaxBps, unsigned int bSub)
{
	int ret;
    VENC_H264_CFG_PARM venc_h264_cfg;

    memset(&venc_h264_cfg, 0x00, sizeof(VENC_H264_CFG_PARM));
    venc_h264_cfg.chip_id = nChipId;
    venc_h264_cfg.chn = nChannel;
    venc_h264_cfg.streamType = (enum ECHANFUNCPRO)bSub;
	venc_h264_cfg.cfg.i_bps = lMaxBps;
    venc_h264_cfg.cfg.change_mask_flag = ISIL_h264_ENCODER_PARAM_BPS_MASK;
    ret = isil_set_venc_h264_cfg_parm(&venc_h264_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;

}

ISILERR_CODE  ISIL_MediaSDK_ForceIFrameEncode(unsigned int nChipId, unsigned int nChannel,
    unsigned int bSub)
{
    int ret;
    VENC_H264_CFG_PARM venc_h264_cfg;

    memset(&venc_h264_cfg, 0x00, sizeof(VENC_H264_CFG_PARM));
    venc_h264_cfg.chip_id = nChipId;
    venc_h264_cfg.chn = nChannel;
    venc_h264_cfg.streamType = (enum ECHANFUNCPRO)bSub;
    venc_h264_cfg.cfg.b_force_I_flag = 1;
    venc_h264_cfg.cfg.change_mask_flag = ISIL_h264_ENCODER_PARAM_FORCE_I_MASK;
    ret = isil_set_venc_h264_cfg_parm(&venc_h264_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetIBPMode(unsigned int nChipId, unsigned int nChannel,
    int nKeyFrameIntervals,int nBFrames,int nPFrames,
    int nFrameRate, unsigned int bSub)
{
    int ret;
    VENC_H264_CFG_PARM venc_h264_cfg;

    memset(&venc_h264_cfg, 0x00, sizeof(VENC_H264_CFG_PARM));
    venc_h264_cfg.chip_id = nChipId;
    venc_h264_cfg.chn = nChannel;
    venc_h264_cfg.streamType = (enum ECHANFUNCPRO)bSub;
    venc_h264_cfg.cfg.i_fps = nFrameRate;
    venc_h264_cfg.cfg.i_I_P_stride = nKeyFrameIntervals;
    venc_h264_cfg.cfg.change_mask_flag = ISIL_h264_ENCODER_PARAM_IP_STRIDE_MASK |
                                         ISIL_h264_ENCODER_PARAM_FPS_MASK;

    ret = isil_set_venc_h264_cfg_parm(&venc_h264_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_SetEncoderVideoSize(unsigned int nChipId, unsigned int nChannel,
    ISIL_VIDEO_SIZE_TYPE eVideoSize, unsigned int bSub)
{

	int ret;
    VENC_H264_CFG_PARM venc_h264_cfg;
	enum ISIL_VIDEO_STANDARD VideoStandard;
    memset(&venc_h264_cfg, 0x00, sizeof(VENC_H264_CFG_PARM));
    venc_h264_cfg.chip_id = nChipId;
    venc_h264_cfg.chn = nChannel;
    venc_h264_cfg.streamType = (enum ECHANFUNCPRO)bSub;
    
	ISIL_MediaSDK_GetVideoStandard(nChipId,&VideoStandard);
    switch(eVideoSize) {
        case ISIL_VIDEO_SIZE_CIF_E:{
			if(VideoStandard == ISIL_VIDEO_STANDARD_PAL)
			{
				venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_CIF_PAL >> 4;
				venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_CIF_PAL >>4;
			}
			else if(VideoStandard == ISIL_VIDEO_STANDARD_NTSC)
			{
				venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_CIF_NTSC >> 4;
				venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_CIF_NTSC >>4;
			}
		}
        break;

		case ISIL_VIDEO_SIZE_D1_E:{
			if(VideoStandard == ISIL_VIDEO_STANDARD_PAL)
			{
				venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_D1_PAL >> 4;
				venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_D1_PAL >>4;
			}
			else if(VideoStandard == ISIL_VIDEO_STANDARD_NTSC)
			{
				venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_D1_NTSC >> 4;
				venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_D1_NTSC >>4;
			}
		}
		break;
        case ISIL_VIDEO_SIZE_4CIF_E:{
		   if(VideoStandard == ISIL_VIDEO_STANDARD_PAL)
			{
				venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_4CIF_PAL >> 4;
				venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_4CIF_PAL >>4;
			}
			else if(VideoStandard == ISIL_VIDEO_STANDARD_NTSC)
			{
				venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_4CIF_NTSC >> 4;
				venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_4CIF_NTSC >>4;
			}
		}
		break;
        case ISIL_VIDEO_SIZE_QCIF_E:{
		   if(VideoStandard == ISIL_VIDEO_STANDARD_PAL)
			{
				venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_QCIF_PAL >> 4;
				venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_QCIF_PAL >>4;
			}
			else if(VideoStandard == ISIL_VIDEO_STANDARD_NTSC)
			{
				venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_QCIF_NTSC >> 4;
				venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_QCIF_NTSC >>4;
			}
		}
        break;
        case ISIL_VIDEO_SIZE_HALF_D1_E:
        case ISIL_VIDEO_SIZE_2CIF_E:{
				if(VideoStandard == ISIL_VIDEO_STANDARD_PAL)
				{
					venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_HALF_D1_PAL >> 4;
					venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_HALF_D1_PAL >>4;
				}
				else if(VideoStandard == ISIL_VIDEO_STANDARD_NTSC)
				{
					venc_h264_cfg.cfg.i_logic_video_width_mb_size = WIDTH_FRAME_HALF_D1_NTSC >> 4;
					venc_h264_cfg.cfg.i_logic_video_height_mb_size = HEIGHT_FRAME_HALF_D1_NTSC >>4;
				}
		}
        break;

        default:
        break;
    }
    venc_h264_cfg.cfg.change_mask_flag = ISIL_h264_ENCODER_PARAM_WIDTH_MASK |
                                         ISIL_h264_ENCODER_PARAM_HEIGHT_MASK;
    fprintf(stderr, "%s ms %d, w = %d h = %d\n", __FUNCTION__,  bSub,venc_h264_cfg.cfg.i_logic_video_width_mb_size,
    		venc_h264_cfg.cfg.i_logic_video_height_mb_size);
    ret = isil_set_venc_h264_cfg_parm(&venc_h264_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetEncoderVideoSizeByWH(unsigned int nChipId, unsigned int nChannel,
    unsigned int w, unsigned int h,unsigned int bSub)
{

	int ret;
    VENC_H264_CFG_PARM venc_h264_cfg;
    fprintf(stderr, "%s w = %d h=%d\n", __FUNCTION__, w, h);
    memset(&venc_h264_cfg, 0x00, sizeof(VENC_H264_CFG_PARM));
    venc_h264_cfg.chip_id = nChipId;
    venc_h264_cfg.chn = nChannel;
    venc_h264_cfg.streamType = (enum ECHANFUNCPRO)bSub;

    venc_h264_cfg.cfg.i_logic_video_width_mb_size = (w>>4);
    venc_h264_cfg.cfg.i_logic_video_height_mb_size= (h>>4);

    venc_h264_cfg.cfg.change_mask_flag = ISIL_h264_ENCODER_PARAM_WIDTH_MASK |
                                         ISIL_h264_ENCODER_PARAM_HEIGHT_MASK;

    ret = isil_set_venc_h264_cfg_parm(&venc_h264_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetH264CfgAll(unsigned int nChipId, unsigned int nChannel,
		VENC_H264_CFG* cfg, unsigned int bSub)
{

	int ret;
    VENC_H264_CFG_PARM venc_h264_cfg;
    memset(&venc_h264_cfg, 0x00, sizeof(VENC_H264_CFG_PARM));
    venc_h264_cfg.chip_id = nChipId;
    venc_h264_cfg.chn = nChannel;
    venc_h264_cfg.streamType = (enum ECHANFUNCPRO)bSub;
    memcpy(&venc_h264_cfg.cfg, cfg, sizeof(VENC_H264_CFG));
	venc_h264_cfg.cfg.change_mask_flag = -1;

    ret = isil_set_venc_h264_cfg_parm(&venc_h264_cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetRcAll(unsigned int nChipId, unsigned int nChannel,
		VENC_H264_RC* cfg, unsigned int bSub)
{
    int ret;
    VENC_H264_RC_PARM rcCfg;


    memset(&rcCfg, 0x00, sizeof(VENC_H264_RC_PARM));

    rcCfg.chip_id = nChipId;
    rcCfg.chn = nChannel;
    rcCfg.streamType = (enum ECHANFUNCPRO)bSub;
    
    memcpy(&rcCfg.cfg, cfg, sizeof(VENC_H264_RC));
	rcCfg.cfg.change_mask_flag = -1;
    ret = isil_set_venc_h264_rc_parm(&rcCfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_SetAudioCfgAll(unsigned int nChipId, unsigned int nChannel,
		VENC_AUD_CFG* cfg)
{
	int ret;
	VENC_AUDIO_CFG_PARM audioCfg;

	memset(&audioCfg, 0x00, sizeof(VENC_AUDIO_CFG_PARM));

	audioCfg.chip_id = nChipId;
	audioCfg.chn = nChannel;
	audioCfg.streamType = IS_AUDIO_STREAM;
	memcpy(&audioCfg.cfg, cfg, sizeof(VENC_AUD_CFG));
	audioCfg.cfg.change_mask_flag = -1;
	ret = isil_set_venc_audio_cfg_parm(&audioCfg);
	if(ret < 0)
		return ISIL_ERR_FAILURE;

	return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_SetH264Feature(unsigned int nChipId, unsigned int nChannel,
    VENC_H264_FEATURE* cfg,  unsigned int bSub)
{

	int ret;
    VENC_H264_FEATURE_PARM venc_h264_feature;
    venc_h264_feature.chip_id = nChipId;
    venc_h264_feature.chn = nChannel;
    venc_h264_feature.streamType = (enum ECHANFUNCPRO)bSub;


    memcpy(&venc_h264_feature.cfg, cfg, sizeof(VENC_H264_FEATURE));

    venc_h264_feature.cfg.change_mask_flag = -1;


    /*fprintf(stderr, "----%d, %d, %d, %d, %d, %d, %d-------\n", venc_h264_feature.cfg.b_enable_I_4X4, venc_h264_feature.cfg.b_enable_deinterlace,
    		venc_h264_feature.cfg.b_enable_half_pixel, venc_h264_feature.cfg.b_enable_quarter_pixel, venc_h264_feature.cfg.b_enable_skip,
    		venc_h264_feature.cfg.change_mask_flag, venc_h264_feature.cfg.i_mb_delay_value);
	*/
    ret = isil_set_venc_h264_feature_parm(&venc_h264_feature);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetVdCfgRealtime(VENC_VD_CONFIG_REALTIME cfg)
{
    int ret;
    ret = isil_venc_set_vd_config_realtime(&cfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}


/*ChipSDK special*/
ISILERR_CODE  ISIL_MediaSDK_ReadReg(unsigned int nChipIndex, unsigned long lStartAddr,
    unsigned long *lpBuffer, unsigned long lCount)
{
	int ret;
	struct reg_info info;
	if(NULL == lpBuffer)
	{
		fprintf(stderr,"%s buff is null\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	info.startaddr = lStartAddr;
	info.count = lCount;
	info.buffer = lpBuffer;
	ret = isil_chip_read_reg(nChipIndex, &info);
	if(ret < 0)
	{
		fprintf(stderr,"%s fail\n", __FUNCTION__);
	    return ISIL_ERR_FAILURE;
	}
    return ISIL_ERR_SUCCESS;
}
ISILERR_CODE  ISIL_MediaSDK_WriteReg(unsigned int nChipIndex, unsigned long lStartAddr,
    unsigned long *lpBuffer, unsigned long lCount)
{

	int ret;
	struct reg_info info;
	if(NULL == lpBuffer)
	{
		fprintf(stderr,"%s buff is null\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	info.startaddr = lStartAddr;
	info.count = lCount;
	info.buffer = lpBuffer;
	ret = isil_chip_write_reg(nChipIndex, &info);
	if(ret < 0)
	{
		fprintf(stderr,"%s fail\n", __FUNCTION__);
	    return ISIL_ERR_FAILURE;
	}
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_ReadMPB(unsigned int nChipIndex, unsigned long lStartAddr,
    unsigned char *lpBuffer, unsigned long lCount)
{
	int ret;
	struct mpb_info info;
	if(NULL == lpBuffer)
	{
		fprintf(stderr,"%s buff is null\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	info.startaddr = lStartAddr;
	info.count = lCount;
	info.buffer = lpBuffer;
	ret = isil_chip_read_mpb(nChipIndex, &info);
	if(ret < 0)
	{
		fprintf(stderr,"%s fail\n", __FUNCTION__);
	    return ISIL_ERR_FAILURE;
	}
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_WriteMPB(unsigned int nChipIndex, unsigned long lStartAddr,
    unsigned char *lpBuffer, unsigned long lCount)
{

	int ret;
	struct mpb_info info;
	if(NULL == lpBuffer)
	{
		fprintf(stderr,"%s buff is null\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	info.startaddr = lStartAddr;
	info.count = lCount;
	info.buffer = lpBuffer;
	ret = isil_chip_write_mpb(nChipIndex, &info);
	if(ret < 0)
	{
		fprintf(stderr,"%s fail\n", __FUNCTION__);
	    return ISIL_ERR_FAILURE;
	}
    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_ReadI2C(unsigned int nChipIndex, unsigned long lBusAddr ,
    unsigned long lOffsetAddr , unsigned char *pValue, unsigned long lCount,unsigned char GPIOorIIC)
{
	int ret;
	struct i2c_info info;

	if(NULL == pValue)
	{
		fprintf(stderr,"%s value null\n", __FUNCTION__);
	    return ISIL_ERR_FAILURE;
	}
	info.busaddr = lBusAddr;
	info.offset = lOffsetAddr;
	info.count = lCount;
	info.buffer = pValue;
	ret = isil_chip_read_i2c(nChipIndex, &info);
	if(ret < 0)
	{
		fprintf(stderr,"%s fail\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_WriteI2C(unsigned int nChipIndex, unsigned long lBusAddr ,
    unsigned long lOffsetAddr , unsigned char *pValue,
    unsigned long lCount,unsigned char GPIOorIIC)
{
	int ret;
	struct i2c_info info;

	if(NULL == pValue)
	{
		fprintf(stderr,"%s value null\n", __FUNCTION__);
	    return ISIL_ERR_FAILURE;
	}
	info.busaddr = lBusAddr;
	info.offset = lOffsetAddr;
	info.count = lCount;
	info.buffer = pValue;
	ret = isil_chip_write_i2c(nChipIndex, &info);
	if(ret < 0)
	{
		fprintf(stderr,"%s fail\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_ReadDDR(unsigned int nChipIndex, unsigned long lStartAddr,
    unsigned long *lpBuffer, unsigned long lCount)
{
	int ret;
	struct ddr_info info;
	if(NULL == lpBuffer)
	{
		fprintf(stderr,"%s buff is null\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	info.startaddr = lStartAddr;
	info.count = lCount;
	info.buffer = lpBuffer;
	ret = isil_chip_read_ddr(nChipIndex, &info);
	if(ret < 0)
	{
		fprintf(stderr,"%s fail\n", __FUNCTION__);
	    return ISIL_ERR_FAILURE;
	}
    return ISIL_ERR_SUCCESS;
}
ISILERR_CODE  ISIL_MediaSDK_WriteDDR(unsigned int nChipIndex, unsigned long lStartAddr,
    unsigned long *lpBuffer, unsigned long lCount)
{
	int ret;
	struct ddr_info info;
	if(NULL == lpBuffer)
	{
		fprintf(stderr,"%s buff is null\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	info.startaddr = lStartAddr;
	info.count = lCount;
	info.buffer = lpBuffer;
	ret = isil_chip_write_ddr(nChipIndex, &info);
	if(ret < 0)
	{
		fprintf(stderr,"%s fail\n", __FUNCTION__);
	    return ISIL_ERR_FAILURE;
	}
    return ISIL_ERR_SUCCESS;
}
//ISILERR_CODE  ISIL_MediaSDK_GPIO(unsigned int nChipIndex, ISIL_ACCESS_GPIO *pGPIO);
ISILERR_CODE  ISIL_MediaSDK_GPIO_Read(unsigned int nChipIndex, ISIL_ACCESS_GPIO *pGPIO)
{
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_GPIO_Write(unsigned int nChipIndex, ISIL_ACCESS_GPIO *pGPIO)
{
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetPreviewMultiParam(ISIL_MediaSDK_PREVIEW_PARAM *previewParm,
    int dwCount, void *pVoid)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetEncodeMultiParam(ISIL_MediaSDK_ENCODE_PARAM *encodeParm,
    int dwCount)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetSubEncodeMultiParam(ISIL_MediaSDK_ENCODE_PARAM* encodeParm,
    int dwCount)
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetOsdCfg(unsigned int nChipId, unsigned int nChannel,
		ISIL_OSD_CFG* cfg,  unsigned int bSub)
{

	int ret;
	ISIL_OSD_CFG_PARM osdcfg;

	if(NULL == cfg)
	{
		fprintf(stderr, "%s cfg null\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}

	osdcfg.chip_id = nChipId;
	osdcfg.chn = nChannel;
	osdcfg.streamType = (enum ECHANFUNCPRO)bSub;
    memcpy(&osdcfg.cfg, cfg, sizeof(ISIL_OSD_CFG));
    ret = isil_set_osd_cfg(&osdcfg);
    if(ret < 0)
        return ISIL_ERR_FAILURE;

    return ISIL_ERR_SUCCESS;
}
ISILERR_CODE ISIL_MediaSDK_ClacSetVd(unsigned int nChipIndex, VD_CONFIG_REALTIME* pRealtime)
{
	int ret;
	if(NULL == pRealtime)
	{
		fprintf(stderr, "%s parm is null\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	ret = isil_clac_set_vd(nChipIndex, pRealtime);
	if(ret < 0)
	{
		fprintf(stderr, "%s set cfg fail\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE ISIL_MediaSDK_SetChannelAlarmType(unsigned int nChipIndex, unsigned int ch, unsigned int alarmType)
{
	int ret;
	fprintf(stderr, "%s chipid %d chnid %d\n", __FUNCTION__, nChipIndex, ch);
	ret = ISIL_VI_StartAlarmType(nChipIndex, ch, alarmType);
	if(ret < 0)
	{
		fprintf(stderr, "%s set alarm type fail\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE ISIL_MediaSDK_ClearChannelAlarmType(unsigned int nChipIndex, unsigned int ch, unsigned int alarmType)
{
	int ret;
	ret = ISIL_VI_StopAlarmType(nChipIndex, ch, alarmType);
	if(ret < 0)
	{
		fprintf(stderr, "%s clear alarm type fail\n", __FUNCTION__);
		return ISIL_ERR_FAILURE;
	}
	return ISIL_ERR_SUCCESS;
}


int get_chip_chn_id_by_logic_id(unsigned int log_id , CHIP_CHN_PHY_ID *phy_id_ptr)
{
    unsigned int chip_id ,chan_id,enc_total_num;

	fprintf(stderr,"log_id[%d].\n",log_id);

	enc_total_num = get_total_enc_count();
	fprintf(stderr,"enc_total_num[%d].\n",enc_total_num);

	if(enc_total_num == 0){
		return -1;
	}
    if(log_id >= enc_total_num){
        return -1;
    }

    if( !phy_id_ptr ) {
        return -1;
    
    }

    chip_id = log_id/MAX_ENC_NUM_PER_CHIP;
    chan_id = log_id%MAX_ENC_NUM_PER_CHIP;
	
	fprintf(stderr,"log_id[%d].\n",log_id);
    
    phy_id_ptr->chipid = chip_id;
    phy_id_ptr->chanid  = chan_id;

	fprintf(stderr,"chip_id[%d],chan_id[%d].\n",chip_id,chan_id);
    
    return 0;
}


int get_logic_id_by_chip_chn_id(unsigned int *log_id , CHIP_CHN_PHY_ID *phy_id_ptr)
{
    unsigned int chip_id ,chan_id ;
	int tmp_log_id;

    if( !phy_id_ptr || !log_id) {
        return -1;
    }

    chip_id = phy_id_ptr->chipid;
    chan_id = phy_id_ptr->chanid;
    
    tmp_log_id = (chip_id * MAX_ENC_NUM_PER_CHIP + chan_id);
    
    if(tmp_log_id >= get_total_enc_count() ) {
        return -1;
    }

    
    *log_id = tmp_log_id;
    
    return 0;
}



int dec_get_chip_chn_id_by_logic_id(unsigned int log_id , CHIP_CHN_PHY_ID *phy_id_ptr)
{
    unsigned int chip_id ,chan_id,dec_total_num,dec_chan = 0;
	int total_chip_num,i;

	S_SINGLE_CHIP_MGT * chip_mgt;
	
	

	fprintf(stderr,"log_id[%d].\n",log_id);

	total_chip_num = isil_get_chips_count();

	if( total_chip_num == 0 ){
		fprintf( stderr,"isil_get_chips_count is zero .\n");
		return -1;
	}

	dec_total_num = get_total_dec_count();
	fprintf(stderr,"dec_total_num[%d].\n",dec_total_num);

	if(dec_total_num == 0){
		return -1;
	}
    if(log_id >= dec_total_num){
        return -1;
    }

    if( !phy_id_ptr ) {
        return -1;
    
    }

	for( i = 0 ; i < total_chip_num ; i++ ){

		chip_mgt = get_single_chip_mgt_by_id((unsigned int)i);
		if(!chip_mgt) {
			fprintf( stderr,"get_single_chip_mgt_by_id err.\n");
			return -1;
		}

		if(chip_mgt->chip_pro & CHIP_CAN_DEC ){
			dec_chan += chip_mgt->dec_max_chan_num;
			if(log_id >= dec_chan){
				continue;
			}
			else{
				phy_id_ptr->chanid = log_id%MAX_ENC_NUM_PER_CHIP;
				phy_id_ptr->chipid = chip_mgt->cur_chip_id;
				return 0;
			}
		}


	}
	
    return -1;
}


int dec_get_logic_id_by_chip_chn_id(unsigned int *log_id , CHIP_CHN_PHY_ID *phy_id_ptr)
{
    unsigned int dec_total_num ,dec_chan = 0;
	int total_chip_num,i;
	S_SINGLE_CHIP_MGT * chip_mgt;

    if( !phy_id_ptr || !log_id) {
        return -1;
    }

	total_chip_num = isil_get_chips_count();

	if( total_chip_num == 0 ){
		fprintf( stderr,"isil_get_chips_count is zero .\n");
		return -1;
	}

	dec_total_num = get_total_dec_count();
	fprintf(stderr,"dec_total_num[%d].\n",dec_total_num);

	if( dec_total_num == 0){
		return -1;
	}

	for( i = 0 ; i < total_chip_num ; i++ ){

		chip_mgt = get_single_chip_mgt_by_id((unsigned int)i);
		if(!chip_mgt) {
			fprintf( stderr,"get_single_chip_mgt_by_id err.\n");
			return -1;
		}
		
		if(chip_mgt->cur_chip_id == phy_id_ptr->chipid){
			
			if(!(chip_mgt->cur_chip_id & CHIP_CAN_DEC) ){
				return -1;
			}

			if( phy_id_ptr->chanid >= chip_mgt->dec_max_chan_num ){
				return -1;
			}
			
			*log_id = (phy_id_ptr->chanid + dec_chan);

			return 0;

		}

		if(chip_mgt->chip_pro & CHIP_CAN_DEC ){
			dec_chan += chip_mgt->dec_max_chan_num;

		}


	}
    
    return -1;
}


