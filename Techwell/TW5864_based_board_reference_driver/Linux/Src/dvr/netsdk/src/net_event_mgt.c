#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ev.h>
#include <time.h>

#include"net_event_mgt.h"
#include "isil_net_config.h"
#include "net_debug.h"

static NET_EV_CB glb_net_ev_cb;

/*
 *Function Name:init_NET_EV_CB
 *
 *Parameters:NET_EV_CB * int
 *
 *Description: NET_EV_CB
 *
 *Returns: None
 *
 */


static int net_ev_loop (NET_EV_CB *ev_cb)
{
    DEBUG_FUNCTION();
    ev_loop(ev_cb->evbase ,0);
    DEBUG_FUNCTION();
    return 0;

}

void init_net_ev_cb(NET_EV_CB *net_ev_cb,int timeout)
{
    
    if( net_ev_cb ) {

        if(!net_ev_cb->is_init) {

            net_ev_cb->evbase = (struct ev_loop*)ev_loop_new(0);
            if( !net_ev_cb->evbase ) {
                fprintf(stderr,"ev_loop_new failed .\n");
                return;
            }
            net_ev_cb->timeout = timeout;
            if(timeout > 0) {
                /*set event timeout*/
            }
        
            net_ev_cb->loop = net_ev_loop;
            net_ev_cb->is_init = 1;
        }
        
    }
    return;
}

void init_global_net_ev_cb(int timeout)
{
    init_net_ev_cb(&glb_net_ev_cb,timeout);
    return;
}

void release_global_net_ev_cb( void )
{
    if(get_glb_net_ev_base()) {
        ev_loop_destroy (get_glb_net_ev_base());
//        event_base_free(get_glb_ev_cb()->evbase);
    }
}


NET_EV_CB * get_glb_net_ev_cb(void)
{
     return &glb_net_ev_cb;
}



static void net_ev_arg_t_release(struct NET_EV_MGT_T *ev_arg)
{
    if( ev_arg ) {
        ev_io_stop(ev_arg->evBase ,&ev_arg->s_event);
        ev_arg->is_active = 0 ;
        
        if(ev_arg->alloc_type == NET_DYNAMIC_ALLOC_TYPE) {
            free(ev_arg);
            ev_arg = NULL;
        }
    }
    
    
    // TODO: close? ,free?
}

void reg_net_ev_to_loop(struct NET_EV_MGT_T *ev_arg ,
                                  int fd ,
                                  short event_type,
                                  struct timeval* tv ,
                                  NETEVHOOK *evcb ,
                                  void *arg)
{

    if( !ev_arg || !ev_arg->evBase) {
        return;
    }

    ev_arg->arg = arg ;
    //TODO: add EVHOOK to EV_ARG
    ev_arg->callback = evcb ;

    ev_io *event_ptr = &ev_arg->s_event;

    if(event_type & (NET_EV_READ| NET_EV_WRITE )){
        ev_io_init(event_ptr ,evcb,fd ,event_type );
        ev_io_start (ev_arg->evBase, event_ptr);
    }
     else{
        fprintf(stderr,"ev unkown .\n");
        return;
    }

    ev_arg->is_active = 1;
}


struct NET_EV_MGT_T *net_ev_arg_t_minit( void )
{
    struct NET_EV_MGT_T * arg_t = (struct NET_EV_MGT_T *)calloc( 1 ,sizeof(struct NET_EV_MGT_T));
    if( !arg_t ) {
        return NULL;
    }

    arg_t->evBase = get_glb_net_ev_base();
    arg_t->alloc_type = NET_DYNAMIC_ALLOC_TYPE;
    arg_t->release = net_ev_arg_t_release;
    return arg_t;
}

void init_static_net_ev_arg_t(struct NET_EV_MGT_T *arg_t)
{
    memset( arg_t  ,0x00 ,sizeof(*arg_t));
    arg_t->alloc_type = NET_STATIC_ALLOC_TYPE;
    arg_t->evBase = get_glb_net_ev_base();
    arg_t->release = net_ev_arg_t_release;
}

















