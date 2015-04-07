#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>


#include "isil_udp.h"
#include "io_utils.h"
#include "net_parse_reg.h"
#include "net_event_mgt.h"
#include "net_task.h"
#include "isil_data_thread_mgt.h"
#include "net_debug.h"


#define UDP_RECV_BUFF_SIZE (2048)


static unsigned char udpport_array[UDP_MAX_PORT_NUM];


static unsigned short udp_get_portnum( void )
{
    int i;
    for(i = 0; i < UDP_MAX_PORT_NUM; i++)
    {
        if(udpport_array[i] == 0)
        {
            
            udpport_array[i] = 1;
            
            return UDP_BASE_PORT+i;
        }
        else
        {
            continue;
        }
    }
    
    
    return 0;
}


void udp_clear_portnum(unsigned short portnum)
{
    if((portnum >= UDP_MAX_PORT_NUM) &&
        (portnum < UDP_MAX_PORT_NUM + UDP_MAX_PORT_NUM))
        udpport_array[portnum - UDP_BASE_PORT] = 0;
}


static void isil_udp_client_data_read_ev_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    int ret ,len;
    struct NET_EV_MGT_T *ev_arg;
    struct NET_TASK *client_task;
    
    char net_buff[UDP_RECV_BUFF_SIZE];

    

    ev_arg = isil_container_of(ev,struct NET_EV_MGT_T,s_event);
    client_task = (struct NET_TASK *)(ev_arg->arg);
    assert(client_task);

    if(revents & NET_EV_READ){
        
        DEBUG_FUNCTION();
        ret = u_recvfrom(client_task->fd , &net_buff ,UDP_RECV_BUFF_SIZE,&client_task->sa);
        if( ret <= 0 ) {
            fprintf(stderr,"Recv form udp err.\n");
            client_task->release(client_task);
            return;
        }
        
        len = ret;

        if(client_task->hook){
            
            ret = client_task->hook(len,(void *)net_buff ,client_task);
            if( ret < 0) {
                if(client_task->release) {
                    DEBUG_FUNCTION();
                    client_task->release(client_task);
                }
                return ;
            }
        }
    }
}

int set_udp_task( void )
{
    
    int ret;
    unsigned short port;

   
   
    struct NET_TASK * client_task = net_ev_task_minit();
    if( !client_task ) {
        fprintf(stderr,"net_ev_task_minit  client_task failed .\n");
        return -1;
    }

    port = udp_get_portnum();
    if( port == 0) {
        fprintf(stderr,"udp_get_portnum failed .\n");
        client_task->release(client_task);
        return -1;
    }

    fprintf(stderr,"Get udp port %u .\n",port);

    ret = u_openudp(port , &client_task->sa);
    if( ret < 0 ) {
        
        client_task->release(client_task);
        return -1;
    }

    

    set_noblock(ret);

    SET_NET_TASK_FD(client_task ,ret);
    fprintf(stderr,"Get udp fd %u .\n",client_task->fd);
    SET_NET_TASK_TRANS_TYPE(client_task,NET_UDP_E);
    client_task->port = port;
    client_task->clear_port = udp_clear_portnum;

    net_parse_reg_t *glb_net_ret_ptr = get_glb_net_parse_reg_t();

    reg_net_task_callback(client_task ,NULL ,glb_net_ret_ptr->net_data_cb);
    
    reg_net_ev_to_loop(client_task->ev ,
                       client_task->fd,
                       NET_EV_READ,
                       NULL,
                       isil_udp_client_data_read_ev_cb,
                       (void *)client_task);

    if(net_data_thr_alloc_task(client_task)< 0){
        fprintf(stderr,"net_data_thr_alloc_task failed  .\n");
        if(client_task->release) {
            DEBUG_FUNCTION();
            client_task->release(client_task);
        }
        return -1;
    }

    DEBUG_FUNCTION();
    return port;

}



int set_udp_task2( unsigned int net_addr )
{
    
    int ret;
    unsigned short port;

   
    DEBUG_FUNCTION();
    struct NET_TASK * client_task = net_ev_task_minit();
    if( !client_task ) {
        fprintf(stderr,"net_ev_task_minit  client_task failed .\n");
        return -1;
    }

    port = udp_get_portnum();
    if( port == 0) {
        fprintf(stderr,"udp_get_portnum failed .\n");
        client_task->release(client_task);
        return -1;
    }

    fprintf(stderr,"Get udp port %u .\n",port);

    ret = u_openudp2( net_addr ,port , &client_task->sa);
    if( ret < 0 ) {
        DEBUG_FUNCTION();
        client_task->release(client_task);
        return -1;
    }

    

    set_noblock(ret);

    SET_NET_TASK_FD(client_task ,ret);
    fprintf(stderr,"Get udp fd %u .\n",client_task->fd);
    SET_NET_TASK_TRANS_TYPE(client_task,NET_UDP_E);
    client_task->port = port;
    client_task->clear_port = udp_clear_portnum;

    net_parse_reg_t *glb_net_ret_ptr = get_glb_net_parse_reg_t();

    reg_net_task_callback(client_task ,NULL ,glb_net_ret_ptr->net_data_cb);
    
    reg_net_ev_to_loop(client_task->ev ,
                       client_task->fd,
                       NET_EV_READ,
                       NULL,
                       isil_udp_client_data_read_ev_cb,
                       (void *)client_task);

    if(net_data_thr_alloc_task(client_task)< 0){
        fprintf(stderr,"net_data_thr_alloc_task failed  .\n");
        if(client_task->release) {
            DEBUG_FUNCTION();
            client_task->release(client_task);
        }
        return -1;
    }

    DEBUG_FUNCTION();
    return port;

}

