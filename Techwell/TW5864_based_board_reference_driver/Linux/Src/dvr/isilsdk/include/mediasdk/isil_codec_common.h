#ifndef _ISIL_CODEC_COMMON_H_

#define _ISIL_CODEC_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

//fd
#define CODEC_HANDLE int

#define CODEC_MAXCHIP  TOTAL_CHIPS_NUM //(encoder chip + decoder chip - codec chip)


#if 1

typedef enum
{
    /****video codec type*****/
    CODEC_VIDEO_H264 = 0,//h264 
    CODEC_VIDEO_MJPG,//mjpg
    /*****audio codec type****/
    CODEC_AUDIO_ALAW,//G.711
    CODEC_AUDIO_ULAW,//G.711
    CODEC_AUDIO_ABADPCM,//G.722
    CODEC_AUDIO_MPMLQ,//G.723.1
    CODEC_AUDIO_ADPCM,//G.726
    CODEC_AUDIO_EMBED_ADPCM,//G.727 embeded adpcm
    CODEC_AUDIO_LDCELP,//G.728
    CODEC_AUDIO_CSACELP,//G.729
}CODEC_TYPE;


typedef enum
{
    STREAM_TYPE_MAJOR = 0,//major stream
    STREAM_TYPE_MINOR,//minor stream
    STREAM_TYPE_PIC,//picture stream, capture
    STREAM_TYPE_MV,//motion vector stream
    STREAM_TYPE_AUDIO,//audio stream
    STREAM_TYPE_NOTHING = 0xFFFF,
}STREAM_TYPE;
#endif

typedef enum
{
	IO_TYPE_NONE,//none
    IO_TYPE_BLK,//block I/O
    IO_TYPE_NOBLK,//no block I/O
}IO_BLK_TYPE;//I/O block

typedef enum
{
    TV_FORMAT_PAL,
    TV_FORMAT_NTSC,
    TV_FORMAT_AUTO,
}CODEC_TV_FORMAT;


typedef enum
{
    YUV_MOD_420 = 0x01,
    YUV_MOD_422 = 0x02,
    YUV_MOD_444 = 0x04,
}CODEC_YUV_MOD;


/***************video/audio encoding/decoding common*************/
typedef struct
{
    CODEC_TYPE    eCodecType;//codec type
    STREAM_TYPE   eStreamType;//stream type for channel
    unsigned int  u32ChipID;//chip ID,user define
    unsigned int  u32Ch;//channel id
}CODEC_CHANNEL;//codec channel


/**************************stream I/O*******************/
typedef enum
{
    DATA_ACCESS_MODE_MMAP,//by mmap way
	DATA_ACCESS_MODE_RW, // directly read or write
}DATA_ACCESS_MODE;

typedef struct
{
    char*             pUsrBuf;//usr buff address,all driver data save in here,see struct ISIL_frame_header
	unsigned int      u32UsrBufLen;//user buffer length, user to SDK,
	unsigned int      u32ActiveLen;//active data len in usr buffer
	unsigned int      u32FrameCnt;//totle frame count
}CODEC_STREAM_DATA;


typedef enum
{
    ISIL_AUDIO_16BIT = 0,
    ISIL_AUDIO_8BIT,
}AUDIO_BIT_WIDTH;

typedef enum
{
    ISIL_AUDIO_8000 = 0,
    ISIL_AUDIO_16000,
    ISIL_AUDIO_32000,
    ISIL_AUDIO_44100,
    ISIL_AUDIO_48000,
    ISIL_AUDIO_RESERVED,
}AUDIO_SAMPLE_RATE;


enum ISIL_AUDIO_TYPE{
    ISIL_AUDIO_PCM = 0,
    ISIL_AUDIO_ALAW,
    ISIL_AUDIO_ULAW,
    ISIL_AUDIO_ADPCM_32K,
    ISIL_AUDIO_ADPCM_16K,
    ISIL_AUDIO_ADPCM_48K
};

#ifdef __cplusplus
}
#endif

#endif
