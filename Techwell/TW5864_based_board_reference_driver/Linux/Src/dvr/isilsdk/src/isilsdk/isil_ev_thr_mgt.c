#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "isil_ev_thr_mgt.h"
#include "isil_thread_pool.h"
#include "linux_list.h"
#include "isil_frame.h"
#include "isil_debug.h"
#include "isil_data_stream.h"
#include "isil_media_sdk_ev.h"

static ISIL_ENC_THR_MGT_T glb_enc_thr_mgt;



static void enc_thr_mgt_init(ISIL_ENC_THR_MGT_T *enc_thr_mgt)
{
    int i;

    for( i = 0 ; i < DEFAULT_ENC_THR_NUM ; i++) {

        task_mgt_sinit(&enc_thr_mgt->enc_task_mgt[i]);

    }

    return ;
    
}


void glb_enc_thr_mgt_init( void )
{
    enc_thr_mgt_init(&glb_enc_thr_mgt);
}


ISIL_ENC_THR_MGT_T *get_glb_enc_thr_mgt(void)
{
    return &glb_enc_thr_mgt;
}

void glb_enc_thr_mgt_release(void)
{
    int i;

    struct ISIL_TASK_MGT *task_mgt;

    ISIL_ENC_THR_MGT_T *enc_thr_mgt  = get_glb_enc_thr_mgt();

    for(i = 0 ; i < DEFAULT_ENC_THR_NUM ; i++) {
        task_mgt = &enc_thr_mgt->enc_task_mgt[i];
        
        
        
        if(task_mgt->wakeup) {
            task_mgt->wakeup(task_mgt);
        }

        task_mgt->is_run = 0;
    }
}


static void ev_task_data_notify_user(struct ISIL_EV_TASK *ev_task)
{
    frame_t *frame;
    struct ISIL_NOTIFY_LST *nty_lst_p;
    struct ISIL_DATE_MGT *data_mgt;
    ISIL_AV_PACKET *av_packet;

    if( !ev_task ) {
        return;
    }

    data_mgt = ev_task->date_mgt;
    if( !data_mgt ) {
        return ;
    }

    do{
        frame = data_mgt->try_get_frame_header(data_mgt);
        if( !frame ) {
            break ;
        }
    
        nty_lst_p = &ev_task->notify_list;  

        av_packet = handle_frame_to_av_packet(frame,ev_task);
        if( av_packet ) {
            nty_lst_p->notify(nty_lst_p ,(void *)av_packet);
        }
        
        DEBUG_FUNCTION();
        frame->release(frame);

    }while(1);


     return;

}


static int enc_task_mgt_run_cb(struct ISIL_TASK_MGT *task_mgt)
{
    

    struct ISIL_EV_TASK *ev_task;
    struct list_head *ln1, *ln2;

    if( !task_mgt ) {
        return -1;
    }

   
    pthread_mutex_lock(&task_mgt->lock);

    if(list_empty(&task_mgt->ev_act_tsk_lst)) {

        pthread_mutex_unlock(&task_mgt->lock);

        return 0;
    }

    list_for_each_safe(ln1, ln2, &task_mgt->ev_act_tsk_lst){
        ev_task = list_entry(ln1,struct ISIL_EV_TASK ,entry);
        if( ev_task ) {
            
            ev_task_data_notify_user(ev_task);

        }
    }

    
    pthread_mutex_unlock(&task_mgt->lock);
    
    return 0;
}



static void task_mgt_run( void *arg)
{

    int ret;

    struct ISIL_TASK_MGT *task_mgt = (struct ISIL_TASK_MGT *)arg;
    
    if( !task_mgt ) {
        return ;
    }

    reg_task_mgt_callback(task_mgt ,enc_task_mgt_run_cb);

    task_mgt->is_run = 1;

    while(task_mgt->is_run) {

        if(task_mgt->wait) {
            task_mgt->wait(task_mgt);
        }

        

        if(task_mgt->callback) {
            ret = task_mgt->callback(task_mgt);
            if( ret < 0 ) {
                fprintf(stderr,"task_mgt callback err .\n");
                break;
            }
        }
        
        

        clear_task_mgt_ev(task_mgt);

    }
    
    task_mgt->is_run = 0;
    task_mgt->release(task_mgt);

}


int task_mgt_thr_create(struct ISIL_TASK_MGT *task_mgt)
{
    int ret =0 ;

    isil_thread_pool_t * thr_pool =get_glb_thread_pool();
    
    if(!task_mgt->is_run) {
        ret = thread_dispatch(thr_pool,task_mgt_run,(void *)task_mgt);
    }

    return ret;
}


static void add_all_task_mgt_max_task_num( void )
{
    int i;
    
    ISIL_ENC_THR_MGT_T *data_thr_mgt  = get_glb_enc_thr_mgt();
    
    for( i = 0 ; i < DEFAULT_ENC_THR_NUM ; i++ ) {

        ++data_thr_mgt->enc_task_mgt[i].max_task_num;
    }

}


int enc_task_thr_alloc_task(struct ISIL_EV_TASK *ev_task)
{

    int i , ret = 0 ;
    struct ISIL_TASK_MGT *task_mgt ;

    ISIL_ENC_THR_MGT_T *data_thr_mgt  = get_glb_enc_thr_mgt();

    if( !ev_task ) {
        return -1;
    }

    if( ev_task->task_mgt ) {
        assert(0);
        return 0;
    }

    
    
    for( i = 0 ; i < DEFAULT_ENC_THR_NUM ; i++) {

        task_mgt = &data_thr_mgt->enc_task_mgt[i];
        
        

        if( task_mgt->ev_act_num == (int)task_mgt->max_task_num ) {
                
            continue;

#if 0
            if(DEFAULT_ENC_THR_NUM %(i+1) != 0) {
                continue;
            }
            else{
                
                add_all_task_mgt_max_task_num();
                task_mgt = &data_thr_mgt->enc_task_mgt[0];
            }
#endif
            
        }

        
            
        
        if(!task_mgt->is_run) {

            ret = task_mgt_thr_create(task_mgt);
            if( ret < 0 ) {
                fprintf( stderr,"task_mgt_thr_create failed .\n");
                return -1;
            }
            
            task_mgt->is_run = 1;
        }
            //TODO :must reg callback in here?

        

        task_mgt->put_ev_task_to_act(task_mgt,ev_task);
        return 0;
            
    }

    return -1;

}




