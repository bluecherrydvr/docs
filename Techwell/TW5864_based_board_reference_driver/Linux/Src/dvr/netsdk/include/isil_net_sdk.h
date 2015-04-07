#ifndef  ISIL_NET_SDK_H
#define  ISIL_NET_SDK_H


#ifdef __cplusplus
extern "C" {
#endif

#include <semaphore.h> 


#define NETCTRLPORT (10000)

#define NETDATAPORT (10001)

#define NETLOCALPORT (10002)

#define NETASYNCNOTIFYPORT (10003)

typedef struct _ISIL_NET_SDK_MGT{
    sem_t net_sdk_sem;
}ISIL_NET_SDK_MGT;

extern void isil_net_sdk_init(void);

extern void isil_net_sdk_cleanup(void);

extern int isil_net_sdk_run(void *arg);

extern int isil_net_thread_run( void );


#ifdef __cplusplus
}
#endif //__cplusplus

#endif



