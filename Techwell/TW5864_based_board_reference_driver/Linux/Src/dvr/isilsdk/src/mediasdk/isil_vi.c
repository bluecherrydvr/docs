/******************** (C) COPYRIGHT 2010 Techwell Inc. ********************
* File Name         : isil_vi.c
* Author            : dengjing
* Version           : V0.1
* Date              : 2010-05-12
* Description       : video input API,all opeatator of video input
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

#include "isil_vi.h"
#include "isil_codec_debug.h"
#include "isil_codec_errcode.h"
#include "isil_interface.h"



#define VI_DEBUG

/*save vi chip info*/

typedef struct
{
    char         u8ChipOpen;//chip had been open
	char         u8Reserved[3];
    int          s32ChipFd;//chip fd
    //VI_CH_INFO   sChInfo[VI_CHIP_CH_MAX];
}VI_CHIP_INFO;

VI_CHIP_INFO VI_CHIP_TBL[VI_CHIP_MAX];

/***********************************************************
 function:
     ISIL_VI_SetCropping
 discription:
     set video input channel croppong
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VI_CH_CROPPER* pCropper: point to cropping data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_SetCropping(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_CROPPER* pCropper)
{
    if(u32Ch >= VI_CHIP_CH_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi channel ID[%d] > Max[%d],invalid", u32Ch, VI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pCropper == NULL)
    {
       #ifdef VI_DEBUG
        CODEC_DEBUG(" pointor is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(VI_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VI_SET_ANALOG_INPUT_CROPPER,
                                                pCropper) < 0){
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VI_SetVideoType
 discription:
     set video input channel video type,eg PAL or NTSC etc
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VI_VIDEO_TYPE* pVideoType: tv foramt
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_SetVideoType(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_VIDEOTYPE* pVideoType)
{
    if(u32Ch >= VI_CHIP_CH_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi channel ID[%d] > Max[%d],invalid", u32Ch, VI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if( pVideoType == NULL)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set video type*/
    if(ioctl(VI_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VI_SET_VIDEO_TYPE,
                                                pVideoType) < 0){
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("fail ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VI_SetScaler
 discription:
     set channel scaler
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VI_CH_SCALER* pScaler: point scaler factor
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_SetScaler(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_SCALER* pScaler)
{
    if(u32Ch >= VI_CHIP_CH_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi channel ID[%d] > Max[%d],invalid", u32Ch, VI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pScaler == NULL)
    {
       #ifdef VI_DEBUG
        CODEC_DEBUG(" pointor is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(VI_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VI_SET_ANALOG_INPUT_SCALER, pScaler) < 0)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}



/***********************************************************
 function:
     ISIL_VI_SetNR
 discription:
     turn on/off noise reduce
 input:
     unsigned int u32Ch: channel id
     unsigned int u32ChipID : chip id
     VI_CH_NR*    pNR: point to VI_CH_NR
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_SetNR(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_NR* pNR)
{
    if(u32Ch >= VI_CHIP_CH_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi channel ID[%d] > Max[%d],invalid", u32Ch, VI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pNR == NULL)
    {
       #ifdef VI_DEBUG
        CODEC_DEBUG(" pointor is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(VI_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VI_SET_NOISE_REDUCTION, pNR) < 0)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VI_SelectChPort
 discription:
     select video input channel port
 input:
     unsigned int u32Ch: channel id
     unsigned int u32ChipID : chip id
     VI_CH_PORT* pChPort: point to VI_CH_PORT
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_SelectChPort(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_PORT* pChPort)
{
    if(u32Ch >= VI_CHIP_CH_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi channel ID[%d] > Max[%d],invalid", u32Ch, VI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pChPort == NULL)
    {
       #ifdef VI_DEBUG
        CODEC_DEBUG(" pointor is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(VI_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VI_SET_CHANNEL_PORT, pChPort) < 0)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VI_SetCfg
 discription:
     set video input channel config
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VI_CH_CFG* pViCfg: point to config data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_SetCfg(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_CFG* pViCfg)
{
    //fprintf(stderr, "----chip: %d, ch: %d------\n", u32ChipID, u32Ch);
    if(u32Ch >= VI_CHIP_CH_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi channel ID[%d] > Max[%d],invalid", u32Ch, VI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pViCfg == NULL)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(VI_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VI_SET_VIDEO_AD_PARM, pViCfg) < 0)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VI_GetCfg
 discription:
     get video input config
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VI_CH_CFG* pViCfg: point to config data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_GetCfg(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_CFG* pViCfg)
{
    if(u32Ch >= VI_CHIP_CH_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi channel ID[%d] > Max[%d],invalid", u32Ch, VI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pViCfg == NULL)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(VI_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VI_GET_VIDEO_AD_PARM, pViCfg) < 0)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}



/***********************************************************
 function:
     ISIL_VI_SendData, now not use
 discription:
     send data to vi , for testing
 paramnet:
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
int ISIL_VI_SendData(unsigned int u32ChipID, unsigned int u32Ch, char* pBuf, unsigned int u32Len)//now not use
{
	u32ChipID = u32ChipID;
	u32Ch = u32Ch;
	pBuf = pBuf;
	u32Len = u32Len;
	return 0;
}


/***********************************************************
 function:
     ISIL_VI_GetData, now not use
 discription:
     get vi raw data , for testing
 paramnet:
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
int ISIL_VI_GetData(unsigned int u32ChipID, unsigned int u8Ch, CODEC_STREAM_DATA* pdata)//now not use
{
	int ret = 0;

    if(!pdata || !pdata->pUsrBuf || !pdata->u32UsrBufLen){
		#ifdef VI_DEBUG
		CODEC_DEBUG(" point is null or len is zero ");
		#endif
		return CODEC_ERR_INVAL;
    }

    ret = read(VI_CHIP_TBL[u32ChipID].s32ChipFd, pdata->pUsrBuf, pdata->u32UsrBufLen);
    if(ret < 0){
		#ifdef VI_DEBUG
		CODEC_DEBUG(" read_r fail ");
		#endif
		return CODEC_ERR_FAIL;
    }

    pdata->u32ActiveLen = ret;
    pdata->u32FrameCnt = 1;


	return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VI_GetChHandle
 discription:
     get vi channel handle
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
     output:
         VI_HANDLE* pChHandle: point to vi channel handle
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_GetChHandle(unsigned int u32ChipID, unsigned int u32Ch, VI_HANDLE* pChHandle)
{
    if(u32Ch >= VI_CHIP_CH_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi channel ID[%d] > Max[%d],invalid", u32Ch, VI_CHIP_CH_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pChHandle == NULL)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    *pChHandle = VI_CHIP_TBL[u32ChipID].s32ChipFd;

    return CODEC_ERR_OK;
}


/*vi chip control*/

/***********************************************************
 function:
     ISIL_VI_CTL_OpenChip
 discription:
     open chip control device and get fd
 input:
         char *pChipNode: chip device node path
         unsigned int u32ChipID: chip id
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_EXIT: chip had been open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID)
{
    int fd = 0;
	
	

	
    if(u32ChipID >= VI_CHIP_MAX)
    {
		
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi chip ID[%d] > Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
		
        #ifdef VI_DEBUG
        CODEC_DEBUG(" had open ");
        #endif
        return CODEC_ERR_EXIST;
    }

	
	

    fd = open(pChipNode, O_RDWR);
    if(fd < 0)
    {
		
        #ifdef VI_DEBUG
        CODEC_DEBUG(" open vi chip: %s fail", pChipNode);
		perror("why: ");
        #endif
		
        return CODEC_ERR_FAIL;
    }

	
	
	
    VI_CHIP_TBL[u32ChipID].u8ChipOpen = 1;
    VI_CHIP_TBL[u32ChipID].s32ChipFd = fd;
	
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VI_CTL_CloseChip
 discription:
     close chip control device
 paramnet:
     input:
         unsigned int u32ChipID: chip id
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not  open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_CTL_CloseChip(unsigned int u32ChipID)
{
    int iRet = 0;

    if(u32ChipID >= VI_CHIP_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi chip ID[%d] > Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    iRet = close(VI_CHIP_TBL[u32ChipID].s32ChipFd);
    if(iRet < 0)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" close vi chip: %d fail", u32ChipID);
        #endif
        return CODEC_ERR_FAIL;
    }

    VI_CHIP_TBL[u32ChipID].u8ChipOpen = 0;
    VI_CHIP_TBL[u32ChipID].s32ChipFd = 0;
    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VI_CTL_GetChipHandle
 discription:
     get chip fd
 paramnet:
     input:
         unsigned int u32ChipID: chip id
     output:
         VI_HANDLE* pChipHandle: chip fd
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_NOT_EXIT: chip had been close
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
int ISIL_VI_CTL_GetChipHandle(unsigned int u32ChipID, VI_HANDLE* pChipHandle)
{
     int iRet = 0;

	 iRet = iRet;

    if(u32ChipID >= VI_CHIP_MAX)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" vi chip ID[%d] > Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    *pChipHandle = VI_CHIP_TBL[u32ChipID].s32ChipFd;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VI_CTL_SendMsg
 discription:
     all chip control operate
 paramnet:
 	 input:
         unsigned int u32ChipID: chip id
         VI_CHIP_MSG *pMsg : control data struct
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

int ISIL_VI_CTL_SendMsg(unsigned int u32ChipID, VI_CHIP_MSG *pMsg)//finish all operator of chip,eg:bind VO, mux ,demux etc
{
    int fd = 0;

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
        CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(pMsg == NULL)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(!VI_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" chip had not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = VI_CHIP_TBL[u32ChipID].s32ChipFd;
    if(ioctl(fd, ISIL_CHIP_CTL_OP, pMsg) < 0)
    {
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VI_StartAlarmType(unsigned int u32ChipID, unsigned int u32Chan, unsigned int alarmType)
{
	unsigned char *buff = NULL;
	int fd = 0;
	int offset = 0;
	isil_transparent_msg_header_t msgheader;
	isil_video_lost_detection_msg_t vlmsg;
	isil_video_night_detection_msg_t vnmsg;
	isil_video_blind_detection_msg_t vbmsg;
	isil_video_standard_change_detection_msg_t stdmsg;

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
		CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
		#endif
		return CODEC_ERR_INVAL;
	}

	if(u32Chan >= VI_CHIP_CH_MAX){
		#ifdef VI_DEBUG
		CODEC_DEBUG(" vi u32Chan[%d] >= Max[%d],invalid", u32Chan, VI_CHIP_CH_MAX);
		#endif
		return CODEC_ERR_INVAL;
	}

	//malloc buff
	buff = (unsigned char *)malloc(sizeof(msgheader) + sizeof(vlmsg) + sizeof(vnmsg) + sizeof(vbmsg) + sizeof(stdmsg));

	memset(&msgheader, 0x00, sizeof(msgheader));
	msgheader.isil_transparent_msg_total_len = sizeof(msgheader);

	offset += sizeof(msgheader);

	if((alarmType&VI_ALARM_VIDEO_LOST) || (alarmType&VI_ALARM_VIDEO_CONNECT)){
		msgheader.isil_transparent_msg_number++;
		msgheader.isil_transparent_msg_total_len += sizeof(vlmsg);

		vlmsg.msg_type = ISIL_VIDEO_LOST_DETECTION_MSG;
		vlmsg.msg_len = sizeof(vlmsg);
		vlmsg.chan_id = u32Chan;

		if(alarmType&VI_ALARM_VIDEO_LOST){
			vlmsg.video_lost_valid = 1;
		}

		if(alarmType&VI_ALARM_VIDEO_LOST){
			vlmsg.video_contect_valid = 1;
		}

		memcpy(buff+offset, &vlmsg, sizeof(vlmsg));
		offset += sizeof(vlmsg);
	}

	if((alarmType&VI_ALARM_VIDEO_NIGHT_DAYTONIGHT) || (alarmType&VI_ALARM_VIDEO_NIGHT_NIGHTTODAY)){
		msgheader.isil_transparent_msg_number++;
		msgheader.isil_transparent_msg_total_len += sizeof(vnmsg);

		vnmsg.msg_type = ISIL_VIDEO_NIGHT_DETECTION_MSG;
		vnmsg.msg_len = sizeof(vnmsg);
		vnmsg.chan_id = u32Chan;

		if(alarmType&VI_ALARM_VIDEO_NIGHT_DAYTONIGHT){
			vnmsg.video_night_valid_from_day_to_night = 1;
		}

		if(alarmType&VI_ALARM_VIDEO_NIGHT_NIGHTTODAY){
			vnmsg.video_night_valid_from_night_to_day = 1;
		}

		memcpy(buff+offset, &vnmsg, sizeof(vnmsg));
		offset += sizeof(vnmsg);
	}

	if((alarmType&VI_ALARM_VIDEO_BLIND_ADD) || (alarmType&VI_ALARM_VIDEO_BLIND_REMOVE)){
		msgheader.isil_transparent_msg_number++;
		msgheader.isil_transparent_msg_total_len += sizeof(vbmsg);

		vbmsg.msg_type = ISIL_VIDEO_BLIND_DETECTION_MSG;
		vbmsg.msg_len = sizeof(vbmsg);
		vbmsg.chan_id = u32Chan;

		if(alarmType&VI_ALARM_VIDEO_BLIND_ADD){
			vbmsg.video_blind_add_valid = 1;
		}

		if(alarmType&VI_ALARM_VIDEO_BLIND_REMOVE){
			vbmsg.video_blind_remove_valid = 1;
		}

		memcpy(buff+offset, &vbmsg, sizeof(vbmsg));
		offset += sizeof(vbmsg);
	}

	if(alarmType&VI_ALARM_VIDEO_STD_CHANGE){
		msgheader.isil_transparent_msg_number++;
		msgheader.isil_transparent_msg_total_len += sizeof(stdmsg);

		stdmsg.msg_type = ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG;
		stdmsg.msg_len = sizeof(stdmsg);
		stdmsg.chan_id = u32Chan;
		stdmsg.latest_video_standard = 1;
		stdmsg.curr_video_standard = 1;

		memcpy(buff+offset, &stdmsg, sizeof(stdmsg));
		offset += sizeof(stdmsg);
	}

	memcpy(buff, &msgheader, sizeof(msgheader));


	//how to do, ioctl
	fd = VI_CHIP_TBL[u32ChipID].s32ChipFd;
	fprintf(stderr, "-------chipid: %d,fd: %d---------\n", u32ChipID, fd);
	if(ioctl(fd, ISIL_VI_ALARM_CMD, buff) < 0)
	{
		#ifdef VI_DEBUG
		CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
		#endif
		return CODEC_ERR_FAIL;
	}

	free(buff);
	buff = NULL;
	return CODEC_ERR_OK;
}

int ISIL_VI_StopAlarmType(unsigned int u32ChipID, unsigned int u32Chan, unsigned int alarmType)
{
	unsigned char *buff = NULL;
	int fd = 0;
	int offset = 0;
	isil_transparent_msg_header_t msgheader;
	isil_video_lost_detection_msg_t vlmsg;
	isil_video_night_detection_msg_t vnmsg;
	isil_video_blind_detection_msg_t vbmsg;
	isil_video_standard_change_detection_msg_t stdmsg;

	if(u32ChipID >= VI_CHIP_MAX){
		#ifdef VI_DEBUG
		CODEC_DEBUG(" vi chipID[%d] >= Max[%d],invalid", u32ChipID, VI_CHIP_MAX);
		#endif
		return CODEC_ERR_INVAL;
	}

	if(u32Chan >= VI_CHIP_CH_MAX){
		#ifdef VI_DEBUG
		CODEC_DEBUG(" vi u32Chan[%d] >= Max[%d],invalid", u32Chan, VI_CHIP_CH_MAX);
		#endif
		return CODEC_ERR_INVAL;
	}

	//malloc buff
	buff = (unsigned char *)malloc(sizeof(msgheader) + sizeof(vlmsg) + sizeof(vnmsg) + sizeof(vbmsg) + sizeof(stdmsg));

	memset(&msgheader, 0x00, sizeof(msgheader));
	msgheader.isil_transparent_msg_total_len = sizeof(msgheader);

	offset += sizeof(msgheader);

	if((alarmType&VI_ALARM_VIDEO_LOST) || (alarmType&VI_ALARM_VIDEO_CONNECT)){
		msgheader.isil_transparent_msg_number++;
		msgheader.isil_transparent_msg_total_len += sizeof(vlmsg);

		vlmsg.msg_type = ISIL_VIDEO_LOST_DETECTION_MSG;
		vlmsg.msg_len = sizeof(vlmsg);
		vlmsg.chan_id = u32Chan;

		if(alarmType&VI_ALARM_VIDEO_LOST){
			vlmsg.video_lost_valid = 0;
		}

		if(alarmType&VI_ALARM_VIDEO_LOST){
			vlmsg.video_contect_valid = 0;
		}

		memcpy(buff+offset, &vlmsg, sizeof(vlmsg));
		offset += sizeof(vlmsg);
	}

	if((alarmType&VI_ALARM_VIDEO_NIGHT_DAYTONIGHT) || (alarmType&VI_ALARM_VIDEO_NIGHT_NIGHTTODAY)){
		msgheader.isil_transparent_msg_number++;
		msgheader.isil_transparent_msg_total_len += sizeof(vnmsg);

		vnmsg.msg_type = ISIL_VIDEO_NIGHT_DETECTION_MSG;
		vnmsg.msg_len = sizeof(vnmsg);
		vnmsg.chan_id = u32Chan;

		if(alarmType&VI_ALARM_VIDEO_NIGHT_DAYTONIGHT){
			vnmsg.video_night_valid_from_day_to_night = 0;
		}

		if(alarmType&VI_ALARM_VIDEO_NIGHT_NIGHTTODAY){
			vnmsg.video_night_valid_from_night_to_day = 0;
		}

		memcpy(buff+offset, &vnmsg, sizeof(vnmsg));
		offset += sizeof(vnmsg);
	}

	if((alarmType&VI_ALARM_VIDEO_BLIND_ADD) || (alarmType&VI_ALARM_VIDEO_BLIND_REMOVE)){
		msgheader.isil_transparent_msg_number++;
		msgheader.isil_transparent_msg_total_len += sizeof(vbmsg);

		vbmsg.msg_type = ISIL_VIDEO_BLIND_DETECTION_MSG;
		vbmsg.msg_len = sizeof(vbmsg);
		vbmsg.chan_id = u32Chan;

		if(alarmType&VI_ALARM_VIDEO_BLIND_ADD){
			vbmsg.video_blind_add_valid = 0;
		}

		if(alarmType&VI_ALARM_VIDEO_BLIND_REMOVE){
			vbmsg.video_blind_remove_valid = 0;
		}

		memcpy(buff+offset, &vbmsg, sizeof(vbmsg));
		offset += sizeof(vbmsg);
	}

	if(alarmType&VI_ALARM_VIDEO_STD_CHANGE){
		msgheader.isil_transparent_msg_number++;
		msgheader.isil_transparent_msg_total_len += sizeof(stdmsg);

		stdmsg.msg_type = ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG;
		stdmsg.msg_len = sizeof(stdmsg);
		stdmsg.chan_id = u32Chan;
		stdmsg.latest_video_standard = 0;
		stdmsg.curr_video_standard = 0;

		memcpy(buff+offset, &stdmsg, sizeof(stdmsg));
		offset += sizeof(stdmsg);
	}

	memcpy(buff, &msgheader, sizeof(msgheader));

	//how to do, ioc
	fd = VI_CHIP_TBL[u32ChipID].s32ChipFd;
	if(ioctl(fd, ISIL_VI_ALARM_CMD, buff) < 0)
	{
		#ifdef VI_DEBUG
		CODEC_DEBUG(" ioctl fail ");
		perror("why: ");
		#endif
		return CODEC_ERR_FAIL;
	}

	free(buff);
	buff = NULL;
	return CODEC_ERR_OK;
}

int ISIL_VI_SoftReset(unsigned int u32ChipID)
{
	int fd = 0;
	fd = VI_CHIP_TBL[u32ChipID].s32ChipFd;
	if(ioctl(fd, ISIL_CHIP_SOFT_RESET,u32ChipID)<0)
	{
        #ifdef VI_DEBUG
        CODEC_DEBUG(" ioctl fail ");
        #endif
        return CODEC_ERR_FAIL;
    }
	 return CODEC_ERR_OK;
}

