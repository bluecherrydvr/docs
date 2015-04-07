/******************** (C) COPYRIGHT 2010 Techwell Inc. ********************
* File Name     : isil_encoder.h
* Author        : dengjng
* Versio        : V0.1
* Date          : 2010-05-12
* Description   :
* all encoder API and data struct, include h264,mjpg,audio
********************************************************************************
* THE PRESENT SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL PROVIDE CUSTOMERS
* WITH CODING INFORMATION TO SPEED UP THEIR PRODUCT DEVELOPMENT, BUT WITHOUT
* ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
* FOR A PARTICULAR PURPOSE. AS A RESULT, TECHWELL SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


#ifndef _ISIL_ENCODER_H_
#define _ISIL_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "isil_codec_common.h"
#include "isil_codec_errcode.h"
#include "isil_interface.h"
#include "config.h"

#define ENC_MAXCH   PER_CHIP_MAX_CHAN_NUM //max channel of chips

/********************************data struct************************/
/*************Video h264 encoding ***************/

//h264 encoding config paraments
#define ISIL_h264_ENCODER_PARAM_BPS_MASK          0x0001
#define ISIL_h264_ENCODER_PARAM_FPS_MASK          0x0002
#define ISIL_h264_ENCODER_PARAM_IP_STRIDE_MASK    0x0004
#define ISIL_h264_ENCODER_PARAM_PB_STRIDE_MASK    0x0008
#define ISIL_h264_ENCODER_PARAM_GOP_MASK          0x0010
#define ISIL_h264_ENCODER_PARAM_FORCE_I_MASK      0x0020
#define ISIL_h264_ENCODER_PARAM_WIDTH_MASK        0x0040
#define ISIL_h264_ENCODER_PARAM_HEIGHT_MASK       0x0080

#define ISIL_BOOLEN_TURE 1
#define ISIL_BOOLEN_FALSE 0

typedef struct
{
    unsigned int change_mask_flag;
    unsigned int i_bps;
    unsigned int i_fps;
    unsigned int i_I_P_stride;
    unsigned int i_P_B_stride;
    unsigned int i_gop_value;
    unsigned int b_force_I_flag;
    unsigned short i_logic_video_width_mb_size;
    unsigned short i_logic_video_height_mb_size;
}VENC_H264_CFG;

//h264 private feature parament
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK      0x00000001
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_SKIP_MASK             0x00000002
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_I_4X4_MASK            0x00000004
#define ISIL_H264_ENABLE_CHANGE_HALF_PIXEL_MASK                      0x00000008
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK    0x00000010
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK         0x00000020

#define ISIL_H264_FEATURE_ON      1
#define ISIL_H264_FEATURE_OFF     0

typedef struct{
    unsigned int   change_mask_flag;
    unsigned char  b_enable_deinterlace;
    unsigned char  b_enable_skip;
    unsigned char  b_enable_I_4X4;
    unsigned char  b_enable_half_pixel;
    unsigned char  b_enable_quarter_pixel;
	unsigned char  reserved[3];
    unsigned int   i_mb_delay_value;
}VENC_H264_FEATURE;



//h264 rc

enum ISIL_H264_RC_TYPE{
	ISIL_H264_NO_RC,
	ISIL_H264_CBR,
	ISIL_H264_VBR,
};

enum ISIL_RC_IMAGE_PRIORITY{
    ISIL_RC_IMAGE_QUALITY_FIRST = 0,
    ISIL_RC_IMAGE_SMOOTH_FIRST,
};

#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK              0x00000001
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK       0x00000002
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK                  0x00000004
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK                  0x00000008
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK                  0x00000010
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_ALL_MASK                  0x0000001f

typedef struct ISIL_h264_encode_rc{
    unsigned int change_mask_flag;
    unsigned int e_rc_type;
    unsigned int e_image_priority;
    unsigned int i_qpi;
    unsigned int i_qpp;
    unsigned int i_qpb;
}VENC_H264_RC;


typedef struct ISIL_h264_encode_rc_param{
    unsigned int  e_rc_type;
    unsigned char rc_param[0];
}VENC_H264_RC_PARAM;


//get channel map info
typedef struct ISIL_h264_chan_map_info{
    unsigned short i_logic_chan_id;
    unsigned short i_phy_chan_id;
}VENC_H264_CHAN_MAP_INFO;


//OSD config
#define            NAME_LEN                (32)
#define            SUB_LEN                 (44)
#define            OSD_ATTR_DISPLAY_OFF    (0)
#define            OSD_ATTR_DISPLAY_ON     (0x1)
#define            OSD_FONT_12             (0x2)
#define            OSD_FONT_24             (0x8)
#define            OSD_FONT_MASK           (0xe)
#define            CHAR_RECTANGLE_X_LEFT   (8)
#define            CHAR_RECTANGLE_Y_LEFT   (2)

typedef struct _ISIL_OSD_PARAM {
	unsigned int    channel;                                  //chan_id, this domain no use
	char            name[NAME_LEN];                    //chan’s name, the maxim number is 16
	unsigned int    name_attrib;                        //name’s attribute
	unsigned short  name_pos_x;                    //display position in video window
	unsigned short  name_pos_y;
	unsigned int    time_attrib;                           //display time attribute
	unsigned short  time_pos_x;                      // display position in video window
	unsigned short  time_pos_y;
	unsigned int    shelter1_attrib;                   //shelter rectangle 1
	unsigned short  shelter1_pos_x;                               //shelter rectangle 1 left top position video window
	unsigned short  shelter1_pos_y;
	unsigned short  shelter1_width;
	unsigned short  shelter1_height;
	unsigned int    shelter2_attrib;                   //shelter rectangle 2
	unsigned short  shelter2_pos_x;                               //shelter rectangle 2 left top position video window
	unsigned short  shelter2_pos_y;
	unsigned short  shelter2_width;
	unsigned short  shelter2_height;
	char            subtitle1[SUB_LEN];                  //char rectangle 1 content
	unsigned int    subtitle1_attrib;                  // char rectangle 1 attribute
	unsigned short  subtitle1_pos_x;                              // char rectangle 1 position
	unsigned short  subtitle1_pos_y;
	char            subtitle2[SUB_LEN];                  //char rectangle 2 content
	unsigned int    subtitle2_attrib;                  // char rectangle 2 attribute
	unsigned short  subtitle2_pos_x;              // char rectangle 2 position
	unsigned short  subtitle2_pos_y;
	char            subtitle3[SUB_LEN];                  //char rectangle 3 content
	unsigned int    subtitle3_attrib;                  // char rectangle 3 attribute
	unsigned short  subtitle3_pos_x;              // char rectangle 3 position
	unsigned short  subtitle3_pos_y;
	char            subtitle4[SUB_LEN];                  //char rectangle 3 content
	unsigned int    subtitle4_attrib;                  // char rectangle 3 attribute
	unsigned short  subtitle4_pos_x;              // char rectangle 3 position
	unsigned short  subtitle4_pos_y;

}__attribute__ ((packed))ISIL_OSD_CFG;




/**************************video mjpg encoding******************************/
enum ISIL_MJPEG_IMAGE_LEVEL_E{
    MJPEG_IMAGE_LEVEL_0 = 0,
    MJPEG_IMAGE_LEVEL_1,
    MJPEG_IMAGE_LEVEL_2,
    MJPEG_IMAGE_LEVEL_3,
    MJPEG_IMAGE_LEVEL_4,
    MJPEG_IMAGE_LEVEL_RESERVE,
};

#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_LEVEL_MASK           0x00000001
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_WIDTH_MASK           0x00000002
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_HEIGHT_MASK          0x00000004
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_NUMBER_MASK  0x00000008
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_STRIDE_MASK  0x00000010
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_TYPE  			   0x00000020


#define MJPEG_WIDTH_FRAME_D1_PAL    (720)
#define MJPEG_WIDTH_FRAME_D1_NTSC   (720)
#define MJPEG_HEIGHT_FRAME_D1_PAL   (576)
#define MJPEG_HEIGHT_FRAME_D1_NTSC  (480)
//mjpg encoding cfg
typedef struct ISIL_mjpeg_encode_param{
    unsigned int change_mask_flag;
    unsigned int e_image_level;
    unsigned int i_image_width_mb_size;
    unsigned int i_image_height_mb_size;
    unsigned int i_capture_frame_number;
    unsigned int i_capture_frame_stride;
    unsigned int i_capture_type;
}VENC_MJPG_CFG;

//mjpg capture type
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER   (1 << 31)//auto
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_USER    (1 << 30)//manual

//mjpg capture event
#define ISIL_MJPG_ENCODER_CAPTURE_EVENT_TYPE_VL  (1 << 29) //video lost
#define ISIL_MJPG_ENCODER_CAPTURE_EVENT_TYPE_VS  (1 << 28) //video shelter

//stream type
#define ISIL_MJPG_ENCODER_CAPTURE_STREAM_TYPE_M  (1<<0)//main stream
#define ISIL_MJPG_ENCODER_CAPTURE_STREAM_TYPE_S   (1<<1)//sub stream

/***************************audio ***********************/

//audio config parament
#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_BIT_WIDE_MASK      (0x00000001)
#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_SAMPLE_RATE_MASK   (0x00000002)
#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_ENCODE_TYPE_MASK   (0x00000004)


typedef struct ISIL_chip_audio_param{
    unsigned int   change_mask_flag;
    unsigned int   channel;
    unsigned char  i_bit_wide;
    unsigned char  i_sample_rate;
    unsigned short e_audio_type;
}VENC_AUD_CFG;


//encoding
/*************************************************************
function name : ISIL_CODEC_ENC_CreateCh
description   : create video/audio encoding channel
input         :
    CODEC_CHANNEL *pEncChan : point data struct
output        : none
return        :
   CODEC_ERR_OK  : sucess
   CODEC_ERR_INVAL:input parament is invalid
   CODEC_ERR_FAIL: fail
***************************************************************/
extern int ISIL_CODEC_ENC_CreateCh(CODEC_CHANNEL *pEncChan, CODEC_HANDLE* pChipHandle, char* pChDir);


/***********************************************
function name : ISIL_CODEC_ENC_ReleaseCh
description   : release video encoding channel
input         :
  CODEC_CHANNEL *pEncChan : point data struct
output        : none
return        :
  CODEC_ERR_OK  : sucess
  CODEC_ERR_INVAL:input parament is invalid
  CODEC_ERR_FAIL: fail
***************************************************/
extern int ISIL_CODEC_ENC_ReleaseCh(CODEC_CHANNEL *pEncChan, CODEC_HANDLE* pChipHandle);


/*************************************************************
function name : ISIL_CODEC_ENC_GetChHandle
description   : get video encoding channel fd
input         :
    CODEC_CHANNEL *pEncChan: channel info
output        :
    CODEC_HANDLE *pEncHandle: point to got fd
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
*****************************************************************/

extern int ISIL_CODEC_ENC_GetChHandle(CODEC_CHANNEL *pEncChan, CODEC_HANDLE *pEncHandle);


/********************************************************
function name : ISIL_CODEC_ENC_EnableCh
description   : enable video encoding channel
input         :
    CODEC_CHANNEL *pEncChan : point data struct
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
*******************************************************/

extern int ISIL_CODEC_ENC_EnableCh(CODEC_CHANNEL *pEncChan);

/******************************************************************
function name : ISIL_CODEC_ENC_DisableCh
description   : disable video encoding channel
input         :
     WORK_MODE eWorkMode: work mode
    CODEC_CHANNEL *pEncChan : point data struct
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
*********************************************************************/
extern int ISIL_CODEC_ENC_DisableCh(CODEC_CHANNEL *pEncChan);


/********************************************************
function name : ISIL_CODEC_ENC_SetCfg
description   : set video encoding cfg
input         :
    CODEC_CHANNEL *pEncChan: point to channel
    void* pEncCfg : point to cfg data struct
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
*******************************************************/

extern int ISIL_CODEC_ENC_SetCfg(CODEC_CHANNEL *pEncChan, void* pEncCfg);

/*************************************************************
function name : ISIL_CODEC_ENC_GetCfg
description   : get video encoding cfg
input         :
    CODEC_CHANNEL *pEncChan: point to channel
output        :
    void* pEncCfg : point to cfg data struct
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/
extern int ISIL_CODEC_ENC_GetCfg(CODEC_CHANNEL *pEncChan, void* pEncCfg);


/*************************************************************
function name : ISIL_CODEC_ENC_SetFeature
description   : set encoder channel feature, now only h264 valid
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
	void* pEncFeature: point to feature data
output        :
    none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/

extern int ISIL_CODEC_ENC_SetFeature(CODEC_CHANNEL *pEncChan, void* pEncFeature);

/*************************************************************
function name : ISIL_CODEC_ENC_GetFeature
description   : get encoder channel feature, now only h264 valid
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
output        :
    void* pEncFeature: point to feature data
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/
extern int ISIL_CODEC_ENC_GetFeature(CODEC_CHANNEL *pEncChan, void* pEncFeature);


/*************************************************************
function name : ISIL_CODEC_ENC_SetRC
description   : set rc parament, now only for h264
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
	void* pEncRC: point to data
output        :
    none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/
extern int ISIL_CODEC_ENC_SetRC(CODEC_CHANNEL *pEncChan, void* pEncRC);

/*************************************************************
function name : ISIL_CODEC_ENC_GetRC
description   : get now use rc parament
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
output        :
    void* pEncRC: point to data
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/
extern int ISIL_CODEC_ENC_GetRC(CODEC_CHANNEL *pEncChan, void* pEncRC);

/*************************************************************
function name : ISIL_CODEC_ENC_SetRCParam
description   : add new rc Algorithm, call it
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
	void* pEncRCParam: point to data
output        :
    none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/

extern int ISIL_CODEC_ENC_SetRCParam(CODEC_CHANNEL *pEncChan, void* pEncRCParam);

/*************************************************************
function name : ISIL_CODEC_ENC_GetRCParam
description   : get new rc parament
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
output        :
    void* pEncRCParam: point to data
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/
extern int ISIL_CODEC_ENC_GetRCParam(CODEC_CHANNEL *pEncChan, void* pEncRCParam);


/*************************************************************
function name : ISIL_CODEC_ENC_GetChMapInfo
description   : get map table between logic channel and physical channel
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
output        :
    void* pEncChMap: point to data
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/
extern int ISIL_CODEC_ENC_GetChMapInfo(CODEC_CHANNEL *pEncChan, void* pEncChMap);


/*************************************************************
function name : ISIL_CODEC_ENC_StartCap
description   : start capture, only for mjpg
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
 	unsigned int u32Type: capture type
//u32Type  need parsing as bit area
//bit 31: 0==stop auto capture,1== enable auto capture
//bit 30: 0== stop manual capture, 1== enable manual capture
//bit 29-16:capture event, eg alarm
//bit 15-0:mjpg stream type 
//mjpg capture type
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER   (1 << 31)//auto
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_USER    (1 << 30)//manual

//mjpg capture event
#define ISIL_MJPG_ENCODER_CAPTURE_EVENT_TYPE_VL  (1 << 29) //video lost
#define ISIL_MJPG_ENCODER_CAPTURE_EVENT_TYPE_VS  (1 << 28) //video shelter

//stream type
#define ISIL_MJPG_ENCODER_CAPTURE_STREAM_TYPE_M  (1<<0)//main stream
#define ISIL_MJPG_ENCODER_CAPTURE_STREAM_TYPE_S   (1<<1)//sub stream 
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/

extern int ISIL_CODEC_ENC_StartCap(CODEC_CHANNEL *pEncChan, unsigned int u32Type);


/*************************************************************
function name : ISIL_CODEC_ENC_StopCap
description   : stop capture, only for mjpg
input         :
 	CODEC_CHANNEL *pEncChan: point to channel
 	unsigned int u32Type: capture type
//u32Type  need parsing as bit area
//bit 31: 0==stop auto capture,1== enable auto capture
//bit 30: 0== stop manual capture, 1== enable manual capture
//bit 29-16:capture event, eg alarm
//bit 15-0:mjpg stream type 
//mjpg capture type
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER   (1 << 31)//auto
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_USER    (1 << 30)//manual

//mjpg capture event
#define ISIL_MJPG_ENCODER_CAPTURE_EVENT_TYPE_VL  (1 << 29) //video lost
#define ISIL_MJPG_ENCODER_CAPTURE_EVENT_TYPE_VS  (1 << 28) //video shelter

//stream type
#define ISIL_MJPG_ENCODER_CAPTURE_STREAM_TYPE_M  (1<<0)//main stream
#define ISIL_MJPG_ENCODER_CAPTURE_STREAM_TYPE_S   (1<<1)//sub stream 
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/

extern int ISIL_CODEC_ENC_StopCap(CODEC_CHANNEL *pEncChan, unsigned int u32Type);


/***************************************************************************
function name : ISIL_CODEC_ENC_GetData
description   : get video encoding all data
input         :
    CODEC_CHANNEL *pEncChan: point to channel info
 	CODEC_STREAM_DATA   *pstStream : point to data
 	DATA_ACCESS_MODE    eReadway   : read data way
	IO_BLK_TYPE 		eBlk	   :io block or none block
output        :
     CODEC_STREAM_DATA   *pstStream : point to data
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
note:
    if return val < 0, pStream->u32ActiveLen > 0;usr's buffer is too small
    if return val < 0, pStream->u32ActiveLen <= 0; read fail
    if return val == 0, pStream->u32ActiveLen == 0, no data
    if return val == 0, pStream->u32ActiveLen > 0, read data ok
****************************************************************************/

extern int ISIL_CODEC_ENC_GetData(CODEC_CHANNEL *pEncChan, CODEC_STREAM_DATA *pStream, DATA_ACCESS_MODE eReadway, IO_BLK_TYPE eBlk);

/********************************************************
* function name :   ISIL_CODEC_ENC_DiscardLargeFrame
* discription   :   when video data frame is larger
*                    than all buffers we can use,tell driver
*                    to discard this frame
* input         :
*      CODEC_CHANNEL *pEncChan:channel information
* output        :
*                   none
* return        :
*   CODEC_ERR_INVAL: input parament is invalid
*   CODEC_ERR_OK   : ok
*   CODEC_ERR_FAIL : other errors
********************************************************/
extern int ISIL_CODEC_ENC_DiscardLargeFrame(CODEC_CHANNEL *pEncChan);


//osd
/********************************************************
* function name :   ISIL_CODEC_ENC_SetOSDCfg
* discription   :   set osd config parament
* input         :
*      CODEC_CHANNEL *pEncChan:channel information
*      void*         pOSDCfg :pointer of osd config context,now only for h264,see ISIL_OSD_CFG
* output        :
*                   none
* return        :
*   CODEC_ERR_INVAL: input parament is invalid
*   CODEC_ERR_OK   : ok
*   CODEC_ERR_FAIL : other errors
********************************************************/
extern int ISIL_CODEC_ENC_SetOSDCfg(CODEC_CHANNEL *pEncChan, void* pOSDCfg);

/********************************************************
* function name :   ISIL_CODEC_ENC_GetOSDCfg
* discription   :   get osd config parament
* input         :
*      CODEC_CHANNEL *pEncChan:channel information
* output        :
*      void*         pOSDCfg :pointer of osd config context,now only for h264,see ISIL_OSD_CFG
* return        :
*   CODEC_ERR_INVAL: input parament is invalid
*   CODEC_ERR_OK   : ok
*   CODEC_ERR_FAIL : other errors
********************************************************/
extern int ISIL_CODEC_ENC_GetOSDCfg(CODEC_CHANNEL *pEncChan, void* pOSDCfg);

//flush driver buffer,clear all data in buffer queue
extern int ISIL_CODEC_ENC_Flush(CODEC_CHANNEL *pEncChan);


#ifdef __cplusplus
}
#endif
#endif
