#include "isil_stream_manage.h"
#include "isil_messagestruct.h"
#include "isil_cfg_file_manage.h"
#include "isil_debug.h"
#include "isil_video_manage.h"
#include "isil_endian_trans.h"
#include "default_cfg_value.h"

static U08 EncodeDefaultCfg[CHANNEL_NUMBER]; //= {1, 1, 1, 1,1, 1, 1, 1,1, 1, 1, 1,1, 1, 1, 1};
static U08 H264FeatureDefaultCfg[CHANNEL_NUMBER]; //= {1, 1, 1, 1,1, 1, 1, 1,1, 1, 1, 1,1, 1, 1, 1};
S32 ISIL_SaveStreamCfg(G_STREAM_CFG* pCfg);
static void ISIL_SetDefaultWinEncodeCfg(ISIL_ENCODE_CFG_S* cfg);
static void ISIL_SetDefaultWinH264FeatureCfg(VENC_H264_FEATURE* cfg);
static void ISIL_SetDefaultWinMjpeg(ISIL_MJPEG_CFG* cfg);

void ISIL_SaveChannelInfo(S32 channelNum, S08 *channelName)
{
    return;
}
void ISIL_GetChannelInfo(S32 channelNum, S08 *pChannelName)
{
    S08 channelName[ISIL_NAME_LEN];
    if(NULL == channelName)
    {
        return;
    }
    sprintf(channelName, "Channel%d", channelNum);
    strncpy(pChannelName, channelName, ISIL_NAME_LEN);
    return;
}

S32 ISIL_GetStreamCfg(G_STREAM_CFG* pCfg)
{
	S32 iRet;
	S32 iType;
	S08 sPath[ISIL_CFG_PATH_LEN];
	if(NULL == pCfg)
	{
		return ISIL_FAIL;
	}
    if(pCfg->channel < 0 || pCfg->channel > CHANNEL_NUMBER)
        return ISIL_FAIL;

	sprintf(sPath, "%sch%d", ISIL_STREAM_CFG_FILE, pCfg->channel);

	iRet = ISIL_ReadConfigFile(sPath, 0, sizeof(G_STREAM_CFG),
		&iType, pCfg);
	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
    if(EncodeDefaultCfg[pCfg->channel] == 0)/*power on*/
    {
        ISIL_SetDefaultWinEncodeCfg(&pCfg->sStreamCfg.stStreamEncode);
        ISIL_SetDefaultWinMjpeg(&pCfg->sStreamCfg.stMJPEGCfg);
        ISIL_SaveStreamCfg(pCfg);
        EncodeDefaultCfg[pCfg->channel] = 1;
    }
	return ISIL_SUCCESS;
}
S32 ISIL_SaveStreamCfg(G_STREAM_CFG* pCfg)
{
	S32 iRet;
	S32 iType;
	S08 sPath[ISIL_CFG_PATH_LEN];


	if(NULL == pCfg)
	{
		return ISIL_FAIL;
	}
	sprintf(sPath, "%sch%d", ISIL_STREAM_CFG_FILE, pCfg->channel);

	iRet = ISIL_WriteConfigFile(sPath, 0, sizeof(G_STREAM_CFG),
		&iType, pCfg);
	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
	return ISIL_SUCCESS;
}



S32 ISIL_GetH264Cfg(S32 ch, void* pCfg, S32 len)
{
	S32 iRet;
	S32 iType;
	S08 sPath[ISIL_CFG_PATH_LEN];
    VENC_H264_FEATURE* tmp;
	if(NULL == pCfg)
	{
		return ISIL_FAIL;
	}
    tmp = (VENC_H264_FEATURE*)pCfg;
	sprintf(sPath, "%sch%d", ISIL_CH_H264_FEATURE, ch);


    if(H264FeatureDefaultCfg[ch] == 0)//power on
    {
        ISIL_SetDefaultWinH264FeatureCfg(tmp);/*主码流*/
        tmp++;
        ISIL_SetDefaultWinH264FeatureCfg(tmp);/*子码流*/
        H264FeatureDefaultCfg[ch] = 1;

        ISIL_SaveH264Cfg(ch, pCfg, len);
    }

    iRet = ISIL_ReadConfigFile(sPath, 0, len, &iType, pCfg);
	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
	return ISIL_SUCCESS;
}

S32 ISIL_SaveH264Cfg(S32 ch, void* pCfg, S32 len)
{
    S32 ret;
    S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];

    if(NULL == pCfg)
        return ISIL_FAIL;
    sprintf(path, "%sch%d", ISIL_CH_H264_FEATURE, ch);
    ret = ISIL_WriteConfigFile(path, 0, len, &iType, pCfg);
    return ret;
}

static void ISIL_SetDefaultWinEncodeCfg(ISIL_ENCODE_CFG_S* cfg)
{
    if(NULL == cfg)
        return;
    cfg->stStreamEncode.cAudioBitAlign = 0;
    cfg->stStreamEncode.cAudioSampleRate = 0;
    cfg->stStreamEncode.cAudioType = 3;
    cfg->stStreamEncode.cBitMode = 1;
    cfg->stStreamEncode.cBQP = 0;
    cfg->stStreamEncode.cChannelPri = 1;
    cfg->stStreamEncode.cFPS = DEFAULT_FRAMERATE_PAL;
    cfg->stStreamEncode.cHorizontalFlip = 0;
    cfg->stStreamEncode.cVerticalFlip = 0;

    cfg->stStreamEncode.cImageQuality = 0;
    cfg->stStreamEncode.cImageSize = CIF_VIDEO_IMG;
    cfg->stStreamEncode.cIQP = DEFAULT_QP;
    cfg->stStreamEncode.cPQP = DEFAULT_QP;
    cfg->stStreamEncode.cRCType = ISIL_H264_CBR;
    cfg->stStreamEncode.cStreamType = 3;
    cfg->stStreamEncode.nBitRate = 1000000/1000;
    cfg->stStreamEncode.nBPFrameInterval= 0;
    cfg->stStreamEncode.nGOPIntervals = DEFAULT_GOP_VALUE;
    cfg->stStreamEncode.nHeightMBSize = (HEIGHT_FRAME_CIF_PAL>>4);
    cfg->stStreamEncode.nWidthMBSize = (WIDTH_FRAME_CIF_PAL >> 4);
    cfg->stStreamEncode.nIFrameIntervals = DEFAULT_I_P_STRIDE;

    cfg->stStreamEncode2.cAudioBitAlign = 0;
    cfg->stStreamEncode2.cAudioSampleRate = 0;
    cfg->stStreamEncode2.cAudioType = 3;
    cfg->stStreamEncode2.cBitMode = 1;
    cfg->stStreamEncode2.cBQP = 0;
    cfg->stStreamEncode2.cChannelPri = 1;
    cfg->stStreamEncode2.cFPS = DEFAULT_FRAMERATE_PAL;
    cfg->stStreamEncode2.cHorizontalFlip = 0;
    cfg->stStreamEncode2.cVerticalFlip = 0;

    cfg->stStreamEncode2.cImageQuality = 0;
    cfg->stStreamEncode2.cImageSize = QCIF_VIDEO_IMG;
    cfg->stStreamEncode2.cIQP = DEFAULT_QP;
    cfg->stStreamEncode2.cPQP = DEFAULT_QP;
    cfg->stStreamEncode2.cRCType = ISIL_H264_CBR;
    cfg->stStreamEncode2.cStreamType = 3;
    cfg->stStreamEncode2.nBitRate = DEFAULT_BITRATE_CIFSIZE/1000;
    cfg->stStreamEncode2.nBPFrameInterval= 0;
    cfg->stStreamEncode2.nGOPIntervals = DEFAULT_GOP_VALUE;
    cfg->stStreamEncode2.nHeightMBSize = (HEIGHT_FRAME_QCIF_PAL>>4);
    cfg->stStreamEncode2.nWidthMBSize = (WIDTH_FRAME_QCIF_PAL >> 4);
    cfg->stStreamEncode2.nIFrameIntervals = DEFAULT_I_P_STRIDE;
    return;
}

static void ISIL_SetDefaultWinH264FeatureCfg(VENC_H264_FEATURE* cfg)
{
    if(NULL == cfg)
        return;
    cfg->b_enable_deinterlace = 1;
    cfg->b_enable_half_pixel = 1;
    cfg->b_enable_I_4X4 = 0;
    cfg->b_enable_quarter_pixel = 1;
    cfg->b_enable_skip = 0;
    cfg->i_mb_delay_value = 0xf0;
    return;
}

static void ISIL_SetDefaultWinMjpeg(ISIL_MJPEG_CFG* cfg)
{
    cfg->cEnable = 1;
    cfg->cFPS = 5;
    cfg->cImageQuality = 20;
    cfg->cImageSize = D1_VIDEO_IMG;

    cfg->nHeightMBSize = (HEIGHT_FRAME_D1_PAL>>4);
    cfg->nWidthMBSize = (WIDTH_FRAME_D1_PAL >> 4);
}


