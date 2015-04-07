/******************** (C) COPYRIGHT 2010 Techwell Inc. ********************
* File Name         : isil_ai.c
* Author            : dengjing
* Version           : V0.1
* Date              : 2010-05-12
* Description       : audio input API,all opeatator of audio input
* Change log		:
*
********************************************************************************
* THE PRESENT SOFISILARE IS DISTRIBUTED IN THE HOPE THAT IT WILL PROAIDE CUSTOMERS
* WITH CODING INFORMATION TO SPEED UP THEIR PRODUCT DEVELOPMENT, BUT WITHOUT
* ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
* FOR A PARTICULAR PURPOSE. AS A RESULT, TECHWELL SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH SOFISILARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>

#include "isil_ai.h"
#include "isil_codec_debug.h"
#include "isil_codec_errcode.h"
#include "isil_codec_common.h"
#include "isil_interface.h"


#define AI_DEBUG


typedef struct
{
    char         u8ChipOpen;//chip had been open
	char         u8Reserved[3];
    int          s32ChipFd;//chip fd
}AI_CHIP_INFO;

AI_CHIP_INFO AI_CHIP_TBL[AI_CHIP_MAX];


/***********************************************************
function:
    ISIL_AI_CTL_OpenChip
description:
    open chip control deaice and get fd
input:
    char *pChipNode: chip deaice node path
     unsigned int u32ChipID: chip id
output:
    none
return:
    CODEC_ERR_OK: success
    CODEC_ERR_FAIL:fail
    CODEC_ERR_EXIT: chip had been open
    CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_AI_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID)
{
    int fd = 0;

    if(u32ChipID >= AI_CHIP_MAX){
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ai chip ID[%d] > Max[%d],invalid", u32ChipID, AI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(pChipNode == NULL){
		#ifdef AI_DEBUG
        CODEC_DEBUG(" pointer is null\n");
        #endif
        return CODEC_ERR_INVAL;
	}


    if(AI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" had open ");
        #endif
        return CODEC_ERR_EXIST;
    }

    fd = open(pChipNode, O_RDWR);
    if(fd < 0)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" open ai chip: %s fail", pChipNode);
        #endif
        return CODEC_ERR_FAIL;
    }

    AI_CHIP_TBL[u32ChipID].u8ChipOpen = 1;
    AI_CHIP_TBL[u32ChipID].s32ChipFd = fd;
    return CODEC_ERR_OK;
}


/***********************************************************
function  :
    ISIL_AI_CTL_CloseChip
discription :
    close chip
input:
    unsigned int u32ChipID: chip id
output:
    none
return:
    CODEC_ERR_OK: success
    CODEC_ERR_FAIL:fail
    CODEC_ERR_NOT_EXIT: chip had been closed
    CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AI_CTL_CloseChip(unsigned int u32ChipID)
{
    int iRet = 0;

    if(u32ChipID >= AI_CHIP_MAX)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ai chip ID[%d] > Max[%d],invalid", u32ChipID, AI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    iRet = close(AI_CHIP_TBL[u32ChipID].s32ChipFd);
    if(iRet < 0)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" close ai chip: %d fail", u32ChipID);
        #endif
        return CODEC_ERR_FAIL;
    }

    AI_CHIP_TBL[u32ChipID].u8ChipOpen = 0;
    AI_CHIP_TBL[u32ChipID].s32ChipFd = 0;
    return CODEC_ERR_OK;
}


/***********************************************************
function:
    ISIL_AI_CTL_GetChipHandle
discription:
    get chip fd
input:
    unsigned int u32ChipID: chip id
output:
    AI_HANDLE* pChipHandle: chip fd
return:
    CODEC_ERR_OK: success
    CODEC_ERR_NOT_EXIT: chip had been close
    CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AI_CTL_GetChipHandle(unsigned int u32ChipID, AI_HANDLE* pChipHandle)
{
    // int iRet = 0;

    if(u32ChipID >= AI_CHIP_MAX)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ai chip ID[%d] > Max[%d],invalid", u32ChipID, AI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" chip had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    *pChipHandle = AI_CHIP_TBL[u32ChipID].s32ChipFd;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_AI_CTL_SendMsg
 discription:
     all chip control operate
 paramnet:
     input:
         AI_CHIP_MSG *pMsg : control data struct
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_EXIST;chip had open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AI_CTL_SendMsg(unsigned int u32ChipID, AI_CHIP_MSG *pMsg)//finish all operator of chip,eg:bind VO, mux ,demux etc
{
    int fd = 0;

	if(u32ChipID >= AI_CHIP_MAX){
		#ifdef AI_DEBUG
		CODEC_DEBUG(" ai channel is overflow ");
		#endif
		return CODEC_ERR_INVAL;
	}

    if(pMsg == NULL)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(AI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" chip had open ");
        #endif
        return CODEC_ERR_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = AI_CHIP_TBL[u32ChipID].s32ChipFd;
    if(ioctl(fd, ISIL_AI_CTL_OP, pMsg) < 0)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_AI_CHIP_SetSystem
 discription:
     set ai chip system config
 input:
     unsigned int u32ChipID: chip id
     AI_CHIP_SYS  *pAiSys :pointe to AI_CHIP_SYS
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST:chip not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

int ISIL_AI_CHIP_SetSystem(unsigned int u32ChipID, AI_CHIP_SYS* pAiSys)//finish all operator of chip,eg:bind VO, mux ,demux etc
{
    int fd = 0;

    if(pAiSys == NULL)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= AI_CHIP_MAX){
		#ifdef AI_DEBUG
        CODEC_DEBUG(" ai channel is overflow ");
        #endif
        return CODEC_ERR_INVAL;
	}

    if(!AI_CHIP_TBL[u32ChipID].u8ChipOpen){
        #ifdef AI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = AI_CHIP_TBL[u32ChipID].s32ChipFd;
    if(ioctl(fd, ISIL_AI_CTL_OP, pAiSys) < 0)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_AI_SetCfg
 discription:
     set aideo input channel config
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         AI_CH_CFG* pViCfg: point to config data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AI_SetCfg(unsigned int u32Ch, unsigned int u32ChipID, AI_CH_CFG* pViCfg)
{
    if(u32Ch >= AI_CHIP_CH_MAX){
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ai channel ID[%d] >= Max[%d],invalid", u32Ch, AI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= AI_CHIP_MAX){
		#ifdef AI_DEBUG
        CODEC_DEBUG(" ai chipID[%d] >= Max[%d],invalid", u32ChipID, AI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pViCfg == NULL)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_AI_GetCfg
 discription:
     get aideo input config
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         AI_CH_CFG* pViCfg: point to config data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AI_GetCfg(unsigned int u32Ch, unsigned int u32ChipID, AI_CH_CFG* pViCfg)
{
    if(u32Ch >= AI_CHIP_CH_MAX)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ai channel ID[%d] > Max[%d],invalid", u32Ch, AI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= AI_CHIP_MAX){
		#ifdef AI_DEBUG
        CODEC_DEBUG(" ai chipID[%d] >= Max[%d],invalid", u32ChipID, AI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}


    if(pViCfg == NULL)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/

    return CODEC_ERR_OK;
}



/***********************************************************
function:
    ISIL_AI_SendData, now not use
discription:
    send data to ai , for testing
input:
    none
output:
    none
return:
    CODEC_ERR_OK: success
    CODEC_ERR_FAIL:fail
    CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
    CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_AI_SendData(unsigned int u32ChipID, unsigned int u8Ch, char* pBuf, unsigned int u32Len)//now not use
{
	u32ChipID = u32ChipID;
	u8Ch = u8Ch;
	pBuf = pBuf;
	u32Len = u32Len;
    return 0;
}


/***********************************************************
 function:
     ISIL_AI_GetData, now not use
 discription:
     get ai raw data , for testing
 input:
     none
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AI_GetData(unsigned int u32ChipID, unsigned int u8Ch, char* pBuf, unsigned int u32Len)//now not use
{
	u32ChipID = u32ChipID;
	u8Ch = u8Ch;
	pBuf = pBuf;
	u32Len = u32Len;
    return 0;
}


/***********************************************************
 function:
     ISIL_AI_GetChHandle
 discription:
     get ai channel handle
 input:
     unsigned int u32Ch: channel id
     unsigned int u32ChipID : chip id
 output:
     AI_HANDLE* pChHandle: point to ai channel handle
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AI_GetChHandle(unsigned int u32ChipID, unsigned int u32Ch, AI_HANDLE* pChHandle)
{
    if(u32Ch >= AI_CHIP_CH_MAX)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ai channel ID[%d] > Max[%d],invalid", u32Ch, AI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= AI_CHIP_MAX){
		#ifdef AI_DEBUG
        CODEC_DEBUG(" ai chipID[%d] >= Max[%d],invalid", u32ChipID, AI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}


    if(pChHandle == NULL)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    *pChHandle = AI_CHIP_TBL[u32ChipID].s32ChipFd;

    return CODEC_ERR_OK;
}


/***********************************************************
function:
    ISIL_AI_Adjust_Volume
discription:
    get chip fd
input:
    unsigned int u32ChipID: chip id
    unsigned int u32Ch : channel
    AI_CH_VOLUME* pAiVol: point to volume data
output:
    none
return:
    CODEC_ERR_OK: success
    CODEC_ERR_NOT_EXIT: chip had been close
    CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
int ISIL_AI_Adjust_Volume(unsigned int u32ChipID, unsigned int u32Ch, AI_CH_VOLUME* pAiVol)
{
    if(u32Ch >= AI_CHIP_CH_MAX)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" ai channel ID[%d] > Max[%d],invalid", u32Ch, AI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= AI_CHIP_MAX){
		#ifdef AI_DEBUG
        CODEC_DEBUG(" ai chipID[%d] >= Max[%d],invalid", u32ChipID, AI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pAiVol == NULL)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(AI_CHIP_TBL[u32ChipID].s32ChipFd,
             ISIL_AI_SET_AUDIO_GAIN, pAiVol) < 0){

        #ifdef VO_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}














