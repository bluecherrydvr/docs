#include "isil_encode_manage.h"
#include "isil_cfg_file_manage.h"
#include "isil_messagestruct.h"

#define ISIL_ENCODE_CFG_FILE "/flash/config/encode/encode_cfg_"
S32 ISIL_SaveEncodeCfg(G_ENCODE_CFG_S* pCfg)
{
	S32 iRet;
	S32 iType;
	S08 sPath[ISIL_CFG_PATH_LEN];


	if(NULL == pCfg)
	{
		return ISIL_FAIL;
	}
	sprintf(sPath, "%s%d", ISIL_ENCODE_CFG_FILE, pCfg->channel);

	iRet = ISIL_WriteConfigFile(sPath, 0, sizeof(G_ENCODE_CFG_S),
		&iType, pCfg);
	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
	return ISIL_SUCCESS;
}

S32 ISIL_GetEncodeCfg(G_ENCODE_CFG_S* pCfg)
{
	S32 iRet;
	S32 iType;
	S08 sPath[ISIL_CFG_PATH_LEN];

	if(NULL == pCfg)
	{
		return ISIL_FAIL;
	}
	sprintf(sPath, "%s%d", ISIL_ENCODE_CFG_FILE, pCfg->channel);

	iRet = ISIL_ReadConfigFile(sPath, 0, sizeof(G_ENCODE_CFG_S),
		&iType, pCfg);
	if(iRet < 0)
	{
		return ISIL_FAIL;
	}
	return ISIL_SUCCESS;
}

