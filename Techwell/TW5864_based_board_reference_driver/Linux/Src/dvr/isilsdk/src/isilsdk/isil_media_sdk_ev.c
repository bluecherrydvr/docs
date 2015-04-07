#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "isil_media_sdk_ev.h"
#include "isil_frame.h"
#include "isil_media_config.h"
#include "isil_encoder.h"
#include "isil_codec_errcode.h"
#include "isil_msg.h"
#include "isil_sdk_config.h"
#include "isil_ev_thr_mgt.h"
#include "isil_io_utils.h"
#include "isil_debug.h"
#include "isil_codec_common.h"
#include "isil_drv_to_sdk_data.h"


static int chip_fd_map[4];
static int get_alarm_chip_id_by_fd(int fd){
    int i;
    for(i = 0; i < 4; i++) {
        if(chip_fd_map[i] == fd) {
            return i;
        }
    }
    return -1;
}


S_ISIL_SINGLE_CHAN_MGT * isil_open_enc_chn(int chipid,int chn ,enum ECHANFUNCPRO type,int enable)
{
    
   
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;


    sin_chan_mgt = get_sin_chn_mgt_by_chip_chn_type(chipid,chn,CHAN_IS_ENC,type);
    if( !sin_chan_mgt ) {
        fprintf(stderr,"get_sin_chn_mgt_by_chip_chn_type err .\n");
        return NULL;
    }

     
    

     if( enable ) {
         
        DEBUG_FUNCTION();
        if( GET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt) != 1 ) {
            
            if( ISIL_CODEC_ENC_EnableCh(&sin_chan_mgt->codec_channel_t) != CODEC_ERR_OK ){
                fprintf(stderr, "ISIL_CODEC_ENC_EnableCh err Line[%d] .\n", __LINE__);
                sin_chan_mgt->release(sin_chan_mgt);
                return NULL;
            }
            
            
        }
        DEBUG_FUNCTION();
        
        SET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt,1);

     }
     else{
        
         if(GET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt) != 0) {
             
             DEBUG_FUNCTION();
             if( ISIL_CODEC_ENC_DisableCh(&sin_chan_mgt->codec_channel_t) != CODEC_ERR_OK ){
                fprintf(stderr, "ISIL_CODEC_ENC_EnableCh err Line[%d] .\n", __LINE__);
               
                sin_chan_mgt->release(sin_chan_mgt);
                
                return NULL;
            }

            fprintf(stderr, "start ISIL_CODEC_ENC_Flush.\n");
            if(ISIL_CODEC_ENC_Flush(&sin_chan_mgt->codec_channel_t) != 0){
                fprintf(stderr, "ISIL_CODEC_ENC_Flush err .\n");
            }
            DEBUG_FUNCTION();
            
         }
         
         
         SET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt,0);
         
     }
     
   
     return sin_chan_mgt;

}


#ifdef USEOLDHEADER

#include "isil_old_drv_header.h"

int old_drv_frame_to_packet(frame_t *frame,struct ISIL_EV_TASK *ev_task)
{
    unsigned int chip_id,chan_id,i;
    STREAM_TYPE_E stream_type;
    ISIL_AV_PACKET * av_packet;
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    struct ISIL_NOTIFY_LST *notify_lst;
    

    notify_lst = &ev_task->notify_list;

    if(!frame || !ev_task ) {
        return -1;
    }

    if( !ev_task->priv ) {
        return -1;
    }

    sin_chan_mgt = (S_ISIL_SINGLE_CHAN_MGT *)ev_task->priv;
    if(sin_chan_mgt->codec_channel_t.eCodecType == CODEC_VIDEO_H264) {

        if(sin_chan_mgt->codec_channel_t.eStreamType == STREAM_TYPE_MAJOR ) {
            stream_type = ISIL_MAIN_STREAM_E;
        }
        else{
            stream_type = ISIL_SUB_STREAM_E;
        }
    }

    chip_id = sin_chan_mgt->codec_channel_t.u32ChipID;
    chan_id = sin_chan_mgt->codec_channel_t.u32Ch;

    

    struct isil_frame_header *frame_h_ptr = (struct isil_frame_header *)frame->buff;
    

    if(frame_h_ptr->codecType == ISIL_VIDEO_H264_CODEC_TYPE) {
        
        if( frame_h_ptr->frameType == H264_FRAME_TYPE_IDR ) {
            struct isil_h264_idr_frame_pad *idr_inf = (struct isil_h264_idr_frame_pad *)frame_h_ptr->pad;
            
            
            for(i = 0 ; i < 3 ; i++) {

                av_packet = isil_av_packet_minit();
                if( !av_packet ) {
                    return -1;
                }

                av_packet->chan_id = chan_id;
                av_packet->chip_id = chip_id;
                av_packet->pts = frame_h_ptr->timeStamp;
                av_packet->frm_seq = frame_h_ptr->frameSerial;
                av_packet->frm_type = ISIL_H264_DATA;
                av_packet->data_type_u.h264_inf.stream_type = stream_type;
                if( i == 0 ) {
                    av_packet->data_type_u.h264_inf.nalu_type = H264_NALU_SPS;
                    av_packet->buff = (unsigned char *)idr_inf->nal;
                    av_packet->date_len = idr_inf->sps_frame_size;
                }
                else if( i == 1 ) {
                    av_packet->data_type_u.h264_inf.nalu_type = H264_NALU_PPS;
                    av_packet->date_len = idr_inf->pps_frame_size;
                    av_packet->buff = (unsigned char *)(idr_inf->nal + idr_inf->sps_frame_size);
                }
                else{
                    av_packet->data_type_u.h264_inf.nalu_type = H264_NALU_IDRSLICE;
                    av_packet->date_len = idr_inf->idr_frame_size;
                    av_packet->buff = (unsigned char *)(idr_inf->nal + idr_inf->sps_frame_size + idr_inf->pps_frame_size);
                }

                av_packet->priv = (void *)frame;
                add_frame_ref(frame);
                notify_lst->notify(notify_lst ,(void *)av_packet);
        
                isil_av_packet_release(av_packet);



            }

        }
        else{

            av_packet = isil_av_packet_minit();
            if( !av_packet ) {
                return -1;
            }

            av_packet->buff = (unsigned char *)frame_h_ptr->pad;
            av_packet->date_len = frame_h_ptr->payloadLen;
            
            av_packet->chan_id = chan_id;
            av_packet->chip_id = chip_id;
            av_packet->pts = frame_h_ptr->timeStamp;
            av_packet->frm_seq = frame_h_ptr->frameSerial;
            av_packet->frm_type = ISIL_H264_DATA;
            av_packet->data_type_u.h264_inf.stream_type = stream_type;
            av_packet->data_type_u.h264_inf.nalu_type = H264_NALU_PSLICE;
            
            av_packet->priv = (void *)frame;
            add_frame_ref(frame);
            notify_lst->notify(notify_lst ,(void *)av_packet);
        
            isil_av_packet_release(av_packet);

        }

    }
    else{
        return -1;
    }


    return 0;


}

#endif



int frame_to_av_packet(frame_t *frame,struct ISIL_EV_TASK *ev_task)
{
    unsigned int chip_id,chan_id,msg_type ,i;
    int offset = 0;
    STREAM_TYPE_E stream_type;
    ISIL_AV_PACKET * av_packet;
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    H264_INF_T *h264_inf_ptr;

    isil_common_stream_header_t* commheader;
    isil_transparent_msg_header_t* msgheader;
    isil_h264_encode_msg_frame_type_t* h264_frame_type = NULL;
    isil_audio_param_msg_t* aud_param = NULL;
    isil_jpeg_param_msg_t* mjpg_param = NULL;
    struct ISIL_NOTIFY_LST *notify_lst;


    notify_lst = &ev_task->notify_list;
    

    if(!frame || !ev_task ) {
        DEBUG_FUNCTION();
        return -1;
    }

    if( !ev_task->priv ) {
        DEBUG_FUNCTION();
        return -1;
    }

    sin_chan_mgt = (S_ISIL_SINGLE_CHAN_MGT *)ev_task->priv;

    if(sin_chan_mgt->codec_channel_t.eCodecType == CODEC_VIDEO_H264) {

        if(sin_chan_mgt->codec_channel_t.eStreamType == STREAM_TYPE_MAJOR ) {
            stream_type = ISIL_MAIN_STREAM_E;
        }
        else{
            stream_type = ISIL_SUB_STREAM_E;
        }
    }

    chip_id = sin_chan_mgt->codec_channel_t.u32ChipID;
    chan_id = sin_chan_mgt->codec_channel_t.u32Ch;

    commheader = (isil_common_stream_header_t*)(frame->buff);
    offset += sizeof(isil_common_stream_header_t);
    //fprintf(stderr,"offset[%d] .\n",offset);
    if(commheader->stream_len & ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG){

        msgheader = (isil_transparent_msg_header_t*)(frame->buff + offset);
        offset += sizeof(isil_transparent_msg_header_t);
        //fprintf(stderr,"offset[%d] .\n",offset);
        if( !msgheader->isil_transparent_msg_number ) {
            DEBUG_FUNCTION();
            return -1;
        }

        //fprintf(stderr,"isil_transparent_msg_number[%d] .\n",msgheader->isil_transparent_msg_number);
        for(i = 0 ; i < msgheader->isil_transparent_msg_number ; i++) {
            msg_type = *((unsigned int*)(frame->buff + offset ));
            //fprintf(stderr,"offset[%d] ,msg_type[%d].\n",offset,msg_type);
            switch( msg_type ){
            case ISIL_H264_ENCODE_FRAME_TYPE:{

                h264_frame_type = (isil_h264_encode_msg_frame_type_t *)(frame->buff + offset);
                offset += h264_frame_type->msg_len;

            }
                break;

            case ISIL_AUDIO_ENCODE_PARAM_MSG:{
                //DEBUG_FUNCTION();
                aud_param = (isil_audio_param_msg_t *)(frame->buff + offset);
                offset += sizeof(isil_audio_param_msg_t);
                //offset += aud_param->msg_len;
                //fprintf(stderr,"offset[%d] ,aud_param->msg_len[%d].\n",offset,aud_param->msg_len);
            }
                break;

            case ISIL_MJPEG_ENCODE_PARAM_MSG:{
                //DEBUG_FUNCTION();
                mjpg_param = (isil_jpeg_param_msg_t *)(frame->buff + offset);
                offset += sizeof(isil_jpeg_param_msg_t);
                //offset += mjpg_param->msg_len;
                //fprintf(stderr,"offset[%d] ,mjpg_param->msg_len[%d].\n",offset,mjpg_param->msg_len);
            }
                break;

            case ISIL_MJPEG_ENCODE_FRAME_MSG:{
                
                if(!mjpg_param) {
                    return -1;
                }

                isil_frame_msg_t* frame_msg = (isil_frame_msg_t *)(frame->buff + offset);
                av_packet = isil_av_packet_minit();
                if( !av_packet ) {
                    return -1;
                }

                av_packet->chan_id = chan_id;
                av_packet->chip_id = chip_id;

                av_packet->pts = frame_msg->frame_timestamp;
                av_packet->frm_seq = frame_msg->frame_serial;
                av_packet->buff = frame_msg->frame_payload;
                av_packet->date_len = frame_msg->msg_len - sizeof(isil_frame_msg_t);
                
                av_packet->frm_type = ISIL_MJPEG_DATA;

                if( mjpg_param->capture_type == (unsigned int)ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER) {
                    av_packet->data_type_u.mjpeg_inf.capture_type = ISIL_MJPEG_CAPTURE_TYPE_TIMER;
                    
                }
                else{
                   av_packet->data_type_u.mjpeg_inf.capture_type =  ISIL_MJPEG_CAPTURE_TYPE_USER;
                }

                av_packet->data_type_u.mjpeg_inf.image_size.x_width = mjpg_param->i_mb_x;
                av_packet->data_type_u.mjpeg_inf.image_size.y_height = mjpg_param->i_mb_y;

                av_packet->priv = (void *)frame;
                add_frame_ref(frame);

                //isil_av_packet_debug(av_packet);
                    /*todo */
                notify_lst->notify(notify_lst ,(void *)av_packet);
        
                isil_av_packet_release(av_packet);

                mjpg_param = NULL;    
                
                offset += frame_msg->msg_len;
                    
                }
                break;
			case ISIL_AUDIO_ENCODE_FRAME_MSG:{
                
                if( !aud_param ){
                    DEBUG_FUNCTION();
                    return -1;
                }

                isil_frame_msg_t* frame_msg = (isil_frame_msg_t *)(frame->buff + offset);

                av_packet = isil_av_packet_minit();
                if( !av_packet ) {
                    return -1;
                }

                av_packet->chan_id = chan_id;
                av_packet->chip_id = chip_id;
                av_packet->pts = frame_msg->frame_timestamp;
                av_packet->frm_seq = frame_msg->frame_serial;
                av_packet->buff = frame_msg->frame_payload;
                av_packet->date_len = frame_msg->msg_len - sizeof(isil_frame_msg_t);
                av_packet->frm_type = ISIL_AUDIO_DATA;
                av_packet->data_type_u.audio_inf.bit_rate  = aud_param->audio_bit_wide;
                av_packet->data_type_u.audio_inf.sample  = aud_param->audio_sample_rate;

                
                
                if(aud_param->audio_type == ISIL_AUDIO_PCM){

                    
						
                    av_packet->data_type_u.audio_inf.audio_type_e = ISIL_AUDIO_PCM_E;
                        
				}else{

                    

                    av_packet->data_type_u.audio_inf.audio_type_e = ISIL_AUDIO_ADPCM_E;
                    
				}

                /*todo:  ?*/
                av_packet->priv = (void *)frame;
                add_frame_ref(frame);

                //isil_av_packet_debug(av_packet);
                
                notify_lst->notify(notify_lst ,(void *)av_packet);
        
                isil_av_packet_release(av_packet);
                
                aud_param = NULL;
                
                offset += frame_msg->msg_len;
            }
                break;
            case ISIL_H264_ENCODE_FRAME_MSG:{

                if( !h264_frame_type ){

                    return -1;
                    
                }

                isil_frame_msg_t* frame_msg = (isil_frame_msg_t *)(frame->buff + offset);
                av_packet = isil_av_packet_minit();
                if( !av_packet ) {
                    return -1;
                }

                av_packet->chan_id = chan_id;
                av_packet->chip_id = chip_id;

                av_packet->pts = frame_msg->frame_timestamp;
                av_packet->frm_seq = frame_msg->frame_serial;
                av_packet->buff = frame_msg->frame_payload;
                av_packet->date_len = frame_msg->msg_len - sizeof(isil_frame_msg_t);
                av_packet->frm_type = ISIL_H264_DATA;

                h264_inf_ptr = &(av_packet->data_type_u.h264_inf);
                
                h264_inf_ptr->image_size.x_width = h264_frame_type->i_mb_x;
                
                h264_inf_ptr->image_size.y_height = h264_frame_type->i_mb_y;
                

                h264_inf_ptr->stream_type = stream_type;

                switch(h264_frame_type->frame_type) {
                case ISIL_SPS_FRAME:
                    
                    h264_inf_ptr->nalu_type = H264_NALU_SPS;
                    
                    //h264_inf_ptr->nalu_type = 7;
                    
                    break;
                case ISIL_PPS_FRAME:
                    
                    h264_inf_ptr->nalu_type = H264_NALU_PPS;
                    //h264_inf_ptr->nalu_type = 8;
                    
                    
                    break;
                case ISIL_IDR_FRAME:
                    
                    h264_inf_ptr->nalu_type = H264_NALU_IDRSLICE;
                    //h264_inf_ptr->nalu_type = 6;
                    
                    break;
                case ISIL_I_FRAME:
                    
                    h264_inf_ptr->nalu_type = H264_NALU_ISLICE;
                    //h264_inf_ptr->nalu_type = 5;
                    
                    break;
                case ISIL_P_FRAME:
                    
                    h264_inf_ptr->nalu_type = H264_NALU_PSLICE;
                    //h264_inf_ptr->nalu_type = 1;
                    
                    break;
                default:
                    fprintf(stderr,"NAL TYPE UNKOWN .\n");
                    isil_av_packet_release(av_packet);
                    return -1;
                }
                
                av_packet->priv = (void *)frame;
                add_frame_ref(frame);
                

                //isil_av_packet_debug(av_packet);
                //isil_av_packet_bit_debug(av_packet);
                
                notify_lst->notify(notify_lst ,(void *)av_packet);
        
                isil_av_packet_release(av_packet);
                
                h264_frame_type = NULL;
                
                offset += frame_msg->msg_len;
            }
                break;

            default:
                fprintf(stderr,"msg type[%d] offset[%d .\n].\n",msg_type ,offset);
                DEBUG_FUNCTION();
                return -1;

            }
        }
    }
    else {
        DEBUG_FUNCTION();
        return -1;
    }


    return 0;

}




ISIL_AV_PACKET *handle_frame_to_av_packet(frame_t *frame,
                                          struct ISIL_EV_TASK *ev_task) 
{
    unsigned int chip_id,chan_id;
    int ret;
    STREAM_TYPE_E stream_type;
    ISIL_AV_PACKET * av_packet;
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    

    if(!frame || !ev_task ) {
        return NULL;
    }

    if( !ev_task->priv ) {
        return NULL;
    }

    sin_chan_mgt = (S_ISIL_SINGLE_CHAN_MGT *)ev_task->priv;

    av_packet = isil_av_packet_minit();
    if( !av_packet ) {
        return NULL;
    }
    
    if(sin_chan_mgt->codec_channel_t.eCodecType == CODEC_VIDEO_H264) {

        if(sin_chan_mgt->codec_channel_t.eStreamType == STREAM_TYPE_MAJOR ) {
            stream_type = ISIL_MAIN_STREAM_E;
        }
        else{
            stream_type = ISIL_SUB_STREAM_E;
        }
    }
    chip_id = sin_chan_mgt->codec_channel_t.u32ChipID;
    chan_id = sin_chan_mgt->codec_channel_t.u32Ch;
    ret = parse_drv_stream_to_av_pck(chip_id,
                               chan_id,
                               stream_type,
                               frame->buff,
                               av_packet);
    if( ret < 0 ) {
        isil_av_packet_release(av_packet);
        return NULL;
    }
    
    add_frame_ref(frame);
    fprintf(stderr,"frame ref [%d].\n",frame->ref);
    av_packet->priv = (void *)frame;

    return av_packet;
}


/*one day ,maybe modify this callback*/

static void put_date_to_consumer_cb(void * arg,struct ISIL_EV_TASK * ev_task)
{

#ifdef USEOLDHEADER
    old_drv_frame_to_packet((frame_t *)arg , ev_task);
#else
    frame_to_av_packet((frame_t *)arg , ev_task);
#endif    
    return ;
 
}


static void put_date_to_data_list(void * arg,struct ISIL_EV_TASK * ev_task)
{
    
    frame_t *frame;

    if(ev_task && arg) {
        

        if(ev_task->date_mgt) {

            frame = (frame_t *)arg;
            
            add_frame_ref(frame);

            ev_task->date_mgt->put_frame_tail(ev_task->date_mgt ,frame);

        }

        if(ev_task->task_mgt){
            if(ev_task->task_mgt->wakeup) {
                ev_task->task_mgt->wakeup(ev_task->task_mgt);
            }
        }
                
    }

    return ;
 
}


static void data_handle_ev_cb(int fd,short event_type,void *arg)
{

    int ret;

    frame_t *frame;
    FramePool *framepool_ptr;
    
  
    struct ISIL_EV_TASK * ev_task = (struct ISIL_EV_TASK *)arg;

    if(ev_task) {
        
        
        frame = get_frame_from_glb_pool();

        framepool_ptr = frame->root;
        assert(framepool_ptr);
        
        ret = r_read(fd,(char *)frame->buff,framepool_ptr->max_buff_size);
        
        if(ret < 0 ) {

            assert(0);

            frame->release(frame);
            if( errno == ENOMEM) {
                //TODO:
                
                return ;
            }

            fprintf(stderr,"Read date err .\n");
            ev_task->release(ev_task);
            return ;

        }
        else if( ret == 0 ) {
            fprintf(stderr,"buff too small .\n");
            
            if(ev_task->priv) {
                S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt = (S_ISIL_SINGLE_CHAN_MGT *)ev_task->priv;
                fprintf( stderr,"eCodecType[%d] , eStreamType[%d] , u32Ch[%d] , u32ChipID[%d] .\n",
                                 sin_chan_mgt->codec_channel_t.eCodecType,
                                 sin_chan_mgt->codec_channel_t.eStreamType,
                                 sin_chan_mgt->codec_channel_t.u32Ch,
                                 sin_chan_mgt->codec_channel_t.u32ChipID);
            }
            
            
            frame->release(frame);
            
            
            return ;
        }
        else{
            frame->len = ret;
        }

        
        
        /*put here*/
        if(ev_task->hook) {
            ev_task->hook(frame,ev_task);
        }
        else{
            assert(0);
        }

        
        frame->release(frame);

    }

    return ;
}


static struct ISIL_EV_TASK *create_enc_task_ev( int fd )
{
    struct ISIL_EV_ARG_T *ev_arg;
    struct ISIL_EV_TASK *ev_task;

    if( fd <= 0 ) {
        return NULL;
    }

    ev_task = ev_task_minit();

    if(!ev_task) {
        fprintf(stderr,"ev_task_minit err .\n");
        return NULL;
    }

    set_noblock(fd);

    SET_EV_TASK_FD(ev_task, fd);
    

#ifdef ENCUSEMULTITHREAD
    
    if(enc_task_thr_alloc_task(ev_task) < 0 ) {
        
        
        ev_task->release(ev_task);

        return NULL;

    }

    reg_ev_task_callback(ev_task ,NULL, put_date_to_data_list);

#else

    struct ISIL_TASK_MGT *task_mgt =get_glb_task_mgt(); 
    task_mgt->put_ev_task_to_act(task_mgt,ev_task);

    reg_ev_task_callback(ev_task ,NULL, put_date_to_consumer_cb);
    
#endif 

    ev_arg = get_ev_task_arg(ev_task);
    reg_ev_to_loop( ev_arg ,GET_EV_TASK_FD(ev_task), EVENT_READ| EVENT_PERSIST,NULL , data_handle_ev_cb ,ev_task);

   
    return ev_task;

}


static void ev_task_priv_release_cb(struct ISIL_EV_TASK * ev_task)
{
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    DEBUG_FUNCTION();
    if(ev_task) {
        DEBUG_FUNCTION();
        if(ev_task->priv) {
            DEBUG_FUNCTION();
            sin_chan_mgt = (S_ISIL_SINGLE_CHAN_MGT *)ev_task->priv;
            sin_chan_mgt->is_enable = CHAN_INVALID_MODE;
            sin_chan_mgt->priv = NULL;
            if(sin_chan_mgt->release) {
                DEBUG_FUNCTION();
                sin_chan_mgt->release(sin_chan_mgt);
            }
            
        }
    }
    DEBUG_FUNCTION();
}


int reg_date_rec_handle(int chipid,
                        int chn ,
                        enum ECHANFUNCPRO type ,
                        int enable)
{

   
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    struct ISIL_EV_TASK * ev_task;
    

    if( chipid >= isil_get_chips_count()) {
        fprintf(stderr,"chipid[%d] > total chips [%d] .\n",
                chipid ,isil_get_chips_count());
        return -1;
    }

    
    sin_chan_mgt = isil_open_enc_chn(chipid,chn,type,enable);
    if( !sin_chan_mgt ) {
        fprintf(stderr,"isil_open_enc_chn err .\n");
        return -1;
    }


    if( !sin_chan_mgt->priv ) {
        
        ev_task = create_enc_task_ev(GET_SINGLE_CHAN_FD(sin_chan_mgt));
        if( !ev_task ) {
            fprintf(stderr,"create_enc_task_ev err .\n");
            return -1;
        }

        sin_chan_mgt->priv = (void *)ev_task;
        ev_task->priv = (void *)sin_chan_mgt;
        //reg_ev_task_priv_release(ev_task ,ev_task_priv_release_cb );
        
    }

    return 0;
    
}


static void debug_mem(int len ,char *buff)
{
	int i;

	printf(" \n");
	for( i = 0 ; i < len ; i++ ){

		printf(" %x " ,buff[i]);

	}

	printf(" \n");
}


static int parse_alarm_data(char *buff, struct ISIL_EV_TASK * ev_task)
{

    unsigned int msg_type ,i;
    int offset = 0;
    isil_common_stream_header_t* commheader;
    isil_transparent_msg_header_t* msgheader;
    struct ISIL_NOTIFY_LST *notify_lst;

    ISIL_ALARM_PACKET *alarm_pck = NULL;


    notify_lst = &ev_task->notify_list;

    if( !buff || !ev_task ) {
        return -1;
    }

    //debug_mem(64,buff);

    commheader = (isil_common_stream_header_t*)buff;
    offset += sizeof(isil_common_stream_header_t);

    if(commheader->stream_len & ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG){
        
        msgheader = (isil_transparent_msg_header_t*)(buff + offset);
        offset += sizeof(isil_transparent_msg_header_t);

        if( !msgheader->isil_transparent_msg_number ) {
            DEBUG_FUNCTION();
            return -1;
        }

        for(i = 0 ; i < msgheader->isil_transparent_msg_number ; i++) {
            
            msg_type = *((unsigned int*)( buff + offset) );
            switch( msg_type ){
                case ISIL_VIDEO_LOST_DETECTION_MSG:{

                    isil_video_lost_detection_msg_t *v_lost_detect_ptr = (isil_video_lost_detection_msg_t *)(buff + offset);
                    if(v_lost_detect_ptr->msg_len != sizeof(isil_video_lost_detection_msg_t)) {
                        fprintf( stderr ,"v_lost_detect_ptr len err .\n");
                        return -1;
                    }
                    
                    
                    /*TODO:*/

                    alarm_pck = isil_alarm_packet_minit();
                    if( !alarm_pck ) {
                        fprintf( stderr,"isil_alarm_packet_minit err .\n");
                        return -1;
                    }

                    alarm_pck->alarm_type_e = VIDEO_LOST_DETECTION_E;
                    alarm_pck->alarm_type_u.v_lost_detect_inf.chan_id = v_lost_detect_ptr->chan_id;
                    alarm_pck->alarm_type_u.v_lost_detect_inf.chip_id = get_alarm_chip_id_by_fd(ev_task->fd);
                    alarm_pck->alarm_type_u.v_lost_detect_inf.video_contect_valid = v_lost_detect_ptr->video_contect_valid;
                    alarm_pck->alarm_type_u.v_lost_detect_inf.video_lost_valid = v_lost_detect_ptr->video_lost_valid;
                    
                    notify_lst->notify(notify_lst ,(void *)alarm_pck);

                    isil_alarm_packet_release(alarm_pck);
                    
                    offset += v_lost_detect_ptr->msg_len;
                }
                    break;
                case ISIL_VIDEO_NIGHT_DETECTION_MSG:{

                    /*TODO:*/
                    isil_video_night_detection_msg_t *v_night_detect_ptr = (isil_video_night_detection_msg_t *)(buff + offset);
                    if(v_night_detect_ptr->msg_len != sizeof(isil_video_night_detection_msg_t)) {
                        fprintf( stderr ,"v_night_detect_ptr len err .\n");
                        return -1;
                    }

                                        
                    
                    /*TODO:*/

                    alarm_pck = isil_alarm_packet_minit();
                    if( !alarm_pck ) {
                        fprintf( stderr,"isil_alarm_packet_minit err .\n");
                        return -1;
                    }

                    alarm_pck->alarm_type_e = VIDEO_NIGHT_DETECTION_E;
                    alarm_pck->alarm_type_u.v_night_detect_inf.chan_id = v_night_detect_ptr->chan_id;
                    alarm_pck->alarm_type_u.v_night_detect_inf.chip_id = get_alarm_chip_id_by_fd(ev_task->fd);
                    alarm_pck->alarm_type_u.v_night_detect_inf.video_night_valid_from_day_to_night = v_night_detect_ptr->video_night_valid_from_day_to_night;
                    alarm_pck->alarm_type_u.v_night_detect_inf.video_night_valid_from_night_to_day = v_night_detect_ptr->video_night_valid_from_night_to_day;
                    
                    notify_lst->notify(notify_lst ,(void *)alarm_pck);

                    isil_alarm_packet_release(alarm_pck);
                    
                    offset += v_night_detect_ptr->msg_len;

                }
                    break;
                case ISIL_VIDEO_BLIND_DETECTION_MSG:{

        
                    /*TODO:*/
                    isil_video_blind_detection_msg_t *v_blind_detect_ptr = (isil_video_blind_detection_msg_t *)(buff + offset);
                    if(v_blind_detect_ptr->msg_len != sizeof(isil_video_blind_detection_msg_t)) {
                        fprintf( stderr ,"v_blind_detect_ptr len err .\n");
                        return -1;
                    }

                                        
                    
                    /*TODO:*/

                    alarm_pck = isil_alarm_packet_minit();
                    if( !alarm_pck ) {
                        fprintf( stderr,"isil_alarm_packet_minit err .\n");
                        return -1;
                    }

                    alarm_pck->alarm_type_e = VIDEO_BLIND_DETECTION_E;
                    alarm_pck->alarm_type_u.v_blind_detect_inf.chan_id = v_blind_detect_ptr->chan_id;
                    alarm_pck->alarm_type_u.v_blind_detect_inf.chip_id = get_alarm_chip_id_by_fd(ev_task->fd);
                    alarm_pck->alarm_type_u.v_blind_detect_inf.video_blind_add_valid = v_blind_detect_ptr->video_blind_add_valid;
                    alarm_pck->alarm_type_u.v_blind_detect_inf.video_blind_remove_valid = v_blind_detect_ptr->video_blind_remove_valid;
                    
                    notify_lst->notify(notify_lst ,(void *)alarm_pck);

                    isil_alarm_packet_release(alarm_pck);
                    
                    offset += v_blind_detect_ptr->msg_len;
                }
                    break;
                case ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG:{

                    /*TODO:*/

                    isil_video_standard_change_detection_msg_t *v_standard_detect_ptr = (isil_video_standard_change_detection_msg_t *)(buff + offset);
                    if(v_standard_detect_ptr->msg_len != sizeof(isil_video_standard_change_detection_msg_t)) {
                        fprintf( stderr ,"v_standard_detect_ptr len err .\n");
                        return -1;
                    }

                                        
                    
                    /*TODO:*/

                    alarm_pck = isil_alarm_packet_minit();
                    if( !alarm_pck ) {
                        fprintf( stderr,"isil_alarm_packet_minit err .\n");
                        return -1;
                    }

                    alarm_pck->alarm_type_e = VIDEO_STANDARD_MODIFY_DETECTION_E;
                    alarm_pck->alarm_type_u.v_standard_detect_inf.chan_id = v_standard_detect_ptr->chan_id;
                     alarm_pck->alarm_type_u.v_standard_detect_inf.chip_id = get_alarm_chip_id_by_fd(ev_task->fd);
                    alarm_pck->alarm_type_u.v_standard_detect_inf.latest_video_standard = v_standard_detect_ptr->latest_video_standard;
                    alarm_pck->alarm_type_u.v_standard_detect_inf.curr_video_standard = v_standard_detect_ptr->curr_video_standard;
                    
                    notify_lst->notify(notify_lst ,(void *)alarm_pck);

                    isil_alarm_packet_release(alarm_pck);
                    
                    offset += v_standard_detect_ptr->msg_len;
                }
                    break;

            default:
                DEBUG_FUNCTION();
                return -1;
                
            }   

        }
    }
    else {
        DEBUG_FUNCTION();
        return -1;
    }


    return 0;

}

static void put_alarm_to_consumer_cb(void * arg,struct ISIL_EV_TASK * ev_task)
{
    int ret;

    ret = parse_alarm_data((char *)arg ,ev_task );
    if( ret < 0 ) {

        fprintf( stderr ,"parse_alarm_data  err .\n");
        ev_task->release(ev_task);
    }

}




static void alarm_handle_ev_cb(int fd,short event_type,void *arg)
{

    int ret;
    
    char buff[ALARM_BUFF_SIZE];

      
    struct ISIL_EV_TASK * ev_task = (struct ISIL_EV_TASK *)arg;

    if(ev_task) {
        
        ret = r_read(fd,(char *)buff,ALARM_BUFF_SIZE);
        
        if(ret < 0 ) {

            
            if( errno == ENOMEM) {
                //TODO:
                
                return ;
            }
            fprintf(stderr,"Read alarm data err .\n");
            ev_task->release(ev_task);
            return ;

        }
        else if( ret == 0 ) {

            fprintf(stderr,"Get alarm date err .\n");
            ev_task->release(ev_task);
            DEBUG_FUNCTION();
            return ;
        }
        else{
            /*put here*/
            if(ev_task->hook) {
                ev_task->hook( (void *)buff,ev_task);
            }
        }


    }

    return ;
}


int create_alarm_ev(unsigned int chipid )
{
    int fd;
    struct ISIL_EV_TASK * ev_task;
    struct ISIL_EV_ARG_T *ev_arg;
    IO_NODE_MGT *io_mgt = NULL;

    io_mgt = get_io_node_mgt_by_type_id(chipid , NODE_TYPE_VI);
    if( !io_mgt ) {
        fprintf( stderr,"get_io_node_mgt_by_type_id err .\n");
        return -1;
    }

#if 0
    if(io_mgt->is_enable != 1 ) {
        fprintf( stderr,"io_mgt  is_enable err .\n");
        return -1;
    }
#endif

    if(io_mgt->priv) {
    	DEBUG_FUNCTION();
        return 0;
    }

    fd = io_mgt->fd;

    if( fd <= 0 ) {
    	DEBUG_FUNCTION();
        return -1;
    }

    DEBUG_FUNCTION();
    ev_task = ev_task_minit();

    if(!ev_task) {
        fprintf(stderr,"ev_task_minit err .\n");
        return -1;
    }

    DEBUG_FUNCTION();

    io_mgt->priv = (void *)ev_task;

    set_noblock(fd);

    SET_EV_TASK_FD(ev_task, fd);

    fprintf(stderr,"Alarm fd[%d].\n",fd);
    chip_fd_map[chipid] = fd;
    
    struct ISIL_TASK_MGT *task_mgt =get_glb_task_mgt(); 

    task_mgt->put_ev_task_to_act(task_mgt,ev_task);

    reg_ev_task_callback(ev_task ,NULL, put_alarm_to_consumer_cb);


    ev_arg = get_ev_task_arg(ev_task);
    if( !ev_arg ) {
        return -1;
    }

    reg_ev_to_loop( ev_arg ,GET_EV_TASK_FD(ev_task), EVENT_READ| EVENT_PERSIST,NULL , alarm_handle_ev_cb ,ev_task);
    DEBUG_FUNCTION();
    return 0;
    
}


















