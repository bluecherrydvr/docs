#ifndef	DDR_BURST_INTERFACE_H
#define	DDR_BURST_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MPI_ACCESS_PAGE_MODE_REG_OFFSET (0x00030)
#define MPI_CMD_STATUS_REG_OFFSET       (0x80000)
#define MPI_BURST_DDR_ADDR_REG_OFFSET   (0x80004)
#define MPI_BURST_SRAM_ADDR_REG_OFFSET  (0x80008)
#define MPI_DPRAM_BASE_OFFSET           (0x84000)

#define DPRAM_SEGMENT_SIZE              (0x0800)
#define DPRAM_PAGE_SIZE                 (0x0800)
#define DPRAM_PAGE_NUMBER               (1)

#define BURST_INTERFACE_CHECKUP_TIMEOUT (200)

#define VIDEO_BITSTREAM_DDR_SECTION_NUMBER  (8)

    struct access_page_mode_reg{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32	page_id : 8;
	u32 reserve8_12 : 5;
	u32 mpi_burst_read_4_enable : 1;
	u32 ddr_a_b_select : 1;
	u32 burst_mode_en : 1;
	u32 reserve16_31 : 16;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve16_31 : 16;
	u32 burst_mode_en : 1;
	u32 ddr_a_b_select : 1;
	u32 mpi_burst_read_4_enable : 1;
	u32 reserve8_12 : 5;
	u32	page_id : 8;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    };

    typedef union{
	struct access_page_mode_reg bit_value;
	u32 value;
    }access_page_mode_reg_value;

    struct burst_cmd_status_reg{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 burst_len : 16;
	u32 burst_dir : 1;
	u32 burst_trigger : 1;
	u32 reserve18_23 : 6;
	u32 ddr_burst_end_flag : 1;
	u32 ddr_single_access_err_int_enable : 1;
	u32 ddr_burst_access_err_int_enable : 1;
	u32 burst_end_int_enable : 1;
	u32 ddr_single_access_err_flag : 1;
	u32 ddr_single_access_busy_flag : 1;
	u32 ddr_burst_err_flag : 1;
	u32 ddr_burst_busy_flag : 1;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 ddr_burst_busy_flag : 1;
	u32 ddr_burst_err_flag : 1;
	u32 ddr_single_access_busy_flag : 1;
	u32 ddr_single_access_err_flag : 1;
	u32 burst_end_int_enable : 1;
	u32 ddr_burst_access_err_int_enable : 1;
	u32 ddr_single_access_err_int_enable : 1;
	u32 ddr_burst_end_flag : 1;
	u32	reserve18_23 : 6;
	u32 burst_trigger : 1;
	u32 burst_dir : 1;
	u32 burst_len : 16;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    };

    typedef union{
	struct burst_cmd_status_reg bit_value;
	u32 value;
    }burst_cmd_status_reg_value;

    struct burst_ddr_base_addr_reg{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32	ddr_base_addr : 28;
	u32	reserve28_31 : 4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve28_31 : 4;
	u32 ddr_base_addr : 28;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    };

    typedef union{
	struct burst_ddr_base_addr_reg  bit_value;
	u32 value;
    }burst_ddr_base_addr_reg_value;

    struct burst_sram_base_addr_reg{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 sram_base_addr : 16;
	u32 reserve16_31 : 16;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve16_31 : 16;
	u32 sram_base_addr : 16;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    };

    typedef union{
	struct burst_sram_base_addr_reg bit_value;
	u32 value;
    }burst_sram_base_addr_reg_value;

    struct ddr_burst_interface_reg_operation{
	void    (*init)(ddr_burst_interface_t *);
	void    (*reset)(ddr_burst_interface_t *);
	void    (*get_page_mode_reg)(ddr_burst_interface_t *, isil_chip_t *);
	void    (*update_page_mode_reg)(ddr_burst_interface_t *, isil_chip_t *);
	void    (*get_burst_interface_param)(ddr_burst_interface_t *, isil_chip_t *);
	void    (*update_burst_interface_param)(ddr_burst_interface_t *, isil_chip_t *);

	void    (*enable_access_sram_block)(ddr_burst_interface_t *);
	void    (*enable_access_ddr_block)(ddr_burst_interface_t *);
	void    (*select_ddr_A)(ddr_burst_interface_t *);
	void    (*select_ddr_B)(ddr_burst_interface_t *);
	void    (*enable_read_4)(ddr_burst_interface_t *);
	void    (*disable_read_4)(ddr_burst_interface_t *);
	void    (*set_ddr_page_number)(ddr_burst_interface_t *, int page_id);

	void    (*set_ddr_burst_param)(ddr_burst_interface_t *, int len, int dir);
	void    (*start_ddr_burst)(ddr_burst_interface_t *);
	void    (*int_enable)(ddr_burst_interface_t *, int , int , int);
	int	    (*get_single_access_err_flag)(ddr_burst_interface_t *);
	int     (*get_burst_access_err_flag)(ddr_burst_interface_t *);
	int     (*get_burst_busy_flag)(ddr_burst_interface_t *);
	int     (*get_burst_end_flag)(ddr_burst_interface_t *);
	void    (*clear_burst_end_flag)(ddr_burst_interface_t *);
	void    (*set_burst_ddr_addr)(ddr_burst_interface_t *, u32 addr_offset);
	void    (*set_burst_sram_addr)(ddr_burst_interface_t *, u32 addr_offset);

	int     (*pio_host_to_sram_write)(ddr_burst_interface_t *, dpram_page_node_t *, u32 *, int , int);
	int     (*dma_host_to_sram_write)(ddr_burst_interface_t *, dpram_page_node_t *, dma_addr_t , int , int);
	int     (*pio_host_to_sram_read)(ddr_burst_interface_t *, dpram_page_node_t *, u32 *, int , int);
	int     (*dma_host_to_sram_read)(ddr_burst_interface_t *, dpram_page_node_t *, dma_addr_t , int , int);

	int     (*start_block_transfer_from_sram_to_ddr)(ddr_burst_interface_t *, dpram_page_node_t *, u32 , int , int , int);
	int     (*start_nonblock_transfer_from_sram_to_ddr)(ddr_burst_interface_t *, dpram_page_node_t *, u32 , int , int , int);
	int     (*start_block_transfer_from_ddr_to_sram)(ddr_burst_interface_t *, dpram_page_node_t *, u32 , int , int , int);
	int     (*start_nonblock_transfer_from_ddr_to_sram)(ddr_burst_interface_t *, dpram_page_node_t *, u32 , int , int , int);
	void    (*clear_burst_done)(ddr_burst_interface_t *);
    };

    struct ddr_burst_interface_reg{
	access_page_mode_reg_value  page_mode_reg_value;
	u32	access_page_mode_reg_offset;

	burst_cmd_status_reg_value  cmd_status_reg_value;
	u32	burst_cmd_status_reg_offset;

	burst_ddr_base_addr_reg_value   ddr_base_addr_reg_value;
	u32	burst_ddr_addr_reg_offset;

	burst_sram_base_addr_reg_value  sram_base_addr_reg_value;
	u32	burst_sram_addr_reg_offset;

	struct ddr_burst_interface_reg_operation    *op;
	isil_chip_t  *chip;
    };

    struct dpram_page_node_operation{
	u32 (*get_page_base)(dpram_page_node_t *);
	u32 (*get_page_offset)(dpram_page_node_t *);
	int (*get_page_size)(dpram_page_node_t *);
    };

    struct dpram_page_node{
	u32 dpram_base;
	u32 dpram_page_base_offset;
	int page_size;

	struct dpram_page_node_operation    *op;
    };

    struct dpram_page_resource_manage_operation{
	void    (*init)(dpram_page_resource_manage_t *);
	void    (*get_dpram_page)(dpram_page_resource_manage_t *, dpram_page_node_t **);
	void    (*put_dpram_page)(dpram_page_resource_manage_t *, dpram_page_node_t **);
	int     (*get_can_use_resource_number)(dpram_page_resource_manage_t *);
    };

    struct dpram_page_resource_manage{
	dpram_page_node_t   dpram[DPRAM_PAGE_NUMBER];
	int         dpram_number, curr_used_dpram_index;
	atomic_t    curr_can_use_dpram_page_number;
	spinlock_t  lock;

	struct dpram_page_resource_manage_operation *op;
    };

    struct ddr_video_bitstream_buf_node_operation{
	int (*get_buf_id)(ddr_video_bitstream_buf_node_t *);
	int (*get_buf_in_chip_a_or_b)(ddr_video_bitstream_buf_node_t *);
	int (*get_page_id)(ddr_video_bitstream_buf_node_t *);
	u32 (*get_buf_base_offset)(ddr_video_bitstream_buf_node_t *);
    };

#define DDR_CHIP_A      (0)
#define DDR_CHIP_B      (1)
    struct ddr_video_bitstream_buf_node{
	u32 buf_base_offset;
	int buf_id, ddr_phy_page_id;
	int chip_a_or_b;

	struct ddr_video_bitstream_buf_node_operation *op;
    };

    struct ddr_video_bitstream_resource_manage_operation{
	void    (*init)(ddr_video_bitstream_resource_manage_t *);
	void    (*get_video_bitstream)(ddr_video_bitstream_resource_manage_t *, ddr_video_bitstream_buf_node_t **);
	void    (*put_video_bitstream)(ddr_video_bitstream_resource_manage_t *, ddr_video_bitstream_buf_node_t **);
	int     (*get_can_use_resource_number)(ddr_video_bitstream_resource_manage_t *);
    };

    struct ddr_video_bitstream_resource_manage{
	ddr_video_bitstream_buf_node_t  ddr_buf_section[VIDEO_BITSTREAM_DDR_SECTION_NUMBER];
	int ddr_buf_section_number, curr_used_ddr_buf_section_index;
	atomic_t    curr_can_use_ddr_buf_section_number;
	spinlock_t  lock;

	struct ddr_video_bitstream_resource_manage_operation    *op;
    };

#define DPRAM_BLOCK_TRANSFER_REQUEST    (0)
#define DPRAM_NONBLOCK_TRANSFER_REQUEST (1)
#define to_get_dpram_request_with_request_node(node)    container_of(node, dpram_request_t, request_node)
    struct dpram_request{
	isil_chip_t  *chip;
	int     type;
	void    *context;
	int     (*req_callback)(dpram_request_t *, void *);
        int     (*delete_req_notify)(dpram_request_t *, void *context);
	tcb_node_t  request_node;
    };

    struct dpram_request_queue_operation{
	void    (*init)(dpram_request_queue_t *);
	int     (*get_queue_curr_entry_number)(dpram_request_queue_t *);
	void    (*put_service_request_into_queue)(dpram_request_queue_t *, dpram_request_t *);
	void    (*put_service_request_into_queue_header)(dpram_request_queue_t *, dpram_request_t *);
	int     (*delete_service_request_from_queue)(dpram_request_queue_t *, dpram_request_t *);
	void    (*try_get_curr_consumer_from_queue)(dpram_request_queue_t *);
	void    (*release_curr_consumer)(dpram_request_queue_t *);
	void    (*trigger_chip_pending_service_request)(dpram_request_queue_t *);
        int     (*service_request_queue_is_empty)(dpram_request_queue_t *);
    };

    struct dpram_request_queue{
	tcb_node_queue_t    request_queue;
	dpram_request_t     *curr_service_req;
	spinlock_t          lock;

	struct dpram_request_queue_operation    *op;
    };

    struct chip_ddr_burst_engine_operation{
	void    (*init)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, isil_chip_t *chip);
	void    (*get_ddr_burst_interface)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, ddr_burst_interface_t **ptr_burst_interface);
	void    (*get_dpram_page_resource_manage)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_resource_manage_t **ptr_dpram_resource);
	void    (*get_ddr_video_bitstream_resource_manage)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, ddr_video_bitstream_resource_manage_t **ptr_video_bitstream_resource);

	int     (*is_can_submit_h264_encode_service_req)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, ddr_video_bitstream_buf_node_t **);
	void    (*release_h264_ddr_page_resource)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, ddr_video_bitstream_buf_node_t **);
	int     (*is_can_submit_move_data_from_host_to_dpram_service_req)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **);
	void    (*notify_end_move_data_from_dpram_to_ddr_service_req)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **);
	void    (*release_end_move_data_from_dpram_to_ddr_service_req)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **);
	int     (*is_can_submit_move_data_from_ddr_to_dpram_service_req)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **);
	void    (*notify_end_move_data_from_dpram_to_host_service_req)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **);
	void    (*release_end_move_data_from_dpram_to_host_service_req)(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **);
    };

    struct chip_ddr_burst_engine{
	atomic_t                        bus_state;
	ddr_burst_interface_t           burst_interface;
	dpram_page_resource_manage_t    dpram_resource;
	ddr_video_bitstream_resource_manage_t   video_bitstream_resource;

	struct chip_ddr_burst_engine_operation  *op;
    };    

    struct dpram_control_operation{
	void    (*init)(dpram_control_t *);
	int     (*is_can_submit_h264_encode_service_req)(dpram_control_t *, ddr_video_bitstream_buf_node_t **, isil_chip_t *chip);
	void    (*release_h264_ddr_page_resource)(dpram_control_t *, ddr_video_bitstream_buf_node_t **, isil_chip_t *chip);
	int     (*is_can_submit_move_data_from_host_to_dpram_service_req)(dpram_control_t *, dpram_page_node_t **, isil_chip_t *chip);
	void    (*notify_end_move_data_from_dpram_to_ddr_service_req)(dpram_control_t *, dpram_page_node_t **, isil_chip_t *chip);
	void    (*release_end_move_data_from_dpram_to_ddr_service_req)(dpram_control_t *, dpram_page_node_t **, isil_chip_t *chip);
	int     (*is_can_submit_move_data_from_ddr_to_dpram_service_req)(dpram_control_t *, dpram_page_node_t **, isil_chip_t *chip);
	void    (*notify_end_move_data_from_dpram_to_host_service_req)(dpram_control_t *, dpram_page_node_t **, isil_chip_t *chip);
	void    (*release_end_move_data_from_dpram_to_host_service_req)(dpram_control_t *, dpram_page_node_t **, isil_chip_t *chip);
	void    (*submit_read_h264_bitstream_req)(dpram_control_t *, isil_h264_logic_encode_chan_t *);
	void    (*ack_read_h264_bitstream_req)(dpram_control_t *, isil_h264_logic_encode_chan_t *);
	int     (*delete_read_h264_bitstream_req)(dpram_control_t *, isil_h264_logic_encode_chan_t *);
	void    (*submit_write_video_encode_osd_data_req)(dpram_control_t *, osd_rectangle_entry_t *);
	void    (*ack_write_video_encode_osd_data_req)(dpram_control_t *, osd_rectangle_entry_t *);
	int     (*delete_wirte_video_encode_osd_data_req)(dpram_control_t *, osd_rectangle_entry_t *);
	void    (*submit_write_video_encode_osd_attr_req)(dpram_control_t *, osd_attribute_regs_group_t *);
	void    (*ack_write_video_encode_osd_attr_req)(dpram_control_t *, osd_attribute_regs_group_t *);
	int     (*delete_write_video_encode_osd_attr_req)(dpram_control_t *, osd_attribute_regs_group_t *);
	void    (*submit_read_audio_req)(dpram_control_t *, isil_audio_driver_t *);
	void    (*ack_read_audio_req)(dpram_control_t *, isil_audio_driver_t *);
	int     (*delete_read_audio_req)(dpram_control_t *, isil_audio_driver_t *);
	void    (*submit_write_audio_req)(dpram_control_t *, isil_audio_driver_t *);
	void    (*ack_write_audio_req)(dpram_control_t *, isil_audio_driver_t *);
	int     (*delete_write_audio_req)(dpram_control_t *, isil_audio_driver_t *);
	void    (*submit_read_mjpeg_req)(dpram_control_t *, isil_jpeg_logic_encode_chan_t *);
	void    (*ack_read_mjpeg_req)(dpram_control_t *, isil_jpeg_logic_encode_chan_t *);
	int     (*delete_read_mjpeg_req)(dpram_control_t *, isil_jpeg_logic_encode_chan_t *);
#ifdef  MV_MODULE
	void    (*submit_read_mvflag_req)(dpram_control_t *, isil_h264_logic_encode_chan_t *);
	void    (*ack_read_mvflag_req)(dpram_control_t *, isil_h264_logic_encode_chan_t *);
	int     (*delete_read_mvflag_req)(dpram_control_t *, isil_h264_logic_encode_chan_t *);
#endif
    };

#define HOST_END_BUS_BUSY       (1)
#define DDR_END_BUS_BUSY        (2)
#define BUS_IDLE                (0)
    struct dpram_control{
	spinlock_t  lock;
	dpram_request_queue_t   request_queue;

	struct dpram_control_operation  *op;
    };

    extern void init_ddr_burst_interface_t(ddr_burst_interface_t *burst_interface, isil_chip_t *chip);
    extern void init_dpram_page_resource_manage(dpram_page_resource_manage_t *dpram_page_resource);
    extern void init_ddr_video_bitstream_resource_manage(ddr_video_bitstream_resource_manage_t *video_bitstream_resource);
    extern void init_dpram_request_service_tcb(dpram_request_t *request_tcb);
    extern void init_dpram_request_queue(dpram_request_queue_t *request_queue);
    extern void init_chip_ddr_burst_engine(chip_ddr_burst_engine_t *chip_ddr_burst_interface, isil_chip_t *chip);
    extern void init_dpram_control(dpram_control_t *dpram);

#ifdef __cplusplus
}
#endif

#endif	//DDR_BURST_INTERFACE_H

