#ifndef  _ISIL_EVENT_MGT_H
#define _ISIL_EVENT_MGT_H




#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>

#include <event.h>

#include "isil_sdk_config.h"

typedef enum{
    EVENT_LOOP_ONCE	 = 0x01 ,	/**< Block at most once. */
    EVENT_LOOP_NONBLOCK	 = 0x02 ,	/**< Do not block. */
}ELOOPTYPE;

typedef enum{
     EVENT_TIMEOUT	 = 0x01,
     EVENT_READ		 = 0x02,
     EVENT_WRITE	 = 0x04,
     EVENT_SIGNAL	 = 0x08,
     EVENT_PERSIST	 = 0x10	,/* Persistant event */
}EEVTYPE;



typedef void (EVHOOK)(int fd,short event_type,void *arg);

typedef struct _EVENT_CB EVENT_CB;

struct _EVENT_CB{
    struct event_base* evbase;
    int timeout;  //event timeout
    int is_init;

    int (*loop)(struct _EVENT_CB *ev_cb);
	int (*unloop)(struct _EVENT_CB *ev_cb);
};

extern void init_event_cb(EVENT_CB *event_cb,int timeout);

//extern void release_event_cb(EVENT_CB *event_cb);
extern void init_global_event_cb(int timeout);

extern void release_global_event_cb( void );

extern EVENT_CB* get_glb_ev_cb(void);

static inline struct event_base* get_global_ev_base(void)
{
    return get_glb_ev_cb()->evbase;
}



struct ISIL_EV_ARG_T{
    short  event_type;
    char  is_active;
    char  alloc_type;
    struct event   s_event;
    struct event_base *evBase;
    void *arg;
    EVHOOK *callback;

    void (*release)(struct ISIL_EV_ARG_T *EV_ARG);
};


#define EV_ARG_ADD_TIME(x,time) \
    evtimer_add(&((x)->s_event),time)

#define EV_ADD(ev,time) \
    event_add(ev, time)

#define EV_TIME_ADD(ev,time) \
    EV_ADD(ev,time)

#define EV_SIG_ADD(ev,time) \
    signal_add(ev,time)

#define EV_SET(ev,fd,type,callback,arg) \
    event_set(ev,fd,type,callback,arg)

#define EV_TIME_SET(ev, callback,arg) \
    EV_SET(ev,-1 ,0 ,callback ,arg)

#define EV_SIG_SET(ev, x, cb, arg) \
    signal_set(ev, x, cb, arg)	

#define EV_BASE_SET(evbase,x) \
    event_base_set(evbase ,x)

#define EV_DEL(x) \
    event_del(&((x)->s_event))


#define GET_S_EVENT(x) \
    (&((x)->s_event))

    
extern struct ISIL_EV_ARG_T *ev_arg_t_minit( void );

extern void init_static_ev_arg_t(struct ISIL_EV_ARG_T *arg_t);

extern void reg_ev_to_loop(struct ISIL_EV_ARG_T *EV_ARG ,
                                  int fd ,
                                  short event_type,
                                  struct timeval* tv ,
                                  EVHOOK *evcb ,
                                  void *arg);








#ifdef __cplusplus
}
#endif //__cplusplus

#endif
