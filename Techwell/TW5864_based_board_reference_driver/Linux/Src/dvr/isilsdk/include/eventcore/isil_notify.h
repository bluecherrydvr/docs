#ifndef _ISIL_NOTIFY_H
#define _ISIL_NOTIFY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "linux_list.h"
#include "isil_sdk_config.h"


typedef int (notify_entry_callback)(void *product,void *arg);


struct ISIL_NOTIFY_BLK{
    struct list_head entry;
    pthread_mutex_t entry_lock;
    pthread_cond_t   entry_cond;
    int       is_ready;                    //data ready ,for block method
    volatile int ref;

    enum ISIL_E_NOTIFY_TYPE notify_type;
    enum ISIL_E_BLOCK_TYPE  block_type;
    void *arg;
    notify_entry_callback *hook;

    
    void (*wait)(struct ISIL_NOTIFY_BLK *blk);
    void (*weekup)(struct ISIL_NOTIFY_BLK *blk);
    void (*release)(struct ISIL_NOTIFY_BLK *blk);
};



struct ISIL_NOTIFY_LST{
    struct list_head notify_list;       
    volatile int notify_num;
    pthread_mutex_t   notify_ev_lock;      
    int alloc_type;
    
    void (*put_notify_blk)(struct ISIL_NOTIFY_BLK *blk , struct ISIL_NOTIFY_LST *lst);
    struct ISIL_NOTIFY_BLK* (*get_blk_from_head)(struct ISIL_NOTIFY_LST *lst);
    void (*release)(struct ISIL_NOTIFY_LST *lst);
    void (*notify)(struct ISIL_NOTIFY_LST *lst ,void *product);
};

#define GET_NOTIFY_BLK_NOTIFY_TYPE(x) \
    ((x)->notify_type)

#define SET_NOTIFY_BLK_NOTIFY_TYPE(x ,value) \
    ((x)->notify_type = value)

#define GET_NOTIFY_BLK_BLOCK_TYPE(x) \
    ((x)->block_type)

#define SET_NOTIFY_BLK_BLOCK_TYPE(x ,value) \
    ((x)->block_type = value)

#define GET_NOTIFY_BLK_IS_READY(x) \
    ((x)->is_ready)

#define SET_NOTIFY_BLK_IS_READY(x ,value) \
    ((x)->is_ready = value)

static inline void add_notify_blk_ref(struct ISIL_NOTIFY_BLK *notify_blk)
{
    ++notify_blk->ref;
}

static inline void minus_notify_blk_ref(struct ISIL_NOTIFY_BLK *notify_blk)
{
    --notify_blk->ref;
}


static inline void reg_notify_blk_callback(struct ISIL_NOTIFY_BLK *blk,
                                           void *param,
                                           notify_entry_callback *callback)
{
    if(blk) {
        blk->arg = param;
        blk->hook = callback;
    }
}

extern struct ISIL_NOTIFY_BLK *notify_blk_init( void );

extern struct ISIL_NOTIFY_LST *notify_lst_minit( void );

extern void notify_lst_sinit( struct ISIL_NOTIFY_LST *notify_lst );

extern void notify_blk_wait(struct ISIL_NOTIFY_BLK *blk);

extern void notify_blk_weekup(struct ISIL_NOTIFY_BLK *blk);







#ifdef __cplusplus
}
#endif

#endif
