#ifndef _ISIL_CODEC_DEBUG_H_
#define _ISIL_CODEC_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

/********************define codec debug*********************/
#define CODEC_DEBUG_ENABLE

#ifdef CODEC_DEBUG_ENABLE
#define  CODEC_DEBUG(fmt, args...) \
         do{\
               fprintf(stderr, "\nfun: %s, line: %d\n", __FUNCTION__, __LINE__);\
               fprintf(stderr, fmt, ##args);\
               fprintf(stderr, "\n");\
           }while(0)
#else
#define CODEC_DEBUG(fmt, args...)
#endif


#ifdef __cplusplus
}
#endif
#endif



