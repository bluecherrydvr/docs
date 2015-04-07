
/********************************************************
 file name : isil_vo.c
 discription: vodeo output related interface
 author    : dengjing
 create date: 2010-3-31
 history   : none
 others    : none
 ********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>

#include "isil_vo.h"
#include "isil_codec_debug.h"
#include "isil_codec_errcode.h"


#define VO_DEBUG

/*save vo chip info*/
typedef struct
{
    char         u8ChipOpen;//chip had been open
    char         u8Reserved[3];
    int          s32ChipFd;//chip fd
}VO_CHIP_INFO;

VO_CHIP_INFO VO_CHIP_TBL[VO_CHIP_MAX];

/***********************************************************
 function:
     ISIL_VO_SetCropping
 discription:
     set vodeo output channel croppong
 paramnet:
     output:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VO_CH_RECT* pstRect: point to cropping data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_SetCropping(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_CH_RECT* pstRect)
{
    if(eChType != VO_CH_TYPE_CVBS)
    {
        return CODEC_ERR_FAIL;
    }

    if(u32Ch >= VO_CHIP_CVBS_MAXCH)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo channel ID[%d] > Max[%d],invalid", u32Ch, VO_CHIP_CVBS_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pstRect == NULL)
    {
       #ifdef VO_DEBUG
        CODEC_DEBUG(" pointor is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VO_SetVodeoType
 discription:
     set vodeo output channel vodeo type,eg PAL or NTSC etc
 paramnet:
     output:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         unsigned int u32VodeoType: tv foramt
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_SetVodeoType(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, unsigned int u32VodeoType)
{
    if(eChType != VO_CH_TYPE_CVBS)
    {
        return CODEC_ERR_FAIL;
    }

    if(u32Ch >= VO_CHIP_CVBS_MAXCH)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo channel ID[%d] > Max[%d],invalid", u32Ch, VO_CHIP_CVBS_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    u32VodeoType = u32VodeoType;

    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VO_SetScaler
 discription:
     set channel scaler
 paramnet:
     output:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VO_SCALER_FACTOR pstFac: point scaler factor
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_SetCvbsScaler(unsigned int u32ChipID, VO_CVBS_SCALER* pstFac)
{
    if(pstFac == NULL)
    {
       #ifdef VO_DEBUG
        CODEC_DEBUG(" pointor is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    if(ioctl(VO_CHIP_TBL[u32ChipID].s32ChipFd,
             ISIL_VO_SET_ANALOG_OUTPUT_SCALER, pstFac) < 0)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" ioctl fail ");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VO_SetCfg
 discription:
     set vodeo output channel config
 paramnet:
     output:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VO_CH_CFG* pVoCfg: point to config data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_SetCfg(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_CH_CFG* pVoCfg)
{
    if(u32Ch >= VO_CHIP_CVBS_MAXCH)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo channel ID[%d] > Max[%d],invalid", u32Ch, VO_CHIP_CVBS_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pVoCfg == NULL)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    eChType = eChType;

    return CODEC_ERR_OK;
}
/***********************************************************
 function:
     ISIL_VO_GetCfg
 discription:
     get vodeo output config
 paramnet:
     output:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         VO_CH_CFG* pVoCfg: point to config data
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_GetCfg(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_CH_CFG* pVoCfg)
{
    if(u32Ch >= VO_CHIP_CVBS_MAXCH)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo channel ID[%d] > Max[%d],invalid", u32Ch, VO_CHIP_CVBS_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pVoCfg == NULL)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" chip not open ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    /*how todo, ioctl driver,set cropping*/
    eChType= eChType;

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VO_SendData, now not use
 discription:
     send data to vo , for testing
 paramnet:
     output:
         none
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_SendData(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, char* pBuf, unsigned int u32Len)//now not use
{
    u32ChipID = u32ChipID;
    u32Ch = u32Ch;
    eChType = eChType;
    pBuf = pBuf;
    u32Len = u32Len;
    return 0;
}


/***********************************************************
 function:
     ISIL_VO_GetData, now not use
 discription:
     get vo raw data , for testing
 paramnet:
     output:
         none
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_GetData(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, char* pBuf, unsigned int u32Len)//now not use
{
    u32ChipID = u32ChipID;
    u32Ch = u32Ch;
    eChType = eChType;
    pBuf = pBuf;
    u32Len = u32Len;
    return 0;
}


/***********************************************************
 function:
     ISIL_VO_GetChHandle
 discription:
     get vo channel handle
 paramnet:
     output:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
     output:
         VO_HANDLE* pChHandle: point to vo channel handle
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_GetChHandle(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_HANDLE* pChHandle)
{
    if(eChType == VO_CH_TYPE_CVBS){
        if(u32Ch >= VO_CHIP_CVBS_MAXCH)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" vo channel ID[%d] > Max[%d],invalid", u32Ch, VO_CHIP_CVBS_MAXCH);
            #endif
            return CODEC_ERR_INVAL;
        }

        if(pChHandle == NULL)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" point is null");
            #endif
            return CODEC_ERR_INVAL;
        }

        if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" chip not open ");
            #endif
            return CODEC_ERR_NOT_EXIST;
        }

        /*how todo, ioctl driver,set cropping*/
        *pChHandle = VO_CHIP_TBL[u32ChipID].s32ChipFd;

        return CODEC_ERR_OK;
    }
    else if(eChType == VO_CH_TYPE_VGA){
        if(u32Ch >= VO_CHIP_VGA_MAXCH)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" vo channel ID[%d] > Max[%d],invalid", u32Ch, VO_CHIP_VGA_MAXCH);
            #endif
            return CODEC_ERR_INVAL;
        }

        if(pChHandle == NULL)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" point is null");
            #endif
            return CODEC_ERR_INVAL;
        }

        if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" chip not open ");
            #endif
            return CODEC_ERR_NOT_EXIST;
        }

        /*how todo, ioctl driver,set cropping*/
        *pChHandle = VO_CHIP_TBL[u32ChipID].s32ChipFd;

        return CODEC_ERR_OK;
    }
    else{
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo ch type is error ");
        #endif
        return CODEC_ERR_INVAL;
    }
}


/***********************************************************
 function:
     ISIL_VO_Bind2Wnd
 discription:
     bind a input channel to any display window in any position
 input:
     unsigned int u32ChipID: chip id
     unsigned int u32Ch: channel id
     VO_CH_TYPE eChType: ouput channel type
     VO_WND_BIND* pVoWnd: vodeo output display window position
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_Bind2Wnd(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_WND_BIND* pVoWnd)
{
    if(eChType == VO_CH_TYPE_CVBS){
        if(u32Ch >= VO_CHIP_CVBS_MAXCH)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" vo channel ID[%d] > Max[%d],invalid", u32Ch, VO_CHIP_CVBS_MAXCH);
            #endif
            return CODEC_ERR_INVAL;
        }

        if(pVoWnd == NULL)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" point is null");
            #endif
            return CODEC_ERR_INVAL;
        }

        if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" chip not open ");
            #endif
            return CODEC_ERR_NOT_EXIST;
        }

        /*how todo*/
        if(ioctl(VO_CHIP_TBL[u32ChipID].s32ChipFd,
                 ISIL_VO_SET_DISPLAY_PLAYBACK_CHANNEL, pVoWnd) < 0){
            #ifdef VO_DEBUG
            CODEC_DEBUG(" ioctl fail");
            #endif
            return CODEC_ERR_FAIL;
        }
        return CODEC_ERR_OK;
    }
    else if(eChType == VO_CH_TYPE_VGA){
        if(u32Ch >= VO_CHIP_VGA_MAXCH)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" vo channel ID[%d] > Max[%d],invalid", u32Ch, VO_CHIP_VGA_MAXCH);
            #endif
            return CODEC_ERR_INVAL;
        }

        if(pVoWnd == NULL)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" point is null");
            #endif
            return CODEC_ERR_INVAL;
        }

        if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
        {
            #ifdef VO_DEBUG
            CODEC_DEBUG(" chip not open ");
            #endif
            return CODEC_ERR_NOT_EXIST;
        }

        /*how todo*/
        if(ioctl(VO_CHIP_TBL[u32ChipID].s32ChipFd,
                 ISIL_VO_SET_DISPLAY_PLAYBACK_CHANNEL, pVoWnd) < 0){
            #ifdef VO_DEBUG
            CODEC_DEBUG(" ioctl fail");
            #endif
            return CODEC_ERR_FAIL;
        }
        return CODEC_ERR_OK;
    }
    else{
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo ch type is error ");
        #endif
        return CODEC_ERR_INVAL;
    }
}

int ISIL_VO_SetMuxPath(unsigned int u32ChipID, VO_MUX_PATH *pVoPath)
{
    if(u32ChipID >= VO_CHIP_MAX)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo chip ID[%d] > Max[%d],invalid", u32ChipID, VO_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pVoPath == NULL)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" chip had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    if(ioctl(VO_CHIP_TBL[u32ChipID].s32ChipFd,
    		ISIL_VO_SET_VIDEO_ENCODER_SOURCE,
       pVoPath) < 0){
        #ifdef VO_DEBUG
        CODEC_DEBUG(" ---ioctl fail---\n ");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VO_CTL_OpenChip
 discription:
     open chip control devoce and get fd
 output:
         char *pChipNode: chip devoce node path
         unsigned int u32ChipID: chip id
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_EXIT: chip had been open
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID)
{
    int fd = 0;

    if(u32ChipID >= VO_CHIP_MAX)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo chip ID[%d] > Max[%d],invalid", u32ChipID, VO_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" had open ");
        #endif
        return CODEC_ERR_EXIST;
    }

    fd = open(pChipNode, O_RDWR);
    if(fd < 0)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" open vo chip: %s fail", pChipNode);
        #endif
        return CODEC_ERR_FAIL;
    }

    VO_CHIP_TBL[u32ChipID].s32ChipFd = fd;
    VO_CHIP_TBL[u32ChipID].u8ChipOpen = 1;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VO_CTL_CloseChip
 discription:
     close chip control devoce
 paramnet:
     output:
         unsigned int u32ChipID: chip id
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip had been closed
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_CTL_CloseChip(unsigned int u32ChipID)
{
    int iRet = 0;

    if(u32ChipID >= VO_CHIP_MAX)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo chip ID[%d] > Max[%d],invalid", u32ChipID, VO_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    iRet = close(VO_CHIP_TBL[u32ChipID].s32ChipFd);
    if(iRet < 0)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" close vo chip: %d fail", u32ChipID);
        #endif
        return CODEC_ERR_FAIL;
    }

    VO_CHIP_TBL[u32ChipID].u8ChipOpen = 0;
    VO_CHIP_TBL[u32ChipID].s32ChipFd = 0;
    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VO_CTL_GetChipHandle
 discription:
     get chip fd
 paramnet:
     output:
         unsigned int u32ChipID: chip id
     output:
         VO_HANDLE* pChipHandle: chip fd
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_NOT_EXIT: chip had been close
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_CTL_GetChipHandle(unsigned int u32ChipID, VO_HANDLE* pChipHandle)
{
    int iRet = 0;

    if(u32ChipID >= VO_CHIP_MAX)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" vo chip ID[%d] > Max[%d],invalid", u32ChipID, VO_CHIP_MAX);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VO_CHIP_TBL[u32ChipID].u8ChipOpen)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" chip had close ");
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    *pChipHandle = VO_CHIP_TBL[u32ChipID].s32ChipFd;
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_VO_CTL_SendMsg
 discription:
     all chip control operate by msg
 paramnet:
     output:
         VO_CTL_MSG *pMsg: control data struct
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/
int ISIL_VO_CTL_SendMsg(unsigned int u32ChipID, VO_CTL_MSG *pMsg)//finish all operator of chip,eg:bind VO, mux ,demux etc
{
    int fd = 0;

    if(pMsg == NULL)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    /*how to do*/
    /*ioctl driver*/
    fd = VO_CHIP_TBL[u32ChipID].s32ChipFd;

    if(ioctl(fd, ISIL_VO_CTL_OP, pMsg) < 0)
    {
        #ifdef VO_DEBUG
        CODEC_DEBUG(" ioctl fail ");

        #endif
        perror("ioctrl");
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


/***********************************************************
 function:
     ISIL_VO_GetAlphaBlending
 discription:
    2010-11-03 ligc++ for Alphablending configure of TW2851;
 paramnet:
     intput:
         osg_alpha_config_t * pVoAlphaCfg
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/

int ISIL_VO_GetAlphaBlending(unsigned int u32ChipID, osg_alpha_config_t* pVoAlphaCfg)
{
	if(u32ChipID >= VO_CHIP_MAX)
	{
		#ifdef VO_DEBUG
		CODEC_DEBUG(" vo chip ID[%d] > Max[%d],invalid", u32ChipID, VO_CHIP_MAX);
		#endif
		return CODEC_ERR_INVAL;
	}

    if(ioctl(VO_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VO_GET_ALPHA_VALUE, pVoAlphaCfg) < 0)
    {
    #ifdef VO_DEBUG
        CODEC_DEBUG(" ioctl fail ");
     #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}

/***********************************************************
 function:
     ISIL_TW_SetAlphaBlending
 discription:
    2010-11-03 ligc++; for Alphablending configure of TW2851;
 paramnet:
     intput:
         none
     output:
        osg_alpha_config_t * pVoAlphaCfg
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_INVALID: output parament is invalid
 **********************************************************/

int ISIL_VO_SetAlphaBlending(unsigned int u32ChipID, osg_alpha_config_t* pVoAlphaCfg)
{
	if(u32ChipID >= VO_CHIP_MAX)
	{
		#ifdef VO_DEBUG
		CODEC_DEBUG(" vo chip ID[%d] > Max[%d],invalid", u32ChipID, VO_CHIP_MAX);
		#endif
		return CODEC_ERR_INVAL;
	}

    if(ioctl(VO_CHIP_TBL[u32ChipID].s32ChipFd, ISIL_VO_SET_ALPHA_VALUE, pVoAlphaCfg) < 0)
    {
		#ifdef VO_DEBUG
		CODEC_DEBUG(" ioctl fail ");
		#endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}




