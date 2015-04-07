#ifndef _ISIL_PREVIEW_H
#define _ISIL_PREVIEW_H

#ifdef __cplusplus
extern "C"
{
#endif

    /* preview related regs */
#define VP_CHANNEL_EN_STATUS	    (0xc000)
#define VP_CHANNEL_EN		    (0xc004)
#define VP_CHANNEL_DOWNSAMPLE	    (0xc008)
#define VP_CHANNEL_INTERLACE	    (0xc00c)
#define VP_BUS_MAX_CHANNEL	    (0xc010)
#define VP_BUS_MAX_LINE0            (0xc014)
#define VP_BUS_MAX_LINE1            (0xc018)
#define VP_CHANNEL_FORMAT0	    (0xc020)
#define VP_CHANNEL_FORMAT1	    (0xc024)
#define VP_FIELD_SELECT		    (0xc028)
#define VP_CMD_START		    (0xc02c)
#define VP_CHANNEL_LMAP(pos)	    (0xc100 + (pos) * sizeof(int) * 2)
#define VP_CHANNEL_HMAP(pos)        (0xc100 + (pos) * sizeof(int) * 2 + sizeof(int))
#define VP_BUS_MAP(bus)             (0xc200 + (bus) * sizeof(int))

#define PCI_PREVIEW_BUS4_ENB        (0x18040)
#define PCI_PREVIEW_FORMAT          (0x18044)

#define PCI_PREVIEW_BASE_ADDRA	    (0x18100)
#define PCI_PREVIEW_BASE_ADDRB	    (0x18104)

#define ISIL_VP_MAP_CHAN_NUM (16)
#define VP_BUS_CNT		    (4)
#define VP_BUS_CHAN_CNT		    (4)

#define MAX_BUF_CNT		    (32)

#define BIT_MASTER 	BIT(0)
#define BIT_VLC 	BIT(1)
#define BIT_MV 		BIT(2)
#define BIT_PREV 	BIT(4)
#define BIT_PREVOF 	BIT(5)
#define BIT_AUDIO 	BIT(8)

    /* format and standard */
#define PIX_DEPTH		(12)

#define CROP_D1_WIDTH		(704)

#define PAL_D1_WIDTH		(720)
#define PAL_D1_HEIGHT		(576)
#define PAL_D1_SIZE ((PAL_D1_WIDTH * PAL_D1_HEIGHT * PIX_DEPTH) / 8)
#define NTSC_D1_WIDTH		(720)
#define NTSC_D1_HEIGHT		(480)
#define NTSC_D1_SIZE ((NTSC_D1_WIDTH * NTSC_D1_HEIGHT * PIX_DEPTH) / 8)
#define D1_ORDER		(8)
#define D1_BUF_SIZE PAGE_ALIGN(PAL_D1_SIZE)

#define PAL_CIF_WIDTH		(352)
#define PAL_CIF_HEIGHT		(288)
#define PAL_CIF_SIZE ((PAL_CIF_WIDTH * PAL_CIF_HEIGHT * PIX_DEPTH) / 8)
#define NTSC_CIF_WIDTH		(352)
#define NTSC_CIF_HEIGHT		(240)
#define NTSC_CIF_SIZE ((NTSC_CIF_WIDTH * NTSC_CIF_HEIGHT * PIX_DEPTH) / 8)
#define CIF_ORDER		(7)
#define CIF_BUF_SIZE PAGE_ALIGN(PAL_CIF_SIZE)

#define PAL_QCIF_WIDTH		(176)
#define PAL_QCIF_HEIGHT		(144)
#define PAL_QCIF_SIZE ((PAL_QCIF_WIDTH * PAL_QCIF_HEIGHT * PIX_DEPTH) / 8)
#define NTSC_QCIF_WIDTH		(176)
#define NTSC_QCIF_HEIGHT	(120)
#define NTSC_QCIF_SIZE ((NTSC_QCIF_WIDTH * NTSC_QCIF_HEIGHT * PIX_DEPTH) / 8)
#define QCIF_ORDER		(4)
#define QCIF_BUF_SIZE PAGE_ALIGN(PAL_QCIF_SIZE)

#define FULL_FPS_PAL		(25)
#define FULL_DISCARD_PAL	(0x1ffffff)
#define STRIDE_PAL		(24)

#define FULL_FPS_NTSC		(30)
#define FULL_DISCARD_NTSC	(0x3fffffff)
#define STRIDE_NTSC		(29)

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

    typedef struct isil_vp_bus isil_vp_bus_t;
    typedef struct vp_map_info vp_map_info_t;
    typedef struct isil_prev_norm isil_prev_norm_t;
    typedef struct tasklet_priv tasklet_priv_t;
    typedef struct isil_prev_frame isil_prev_frame_t;
    typedef struct isil_prev_param isil_prev_param_t;
    typedef struct isil_prev_property isil_prev_property_t;

    enum prev_frame_state {
        F_UNUSED,
        F_QUEUED,
        F_GRABBING,
        F_DONE,
        F_ERROR,
    };

    enum bus_map_reason {
        R_RESET,
        R_FMT_CHANGE,
        R_FPS_CHANGE,
        R_RESERVE,
    };

#define	to_get_prev_channel_with_property(node)	container_of(node, isil_prev_logic_chan_t, prev_property)
#define	to_get_prev_channel_with_opened_logic_chan_ed(node) container_of(node, isil_prev_logic_chan_t, opened_logic_chan_ed)

    struct tasklet_priv {
        u32 chan_flag;
        u32 ab_flag;
        isil_chip_t *chip;
    };

    struct vp_map_info{
	u32	phy_slot_id;
	u32	map_id;
	u32     enable;
	u32	fps;
	u32	stride;
	int     video_size;	
	u32	discard;
    };

    struct isil_vp_bus_operation {
        int (*init)(isil_vp_bus_t *);
        int (*reset)(isil_vp_bus_t *);
        int (*release)(isil_vp_bus_t *);

        int (*notify_logic_chan_change)(isil_vp_bus_t *);
        int (*notify_suspend_logic_chan)(isil_vp_bus_t *);
        int (*notify_resume_logic_chan)(isil_vp_bus_t *);

        int (*get_norm)(isil_vp_bus_t *);
        int (*set_norm)(isil_vp_bus_t *, int);
        int (*bus_map)(isil_vp_bus_t *, int);

	int (*calculate_vp_bus_map_table)(isil_vp_bus_t *, vp_map_info_t *);
        int (*bus_map_apply)(isil_vp_bus_t *, vp_map_info_t *);

        void (*polling_task)(isil_vp_bus_t *);
    };

    struct isil_vp_bus {
        int norm;
        tasklet_priv_t tasklet_priv;
        struct tasklet_struct prev_tasklet;
        robust_processing_control_t robust_processing_control;
        struct mutex vp_mutex; 
        struct isil_vp_bus_operation *op;
    };

    struct isil_prev_norm {
        int   v4l2_id;
        char  *name;
    };

    struct isil_prev_frame {
        int state;
        void *data_virt;
        struct v4l2_buffer buf;
        struct list_head frame;
        unsigned int vma_use_count;
    };

    struct isil_prev_param {
        int video_size;
        u32 fps;
        u32 discard;
        u32 stride;
        struct v4l2_cropcap cropcap;
        struct v4l2_pix_format pix_format;
        struct v4l2_rect rect;
    };

    struct isil_prev_property_operation {
        int	(*init)(isil_prev_property_t *, isil_prev_param_t *);
        int	(*reset)(isil_prev_property_t *, isil_prev_param_t *);

        int	(*get_enable)(isil_prev_property_t *);
        int	(*get_video_size)(isil_prev_property_t *);
        u32	(*get_fps)(isil_prev_property_t *);
        u32	(*get_discard)(isil_prev_property_t *);
        u32	(*get_stride)(isil_prev_property_t *);
        int	(*get_cropcap)(isil_prev_property_t *, struct v4l2_cropcap *);
        int	(*get_pix_format)(isil_prev_property_t *, struct v4l2_pix_format *);
        int	(*get_rect)(isil_prev_property_t *, struct v4l2_rect *);
        
        int	(*set_enable)(isil_prev_property_t *, int);
        int	(*set_video_size)(isil_prev_property_t *, int);
        int	(*set_fps)(isil_prev_property_t *, u32);
        int	(*set_discard)(isil_prev_property_t *, u32);
        int	(*set_stride)(isil_prev_property_t *, u32);
        int	(*set_cropcap)(isil_prev_property_t *, struct v4l2_cropcap *);
        int	(*set_pix_format)(isil_prev_property_t *, struct v4l2_pix_format *);
        int	(*set_rect)(isil_prev_property_t *, struct v4l2_rect *);
    };

    struct isil_prev_property {
        isil_prev_param_t config_param;
        isil_prev_param_t running_param;
        spinlock_t lock;
        struct isil_prev_property_operation	*op;
    };

    struct isil_prev_logic_chan {
        isil_timestamp_t timestamp;
        int phy_slot_id;
        int logic_chan_id;
        atomic_t    opened_flag;
        robust_processing_control_t robust_processing_control;
        ed_tcb_t    opened_logic_chan_ed;
        isil_send_msg_controller_t  send_msg_contr;
        isil_prev_property_t prev_property;

        u32 serial;
        u32 discard_number;
        int enable;
        int map_refer;
        int stream_on;
        int app_norm;
        
        u32 nbuffers;
        isil_prev_frame_t frame[MAX_BUF_CNT];
        void *pushA_buf;
        void *pushB_buf;
        struct list_head inqueue, outqueue;
        u32 inq_cnt, outq_cnt;
        spinlock_t queue_lock;
        wait_queue_head_t wait_frame;

        isil_chip_t *chip;
        struct video_device* v4ldev;
        struct semaphore sem; 
    };

    extern int init_isil_vp_bus(isil_vp_bus_t *vp_bus);
    extern void remove_isil_vp_bus(isil_vp_bus_t *vp_bus);

    void reset_isil_prev_chan_preprocessing(ed_tcb_t *ed_tcb);
    void reset_isil_prev_chan_postprocessing(ed_tcb_t *ed_tcb);
    void reset_isil_prev_chan_processing(ed_tcb_t *ed_tcb);

    void isil_chip_prev_robust_process_done(isil_vp_bus_t *vp);

#ifdef __cplusplus
}
#endif

#endif
