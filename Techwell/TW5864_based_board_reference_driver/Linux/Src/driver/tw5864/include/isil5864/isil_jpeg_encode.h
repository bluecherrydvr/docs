#ifndef	_ISIL_JPEG_CODEC_H__
#define	_ISIL_JPEG_CODEC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define VIDEO_JPEG_CMD_PATH0             (0xC800)
#define VIDEO_JPEG_CMD_PATH1             (0xC804)
#define VIDEO_JPEG_DDR_BASE              (0x28800)
#define	VIDEO_JPEG_BUF_POOL_LEN          (0x80000)	//512KB

#define JPEG_CMD_SINGLE                 (0x001)
#define JPEG_CMD_BURST                  (0x002)
#define JPEG_CMD_CHANNEL(n)             (((n) & 0xf)<<2)
#define JPEG_CMD_FORMAT(fmt)            (((fmt) & 0x3) << 6)
#define JPEG_CMD_PROG                   (0x100)
#define JPEG_CMD_DSAMPLE                (0x200)

#define JPEG_ENCODE_QSCALE          (0xd000)
#define JPEG_FRAME_VLD_0            (0xd004)
#define JPEG_FRAME_VLD_1            (0xd008)
#define JPEG_ENCODE_CONTROL         (0xd00c)
#define JPEG_IRQ                    (0xd010)
#define JPEG_LENGTH(n)	            (0xd040 + (n) * sizeof(int))  
#define JPEG_TIMESTAMP(bufid)       (0xd060 + ((bufid) / 2) * sizeof(int))
#define JPEG_VIDEO_FORMAT           (0xd07c)
#define JPEG_DBG_OVERFLOW           (0xd880)
#define JPEG_PCI_MASTER             (0xd0f8)
#define JPEG_MODULE_REST            (0xd0fc)

#define JPEG_CAP_PATH0_ENABLE       (0xd014)
#define JPEG_CAP_PATH1_ENABLE       (0xd018)
#define JPEG_CAP_TIMEOUT            (0xd01c)

//#define JPEG_MAX_RESOURCE_POOL      (2)
#define JPEG_MAX_RESOURCE_POOL      (1)
#define JPEG_MAX_RESOURCE_SIZE      (4)

    /*ioctl*/
#define DVM_JPEG_IOC_MAGIC          ISIL_CODEC_IOC_MAGIC
#define	ISIL_MJPEG_ENCODE_PARAM_GET       	_IOR(DVM_JPEG_IOC_MAGIC, 17, unsigned int)
#define	ISIL_MJPEG_ENCODE_PARAM_SET	        _IOW(DVM_JPEG_IOC_MAGIC, 18, unsigned int)

#define ISIL_MJPEG_MIN_INTERVAL       (80)
#define ISIL_MJPEG_MAX_INTERVAL       (60000)
    typedef struct isil_vj_resource_queue_operation isil_vj_resource_queue_operation_t;
    typedef struct isil_vj_resource_queue           isil_vj_resource_queue_t;
    typedef struct isil_vj_resource_tcb_operation isil_vj_resource_tcb_operation_t;
    typedef struct isil_vj_resource_tcb           isil_vj_resource_tcb_t;
    typedef struct isil_vj_resource_pool_operation isil_vj_resource_pool_operation_t;
    typedef struct isil_vj_resource_pool          isil_vj_resource_pool_t;
    typedef struct isil_jpeg_encode_property      isil_jpeg_encode_property_t;
    typedef struct isil_jpeg_encode_control_operation   isil_jpeg_encode_control_operation_t;
    typedef struct isil_jpeg_encode_control       isil_jpeg_encode_control_t;
    typedef struct isil_mjpeg_encode_param        isil_mjpeg_encode_param_t;
    typedef struct isil_jpeg_phy_video_slot_operation isil_jpeg_phy_video_slot_operation_t;
    typedef struct isil_jpeg_phy_video_slot       isil_jpeg_phy_video_slot_t;
    typedef struct isil_vj_bus                isil_vj_bus_t;
    typedef struct isil_vj_bus_operation      isil_vj_bus_operation_t;
    typedef struct jpeg_service_queue           jpeg_service_queue_t;
    typedef struct jpeg_service_tcb             jpeg_service_tcb_t;


#define	to_get_jpeg_service_tcb_with_service_tcb(node)	container_of(node, jpeg_service_tcb_t, service_tcb)
#define	to_get_jpeg_channel_with_encode_property(node)	container_of(node, isil_jpeg_logic_encode_chan_t, encode_property)
#define	to_get_jpeg_channel_with_logic_chan_ed(node)	container_of(node, isil_jpeg_logic_encode_chan_t, logic_chan_ed)
#define	to_get_jpeg_channel_with_opened_logic_chan_ed(node)	container_of(node, isil_jpeg_logic_encode_chan_t, opened_logic_chan_ed)
#define	to_get_jpeg_channel_with_encode_control(node)	container_of(node, isil_jpeg_logic_encode_chan_t, encode_control)

    typedef enum {
	JPEG_SERVICE_CAPTURE = 0,
	JPEG_SERVICE_ENCODE,
	JPEG_SERVICE_RESERVED,
    }JPEG_SERVICE_TYPE;
    struct jpeg_service_tcb{
	tcb_node_t	service_tcb;

	JPEG_SERVICE_TYPE type;

	void    *context;
	int    (*req_callback)(void *context);
    };

    struct jpeg_service_queue_operation{
	void    (*init)(jpeg_service_queue_t *);
	int     (*get_queue_curr_entry_number)(jpeg_service_queue_t *);
	void    (*put_service_request_into_queue)(jpeg_service_queue_t *, jpeg_service_tcb_t *);
	void    (*put_service_request_into_queue_header)(jpeg_service_queue_t *, jpeg_service_tcb_t *);
	void    (*try_get_curr_consumer_from_queue)(jpeg_service_queue_t *);
	void    (*release_curr_consumer)(jpeg_service_queue_t *);
	void    (*trigger_pending_service_request)(jpeg_service_queue_t *);
	int     (*delete_pending_service_request)(jpeg_service_queue_t *, jpeg_service_tcb_t *);
    };

    struct jpeg_service_queue{
	spinlock_t  lock;
	tcb_node_queue_t    service_queue;
	jpeg_service_tcb_t *curr_consumer;
	atomic_t service_count;

	struct jpeg_service_queue_operation *op;
    };
    struct isil_jpeg_phy_video_slot_operation{
        int (*init)(isil_jpeg_phy_video_slot_t *);
        int (*reset)(isil_jpeg_phy_video_slot_t *);
        int (*release)(isil_jpeg_phy_video_slot_t *);

        int (*set_video_size)(isil_jpeg_phy_video_slot_t *, enum ISIL_VIDEO_SIZE);
        int (*get_video_size)(isil_jpeg_phy_video_slot_t *);
        int (*register_logic_chan)(isil_jpeg_phy_video_slot_t *, isil_jpeg_logic_encode_chan_t *);
        int (*find_register_logic_chan)(isil_jpeg_phy_video_slot_t*, int, isil_jpeg_logic_encode_chan_t **);
        int (*unregister_logic_chan)(isil_jpeg_phy_video_slot_t *, isil_jpeg_logic_encode_chan_t *);
        int (*touch_each_register_logic_chan)(isil_jpeg_phy_video_slot_t *, void (*)(void *));
    };
    struct isil_jpeg_phy_video_slot {
        int phy_slot_id;
	enum ISIL_VIDEO_SIZE video_size;
	isil_register_table_t logic_chan_table;

	struct isil_jpeg_phy_video_slot_operation *op;
    };

    enum ISIL_MJPEG_IMAGE_LEVEL_E{
	MJPEG_IMAGE_LEVEL_0 = 0,
	MJPEG_IMAGE_LEVEL_1,
	MJPEG_IMAGE_LEVEL_2,
	MJPEG_IMAGE_LEVEL_3,
	MJPEG_IMAGE_LEVEL_4,
	MJPEG_IMAGE_LEVEL_RESERVE,
    };

#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_LEVEL_MASK           0x00000001
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_WIDTH_MASK           0x00000002
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_HEIGHT_MASK          0x00000004
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_NUMBER_MASK  0x00000008
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_STRIDE_MASK  0x00000010
#define ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_TYPE_MASK          0x00000020

#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER                       (1 << 31)
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_USER                        (1 << 30)

    struct isil_mjpeg_encode_param{
	u32 change_mask_flag;
	u32 e_image_level;
	u32 i_image_width_mb_size;
	u32 i_image_height_mb_size;
	u32 i_capture_frame_number;
	u32 i_capture_frame_stride;
	u32 i_capture_type;
    };

    struct isil_jpeg_encode_property_operation{
	int (*init)(isil_jpeg_encode_property_t *);
	int (*reset)(isil_jpeg_encode_property_t *);

	u32 (*get_image_level)(isil_jpeg_encode_property_t *);
	u32 (*get_mb_width)(isil_jpeg_encode_property_t *);
	u32 (*get_mb_height)(isil_jpeg_encode_property_t *);
	u32 (*get_capture_number)(isil_jpeg_encode_property_t *);
	u32 (*get_capture_stride)(isil_jpeg_encode_property_t *);
	u32 (*get_capture_type)(isil_jpeg_encode_property_t *);

	int (*set_image_level)(isil_jpeg_encode_property_t *, enum ISIL_MJPEG_IMAGE_LEVEL_E);
	int (*set_mb_width)(isil_jpeg_encode_property_t *, u32);
	int (*set_mb_height)(isil_jpeg_encode_property_t *, u32);
	int (*set_capture_number)(isil_jpeg_encode_property_t *, u32);
	int (*set_capture_stride)(isil_jpeg_encode_property_t *, u32);
	int (*set_capture_type)(isil_jpeg_encode_property_t *, u32);
    };

    struct isil_jpeg_encode_property{
	isil_mjpeg_encode_param_t		encode_property;
	isil_mjpeg_encode_param_t		running_encode_property;

	spinlock_t	lock;
	struct isil_jpeg_encode_property_operation	*op;
    };

    struct isil_jpeg_encode_control_operation{
	int (*init)(isil_jpeg_encode_control_t *);
	int (*reset)(isil_jpeg_encode_control_t *);
	int (*release)(isil_jpeg_encode_control_t *);

	int (*first_start)(isil_jpeg_encode_control_t *);
	int (*start_capture)(void *);
	int (*capture_notify)(void *);
	int (*start_encode)(void *);
	int (*irq_encode_fun)(int , void *);
	int (*move_data)(isil_jpeg_encode_control_t *);
	int (*notify)(isil_jpeg_encode_control_t *);

	void    (*flush_all_frame)(isil_jpeg_encode_control_t *);
	void    (*flush_curr_producer_frame)(isil_jpeg_encode_control_t *);
	void    (*flush_curr_consumer_frame)(isil_jpeg_encode_control_t *);
    void    (*flush_frame_queue)(isil_jpeg_encode_control_t *);
    };

    struct isil_jpeg_encode_control{
	u32 total_frame_len;
	atomic_t first_packet;
	int frame_base_addr;
	int section_number;
	int have_recv_section_number;
	int section_len;
	int trans_len;
	int tailer_len;
	int b_flush_all_frame;
 
	u32 last_jiffies;
	u32 int_time;
	u32 schedule_time;
	u32 lost_frame;
	u32 i_frame_serial;

        isil_jpeg_encode_control_operation_t *op;
    };

#define	to_get_isil_jpeg_encode_chan_with_logic_chan_ed(node)		container_of(node, isil_jpeg_logic_encode_chan_t, logic_chan_ed)
#define	to_get_isil_jpeg_encode_chan_with_opened_logic_chan_ed(node)	container_of(node, isil_jpeg_logic_encode_chan_t, opened_logic_chan_ed)
#define	to_get_isil_jpeg_encode_chan_with_encode_control(node)		container_of(node, isil_jpeg_logic_encode_chan_t, encode_control)
#define	to_get_isil_jpeg_encode_chan_with_encode_request_tcb(node)	container_of(node, isil_jpeg_logic_encode_chan_t, encode_request_tcb)

    struct isil_vj_resource_queue_operation{
	int (*init)(isil_vj_resource_queue_t *);
	int (*reset)(isil_vj_resource_queue_t *);
	int (*release)(isil_vj_resource_queue_t *);

	int (*get)(isil_vj_resource_queue_t *, isil_vj_resource_tcb_t **);
	int (*try_get)(isil_vj_resource_queue_t *, isil_vj_resource_tcb_t **);
	int (*try_get_curr_consumer)(isil_vj_resource_queue_t *);
	int (*put)(isil_vj_resource_queue_t *, isil_vj_resource_tcb_t *);

	u32 (*get_curr_entry_number)(isil_vj_resource_queue_t *);
    };

    struct isil_vj_resource_queue{
	tcb_node_queue_t queue;

	isil_vj_resource_tcb_t *curr_consumer;
	isil_vj_resource_tcb_t *curr_producer;

	isil_vj_resource_queue_operation_t *op; 
    };

    struct isil_jpeg_logic_encode_chan
    {
	int		phy_slot_id;
	int		logic_chan_id;
	atomic_t    opened_flag;
	robust_processing_control_t robust_processing_control;
	ed_tcb_t	logic_chan_ed;
	ed_tcb_t	opened_logic_chan_ed;
	isil_timestamp_t timestamp;
	isil_send_msg_controller_t send_msg_contr;
    spinlock_t  fsm_matrix_call_lock;

	isil_chip_t	*chip;
	isil_jpeg_phy_video_slot_t *phy_video_slot;
	isil_avSync_dev_t         *isil_device_chan;
	isil_jpeg_encode_property_t   encode_property;
	isil_jpeg_encode_control_t    encode_control;
	jpeg_service_tcb_t    encode_request_tcb;

	isil_vj_resource_queue_t    capture_resource_queue;
	isil_vj_resource_queue_t    encode_resource_queue;
	dpram_request_t             read_video_bitstream_req;
	dpram_page_node_t           *dpram_page;

	isil_vb_pool_t	pool;
	isil_vb_frame_tcb_queue_t	jpeg_frame_queue;
	u8 *push_buf;	
    };

    struct isil_vj_resource_tcb_operation{
	int (*init)(isil_vj_resource_tcb_t *);
	int (*reset)(isil_vj_resource_tcb_t *);
	int (*release)(isil_vj_resource_tcb_t *);

	int (*reference)(isil_vj_resource_tcb_t *, isil_vj_resource_tcb_t **);

	u32 (*get_video_size)(isil_vj_resource_tcb_t *);
	int (*set_video_size)(isil_vj_resource_tcb_t *, enum ISIL_VIDEO_SIZE);
	u32 (*get_image_level)(isil_vj_resource_tcb_t *);
	int (*set_image_level)(isil_vj_resource_tcb_t *, enum ISIL_MJPEG_IMAGE_LEVEL_E);
	u32 (*get_ddr_phy_addr)(isil_vj_resource_tcb_t *);
	u32 (*get_data_size)(isil_vj_resource_tcb_t *);
	u32 (*get_timestamp)(isil_vj_resource_tcb_t *);
	int (*is_ready)(isil_vj_resource_tcb_t *);
	int (*register_event_notify)(isil_vj_resource_tcb_t *, int (*)(void *), void *);
	int (*unregister_event_notify)(isil_vj_resource_tcb_t *);
	int (*start_polling_check)(isil_vj_resource_tcb_t *);
	int (*stop_polling_check)(isil_vj_resource_tcb_t *);
    };

#define to_get_vj_resource_tcb_with_node(node) container_of(node, isil_vj_resource_tcb_t, node)

    struct isil_vj_resource_tcb{
	tcb_node_t node;
	u32 resource_id;
	u32 timer_id;

	isil_vj_resource_pool_t *pool;

	enum ISIL_VIDEO_SIZE video_size;
	enum ISIL_MJPEG_IMAGE_LEVEL_E image_level;
	u32 timestamp;
	u32 data_size;

	int (*callback)(void *);
	void *context;

	isil_vj_resource_tcb_operation_t *op;
    };

    struct isil_vj_resource_pool_operation{
	int (*init)(isil_vj_resource_pool_t *);
	int (*reset)(isil_vj_resource_pool_t *);
	int (*release)(isil_vj_resource_pool_t *);

	int (*try_get_resource_tcb)(isil_vj_resource_pool_t *, isil_vj_resource_tcb_t **);
	int (*get_resource_tcb)(isil_vj_resource_pool_t *, isil_vj_resource_tcb_t **);
	int (*put_resource_tcb)(isil_vj_resource_pool_t *, isil_vj_resource_tcb_t *);

	int (*get_curr_entry_number)(isil_vj_resource_pool_t *);
    };

    struct isil_vj_resource_pool{
	u32 pool_id;
	u32 entry_number;
	isil_chip_t *chip;
	spinlock_t lock;

	u32 rd,wr;

	tcb_node_pool_t  pool;
	isil_vj_resource_tcb_t *resource_tcb_pool;
	isil_vj_resource_pool_operation_t *op;
    };

    struct isil_vj_bus_operation{
	int (*init)(isil_vj_bus_t *);
	int (*reset)(isil_vj_bus_t *);
	int (*release)(isil_vj_bus_t *);

	int (*get_ddr_pages)(isil_vj_bus_t *);
	int (*free_ddr_pages)(isil_vj_bus_t *);
    u32 (*get_ddr_page_base)(isil_vj_bus_t *);
    void (*polling_task)(isil_vj_bus_t *);
    };

#define to_get_vj_bus_with_resource_manager(manager) container_of(manager, isil_vj_bus_t, resource_manager)

    struct isil_vj_bus{
	jpeg_service_queue_t        jpeg_encode_service_queue;
	jpeg_service_queue_t        jpeg_capture_service_queue[JPEG_MAX_RESOURCE_POOL];
	isil_vj_resource_pool_t     resource_pool[JPEG_MAX_RESOURCE_POOL];
        isil_jpeg_phy_video_slot_t  phy_video_slot[ISIL_PHY_VJ_CHAN_NUMBER];

        robust_processing_control_t robust_processing_control;
        
        u32                         overflow_times;
        u32                         ddr_page_base;
        u32                         i_static_second_counter;
        u32                         i_temp_fps_counter;
        u32                         current_fps;

        struct isil_vj_bus_operation	*op;
    };

    extern int  init_isil_vj_bus(isil_vj_bus_t *);
    extern void remove_isil_vj_bus(isil_vj_bus_t *);
    extern int  init_isil_jpeg_encode_chan(isil_jpeg_logic_encode_chan_t *, int , int , int , isil_chip_t *);
    extern void remove_isil_jpeg_encode_chan(isil_jpeg_logic_encode_chan_t *);
    void    reset_isil_jpeg_encode_chan_preprocessing(isil_jpeg_logic_encode_chan_t *);
    void    reset_isil_jpeg_encode_chan_postprocessing(isil_jpeg_logic_encode_chan_t *);
    void    reset_isil_jpeg_encode_chan_processing(isil_jpeg_logic_encode_chan_t *);

    void isil_chip_jpeg_start_robust_processing(isil_vj_bus_t *vj_bus);
    void isil_chip_jpeg_wait_robust_processing_done(isil_vj_bus_t *vj_bus);
    int  isil_chip_jpeg_is_in_robust_processing(isil_vj_bus_t *vj_bus);
    void isil_chip_jpeg_robust_process_done(isil_vj_bus_t *vj_bus);
    /*JFIF define*/

#define JFIF_HEADER_SIZE 

    /*JFIF Segment Format*/
#define JFIF_ID	("JFIF")//Identifier
#define SOF(n)  (0xffc0 + (n))//Start Of Frame, (n=0 Baseline DCT, n=1 Extended sequential DCT, n=2 Progressive DCT, 
    //n=3 Lossless (sequential), n=5 Differential sequential DCT, n=6 Differential progressive DCT,
    //n=7 Differential lossless (sequential), n=8 Reserved for JPEG extensions, 
    //n=9 Extended sequential DCT, n=10 Progressive DCT, n=11 Lossless (sequential), 
    //n=13 Differential sequential DCT, n=14 Differential progressive DCT, n=15 Differential lossless (sequential))
#define DHT	    (0xffc4)//Define Huffman Table
#define DAC     (0xffcc)//Define arithmetic coding conditioning
#define RST(m)  (0xffd0 + (m))//Restart with modulo 8 count 'm'
#define SOI 	(0xffd8)//Start Of Image
#define EOI 	(0xffd9)//End Of Image
#define SOS 	(0xffda)//Start Of Scan
#define DQT	    (0xffdb)//Define Quantization Table
#define DNL	    (0xffdc)//Define Number of Lines
#define DRI	    (0xffdd)//Define Restart Interval
#define DHP     (0xffde)//Define hierarchical progression
#define EXP     (0xffdf)//Expand reference component
#define APP(n)  (0xffe0 + (n))//Identify a JPG file
#define COM     (0xfffe)//Comment

#pragma pack(1) /*align by 1byte*/

    /*application*/
    struct APP_mark{
	unsigned short flag;
	unsigned short segment_len; /*size of this segment*/
	unsigned char  string[5];
	unsigned short version;
	unsigned char  unit; /*0:none, 1:in, 2:cm*/ 
	unsigned short width;
	unsigned short height;
	unsigned char  scale_width;
	unsigned char  scale_height;
	unsigned char  scale_nbits[0];
    };

    /*define quantization table*/
    struct DQT_table{
	unsigned short flag;
	unsigned short segment_len;
	struct quant_tab{
#if defined(__BIG_ENDIAN_BITFIELD)
	    unsigned char bits:4; /*0:8 bits, 1:16 bits*/
	    unsigned char id:4;   /*0 ~ 3*/
#elif defined(__LITTLE_ENDIAN_BITFIELD)
	    unsigned char id:4;   /*0 ~ 3*/
	    unsigned char bits:4; /*0:8 bits, 1:16 bits*/
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	    unsigned char DQT_TABLE[0]; /*length = 64 x (table->bits + 1)*/
	}table;
    };

    /*start of frame*/
    struct SOF_table{
	unsigned short flag;
	unsigned short segment_len;
	unsigned char  bits; /*default 8bits*/
	unsigned short height;
	unsigned short width;
	unsigned char color;/*1:black white, 3:YCrCb ,YIQ, 4:CMYK*/
	struct color_comp{
	    unsigned char color_id;
	    unsigned char HV;/*high 4bit:Horizontal, low 4bit:Vertical*/
	    unsigned char QDT_id;
	}comp[0];/*count by color format*/
    };

    /*Difine Huffman Table*/
    struct DHT_table{
	unsigned short flag;
	unsigned short segment_len;
	struct huffman_table{
#if defined(__BIG_ENDIAN_BITFIELD)
	    unsigned char id:4; 
	    unsigned char type:4;/*0->DC, 1->AC*/
#elif defined(__LITTLE_ENDIAN_BITFIELD)
	    unsigned char type:4;/*0->DC, 1->AC*/
	    unsigned char id:4; 
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	    unsigned char nbits[16]; /*The median number of different code word*/
	    unsigned char table_data[0];
	}huff_table[0];
    };

    /*define restart interval*/
    struct DRI_table{
	unsigned short flag;
	unsigned short segment_len;
	unsigned short interval;
    };

    struct SOS_table{
	unsigned short flag;
	unsigned short segment_len;
	unsigned char  color_comp_id;
	/*can repeat, deciding by color compnment count*/
	struct START_INFO{
#if defined(__BIG_ENDIAN_BITFIELD)
	    unsigned char  DC_table:4; /*DC huffman talbe*/
	    unsigned char  AC_table:4;
#elif defined(__LITTLE_ENDIAN_BITFIELD)
	    unsigned char  AC_table:4;
	    unsigned char  DC_table:4; /*DC huffman talbe*/
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	    unsigned char  data_start;/*always 0x00*/
	    unsigned char  data_end;  /*always 0x3f*/
	    unsigned char  data_select; /*usually 0x00*/
	}start_info[0];
    };

    struct COM_segment{
	unsigned short flag;
	unsigned short segment_len;
	char string[0];
    };
#pragma pack()

    static inline short swap_short(unsigned short data)
    {
#ifdef __BIG_ENDIAN_BITFIELD
	return data;
#else
	unsigned short temp = 0;

	temp = data & 0x00ff;

	return  (temp<<8 | data>>8);
#endif
    }

#ifdef __cplusplus
}
#endif

#endif //_ISIL_JPEG_CODEC_H__

