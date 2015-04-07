#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


#include "isil_codec_chip_ctl.h"
#include "isil_codec_debug.h"
#include "isil_codec_errcode.h"
#include "isil_interface.h"
#include "bar_ctl.h"

//debug
#define CODEC_CTL_DEBUG

//private data struct
typedef struct
{
    int s32Chipfd;//chip fd
    char s8Open;//chip had open
    char reserved[3];
}CHIP_INFO;

CHIP_INFO CODEC_CHIP_TABLE[CODEC_MAXCHIP];//encoder chip

//API
/***********************************************************
 function:
     ISIL_CODEC_CTL_OpenChip
 discription:
     open chip control device and get fd
 input:
         char *pChipNode: chip device node path
         unsigned int u32ChipID: chip id
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_EXIST: chip had been open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

int ISIL_CODEC_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID)
{
    int fd = 0;

    if(pChipNode == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id over");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had open");
        #endif
        return CODEC_ERR_EXIST;
    }

    //todo
	fprintf(stderr, "-----dec: %s---\n", pChipNode);
    fd = open(pChipNode, O_RDWR);
    if(fd < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" open chip fail,fd[%d] < 0 ", fd);
        #endif
        return CODEC_ERR_FAIL;
    }

    CODEC_CHIP_TABLE[u32ChipID].s8Open = 1;
    CODEC_CHIP_TABLE[u32ChipID].s32Chipfd = fd;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_CODEC_CTL_CloseChip
 discription:
     close chip and chip fd
 input:
     unsigned int u32ChipID: chip id
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_CODEC_CTL_CloseChip(unsigned int u32ChipID)
{
    int fd = 0;

    if(u32ChipID > CODEC_MAXCHIP)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id over");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had close");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    //todo
	fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
	if(fd){
		close(CODEC_CHIP_TABLE[u32ChipID].s32Chipfd);
	}
    CODEC_CHIP_TABLE[u32ChipID].s8Open = 0;
    CODEC_CHIP_TABLE[u32ChipID].s32Chipfd = 0;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_CODEC_CTL_GetChipHandle
 discription:
     get chip fd
 input:
     unsigned int u32ChipID: chip id
 output:
    CODEC_HANDLE* pChipHandle: get chip fd
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_CODEC_CTL_GetChipHandle(unsigned int u32ChipID, CODEC_HANDLE* pChipHandle)
{
    int fd = 0;

	fd = fd;

    if(u32ChipID > CODEC_MAXCHIP)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id over");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had close");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    *pChipHandle = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_CODEC_CTL_SendMsg
 discription:
     get chip fd
 input:
     unsigned int u32ChipID: chip id
     CODEC_CHIP_MSG *pMsg: control message
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_CODEC_CTL_SendMsg(unsigned int u32ChipID, CODEC_CHIP_MSG *pMsg)
{
    int fd = 0;

    if(pMsg == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[pMsg->u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_CTL_OP, pMsg) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_CODEC_CTL_GetChipVersion
 discription:
     get codec chip version and verdor info
 input:
     unsigned int u32ChipID: chip id
 output:
	CODEC_CHIP_VERSION *pVersion: pointor of version data
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_CODEC_CTL_GetChipVersion(unsigned int u32ChipID, CODEC_CHIP_VERSION *pVersion)
{
    int fd = 0;

    if(pVersion == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_ID_GET, pVersion) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_CODEC_CTL_SetVideoSTD
 discription:
     set codec chip video standard
 input:
 	 unsigned int u32ChipID: chip id
	 CODEC_CHIP_VIDEO_STD* pVideoStd: video standard
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_CODEC_CTL_SetVideoSTD(unsigned int u32ChipID, CODEC_CHIP_VIDEO_STD* pVideoStd)
{
    int fd = 0;

    if(pVideoStd == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_VIDEO_BUS_PARAM_SET, pVideoStd) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_CODEC_CTL_GetVideoSTD
 discription:
     get codec chip video standard
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 CODEC_CHIP_VIDEO_STD* pVideoStd: video standard
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_CODEC_CTL_GetVideoSTD(unsigned int u32ChipID, CODEC_CHIP_VIDEO_STD* pVideoStd)
{
    int fd = 0;

    if(pVideoStd == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_VIDEO_BUS_PARAM_GET, pVideoStd) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_ENC_CTL_SetVDCfg
 discription:
     only for encoder chip, set video bus font end config 
 input:
 	 unsigned int u32ChipID: chip id
	 ENC_CTL_VD_CFG* pVdCfg: encoder vd cfg
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_ENC_CTL_SetVDCfg(unsigned int u32ChipID, ENC_CTL_VD_CFG* pVdCfg)
{
    int fd = 0;

    if(pVdCfg == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_VD_CONFIG_PARAM_SET, pVdCfg) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_ENC_CTL_GetVDCfg
 discription:
     only for encoder chip, get video bus font end config 
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 ENC_CTL_VD_CFG* pVdCfg: encoder vd cfg
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_ENC_CTL_GetVDCfg(unsigned int u32ChipID, ENC_CTL_VD_CFG* pVdCfg)
{
    int fd = 0;

    if(pVdCfg == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_VD_CONFIG_PARAM_GET, pVdCfg) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_ENC_CTL_TrySetVDCfg(unsigned int u32ChipID, struct ISIL_vd_chan_param* pChanvdcfg)
{
    int fd = 0;

    if(pChanvdcfg == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_VD_CHNL_CONFIG_SET, pChanvdcfg) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_ENC_CTL_TryGetVDCfg(unsigned int u32ChipID, struct ISIL_vd_chan_param* pChanvdcfg)
{
    int fd = 0;

    if(pChanvdcfg == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_VD_CHNL_CONFIG_GET, pChanvdcfg) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_CODEC_CTL_SetChLogic2PhyMap
 discription:
     set logic channel to physical channel map table
 input:
 	 unsigned int u32ChipID: chip id
	 CODEC_CHIP_CHLOGIC2PHY_MAP* pMapTbl: map table
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_CODEC_CTL_SetChLogic2PhyMap(unsigned int u32ChipID, CODEC_CHIP_CHLOGIC2PHY_MAP *pMapTbl)
{
    int fd = 0;

    if(pMapTbl == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_LOGIC_MAP_TABLE_SET, pMapTbl) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_CODEC_CTL_SetChLogic2PhyMap
 discription:
     get logic channel to physical channel map table
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 CODEC_CHIP_CHLOGIC2PHY_MAP* pMapTbl: map table
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_CODEC_CTL_GetChLogic2PhyMap(unsigned int u32ChipID, CODEC_CHIP_CHLOGIC2PHY_MAP* pMapTbl)
{
    int fd = 0;

    if(pMapTbl == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_LOGIC_MAP_TABLE_GET, pMapTbl) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_CODEC_CTL_SetVdConfigParam
 discription:
     set vd config parament
 input:
 	 unsigned int u32ChipID: chip id
 	 VD_CONFIG_PARAM* pVdParam: vd parament data
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_CODEC_CTL_SetVdConfigParam(unsigned int u32ChipID, VD_CONFIG_PARAM* pVdParam)
{
    int fd = 0;
    if(pVdParam == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/

    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;

    if(ioctl(fd, ISIL_CHIP_VD_MAP_PARAM_SET, pVdParam) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_CODEC_CTL_GetVdConfigParam
 discription:
     Get vd config parament
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 VD_CONFIG_PARAM* pVdParam: vd parament data
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_CODEC_CTL_GetVdConfigParam(unsigned int u32ChipID, VD_CONFIG_PARAM* pVdParam)
{
    int fd = 0;

    if(pVdParam == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_VD_MAP_PARAM_GET, pVdParam) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_CODEC_CTL_SetVpConfigParam
 discription:
     set vp config parament
 input:
 	 unsigned int u32ChipID: chip id
 	 VP_CONFIG_PARAM* pVdParam: vp parament data
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_CODEC_CTL_SetVpConfigParam(unsigned int u32ChipID, VP_CONFIG_PARAM* pVpParam)
{
    int fd = 0;

    if(pVpParam == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_VP_MAP_PARAM_SET, pVpParam) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_CODEC_CTL_GetVpConfigParam
 discription:
     Get vp config parament
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 VP_CONFIG_PARAM* pVpParam: vp parament data
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_CODEC_CTL_GetVpConfigParam(unsigned int u32ChipID, VP_CONFIG_PARAM* pVpParam)
{
    int fd = 0;

    if(pVpParam == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_VP_MAP_PARAM_GET, pVpParam) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/********************************************************
function:
     ISIL_ENC_CTL_GetViChipMaxCh
 discription:
     get codec chip max vi channel
 input:
		unsigned int u32ChipID: chip id
 output:
         ENC_VI_MAXCH *pChipCh: pointer to chip max channel
 return:
     CODEC_ERR_OK: success
 	 CODEC_ERR_FAIL:fail
	 CODEC_ERR_NOT_EXIST: chip not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

int ISIL_ENC_CTL_GetViChipMaxCh(unsigned int u32ChipID, ENC_VI_MAXCH *pChipCh)
{
	int fd = 0;

	if(u32ChipID >= CODEC_MAXCHIP){
		#ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, CODEC_MAXCHIP);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pChipCh == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(!CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_VIN_CHAN_NUMBER_GET, pChipCh) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/********************************************************
function:
     ISIL_ENC_CTL_GetAiChipMaxCh
 discription:
     get codec chip max ai channel
 input:
		unsigned int u32ChipID: chip id
 output:
         ENC_AI_MAXCH *pChipCh: pointer to chip max channel
 return:
     CODEC_ERR_OK: success
 	 CODEC_ERR_FAIL:fail
	 CODEC_ERR_NOT_EXIST: chip not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

int ISIL_ENC_CTL_GetAiChipMaxCh(unsigned int u32ChipID, ENC_AI_MAXCH *pAiCh)
{
	int fd = 0;

	if(u32ChipID >= CODEC_MAXCHIP){
	    #ifdef CODEC_CTL_DEBUG
	    CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, CODEC_MAXCHIP);
	    #endif
	    return CODEC_ERR_INVAL;
	}

    if(pAiCh == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(!CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_AIN_CHAN_NUMBER_GET, pAiCh) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
        perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


  
//as usr config, calc and set new vd parament,open or close nonrealtime
int ISIL_CODEC_CTL_CalcSetVd(unsigned int u32ChipID, VD_CONFIG_REALTIME* pRealtime)
{
    VGROUP_S vgroup[ISIL_MAX_GRP];
    VD_CONFIG_PARAM VdParam;
    VD_CONFIG_REALTIME vdtmp;
    int hdno = 0;
    int i = 0;
    int valid_cnt = 0;

    if(!pRealtime || !pRealtime->chan || !pRealtime->video_size || !pRealtime->frame_rate){
		#ifdef CODEC_CTL_DEBUG
		CODEC_DEBUG(" pointor is NULL ");
		#endif
		return CODEC_ERR_INVAL;
    }
    
    memcpy(&vdtmp, pRealtime, sizeof(vdtmp));

    for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++){
		if(i < 8){
			if((pRealtime->video_size[i] == ISIL_VIDEO_SIZE_H960)
			|| (pRealtime->video_size[i] == ISIL_VIDEO_SIZE_720P)
			|| (pRealtime->video_size[i] == ISIL_VIDEO_SIZE_1080P)){
				#ifdef CODEC_CTL_DEBUG
				CODEC_DEBUG(" HD only exist in channel 8 and 12 ");
				#endif
				return CODEC_ERR_INVAL;
			}
		}

		//calc vd config parament
		//paraments convert
		if((pRealtime->video_size[8] == ISIL_VIDEO_SIZE_H960)
		|| (pRealtime->video_size[8] == ISIL_VIDEO_SIZE_720P)
		|| (pRealtime->video_size[8] == ISIL_VIDEO_SIZE_1080P)){
			hdno++;
		}

		if((pRealtime->video_size[12] == ISIL_VIDEO_SIZE_H960)
			|| (pRealtime->video_size[12] == ISIL_VIDEO_SIZE_720P)
			|| (pRealtime->video_size[12] == ISIL_VIDEO_SIZE_1080P)){
				hdno++;
		}

		//video size convert
		if(pRealtime->video_size[i] == ISIL_VIDEO_SIZE_D1){
			vdtmp.video_size[i]  = ISIL_D1;
		}else if(pRealtime->video_size[i] == ISIL_VIDEO_SIZE_HALF_D1){
			vdtmp.video_size[i]  = ISIL_HalfD1;
		}else if(pRealtime->video_size[i] == ISIL_VIDEO_SIZE_CIF){
			vdtmp.video_size[i]  = ISIL_CIF;
		}else if(pRealtime->video_size[i] == ISIL_VIDEO_SIZE_HCIF){
			vdtmp.video_size[i]  = ISIL_HCIF;
		}else if(pRealtime->video_size[i] == ISIL_VIDEO_SIZE_QCIF){
			vdtmp.video_size[i]  = ISIL_QCIF;
		}
    }

	if(hdno){
		for(i = 8; i < ISIL_PHY_VD_CHAN_NUMBER; i++){
			vdtmp.video_size[i] = 0;
			vdtmp.frame_rate[i] = 0;
		}
	}


	for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++){
		if(vdtmp.frame_rate[i]){
			valid_cnt++;
		}
	}


	fprintf(stderr, "-----valid_cnt: %d---\n", valid_cnt);

#if 0
	int xx;
	for(xx = 0; xx < 16; xx++){
		fprintf(stderr, "chan: %d,vsize: %d,frate: %d,std: %d,drop: %d\n",
				vdtmp.chan[xx],vdtmp.video_size[xx],vdtmp.frame_rate[xx],vdtmp.video_std,vdtmp.drop_frame);
	}
#endif

    if(cb_ctrl_calculate(vdtmp.chan, vdtmp.video_size, vdtmp.frame_rate, vdtmp.video_std,
			 vgroup, valid_cnt, vdtmp.drop_frame) < 0){

		#ifdef CODEC_CTL_DEBUG
		CODEC_DEBUG(" cb_ctrl_calculate fail");
		#endif
		return CODEC_ERR_FAIL;
    }

#if 1
    int xx;
    for(xx = 0; xx < 16; xx++){
    	fprintf(stderr, "input chan: %d,vsize: %d,frate: %d,std: %d,drop: %d\n",
    			vdtmp.chan[xx],vdtmp.video_size[xx],vdtmp.frame_rate[xx],vdtmp.video_std,vdtmp.drop_frame);


        fprintf(stderr, "--wb out---%d,%d,%d,%d,%d,%d,%d,%d,%d-----\n",
        		vgroup[xx/4].grp_id,
        		vgroup[xx/4].in_select[xx%ISIL_CHN_NO],
        		vgroup[xx/4].in_map[xx%ISIL_CHN_NO],
        		vgroup[xx/4].in_fmt[xx%ISIL_CHN_NO],
        		vgroup[xx/4].in_rate[xx%ISIL_CHN_NO],
        		vgroup[xx/4].out_rate[xx%ISIL_CHN_NO],
        		vgroup[xx/4].totalfrm,
        		vgroup[xx/4].mode,
        		vgroup[xx/4].total
        		);

    }
#endif

    //parament 
    for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++){

		VdParam.map_info[i].phy_slot_id = vgroup[i/4].in_select[i%ISIL_CHN_NO];

		printf("\nphy_slot_id: %d ", VdParam.map_info[i].phy_slot_id);

		VdParam.map_info[i].map_logic_slot_id = i;

		printf("map_logic_slot_id: %d ", VdParam.map_info[i].map_logic_slot_id);

		VdParam.map_info[i].fps = vgroup[i/4].out_rate[i%ISIL_CHN_NO];

		printf("fps: %d ", VdParam.map_info[i].fps);

		if(VdParam.map_info[i].fps){
			VdParam.map_info[i].enable = 1;
		}else{
			VdParam.map_info[i].enable = 0;
		}

		printf("enable: %d ", VdParam.map_info[i].enable);

		if(pRealtime->video_std == ISIL_VI_PAL){
			VdParam.map_info[i].roundTableStride = 24;
		}else if(pRealtime->video_std == ISIL_VI_NTSC){
			VdParam.map_info[i].roundTableStride = 29;
		}else{
			#ifdef CODEC_CTL_DEBUG
			CODEC_DEBUG(" video_std not support ");
			#endif
			return CODEC_ERR_INVAL;
		}

		printf("roundTableStride: %d ", VdParam.map_info[i].roundTableStride);


		switch(vgroup[i/4].in_fmt[i%ISIL_CHN_NO]){
			case ISIL_D1:
			VdParam.map_info[i].video_size = ISIL_VIDEO_SIZE_D1;
			break;

			case ISIL_HalfD1:
			VdParam.map_info[i].video_size = ISIL_VIDEO_SIZE_HALF_D1;
			break;

			case ISIL_CIF:
			VdParam.map_info[i].video_size = ISIL_VIDEO_SIZE_CIF;
			break;

			case ISIL_HCIF:
			VdParam.map_info[i].video_size = ISIL_VIDEO_SIZE_HCIF;
			break;

			case ISIL_QCIF:
			VdParam.map_info[i].video_size = ISIL_VIDEO_SIZE_QCIF;
			break;

			default:
			#ifdef CODEC_CTL_DEBUG
			CODEC_DEBUG(" in_fmt not support ");
			#endif
			return CODEC_ERR_INVAL;
			break;
		}

		printf("video_size: %d ", VdParam.map_info[i].video_size);
	
		VdParam.map_info[i].logic_slot_discard_table = vgroup[i/4].in_map[i%ISIL_CHN_NO];
	
		printf("logic_slot_discard_table: %d\n", VdParam.map_info[i].logic_slot_discard_table);

		VdParam.map_info[i].interleave = pRealtime->interleave[i];
    }

    for(i = 0; i < hdno; i++){
    	//printf("%s i = %d, map index\n", __FUNCTION__, i, 8+(i-1)*4);
    	VdParam.map_info[8+(i-1)*4].enable = 1;
    	VdParam.map_info[8+(i-1)*4].fps = pRealtime->frame_rate[8+(i-1)*4];
    	VdParam.map_info[8+(i-1)*4].video_size = pRealtime->video_size[8+(i-1)*4];
    }

    //set vd parament to driver by ioctl
    if(ISIL_CODEC_CTL_SetVdConfigParam(u32ChipID, &VdParam) < 0){
		#ifdef CODEC_CTL_DEBUG
		CODEC_DEBUG(" ISIL_CODEC_CTL_SetVdConfigParam fail ");
		#endif
		return CODEC_ERR_FAIL;
    }
    return 0;
}


//as usr config, calc and set new vd parament,open or close nonrealtime
int ISIL_CODEC_CTL_CalcSetVp(unsigned int u32ChipID, VP_CONFIG* pVpcfg)
{
	u32ChipID = u32ChipID;
	pVpcfg = pVpcfg;
    return 0;
}

//av sync start
/***********************************************************
 function:
     ISIL_DEC_CTL_StartAVSync
 discription:
     when decoding, set av sync start
 input:
 	 unsigned int u32ChipID: chip id
	 DEC_AV_SYNC* pAVSync: av sync data
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/

int ISIL_DEC_CTL_StartAVSync(unsigned int u32ChipID, DEC_AV_SYNC* pAVSync)
{
    int fd = 0;

    if(pAVSync == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_START_AVSYNC, pAVSync) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

//av sync stop
/***********************************************************
 function:
     ISIL_DEC_CTL_StopAVSync
 discription:
     when decoding, set av sync stop
 input:
 	 unsigned int u32ChipID: chip id
	 DEC_AV_SYNC* pAVSync: av sync data
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_DEC_CTL_StopAVSync(unsigned int u32ChipID, DEC_AV_SYNC* pAVSync)
{
    int fd = 0;

    if(pAVSync == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_STOP_AVSYNC, pAVSync) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_DEC_CTL_TSResync(unsigned int u32ChipID, VDEC_CH_TSRESYNC* pTS)
{
    int fd = 0;

    if(pTS == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(u32ChipID > CODEC_MAXCHIP){
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip id > max chip ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CODEC_CHAN_TS_RESYNC, pTS) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


int ISIL_DEC_CTL_BindDec2VO(unsigned int u32ChipID, BIND_DEC2VO *pD2VO)
{
	int fd = 0;

	if(u32ChipID >= CODEC_MAXCHIP){
		#ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, CODEC_MAXCHIP);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pD2VO == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(!CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_BIND_H264D2VO, pD2VO) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;

}


int ISIL_DEC_CTL_UnbindDec2VO(unsigned int u32ChipID, BIND_DEC2VO *pD2VO)
{
	int fd = 0;

	if(u32ChipID >= CODEC_MAXCHIP){
		#ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, CODEC_MAXCHIP);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pD2VO == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(!CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_UNBIND_H264D2VO, pD2VO) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;

}



int ISIL_DEC_CTL_BindVI2VO(unsigned int u32ChipID, BIND_VI2VO *pVi2VO)
{
	int fd = 0;

	if(u32ChipID >= CODEC_MAXCHIP){
		#ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, CODEC_MAXCHIP);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pVi2VO == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(!CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_BIND_VI2VO, pVi2VO) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;

}


int ISIL_DEC_CTL_UnbindVI2VO(unsigned int u32ChipID, BIND_VI2VO *pVi2VO)
{
    int fd = 0;

    if(u32ChipID >= CODEC_MAXCHIP){
	#ifdef CODEC_CTL_DEBUG
	CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, CODEC_MAXCHIP);
	#endif
	return CODEC_ERR_INVAL;
    }

    if(pVi2VO == NULL)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!CODEC_CHIP_TABLE[u32ChipID].s8Open)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" chip had not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = CODEC_CHIP_TABLE[u32ChipID].s32Chipfd;
    if(ioctl(fd, ISIL_CHIP_UNBIND_VI2VO, pVi2VO) < 0)
    {
        #ifdef CODEC_CTL_DEBUG
        CODEC_DEBUG(" ioctl fail ");
	perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;

}



