#ifndef  _ISIL_DEBUG_H
#define _ISIL_DEBUG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif



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
