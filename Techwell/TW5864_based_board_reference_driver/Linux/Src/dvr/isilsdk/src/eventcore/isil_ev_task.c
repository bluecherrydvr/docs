#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "isil_ev_task.h"
#include "isil_sdk_config.h"
#include "isil_debug.h"


static struct ISIL_TASK_MGT glb_task_mgt;

static void ev_task_release(struct ISIL_EV_TASK * ev_task)
{

    struct ISIL_EV_ARG_T *ev_arg;
    if(ev_task) {
        
        if( !ev_task->ref ) {
            DEBUG_FUNCTION();
            return;
        }

        minus_ev_task_ref(ev_task);

        if(!ev_task->ref){ 
    
                
    
    
            
    
            if(ev_task->task_mgt) {
                ev_task->del_from_task_mgt(ev_task);
                ev_task->task_mgt = NULL;
            }
    
            
    
            
            if(ev_task->ev) {
                
                ev_arg = ev_task->ev;
                if(ev_arg->release) {
                    
                    ev_arg->release(ev_arg);
                    
                }
                
                ev_task->ev = NULL;
            }
    
           
    
            if(ev_task->priv_release) {
                
                ev_task->priv_release(ev_task);
                ev_task->priv_release = NULL;
    
                ev_task->priv = NULL;
                
            }
    
            
    
            if(ev_task->date_mgt) {
                ev_task->date_mgt->release(ev_task->date_mgt);
                ev_task->date_mgt = NULL;
            }
    
            
    
            ev_task->notify_list.release(&ev_task->notify_list);
        
            INIT_LIST_HEAD(&ev_task->entry);

            free(ev_task);
            ev_task = NULL;
    
        }
        
        
        
        
        
    }

}


static void del_self_from_task_mgt(struct ISIL_EV_TASK *ev_task)
{
    if(ev_task) {
        if(ev_task->task_mgt) {

            pthread_mutex_lock(&ev_task->task_mgt->lock);
            list_del_init(&ev_task->entry);
            --ev_task->task_mgt->ev_act_num;
            minus_ev_task_ref(ev_task);
            pthread_mutex_unlock(&ev_task->task_mgt->lock);
            ev_task->task_mgt = NULL;

        }
    }
}



struct ISIL_EV_TASK *ev_task_minit(void)
{
    struct ISIL_EV_TASK *ev_task =(struct ISIL_EV_TASK *)calloc( 1 , sizeof(struct ISIL_EV_TASK));
    if(ev_task) {
        
        INIT_LIST_HEAD(&ev_task->entry);
        
        struct ISIL_EV_ARG_T *ev_arg = ev_arg_t_minit();
        if(!ev_arg) {
            free(ev_task);
            ev_task = NULL;
            return NULL;
        }

        ev_task->ev = ev_arg;

        ev_task->date_mgt = date_mgt_minit();
        if( !ev_task->date_mgt ) {
            ev_task->ev->release(ev_arg);
            ev_task->ev = NULL;
            return NULL;
        }

        notify_lst_sinit(&ev_task->notify_list);
        ev_task->del_from_task_mgt = del_self_from_task_mgt;
        ev_task->release = ev_task_release;
        add_ev_task_ref(ev_task);
    }

    return ev_task;
}




static void put_ev_task_to_act_list(struct ISIL_TASK_MGT *task_mgt,struct ISIL_EV_TASK * ev_task)
{
    if( task_mgt && ev_task) {

        pthread_mutex_lock(&task_mgt->lock);
        list_add_tail(&ev_task->entry, &task_mgt->ev_act_tsk_lst);
        ++task_mgt->ev_act_num;
        ev_task->task_mgt = task_mgt;
        add_ev_task_ref(ev_task);
        pthread_mutex_unlock(&task_mgt->lock);
    }
}


static struct ISIL_EV_TASK *get_ev_task_from_act_lst_head(struct ISIL_TASK_MGT *task_mgt)
{
    struct ISIL_EV_TASK *ev_task = NULL;

    if(task_mgt) {

        if(!list_empty(&task_mgt->ev_act_tsk_lst)) {

            pthread_mutex_lock(&task_mgt->lock);
            ev_task = list_entry(task_mgt->ev_act_tsk_lst.next,struct ISIL_EV_TASK,entry);
            list_del_init(&ev_task->entry);
            --task_mgt->ev_act_num;
            ev_task->task_mgt = NULL;
            minus_ev_task_ref(ev_task);
            pthread_mutex_unlock(&task_mgt->lock);
        }
    }

    return ev_task;
}


static void wait_for_task_ev(struct ISIL_TASK_MGT *task_mgt)
{

    int ret;
    struct timespec timeout;

    if(task_mgt) {
        
        pthread_mutex_lock(&task_mgt->ev_lock);
        
        timeout.tv_sec = time(NULL) + 1;
        timeout.tv_nsec = 0;
        while( !task_mgt->have_ev ) {
            ret = pthread_cond_timedwait(&task_mgt->ev_cond ,&task_mgt->ev_lock ,&timeout);
            if( ret == ETIMEDOUT) {
                break;
            }
        }

        pthread_mutex_unlock(&task_mgt->ev_lock);
    }
}


static void task_ev_wakeup(struct ISIL_TASK_MGT *task_mgt)
{

    if(task_mgt) {
        
        if(task_mgt->have_ev == 0) {
            task_mgt->have_ev = 1;
            pthread_cond_signal(&task_mgt->ev_cond);
        }
    }
    
}



static void task_mgt_release(struct ISIL_TASK_MGT *task_mgt)
{
    
    struct list_head *ln1, *ln2;
    struct ISIL_EV_TASK *ev_task = NULL;

    if(task_mgt) {

        if(!list_empty(&task_mgt->ev_act_tsk_lst)){

            pthread_mutex_lock(&task_mgt->lock);

            list_for_each_safe(ln1, ln2, &task_mgt->ev_act_tsk_lst){
    
                ev_task = list_entry(ln1,struct ISIL_EV_TASK ,entry);

                if(ev_task) {
                    
                    list_del_init(&ev_task->entry);
                    --task_mgt->ev_act_num;
                    minus_ev_task_ref(ev_task);
                    ev_task->task_mgt = NULL;
                   
                    if(ev_task->release) {
                        ev_task->release(ev_task);
                    }
        
                }

            }

            

            pthread_mutex_unlock(&task_mgt->lock);
        }


       

        pthread_mutex_destroy(&task_mgt->lock);

        pthread_mutex_destroy(&task_mgt->ev_lock);

        pthread_cond_destroy(&task_mgt->ev_cond);

        if(task_mgt->alloc_type == ISIL_DYNAMIC_ALLOC_TYPE) {
            free(task_mgt);
            task_mgt = NULL;
        }
       
    }
}


static void task_mgt_init(struct ISIL_TASK_MGT *task_mgt)
{
    if(task_mgt) {

        if(!task_mgt->is_init) {

            pthread_mutex_init(&task_mgt->lock,NULL);
            INIT_LIST_HEAD(&task_mgt->ev_act_tsk_lst);

            pthread_mutex_init(&task_mgt->ev_lock,NULL);
            pthread_cond_init(&task_mgt->ev_cond,NULL);

            INIT_LIST_HEAD(&task_mgt->ev_act_tsk_lst);

            task_mgt->ev_act_num = 0;
            task_mgt->have_ev    = 0;
            task_mgt->max_task_num = PER_ENC_THR_MAX_TASKS;

            task_mgt->callback = NULL;

            task_mgt->wait = wait_for_task_ev;
            task_mgt->wakeup = task_ev_wakeup;

//            task_mgt->alloc_type = ISIL_STATIC_ALLOC_TYPE;
            task_mgt->get_ev_task_from_act_head = get_ev_task_from_act_lst_head;
            task_mgt->put_ev_task_to_act = put_ev_task_to_act_list;
            task_mgt->release = task_mgt_release;
            task_mgt->is_init = 1;
        }

    }
}


void task_mgt_sinit(struct ISIL_TASK_MGT *task_mgt)
{
    task_mgt_init(task_mgt);
    task_mgt->alloc_type = ISIL_STATIC_ALLOC_TYPE;

}


struct ISIL_TASK_MGT * task_mgt_minit( void )
{
    struct ISIL_TASK_MGT * task_mgt = (struct ISIL_TASK_MGT *)calloc( 1 ,sizeof(struct ISIL_TASK_MGT));
    if( !task_mgt ) {
        return NULL;
    }
    task_mgt_init(task_mgt);
    task_mgt->alloc_type = ISIL_DYNAMIC_ALLOC_TYPE;

    return task_mgt;
}


void glb_task_mgt_init(void)
{
    task_mgt_sinit(&glb_task_mgt);
}


struct ISIL_TASK_MGT *get_glb_task_mgt(void)
{
    return &glb_task_mgt;
}
