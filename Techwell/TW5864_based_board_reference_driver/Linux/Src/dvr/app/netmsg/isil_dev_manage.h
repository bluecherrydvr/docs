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

#ifndef _ISIL_DEV_MANAGE_H_
#define _ISIL_DEV_MANAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "isil_type.h"
#include "isil_common.h"
#include "isil_net_manage.h"
#include "isil_usr_manage.h"
#include "isil_ptz_manage.h"

#define DEV_BASE_CONFIG_FILE   "/flash/config/device/device_base_config"
#define DEV_CONFIG_FILE   "/flash/config/device/device_config"

#if 0

/**************************ISIL time CFG parameter begin****************************/

/*Time struct*/
typedef struct _ISIL_TIME_STRU
{
	U16 nYear;
	U08 cMonth;
	U08 cDay;
	U08 cHour;
	U08 cMinute;
	U08 cSecond;
	U08 cReserve;
}ISIL_TIME_S;
/**************************ISIL time CFG parameter end****************************/


/**************************ISIL base CFG parameter begin****************************/

/*Device base config*/
typedef struct _ISIL_DEV_BASE_CFG_STRU
{
	S08 sDeviceName[ISIL_NAME_LEN];
	S08 sSerialNumber[ISIL_SERIALNO_LEN];
	U32 lDeviceID;
	/*
	*******************************************************************

	      b31                                              b15                                                   b0
		|----------------------------|-------------------------------|
			SW main ver number                 SW sub ver numer

	******************************************************************
	*/
	U32 lSoftwareVersion;
	U32 lSoftwareBuildDate;

	/*
	******************************************************************

	      b31                                              b15                                                   b0
		|----------------------------|-------------------------------|
			DSP main ver number                             DSP sub ver numer

	******************************************************************
	*/
	U32 lDSPSoftwareVersion;
	U32 lDSPSoftwareBuildDate;
	U32 lPanelVersion;

	/*
	******************************************************************

	      b31                                              b15                                                   b0
		|----------------------------|-------------------------------|
			HW main ver number                             HW sub ver numer

	******************************************************************
	*/
	U32 lHardwareVersion;
	U32 lDeviceClass;
	U32 lDeviceType;

	U08 cChannelCount;
	U08 cAlarmInCount;
	U08 cOutCtrlCount;
	U08 cRS232Count;
	U08 cRS485Count;
	U08 cNetworkPortCount;
	U08 cDiskCtrlCount;
	U08 cDiskCount;

	U08 cDecordCount;
	U08 cVGACount;
	U08 cUSBCount;
	U08 cStandard;
	ISIL_TIME_S stSysTime;
	S08 ReservedData[6];
}ISIL_DEV_BASE_CFG_S;
/**************************ISIL base CFG parameter end****************************/



/********************ISIL DEV CFG parameter begin***********************/

/*ISIL DEV Config*/
typedef struct _ISIL_DEV_CFG_STRU
{
	U08 cDeviceBaseEnable;
	U08 cNetEnable;
	U08 cPTZProEnable;
	U08 cReserve;
	ISIL_DEV_BASE_CFG_S stDeviceBase;
	ISIL_NET_CFG_S stNet;
	ISIL_PTZ_PROTOCOL_S stPTZProtocol[ISIL_PTZ_PROTOCOL_MAX];
}ISIL_DEV_CFG_S;
/***********************ISIL DEV CFG parameter end*************************/
#endif

extern S32 ISIL_GetDevInfo(ISIL_DEV_CFG_S *sDevInfo);
extern S32 ISIL_SetDevInfo(ISIL_DEV_CFG_S *sDevInfo);

extern S32 ISIL_GetDevBaseInfo(ISIL_DEV_BASE_CFG_S *sDevInfo);
extern S32 ISIL_SetDevBaseInfo(ISIL_DEV_BASE_CFG_S *sDevInfo);

extern U32 ISIL_GetChCnt();
extern U32 ISIL_GetOutCtlCnt();
extern S32 ISIL_GetComRS232Cnt();
extern S32 ISIL_GetComRS485Cnt();


#ifdef __cplusplus
}
#endif
#endif

