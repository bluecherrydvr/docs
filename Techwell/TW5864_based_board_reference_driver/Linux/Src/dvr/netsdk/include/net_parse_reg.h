#ifndef   NET_PARSE_REG_H
#define   NET_PARSE_REG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "net_task.h"


typedef struct _net_parse_reg_t{
    
    NET_TASK_CALLBACK *net_ctl_cb;
    NET_TASK_CALLBACK *net_data_cb;
	NET_TASK_CALLBACK *net_local_cb;
	NET_TASK_CALLBACK *net_async_notify_cb;

}net_parse_reg_t;


extern void reg_glb_net_ctl_handle_cb(NET_TASK_CALLBACK *cb);

extern void reg_glb_net_data_handle_cb(NET_TASK_CALLBACK *cb);

extern void reg_glb_net_local_handle_cb(NET_TASK_CALLBACK *cb);

extern net_parse_reg_t *get_glb_net_parse_reg_t( void );

extern void init_net_msg_reg( void );

extern void init_glb_net_parse_reg_t( void );







#ifdef __cplusplus
}
#endif

#endif


