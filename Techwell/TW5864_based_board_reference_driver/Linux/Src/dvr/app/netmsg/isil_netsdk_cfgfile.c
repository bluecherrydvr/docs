#include "isil_netsdk_cfgfile.h"
#include "default_cfg_value.h"
#include "isil_netsdk_activecfg.h"
#include "isil_media_config.h"
#include "isil_channel_map.h"

/*与下面数组对应*/
typedef enum
{
    DEVICE_INFO = 0,
    SYSTEM_ATTR_CFG,
    NET_CFG,
    RECORD_CFG,
    CHANNEL_CFG,
    COM_CFG,
    ALARM_SCHEDULE,
    PREVIEW_CFG,
    CHANNEL_CFG_ALL,
    SNAP_PICTURE,
    MEDIA_SDK_ENCODE,
    MEDIA_SDK_MJPEG,
    MEDIA_SDK_AUDIO,
    MEDIA_SDK_NO_RT,
    MEDIA_SDK_AD,
    LOC_GUI_REC,
    MEDIA_SDK_CHN_MAP,
    ALARM_TYPE_CFG,
	MEDIA_SDK_OSD,
    FILE_NAME_MAX,
}E_FILE_CFG;
/*与上面Enum对应*/
static S08* filename[]=
{
    "deviceinfo",
    "sysattr",
    "netcfg",
    "reccfg",
    "channelcfg",
    "comcfg",
    "alarmschddule",
    "previewcfg",
    "channelcfg_all",
    "snappic",
    "mediasdk_encode",
    "mediasdk_mjpeg",
    "mediasdk_audio",
    "mediasdk_no_rt",
    "mediasdk_ad",
    "locguirec",
    "chnmap",
    "alarmtype",
	"chnosd"
    " ",
};
extern S08 * gNetSDKCfgType[];


#if 1
/*如果需要channel:path:/xxx/xxx/xxx_x
  不需要channel:path:/xxx/xxx/xxx
*/
static void ISIL_GetNetSDKCfgFilePath(S08* dest, S32 cfgtype,S08* fileName, S32 channel)
{
    if(NULL == dest || NULL == fileName)
        return;
    if(channel < 0)
        sprintf(dest, "%s/%s/%s", ISIL_LOCAL_NET_SDK_CFG_PATH,
    	        gNetSDKCfgType[cfgtype],
    	        fileName);
    else
        sprintf(dest, "%s/%s/%s_%d", ISIL_LOCAL_NET_SDK_CFG_PATH,
    	        gNetSDKCfgType[cfgtype],
    	        fileName, channel);
    return;

}


S32 ISIL_GetDeviceInfo(void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[DEVICE_INFO], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_ReadConfigFile(path, 0, sizeof(NET_DEVICEINFO),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_SetDeviceInfo(void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[DEVICE_INFO], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_WriteConfigFile(path, 0, sizeof(NET_DEVICEINFO),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}




S32 ISIL_GetSysAttrCfg(void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[SYSTEM_ATTR_CFG], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_ReadConfigFile(path, 0, sizeof(ISIL_DEV_SYSTEM_ATTR_CFG),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_SetSysAttrCfg(void *buff)
{
	S32 iRet;
	S32 iType;
    ISIL_DEV_SYSTEM_ATTR_CFG *attr;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    attr = (ISIL_DEV_SYSTEM_ATTR_CFG*)buff;
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[SYSTEM_ATTR_CFG], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_WriteConfigFile(path, 0, sizeof(ISIL_DEV_SYSTEM_ATTR_CFG),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
    ISIL_NetSDK_SetVideoStandard(attr->byVideoStandard);
	return ISIL_SUCCESS;
}

S32 ISIL_GetNetCfg(void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[NET_CFG], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_ReadConfigFile(path, 0, sizeof(ISIL_DEV_NET_CFG),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_SetNetCfg(void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[NET_CFG], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_WriteConfigFile(path, 0, sizeof(ISIL_DEV_NET_CFG),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_GetChannelCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/

    if(channel < 0)/*所有通道*/
    {
        iChCnt = CHANNEL_CFG_ALL;

		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = get_media_max_chan_number()*(sizeof(ISIL_DEV_CHANNEL_CFG));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(ISIL_DEV_CHANNEL_CFG));
		#endif
    }
    else
    {
        iChCnt = CHANNEL_CFG;
        iDataSize = sizeof(ISIL_DEV_CHANNEL_CFG);
    }

    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);

	iRet = ISIL_ReadConfigFile(path, 0, iDataSize, &iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_SetChannelCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}


    if(channel < 0)/*所有通道*/
    {
        iChCnt = CHANNEL_CFG_ALL;
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(ISIL_DEV_CHANNEL_CFG));
		#else
        //for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(ISIL_DEV_CHANNEL_CFG));
        #endif
    }
    else
    {
        iChCnt = CHANNEL_CFG;
        iDataSize = sizeof(ISIL_DEV_CHANNEL_CFG);
    }
    /*设置编码参数*/
    iRet = ISIL_NetSDK_ActiveEncodeParameter(channel, -1, buff);
    if(iRet < 0)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_WriteConfigFile(path, 0, iDataSize, &iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_GetRecordCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[RECORD_CFG], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_ReadConfigFile(path, 0, sizeof(ISIL_RECORD_CFG),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_SetRecordCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[RECORD_CFG], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_WriteConfigFile(path, 0, sizeof(ISIL_RECORD_CFG),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}


S32 ISIL_GetNetSDKComCfg(void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[COM_CFG], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_ReadConfigFile(path, 0, sizeof(ISIL_DEV_COMM_CFG),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_SetNetSDKComCfg(void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[COM_CFG], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	iRet = ISIL_WriteConfigFile(path, 0, sizeof(ISIL_DEV_COMM_CFG),
		&iType, buff);

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}
S32 ISIL_SetSnapPictureCfg(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[SNAP_PICTURE], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_WriteConfigFile(path, 0, sizeof(SNAP_PARAMS),
        &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}

S32 ISIL_GetSnapPictureCfg(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[SNAP_PICTURE], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_ReadConfigFile(path, 0, sizeof(SNAP_PARAMS),
        &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}


static void _TransMediaSdkEncodeByCh(S32 channel,void *buff)
{
    VENC_H264_INFO* cfg;
    S32 i;
    cfg = (VENC_H264_INFO*)buff;

    if(channel < 0) /*所有通道*/
    {
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
		for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
		#endif
        {
            /*进行结构体转换*/
            ISIL_LOCAL_Trans_VENC_H264_INFO(cfg);
            cfg++;
        }
    }
    else /*单独通道*/
    {
        ISIL_LOCAL_Trans_VENC_H264_INFO(cfg);
    }
    return;
}
S32 ISIL_GetMediaSdkEncode(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    static S32 time = 1;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_ENCODE;
        
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(VENC_H264_INFO));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(VENC_H264_INFO));
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_ENCODE;
        iDataSize = sizeof(VENC_H264_INFO);
        fprintf(stderr, "----gui get single channel,not support!!!------\n");
        return ISIL_FAIL;
    }

    fprintf(stderr, "------ISIL_GetMediaSdkEncode 000000-----\n");
    if(time == 1)
    {
    	fprintf(stderr, "------ISIL_GetMediaSdkEncode 1111111-----\n");
        ISIL_SaveMediaSdkEncodeDefault();
        time++;
    }
     /**/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);

    /*读取通道正常参数*/
    iRet = ISIL_ReadConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    /*读取通道默认参数*/
    ISIL_GetMediaSdkEncodeDefault(buff + iDataSize);

    _TransMediaSdkEncodeByCh(channel, buff);
    _TransMediaSdkEncodeByCh(0, buff + iDataSize);

    return ISIL_SUCCESS;
}
S32 ISIL_SetMediaSdkEncode(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_ENCODE;

		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(VENC_H264_INFO));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(VENC_H264_INFO));
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_ENCODE;
        iDataSize = sizeof(VENC_H264_INFO);
    }
    /*设置编码参数*/
    _TransMediaSdkEncodeByCh(channel, buff);
    iRet = ISIL_NetSDK_ActiveMediaSdkEncodeParameter(channel, -1, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_WriteConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}



/*mjpeg*/
static void _TransMediaSdkMjpegByCh(S32 channel,void *buff)
{
    VENC_MJPG_INFO* cfg;
    S32 i;
    cfg = (VENC_MJPG_INFO*)buff;

    if(channel < 0) /*所有通道*/
    {

        
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
		for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
		#endif
        {
            /*进行结构体转换*/
            ISIL_LOCAL_Trans_VENC_MJPG_INFO(cfg);
            cfg++;
        }
    }
    else /*单独通道*/
    {
        ISIL_LOCAL_Trans_VENC_MJPG_INFO(cfg);
    }
    return;
}

S32 ISIL_GetAllChMjpegCfg(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_MJPEG;
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(VENC_MJPG_INFO));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(VENC_MJPG_INFO));
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_MJPEG;
        iDataSize = sizeof(VENC_MJPG_INFO);
        fprintf(stderr, "----gui get single channel,not support!!!------\n");
        return ISIL_FAIL;
    }

    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);

    iRet = ISIL_ReadConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
    	fprintf(stderr, "-----%s,ISIL_ReadConfigFile fail-----\n", __FUNCTION__);
        return ISIL_FAIL;
    }
    return ISIL_SUCCESS;
}


static void _TransMediaSdkAudioByCh(S32 channel,void *buff)
{
    VENC_AUDIO_INFO* cfg;
    S32 i;
    cfg = (VENC_AUDIO_INFO*)buff;

    if(channel < 0) /*锟斤拷锟斤拷通锟斤拷*/
    {


		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
		for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
		#endif
        {
            /*锟斤拷锟叫结构锟斤拷转锟斤拷*/
			ISIL_LOCAL_Trans_VENC_AUDIO_INFO(cfg);
            cfg++;
        }
    }
    else /*锟斤拷锟斤拷通锟斤拷*/
    {
        ISIL_LOCAL_Trans_VENC_AUDIO_INFO(cfg);
    }
    return;
}

S32 ISIL_GetAllChAudioCfg(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }
    /*锟斤拷取路锟斤拷*/

    if(channel < 0)/*锟斤拷锟斤拷通锟斤拷*/
    {
        iChCnt = MEDIA_SDK_AUDIO;
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(VENC_AUDIO_INFO));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(VENC_AUDIO_INFO));
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_AUDIO;
        iDataSize = sizeof(VENC_AUDIO_INFO);
        fprintf(stderr, "----gui get single channel,not support!!!------\n");
        return ISIL_FAIL;
    }

    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);

    iRet = ISIL_ReadConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
    	fprintf(stderr, "-----%s,ISIL_ReadConfigFile fail-----\n", __FUNCTION__);
        return ISIL_FAIL;
    }
    return ISIL_SUCCESS;
}

S32 ISIL_GetMediaSdkMjpeg(S32 channel,void *buff)
{
    if(buff == NULL)
    {
    	fprintf(stderr, "-----%s, buff null----\n", __FUNCTION__);
        return ISIL_FAIL;
    }
    #if 0
    /*获取路径*/

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_MJPEG;
        iDataSize = CHANNEL_NUMBER*(sizeof(VENC_MJPG_INFO));
    }
    else
    {
        iChCnt = MEDIA_SDK_MJPEG;
        iDataSize = sizeof(VENC_MJPG_INFO);
    }

    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);

    iRet = ISIL_ReadConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    #else
    if(ISIL_GetAllChMjpegCfg(channel, buff) < 0){
    	fprintf(stderr, "-----ISIL_GetAllChMjpegCfg fail----\n");
    	return ISIL_FAIL;
    }
    #endif
    _TransMediaSdkMjpegByCh(channel, buff);
    return ISIL_SUCCESS;
}

S32 ISIL_SetMediaSdkMjpeg(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_MJPEG;
        //iDataSize = CHANNEL_NUMBER*(sizeof(VENC_MJPG_INFO));
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(VENC_MJPG_INFO));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(VENC_MJPG_INFO));
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_MJPEG;
        iDataSize = sizeof(VENC_MJPG_INFO);
        fprintf(stderr, "----gui set single channel,not support!!!------\n");
        return ISIL_FAIL;
    }
    /*设置编码参数*/
    _TransMediaSdkMjpegByCh(channel, buff);
    iRet = ISIL_NetSDK_ActiveMediaSdkMjpeg(channel, -1, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_WriteConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}

S32 ISIL_GetMediaSdkAudio(S32 channel, void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }


#if 0
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[MEDIA_SDK_AUDIO], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);

    iRet = ISIL_ReadConfigFile(path, 0, sizeof(VENC_AUDIO_INFO), &iType, buff);
#endif

    iRet = ISIL_GetAllChAudioCfg(channel, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    _TransMediaSdkAudioByCh(channel, buff);
    return ISIL_SUCCESS;
}

#if 0
S32 ISIL_SetMediaSdkAudio(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }

    ISIL_LOCAL_Trans_VENC_AUDIO_INFO(buff);
    /*锟斤拷锟矫憋拷锟斤拷锟斤拷锟*/
    iRet = ISIL_NetSDK_ActiveMediaSdkAudio(channel, -1, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    /*锟斤拷取路锟斤拷*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[MEDIA_SDK_AUDIO], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_WriteConfigFile(path, 0, sizeof(VENC_AUDIO_INFO), &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}
#else
S32 ISIL_SetMediaSdkAudio(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }

    if(channel < 0)/*锟斤拷锟斤拷通锟斤拷*/
    {
        iChCnt = MEDIA_SDK_AUDIO;
        //iDataSize = CHANNEL_NUMBER*(sizeof(VENC_MJPG_INFO));
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(VENC_AUDIO_INFO));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(VENC_AUDIO_INFO));
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_AUDIO;
        iDataSize = sizeof(VENC_AUDIO_INFO);
        fprintf(stderr, "----gui set single channel,not support!!!------\n");
        return ISIL_FAIL;
    }
    /*锟斤拷锟矫憋拷锟斤拷锟斤拷锟*/
    _TransMediaSdkAudioByCh(channel, buff);
    iRet = ISIL_NetSDK_ActiveMediaSdkAudio(channel, -1, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    /* */
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_WriteConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}
#endif

S32 ISIL_GetMediaSdkNoRT(void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[MEDIA_SDK_NO_RT], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);

    iRet = ISIL_ReadConfigFile(path, 0, sizeof(ENC_CTL_VD_INFO), &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    ISIL_LOCAL_Trans_NotRT(buff);
    return ISIL_SUCCESS;
}

S32 ISIL_SetMediaSdkNoRT(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }

    ISIL_LOCAL_Trans_NotRT(buff);
    /*设置编码参数*/
    iRet = ISIL_NetSDK_ActiveMediaSdkNotRT(channel, -1, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[MEDIA_SDK_NO_RT], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_WriteConfigFile(path, 0, sizeof(ENC_CTL_VD_INFO), &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}

static void _TransMediaSdkADByCh(S32 channel,void *buff)
{
    VI_ANALOG_INFO* cfg;
    S32 i;
    cfg = (VI_ANALOG_INFO*)buff;

    if(channel < 0) /*所有通道*/
    {
        
		#ifndef ISIL_USE_SYS_CFG_FILE
        for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
		for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++) 
		#endif
        {
            /*进行结构体转换*/
            ISIL_LOCAL_Trans_AD(cfg);
            cfg++;
        }
    }
    else /*单独通道*/
    {
        ISIL_LOCAL_Trans_AD(cfg);
    }
    return;
}


S32 ISIL_SetMediaSdkAD(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_AD;
        
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(VI_ANALOG_INFO));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(VI_ANALOG_INFO)); 
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_AD;
        iDataSize = sizeof(VI_ANALOG_INFO);
    }
    /*设置编码参数*/
    _TransMediaSdkADByCh(channel, buff);
    iRet = ISIL_NetSDK_ActiveMediaSdkAD(channel, -1, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_WriteConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}
S32 ISIL_GetMediaSdkAD(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    static S32 time = 1;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_AD;
        //iDataSize = CHANNEL_NUMBER*(sizeof(VI_ANALOG_INFO));
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(VI_ANALOG_INFO));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(VI_ANALOG_INFO)); 
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_AD;
        iDataSize = sizeof(VI_ANALOG_INFO);
    }
    if(1 == time)
    {
        ISIL_SaveMediaSdkADDefault();
        time++;
    }


    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);

    iRet = ISIL_ReadConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    _TransMediaSdkADByCh(channel, buff);
    return ISIL_SUCCESS;
}

S32 ISIL_SaveMediaSdkADDefault()
{

    S32 i;
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    VI_ANALOG_INFO info[CHANNEL_NUMBER];
    VI_ANALOG_INFO* tmp;
    CODEC_CHANNEL* codec_ch = NULL;


    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[MEDIA_SDK_AD], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iDataSize = sizeof(info);
    tmp = info;
    memset(tmp, 0, iDataSize);

    
	#ifndef ISIL_USE_SYS_CFG_FILE
	for(i = 0; i < CHANNEL_NUMBER; i++)
	#else
	for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
	#endif
    {
        #if 1
       tmp->vichcfg.u32Brightness = DEFAULT_VI_BRIGHTNESS;
       tmp->vichcfg.u32Contrast = DEFAULT_VI_CONTRAST;
       tmp->vichcfg.u32Hue = DEFAULT_VI_HUE;
       tmp->vichcfg.u32Saturation = DEFAULT_VI_SATURATION;
       tmp->vichcfg.u32Sharpness = DEFAULT_VI_SHARPNESS;
       tmp->videostd.eVideoType = DEFAULT_VI_STANDARD;
       #endif
       tmp++;
    }
    iRet = ISIL_WriteConfigFile(path, 0, iDataSize, &iType, info);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}


S32 ISIL_GetMediaSdkEncodeDefault(VENC_H264_INFO* tmp)
{
    if(NULL == tmp)
    {
        return ISIL_FAIL;
    }

    #if 1
    tmp->channel_enable[0] = 1;

    tmp->venc_h264cfg[0].i_bps = DEFAULT_BITRATE_D1SIZE;
    tmp->venc_h264cfg[0].i_fps = DEFAULT_FRAMERATE_PAL;
    tmp->venc_h264cfg[0].i_gop_value = DEFAULT_GOP_VALUE;
    tmp->venc_h264cfg[0].i_I_P_stride = DEFAULT_I_P_STRIDE;
    tmp->venc_h264cfg[0].i_logic_video_height_mb_size = (HEIGHT_FRAME_D1_PAL>>4);
    tmp->venc_h264cfg[0].i_logic_video_width_mb_size = (WIDTH_FRAME_D1_PAL >> 4);
    tmp->venc_h264feature[0].b_enable_deinterlace = 1;
    tmp->venc_h264feature[0].b_enable_half_pixel = 1;
    tmp->venc_h264feature[0].b_enable_I_4X4 = 1;
    tmp->venc_h264feature[0].b_enable_quarter_pixel =1;
    tmp->venc_h264feature[0].b_enable_skip = 1;
    tmp->venc_h264feature[0].i_mb_delay_value = 0xf0;
    tmp->venc_h264rc[0].e_image_priority = ISIL_RC_IMAGE_QUALITY_FIRST;
    tmp->venc_h264rc[0].e_rc_type = ISIL_H264_CBR;
    tmp->venc_h264rc[0].i_qpb = DEFAULT_QP;
	tmp->venc_h264rc[0].i_qpi = DEFAULT_QP;
	tmp->venc_h264rc[0].i_qpp = DEFAULT_QP;

    tmp->venc_h264cfg[1].i_bps = DEFAULT_BITRATE_CIFSIZE;
    tmp->venc_h264cfg[1].i_fps = DEFAULT_FRAMERATE_PAL;
    tmp->venc_h264cfg[1].i_gop_value = DEFAULT_GOP_VALUE;
    tmp->venc_h264cfg[1].i_I_P_stride = DEFAULT_I_P_STRIDE;
    tmp->venc_h264cfg[1].i_logic_video_height_mb_size = (HEIGHT_FRAME_CIF_PAL>>4);
    tmp->venc_h264cfg[1].i_logic_video_width_mb_size = (WIDTH_FRAME_CIF_PAL >> 4);
    tmp->venc_h264feature[1].b_enable_deinterlace = 1;
    tmp->venc_h264feature[1].b_enable_half_pixel = 1;
    tmp->venc_h264feature[1].b_enable_I_4X4 = 1;
    tmp->venc_h264feature[1].b_enable_quarter_pixel =1;
    tmp->venc_h264feature[1].b_enable_skip = 1;
    tmp->venc_h264feature[1].i_mb_delay_value = 0xf0;
    tmp->venc_h264rc[1].e_image_priority = ISIL_RC_IMAGE_QUALITY_FIRST;
    tmp->venc_h264rc[1].e_rc_type = ISIL_H264_CBR;
    tmp->venc_h264rc[1].i_qpb = DEFAULT_QP;
    tmp->venc_h264rc[1].i_qpi = DEFAULT_QP;
    tmp->venc_h264rc[1].i_qpp = DEFAULT_QP;
    tmp->channel_enable[1] = 1;
    #endif
    return ISIL_SUCCESS;
}

S32 ISIL_SaveMediaSdkEncodeDefault()
{

    S32 i;
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    VENC_H264_INFO info[CHANNEL_NUMBER];
    VENC_H264_INFO* tmp;
    CODEC_CHANNEL* codec_ch = NULL;
    CHIP_CHN_PHY_ID phy_id;
    enum ISIL_VIDEO_STANDARD video_std;

    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[MEDIA_SDK_ENCODE], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iDataSize = sizeof(info);
    tmp = info;
    memset(tmp, 0, iDataSize);
    //for(i = 0; i < CHANNEL_NUMBER; i++)
	#ifndef ISIL_USE_SYS_CFG_FILE
	for(i = 0; i < CHANNEL_NUMBER; i++)
	#else
	for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
	#endif
    {
		if(ISIL_GetChipChnIDByLogicID(i, &phy_id) < 0){
			fprintf(stderr, "----ISIL_GetChipChnIDByLogicID fail----\n");
			return ISIL_FAIL;
		}

		if(ISIL_MediaSDK_GetVideoStandard(phy_id.chipid, &video_std) < 0){
			fprintf(stderr, "----ISIL_MediaSDK_GetVideoStandard fail----\n");
			return ISIL_FAIL;
		}

		fprintf(stderr, "---ISIL_SaveMediaSdkEncodeDefault, -video_std: %d ----\n", video_std);
        #if 1
        tmp->channel_enable[0] = 1;
        tmp->venc_h264cfg[0].i_bps = DEFAULT_BITRATE_D1SIZE;
        tmp->venc_h264cfg[0].i_fps = DEFAULT_FRAMERATE_PAL;
        tmp->venc_h264cfg[0].i_gop_value = DEFAULT_GOP_VALUE;
        tmp->venc_h264cfg[0].i_I_P_stride = DEFAULT_I_P_STRIDE;
        if(video_std == ISIL_VIDEO_STANDARD_PAL){
        	tmp->venc_h264cfg[0].i_logic_video_height_mb_size = (HEIGHT_FRAME_D1_PAL>>4);
        	tmp->venc_h264cfg[0].i_logic_video_width_mb_size = (WIDTH_FRAME_D1_PAL >> 4);
        }else{
        	tmp->venc_h264cfg[0].i_logic_video_height_mb_size = (HEIGHT_FRAME_D1_NTSC>>4);
        	tmp->venc_h264cfg[0].i_logic_video_width_mb_size = (WIDTH_FRAME_D1_NTSC >> 4);
        }
        tmp->venc_h264feature[0].b_enable_deinterlace = 1;
        tmp->venc_h264feature[0].b_enable_half_pixel = 1;
        tmp->venc_h264feature[0].b_enable_I_4X4 = 1;
        tmp->venc_h264feature[0].b_enable_quarter_pixel =1;
        tmp->venc_h264feature[0].b_enable_skip = 1;
        tmp->venc_h264feature[0].i_mb_delay_value = 0xf0;
        tmp->venc_h264rc[0].e_image_priority = ISIL_RC_IMAGE_QUALITY_FIRST;
        tmp->venc_h264rc[0].e_rc_type = ISIL_H264_CBR;
        tmp->venc_h264rc[0].i_qpb = DEFAULT_QP;
        tmp->venc_h264rc[0].i_qpi = DEFAULT_QP;
        tmp->venc_h264rc[0].i_qpp = DEFAULT_QP;

        tmp->venc_h264cfg[1].i_bps = DEFAULT_BITRATE_CIFSIZE;
        tmp->venc_h264cfg[1].i_fps = DEFAULT_FRAMERATE_PAL;
        tmp->venc_h264cfg[1].i_gop_value = DEFAULT_GOP_VALUE;
        tmp->venc_h264cfg[1].i_I_P_stride = DEFAULT_I_P_STRIDE;
        if(video_std == ISIL_VIDEO_STANDARD_PAL){
			tmp->venc_h264cfg[1].i_logic_video_height_mb_size = (HEIGHT_FRAME_CIF_PAL>>4);
			tmp->venc_h264cfg[1].i_logic_video_width_mb_size = ((WIDTH_FRAME_D1_PAL>>1)>>4);
		}else{
	        tmp->venc_h264cfg[1].i_logic_video_height_mb_size = (HEIGHT_FRAME_CIF_NTSC>>4);
	        tmp->venc_h264cfg[1].i_logic_video_width_mb_size = ((WIDTH_FRAME_D1_NTSC>>1)>>4);//(WIDTH_FRAME_CIF_PAL >> 4);
		}
        tmp->venc_h264feature[1].b_enable_deinterlace = 1;
        tmp->venc_h264feature[1].b_enable_half_pixel = 1;
        tmp->venc_h264feature[1].b_enable_I_4X4 = 1;
        tmp->venc_h264feature[1].b_enable_quarter_pixel =1;
        tmp->venc_h264feature[1].b_enable_skip = 1;
        tmp->venc_h264feature[1].i_mb_delay_value = 0xf0;
        tmp->venc_h264rc[1].e_image_priority = ISIL_RC_IMAGE_QUALITY_FIRST;
        tmp->venc_h264rc[1].e_rc_type = ISIL_H264_CBR;
        tmp->venc_h264rc[1].i_qpb = DEFAULT_QP;
        tmp->venc_h264rc[1].i_qpi = DEFAULT_QP;
        tmp->venc_h264rc[1].i_qpp = DEFAULT_QP;
        tmp->channel_enable[1] = 1;
        #endif
        //ISIL_GetMediaSdkEncodeDefault(tmp);
        tmp++;
    }
    iRet = ISIL_WriteConfigFile(path, 0, iDataSize, &iType, info);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }

    return ISIL_SUCCESS;
}

S32 ISIL_GetLocGuiRecordCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[LOC_GUI_REC], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
	
	#ifndef ISIL_USE_SYS_CFG_FILE
	iRet = ISIL_ReadConfigFile(path, 0, CHANNEL_NUMBER*sizeof(ISIL_RECORD_CFG),
		&iType, buff);
	#else
	iRet = ISIL_ReadConfigFile(path, 0, ISIL_GetSysMaxChnCnt()*sizeof(ISIL_RECORD_CFG),
		&iType, buff);
	#endif

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_SetLocGuiRecordCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[LOC_GUI_REC], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
	//iRet = ISIL_WriteConfigFile(path, 0, CHANNEL_NUMBER*sizeof(ISIL_RECORD_CFG),
	//	&iType, buff);
	#ifndef ISIL_USE_SYS_CFG_FILE
	iRet = ISIL_WriteConfigFile(path, 0, CHANNEL_NUMBER*sizeof(ISIL_RECORD_CFG),
		&iType, buff);
	#else
	iRet = ISIL_WriteConfigFile(path, 0, ISIL_GetSysMaxChnCnt()*sizeof(ISIL_RECORD_CFG),
		&iType, buff);
	#endif

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_GetLocGuiAlarmTypeCfg(void *buff)
{
	S32 iRet;
	//S32 i;
	S32 iType;
	//U32 *tmp;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[ALARM_TYPE_CFG], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	#ifndef ISIL_USE_SYS_CFG_FILE
	iRet = ISIL_ReadConfigFile(path, 0, CHANNEL_NUMBER*sizeof(ISIL_ALARM_TYPE),
		&iType, buff);
	#else
	iRet = ISIL_ReadConfigFile(path, 0, ISIL_GetSysMaxChnCnt()*sizeof(ISIL_ALARM_TYPE),
		&iType, buff);
	#endif

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
	/*
	tmp = (U32*)buff;
	for(i = 0; i < CHANNEL_NUMBER; i++){
		ISIL_TransAlarmType(tmp);
		tmp++;
	}
	*/
	return ISIL_SUCCESS;
}

S32 ISIL_SetLocGuiAlarmTypeCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 i;
	S32 iType;
	U32 *tmp;
	U32 clearType;
    S08 path[ISIL_CFG_PATH_LEN];
    U32 AlarmType[CHANNEL_NUMBER];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
	tmp = (U32*)buff;
	ISIL_GetLocGuiAlarmTypeCfg(AlarmType);
	if(channel < 0){
		#ifndef ISIL_USE_SYS_CFG_FILE
		for(i = 0; i < CHANNEL_NUMBER; i++)
		#else
		for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
		#endif
			{
			printf("alarm type %x\n", *tmp);
			iRet = ISIL_NetSDK_ActiveAlarmType(i, *tmp);
			if(iRet < 0)
				return ISIL_FAIL;
			#if 0
			clearType = ~(*tmp) & AlarmType[i];
			fprintf(stderr, "clear alarm type 0x%x\n", clearType);
			iRet = ISIL_NetSDK_ClearAlarmType(i, clearType);
			if(iRet < 0)
				return ISIL_FAIL;
			#endif
			tmp++;
		}
	}
	else{
		printf("alarm type %x\n", tmp[channel]);
		iRet = ISIL_NetSDK_ActiveAlarmType(channel, tmp[channel]);
		if(iRet < 0)
			return ISIL_FAIL;
		#if 0
		clearType = ~(tmp[channel]) & AlarmType[channel];
		fprintf(stderr, "clear alarm type 0x%x\n", clearType);
		iRet = ISIL_NetSDK_ClearAlarmType(i, clearType);
		if(iRet < 0)
			return ISIL_FAIL;
		#endif
	}

    /*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[ALARM_TYPE_CFG], -1);
    printf("%s, path:%s\n", __FUNCTION__, path);
	#ifndef ISIL_USE_SYS_CFG_FILE
	iRet = ISIL_WriteConfigFile(path, 0, CHANNEL_NUMBER*sizeof(ISIL_ALARM_TYPE),
		&iType, buff);
	#else
	iRet = ISIL_WriteConfigFile(path, 0, ISIL_GetSysMaxChnCnt()*sizeof(ISIL_ALARM_TYPE),
		&iType, buff);
	#endif

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_GetMediaSdkOsd(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }
    /*获取路径*/

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_OSD;
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(ISIL_OSD_CFG));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(ISIL_OSD_CFG));
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_OSD;
        iDataSize = sizeof(ISIL_OSD_CFG);
        fprintf(stderr, "----get osd cfg,single channel not support!!----\n");
        return ISIL_FAIL;
    }

    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);

    /*读取通道正常参数*/
    iRet = ISIL_ReadConfigFile(path, 0, iDataSize, &iType, buff);

    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
	//TransMediaSdkOsdByCh(channel, buff);
    return ISIL_SUCCESS;
}

S32 ISIL_SetMediaSdkOsd(S32 channel,void *buff)
{
    S32 iRet;
    S32 iType;
    S32 iChCnt;
    S32 iDataSize;
    S08 path[ISIL_CFG_PATH_LEN];
    if(buff == NULL)
    {
        return ISIL_FAIL;
    }

    if(channel < 0)/*所有通道*/
    {
        iChCnt = MEDIA_SDK_OSD;
		#ifndef ISIL_USE_SYS_CFG_FILE
        iDataSize = CHANNEL_NUMBER*(sizeof(ISIL_OSD_CFG));
		#else
		iDataSize = ISIL_GetSysMaxChnCnt()*(sizeof(ISIL_OSD_CFG));
		#endif
    }
    else
    {
        iChCnt = MEDIA_SDK_OSD;
        iDataSize = sizeof(ISIL_OSD_CFG);
    }


	//TransMediaSdkOsdByCh(channel, buff);
	/*获取路径*/
    ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[iChCnt], channel);
    printf("%s, path:%s\n", __FUNCTION__, path);
    iRet = ISIL_WriteConfigFile(path, 0, iDataSize, &iType, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    /*设置编码参数*/
	iRet = ISIL_NetSDK_ActiveMediaSdkOsd(channel, -1, buff);
    if(iRet < 0)
    {
        return ISIL_FAIL;
    }
    

    return ISIL_SUCCESS;
}


#endif

