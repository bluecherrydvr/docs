#include	<isil5864/isil_common.h>


/****************************************************************************************/
/****************************************************************************************/
static int  h264_gen_sps_rbsp(void *p_data, h264_sps_t *sps)
{
    bs_t    bs, *s;

    s = &bs;
    bs_init(s, p_data, MAX_SPS_RBSP_LEN);
    bs_write(s, 8, sps->i_profile_idc);
    bs_write(s, 8, ((sps->b_constraint_set0<<7)|(sps->b_constraint_set1<<6)|(sps->b_constraint_set2<<5)));
    bs_write(s, 8, sps->i_level_idc );
    bs_write_ue(s, sps->i_id);
    bs_write_ue(s, sps->i_log2_max_frame_num - 4);
    bs_write_ue(s, sps->i_poc_type);
    if(sps->i_poc_type == 0)
        bs_write_ue(s, sps->i_log2_max_poc_lsb - 4);
    else if (sps->i_poc_type == 1) {
        int i;
        bs_write(s, 1, sps->b_delta_pic_order_always_zero);
        bs_write_se(s, sps->i_offset_for_non_ref_pic);
        bs_write_se(s, sps->i_offset_for_top_to_bottom_field);
        bs_write_ue(s, sps->i_num_ref_frames_in_poc_cycle);
        for(i = 0; i < sps->i_num_ref_frames_in_poc_cycle; i++)
            bs_write_se(s, sps->i_offset_for_ref_frame[i]);
    }
    bs_write_ue(s, sps->i_num_ref_frames);
    bs_write(s, 1, sps->b_gaps_in_frame_num_value_allowed);
    bs_write_ue(s, sps->i_mb_width - 1);
    bs_write_ue(s, sps->i_mb_height - 1);
    bs_write(s, 1, sps->b_frame_mbs_only);
    if( !sps->b_frame_mbs_only)
        bs_write(s, 1, sps->b_mb_adaptive_frame_field);
    bs_write(s, 1, sps->b_direct8x8_inference);
    bs_write(s, 1, 0);
    bs_write(s, 1, 0);
    bs_rbsp_trailing(s);
    return bs_len(s);
}

int  gen_h264_sps(isil_h264_encode_control_t *h264_encode_control, isil_video_frame_tcb_t *frame)
{
    h264_nal_t              *nal;
    h264_nal_bitstream_t    *h264_nal_sps_head;
    H264_NAL_INFO           *nal_info;
#ifdef  ADD_DVM_NAL_HEAD
    DVM_NAL_HEAD            *nal_sps_head;
    int len;
#endif
    isil_video_packet_tcb_queue_t	*video_packet_queue;
    isil_video_packet_tcb_t	*packet;

    nal = &frame->nal;
    video_packet_queue = &frame->video_packet_queue;
    packet = video_packet_queue->curr_producer;
    nal->sps_payload = &packet->data[frame->frame_len];
    memset(nal->sps_payload, 0, SLICE_HEAD_BUF_SIZE);
    h264_nal_sps_head = (h264_nal_bitstream_t*)nal->sps_payload;
#ifdef  ADD_DVM_NAL_HEAD
    nal_sps_head = &h264_nal_sps_head->head;
#endif
    nal_info = &h264_nal_sps_head->nal;

#ifdef  ADD_DVM_NAL_HEAD
    nal_sps_head->e_nal_type = DVM_NAL_SPS;
#endif
#if NAL_FILL_ZERO_NUM
    nal_info->zero1 = 0;
    nal_info->zero2 = 0;
#endif
    nal_info->zero3 = 0;
    nal_info->zero4 = 0;
    nal_info->zero5 = 0;
    nal_info->constant_value_1 = 1;
    nal_info->nal_ref_idc = nal->i_ref_idc = NAL_PRIORITY_HIGHEST;
    nal_info->nal_unit_type = nal->i_type = NAL_SPS;
    nal_info->forbidden_zero_bit = 0;

    nal->sps_paysize = sizeof(h264_nal_bitstream_t);
    nal->sps_paysize += h264_gen_sps_rbsp(h264_nal_sps_head->rbsp, h264_encode_control->sps);
    frame->frame_len += nal->sps_paysize;

#ifdef  ADD_DVM_NAL_HEAD
    len = nal->sps_paysize - sizeof(DVM_NAL_HEAD);
    nal_sps_head->i_nal_size_0_7 = (len & 0xff);
    nal_sps_head->i_nal_size_8_15 = ((len>>8) & 0xff);
    nal_sps_head->i_nal_size_16_23 = ((len>>16) & 0xff);
#endif
    return nal->sps_paysize;
}

static int  h264_gen_pps_rbsp(void *p_data, h264_pps_t *pps)
{
    bs_t    bs, *s;

    s = &bs;
    bs_init(s, p_data, MAX_PPS_RBSP_LEN);
    bs_write_ue(s, pps->i_id);
    bs_write_ue(s, pps->i_sps_id);
    bs_write(s, 1, pps->b_cabac);
    bs_write(s, 1, pps->b_pic_order);
    bs_write_ue(s, pps->i_num_slice_groups - 1);
    bs_write_ue(s, pps->i_num_ref_idx_l0_active - 1);
    bs_write_ue(s, pps->i_num_ref_idx_l1_active - 1);
    bs_write(s, 1, pps->b_weighted_pred);
    bs_write(s, 2, pps->b_weighted_bipred);
    bs_write_se(s, pps->i_pic_init_qp - 26);
    bs_write_se(s, pps->i_pic_init_qs - 26);
    bs_write_se(s, pps->i_chroma_qp_index_offset);
    bs_write(s, 1, pps->b_deblocking_filter_control);
    bs_write(s, 1, pps->b_constrained_intra_pred);
    bs_write(s, 1, pps->b_redundant_pic_cnt);
    bs_rbsp_trailing(s);
    return bs_len(s);
}

int gen_h264_pps(isil_h264_encode_control_t *h264_encode_control, isil_video_frame_tcb_t *frame)
{
    h264_nal_t              *nal;
    h264_nal_bitstream_t    *h264_nal_pps_head;
    H264_NAL_INFO           *nal_info;
#ifdef  ADD_DVM_NAL_HEAD
    DVM_NAL_HEAD            *nal_pps_head;
    int     len;
#endif
    isil_video_packet_tcb_queue_t	*video_packet_queue;
    isil_video_packet_tcb_t	*packet;

    nal = &frame->nal;
    video_packet_queue = &frame->video_packet_queue;
    packet = video_packet_queue->curr_producer;
    nal->pps_payload = &packet->data[frame->frame_len]; 
    memset(nal->pps_payload, 0, 64);
    h264_nal_pps_head = (h264_nal_bitstream_t*)nal->pps_payload;
#ifdef  ADD_DVM_NAL_HEAD
    nal_pps_head = &h264_nal_pps_head->head;
#endif
    nal_info = &h264_nal_pps_head->nal;

#ifdef  ADD_DVM_NAL_HEAD
    nal_pps_head->e_nal_type = DVM_NAL_PPS;
#endif
#if NAL_FILL_ZERO_NUM
    nal_info->zero1 = 0;
    nal_info->zero2 = 0;
#endif
    nal_info->zero3 = 0;
    nal_info->zero4 = 0;
    nal_info->zero5 = 0;
    nal_info->constant_value_1 = 1;
    nal_info->nal_ref_idc = nal->i_ref_idc = NAL_PRIORITY_HIGHEST;
    nal_info->nal_unit_type = nal->i_type = NAL_PPS;
    nal_info->forbidden_zero_bit = 0;

    nal->pps_paysize = sizeof(h264_nal_bitstream_t);
    nal->pps_paysize += h264_gen_pps_rbsp(h264_nal_pps_head->rbsp, h264_encode_control->pps);
    frame->frame_len += nal->pps_paysize;

#ifdef  ADD_DVM_NAL_HEAD
    len = nal->pps_paysize - sizeof(DVM_NAL_HEAD);
    nal_pps_head->i_nal_size_0_7 = (len & 0xff);
    nal_pps_head->i_nal_size_8_15 = ((len>>8) & 0xff);
    nal_pps_head->i_nal_size_16_23 = ((len>>16) & 0xff);
#endif
    return nal->pps_paysize;
}

static int  h264_gen_slice_head(void *p_data, h264_slice_header_t *slice_head, h264_nal_t *nal)
{
    bs_t    bs, *s;

    s = &bs;
    bs_init(s, p_data, MAX_SLICE_HEAD_LEN);
    bs_write_ue(s, slice_head->i_first_mb);
    bs_write_ue(s, slice_head->i_type);
    bs_write_ue(s, slice_head->i_pps_id);
    bs_write(s, slice_head->sps->i_log2_max_frame_num, slice_head->i_frame_num);
    if(nal->i_type == NAL_SLICE_IDR)
        bs_write_ue(s, slice_head->i_idr_pic_id);
    if(slice_head->sps->i_poc_type == 0) {
        slice_head->i_poc_lsb = (slice_head->i_frame_num<<1);
        bs_write(s, slice_head->sps->i_log2_max_poc_lsb, slice_head->i_poc_lsb);
    }
    if(slice_head->i_type == SLICE_TYPE_P || slice_head->i_type == SLICE_TYPE_SP || slice_head->i_type == SLICE_TYPE_B ) {
        bs_write1(s, slice_head->b_num_ref_idx_override);
        if(slice_head->b_num_ref_idx_override==1) {
            if (slice_head->i_type == SLICE_TYPE_P)
                bs_write_ue(s, slice_head->i_frame_num-1);
        }
    }

    /* ref pic list reordering */
    if (slice_head->i_type != SLICE_TYPE_I && slice_head->i_type != SLICE_TYPE_SI) {
        slice_head->b_ref_pic_list_reordering_l0 = 0;
        bs_write1(s, slice_head->b_ref_pic_list_reordering_l0);
    }
    if (nal->i_ref_idc != NAL_PRIORITY_DISPOSABLE) {
        if (nal->i_type == NAL_SLICE_IDR) {
            bs_write1(s, 0);  /* no output of prior pics flag */
            bs_write1(s, 0);  /* long term reference flag */
        } else
            bs_write1(s, 0);  /* adaptive_ref_pic_marking_mode_flag */
    }
    bs_write_se(s, slice_head->i_qp_delta );

    if (s->i_left != 8){
        nal->i_temp_bitalign_content = ((s->p[0])<<s->i_left);
        nal->i_temp_bitalign_number = 8-s->i_left;
    } else {
        nal->i_temp_bitalign_content = 0;
        nal->i_temp_bitalign_number = 0;
    }

    return bs_len(s);
}

int gen_h264_slicehead(isil_h264_encode_control_t *h264_encode_control, isil_video_frame_tcb_t *frame, int frame_type)
{
    h264_nal_t              *nal;
    h264_nal_bitstream_t    *h264_nal_slice_head;
    H264_NAL_INFO           *nal_info;
#ifdef  ADD_DVM_NAL_HEAD
    DVM_NAL_HEAD            *nal_slice_head;
    int     len;
#endif
    isil_video_packet_tcb_queue_t	*video_packet_queue;
    isil_video_packet_tcb_t	*packet;
    u8  *ch;
    u32 temp_value;

    nal = &frame->nal;
    video_packet_queue = &frame->video_packet_queue;
    packet = video_packet_queue->curr_producer;
    nal->slice_payload = &packet->data[frame->frame_len]; 
    memset(nal->slice_payload, 0, 64);
    h264_nal_slice_head = (h264_nal_bitstream_t*)nal->slice_payload;
#ifdef  ADD_DVM_NAL_HEAD
    nal_slice_head = &h264_nal_slice_head->head;
#endif
    nal_info = &h264_nal_slice_head->nal;

    switch(frame_type){
        case H264_FRAME_TYPE_P:
#ifdef  ADD_DVM_NAL_HEAD
            nal_slice_head->e_nal_type = DVM_NAL_SLICE_P;
            nal_slice_head->i_nal_size_0_7 = 0;    
            nal_slice_head->i_nal_size_8_15 = 0;
            nal_slice_head->i_nal_size_16_23 = 0;
#endif
            nal_info->nal_unit_type = nal->i_type = NAL_SLICE;
            break;
        case H264_FRAME_TYPE_I:
#ifdef  ADD_DVM_NAL_HEAD
            nal_slice_head->e_nal_type = DVM_NAL_SLICE_I;
            nal_slice_head->i_nal_size_0_7 = 0;    
            nal_slice_head->i_nal_size_8_15 = 0;
            nal_slice_head->i_nal_size_16_23 = 0;
#endif
            nal_info->nal_unit_type = nal->i_type = NAL_SLICE;
            break;
        default:
#ifdef  ADD_DVM_NAL_HEAD
            nal_slice_head->e_nal_type = DVM_NAL_SLICE_IDR;
            nal_slice_head->i_nal_size_0_7 = 0;    
            nal_slice_head->i_nal_size_8_15 = 0;
            nal_slice_head->i_nal_size_16_23 = 0;
#endif
            nal_info->nal_unit_type = nal->i_type = NAL_SLICE_IDR;
            break;            
    }
#if NAL_FILL_ZERO_NUM
    nal_info->zero1 = 0;
    nal_info->zero2 = 0;
#endif
    nal_info->zero3 = 0;
    nal_info->zero4 = 0;
    nal_info->zero5 = 0;
    nal_info->constant_value_1 = 1;
    nal_info->forbidden_zero_bit = 0;
    nal_info->nal_ref_idc = nal->i_ref_idc = NAL_PRIORITY_LOW;

    nal->slice_paysize = sizeof(h264_nal_bitstream_t);
    nal->slice_paysize += h264_gen_slice_head(h264_nal_slice_head->rbsp, h264_encode_control->slice_head, nal);
    frame->frame_len += nal->slice_paysize;
    ch =  &packet->data[frame->frame_len];
    switch(frame->frame_len&0x1) {
        case 0:
            nal->i_temp_bitalign_content <<= 8;
            break;
        default:
            frame->frame_len--;
            ch--;
            temp_value = *ch;
            temp_value <<= 8;
            temp_value |= (nal->i_temp_bitalign_content);
            nal->i_temp_bitalign_content = temp_value;
            nal->i_temp_bitalign_number += 8;
            break;
    }

    frame->i_init_qp = h264_encode_control->pps->i_pic_init_qp;
    return nal->slice_paysize;
}

int gen_ext_nal_audio_header(char *buf, int audio_type, int len)
{
    ext_h264_nal_bitstream_t    *ext_nal_audio_head = (ext_h264_nal_bitstream_t*)buf;
#ifdef  ADD_DVM_NAL_HEAD
    DVM_NAL_HEAD                *nal_audio_head;
#endif
    H264_NAL_INFO               *nal_info;
    int ret ;

#ifdef  ADD_DVM_NAL_HEAD 
    nal_audio_head = &ext_nal_audio_head->head;
#endif
    nal_info = &ext_nal_audio_head->nal;

#ifdef  ADD_DVM_NAL_HEAD
    ret = sizeof(ext_h264_nal_bitstream_t) + len - sizeof(DVM_NAL_HEAD);
    nal_audio_head->i_nal_size_0_7 = (ret&0xff);    
    nal_audio_head->i_nal_size_8_15 = ((ret>>8)&0xff);
    nal_audio_head->i_nal_size_16_23 = ((ret>>16)&0xff);
#endif
    switch(audio_type){
        default:
        case ISIL_AUDIO_PCM:
#ifdef  ADD_DVM_NAL_HEAD
            nal_audio_head->e_nal_type = DVM_NAL_AUDIO_PCM;
#endif 
            ext_nal_audio_head->ext_type = NAL_TYPE_EX_A_PCM;
            break;
        case ISIL_AUDIO_ALAW:
#ifdef  ADD_DVM_NAL_HEAD
            nal_audio_head->e_nal_type = DVM_NAL_AUDIO_ALAW; 
#endif 
            ext_nal_audio_head->ext_type = NAL_TYPE_EX_A_ALAW;
            break;
        case ISIL_AUDIO_ULAW:
#ifdef  ADD_DVM_NAL_HEAD
            nal_audio_head->e_nal_type = DVM_NAL_AUDIO_ULAW; 
#endif 
            ext_nal_audio_head->ext_type = NAL_TYPE_EX_A_ULAW;
            break;
        case ISIL_AUDIO_ADPCM_32K:
#ifdef  ADD_DVM_NAL_HEAD
            nal_audio_head->e_nal_type = DVM_NAL_AUDIO_IMAADPCM; 
#endif 
            ext_nal_audio_head->ext_type = NAL_TYPE_EX_A_IMAADPCM;
            break;
        case ISIL_AUDIO_ADPCM_48K:
#ifdef  ADD_DVM_NAL_HEAD
            nal_audio_head->e_nal_type = DVM_NAL_AUDIO_IMAADPCM; 
#endif 
            ext_nal_audio_head->ext_type = NAL_TYPE_EX_A_IMAADPCM;
            break;
        case ISIL_AUDIO_ADPCM_16K:
#ifdef  ADD_DVM_NAL_HEAD
            nal_audio_head->e_nal_type = DVM_NAL_AUDIO_IMAADPCM; 
#endif 
            ext_nal_audio_head->ext_type = NAL_TYPE_EX_A_IMAADPCM;
            break;
    }
#if NAL_FILL_ZERO_NUM
    nal_info->zero1 = 0;
    nal_info->zero2 = 0;
#endif
    nal_info->zero3 = 0;
    nal_info->zero4 = 0;
    nal_info->zero5 = 0;
    nal_info->constant_value_1 = 1;
    nal_info->forbidden_zero_bit = 0;
    nal_info->nal_ref_idc = NAL_PRIORITY_LOW;
    nal_info->nal_unit_type = NAL_TYPE_EX_AUDIO;

    ret = sizeof(ext_h264_nal_bitstream_t) + len;
    return ret;
}

int  gen_ext_mjpeg_header(char *buf, int len)
{
    ext_h264_nal_bitstream_t    *ext_nal_mjpeg_head = (ext_h264_nal_bitstream_t*)buf;
#ifdef  ADD_DVM_NAL_HEAD
    DVM_NAL_HEAD                *nal_mjpeg_head;
#endif
    H264_NAL_INFO               *nal_info;
    int ret ;

#ifdef  ADD_DVM_NAL_HEAD 
    nal_mjpeg_head = &ext_nal_mjpeg_head->head;
#endif
    nal_info = &ext_nal_mjpeg_head->nal;
#ifdef  ADD_DVM_NAL_HEAD
    ret = sizeof(ext_h264_nal_bitstream_t) + len - sizeof(DVM_NAL_HEAD);
    nal_mjpeg_head->i_nal_size_0_7 = (ret&0xff);
    nal_mjpeg_head->i_nal_size_8_15 = ((ret>>8)&0xff);
    nal_mjpeg_head->i_nal_size_16_23 = ((ret>>16)&0xff);
    nal_mjpeg_head->e_nal_type = DVM_NAL_MJPEG;
#endif
#if NAL_FILL_ZERO_NUM
    nal_info->zero1 = 0;
    nal_info->zero2 = 0;
#endif
    nal_info->zero3 = 0;
    nal_info->zero4 = 0;
    nal_info->zero5 = 0;
    nal_info->constant_value_1 = 1;
    nal_info->forbidden_zero_bit = 0;
    nal_info->nal_ref_idc = NAL_PRIORITY_LOW;
    nal_info->nal_unit_type = NAL_TYPE_EX_PIC;

    ext_nal_mjpeg_head->ext_type = NAL_TYPE_EX_P_MJPEG;
    ret = sizeof(ext_h264_nal_bitstream_t) + len;
    return ret;
}

int  gen_ext_mv_header(char *buf, int len)
{
    ext_h264_nal_bitstream_t    *ext_nal_mv_head = (ext_h264_nal_bitstream_t*)buf;
#ifdef  ADD_DVM_NAL_HEAD
    DVM_NAL_HEAD                *nal_mv_head;
#endif
    H264_NAL_INFO               *nal_info;
    int ret ;

#ifdef  ADD_DVM_NAL_HEAD 
    nal_mv_head = &ext_nal_mv_head->head;
#endif
    nal_info = &ext_nal_mv_head->nal;

#ifdef  ADD_DVM_NAL_HEAD
    ret = sizeof(ext_h264_nal_bitstream_t) + len - sizeof(DVM_NAL_HEAD);
    nal_mv_head->i_nal_size_0_7 = (ret&0xff);
    nal_mv_head->i_nal_size_8_15 = ((ret>>8)&0xff);
    nal_mv_head->i_nal_size_16_23 = ((ret>>16)&0xff);
    nal_mv_head->e_nal_type = DVM_NAL_MV;
#endif
#if NAL_FILL_ZERO_NUM
    nal_info->zero1 = 0;
    nal_info->zero2 = 0;
#endif
    nal_info->zero3 = 0;
    nal_info->zero4 = 0;
    nal_info->zero5 = 0;
    nal_info->constant_value_1 = 1;
    nal_info->forbidden_zero_bit = 0;
    nal_info->nal_ref_idc = NAL_PRIORITY_LOW;
    nal_info->nal_unit_type = NAL_TYPE_EX_MV;

    ext_nal_mv_head->ext_type = NAL_TYPE_EX_MV_D1;
    ret = sizeof(ext_h264_nal_bitstream_t) + len;
    return ret;
}


