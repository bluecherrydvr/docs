
/******************** (C) COPYRIGHT 2010 Techwell Inc. ********************
* File Name         : isil_ao.c
* Author            : dengjing
* Version           : V0.1
* Date              : 2010-05-12
* Description       : audio input API,all opeatator of audio input
* Change log		: 
*                   
********************************************************************************
* THE PRESENT SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL PROAODE CUSTOMERS 
* WITH CODING INFORMATION TO SPEED UP THEIR PRODUCT DEVELOPMENT, BUT WITHOUT
* ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
* FOR A PARTICULAR PURPOSE. AS A RESULT, TECHWELL SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAOMS ARISING
* FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAONED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>

#include "isil_ao.h"
#include "isil_codec_debug.h"
#include "isil_codec_errcode.h"



#define AO_DEBUG

/*save ao chip info*/
typedef struct
{
    char         u8ChipOpen;//chip had been open
    char         u8Reserved[3];
    int          s32ChipFd;//chip fd 
}AO_CHIP_INFO;

AO_CHIP_INFO AO_CHIP_TBL[AO_CHIP_MAX];



/***********************************************************
 function:
     ISIL_AO_CTL_OpenChip
 discription:
     open chip control deaoce and get fd
 input:
     char *pChipNode: chip deaoce node path
     unsigned int u32ChipID: chip id
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:faol
     CODEC_ERR_EXIT: chip had been open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID)
{
    int fd = 0;

    if(u32ChipID >= AO_CHIP_MAX)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" ao chip ID[%d] > Max[%d],invalid", u32ChipID, AO_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(AO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" had open ");
        #endif
        return CODEC_ERR_EXIST;
    }

    fd = open(pChipNode, O_RDWR);
    if(fd < 0)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" open ao chip: %s faol", pChipNode);
        #endif
        return CODEC_ERR_FAIL;
    }

    AO_CHIP_TBL[u32ChipID].u8ChipOpen = 1;
    AO_CHIP_TBL[u32ChipID].s32ChipFd = fd;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_AO_CTL_CloseChip
 discription:
     close chip control deaoce
 paramnet:
     input:
         unsigned int u32ChipID: chip id
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:faol
     CODEC_ERR_NOT_EXIT: chip had been closed
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_CTL_CloseChip(unsigned int u32ChipID)
{
    int iRet = 0;

    if(u32ChipID >= AO_CHIP_MAX)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" ao chip ID[%d] > Max[%d],invalid", u32ChipID, AO_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    iRet = close(AO_CHIP_TBL[u32ChipID].s32ChipFd);
    if(iRet < 0)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" close ao chip: %d faol", u32ChipID);
        #endif
        return CODEC_ERR_FAIL;
    }

    AO_CHIP_TBL[u32ChipID].u8ChipOpen = 0;
    AO_CHIP_TBL[u32ChipID].s32ChipFd = 0;
    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_AO_CTL_GetChipHandle
 discription:
     get chip fd
 paramnet:
     input:
         unsigned int u32ChipID: chip id
     output:
         AO_HANDLE* pChipHandle: chip fd
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_NOT_EXIT: chip had been close
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_CTL_GetChipHandle(unsigned int u32ChipID, AO_HANDLE* pChipHandle)
{
     int iRet = 0;

     iRet = iRet;

    if(u32ChipID >= AO_CHIP_MAX)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" ao chip ID[%d] > Max[%d],invalid", u32ChipID, AO_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" chip had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    *pChipHandle = AO_CHIP_TBL[u32ChipID].s32ChipFd;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_AO_CTL_SendMsg
 discription:
     all chip control operate by msg 
 paramnet:
     input:
         AO_CTL_MSG *pMsg: control data struct
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:faol
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_CTL_SendMsg(unsigned int u32ChipID, AO_CTL_MSG *pMsg)//finish all operator of chip,eg:bind VO, mux ,demux etc
{
    int fd = 0;

    if(pMsg == NULL)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = AO_CHIP_TBL[u32ChipID].s32ChipFd;
    if(ioctl(fd, ISIL_AO_CTL_OP, pMsg) < 0)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" ioctl faol ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_AO_SetCfg
 discription:
     set aodeo input channel config
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         AO_CH_CFG* pViCfg: point to config data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:faol
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_SetCfg(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_CH_CFG* pAoCfg)
{
    u32ChipID = u32ChipID;
    u32Ch = u32Ch;
    eChType = eChType;
    pAoCfg = pAoCfg;

    return 0;
}

/***********************************************************
 function:
     ISIL_AO_GetCfg
 discription:
     get aodeo input config
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         AO_CH_CFG* pViCfg: point to config data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:faol
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_GetCfg(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_CH_CFG* pAoCfg)
{
    u32ChipID = u32ChipID;
    u32Ch = u32Ch;
    eChType = eChType;
    pAoCfg = pAoCfg;
   return 0;
}


/***********************************************************
 function:
     ISIL_AO_SendData, now not use
 discription:
     send data to ao , for testing
 paramnet:
     input:
         none
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:faol
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_SendData(unsigned int u32ChipID, unsigned int u8Ch, AO_CH_TYPE eChType, char* pBuf, unsigned int u32Len)//now not use
{
   u32ChipID = u32ChipID;
    u8Ch = u8Ch;
    eChType = eChType;
    pBuf = pBuf;
    u32Len = u32Len;
    return 0;
}


/***********************************************************
 function:
     ISIL_AO_GetData, now not use
 discription:
     get ao raw data , for testing
 paramnet:
     input:
         none
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:faol
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_GetData(unsigned int u32ChipID, unsigned int u8Ch, AO_CH_TYPE eChType, char* pBuf, unsigned int u32Len)//now not use
{
    u32ChipID = u32ChipID;
    u8Ch = u8Ch;
    eChType = eChType;
    pBuf = pBuf;
    u32Len = u32Len;
    return 0;
}


/***********************************************************
 function:
     ISIL_AO_GetChHandle
 discription:
     get ao channel handle
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
     output:
         AO_HANDLE* pChHandle: point to ao channel handle
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:faol
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_AO_GetChHandle(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_HANDLE* pChHandle)
{
    if(eChType == AO_CH_TYPE_ANALOG)
    {
        if(u32Ch >= AO_CHIP_ANALOG_MAXCH)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" ao channel ID[%d] > Max[%d],invalid", u32Ch, AO_CHIP_ANALOG_MAXCH);
            #endif
            return CODEC_ERR_INVAL;
        }
    
        if(pChHandle == NULL)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" point is null");
            #endif
            return CODEC_ERR_INVAL;
        }
    
        if(!AO_CHIP_TBL[u32ChipID].u8ChipOpen)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" chip not open ");
            #endif
            return CODEC_ERR_NOT_EXIST;
        }

        /*how todo, ioctl driver,set cropping*/
        *pChHandle = AO_CHIP_TBL[u32ChipID].s32ChipFd;
    
        return CODEC_ERR_OK;
    }
    else if(eChType == AO_CH_TYPE_DIGITAL)
    {
        if(u32Ch >= AO_CHIP_DIGITAL_MAXCH)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" ao channel ID[%d] > Max[%d],invalid", u32Ch, AO_CHIP_DIGITAL_MAXCH);
            #endif
            return CODEC_ERR_INVAL;
        }
    
        if(pChHandle == NULL)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" point is null");
            #endif
            return CODEC_ERR_INVAL;
        }
    
        if(!AO_CHIP_TBL[u32ChipID].u8ChipOpen)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" chip not open ");
            #endif
            return CODEC_ERR_NOT_EXIST;
        }

        /*how todo, ioctl driver,set cropping*/
        *pChHandle = AO_CHIP_TBL[u32ChipID].s32ChipFd;
    
        return CODEC_ERR_OK;
    }
    else 
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" ao ch type is error ");
        #endif
        return CODEC_ERR_INVAL;
    }
}

int ISIL_AO_Mute(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_CH_MUTE *pAoMute)
{
    if(eChType == AO_CH_TYPE_ANALOG)
    {
        if(u32Ch >= AO_CHIP_ANALOG_MAXCH)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" ao channel ID[%d] > Max[%d],invalid", u32Ch, AO_CHIP_ANALOG_MAXCH);
            #endif
            return CODEC_ERR_INVAL;
        }
    
        if(pAoMute == NULL)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" point is null");
            #endif
            return CODEC_ERR_INVAL;
        }
    
        if(!AO_CHIP_TBL[u32ChipID].u8ChipOpen)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" chip not open ");
            #endif
            return CODEC_ERR_NOT_EXIST;
        }
    
        /*how todo, ioctl driver,set cropping*/
        if(ioctl(AO_CHIP_TBL[u32ChipID].s32ChipFd,
                 ISIL_AO_SET_SINGLE_AUDIO_MUTE_CHANNEL, pAoMute) < 0){
            #ifdef AO_DEBUG
            CODEC_DEBUG(" ioctl fail");
            #endif
            return CODEC_ERR_FAIL;
        }
    
        return CODEC_ERR_OK;
    }
    else if(eChType == AO_CH_TYPE_DIGITAL)
    {
        if(u32Ch >= AO_CHIP_DIGITAL_MAXCH)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" ao channel ID[%d] > Max[%d],invalid", u32Ch, AO_CHIP_DIGITAL_MAXCH);
            #endif
            return CODEC_ERR_INVAL;
        }
    
        if(pAoMute == NULL)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" point is null");
            #endif
            return CODEC_ERR_INVAL;
        }
    
        if(!AO_CHIP_TBL[u32ChipID].u8ChipOpen)
        {
            #ifdef AO_DEBUG
            CODEC_DEBUG(" chip not open ");
            #endif
            return CODEC_ERR_NOT_EXIST;
        }
    
        /*how todo, ioctl driver,set cropping*/
         if(ioctl(AO_CHIP_TBL[u32ChipID].s32ChipFd,
                 ISIL_AO_SET_SINGLE_AUDIO_MUTE_CHANNEL, pAoMute) < 0){
            #ifdef AO_DEBUG
            CODEC_DEBUG(" ioctl fail");
            #endif
            return CODEC_ERR_FAIL;
         }
        return CODEC_ERR_OK;
    }
    else 
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" ao ch type is error ");
        #endif
        return CODEC_ERR_INVAL;
    }
}

int ISIL_AO_MuxFinalCh(unsigned int u32ChipID, AO_MUX_FINAL_OUTPUT_CH* pAoFinalCh)
{
    if(pAoFinalCh == NULL)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!AO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef AO_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(AO_CHIP_TBL[u32ChipID].s32ChipFd,
             ISIL_AO_SET_FINAL_MIX_AUDIO_OUTPUT, pAoFinalCh) < 0){
        #ifdef AO_DEBUG
        CODEC_DEBUG(" ioctl fail");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_AO_SetVol(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_CH_VOLUME *pvol)
{
	u32ChipID = u32ChipID;
	u32Ch = u32Ch;
	eChType =eChType;
	pvol = pvol;
	return CODEC_ERR_OK;
}








