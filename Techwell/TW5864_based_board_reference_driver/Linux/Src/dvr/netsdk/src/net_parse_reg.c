#include "net_parse_reg.h"

net_parse_reg_t glb_net_parse_reg_t;

net_parse_reg_t *get_glb_net_parse_reg_t( void )
{
    return &glb_net_parse_reg_t;
}

void reg_glb_net_ctl_handle_cb(NET_TASK_CALLBACK *cb)
{
    glb_net_parse_reg_t.net_ctl_cb = cb;
}


void reg_glb_net_data_handle_cb(NET_TASK_CALLBACK *cb)
{
    glb_net_parse_reg_t.net_data_cb = cb;
}

void reg_glb_net_local_handle_cb(NET_TASK_CALLBACK *cb)
{
    glb_net_parse_reg_t.net_local_cb = cb;
}

