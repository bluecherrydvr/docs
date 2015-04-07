/**********************************************************
file name    : isil_codec_interface.c
author       : dengjing
description  : codec interface between CODEC SDK and user program
create data  : 2010-03-03
history      : none
other        : none
***********************************************************/
//header file
#include "isil_codec_debug.h"
#include "isil_encoder.h"
#include "isil_venc_h264.h"
#include "isil_venc_mjpg.h"
#include "isil_venc_audio.h"


//debug
#define ENCODER_DEBUG


//API,function
//video encoding

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
int ISIL_CODEC_ENC_CreateCh(CODEC_CHANNEL *pEncChan, CODEC_HANDLE* pChipHandle, char* pChDir)
{
   int s32Ret = 0;

   if((pEncChan == NULL) || (pChipHandle == NULL) || (pChDir == NULL))
   {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
   }

   if(pEncChan->eCodecType == CODEC_VIDEO_H264)
   {
       //fprintf(stderr, "---enter create h264 channel----\n");
       s32Ret = ISIL_VENC_H264_CreateCh(pEncChan, pChipHandle, pChDir);
   }
   else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
   {
       s32Ret = ISIL_VENC_MJPG_CreateCh(pEncChan, pChipHandle, pChDir);
   }
   else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
   {
       s32Ret = ISIL_VENC_AUD_CreateCh(pEncChan, pChipHandle, pChDir);
   }
   else
   {
       #ifdef ENCODER_DEBUG
       CODEC_DEBUG(" encode type is null ");
       #endif
       s32Ret =  CODEC_ERR_INVAL;
   }
   return s32Ret;
}


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
int ISIL_CODEC_ENC_ReleaseCh(CODEC_CHANNEL *pEncChan, CODEC_HANDLE* pChipHandle)
{
    int s32Ret = 0;

    if(!pEncChan || !pChipHandle)
    {
        #ifdef ENCODER_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VENC_H264_ReleaseCh(pEncChan, pChipHandle);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VENC_MJPG_ReleaseCh(pEncChan, pChipHandle);
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VENC_AUD_ReleaseCh(pEncChan, pChipHandle);
    }
    else
    {
       #ifdef ENCODER_DEBUG
       CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret =  CODEC_ERR_INVAL;
    }

    return s32Ret;
}

/***************************************************************************
function name : ISIL_CODEC_ENC_EnableCh
description   : enable video encoding channel
input         :
    CODEC_CHANNEL *pEncChan : point data struct
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
****************************************************************************/
int ISIL_CODEC_ENC_EnableCh(CODEC_CHANNEL *pEncChan)
{
    int s32Ret = 0;
    if(pEncChan == NULL)
    {
      #ifdef ENCODER_DEBUG
	  CODEC_DEBUG(" point is null");
	  #endif
	  return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VENC_H264_Enable(pEncChan);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VENC_MJPG_Enable(pEncChan);
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VENC_AUD_Enable(pEncChan);
    }
    else
    {
       #ifdef ENCODER_DEBUG
       CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
int ISIL_CODEC_ENC_DisableCh(CODEC_CHANNEL *pEncChan)
{
    int s32Ret = 0;

    if(pEncChan == NULL)
    {
      #ifdef ENCODER_DEBUG
	  CODEC_DEBUG(" point is null");
	  #endif
	  return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VENC_H264_Disable(pEncChan);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VENC_MJPG_Disable(pEncChan);
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VENC_AUD_Disable(pEncChan);
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
	   s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}

/***************************************************************************
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
****************************************************************************/

int ISIL_CODEC_ENC_SetCfg(CODEC_CHANNEL *pEncChan, void* pEncCfg)
{
    int s32Ret = 0;

    if(!pEncCfg || !pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }
    
    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
        //fprintf(stderr, "%s set h264 cfg w %d, h %d\n", __FUNCTION__, ((VENC_H264_CFG*)pEncCfg)->i_logic_video_width_mb_size, 
            //((VENC_H264_CFG*)pEncCfg)->i_logic_video_height_mb_size);
         s32Ret = ISIL_VENC_H264_SetChCfg(pEncChan, (VENC_H264_CFG*)pEncCfg);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VENC_MJPG_SetChCfg(pEncChan, (VENC_MJPG_CFG*)pEncCfg);
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VENC_AUD_SetChCfg(pEncChan, (VENC_AUD_CFG *)pEncCfg);
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
	s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}

/*****************************************************************
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
int ISIL_CODEC_ENC_GetCfg(CODEC_CHANNEL *pEncChan, void* pEncCfg)
{
    int s32Ret = 0;

    if((pEncChan == NULL) || (pEncChan == NULL))
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
            s32Ret = ISIL_VENC_H264_GetChCfg(pEncChan, (VENC_H264_CFG*)pEncCfg);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VENC_MJPG_GetChCfg(pEncChan, (VENC_MJPG_CFG*)pEncCfg);
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VENC_AUD_GetChCfg(pEncChan, (VENC_AUD_CFG *)pEncCfg);
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
	   s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
int ISIL_CODEC_ENC_SetFeature(CODEC_CHANNEL *pEncChan, void* pEncFeature)
{
    int s32Ret = 0;

    if(!pEncFeature || !pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
         s32Ret = ISIL_VENC_H264_SetChFeature(pEncChan, (VENC_H264_FEATURE *)pEncFeature);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
       
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
int ISIL_CODEC_ENC_GetFeature(CODEC_CHANNEL *pEncChan, void* pEncFeature)
{
    int s32Ret = 0;

    if((pEncChan == NULL) || (pEncFeature == NULL))
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VENC_H264_GetChFeature(pEncChan, (VENC_H264_FEATURE *)pEncFeature);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
	   s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}

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
int ISIL_CODEC_ENC_SetRC(CODEC_CHANNEL *pEncChan, void* pEncRC)
{
    int s32Ret = 0;

    if(!pEncRC || !pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
         s32Ret = ISIL_VENC_H264_SetChRC(pEncChan, (VENC_H264_RC *)pEncRC);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
       
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
int ISIL_CODEC_ENC_GetRC(CODEC_CHANNEL *pEncChan, void* pEncRC)
{
    int s32Ret = 0;

    if(!pEncRC || !pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
         s32Ret = ISIL_VENC_H264_GetChRC(pEncChan, (VENC_H264_RC *)pEncRC);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
       
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
int ISIL_CODEC_ENC_SetRCParam(CODEC_CHANNEL *pEncChan, void* pEncRCParam)
{
    int s32Ret = 0;

    if(!pEncRCParam || !pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
         s32Ret = ISIL_VENC_H264_SetChRCParam(pEncChan, (VENC_H264_RC_PARAM *)pEncRCParam);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
       
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
int ISIL_CODEC_ENC_GetRCParam(CODEC_CHANNEL *pEncChan, void* pEncRCParam)
{
    int s32Ret = 0;

    if(!pEncRCParam || !pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
         s32Ret = ISIL_VENC_H264_GetChRCParam(pEncChan, (VENC_H264_RC_PARAM *)pEncRCParam);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
       
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
int ISIL_CODEC_ENC_GetChMapInfo(CODEC_CHANNEL *pEncChan, void* pEncChMap)
{
    int s32Ret = 0;

    if(!pEncChMap || !pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
         s32Ret = ISIL_VENC_H264_GetChMapInfo(pEncChan, (VENC_H264_CHAN_MAP_INFO *)pEncChMap);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
       
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/
int ISIL_CODEC_ENC_StartCap(CODEC_CHANNEL *pEncChan, unsigned int u32Type)
{
    int s32Ret = 0;

    if(!pEncChan || !pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
         
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VENC_MJPG_StartCap(pEncChan, u32Type);
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
       
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
***********************************************************/
int ISIL_CODEC_ENC_StopCap(CODEC_CHANNEL *pEncChan, unsigned int u32Type)
{
    int s32Ret = 0;

    if(!pEncChan)
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
         
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VENC_MJPG_StopCap(pEncChan, u32Type);
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
       
    }
    else
    {
       #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


/***************************************************************************
function name : ISIL_CODEC_ENC_GetData
description   : get video encoding all data
input         :
    CODEC_CHANNEL *pEncChan: point to channel info
    CODEC_STREAM_DATA   *pstStream : point to data
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
int ISIL_CODEC_ENC_GetData(CODEC_CHANNEL *pEncChan, CODEC_STREAM_DATA *pStream, DATA_ACCESS_MODE eReadway, IO_BLK_TYPE eBlk)
{
    int s32Ret = 0;


    if((pStream == NULL) || (pEncChan == NULL))
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    //todo
    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
       s32Ret = ISIL_VENC_H264_GetData(pEncChan, pStream, eReadway, eBlk);
    }
	else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
       s32Ret = ISIL_VENC_MJPG_GetData(pEncChan, pStream, eReadway, eBlk);
    }
	else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VENC_AUD_GetData(pEncChan, pStream, eReadway, eBlk);
    }
    else
    {
       #ifdef ENCODER_DEBUG
           CODEC_DEBUG(" codec type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }
    return s32Ret;
}


/***************************************************************************
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
****************************************************************************/
int ISIL_CODEC_ENC_GetChHandle(CODEC_CHANNEL *pEncChan, CODEC_HANDLE *pEncHandle)
{
    int s32Ret = 0;

    if((pEncChan == NULL) || (pEncHandle == NULL))
    {
      #ifdef ENCODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VENC_H264_GetChHandle(pEncChan, pEncHandle);
    }
    else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VENC_MJPG_GetChHandle(pEncChan, pEncHandle);
    }
    else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VENC_AUD_GetChHandle(pEncChan, pEncHandle);
    }
    else
    {
       #ifdef ENCODER_DEBUG
       CODEC_DEBUG(" codec type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


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

int ISIL_CODEC_ENC_DiscardLargeFrame(CODEC_CHANNEL *pEncChan)
{
    int s32Ret = CODEC_ERR_OK;

    if(!pEncChan){
        #ifdef ENCODER_DEBUG
        CODEC_DEBUG(" pointor is NULL ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pEncChan->eCodecType == CODEC_VIDEO_H264){
        s32Ret = ISIL_VENC_H264_DiscardFrame(pEncChan);
    }else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG){
        s32Ret = ISIL_VENC_MJPG_DiscardFrame(pEncChan);
    }else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO){
        s32Ret = ISIL_VENC_AUD_DiscardFrame(pEncChan);
    }else{
       #ifdef ENCODER_DEBUG
       CODEC_DEBUG(" encode type is invalid ");
       #endif
       s32Ret =  CODEC_ERR_INVAL;
    }

    return s32Ret;
}


int ISIL_CODEC_ENC_SetOSDCfg(CODEC_CHANNEL *pEncChan, void* pOSDCfg)
{
	int s32Ret = 0;

	if(!pOSDCfg || !pEncChan)
	{
	  #ifdef ENCODER_DEBUG
	  CODEC_DEBUG(" point is null");
	  #endif
	  return CODEC_ERR_INVAL;
	}

	if(pEncChan->eCodecType == CODEC_VIDEO_H264)
	{
		 s32Ret = ISIL_VENC_H264_SetOSDCfg(pEncChan, (ISIL_OSD_CFG *)pOSDCfg);
	}
	else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
	{

	}
	else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
	{

	}
	else
	{
	   #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
	   #endif
	   s32Ret = CODEC_ERR_INVAL;
	}

	return s32Ret;
}


int ISIL_CODEC_ENC_GetOSDCfg(CODEC_CHANNEL *pEncChan, void* pOSDCfg)
{
	int s32Ret = 0;

	if(!pOSDCfg || !pEncChan)
	{
	  #ifdef ENCODER_DEBUG
	  CODEC_DEBUG(" point is null");
	  #endif
	  return CODEC_ERR_INVAL;
	}

	if(pEncChan->eCodecType == CODEC_VIDEO_H264)
	{
		 s32Ret = ISIL_VENC_H264_GetOSDCfg(pEncChan, (ISIL_OSD_CFG *)pOSDCfg);
	}
	else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
	{

	}
	else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
	{

	}
	else
	{
	   #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
	   #endif
	   s32Ret = CODEC_ERR_INVAL;
	}

	return s32Ret;
}


int ISIL_CODEC_ENC_Flush(CODEC_CHANNEL *pEncChan)
{
	int s32Ret = 0;

	if(!pEncChan)
	{
	  #ifdef ENCODER_DEBUG
	  CODEC_DEBUG(" point is null");
	  #endif
	  return CODEC_ERR_INVAL;
	}

	if(pEncChan->eCodecType == CODEC_VIDEO_H264)
	{
		 s32Ret = ISIL_VENC_H264_Flush(pEncChan);
	}
	else if(pEncChan->eCodecType == CODEC_VIDEO_MJPG)
	{
		s32Ret = ISIL_VENC_MJPG_Flush(pEncChan);
	}
	else if(pEncChan->eStreamType == STREAM_TYPE_AUDIO)
	{
		s32Ret = ISIL_VENC_AUD_Flush(pEncChan);
	}
	else
	{
	   #ifdef ENCODER_DEBUG
	   CODEC_DEBUG(" encode type is invalid ");
	   #endif
	   s32Ret = CODEC_ERR_INVAL;
	}

	return s32Ret;
}
