/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_usr_information.c
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:handle user information.


	Other:
	Function list:
	History:

*******************************************************************
*/
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/types.h>

#include "isil_usr_manage.h"
#include "isil_debug.h"
#include "isil_cfg_file_manage.h"

#define USER_DEFAULT_PWD	"123"


#ifdef MANAGE_USER_SIGNAL
#define LINE_BUF_LEN		1024

/*Define some function*/
S32 ISIL_VerifyUsrInfo(ISIL_USR_LOGIN_S *sUsrLogin);
S32 ISIL_AddUser(ISIL_USR_CFG_S* sUsrInf);
S32 ISIL_DelUser(S08* sUsrName);
S32 ISIL_ModifyUser(S08 *sUsrName, ISIL_USR_CFG_S * sUsrInf);
S32 ISIL_GetUsrInfoByName(S08 *sName, ISIL_USR_CFG_S* sUsrInf);
S32 ISIL_GetUsrInfoByID(U32 uiUsrId, ISIL_USR_CFG_S* sUsrInf);
S32 ISIL_GetUsrInfoCnt(void);

S32 ISIL_AddUsrGroup(ISIL_USR_GROUP_S* sUsrGrp);
S32 ISIL_GetUsrGroupInfoByName(S08* sUsrGrpName, ISIL_USR_GROUP_S* sGrp);
S32 ISIL_GetUsrGroupInfoByID(U32  uiUsrGrpID, ISIL_USR_GROUP_S* sGrp);
S32 ISIL_ModifyUsrGroup(S08* sUsrGrpName, ISIL_USR_GROUP_S* sGrp);
S32 ISIL_DelUsrGroup(S08* sUsrGrpName);
S32 ISIL_AddUsrToGroup(S08 *sUsrName, S08 *sGrpName);
S32 ISIL_DelUsrFromGroup(S08 *sUsrName, S08 *sGrpName);
S32 ISIL_ChangeUsrNameInGrp(S08* sGrpName, S08 * sOldName, S08 * sNewName);
S32 ISIL_UsrLevelMatch(U32 uiUsrID, S32 iLevel);


S32 ISIL_UsrInfoExit(S08 *sUsrName)
{
	S32 iDataSize;
	S32 iLoop = 0;
	ISIL_USR_CFG_S sUsrCfg;
	FILE *fp;
	if(sUsrName == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] name null", __FUNCTION__);
		return ISIL_FAIL;
	}
	if ((fp = fopen(ISIL_USER_INFO_FILE, "r")) == NULL)
	{
		return ISIL_FAIL;
	}

	iDataSize = sizeof(ISIL_USR_CFG_S);
	while((fread(&sUsrCfg, iDataSize, 1, fp)) > 0)
	{
		if((strcmp(sUsrCfg.stUsrNetCfg.sName, sUsrName)) == 0)/*found*/
		{
			fclose(fp);
			return iLoop;
		}
		iLoop++;
	}

	fclose(fp);
	return ISIL_FAIL;
}

/********************************************************
 *Function: ISIL_GetUsrInfoCnt
 *Description:  在用户配置文件中，查找有多少用户
 *			配置项
 *Input:
 *	无
 *output:
 *       无
 *return:
 *        Success: 配置项数; Fail : -1
 *******************************************************/

S32 ISIL_GetUsrInfoCnt()
{
	S32 iLoop = 1;
	S32 iDataSize;
	FILE *fp;
	ISIL_USR_CFG_S sUsrIfo;
	if ((fp = fopen(ISIL_USER_INFO_FILE, "r")) == NULL)
	{
		return ISIL_FAIL;
	}
	iDataSize = sizeof(ISIL_USR_CFG_S);
	while((fread(&sUsrIfo, iDataSize, 1, fp)) > 0)
	{
		iLoop++;
	}
	ISIL_TRACE(BUG_NORMAL, "[%s] user information count %d", __FUNCTION__, iLoop);
	fclose(fp);
	return iLoop;
}


/********************************************************
 *Function: ISIL_GetLastUsrID
 *Description:从用户配置项中获得最后一个配置
 			的用户ID
 *Input:
 *	无
 *output:
 *    无
 *return:
 *        Success: 最后配置项的 用户ID; Fail : -1
 *******************************************************/

S32 ISIL_GetLastUsrID()
{
	S32 iRet;
	S32 iDataSize;
	ISIL_USR_CFG_S sUsrCfg;
	FILE *fp;

	/*空文件*/
    if(ISIL_GetFileSize(ISIL_USER_INFO_FILE) == 0)
    {
        return 0;
    }

	if ((fp = fopen(ISIL_USER_INFO_FILE, "r")) == NULL)
	{
		return ISIL_FAIL;
	}
	iDataSize = sizeof(ISIL_USR_CFG_S);
	iRet = fseek(fp, -iDataSize, SEEK_END);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] seek fail", __FUNCTION__);
		fclose(fp);
		return ISIL_FAIL;
	}

	iRet = fread(&sUsrCfg, iDataSize, 1, fp);
	if(iRet > 0)
	{
		fclose(fp);
		return sUsrCfg.iUId;
	}

	fclose(fp);
	return ISIL_FAIL;
}


/********************************************************
 *Function: ISIL_AddUser
 *Description: 增加一个用户
 *Input:
 *	ISIL_USR_CFG_S* sUsrInf:增加的用户信息的指针
 *output:
 *       无
 *return:
 *        Success: 0; Fail : -1
 *******************************************************/

S32 ISIL_AddUser(ISIL_USR_CFG_S* sUsrInf)
{

	S32 iRet;
	S32 iDataSize;
	FILE *fp;
	iRet = ISIL_UsrInfoExit(sUsrInf->stUsrNetCfg.sName);
	if(iRet != ISIL_FAIL)/*已经有该用户*/
	{
		ISIL_TRACE(BUG_ERROR, "[%s] user %s exit", __FUNCTION__, sUsrInf->stUsrNetCfg.sName);
		return ISIL_FAIL;
	}

	if ((fp = fopen(ISIL_USER_INFO_FILE, "a+")) == NULL)
	{
		return ISIL_FAIL;
	}
	iDataSize = sizeof(ISIL_USR_CFG_S);
	iRet = fwrite(sUsrInf, iDataSize, 1, fp);
	fclose(fp);
	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
	return ISIL_SUCCESS;
}

/********************************************************
 *Function: ISIL_AddUser
 *Description: 增加一个管理员用户
 *Input:
 *	ISIL_USR_CFG_S* sUsrInf:增加的用户信息的指针
 *output:
 *       无
 *return:
 *        Success: 0; Fail : -1
 *******************************************************/

S32 ISIL_AddAdminUser(ISIL_USR_CFG_S* sUsrInf)
{
	S32 iRet;
	sUsrInf->iGId = 0;
	sUsrInf->iUId = ADMIN_USR_ID;
	memset(sUsrInf->stUsrNetCfg.lLevel, 0xff,
        sizeof(sUsrInf->stUsrNetCfg.lLevel));
	iRet = ISIL_AddUser(sUsrInf);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] add admin %s fail", __FUNCTION__, sUsrInf->stUsrNetCfg.sName);
		return ISIL_FAIL;
	}
	return ISIL_SUCCESS;
}

/********************************************************
 *Function: ISIL_AddUser
 *Description: 增加一个普通用户
 *Input:
 *	ISIL_USR_CFG_S* sUsrInf:增加的用户信息的指针
 *output:
 *       无
 *return:
 *        Success: 0; Fail : -1
 *******************************************************/


S32 ISIL_AddPublicUser(ISIL_USR_CFG_S* sUsrInf)
{
	S32 iLastUsrID;
	S32 iRet;
	iLastUsrID = ISIL_GetLastUsrID();

	ISIL_TRACE(BUG_ERROR, "[%s] last uid %d", __FUNCTION__, iLastUsrID);
	/*无法获得当前增加用户的ID*/
	if(iLastUsrID < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] last uid error", __FUNCTION__);
		return ISIL_FAIL;
	}
	else if(iLastUsrID == 0)
	{
		sUsrInf->iUId = USER_START_ID;
	}
	else
	{
		sUsrInf->iUId = iLastUsrID + 1;
	}

	iRet = ISIL_AddUser(sUsrInf);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] add public %s fail", __FUNCTION__, sUsrInf->stUsrNetCfg.sName);
		return ISIL_FAIL;
	}

    /*增加用户到用户组*/
    if(NULL != sUsrInf->stUsrNetCfg.sUsrGrpName)
    {
    	iRet = ISIL_AddUsrToGroup(sUsrInf->stUsrNetCfg.sName,
			sUsrInf->stUsrNetCfg.sUsrGrpName);
		if(iRet < 0)
		{
			return ISIL_FAIL;
		}
    }
	return ISIL_SUCCESS;
}

/********************************************************
 *Function: ISIL_DelUser
 *Description: 根据用户名，删除一个用户
 *Input:
 *	S08* sUsrName:所要删除的用户名指针
 *output:
 *       无
 *return:
 *        Success: 0; Fail : -1
 *******************************************************/

S32 ISIL_DelUser(S08* sUsrName)
{
	FILE *fp, *tmp;
	S08 tmpname[ISIL_PATHNAME_LEN];
	S32 iLine = 0;
	S32 iRet;
	S32 iDataSize;
	ISIL_USR_CFG_S sUsrCfg;

	if(NULL == sUsrName)
	{
		return ISIL_FAIL;
	}
	/*查找该用户是否存在*/
	iRet = ISIL_GetUsrInfoByName(sUsrName, &sUsrCfg);
	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
	/*删除用户组中该用户信息*/
	iRet = ISIL_DelUsrFromGroup(sUsrName, sUsrCfg.stUsrNetCfg.sUsrGrpName);
	if(iRet < 0)
	{
		return ISIL_FAIL;
	}

	/*找到用户在文件中的位置*/
	iLine = ISIL_UsrInfoExit(sUsrName);
	if(ISIL_FAIL == iLine)/*user not exit*/
	{
		ISIL_TRACE(BUG_ERROR, "[%s] User %s not exit", __FUNCTION__, sUsrName);
		return ISIL_FAIL;
	}

	sprintf(tmpname, "%s%s", ISIL_USER_INFO_FILE, "tmp");
	/*备份文件*/
	if (ISIL_FileCP(ISIL_USER_INFO_FILE, tmpname) != ISIL_TRUE)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] copy file fail", __FUNCTION__);
		return ISIL_FAIL;
	}
	if ((tmp = fopen(tmpname, "r+")) == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] open temp file fail", __FUNCTION__);
		return ISIL_FAIL;
	}
	iDataSize = sizeof(ISIL_USR_CFG_S);
	unlink(tmpname);
	fseek(tmp, iDataSize*iLine, SEEK_SET);
	truncate(ISIL_USER_INFO_FILE, ftell(tmp));

	if((fp = fopen(ISIL_USER_INFO_FILE, "a")) == NULL)
	{
		fclose(tmp);
		return ISIL_FAIL;
	}

	fseek(fp, 0, SEEK_END);	/* to last place */
	fseek(tmp, iDataSize*(iLine+1), SEEK_SET);


	while((fread(&sUsrCfg, iDataSize, 1, tmp)) > 0)
	{
		fwrite(&sUsrCfg, iDataSize, 1, fp);
	}

	fclose(fp);
	fclose(tmp);

	return ISIL_SUCCESS;
}


/********************************************************
 *Function: ISIL_ModifyUser
 *Description: 根据用户名，修改一个用户信息
 *Input:
 *	S08* sUsrName:所要修改的用户名指针
 *	ISIL_USR_CFG_S * sUsrInf:用户信息指针
 *output:
 *       无
 *return:
 *        Success: 0; Fail : -1
 *******************************************************/

S32 ISIL_ModifyUser(S08 *sUsrName, ISIL_USR_CFG_S * sUsrInf)
{
	S32 iRet = 0;
	S32 iDataSize;
	S32 iLoc;
	FILE * fp;

	if(sUsrName == NULL || sUsrInf == NULL)
	{

		ISIL_TRACE(BUG_ERROR, "[%s] pointer is null", __FUNCTION__);
		return ISIL_FAIL;
	}

	/*查找用户是否存在*/
	iLoc = ISIL_UsrInfoExit(sUsrName);
	if(iLoc == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] not find %s", __FUNCTION__, sUsrName);
		return ISIL_FAIL;
	}

	ISIL_TRACE(BUG_ERROR, "[%s] find %s Loc %d", __FUNCTION__, sUsrName, iLoc);
	fp=fopen(ISIL_USER_INFO_FILE, "r+");
	if(fp == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] open file fail", __FUNCTION__);
		return ISIL_FAIL;
	}
	iDataSize = sizeof(ISIL_USR_CFG_S);

	iRet = fseek(fp, iLoc*iDataSize, SEEK_SET);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] Seek file fail", __FUNCTION__);
		fclose(fp);
		return ISIL_FAIL;
	}

	iRet = fwrite(sUsrInf, iDataSize, 1, fp);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] read file error", __FUNCTION__);
	}
	else
	{
		/*修改用户和用户组相关的信息*/
		if((strcmp(sUsrName, sUsrInf->stUsrNetCfg.sName)) == 0)
		{
			iRet = ISIL_ChangeUsrNameInGrp(sUsrInf->stUsrNetCfg.sUsrGrpName,
				sUsrName, sUsrInf->stUsrNetCfg.sName);
		}
	}
	fclose(fp);
	return iRet;
}



/********************************************************
 *Function: ISIL_UsrGrpExit
 *Description: 根据输入，判断用户组是否在配
 	置文件中
 *Input:
 *	S08 *cUsraGrpName:用户组名
 *output:
 *       No
 *return:
 *        Success,返回在在配置文件中的位置;
 *		-1: Fail
 *******************************************************/

S32 ISIL_UsrGrpExit(FILE* fp, S08 *cUsraGrpName)
{
	S32 iDatasize = 0;
	S32 iGropNo = 0;
	ISIL_USR_GROUP_S sGrp;

	if(NULL == cUsraGrpName)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] usr name null", __FUNCTION__);
		return ISIL_FAIL;
	}

	iDatasize = sizeof(ISIL_USR_GROUP_S);

	while(fread(&sGrp, iDatasize, 1, fp) > 0)
	{
		if(0 == strcmp(cUsraGrpName, sGrp.stNetUsrGrp.sName))
		{
			ISIL_TRACE(BUG_ERROR, "[%s] Group %s exit, No %d",
				__FUNCTION__, cUsraGrpName, iGropNo);
			return iGropNo;
		}
		iGropNo++;
	}
	return ISIL_FAIL;
}

/********************************************************
 *Function: ISIL_AddUsrGroup
 *Description: 根据输入，增加一个用户组
 *Input:
 *	S08 *cUsrName:用户组参数
 *output:
 *       No
 *return:
 *	Success, 0; fail, < 0
 *******************************************************/

S32 ISIL_AddUsrGroup(ISIL_USR_GROUP_S* sUsrGrp)
{
	S32 iRet;
	S32 iDataSize;
	S32 iFileSize;
	FILE * fp;
	ISIL_USR_GROUP_S sGrp;

	fp=fopen(ISIL_USER_GROUP_FILE, "r+");
	if(fp == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] open file fail", __FUNCTION__);
		return ISIL_FAIL;
	}

	/*检查用户组已经存在*/
	iRet = ISIL_UsrGrpExit(fp, sUsrGrp->stNetUsrGrp.sName);
	if(iRet >= 0)/*存在*/
	{
		ISIL_TRACE(BUG_ERROR, "[%s] group %s exit", __FUNCTION__,
			sUsrGrp->stNetUsrGrp.sName);
		fclose(fp);
		return ISIL_FAIL;
	}

	iFileSize = ISIL_GetFileSize(ISIL_USER_GROUP_FILE);
	if(iFileSize < 0)
	{
		fclose(fp);
		return ISIL_FAIL;
	}

	iDataSize = sizeof(ISIL_USR_GROUP_S);

	if(iFileSize > 0)
	{
		/*定位到最后一个用户组信息处*/
		iRet = fseek(fp, -iDataSize, SEEK_END);
		if(iRet == ISIL_FAIL)
		{
			ISIL_TRACE(BUG_ERROR, "[%s] Seek file fail", __FUNCTION__);
			fclose(fp);
			return ISIL_FAIL;
		}
		iRet = fread(&sGrp, iDataSize, 1, fp);
		if(iRet < 0)
		{
			ISIL_TRACE(BUG_ERROR, "[%s] read file error", __FUNCTION__);
			fclose(fp);
			return ISIL_FAIL;
		}
		sUsrGrp->iGroupID = sGrp.iGroupID + 1;
	}
	else
	{
		sUsrGrp->iGroupID = 0;
	}

	iRet =  fwrite((S08*)sUsrGrp, sizeof(ISIL_USR_GROUP_S), 1, fp);
	fclose(fp);

	ISIL_TRACE(BUG_NORMAL, "[%s] add grp %s ", __FUNCTION__, sUsrGrp->stNetUsrGrp.sName);
	return iRet;

}


/********************************************************
 *Function: ISIL_SyncUsrInfoByGrp
 *Description: 根据用户组信息的改变，同步到每个用户
 *Input:
 *	ISIL_USR_GROUP_S *stUsrGrp:用户组信息
 *	S32 iSynFlag: 0,修改；1，删除
 *output:
 *       No
 *return:
 *        Success :0
 *		  Fail	  :-1
 *******************************************************/

S32 ISIL_SyncUsrInfoByGrp(ISIL_USR_GROUP_S *stUsrGrp, S32 iSyncFlag)
{
	S32 iRet;
	S32 iLoop;
	S32 iLoop2;
	S32 iUsrCnt;
	S08 *pTemp;
	ISIL_USR_CFG_S sUsrInf;
	if(NULL == stUsrGrp)
	{
		return ISIL_FAIL;
	}
	iUsrCnt = stUsrGrp->stNetUsrGrp.iUsrCnt;
	ISIL_TRACE(BUG_NORMAL, "[%s] usr cnt %d", __FUNCTION__, iUsrCnt);
	for(iLoop = 0; iLoop < iUsrCnt; iLoop++)
	{
		pTemp = stUsrGrp->stNetUsrGrp.cGropList[iLoop];
		if(NULL != pTemp)
		{
			if(iSyncFlag == 0)/*修改*/
			{
				memset(&sUsrInf, 0, sizeof(ISIL_USR_CFG_S));
				iRet = ISIL_GetUsrInfoByName(pTemp, &sUsrInf);
				if(iRet < 0)
				{
					continue;
				}
				else
				{
					strncpy(sUsrInf.stUsrNetCfg.sUsrGrpName,
						stUsrGrp->stNetUsrGrp.sName, ISIL_NAME_LEN);

					for(iLoop2 = 0; iLoop2 < ISIL_MAX_LEVEL; iLoop2++)
					{
						sUsrInf.stUsrNetCfg.lLevel[iLoop2] |= stUsrGrp->stNetUsrGrp.iLevel[iLoop2];
					}
					ISIL_ModifyUser(pTemp, &sUsrInf);
				}
			}

			else if(iSyncFlag == 1)/*删除*/
			{
				ISIL_DelUser(pTemp);
			}
		}
	}
}

/********************************************************
 *Function: ISIL_ModifyUsrGroup
 *Description: 根据输入，修改用户组
 *Input:
 *	S08 *cUsrName:用户组名
 *	ISIL_USR_GROUP_S* sGrp:修改用户组信息指针
 *output:
 *       No
 *return:
 *        Success, 0
 *		   Fail,-1
 *******************************************************/

S32 ISIL_ModifyUsrGroup(S08* sUsrGrpName, ISIL_USR_GROUP_S* sGrp)
{
	S32 iRet = 0;
	S32 iDataSize;
	S32 iLoc;
	FILE * fp;

	if(sUsrGrpName == NULL || sGrp == NULL)
	{

		ISIL_TRACE(BUG_ERROR, "[%s] pointer is null", __FUNCTION__);
		return ISIL_FAIL;
	}
	fp=fopen(ISIL_USER_GROUP_FILE, "r+");
	if(fp == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] open file fail", __FUNCTION__);
		return ISIL_FAIL;
	}

	iLoc = ISIL_UsrGrpExit(fp, sUsrGrpName);
	if(iLoc == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] not find %s", __FUNCTION__, sUsrGrpName);
		fclose(fp);
		return ISIL_FAIL;
	}

	iDataSize = sizeof(ISIL_USR_GROUP_S);

	iRet = fseek(fp, iLoc*iDataSize, SEEK_SET);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] Seek file fail", __FUNCTION__);
		fclose(fp);
		return ISIL_FAIL;
	}

	iRet = fwrite(sGrp, iDataSize, 1, fp);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] read file error", __FUNCTION__);
	}
	/*同步改变成员信息*/
	//if()
	fclose(fp);
	return iRet;
}

/********************************************************
 *Function: ISIL_UsrGrpExit
 *Description: 根据输入，删除一个用户组
 *Input:
 *	S08 *cUsrName:用户组名
 *output:
 *       No
 *return:
 *        Success, 0
 *		   Fail,-1
 *******************************************************/

S32 ISIL_DelUsrGroup(S08* sUsrGrpName)
{
	FILE *fp, *tmp;
	S08 tmpname[ISIL_PATHNAME_LEN];
	S32 iLine = 0;
	S32 iRet;
	S32 iDataSize;
	ISIL_USR_GROUP_S sUsrGrp;

	if ((fp = fopen(ISIL_USER_GROUP_FILE, "r+")) == NULL)
	{
		fclose(fp);
		return ISIL_FAIL;
	}

	iLine = ISIL_UsrGrpExit(fp, sUsrGrpName);
	if(iLine == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] not find %s", __FUNCTION__, sUsrGrpName);
		fclose(fp);
		return ISIL_FAIL;
	}

	sprintf(tmpname, "%s%s", ISIL_USER_GROUP_FILE, "tmp");
	if (ISIL_FileCP(ISIL_USER_GROUP_FILE, tmpname) != ISIL_TRUE)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] copy file fail", __FUNCTION__);
		fclose(fp);
		return ISIL_FAIL;
	}

	if ((tmp = fopen(tmpname, "r+")) == NULL)
	{

		ISIL_TRACE(BUG_ERROR, "[%s] open temp file fail", __FUNCTION__);
		fclose(fp);
		return ISIL_FAIL;
	}
	iDataSize = sizeof(ISIL_USR_GROUP_S);
	unlink(tmpname);
	iRet = fseek(tmp, iLine*iDataSize, SEEK_SET);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d] seek temp file fail", __FUNCTION__, __LINE__);
		goto FAIL;
	}


	ISIL_TRACE(BUG_ERROR, "[%s %d] file part size %d", __FUNCTION__, __LINE__, ftell(tmp));
	truncate(ISIL_USER_GROUP_FILE, ftell(tmp));
	iRet = fseek(fp, 0, SEEK_END);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d] seek file fail", __FUNCTION__, __LINE__);
		goto FAIL;
	}
	iRet = fseek(tmp, (iLine+1)*iDataSize, SEEK_SET);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d] seek file fail", __FUNCTION__, __LINE__);
		goto FAIL;
	}
	while((fread(&sUsrGrp, iDataSize, 1, tmp)) > 0)
	{
		if((fwrite(&sUsrGrp, iDataSize, 1, fp)) < 0)
		{
			ISIL_TRACE(BUG_ERROR, "[%s %d] write file fail", __FUNCTION__, __LINE__);
			goto FAIL;
		}
	}
	/*需要删除组成员的相关信息?*/
	fclose(fp);
	fclose(tmp);
	return ISIL_SUCCESS;
	FAIL:
	fclose(fp);
	fclose(tmp);
	return ISIL_FAIL;
}
/********************************************************
 *Function: ISIL_UsrGrpExit
 *Description: 根据输入，在特定组中添加一个
 *			用户
 *Input:
 *	S08 *cUsrName:要删除的用户名
 *	S08 *sGrpName,用户组名
 *output:
 *       No
 *return:
 *        Success, 0
 *		   Fail,-1
 *******************************************************/

S32 ISIL_AddUsrToGroup(S08 *sUsrName, S08 *sGrpName)
{
	S32 iRet;
	S32 iLine;
	S32 iLoop;
	S32 iLevel;
	ISIL_USR_GROUP_S sGrp;
	ISIL_USR_CFG_S sUsr;
	FILE * fp;
	if(NULL == sGrpName || NULL == sGrpName)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] pointor is null", __FUNCTION__);
		return ISIL_FAIL;
	}
	/*检查用户是否存在*/
	/*iLine = ISIL_ItemIsInFileField(ISIL_USER_INFO_FILE, sUsrName, 1);*/
	iLine = ISIL_UsrInfoExit(sUsrName);
	if(ISIL_FAIL != iLine)/*user exit*/
	{
		ISIL_TRACE(BUG_NORMAL, "[%s] User %s exit", __FUNCTION__, sUsrName);
	}
	else/*User not exit*/
	{
		return ISIL_FAIL;
	}
	/*检查用户组是否存在*/
	fp = fopen(ISIL_USER_GROUP_FILE, "a+");
	if(fp == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] open file fail", __FUNCTION__);
		return ISIL_FAIL;
	}

	iRet = ISIL_UsrGrpExit(fp, sGrpName);

	if(iRet != ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] Usergroup %s exit", __FUNCTION__, sGrpName);
	}
	else
	{
		fclose(fp);
		return ISIL_FAIL;
	}
	iRet = ISIL_GetUsrGroupInfoByName(sGrpName, &sGrp);
	printf("[%s] ret %d , usrcnt %d\n", __FUNCTION__, iRet, sGrp.stNetUsrGrp.iUsrCnt);
	if(iRet != ISIL_FAIL)
	{
		for(iLoop = 0; iLoop < sGrp.stNetUsrGrp.iUsrCnt; iLoop++)
		{
			if((strcmp(sGrp.stNetUsrGrp.cGropList[iLoop], sUsrName)) == 0)
			{
				fclose(fp);
				return ISIL_FAIL;
			}
		}
		strncpy((sGrp.stNetUsrGrp.cGropList[sGrp.stNetUsrGrp.iUsrCnt]), sUsrName, ISIL_NAME_LEN);
		sGrp.stNetUsrGrp.iUsrCnt++;
		iRet = ISIL_ModifyUsrGroup(sGrpName, &sGrp);
		if(iRet == ISIL_FAIL)
		{	fclose(fp);
			return ISIL_FAIL;
		}
	}
	else
	{
		fclose(fp);
		return ISIL_FAIL;
	}

	iRet = ISIL_GetUsrInfoByName(sUsrName, &sUsr);

	if(iRet != ISIL_FAIL)
	{
		iLevel = sizeof(sUsr.stUsrNetCfg.lLevel) / sizeof(sUsr.stUsrNetCfg.lLevel[0]);
		ISIL_TRACE(BUG_NORMAL, "[%s] level size %d", __FUNCTION__, iLevel);
		for(iLoop = 0; iLoop < iLevel; iLoop++)
		{
			sUsr.stUsrNetCfg.lLevel[iLoop] |= sGrp.stNetUsrGrp.iLevel[iLoop];
		}
	}
	else
	{
		fclose(fp);
		return ISIL_FAIL;
	}

	iRet = ISIL_ModifyUser(sUsrName, &sUsr);
	fclose(fp);
	return iRet;

}
/********************************************************
 *Function: ISIL_UsrGrpExit
 *Description: 根据输入，在特定组中删除一个
 *			用户
 *Input:
 *	S08 *cUsrName:要删除的用户名
 *	S08 *sGrpName,用户组名
 *output:
 *       No
 *return:
 *        Success, 0
 *		   Fail,-1
 *******************************************************/

S32 ISIL_DelUsrFromGroup(S08 *sUsrName, S08 *sGrpName)
{
	S32 iRet;
	S32 iLine;
	S32 iLoop;
	S32 iLoop2;
	S32 iLevel;
	ISIL_USR_GROUP_S sGrp;
	ISIL_USR_CFG_S sUsr;
	FILE * fp;
	if(NULL == sGrpName || NULL == sGrpName)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] pointor is null", __FUNCTION__);
		return ISIL_FAIL;
	}
	/*检查用户是否存在*/
	/*iLine = ISIL_ItemIsInFileField(ISIL_USER_INFO_FILE, sUsrName, 1);*/
	iLine = ISIL_UsrInfoExit(sUsrName);
	if(0 != iLine)/*user exit*/
	{
		ISIL_TRACE(BUG_ERROR, "[%s] User %s exit", __FUNCTION__, sUsrName);
	}
	/*检查用户组是否存在*/
	fp=fopen(ISIL_USER_GROUP_FILE, "r+");
	if(fp == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] open file fail", __FUNCTION__);
		return ISIL_FAIL;
	}
	iRet = ISIL_UsrGrpExit(fp, sGrpName);
	if(iRet == ISIL_FAIL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] Usergroup %s not exit", __FUNCTION__, sGrpName);
		fclose(fp);
		return ISIL_FAIL;
	}

	iRet = ISIL_GetUsrGroupInfoByName(sGrpName, &sGrp);
	/*删除一个用户，并将其后边的用户向上移动*/
	if(iRet != ISIL_FAIL)
	{
		for(iLoop = 0; iLoop < sGrp.stNetUsrGrp.iUsrCnt; iLoop++)
		{
			if((strcmp(sGrp.stNetUsrGrp.cGropList[iLoop], sGrpName)) == 0)
			{
				for(iLoop2 = iLoop; iLoop2 < sGrp.stNetUsrGrp.iUsrCnt; iLoop2++)
				{
					strncpy(sGrp.stNetUsrGrp.cGropList[iLoop2],
						sGrp.stNetUsrGrp.cGropList[iLoop2+1], ISIL_NAME_LEN);
				}

				break;
			}
		}
		sGrp.stNetUsrGrp.iUsrCnt--;
	}
	else
	{
		fclose(fp);
		return ISIL_FAIL;
	}

	iRet = ISIL_GetUsrInfoByName(sUsrName, &sUsr);
	if(iRet != ISIL_FAIL)
	{
		iLevel = sizeof(sUsr.stUsrNetCfg.lLevel) / sizeof(sUsr.stUsrNetCfg.lLevel[0]);
		for(iLoop = 0; iLoop < iLevel; iLoop++)
		{
			sUsr.stUsrNetCfg.lLevel[iLoop] &= sGrp.stNetUsrGrp.iLevel[iLoop];

		}
	}
	else
	{
		fclose(fp);
		return ISIL_FAIL;
	}

	iRet = ISIL_ModifyUser(sUsrName, &sUsr);
	fclose(fp);
	return iRet;

}

S32 ISIL_ChangeUsrNameInGrp(S08* sGrpName, S08 * sOldName, S08 * sNewName)
{
	S32 iRet;
	S32 iLoop;
	ISIL_USR_GROUP_S sGrp;
	if(NULL == sGrpName
		|| NULL == sOldName
		|| NULL == sNewName)
	{
		return ISIL_FAIL;
	}

	iRet = ISIL_GetUsrGroupInfoByName(sGrpName, &sGrp);
	ISIL_TRACE(BUG_NORMAL, "[%s] usr cnt %d", __FUNCTION__, sGrp.stNetUsrGrp.iUsrCnt);
	if(iRet == ISIL_SUCCESS)
	{
		for(iLoop = 0; iLoop < sGrp.stNetUsrGrp.iUsrCnt; iLoop++)
		{
			if((strcmp(sGrp.stNetUsrGrp.cGropList[iLoop], sOldName)) == 0)
			{
				strncpy((sGrp.stNetUsrGrp.cGropList[sGrp.stNetUsrGrp.iUsrCnt]), sNewName, ISIL_NAME_LEN);
				return ISIL_SUCCESS;
			}
		}
	}
	return ISIL_FAIL;
}

#endif



/********************************************************
 *Function: ISIL_GetUsrInfoByName
 *Description: 根据用户名，查找用户信息
 *Input:
 *	S08 *sName:需要查找的用户名
 *	ISIL_USR_CFG_S* sUsrInf:获得用户信息的指针
 *output:
 *       ISIL_USR_CFG_S* sUsrInf:用户信息指针
 *return:
 *        Success: 0; Fail : -1
 *******************************************************/

S32 ISIL_GetUsrInfoByName(S08 *sName, ISIL_USR_CFG_S* sUsrInf)
{
	S32 iFound = 0;
	S32 iDataSize;
	FILE *fp;

	if(NULL == sName || NULL == sUsrInf)
	{
		return ISIL_FAIL;
	}

	if ((fp = fopen(ISIL_USER_INFO_FILE, "r")) == NULL)
	{
		return ISIL_FAIL;
	}
	iDataSize = sizeof(ISIL_USR_CFG_S);
	while((fread(sUsrInf, iDataSize, 1, fp)) > 0)
	{
		if((strcmp(sUsrInf->sName, sName)) == 0) /*found*/
		{
			iFound = 1;
			break;
		}
	}

	fclose(fp);
	return iFound ? ISIL_SUCCESS : ISIL_FAIL;
}

/********************************************************
 *Function: ISIL_GetUsrInfoByName
 *Description: 根据用户ID，查找用户信息
 *Input:
 *	S08 *sName:需要查找的用户名
 *	ISIL_USR_CFG_S* sUsrInf:获得用户信息的指针
 *output:
 *       ISIL_USR_CFG_S* sUsrInf:用户信息指针
 *return:
 *        Success: 0; Fail : -1
 *******************************************************/

S32 ISIL_GetUsrInfoByID(U32 uiUsrId, ISIL_USR_CFG_S* sUsrInf)
{
	S32 iFound = 0;
	S32 iDataSize;
	FILE *fp;

	if(NULL == sUsrInf)
	{
		return ISIL_FAIL;
	}

	if ((fp = fopen(ISIL_USER_INFO_FILE, "r")) == NULL)
	{
		return ISIL_FAIL;
	}
	iDataSize = sizeof(ISIL_USR_CFG_S);
	while((fread(sUsrInf, iDataSize, 1, fp)) > 0)
	{
		if(sUsrInf->lUserID == uiUsrId)/*found*/
		{
			iFound = 1;
			break;
		}
	}
	fclose(fp);
	return iFound ? ISIL_SUCCESS : ISIL_FAIL;
}

/********************************************************
 *Function: ISIL_GetUsrGroupInfoByName
 *Description: 根据输入用户组名，获得一个用户组的信息
 *Input:
 *	S08 *cUsrName:用户组名称
 *	ISIL_USR_GROUP_S* sGrp:用户组信息保存指针
 *output:
 *       ISIL_USR_GROUP_S* sGrp:用户组信息保存指针
 *return:
 *	Success, 0; fail, -1
 *******************************************************/

S32 ISIL_GetUsrGroupInfoByName(S08* sUsrGrpName, ISIL_USR_GROUP_S* sGrp)
{
	S32 iDataSize;
	S32 iFound;
	FILE * fp;

	if(sUsrGrpName == NULL || sGrp == NULL)
	{

		ISIL_TRACE(BUG_ERROR, "[%s] pointer is null", __FUNCTION__);
		return ISIL_FAIL;
	}

	fp=fopen(ISIL_USER_GROUP_FILE, "r");
	if(fp == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] open file fail", __FUNCTION__);
		return ISIL_FAIL;
	}

	iDataSize = sizeof(ISIL_USR_GROUP_S);

	while((fread(sGrp, iDataSize, 1, fp)) > 0)
	{
		if((strcmp(sGrp->sGroupName, sUsrGrpName)) == 0) /*found*/
		{
			iFound = 1;
			break;
		}
	}
	fclose(fp);
	return iFound ? ISIL_SUCCESS : ISIL_FAIL;;

}

/********************************************************
 *Function: ISIL_GetUsrGroupInfoByID
 *Description: 根据输入用户组ID，获得一个用户组的信息
 *Input:
 *	S08 *cUsrName:用户组名称
 *	ISIL_USR_GROUP_S* sGrp:用户组信息保存指针
 *output:
 *       ISIL_USR_GROUP_S* sGrp:用户组信息保存指针
 *return:
 *	Success, 0; fail, -1
 *******************************************************/

S32 ISIL_GetUsrGroupInfoByID(U32 uiUsrGrpID, ISIL_USR_GROUP_S* sGrp)
{
	S32 iFound = 0;
	S32 iDataSize;
	FILE * fp;

	if(sGrp == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] pointer is null", __FUNCTION__);
		return ISIL_FAIL;
	}
	fp=fopen(ISIL_USER_GROUP_FILE, "r+");
	if(fp == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s] open file fail", __FUNCTION__);
		return ISIL_FAIL;
	}

	iDataSize = sizeof(ISIL_USR_GROUP_S);
	while((fread(sGrp, iDataSize, 1, fp)) > 0)
	{
		if(sGrp->lGroupID == uiUsrGrpID)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s] get gid %d", __FUNCTION__, uiUsrGrpID);
			iFound = 1;
			break;
		}
	}

	fclose(fp);
	return (iFound ? ISIL_SUCCESS : ISIL_FAIL);

}


/********************************************************
 *Function: ISIL_SaveAllUserInfo
 *Description: 保存用户信息
 *Input:
 *	void *pAllInfo: 用户信息指针(用户/用户组)
 *	S32 iInfoSize: 需要保存的用户信息大小
 *	S32 iInfoType: 保存的用户信息类型
 *				   0:用户信息
 *				   1:用户组信息
 *output:
 *       ISIL_USR_GROUP_S* sGrp:用户组信息保存指针
 *return:
 *	Success, 0; fail, -1
 *******************************************************/

S32 ISIL_SaveAllUserInfo(void *pAllInfo, S32 iInfoSize, S32 iInfoType)
{
	S32 iRet = ISIL_FAIL;
	S32 iType = 1;
	if(NULL == pAllInfo)
	{
		return ISIL_FAIL;
	}

	if(iInfoType == 0) /*Usr information*/
	{
		iRet = ISIL_WriteConfigFile(ISIL_USER_INFO_FILE, 0, iInfoSize,
			&iType, pAllInfo);
	}
	else if(iInfoType == 1)
	{
		iRet = ISIL_WriteConfigFile(ISIL_USER_GROUP_FILE, 0, iInfoSize,
			&iType, pAllInfo);
	}
	return iRet;
}

/********************************************************
 *Function: ISIL_GetAllUserInfo
 *Description: 获得用户信息
 *Input:
 *	void *pAllInfo: 用户信息指针(用户/用户组)
 *	S32 iInfoSize: 需要获得的用户信息大小
 *	S32 iInfoType: 用户信息类型
 *				   0:用户信息
 *				   1:用户组信息
 *output:
 *       void *pAllInfo:用户组信息指针
 *return:
 *	Success, 0; fail, -1
 *******************************************************/

S32 ISIL_GetAllUserInfo(void *pAllInfo, S32 iInfoSize, S32 iInfoType)
{
	S32 iRet = ISIL_FAIL;
	S32 iType = 0;
	if(NULL == pAllInfo)
	{
		return ISIL_FAIL;
	}

	if(iInfoType == 0) /*Usr information*/
	{
		iRet = ISIL_ReadConfigFile(ISIL_USER_INFO_FILE, 0, iInfoSize,
			&iType, pAllInfo);
	}
	else if(iInfoType == 1)
	{
		iRet = ISIL_ReadConfigFile(ISIL_USER_GROUP_FILE, 0, iInfoSize,
			&iType, pAllInfo);
	}
	return iRet;
}

S32 ISIL_GetAllUsrInfoSize()
{
    return ISIL_GetFileSize(ISIL_USER_INFO_FILE);
}
S32 ISIL_GetAllUsrGrpInfoSize()
{
    return ISIL_GetFileSize(ISIL_USER_GROUP_FILE);
}


S32 ISIL_InitUsrCfgFile()
{
	S32 fd;
	S32 iRet;
	ISIL_USR_GROUP_S sGrp;
	ISIL_USR_CFG_S sUsr;

	fd = open(ISIL_USER_INFO_FILE, O_CREAT | O_RDONLY,0644);
	if (fd < 0)
	{
		return ISIL_FALSE;
	}
	close(fd);

	fd = open(ISIL_USER_GROUP_FILE, O_CREAT | O_RDONLY,0644);
	if (fd < 0)
	{
		return ISIL_FALSE;
	}
	close(fd);

	iRet = ISIL_GetFileSize(ISIL_USER_INFO_FILE);
	if(iRet == 0)/*创建admin*/
	{
		memset(&sUsr, 0, sizeof(ISIL_USR_CFG_S));
		sUsr.lGroupID = 0;
		sUsr.lUserID = 0;
		strncpy(sUsr.sName, "admin", ISIL_NAME_LEN);
		strncpy(sUsr.sPassword, "admin", ISIL_PASSWORD_LEN);
		memset(sUsr.cLevel, 0xff, sizeof(sUsr.cLevel));

		ISIL_SaveAllUserInfo(&sUsr, sizeof(ISIL_USR_CFG_S), 0);
	}
	iRet = ISIL_GetFileSize(ISIL_USER_GROUP_FILE);
	if(iRet == 0)/*创建admin*/
	{
		memset(&sGrp, 0, sizeof(ISIL_USR_GROUP_S));
		sGrp.lGroupID = 0;
		strncpy(sGrp.sGroupName, "admin", ISIL_NAME_LEN);
		strncpy(sGrp.sGroupPassword, "admin", ISIL_PASSWORD_LEN);
		memset(sGrp.cLevel, 0xff, sizeof(sGrp.cLevel));

		ISIL_SaveAllUserInfo(&sGrp, sizeof(ISIL_USR_GROUP_S), 1);
	}
	return ISIL_SUCCESS;
}

/********************************************************
 *Function: ISIL_UsrLevelMatch
 *Description: 根据用户ID,查看用户权限是否匹配
 *Input:
 *	S32 iUsrID:用户ID
 *	S32 iLevel,当前需要的权限
 *output:
 *       No
 *return:
 *        Success, 1
 *		  Fail,	   0
 *******************************************************/

S32 ISIL_UsrLevelMatch(U32 uiUsrID, S32 iLevel)
{
	S32 iRet;
	S32 iLevel1;
	S32	iLevelBit;
	ISIL_USR_CFG_S stUsrInfo;
	iLevel1 = iLevel / ISIL_USR_LEVEL_BIT;
	iLevelBit = iLevel % ISIL_USR_LEVEL_BIT;
	iRet = ISIL_GetUsrInfoByID(uiUsrID, &stUsrInfo);
	if(iRet != ISIL_FAIL)
	{
		if(((stUsrInfo.cLevel[iLevel1] >> iLevelBit) & 0x1)
			== 1)
		{
			ISIL_TRACE(BUG_NORMAL, "[%s] user level %d is match", __FUNCTION__, iLevel);
			return ISIL_TRUE;
		}
	}

	ISIL_TRACE(BUG_NORMAL, "[%s] user level %d not match", __FUNCTION__, iLevel);
	return ISIL_FALSE;
}

/********************************************************
 *Function: ISIL_VerifyUsrInfo
 *Description: 检查网络用户参数是否正确
 *Input:
 *	ISIL_USR_CFG_S * sUsrInfo: 用户输入的参数
 *output:
 *       No
 *return:
 *        not -1: Success; -1: Fail
 *******************************************************/

S32 ISIL_VerifyUsrInfo(ISIL_USR_LOGIN_S *sUsrLogin)
{
	S32 iRet;
	ISIL_USR_CFG_S sUsrInf;
	if(NULL == sUsrLogin)
	{
		ISIL_TRACE(BUG_ERROR,"User infomation pointer null");
		return ISIL_FAIL;
	}
	iRet = ISIL_GetUsrInfoByName(sUsrLogin->sName, &sUsrInf);
	if(ISIL_FAIL != iRet)
	{
		ISIL_TRACE(BUG_NORMAL, "[%s] find usr %s, pwd %s", __FUNCTION__, sUsrInf.sName, sUsrInf.sPassword);
		if((strcmp(sUsrLogin->sPassword, sUsrInf.sPassword) == 0)
			|| (strcmp(sUsrInf.sPassword, USER_DEFAULT_PWD) == 0
			&& strcmp(sUsrLogin->sPassword, "") == 0))

		{
			ISIL_TRACE(BUG_NORMAL, "[%s] %s login success", __FUNCTION__, sUsrLogin->sName);
			return ISIL_SUCCESS;
		}

	}
	ISIL_TRACE(BUG_NORMAL, "[%s] %s login fail", __FUNCTION__, sUsrLogin->sName);
	return ISIL_FAIL;
}


