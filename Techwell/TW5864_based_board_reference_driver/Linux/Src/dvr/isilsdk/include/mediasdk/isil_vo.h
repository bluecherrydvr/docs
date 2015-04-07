#ifndef _ISIL_VO_H_
#define _ISIL_VO_H_

#ifdef __cpluscplus
extern "C" {
#endif

#include "isil_codec_common.h"

#ifndef ISIL_DRIVER_IOC_MAGIC
#define ISIL_DRIVER_IOC_MAGIC  'H'
#endif

#ifndef ISIL_VO_CTL_OP
#define ISIL_VO_CTL_OP    _IOW(ISIL_DRIVER_IOC_MAGIC,  100, int)
#endif

#ifndef ISIL_VO_SET_VGA
#define ISIL_VO_SET_VGA   _IOR(ISIL_DRIVER_IOC_MAGIC,  101, int)
#endif

#ifndef ISIL_VO_SET_VIDEO_ENCODER_SOURCE
#define ISIL_VO_SET_VIDEO_ENCODER_SOURCE      _IOR(ISIL_DRIVER_IOC_MAGIC,  102, int)
#endif

#ifndef ISIL_VO_SET_RECORD_VIDEO_CAPTURE_CONTROL_MODE
#define ISIL_VO_SET_RECORD_VIDEO_CAPTURE_CONTROL_MODE     _IOW(ISIL_DRIVER_IOC_MAGIC,  103, int)
#endif

#ifndef ISIL_VO_SET_DISPLAY_VIDEO_CAPTURE_CONTROL_MODE
#define ISIL_VO_SET_DISPLAY_VIDEO_CAPTURE_CONTROL_MODE    _IOW(ISIL_DRIVER_IOC_MAGIC,  104, int)
#endif

#ifndef ISIL_VO_SET_RECORD_DISPLAY_PARM
#define ISIL_VO_SET_RECORD_DISPLAY_PARM   _IOW(ISIL_DRIVER_IOC_MAGIC,  105, int)
#endif

#ifndef ISIL_VO_SET_DISPLAY_WINDOW
#define ISIL_VO_SET_DISPLAY_WINDOW    _IOW(ISIL_DRIVER_IOC_MAGIC,  106, int)
#endif

#ifndef ISIL_VO_SET_DISPLAY_PLAYBACK_CHANNEL//bind
#define ISIL_VO_SET_DISPLAY_PLAYBACK_CHANNEL      _IOW(ISIL_DRIVER_IOC_MAGIC,  111, int)
#endif

#ifndef ISIL_VO_SET_ANALOG_OUTPUT_SCALER
#define ISIL_VO_SET_ANALOG_OUTPUT_SCALER  _IOW(ISIL_DRIVER_IOC_MAGIC,  113, int)
#endif

#ifndef ISIL_VO_SET_ALPHA_VALUE
#define ISIL_VO_SET_ALPHA_VALUE  _IOW(ISIL_DRIVER_IOC_MAGIC,  118, int)
#endif

#ifndef ISIL_VO_GET_ALPHA_VALUE
#define ISIL_VO_GET_ALPHA_VALUE  _IOW(ISIL_DRIVER_IOC_MAGIC,  119, int)
#endif



//system config
#define VO_CHIP_MAX TOTAL_CHIPS_NUM //maxium vo chip
#define VO_CHIP_CVBS_MAXCH PER_CHIP_MAX_CHAN_NUM //maxium supported CVBS channel in a chip
#define VO_CHIP_VGA_MAXCH PER_CHIP_MAX_CHAN_NUM //maxium supported VGA channel in a chip

#define VO_HANDLE int // video output channel fd
//define output type
typedef enum
{
    VO_CH_TYPE_CVBS,//cvbs
    VO_CH_TYPE_VGA,//vga
}VO_CH_TYPE;

//define rectangle area for input channel
typedef struct
{
    unsigned int u32ChipID;
    unsigned int u32Channel;
    unsigned int u32X;//x offset
    unsigned int u32Y;//y offset
    unsigned int u32Width;//width
    unsigned int u32Height;//height
}VO_CH_RECT;

//scaler factor
typedef struct
{
    unsigned int u32ChipID;
    unsigned int u32Path;
    unsigned int u32HFactor;/* 0x1FFF is scaling factor of 1. */
    unsigned int u32VFactor;/* 0x1FFF is scaling factor of 1. */
}VO_CVBS_SCALER;

//each vi channel config parament
typedef struct
{
    unsigned int    u32ChipID;
    unsigned int    u32Channel;//physical channel ID
    unsigned int    u32Mask;//mask bit
	unsigned int    u32Brightness;
	unsigned int    u32Contrast;
	unsigned int    u32Saturation;
	unsigned int    u32Hue;
	unsigned int    u32Sharpness;
}VO_CH_CFG;

//bind input channel to display window
typedef struct {
	unsigned int u32ChipID;
	unsigned int u32WndID;
	unsigned short u16Left;
	unsigned short u16Right;
	unsigned short u16Top;
	unsigned short u16Bottom;
}VO_WND_SIZE;


enum VIDEO_INPUT_TYPE {
	PB_INPUT = 0,
	LIVE_INPUT,
};

typedef struct {
	unsigned int u32ChipID;//chip id
	enum VIDEO_INPUT_TYPE video_input_type;	/* PB or live */
	unsigned int u32WndEnable;//window enable
	unsigned int u32Channel;//physical channel ID
	VO_WND_SIZE	 sWindow;//window dimension
}VO_WND_BIND;

typedef struct{
    unsigned int u32ChipID;//chip id
	unsigned int u32SrcPath;
	unsigned int u32WhichEncoder;//select encoder
}VO_MUX_PATH;


struct ISIL_chip_vo_info{
    unsigned int i_vout_chan_nubmer;
    unsigned int i_vout_ad_chan_number;
};

//chip control cmd and data struct
typedef struct
{
    unsigned int u32ChipID;//chip handle
    unsigned int u32Cmd;//control cmd
    unsigned int u32Blk;//block or nonblock operator
    unsigned int u32MsgLen;//msg length
    char         pMsg[0];//msg content
}VO_CTL_MSG;


enum OSG_PATH {
	VGA_DISPLAY_OSG = 0,
	CVBS_DISPLAY_OSG,
	RECORD_PORT0_OSG,
	RECORD_PORT1_OSG,
	SPOT_OSG,
};

typedef struct {
	int chip_num;
	enum OSG_PATH osg_path;
	int main_window_alpha_value;
	int sub_window_alpha_value;
} osg_alpha_config_t;

/*video input API*/
extern int ISIL_VO_SetCropping(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_CH_RECT* pstRect);
extern int ISIL_VO_SetCvbsScaler(unsigned int u32ChipID, VO_CVBS_SCALER* pstFac);

extern int ISIL_VO_SetVideoType(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, unsigned int u32VideoType);

//extern int ISIL_VO_Pause(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType);
//extern int ISIL_VO_Resume(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType);

//put any one of input channel to any display position
extern int ISIL_VO_Bind2Wnd(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_WND_BIND* pVoWnd);

extern int ISIL_VO_SetMuxPath(unsigned int u32ChipID, VO_MUX_PATH *pVoPath);

extern int ISIL_VO_SetCfg(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_CH_CFG* pVoCfg);
extern int ISIL_VO_GetCfg(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_CH_CFG* pVoCfg);

//extern int ISIL_VO_GetChHandle(unsigned int u32ChipID, unsigned int u32Ch, VO_CH_TYPE eChType, VO_HANDLE* pChHandle);

/*vi chip control*/
extern int ISIL_VO_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID);
extern int ISIL_VO_CTL_CloseChip(unsigned int u32ChipID);

extern int ISIL_VO_CTL_GetChipHandle(unsigned int u32ChipID, VO_HANDLE* pChipHandle);

extern int ISIL_VO_CTL_SendMsg(unsigned int u32ChipID, VO_CTL_MSG *pMsg);//finish all operator of chip,eg:bind VO, mux ,demux etc

extern int ISIL_VO_GetAlphaBlending(unsigned int u32ChipID, osg_alpha_config_t* pVoAlphaCfg);
extern int ISIL_VO_SetAlphaBlending(unsigned int u32ChipID, osg_alpha_config_t* pVoAlphaCfg);

#ifdef __cpluscplus
}
#endif
#endif




