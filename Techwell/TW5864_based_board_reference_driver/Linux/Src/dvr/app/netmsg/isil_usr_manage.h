/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_common.h
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:define user information.


	Other:
	Function list:
	History:

*******************************************************************
*/
#ifndef _ISIL_USR_MANAGE_H_
#define _ISIL_USR_MANAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "isil_type.h"
#include "isil_common.h"

#define ISIL_USER_INFO_FILE   "/flash/config/user/userinfo"
#define ISIL_USER_GROUP_FILE   "/flash/config/user/usergroup"



#define ADMIN_USR_ID		0
#define USER_START_ID 		500
#define ISIL_DEFAULT_GROUP	0
#define ISIL_MAX_GROUPNUMBER 32

#define ISIL_MAX_LEVEL		256 /*256*8个权限*/
#define ISIL_USR_LEVEL_BIT	8


/*************************ISIL user CFG parameter begin***************************/
/*ISIL USR Config*/
#ifdef MANAGE_USER_SIGNAL
typedef struct
{
    U08 lLevel[ISIL_MAX_LEVEL];
	S08 sName[ISIL_NAME_LEN];
	S08 sPassword[ISIL_PASSWORD_LEN];
	S08 sUsrGrpName[ISIL_NAME_LEN];
}ISIL_USR_NET_CFG;
#endif
typedef struct _ISIL_USR_CFG_STRU
{
    #ifdef MANAGE_USER_SIGNAL
    ISIL_USR_NET_CFG stUsrNetCfg;
    S32 iUId;
	S32 iGId;
	#endif
	U32 lUserID;
	U32 lGroupID;
	U08 cLevel[ISIL_MAX_LEVEL];
	S08 sName[ISIL_NAME_LEN];
	S08 sPassword[ISIL_PASSWORD_LEN];
}ISIL_USR_CFG_S;

typedef struct _ISIL_USR_LOGIN_STRU
{
	S08 sName[ISIL_NAME_LEN];
	S08 sPassword[ISIL_PASSWORD_LEN];
	char szDeviceName[ISIL_NAME_LEN];       /*设备名称*/
	unsigned long lDeviceID;
}ISIL_USR_LOGIN_S;

#ifdef MANAGE_USER_SIGNAL

typedef struct
{
	S08 sName[ISIL_NAME_LEN];
	S08 sPassword[ISIL_PASSWORD_LEN];
	U08 iLevel[ISIL_MAX_LEVEL];
	S32 iUsrCnt;
	S08 cGropList[ISIL_MAX_GROUPNUMBER][ISIL_NAME_LEN];
}ISIL_NET_USR_GRP_S;
#endif

typedef struct _ISIL_USR_GROUP_STRU
{
	#ifdef MANAGE_USER_SIGNAL
	ISIL_NET_USR_GRP_S stNetUsrGrp;
	S32 iGroupID;
	#endif
	U32 lGroupID;
	U08 cLevel[ISIL_MAX_LEVEL];
	S08 sGroupName[ISIL_NAME_LEN];
	S08 sGroupPassword[ISIL_PASSWORD_LEN];
}ISIL_USR_GROUP_S;

/**************************ISIL usr CFG PARAMETER end****************************/

extern S32 ISIL_VerifyUsrInfo(ISIL_USR_LOGIN_S *sUsrLogin);
extern S32 ISIL_AddUser(ISIL_USR_CFG_S* sUsrInf);
extern S32 ISIL_DelUser(S08* sUsrName);
extern S32 ISIL_ModifyUser(S08 *sUsrName, ISIL_USR_CFG_S * sUsrInf);
extern S32 ISIL_GetUsrInfoByName(S08 *sName, ISIL_USR_CFG_S* sUsrInf);
extern S32 ISIL_GetUsrInfoByID(U32 uiUsrId, ISIL_USR_CFG_S* sUsrInf);
extern S32 ISIL_GetUsrInfoCnt();
extern S32 ISIL_AddUsrGroup(ISIL_USR_GROUP_S* sUsrGrp);
extern S32 ISIL_GetUsrGroupInfoByName(S08* sUsrGrpName, ISIL_USR_GROUP_S* sGrp);
extern S32 ISIL_GetUsrGroupInfoByID(U32 sUsrGrpID, ISIL_USR_GROUP_S* sGrp);
extern S32 ISIL_ModifyUsrGroup(S08* sUsrGrpName, ISIL_USR_GROUP_S* sGrp);
extern S32 ISIL_DelUsrGroup(S08* sUsrGrpName);
extern S32 ISIL_AddUsrToGroup(S08 *sUsrName, S08 *sGrpName);
extern S32 ISIL_DelUsrFromGroup(S08 *sUsrName, S08 *sGrpName);
extern S32 ISIL_UsrLevelMatch(U32 uiUsrID, S32 iLevel);
extern S32 ISIL_InitUsrCfgFile();

extern S32 ISIL_SaveAllUserInfo(void *pAllInfo, S32 iInfoSize, S32 iInfoType);
extern S32 ISIL_GetAllUserInfo(void *pAllInfo, S32 iInfoSize, S32 iInfoType);
extern S32 ISIL_GetAllUsrInfoSize();
extern S32 ISIL_GetAllUsrGrpInfoSize();

#ifdef __cplusplus
}
#endif
#endif

