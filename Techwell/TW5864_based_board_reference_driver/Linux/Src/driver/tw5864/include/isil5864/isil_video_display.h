#ifndef __ISIL_VIDEO_DISPLAY_H__
#define __ISIL_VIDEO_DISPLAY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_DISPLAY_INTERFACE (4)
#define MAX_DISPLAY_POSITION  (16)

#define DISP_REG_BASE               (0xa800)
#define DISP_RD_WR_PTR(ch)          (DISP_REG_BASE + ((ch)<< 4) + 0x00)
#define DISP_CH_BUF(ch)             (DISP_REG_BASE + ((ch)<< 4) + 0x04)
#define DISP_DMA_MODE(ch)           (DISP_REG_BASE + ((ch)<< 4) + 0x08)
#define DISP_MODE                   (DISP_REG_BASE + 0x0104)
#define DISP_CHNL_FMT07             (DISP_REG_BASE + 0x0108)
#define DISP_CHNL_FMT8f             (DISP_REG_BASE + 0x010c)
#define DISP_OUT_MODE               (DISP_REG_BASE + 0x0118)
#define DISP_OUT_NORM               (DISP_REG_BASE + 0x011c)
#define DISP_SIE_FMT(grp)           (DISP_REG_BASE + 0x0120 + ((grp) << 4))
#define DISP_SIE_SEL_L(grp)         (DISP_REG_BASE + 0x0124 + ((grp) << 4))
#define DISP_SIE_SEL_H(grp)         (DISP_REG_BASE + 0x0128 + ((grp) << 4))

#define DISP_IRQ_MODE               (DISP_REG_BASE + 0x01fc)
#define DISP_IRQ_FLAGS              (DISP_REG_BASE + 0x01f8)
#define DISP_IRQ_DMA_FLAGS          (DISP_REG_BASE + 0x01f4)
#define DISP_IRQ_LOST_SYNC          (DISP_REG_BASE + 0x01f0)

    enum ISIL_DISPLAY_INTERFACE_MODE{
	ISIL_DISPLAY_INTERFACE_MODE_27M = 0,
	ISIL_DISPLAY_INTERFACE_MODE_54M,
	ISIL_DISPLAY_INTERFACE_MODE_108M,
	ISIL_DISPLAY_INTERFACE_MODE_RESERVE,
    };

    enum ISIL_DISPLAY_POSITION_STATE{
	ISIL_DISPLAY_POSITION_STATE_UNUSED = 0,
	ISIL_DISPLAY_POSITION_STATE_USED,
	ISIL_DISPLAY_POSITION_STATE_RESERVE,
    };

    typedef struct isil_display_driver                isil_display_driver_t;
    typedef struct isil_display_operation             isil_display_operation_t;
    typedef struct isil_display_interface             isil_display_interface_t;
    typedef struct isil_display_interface_operation   isil_display_interface_operation_t;
    typedef struct isil_display_phase                 isil_display_phase_t;
    typedef struct isil_display_phase_operation       isil_display_phase_operation_t;
    typedef struct isil_display_position              isil_display_position_t;
    typedef struct isil_display_position_operation    isil_display_position_operation_t;

    struct isil_display_position_operation{
	int (*init)(isil_display_position_t *, isil_display_phase_t*, u32);
	int (*reset)(isil_display_position_t *);
	int (*release)(isil_display_position_t *);

	int (*get_state)(isil_display_position_t *);
	int (*get_position)(isil_display_position_t *);
	int (*change_pos_size)(isil_display_position_t *, u32 , u32, u32 , u32);

	int (*bind)(isil_display_position_t *, void *);
	int (*unbind)(isil_display_position_t *, void *);
	int (*sync)(isil_display_position_t *, void *);

	int (*send)(isil_display_position_t *);
	int (*pause)(isil_display_position_t *);
	int (*resume)(isil_display_position_t *);
	int (*discard)(isil_display_position_t *);
    };

    struct isil_display_position{
	spinlock_t lock;
	u32 position;
	enum ISIL_DISPLAY_POSITION_STATE state;

	u32 startx, starty, width, height;

	isil_display_phase_t *phase;
	void *context;

	isil_display_position_operation_t *op;
    };

    struct isil_display_phase_operation{
	int (*init)(isil_display_phase_t *);
	int (*reset)(isil_display_phase_t *);
	int (*release)(isil_display_phase_t *);

	int (*check_boundary)(isil_display_phase_t *, isil_display_position_t *);
    };

    struct isil_display_phase{
	u32 phase;

	isil_display_interface_t *interface;
	isil_display_position_t position[MAX_DISPLAY_POSITION];

	isil_display_phase_t *parent, *sibling, *child;
	isil_display_phase_operation_t *op;
    };

    struct isil_display_interface_operation{
	int (*init)(isil_display_interface_t *, u32 port, isil_chip_t *);
	int (*reset)(isil_display_interface_t *);
	int (*release)(isil_display_interface_t *);

	int (*update)(isil_display_interface_t *);

	u32 (*get_port)(isil_display_interface_t *);
	u32 (*get_capability)(isil_display_interface_t *);
	u32 (*get_mode)(isil_display_interface_t *);
	int (*set_mode)(isil_display_interface_t *, enum ISIL_DISPLAY_INTERFACE_MODE);
    };

    struct isil_display_interface{
	enum ISIL_DISPLAY_INTERFACE_MODE capability;/*108M/54M/27M*/
	u32 port;
	enum ISIL_DISPLAY_INTERFACE_MODE mode;

	isil_display_phase_t *root;
	isil_chip_t         *chip;
	isil_display_interface_operation_t *op;
    };

    struct isil_display_operation{
	int (*init)(isil_display_driver_t *);
	int (*reset)(isil_display_driver_t *);
	int (*release)(isil_display_driver_t *);

	int (*get_video_standard)(isil_display_driver_t *);
	int (*set_video_standard)(isil_display_driver_t *, enum ISIL_VIDEO_STANDARD);
    };

    struct isil_display_driver{
	atomic_t    opened_flag;
	ed_tcb_t	opened_display_ed;

	enum ISIL_VIDEO_STANDARD video_standard;
	isil_chip_t  *chip;
	isil_display_interface_t interface[MAX_DISPLAY_INTERFACE];

	isil_display_operation_t *op;
	isil_proc_register_s display_proc;
    };

    int init_display_driver(isil_chip_t *chip);
    int remove_display_driver(isil_display_driver_t *driver);

#ifdef __cplusplus
}
#endif

#endif //__ISIL_VIDEO_DISPLAY_H__
