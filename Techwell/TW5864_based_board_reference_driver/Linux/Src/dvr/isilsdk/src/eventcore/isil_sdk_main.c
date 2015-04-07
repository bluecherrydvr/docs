#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <semaphore.h> 
#include <unistd.h>

#include "isil_sdk_main.h"
#include "isil_msg_mgt.h"
#include "isil_notify.h"
#include "isil_event_mgt.h"
#include "isil_thread_pool.h"
#include "isil_sdk_center_mgt.h"
#include "isil_sdk_config.h"
#include "isil_debug.h"



static void isil_sigIntHandler( int fd, short flag, void * arg )
{
    struct ISIL_CENTER_MGT * center_mgt;
    center_mgt = get_glb_center_mgt();
   
//    glb_enc_thr_mgt_release();
    set_sdk_run_flag(center_mgt , 0);
    
}

static void isil_sigTermHandler( int fd, short flag, void * arg )
{
    struct ISIL_CENTER_MGT * center_mgt;
    center_mgt = get_glb_center_mgt();
    
    set_sdk_run_flag(center_mgt , 0);
    
}


static void isil_sdk_run_callback(void * arg)
{
    
    struct ISIL_EV_ARG_T evSigInt;
    struct ISIL_EV_ARG_T  evSigTerm;
    
    struct ISIL_CENTER_MGT * center_mgt;
    EVENT_CB *glb_ev_cb_ptr;
    struct ISIL_MSG_MGT *msg_mgt_ptr;

    
    /*init must be before run ,because testing ,put it here*/
    
    center_mgt = get_glb_center_mgt();
    glb_ev_cb_ptr = GET_CENTER_MGT_EV_CB(center_mgt);

    msg_mgt_ptr = GET_CENTER_MGT_MSG_MGT(center_mgt);
    
    
    init_static_ev_arg_t(&evSigInt);
    init_static_ev_arg_t(&evSigTerm);
    reg_ev_to_loop( &evSigInt ,SIGINT, EVENT_SIGNAL ,NULL , isil_sigIntHandler ,NULL);
    reg_ev_to_loop( &evSigInt ,SIGTERM, EVENT_SIGNAL ,NULL , isil_sigTermHandler ,NULL);

    set_sdk_run_flag(center_mgt , 1);
    
    while( get_sdk_run_flag(center_mgt) ) {

        glb_ev_cb_ptr->loop(glb_ev_cb_ptr);
        
        
        //TODO: msg or loop in one thread?
        msg_mgt_ptr->notify(msg_mgt_ptr);
    }

    
    fprintf(stderr,"exit main.\n");

    evSigInt.release(&evSigInt);
    evSigTerm.release(&evSigTerm);

//    sem_post( &center_mgt->resource_sem);

//    release_global_event_cb();

   

}


int isil_sdk_run(void)
{

    
#ifdef SDKRUNUSETHREAD

    if(glb_thread_dispatch(isil_sdk_run_callback ,NULL) < 0){
        
        fprintf(stderr,"isil_sdk_run failed .\n");
        return -1;
    }

#else
    
    isil_sdk_run_callback(NULL);

#endif

    

    return 0;
}
