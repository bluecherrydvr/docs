#ifndef _ISIL_AO_H_
#define _ISIL_AO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_codec_common.h"

/* Use 'H' as magic number */
#ifndef ISIL_DRIVER_IOC_MAGIC
#define ISIL_DRIVER_IOC_MAGIC  'H'
#endif

//mute
#ifndef ISIL_AO_SET_SINGLE_AUDIO_MUTE_CHANNEL
#define ISIL_AO_SET_SINGLE_AUDIO_MUTE_CHANNEL    _IOW(ISIL_DRIVER_IOC_MAGIC,  157, int)
#endif

//select audio output channel
#ifndef ISIL_AO_SET_FINAL_MIX_AUDIO_OUTPUT
#define ISIL_AO_SET_FINAL_MIX_AUDIO_OUTPUT	_IOW(ISIL_DRIVER_IOC_MAGIC,  156, int)
#endif

//audio output mix ratio
#ifndef ISIL_AO_SET_AUDIO_MIX_RATIO
#define ISIL_AO_SET_AUDIO_MIX_RATIO			_IOW(ISIL_DRIVER_IOC_MAGIC,  153, int)
#endif

#ifndef ISIL_AO_CTL_OP
#define ISIL_AO_CTL_OP	_IOW(ISIL_DRIVER_IOC_MAGIC,  150, int)
#endif


//system config
#define AO_CHIP_MAX TOTAL_CHIPS_NUM //maxium AO chip
//#define AO_CHIP_CH_MAX (8)//maxium supported channel in a chip
#define AO_CHIP_ANALOG_MAXCH PER_CHIP_MAX_CHAN_NUM //maxium supported analog channel in a chip
#define AO_CHIP_DIGITAL_MAXCH PER_CHIP_MAX_CHAN_NUM //maxium supported digital channel in a chip

#define AO_HANDLE int //audio output channel fd

//define audio output channel type
typedef enum
{
     AO_CH_TYPE_ANALOG,  //analog channel
     AO_CH_TYPE_DIGITAL, //digital channel
}AO_CH_TYPE;

//each ao channel config parament
typedef struct
{
    unsigned int    u32Channel;//physical channel ID
    unsigned int    u32Mask;
    unsigned int    u32BitWidth;//bit width
    unsigned int    u32BitRate;//bit rate
    unsigned int    u32SampleRate;//sample rate
}AO_CH_CFG;

//mute channel
typedef struct
{
    unsigned int u32ChipID;//chip id
    unsigned int u32Channel;//channel id
    //AO_CH_TYPE   eChType;//channel type:analog or digital
    unsigned int u32IsMute;//0:mute;1:not mute
}AO_CH_MUTE;

//volume adjust
typedef struct
{
    unsigned int u32ChipID;//chip id
    unsigned int u32Channel;//channel id
    unsigned int u32Volume;//volume value
} AO_CH_VOLUME;

//mux output channel
typedef struct {
    unsigned int u32ChipID;
    unsigned int u32Channel;
} AO_MUX_FINAL_OUTPUT_CH;

//chip control cmd and data struct
struct ISIL_chip_ao_info{
    unsigned int i_aout_chan_nubmer;
    unsigned int i_aout_ad_chan_number;
};

typedef struct
{
    unsigned int u32ChipID;//chip handle
    unsigned int u32Cmd;//control cmd
    unsigned int u32Blk;//block or nonblock operator
    unsigned int u32MsgLen;//msg length
    void*        pMsg;//msg content
}AO_CTL_MSG;


/*Audio output API*/
extern int ISIL_AO_SetCfg(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_CH_CFG* pAoCfg);
extern int ISIL_AO_GetCfg(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_CH_CFG* pAoCfg);

//extern int ISIL_AO_Pause(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType);
//extern int ISIL_AO_Resume(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType);

extern int ISIL_AO_Mute(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_CH_MUTE *pAoMute);


extern int ISIL_AO_SetVol(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_CH_VOLUME *pvol);

//extern int ISIL_AO_GetChHandle(unsigned int u32ChipID, unsigned int u32Ch, AO_CH_TYPE eChType, AO_HANDLE* pChHandle);

/*vo chip control*/
extern int ISIL_AO_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID);
extern int ISIL_AO_CTL_CloseChip(unsigned int u32ChipID);

extern int ISIL_AO_CTL_GetChipHandle(unsigned int u32ChipID, AO_HANDLE* pChipHandle);

extern int ISIL_AO_CTL_SendMsg(unsigned int u32ChipID, AO_CTL_MSG *pMsg);//finish all operator of chip,eg:bind VO, mux ,demux etc

extern int ISIL_AO_MuxFinalCh(unsigned int u32ChipID, AO_MUX_FINAL_OUTPUT_CH* pAoFinalCh);

#ifdef __cplusplus
}
#endif
#endif


