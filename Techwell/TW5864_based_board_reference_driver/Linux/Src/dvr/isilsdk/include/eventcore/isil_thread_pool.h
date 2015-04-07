#ifndef   _ISIL__THREAD_POOL_H_
#define   _ISIL__THREAD_POOL_H_



#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <pthread.h>

#include "isil_sdk_config.h"



#define MIN_STACK_SIZE  1<<20

typedef struct _isil_thread_t isil_thread_t;  
typedef struct _isil_thread_pool_t isil_thread_pool_t; 

typedef void ( * DispatchFunc_t)( void * );

struct _isil_thread_t{
    pthread_t tid;
    pthread_mutex_t thread_lock;
    pthread_cond_t  thread_cond;
    DispatchFunc_t  mFunc;
    void*           arg;
    isil_thread_pool_t *parent;
    
};

struct _isil_thread_pool_t{
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

    isil_thread_t ** threads_list;
};


extern int create_thread_pool(isil_thread_pool_t *thread_pool,int maxThreads);

extern void free_thread_pool(isil_thread_pool_t *thread_pool);

extern int thread_dispatch(isil_thread_pool_t *thread_pool,DispatchFunc_t DispatchFunc,void* arg);

extern int create_glb_thread_pool(int maxThreads);

extern isil_thread_pool_t * get_glb_thread_pool(void);

extern void monitor_glb_threads_num( void );

extern void release_glb_thread_pool(void);

extern int glb_thread_dispatch(DispatchFunc_t DispatchFunc,void* arg);




#ifdef __cplusplus
}
#endif //__cplusplus

#endif
