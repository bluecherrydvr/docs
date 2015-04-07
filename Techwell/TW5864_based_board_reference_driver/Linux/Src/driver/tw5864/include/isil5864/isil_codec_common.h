#ifndef __ISIL_CODEC_COMMON_H__
#define __ISIL_CODEC_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define ISIL_CODEC_CHAN_REQUEST_MEM                      _IOW(ISIL_CODEC_IOC_MAGIC, 243, unsigned int)                            
#define ISIL_CODEC_CHAN_RELEASE_MEM                      _IOW(ISIL_CODEC_IOC_MAGIC, 242, unsigned int)
#define ISIL_CODEC_CHAN_DISCAED_FRAME		             _IOW(ISIL_CODEC_IOC_MAGIC, 244, unsigned int)
#define ISIL_CODEC_CHAN_FLUSH			                 _IOW(ISIL_CODEC_IOC_MAGIC, 150, unsigned int)
#define ISIL_CODEC_CHAN_CHANG_MODE		                 _IOW(ISIL_CODEC_IOC_MAGIC, 151, unsigned int)	//----struct decoder_mode
#define ISIL_CODEC_CHAN_TS_RESYNC			             _IOW(ISIL_CODEC_IOC_MAGIC, 152, unsigned int)
#define ISIL_CODEC_CHAN_SET_STRATEGY                     _IOW(ISIL_CODEC_IOC_MAGIC, 153, unsigned int)	//----set push/pull strategy
#define ISIL_CODEC_CHAN_SET_TIME_CONFIG                  _IOW(ISIL_CODEC_IOC_MAGIC, 154, unsigned int)	//----struct time_config
#define ISIL_CODEC_CHAN_READY_TO_DIE                     _IOW(ISIL_CODEC_IOC_MAGIC, 155, unsigned int)
//#define ISIL_CHIP_START_AVSYNC		               _IOW(ISIL_CHIP_IOC_MAGIC, 243, unsigned int)	//----struct  syn_arg
//#define ISIL_CHIP_STOP_AVSYNC                            _IOW(ISIL_CHIP_IOC_MAGIC, 242, unsigned int)	//----struct  syn_arg


#ifdef __cplusplus
}
#endif

#endif //__ISIL_CHIP_DRIVER_H__

