#ifndef	ALGO_NAL_DRIVER_H
#define	ALGO_NAL_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define	SLICE_HEAD_BUF_SIZE (128)
#ifdef  ADD_DVM_NAL_HEAD
#define	NAL_HEAD_LEN        (12)
#else
#define	NAL_HEAD_LEN        (8)
#endif
#define	MAX_SPS_RBSP_LEN    (SLICE_HEAD_BUF_SIZE-NAL_HEAD_LEN)
#define	MAX_PPS_RBSP_LEN    MAX_SPS_RBSP_LEN
#define	MAX_SLICE_HEAD_LEN  MAX_SPS_RBSP_LEN

#define NAL_FILL_ZERO_NUM 0
    enum profile_e
    {
        PROFILE_BASELINE    = 66,
        PROFILE_MAIN        = 77,
        PROFILE_EXTENTED    = 88,
        PROFILE_HIGH        = 100,
        PROFILE_HIGH10      = 110,
        PROFILE_HIGH422     = 122,
        PROFILE_HIGH444     = 144
    };

    enum nal_unit_type {
        NAL_UNKNOWN         = 0,
        NAL_SLICE           = 1,
        NAL_SLICE_DPA       = 2,
        NAL_SLICE_DPB       = 3,
        NAL_SLICE_DPC       = 4,
        NAL_SLICE_IDR       = 5,
        NAL_SEI             = 6,
        NAL_SPS             = 7,
        NAL_PPS             = 8,
        NAL_AUD             = 9,
        NAL_SERIAL_OVER     = 10,
        NAL_STREAM_OVER     = 11,
        NAL_PADDING         = 12,
        NAL_TYPE_EX_PIC     = 24,
        NAL_TYPE_EX_VIDEO   = 25,
        NAL_TYPE_EX_AUDIO   = 26,
        NAL_TYPE_EX_MV      = 27,
        NAL_TIMESTAMP_INFO = 28,
    };

    enum	NAL_TYPE_EXT_TIMESTAMP_INFO{
        NAL_TYPE_EXT_H264_ENCODE_TIMESTAMP = 0,
        NAL_TYPE_EXT_AUDIO_ENCODE_TIMESTAMP,
        NAL_TYPE_EXT_ENCODE_MJPEG_TIMESTAMP,
        NAL_TYPE_EXT_ENCODE_MVF_TIMESTAMP,
        NAL_TYPE_EXT_ENCODE_MVD_TIMESTAMP,
    };

    enum DVM_NAL_TYPE_EX_PIC{
        NAL_TYPE_EX_P_MJPEG = 0x80,
        NAL_TYPE_EX_P_BMP   = 0x81,
    };

    enum DVM_NAL_TYPE_EX_V{
        NAL_TYPE_EX_V_MPEG  = 0x90,
    };

    enum DVM_NAL_TYPE_EX_A{
        NAL_TYPE_EX_A_PCM       = 0xC0,
        NAL_TYPE_EX_A_ALAW      = 0xC1,
        NAL_TYPE_EX_A_ULAW      = 0xC2,
        NAL_TYPE_EX_A_ADPCM     = 0xC3,
        NAL_TYPE_EX_A_IMAADPCM  = 0xC4,
    };

    enum DVM_NAL_TYPE_EX_MV{
        NAL_TYPE_EX_MV_D1   = 0xD1,
    };

    enum nal_priority
    {
        NAL_PRIORITY_DISPOSABLE = 0,
        NAL_PRIORITY_LOW = 1,
        NAL_PRIORITY_HIGH = 2,
        NAL_PRIORITY_HIGHEST = 3
    };

    enum eisil_nal_type
    {
        DVM_NAL_UNKNOWN      = 0,
        DVM_NAL_MV           = 0x50,
        DVM_NAL_MJPEG        = 0x80,
        DVM_NAL_BMP          = 0x81,
        DVM_NAL_SPS          = 0xA0,
        DVM_NAL_PPS          = 0xA1,
        DVM_NAL_SLICE        = 0xB0,
        DVM_NAL_SLICE_IDR    = 0xB1,
        DVM_NAL_SLICE_DPA    = 0xB2,
        DVM_NAL_SLICE_DPB    = 0xB3,
        DVM_NAL_SLICE_DPC    = 0xB4,
        DVM_NAL_SLICE_P      = 0xB5,
        DVM_NAL_SLICE_I      = 0xB6,
        DVM_NAL_AUDIO_PCM    = 0xC0,
        DVM_NAL_AUDIO_ALAW   = 0xC1,
        DVM_NAL_AUDIO_ULAW   = 0xC2,
        DVM_NAL_AUDIO_ADPCM  = 0xC3,
        DVM_NAL_AUDIO_IMAADPCM  = 0xC4,
    };

#ifdef  ADD_DVM_NAL_HEAD
    struct _tag_isil_nal_head {
        u8  e_nal_type;
        u8  i_nal_size_0_7;
        u8  i_nal_size_8_15;
        u8  i_nal_size_16_23;
    }STRUCT_PACKET_ALIGN(1);
#endif

    typedef struct nal_info{
#if NAL_FILL_ZERO_NUM
        __u8    zero1;
        __u8    zero2;
#endif
        __u8    zero3;
        __u8    zero4;
        __u8    zero5;
        __u8    constant_value_1;
#if defined(__LITTLE_ENDIAN_BITFIELD)
        __u8    nal_unit_type : 5;
        __u8    nal_ref_idc : 2;
        __u8    forbidden_zero_bit : 1;
#elif defined (__BIG_ENDIAN_BITFIELD)
        __u8    forbidden_zero_bit : 1;
        __u8    nal_ref_idc : 2;
        __u8    nal_unit_type : 5;
#else
#error	one of __LITTLE_ENDIAN_BITFIELD or __BIG_ENDIAN_BITFIELD must be defined
#endif
    }STRUCT_PACKET_ALIGN(1) NAL_INFO;

    struct h264_nal_head{
#ifdef  ADD_DVM_NAL_HEAD
        DVM_NAL_HEAD    head;
#endif
        H264_NAL_INFO   nal;
        u8              rbsp[0];
    }STRUCT_PACKET_ALIGN(1);

    struct ext_h264_nal_head{
#ifdef  ADD_DVM_NAL_HEAD
        DVM_NAL_HEAD    head;
#endif
        H264_NAL_INFO   nal;
        u8              ext_type;
        u8              rbsp[0];
    }STRUCT_PACKET_ALIGN(1);

    struct h264_nal {
        int     i_ref_idc;
        int     i_type;
        int     header_align_number;
        int     i_temp_bitalign_number;
        __u32   i_temp_bitalign_content;
        int     i_end_bitalign_number;
        __u8    *slice_head_buf;
        __u8    *sps_payload;
        int     sps_paysize;
        __u8    *pps_payload;
        int     pps_paysize;
        __u8    *slice_payload;
        int     slice_paysize;
    };

    struct h264_sps
    {
        int i_profile_idc;
        int b_constraint_set0;
        int b_constraint_set1;
        int b_constraint_set2;
        int i_level_idc;
        int i_id;
        int i_log2_max_frame_num;
        int i_poc_type;
        /* poc 0 */
        int i_log2_max_poc_lsb;
        /* poc 1 */
        int b_delta_pic_order_always_zero;
        int i_offset_for_non_ref_pic;
        int i_offset_for_top_to_bottom_field;
        int i_num_ref_frames_in_poc_cycle;
        int i_offset_for_ref_frame[256];

        int i_num_ref_frames;
        int b_gaps_in_frame_num_value_allowed;
        int i_mb_width;
        int i_mb_height;
        int b_frame_mbs_only;
        int b_mb_adaptive_frame_field;
        int b_direct8x8_inference;

        int b_crop;
        int b_vui;
    };

    struct h264_pps
    {
        int i_id;
        int i_sps_id;

        int b_cabac;

        int b_pic_order;
        int i_num_slice_groups;

        int i_num_ref_idx_l0_active;
        int i_num_ref_idx_l1_active;

        int b_weighted_pred;
        int b_weighted_bipred;

        int i_pic_init_qp;
        int i_pic_init_qs;

        int i_chroma_qp_index_offset;

        int b_deblocking_filter_control;
        int b_constrained_intra_pred;
        int b_redundant_pic_cnt;
    };

    enum slice_type
    {
        SLICE_TYPE_P = 0,
        SLICE_TYPE_B = 1,
        SLICE_TYPE_I = 2,
        SLICE_TYPE_SP = 3,
        SLICE_TYPE_SI = 4
    };

    struct h264_slice_header
    {
        h264_sps_t *sps;
        h264_pps_t *pps;

        int i_type;
        int i_first_mb;
        int i_last_mb;
        int i_pps_id;
        int i_frame_num;
        int b_field_pic;
        int b_bottom_field;
        int i_idr_pic_id;   /* -1 if nal_type != 5 */
        int i_poc_lsb;
        int i_delta_poc_bottom;
        int i_delta_poc[2];
        int i_redundant_pic_cnt;
        int b_direct_spatial_mv_pred;
        int b_num_ref_idx_override;
        int i_num_ref_idx_l0_active;
        int i_num_ref_idx_l1_active;
        int b_ref_pic_list_reordering_l0;
        int b_ref_pic_list_reordering_l1;
        struct
        {
            int idc;
            int arg;
        }ref_pic_list_order[2][16];

        int i_cabac_init_idc;
        int i_qp_delta;
        int b_sp_for_swidth;
        int i_qs_delta;

        /* deblocking filter */
        int i_disable_deblocking_filter_idc;
        int i_alpha_c0_offset;
        int i_beta_offset;
    };


    extern int  gen_h264_sps(isil_h264_encode_control_t *h264_encode_control, isil_video_frame_tcb_t *frame);
    extern int  gen_h264_pps(isil_h264_encode_control_t *h264_encode_control, isil_video_frame_tcb_t *frame);
    extern int  gen_h264_slicehead(isil_h264_encode_control_t *h264_encode_control, isil_video_frame_tcb_t *frame, int frame_type);
    extern int  gen_ext_nal_audio_header(char *buf, int audio_type, int len);
    extern int  gen_ext_mjpeg_header(char *buf, int len);
    extern int  gen_ext_mv_header(char *buf, int len);


#ifdef __cplusplus
}
#endif

#endif	//ALGO_NAL_DRIVER_H

