#ifndef	DVM_OSD_H
#define	DVM_OSD_H

#ifdef __cplusplus
extern "C"
{
#endif

#define OSD_MIN_RECTANGLE_ID    (0)
#define OSD_MAX_RECTANGLE_ID    (7)

#define	TIMER_RECTANGLE_ID	(0)
#define	CHAN_NAME_RECTANGLE_ID	(1)
#define	SUBTILTE1_RECTANGLE_ID	(2)
#define	SUBTILTE2_RECTANGLE_ID	(3)
#define	SUBTILTE3_RECTANGLE_ID	(4)
#define	SUBTILTE4_RECTANGLE_ID	(5)
#define	SHELTER1_RECTANGLE_ID	(6)
#define	SHELTER2_RECTANGLE_ID	(7)
#define OSD_RECTANGLE_MASK      (0xff)
#define ATTRIBUT_DATA_ID        (8)
#define OSD_RECTANGLE_AND_ATTR_MASK (0x1ff)
#define ATTRIBUT_DONE_ID        (9)
#define OSD_ATTR_MASK           (0x300)

#define	NAME_LEN                (32)
#define	SUB_LEN                 (44)
#define	OSD_ATTR_DISPLAY_OFF    (0)
#define	OSD_ATTR_DISPLAY_ON     (0x1)
#define	OSD_FONT_12		(0x2)
#define	OSD_FONT_24             (0x8)
#define	OSD_FONT_MASK           (0xe)
#define	CHAR_RECTANGLE_X_LEFT   (8)
#define	CHAR_RECTANGLE_Y_LEFT   (2)

    struct _ISIL_OSD_PARAM {
	unsigned int    channel;
	char            name[NAME_LEN];
	unsigned int    name_attrib;
	unsigned short  name_pos_x;
	unsigned short  name_pos_y;
	unsigned int    time_attrib;
	unsigned short  time_pos_x;
	unsigned short  time_pos_y;
	unsigned int    shelter1_attrib;
	unsigned short  shelter1_pos_x;
	unsigned short  shelter1_pos_y;
	unsigned short  shelter1_width;
	unsigned short  shelter1_height;
	unsigned int    shelter2_attrib;
	unsigned short  shelter2_pos_x;
	unsigned short  shelter2_pos_y;
	unsigned short  shelter2_width;
	unsigned short  shelter2_height;
	char            subtitle1[SUB_LEN];
	unsigned int    subtitle1_attrib;
	unsigned short  subtitle1_pos_x;
	unsigned short  subtitle1_pos_y;
	char            subtitle2[SUB_LEN];
	unsigned int    subtitle2_attrib;
	unsigned short  subtitle2_pos_x;
	unsigned short  subtitle2_pos_y;
	char            subtitle3[SUB_LEN];
	unsigned int    subtitle3_attrib;
	unsigned short  subtitle3_pos_x;
	unsigned short  subtitle3_pos_y;
	char            subtitle4[SUB_LEN];
	unsigned int    subtitle4_attrib;
	unsigned short  subtitle4_pos_x;
	unsigned short  subtitle4_pos_y;
    };

#define	OSD_CHAN_RECTANGLE_NUMBER       (8)
#define	OSD_CHAN_MAX_STRING_RECTANGLE_NUMBER	(7)
#define	OSD_MB_ENTRY_NUMBER             (1024)
#define	OSD_MB_BUF_CHAR_BITMAP_LEN      (32>>2)

#define	INVALID_OSD_VALUE_ID            (-1)

#define	ENCODE_OSD_MODE0                (0)
#define	ENCODE_OSD_MODE1                (1)
#define	ENCODE_OSD_MODE2                (2)
#define	ENCODE_OSD_MODE3                (3)


    struct osd_attribute_regs1{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 rectangle_mb_x:7;
	u32 rectangle_mb_y:7;
	u32 rectangle_mb_x_width:7;
	u32 rectangle_mb_y_height:7;
	u32 data_base_addr8:1;
	u32 mode:2;
	u32 enable:1;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 enable:1;
	u32 mode:2;
	u32 data_base_addr8:1;
	u32 rectangle_mb_y_height:7;
	u32 rectangle_mb_x_width:7;
	u32 rectangle_mb_y:7;
	u32 rectangle_mb_x:7;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    union osd_attr_reg1_value{
	struct osd_attribute_regs1	reg1;
	u32	value;
    }STRUCT_PACKET_ALIGN(1);

    struct osd_attribute_regs2{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32	mode1_front_v:8;
	u32	mode1_front_u:8;
	u32	mode1_front_y:8;
	u32	data_base_addr_7_0:8;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32	data_base_addr_7_0:8;
	u32	mode1_front_y:8;
	u32	mode1_front_u:8;
	u32	mode1_front_v:8;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    union osd_attr_reg2_value{
	struct osd_attribute_regs2	reg2;
	u32	value;
    }STRUCT_PACKET_ALIGN(1);

    struct osd_attribute_regs3{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 mode1_background_v:8;
	u32 mode1_background_u:8;
	u32 mode1_background_y:8;
	u32 data_base_map_mode:2;
	u32	osd_data_address_offset:3;
	u32 reserved:3;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32	reserved:3;
	u32	osd_data_address_offset:3;
	u32 data_base_map_mode:2;
	u32 mode1_background_y:8;
	u32 mode1_background_u:8;
	u32 mode1_background_v:8;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    union osd_attr_reg3_value{
	struct osd_attribute_regs3	reg3;
	u32	value;
    }STRUCT_PACKET_ALIGN(1);

    struct osd_attribute_regs_operation{
	void (*set)(osd_attribute_regs_t *);
	void (*set_no_display)(osd_attribute_regs_t *);
	void (*reset)(osd_attribute_regs_t *);
    };

    struct osd_attribute_regs{
	union osd_attr_reg1_value	reg1_value;
	union osd_attr_reg2_value	reg2_value;
	union osd_attr_reg3_value	reg3_value;
    };

    enum ENCODE_OSD_TIMER_DISPLAY{
	YEAR_MONTH_DAY_HOUR_MINUTE_SECOND=0,
	YEAR_NONTH_DAY_NEWLINE_HOUR_MINUTE_SECOND_LEFT_ALIGN,
	YEAR_MONTH_DAY_NEWLINE_HOUR_MINUTE_SECOND_RIGHT_ALIGN,
	DONT_DISPLAY_TIMER,
    };

    struct encode_osd_timer_attr_operation{
	void    (*reset)(encode_osd_timer_attr_t *, int , int);
    };

    struct encode_osd_timer_attr{
	int	timer_display_mode;
	int	mb_x, mb_y;
	int	font_type;
	int	need_update_flag;
	osd_system_timer_t	*timer;
	struct encode_osd_timer_attr_operation  *op;
    };

    enum ENCODE_OSD_STRING_DATA_ADDR_MAP_MODE{
	Y8MB_X128MB_OSDMODE013 = 0,
	Y16MB_X64MB_OSDMODE013,
	Y32MB_X32MB_OSDMODE013,
	Y64MB_X16MB_OSDMODE013,
    };

    struct encode_osd_string_attr_operation{
	void    (*reset)(encode_osd_string_attr_t *);
    };

#define	FONT_TYPE_12X12			(0x02)
#define	ASCII12_PIXEL_WIDTH		(8)
#define	FONT12_PIXEL_WIDTH		(12)
#define	FONT_TYPE_16X16			(0x04)
#define	ASCII16_PIXEL_WIDTH		(8)
#define	FONT16_PIXEL_WIDTH		(16)
#define	FONT_TYPE_24X24			(0x08)
#define	ASCII24_PIXEL_WIDTH		(12)
#define	FONT24_PIXEL_WIDTH		(24)
#define	MB_PIXEL_WIDTH			(16)
    struct encode_osd_string_attr{
	int	mb_x, mb_y;
	int	mb_width, mb_height;
	int	data_addr_map_mode;
	int	need_update_flag;
	u8	*string;
	int	string_len;
	int	font_type;
	struct encode_osd_string_attr_operation	*op;
    };

    struct encode_osd_mask_attr_operation{
	void    (*reset)(encode_osd_mask_attr_t *);
    };

    struct encode_osd_mask_attr{
	int	mb_x, mb_y;
	int	mask_width, mask_height;
	int	y_value, u_value, v_value;
	int	data_addr_map_mode;
	int	need_update_flag;
	int	is_display;
	struct encode_osd_mask_attr_operation   *op;
    };

    struct osd_mb_pool_operation{
	int     (*create)(osd_mb_pool_t *chan_osd_mb_buf_pool, int);
	void    (*release)(osd_mb_pool_t *chan_osd_mb_buf_pool);

	void    (*get)(osd_mb_pool_t *, osd_mb_char_entry_t **, osd_rectangle_entry_t *);
	void    (*try_get)(osd_mb_pool_t *, osd_mb_char_entry_t **, osd_rectangle_entry_t *);
	void    (*put)(osd_mb_pool_t *, osd_mb_char_entry_t *);
	int     (*get_osd_mb_tcb_pool_entry_number)(osd_mb_pool_t *chan_osd_buf_pool);
    };

    struct osd_mb_pool{
	int             mb_char_cache_order;
	tcb_node_pool_t osd_mb_char_pool_tcb;
	isil_chip_t      *chip;
	osd_mb_char_entry_t *mb_char_entry_cache;

	struct osd_mb_pool_operation    *op;
    };

    struct osd_mb_char_operation{
	void    (*reset)(osd_mb_char_entry_t *);
	void    (*release)(osd_mb_char_entry_t *);
	void    (*update_mb_virtual_data)(osd_mb_char_entry_t *, int , int);
	void    (*update_fpga_osd_data)(osd_mb_char_entry_t *);
    };

#define to_get_osd_mb_char_entry_with_node(node)        container_of(node, osd_mb_char_entry_t, osd_mb_node)
    struct osd_mb_char_entry{
	osd_mb_pool_t           *pool;
	osd_rectangle_entry_t   *rectangle;
	tcb_node_t  osd_mb_node;
	int ddr_page, ddr_page_offset;
	u32 osd_data_addr_base;

	u8  mb_char_buf[OSD_MB_BUF_CHAR_BITMAP_LEN<<2];
	u32 *payload, payload_len;
	int ref_mb_x, ref_mb_y;

	struct osd_mb_char_operation    *op;
    };

    struct osd_rectangle_dpram_tcb_operation{
	void    (*init)(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip);
	void    (*submit_write_osd_rectangle_data_req)(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip);
	int     (*delete_write_osd_rectangle_data_req)(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip);
	void    (*submit_write_osd_rectangle_attr)(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip);
	int     (*delete_write_osd_rectangle_attr_req)(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip);
	void    (*response_write_osd_rectangle)(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip);
    };

    struct osd_rectangle_dpram_tcb{
	dpram_request_t     osd_data_req;
	dpram_page_node_t   *dpram_page;

	struct osd_rectangle_dpram_tcb_operation    *op;
    };

    struct osd_rectangle_operation{
	void    (*reset)(osd_rectangle_entry_t *);
	void    (*close_rectangle)(osd_rectangle_entry_t *);
	int     (*get_mb_number_at_rectangle)(osd_rectangle_entry_t *);
	int     (*get_mb24_number_at_rectangle)(osd_rectangle_entry_t *);
	void    (*put_mb_into_rectangle)(osd_rectangle_entry_t *, osd_mb_char_entry_t *);
	void    (*put_mb24_into_rectangle)(osd_rectangle_entry_t *, osd_mb_char_entry_t *);
	void    (*try_get_mb_from_rectangle)(osd_rectangle_entry_t *, osd_mb_char_entry_t **);
	int     (*update_rectangle_fpga_buf)(osd_rectangle_entry_t *);
	int     (*update_rectangle_attr_regs)(osd_rectangle_entry_t *);
	void    (*decide_rectangle_data_addr_map)(osd_rectangle_entry_t *, int video_source_size);
	void    (*set_rectangle_map_mode)(osd_rectangle_entry_t *, int);
	int     (*delete_rectangle_data_req)(osd_rectangle_entry_t *);
    };

    union rectangle_attribute{
	void    *empty_rectangle_attribute;
	encode_osd_timer_attr_t     *time_rectangle_attribute;
	encode_osd_string_attr_t    *string_rectangle_attribute;
	encode_osd_mask_attr_t      *mask_rectangle_attribute;
    };

    enum ENCODE_OSD_RECTANGLE_TYPE{
	EMPTY_RECTANGLE_ENCODE_OSD,
	TIMER_RECTANGLE_ENCODE_OSD,
	CHAR_RECTANGLE_ENCODE_OSD,
	MASK_RECTANGLE_ENCODE_OSD,
    };

#define	to_osd_rectangle_entry_with_osd_rectangle_dpram_tcb(node)   container_of(node, osd_rectangle_entry_t, osd_dpram_req)
    struct osd_rectangle_entry{
	isil_chip_t  *chip;
	osd_chan_engine_t   *osd_chan_engine;
	tcb_node_queue_t    mb_char_queue_node;
	tcb_node_queue_t    mb24_char_queue_node;
	int osd_enable;
	int need_update_data;
	int need_update_attr;
	int rectangle_mode;
	int rectangle_id;
	int chan_id;
	int rectangle_left_x, rectangle_left_y, rectangle_right_x, rectangle_right_y;
	rectangle_attribute_t	private_attri;
	osd_attribute_regs_t	*attr_regs;
	unsigned int    osd_data_addr_base;
	osd_rectangle_dpram_tcb_t   osd_dpram_req;

	struct osd_rectangle_operation  *op;
    };

    struct osd_chan_engine_operation{
	int     (*create_char_rectangle)(osd_chan_engine_t *, int);
	int     (*create_mask_rectangle)(osd_chan_engine_t *, int);
#ifdef	TESTING_TIMESTAMP
	int     (*create_timer_rectangle)(osd_chan_engine_t *, unsigned int, int, int, int, int);
	int     (*create_rectangle)(osd_chan_engine_t *, ISIL_OSD_PARAM *, int, int, unsigned int, int, int, int, int);
#else
	int     (*create_timer_rectangle)(osd_chan_engine_t *);
	int     (*create_rectangle)(osd_chan_engine_t *, ISIL_OSD_PARAM *, int, int);
#endif

	void    (*release_char_rectangle)(osd_chan_engine_t *, int);
	void    (*release_mask_rectangle)(osd_chan_engine_t *, int);
	void    (*release_timer_rectangle)(osd_chan_engine_t *);
	void    (*release_rectangle)(osd_chan_engine_t *);

	int     (*close_chan_engine_rectangles)(osd_chan_engine_t *);
	void    (*update_encode_osd_property)(osd_chan_engine_t *, ISIL_OSD_PARAM *);
	void    (*update_fpga_osd_encode)(osd_chan_engine_t *);
	void    (*get_rectangle)(osd_chan_engine_t *, osd_rectangle_entry_t **, int , int);
	void    (*put_rectangle)(osd_chan_engine_t *, osd_rectangle_entry_t *);

	int     (*delete_chan_engine_rectangles)(osd_chan_engine_t *);
        void    (*reset_chan_engine)(osd_chan_engine_t *);

        void    (*submit_osd_data)(osd_chan_engine_t *);
        void    (*submit_osd_attribute)(osd_chan_engine_t *);
    };

    struct osd_attribute_regs_group_operation{
	void    (*init_osd_chan_rectangle_attr_regs_group)(osd_attribute_regs_group_t *);
	void    (*chan_rectangle_attr_regs_set)(osd_attribute_regs_group_t *, int rect_id);
	void    (*chan_rectangle_attr_regs_set_no_display)(osd_attribute_regs_group_t *, int rect_id);
	void    (*chan_rectangle_attr_regs_reset)(osd_attribute_regs_group_t *, int rect_id);
	void    (*update_chan_rectangle_attr_regs)(osd_attribute_regs_group_t *, isil_chip_t *);
    };

#define to_osd_attribute_regs_group_with_osd_dpram_req(node)    container_of(node, osd_attribute_regs_group_t, osd_dpram_req)
    struct osd_attribute_regs_group{
	osd_attribute_regs_t    attribute_regs_group[OSD_CHAN_RECTANGLE_NUMBER];
	u32     osd_chan_attr_reg_base;
	u32     osd_chan_attr_reg_page_id;
	osd_rectangle_dpram_tcb_t   osd_dpram_req;

	struct osd_attribute_regs_group_operation   *chan_rect_attr_regs_group_op;
	struct osd_attribute_regs_operation         *rect_attr_regs_op;
    };

#define	to_osd_chan_engine_with_osd_attribute_regs_group(node)   container_of(node, osd_chan_engine_t, attribute_regs)
    struct osd_chan_engine{
	spinlock_t	osd_lock;
	osd_attribute_regs_group_t attribute_regs;
	osd_mb_pool_t   osd_chan_pool;

	isil_h264_logic_encode_chan_t     *encode_chan;
	ISIL_OSD_PARAM    osd_running_param;
	atomic_t        sync_osd_param;
	atomic_t        osd_working;
        atomic_t        osd_encode_start_close;
        unsigned int    osd_rectangle_need_transfer_data_flag;
	struct completion   osd_transfering_done;
	u32 chan_rectangle_used_map_table;
	u32 chan_data_base;
	int masterOrSub;
	int videoSizeMode;
	osd_rectangle_entry_t       rectangle[OSD_CHAN_RECTANGLE_NUMBER];
	encode_osd_timer_attr_t     timer_attr;
	encode_osd_string_attr_t    string_attr[OSD_CHAN_MAX_STRING_RECTANGLE_NUMBER];
	encode_osd_mask_attr_t      mask_attr[OSD_CHAN_MAX_STRING_RECTANGLE_NUMBER];

	struct osd_chan_engine_operation	*op;
    };

    extern void close_encode_osd_chan_engine(osd_chan_engine_t *);
    extern void remove_encode_osd_chan_engine(osd_chan_engine_t *);
    extern int  init_encode_osd_chan_engine(osd_chan_engine_t *, int);



#ifdef __cplusplus
}
#endif

#endif
