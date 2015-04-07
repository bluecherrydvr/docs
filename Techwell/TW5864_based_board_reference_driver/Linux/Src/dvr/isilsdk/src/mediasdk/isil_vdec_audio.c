/************************************************************************
	file name  	: isil_vdec_audio.c
	author     	: dengjing
    create date : 2010-03-03
    description	: audio decoding related controll
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

#include "isil_vdec_audio.h"
#include "isil_codec_debug.h"
#include "isil_codec_errcode.h"
#include "isil_interface.h"

#define VDEC_AUD_DEBUG

typedef struct//save channel info when system running
{
    unsigned char u8IsEnable;//whether the channel is enable or not
    unsigned char u8IsCreate;//whether the channel is create or not
    int  s32ChFd;//channel fd
    unsigned char u8Reserved[2];
}VDEC_AUD_INFO;//initial value is zero

VDEC_AUD_INFO VDEC_AUD_TABLE[DEC_MAXCHIP][DEC_MAXCH];//save decoding channel info

/************************************************************************
    function    : ISIL_VDEC_AUD_CreateCh
    description : create a audio decoding channel
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
int ISIL_VDEC_AUD_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir)
{
    int ch_fd = 0;
    int chip_fd = 0;
    int ret,major,minor = 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;
    char dir[256] = {};

    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
		#ifdef VDEC_AUD_DEBUG
		CODEC_DEBUG(" chan: %d had create,fail", pChan->u32Ch);
		#endif
		return CODEC_ERR_EXIST;
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
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
        #ifdef VDEC_AUD_DEBUG
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
	#ifdef VDEC_AUD_DEBUG
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
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" open chann: %d fail", pChan->u32Ch);
        #endif
        ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
        free(data);
	data = NULL;
        return CODEC_ERR_FAIL;
    }
    
    VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd = ch_fd;
    VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 1;

    free(data);
    data = NULL;
    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_AUD_ReleaseData
    description : release a audio decoding channel
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
int ISIL_VDEC_AUD_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle)
{
    int chip_fd = 0;
    int ret= 0;
    struct cmd_arg *data = NULL;
    //struct device_info* dev_info = NULL;

    if(!pChan || !pChipHandle){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH)
    {
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {	
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d had release,fail", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }
    
    if(VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
        if(ISIL_VDEC_AUD_Disable(pChan) < 0)
        {
            #ifdef VDEC_AUD_DEBUG
            CODEC_DEBUG(" disable chann: %d fail", pChan->u32Ch);
            #endif
            return CODEC_ERR_FAIL;
        }
    }
    
    fprintf(stderr, "---close audio: %x--\n", pChan->u32Ch);
    close(VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd);

    fprintf(stderr, "---close audio: %x ok--\n", pChan->u32Ch);

    data = (struct cmd_arg*)malloc(sizeof(struct cmd_arg) 
                            + sizeof(struct device_info));

    data->type = ISIL_DECODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;

    chip_fd = *pChipHandle;
    
    fprintf(stderr, "---release audio: %x--\n", pChan->u32Ch);

    ret = ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
    if(ret < 0){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" ioctl fail");
        #endif
        free(data);
	data = NULL;
        return CODEC_ERR_FAIL;
    }

    fprintf(stderr, "---release audio: %x ok--\n", pChan->u32Ch);

    VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 0;
    free(data);
    data = NULL;
    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_AUD_Enable
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
int ISIL_VDEC_AUD_Enable(CODEC_CHANNEL *pChan)//start decoding
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){	
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    
    if(VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable){	
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d had enable", pChan->u32Ch);
        #endif
        return CODEC_ERR_EXIST;
    }

    //send start decoding command to driver
    fd = VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //fprintf(stderr, "---set audio dec enable: %x--\n", fd);

    if(ioctl(fd, ISIL_LOGIC_CHAN_ENABLE_SET, NULL) < 0) {
      #ifdef VDEC_AUD_DEBUG
      CODEC_DEBUG("enable channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }
    
    VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 1;
    //fprintf(stderr, "---set audio dec enable ok--\n");
    return CODEC_ERR_OK;}

/************************************************************************
    function    : ISIL_VDEC_AUD_Disable
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

int ISIL_VDEC_AUD_Disable(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){	
		#ifdef VDEC_AUD_DEBUG
		CODEC_DEBUG("chann:%d had disable", pChan->u32Ch);
		#endif
		return CODEC_ERR_NOT_EXIST;
    }
	
    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable){	
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d had disable", pChan->u32Ch);
        #endif
        return CODEC_ERR_EXIST;
    }
 
    //send stop decoding command to driver
    fd = VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    fprintf(stderr, "---disable audio: %x--\n", pChan->u32Ch);

    if(ioctl(fd, ISIL_LOGIC_CHAN_DISABLE_SET, NULL) < 0) {
      #ifdef VDEC_AUD_DEBUG
      CODEC_DEBUG("disable channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }

    fprintf(stderr, "---disable audio: %x ok--\n", pChan->u32Ch);

    VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 0;
    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_AUD_SetChCfg
    description : set audio decoding channel config parament
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        VDEC_AUD_CFG* stpVdecAudCfg: pointer of the decoding parament
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail,set cfg fail for driver
*************************************************************************/
int ISIL_VDEC_AUD_SetChCfg(CODEC_CHANNEL *pChan, VDEC_AUD_CFG* stpVdecAudCfg)
{
    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVdecAudCfg == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){	
		#ifdef VDEC_AUD_DEBUG
		CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
		#endif
		return CODEC_ERR_NOT_EXIST;
    }

    //todo

    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VDEC_AUD_GetChCfg
    description : get audio decoding channel config parament
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        VDEC_AUD_CFG* stpVdecAudCfg: pointer of the decoding parament
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail,get cfg fail for driver
*************************************************************************/
int ISIL_VDEC_AUD_GetChCfg(CODEC_CHANNEL *pChan, VDEC_AUD_CFG* stpVdecAudCfg)
{
    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVdecAudCfg == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

	if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){	
		#ifdef VDEC_AUD_DEBUG
		CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
		#endif
		return CODEC_ERR_NOT_EXIST;
    }
    //todo

    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_AUD_SentData
    description : send audio data to decoder
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

int ISIL_VDEC_AUD_SendData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *stpStream)
{
    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpStream == NULL){
      #ifdef VDEC_AUD_DEBUG
      CODEC_DEBUG(" decoding data pointer is null");
      #endif
      return CODEC_ERR_INVAL;
    }

	if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
		#ifdef VDEC_AUD_DEBUG
		CODEC_DEBUG(" channel not create ");
		#endif
		return CODEC_ERR_NOT_EXIST;
    }

    //todo

    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_AUD_GetChHandle
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

int ISIL_VDEC_AUD_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle)
{
    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pHandle == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    *pHandle = VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VDEC_AUD_Flush
    description : clear AUD decoder data
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

int ISIL_VDEC_AUD_Flush(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
	#ifdef VDEC_AUD_DEBUG
	CODEC_DEBUG(" channel not create ");
	#endif
	return CODEC_ERR_NOT_EXIST;
    }

    //todo
    fd = VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_CODEC_CHAN_FLUSH, NULL) < 0) {
      #ifdef VDEC_AUD_DEBUG
      CODEC_DEBUG("flush channel fail");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

//set playback strategy
int ISIL_VDEC_AUD_SetStrategy(CODEC_CHANNEL *pChan, VDEC_CH_STRATEGY *pStrategy)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pStrategy == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_STRATEGY, pStrategy) < 0){
	#ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}

//set playback mode
int ISIL_VDEC_AUD_SetPBMode(CODEC_CHANNEL *pChan, VDEC_CH_MODE *pMode)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pMode == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_CHANG_MODE, pMode) < 0){
	#ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}



//set request-frame command timeout interval
int ISIL_VDEC_AUD_SetTimeout(CODEC_CHANNEL *pChan, VDEC_CH_TIMEOUT *pTimeout)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pTimeout == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_TIME_CONFIG, pTimeout) < 0){
	#ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


int ISIL_VDEC_AUD_Readydie(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;       
    }

    if(pChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_READY_TO_DIE, NULL) < 0){
	#ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;

}

int ISIL_VDEC_AUD_SetStreamType(CODEC_CHANNEL *pDecChan, VDEC_CH_HEADER* pHeader)
{
    int fd = 0;

    if(pDecChan == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pDecChan->u32Ch >= DEC_MAXCH){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pDecChan->u32Ch, DEC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pHeader == NULL){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VDEC_AUD_TABLE[pDecChan->u32ChipID][pDecChan->u32Ch].u8IsCreate){
        #ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("chann:%d not exist", pDecChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
    }

    fd = VDEC_AUD_TABLE[pDecChan->u32ChipID][pDecChan->u32Ch].s32ChFd;

    //send data to driver
    if(ioctl(fd, ISIL_CODEC_CHAN_SET_DEC_STREAM_TYPE, pHeader) < 0){
	#ifdef VDEC_AUD_DEBUG
        CODEC_DEBUG("ioctl fail\n");
        #endif
        return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}



