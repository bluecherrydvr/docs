/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_ptz_manage.h
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:handle ptz information.


	Other:
	Function list:
	History:

*******************************************************************
*/

#ifndef _ISIL_PTZ_MANAGE_H_
#define _ISIL_PTZ_MANAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "isil_type.h"
#include "isil_common.h"
#include "isil_messagestruct.h"

#define RS485_CONFIG_FILE  "/flash/config/RS485/ptz_config_"

#define ISIL_PTZ_ENABLE			1
#define ISIL_PTZ_UNABLE			0

#define ISIL_PTZ_SEND_BUFF_LEN	16

/*PTZ control command*/
#define ISIL_PTZ_UP				1
#define ISIL_PTZ_DOWN			2
#define ISIL_PTZ_LEFT			3
#define ISIL_PTZ_RIGHT			4
#define ISIL_PTZ_FOCUSADD		5
#define ISIL_PTZ_FOCUSSUB		6
#define ISIL_PTZ_IRISADD			7
#define ISIL_PTZ_IRISSUB			8
#define ISIL_PTZ_ZOOMADD			9
#define ISIL_PTZ_ZOOMSUB			10
#define ISIL_PTZ_AUTOOPEN		11
#define ISIL_PTZ_AUTOCLOSE		12
#define ISIL_PTZ_LAMPOPEN		13
#define ISIL_PTZ_LAMPCLOSE		14
#define ISIL_PTZ_BRUSHOPEN		15
#define ISIL_PTZ_BRUSHCLOSE		16
#define ISIL_PTZ_WATEROPEN		17
#define ISIL_PTZ_WATERCLOSE		18
#define ISIL_PTZ_PRESET			19
#define ISIL_PTZ_CALL			20
#define ISIL_PTZ_STOP			21
#define ISIL_PTZ_UP_STOP			30
#define ISIL_PTZ_DOWN_STOP		31
#define ISIL_PTZ_LEFT_STOP		32
#define ISIL_PTZ_RIGHT_STOP		33
#define ISIL_PTZ_FOCUSADD_STOP	34
#define ISIL_PTZ_FOCUSSUB_STOP	35
#define ISIL_PTZ_IRISADD_STOP	36
#define ISIL_PTZ_IRISSUB_STOP	37
#define ISIL_PTZ_ZOOMADD_STOP	38
#define ISIL_PTZ_ZOOMSUB_STOP	39

/*PTZ protocol type*/
#define PTZ_PROTOCOL_NONE	0
#define PTZ_PROTOCOL_PELCOD	1
#define PTZ_PROTOCOL_PELCOP	2
#define PTZ_PROTOCOL_SONY	3

typedef S32 (*pFun)(U08);

extern S32 ISIL_GetPTZProtocol(U32 lChannel, ISIL_PTZ_PROTOCOL_S * ptzProtocol);
extern S32 ISIL_SetPTZContrl(ISIL_PTZ_CTRL_S* ptzCtrl);
extern S32 ISIL_SetPTZInfo(ISIL_PTZ_CFG_S* ptzCfg, S32 channel);
extern S32 ISIL_GetPTZInfo(ISIL_PTZ_CFG_S* ptzCfg, S32 channel);


#ifdef __cplusplus
}
#endif
#endif

