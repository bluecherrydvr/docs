#ifndef _ISIL_CHIP_VI_H__
#define _ISIL_CHIP_VI_H__

#ifdef __cplusplus
extern "C"
{
#endif


    struct isil_service_device;


#define ISIL_DRIVER_VI_IOC_MAGIC      'H'

#define ISIL_VI_SET_VIDEO_AD_PARM		_IOW(ISIL_DRIVER_VI_IOC_MAGIC,  51, int)
#define ISIL_VI_GET_VIDEO_AD_PARM		_IOR(ISIL_DRIVER_VI_IOC_MAGIC,  52, int)
#define ISIL_VI_GET_FEATURE           _IOR(ISIL_DRIVER_VI_IOC_MAGIC,  147, int)
#define ISIL_VI_SET_FEATURE           _IOW(ISIL_DRIVER_VI_IOC_MAGIC,  146, int)
#define ISIL_VI_GET_VIDEO_STANDARD    _IOR(ISIL_DRIVER_VI_IOC_MAGIC,  145, int)
    //#define ISIL_VI_SET_VIDEO_STANDARD    _IOW(ISIL_DRIVER_VI_IOC_MAGIC,  144, int)
#define ISIL_VI_SET_VIDEO_TYPE        _IOW(ISIL_DRIVER_VI_IOC_MAGIC,  55, int)
#define ISIL_VI_VIDEO_LOST_CMD        _IOW(ISIL_DRIVER_VI_IOC_MAGIC, 148, int)

#define ISIL_VI_MAX_CHNL      (16)
#define ISIL_VI_ON_CHIP_CHNL  (4)  /*analog vi number of on chip*/

#define ISIL_VI_HDELAY_XY         (0x002)
#define ISIL_VI_HACTIVE_XY        (0x003)
#define ISIL_VI_VDELAY_XY         (0x004)
#define ISIL_VI_VACTIVE_XY        (0x005)
#define ISIL_VI_HV_XY             (0x006)

#define ISIL_VI_BRIGHNESS         (0x00A)
#define ISIL_VI_CONTRAST          (0x009)
#define ISIL_VI_SATURATION_U      (0x00B)
#define ISIL_VI_SATURATION_V      (0x00C)
#define ISIL_VI_HUE               (0x007)
#define ISIL_VI_SHARPNESS         (0x008)

#define ISIL_VI_STANDARD          (0x00e)

#define ISIL_VI_VD_EDGE           (0x041)
#define ISIL_VI_AGCGGAIN          (0x046)
#define ISIL_VI_BGNDEN            (0x0c0)

    /*system configure*/
#define ISIL_VI_SYSTEM_RESET          (0xef0)
#define ISIL_VI_SYSTEM_CLOCK          (0xefc)
#define ISIL_VI_SYSTEM_CLOCK_REVERSE  (0xefd) /*reverse preview & VD input clock polarity*/
#define ISIL_VI_SYSTEM_VIDEO_STANDARD (0x260) /*video norm*/
#define ISIL_VI_SYSTEM_IN_WIDTH(n)    (0x200 + (n<<2)) /*input width, 720/4*/
#define ISIL_VI_SYSTEM_IN_HEIGHT(n)   (0x201 + (n<<2)) /*input height, 288/4*/
#define ISIL_VI_SYSTEM_OUT_WIDTH(n)   (0x202 + (n<<2)) /*output width, 360/4*/
#define ISIL_VI_SYSTEM_OUT_HEIGHT(n)  (0x203 + (n<<2)) /*output height 144/4*/

    /*event control*/
#define ISIL_VI_EVENT_CONTROL(n)      (0x300 + (n<<3))
#define ISIL_VI_EVENT_MD_TMPSENS(n)   (0x301 + (n<<3)) /*MD_TMPSENS & MD_PIXEL_OS*/
#define ISIL_VI_EVENT_MD_FIELD(n)     (0x302 + (n<<3)) /*MD_REFFLD & MD_FIELD & MD_LVSENS*/
#define ISIL_VI_EVENT_MD_CELL(n)      (0x303 + (n<<3)) /*MD_CELSENS & MD_SPEED*/
#define ISIL_VI_EVENT_MD_SPSENS(n)    (0x304 + (n<<3)) /*MD_SPSENS & BD_LVSENS*/
#define ISIL_VI_EVENT_ND_TMPSENS(n)   (0x305 + (n<<3)) /*ND_TMPSENS & ND_LVSENS*/

    /*event state*/
#define ISIL_VI_EVENT_MOTION_L      (0x2d0)
#define ISIL_VI_EVENT_MOTION_H      (0x2d1)
#define ISIL_VI_EVENT_NIGHT_L       (0x2d2)
#define ISIL_VI_EVENT_NIGHT_H       (0x2d3)
#define ISIL_VI_EVENT_BLIND_L       (0x2d4)
#define ISIL_VI_EVENT_BLIND_H       (0x2d5)
#define ISIL_VI_EVENT_NOVIDEO_L     (0x2d6)
#define ISIL_VI_EVENT_NOVIDEO_H     (0x2d7)
#define ISIL_VI_EVENT_UNDERFLOW_L   (0x2e0)
#define ISIL_VI_EVENT_UNDERFLOW_H   (0x2e1)
#define ISIL_VI_EVENT_OVERFLOW_L    (0x2e2)
#define ISIL_VI_EVENT_OVERFLOW_H    (0x2e3)
    /*event mask*/
#define ISIL_VI_EVENT_MASK_MOTION_L      (0x2d8)
#define ISIL_VI_EVENT_MASK_MOTION_H      (0x2d9)
#define ISIL_VI_EVENT_MASK_NIGHT_L       (0x2da)
#define ISIL_VI_EVENT_MASK_NIGHT_H       (0x2db)
#define ISIL_VI_EVENT_MASK_BLIND_L       (0x2dc)
#define ISIL_VI_EVENT_MASK_BLIND_H       (0x2dd)
#define ISIL_VI_EVENT_MASK_NOVIDEO_L     (0x2de)
#define ISIL_VI_EVENT_MASK_NOVIDEO_H     (0x2df)
#define ISIL_VI_EVENT_MASK_UNDERFLOW_L   (0x2e8)
#define ISIL_VI_EVENT_MASK_UNDERFLOW_H   (0x2e9)
#define ISIL_VI_EVENT_MASK_OVERFLOW_L    (0x2ea)
#define ISIL_VI_EVENT_MASK_OVERFLOW_H    (0x2eb)
    /*event summary*/
#define ISIL_VI_EVENT_SUMMARY_L     (0x2f0)
#define ISIL_VI_EVENT_SUMMARY_H     (0x2f1)

#define DDRA_DLL_DQS_SEL0              (0xee6)
#define DDRA_DLL_DQS_SEL1              (0xee7)
#define DDRA_DLL_CLK90_SEL             (0xee8)
#define DDRB_DLL_DQS_SEL0              (0xeeb)
#define DDRB_DLL_DQS_SEL1              (0xeec)
#define DDRB_DLL_CLK90_SEL             (0xeed)

#define PV0_LM_CHID0                   (0x240)
#define PV1_LM_CHID0                   (0x250)

#define PV_CLK_DLY10                   (0xef8)
#define PV_CLK_DLY32                   (0xef9)
#define PV_SEL_10                      (0xefa)
#define PV_SEL_32                      (0xefb)
#define PV_CLK_OUT                     (0xefe)

#define RGR_MOTION_SEL                 (0x382)
#define NOVID_DETL                     (0x388)
#define NOVID_DETH                     (0x389)
#define MD_DETL                        (0x38a)
#define MD_DETH                        (0x38b)
#define BD_DETL                        (0x38c)
#define BD_DETH                        (0x38d)
#define ND_DETL                        (0x38e)
#define ND_DETH                        (0x38f)
#define MD_DI_CNTL                     (0x3b8)
#define MD_DI_CNTH                     (0x3b9)
#define MD_DI_CELLSENS                 (0x3ba)
#define MD_DI_LVSENS                   (0x3bb)

#define CHIP_VI_EVENT_MIN_SENSITIVE (0)         /*high sensitive*/
#define CHIP_VI_EVENT_MAX_SENSITIVE (15)        /*low sensitive*/

    enum CHIP_VI_STANDARD {
	CHIP_VI_STANDARD_NTSC_M,    /*NTSC(M)*/
	CHIP_VI_STANDARD_PAL_BDGHI, /*PAL (B,D,G,H,I)*/
	CHIP_VI_STANDARD_SECAM,     /*SECAM*/
	CHIP_VI_STANDARD_NTSC443,   /*NTSC 4.43*/
	CHIP_VI_STANDARD_PAL_M,     /*PAL(M)*/
	CHIP_VI_STANDARD_PAL_CN,    /*PAL(CN)*/
	CHIP_VI_STANDARD_PAL_60,    /*PAL 60*/
	CHIP_VI_STANDARD_INVALID,   /*invalid input*/
	CHIP_VI_STANDARD_AUTO,      /*auto detection*/
    };

    enum CHIP_VI_EVENT{
	CHIP_VI_EVENT_MOTION = 0, /*motion*/
	CHIP_VI_EVENT_NIGHT,      /*night*/
	CHIP_VI_EVENT_BLIND,      /*blind*/
	CHIP_VI_EVENT_LOST,       /*video lost*/
	CHIP_VI_EVENT_UNDERFLOW,  /*Line mode underflow*/
	CHIP_VI_EVENT_OVERFLOW,   /*Line mode overflow*/
	CHIP_VI_EVENT_RESERVED,
    };

    enum MOTION_TRIGGER_MODE{
	MOTION_TRIGGER_AUTIO = 0,   /*auto detection*/
	MOTION_TRIGGER_USER,    /*only detection once, */
	MOTION_TRIGGER_RESERVED,
    };

    typedef struct isil_chip_vi_feature_ext        isil_chip_vi_feature_ext_t; 
    typedef struct isil_chip_vi_feature            isil_chip_vi_feature_t; 
    typedef struct isil_chip_vi_property_operation isil_chip_vi_property_operation_t;
    typedef struct isil_chip_vi_config_property    isil_chip_vi_config_property_t;
    typedef struct isil_chip_vi_property  isil_chip_vi_property_t;
    typedef struct isil_chip_vi_operation isil_chip_vi_operation_t;
    typedef struct isil_vi_event_action   isil_vi_event_action_t;
    typedef struct isil_chip_vi_event     isil_chip_vi_event_t;
    typedef struct isil_chip_vi_driver    isil_chip_vi_driver_t;
    typedef int  (*event_handler)(u32, void *);

#define	to_get_chip_vi_driver_with_opened_logic_chan_ed(node)	container_of(node, isil_chip_vi_driver_t, opened_vi_ed)

#define MAX_VI_BRIGHNESS        (255)
#define DEFAULT_VI_BRIGHTNESS   (128)
#define MIN_VI_BRIGHNESS        (0)

#define MAX_VI_CONTRAST         (255)
#define DEFAULT_VI_CONTRAST     (100)
#define MIN_VI_CONTRAST         (0)

#define MAX_VI_SATURATION       (255)
#define DEFAULT_VI_SATURATION   (128)
#define MIN_VI_SATURATION       (0)

#define MAX_VI_HUE              (255)
#define DEFAULT_VI_HUE          (128)
#define MIN_VI_HUE              (0)

#define MAX_VI_SHARPNESS        (15)
#define DEFAULT_VI_SHARPNESS    (1)
#define MIN_VI_SHARPNESS        (0)

#define ISIL_VI_FEATURE_ENABLE_CHANGE_MODE_MASK      (0x01)
#define ISIL_VI_FEATURE_ENABLE_CHANGE_MOTION_MASK    (0x02)
#define ISIL_VI_FEATURE_ENABLE_CHANGE_NIGHT_MASK     (0x04)
#define ISIL_VI_FEATURE_ENABLE_CHANGE_BLIND_MASK     (0x08)
#define ISIL_VI_FEATURE_ENABLE_CHANGE_NOVIDEO_MASK   (0x010)
#define ISIL_VI_FEATURE_ENABLE_CHANGE_UNDERFLOW_MASK (0x020)
#define ISIL_VI_FEATURE_ENABLE_CHANGE_OVERFLOW_MASK  (0x040)
#define ISIL_VI_FEATURE_ENABLE_CHANGE_MASK_ALL       (0x07f)

#define ISIL_VI_FEATURE_ON  (1)
#define ISIL_VI_FEATURE_OFF (0)

    struct isil_chip_vi_feature{
	u32 channel;
	u32 change_mask;

	enum MOTION_TRIGGER_MODE e_motion_mode;
	u32 i_motion_sensitive;    /*motion detection*/
	u32 i_night_sensitive;     /*night detection*/
	u32 i_blind_sensitive;     /*blind detection*/

	u8 b_novideo_enable;   /*video lost*/
	u8 b_underflow_enable; /*line mode underflow*/
	u8 b_overflow_enable;  /*line mode overflow*/
	u8 b_reserved[1];    
    };

    struct isil_chip_vi_feature_ext{
        u8 b_video_connect;
        u8 b_video_disconnect;
        u8 b_video_night_add;
        u8 b_video_night_del;
        u8 b_video_blind_add;
        u8 b_video_blind_del;
        u8 b_video_standard_change;
        u8 b_video_underflow;
        u8 b_video_overflow;
    };

    struct isil_chip_vi_property{
	u32 chip_num;
	u32 u32Channel;//physical channel ID
	u32 u32Mask;   //mask bit 

#define MASK_VI_BRIGHTNESS (0x01)
#define MASK_VI_CONTRAST   (0x02)
#define MASK_VI_SATURATION (0x04)
#define MASK_VI_HUE	       (0x08)
#define MASK_VI_SHARPNESS  (0x10)
#define MASK_VI_ALL        (0x1f)

	u32 u32Brightness;
	u32 u32Contrast;
	u32 u32Saturation;
	u32 u32Hue;
	u32 u32Sharpness;  
    };

    typedef struct
    {
	unsigned int u32ChipID;//chip id
	unsigned int u32Channel;//channel id
	enum CHIP_VI_STANDARD eVideoType;//video type
    }VI_CH_VIDEOTYPE;

    struct isil_chip_vi_property_operation{
	int (*init)(isil_chip_vi_property_t *, u32 chn);
	int (*reset)(isil_chip_vi_property_t *);

	u32 (*get_brightness)(isil_chip_vi_property_t *);
	int (*set_brightness)(isil_chip_vi_property_t *, u32);
	u32 (*get_contrast)(isil_chip_vi_property_t *);
	int (*set_contrast)(isil_chip_vi_property_t *, u32);
	u32 (*get_saturation)(isil_chip_vi_property_t *);
	int (*set_saturation)(isil_chip_vi_property_t *, u32);
	u32 (*get_hue)(isil_chip_vi_property_t *);
	int (*set_hue)(isil_chip_vi_property_t *, u32);
	u32 (*get_sharpness)(isil_chip_vi_property_t *);
	int (*set_sharpness)(isil_chip_vi_property_t *, u32);

	int (*update)(isil_chip_vi_driver_t *, isil_chip_vi_config_property_t *);
    };

    struct isil_chip_vi_config_property{
	isil_chip_vi_property_t video_property;
	isil_chip_vi_feature_t  video_feature;
        isil_chip_vi_feature_ext_t video_feature_ext;
	isil_chip_vi_property_operation_t *op;
    };

    struct isil_chip_vi_operation{
	int (*init)(isil_chip_vi_driver_t*);
	int (*reset)(isil_chip_vi_driver_t*);
	int (*release)(isil_chip_vi_driver_t*);

	int (*set_video_standard)(isil_chip_vi_driver_t *, enum CHIP_VI_STANDARD);
	int (*set_video_standard_ext)(isil_chip_vi_driver_t *, enum CHIP_VI_STANDARD);
	int (*get_video_standard)(isil_chip_vi_driver_t *);
	int (*get_hw_video_standard)(isil_chip_vi_driver_t *);
	int (*polling_hw_video_standard)(isil_chip_vi_driver_t *);

	u32 (*get_video_event)(isil_chip_vi_driver_t *);
    };

    struct isil_vi_event_action{
	event_handler   handler;
	u32             event;
	void            *context;
	isil_vi_event_action_t *next;
    };

    struct isil_chip_vi_event{
	enum CHIP_VI_EVENT    event;
	u32                   count;
	u32                   unhandled;
	u32                   event_flags; /*channel event*/
	spinlock_t            lock;
	isil_vi_event_action_t  action;
	const char            *name;
    };

    struct isil_chip_vi_driver{
	atomic_t    opened_flag;
	ed_tcb_t	opened_vi_ed;

	isil_chip_t  *chip;
	enum CHIP_VI_STANDARD video_standard, hw_video_standard, pre_hw_video_standard;

    isil_msg_queue_t msg_queue;
	isil_send_msg_controller_t send_msg_contr;
	u32                event_flags;   /*event type*/
	u32                event_timer;
	isil_chip_vi_event_t driver_event[CHIP_VI_EVENT_RESERVED];
	isil_chip_vi_config_property_t video_property[ISIL_VI_MAX_CHNL];

	isil_chip_vi_operation_t *op;

	isil_proc_register_s vi_proc; /*proc info*/
    };

    int init_chip_vi_driver(struct isil_service_device *tsd, isil_chip_t *chip);
    int remove_chip_vi_driver(struct isil_service_device *tsd);
    int isil_chip_vi_driver_init(void);
    void isil_chip_vi_driver_remove(void);

#ifdef __cplusplus
}
#endif

#endif //_ISIL_CHIP_VI_H__

