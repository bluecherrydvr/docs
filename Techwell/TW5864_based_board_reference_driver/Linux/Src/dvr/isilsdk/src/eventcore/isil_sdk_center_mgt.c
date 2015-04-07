#include <stdlib.h>
#include <stdio.h>

#include "isil_sdk_center_mgt.h"
#include "linux_list.h"
#include "isil_sdk_config.h"
#include "isil_msg_mgt.h"
#include "isil_notify.h"
#include "isil_event_mgt.h"
#include "isil_thread_pool.h"
#include "isil_frame.h"
#include "isil_ev_task.h"
#include "isil_debug.h"




static struct ISIL_CENTER_MGT glb_center_mgt;


struct ISIL_CENTER_MGT *get_glb_center_mgt( void )
{
    return &glb_center_mgt;
}

int isil_init_sdk( void )
{
    
    
    struct ISIL_CENTER_MGT *glb_cent_mgt_ptr = get_glb_center_mgt();

    if( !glb_cent_mgt_ptr->is_init ) {

        

        glb_msg_mgt_init();

        glb_task_mgt_init();
        
        

        glb_cent_mgt_ptr->sync_msg_mgt = get_glb_msg_mgt();

        if(create_glb_thread_pool(DEFAULT_THREAD_NUM) < 0){
            fprintf(stderr,"Create glb_thread_pool failed .\n");
            return -1;
        }

        

        
        
        
        if(  init_glb_frame_pool_ex()  < 0 ) {
            fprintf(stderr,"Init glb frame pool failed .\n");
            release_glb_thread_pool();
            return -1;
        }

        

        

        glb_cent_mgt_ptr->frame_pool = get_glb_frame_pool();

        init_global_event_cb(0);
        
       
        glb_cent_mgt_ptr->ev_cb_t = get_glb_ev_cb();

        sem_init( &glb_cent_mgt_ptr->resource_sem , 0 ,0);

        glb_cent_mgt_ptr->is_init = 1;
    }

    return 0;

}


void isil_sdk_release( void )
{
    struct ISIL_CENTER_MGT *glb_cent_mgt_ptr = get_glb_center_mgt();
    struct ISIL_TASK_MGT *glb_task_mgt_ptr = get_glb_task_mgt();
    if( glb_cent_mgt_ptr->is_init ) {
        
        
        

        
        release_glb_thread_pool();

//        sem_wait(&glb_cent_mgt_ptr->resource_sem);
//        release_global_event_cb();
       
        glb_task_mgt_ptr->release(glb_task_mgt_ptr);

        DEBUG_FUNCTION();
        if(glb_cent_mgt_ptr->sync_msg_mgt) {
            if(glb_cent_mgt_ptr->sync_msg_mgt->release) {
                glb_cent_mgt_ptr->sync_msg_mgt->release(glb_cent_mgt_ptr->sync_msg_mgt);
            }
        }
        DEBUG_FUNCTION();
        if(glb_cent_mgt_ptr->frame_pool) {
            if(glb_cent_mgt_ptr->frame_pool->release) {
                glb_cent_mgt_ptr->frame_pool->release(glb_cent_mgt_ptr->frame_pool);
            }
        }
        
        DEBUG_FUNCTION();
        sem_destroy( &glb_cent_mgt_ptr->resource_sem);

        DEBUG_FUNCTION();

        glb_cent_mgt_ptr->is_init = 0;
    }
}
void sdk_thr_exit( void )
{
    fprintf(stderr, "enter %s\n", __FUNCTION__);
    glb_center_mgt.sdk_is_run = 0;
    if(glb_center_mgt.ev_cb_t) {
        EVENT_CB *ev_cb = glb_center_mgt.ev_cb_t;
        if(ev_cb->unloop) {
            ev_cb->unloop(ev_cb);        
        }
    }
    fprintf(stderr, "out %s\n", __FUNCTION__);
}
