/************************************************************************
	file name  	: isil_venc_h264.c
	author     	: dengjing
    create date    	: 2010-02-25
    description	: video h264 encoding related controll
    history		: none
*************************************************************************/
#include "isil_venc_h264.h"
#include "isil_codec_debug.h"
#include "config.h"
#include "isil_private_func.h"
#include "isil_interface.h"
//#include <errno.h>
#include <fcntl.h>
#define VENC_H264_DEBUG


typedef struct//save channel info when system running
{
    unsigned char u8IsEnable;//whether the channel is enable or not
    unsigned char u8IsCreate;//whether the channel is create or not
    unsigned char u8IsMdEnable;//motion detecting enable
    unsigned char u8Reserved;
    int  s32ChFd;//channel fd
    char* mmap_addr;//mmap_addr
}CH_INFO_TMP;//initial value is zero

CH_INFO_TMP VENC_H264_TABLE[CODEC_MAXCHIP][ENC_MAXCH];//save encoding channel info for major stream
CH_INFO_TMP VENC_H264_MINOR_TABLE[CODEC_MAXCHIP][ENC_MAXCH];//save encoding channel info for minor stream

/************************************************************************
    function    : ISIL_VENC_H264_CreateCh
    description : create a h264 encoding channel
    input:
        unsigned char  u8Ch : channel number,0 ~ (max channel -1)
        unsigned char eStreamType: stream type: major or minor,STREAM_TYPE
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_EXIST  :  had create this channel
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  create fail
*************************************************************************/
int ISIL_VENC_H264_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir)
{
    int ch_fd = 0;
    int chip_fd = 0;
    int ret,major,minor = 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;
    char dir[256] = {};

    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        if(VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG(" chan: %d had create,fail", pChan->u32Ch);
            #endif
            return CODEC_ERR_EXIST;
        }
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        if(VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG(" chan: %d had create,fail", pChan->u32Ch);
            #endif
            return CODEC_ERR_EXIST;
        }
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    chip_fd = *pChipHandle;
    data = (struct cmd_arg*)malloc(sizeof(struct cmd_arg)
                            + sizeof(struct device_info));
    data->type = ISIL_ENCODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;
    dev_info = (struct device_info *)(data->dev_info);

    sprintf(dir, "mkdir -p %s/enc", pChDir);
    system(dir);

   	if(pChan->eStreamType == STREAM_TYPE_MAJOR)
	{
      printf("----create h264 main------\n");
      ret = ioctl(chip_fd, ISIL_CHIP_CREATE_CHAN, data);
      if(ret < 0){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" ioctl fail");
        perror("why: ");
        #endif
        free(data);
        data = NULL;
        return CODEC_ERR_FAIL;
      }

      major = dev_info->major;
      minor = dev_info->minor;

      //fprintf(stderr, "---open major: %d, minor: %d-----\n", major, minor);

      //fprintf(stderr, "---dev_info path 11: %s-----\n", dev_info->path);

      memset(dir, 0x00, 256);
      sprintf(dir, "%s/enc/%s", pChDir, dev_info->path);

      //fprintf(stderr, "---dev_info path 22: %s-----\n", dir);
      remove(dir);
      if(mknod(dir,  S_IFCHR, ((major<<8) | minor)) < 0){
        #ifdef VENC_H264_DEBUG
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
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" open chann: %d fail", pChan->u32Ch);
        #endif
        ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
        free(data);
        data = NULL;
        return CODEC_ERR_FAIL;
      }

      VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd = ch_fd;
      VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 1;
      //fprintf(stderr, "----create ch[%d], fd: %d---\n", pChan->u32Ch, ch_fd);
	}
	else if(pChan->eStreamType == STREAM_TYPE_MINOR)
	{
      printf("----create h264 sub------\n");
      ret = ioctl(chip_fd, ISIL_CHIP_CREATE_CHAN, data);
      if(ret < 0){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" ioctl fail");
        perror("why: ");
        #endif
        free(data);
        data = NULL;
        return CODEC_ERR_FAIL;
      }

      major = dev_info->major;
      minor = dev_info->minor;

      //fprintf(stderr, "---open major: %d, minor: %d-----\n", major, minor);

      //fprintf(stderr, "---dev_info path 11: %s-----\n", dev_info->path);

      memset(dir, 0x00, 256);
      sprintf(dir, "%s/%s", pChDir, dev_info->path);

      //fprintf(stderr, "---dev_info path 22: %s-----\n", dir);
      remove(dir);
      if(mknod(dir,  S_IFCHR, ((major<<8) | minor)) < 0){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" mknod fail");
        #endif
        ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
        free(data);
        data = NULL;
        return CODEC_ERR_FAIL;
      }

     //fprintf(stderr, "---open path: %s-----\n", dev_info->path);

      ch_fd = open(dir, O_RDWR);
      if(ch_fd < 0)
      {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" open chann: %d fail", pChan->u32Ch);
        #endif
        ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
        free(data);
        data = NULL;
        return CODEC_ERR_FAIL;
      }

      VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd = ch_fd;
      VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 1;
      //fprintf(stderr, "----create ch[%d], fd: %d---\n", pChan->u32Ch, ch_fd);
	}
	else
	{
       #ifdef VENC_H264_DEBUG
       CODEC_DEBUG("input parament is invalid");
       #endif
       return CODEC_ERR_INVAL;
	}

	free(data);
    data = NULL;
    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VENC_H264_ReleaseCh
    description : release a h264 encoding channel
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        unsigned char eStreamType: stream type: major or minor,STREAM_TYPE
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid,channel is over max channel
        CODEC_ERR_NOT_EXIST  :  this channel had released
        CODEC_ERR_FAIL   :  release fail
*************************************************************************/
int ISIL_VENC_H264_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle)
{
    int chip_fd = 0;
    int ret = 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;

    char buff[sizeof(struct cmd_arg)
                            + sizeof(struct device_info)];
    
    
    
    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChipHandle == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    chip_fd = *pChipHandle;

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }
    


    data = (struct cmd_arg*)buff;



    data->type = ISIL_ENCODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;

	if(pChan->eStreamType == STREAM_TYPE_MAJOR)
	{
      if(!VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
      {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("chann:%d had release,fail", pChan->u32Ch);
        #endif
        //free(data);
        return CODEC_ERR_NOT_EXIST;
      }

      
      if(VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
      {
      
        if(ISIL_VENC_H264_Disable(pChan) < 0)
        {
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG(" disable chann: %d fail", pChan->u32Ch);
            #endif
            //free(data);
            return CODEC_ERR_FAIL;
        }
       
      }

     

      close(VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd);
      

      ret = ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
      if(ret < 0){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" ioctl fail");
        perror("why: ");
        #endif
        //free(data);
        return CODEC_ERR_FAIL;
      }
      VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 0;
      
   }
   else if(pChan->eStreamType == STREAM_TYPE_MINOR)
   {
      if(!VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
      {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("chann:%d had release,fail", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
      }

      if(VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
      {
        if(ISIL_VENC_H264_Disable(pChan) < 0)
        {
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG(" disable chann: %d fail", pChan->u32Ch);
            #endif
            return CODEC_ERR_FAIL;
        }
      }
      
     
      close(VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd);
     

      ret = ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
      if(ret < 0){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" ioctl fail");
        perror("why: ");
        #endif
        //free(data);
        return CODEC_ERR_FAIL;
      }
      VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 0;
     
   }
   else
   {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      //free(data);
      return CODEC_ERR_INVAL;
   }

   //free(data);
   return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VENC_H264_Enable
    description : enable a channel,start encoding
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        unsigned char eStreamType: stream type: major or minor,STREAM_TYPE
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_NOT_EXIST: channel not create
        CODEC_ERR_FAIL   :  fail
*************************************************************************/
int ISIL_VENC_H264_Enable(CODEC_CHANNEL *pChan)//start encoding
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

	if(pChan->eStreamType == STREAM_TYPE_MAJOR)
	{
          printf("----enable h264 main------\n");

          if(!VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
          {
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
            #endif
            return CODEC_ERR_NOT_EXIST;
          }

          #if 0
          if(VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
          {
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG("chann:%d had enable", pChan->u32Ch);
            #endif
            return CODEC_ERR_EXIST;
          }
          #endif

          /*how to do, now unknown*/
          //send start encoding command to driver
          fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
          if(ioctl(fd, ISIL_LOGIC_CHAN_ENABLE_SET, NULL) < 0) {
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG("enable channel fail");
            #endif
            return CODEC_ERR_FAIL;
          }
          VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 1;
      }
      else if(pChan->eStreamType == STREAM_TYPE_MINOR)
      {
          printf("----enable h264 sub------\n");
          if(!VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
          {
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG("chann:%d not create", pChan->u32Ch);
            #endif
            return CODEC_ERR_NOT_EXIST;
          }
          
          #if 0
          if(VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
          {
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG("chann:%d had enable", pChan->u32Ch);
            #endif
            return CODEC_ERR_EXIST;
          }
          #endif

          /*how to do, now unknown*/
          //send start encoding command to driver
          fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
          if(ioctl(fd, ISIL_LOGIC_CHAN_ENABLE_SET, NULL) < 0) {
            #ifdef VENC_H264_DEBUG
            CODEC_DEBUG("enable channel fail");
            #endif
            return CODEC_ERR_FAIL;
          }
          VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 1;
    }
    else
    {
          #ifdef VENC_H264_DEBUG
          CODEC_DEBUG("input parament is invalid");
          #endif
          return CODEC_ERR_INVAL;
    }

    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VENC_H264_Disable
    description : disable a channel,stop encoding
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        unsigned char eStreamType: stream type: major or minor,STREAM_TYPE
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_NOT_EXIST  :  channel had disable
        CODEC_ERR_FAIL   :  fail
*************************************************************************/

int ISIL_VENC_H264_Disable(CODEC_CHANNEL *pChan)
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR)
    {
      #if 0
      if(!VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
      {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("chann:%d had disable", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
      }
      #endif
      /*how to do, now unknown*/
      //send stop encoding command to driver
      fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
      if(ioctl(fd, ISIL_LOGIC_CHAN_DISABLE_SET, NULL) < 0) {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("disable channel fail");
        #endif
        return CODEC_ERR_FAIL;
      }
      VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 0;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR)
    {
#if 0
      if(!VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
      {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("chann:%d had disable", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
      }
#endif
      /*how to do, now unknown*/
      //send stop encoding command to driver
      fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
      if(ioctl(fd, ISIL_LOGIC_CHAN_DISABLE_SET, NULL) < 0) {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("disable channel fail");
        #endif
        return CODEC_ERR_FAIL;
      }
      VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 0;
    }else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VENC_H264_SetChCfg
    description : set h264 encoding channel config parament
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        unsigned char eStreamType: stream type: major or minor,STREAM_TYPE
        VENC_H264_CFG* stpVench264Cfg: pointer of the encoding parament
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail,set cfg fail for driver
*************************************************************************/
int ISIL_VENC_H264_SetChCfg(CODEC_CHANNEL *pChan, VENC_H264_CFG* stpVench264Cfg)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVench264Cfg == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_CODEC_SET_VIDEO_ENCODER_PARAM, stpVench264Cfg) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

/************************************************************************
    function    : ISIL_VENC_H264_GetChCfg
    description : get h264 encoding channel config parament
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        unsigned char eStreamType: stream type: major or minor,STREAM_TYPE
        VENC_CHANN_CFG *stpVencCfg: pointer of the encoding parament
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail,get cfg fail for driver
*************************************************************************/
int ISIL_VENC_H264_GetChCfg(CODEC_CHANNEL *pChan, VENC_H264_CFG* stpVench264Cfg)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVench264Cfg == NULL)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo

    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR)
    {
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else if(pChan->eStreamType == STREAM_TYPE_MINOR)
    {
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else
    {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_CODEC_GET_VIDEO_ENCODER_PARAM, stpVench264Cfg) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    fprintf(stderr, "264 bps: %d, fps: %d \n",
            stpVench264Cfg->i_bps,
            stpVench264Cfg->i_fps
            );

    return CODEC_ERR_OK;
}


int ISIL_VENC_H264_SetChFeature(CODEC_CHANNEL *pChan, VENC_H264_FEATURE* stpVench264Feature)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVench264Feature == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

	CODEC_DEBUG(" set ioctl ");
    if(ioctl(fd, ISIL_H264_ENCODE_FEATURE_SET, stpVench264Feature) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VENC_H264_GetChFeature(CODEC_CHANNEL *pChan, VENC_H264_FEATURE* stpVench264Feature)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVench264Feature == NULL)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR)
    {
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else if(pChan->eStreamType == STREAM_TYPE_MINOR)
    {
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else
    {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_H264_ENCODE_FEATURE_GET, stpVench264Feature) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    fprintf(stderr, "get feature,de:%d,skip:%d",
            stpVench264Feature->b_enable_deinterlace,
            stpVench264Feature->b_enable_skip
            );

    return CODEC_ERR_OK;
}


int ISIL_VENC_H264_SetChRC(CODEC_CHANNEL *pChan, VENC_H264_RC* stpVencH264RC)
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVencH264RC == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_H264_ENCODE_RC_SET, stpVencH264RC) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VENC_H264_GetChRC(CODEC_CHANNEL *pChan, VENC_H264_RC* stpVencH264RC)
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVencH264RC == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_H264_ENCODE_RC_GET, stpVencH264RC) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VENC_H264_SetChRCParam(CODEC_CHANNEL *pChan, VENC_H264_RC_PARAM* stpVencH264RCParam)
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVencH264RCParam == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_H264_ENCODE_RC_PARAM_SET, stpVencH264RCParam) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }
    DEBUG_FUNCTION();
    return CODEC_ERR_OK;
}

int ISIL_VENC_H264_GetChRCParam(CODEC_CHANNEL *pChan, VENC_H264_RC_PARAM* stpVencH264RCParam)
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVencH264RCParam == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_H264_ENCODE_RC_PARAM_GET, stpVencH264RCParam) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VENC_H264_GetChMapInfo(CODEC_CHANNEL *pChan, VENC_H264_CHAN_MAP_INFO* stpVencH264Map)
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVencH264Map == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_H264_ENCODE_CHAN_MAP_GET, stpVencH264Map) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    fprintf(stderr, " logic ch: %d, phy ch: %d\n",
            stpVencH264Map->i_phy_chan_id,
            stpVencH264Map->i_logic_chan_id);

    return CODEC_ERR_OK;
}


/************************************************************************
    function    : ISIL_VENC_H264_GetData
    description : get all h264 encoding data,include h264 data and md data
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        unsigned char eStreamType: stream type: major or minor,STREAM_TYPE
        CODEC_STREAM_DATA *stpStream: point to  got data struct
        DATA_ACCESS_MODE    eReadway   : read data way, mmap or readwrite
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_FAIL   :  fail
*************************************************************************/

int ISIL_VENC_H264_GetData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *stpStream, DATA_ACCESS_MODE eReadway, IO_BLK_TYPE eBlk)
{
    int ch_fd = 0;
    char* addr = 0;
    int ret = 0;
    int flag = 0;

    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
	}

    if(stpStream == NULL)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input point is NULL");
        #endif
        return CODEC_ERR_INVAL;
	}

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
    	ch_fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
        if(eReadway == DATA_ACCESS_MODE_MMAP){
            addr = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].mmap_addr;
        }
        //fprintf(stderr, "-------h264 major ch: %d-----\n", pChan->u32Ch);
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        ch_fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
        if(eReadway == DATA_ACCESS_MODE_MMAP){
            addr = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].mmap_addr;
        }
        //fprintf(stderr, "-------h264 minor ch: %d-----\n", pChan->u32Ch);
	}else{
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input parament is invalid");
        #endif
        return CODEC_ERR_INVAL;
	}

    //fprintf(stderr, "--------str: %d, ch: %d-----\n", pChan->eStreamType, pChan->u32Ch);
    //fprintf(stderr, "--------before addr: %p-----\n", addr);


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
        ret = ISIL_ENC_GetDataBymmap(&ch_fd, &addr, stpStream);
    }else{//directly read or write data from ch_fd
        ret = ISIL_ENC_GetDataByRead(&ch_fd, stpStream);
    }

    //fprintf(stderr, "--------return addr: %p-----\n", addr);

    if(eReadway == DATA_ACCESS_MODE_MMAP){
        if(pChan->eStreamType == STREAM_TYPE_MAJOR){
            VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].mmap_addr = addr;
        }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
            VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].mmap_addr = addr;
    	}
	}

    return ret;
}


/************************************************************************
    function    : ISIL_VENC_H264_GetChHandle
    description : get channel handle(fd)
    input:
        unsigned char  u8Ch : channel number,0 ~ max channel
        unsigned char eStreamType: stream type: major or minor,STREAM_TYPE
        CODEC_HANDLE *pHandle: point to fd
    output:
        none
    return:
        CODEC_ERR_OK     :  it's sucess
        CODEC_ERR_INVAL  :  input parament is invalid
        CODEC_ERR_NOT_EXIST:channel not create
        CODEC_ERR_FAIL   :  fail
*************************************************************************/

int ISIL_VENC_H264_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle)
{
     if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pHandle == NULL)
    {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR)
    {
      if(!VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
      {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
      }

      *pHandle = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
	else if(pChan->eStreamType == STREAM_TYPE_MINOR)
    {
      if(!VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
      {
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("chann:%d not exist", pChan->u32Ch);
        #endif
        return CODEC_ERR_NOT_EXIST;
      }

      *pHandle = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
	else
	{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
	}

    return CODEC_ERR_OK;
}

int ISIL_VENC_H264_DiscardFrame(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }
    else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_CODEC_CHAN_DISCAED_FRAME, NULL) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


int ISIL_VENC_H264_SetOSDCfg(CODEC_CHANNEL *pChan, ISIL_OSD_CFG* pOSDCfg)
{
    int fd = 0;

    if(!pChan || !pOSDCfg){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_OSD_SET_PARAM, pOSDCfg) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}


int ISIL_VENC_H264_GetOSDCfg(CODEC_CHANNEL *pChan, ISIL_OSD_CFG* pOSDCfg)
{
    int fd = 0;

    if(!pChan || !pOSDCfg){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_OSD_GET_PARAM, pOSDCfg) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VENC_H264_Flush(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(!pChan){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_H264_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    //todo
    unsigned char IsCreate = 0;
    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        IsCreate = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        IsCreate = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate;
    }

    if(!IsCreate){//not create
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("channel not creat\n");
      #endif
      return CODEC_ERR_NOT_EXIST;
    }

    if(pChan->eStreamType == STREAM_TYPE_MAJOR){
        fd = VENC_H264_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else if(pChan->eStreamType == STREAM_TYPE_MINOR){
        fd = VENC_H264_MINOR_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    }else{
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("input parament is invalid");
      #endif
      return CODEC_ERR_INVAL;
    }

    if(ioctl(fd, ISIL_CODEC_CHAN_FLUSH, NULL) < 0) {
      #ifdef VENC_H264_DEBUG
      CODEC_DEBUG("ioctl fail");
      perror("why: ");
      #endif
      return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}
