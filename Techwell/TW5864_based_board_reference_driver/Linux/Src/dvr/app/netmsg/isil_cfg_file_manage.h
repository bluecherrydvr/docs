#ifndef _ISIL_CFG_FILE_MANAGE_H_
#define _ISIL_CFG_FILE_MANAGE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "isil_type.h"

#define LINE_BUF_LEN		2048
#define max(a, b)       ((a) > (b) ? (a) : (b))
#define min(a, b)       ((a) < (b) ? (a) : (b))

typedef enum
{
    NET_SDK_DEV_CFG = 0,
    NET_SDK_CFG_MAX
}NET_SDK_CFG_TYPE;

#define ISIL_DEV_HAVE_FLASH

#ifdef ISIL_DEV_HAVE_FLASH
	#define NVS_CFG_ROOT        "/flash"
	#define NVS_CONFIG_DIR		"/flash/config"
	/*Local net sdk config file path*/
	#define ISIL_LOCAL_NET_SDK_CFG_PATH "/flash/netsdkcfg"
#else
	#define NVS_CFG_ROOT        "/tmp"
	#define NVS_CONFIG_DIR		"/tmp/config"
	/*Local net sdk config file path*/
	#define ISIL_LOCAL_NET_SDK_CFG_PATH "/tmp/netsdkcfg"
#endif
#define ISIL_LOCAL_NET_SDK_DEV_CFG  "devcfg"


/*定义系统网络配置文件的路径*/
#define ISIL_OS_NET_CFG_PATH         "/etc/network/interfaces"
#define ISIL_OS_NET_CFG_IP_KEY       "address"
#define ISIL_OS_NET_CFG_NET_MASK_KEY "netmask"
#define ISIL_OS_NET_CFG_GATE_WAY_KEY "gateway"
#define ISIL_OS_NET_CFG_NET_WORK_KEY "network"

typedef struct _ISIL_CFG_PARAM_STRUC
{
	S32 item;
	S08* name;
	S08* default_value;
}ISIL_CFG_PARAM_S;

extern S32 ISIL_FileExit(S08 *path);
extern S32 ISIL_ReadConfigFile(S08 *path,S32 offset,S32 size, S32 *type,void *data);
extern S32 ISIL_WriteConfigFile(S08 *path,S32 offset,S32 size, S32 *type,void *data);
extern S32 ISIL_MkConfigDir(void);
extern S32 ISIL_GetCfgItem(S08* sPath, ISIL_CFG_PARAM_S* pstParam, S32 iCfgSize, S32 iItemId, S08 *buf);
extern S32 ISIL_SetCfgItem(S08* sPath, ISIL_CFG_PARAM_S* pstParam, S32 iCfgSize, S32 iItemId, S08*fmt, ...);
extern S32 ISIL_GetFileSize(S08 *sPath);
extern S32 ISIL_NetSDKCfgDir(void);
extern void ISIL_ModifyOSSysCfg(S08 *name,S08 *key,S08 *value);

#ifdef __cplusplus
}
#endif

#endif

