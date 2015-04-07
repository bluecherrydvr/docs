#ifndef  _NET_DEBUG_H
#define _NET_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define DEBUG_FUNCTION() \
do{\
   fprintf(stderr,"[file] = %s ,[func] %s,[line] =%d.\n",__FILE__,__FUNCTION__,__LINE__); \
}while(0)

#define SINGLE_DEBUG() \
do{\
    fprintf(stderr,"[file] = %s ,[func] %s,[line] =%d.\n",__FILE__,__FUNCTION__,__LINE__); \
    pause(); \
}while(0)


#ifdef __cplusplus
}
#endif

#endif
