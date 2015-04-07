
/************************************************************************
	file name  	: isil_venc_audio.c
	author     	: dengjing
    create date    	: 2010-02-25
    description	: audio audio encoding related controll
    history		: none
*************************************************************************/
#include "isil_venc_audio.h"
#include "isil_codec_debug.h"
#include "isil_private_func.h"
#include "isil_interface.h"
//#include <errno.h>

#define VENC_AUD_DEBUG

typedef struct//save channel info when system running
{
    unsigned char u8IsEnable;//whether the channel is enable or not
    unsigned char u8IsCreate;//whether the channel is create or not
    int  s32ChFd;//channel fd
    unsigned char u8Reserved[2];
    char *mmap_addr;//org addr
}AUD_INFO_TMP;//initial value is zero

AUD_INFO_TMP VENC_AUD_TABLE[CODEC_MAXCHIP][ENC_MAXCH];//save encoding channel info


/******************************************define function***************************************/
/***********************************************************************************************
function: ISIL_VENC_AUD_CreateCh
description:create a audio channel
input:
    CODEC_CHANNEL* pChan: point to channel info struct
output: 
    none
return:
    CODEC_ERR_OK : sucess
    CODEC_ERR_EXIST : channel had create
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/

int ISIL_VENC_AUD_CreateCh(CODEC_CHANNEL* pChan, CODEC_HANDLE* pChipHandle, char* pChDir)
{
    int ch_fd = 0;
    int chip_fd = 0;
    int ret,major,minor = 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;
    char dir[256] = {};

    printf("----create audio------\n");
    if((pChan == NULL) || (pChipHandle == NULL) || (pChDir == NULL)){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
       #endif
       return CODEC_ERR_INVAL;
    }

    if(VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel %d had create", pChan->u32Ch);
       #endif
       return CODEC_ERR_EXIST;
    }

    chip_fd = *pChipHandle;

    data = (struct cmd_arg*)malloc(sizeof(struct cmd_arg) 
                            + sizeof(struct device_info));

    data->type = ISIL_ENCODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;

    //fprintf(stderr, "--aud, data type: %d, ch: %d, algo: %d, stream: %d--\n", 
     //       data->type, data->channel_idx, data->algorithm, data->stream);

    sprintf(dir, "mkdir -p %s/enc", pChDir);
    system(dir);

    dev_info = (struct device_info *)(data->dev_info);

    ret = ioctl(chip_fd, ISIL_CHIP_CREATE_CHAN, data);
    if(ret < 0){
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG(" ioctl fail");
        #endif
        free(data);
        data = NULL;
        return CODEC_ERR_FAIL;
    }

    major = dev_info->major;
    minor = dev_info->minor;

    memset(dir, 0x00, 256);
    sprintf(dir, "%s/enc/%s", pChDir, dev_info->path);
    remove(dir);
        
    if(mknod(dir,  S_IFCHR, ((major<<8) | minor)) < 0){
	#ifdef VENC_AUD_DEBUG
	CODEC_DEBUG(" mknod fail");
	#endif
	ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
	free(data);
        data = NULL;
    }
    
    ch_fd = open(dir, O_RDWR);
    if(ch_fd < 0)
    {
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG(" open chann: %d fail", pChan->u32Ch);
        #endif
		ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
        free(data);
        data = NULL;
        return CODEC_ERR_FAIL;
    }

    VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd = ch_fd;
    VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 1;
    free(data);
    data = NULL;
    return CODEC_ERR_OK;
}

/***********************************************************************************************
function: ISIL_VENC_AUD_ReleaseCh
description:release a audio channel
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    output:none
return:
    CODEC_ERR_OK : sucess
    CODEC_ERR_NOT_EXIST : channel had release
    CODEC_ERR_INVAL : input parament is invalid
    CODEC_ERR_FAIL  : fail
*************************************************************************************************/
int ISIL_VENC_AUD_ReleaseCh(CODEC_CHANNEL* pChan, CODEC_HANDLE* pChipHandle)
{
    int chip_fd = 0;
    int ret= 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;
    char buff[sizeof(struct cmd_arg)
                            + sizeof(struct device_info)];

    if(pChan == NULL){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
       #endif
       return CODEC_ERR_INVAL;
    }

    if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel %d had release", pChan->u32Ch);
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    //todo
    if(VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
       if(ISIL_VENC_AUD_Disable(pChan) < 0)
       {
          #ifdef VENC_H264_DEBUG
          CODEC_DEBUG(" disable audio chann: %d fail", pChan->u32Ch);
          #endif
          return CODEC_ERR_FAIL;
       }
    }

    close(VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd);

    data = (struct cmd_arg*)buff;
#if 0
    data = (struct cmd_arg*)malloc(sizeof(struct cmd_arg) 
                            + sizeof(struct device_info));
#endif

    data->type = ISIL_ENCODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;

	chip_fd = *pChipHandle;

    //dev_info = (struct device_info *)(data->dev_info);

    ret = ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
    if(ret < 0){
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG(" ioctl fail");
	perror("why: ");
        #endif
        //free(data);
        return CODEC_ERR_FAIL;
    }

    VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 0;
    //free(data);
    return CODEC_ERR_OK;
}

/***********************************************************************************************
function: ISIL_VENC_AUD_Enable
description:start audio encoding
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    output:none
return:
    CODEC_ERR_OK : sucess
    CODEC_ERR_NOT_EXIST : channel not create
    CODEC_ERR_EXIST: channel had enable
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/
int ISIL_VENC_AUD_Enable(CODEC_CHANNEL* pChan)//start encoding
{
    int fd = 0;

    printf("----enable audio------\n");

    if(pChan == NULL){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
       #endif
       return CODEC_ERR_INVAL;
    }

    if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel %d not create", pChan->u32Ch);
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    if(VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel %d had enbale", pChan->u32Ch);
       #endif
       return CODEC_ERR_EXIST;
    }
    //todo
    //send command to driver
    fd = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    if(ioctl(fd, ISIL_LOGIC_CHAN_ENABLE_SET, NULL) < 0) {
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG("enable channel fail");
        #endif
        return CODEC_ERR_FAIL;
    }
    VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 1;
    
    CODEC_DEBUG("enable audio");

    return CODEC_ERR_OK;
}

/***********************************************************************************************
function: ISIL_VENC_AUD_Disable
description:stop audio encoding
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    output:none
return:
    CODEC_ERR_OK : sucess
    CODEC_ERR_NOT_EXIST : channel not create
    CODEC_ERR_EXIST: channel had disable
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/
int ISIL_VENC_AUD_Disable(CODEC_CHANNEL* pChan)
{
    int fd = 0;

    if(pChan == NULL){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
       #endif
       return CODEC_ERR_INVAL;
    }

    if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel %d not create", pChan->u32Ch);
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel %d had release", pChan->u32Ch);
       #endif
       return CODEC_ERR_EXIST;
    }
    //todo
    //send command to driver
    fd = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    if(ioctl(fd, ISIL_LOGIC_CHAN_DISABLE_SET, NULL) < 0) {
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG("disable channel fail");
        #endif
        return CODEC_ERR_FAIL;
    }
    VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 0;
    return CODEC_ERR_OK;
}

/***********************************************************************************************
function: ISIL_VENC_AUD_SetChCfg
description:set audio encoding config
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
           VENC_MJPEG_CFG* stpVencAudCfg:point to cfg data
    output:none
return:
    CODEC_ERR_OK : sucess
    CODEC_ERR_INVAL: input parament is invalid
    CODEC_ERR_NOT_EXIST: channel not create
*************************************************************************************************/
int ISIL_VENC_AUD_SetChCfg(CODEC_CHANNEL* pChan, VENC_AUD_CFG* stpVencAudCfg)
{
    int fd = 0;

    if(pChan == NULL){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
       #endif
       return CODEC_ERR_INVAL;
    }

    if(stpVencAudCfg == NULL)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" input parament is NULL");
       #endif
       return CODEC_ERR_INVAL;
    }

    if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    //todo
    //ioctl
    fd = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    fprintf(stderr, "%s audio type %d\n", __FUNCTION__, stpVencAudCfg->e_audio_type);
    if(ioctl(fd, ISIL_CHIP_AUDIO_ENCODE_PARAM_SET, stpVencAudCfg) < 0) {

       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" ioctl fail");
       perror("why: ");
       #endif
       return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}


/***********************************************************************************************
function: ISIL_VENC_AUD_GetChCfg
description:get audio encoding config
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    stpVencAudCfg:point to cfg data
    output:none
return:
    CODEC_ERR_OK : sucess
    CODEC_ERR_INVAL: input parament is invalid
    CODEC_ERR_NOT_EXIST: channel not create
*************************************************************************************************/

int ISIL_VENC_AUD_GetChCfg(CODEC_CHANNEL* pChan, VENC_AUD_CFG* stpVencAudCfg)
{
    int fd = 0;

    if(pChan == NULL){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

	if(pChan->u32Ch >= ENC_MAXCH)
	{
	   #ifdef VENC_AUD_DEBUG
	   CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
	   #endif
	   return CODEC_ERR_INVAL;
	}

    if(stpVencAudCfg == NULL)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" input parament is NULL");
       #endif
       return CODEC_ERR_INVAL;
    }

    if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    //todo
    //ioctl
    fd = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_CHIP_AUDIO_ENCODE_PARAM_GET, stpVencAudCfg) < 0) {

       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" ioctl fail");
       perror("why: ");
       #endif
       return CODEC_ERR_FAIL;
    }

    fprintf(stderr, " aud ch: %d, bw: %d, sr: %d, type: %d\n", 
            stpVencAudCfg->channel,
            stpVencAudCfg->i_bit_wide,
            stpVencAudCfg->i_sample_rate,
            stpVencAudCfg->e_audio_type
            );

    return CODEC_ERR_OK;
}


/***********************************************************************************************
function: ISIL_VENC_AUD_GetData
description:get audio data
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    CODEC_STREAM_DATA *stpStream: point to audio data
	DATA_ACCESS_MODE    eReadway   : read data way, mmap or readwrite
    output:none
return:
    CODEC_ERR_OK : sucess
    CODEC_ERR_NOT_EXIST : channel not create
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/

int ISIL_VENC_AUD_GetData(CODEC_CHANNEL* pChan, CODEC_STREAM_DATA *stpStream, DATA_ACCESS_MODE eReadway, IO_BLK_TYPE eBlk)
{
    int ch_fd = 0;
    char *addr = NULL;
    int ret = 0;
    int flag = 0;

    if(pChan == NULL){
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(stpStream == NULL)
    {
        #ifdef VENC_AUD_DEBUG
        CODEC_DEBUG("input point is NULL");
        #endif
        return CODEC_ERR_INVAL;
	}

	if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    //todo
    ch_fd = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    //blk
    flag = fcntl(ch_fd, F_GETFL, 0);
    if(flag < 0){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("fcntl F_GETFL fail");
        #endif
        return CODEC_ERR_FAIL;
    }

    if(eBlk == IO_TYPE_NOBLK){
        flag |= O_NONBLOCK;
    }else if(eBlk == IO_TYPE_BLK){
        flag &= ~O_NONBLOCK;
    }
    
    if(fcntl(ch_fd, F_SETFL, flag) < 0){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("fcntl F_SETFL fail");
        #endif
        return CODEC_ERR_FAIL;
    }


    if(eReadway == DATA_ACCESS_MODE_MMAP){
	addr = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].mmap_addr;
	ret = ISIL_ENC_GetDataBymmap(&ch_fd, &addr, stpStream);
	VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].mmap_addr = addr;
    }else{
	ret = ISIL_ENC_GetDataByRead(&ch_fd, stpStream);
    }
   
    return ret;
}


/***********************************************************************************************
function: ISIL_VENC_AUD_GetChHandle
description:get audio channel fd
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    CODEC_HANDLE *pHandle: fd
    output:none
return:
    CODEC_ERR_OK : sucess
    CODEC_ERR_NOT_EXIST : channel not create
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/
int ISIL_VENC_AUD_GetChHandle(CODEC_CHANNEL* pChan, CODEC_HANDLE *pHandle)
{
    if(pChan == NULL){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
	{
	   #ifdef VENC_AUD_DEBUG
	   CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
	   #endif
	   return CODEC_ERR_INVAL;
	}

	if(pHandle == NULL)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" input parament is NULL");
       #endif
       return CODEC_ERR_INVAL;
    }


	if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
	{
	   #ifdef VENC_AUD_DEBUG
	   CODEC_DEBUG(" channel not create ");
	   #endif
	   return CODEC_ERR_NOT_EXIST;
	}

	*pHandle = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    return CODEC_ERR_OK;
}

int ISIL_VENC_AUD_DiscardFrame(CODEC_CHANNEL* pChan)
{
    int fd = 0;

    if(pChan == NULL){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

	if(pChan->u32Ch >= ENC_MAXCH)
	{
	   #ifdef VENC_AUD_DEBUG
	   CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
	   #endif
	   return CODEC_ERR_INVAL;
	}

    if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    //todo
    //ioctl
    fd = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_CODEC_CHAN_DISCAED_FRAME, NULL) < 0) {

       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" ioctl fail");
       perror("why: ");
       #endif
       return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VENC_AUD_Flush(CODEC_CHANNEL* pChan)
{
    int fd = 0;

    if(pChan == NULL){
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" point is null");
       #endif
       return CODEC_ERR_INVAL;
    }

	if(pChan->u32Ch >= ENC_MAXCH)
	{
	   #ifdef VENC_AUD_DEBUG
	   CODEC_DEBUG(" channel: %d >= maxch: %d ", pChan->u32Ch, ENC_MAXCH);
	   #endif
	   return CODEC_ERR_INVAL;
	}

    if(!VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    //todo
    //ioctl
    fd = VENC_AUD_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;

    if(ioctl(fd, ISIL_CODEC_CHAN_FLUSH, NULL) < 0) {

       #ifdef VENC_AUD_DEBUG
       CODEC_DEBUG(" ioctl fail");
       perror("why: ");
       #endif
       return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

