#ifndef _ISIL_CHIP_VO_H__
#define _ISIL_CHIP_VO_H__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct isil_vo_driver    isil_vo_driver_t;
    typedef struct isil_vo_operation isil_vo_operation_t;

    struct isil_vo_operation{
	int (*init)(isil_vo_driver_t *);
	int (*reset)(isil_vo_driver_t *);
	int (*release)(isil_vo_driver_t *);
    };

    struct isil_vo_driver{
	atomic_t    opened_flag;
	ed_tcb_t	opened_vo_ed;

	isil_chip_t  *chip;
	enum CHIP_VI_STANDARD  video_standard;

	isil_vo_operation_t *op;
	isil_proc_register_s vo_proc;
    };

#ifdef __cplusplus
}
#endif

#endif //_ISIL_CHIP_VO_H__

