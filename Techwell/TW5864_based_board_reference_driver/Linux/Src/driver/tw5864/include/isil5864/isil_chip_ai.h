#ifndef _ISIL_CHIP_AI_H__
#define _ISIL_CHIP_AI_H__

#ifdef __cplusplus
extern "C"
{
#endif

    struct isil_service_device;

#define ISIL_DRIVER_AI_IOC_MAGIC      'H'

#define ISIL_AI_SET_AUDIO_SYS_PARM		    _IOW(ISIL_DRIVER_AI_IOC_MAGIC,  49, int)
#define ISIL_AI_GET_AUDIO_SYS_PARM		    _IOR(ISIL_DRIVER_AI_IOC_MAGIC,  48, int)
#define ISIL_AI_GET_AUDIO_CHN_PARM            _IOR(ISIL_DRIVER_AI_IOC_MAGIC,  47, int)
#define ISIL_AI_SET_AUDIO_GAIN			    _IOW(ISIL_DRIVER_AI_IOC_MAGIC,  02, int)
#define ISIL_AI_MAX_CHNL      (4)

#define ISIL_AI_CLOCK_MODE    (0x06c)
#define ISIL_AI_SYNC_MODE     (0x062)
#define ISIL_AI_BITS          (0x06c)
#define ISIL_AI_SAMPLE_RATE   (0x075)

#define ISIL_AI_GAIN_21       (0x060)
#define ISIL_AI_GAIN_43       (0x061)
#define MAX_AI_GAIN         (15)
#define DEFAULT_AI_GAIN     (8)
#define MIN_AI_GAIN         (0)

#define	to_get_chip_ai_driver_with_opened_logic_chan_ed(node)	container_of(node, isil_chip_ai_driver_t, opened_ai_ed)

    enum AUDIO_SYNC_MODE{
	AUDIO_SYNC_MODE_I2S = 0,
	AUDIO_SYNC_MODE_DSP,
    };

    enum AUDIO_CLOCK_MODE{
	AUDIO_CLOCK_SLAVE = 0,
	AUDIO_CLOCK_MASTER,/*default*/
    };

    typedef struct isil_chip_ai_driver          isil_chip_ai_driver_t;
    typedef struct isil_chip_ai_operation       isil_chip_ai_operation_t;
    typedef struct isil_chip_ai_property        isil_chip_ai_property_t;
    typedef struct isil_chip_ai_property_operation isil_chip_ai_property_operation_t;
    typedef struct isil_chip_ai_config_property isil_chip_ai_config_property_t;

    struct isil_chip_ai_sys{
	u32 changemask;
#define ISIL_CHIP_AI_CHANGEMASK_CLOCK_MODE    (0x0001)
#define ISIL_CHIP_AI_CHANGEMASK_SYNC_MODE     (0x0002)
#define ISIL_CHIP_AI_CHANGEMASK_SAMPLE_RATE   (0x0004)
#define ISIL_CHIP_AI_CHANGEMASK_BITS          (0x0008)
#define ISIL_CHIP_AI_CHANGEMASK_MASKALL       (0x000f)

	enum AUDIO_CLOCK_MODE clock_mode;
	enum AUDIO_SYNC_MODE  sync_mode;
	enum ISIL_AUDIO_SAMPLE_RATE sample_rate;
	enum ISIL_AUDIO_BIT     bits;
    };

    typedef struct
    {
	unsigned int u32ChipID;//chip id
	unsigned int u32Channel;//channel id
	unsigned int u32Volume;//volume value
    } AI_CH_VOLUME;

    struct isil_chip_ai_property{
	u32 channel;
	u32 changemask;
#define ISIL_CHIP_AI_CHANGEMASK_GAIN (0x0001)
	u32 gain;
    };

    struct isil_chip_ai_property_operation{
	int (*init)(isil_chip_ai_property_t *, u32);
	int (*reset)(isil_chip_ai_property_t *);

	u32 (*get_gain)(isil_chip_ai_property_t *);
	int (*set_gain)(isil_chip_ai_property_t *, u32);
	int (*update)(isil_chip_ai_property_t *, isil_chip_t *chip);
    };

    struct isil_chip_ai_config_property{
	isil_chip_ai_property_t audio_property;
	isil_chip_ai_property_operation_t *op;
    };

    struct isil_chip_ai_operation{
	int (*init)(isil_chip_ai_driver_t *);
	void (*release)(isil_chip_ai_driver_t *);
	int (*reset)(isil_chip_ai_driver_t *);

	int (*get_clock_mode)(isil_chip_ai_driver_t *);
	int (*set_clock_mode)(isil_chip_ai_driver_t *, enum AUDIO_CLOCK_MODE);
	int (*get_sync_mode)(isil_chip_ai_driver_t *);
	int (*set_sync_mode)(isil_chip_ai_driver_t *, enum AUDIO_SYNC_MODE);
	int (*get_sample_rate)(isil_chip_ai_driver_t *);
	int (*set_sample_rate)(isil_chip_ai_driver_t *, enum ISIL_AUDIO_SAMPLE_RATE);
	int (*get_bits)(isil_chip_ai_driver_t *);
	int (*set_bits)(isil_chip_ai_driver_t *, enum ISIL_AUDIO_BIT);
    };

    struct isil_chip_ai_driver{
	atomic_t    opened_flag;
	ed_tcb_t	opened_ai_ed;

	isil_chip_t  *chip;

	enum AUDIO_CLOCK_MODE clock_mode;
	enum AUDIO_SYNC_MODE  sync_mode;
	enum ISIL_AUDIO_SAMPLE_RATE sample_rate;
	enum ISIL_AUDIO_BIT     bits;

	isil_chip_ai_config_property_t audio_property[ISIL_AI_MAX_CHNL];

	isil_chip_ai_operation_t *op;
    };

    extern int init_chip_ai_driver(struct isil_service_device *tsd,isil_chip_t *chip);
    extern int remove_chip_ai_driver(struct isil_service_device *tsd);
    extern int isil_chip_ai_driver_init(void);
    extern void isil_chip_ai_driver_remove(void);

#ifdef __cplusplus
}
#endif

#endif //_ISIL_CHIP_AI_H__

