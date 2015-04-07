#include "isil_msg.h"
#include "isil_debug.h"

int isil_msg_set_param(void *arg ,msg_callback *hook ,int timeout)
{
    int ret;
    struct ISIL_MSG_MGT *msg_mgt;

    if( !hook ) {
        return -1;
    }

    
    struct ISIL_MSG_BLK *msg_blk = msg_blk_minit();
    if( !msg_blk ) {
        return -1;
    }

    reg_msg_callback(msg_blk ,arg ,hook);

    msg_mgt = get_glb_msg_mgt();
    msg_mgt->put_blk(msg_blk,msg_mgt);
    

    if(msg_blk->wait) {
        msg_blk->wait(msg_blk);
    }

    
    ret = GET_MSG_BLK_RESULT(msg_blk);

    if( msg_blk->release ) {
        DEBUG_FUNCTION();
        msg_blk->release(msg_blk);
        DEBUG_FUNCTION();
    }
    
    return ret;

}
