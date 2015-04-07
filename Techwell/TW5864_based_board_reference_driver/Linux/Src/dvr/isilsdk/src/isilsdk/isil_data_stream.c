#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "isil_data_stream.h"
#include "isil_frame.h"
#include "isil_debug.h"





void isil_av_packet_release(ISIL_AV_PACKET *av_packet)
{
    if( av_packet ) {
        
        pthread_mutex_lock(&av_packet->lock);
        if( av_packet->ref <= 0 ) {
            pthread_mutex_unlock(&av_packet->lock);
            return;
        }
        
        --av_packet->ref; 
        
        if( !av_packet->ref ) {
            
            if( av_packet->priv ) {
                
                
                frame_t *frame = (frame_t *)(av_packet->priv);
                
                frame->release(frame);
                av_packet->priv = NULL;
            }
            else{

                if(av_packet->buff && av_packet->buff_size) {
                    free(av_packet->buff);
                    av_packet->buff = NULL;
                }
            }
            
            pthread_mutex_unlock(&av_packet->lock);
            pthread_mutex_destroy(&av_packet->lock);
            
            free(av_packet);
            av_packet = NULL;

            return;
            
            
        }

        pthread_mutex_unlock(&av_packet->lock);
        
        
    }
}


ISIL_AV_PACKET * isil_av_packet_minit( void )
{
    ISIL_AV_PACKET *av_packet;
    av_packet = (ISIL_AV_PACKET *)calloc( 1 , sizeof(ISIL_AV_PACKET));
    if( !av_packet ) {
        return NULL;
    }

    pthread_mutex_init(&av_packet->lock ,NULL);

    isil_av_packet_clone(av_packet);
    return av_packet;
}


void isil_av_packet_debug(ISIL_AV_PACKET *av_packet)
{
    if(av_packet) {
        fprintf(stderr,"\nAV_PACKET:chip[%u],"
                       "chan[%u],"
                       "data_len[%d],"
                       "\nfrm_seq[%u]"
                       "pts[%u]"
                       "frm_type[%d],"
                       "ref[%d].\n",
                av_packet->chip_id,
                av_packet->chan_id,
                av_packet->date_len,
                av_packet->frm_seq,
                av_packet->pts,
                av_packet->frm_type,
                av_packet->ref);
        if( av_packet->frm_type == ISIL_H264_DATA ) {
            H264_INF_T *h264_inf_ptr;
            h264_inf_ptr = &(av_packet->data_type_u.h264_inf);
            fprintf(stderr,"stream_type[%d],nalu_type[%d].\n",
                    h264_inf_ptr->stream_type,
                    h264_inf_ptr->nalu_type);
        }
    }
}


void isil_av_packet_bit_debug(ISIL_AV_PACKET *av_packet)
{
    int i,len;
    char *tmp;
    
    len = sizeof(ISIL_AV_PACKET);
    tmp = (char *)av_packet;
    printf("\n\n");
    for( i = 0 ; i <  len ; i++) {
        printf("%d ",tmp[i]);
        
    }
    printf("\n");
}


ISIL_ALARM_PACKET *isil_alarm_packet_minit( void )
{
    ISIL_ALARM_PACKET *alarm_pck = (ISIL_ALARM_PACKET *)calloc( 1 , sizeof(ISIL_ALARM_PACKET));
    if( !alarm_pck ) {
        return NULL;
    }

    isil_alarm_packet_clone(alarm_pck);

    return alarm_pck;

    
}


void isil_alarm_packet_release(ISIL_ALARM_PACKET *alarm_packet)
{
    if(alarm_packet) {

        if( !alarm_packet->ref ) {
            return;
        }
        
        isil_alarm_packet_ref_minus(alarm_packet);

        if( !alarm_packet->ref ) {
            free(alarm_packet);
            alarm_packet = NULL;
        }
    }
}







