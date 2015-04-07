#include "isil_channel_map_cfg.h"
#include "isil_cfg_file_manage.h"
#include "isil_channel_map.h"

#define CHANNEL_MNP "chnmap_"

const static char* maptype[] =
{
    "h264e",
    "h264d",
    "aenc",
    "adec",
    "jpeg",
    " ",
};
extern S08 * gNetSDKCfgType[];

S32 ISIL_GetLocGuiChnMapCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*??¨¨??¡¤??*/
    //ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[MEDIA_SDK_CHN_MAP], channel);
    //sprintf(path, "%s%s", ISIL_CHANNEL_MNP_CFG_PATH, maptype[0]);
    sprintf(path, "%s/%s/%s%s", ISIL_LOCAL_NET_SDK_CFG_PATH,
    	        gNetSDKCfgType[0],
    	        CHANNEL_MNP, maptype[0]);
    printf("%s, path:%s\n", __FUNCTION__, path);
	#ifndef ISIL_USE_SYS_CFG_FILE
	iRet = ISIL_ReadConfigFile(path, 0, CHANNEL_NUMBER*(sizeof(LOC_GUI_CHN_MAP)),
		&iType, buff);
	#else
	iRet = ISIL_ReadConfigFile(path, 0, ISIL_GetSysMaxChnCnt()*(sizeof(LOC_GUI_CHN_MAP)),
		&iType, buff);
	#endif

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

S32 ISIL_SetLocGuiChnMapCfg(S32 channel,void *buff)
{
	S32 iRet;
	S32 iType;
    S08 path[ISIL_CFG_PATH_LEN];
	if(buff == NULL)
	{
		return ISIL_FAIL;
	}
    /*??¨¨??¡¤??*/
    //ISIL_GetNetSDKCfgFilePath(path, NET_SDK_DEV_CFG, filename[MEDIA_SDK_CHN_MAP], channel);
    //sprintf(path, "%s%s", ISIL_CHANNEL_MNP_CFG_PATH, maptype[0]);
    sprintf(path, "%s/%s/%s%s", ISIL_LOCAL_NET_SDK_CFG_PATH,
    	        gNetSDKCfgType[0],
    	        CHANNEL_MNP, maptype[0]);
    printf("%s, path:%s\n", __FUNCTION__, path);
	#ifndef ISIL_USE_SYS_CFG_FILE
	iRet = ISIL_WriteConfigFile(path, 0, CHANNEL_NUMBER*(sizeof(LOC_GUI_CHN_MAP)),
		&iType, buff);
	#else
	iRet = ISIL_WriteConfigFile(path, 0, ISIL_GetSysMaxChnCnt()*(sizeof(LOC_GUI_CHN_MAP)),
		&iType, buff);
	#endif

	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}




