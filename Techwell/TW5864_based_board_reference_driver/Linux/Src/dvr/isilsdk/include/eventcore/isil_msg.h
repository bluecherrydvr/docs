#ifndef  _ISIL_MSG_H
#define _ISIL_MSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "isil_msg_mgt.h"


struct isil_msg_header{
    int msg_type;
    int msg_num;
    int msg_total_len;
};


extern int isil_msg_set_param(void *arg ,msg_callback *hook ,int timeout);


#ifdef __cplusplus
}
#endif

#endif

