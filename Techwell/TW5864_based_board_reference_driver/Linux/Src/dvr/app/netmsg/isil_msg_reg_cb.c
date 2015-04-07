

#include "isil_msg_reg_cb.h"

static MSG_FILE_DEC_REG_T glb_file_dec_reg_t;

static MSG_PREVIEW_DEC_REG_T  glb_preview_dec_reg_t;

static net_msg_reg_func glb_net_msg_reg_t;

net_msg_reg_func *get_glb_net_msg_reg_t( void )
{
    return &glb_net_msg_reg_t;
}


MSG_FILE_DEC_REG_T *get_glb_file_dec_reg_ptr( void )
{
    return &glb_file_dec_reg_t;
}


MSG_PREVIEW_DEC_REG_T *get_glb_preview_dec_reg_t( void )
{
    return &glb_preview_dec_reg_t;
}

