#ifndef   _ISIL_MSG_MGT_H
#define  _ISIL_MSG_MGT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "linux_list.h"
#include "isil_sdk_config.h"

typedef struct ISIL_MSG_BLK MSG_BLK_T;
struct ISIL_MSG_MGT;

typedef int (msg_callback)(void *arg);

struct ISIL_MSG_BLK{
    struct list_head entry;
    pthread_mutex_t user_lock;
    pthread_cond_t  user_cond;
    int       is_ok; 
    int       result;
    volatile int ref;
	struct ISIL_MSG_MGT *root;
    void *param;
    msg_callback *hook;
    void (*wait)(struct ISIL_MSG_BLK *blk);
    void (*weekup)(struct ISIL_MSG_BLK *blk);
    void (*release)(struct ISIL_MSG_BLK *blk);

};


struct ISIL_MSG_MGT{
    struct list_head msg_list; 
    int msg_num; 
    pthread_mutex_t   list_lock; 
    int is_init;
    int alloc_type;
    struct ISIL_MSG_BLK * (*get_blk_from_head)(struct ISIL_MSG_MGT *msg_mgt);
    void (*put_blk)(struct ISIL_MSG_BLK *blk , struct ISIL_MSG_MGT *msg_mgt);
    void (*release)(struct ISIL_MSG_MGT *msg_mgt);
    void (*notify)(struct ISIL_MSG_MGT *msg_mgt);
};


#define GET_MSG_BLK_RESULT(x) \
    ((x)->result)

#define SET_MSG_BLK_RESULT(x,value) \
    (((x)->result) = value)

static inline void add_msg_blk_ref(struct ISIL_MSG_BLK *blk)
{
    ++blk->ref;
}

static inline void minus_msg_blk_ref(struct ISIL_MSG_BLK *blk)
{
    --blk->ref;
}

static inline void reg_msg_callback(struct ISIL_MSG_BLK *blk , void *param , msg_callback *hook)
{
    if(blk) {
        blk->param = param ;
        blk->hook    = hook ;
    }
}

extern struct ISIL_MSG_BLK *msg_blk_minit( void );

extern struct ISIL_MSG_MGT *msg_mgt_minit( void );

extern int msg_mgt_sinit( struct ISIL_MSG_MGT *msg_mgt );

extern void msg_blk_wait(struct ISIL_MSG_BLK *blk);

extern void msg_blk_weekup(struct ISIL_MSG_BLK *blk);


extern void glb_msg_mgt_init( void );

extern struct ISIL_MSG_MGT *get_glb_msg_mgt( void );





#ifdef __cplusplus
}
#endif

#endif
