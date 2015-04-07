/******************** (C) COPYRIGHT 2010 Techwell Inc. ********************
* File Name     : isil_ai.h
* Author        : dengjng
* Versio        : V0.1
* Date          : 2010-05-12
* Description   :
* all audio input data struct
* all audio input operate API:chip and channel
********************************************************************************
* THE PRESENT SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL PROVIDE CUSTOMERS
* WITH CODING INFORMATION TO SPEED UP THEIR PRODUCT DEVELOPMENT, BUT WITHOUT
* ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
* FOR A PARTICULAR PURPOSE. AS A RESULT, TECHWELL SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef _ISIL_AI_H_
#define _ISIL_AI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_codec_common.h"
//#include "config.h"

#ifndef ISIL_DRIVER_IOC_MAGIC
#define ISIL_DRIVER_IOC_MAGIC      'H'  /*ai,ao,vi,vo,chip control use it*/
#endif

#ifndef ISIL_AI_SET_AUDIO_SYS_PARM
#define ISIL_AI_SET_AUDIO_SYS_PARM	        _IOW(ISIL_DRIVER_IOC_MAGIC,  49, int)
#endif

#ifndef ISIL_AI_GET_AUDIO_SYS_PARM
#define ISIL_AI_GET_AUDIO_SYS_PARM	        _IOR(ISIL_DRIVER_IOC_MAGIC,  48, int)
#endif

//volume setting cmd
#ifndef ISIL_AI_SET_AUDIO_GAIN
#define ISIL_AI_SET_AUDIO_GAIN			_IOW(ISIL_DRIVER_IOC_MAGIC,  02, int)
#endif

//auduo input control cmd
#ifndef ISIL_AI_CTL_OP
#define ISIL_AI_CTL_OP			_IOW(ISIL_DRIVER_IOC_MAGIC,  00, int)
#endif


//system config
#define AI_CHIP_MAX TOTAL_CHIPS_NUM //maxium AI chip
#define AI_CHIP_CH_MAX  PER_CHIP_MAX_CHAN_NUM //the maxium channel of all chips

#define AI_HANDLE int //audio input channel fd

//volume adjust of audio input channel
typedef struct
{
    unsigned int u32ChipID;//chip id
    unsigned int u32Channel;//channel id
    unsigned int u32Volume;//volume value
} AI_CH_VOLUME;

//audio input channel cfg
typedef struct
{
	unsigned int 	u32ChipID;//chip id
    unsigned int    u32Channel;//physical channel ID
    unsigned int    u32Mask;
    unsigned int    u32BitWidth;//bit width
    unsigned int    u32BitRate;//bit rate
    unsigned int    u32SampleRate;//sample rate
}AI_CH_CFG;


//audio chip system config
enum AUDIO_SYNC_MODE{
    AUDIO_SYNC_MODE_I2S = 0,
    AUDIO_SYNC_MODE_DSP,
};

enum AUDIO_CLOCK_MODE{
	AUDIO_CLOCK_SLAVE = 0,
	AUDIO_CLOCK_MASTER,/*default*/
};


#define ISIL_CHIP_AI_CHANGEMASK_CLOCK_MODE    (0x0001)
#define ISIL_CHIP_AI_CHANGEMASK_SYNC_MODE     (0x0002)
#define ISIL_CHIP_AI_CHANGEMASK_SAMPLE_RATE   (0x0004)
#define ISIL_CHIP_AI_CHANGEMASK_BITS          (0x0008)
#define ISIL_CHIP_AI_CHANGEMASK_MASKALL       (0x000f)

typedef struct ISIL_chip_ai_sys{
    unsigned int changemask;
    enum AUDIO_CLOCK_MODE clock_mode;
    enum AUDIO_SYNC_MODE  sync_mode;
    AUDIO_SAMPLE_RATE 	  sample_rate;
    AUDIO_BIT_WIDTH       bits;
}AI_CHIP_SYS;


//chip control cmd and data struct,control ai chip all other function ,not list in api
typedef struct
{
    unsigned int u32ChipID;//chip id
    unsigned int u32Cmd;//control cmd
    unsigned int u32Blk;//block or nonblock operator
    unsigned int u32MsgLen;//msg length
    char         pMsg[0];//msg content
}AI_CHIP_MSG;


/*Audio input API for channel, all channel operation api can be call,
only after chip been open */


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

extern int ISIL_AI_Adjust_Volume(unsigned int u32ChipID, unsigned int u32Ch, AI_CH_VOLUME* pAiVol);


/***********************************************************
 function:
     ISIL_AI_SetCfg, now not use
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

extern int ISIL_AI_SetCfg(unsigned int u32ChipID, unsigned int u32Ch, AI_CH_CFG* pAiCfg);


/***********************************************************
 function:
     ISIL_AI_GetCfg, now not use
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

extern int ISIL_AI_GetCfg(unsigned int u32ChipID, unsigned int u32Ch, AI_CH_CFG* pAiCfg);



/*Audio input chip control*/

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

extern int ISIL_AI_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID);

/***********************************************************
function  :
    ISIL_AI_CTL_CloseChip
discription :
    close chip control device
input:
    unsigned int u32ChipID: chip id
output:
    none
return:
    CODEC_ERR_OK: success
    CODEC_ERR_FAIL:fail
    CODEC_ERR_NOT_EXIT: chip not open
    CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

 extern int ISIL_AI_CTL_CloseChip(unsigned int u32ChipID);


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
 extern int ISIL_AI_CTL_SendMsg(unsigned int u32ChipID, AI_CHIP_MSG *pMsg);//finish all operator of chip,eg:bind VO, mux ,demux etc


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
    CODEC_ERR_NOT_EXIT: chip not open
    CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

extern int ISIL_AI_CTL_GetChipHandle(unsigned int u32ChipID, AI_HANDLE* pChipHandle);


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

extern int ISIL_AI_CHIP_SetSystem(unsigned int u32ChipID, AI_CHIP_SYS* pAiSys);




#ifdef __cplusplus
}
#endif
#endif



