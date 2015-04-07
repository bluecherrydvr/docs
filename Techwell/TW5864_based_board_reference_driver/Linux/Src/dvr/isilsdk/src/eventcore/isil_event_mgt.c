#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <event.h>

#include"isil_event_mgt.h"
#include "isil_debug.h"

static EVENT_CB isil_ev_cb;

/*
 *Function Name:init_event_cb
 *
 *Parameters:EVENT_CB * int
 *
 *Description:³õÊ¼»¯ event_cb
 *
 *Returns: None
 *
 */


static int isil_ev_loop (EVENT_CB *ev_cb)
{

    int ret;
    ret = event_base_loop(ev_cb->evbase ,EVENT_LOOP_ONCE |EVENT_LOOP_NONBLOCK);
    if( ret < 0) {
        fprintf(stderr,"isil_ev_loop err .\n");
        return -1;
    }
    else if( ret == 1) {
        usleep(2000);           
    }
    else{
        usleep(2000);
    }

    return 0;

}


static int isil_ev_loop_exit(EVENT_CB *ev_cb)
{
    int ret = 0;

    if(ev_cb) {
        if(ev_cb->evbase) {
        
            ret = event_base_loopbreak(ev_cb->evbase);
            if( ret < 0) {
                fprintf( stderr,"event_base_loopbreak err .\n");

            }
            else{
                fprintf( stderr,"event_base_loopbreak ok .\n");
            }
        }
    }

    return 0;
}

void init_event_cb(EVENT_CB *event_cb,int timeout)
{
    
    if( event_cb ) {

        if(!event_cb->is_init) {

            event_cb->evbase = (struct event_base*)event_init();
            
            event_cb->timeout = timeout;
            if(timeout > 0) {
                /*set event timeout*/
            }
        
            event_cb->loop = isil_ev_loop;
            event_cb->unloop = isil_ev_loop_exit;
            event_cb->is_init = 1;
        }
        
    }
    return;
}

void init_global_event_cb(int timeout)
{
    init_event_cb(&isil_ev_cb,timeout);
    return;
}

void release_global_event_cb( void )
{
    if(get_glb_ev_cb()->evbase) {
        event_base_loopexit(get_glb_ev_cb()->evbase,NULL);
//        event_base_free(get_glb_ev_cb()->evbase);
    }
}


EVENT_CB * get_glb_ev_cb(void)
{
     return &isil_ev_cb;
}



static void ev_arg_t_release(struct ISIL_EV_ARG_T *EV_ARG)
{
    if( EV_ARG ) {

        if(EV_ARG->is_active) {
            //EV_DEL(EV_ARG);
            EV_ARG->is_active = 0 ;
            
           
            if(EV_ARG->alloc_type == ISIL_DYNAMIC_ALLOC_TYPE) {
                free(EV_ARG);
                EV_ARG = NULL;
            }
            
        }
    }
    
    
    // TODO: close? ,free?
}

void reg_ev_to_loop(struct ISIL_EV_ARG_T *EV_ARG ,
                                  int fd ,
                                  short event_type,
                                  struct timeval* tv ,
                                  EVHOOK *evcb ,
                                  void *arg)
{

    if( !EV_ARG || !EV_ARG->evBase) {
        return;
    }

    EV_ARG->arg = arg ;
    //TODO: add EVHOOK to EV_ARG
    EV_ARG->callback = evcb ;

    struct event *event_ptr = GET_S_EVENT(EV_ARG);
    if( event_type & (EVENT_READ |  EVENT_WRITE )){
        EV_SET(event_ptr , fd ,event_type ,evcb ,arg);
        EV_BASE_SET(EV_ARG->evBase ,event_ptr);
        EV_ADD(event_ptr , tv);
    }
    else if(event_type & EVENT_TIMEOUT ){
        EV_TIME_SET(event_ptr ,evcb ,arg);
        EV_BASE_SET(EV_ARG->evBase ,event_ptr);
        EV_TIME_ADD(event_ptr , tv);
    }
    else if( (event_type & EVENT_SIGNAL) ){
        EV_SIG_SET(event_ptr,fd , evcb ,arg);
        EV_BASE_SET(EV_ARG->evBase ,event_ptr);
        EV_SIG_ADD(event_ptr,NULL);
    }
    else{
        fprintf(stderr,"ev unkown .\n");
        return;
    }

    EV_ARG->is_active = 1;
}


struct ISIL_EV_ARG_T *ev_arg_t_minit( void )
{
    struct ISIL_EV_ARG_T * arg_t = (struct ISIL_EV_ARG_T *)calloc( 1 ,sizeof(struct ISIL_EV_ARG_T));
    if( !arg_t ) {
        return NULL;
    }

    arg_t->evBase = get_global_ev_base();
    arg_t->alloc_type = ISIL_DYNAMIC_ALLOC_TYPE;
    arg_t->release = ev_arg_t_release;
    return arg_t;
}

void init_static_ev_arg_t(struct ISIL_EV_ARG_T *arg_t)
{
    memset( arg_t  ,0x00 ,sizeof(*arg_t));
    arg_t->alloc_type = ISIL_STATIC_ALLOC_TYPE;
    arg_t->evBase = get_global_ev_base();
    arg_t->release = ev_arg_t_release;
}

















