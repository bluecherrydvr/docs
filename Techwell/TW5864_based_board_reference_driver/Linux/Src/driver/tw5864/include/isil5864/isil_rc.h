#ifndef _DVM_RC_H_
#define _DVM_RC_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef __u8   boolean;

#define	SMOOTH_WINDOW_MIN_SIZE      (10)
#define	SMOOTH_WINDOW_MAX_SIZE      (60)
#define	RC_WINDOWS_VALUE            (20)

#define RC_MIN_BIT_BUDGET_CONST     (45)
#define RC_RHO_BITRATE_A            (1597)
#define MAX_PERCENT_QP_CHANGE_DOWN  (5)
#define MAX_PERCENT_QP_CHANGE_UP    (10)
#define QUANT_SCALE_MIN             (16)
#define QUANT_SCALE_MAX             (51)
#define QUANT_SCALE_MAX_ADD1        (QUANT_SCALE_MAX+1)
    //#define HIGH_TEXT_PERCENT           (256)
#define HIGH_TEXT_PERCENT           (2560)
#define	QP_LOOKUP_TABLE_STEP        (52)

#define FIRSTFRAME                  (103)

    struct cbr_rc_struct       
    {
	int b_flag_update;
	int bits_used_frame;
	int norm_frame_bit_budget;
	int bits_for_nontext_est;
	int min_frame_bit_budget;
	int max_frame_bit_budget;
	int prev_norm_frame_bits_used;
	int bits_per_frame;
	int total_bit_delta;
	int rho;
	int A;

	int nz_count;
	int rc_window;
	int init_rc_window;
	int prev_qp;
	int prev_two_qp;
	int prev_bit_budget;

	int ratio;
	int total_ratio;
	int first_frame_qp; 
	int first_10_max_qp;
	int first_10_min_qp;
	u32 rho_qp_table[QP_LOOKUP_TABLE_STEP];

	int intraperiod;
	int firstGOP;
	int scencechangeflag;
	int allratio;
	int statusprama;
	int framepos;
	int frameNc;

	int minQP;
	int offsetQP;
	int normts;
	int min_normts;
	int max_normts;
	unsigned    fps_true;
    };

    struct rate_control_driver_op
    {
	void	(*init_rc)(isil_h264_logic_encode_chan_t *);
	void	(*set_qp)(isil_h264_logic_encode_chan_t *, isil_video_frame_tcb_t *);
	void	(*update_rc)(isil_h264_logic_encode_chan_t *);
	void	(*jump_rc)(isil_h264_logic_encode_chan_t *);
    };

    struct cbr_driver
    {
	isil_cbr_rc_param_t rc_param;
	isil_rc_driver_op   *rc_op;
    };

    struct vbr_rc_struct
    {
	int b_flag_update;
	int bits_used_frame;
	int norm_frame_bit_budget;
	int bits_for_nontext_est;
	int min_frame_bit_budget;
	int max_frame_bit_budget;
	int prev_norm_frame_bits_used;
	int bits_per_frame;
	int total_bit_delta;
	int rho;
	int A;

	int nz_count;
	int rc_window;
	int init_rc_window;
	int prev_qp;
	int prev_two_qp;
	int prev_bit_budget;
	int bits_pFrame;

	int ratio ;
	int total_ratio;
	int first_frame_qp; 
	int first_10_max_qp;
	int first_10_min_qp;
	u32 rho_qp_table[QP_LOOKUP_TABLE_STEP];

	int leak_buffer;
	int buffer_threshold;
	int target_init_qp;
	int pre_skipnum;
    };

    struct vbr_driver
    {
	isil_vbr_rc_struct_t    rc_param;
	isil_rc_driver_op       *rc_op;
    };

    struct rc_top_driver
    {
	int is_rc_image_level_priority;
	int is_rc_image_smoothly_priority;
	int discard_frame_number_by_level;
	isil_rc_driver_op   *rc_op;
	isil_cbr_driver_t   cbr;	
	isil_vbr_driver_t   vbr;	
    };

    extern void	init_rc_driver(isil_h264_logic_encode_chan_t *h264_logic_encode_chan);

#ifdef __cplusplus
}
#endif

#endif

