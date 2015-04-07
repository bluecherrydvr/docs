#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "isil_pb_process.h"
#include "isil_pb_mgt.h"
#include "isil_interface.h"
#include "isil_msg_mgt.h"
#include "linux_list.h"
#include "net_event_mgt.h"
#include "isil_decode_api.h"




#define TC_NLK_BUFF_SIZE 128


static int send_msg_to_single_pb_mgt(single_pb_mgt *pb_mgt_ptr ,int len ,void *buff)
{
    
    struct ISIL_MSG_MGT *msg_list;

    if( !pb_mgt_ptr || !len || !buff ) {
        return -1;
    }

    msg_list = pb_mgt_ptr->msg_list;
    if( !msg_list ) {
        return -1;
    }



    return 0;


}



static int parse_tc_netlink_recv_msg(char *buff , int len)
{
    int ret;
    struct tn_msg *p_tn_msg;
    struct  query_info *query_ptr;
    
    
    if( !buff || !len) {
        return -1;
    }

    p_tn_msg = (struct tn_msg *)buff;

    switch (p_tn_msg->msg_type) {
    case TN_MSG_REQ_FRAMES:{
        if( p_tn_msg->msg_len != sizeof(struct query_info) ) {
            fprintf(stderr," net link msg size err .\n");
            return -1;
        }

        query_ptr = (struct query_info *)(&p_tn_msg->payload[0]);
        if(  !query_ptr ) {
            fprintf( stderr,"query_ptr is NULL .\n");
            return -1;
        }
	   

        ret = isil_pb_netlink_req_frames_cb(query_ptr);
        if(  ret < 0 ) {
            fprintf( stderr,"isil_pb_netlink_req_frames_cb err .\n");
            return -1;
        }


    }
    break;
    case TN_MSG_DISCARD_FRAME_REASON:{
        if( p_tn_msg->msg_len != sizeof(struct  query_info) ) {
            fprintf(stderr," net link msg size err .\n");
            return -1;
        }

    }
    break;
    default:
        break;

    }

    return 0;
    
    
    

}



static struct sockaddr_nl  tc_src_addr;
static struct sockaddr_nl  tc_dest_addr;


int tc_create_netlink_connect(void)
{
    int netlink_fd;
	netlink_fd = socket(PF_NETLINK, SOCK_RAW, TC_NETLINK_GENERIC);
	if(netlink_fd == -1){
		fprintf(stderr,"Create netlink fd failed.\n");
		return -1;
	}
	memset(&tc_src_addr, 0, sizeof(tc_src_addr));
	tc_src_addr.nl_family = AF_NETLINK;
	tc_src_addr.nl_pid = getpid();
	tc_src_addr.nl_groups = 1; /* not in mcast groups */

	if (bind(netlink_fd, (struct sockaddr *)&tc_src_addr, sizeof(tc_src_addr))) {
        fprintf(stderr, "---bind netlink fail---\n");
		return -1;
	}

	memset(&tc_dest_addr, 0, sizeof(tc_dest_addr));
	tc_dest_addr.nl_family = AF_NETLINK;
	tc_dest_addr.nl_pid = 0; /* kernel */
	tc_dest_addr.nl_groups = 1; /* unicast */
	return netlink_fd;
}


static int tc_handle_netlink_recv_msg( struct NET_TASK *task )
{
    int ret ,fd;
    struct iovec iov[2];
    struct msghdr msg;
    struct nlmsghdr nlh;

    char tc_netlink_recvbuff[TC_NLK_BUFF_SIZE];

    fd = task->fd;
    memset(&msg, 0, sizeof(struct msghdr));
    iov[0].iov_base = &nlh;
    iov[0].iov_len = sizeof(nlh);
    iov[1].iov_base = tc_netlink_recvbuff;
    iov[1].iov_len =   TC_NLK_BUFF_SIZE;

    msg.msg_name= (void*)&tc_src_addr;
    msg.msg_namelen = sizeof(tc_src_addr);
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;

    memset(tc_netlink_recvbuff ,0x00,sizeof(tc_netlink_recvbuff));
    ret = recvmsg(fd, &msg, MSG_DONTWAIT);
    if( ret < 0 && (errno != EINTR) ) {
        fprintf( stderr," Recv msg from nlk err.\n");
        return -1;
    }

    ret = parse_tc_netlink_recv_msg(tc_netlink_recvbuff,ret);
    if( ret < 0 ) {
        return -1;
    }

    return 0;
}


static void net_link_handle_ev_cb(struct ev_loop *loop,ev_io *ev,int revents)
{
    int ret ;
    struct NET_EV_MGT_T *ev_arg;

    ev_arg = isil_container_of(ev,struct NET_EV_MGT_T,s_event);

    struct NET_TASK *net_link_task = (struct NET_TASK *)(ev_arg->arg);
    assert(net_link_task);
    
    if(revents & NET_EV_READ) {
        ret = tc_handle_netlink_recv_msg(net_link_task);
        if( ret < 0 ) {
            fprintf( stderr ,"tc_handle_netlink_recv_msg err .\n");
            ev_io_stop(loop,ev);
            if( net_link_task->release ) {
                net_link_task->release(net_link_task);
            }
        }

     
    }
}


int create_dec_netlink_ev( void )
{
    int fd;
    struct NET_TASK *net_link_task;
    struct NET_EV_MGT_T *ev_arg;
    
    fd = tc_create_netlink_connect();
    if( fd < 0 ) {
        fprintf( stderr,"tc_create_netlink_connect err .\n");
        return -1;
    }

    net_link_task = net_ev_task_minit();
    if( !net_link_task ) {
        fprintf(stderr,"net_link_task  net_ev_task_minit failed .\n");
        return -1;
    }

    SET_NET_TASK_FD(net_link_task,fd);

    SET_NET_TASK_TRANS_TYPE(net_link_task,NET_NETLINK_E);

    ev_arg = get_net_task_ev_arg(net_link_task);
    if( !ev_arg ) {
        fprintf(stderr,"net_link_task  get_net_task_ev_arg failed .\n");

        if( net_link_task->release ) {
            net_link_task->release(net_link_task);
        }

        return -1;
    }

    reg_net_ev_to_loop(ev_arg ,
                       fd,
                       NET_EV_READ,
                       NULL,
                       net_link_handle_ev_cb,
                       (void *)net_link_task);

    struct NET_TASK_MGT *glb_net_task_mgt_ptr =get_glb_net_task_mgt();

    glb_net_task_mgt_ptr->put_ev_task_to_act(glb_net_task_mgt_ptr,net_link_task);

    return 0;

}






