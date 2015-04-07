#ifndef   _ISIL_ENC_DATA_REG_H
#define  _ISIL_ENC_DATA_REG_H

#ifdef __cplusplus
extern "C" {
#endif


#include "isil_enc_data_mgt_cent.h"

#include "net_task.h"

#include "isil_net_nty.h"

#include "isil_media_config.h"



extern int net_task_reg_enc(struct NET_TASK *net_task,
                                unsigned int chipId,
                                unsigned int chanId, 
                                enum ECHANFUNCPRO chanpro );


#ifdef __cplusplus
}
#endif

#endif
