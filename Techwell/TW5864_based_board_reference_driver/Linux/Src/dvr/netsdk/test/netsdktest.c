#include <stdio.h>
#include <ev.h>


#include "isil_net_sdk.h"
#include "net_task.h"
#include "net_event_mgt.h"
#include "isil_net_config.h"
#include "isil_data_thread_mgt.h"
#include "net_thread_pool.h"
#include "net_debug.h"


static void sig_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    if(loop) {
        DEBUG_FUNCTION();
        ev_break (loop, EVBREAK_ALL);
    }
}



int main(int argc ,char *argv[])
{
    
    int ret;

    NET_EV_CB* ev_cb;

    ev_signal exitsig;
    ev_signal termsig;
    
    isil_net_sdk_init();

    ev_cb = get_glb_net_ev_cb();

    ev_signal_init (&exitsig, sig_cb, SIGINT);
    ev_signal_start (ev_cb->evbase, &exitsig);
    ev_unref (ev_cb->evbase);

    ev_signal_init (&termsig, sig_cb, SIGTERM);
    ev_signal_start (ev_cb->evbase, &termsig);
    ev_unref (ev_cb->evbase);


    

//    ev_mgt = net_ev_arg_t_minit();
//    reg_net_ev_to_loop(ev_mgt,0,NET_EV_READ,NULL,stdin_cb,NULL);
    ret = isil_net_sdk_run(NULL);
    if(ret < 0) {
        fprintf(stderr,"isil_net_sdk_run failed .\n");
        return -1;
    }

    
    ev_cb->loop(ev_cb);


    ev_ref (ev_cb->evbase);
    ev_signal_stop (ev_cb->evbase, &exitsig);


//    ev_mgt->release(ev_mgt);
    isil_net_sdk_cleanup();
    
    return 0;
}
