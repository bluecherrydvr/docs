#ifndef _ISIL_DECODER_H_
#define _ISIL_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_codec_common.h"
#include "config.h"

#define DEC_MAXCHIP CODEC_MAXCHIP

#define DEC_MAXCH   PER_CHIP_MAX_CHAN_NUM


/*************************data struct*********************/
/*******h264 decoding******/
typedef struct
{
    unsigned int u32Ch;
    unsigned int u32Priority;
    unsigned int u32Width;
    unsigned int u32Height;
    unsigned int u32TVFormat;//CODEC_TV_FORMAT,PAL/NTSC/AUTO
    unsigned int u32BitRate;//bitrate of decoding
    unsigned int u32Fps;//f/s
    unsigned int u32TvForamt;       /*NTSC or PAL*/
    void *       pExtPrivate;
	IO_BLK_TYPE  eBlk;        //IO_BLK_TYPE
	unsigned int u32IsDone;/*if driver had been config sucess */
}VDEC_H264_CFG;


/**********************video mjpg decoding*********************/
typedef struct
{
    unsigned int u32Ch;
    unsigned int u32Priority;
    unsigned int u32Width;
    unsigned int u32Heigth;
    unsigned int u32TvForamt;       /*NTSC or PAL*/
    void *       pExtPrivate;
	IO_BLK_TYPE  eBlk;        //IO_BLK_TYPE
	unsigned int u32IsDone;         /*if driver had been config sucess */
}VDEC_MJPG_CFG;


/***********************audio decoder cfg ************/

typedef struct
{
    unsigned int      u32Ch;//channel ID
    AUDIO_BIT_WIDTH   eBitWidth;//bit width
    AUDIO_SAMPLE_RATE eSampleRate;//sample rate
}VDEC_AUD_CFG;

//audio/h264/mjpg use the same command and struct

//pull or push path
typedef enum
{
	DECODE_STRATEGY_PUSH = 0x00000001,
	DECODE_STRATEGY_PULL = 0x00000002,
}VDEC_CH_STRATEGY;

//set playback mode

enum  X_MODE{
	ISIL_CODEC_DEC_NON_RT_FPS = 0,// non-real-time
	ISIL_CODEC_DEC_SPEED_NORMAL,//speed x-1
	ISIL_CODEC_DEC_SPEED_FAST,//speed larger than x-1
	ISIL_CODEC_DEC_SPEED_SLOW,//speed smaller than x-1
};

struct  x_speed
{
	enum  X_MODE    mode;//the mode specified
	unsigned long   x_speed_value;//the real speed value
};

typedef struct decoder_mode
{
	unsigned char 	loop;		//0: not loop,  1: loop
	unsigned char	direction;	//0: backward 1: forward
	unsigned char 	key_frame;	//0: frame one by one 1:only for key frames
	unsigned char 	is_continue;//0: stop after display one frame
								//1: continue after after display one frame
	struct  x_speed speed;		// -1 means nonsense
}VDEC_CH_MODE;


//set request-frame command timeout interval
typedef struct  time_config
{
	unsigned int	max_retry_times;
	unsigned int	max_respond_latency;	//from request send to first frame get
	unsigned int	max_timeout_latency;   //from request send to last frame get
}VDEC_CH_TIMEOUT;

typedef enum{
	STREAM_WITHOUT_HEADER,
	STREAM_WITH_HEADER,
}VDEC_CH_HEADER;


/*************************API*****************************/
extern int ISIL_CODEC_DEC_CreateCh(CODEC_CHANNEL *pDecChan, CODEC_HANDLE* pChipHandle, char* pChDir);
extern int ISIL_CODEC_DEC_ReleaseCh(CODEC_CHANNEL *pDecChan, CODEC_HANDLE* pChipHandle);

extern int ISIL_CODEC_DEC_GetChHandle(CODEC_CHANNEL *pDecChan, CODEC_HANDLE *pDecHandle);

extern int ISIL_CODEC_DEC_EnableCh(CODEC_CHANNEL *pDecChan);
extern int ISIL_CODEC_DEC_DisableCh(CODEC_CHANNEL *pDecChan);

extern int ISIL_CODEC_DEC_SetCfg(CODEC_CHANNEL *pDecChan, void* pDecCfg);
extern int ISIL_CODEC_DEC_GetCfg(CODEC_CHANNEL *pDecChan, void* pDecCfg);

extern int ISIL_CODEC_DEC_SendData(CODEC_CHANNEL *pDecChan, CODEC_STREAM_DATA *pStream);

extern int ISIL_CODEC_DEC_Flush(CODEC_CHANNEL *pDecChan);

/*decoding control API*/
//pull or push
extern int ISIL_CODEC_DEC_SetStrategy(CODEC_CHANNEL *pDecChan, VDEC_CH_STRATEGY *pStrategy);

//set playback mode
extern int ISIL_CODEC_DEC_SetPBMode(CODEC_CHANNEL *pDecChan, VDEC_CH_MODE *pMode);

//set request-frame command timeout interval
extern int ISIL_CODEC_DEC_SetTimeout(CODEC_CHANNEL *pDecChan, VDEC_CH_TIMEOUT *pTimeout);

//set decoding stream type, have deader or no header that's raw data
extern int ISIL_CODEC_DEC_SetStreamType(CODEC_CHANNEL *pDecChan, VDEC_CH_HEADER* pHeader);

//set decoder channel ready to die
extern int ISIL_CODEC_DEC_Readydie(CODEC_CHANNEL *pDecChan);

#ifdef __cplusplus
}
#endif
#endif
