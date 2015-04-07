#ifndef  _ISIL_EV_THR_MGT_H
#define  _ISIL_EV_THR_MGT_H



#ifdef __cplusplus
extern "C" {
#endif

#include "isil_sdk_config.h"
#include "isil_ev_task.h"


typedef struct _ISIL_ENC_THR_MGT{
    struct ISIL_TASK_MGT enc_task_mgt[DEFAULT_ENC_THR_NUM];
}ISIL_ENC_THR_MGT_T;


extern void glb_enc_thr_mgt_init( void );

extern ISIL_ENC_THR_MGT_T *get_glb_enc_thr_mgt(void);

extern void glb_enc_thr_mgt_release(void);

extern int task_mgt_thr_create(struct ISIL_TASK_MGT *task_mgt);

extern int enc_task_thr_alloc_task(struct ISIL_EV_TASK *ev_task);





#ifdef __cplusplus
}
#endif

#endif



