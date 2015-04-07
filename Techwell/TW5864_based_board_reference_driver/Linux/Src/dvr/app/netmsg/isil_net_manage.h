/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_net_manage.h
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:handle net information.


	Other:
	Function list:
	History:

*******************************************************************
*/

#ifndef _ISIL_NET_MANAGE_H_
#define _ISIL_NET_MANAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "isil_common.h"
#include "isil_type.h"
#include "isil_messagestruct.h"

#define NETWORK_CONFIG_FILE   "/flash/config/network/network_config"

#define TCP_PORT 10000
#define UDP_PORT 10001
#define HTTP_PORT 80

#define NET_ETH_X 1

/*Network interface*/
#define Base10M_T2_Base10M_T 	1
#define Base100M_TX4_100M 		3
#define AUTO_10M_100M			5

extern S32 ISIL_GetNetworkConfig(ISIL_NET_CFG_S *param);
extern S32 ISIL_SetNetworkConfig(ISIL_NET_CFG_S *param);
extern U32 ISIL_GetLocalNetIP(void);
extern void ISIL_InitLocalIP();

#ifdef __cplusplus
}
#endif
#endif

