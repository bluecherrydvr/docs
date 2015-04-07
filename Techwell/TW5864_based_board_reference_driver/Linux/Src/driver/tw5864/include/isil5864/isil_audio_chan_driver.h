#ifndef	ISIL_AUDIO_CHAN_DRIVER_H
#define	ISIL_AUDIO_CHAN_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

    struct audio_section_ptr_info_operation{
	void    (*init)(audio_section_ptr_info_t *audio_section_param);
	void    (*update_header_ptr)(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio);
	void    (*update_tailer_ptr)(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio);
	int     (*queue_capacity)(audio_section_ptr_info_t *audio_section_param);
	void    (*update_curr_section_offset)(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio);
	int     (*get_update_finish_id)(audio_section_ptr_info_t *audio_section_param);
    };

    struct audio_section_ptr_info{
	int audio_section_queue_header_ptr;
	int audio_section_queue_header_ptr_ext_flag;
	int last_audio_section_queue_header_ptr_ext_flag;
	int audio_section_queue_tailer_ptr;
	int audio_section_queue_tailer_ptr_ext_flag;
	int section_queue_size;
	int discard_number;
	int page_id;
	int chip_a_or_b;
	int ddr_end_addr;

	struct audio_section_ptr_info_operation *op;
    };

    struct isil_audio_chan_push_interface_operation{
	int	(*chan_is_open)(isil_audio_chan_push_interface_t *, isil_audio_driver_t *);
	void	(*copy_data)(isil_audio_chan_push_interface_t *, isil_audio_driver_t *, char *data, int len);
    };

    struct isil_audio_chan_push_interface{
	struct isil_audio_chan_push_interface_operation	*op;
    };
    enum audio_driver_type{
	ISIL_AUDIO_ENCODE = 0,
	ISIL_AUDIO_DECODE
    };

#define	to_isil_audio_chan_driver_with_logic_chan_ed(node)          container_of(node, isil_audio_driver_t, logic_chan_ed)
#define	to_isil_audio_chan_driver_with_opened_logic_chan_ed(node)   container_of(node, isil_audio_driver_t, opened_logic_chan_ed)
#define	to_get_audio_chan_driver_with_audio_section(node)           container_of(node, isil_audio_driver_t, audio_section_param)
    struct isil_audio_chan_driver {
	isil_timestamp_t            timestamp;
	isil_audio_packet_queue_t   audio_packet_queue;
	isil_audio_chan_buf_pool_t  audio_buf_pool;
	audio_section_ptr_info_t    audio_section_param;

	enum audio_driver_type      type;
	int read_bitstream_mode;
	int b_flush_all_frame;
	ed_tcb_t    logic_chan_ed;
	ed_tcb_t    opened_logic_chan_ed;
	isil_send_msg_controller_t  send_msg_contr;
	int audio_logic_chan_id;
	int audio_phy_chan_id;
	atomic_t    audio_sync_discard_number;
        spinlock_t  fsm_matrix_call_lock;
	u32 i_frame_serial;
	u32 discard_number;
	atomic_t    opened_flag;
	robust_processing_control_t audio_chan_robust;
	dpram_request_t     audio_bitstream_req;
	dpram_page_node_t   *dpram_page;
	chip_audio_t        *chip_audio;
	isil_avSync_dev_t *isil_device_chan;
	isil_audio_chan_push_interface_t    audio_chan_push_interface;
    };

    extern void reset_isil_audio_encode_chan(isil_audio_driver_t *audio_driver);
    extern void reset_isil_audio_encode_chan_preprocessing(ed_tcb_t *opened_logic_chan_ed);
    extern void reset_isil_audio_encode_chan_postprocessing(ed_tcb_t *opened_logic_chan_ed);
    extern void reset_isil_audio_encode_chan_processing(ed_tcb_t *opened_logic_chan_ed);
    extern void start_audio_chan_robust_process(isil_audio_driver_t *audio_driver);
    extern void isil_audio_chan_robust_process_done(isil_audio_driver_t *audio_driver);
    extern void isil_audio_chan_wait_robust_process_done(isil_audio_driver_t *audio_driver);
    extern int  isil_audio_chan_is_in_robust_processing(isil_audio_driver_t *audio_driver);
    extern int  init_isil_audio_encode_chan(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio, int bus_id, int chip_id, int group_chan_id);
    extern void remove_isil_audio_encode_chan(isil_audio_driver_t *audio_driver);
    extern void reset_isil_audio_decode_chan(isil_audio_driver_t *audio_driver);
    extern void reset_isil_audio_decode_chan_preprocessing(ed_tcb_t *opened_logic_chan_ed);
    extern void reset_isil_audio_decode_chan_postprocessing(ed_tcb_t *opened_logic_chan_ed);
    extern void reset_isil_audio_decode_chan_processing(ed_tcb_t *opened_logic_chan_ed);
    extern int  init_isil_audio_decode_chan(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio, int bus_id, int chip_id, int group_chan_id);
    extern void remove_isil_audio_decode_chan(isil_audio_driver_t *audio_driver);
    extern void flush_audio_encode_curr_consumer_frame(isil_audio_driver_t *audio_driver);
    extern void flush_audio_encode_curr_producer_frame(isil_audio_driver_t *audio_driver);
    extern void flush_audio_encode_frame_queue(isil_audio_driver_t *audio_driver);
    extern void flush_all_audio_encode_frame(isil_audio_driver_t *audio_driver);
    extern void flush_audio_decode_curr_consumer_frame(isil_audio_driver_t *audio_driver);
    extern void flush_audio_decode_curr_producer_frame(isil_audio_driver_t *audio_driver);
    extern void flush_audio_decode_frame_queue(isil_audio_driver_t *audio_driver);
    extern void flush_all_audio_decode_frame(isil_audio_driver_t *audio_driver);

#ifdef __cplusplus
}
#endif

#endif	//ISIL_AUDIO_CHAN_DRIVER_H

