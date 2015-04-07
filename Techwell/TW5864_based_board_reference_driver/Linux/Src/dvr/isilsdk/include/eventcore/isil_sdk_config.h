#ifndef  _ISIL_SDK_CONFIG_H
#define _ISIL_SDK_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

enum ISIL_E_NOTIFY_TYPE{
    ISIL_NOTIFY_ONCE = 0,
    ISIL_NOTIFY_FOREVER
};

enum ISIL_E_BLOCK_TYPE{
    ISIL_BLOCK = 0 ,
    ISIL_NONEBLOCK
};

enum ISILEALLOCTYPE{
    ISIL_STATIC_ALLOC_TYPE = 0 ,
    ISIL_DYNAMIC_ALLOC_TYPE 
};

#define PER_ENC_THR_MAX_TASKS (5)

#define DEFAULT_ENC_THR_NUM (4)

#define DEFAULT_THREAD_NUM (30)

#define DEFAULT_FRAME_NUMS (30)

#define DEFAULT_FRAME_BUFF_SIZE (1024000)

#define MINFRAMEBUFFSIZE (40960)

#define SDKRUNUSETHREAD

#define ENCUSEMULTITHREAD

#undef ENCUSEMULTITHREAD

#define CHIPS_INF_CONFIG_PATH "/flash/chips.conf"

//#define USEOLDHEADER


#ifdef __cplusplus
}
#endif

#endif
