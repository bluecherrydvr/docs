/******************** (C) COPYRIGHT 2010 Techwell Inc. ********************
* File Name     : isil_vi.h
* Author        : dengjng
* Versio        : V0.1
* Date          : 2010-05-12
* Description   :
* all video input data struct
* all video input operate API:chip and channel
********************************************************************************
* THE PRESENT SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL PROVIDE CUSTOMERS
* WITH CODING INFORMATION TO SPEED UP THEIR PRODUCT DEVELOPMENT, BUT WITHOUT
* ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
* FOR A PARTICULAR PURPOSE. AS A RESULT, TECHWELL SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef _ISIL_VI_H_
#define _ISIL_VI_H_


#ifdef __cpluscplus
extern "C" {
#endif

#include "isil_codec_common.h"


/******************************************************************************
     vi io ctl cmds
******************************************************************************/
#ifndef ISIL_DRIVER_IOC_MAGIC
#define ISIL_DRIVER_IOC_MAGIC  'H'
#endif

#ifndef ISIL_VI_CTL_OP
#define ISIL_VI_CTL_OP									_IOW(ISIL_DRIVER_IOC_MAGIC,  50, int)
#endif

#ifndef ISIL_VI_SET_VIDEO_AD_PARM
#define ISIL_VI_SET_VIDEO_AD_PARM							_IOW(ISIL_DRIVER_IOC_MAGIC,  51, int)
#endif

#ifndef ISIL_VI_GET_VIDEO_AD_PARM
#define ISIL_VI_GET_VIDEO_AD_PARM							_IOR(ISIL_DRIVER_IOC_MAGIC,  52, int)
#endif

#ifndef ISIL_VI_SET_ANALOG_INPUT_SCALER
#define ISIL_VI_SET_ANALOG_INPUT_SCALER					_IOW(ISIL_DRIVER_IOC_MAGIC,  53, int)
#endif

#ifndef ISIL_VI_SET_CHANNEL_PORT
#define ISIL_VI_SET_CHANNEL_PORT  _IOW(ISIL_DRIVER_IOC_MAGIC,  54, int)
#endif

#ifndef ISIL_VI_SET_VIDEO_TYPE
#define ISIL_VI_SET_VIDEO_TYPE    _IOW(ISIL_DRIVER_IOC_MAGIC,  55, int)
#endif

#ifndef ISIL_VI_SET_NOISE_REDUCTION
#define ISIL_VI_SET_NOISE_REDUCTION   _IOW(ISIL_DRIVER_IOC_MAGIC,  56, int)
#endif

#ifndef ISIL_VI_SET_ANALOG_INPUT_CROPPER
#define ISIL_VI_SET_ANALOG_INPUT_CROPPER  _IOW(ISIL_DRIVER_IOC_MAGIC,  57, int)
#endif

#ifndef ISIL_VI_SET_FEATURE
#define ISIL_VI_SET_FEATURE           _IOW(ISIL_DRIVER_IOC_MAGIC,  146, int)
#endif

#ifndef ISIL_VI_ALARM_CMD
#define ISIL_VI_ALARM_CMD        _IOW(ISIL_DRIVER_IOC_MAGIC, 148, int)
#endif

#ifndef ISIL_CHIP_SOFT_RESET
#define ISIL_CHIP_SOFT_RESET 		_IOW(ISIL_DRIVER_IOC_MAGIC,  203, int)
#endif

//system config
#define VI_CHIP_MAX TOTAL_CHIPS_NUM //maxium vi chip number
#define VI_CHIP_CH_MAX PER_CHIP_MAX_CHAN_NUM //maxium supported channel in a chip

#define VI_HANDLE int // video input channel fd

//define rectangle area for input channel
typedef struct
{
	/* for display / record path */
	unsigned int u32ChipID;//chip id
	unsigned int u32Channel;	//channel id/* port num 0,1,2,3 */
	unsigned int u32HStart;		/*starting location of horizontal active pixel,A unit is 1 pixel.*/
	unsigned int u32HPixelN;	/*number of horizontal active pixel,A unit is 1 pixel.*/
	unsigned int u32VStart;		/*starting location of vertical active line,A unit is 1 line.*/
	unsigned int u32VPixelN;	/*vertical active lines,A unit is 1 line.*/
}VI_CH_CROPPER;


//scaler factor
typedef struct
{
	unsigned int u32ChipID;//chip id
	unsigned int u32Path;//which path
	unsigned int u32Channel;//channel id /* port num 1,2,3,4 */
	unsigned int u32SrcHSize;//source_horizontal_size;	/*unit is in multiple of 16 pixels*/
	unsigned int u32SrcVSize;//source_vertical_size;		/*unit is in multiple of 8 lines*/
	unsigned int u32TgtHSize;//target_horizontal_size;	/*unit is in multiple of 16 pixels*/
	unsigned int u32TgtVSize;//target_vertical_size;		/*unit is in multiple of 8 lines*/
}VI_CH_SCALER;

//each vi channel config parament
#define MASK_VI_BRIGHTNESS (0x01)
#define MASK_VI_CONTRAST   (0x02)
#define MASK_VI_SATURATION (0x04)
#define MASK_VI_HUE	       (0x08)
#define MASK_VI_SHARPNESS  (0x10)
#define MASK_VI_ALL        (0x1f)

typedef struct
{
    unsigned int    u32ChipID;//chip id
    unsigned int    u32Channel;//physical channel ID
    unsigned int    u32Mask;//mask bit
	unsigned int    u32Brightness;//brigghtness
	unsigned int    u32Contrast;//contrast
	unsigned int    u32Saturation;//saturation
	unsigned int    u32Hue;//hue
	unsigned int    u32Sharpness;//sharpness
}VI_CH_CFG;


//video standerd
enum CHIP_VI_STANDARD {
    CHIP_VI_STANDARD_NTSC_M,    /*NTSC(M)*/
    CHIP_VI_STANDARD_PAL_BDGHI, /*PAL (B,D,G,H,I)*/
    CHIP_VI_STANDARD_SECAM,     /*SECAM*/
    CHIP_VI_STANDARD_NTSC443,   /*NTSC 4.43*/
    CHIP_VI_STANDARD_PAL_M,     /*PAL(M)*/
    CHIP_VI_STANDARD_PAL_CN,    /*PAL(CN)*/
    CHIP_VI_STANDARD_PAL_60,    /*PAL 60*/
    CHIP_VI_STANDARD_INVALID,   /*invalid input*/
    CHIP_VI_STANDARD_AUTO,      /*auto detection*/
};

typedef struct
{
    unsigned int u32ChipID;//chip id
    unsigned int u32Channel;//channel id
    enum CHIP_VI_STANDARD eVideoType;//video type
}VI_CH_VIDEOTYPE;

typedef struct
{
    unsigned int u32ChipID;//chip id
    unsigned int u32Channel;//channel id
    unsigned int u32Enable;//enable
}VI_CH_NR;


//which port selected
typedef struct
{
    unsigned int u32ChipID;//chip id
    unsigned int u32Channel;//channel id
    unsigned int u32WhichPort;//enable
}VI_CH_PORT;


//chip control cmd and data struct
typedef struct
{
    unsigned int u32ChipID;//chip ID
    unsigned int u32Cmd;//control cmd
    unsigned int u32Blk;//block or nonblock operator
    unsigned int u32MsgLen;//msg length
	char         pMsg[0];
}VI_CHIP_MSG;

/*video signal alarm type*/
typedef enum{
	VI_ALARM_VIDEO_LOST = 0x01,//video lost
	VI_ALARM_VIDEO_CONNECT= 0x02,//video connect
	VI_ALARM_VIDEO_NIGHT_DAYTONIGHT= 0x04,//video night day to night
	VI_ALARM_VIDEO_NIGHT_NIGHTTODAY= 0x08,//video night night to day
	VI_ALARM_VIDEO_BLIND_ADD= 0x10,//video blind add
	VI_ALARM_VIDEO_BLIND_REMOVE= 0x20,//video blind remove
	VI_ALARM_VIDEO_STD_CHANGE = 0x40,//video standard change
}ISIL_VI_ALARM_TYPE;

/*video input API for channel*/


/***********************************************************
 function:
     ISIL_VI_SetCropping
 discription:
     set video input channel croppong
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

extern int ISIL_VI_SetCropping(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_CROPPER* pCropper);



/***********************************************************
 function:
     ISIL_VI_SetVideoType
 discription:
     set video input channel video type,eg PAL or NTSC etc
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

extern int ISIL_VI_SetVideoType(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_VIDEOTYPE* pVideoType);


/***********************************************************
 function:
     ISIL_VI_SetScaler
 discription:
     set channel scaler
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

extern int ISIL_VI_SetScaler(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_SCALER* pScaler);



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

extern int ISIL_VI_SetNR(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_NR* pNR);


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

extern int ISIL_VI_SelectChPort(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_PORT* pChPort);


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

extern int ISIL_VI_SetCfg(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_CFG* pViCfg);


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

extern int ISIL_VI_GetCfg(unsigned int u32ChipID, unsigned int u32Ch, VI_CH_CFG* pViCfg);


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

extern int ISIL_VI_SendData(unsigned int u32ChipID, unsigned int u8Ch, char* pBuf, unsigned int u32Len);//now not use


/***********************************************************
 function:
     ISIL_VI_GetData, use for alarm
 discription:
     get vi data , for alarm
 paramnet:
     input:
         unsigned int u32Ch: channel id
         unsigned int u32ChipID : chip id
         CODEC_STREAM_DATA* pdata: buff
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIT: chip not open or channel not create or channel not enable
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

extern int ISIL_VI_GetData(unsigned int u32ChipID, unsigned int u8Ch, CODEC_STREAM_DATA* pdata);//now not use


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

extern int ISIL_VI_GetChHandle(unsigned int u32ChipID, unsigned int u32Ch, VI_HANDLE* pChHandle);



/*vi chip api for chip*/


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

extern int ISIL_VI_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID);


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
     CODEC_ERR_NOT_EXIT: chip not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
extern int ISIL_VI_CTL_CloseChip(unsigned int u32ChipID);


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

extern int ISIL_VI_CTL_GetChipHandle(unsigned int u32ChipID, VI_HANDLE* pChipHandle);



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

extern int ISIL_VI_CTL_SendMsg(unsigned int u32ChipID, VI_CHIP_MSG *pMsg);//finish all operator of chip,eg:bind VO, mux ,demux etc


/***********************************************************
 function:
     ISIL_VI_StartAlarmType
 discription:
     start alarm type
 paramnet:
 	 input:
         unsigned int u32ChipID: chip id
         unsigned int u32Chan  : chan id
         unsigned int alarmType : alarmtype, see ISIL_VI_ALARM_TYPE
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

extern int ISIL_VI_StartAlarmType(unsigned int u32ChipID, unsigned int u32Chan, unsigned int alarmType);

/***********************************************************
 function:
     ISIL_VI_StopAlarmType
 discription:
     stop alarm type
 paramnet:
 	 input:
         unsigned int u32ChipID: chip id
         unsigned int u32Chan  : chan id
         unsigned int alarmType : alarmtype, see ISIL_VI_ALARM_TYPE
     output:
         none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

extern int ISIL_VI_StopAlarmType(unsigned int u32ChipID, unsigned int u32Chan, unsigned int alarmType);


extern int ISIL_VI_SoftReset(unsigned int u32ChipID);

#ifdef __cpluscplus
}
#endif 

#endif

