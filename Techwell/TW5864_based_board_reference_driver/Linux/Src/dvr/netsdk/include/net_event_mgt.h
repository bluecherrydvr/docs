#ifndef  _NET_EVENT_MGT_H
#define  _NET_EVENT_MGT_H


#ifdef __cplusplus
extern "C" {
#endif

#include<sys/time.h>

#include<ev.h>



typedef enum{
     
     NET_EV_READ     = 0x01,
     NET_EV_WRITE	 = 0x02,
     
}ENETEVTYPE;



typedef void (NETEVHOOK)(struct ev_loop *loop,ev_io *ev,int revents);

typedef struct _NET_EV_CB NET_EV_CB;

struct _NET_EV_CB{
    struct ev_loop* evbase;
    int timeout;  //event timeout
    int is_init;

    int (*loop)(struct _NET_EV_CB *ev_cb);
};

extern void init_net_ev_cb(NET_EV_CB *net_ev_cb,int timeout);

//extern void release_NET_EV_CB(NET_EV_CB *NET_EV_CB);
extern void init_global_net_ev_cb(int timeout);

extern void release_global_net_ev_cb( void );

extern NET_EV_CB* get_glb_net_ev_cb(void);

static inline struct ev_loop* get_glb_net_ev_base(void)
{
    return get_glb_net_ev_cb()->evbase;
}



struct NET_EV_MGT_T{
    short  event_type;
    char  is_active;
    char  alloc_type;
    ev_io   s_event;
    struct ev_loop *evBase;
    void *arg;
    NETEVHOOK *callback;

    void (*release)(struct NET_EV_MGT_T *EV_ARG);
};



    
extern struct NET_EV_MGT_T *net_ev_arg_t_minit( void );

extern void init_static_net_ev_arg_t(struct NET_EV_MGT_T *arg_t);

extern void reg_net_ev_to_loop(struct NET_EV_MGT_T *EV_ARG ,
                                  int fd ,
                                  short event_type,
                                  struct timeval* tv ,
                                  NETEVHOOK *evcb ,
                                  void *arg);








#ifdef __cplusplus
}
#endif //__cplusplus

#endif
