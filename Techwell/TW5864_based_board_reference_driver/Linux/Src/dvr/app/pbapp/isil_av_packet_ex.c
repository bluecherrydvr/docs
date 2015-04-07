#include <stdlib.h>
#include <stdio.h>

#include "isil_av_packet_ex.h"

ISIL_AV_PACKET *alloc_av_packet_ex( int len)
{
    
    ISIL_AV_PACKET *av_packet;

    if( !len ) {
        return NULL;
    }

    av_packet = isil_av_packet_minit();
    if( !av_packet ) {
        return NULL;
    }

    av_packet->buff = (unsigned char *)malloc(len);
    if( !av_packet->buff ) {
        fprintf(stderr,"Malloc pck buff err .\n");
        free(av_packet);
        av_packet = NULL;
        return NULL;
    }

    av_packet->buff_size = len;

    return av_packet;

}


void release_av_packet_ex(ISIL_AV_PACKET *av_packet)
{
    if(av_packet) {

        if(av_packet->buff) {
            free(av_packet->buff);
            av_packet->buff = NULL;
        }

        free(av_packet);
        av_packet = NULL;
    }
}


static void data_node_pck_release_cb(struct NET_DATA_NODE_T *net_data_node)
{
    ISIL_AV_PACKET *av_packet;

    if(net_data_node) {

        pthread_mutex_lock(&net_data_node->lock);
        if(!net_data_node->ref) {
            pthread_mutex_unlock(&net_data_node->lock);
            return;
        }

        --net_data_node->ref;
        if(!net_data_node->ref) {
    
            if(net_data_node->arg) {
                
                
                av_packet = (ISIL_AV_PACKET *)net_data_node->arg;
                release_av_packet_ex(av_packet);
                net_data_node->arg = NULL;
    
            }
            
            pthread_mutex_unlock(&net_data_node->lock);
            free(net_data_node);
            net_data_node = NULL;
            return;
        }

        pthread_mutex_unlock(&net_data_node->lock);

    }
}


struct NET_DATA_NODE_T * put_av_packet_to_net_data_node(ISIL_AV_PACKET *av_packet)
{
    struct NET_DATA_NODE_T *data_node = NULL;

    if(av_packet) {
        data_node = net_data_node_minit();
        if( !data_node ) {
            return NULL;
        }
        
        data_node->arg = (void *)av_packet;
        data_node->release = data_node_pck_release_cb;
    }

    return data_node;
}
