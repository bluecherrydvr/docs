

#include "isil_drv_to_sdk_data.h"


int parse_drv_stream_to_av_pck(unsigned int chip_id,
                               unsigned int chan_id,
                               STREAM_TYPE_E stream_type,
                               unsigned char * buff ,
                               ISIL_AV_PACKET *av_packet)
{

    int  offset = 0;
    unsigned int i;
    
    unsigned int msg_type;
    unsigned char *buffOffset;

    isil_common_stream_header_t* commheader;
    isil_transparent_msg_header_t* msgheader;
    isil_audio_param_msg_t* aud_param;
    

    if( !buff || !av_packet) {
        return -1;
    }

    av_packet->chip_id = chip_id;
    av_packet->chip_id = chan_id;

    buffOffset = buff;
    commheader = (isil_common_stream_header_t*)buff;
    
    offset += sizeof(isil_common_stream_header_t);
    if(commheader->stream_len & ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG){
        
        msgheader = (isil_transparent_msg_header_t*)(buffOffset+offset);

        offset += sizeof(isil_transparent_msg_header_t);


        for( i = 0 ; i < msgheader->isil_transparent_msg_number; i++) {

            msg_type = *((unsigned int*)(buffOffset+offset));

            switch( msg_type ){
            
                case ISIL_H264_ENCODE_FRAME_TYPE:{
                    
                    isil_h264_encode_msg_frame_type_t* h264_frame_type = (isil_h264_encode_msg_frame_type_t*)(buffOffset+offset);

                    av_packet->frm_type = ISIL_H264_DATA;
                    av_packet->data_type_u.h264_inf.image_size.x_width = h264_frame_type->i_mb_x;
                    av_packet->data_type_u.h264_inf.image_size.y_height = h264_frame_type->i_mb_y;
                    av_packet->data_type_u.h264_inf.stream_type = stream_type;
                    
                    if(h264_frame_type->frame_type == ISIL_SPS_FRAME)

                         av_packet->data_type_u.h264_inf.nalu_type = H264_NALU_SPS;

                    else if( h264_frame_type->frame_type == ISIL_PPS_FRAME )
                        av_packet->data_type_u.h264_inf.nalu_type = H264_NALU_PPS;
                    else if(h264_frame_type->frame_type == ISIL_IDR_FRAME)
                        av_packet->data_type_u.h264_inf.nalu_type = H264_NALU_IDRSLICE; 
                    else
                        av_packet->data_type_u.h264_inf.nalu_type = H264_NALU_PSLICE; 

                    offset += h264_frame_type->msg_len;
                    
                }
                break;

                case ISIL_AUDIO_ENCODE_PARAM_MSG:{
                    
                     aud_param = (isil_audio_param_msg_t*)(buffOffset+offset);
                     av_packet->frm_type = ISIL_AUDIO_DATA;
                     av_packet->data_type_u.audio_inf.bit_rate  = aud_param->audio_bit_wide;
                     av_packet->data_type_u.audio_inf.sample  = aud_param->audio_sample_rate;

                     if(aud_param->audio_type == ISIL_AUDIO_PCM){
						
                        av_packet->data_type_u.audio_inf.audio_type_e = ISIL_AUDIO_PCM_E;
                        
					}else{

						av_packet->data_type_u.audio_inf.audio_type_e = ISIL_AUDIO_ADPCM_E;

					}

                    offset += aud_param->msg_len;
                }
                break;

                case ISIL_MJPEG_ENCODE_PARAM_MSG:{

                    isil_jpeg_param_msg_t* mjpg_param = (isil_jpeg_param_msg_t*)(buffOffset+offset);
                    av_packet->frm_type = ISIL_MJPEG_DATA;
                    
                    offset += mjpg_param->msg_len;
                }
                break;

                case ISIL_MJPEG_ENCODE_FRAME_MSG:
				case ISIL_AUDIO_ENCODE_FRAME_MSG:
				case ISIL_H264_ENCODE_FRAME_MSG:{

                    isil_frame_msg_t* frame_msg = (isil_frame_msg_t *)(buffOffset+offset);
                    int data_len = (frame_msg->msg_len - sizeof(isil_frame_msg_t)); 
                    av_packet->pts = frame_msg->frame_timestamp;
                    av_packet->frm_seq = frame_msg->frame_serial;
                    av_packet->buff = frame_msg->frame_payload;
                    av_packet->date_len = data_len;
                }
                break;
                
            default:
                break;
            }
        }
    }
    else{
        return -1;
    }
    
    return 0;
}
