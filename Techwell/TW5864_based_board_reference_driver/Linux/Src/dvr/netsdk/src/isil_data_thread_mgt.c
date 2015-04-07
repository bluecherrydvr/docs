#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "isil_data_thread_mgt.h"
#include "net_thread_pool.h"
#include "isil_net_config.h"
#include "net_debug.h"



static struct ISIL_DATA_THR_MGT glb_net_data_thr;

static void data_thr_mgt_init(struct ISIL_DATA_THR_MGT *data_thr_mgt)
{
    int i;
    for( i = 0 ; i < DEFAULT_NET_DATA_THR_NUM ; i++) {
        net_task_mgt_sinit(&data_thr_mgt->net_tasks_mgt[i]);
    }
    
}

void glb_data_thr_mgt_init( void )
{
    data_thr_mgt_init(&glb_net_data_thr);
}

void glb_data_thr_mgt_release(void)
{
    int i;

    struct NET_TASK_MGT *net_task_mgt;

    struct ISIL_DATA_THR_MGT *data_thr_mgt  = get_glb_data_thr_mgt();

    for(i = 0 ; i < DEFAULT_NET_DATA_THR_NUM ; i++) {
        net_task_mgt = &data_thr_mgt->net_tasks_mgt[i];
        net_task_mgt->is_run = 0;
    }
    
}


struct ISIL_DATA_THR_MGT *get_glb_data_thr_mgt(void)
{
    return &glb_net_data_thr;
}


#if 0

static void net_data_send_user(struct NET_TASK *ev_task)
{
    struct NET_DATA_NODE_T *data_nod;
    struct NET_DATA_MGT_T *data_mgt;
    

    if( !ev_task ) {
        return;
    }

    data_mgt = ev_task->net_data_mgt;
    if( !data_mgt ) {
        return ;
    }



    

    do{

        DEBUG_FUNCTION();
        data_nod = data_mgt->get_net_data_node(data_mgt);
        if( !data_nod ) {
            DEBUG_FUNCTION();
            break ;
        }
                
        //TODO : send to net
        
        DEBUG_FUNCTION();

        data_nod->release(data_nod);
        DEBUG_FUNCTION();

    }while(1);

    DEBUG_FUNCTION();


     return;

}

#endif




static int net_task_mgt_run_cb(struct NET_TASK_MGT *task_mgt)
{
    struct NET_TASK *ev_task =NULL;
    struct list_head *ln1, *ln2;
    int ret;

    if( !task_mgt ) {
        return -1;
    }

    pthread_mutex_lock(&task_mgt->lst_lock);

//    if(list_empty(&task_mgt->ev_act_tsk_lst)) {

      
      if( !task_mgt->ev_act_num ){
        
      
        pthread_mutex_unlock(&task_mgt->lst_lock);

        return 0;
    }

    list_for_each_safe(ln1, ln2, &task_mgt->ev_act_tsk_lst){

        ev_task = list_entry(ln1,struct NET_TASK ,entry);
        if( ev_task ) {
            
            if(ev_task->send) {
                ret = ev_task->send(ev_task ,NULL);
                if(ret < 0 ) {
                    list_del_init(&ev_task->entry);
                    --task_mgt->ev_act_num;
                    ev_task->task_mgt = NULL;
                    
#ifdef USE_NET_TASK_ASYNC_RELEASE
                    if(ev_task->async_release) {
                        ev_task->async_release(ev_task);
                    }
#else

                    if(ev_task->release) {
                        
                        ev_task->release(ev_task);
                    }
#endif

                }
            }

        }
        ev_task = NULL;
    }

    
    pthread_mutex_unlock(&task_mgt->lst_lock);

    return 0;

}


static void net_task_mgt_run( void *arg)
{
    int ret;

    struct NET_TASK_MGT *net_task_mgt = (struct NET_TASK_MGT *)arg;

    if( !net_task_mgt ) {

        return;

    }

    reg_net_task_mgt_callback(net_task_mgt ,net_task_mgt_run_cb);

    net_task_mgt->is_run = 1;

    while(net_task_mgt->is_run) {

        if(net_task_mgt->wait) {
            net_task_mgt->wait(net_task_mgt);
        }

       

        if(net_task_mgt->callback) {
            ret = net_task_mgt->callback(net_task_mgt);
            if( ret < 0 ) {
                fprintf(stderr,"net_task_mgt callback err .\n");
                break;
            }
        }
        net_task_mgt->have_ev = 0;
    }
    
    net_task_mgt->is_run = 0;
    net_task_mgt->release(net_task_mgt);

}



static void new_net_task_mgt_run( void *arg)
{
    int ret;

    struct NET_DATA_NODE_T *data_node_p;
    struct NET_DATA_MGT_T *data_mgt;
    struct NET_TASK *net_task;

    struct NET_TASK_MGT *net_task_mgt = (struct NET_TASK_MGT *)arg;

    if( !net_task_mgt ) {

        return;

    }

//    reg_net_task_mgt_callback(net_task_mgt ,net_task_mgt_run_cb);

    net_task_mgt->is_run = 1;

    while(net_task_mgt->is_run) {

        if(net_task_mgt->get_data_node) {

            data_node_p = net_task_mgt->wait_get_data_node(net_task_mgt);
            if( !data_node_p ) {
                continue ;
            }

            if(data_node_p->net_data_mgt) {

                data_mgt = data_node_p->net_data_mgt;

                if(data_mgt->net_task) {

                    net_task = (struct NET_TASK *)data_mgt->net_task;
                    ret = net_task->send(net_task, (void *)data_node_p);
                    if(ret < 0 ) {
                        fprintf( stderr,"net_task->send err .\n");
                        if(net_task->release) {
                            net_task->release(net_task);
                        }
                    }

                }
                
                
            }

            if(data_node_p->release) {
                data_node_p->release(data_node_p);
            }

        }

        
        
    }
    
    net_task_mgt->is_run = 0;
    net_task_mgt->release(net_task_mgt);

}





int net_task_mgt_thr_create(struct NET_TASK_MGT *net_task_mgt)
{
    int ret = -1 ;

   
    
    if(!net_task_mgt->is_run) {

        net_task_mgt->is_run = 1;

#ifdef USE_NEW_DATA_DISPATCH_POLICY

        ret = net_glb_thread_dispatch(new_net_task_mgt_run,(void *)net_task_mgt);
        if( ret < 0 ) {
            net_task_mgt->is_run = 0;
        }

#else

        ret = net_glb_thread_dispatch(net_task_mgt_run,(void *)net_task_mgt);
        if( ret < 0 ) {
            net_task_mgt->is_run = 0;
        }

#endif

    }

    fprintf( stderr,"data thr exit .\n");

    return ret;
}

int net_data_thr_alloc_task(struct NET_TASK *net_task)
{

    int i ,ret ;
    struct NET_TASK_MGT *net_task_mgt ;

    struct ISIL_DATA_THR_MGT *data_thr_mgt  = get_glb_data_thr_mgt();

    if(net_task->task_mgt) {
        assert(0);
    }
    
    for( i = 0 ; i < DEFAULT_NET_DATA_THR_NUM ; i++) {
        net_task_mgt = &data_thr_mgt->net_tasks_mgt[i];
        if(net_task_mgt->ev_act_num > PER_THR_MAX_TASKS) {
            continue ;
        }
        
        if(!net_task_mgt->is_run) {

            ret = net_task_mgt_thr_create(net_task_mgt);
            if( ret < 0 ) {
                fprintf( stderr,"net_task_mgt_thr_create failed .\n");
                return -1;
            }

            net_task_mgt->is_run = 1;
            
            //TODO :must reg callback in here?
      
            
        }

        
        net_task_mgt->put_ev_task_to_act(net_task_mgt,net_task);
        return 0;
         
        
    }

    return -1;

}
















