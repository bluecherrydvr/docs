/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_dev_manage.h
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:handle device information.


	Other:
	Function list:
	History:

*******************************************************************
*/

#include "isil_dev_manage.h"
#include "isil_cfg_file_manage.h"
#include "isil_debug.h"
#include "isil_messagestruct.h"
#include "isil_netsdk_netmsg.h"
#include "isil_endian_trans.h"
#include "isil_media_config.h"
#include "isil_channel_map.h"


static ISIL_DEV_BASE_CFG_S ISIL_DevBaseCfg =
{

	.sDeviceName = "ISILNVS",
	.sSerialNumber = "01234",
	.lDeviceID = 1,
	.lSoftwareVersion = 1,
	.lSoftwareBuildDate = 20090326,

	.lDSPSoftwareVersion = 1,
	.lDSPSoftwareBuildDate = 20090326,
	.lPanelVersion = 1,

	.lHardwareVersion = 1,
	.lDeviceClass = 1,
	.lDeviceType = 1,

	.cChannelCount = 4,
	.cAlarmInCount = 4,
	.cOutCtrlCount = 2,
	.cRS232Count = 1,
	.cRS485Count = 1,
	.cNetworkPortCount = 1,
	.cDiskCtrlCount = 1,
	.cDiskCount = 1,

	.cDecordCount = 1,
	.cVGACount = 1,
	.cUSBCount = 1,
	.cStandard = 0,
	.stSysTime = {2009, 1,1,12,12,30}
	//.ReservedData = 0

};

static ISIL_DEV_CFG_S ISIL_DevCfg =
{
	.cDeviceBaseEnable = ISIL_ENABLE,
	.cNetEnable        = ISIL_ENABLE,
	.cPTZProEnable 	   = ISIL_ENABLE,
	.cReserve          = 0,
	.stDeviceBase =
	{
		"ISILNVS","0", 1,1,20090326,1,20090326,1,1,1,1,16,16,2,1,1,1,1,1,1,1,
		1,0,{2009, 1,1,12,12,30}
	},

	.stNet =
	{
		0,0x65845221,0xffffff0,0,0,2,11000,80,0,"0", 0,0,0,0,"0","0",0,0, "0","0","0",0,0,0,0,
	}
	
};

static U32 g_ChCnt = 0;
static U32 g_OutctlCnt = 0;
static S32 g_ComRS232Cnt = 0;
static S32 g_ComRS485Cnt = 0;

#if 0
void ISIL_TransDevBaseInfo(ISIL_DEV_BASE_CFG_S *sDevInfo)
{
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
#endif

/********************************************************
 *Function: ISIL_SetDevInfo
 *Description:
 *	根据输入，设置设备信息
 *Input:
 *	ISIL_DEV_CFG_S *sDevInfo:设备配置信息
 *output:
 *      无
 *return:
 *       Success: 0, Fail: -1
 *******************************************************/

S32 ISIL_SetDevInfo(ISIL_DEV_CFG_S *sDevInfo)
{
	S32 iRet;
	S32 iType;
	if(sDevInfo == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], dev pointer null\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

    /*转换字节序*/
    ISIL_TransDevBaseInfo(&sDevInfo->stDeviceBase);
    ISIL_TransNetworkConfig(&sDevInfo->stNet);
	iRet = ISIL_WriteConfigFile(DEV_CONFIG_FILE, 0, sizeof(ISIL_DEV_CFG_S),
		&iType, sDevInfo);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], write dev cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

/*暂时屏蔽掉*/
/*
static S32 ISIL_SetDevDefault()
{
	S32 iRet;
	iRet = ISIL_SetDevInfo(&ISIL_DevCfg);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "Set dev default cfg fail\n");
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}
*/
U32 ISIL_GetChCnt()
{
    S32 iRet;
	ISIL_DEV_BASE_CFG_S sDevBaseInfo;
	memset(&sDevBaseInfo, 0, sizeof(ISIL_DEV_BASE_CFG_S));
    iRet = ISIL_GetDevBaseInfo(&sDevBaseInfo);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], read devbase cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
    g_ChCnt = sDevBaseInfo.cChannelCount;
    return g_ChCnt;
}

U32 ISIL_GetOutCtlCnt()
{
    return g_OutctlCnt;
}

S32 ISIL_GetComRS232Cnt()
{
    return g_ComRS232Cnt;
}

S32 ISIL_GetComRS485Cnt()
{
    return g_ComRS485Cnt;
}


/*********************************************************
 *Function: ISIL_SetDevInfo
 *Description:
 *	从配置文件中获得设备信息
 *Input:
 *	ISIL_DEV_CFG_S *sDevInfo:放置设备配置信息的指针
 *output:
 *      ISIL_DEV_CFG_S *sDevInfo:放置设备配置信息的指针
 *return:
 *       Success: 0, Fail: -1
 *******************************************************/

S32 ISIL_GetDevInfo(ISIL_DEV_CFG_S *sDevInfo)
{
	S32 iRet;
	ISIL_DEV_BASE_CFG_S sDevBaseInfo;
    ISIL_NET_CFG_S param;
    ISIL_PTZ_PROTOCOL_S ptzProtocol;
	if(sDevInfo == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], dev base pointer null\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
    iRet = ISIL_GetDevBaseInfo(&sDevBaseInfo);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], read dev cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

    /*获得通道数目*/
	#ifndef ISIL_USE_SYS_CFG_FILE
    sDevBaseInfo.cChannelCount = get_media_max_chan_number();
	#else
	sDevBaseInfo.cChannelCount = ISIL_GetSysMaxChnCnt();
	#endif
    ISIL_TransDevBaseInfo(&sDevBaseInfo);

    iRet = ISIL_GetNetworkConfig(&param);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], read dev cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

    ISIL_TransNetworkConfig(&param);

    iRet = ISIL_GetPTZProtocol(0, &ptzProtocol);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], read dev cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
    memcpy(&sDevInfo->stDeviceBase, &sDevBaseInfo, sizeof(ISIL_DEV_BASE_CFG_S));
    memcpy(&sDevInfo->stNet, &param, sizeof(ISIL_NET_CFG_S));
    memcpy(&sDevInfo->stPTZProtocol, &ptzProtocol, sizeof(ISIL_PTZ_PROTOCOL_S));
    /*保存供其他地方使用*/
    g_ChCnt = sDevBaseInfo.cChannelCount;
    g_OutctlCnt = sDevBaseInfo.cOutCtrlCount;
    g_ComRS232Cnt = sDevBaseInfo.cRS232Count;
    g_ComRS485Cnt = sDevBaseInfo.cRS485Count;
	return ISIL_SUCCESS;
}

/*set Base dev*/
/********************************************************
 *Function: ISIL_SetDevBaseInfo
 *Description:
 *	根据输入，设置设备基本信息
 *Input:
 *	ISIL_DEV_CFG_S *sDevInfo:设备基本配置信息
 *output:
 *      无
 *return:
 *       Success: 0, Fail: -1
 *******************************************************/

S32 ISIL_SetDevBaseInfo(ISIL_DEV_BASE_CFG_S *sDevInfo)
{
	S32 iRet;
	S32 iType;
	if(sDevInfo == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], dev base pointer null\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
    /*设置通道数目*/
	#ifndef ISIL_USE_SYS_CFG_FILE
    sDevInfo->cChannelCount = get_media_max_chan_number();
	#else
	sDevInfo->cChannelCount = ISIL_GetSysMaxChnCnt();
	#endif

	iRet = ISIL_WriteConfigFile(DEV_BASE_CONFIG_FILE, 0, sizeof(ISIL_DEV_BASE_CFG_S),
		&iType, sDevInfo);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], write dev base cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

static S32 ISIL_SetDevBaseDefault()
{
	S32 iRet;
	iRet = ISIL_SetDevBaseInfo(&ISIL_DevBaseCfg);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "Set dev base default cfg fail\n");
		return ISIL_FAIL;
	}

	return ISIL_SUCCESS;
}

/*********************************************************
 *Function: ISIL_GetDevBaseInfo
 *Description:
 *	从配置文件中获得设备基本配置信息
 *Input:
 *	ISIL_DEV_CFG_S *sDevInfo:放置设备配置基本信息
 *	的指针
 *output:
 * 	ISIL_DEV_CFG_S *sDevInfo:放置设备配置基本信息
 *	的指针
 *return:
 *       Success: 0, Fail: -1
 *******************************************************/

S32 ISIL_GetDevBaseInfo(ISIL_DEV_BASE_CFG_S *sDevInfo)
{
	S32 iRet;
	S32 iType;
	if(sDevInfo == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], dev base pointer null\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}


    iRet = ISIL_FileExit(DEV_BASE_CONFIG_FILE);
	if(iRet == ISIL_FALSE)
	{
		iRet = ISIL_SetDevBaseDefault();
		if(iRet == ISIL_FAIL)
		{
			memcpy(sDevInfo, &ISIL_DevBaseCfg, sizeof(ISIL_DEV_BASE_CFG_S));
			return ISIL_SUCCESS;
		}
	}

	iRet = ISIL_ReadConfigFile(DEV_BASE_CONFIG_FILE, 0, sizeof(ISIL_DEV_BASE_CFG_S),
		&iType, sDevInfo);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], read dev base cfg fail\n", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}
	#ifndef ISIL_USE_SYS_CFG_FILE
    sDevInfo->cChannelCount = get_media_max_chan_number();
	#else
	sDevInfo->cChannelCount = ISIL_GetSysMaxChnCnt();
	#endif
	return ISIL_SUCCESS;
}







