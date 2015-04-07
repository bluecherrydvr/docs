/************************************************************************
	file name  	: isil_vdec_h264.c
	author     	: dengjing
    create date : 2010-03-03
    description	: video h264 decoding related controll
    history		: none
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
//#include <errno.h>

#include "isil_vdec_h264.h"
#include "isil_codec_debug.h"
#include "isil_codec_errcode.h"
#include "isil_interface.h"

#define VDEC_H264_DEBUG


typedef struct//save channel info when system running
{
    unsigned char u8IsEnable;//whether the channel is enable or not
    unsigned char u8IsCreate;//whether the channel is create or not
    int  s32ChFd;//channel fd
    unsigned char u8Reserved[2];
}VDEC_H264_INFO;//initial value is zero

VDEC_H264_INFO VDEC_H264_TABLE[DEC_MAXCHIP][DEC_MAXCH];//save decoding channel info

/************************************************************************
    function    : ISIL_VDEC_H264_CreateCh
    description : create a h264 decoding channel
    input:
        unsigned char  u8Ch : channel number,0 ~ (max channel -1)
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_EXIST  :  had create this channel
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  create fail
*************************************************************************/
int ISIL_VDEC_H264_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir)
{
    int ch_fd = 0;
    int chip_fd = 0;
    int ret,major,minor = 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;
    char dir[256] = {};

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" chan: %d had create,fail", pChan->u32Ch);
        #endif
        return CODEC_ERR_EXIST;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    chip_fd = *pChipHandle;

    data = (struct cmd_arg*)malloc(sizeof(struct cmd_arg)
                            + sizeof(struct device_info));

    data->type = ISIL_DECODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;

    dev_info = (struct device_info *)(data->dev_info);

    sprintf(dir, "mkdir -p %s/dec", pChDir);
    system(dir);

    ret = ioctl(chip_fd, ISIL_CHIP_CREATE_CHAN, data);
    if(ret < 0){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" ioctl fail");
	perror("why: ");
        #endif
        free(data);
	data = NULL;
        return CODEC_ERR_FAIL;
    }

    major = dev_info->major;
    minor = dev_info->minor;

    memset(dir, 0x00, 256);
    sprintf(dir, "%s/dec/%s", pChDir, dev_info->path);
    remove(dir);
    if(mknod(dir,  S_IFCHR, ((major<<8) | minor)) < 0){
	    #ifdef VDEC_H264_DEBUG
	    CODEC_DEBUG(" mknod fail");
	    #endif
	    ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
	    free(data);
	    data = NULL;
	    return CODEC_ERR_FAIL;
    }

    //fprintf(stderr, "---open path: %s-----\n", dev_info->path);

    ch_fd = open(dir, O_RDWR);
    if(ch_fd < 0){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" open chann: %d fail", pChan->u32Ch);
        #endif
        ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
        free(data);
	data = NULL;
        return CODEC_ERR_FAIL;
    }

    VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd = ch_fd;
    VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 1;

    free(data);
    data = NULL;
    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_H264_ReleaseData
    description : release a h264 decoding channel
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid,channel is over max channel
        CODEC_ERR_NOT_EXIST :  this channel had released
        CODEC_ERR_FAIL   :  release fail
*************************************************************************/
int ISIL_VDEC_H264_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle)
{
    int chip_fd = 0;
    int ret= 0;
    struct cmd_arg *data = NULL;
    //struct device_info* dev_info = NULL;

    if(!pChan || !pChipHandle){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH)
    {
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d had release,fail", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    if(VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
        if(ISIL_VDEC_H264_Disable(pChan) < 0)
        {
            #ifdef VDEC_H264_DEBUG
            CODEC_DEBUG(" disable chann: %d fail", pChan->u32Ch);
            #endif
            return CODEC_ERR_FAIL;
        }
    }

    close(VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd);

    chip_fd = *pChipHandle;

    data = (struct cmd_arg*)malloc(sizeof(struct cmd_arg)
                            + sizeof(struct device_info));

    data->type = ISIL_DECODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;

    //dev_info = (struct device_info *)(data->dev_info);

    ret = ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
    if(ret < 0){
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG(" ioctl fail");
        #endif
        free(data);
	data = NULL;
        return CODEC_ERR_FAIL;
    }

    VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 0;
    free(data);
    data = NULL;
    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_H264_Enable
    description : enable a channel,start decoding
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_EXIST  :  channel had enable
        CODEC_ERR_NOT_EXIST: channel not create
        CODEC_ERR_FAIL   :  fail
*************************************************************************/
int ISIL_VDEC_H264_Enable(CODEC_CHANNEL *pChan)//start decoding
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }


    if(VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d had enable", pChan->u32Ch);
        #endif
        return CODEC_ERR_EXIST;
    }

    //send start decoding command to driver
    fd = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_LOGIC_CHAN_ENABLE_SET, NULL) < 0) {
      #ifdef VDEC_H264_DEBUG
      CODEC_DEBUG("enable channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }

    VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 1;
    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_H264_Disable
    description : disable a channel,stop decoding
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_NOT_EXIST  :  channel had disable
        CODEC_ERR_FAIL   :  fail
*************************************************************************/

int ISIL_VDEC_H264_Disable(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH)
    {
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_H264_DEBUG
	CODEC_DEBUG("chann:%d had disable", pChan->u32Ch);
	#endif
	return CODEC_ERR_NOT_EXIST;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d had disable", pChan->u32Ch);
        #endif
        return CODEC_ERR_EXIST;
    }

    //send stop decoding command to driver
    fd = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_LOGIC_CHAN_DISABLE_SET, NULL) < 0) {
      #ifdef VDEC_H264_DEBUG
      CODEC_DEBUG("disable channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }

    VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 0;
    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_H264_SetChCfg
    description : set h264 decoding channel config parament
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        VDEC_H264_CFG* stpVdecH264Cfg: pointer of the decoding parament
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail,set cfg fail for driver
*************************************************************************/
int ISIL_VDEC_H264_SetChCfg(CODEC_CHANNEL *pChan, VDEC_H264_CFG* stpVdecH264Cfg)
{
    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVdecH264Cfg == NULL){
	#ifdef VDEC_H264_DEBUG
	CODEC_DEBUG(" point is null ");
	#endif
	return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_H264_DEBUG
	CODEC_DEBUG(" channel not create ");
	#endif
	return CODEC_ERR_NOT_EXIST;
    }


    //todo

    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_H264_GetChCfg
    description : get h264 decoding channel config parament
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        VDEC_H264_CFG* stpVdecH264Cfg: pointer of the decoding parament
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail,get cfg fail for driver
*************************************************************************/
int ISIL_VDEC_H264_GetChCfg(CODEC_CHANNEL *pChan, VDEC_H264_CFG* stpVdecH264Cfg)
{
    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVdecH264Cfg == NULL){
	#ifdef VDEC_H264_DEBUG
	CODEC_DEBUG(" point is null ");
	#endif
	return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_H264_DEBUG
	CODEC_DEBUG(" channel not create ");
	#endif
	return CODEC_ERR_NOT_EXIST;
    }

    //todo

    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_H264_SendData
    description : send h264 data to decoder
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        CODEC_STREAM_DATA *stpStream: point to  got data struct
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail
*************************************************************************/

int ISIL_VDEC_H264_SendData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *pstStream)
{
    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pstStream == NULL){
      #ifdef VDEC_H264_DEBUG
      CODEC_DEBUG(" decoding data pointer is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_H264_DEBUG
	CODEC_DEBUG(" channel not create ");
	#endif
	return CODEC_ERR_NOT_EXIST;
    }
    //todo

    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_H264_GetChHandle
    description : get channel handle(fd)
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        CODEC_HANDLE *pHandle: point to fd
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_NOT_EXIST:channel not create
        CODEC_ERR_FAIL   :  fail
*************************************************************************/

int ISIL_VDEC_H264_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle)
{
    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pHandle == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    *pHandle = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_H264_Flush
    description : clear h264 decoder data
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        CODEC_STREAM_DATA *stpStream: point to  got data struct
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail
*************************************************************************/

int ISIL_VDEC_H264_Flush(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_H264_DEBUG
	CODEC_DEBUG(" channel not create ");
	#endif
	return CODEC_ERR_NOT_EXIST;
    }

    //todo
    fd = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_CODEC_CHAN_FLUSH, NULL) < 0) {
      #ifdef VDEC_H264_DEBUG
      CODEC_DEBUG("flush channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

//set playback strategy
int ISIL_VDEC_H264_SetStrategy(CODEC_CHANNEL *pChan, VDEC_CH_STRATEGY *pStrategy)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pStrategy == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_STRATEGY, pStrategy) < 0){
	#ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}

//set playback mode
int ISIL_VDEC_H264_SetPBMode(CODEC_CHANNEL *pChan, VDEC_CH_MODE *pMode)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pMode == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    fprintf(stderr, "---h264 dec set pb mode---\n");
    fprintf(stderr, "--pb,d:%x,c:%x,l:%x,k:%x,x:%lx--\n", 
	    pMode->direction, pMode->is_continue, pMode->loop,
	    pMode->key_frame, pMode->speed.x_speed_value);

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_CHANG_MODE, pMode) < 0){
	#ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }
    
    fprintf(stderr, "---set pb mode ok---\n");
    return CODEC_ERR_OK;
}


//set request-frame command timeout interval
int ISIL_VDEC_H264_SetTimeout(CODEC_CHANNEL *pChan, VDEC_CH_TIMEOUT *pTimeout)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pTimeout == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_TIME_CONFIG, pTimeout) < 0){
	#ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


int ISIL_VDEC_H264_Readydie(CODEC_CHANNEL *pChan)
{
 int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_READY_TO_DIE, NULL) < 0){
	#ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VDEC_H264_SetStreamType(CODEC_CHANNEL *pChan, VDEC_CH_HEADER* pHeader)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pHeader == NULL){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_DEC_STREAM_TYPE, pHeader) < 0){
	#ifdef VDEC_H264_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}



