#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "isil_searchdev.h"
#include "isil_net_manage.h"

/********************************************************
 *Function: ISIL_SearchDevInfo
 *Description:搜索设备的相关信息给客户端
 *Input:
 *  info:客户端所要的设备信息结构
 *output:
 *       No
 *return:
 *       No
 *******************************************************/

void ISIL_SearchDevInfo(ISIL_RET_SEARCH_DEVICE_INFO_S* info)
{
    S08 *ipaddr = NULL;
    struct in_addr addr;
    U32 ip;
    if(NULL == info)
    {
        return;
    }
    /*通过dev base info、net info等接口填充info*/
    info->cAlarmInCount = 4;
    info->cChannelCount = 4;
    info->cDecordCount = 1;
    info->cDiskCount = 1;
    info->cDiskCtrlCount = 1;
    strcpy(info->sDeviceName, "isil");
    ip = ISIL_GetLocalNetIP();
    addr.s_addr = ip;
    ipaddr = (char *)inet_ntoa(addr);
    strcpy(info->sIPAddr,ipaddr);
    info->nDataPort = _swab16(10000);
}

