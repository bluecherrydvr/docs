#ifndef  _ISIL_EV_TASK_H
#define _ISIL_EV_TASK_H



#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "linux_list.h"
#include "isil_event_mgt.h"
#include "isil_notify.h"
#include "isil_frame.h"


struct ISIL_EV_TASK ;
struct ISIL_TASK_MGT;

typedef void (EV_TASK_CALLBACK)(void *arg ,struct ISIL_EV_TASK *ev_task);

typedef int (TASK_MGT_CALLBACK)(struct ISIL_TASK_MGT *task_mgt); 

typedef void (EV_TASK_PRIV_RELEASE)(struct ISIL_EV_TASK *task_mgt);

struct ISIL_EV_TASK{
    
    struct list_head entry;
    struct ISIL_DATE_MGT *date_mgt; 
    struct ISIL_NOTIFY_LST notify_list;
    struct ISIL_EV_ARG_T *ev;
    struct ISIL_TASK_MGT *task_mgt;
    int fd;
    volatile int ref;
    void *priv;
    EV_TASK_PRIV_RELEASE *priv_release;

    void *param;
    EV_TASK_CALLBACK *hook;
    
    void (*del_from_task_mgt)(struct ISIL_EV_TASK *ev_task);
    void (*release)(struct ISIL_EV_TASK *release);
};


struct ISIL_TASK_MGT{
    struct list_head ev_act_tsk_lst;
    int ev_act_num;
    pthread_mutex_t lock;

    
    volatile int have_ev;
    pthread_mutex_t ev_lock;
    pthread_cond_t  ev_cond;
    

    int alloc_type;
    int is_init;
    int is_run;
    

    unsigned int max_task_num;
   


    TASK_MGT_CALLBACK *callback;

    void (*wait)(struct ISIL_TASK_MGT *task_mgt);
    void (*wakeup)(struct ISIL_TASK_MGT *task_mgt);

    void (*put_ev_task_to_act)(struct ISIL_TASK_MGT *task_mgt,struct ISIL_EV_TASK * ev_task);
    struct ISIL_EV_TASK* (*get_ev_task_from_act_head)(struct ISIL_TASK_MGT *task_mgt);
    void (*release)(struct ISIL_TASK_MGT *task_mgt);
};


#define SET_EV_TASK_FD(x,value) \
    (((x)->fd) =  value)

#define GET_EV_TASK_FD(x) \
    ((x)->fd)

static inline void add_ev_task_ref(struct ISIL_EV_TASK *ev_task)
{
    ++ev_task->ref;
}

static inline void minus_ev_task_ref(struct ISIL_EV_TASK *ev_task)
{
    --ev_task->ref;
}

static inline void reg_ev_task_callback(struct ISIL_EV_TASK *ev_task,void *arg, EV_TASK_CALLBACK *callback)
{
    if(ev_task) {
        ev_task->param = arg;
        ev_task->hook  = callback;
    }
}


static inline void reg_ev_task_priv_release(struct ISIL_EV_TASK *ev_task,EV_TASK_PRIV_RELEASE *pri_release_cb)
{
    if(ev_task) {
        ev_task->priv_release = pri_release_cb;
     }
}



static inline  struct ISIL_EV_ARG_T *get_ev_task_arg(struct ISIL_EV_TASK *ev_task)
{
    return ev_task->ev;
}


static inline void reg_task_mgt_callback(struct ISIL_TASK_MGT *task_mgt,TASK_MGT_CALLBACK *hook)
{
    if(task_mgt) {

        task_mgt->callback = hook;
       
    }
}

static inline void clear_task_mgt_ev(struct ISIL_TASK_MGT *task_mgt)
{
    task_mgt->have_ev = 0;
}

extern void task_mgt_sinit(struct ISIL_TASK_MGT *task_mgt);

extern struct ISIL_TASK_MGT * task_mgt_minit( void );

extern struct ISIL_EV_TASK *ev_task_minit(void);

extern void glb_task_mgt_init(void);

extern struct ISIL_TASK_MGT *get_glb_task_mgt(void);

#ifdef __cplusplus
}
#endif

#endif
