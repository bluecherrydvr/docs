#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "net_task.h"
#include "isil_net_send_session.h"
#include "isil_net_data_header.h"
#include "isil_data_stream.h"
#include "isil_win_net_header.h"
#include "net_debug.h"



static void update_h264_sps_buff(struct NET_TASK *ev_task ,ISIL_AV_PACKET *av_pck)
{

	struct h264_param_t *h264_param_ptr;

	if( !ev_task->enc_param ) {
		return;
	}

	if( !ev_task || !av_pck ) {
		return;
	}

	if( !av_pck->buff || !av_pck->date_len ) {
		return;
	}

	if(av_pck->date_len > ISIL_SPS_BUFF_LEN) {
		return;
	}

	h264_param_ptr = (struct h264_param_t *)ev_task->enc_param;

	memcpy(h264_param_ptr->sps_buff ,av_pck->buff ,av_pck->date_len);
	h264_param_ptr->sps_len = av_pck->date_len;

	return;
}



static void update_h264_pps_buff(struct NET_TASK *ev_task ,ISIL_AV_PACKET *av_pck)
{

	struct h264_param_t *h264_param_ptr;

	if( !ev_task->enc_param ) {
		return;
	}

	if( !ev_task || !av_pck ) {
		return;
	}

	if( !av_pck->buff || !av_pck->date_len ) {
		return;
	}

	if(av_pck->date_len > ISIL_PPS_BUFF_LEN) {
		return;
	}

	h264_param_ptr = (struct h264_param_t *)ev_task->enc_param;

	memcpy(h264_param_ptr->pps_buff ,av_pck->buff ,av_pck->date_len);
	h264_param_ptr->pps_len = av_pck->date_len;

	return;
}


static int send_data_net(struct NET_TASK *ev_task ,struct NET_DATA_NODE_T *data_nod)
{
	int ret;

	ISIL_AV_PACKET *av_pck;
    struct sockaddr_in *sa = NULL;

	if( !ev_task || !data_nod ) {
		return -1;
	}

	if( !data_nod->arg ) {
		return -1;
	}

	av_pck = (ISIL_AV_PACKET *)(data_nod->arg);

	if( !av_pck->buff ) {
		return -1;
	}

    //isil_av_packet_debug(av_pck);
    
    
#if 1
    if(ev_task->trans_type == NET_TCP_E) {
        sa = NULL;
    }
    else{
        sa = &ev_task->sa;
    }
#endif

	switch (av_pck->frm_type) {
	case ISIL_H264_DATA:{

		H264_INF_T *h264_inf_ptr = (H264_INF_T *)( &(av_pck->data_type_u.h264_inf));

		if(h264_inf_ptr->nalu_type == H264_NALU_SPS) {
			update_h264_sps_buff(ev_task,av_pck);
		}
		else if(h264_inf_ptr->nalu_type == H264_NALU_PPS) {
			update_h264_pps_buff(ev_task ,av_pck);
		}
		else if(h264_inf_ptr->nalu_type == H264_NALU_IDRSLICE) {

			if( !ev_task->enc_param ) {
				return -1;
			}

            

			struct h264_param_t *h264_param_ptr = (struct h264_param_t *)ev_task->enc_param;

			ret = net_handle_5864_IDR_frame(ev_task->fd,
											(void*)sa,
											av_pck,
											REALSTREAM,
											0,
											(void *)h264_param_ptr->sps_buff,
											h264_param_ptr->sps_len,
											(void *)h264_param_ptr->pps_buff,
											h264_param_ptr->pps_len);

			if( ret < 0 ) {
				return -1;
			}
			
			
		}

		else if( h264_inf_ptr->nalu_type == H264_NALU_PSLICE ){

			

			if( ev_task->frame_type == NET_ONLY_NEED_I_FRAME_E ){
				
					DEBUG_FUNCTION();
					return 0;
				
			}

			ret = net_handle_5864_I_P_frame(ev_task->fd,
											(void *)sa,
											av_pck,
											REALSTREAM,
											0);

			if( ret < 0 ) {
				return -1;
			}

		}
		else if( h264_inf_ptr->nalu_type == H264_NALU_ISLICE ) {

			ret = net_handle_5864_I_P_frame(ev_task->fd,
											(void *)sa,
											av_pck,
											REALSTREAM,
											0);

			if( ret < 0 ) {
				return -1;
			}

		}
		else{
			fprintf(stderr,"h264_inf nal type[%d] unkown .\n",h264_inf_ptr->nalu_type);
			return  0;
		}
	}
		break;
	case ISIL_AUDIO_DATA:{
		ret = net_handle_5864_audio_frame(ev_task->fd , (void *)sa,av_pck ,REALSTREAM,0);
		if( ret < 0 ) {
			return -1;
		}
	}
		break;
	case ISIL_MJPEG_DATA:{

		ret = net_handle_5864_MJPEG_frame(ev_task->fd , (void *)sa, av_pck );
		if( ret < 0 ) {
			return -1;
		}
	}
		break;
	default:
		return -1;
	}

	return 0;

}



int send_enc_data_to_client_cb(struct NET_TASK *ev_task ,void *src)
{
	int ret;
	struct NET_DATA_NODE_T *data_nod;
    struct NET_DATA_MGT_T *data_mgt;
    

    if( !ev_task ) {
        return -1;
    }

    data_mgt = ev_task->net_data_mgt;
    if( !data_mgt ) {
        return -1;
    }


    do{

        
        data_nod = data_mgt->get_net_data_node(data_mgt);
        if( !data_nod ) {
            
            break ;
        }
                
        //TODO : send to net
		ret = send_data_net(ev_task,data_nod);
		if( ret < 0) {
			data_nod->release(data_nod);
			return -1;
		}
        

        data_nod->release(data_nod);
        

    }while(1);

    

    return 0;
}


int new_send_enc_data_to_client_cb(struct NET_TASK *ev_task ,void *src)
{
	int ret;

	struct NET_DATA_NODE_T *data_nod;
    
    

    if( !ev_task || !src ) {
        return -1;
    }
    
    data_nod = (struct NET_DATA_NODE_T *)src;
    ret = send_data_net(ev_task,data_nod);
    if( ret < 0) {
        
        fprintf( stderr,"send_data_net err .\n");
        return -1;
    }
    
    return 0;
}
