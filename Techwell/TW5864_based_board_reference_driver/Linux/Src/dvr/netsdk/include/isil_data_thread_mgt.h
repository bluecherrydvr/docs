#ifndef  ISIL_DATA_THREAD_MGT_H
#define  ISIL_DATA_THREAD_MGT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>


#include "net_task.h"
#include "isil_net_config.h"

struct ISIL_DATA_THR_MGT;

typedef void (DATA_THR_CALLBACK)(struct ISIL_DATA_THR_MGT *data_thr_mgt);

struct ISIL_DATA_THR_MGT{
    struct NET_TASK_MGT net_tasks_mgt[DEFAULT_NET_DATA_THR_NUM];
   
};





extern void glb_data_thr_mgt_init( void );

extern void glb_data_thr_mgt_release(void);

extern struct ISIL_DATA_THR_MGT *get_glb_data_thr_mgt(void);

extern int net_task_mgt_thr_create(struct NET_TASK_MGT *net_task_mgt);

extern int net_data_thr_alloc_task(struct NET_TASK *net_task);



#ifdef __cplusplus
}
#endif

#endif

