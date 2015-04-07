#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>

#include <ev.h>

#include "isil_net_sdk.h"
#include "net_task.h"
#include "net_event_mgt.h"
#include "io_utils.h"
#include "isil_net_config.h"
#include "isil_data_thread_mgt.h"
#include "net_thread_pool.h"
#include "net_debug.h"
#include "net_parse_reg.h"


#define NETBUFFSIZE (6114)

static char net_buff[NETBUFFSIZE];


static ISIL_NET_SDK_MGT glb_net_sdk_mgt;


static void init_net_sdk_mgt( void )
{
    sem_init( &glb_net_sdk_mgt.net_sdk_sem , 0 ,0);
}


typedef void (NET_LISTEN_EV_CB)(struct ev_loop *loop,ev_io *ev,int revents);


void isil_net_sdk_init(void)
{
    init_global_net_ev_cb(0);
    glb_net_task_mgt_init();
    glb_data_thr_mgt_init();
    net_create_glb_thread_pool(DEFAULT_NET_DATA_THR_NUM);
    init_net_sdk_mgt();
    
}


void isil_net_sdk_cleanup(void)
{

    struct NET_TASK_MGT *glb_net_task_mgt;
    
    DEBUG_FUNCTION();
    sem_wait( &glb_net_sdk_mgt.net_sdk_sem);
    DEBUG_FUNCTION();
    
    
    glb_net_task_mgt = get_glb_net_task_mgt();
    glb_net_task_mgt->release(glb_net_task_mgt);
    DEBUG_FUNCTION();
    glb_data_thr_mgt_release();
    DEBUG_FUNCTION();
    
    
    release_global_net_ev_cb();
    DEBUG_FUNCTION();
    //net_release_glb_thread_pool();

    DEBUG_FUNCTION();
    sem_destroy( &glb_net_sdk_mgt.net_sdk_sem);
}


static int net_echo_test(int len ,void *buff ,struct NET_TASK *ev_task)
{
    int ret;

    if(buff && len) {
        DEBUG_FUNCTION();
        ret = write(ev_task->fd , buff , len);
        if( ret < 0) {
            fprintf(stderr,"net_echo_test write err .\n");
            if(ev_task->release) {
                DEBUG_FUNCTION();
                ev_task->release(ev_task);
            }
            return -1;
        }

        if(ev_task->task_mgt) {
            if(ev_task->task_mgt->wakeup) {
                ev_task->task_mgt->wakeup(ev_task->task_mgt);
            }
        }
    }

    return 0;
}


static void isil_ctl_client_read_ev_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    int ret ,len;
    struct NET_EV_MGT_T *ev_arg;
    struct NET_TASK *client_task;

    ev_arg = isil_container_of(ev,struct NET_EV_MGT_T,s_event);
    client_task = (struct NET_TASK *)(ev_arg->arg);
    assert(client_task);

    
    if(revents & NET_EV_READ){

        ret = r_read(client_task->fd ,(void *)net_buff ,NETBUFFSIZE);
        if( ret < 0) {
            fprintf(stderr,"isil_client_read_ev_cb err .\n");
            perror("NET:");
            ev_io_stop(loop,ev);
            
                DEBUG_FUNCTION();
#ifdef USE_NET_TASK_ASYNC_RELEASE
                if(client_task->async_release) {
                    client_task->async_release(client_task);
                }
#else
                if(client_task->release) {
                    client_task->release(client_task);
                }
#endif
            
            return ;
        }
        else if( ret == 0) {
            fprintf(stderr,"client break .\n");
            
            ev_io_stop(loop,ev);
#ifdef USE_NET_TASK_ASYNC_RELEASE
                if(client_task->async_release) {
                    client_task->async_release(client_task);
                }
#else
            if(client_task->release) {
                DEBUG_FUNCTION();
                client_task->release(client_task);
            }
#endif
            return ;
        }

        len = ret;

        if(client_task->hook) {
            
            ret = client_task->hook(len,(void *)net_buff ,client_task);
            if( ret < 0) {
#ifdef USE_NET_TASK_ASYNC_RELEASE
                if(client_task->async_release) {
                    client_task->async_release(client_task);
                }
#else
                if(client_task->release) {
                    DEBUG_FUNCTION();
                    client_task->release(client_task);
                }
#endif
                return ;
            }
        }

    }

}


#define ASYNC_RECV_BUFF_SIZE (64)

static void isil_aysnc_client_read_ev_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    int ret ,len;
    char buff[ASYNC_RECV_BUFF_SIZE];
    struct NET_EV_MGT_T *ev_arg;
    struct NET_TASK *client_task;

    ev_arg = isil_container_of(ev,struct NET_EV_MGT_T,s_event);
    client_task = (struct NET_TASK *)(ev_arg->arg);
    assert(client_task);

    
    if(revents & NET_EV_READ){

        ret = r_read(client_task->fd ,(void *)buff ,ASYNC_RECV_BUFF_SIZE);
        if( ret < 0) {
            fprintf(stderr,"isil_client_read_ev_cb err .\n");
            perror("NET:");
            ev_io_stop(loop,ev);
            
                DEBUG_FUNCTION();
#ifdef USE_NET_TASK_ASYNC_RELEASE
                if(client_task->async_release) {
                    client_task->async_release(client_task);
                }
#else
                if(client_task->release) {
                    client_task->release(client_task);
                }
#endif
            
            return ;
        }
        else if( ret == 0) {
            fprintf(stderr,"client break .\n");
            ev_io_stop(loop,ev);
#ifdef USE_NET_TASK_ASYNC_RELEASE
                client_task->async_release(client_task);
#else
            if(client_task->release) {
                DEBUG_FUNCTION();
                client_task->release(client_task);
            }
#endif
            return ;
        }

        len = ret;

        if(client_task->hook) {
            
            ret = client_task->hook(len,(void *)buff ,client_task);
            if( ret < 0) {
#ifdef USE_NET_TASK_ASYNC_RELEASE
                if(client_task->async_release) {
                    client_task->async_release(client_task);
                }
#else
                if(client_task->release) {
                    DEBUG_FUNCTION();
                    client_task->release(client_task);
                }
#endif
                return ;
            }
        }

    }

}


static int set_ctl_client_task(int fd)
{
    struct NET_TASK * client_task = net_ev_task_minit();
    if( !client_task ) {
        fprintf(stderr,"net_ev_task_minit  client_task failed .\n");
        return -1;
    }

    set_noblock(fd);
    SET_NET_TASK_FD(client_task,fd);
    SET_NET_TASK_TRANS_TYPE(client_task,NET_TCP_E);
    DEBUG_FUNCTION();
    // TODO :reg client_task callback function

    //ECHO server for test
    net_parse_reg_t *glb_net_ret_ptr = get_glb_net_parse_reg_t();

    reg_net_task_callback(client_task ,NULL ,glb_net_ret_ptr->net_ctl_cb);

    reg_net_ev_to_loop(client_task->ev ,
                       fd,
                       NET_EV_READ,
                       NULL,
                       isil_ctl_client_read_ev_cb,
                       (void *)client_task);

    struct NET_TASK_MGT *glb_net_task_mgt_ptr =get_glb_net_task_mgt();
    glb_net_task_mgt_ptr->put_ev_task_to_act(glb_net_task_mgt_ptr,client_task);
    DEBUG_FUNCTION();
    return 0;

}


static int set_local_client_task(int fd)
{
    struct NET_TASK * client_task = net_ev_task_minit();
    if( !client_task ) {
        fprintf(stderr,"net_ev_task_minit  client_task failed .\n");
        return -1;
    }

    set_noblock(fd);
    SET_NET_TASK_FD(client_task,fd);
    SET_NET_TASK_TRANS_TYPE(client_task,NET_TCP_E);
    DEBUG_FUNCTION();
    // TODO :reg client_task callback function

    //ECHO server for test
    net_parse_reg_t *glb_net_ret_ptr = get_glb_net_parse_reg_t();

    reg_net_task_callback(client_task ,NULL ,glb_net_ret_ptr->net_local_cb);

    reg_net_ev_to_loop(client_task->ev ,
                       fd,
                       NET_EV_READ,
                       NULL,
                       isil_ctl_client_read_ev_cb,
                       (void *)client_task);

    struct NET_TASK_MGT *glb_net_task_mgt_ptr =get_glb_net_task_mgt();
    glb_net_task_mgt_ptr->put_ev_task_to_act(glb_net_task_mgt_ptr,client_task);
    DEBUG_FUNCTION();
    return 0;

}


static int set_data_client_task(int fd)
{
    struct NET_TASK * client_task = net_ev_task_minit();
    if( !client_task ) {
        fprintf(stderr,"net_ev_task_minit  client_task failed .\n");
        return -1;
    }

    set_noblock(fd);
    SET_NET_TASK_FD(client_task,fd);
    SET_NET_TASK_TRANS_TYPE(client_task,NET_TCP_E);
    // TODO :reg client_task callback function

    //for test

    net_parse_reg_t *glb_net_ret_ptr = get_glb_net_parse_reg_t();

    reg_net_task_callback(client_task ,NULL ,glb_net_ret_ptr->net_data_cb);


    reg_net_ev_to_loop(client_task->ev ,
                       fd,
                       NET_EV_READ,
                       NULL,
                       isil_ctl_client_read_ev_cb,
                       (void *)client_task);


    if(net_data_thr_alloc_task(client_task)< 0){
        fprintf(stderr,"net_data_thr_alloc_task failed  .\n");
        if(client_task->release) {
            DEBUG_FUNCTION();
            client_task->release(client_task);
        }
        return -1;
    }

    
    //struct NET_TASK_MGT *glb_net_task_mgt_ptr =get_glb_net_task_mgt();
    //glb_net_task_mgt_ptr->put_ev_task_to_act(glb_net_task_mgt_ptr,client_task);

    return 0;

}



static int set_async_notify_client_task(int fd)
{
    struct NET_TASK * client_task = net_ev_task_minit();
    if( !client_task ) {
        fprintf(stderr,"net_ev_task_minit  client_task failed .\n");
        return -1;
    }

    set_noblock(fd);
    SET_NET_TASK_FD(client_task,fd);
    SET_NET_TASK_TRANS_TYPE(client_task,NET_TCP_E);
    DEBUG_FUNCTION();

    fprintf(stderr,"net task addr [%p] .\n",client_task);
    // TODO :reg client_task callback function

    //ECHO server for test
    net_parse_reg_t *glb_net_ret_ptr = get_glb_net_parse_reg_t();
    if( !glb_net_ret_ptr->net_async_notify_cb ) {
        fprintf(stderr,"net_async_notify_cb NULL .\n");
        
        return -1;
    }

    reg_net_task_callback(client_task ,NULL ,glb_net_ret_ptr->net_async_notify_cb);

    reg_net_ev_to_loop(client_task->ev ,
                       fd,
                       NET_EV_READ,
                       NULL,
                       isil_aysnc_client_read_ev_cb,
                       (void *)client_task);

    struct NET_TASK_MGT *glb_net_task_mgt_ptr =get_glb_net_task_mgt();
    glb_net_task_mgt_ptr->put_ev_task_to_act(glb_net_task_mgt_ptr,client_task);
    DEBUG_FUNCTION();
    return 0;

}


static void isil_ctrl_accept_ev_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    int client_fd ;
    struct NET_EV_MGT_T *ev_arg;


    ev_arg = isil_container_of(ev,struct NET_EV_MGT_T,s_event);

    struct NET_TASK *listen_task = (struct NET_TASK *)(ev_arg->arg);
    assert(listen_task);

    if(revents & NET_EV_READ) {

        DEBUG_FUNCTION();
        client_fd = u_accept( listen_task->fd , NULL , 0);
        if(client_fd == -1 ) {
            ev_io_stop(loop,ev);
            fprintf(stderr,"accept failed [%s] .\n",strerror(errno));
            listen_task->release(listen_task);
            return;
        }
        fprintf(stderr,"client_fd = %d.\n",client_fd);

        set_ctl_client_task(client_fd);
        ev_io_start (ev_arg->evBase, &ev_arg->s_event);

   }

}


static void isil_local_accept_ev_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    int client_fd ;
    struct NET_EV_MGT_T *ev_arg;


    ev_arg = isil_container_of(ev,struct NET_EV_MGT_T,s_event);

    struct NET_TASK *listen_task = (struct NET_TASK *)(ev_arg->arg);
    assert(listen_task);

    if(revents & NET_EV_READ) {

        DEBUG_FUNCTION();
        client_fd = u_accept( listen_task->fd , NULL , 0);
        if(client_fd == -1 ) {
            ev_io_stop(loop,ev);
            fprintf(stderr,"accept failed [%s] .\n",strerror(errno));
            listen_task->release(listen_task);
            return;
        }
        fprintf(stderr,"client_fd = %d.\n",client_fd);

        set_local_client_task(client_fd);
        ev_io_start (ev_arg->evBase, &ev_arg->s_event);

   }

}



static void isil_data_accept_ev_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    int client_fd ;
    struct NET_EV_MGT_T *ev_arg;


    ev_arg = isil_container_of(ev,struct NET_EV_MGT_T,s_event);

    struct NET_TASK *listen_task = (struct NET_TASK *)(ev_arg->arg);
    assert(listen_task);

    if(revents & NET_EV_READ) {

        

        client_fd = u_accept( listen_task->fd ,NULL ,0);
        if(client_fd == -1 ) {
            ev_io_stop(loop,ev);
            fprintf(stderr,"accept failed [%s] .\n",strerror(errno));
            listen_task->release(listen_task);
            return;
        }

        

        set_data_client_task(client_fd);
        ev_io_start (ev_arg->evBase, &ev_arg->s_event);

   }

}



static void isil_aync_notify_accept_ev_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    int client_fd ;
    struct NET_EV_MGT_T *ev_arg;


    ev_arg = isil_container_of(ev,struct NET_EV_MGT_T,s_event);

    struct NET_TASK *listen_task = (struct NET_TASK *)(ev_arg->arg);
    assert(listen_task);

    if(revents & NET_EV_READ) {

        DEBUG_FUNCTION();
        client_fd = u_accept( listen_task->fd , NULL , 0);
        if(client_fd == -1 ) {
            ev_io_stop(loop,ev);
            printf("accept failed [%s] .\n",strerror(errno));
            listen_task->release(listen_task);
            return;
        }
        fprintf(stderr,"client_fd = %d.\n",client_fd);

        set_async_notify_client_task(client_fd);
        ev_io_start (ev_arg->evBase, &ev_arg->s_event);

   }

}



static int isil_net_listen_init( unsigned int port ,NET_LISTEN_EV_CB *cb)
{
    int fd ;
    struct NET_TASK *listen_task;
    struct NET_EV_MGT_T *listen_ev_arg;

    if( !cb ) {
        return -1;
    }

    fd = u_open2(port);
    if(fd < 0) {
        fprintf(stderr,"tcp_listen failed .\n");
        return -1;
    }


    listen_task = net_ev_task_minit();
    if( !listen_task ) {
        fprintf(stderr,"net_ev_task_minit  listen_task failed .\n");
        return -1;
    }

    SET_NET_TASK_FD(listen_task,fd);
    SET_NET_TASK_TRANS_TYPE(listen_task,NET_TCP_E);

    listen_ev_arg = get_net_task_ev_arg(listen_task);

    reg_net_ev_to_loop(listen_ev_arg ,
                       fd,
                       NET_EV_READ,
                       NULL,
                       cb,
                       (void *)listen_task);

    struct NET_TASK_MGT *glb_net_task_mgt_ptr =get_glb_net_task_mgt();

    glb_net_task_mgt_ptr->put_ev_task_to_act(glb_net_task_mgt_ptr,listen_task);

    return 0;



}   


static void exit_on_signal(struct ev_loop *loop,struct ev_signal *watcher,int revents)
{

#if 0
    NET_EV_CB* ev_cb;
    ev_cb = get_glb_net_ev_cb();

    DEBUG_FUNCTION();
    ev_signal_stop (ev_cb->evbase, watcher);
    DEBUG_FUNCTION();
    release_global_net_ev_cb();
//    ev_unloop(loop, 0);
    DEBUG_FUNCTION();
#endif
    ev_unloop(loop, EVUNLOOP_ALL);
    DEBUG_FUNCTION();
    //ev_unloop(loop, EVBREAK_ALL);
}


int isil_net_sdk_run(void *arg)
{
    int ret;
    ev_signal sigint;
    ev_signal sigterm;
    
    NET_EV_CB* ev_cb;

    ev_cb = get_glb_net_ev_cb();

    ev_signal_init(&sigint, exit_on_signal, SIGINT);
    
    ev_signal_init(&sigterm, exit_on_signal, SIGTERM);

    ev_signal_start (ev_cb->evbase, &sigint);
    ev_signal_start (ev_cb->evbase, &sigterm);


    ret = isil_net_listen_init(NETCTRLPORT , isil_ctrl_accept_ev_cb);
    if(ret < 0) {
        fprintf(stderr,"isil_net_listen_init[net ctl] failed .\n");
        return -1;
    }


    ret = isil_net_listen_init(NETDATAPORT , isil_data_accept_ev_cb);
    if(ret < 0) {
        fprintf(stderr,"isil_net_listen_init[net data] failed .\n");
        return -1;
    }

    ret = isil_net_listen_init(NETLOCALPORT , isil_local_accept_ev_cb);
    if(ret < 0) {
        fprintf(stderr,"isil_net_listen_init[net data] failed .\n");
        return -1;
    }

    ret = isil_net_listen_init(NETASYNCNOTIFYPORT , isil_aync_notify_accept_ev_cb);
    if(ret < 0) {
        fprintf(stderr,"isil_net_listen_init[net data] failed .\n");
        return -1;
    }
    
    
    ev_cb->loop(ev_cb);
    DEBUG_FUNCTION();
    ev_signal_stop (ev_cb->evbase, &sigint);
    ev_signal_stop (ev_cb->evbase, &sigterm);

    sem_post( &glb_net_sdk_mgt.net_sdk_sem);
    DEBUG_FUNCTION();

    return 0;

}


static void isil_net_sdk_run_package( void * arg)
{
    if( isil_net_sdk_run(NULL) < 0 ) {
        exit(-1);
    }
}


int isil_net_thread_run( void )
{
    int ret;
    ret = net_glb_thread_dispatch(isil_net_sdk_run_package ,NULL);
    if( ret < 0 ) {
        fprintf( stderr,"glb_thread_dispatch err .\n");
        exit(-1);
    }

    return 0;
}
