#ifndef __ISIL_CHIP_DRIVER_H__
#define __ISIL_CHIP_DRIVER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ISIL_CHIP_IOC_MAGIC      'H'


#define ISIL_CHIP_VD_WORK_MODE_SET        		_IOW(ISIL_CHIP_IOC_MAGIC, 255, unsigned int)
#define ISIL_CHIP_VD_WORK_MODE_GET        		_IOR(ISIL_CHIP_IOC_MAGIC, 254, unsigned int)
    //#define	ISIL_CHIP_GET_WORK_MODE			        _IOR(ISIL_CHIP_IOC_MAGIC, 255, unsigned int)
    //#define	ISIL_CHIP_SET_WORK_MODE			        _IOW(ISIL_CHIP_IOC_MAGIC, 254, unsigned int)
#define ISIL_VIDEO_BUS_PARAM_SET                  _IOR(ISIL_CHIP_IOC_MAGIC, 253, unsigned int)
#define ISIL_VIDEO_BUS_PARAM_GET                  _IOR(ISIL_CHIP_IOC_MAGIC, 252, unsigned int)
#define ISIL_CHIP_VD_CONFIG_PARAM_SET             _IOR(ISIL_CHIP_IOC_MAGIC, 251, unsigned int)
#define ISIL_CHIP_VD_CONFIG_PARAM_GET             _IOR(ISIL_CHIP_IOC_MAGIC, 250, unsigned int)
#define	ISIL_CHIP_AUDIO_ENCODE_PARAM_SET	        _IOR(ISIL_CHIP_IOC_MAGIC, 249, unsigned int)
#define	ISIL_CHIP_AUDIO_ENCODE_PARAM_GET	        _IOW(ISIL_CHIP_IOC_MAGIC, 248, unsigned int)
#define	ISIL_CHIP_AUDIO_DECODE_PARAM_SET	        _IOR(ISIL_CHIP_IOC_MAGIC, 247, unsigned int)
#define	ISIL_CHIP_AUDIO_DECODE_PARAM_GET	        _IOW(ISIL_CHIP_IOC_MAGIC, 246, unsigned int)
#define ISIL_CHIP_CREATE_CHAN                     _IOW(ISIL_CHIP_IOC_MAGIC, 245, unsigned int)                            
#define ISIL_CHIP_RELEASE_CHAN                    _IOW(ISIL_CHIP_IOC_MAGIC, 244, unsigned int)
#define ISIL_CHIP_ID_GET                          _IOR(ISIL_CHIP_IOC_MAGIC, 243, unsigned int)

#define ISIL_CHIP_GET_VERSION                     _IOR(ISIL_CHIP_IOC_MAGIC, 240, unsigned int)

/* Testing I/O */
#define ISIL_CHIP_REG_READ                              _IOWR(ISIL_CHIP_IOC_MAGIC, 239, struct reg_info)
#define ISIL_CHIP_REG_WRITE                             _IOW(ISIL_CHIP_IOC_MAGIC, 238, struct reg_info)
#define ISIL_CHIP_MPB_READ                              _IOWR(ISIL_CHIP_IOC_MAGIC, 237, struct mpb_info)
#define ISIL_CHIP_MPB_WRITE                             _IOW(ISIL_CHIP_IOC_MAGIC, 236, struct mpb_info)
#define ISIL_CHIP_I2C_READ                              _IOWR(ISIL_CHIP_IOC_MAGIC, 235, struct i2c_info)
#define ISIL_CHIP_I2C_WRITE                             _IOW(ISIL_CHIP_IOC_MAGIC, 234, struct i2c_info)
#define ISIL_CHIP_DDR_READ                              _IOWR(ISIL_CHIP_IOC_MAGIC, 233, struct ddr_info)
#define ISIL_CHIP_DDR_WRITE                             _IOW(ISIL_CHIP_IOC_MAGIC, 232, struct ddr_info)
#define ISIL_CHIP_GPIO_INPUT                            _IOWR(ISIL_CHIP_IOC_MAGIC, 231, struct gpio_info)
#define ISIL_CHIP_GPIO_OUTPUT                           _IOW(ISIL_CHIP_IOC_MAGIC, 230, struct gpio_info)

struct reg_info {
        unsigned long startaddr;
        unsigned long *buffer;
        unsigned long count;            /* in double word  */
};
                    
struct mpb_info {       
        unsigned long startaddr;
        unsigned char *buffer;
        unsigned long count;            /* in byte */
};  

struct i2c_info {
        unsigned long busaddr;
        unsigned long offset;
        unsigned char *buffer;
        unsigned long count;            /* in byte */
};  

struct ddr_info {
        unsigned long startaddr;
        unsigned long *buffer;          /* pattern */
        unsigned long count;            /* in double word */
};

struct gpio_info {
        unsigned long num;		/* gpio number */
        unsigned char value;
};

    /*increase*/
#define ISIL_CHIP_CTL_OP                          _IOW(ISIL_CHIP_IOC_MAGIC,  200, int)
#define ISIL_CHIP_CONFIG_WRITE_REG                _IOW(ISIL_CHIP_IOC_MAGIC,  201, int)
#define ISIL_CHIP_CONFIG_READ_REG                 _IOR(ISIL_CHIP_IOC_MAGIC,  202, int)
#define ISIL_CHIP_VIN_CHAN_NUMBER_GET             _IOR(ISIL_CHIP_IOC_MAGIC,  203, int)
#define ISIL_CHIP_VOUT_CHAN_NUMBER_GET            _IOR(ISIL_CHIP_IOC_MAGIC,  204, int)
#define ISIL_CHIP_AIN_CHAN_NUMBER_GET             _IOR(ISIL_CHIP_IOC_MAGIC,  205, int)
#define ISIL_CHIP_AOUT_CHAN_NUMBER_GET            _IOR(ISIL_CHIP_IOC_MAGIC,  206, int)
#define ISIL_CHIP_LOGIC_MAP_TABLE_SET             _IOW(ISIL_CHIP_IOC_MAGIC,  207, int)
#define ISIL_CHIP_LOGIC_MAP_TABLE_GET             _IOW(ISIL_CHIP_IOC_MAGIC,  208, int)
#define ISIL_CHIP_VD_CHNL_CONFIG_GET              _IOR(ISIL_CHIP_IOC_MAGIC,  209, unsigned int)
#define ISIL_CHIP_VD_CHNL_CONFIG_SET              _IOR(ISIL_CHIP_IOC_MAGIC,  210, unsigned int)
#define ISIL_CHIP_VD_MAP_PARAM_GET                _IOR(ISIL_CHIP_IOC_MAGIC,  215, unsigned int)
#define ISIL_CHIP_VD_MAP_PARAM_SET                _IOR(ISIL_CHIP_IOC_MAGIC,  216, unsigned int)
#define ISIL_CHIP_VP_MAP_PARAM_GET                _IOR(ISIL_CHIP_IOC_MAGIC,  217, unsigned int)
#define ISIL_CHIP_VP_MAP_PARAM_SET                _IOR(ISIL_CHIP_IOC_MAGIC,  218, unsigned int)


#define ISIL_VIDEO_BUS_PARAM_ENABLE_CHANGE_VIDEO_STANDARD_MASK        (0x00000001)

#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_BIT_WIDE_MASK      (0x00000001)
#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_SAMPLE_RATE_MASK   (0x00000002)
#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_ENCODE_TYPE_MASK   (0x00000004)

#define ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_BIT_WIDE_MASK      (0x00000001)
#define ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_SAMPLE_RATE_MASK   (0x00000002)
#define ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_ENCODE_TYPE_MASK   (0x00000004)


#define	to_get_chip_driver_with_opened_logic_chan_ed(node)	container_of(node, chip_driver_t, opened_control_ed)
    /*
#ifndef NEW_HEADER
enum ISIL_AUDIO_SAMPLE_RATE{
ISIL_AUDIO_8000 = 0,
ISIL_AUDIO_16000,
ISIL_AUDIO_32000,
ISIL_AUDIO_44100,
ISIL_AUDIO_48000,
ISIL_AUDIO_RESERVED,
};

enum ISIL_AUDIO_BIT{
ISIL_AUDIO_16BIT = 0,
ISIL_AUDIO_8BIT,
};
#endif
*/

#define MAX_MESSAGE_SIZE    (64)
#define MAX_MESSAGE_NUMBER  (128)

typedef struct isil_chip_video_param   isil_chip_video_param_t;
typedef struct video_bus_operation   video_bus_operation_t;
typedef struct isil_video_bus          isil_video_bus_t;
typedef struct audio_bus_operation   audio_bus_operation_t;
typedef struct isil_audio_bus          isil_audio_bus_t;
typedef struct chip_driver_operation chip_driver_operation_t;
typedef struct chip_driver           chip_driver_t;
typedef struct isil_logic_map_table    isil_logic_map_table_t;
typedef struct isil_logic_phy_table    isil_logic_phy_table_t; 
typedef struct isil_logic_phy_operation isil_logic_phy_operation_t;
typedef struct isil_vd_map_param  isil_vd_config_param_t;
typedef isil_vd_config_param_t    isil_vp_config_param_t;

typedef struct isil_msg_tcb     isil_msg_tcb_t;
typedef struct isil_msg_queue   isil_msg_queue_t;
typedef struct isil_msg_pool    isil_msg_pool_t;
/*
#ifndef NEW_HEADER

enum ISIL_AUDIO_TYPE{
ISIL_AUDIO_PCM = 0,
ISIL_AUDIO_ALAW,
ISIL_AUDIO_ULAW,
ISIL_AUDIO_ADPCM_32K,
ISIL_AUDIO_ADPCM_16K,
ISIL_AUDIO_ADPCM_48K
};
#endif
*/
enum ISIL_VIDEO_MD_TRIGGER_MODE{
    VIDEO_MD_TRIGGER_AUTO = 0,
    VIDEO_MD_TRIGGER_USER,
};

enum ISIL_VIDEO_MD_SEL{
    ISIL_VIDEO_MD_ODD = 0,
    ISIL_VIDEO_MD_EVEN,
    ISIL_VIDEO_MD_BOTH,
};

enum ISIL_VIDEO_MD_REF{
    ISIL_VIDEO_MD_REF_AUTO,
    ISIL_VIDEO_MD_REF_USER,  /*0 ~ 63*/
};
/*
typedef enum{
    ISIL_H264_ENCODE_FRAME_MSG = 0x00000010,
    ISIL_H264_ENCODE_FRAME_TYPE,
    ISIL_H264_DECODE_FRAME_MSG = 0x00010010,
    ISIL_H264_DECDOE_BIT_INFO_MSG,
    ISIL_H264_MVFLAG_FRAME_MSG = 0x00020010,
    ISIL_H264_MVD_FRAME_MSG,
    ISIL_MJPEG_ENCODE_FRAME_MSG = 0x00030010,
    ISIL_IMAGE_ENCODE_FRAME_MSG = 0x00040010,
    ISIL_AUDIO_ENCODE_FRAME_MSG = 0x00050010,
    ISIL_AUDIO_ENCODE_PARAM_MSG,
    ISIL_AUDIO_DECODE_FRAME_MSG,
    ISIL_AUDIO_DECODE_PARAM_MSG,
    ISIL_VIDEO_LOST_DETECTION_MSG = 0x00060010,
    ISIL_VIDEO_NIGHT_DETECTION_MSG,
    ISIL_VIDEO_BLIND_DETECTION_MSG,
    ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG,

    ISIL_INFO_EXT_MSG = 0x00070010,
}ISIL_MSG_TYPE_E;
*/
typedef struct{
    u32 msg_type;
    u32 msg_len;
    u16 chan_id;
    u8  video_lost_valid;
    u8  video_connect_valid;
}isil_video_lost_detection_msg_t;

typedef struct{
    u32 msg_type;
    u32 msg_len;
    u16 chan_id;
    u8  video_night_valid_from_day_to_night;
    u8  video_night_valid_from_night_to_day;
}isil_video_night_detection_msg_t;

typedef struct{
    u32 msg_type;
    u32 msg_len;
    u16 chan_id;
    u8  video_blind_add_valid;
    u8  video_blind_remove_valid;
}isil_video_blind_detection_msg_t;

typedef struct{
    u32 msg_type;
    u32 msg_len;
    u16 chan_id;
    u8  latest_video_standard;
    u8  curr_video_standard;
}isil_video_standard_change_detection_msg_t;

struct isil_msg_tcb_operation{
    int (*init)(isil_msg_tcb_t *);
    int (*reset)(isil_msg_tcb_t *);
    int (*release)(isil_msg_tcb_t *);

    size_t (*submit)(isil_msg_tcb_t *, char __user *, size_t , loff_t *);
};

#define to_get_msg_tcb_with_node(node) container_of(node, isil_msg_tcb_t, node)
struct isil_msg_tcb{
    tcb_node_t node;
    u32 msg_id;
    u32 msg_type;
    u32 msg_len;

    void *msg_data;

    struct isil_msg_tcb_operation *op;
};

struct isil_msg_queue_operation{
    int (*init)(isil_msg_queue_t *);
    int (*reset)(isil_msg_queue_t *);
    int (*release)(isil_msg_queue_t *);

    int (*get)(isil_msg_queue_t *, isil_msg_tcb_t **);
    int (*try_get)(isil_msg_queue_t *, isil_msg_tcb_t **);
    int (*put)(isil_msg_queue_t *, isil_msg_tcb_t *);
    int (*put_header)(isil_msg_queue_t *, isil_msg_tcb_t *);

    int (*get_curr_producer_from_pool)(isil_msg_queue_t *, isil_msg_pool_t *);
    int (*try_get_curr_producer_from_pool)(isil_msg_queue_t *, isil_msg_pool_t *);
    int (*put_curr_producer_into_queue)(isil_msg_queue_t *);
    int (*release_curr_producer)(isil_msg_queue_t *, isil_msg_pool_t *);
    int (*get_curr_consumer_from_queue)(isil_msg_queue_t *);
    int (*try_get_curr_consumer_from_queue)(isil_msg_queue_t *);
    int (*release_curr_consumer)(isil_msg_queue_t *, isil_msg_pool_t *);

    int (*get_curr_queue_entry_number)(isil_msg_queue_t *);
};

struct isil_msg_queue{
    tcb_node_queue_t queue;

    isil_msg_tcb_t	*curr_consumer;
    isil_msg_tcb_t	*curr_producer;
    spinlock_t	lock;

    struct isil_msg_queue_operation *op;
};

struct isil_msg_pool_operation{
    int (*init)(isil_msg_pool_t *);
    int (*reset)(isil_msg_pool_t *);
    int (*release)(isil_msg_pool_t *);

    int (*get_msg_tcb)(isil_msg_pool_t *, isil_msg_tcb_t **);
    int (*try_get_msg_tcb)(isil_msg_pool_t *, isil_msg_tcb_t **);
    int (*put_msg_tcb)(isil_msg_pool_t *, isil_msg_tcb_t *);
    int (*get_msg_tcb_pool_entry_number)(isil_msg_pool_t *);
};

struct isil_msg_pool{
    tcb_node_pool_t pool;
    u32 msg_entry_number;
    u32 cache_order;

    isil_msg_tcb_t *msg_tcb;

    struct isil_msg_pool_operation *op;
};

struct isil_reg_conf{
    u32 addr;
    u32 val;
};

struct isil_video_md_param{

};

struct isil_chip_vi_info{
    u32 i_vin_chan_nubmer;
    u32 i_vin_ad_chan_number;
    u32 i_cascade_vin_ad_chan_number;
};
struct isil_chip_vo_info{
    u32 i_vout_chan_nubmer;
    u32 i_vout_ad_chan_number;
};
struct isil_chip_ai_info{
    u32 i_ain_chan_nubmer;
    u32 i_ain_ad_chan_number;
    u32 i_cascade_ain_ad_chan_number;
};
struct isil_chip_ao_info{
    u32 i_aout_chan_nubmer;
    u32 i_aout_ad_chan_number;
};

struct isil_chip_id_info{
    u32 i_vendor_id;
    u32 i_version_id;
};

struct isil_chip_audio_param{
    u32 change_mask_flag;
    u32 channel;
    u8  i_bit_wide;
    u8  i_sample_rate;
    u16 e_audio_type;
};

struct isil_chip_h264d_bind_info{
    u32 i_h264d_logic_id;
    u32 i_display_phy_id;
};

struct isil_chip_vi2vo_bind_info{
    u32 i_vi_phy_id;
    u32 i_display_phy_id;
};

enum LOGIC_MAP_TABLE_TYPE{
    LOGIC_MAP_TABLE_BIND_H264E = 0, /*bind vi to h264e*/
    LOGIC_MAP_TABLE_BIND_H264D,	/*bind vo to h264d*/
    LOGIC_MAP_TABLE_BIND_AENC,	/*bind ai to aenc*/
    LOGIC_MAP_TABLE_BIND_ADEC,	/*bind ao to adec*/
    LOGIC_MAP_TABLE_BIND_JPEGE,	/*bind vi to jpege*/
    LOGIC_MAP_TABLE_BIND_RESERVED,
};

struct isil_logic_map_table{
    u32 i_chan_number;   /*map channel number*/
    enum LOGIC_MAP_TABLE_TYPE e_bind_type;

    struct {
#if defined(__BIG_ENDIAN_BITFIELD)
	u32 logic_slot_id:16;
	u32 phy_slot_id:16;
#elif defined(__LITTLE_ENDIAN_BITFIELD)
	u32 phy_slot_id:16;
	u32 logic_slot_id:16;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }map_table[0];
};

struct isil_logic_phy_operation{
    int (*init)(isil_logic_phy_table_t *);
    int (*reset)(isil_logic_phy_table_t *);
    int (*release)(isil_logic_phy_table_t *);

    int (*find_logic_by_phy)(isil_logic_phy_table_t *, enum LOGIC_MAP_TABLE_TYPE, u32, u32 *);
    int (*find_phy_by_logic)(isil_logic_phy_table_t *, enum LOGIC_MAP_TABLE_TYPE, u32, u32 *);
    int (*update_map_table)(isil_logic_phy_table_t *, enum LOGIC_MAP_TABLE_TYPE, isil_logic_map_table_t *);
    int (*get_map_table)(isil_logic_phy_table_t *, enum LOGIC_MAP_TABLE_TYPE, isil_logic_map_table_t *);
};

struct isil_logic_phy_table{
    spinlock_t lock;
    isil_logic_map_table_t *logic_talbe[LOGIC_MAP_TABLE_BIND_RESERVED];

    isil_logic_phy_operation_t *op;
};

struct isil_chip_video_param{
    u32    change_mask_flag;
    enum ISIL_VIDEO_STANDARD e_video_standard;
};

struct isil_vd_chan_param{
    u32 change_mask_flag;
    u8  i_phy_chan_id;
    u8  i_phy_chan_enable;
    u8  i_phy_chan_fps;
    u8  i_interlace;
    u32 hor_reverse;    /*Horizontal reverse*/
    u32 ver_reverse;    /*Vertical   reverse*/
    u16 i_phy_video_width_mb_size;
    u16 i_phy_video_height_mb_size;
};

struct isil_chip_vd_param{
    u32 i_vd_chan_number;
    struct isil_vd_chan_param vd_chan_param[ISIL_PHY_VD_CHAN_NUMBER];
};

struct isil_vd_map_param{
    vd_chan_map_info_t map_info[ISIL_PHY_VD_CHAN_NUMBER];
};

struct video_bus_operation{
    int  (*init)(isil_video_bus_t *, isil_chip_t *);
    void (*release)(isil_video_bus_t *);
    int  (*reset)(isil_video_bus_t *);

    int  (*get_video_standard)(isil_video_bus_t *);
    int  (*set_video_standard)(isil_video_bus_t *, int);
    int  (*notify_logic_chan_change)(isil_video_bus_t *);
    int  (*notify_suspend_logic_chan)(isil_video_bus_t *);
    int  (*notify_resume_logic_chan)(isil_video_bus_t *);
};

struct isil_video_bus{
    isil_chip_video_param_t    video_param;

    isil_vd_cross_bus_t   *vd_bus_driver;
    isil_vj_bus_t         *vj_bus_driver;
    isil_vp_bus_t         *vp_bus_driver;

    video_bus_operation_t   *op;
};

struct audio_bus_operation{
    int  (*init)(isil_audio_bus_t *, isil_chip_t *);
    void (*release)(isil_audio_bus_t *);
    int (*reset)(isil_audio_bus_t *);
};

struct isil_audio_bus{
    chip_audio_t            *chip_audio_driver;

    audio_bus_operation_t   *op;
};

struct chip_driver_operation{
    int  (*init)(chip_driver_t *);
    void (*release)(chip_driver_t *);
    int  (*reset)(chip_driver_t *);
};

struct chip_driver{
    atomic_t    opened_flag;
    ed_tcb_t	opened_control_ed;

    isil_chip_t     *chip;
    isil_video_bus_t video_bus;
    isil_audio_bus_t audio_bus;
    isil_logic_phy_table_t logic_map_phy;
    isil_msg_pool_t  msg_pool;

    chip_driver_operation_t     *op;
};


int isil_chip_driver_init(void);
void isil_chip_driver_remove(void);
int init_isil_msg_queue(isil_msg_queue_t *);
int remove_isil_msg_queue(isil_msg_queue_t *);
//int init_chip_driver(struct isil_chip_device * tcd, struct isil_chip_driver * drv);
//void remove_chip_driver(struct isil_chip_device * tcd, struct isil_chip_driver * drv);

#ifdef __cplusplus
}
#endif

#endif //__ISIL_CHIP_DRIVER_H__

