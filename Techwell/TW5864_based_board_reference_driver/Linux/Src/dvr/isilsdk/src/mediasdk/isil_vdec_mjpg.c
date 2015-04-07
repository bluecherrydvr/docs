/************************************************************************
    file name  	: isil_vdec_mjpg.c
    author     	: dengjing
    create date : 2010-03-03
    description	: mjpg decoding related controll
    history	: none
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

#include "isil_vdec_mjpg.h"
#include "isil_codec_debug.h"
#include "isil_codec_errcode.h"
#include "isil_interface.h"

#define VDEC_MJPG_DEBUG

typedef struct//save channel info when system running
{
    unsigned char u8IsEnable;//whether the channel is enable or not
    unsigned char u8IsCreate;//whether the channel is create or not
    int  s32ChFd;//channel fd
    unsigned char u8Reserved[2];
}VDEC_MJPG_INFO;//initial value is zero

VDEC_MJPG_INFO VDEC_MJPG_TABLE[DEC_MAXCHIP][DEC_MAXCH];//save decoding channel info

/************************************************************************
    function    : ISIL_VDEC_MJPG_CreateCh
    description : create a mjpg decoding channel
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
int ISIL_VDEC_MJPG_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir)
{
    int ch_fd = 0;
    int chip_fd = 0;
    int ret,major,minor = 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;
    char dir[256] = {};

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_MJPG_DEBUG
	CODEC_DEBUG(" chan: %d had create,fail", pChan->u32Ch);
	#endif
	return CODEC_ERR_EXIST;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
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
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" ioctl fail");
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
	#ifdef VDEC_MJPG_DEBUG
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
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" open chann: %d fail", pChan->u32Ch);
        #endif
        ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
        free(data);
	data = NULL;
        return CODEC_ERR_FAIL;
    }
    
    VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd = ch_fd;
    VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 1;

    free(data);
    data = NULL;
    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_MJPG_ReleaseData
    description : release a mjpg decoding channel
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
int ISIL_VDEC_MJPG_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle)
{
    int chip_fd = 0;
    int ret= 0;
    struct cmd_arg *data = NULL;
    //struct device_info* dev_info = NULL;

    if(!pChan || !pChipHandle){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH)
    {
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {	
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d had release,fail", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }
    
    if(VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
        if(ISIL_VDEC_MJPG_Disable(pChan) < 0)
        {
            #ifdef VDEC_MJPG_DEBUG
            CODEC_DEBUG(" disable chann: %d fail", pChan->u32Ch);
            #endif
            return CODEC_ERR_FAIL;
        }
    }
    
    close(VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd);

    chip_fd = *pChipHandle;

    data = (struct cmd_arg*)malloc(sizeof(struct cmd_arg) 
                            + sizeof(struct device_info));

    data->type = ISIL_DECODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;

    ret = ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
    if(ret < 0){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" ioctl fail");
        #endif
        free(data);
	data = NULL;
        return CODEC_ERR_FAIL;
    }

    VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 0;
    free(data);
    data = NULL;
    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_MJPG_Enable
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
int ISIL_VDEC_MJPG_Enable(CODEC_CHANNEL *pChan)//start decoding
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){	
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    
    if(VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable){	
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d had enable", pChan->u32Ch);
        #endif
        return CODEC_ERR_EXIST;
    }

    //send start decoding command to driver
    fd = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_LOGIC_CHAN_ENABLE_SET, NULL) < 0) {
      #ifdef VDEC_MJPG_DEBUG
      CODEC_DEBUG("enable channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }
    
    VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 1;
    return CODEC_ERR_OK;}

/************************************************************************
    function    : ISIL_VDEC_MJPG_Disable
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

int ISIL_VDEC_MJPG_Disable(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){	
		#ifdef VDEC_MJPG_DEBUG
		CODEC_DEBUG("chann:%d had disable", pChan->u32Ch);
		#endif
		return CODEC_ERR_NOT_EXIST;
    }
	
    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable){	
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d had disable", pChan->u32Ch);
        #endif
        return CODEC_ERR_EXIST;
    }
 
    //send stop decoding command to driver
    fd = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_LOGIC_CHAN_DISABLE_SET, NULL) < 0) {
      #ifdef VDEC_MJPG_DEBUG
      CODEC_DEBUG("disable channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }

    VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 0;
    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_MJPG_SetChCfg
    description : set mjpg decoding channel config parament
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        VDEC_MJPG_CFG* stpVdecMJPGCfg: pointer of the decoding parament
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail,set cfg fail for driver
*************************************************************************/
int ISIL_VDEC_MJPG_SetChCfg(CODEC_CHANNEL *pChan, VDEC_MJPG_CFG* stpVdecMJPGCfg)
{
    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVdecMJPGCfg == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){	
	#ifdef VDEC_MJPG_DEBUG
	CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
	#endif
	return CODEC_ERR_NOT_EXIST;
    }

    //todo

    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_MJPG_GetChCfg
    description : get mjpg decoding channel config parament
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        VDEC_MJPG_CFG* stpVdecMJPGCfg: pointer of the decoding parament
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail,get cfg fail for driver
*************************************************************************/
int ISIL_VDEC_MJPG_GetChCfg(CODEC_CHANNEL *pChan, VDEC_MJPG_CFG* stpVdecMJPGCfg)
{
    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVdecMJPGCfg == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){	
	#ifdef VDEC_MJPG_DEBUG
	CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
	#endif
	return CODEC_ERR_NOT_EXIST;
    }
    //todo

    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_MJPG_SentData
    description : send mjpg data to decoder
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

int ISIL_VDEC_MJPG_SendData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *stpStream)
{
    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpStream == NULL){
      #ifdef VDEC_MJPG_DEBUG
      CODEC_DEBUG(" decoding data pointer is null");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_MJPG_DEBUG
	CODEC_DEBUG(" channel not create ");
	#endif
	return CODEC_ERR_NOT_EXIST;
    }

    //todo

    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_MJPG_GetChHandle
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

int ISIL_VDEC_MJPG_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle)
{
    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pHandle == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    *pHandle = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_MJPG_Flush
    description : clear MJPG decoder data
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

int ISIL_VDEC_MJPG_Flush(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_MJPG_DEBUG
	CODEC_DEBUG(" channel not create ");
	#endif
	return CODEC_ERR_NOT_EXIST;
    }

    //todo
    fd = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_CODEC_CHAN_FLUSH, NULL) < 0) {
      #ifdef VDEC_MJPG_DEBUG
      CODEC_DEBUG("flush channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


//set playback strategy
int ISIL_VDEC_MJPG_SetStrategy(CODEC_CHANNEL *pChan, VDEC_CH_STRATEGY *pStrategy)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pStrategy == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_STRATEGY, pStrategy) < 0){
	#ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}

//set playback mode
int ISIL_VDEC_MJPG_SetPBMode(CODEC_CHANNEL *pChan, VDEC_CH_MODE *pMode)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pMode == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_CHANG_MODE, pMode) < 0){
	#ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


//set request-frame command timeout interval
int ISIL_VDEC_MJPG_SetTimeout(CODEC_CHANNEL *pChan, VDEC_CH_TIMEOUT *pTimeout)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pTimeout == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_TIME_CONFIG, pTimeout) < 0){
	#ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VDEC_MJPG_Readydie(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_READY_TO_DIE, NULL) < 0){
	#ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VDEC_MJPG_SetStreamType(CODEC_CHANNEL *pChan, VDEC_CH_HEADER* pHeader)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pHeader == NULL){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_DEC_STREAM_TYPE, pHeader) < 0){
	#ifdef VDEC_MJPG_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}




