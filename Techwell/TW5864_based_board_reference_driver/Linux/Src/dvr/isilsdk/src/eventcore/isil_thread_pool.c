#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<string.h>


#include "isil_thread_pool.h"


static isil_thread_pool_t glb_thread_pool;

static int save_thread(isil_thread_pool_t *thread_pool,isil_thread_t *thread)
{
    int ret = -1;
    
    pthread_mutex_lock(&thread_pool->threads_lock);
    if(thread_pool->index < thread_pool->max_threads) {
        thread_pool->threads_list[thread_pool->index] = thread;
        thread_pool->index++;
        ret = 0;

        pthread_cond_signal(&thread_pool->mIdleCond);

        if(thread_pool->index >= thread_pool->threads_num) {
            pthread_cond_signal(&thread_pool->mFullCond);
        }
    }

    pthread_mutex_unlock(&thread_pool->threads_lock);

    return ret;
}

static void* thread_wrapperFunc(void* arg)
{
    isil_thread_t *thread = (isil_thread_t *)arg;
    for( ;thread->parent->is_shutdown == 0; ) {
        thread->mFunc(thread->arg);

        if(thread->parent->is_shutdown != 0) {
            break;
        }
        pthread_mutex_lock(&thread->thread_lock);
        if(save_thread(thread->parent,thread ) == 0){
            pthread_cond_wait(&thread->thread_cond,&thread->thread_lock);
            pthread_mutex_unlock(&thread->thread_lock);
        }else{
            
            pthread_mutex_unlock(&thread->thread_lock);
            pthread_cond_destroy(&thread->thread_cond);
            pthread_mutex_destroy(&thread->thread_lock);
            free(thread);
            thread = NULL;
            break;
        }
    }

    if(thread != NULL) {
        pthread_mutex_lock(&thread->thread_lock);
        thread->parent->threads_num--;
        if(thread->parent->threads_num <= 0) {
            
            pthread_cond_signal(&thread->parent->mEmptyCond);
           
        }
        pthread_mutex_unlock(&thread->thread_lock);
    }
    return 0;
}

int thread_dispatch(isil_thread_pool_t *thread_pool,DispatchFunc_t DispatchFunc,void* arg)
{
    int ret = 0;
    size_t old_size;
    pthread_attr_t attr;
    isil_thread_t *thread = NULL;

    pthread_mutex_lock(&thread_pool->threads_lock);
    for( ;(thread_pool->index <= 0) && (thread_pool->threads_num >= thread_pool->max_threads); ) {
        pthread_cond_wait(&thread_pool->mIdleCond,&thread_pool->threads_lock);
    }

    if(thread_pool->index <= 0) {
        thread = (isil_thread_t *)malloc(sizeof(isil_thread_t));
        memset(&thread->tid,0x00,sizeof(pthread_t));
        pthread_mutex_init(&thread->thread_lock,NULL);
        pthread_cond_init(&thread->thread_cond,NULL);
        thread->arg = arg;
        thread->mFunc = DispatchFunc;
        thread->parent = thread_pool;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        if (pthread_attr_getstacksize(&attr, &old_size) != 0){
            exit(-1);
        }
        
        if (old_size > MIN_STACK_SIZE) {
          if (pthread_attr_setstacksize(&attr, MIN_STACK_SIZE) != 0)
                exit(-1);
        }

       
        
        if(pthread_create(&thread->tid,&attr,thread_wrapperFunc,thread) == 0) {
            thread_pool->threads_num++;
            
        }
        else{
            ret = -1;
            fprintf(stderr,"Can't create thread.\n");
            pthread_mutex_destroy(&thread->thread_lock);
            pthread_cond_destroy(&thread->thread_cond);
            free(thread);
            thread = NULL;
        }
        pthread_attr_destroy(&attr);
    }
    else{
        thread_pool->index--;
        thread = thread_pool->threads_list[thread_pool->index];
        thread_pool->threads_list[thread_pool->index] = NULL;
        thread->mFunc = DispatchFunc;
        thread->arg   = arg;
        thread->parent = thread_pool;

        pthread_mutex_lock(&thread->thread_lock);
        pthread_cond_signal(&thread->thread_cond);
        pthread_mutex_unlock(&thread->thread_lock);
    }
    pthread_mutex_unlock(&thread_pool->threads_lock);

    return ret;
}



int create_thread_pool(isil_thread_pool_t *thread_pool,int maxThreads)
{

    if( !thread_pool->is_init ) {
        if(maxThreads <= 0) {
            thread_pool->max_threads = DEFAULT_THREAD_NUM;
        }
        thread_pool->max_threads = maxThreads;
        
        pthread_mutex_init(&thread_pool->threads_lock,NULL);
        pthread_cond_init(&thread_pool->mEmptyCond,NULL);
        pthread_cond_init(&thread_pool->mFullCond,NULL);
        pthread_cond_init(&thread_pool->mIdleCond,NULL);
    
        thread_pool->index = 0;
        thread_pool->threads_num = 0;
        thread_pool->is_shutdown = 0;
    
        thread_pool->threads_list = (isil_thread_t **)malloc(sizeof(void *) * maxThreads);
        memset(thread_pool->threads_list,0x00 ,sizeof(void *)* maxThreads);

        thread_pool->is_init = 1;
    }
    return 0;
}

void free_thread_pool(isil_thread_pool_t *thread_pool)
{
    int i = 0;
    pthread_mutex_lock(&thread_pool->threads_lock);
    while(thread_pool->index < thread_pool->threads_num) {
        pthread_cond_wait(&thread_pool->mFullCond,&thread_pool->threads_lock);
    }

    fprintf( stderr,"%s %d .\n",__FUNCTION__ ,__LINE__);

    thread_pool->is_shutdown = 1;
    fprintf( stderr,"%s %d .\n",__FUNCTION__ ,__LINE__);

    for(i = 0 ;i < thread_pool->index ; i++) {
        isil_thread_t *thread = thread_pool->threads_list[i];
        if(thread != NULL) {
            pthread_mutex_lock(&thread->thread_lock);
            pthread_cond_signal(&thread->thread_cond);
            pthread_mutex_unlock(&thread->thread_lock);
        }
    }
    fprintf( stderr,"%s %d .\n",__FUNCTION__ ,__LINE__);

    while(thread_pool->threads_num > 0) {
        
        pthread_cond_wait(&thread_pool->mEmptyCond,&thread_pool->threads_lock);
    }
    fprintf( stderr,"%s %d .\n",__FUNCTION__ ,__LINE__);
    fprintf(stderr,"Thread pool destroy %d thread struct.\n",thread_pool->index );
    for(i = 0 ; i < thread_pool->index ; i++) {
        isil_thread_t *thread = thread_pool->threads_list[i];
        if(thread != NULL) {
            pthread_mutex_destroy(&thread->thread_lock);
            pthread_cond_destroy(&thread->thread_cond);
            free(thread);
            thread = NULL;
        }
        thread_pool->threads_list[i] = NULL;
    }
    pthread_mutex_unlock(&thread_pool->threads_lock);

    thread_pool->index = 0;
    pthread_mutex_destroy(&thread_pool->threads_lock);
    pthread_cond_destroy(&thread_pool->mIdleCond);
    pthread_cond_destroy(&thread_pool->mEmptyCond);
    pthread_cond_destroy(&thread_pool->mFullCond);

    if(thread_pool->threads_list != NULL) {
        free(thread_pool->threads_list);
        thread_pool->threads_list = NULL;
    }
    return;
}


int get_max_threads(isil_thread_pool_t *thread_pool)
{
    return thread_pool->max_threads;
}

int create_glb_thread_pool(int maxThreads)
{
    fprintf( stderr,"glb_thread_pool address[%p] .\n",&glb_thread_pool);
    return create_thread_pool(&glb_thread_pool,maxThreads);
}

isil_thread_pool_t * get_glb_thread_pool(void)
{
    return &glb_thread_pool;
}

void monitor_glb_threads_num( void )
{
    fprintf(stderr,"glb_threads_num[%d] .\n",glb_thread_pool.threads_num);
}

void release_glb_thread_pool(void)
{
    free_thread_pool(&glb_thread_pool);
}

int glb_thread_dispatch(DispatchFunc_t DispatchFunc,void* arg)
{
    return thread_dispatch(&glb_thread_pool,DispatchFunc,arg);
}


