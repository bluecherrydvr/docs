#ifndef   _ISIL_NET_NTY_H
#define  _ISIL_NET_NTY_H




#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "linux_list.h"
#include "isil_net_config.h"




struct NET_NOTIFY_RECORD_T;
struct NET_NOTIFY_REG_T;
struct NET_NOTIFY_T;

typedef int net_notify_callback(void *src ,struct NET_NOTIFY_T *net_notify);

struct NET_NOTIFY_T{

    struct list_head reg_entry;
    struct list_head record_entry; 
    struct NET_NOTIFY_REG_T *reg_root;
    struct NET_NOTIFY_RECORD_T *record_root;

    void *param;
    net_notify_callback *hook;

    void (*release)(struct NET_NOTIFY_T *notify_p);
};


struct NET_NOTIFY_REG_T{

    struct list_head reg_list;
    unsigned int reg_num;
    pthread_mutex_t   reg_lock;   
    
    enum NETALLOCTYPE_E alloc_type;
    void *root;
    int  (*root_reg)(void *root);
    int  (*root_unreg)(void *root); 
       
    void (*notify_all)( void *src,struct NET_NOTIFY_REG_T * net_notify_reg_p);
    void (*put_net_nty)(struct NET_NOTIFY_T *notify_p , struct NET_NOTIFY_REG_T * net_notify_reg_p);
    void (*release)(struct NET_NOTIFY_REG_T *net_notify_reg_p);
    
};


struct NET_NOTIFY_RECORD_T{

    struct list_head record_list;
    unsigned int record_num;
    pthread_mutex_t   record_lock;   
    enum NETALLOCTYPE_E alloc_type;

    void (*put_net_nty)(struct NET_NOTIFY_T *notify_p , struct NET_NOTIFY_RECORD_T * net_notify_record_p);
    void (*release)(struct NET_NOTIFY_RECORD_T *net_notify_record_p);


};



static inline void net_notify_callback_reg(struct NET_NOTIFY_T *net_notify, void * param ,net_notify_callback *cb)
{
    if(net_notify) {
        net_notify->param = param;
        net_notify->hook   = cb;
    }
}

extern struct NET_NOTIFY_T * net_notify_minit( void );

extern void net_notify_reg_sinit(struct NET_NOTIFY_REG_T *net_notify_reg_p);

extern struct NET_NOTIFY_REG_T * net_notify_reg_minit( void );

extern void net_notify_record_sinit(struct NET_NOTIFY_RECORD_T *net_notify_record_p);

extern struct NET_NOTIFY_RECORD_T * net_notify_record_minit( void );




#ifdef __cplusplus
}
#endif //__cplusplus

#endif


