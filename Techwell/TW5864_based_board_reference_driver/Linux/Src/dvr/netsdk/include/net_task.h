#ifndef  NET_TASK_H
#define  NET_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <netinet/in.h>

#include "linux_list.h"
#include "net_event_mgt.h"
#include "isil_net_nty.h"
#include "isil_net_data_header.h"



#define NET_MGT_WAIT_TIMEOUT (200000000)

#define GET_DATA_USE_TIMEOUT

struct NET_TASK ;
struct NET_TASK_MGT;


typedef int (NET_TASK_CALLBACK)(int len ,void *buff ,struct NET_TASK *ev_task);

typedef int (NET_TASK_MGT_CALLBACK)(struct NET_TASK_MGT *task_mgt); 

typedef int (NET_TASK_WRITE_CB)(struct NET_TASK *ev_task ,void *src);


#define ISIL_SPS_BUFF_LEN (40)
#define ISIL_PPS_BUFF_LEN (40)

struct h264_param_t{
    
    unsigned int  sps_len;
    unsigned char sps_buff[ISIL_SPS_BUFF_LEN];
    unsigned int  pps_len;
    unsigned char pps_buff[ISIL_PPS_BUFF_LEN];
};


typedef enum{
    NET_TCP_E = 0,
    NET_UDP_E,
    NET_NETLINK_E,
    NET_BROADCAST,
    
    NET_TASK_BUTTON_E
}NET_TASK_TRANS_TYPE_E;

typedef enum{
    NET_NEED_ALL_FRAME_E,
    NET_ONLY_NEED_I_FRAME_E
}ISIL_NET_H264_STREAM_TYPE_E;




struct NET_TASK{
    
    struct list_head entry;
    struct NET_EV_MGT_T *ev;
    struct NET_TASK_MGT *task_mgt;
    struct NET_NOTIFY_RECORD_T *nty_rec_t;
    struct NET_DATA_MGT_T   *net_data_mgt;
    int fd;
    NET_TASK_TRANS_TYPE_E trans_type; 
    struct  sockaddr_in sa;
    unsigned short port;
    unsigned short frame_type;
    volatile int ref;
    unsigned int phy_chip_id;
    unsigned int phy_chan_id;
    void *enc_param;
    void *priv;
    void *param;
    NET_TASK_CALLBACK *hook;
    NET_TASK_WRITE_CB *send;

    void (*task_priv_release)(struct NET_TASK *ev_task);
    void (*clear_port)(unsigned short portnum);
    void (*del_from_task_mgt)(struct NET_TASK *ev_task);
    void (*release)(struct NET_TASK *release);
    void (*async_release)(struct NET_TASK *release);

};


struct NET_TASK_MGT{
    struct list_head ev_act_tsk_lst;
    int ev_act_num;
    pthread_mutex_t lst_lock;
    pthread_cond_t  lst_cond;

    pthread_cond_t  cond;
    pthread_mutex_t  lock;

    volatile int have_ev;

    struct list_head data_send_list;
    int data_send_num;
    pthread_cond_t  data_send_cond;
    pthread_mutex_t data_send_lock;
    
    int alloc_type;
    int is_init;
    volatile int is_run;

    NET_TASK_MGT_CALLBACK *callback;
    

    void (*wait)(struct NET_TASK_MGT *task_mgt);
    void (*wakeup)(struct NET_TASK_MGT *task_mgt);


    struct NET_DATA_NODE_T* (*get_data_node)(struct NET_TASK_MGT *task_mgt);
    struct NET_DATA_NODE_T* (*wait_get_data_node)(struct NET_TASK_MGT *task_mgt);

    void (*put_data_node)(struct NET_TASK_MGT *task_mgt,struct NET_DATA_NODE_T *data_node);

    void (*put_ev_task_to_act)(struct NET_TASK_MGT *task_mgt,struct NET_TASK * ev_task);
    struct NET_TASK* (*get_ev_task_from_act_head)(struct NET_TASK_MGT *task_mgt);
    void (*release)(struct NET_TASK_MGT *task_mgt);
};


#define SET_NET_TASK_FD(x,value) \
    (((x)->fd) =  value)

#define GET_NET_TASK_FD(x) \
    ((x)->fd)


#define SET_NET_TASK_TRANS_TYPE(x,value) \
    (((x)->trans_type) =  value)

#define GET_NET_TASK_TRANS_TYPE(x) \
    ((x)->trans_type)

static inline void add_net_task_ref(struct NET_TASK *ev_task)
{
    if(ev_task) {
        ++ev_task->ref;
    }
}

static inline void minus_net_task_ref(struct NET_TASK *ev_task)
{
    if(ev_task->ref > 0) {
        --ev_task->ref;
    }
}

static inline void reg_net_task_callback(struct NET_TASK *ev_task,void *arg, NET_TASK_CALLBACK *callback)
{
    if(ev_task) {
        ev_task->param = arg;
        ev_task->hook  = callback;
    }
}


static inline void net_task_reg_priv_release_cb(struct NET_TASK *ev_task,void *arg, void (*release_cb)(struct NET_TASK *task))
{
    if(ev_task) {
        ev_task->priv = arg;
        ev_task->task_priv_release  = release_cb;
    }
}

static inline  struct NET_EV_MGT_T *get_net_task_ev_arg(struct NET_TASK *ev_task)
{
    return ev_task->ev;
}

static inline void cleanup_task_mgt_ev(struct NET_TASK_MGT *task_mgt)
{
    task_mgt->have_ev = 0 ;
}

static inline void reg_net_task_mgt_callback(struct NET_TASK_MGT *task_mgt , NET_TASK_MGT_CALLBACK *callback)
{
    if(task_mgt) {
       
        task_mgt->callback  = callback;

    }
}


static inline void reg_net_task_write_callback(struct NET_TASK *ev_task , NET_TASK_WRITE_CB *send_cb)
{
    if(send_cb) {
        ev_task->send = send_cb;
    }
}


extern void net_task_mgt_sinit(struct NET_TASK_MGT *task_mgt);

extern struct NET_TASK_MGT * net_task_mgt_minit( void );

extern struct NET_TASK *net_ev_task_minit(void);

extern void glb_net_task_mgt_init(void);

extern struct NET_TASK_MGT *get_glb_net_task_mgt(void);

extern void alloc_enc_param_t_to_net_task(struct NET_TASK *ev_task);

extern void free_net_task_enc_param(struct NET_TASK *ev_task);

extern void del_data_node_self_from_send_lst(struct NET_DATA_NODE_T *data_node);

#ifdef __cplusplus
}
#endif

#endif
