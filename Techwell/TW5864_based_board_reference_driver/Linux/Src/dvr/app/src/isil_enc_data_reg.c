#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "isil_enc_data_reg.h"
#include "isil_data_stream.h"
#include "isil_net_config.h"
#include "net_debug.h"


static void net_data_node_release_cb(struct NET_DATA_NODE_T *net_data_node)
{
    ISIL_AV_PACKET *av_packet;

    if(net_data_node) {

#ifdef USE_NEW_DATA_DISPATCH_POLICY
        
        //DEBUG_FUNCTION();
        del_data_node_self_from_send_lst(net_data_node);
        del_self_data_node_from_mgt(net_data_node);
        //DEBUG_FUNCTION();
#endif

        pthread_mutex_lock( &net_data_node->lock );

        if(net_data_node->arg) {
            
            
            av_packet = (ISIL_AV_PACKET *)net_data_node->arg;
            if(av_packet) {
                isil_av_packet_release(av_packet);
            }
            net_data_node->arg = NULL;

        }



        if( !net_data_node->ref ) {
            pthread_mutex_unlock( &net_data_node->lock );
            DEBUG_FUNCTION();
            return;
        }

        --net_data_node->ref;

        if(  !net_data_node->ref ) {
            pthread_mutex_unlock( &net_data_node->lock );
            free(net_data_node);
            net_data_node = NULL;
            return;
        }

        pthread_mutex_unlock( &net_data_node->lock );

    }
}

static int net_notify_data_handle_cb(void *src ,struct NET_NOTIFY_T *net_notify)
{
    
    ISIL_AV_PACKET *av_packet;
    struct NET_TASK *net_task;
    struct NET_DATA_MGT_T   *net_data_mgt_ptr;

    struct NET_DATA_NODE_T *data_node_p;

    if( !src ) {
        return -1;
    }

    if( !net_notify->param ) {
        assert(0);
        return -1;
    }

    net_task = (struct NET_TASK *)net_notify->param;

    net_data_mgt_ptr =  net_task->net_data_mgt;
    if( !net_data_mgt_ptr ) {
        
        return -1;
    }
    
    data_node_p = net_data_node_minit();
    if( !data_node_p ) {
        assert(0);
        return -1;
    }
    
    //TODO: clone av_packet
    av_packet = (ISIL_AV_PACKET *)src;

    
    isil_av_packet_clone(av_packet);
    
    data_node_p->arg = src;
    data_node_p->release = net_data_node_release_cb;
    


    net_data_mgt_ptr->put_net_data_node( net_data_mgt_ptr , data_node_p);
    

    if( net_task->task_mgt ) {

#ifdef USE_NEW_DATA_DISPATCH_POLICY
        net_task->task_mgt->put_data_node(net_task->task_mgt ,data_node_p);
   
#else
        if(net_task->task_mgt->wakeup) {
            net_task->task_mgt->wakeup(net_task->task_mgt);
        }
#endif
    }
    

    
    return 0;
    
}


int net_task_reg_enc(struct NET_TASK *net_task,
                                unsigned int chipId,
                                unsigned int chanId, 
                                enum ECHANFUNCPRO chanpro )
{

    struct ENC_DATA_REG_T *data_reg;
    struct NET_NOTIFY_T *net_nty_ptr;
    struct NET_NOTIFY_RECORD_T *net_nty_record;

    if( !net_task ) {
        return -1;
    }
    
    DEBUG_FUNCTION();
    data_reg = get_enc_data_reg_by_inf( chipId ,chanId , chanpro);
    if( !data_reg ) {
        return -1;
    }
    DEBUG_FUNCTION();

    struct NET_NOTIFY_REG_T *nty_reg = &data_reg->nty_reg;

    net_nty_ptr = net_notify_minit();
    if( !net_nty_ptr ) {
        return -1;
    }

    DEBUG_FUNCTION();
    if( !net_task->nty_rec_t ) {
    
        net_task->nty_rec_t = net_notify_record_minit();
        if( !net_task->nty_rec_t ) {
            fprintf(stderr,"net_notify_record_minit err ,.\n");
            return -1;
        }

    }
    DEBUG_FUNCTION();
    net_nty_record = net_task->nty_rec_t;

    net_notify_callback_reg(net_nty_ptr , (void *)net_task , net_notify_data_handle_cb );
    
    nty_reg->put_net_nty(net_nty_ptr ,nty_reg);

    net_nty_record->put_net_nty(net_nty_ptr ,net_nty_record );
    //TODO :reg media enc
    DEBUG_FUNCTION();
    return 0;
  
}
