#ifndef   _ISIL_FILE_REC_MGT_H
#define  _ISIL_FILE_REC_MGT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "net_task.h"
#include "isil_media_config.h"
#include "isil_net_config.h"

struct FILE_REC_TASK_T
{
    unsigned int chip_id;
    unsigned int chan_id;
    unsigned int logic_chan_id;
    enum ECHANFUNCPRO chanpro;

    struct NET_TASK* task;
	
    
};

struct ISIL_FILE_RECORD_THR_MGT{
    struct NET_TASK_MGT file_tasks_mgt[DEFAULT_FILE_RECORD_THR_NUM];
};


extern void init_glb_file_rec_task_array( void );

extern struct FILE_REC_TASK_T *get_file_rec_task_by_inf(unsigned int chip_num,
                                                        unsigned int chan_num );


extern void glb_file_rec_thr_mgt_init( void );

extern struct ISIL_FILE_RECORD_THR_MGT *get_glb_file_rec_thr_mgt(void);

extern void glb_file_rec_thr_mgt_release(void);

extern int file_rec_task_mgt_thr_create(struct NET_TASK_MGT *net_task_mgt);

extern int file_rec_thr_alloc_task(struct NET_TASK *net_task);






#ifdef __cplusplus
}
#endif

#endif


