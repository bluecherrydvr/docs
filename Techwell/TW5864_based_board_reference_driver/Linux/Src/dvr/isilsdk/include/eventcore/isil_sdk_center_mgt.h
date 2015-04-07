#ifndef   _ISIL_SDK_CENTER_MGT_H
#define  _ISIL_SDK_CENTER_MGT_H

#include <pthread.h>
#include <semaphore.h> 

#include "linux_list.h"
#include "isil_sdk_config.h"
#include "isil_msg_mgt.h"
#include "isil_notify.h"
#include "isil_event_mgt.h"
#include "isil_thread_pool.h"
#include "isil_frame.h"
#include "isil_ev_task.h"

#ifdef __cplusplus
extern "C" {
#endif






struct ISIL_CENTER_MGT{
    int is_init ;
    int sdk_is_run;
    struct ISIL_MSG_MGT *sync_msg_mgt;
    struct ISIL_FramePool       *frame_pool;
    isil_thread_pool_t *thread_pool;
    EVENT_CB *ev_cb_t;
	sem_t resource_sem;
    
};

#define GET_CENTER_MGT_MSG_MGT(x) \
    ((x)->sync_msg_mgt) 

#define GET_CENTER_MGT_EV_CB(x) \
    ((x)->ev_cb_t)

static inline void set_sdk_run_flag(struct ISIL_CENTER_MGT * cent_mgt ,int flag)
{
    cent_mgt->sdk_is_run = flag;
}

static inline int get_sdk_run_flag(struct ISIL_CENTER_MGT * cent_mgt )
{
    return cent_mgt->sdk_is_run; 
}

extern struct ISIL_CENTER_MGT *get_glb_center_mgt( void );

extern void sdk_thr_exit( void );

extern int isil_init_sdk( void );

extern void isil_sdk_release( void );


#ifdef __cplusplus
}
#endif

#endif


