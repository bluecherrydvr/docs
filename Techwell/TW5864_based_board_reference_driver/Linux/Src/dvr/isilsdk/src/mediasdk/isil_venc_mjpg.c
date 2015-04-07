
/************************************************************************
	file name  	: isil_venc_mjpg.c
	author     	: dengjing
    create date    	: 2010-02-25
    description	: video mjpg encoding related controll
    history		: none	
*************************************************************************/
#include "isil_venc_mjpg.h"
#include "isil_codec_debug.h"
#include "isil_private_func.h"
#include "isil_interface.h"
//#include <errno.h>

#define VENC_MJPG_DEBUG

typedef struct//save channel info when system running
{
    unsigned char u8IsEnable;//whether the channel is enable or not
    unsigned char u8IsCreate;//whether the channel is create or not
    unsigned char u8Reserved[2];
    int  s32ChFd;//channel fd
    char *mmap_addr;//org addr
}MJPG_INFO_TMP;//initial value is zero

MJPG_INFO_TMP VENC_MJPG_TABLE[CODEC_MAXCHIP][ENC_MAXCH];//save encoding channel info


/******************************************define function***************************************/
/***********************************************************************************************
function: ISIL_VENC_MJPG_CreateCh
description:create a mjpg channel 
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    output:none
return:
    CODEC_ERR_OK : sucess 
    CODEC_ERR_EXIST : channel had create
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/

int ISIL_VENC_MJPG_CreateCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle, char* pChDir)
{
    int ch_fd = 0;
    int chip_fd = 0;
    int ret,major,minor = 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;
    char dir[256] = {};

    printf("----create mjpg------\n");
    
    if((pChan == NULL) || (pChipHandle == NULL) || (pChDir == NULL)){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
       #ifdef VENC_MJPG_DEBUG
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

    sprintf(dir, "mkdir -p %s/enc", pChDir);
    system(dir);

    dev_info = (struct device_info *)(data->dev_info);

    ret = ioctl(chip_fd, ISIL_CHIP_CREATE_CHAN, data);
    if(ret < 0){
        #ifdef VENC_MJPG_DEBUG
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
    sprintf(dir, "%s/enc/%s", pChDir, dev_info->path);
	remove(dir);

    if(mknod(dir,  S_IFCHR, ((major<<8) | minor)) < 0){
		#ifdef VENC_MJPG_DEBUG
		CODEC_DEBUG(" mknod fail");
		#endif
		ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
		free(data);
		data = NULL;
		return CODEC_ERR_FAIL;
    }
    
    ch_fd = open(dir, O_RDWR);
    if(ch_fd < 0){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" open chann: %d fail", pChan->u32Ch);
        #endif
		ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
        free(data);
	data = NULL;
        return CODEC_ERR_FAIL;
    }

    VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd = ch_fd;
    VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 1;
    free(data);
    return CODEC_ERR_OK;  
}

/***********************************************************************************************
function: ISIL_VENC_MJPG_ReleaseCh
description:release a mjpg channel 
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    output:none
return:
    CODEC_ERR_OK : sucess 
    CODEC_ERR_NOT_EXIST : channel had release
    CODEC_ERR_INVAL : input parament is invalid
    CODEC_ERR_FAIL  : fail
*************************************************************************************************/
int ISIL_VENC_MJPG_ReleaseCh(CODEC_CHANNEL *pChan, CODEC_HANDLE* pChipHandle)
{
    int chip_fd = 0;
    int ret = 0;
    struct cmd_arg *data = NULL;
    struct device_info* dev_info = NULL;

    char buff[sizeof(struct cmd_arg)
                            + sizeof(struct device_info)];

    if((pChan == NULL) || (pChipHandle == NULL)){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel %d had release", pChan->u32Ch);
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    chip_fd = *pChipHandle;

    //todo
    
    if(VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {     
       if(ISIL_VENC_MJPG_Disable(pChan) < 0)
       {
          #ifdef VENC_H264_DEBUG
          CODEC_DEBUG(" disable mjpg chann: %d fail", pChan->u32Ch);
          #endif
          return CODEC_ERR_FAIL;
       }
    }

    close(VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd); 
    
#if 0
    data = (struct cmd_arg*)malloc(sizeof(struct cmd_arg) 
                            + sizeof(struct device_info));
#endif
    
    data = (struct cmd_arg*)buff;

    data->type = ISIL_ENCODER;
    data->channel_idx = pChan->u32Ch;
    data->algorithm = pChan->eCodecType;
    data->stream = pChan->eStreamType;

    ret = ioctl(chip_fd, ISIL_CHIP_RELEASE_CHAN, data);
    if(ret < 0){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" ioctl fail");
	perror("why: ");
        #endif
        //free(data);
        return CODEC_ERR_FAIL;
    }

    VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate = 0;
    //free(data);
    return CODEC_ERR_OK;
}

/***********************************************************************************************
function: ISIL_VENC_MJPG_Enable
description:start mjpg encoding
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    output:none
return:
    CODEC_ERR_OK : sucess 
    CODEC_ERR_NOT_EXIST : channel not create
    CODEC_ERR_EXIST: channel had enable
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/
int ISIL_VENC_MJPG_Enable(CODEC_CHANNEL *pChan)//start encoding
{
    int fd = 0;

    printf("----enable mjpg------\n");

    if(pChan == NULL){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel %d not create", pChan->u32Ch);
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    if(VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel %d had enbale", pChan->u32Ch);
       #endif
       return CODEC_ERR_OK;
       //return CODEC_ERR_EXIST;
    }
    //todo
    //send command to driver
    fd = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    if(ioctl(fd, ISIL_LOGIC_CHAN_ENABLE_SET, NULL) < 0) {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("enable channel fail");
        #endif
        return CODEC_ERR_FAIL;
    }
    VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 1;
    return CODEC_ERR_OK;
}

/***********************************************************************************************
function: ISIL_VENC_MJPG_Disable
description:stop mjpg encoding
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    output:none
return:
    CODEC_ERR_OK : sucess 
    CODEC_ERR_NOT_EXIST : channel not create
    CODEC_ERR_EXIST: channel had disable
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/
int ISIL_VENC_MJPG_Disable(CODEC_CHANNEL *pChan)
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel %d not create", pChan->u32Ch);
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel %d had release", pChan->u32Ch);
       #endif
       return CODEC_ERR_EXIST;
    }
    //todo
    //send command to driver
    fd = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    if(ioctl(fd, ISIL_LOGIC_CHAN_DISABLE_SET, NULL) < 0) {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("disable channel fail");
        #endif
        return CODEC_ERR_FAIL;
    }
    VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsEnable = 0;
    return CODEC_ERR_OK;
}

/***********************************************************************************************
function: ISIL_VENC_MJPG_SetChCfg
description:set mjpg encoding config
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
           VENC_MJPEG_CFG* stpVencMjpgCfg:point to cfg data
    output:none
return:
    CODEC_ERR_OK : sucess 
    CODEC_ERR_INVAL: input parament is invalid
    CODEC_ERR_NOT_EXIST: channel not create
*************************************************************************************************/
int ISIL_VENC_MJPG_SetChCfg(CODEC_CHANNEL *pChan, VENC_MJPG_CFG* stpVencMjpgCfg)
{
    int fd = 0;
    if(pChan == NULL){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpVencMjpgCfg == NULL)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" input parament is NULL");
       #endif
       return CODEC_ERR_INVAL;
    } 

    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    } 

    //todo
    //ioctl,send cfg to driver, now unknown

    fd = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    if(ioctl(fd, ISIL_MJPEG_ENCODE_PARAM_SET, stpVencMjpgCfg) < 0) {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" ioctl fail");
       perror("why: ");
       #endif
       return CODEC_ERR_FAIL;
    }
    return CODEC_ERR_OK;
}


/***********************************************************************************************
function: ISIL_VENC_MJPG_GetChCfg
description:get mjpg encoding config
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    stpVencMjpgCfg:point to cfg data
    output:none
return:
    CODEC_ERR_OK : sucess 
    CODEC_ERR_INVAL: input parament is invalid
    CODEC_ERR_NOT_EXIST: channel not create
*************************************************************************************************/

int ISIL_VENC_MJPG_GetChCfg(CODEC_CHANNEL *pChan, VENC_MJPG_CFG* stpVencMjpgCfg)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }
	
    if(stpVencMjpgCfg == NULL)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" input parament is NULL");
       #endif
       return CODEC_ERR_INVAL;
    } 

    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    } 

    //todo
    //ioctl,send cfg to driver, now unknown
    fd = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    if(ioctl(fd, ISIL_MJPEG_ENCODE_PARAM_GET, stpVencMjpgCfg) < 0) {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" ioctl fail");
       perror("why: ");
       #endif
       return CODEC_ERR_FAIL;
    }

    fprintf(stderr, "mjpg level:%d,num:%d,str:%d,type:%d\n",
            stpVencMjpgCfg->e_image_level,
            stpVencMjpgCfg->i_capture_frame_number,
            stpVencMjpgCfg->i_capture_frame_stride,
            stpVencMjpgCfg->i_capture_type
            );

    return CODEC_ERR_OK;
}


/***********************************************************************************************
function: ISIL_VENC_MJPG_GetData
description:get mjpg data
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    CODEC_STREAM_DATA *stpStream: point to mjpg data
    DATA_ACCESS_MODE    eReadway   : read data way, mmap or readwrite
    output:none
return:
    CODEC_ERR_OK : sucess 
    CODEC_ERR_NOT_EXIST : channel not create
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/

int ISIL_VENC_MJPG_GetData(CODEC_CHANNEL *pChan, CODEC_STREAM_DATA *stpStream, DATA_ACCESS_MODE eReadway, IO_BLK_TYPE eBlk)
{
    int ch_fd = 0;
    char *addr = NULL;
    int ret = 0;
    int flag = 0;

    if(pChan == NULL){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(stpStream == NULL)
    {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input point is NULL");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    } 

    //todo
    ch_fd = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    //fprintf(stderr, "-------mjpg ch: %d-----\n", pChan->u32Ch);

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
	addr = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].mmap_addr;
	ret = ISIL_ENC_GetDataBymmap(&ch_fd, &addr, stpStream);
	VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].mmap_addr = addr;
    }else{
	ret = ISIL_ENC_GetDataByRead(&ch_fd, stpStream);
    }
   
    return ret;
}


/***********************************************************************************************
function: ISIL_VENC_MJPG_GetChHandle
description:get mjpg channel fd
paraments:
    input: unsigned char u8Ch : channel ID, form 0~(ENC_MAXCH-1)
    CODEC_HANDLE *pHandle: fd
    output:none
return:
    CODEC_ERR_OK : sucess 
    CODEC_ERR_NOT_EXIST : channel not create
    CODEC_ERR_INVAL : input parament is invalid
*************************************************************************************************/
int ISIL_VENC_MJPG_GetChHandle(CODEC_CHANNEL *pChan, CODEC_HANDLE *pHandle)
{
    if(pChan == NULL){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH)
    {
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }
	
	if(pHandle == NULL)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" input parament is NULL");
       #endif
       return CODEC_ERR_INVAL;
    } 

	
	if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate)
	{
	   #ifdef VENC_MJPG_DEBUG
	   CODEC_DEBUG(" channel not create ");
	   #endif
	   return CODEC_ERR_NOT_EXIST;
	} 
	
	*pHandle = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    return CODEC_ERR_OK;
}

int ISIL_VENC_MJPG_StartCap(CODEC_CHANNEL *pEncChan, unsigned int u32Type)
{
    int s32Ret = 0;
	VENC_MJPG_CFG mjpg_cfg;

    if(pEncChan == NULL)
    {
      #ifdef VENC_MJPG_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

	if(!VENC_MJPG_TABLE[pEncChan->u32ChipID][pEncChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    } 

	memset(&mjpg_cfg, 0x00, sizeof(VENC_MJPG_CFG));

	s32Ret = ISIL_VENC_MJPG_GetChCfg(pEncChan, &mjpg_cfg);
	if(s32Ret < 0) {
	  #ifdef VENC_MJPG_DEBUG
      CODEC_DEBUG(" get mjpg cfg fail\n");
      #endif
      return CODEC_ERR_FAIL;
	}

	mjpg_cfg.i_capture_type |= u32Type;
	mjpg_cfg.change_mask_flag = ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_TYPE;

	s32Ret = ISIL_VENC_MJPG_SetChCfg(pEncChan, &mjpg_cfg);
	if(s32Ret < 0) {
	  #ifdef VENC_MJPG_DEBUG
      CODEC_DEBUG(" set mjpg cfg fail\n");
      #endif
      return CODEC_ERR_FAIL;
	}

	if(!VENC_MJPG_TABLE[pEncChan->u32ChipID][pEncChan->u32Ch].u8IsEnable){
		s32Ret = ISIL_VENC_MJPG_Enable(pEncChan);
	}

    return s32Ret;
}


int ISIL_VENC_MJPG_StopCap(CODEC_CHANNEL *pEncChan, unsigned int u32Type)
{
    int s32Ret = 0;
	VENC_MJPG_CFG mjpg_cfg;

    if(pEncChan == NULL)
    {
      #ifdef VENC_MJPG_DEBUG
      CODEC_DEBUG(" point is null");
      #endif
      return CODEC_ERR_INVAL;
    }

	if(!VENC_MJPG_TABLE[pEncChan->u32ChipID][pEncChan->u32Ch].u8IsCreate)
    {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    } 

	memset(&mjpg_cfg, 0x00, sizeof(VENC_MJPG_CFG));

	s32Ret = ISIL_VENC_MJPG_GetChCfg(pEncChan, &mjpg_cfg);
	if(s32Ret < 0) {
	  #ifdef VENC_MJPG_DEBUG
      CODEC_DEBUG(" get mjpg cfg fail\n");
      #endif
      return CODEC_ERR_FAIL;
	}

	mjpg_cfg.i_capture_type |= u32Type;
	mjpg_cfg.change_mask_flag = ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_TYPE;

	s32Ret = ISIL_VENC_MJPG_SetChCfg(pEncChan, &mjpg_cfg);
	if(s32Ret < 0) {
	  #ifdef VENC_MJPG_DEBUG
      CODEC_DEBUG(" set mjpg cfg fail\n");
      #endif
      return CODEC_ERR_FAIL;
	}
	
	if(VENC_MJPG_TABLE[pEncChan->u32ChipID][pEncChan->u32Ch].u8IsEnable){
		s32Ret = ISIL_VENC_MJPG_Disable(pEncChan);
	}

    return s32Ret;
}

int ISIL_VENC_MJPG_DiscardFrame(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }
	
    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    } 

    //todo
    //ioctl,send cfg to driver, now unknown
    fd = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    if(ioctl(fd, ISIL_CODEC_CHAN_DISCAED_FRAME, NULL) < 0) {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" ioctl fail");
       perror("why: ");
       #endif
       return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}

int ISIL_VENC_MJPG_Flush(CODEC_CHANNEL *pChan)
{
    int fd = 0;

    if(pChan == NULL){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG(" point is null ");
        #endif
        return CODEC_ERR_INVAL;
    }

    if(pChan->u32Ch >= ENC_MAXCH){
        #ifdef VENC_MJPG_DEBUG
        CODEC_DEBUG("input chann: %d > MaxCh: %d", pChan->u32Ch, ENC_MAXCH);
        #endif
        return CODEC_ERR_INVAL;
    }

    if(!VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].u8IsCreate){
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" channel not create");
       #endif
       return CODEC_ERR_NOT_EXIST;
    }

    //todo
    //ioctl,send cfg to driver, now unknown
    fd = VENC_MJPG_TABLE[pChan->u32ChipID][pChan->u32Ch].s32ChFd;
    if(ioctl(fd, ISIL_CODEC_CHAN_FLUSH, NULL) < 0) {
       #ifdef VENC_MJPG_DEBUG
       CODEC_DEBUG(" ioctl fail");
       perror("why: ");
       #endif
       return CODEC_ERR_FAIL;
    }

    return CODEC_ERR_OK;
}
