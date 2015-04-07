#ifndef  _ISIL_NET_CONFIG_H
#define  _ISIL_NET_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum NETALLOCTYPE_E{
    NET_STATIC_ALLOC_TYPE = 0 ,
    NET_DYNAMIC_ALLOC_TYPE 
};


#define ISIL_MEDIA_CHIPS (4)

#define ISIL_PER_CHIP_MAX_CHN (16)


#define DEFAULT_NET_DATA_THR_NUM (6)

#define DEFAULT_FILE_RECORD_THR_NUM (4)



#define PER_THR_MAX_TASKS (6)

#define NET_DEFAULT_THREAD_NUM (30)


#define USE_NET_TASK_ASYNC_RELEASE
//#undef  USE_NET_TASK_ASYNC_RELEASE

#define USE_NEW_DATA_DISPATCH_POLICY

//#undef  USE_NEW_DATA_DISPATCH_POLICY




#define isil_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)


#define isil_container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - isil_offsetof(type,member) );})



#if defined(__LITTLE_ENDIAN_BITFIELD)

#define bswap_16(x)  (x)

#define bswap_32(x)  (x)


#else

#define bswap_16(x) \
({ \
        uint16_t __x = (x); \
        ((uint16_t)( \
                (((uint16_t)(__x) & (uint16_t)0x00ffU) << 8) | \
                (((uint16_t)(__x) & (uint16_t)0xff00U) >> 8) )); \
})

#define bswap_32(x) \
({ \
        uint32_t __x = (x); \
        ((uint32_t)( \
                (((uint32_t)(__x) & (uint32_t)0x000000ffUL) << 24) | \
                (((uint32_t)(__x) & (uint32_t)0x0000ff00UL) <<  8) | \
                (((uint32_t)(__x) & (uint32_t)0x00ff0000UL) >>  8) | \
                (((uint32_t)(__x) & (uint32_t)0xff000000UL) >> 24) )); \
})


#endif


static inline uint16_t bswap16(uint16_t x)
{
    return bswap_16(x);
}

static inline uint32_t bswap32(uint32_t x)
{
    return bswap_32(x);
}




#ifdef __cplusplus
}
#endif

#endif
