/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: isil_net_manage.c
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:handle net information.


	Other:
	Function list:
	History:

*******************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <termios.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <linux/sockios.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
//#include <sys/timeb.h>
#include <semaphore.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "isil_net_manage.h"
#include "isil_cfg_file_manage.h"
#include "isil_debug.h"
#include "isil_messagestruct.h"
#include "isil_endian_trans.h"


static ISIL_NET_CFG_S ISIL_NetDefaultCfg =
{
	.lSize = 0,
	.lIPAddr = 0,
	.lNetMask = 0,
	.lGateWay = 0,
	.lMultiCastIP = 0,

	.nNetInterface = 5,
	.nDataPort = 11000,
	.nWebPort = 80,
	.nMultiCastPort = 0,
	.cMACAddr = "0",

	.cDHCP = 0,
	.cPppoeOpen = 0,
	.cDdnsOpen = 0,
	.cReserve = 0,
	.sPppoeName = "0",
	.sPppoePass = "0",

	.lPppoeTimesl = 0,
	.lPppoeIPAddr = 0,
	.sDdnsName = "0",
	.sDdnsPass = "0",
	.sDdnsIP = "0",

	.nDdnsPortNo = 0,
	.nDdnsMapDataPort = 0,
	.nDdnsMapWebPort = 0,
	.nReserve = 0,
};

static void ISIL_SetNetTempIP();
static S64 ISIL_GetHWAddr(S32 eth_x)
{
	static S64 hwaddr;
	struct ifreq ifreq;
	S32 sock;
	if(hwaddr)
		return hwaddr;
	if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
		return ISIL_FAIL;
	sprintf(ifreq.ifr_name,"eth%d", eth_x);
	if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0)
		return ISIL_FAIL;
	hwaddr = (((long long)(ifreq.ifr_hwaddr.sa_data[5]) & 0xff) << 40) |
	        (((long long)(ifreq.ifr_hwaddr.sa_data[4]) & 0xff) << 32) |
	        (((long long)(ifreq.ifr_hwaddr.sa_data[3]) & 0xff) << 24) |
	        (((long long)(ifreq.ifr_hwaddr.sa_data[2]) & 0xff) << 16) |
	        (((long long)(ifreq.ifr_hwaddr.sa_data[1]) & 0xff) << 8) |
	        (((long long)(ifreq.ifr_hwaddr.sa_data[0]) & 0xff) >> 56);

	return hwaddr;
}

/*暂时屏蔽掉*/
/*
static S32 ISIL_GetDNS(U32 *dns1, U32 *dns2)
{
	S32 ret;
	ret = res_init();
	if(ret == -1)
		return ISIL_FAIL;

	*dns1 = (U32)((_res.nsaddr_list[0]).sin_addr.s_addr);
	*dns2 = (U32)((_res.nsaddr_list[1]).sin_addr.s_addr);
	return ISIL_SUCCESS;
}
*/
/*获得网卡数目*/
static U32 GetEthCnt()
{
    char buf[256];
	FILE *fp;
    U32 eth_ifnum = 0;

	/* init ethernet */
	if ((fp = fopen("/proc/net/dev", "r")) == NULL)
		return 0;
	while (fgets(buf, sizeof (buf), fp))
    {
		if (strstr(buf, "eth") != NULL)
        {   /* one eth */
			eth_ifnum++;
		}
	}
	fclose(fp);
    return eth_ifnum;
}
/*0: ip 1: mask*/
#define NEW_NET_INFO
#ifndef NEW_NET_INFO
static U32 ISIL_GetLocalIPMaskByEth(S32 type, U32 eth_x)
{
	struct ifreq ifr;
	S08 *ipaddr;
	S32 sock;
	struct sockaddr_in sin;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		goto failed;
	}
	//strcpy(ifr.ifr_name,"eth0");
	sprintf(ifr.ifr_name, "eth%d", eth_x);
	ifr.ifr_name[4]=0;

    if(ioctl(sock,SIOCGIFFLAGS,&ifr) < 0)
    {
        goto failed;
    }

    if(!((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING)))
        goto failed;


	if(ioctl(sock, type?SIOCGIFNETMASK:SIOCGIFADDR, &ifr)<0)
	{
		goto failed;
	}


	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
	ipaddr = (char *)inet_ntoa(sin.sin_addr);
	printf("ip %s\n", ipaddr);
	if(ipaddr == NULL)
	{
		goto failed;
	}
	return sin.sin_addr.s_addr;

	failed:
	//ISIL_TRACE(BUG_ERROR, "[%s %d] Get local ip address failed or down! exit!\n", __FUNCTION__, __LINE__);
	return ISIL_FALSE;
}
#else
#define MAXINTERFACES 16
static U32 ISIL_GetLocalIPMaskByEth(S32 type)
{
	S08 *ipaddr;
	S08 str[4];
	S32 sock, intrface;
	struct ifreq ifr[MAXINTERFACES];
	struct ifconf ifc;
	struct sockaddr_in sin;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		return ISIL_FALSE;
	}
	
	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_buf = (caddr_t)ifr;

	if (!ioctl(sock, SIOCGIFCONF, (char *) &ifc)) {
		intrface = ifc.ifc_len / sizeof(struct ifreq);
		printf("%s: interface num is intrface=%d\n\n\n", __FUNCTION__, intrface);

		while (intrface-- > 0) {
			printf("net device %s\n", ifr[intrface].ifr_name);
			memset(str, 0x00, 4);
			strncpy(str, ifr[intrface].ifr_name, 3);

			if(strcmp(str, "eth") != 0) {
				continue;
			}
			if(ioctl(sock,SIOCGIFFLAGS,(char*)&ifr[intrface]) < 0){
				ISIL_TRACE(BUG_NORMAL, "%s get %s SIOCGIFFLAGS fail\n", __FUNCTION__, ifr[intrface].ifr_name);
				continue;
			}
			
			if(!((ifr[intrface].ifr_flags & IFF_UP) && (ifr[intrface].ifr_flags & IFF_RUNNING))){
				ISIL_TRACE(BUG_NORMAL, "%s get %s status is down or not runing\n", __FUNCTION__, ifr[intrface].ifr_name);
				continue;
			}
			if(ioctl(sock, type?SIOCGIFNETMASK:SIOCGIFADDR, (char*)&ifr[intrface])<0){
				ISIL_TRACE(BUG_NORMAL, "%s get %s ip or net mask fail\n", __FUNCTION__, ifr[intrface].ifr_name);
				continue;
			}
			memcpy(&sin,&ifr[intrface].ifr_addr,sizeof(sin));
			ipaddr = (char *)inet_ntoa(sin.sin_addr);
			printf("ip %s\n", ipaddr);
			if(ipaddr == NULL){
				continue;
			}
			else{
				return sin.sin_addr.s_addr;
			}
		}
	}
	ISIL_TRACE(BUG_ERROR, "[%s %d] ioctl SIOCGIFCONF fail\n", __FUNCTION__, __LINE__);
	return ISIL_FALSE;
}
#endif

U32 ISIL_GetLocalNetIP(void)
{
    /*TODO,暂时设置为1*/
    U32 ipaddr = 0;
    U32 ethcnt = 0;
    U32 i = 0;
#ifndef NEW_NET_INFO
    ethcnt = GetEthCnt();
    for(i = 0; i < ethcnt; i++)
    {
        ipaddr = ISIL_GetLocalIPMaskByEth(0, i);
        if(ipaddr)
            return ipaddr;
    }
#else
	ipaddr = ISIL_GetLocalIPMaskByEth(0);
	if(ipaddr)
		return ipaddr;
#endif
    return 0;
	//return ISIL_GetLocalIPMaskByEth(0, NET_ETH_X);
}
static U32 ISIL_GetLocalNetMask(void)
{
    /*TODO,暂时设置为1*/
	#ifndef NEW_NET_INFO
	return ISIL_GetLocalIPMaskByEth(1, NET_ETH_X);
	#else
	return ISIL_GetLocalIPMaskByEth(1);
	#endif
}
static U32 ISIL_GetNetGW(void)
{
	S08 eth[10];
	S08 buf[1024];
	S32 flgs, ref, use, metric, mtu, win, ir;
	U32 d, g, m;
	FILE *fp;

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
	{
		return ISIL_FALSE;
	}

	/* skip first line */
	fgets(buf, sizeof (buf), fp);

	while (fgets(buf, sizeof (buf), fp) != NULL)
	{
		memset(eth,0x00, sizeof (eth));
		if (sscanf(buf, "%s%lx%lx%X%d%d%d%lx%d%d%d",
			   eth, &d, &g, &flgs, &ref, &use, &metric, &m, &mtu,
			   &win, &ir) != 11) {
			fclose(fp);
			return ISIL_FALSE;
		}
	}
	return g;
}
/*暂时屏蔽掉*/
/*

static S32 ISIL_GetLocalHostName(char *hostname)
{
	S32 ret;
	ret = gethostname(hostname,ISIL_NAME_LEN);
	if(ret == -1)
		return ISIL_FAIL;
	return ISIL_SUCCESS;
}
*/
static void ISIL_GetMacAddr(unsigned char *mac)
{
	S64 hwaddr;
    /*暂时定为1*/
	hwaddr = ISIL_GetHWAddr(1);
	mac[0] = (unsigned char)hwaddr;
	mac[1] = (unsigned char)(hwaddr>>8);
	mac[2] = (unsigned char)(hwaddr>>16);
	mac[3] = (unsigned char)(hwaddr>>24);
	mac[4] = (unsigned char)(hwaddr>>32);
	mac[5] = (unsigned char)(hwaddr>>40);
	return;
}


/*/type 0: ipaddr 1: mask*/
static S32 ISIL_SetIPMask(S32 type, U32 newip, S32 eth_x)
{
	struct ifreq ifr;
	S32 sock;
    S08 *ipaddr;
	struct sockaddr_in *sin;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
		goto failed;
	sprintf(ifr.ifr_name,"eth%d", eth_x);
	ifr.ifr_name[4]=0;
	if(type == 0)
	{
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		sin->sin_family = AF_INET;
	}
	else
	{
		sin = (struct sockaddr_in *)&ifr.ifr_netmask;
		sin->sin_family = AF_INET;
	}
	sin->sin_addr.s_addr = newip;

	if(type == 0)
	{
        /*暂时这样处理IP地址*/
        ipaddr = (char *)inet_ntoa(sin->sin_addr);
        ISIL_ModifyOSSysCfg(ISIL_OS_NET_CFG_PATH, ISIL_OS_NET_CFG_IP_KEY, ipaddr);
        ISIL_SetNetTempIP(ipaddr);
		//if(ioctl(sock, SIOCSIFADDR, &ifr)<0)
		//goto failed;
	}
	else
	{
		if(ioctl(sock, SIOCSIFNETMASK, &ifr)<0)
		goto failed;
	}

	return ISIL_SUCCESS;

	failed:
	ISIL_TRACE(BUG_ERROR, "Change IP/netmask failed! type:%d erro:%s",type,strerror(errno));
	return ISIL_FAIL;
}

/*add: 0 del:1*/
static S32 ISIL_SetLocalGW(S32 adddel, U32 newgw)
{
	S08 ifname[32];
	struct rtentry ifr;
	S32 sock;
	struct sockaddr_in *sin;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
		goto failed;

	memset(&ifr, 0x00,sizeof(ifr));
	sin = (struct sockaddr_in *)&ifr.rt_dst;
	sin->sin_family = AF_INET;

	if(inet_pton(AF_INET, "0.0.0.0", &sin->sin_addr) < 0)
		goto failed;

	sin = (struct sockaddr_in *)&ifr.rt_gateway;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = newgw;
	strcpy(ifname,"eth0");
	ifr.rt_dev = ifname;
	ifr.rt_flags = RTF_GATEWAY;
	if(adddel)
	{
		if(ioctl(sock, SIOCDELRT, &ifr)<0)
		{
			if(errno == ESRCH)
				return 0;
			else
				goto failed;
		}
	}
	else
	{
		if(ioctl(sock, SIOCADDRT, &ifr)<0)
		{
			if(errno == EEXIST)
				return ISIL_SUCCESS;
			else
				goto failed;
		}
	}
	return ISIL_SUCCESS;

	failed:
	ISIL_TRACE(BUG_ERROR, "Change GW failed!");
	return ISIL_FAIL;
}

static S32 ISIL_SetLocalIP(U32 newip)
{
	return ISIL_SetIPMask(0,newip, NET_ETH_X);
}
static S32 ISIL_SetLocalMask(U32 newmask)
{
    return ISIL_SetIPMask(1,newmask, NET_ETH_X);
}
static S32 ISIL_GetLocalGW(void)
{
	S08 eth[5];
	S08 buf[1024];
	S32 flgs, ref, use, metric, mtu, win, ir;
	U32 d, g, m;
	FILE *fp;

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
	{
		perror("fopen");
		return ISIL_FAIL;
	}

	fgets(buf, sizeof (buf), fp);

	while (fgets(buf, sizeof (buf), fp) != NULL)
	{
		memset(eth,0x00, sizeof (eth));
		if (sscanf(buf, "%s%lx%lx%X%d%d%d%lx%d%d%d",eth, &d, &g, &flgs, &ref, &use,
			&metric, &m, &mtu,&win, &ir) != 11)
		{
			fclose(fp);
			return ISIL_SUCCESS;
		}
		if (flgs != (RTF_UP|RTF_GATEWAY))
			continue;

		fclose(fp);
		return g;
	}
	fclose(fp);
	return ISIL_SUCCESS;
}

/*void ISIL_TransNetworkConfig(ISIL_NET_CFG_S *param)
{
    if(NULL == param)
        return;

    //param->lGateWay = _swab32(param->lGateWay);
    //param->lIPAddr = _swab32(param->lIPAddr);
    //param->lMultiCastIP = _swab32(param->lMultiCastIP);
    //param->lNetMask = _swab32(param->lNetMask);
    //param->lPppoeIPAddr = _swab32(param->lPppoeIPAddr);
    param->lPppoeTimesl = _swab32(param->lPppoeTimesl);
    param->lSize = _swab32(param->lSize);

    param->nDataPort = _swab16(param->nDataPort);
    param->nDdnsMapDataPort = _swab16(param->nDdnsMapDataPort);
    param->nDdnsMapWebPort = _swab16(param->nDdnsMapWebPort);
    param->nDdnsPortNo = _swab16(param->nDdnsPortNo);
    param->nMultiCastPort = _swab16(param->nMultiCastPort);
    param->nNetInterface = _swab16(param->nNetInterface);
    param->nWebPort = _swab16(param->nWebPort);

    return;
}
*/

/*********************************************************
 *Function: ISIL_SetNetworkConfig
 *Description:
 *	根据输入参数，设置网络配置文件
 *Input:
 *	ISIL_NET_CFG_S *param:网络配置属性指针
 *output:
 *	无
 *return:
 *       Success: 0, Fail: -1
 *******************************************************/

S32 ISIL_SetNetworkConfig(ISIL_NET_CFG_S *param)
{
	S32 iRet;
	S32 iType;
	S32 iOldGW;
    S32 iNeedSetNet = 0;
	ISIL_NET_CFG_S sOldCfg;
	if(param == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], net param pointer null", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	iRet = ISIL_GetNetworkConfig(&sOldCfg);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d] not get old net cfg", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

    /*转换字节序*/
    ISIL_TransNetworkConfig(&sOldCfg);

	if(param->lIPAddr != sOldCfg.lIPAddr)
	{

		iRet = ISIL_SetLocalIP(param->lIPAddr);
		if(iRet == ISIL_FAIL)
		{
			ISIL_TRACE(BUG_ERROR, "[%s %d] set ip fail", __FUNCTION__, __LINE__);
			goto FAILD;
		}
        //iNeedSetNet = 1;
	}

	if(param->lNetMask != sOldCfg.lNetMask)
	{
		iRet = ISIL_SetLocalMask(param->lNetMask);
		if(iRet < 0)
		{
			ISIL_TRACE(BUG_ERROR, "[%s %d] set mask fail", __FUNCTION__, __LINE__);
			goto FAILD;
		}
        iNeedSetNet = 1;
	}

	if(param->lGateWay != sOldCfg.lGateWay)
	{
		iOldGW= ISIL_GetLocalGW();
		if(iOldGW != 0)
		{
			iRet = ISIL_SetLocalGW(1, iOldGW);
			if(iRet < 0)
			{
				ISIL_TRACE(BUG_ERROR, "[%s %d] delete old gw fail", __FUNCTION__, __LINE__);
				goto FAILD;
			}
		}
		iRet = ISIL_SetLocalGW(0, param->lGateWay);
		if(iRet < 0)
		{
			ISIL_TRACE(BUG_ERROR, "[%s %d] set new gw fail", __FUNCTION__, __LINE__);
			goto FAILD;
		}
        iNeedSetNet = 1;
	}

	iRet = ISIL_WriteConfigFile(NETWORK_CONFIG_FILE, 0, sizeof(ISIL_NET_CFG_S),
		&iType, param);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], set net cfg fail", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

    if(1 == iNeedSetNet)
	{
        system("/etc/init.d/network restart");
    }
	return ISIL_SUCCESS;

FAILD:
	ISIL_TRACE(BUG_ERROR, "[%s %d], set net falil", __FUNCTION__, __LINE__);
	iRet = ISIL_WriteConfigFile(NETWORK_CONFIG_FILE, 0, sizeof(ISIL_NET_CFG_S),
					&iType, &sOldCfg);
	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], reset old net cfg fail", __FUNCTION__, __LINE__);
	}
	return ISIL_FAIL;

}

static S32 ISIL_SetNetworkDefault()
{
	S32 iRet;
	S32 iType;
	ISIL_NET_CFG_S* psNetCfg;
	psNetCfg = &ISIL_NetDefaultCfg;

	psNetCfg->nDataPort = TCP_PORT;
	psNetCfg->nWebPort = HTTP_PORT;
	psNetCfg->lSize = sizeof(ISIL_NET_CFG_S);
	psNetCfg->lIPAddr = ISIL_GetLocalNetIP();
	psNetCfg->lNetMask = ISIL_GetLocalNetMask();
	psNetCfg->lGateWay = ISIL_GetNetGW();
	ISIL_GetMacAddr(psNetCfg->cMACAddr);

    //ISIL_TransNetworkConfig(&psNetCfg);

	iRet = ISIL_WriteConfigFile(NETWORK_CONFIG_FILE, 0, sizeof(ISIL_NET_CFG_S),
				&iType, psNetCfg);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "Set net default cfg fail");
		return ISIL_FAIL;
	}
	return ISIL_SUCCESS;
}

/*********************************************************
 *Function: ISIL_SetNetworkConfig
 *Description:
 *	获得网络配置文件
 *Input:
 *	ISIL_NET_CFG_S *param:网络配置属性指针
 *output:
 *	ISIL_NET_CFG_S *param:网络配置属性指针
 *return:
 *       Success: 0, Fail: -1
 *******************************************************/

S32 ISIL_GetNetworkConfig(ISIL_NET_CFG_S *param)
{
	S32 iRet;
	S32 iType;
	if(param == NULL)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], net pointer null", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}

	iRet = ISIL_FileExit(NETWORK_CONFIG_FILE);
	if(iRet == ISIL_FALSE)
	{
		iRet = ISIL_SetNetworkDefault();
		if(iRet < 0)
		{
			memcpy(param, &ISIL_NetDefaultCfg, sizeof(ISIL_NET_CFG_S));
			return ISIL_SUCCESS;
		}
	}

	iRet = ISIL_ReadConfigFile(NETWORK_CONFIG_FILE, 0, sizeof(ISIL_NET_CFG_S),
		&iType, param);

	if(iRet < 0)
	{
		ISIL_TRACE(BUG_ERROR, "[%s %d], read network cfg fail", __FUNCTION__, __LINE__);
		return ISIL_FAIL;
	}


	return ISIL_SUCCESS;
}

static void ISIL_SetNetTempIP(S08* ip)
{
    S08 tmp[64];
    memset(tmp, 0, 64);
    printf("need modify ip %s\n", ip);
    system("ifconfig eth0 down");
    sleep(2);
    sprintf(tmp, "ifconfig eth0 %s", ip);
    system(tmp);
    return;
}

void ISIL_InitLocalIP()
{
    printf("%s %d\n", __FUNCTION__, __LINE__);
    ISIL_NET_CFG_S param;
    struct in_addr in;
    ISIL_GetNetworkConfig(&param);
    in.s_addr = param.lIPAddr;
    printf("local net ip %s\n", ((char *)inet_ntoa(in)));
    ISIL_SetNetTempIP((char *)inet_ntoa(in));
    return;
}




