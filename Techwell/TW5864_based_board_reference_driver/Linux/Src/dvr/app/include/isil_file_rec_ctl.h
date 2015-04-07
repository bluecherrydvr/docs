#ifndef  _ISIL_FILE_REC_CTL_H
#define  _ISIL_FILE_REC_CTL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "net_task.h"
#include "isil_media_config.h"
#include "isil_net_config.h"


extern int reg_file_rec_ev(struct NET_TASK *net_task , 
                                unsigned int chipId,
                                unsigned int chanId, 
                                enum ECHANFUNCPRO chanpro);





#ifdef __cplusplus
}
#endif

#endif
