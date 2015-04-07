#ifndef   _NET_THREAD_POOL_H_
#define   _NET_THREAD_POOL_H_



#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <pthread.h>

#include "isil_net_config.h"


#ifndef MIN_STACK_SIZE
#define MIN_STACK_SIZE  1<<20
#endif


typedef struct _net_thread_t net_thread_t;  
typedef struct net_thread_pool_t net_thread_pool_t; 

typedef void ( * NETDispatchFunc_t)( void * );

struct _net_thread_t{
    pthread_t tid;
    pthread_mutex_t thread_lock;
    pthread_cond_t  thread_cond;
    NETDispatchFunc_t  mFunc;
    void*           arg;
    net_thread_pool_t *parent;
    
};

struct net_thread_pool_t{
//    struct list_head threads_list;
    pthread_mutex_t  threads_lock;
    int threads_num;

    int max_threads;
    int index;
    int is_shutdown;
    int is_init;

    pthread_cond_t mIdleCond;
	pthread_cond_t mFullCond;
	pthread_cond_t mEmptyCond;

    net_thread_t ** threads_list;
};


extern int net_create_thread_pool(net_thread_pool_t *thread_pool,int maxThreads);

extern void net_free_thread_pool(net_thread_pool_t *thread_pool);

extern int net_thread_dispatch(net_thread_pool_t *thread_pool,NETDispatchFunc_t DispatchFunc,void* arg);

extern int net_create_glb_thread_pool(int maxThreads);

extern net_thread_pool_t * net_get_glb_thread_pool(void);

extern void net_monitor_glb_threads_num( void );

extern void net_release_glb_thread_pool(void);

extern int net_glb_thread_dispatch(NETDispatchFunc_t dispatchfunc,void* arg);




#ifdef __cplusplus
}
#endif //__cplusplus

#endif
