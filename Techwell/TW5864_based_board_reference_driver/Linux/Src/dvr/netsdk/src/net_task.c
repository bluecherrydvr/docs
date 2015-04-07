#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "net_task.h"
#include "isil_net_config.h"
#include "net_thread_pool.h"
#include "net_debug.h"


static struct NET_TASK_MGT glb_net_task_mgt;

static void net_task_release(struct NET_TASK * ev_task)
{
    if(ev_task) {


        if( !ev_task->ref ) {
            
            return;
        }

        minus_net_task_ref(ev_task);

        

        if(!ev_task->ref) {


            if(ev_task->nty_rec_t) {
                DEBUG_FUNCTION();
                ev_task->nty_rec_t->release(ev_task->nty_rec_t);
                ev_task->nty_rec_t = NULL;
                DEBUG_FUNCTION();
            }
            
            if(ev_task->ev) {
                DEBUG_FUNCTION();
                ev_task->ev->release(ev_task->ev);
                ev_task->ev = NULL;
                DEBUG_FUNCTION();
            }

            
            if( ev_task->net_data_mgt ) {
                DEBUG_FUNCTION();
                ev_task->net_data_mgt->release(ev_task->net_data_mgt);
                ev_task->net_data_mgt->net_task = NULL;
                ev_task->net_data_mgt = NULL;
                DEBUG_FUNCTION();
            }

            if(ev_task->task_mgt) {
                DEBUG_FUNCTION();
                ev_task->del_from_task_mgt(ev_task);
                ev_task->task_mgt = NULL;
                DEBUG_FUNCTION();
                
            }

            
            free_net_task_enc_param(ev_task);


            

            if(ev_task->trans_type == NET_UDP_E) {
                if( ev_task->port ) {
                    if(ev_task->clear_port) {
                        DEBUG_FUNCTION();
                        ev_task->clear_port(ev_task->port);
                        ev_task->clear_port = NULL;
                        DEBUG_FUNCTION();
                    }
                }
            }

            if( ev_task->task_priv_release) {
                
                fprintf(stderr,"Release task_priv_release .\n");
                ev_task->task_priv_release(ev_task);
            }

            if(ev_task->fd > 0) {
                close(ev_task->fd);
                
            }
            
    
            INIT_LIST_HEAD(&ev_task->entry);
        }

        ev_task->release = NULL;

        free(ev_task);
        ev_task = NULL;
        DEBUG_FUNCTION();
    }

}


static void thr_net_task_async_release_cb( void *arg)
{
    struct NET_TASK * ev_task;

    if( !arg ) {
        return;
    }

    fprintf( stderr,"Aync task thr .\n");

    ev_task = (struct NET_TASK *)arg;
    net_task_release(ev_task);

    fprintf( stderr,"Aync task thr exit.\n");

    return;
}


static void net_task_async_release(struct NET_TASK * ev_task)
{
    if( !ev_task ) {
        return;
    }

    net_glb_thread_dispatch(thr_net_task_async_release_cb , ev_task);
}


static void net_del_self_from_task_mgt(struct NET_TASK *ev_task)
{
    struct NET_TASK_MGT *task_mgt;
    if(ev_task) {
        if(ev_task->task_mgt) {
            DEBUG_FUNCTION();
            task_mgt = ev_task->task_mgt;
            pthread_mutex_lock(&task_mgt->lst_lock);
            list_del_init(&ev_task->entry);
            
            --task_mgt->ev_act_num;
            
            pthread_mutex_unlock(&task_mgt->lst_lock);
            
            ev_task->task_mgt = NULL;

        }
    }
}


static void del_net_task_from_task_mgt(struct NET_TASK *ev_task)
{
    struct NET_TASK_MGT *task_mgt;
    
    struct list_head *ln1, *ln2;
    struct NET_TASK *com_task = NULL;

    if(ev_task) {
        if(ev_task->task_mgt) {
            
            task_mgt = ev_task->task_mgt;
            pthread_mutex_lock(&task_mgt->lst_lock);
            
            if( !task_mgt->ev_act_num ) {
                pthread_mutex_unlock(&task_mgt->lst_lock);
                return;
            }

            DEBUG_FUNCTION();
            list_for_each_safe(ln1, ln2, &task_mgt->ev_act_tsk_lst){
                com_task = list_entry(ln1,struct NET_TASK ,entry);
                if( com_task ) {
                    if(ev_task == com_task) {
                        DEBUG_FUNCTION();
                        list_del_init(&com_task->entry);
                        --task_mgt->ev_act_num;
                        ev_task->task_mgt = NULL;
                        break;
                    }
                }
            }
            
            DEBUG_FUNCTION();
            pthread_mutex_unlock(&task_mgt->lst_lock);
            
        }
    }
}



struct NET_TASK *net_ev_task_minit(void)
{
    struct NET_TASK *ev_task =(struct NET_TASK *)calloc( 1 , sizeof(struct NET_TASK));
    if(ev_task) {
        
        INIT_LIST_HEAD(&ev_task->entry);
        
        struct NET_EV_MGT_T *ev_arg = net_ev_arg_t_minit();
        if(!ev_arg) {
            free(ev_task);
            return NULL;
        }
        ev_task->ev = ev_arg;

        

        ev_task->net_data_mgt = net_data_mgt_minit();
        if( !ev_task->net_data_mgt  ) {
            free(ev_task);
            return NULL;
        }

        ev_task->net_data_mgt->net_task = ( void *)ev_task;
        
        ev_task->del_from_task_mgt = del_net_task_from_task_mgt;
        ev_task->release = net_task_release;
        ev_task->async_release = net_task_async_release;
        add_net_task_ref(ev_task);


    }

    return ev_task;
}




static void wait_for_act_list(struct NET_TASK_MGT *task_mgt)
{
    int ret;
    struct timespec timeout;
    if(task_mgt){

        if( !task_mgt->ev_act_num ) {

            pthread_mutex_lock(&task_mgt->lst_lock);
    
    
            timeout.tv_sec = time(NULL) + 10;
            timeout.tv_nsec = 0;
    
            while( !task_mgt->ev_act_num ) {
                
                ret = pthread_cond_timedwait(&task_mgt->lst_cond ,&task_mgt->lst_lock ,&timeout);
                if( ret == ETIMEDOUT) {
                    break;
                }
                
                
            }
    
            pthread_mutex_unlock(&task_mgt->lst_lock);
        }

    }
}

static void wakeup_act_list(struct NET_TASK_MGT *task_mgt)
{
    if(task_mgt){
        pthread_cond_signal(&task_mgt->lst_cond);
    }
}



static void wait_for_ev_task(struct NET_TASK_MGT *task_mgt)
{

    int ret;
    struct timespec timeout;

    if(task_mgt) {
        
        wait_for_act_list(task_mgt);

        pthread_mutex_lock(&task_mgt->lock);
        
        timeout.tv_sec = time(NULL) ; ;//time(NULL) + 1;
        timeout.tv_nsec = NET_MGT_WAIT_TIMEOUT;
        while( !task_mgt->have_ev ) {
            
            ret = pthread_cond_timedwait(&task_mgt->cond ,&task_mgt->lock ,&timeout);
            if( ret == ETIMEDOUT) {
                break;
            }
            
            
        }

        pthread_mutex_unlock(&task_mgt->lock);
    }
}


static void ev_task_wakeup(struct NET_TASK_MGT *task_mgt)
{

    if(task_mgt) {
        if( !task_mgt->have_ev ) {
            task_mgt->have_ev = 1;
            pthread_cond_signal(&task_mgt->cond);
        }
    }
    
}

static void put_net_ev_task_to_act_list(struct NET_TASK_MGT *task_mgt,struct NET_TASK * ev_task)
{
    if( task_mgt && ev_task) {

        pthread_mutex_lock(&task_mgt->lst_lock);
        list_add_tail(&ev_task->entry, &task_mgt->ev_act_tsk_lst);
        ++task_mgt->ev_act_num;

        wakeup_act_list(task_mgt);
#if 0
        if(task_mgt->wakeup) {
            task_mgt->wakeup(task_mgt);
        }
#endif
        ev_task->task_mgt = task_mgt;
//        add_net_task_ref(ev_task);
        pthread_mutex_unlock(&task_mgt->lst_lock);
    }
}


static struct NET_TASK *get_net_ev_task_from_act_lst_head(struct NET_TASK_MGT *task_mgt)
{
    struct NET_TASK *ev_task = NULL;

    if(task_mgt) {

        if(!list_empty(&task_mgt->ev_act_tsk_lst)) {

            pthread_mutex_lock(&task_mgt->lst_lock);
            ev_task = list_entry(task_mgt->ev_act_tsk_lst.next,struct NET_TASK,entry);
            list_del_init(&ev_task->entry);
            --task_mgt->ev_act_num;
            ev_task->task_mgt = NULL;
            
            pthread_mutex_unlock(&task_mgt->lst_lock);

        }
    }

    return ev_task;
}


static void put_data_node_to_send_list(struct NET_TASK_MGT *task_mgt,struct NET_DATA_NODE_T *data_node)
{
    if( task_mgt && data_node) {

        pthread_mutex_lock(&task_mgt->data_send_lock);

        list_add_tail(&data_node->send_entry, &task_mgt->data_send_list);
        ++task_mgt->data_send_num;
        data_node->send_root = (void *)task_mgt;
        net_data_node_clone(data_node);
        if( task_mgt->data_send_num == 1 ) {
            pthread_cond_signal( &task_mgt->data_send_cond );
        }

        pthread_mutex_unlock(&task_mgt->data_send_lock);
    }
}


static struct NET_DATA_NODE_T *get_data_node_from_send_lst(struct NET_TASK_MGT *task_mgt)
{
    struct NET_DATA_NODE_T *data_node = NULL;

    if(task_mgt) {

        if(!list_empty(&task_mgt->data_send_list)) {

            pthread_mutex_lock(&task_mgt->data_send_lock);
            data_node = list_entry(task_mgt->data_send_list.next,struct NET_DATA_NODE_T,send_entry);
            list_del_init(&data_node->send_entry);
            --task_mgt->data_send_num;
            data_node->send_root = NULL;
            net_data_node_minus_ref(data_node);
            
            pthread_mutex_unlock(&task_mgt->data_send_lock);

        }
    }

    return data_node;
}


static struct NET_DATA_NODE_T *wait_get_data_node_from_send_lst(struct NET_TASK_MGT *task_mgt)
{

#ifdef GET_DATA_USE_TIMEOUT
    int ret;
    struct timespec timeout;
#endif

    struct NET_DATA_NODE_T *data_node = NULL;

    if(task_mgt) {

            

        pthread_mutex_lock(&task_mgt->data_send_lock);

        while( !task_mgt->data_send_num ) {
#ifdef GET_DATA_USE_TIMEOUT
            timeout.tv_sec = time(NULL) + 1;
            timeout.tv_nsec = 0;
            ret = pthread_cond_timedwait(&task_mgt->data_send_cond ,&task_mgt->data_send_lock , &timeout);
            if( ret == ETIMEDOUT) {
                pthread_mutex_unlock(&task_mgt->data_send_lock);
                return NULL;
            }

#else
            pthread_cond_wait(&task_mgt->data_send_cond ,&task_mgt->data_send_lock);

#endif
        }

        data_node = list_entry(task_mgt->data_send_list.next,struct NET_DATA_NODE_T,send_entry);
        list_del_init(&data_node->send_entry);
        --task_mgt->data_send_num;
        data_node->send_root = NULL;
        net_data_node_minus_ref(data_node);
        
        pthread_mutex_unlock(&task_mgt->data_send_lock);

        
    }

    return data_node;
}


void del_data_node_self_from_send_lst(struct NET_DATA_NODE_T *data_node)
{
    struct NET_TASK_MGT *task_mgt;
    if(data_node) {
        
        if(data_node->send_root) {

            task_mgt = (struct NET_TASK_MGT *)data_node->send_root;
            pthread_mutex_lock(&task_mgt->lst_lock);

            list_del_init(&data_node->send_entry);
            --task_mgt->data_send_num;
            data_node->send_root = NULL;
            net_data_node_minus_ref(data_node);
            pthread_mutex_unlock(&task_mgt->lst_lock);

        }
    }
}



static void ev_act_tsk_lst_release(struct NET_TASK_MGT *task_mgt)
{
    struct list_head *ln1, *ln2;
    struct NET_TASK *ev_task = NULL;

    if(task_mgt) {
        if(!list_empty(&task_mgt->ev_act_tsk_lst)){

            pthread_mutex_lock(&task_mgt->lst_lock);

            list_for_each_safe(ln1, ln2, &task_mgt->ev_act_tsk_lst ){

                ev_task = list_entry(ln1 ,struct NET_TASK , entry);

            
                if(ev_task) {

                    list_del_init(&ev_task->entry);
                    --task_mgt->ev_act_num;
                    ev_task->task_mgt = NULL;

                    if( ev_task->release ) {
                        ev_task->release(ev_task);
                    }
                }
            }

            pthread_mutex_unlock(&task_mgt->lst_lock);
        }
    }
}



static void data_send_lst_release(struct NET_TASK_MGT *task_mgt)
{
    struct list_head *ln1, *ln2;
    struct NET_DATA_NODE_T *data_node = NULL;

    if(task_mgt) {
        if(!list_empty(&task_mgt->data_send_list)){

            pthread_mutex_lock(&task_mgt->data_send_lock);
            

            list_for_each_safe(ln1, ln2, &task_mgt->data_send_list ){

                data_node = list_entry(ln1 ,struct NET_DATA_NODE_T , send_entry);

                if(data_node) {
                    list_del_init(&data_node->send_entry);
                    --task_mgt->data_send_num;
                    data_node->send_root = NULL;
                    net_data_node_minus_ref(data_node);
                    if( data_node->release ) {
                        data_node->release(data_node);
                    }
                }
            }

            pthread_mutex_unlock(&task_mgt->data_send_lock);
        }
    }
}



static void net_task_mgt_release(struct NET_TASK_MGT *task_mgt)
{
    
    if(task_mgt) {

        ev_act_tsk_lst_release(task_mgt);

        data_send_lst_release(task_mgt);
        
        pthread_mutex_destroy(&task_mgt->lst_lock);
        pthread_cond_destroy(&task_mgt->lst_cond);

        pthread_mutex_destroy(&task_mgt->lock);
        pthread_cond_destroy(&task_mgt->cond);


        pthread_mutex_destroy(&task_mgt->data_send_lock);
        pthread_cond_destroy(&task_mgt->data_send_cond);
        INIT_LIST_HEAD(&task_mgt->data_send_list);

        if(task_mgt->alloc_type == NET_DYNAMIC_ALLOC_TYPE) {
            free(task_mgt);
            task_mgt = NULL;
        }
    }
}


static void net_task_mgt_init(struct NET_TASK_MGT *task_mgt)
{
    if(task_mgt) {

        if(!task_mgt->is_init) {
            
            pthread_mutex_init(&task_mgt->lst_lock,NULL);
            pthread_cond_init(&task_mgt->lst_cond,NULL);

            pthread_mutex_init(&task_mgt->lock,NULL);
            pthread_cond_init(&task_mgt->cond,NULL);
            INIT_LIST_HEAD(&task_mgt->ev_act_tsk_lst);
            task_mgt->ev_act_num = 0;

            pthread_mutex_init(&task_mgt->data_send_lock,NULL);
            pthread_cond_init(&task_mgt->data_send_cond,NULL);
            INIT_LIST_HEAD(&task_mgt->data_send_list);
            task_mgt->data_send_num = 0;

            
            task_mgt->have_ev = 0 ;
//            task_mgt->alloc_type = ISIL_STATIC_ALLOC_TYPE;

            task_mgt->callback = NULL;

            task_mgt->wait = wait_for_ev_task;
            task_mgt->wakeup = ev_task_wakeup;

            task_mgt->get_data_node = get_data_node_from_send_lst;
            task_mgt->wait_get_data_node = wait_get_data_node_from_send_lst;
            task_mgt->put_data_node = put_data_node_to_send_list;

            task_mgt->get_ev_task_from_act_head = get_net_ev_task_from_act_lst_head;
            task_mgt->put_ev_task_to_act = put_net_ev_task_to_act_list;

            task_mgt->release = net_task_mgt_release;
            task_mgt->is_init = 1;

        }

    }
}


void net_task_mgt_sinit(struct NET_TASK_MGT *task_mgt)
{

    net_task_mgt_init(task_mgt);
    task_mgt->alloc_type = NET_STATIC_ALLOC_TYPE;

}


struct NET_TASK_MGT * net_task_mgt_minit( void )
{
    struct NET_TASK_MGT * task_mgt = (struct NET_TASK_MGT *)calloc( 1 ,sizeof(struct NET_TASK_MGT));
    if( !task_mgt ) {
        return NULL;
    }

    net_task_mgt_init(task_mgt);
    task_mgt->alloc_type = NET_DYNAMIC_ALLOC_TYPE;

    return task_mgt;
}


void glb_net_task_mgt_init(void)
{
    net_task_mgt_sinit(&glb_net_task_mgt);
}


struct NET_TASK_MGT *get_glb_net_task_mgt(void)
{
    return &glb_net_task_mgt;
}


void alloc_enc_param_t_to_net_task(struct NET_TASK *ev_task)
{
    struct h264_param_t *h264_param_ptr;
    
    if( !ev_task ) {
        return;
    }

    if( !ev_task->enc_param ) {
        h264_param_ptr = (struct h264_param_t *)calloc( 1 , sizeof(struct h264_param_t));
        if( !h264_param_ptr ) {
            return;
        }
        
        ev_task->enc_param = (void *)h264_param_ptr;
    }

    return;
}


void free_net_task_enc_param(struct NET_TASK *ev_task)
{
    if(ev_task) {
        if(ev_task->enc_param) {
            free(ev_task->enc_param);
            ev_task->enc_param = NULL;
        }
    }
}
