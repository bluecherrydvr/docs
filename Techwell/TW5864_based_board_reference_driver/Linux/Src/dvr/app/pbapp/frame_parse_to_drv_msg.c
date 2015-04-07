/*
 * frame_parse_to_drv_msg.c
 *
 *  Created on: 2011-6-30
 *      Author: zoucx
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include "frame_parse_to_drv_msg.h"
#include "isil_fm_h264.h"
#include "isil_codec_common.h"




int decode_fill_audio_buff_header(unsigned char *buff,
                                         int frameNum,
                                         PB_FRAME_INFO  *pb_frame_inf_t)
{
    int len;
	struct isil_common_stream_header *com_header;
	isil_transparent_msg_header_t *trsparent_header;
	isil_audio_param_msg_t *audio_param_ptr;
	struct isil_frame_msg *frame_msg_ptr;

	if( !buff || !pb_frame_inf_t){
		return -1;
	}

	if((pb_frame_inf_t->nal[0].naltype < E_FRAME_TYPE_AUDIO)
		|| (pb_frame_inf_t->nal[0].naltype > E_FRAME_TYPE_AUDIO_END)){
		return -1;
	}

   
    len = sizeof( struct isil_common_stream_header) ;
    com_header = (struct isil_common_stream_header *)buff;
	com_header->stream_type = ISIL_AUDIO_STREAM;
	com_header->stream_timestamp = pb_frame_inf_t->ts;

	
	trsparent_header = (isil_transparent_msg_header_t *)(buff + len);
	trsparent_header->isil_transparent_msg_number = 2;
	trsparent_header->isil_transparent_msg_total_len = sizeof(isil_transparent_msg_header_t);
	len += trsparent_header->isil_transparent_msg_total_len;
	audio_param_ptr = (isil_audio_param_msg_t *)(buff + len);
	audio_param_ptr->audio_bit_wide = ISIL_AUDIO_16BIT;
	audio_param_ptr->audio_sample_rate = ISIL_AUDIO_8000;
	audio_param_ptr->audio_type = ISIL_AUDIO_PCM;
	audio_param_ptr->msg_type = ISIL_AUDIO_DECODE_PARAM_MSG;
	audio_param_ptr->msg_len = sizeof(isil_audio_param_msg_t);
	len += audio_param_ptr->msg_len;
	frame_msg_ptr =(struct isil_frame_msg *) (buff +len);
	frame_msg_ptr->msg_type = ISIL_AUDIO_DECODE_FRAME_MSG;
	frame_msg_ptr->frame_serial = frameNum;
	frame_msg_ptr->frame_timestamp = com_header->stream_timestamp;
	frame_msg_ptr->msg_len = sizeof(struct isil_frame_msg);
	frame_msg_ptr->msg_len += pb_frame_inf_t->nal[0].nallen;

	len += frame_msg_ptr->msg_len;
	com_header->stream_len = len;
	com_header->stream_len |= ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG;
	trsparent_header->isil_transparent_msg_total_len = (len - sizeof(isil_common_stream_header_t));
    
    return len;

}




int decode_fill_h264_buff_header(unsigned char *buff,
                                        int frameNum,
                                        PB_FRAME_INFO  *pb_frame_inf_t,
                                        isil_h264_decode_bit_info_msg_t *bit_inf)
{

	int len ,i,offset = 0;
	struct isil_common_stream_header *com_header;
	isil_transparent_msg_header_t *trsparent_header;
	
	struct isil_frame_msg *frame_msg_ptr;
	isil_h264_decode_bit_info_msg_t *dec_bit_inf;
	isil_h264_encode_msg_frame_type_t *frame_type_ptr;

	if( !buff || !pb_frame_inf_t || !bit_inf){
		return -1;
	}

	

	if(pb_frame_inf_t->nalcnt == 1){
		if((pb_frame_inf_t->nal[0].naltype == E_H264_FRAME_TYPE_MAIN_SPS)
		   || (pb_frame_inf_t->nal[0].naltype == E_H264_FRAME_TYPE_MAIN_PPS)
		   || (pb_frame_inf_t->nal[0].naltype == E_H264_FRAME_TYPE_MAIN_IDR)
		   || (pb_frame_inf_t->nal[0].naltype == E_H264_FRAME_TYPE_SUB_IDR)
		   || (pb_frame_inf_t->nal[0].naltype == E_H264_FRAME_TYPE_SUB_SPS)
		   || (pb_frame_inf_t->nal[0].naltype == E_H264_FRAME_TYPE_SUB_PPS)){
			fprintf(stderr,"Get P or I nal type err.\n");
			return -1;

		}
	}
	else if(pb_frame_inf_t->nalcnt == 3){

		if( (pb_frame_inf_t->nal[0].naltype != E_H264_FRAME_TYPE_MAIN_SPS) &&
			(pb_frame_inf_t->nal[0].naltype != E_H264_FRAME_TYPE_SUB_SPS)){
			fprintf(stderr,"Get sps nal type err.\n");
			return -1;
		}

		if( (pb_frame_inf_t->nal[1].naltype != E_H264_FRAME_TYPE_MAIN_PPS) &&
			(pb_frame_inf_t->nal[1].naltype != E_H264_FRAME_TYPE_SUB_PPS)){
			fprintf(stderr,"Get sps nal type err.\n");
			return -1;
		}

		if( (pb_frame_inf_t->nal[2].naltype != E_H264_FRAME_TYPE_MAIN_IDR) &&
			(pb_frame_inf_t->nal[2].naltype != E_H264_FRAME_TYPE_SUB_IDR)){
			fprintf(stderr,"Get sps nal type err.\n");
			return -1;
		}
		
	}
	else{
		return -1;
	}


	
	com_header = (struct isil_common_stream_header *)buff;
	com_header->stream_type = ISIL_H264_STERAM;
	com_header->stream_timestamp = pb_frame_inf_t->ts;
	len = sizeof( struct isil_common_stream_header) ;

	trsparent_header = (isil_transparent_msg_header_t *)(buff + len);
	
	trsparent_header->isil_transparent_msg_total_len = sizeof(isil_transparent_msg_header_t);
	len += trsparent_header->isil_transparent_msg_total_len;

	dec_bit_inf = (isil_h264_decode_bit_info_msg_t *)(buff +len);
	//(*dec_bit_inf) = (*bit_inf);
	memcpy(dec_bit_inf, bit_inf, sizeof(isil_h264_decode_bit_info_msg_t));
	dec_bit_inf->msg_type = ISIL_H264_DECODE_BIT_INFO_MSG;
	dec_bit_inf->msg_len = sizeof(isil_h264_decode_bit_info_msg_t);
	
	len += sizeof(isil_h264_decode_bit_info_msg_t);

	offset += pb_frame_inf_t->offset;

	for( i = 0 ; i < pb_frame_inf_t->nalcnt ; i++){

		frame_type_ptr = (isil_h264_encode_msg_frame_type_t *)(buff+len);
		frame_type_ptr->msg_type = ISIL_H264_ENCODE_FRAME_TYPE;
		frame_type_ptr->i_mb_x = 0;
		frame_type_ptr->i_mb_y = 0;
		frame_type_ptr->msg_len = sizeof(isil_h264_encode_msg_frame_type_t);

		switch(pb_frame_inf_t->nal[i].naltype ) {
		case E_H264_FRAME_TYPE_MAIN_I:
		case E_H264_FRAME_TYPE_SUB_I:
			frame_type_ptr->frame_type = ISIL_I_FRAME;
			break;
		case E_H264_FRAME_TYPE_MAIN_P:
		case E_H264_FRAME_TYPE_SUB_P:
			frame_type_ptr->frame_type = ISIL_P_FRAME;
			break;
			
		case E_H264_FRAME_TYPE_MAIN_SPS:
		case E_H264_FRAME_TYPE_SUB_SPS:
			frame_type_ptr->frame_type = ISIL_SPS_FRAME;
			break;
		case E_H264_FRAME_TYPE_MAIN_PPS:
		case E_H264_FRAME_TYPE_SUB_PPS:
			frame_type_ptr->frame_type = ISIL_PPS_FRAME;
			break;
		case E_H264_FRAME_TYPE_MAIN_IDR:
		case E_H264_FRAME_TYPE_SUB_IDR:
			frame_type_ptr->frame_type = ISIL_IDR_FRAME;
			break;

		default:
			return -1;

		}

		len += frame_type_ptr->msg_len;
		
	
		frame_msg_ptr = (struct isil_frame_msg *)(buff+len);

		frame_msg_ptr->frame_serial = frameNum;
		frame_msg_ptr->frame_timestamp = com_header->stream_timestamp;
		frame_msg_ptr->msg_len = sizeof(struct isil_frame_msg);
		frame_msg_ptr->msg_type = ISIL_H264_DECODE_FRAME_MSG;
		
		len += frame_msg_ptr->msg_len;

		if(ISIL_FM_PLAY_GetFileData( pb_frame_inf_t->fd ,
                                     offset ,
									 pb_frame_inf_t->nal[i].nallen,
                                     (void *)(buff +len) )< 0){
            fprintf(stderr,"ISIL_FM_PLAY_GetFileData err.\n");
            
            return -1;
		}

		offset += pb_frame_inf_t->nal[i].nallen;
		frame_msg_ptr->msg_len += pb_frame_inf_t->nal[i].nallen;
		len += pb_frame_inf_t->nal[i].nallen;
	}
	
	com_header->stream_len = len;
	com_header->stream_len |= ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG;

    trsparent_header->isil_transparent_msg_number = (pb_frame_inf_t->nalcnt*2) + 1;
    trsparent_header->isil_transparent_msg_total_len = (len - sizeof(isil_common_stream_header_t));
		
	return len;
}


int decode_only_fill_h264_bitinfo(unsigned char *buff, unsigned int bit_info)
{
	int offset = 0;
	isil_h264_decode_bit_info_msg_t *dec_bit_inf;

	if( !buff){
		return -1;
	}

	offset = sizeof( struct isil_common_stream_header) + sizeof(isil_transparent_msg_header_t);
	dec_bit_inf = (isil_h264_decode_bit_info_msg_t *)(buff + offset);
	dec_bit_inf->bit_info = bit_info;
	return 0;
}












