#ifndef	ISIL_ENDPOINT
#define	ISIL_ENDPOINT

#ifdef __cplusplus
extern "C"
{
#endif

#define	ISIL_OK         (0)
#define	ISIL_ERR        (-1)

#define ISIL_CAN_DURATION_MAX_COUNTER               (0x7fffffff)
#define ISIL_IDLE_CAN_DURATION_MAX_COUNTER          (250)
#define ISIL_STANDBY_CAN_DURATION_MAX_COUNTER       (50)
#define ISIL_RUNNING_CAN_DURATION_MAX_COUNTER       (20)
#define ISIL_TRANSFERING_CAN_DURATION_MAX_COUNTER   (100)
#define ISIL_DONE_CAN_DURATION_MAX_COUNTER          (5)

    enum ISIL_ED_STATUS{
	ISIL_ED_UNREGISTER = 0,
	ISIL_ED_IDLE,
	ISIL_ED_STANDBY,
	ISIL_ED_SUSPEND,
	ISIL_ED_RUNNING,
	ISIL_ED_TRANSFERING,
	ISIL_ED_DONE,
    };

    enum ISIL_ED_EVENT{
	ISIL_ED_CLOSE_EVENT = 0,
	ISIL_ED_TIMEOUT_EVENT,
	ISIL_ED_SUSPEND_EVENT,
	ISIL_ED_RESUME_EVENT,
	ISIL_ED_OPEN_EVENT,
	ISIL_ED_DELIVER_EVENT,
    };

#define ISIL_ED_STATE_NUMBER      ((ISIL_ED_DONE)+1)
#define ISIL_ED_EVENT_NUMBER      ((ISIL_ED_DELIVER_EVENT)+1)
    struct isil_ed_fsm_transfer_pending{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 close_pending_flag : 1;
	u32 timeout_pending_flag : 1;
	u32 suspend_pending_flag : 1;
	u32 resume_pending_flag : 1;
	u32 open_pending_flag : 1;
	u32 deliver_pending_flag : 1;
	u32 need_sync : 1;
	u32 reserve : 25;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 25;
	u32 need_sync : 1;
	u32 deliver_pending_flag : 1;
	u32 open_pending_flag : 1;
	u32 resume_pending_flag : 1;
	u32 suspend_pending_flag : 1;
	u32 timeout_pending_flag : 1;
	u32 close_pending_flag : 1;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    };

    typedef int (*fsm_transfer_action)(isil_ed_fsm_t *, void *context);
    struct fsm_state_transfer_matrix_table{
	fsm_transfer_action     action[ISIL_ED_STATE_NUMBER][ISIL_ED_EVENT_NUMBER];
	fsm_transfer_action     sync_hook;
	fsm_transfer_action     sync_config_to_running_hook;
	fsm_transfer_action     sync_running_to_config_hook;
    };

    struct isil_ed_fsm_operation{
	void    (*init)(isil_ed_fsm_t *);
	void    (*reset)(isil_ed_fsm_t *);
	void    (*register_table)(isil_ed_fsm_t *, fsm_state_transfer_matrix_table_t *, void *);
	void    (*unregister_table)(isil_ed_fsm_t *);
	void    (*update_need_sync_hook)(isil_ed_fsm_t *, int en);
	void    (*update_enable_state_transfer)(isil_ed_fsm_t *, int en);
	void    (*update_enable_trigger_pending_event)(isil_ed_fsm_t *);
	void    (*change_state)(isil_ed_fsm_t *, enum ISIL_ED_STATUS state);
	void    (*change_state_for_robust)(isil_ed_fsm_t *);
	void    (*req_fsm_event)(isil_ed_fsm_t *, enum ISIL_ED_EVENT event);
	void    (*ack_fsm_event)(isil_ed_fsm_t *, enum ISIL_ED_EVENT event);
	int     (*gen_fsm_event)(isil_ed_fsm_t *, enum ISIL_ED_EVENT event, int can_transfer);
	void    (*save_event_for_robust_processing)(isil_ed_fsm_t *);
	void    (*restore_event_from_robust_processing)(isil_ed_fsm_t *);
	int     (*transfer)(isil_ed_fsm_t *, enum ISIL_ED_EVENT event);
	int     (*get_curr_state)(isil_ed_fsm_t *);
	int     (*get_last_state)(isil_ed_fsm_t *);
	void    (*update_state_timeout_counter)(isil_ed_fsm_t *);
        void    (*feed_state_watchdog)(isil_ed_fsm_t *);
    };

    struct isil_ed_fsm{
	int     last_state, curr_state;
	int     timeout_counter;
	void    *context;
	spinlock_t  lock;
	struct isil_ed_fsm_transfer_pending transfer_pending, robust_processing_pending;
	fsm_state_transfer_matrix_table_t   *table;

	struct isil_ed_fsm_operation    *op;
    };

    enum ISIL_DIRECTION{
	ISIL_READ = 0,
	ISIL_WRITE
    };

    enum ISIL_ED_BUS_ID{
	ISIL_ED_HOST_BUS = 0,
	ISIL_ED_PCI_BUS,
	INVALID_ISIL_ED_BSU_ID
    };

    enum ISIL_ED_TYPE_ID{
	ISIL_ED_CONTROL = 0,
	ISIL_ED_VIDEO_ENCODE_IN,
	ISIL_ED_VIDEO_PREVIEW_IN,
	ISIL_ED_VIDEO_ENCODE_OSD_OUT,
	ISIL_ED_VIDEO_DECODE_OUT,
	ISIL_ED_VIDEO_PREVIEW_OUT,
	ISIL_ED_VIDEO_DECODE_OSG_OUT,
	ISIL_ED_AUDIO_ENCODE_IN,
	ISIL_ED_AUDIO_DECODE_OUT,
	INVALID_ISIL_ED_TYPE_ID
    };

#define	BUS_NUMBER_MASK		(0xf)
#define	CHIP_NUMBER_MASK	(0xf)
#define	TYPE_NUMBER_MASK	(0xf)
#define	GROUP_CHAN_NUMBER_MASK	(0x3ff)
#define	LOGIC_CHAN_NUMBER_MASK	(0x3ff)
    struct endpoint_id{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32	bus_id:4;
	u32	chip_id:4;
	u32	type_id:4;
	u32	group_chan_id:10;
	u32	logic_chan_id:10;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 logic_chan_id:10;
	u32	group_chan_id:10;
	u32	type_id:4;
	u32	chip_id:4;
	u32	bus_id:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    };

#define	to_get_endpoint_tcb_with_register_node(node)    container_of(node, ed_tcb_t, ed)
#define to_get_endpoint_tcb_with_isil_ed_fsm(node)      container_of(node, ed_tcb_t, ed_fsm)
    struct endpoint_tcb{
	ed_id           id;
	atomic_t        state;
	isil_ed_fsm_t   ed_fsm;
	isil_register_node_t    ed;

	isil_hcd_interface_operation *op;
    };

    struct hcd_interface_operation{
	int     (*open)(ed_tcb_t *);
	int     (*close)(ed_tcb_t *);
	void    (*suspend)(ed_tcb_t *);
	void    (*resume)(ed_tcb_t *);
	int     (*ioctl)(ed_tcb_t *, unsigned int , unsigned long );
	int     (*get_state)(ed_tcb_t *);
	void    (*suspend_done)(ed_tcb_t *);
	void    (*resume_done)(ed_tcb_t *);
	void    (*close_done)(ed_tcb_t *);
    };


    struct _ISIL_FRAME_TIME_PAD{
	unsigned char	header_tag_zero0;
	unsigned char	header_tag_zero1;
	unsigned char	header_tag_zero2;
	unsigned char	header_tag_one;
	unsigned char	header_tag_nal_time_type;
	unsigned char	header_tag_nal_time_ext_type;
	unsigned char	year_high;
	unsigned char	year_low;
	unsigned char	month;
	unsigned char	day;
	unsigned char	hour;
	unsigned char	minute;
	unsigned char	second;
	unsigned char	millisecond_high;
	unsigned char	millisecond_low;
	unsigned char	timestamp_24_31;
	unsigned char	timestamp_16_23;
	unsigned char	timestamp_08_15;
	unsigned char	timestamp_00_07;
	unsigned char	reserved;
    };



    struct isil_timestamp_operation{
	void    (*init)(isil_timestamp_t *);
	void    (*reset)(isil_timestamp_t *);
	void    (*set_timestamp)(isil_timestamp_t *, int timestamp, char *call_fn_name);
	u32     (*get_timestamp)(isil_timestamp_t *);
    };

    struct isil_timestamp{
	atomic_t  first_sync;
	int	last_timer_count, curr_timer_count;
	unsigned int    total_delta_timer_count;

	struct isil_timestamp_operation	*op;
    };

    extern int  no_op(isil_ed_fsm_t *ed_fsm, void *context);
    extern int  have_any_pending_event(ed_tcb_t *ed_tcb);
    extern void driver_trigger_pending_event(ed_tcb_t *ed_tcb);
    extern void driver_gen_close_event(ed_tcb_t *ed_tcb, int can_transfer);
    extern void driver_gen_timeout_event(ed_tcb_t *ed_tcb, int can_transfer);
    extern void driver_gen_suspend_event(ed_tcb_t *ed_tcb, int can_transfer);
    extern void driver_gen_resume_event(ed_tcb_t *ed_tcb, int can_transfer);
    extern void driver_gen_open_event(ed_tcb_t *ed_tcb, int can_transfer);
    extern void driver_gen_deliver_event(ed_tcb_t *ed_tcb, int can_transfer);
    extern void direct_driver_config_to_running(ed_tcb_t *ed_tcb);
    extern void driver_sync_config_to_running(ed_tcb_t *ed_tcb);


#ifdef __cplusplus
}
#endif

#endif

