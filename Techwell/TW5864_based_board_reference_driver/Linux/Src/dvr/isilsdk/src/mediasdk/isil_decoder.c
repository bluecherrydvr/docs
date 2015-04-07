/**********************************************************
file name    : isil_codec_interface.c
author       : dengjing
description  : codec interface between CODEC SDK and user program
create data  : 2010-03-03
history      : none
other        : none
***********************************************************/
//header file
#include "isil_interface.h"
#include "isil_codec_debug.h"
#include "isil_decoder.h"
#include "isil_venc_h264.h"
#include "isil_venc_mjpg.h"
#include "isil_venc_audio.h"
#include "isil_vdec_h264.h"
#include "isil_vdec_mjpg.h"
#include "isil_vdec_audio.h"
//debug
#define DECODER_DEBUG

//API,function

//video decoding

/***************************************************************************
function name : ISIL_CODEC_DEC_CreateCh
description   : create video decoding channel
input         :
    CODEC_CHANNEL *pDecChan; point to channel info
    CODEC_HANDLE* pChipHandle: chip fd
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_EXIST  :  had create this channel
    CODEC_ERR_FAIL: fail
****************************************************************************/
int ISIL_CODEC_DEC_CreateCh(CODEC_CHANNEL *pDecChan, CODEC_HANDLE* pChipHandle, char* pChDir)
{
    int s32Ret = 0;

    if(!pDecChan || !pChipHandle)
    {
      #ifdef DECODER_DEBUG
	  CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }
    //DEBUG_FUNCTION();
    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        //DEBUG_FUNCTION();
         s32Ret = ISIL_VDEC_H264_CreateCh(pDecChan, pChipHandle, pChDir);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        //DEBUG_FUNCTION();
        s32Ret = ISIL_VDEC_MJPG_CreateCh(pDecChan, pChipHandle, pChDir);
    }
    else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VDEC_AUD_CreateCh(pDecChan, pChipHandle, pChDir);
    }
    else
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" codec type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }
    //DEBUG_FUNCTION();

    return s32Ret;
}


/***************************************************************************
function name : ISIL_CODEC_DEC_ReleaseCh
description   : release video decoding channel
input         :
    CODEC_CHANNEL *pDecChan; point to channel info
    CODEC_HANDLE* pChipHandle: chip fd
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_EXIST  :  had create this channel
    CODEC_ERR_FAIL: fail
****************************************************************************/
int ISIL_CODEC_DEC_ReleaseCh(CODEC_CHANNEL *pDecChan, CODEC_HANDLE* pChipHandle)
{
    int s32Ret = 0;

   
    if(!pDecChan || !pChipHandle)
    {
      #ifdef DECODER_DEBUG
	  CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        fprintf(stderr, "---call release decoder Vch:%x---\n", pDecChan->u32Ch);
        s32Ret = ISIL_VDEC_H264_ReleaseCh(pDecChan, pChipHandle);
        fprintf(stderr, "---call release decoder Vch:%x ok---\n", pDecChan->u32Ch);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VDEC_MJPG_ReleaseCh(pDecChan, pChipHandle);
    }
    else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        fprintf(stderr, "---call release decoder Ach:%x---\n", pDecChan->u32Ch);
        s32Ret = ISIL_VDEC_AUD_ReleaseCh(pDecChan, pChipHandle);
        fprintf(stderr, "---call release decoder Ach:%x ok---\n", pDecChan->u32Ch);
    }
    else
    {
       #ifdef DECODER_DEBUG
       CODEC_DEBUG(" codec type is invalid ");
       #endif
       s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}

/***************************************************************************
function name : ISIL_CODEC_DEC_EnableCh
description   : enable video decoding channel
input         :
    CODEC_CHANNEL *pDecChan: point to channel info
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_EXIST  :  had create this channel
    CODEC_ERR_FAIL: fail
****************************************************************************/
int ISIL_CODEC_DEC_EnableCh(CODEC_CHANNEL *pDecChan)
{
    int s32Ret = 0;

    if(pDecChan == NULL)
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VDEC_H264_Enable(pDecChan);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VDEC_MJPG_Enable(pDecChan);
    }
    else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VDEC_AUD_Enable(pDecChan);
    }
    else
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" codec type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


/***************************************************************************
function name : ISIL_CODEC_DEC_DisableCh
description   : disable video decoding channel
input         :
    CODEC_CHANNEL *pDecChan: point to channel info
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
****************************************************************************/
int ISIL_CODEC_DEC_DisableCh(CODEC_CHANNEL *pDecChan)
{
    int s32Ret = 0;

    if(pDecChan == NULL)
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VDEC_H264_Disable(pDecChan);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VDEC_MJPG_Disable(pDecChan);
    }
    else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VDEC_AUD_Disable(pDecChan);
    }
    else
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" codec type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


/***************************************************************************
function name : ISIL_CODEC_DEC_SetCfg
description   : set video decoding cfg
input         :
    CODEC_CHANNEL *pDecChan: point to channel info
    void* pDecCfg : point to cfg data struct
output        : none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
****************************************************************************/

int ISIL_CODEC_DEC_SetCfg(CODEC_CHANNEL *pDecChan, void* pDecCfg)
{
    int s32Ret = 0;

    if((pDecChan == NULL) || (pDecCfg == NULL))
    {
      #ifdef DECODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VDEC_H264_SetChCfg(pDecChan, (VDEC_H264_CFG*)pDecCfg);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VDEC_MJPG_SetChCfg(pDecChan, (VDEC_MJPG_CFG*)pDecCfg);
    }
    else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VDEC_AUD_SetChCfg(pDecChan, (VDEC_AUD_CFG*)pDecCfg);
    }
    else
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" encode type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}

/***************************************************************************
function name : ISIL_CODEC_DEC_GetCfg
description   : get video decoding cfg
input         :
    CODEC_CHANNEL *pDecChan: point to channel info
output        :
    void* pDecCfg : point to cfg data struct
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
****************************************************************************/

int ISIL_CODEC_DEC_GetCfg(CODEC_CHANNEL *pDecChan, void* pDecCfg)
{
    int s32Ret = 0;

    if((pDecChan == NULL) || (pDecCfg == NULL))
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VDEC_H264_GetChCfg(pDecChan, (VDEC_H264_CFG*)pDecCfg);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VDEC_MJPG_GetChCfg(pDecChan, (VDEC_MJPG_CFG*)pDecCfg);
    }
    else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VDEC_AUD_GetChCfg(pDecChan, (VDEC_AUD_CFG*)pDecCfg);
    }
    else
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" encode type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


/***************************************************************************
function name : ISIL_CODEC_DEC_SendData
description   : send data to decoder
input         :
    CODEC_CHANNEL *pDecChan: point to channel info
    CODEC_STREAM_DATA *pstStream : point to data
output        :
    none
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
****************************************************************************/
int ISIL_CODEC_DEC_SendData(CODEC_CHANNEL *pDecChan, CODEC_STREAM_DATA *pStream)
{
    int s32Ret = 0;

    if((pDecChan == NULL) || (pStream == NULL))
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

	//todo
    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VDEC_H264_SendData(pDecChan, pStream);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VDEC_MJPG_SendData(pDecChan, pStream);
    }
    else if(pDecChan->eCodecType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VDEC_AUD_SendData(pDecChan, pStream);
    }
    else
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" point is invalid");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


/***************************************************************************
function name : ISIL_CODEC_DEC_GetChHandle
description   : get video decoding channel fd
input         :
    CODEC_CHANNEL *pDecChan: point to channel info
output        :
    CODEC_HANDLE *pDecHandle : point to fd
return        :
    CODEC_ERR_OK  : sucess
    CODEC_ERR_INVAL:input parament is invalid
    CODEC_ERR_FAIL: fail
****************************************************************************/
int ISIL_CODEC_DEC_GetChHandle(CODEC_CHANNEL *pDecChan, CODEC_HANDLE *pDecHandle)
{
    int s32Ret = 0;

    if((pDecChan == NULL) || (pDecHandle == NULL))
    {
      #ifdef DECODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VDEC_H264_GetChHandle(pDecChan, pDecHandle);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VDEC_MJPG_GetChHandle(pDecChan, pDecHandle);
    }
    else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VDEC_AUD_GetChHandle(pDecChan, pDecHandle);
    }
    else
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" encode type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}



/************************************************************************
    function    : ISIL_VDEC_H264_Flush
    description : clear h264 decoder data
    input:
        CODEC_CHANNEL *pDecChan: point to channel info
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail
*************************************************************************/

int ISIL_CODEC_DEC_Flush(CODEC_CHANNEL *pDecChan)
{
    int s32Ret = 0;

    if(pDecChan == NULL)
    {
      #ifdef DECODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }
    //todo
    if(pDecChan->eCodecType == CODEC_VIDEO_H264)
    {
        s32Ret = ISIL_VDEC_H264_Flush(pDecChan);
    }
    else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG)
    {
        s32Ret = ISIL_VDEC_MJPG_Flush(pDecChan);
    }
    else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO)
    {
        s32Ret = ISIL_VDEC_AUD_Flush(pDecChan);
    }
    else
    {
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" point is invalid");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}

//pull or push
int ISIL_CODEC_DEC_SetStrategy(CODEC_CHANNEL *pDecChan, VDEC_CH_STRATEGY *pStrategy)
{
    int s32Ret = 0;

    if((pDecChan == NULL) || (pStrategy == NULL)){
      #ifdef DECODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264){
        s32Ret = ISIL_VDEC_H264_SetStrategy(pDecChan, pStrategy);
    }else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG){
        s32Ret = ISIL_VDEC_MJPG_SetStrategy(pDecChan, pStrategy);
    }else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO){
        s32Ret = ISIL_VDEC_AUD_SetStrategy(pDecChan, pStrategy);
    }else{
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" encode type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}

//set playback mode
int ISIL_CODEC_DEC_SetPBMode(CODEC_CHANNEL *pDecChan, VDEC_CH_MODE *pMode)
{
    int s32Ret = 0;

    if((pDecChan == NULL) || (pMode == NULL)){
      #ifdef DECODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264){
        s32Ret = ISIL_VDEC_H264_SetPBMode(pDecChan, pMode);
    }else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG){
        s32Ret = ISIL_VDEC_MJPG_SetPBMode(pDecChan, pMode);
    }else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO){
        s32Ret = ISIL_VDEC_AUD_SetPBMode(pDecChan, pMode);
    }else{
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" encode type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


//set request-frame command timeout interval
int ISIL_CODEC_DEC_SetTimeout(CODEC_CHANNEL *pDecChan, VDEC_CH_TIMEOUT *pTimeout)
{
    int s32Ret = 0;

    if((pDecChan == NULL) || (pTimeout == NULL)){
      #ifdef DECODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264){
        s32Ret = ISIL_VDEC_H264_SetTimeout(pDecChan, pTimeout);
    }else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG){
        s32Ret = ISIL_VDEC_MJPG_SetTimeout(pDecChan, pTimeout);
    }else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO){
        s32Ret = ISIL_VDEC_AUD_SetTimeout(pDecChan, pTimeout);
    }else{
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" encode type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}


int ISIL_CODEC_DEC_SetStreamType(CODEC_CHANNEL *pDecChan, VDEC_CH_HEADER* pHeader)
{
	int s32Ret = 0;

	if((pDecChan == NULL) || (pHeader == NULL)){
	  #ifdef DECODER_DEBUG
	  CODEC_DEBUG(" point is null");
	  #endif
	  return CODEC_ERR_INVAL;
	}

	if(pDecChan->eCodecType == CODEC_VIDEO_H264){
		s32Ret = ISIL_VDEC_H264_SetStreamType(pDecChan, pHeader);
	}else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG){
		s32Ret = ISIL_VDEC_MJPG_SetStreamType(pDecChan, pHeader);
	}else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO){
		s32Ret = ISIL_VDEC_AUD_SetStreamType(pDecChan, pHeader);
	}else{
		#ifdef DECODER_DEBUG
		CODEC_DEBUG(" encode type is invalid ");
		#endif
		s32Ret = CODEC_ERR_INVAL;
	}

	return s32Ret;
}

int ISIL_CODEC_DEC_Readydie(CODEC_CHANNEL *pDecChan)
{
    int s32Ret = 0;

    if(pDecChan == NULL){
      #ifdef DECODER_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(pDecChan->eCodecType == CODEC_VIDEO_H264){
        s32Ret = ISIL_VDEC_H264_Readydie(pDecChan);
    }else if(pDecChan->eCodecType == CODEC_VIDEO_MJPG){
        s32Ret = ISIL_VDEC_MJPG_Readydie(pDecChan);
    }else if(pDecChan->eStreamType == STREAM_TYPE_AUDIO){
        s32Ret = ISIL_VDEC_AUD_Readydie(pDecChan);
    }else{
        #ifdef DECODER_DEBUG
        CODEC_DEBUG(" encode type is invalid ");
        #endif
        s32Ret = CODEC_ERR_INVAL;
    }

    return s32Ret;
}

