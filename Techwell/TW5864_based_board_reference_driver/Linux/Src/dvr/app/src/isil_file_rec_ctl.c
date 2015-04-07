#include <stdio.h>
#include <stdlib.h>

#include "isil_file_rec_ctl.h"
#include "isil_file_rec_mgt.h"
#include "isil_enc_data_reg.h"
#include "isil_media_config.h"





int reg_file_rec_ev(struct NET_TASK *net_task , 
                                unsigned int chipId,
                                unsigned int chanId, 
                                enum ECHANFUNCPRO chanpro)
{


    
    if( !net_task ) {
        return -1;
    }

    struct FILE_REC_TASK_T *file_rec_task = get_file_rec_task_by_inf(chipId,chanId );
    if( !file_rec_task ) {
        return -1;
    }

    if(file_rec_task->task) {
        return -1;
    }

    file_rec_task->task = net_task;

    

    

#if 0
    ret = net_task_reg_enc(net_task,chipId,chanId,chanpro);
    if( ret < 0 ) {
        return -1;
    }
#endif    
    
    return 0;

}
