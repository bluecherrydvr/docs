
#include <stdio.h>
#include <stdlib.h>

#include "isil_alarm_manage.h"
#include "isil_common.h"
#include "isil_debug.h"
#include "isil_cfg_file_manage.h"
//#include "isil_fm_msg.h"
#include "isil_dev_manage.h"
//#include "isil_pel_gpio.h"

#define ALARM_CFG "/flash/config/alarm/"
static S08 g_alarm_path[ISIL_CFG_PATH_LEN];
static S08* g_alarm_class[] =
{
    "alarm_in_",
    "alarm_out_",
    "alarm_linkage_",
    "alarm_status_",
    ""
};
static S08* g_alarm_type[] =
{
    "motion_",
    "shelter_",
    "lost_",
    "alarmin_",
    ""
};
static S08* g_linkage_type[] =
{
    "pc_",
    "outctl_",
    "ptz_",
    "rec_",
    "shoot_",
    ""
};



static S08* ISIL_GetAlarmPath(U32 iChannel, S32 iAlarmClass, S32 iAlarmType,
                             S32 iLinkageType)
{
	S08 *pTemp = NULL;
	memset(g_alarm_path, 0, ISIL_CFG_PATH_LEN);
	sprintf(g_alarm_path, "%s%s%s%sch%d", ALARM_CFG,
	        g_alarm_class[iAlarmClass],
	        g_alarm_type[iAlarmType-1],
	        g_linkage_type[iLinkageType],
	        iChannel);
	return g_alarm_path;
}

/********************************************************
 *Function: ISIL_SaveVideoCfg
 *Description: 保存告警相关的配置文件
 *Input:
 *	S32 iChannel:通道号
 *	S32 iVideoCfgType:报警配置类型:
 *				 	ALARM_IN    1
 *				 	ALARM_OUT 2
 *	S32 iCfgSize:报警配置结构的大小
 *	void * pVideoCfg:需要保存的数据
 *output:
 *       No
 *return:
 *        Success, 1; FALAE, -1
 *******************************************************/

S32 ISIL_SaveAlarmCfg(U32 iChannel, S32 iAlarmClss, S32 iAlarmType, S32 iLinkageType,
                        S32 iCfgSize, void * pAlarmCfg)
{
	S08 *sPath;
	S32 iRet;
	S32 iType = 1;

	if(NULL == pAlarmCfg)
	{
		return ISIL_FAIL;
	}
	sPath = ISIL_GetAlarmPath(iChannel, iAlarmClss, iAlarmType, iLinkageType);
	ISIL_TRACE(BUG_NORMAL, "[%s]path:%s", __FUNCTION__, sPath);
	if(NULL != sPath)
	{
		iRet = ISIL_WriteConfigFile(sPath, 0, iCfgSize, &iType, pAlarmCfg);
		if(iRet < 0)
		{
			return ISIL_FAIL;
		}
		return ISIL_SUCCESS;
	}
	return ISIL_FAIL;
}

/********************************************************
 *Function: ISIL_SaveVideoCfg
 *Description: 获得告警相关的配置文件
 *Input:
 *	S32 iChannel:通道号
 *	S32 iVideoCfgType:报警配置类型:
 *				 	ALARM_IN    1
 *				 	ALARM_OUT 2
 *	S32 iCfgSize:报警配置结构的大小
 *	void * pVideoCfg:需要保存的数据
 *output:
 *       No
 *return:
 *        Success, 1; FALAE, -1
 *******************************************************/

S32 ISIL_GetAlarmCfg(U32 iChannel, S32 iAlarmClass, S32 iAlarmType, S32 iLinkageType,
                        S32 iCfgSize, void *pAlarmCfg)
{
	S08 *sPath;
	S32 iRet;
	S32 iType = 0;

	if(NULL == pAlarmCfg)
	{
		return ISIL_FAIL;
	}
	sPath = ISIL_GetAlarmPath(iChannel, iAlarmClass, iAlarmType, iLinkageType);
	iRet = ISIL_GetFileSize(sPath);
	ISIL_TRACE(BUG_NORMAL, "[%s] path %s, filesize %d", __FUNCTION__, sPath, iRet);

	if(NULL != sPath)
	{
		iRet = ISIL_ReadConfigFile(sPath, 0, iCfgSize, &iType, pAlarmCfg);
		if(iRet < 0)
		{
			return ISIL_FAIL;
		}
		return ISIL_SUCCESS;
	}
	return ISIL_FAIL;
}

S32 ISIL_AlarmLinkageCfgHandle(U32 Channel, S32 iAlarmType, U32 lLinkageType,
                U32 lIoFlag, S32 iLinkageSize, void* pLinkage)
{
    S32 iRet;
    //return 0;
    if(NULL == pLinkage)
    {
        return ISIL_FAIL;
    }
    ISIL_TRACE(BUG_NORMAL, "[%s] alarm type %d, linkage type %d, ioflag %d", __FUNCTION__,
                iAlarmType, lLinkageType, lIoFlag);


    if(ISIL_GET_PARAMETER == lIoFlag)
    {
        iRet = ISIL_GetAlarmCfg(Channel, ALARM_LINKAGE, iAlarmType,lLinkageType,
                       iLinkageSize, pLinkage);

    }
    else if(ISIL_SET_PARAMETER == lIoFlag)
    {
        iRet = ISIL_SaveAlarmCfg(Channel, ALARM_LINKAGE,iAlarmType,lLinkageType,
                iLinkageSize, pLinkage);
    }
    return iRet;
}

S32 ISIL_GetPcEventSize(U32 iChannel,S32 iAlarmType)
{
    S08* sPath;
    //S32 iPcEvCnt = 0;
    S32 iRet = 0;
    sPath = ISIL_GetAlarmPath(iChannel, ALARM_LINKAGE,
                iAlarmType, ISIL_LINKAGE_TYPE_PC_EVENT);
    if(NULL != sPath)
    {
        iRet = ISIL_GetFileSize(sPath);
    }
    return iRet;
}
/*
*设置通道告警相关的联动为活动状态
*
*/

S32 ISIL_HandleOutCtrlLinkage(void* pLinkageInfo)
{
    U32 lOutCtlCnt;
    ISIL_LINKAGE_OUTCTRL_CFG* pOutCtl = NULL;
    if(NULL == pLinkageInfo)
    {
        return ISIL_FAIL;
    }

    lOutCtlCnt = ISIL_GetOutCtlCnt();
    pOutCtl =(ISIL_LINKAGE_OUTCTRL_CFG *) pLinkageInfo;
    /*TODO:进行联动输出处理*/
	return 0;
}

S32 ISIL_HandlePtzLinkage(void* pLinkageInfo)
{
    U32 lChCnt;
    U32 iLoop;
    ISIL_LINKAGE_PTZ_CFG_S* pPtz = NULL;
    if(NULL == pLinkageInfo)
    {
        return ISIL_FAIL;
    }
    lChCnt = ISIL_GetChCnt();
    pPtz = (ISIL_LINKAGE_PTZ_CFG_S*)pLinkageInfo;
    /*Todo, 进行启动PTZ设置*/
    for(iLoop = 0; iLoop < lChCnt; iLoop++)
    {
        /*处理PTZ联动*/
        if(pPtz->cEnablePTZ)
        {
            ;/*设置PTZ*/
        }
        pPtz++;/*指向下一个设置*/
    }
	return 0;
}

S32 ISIL_SetAlarmInParameter(S32 gpio_no, S32 val)
{

    #if 0
    ISIL_PEL_GPIO gpio;
    S32 nk_fd;
    S32 ret;
    //nk_fd = ISIL_GetNetLinkFd();
    ISIL_TRACE(BUG_ERROR, "%s, nkfd %d io no %d, val %d", __FUNCTION__, nk_fd, gpio_no, val);
    if(nk_fd < 0 || gpio_no < 0)
    {
        ISIL_TRACE(BUG_ERROR, "%s, nkfd %d no %d error", __FUNCTION__, nk_fd, gpio_no);
        return ISIL_FAIL;
    }
    gpio.s32Gpio = gpio_no;
    gpio.s32GpioVal = val;
    ret = ISIL_PEL_GPIO_Set(nk_fd, gpio);
    return ret;
    #endif
	return 0;
}


