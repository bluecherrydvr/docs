#ifndef	ISIL_KTHREAD_MSG_H
#define	ISIL_KTHREAD_MSG_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum{
        REQ_ALGO_SLICE_HEAD = 0,
        REQ_AV_SYNC,
        REQ_ALGO_CHIP_RESET,
        REQ_ALGO_NULL,
        REQ_VIDEO_STANDARD_CHANGED,
        SYSTEM_ESCAPE_MSG,
        INVALID_REQ_ALGO, 
        REQ_MSG_NUMBER,
    }REQ_SYSTEM_MSG;

#define	BLOCK_OP            (0)
#define	NONBLOCK_OP         (1)
#define	DELAY_OP            (2)
#define	NONBLOCK_COUNT      (1)
#define	DELAY_COUNT         (50)


    struct isil_kthread_msg_operation{
        void    (*init)(isil_kthread_msg_t *, isil_kthread_msg_pool_t *);
        void    (*reset)(isil_kthread_msg_t *);
        void    (*release)(isil_kthread_msg_t **);
        void    (*update_param)(isil_kthread_msg_t *, isil_chip_t * , int , void *, void *);
    };

#define	to_get_isil_kthread_msg(node)	container_of(node, isil_kthread_msg_t, msg_node)
    struct isil_kthread_msg{
        tcb_node_t	msg_node;
        atomic_t	ref_count;
        spinlock_t	lock;
        isil_kthread_msg_pool_t	*msg_pool;

        isil_chip_t	*chip;
        int		msg_type;
        void	*msg_context;
        void    *owner_context;

        struct isil_kthread_msg_operation	*op;
    };

    struct isil_kthread_msg_queue_operation{
        void	(*init)(isil_kthread_msg_queue_t *);
        void	(*put)(isil_kthread_msg_queue_t *, isil_kthread_msg_t *);
        void	(*get)(isil_kthread_msg_queue_t *, isil_kthread_msg_t **);
        void	(*try_get)(isil_kthread_msg_queue_t *, isil_kthread_msg_t **);
        void	(*release)(isil_kthread_msg_queue_t *);

        void	(*get_curr_consumer_from_queue)(isil_kthread_msg_queue_t *);
        void	(*release_curr_consumer)(isil_kthread_msg_queue_t *);
        int     (*get_curr_queue_entry_number)(isil_kthread_msg_queue_t *);
    };

    struct isil_kthread_msg_queue{
        tcb_node_queue_t    queue_node;
        isil_kthread_msg_t  *curr_consumer;
        spinlock_t          lock;

        struct isil_kthread_msg_queue_operation	*op;
    };

    struct isil_kthread_msg_pool_operation{
        void	(*get)(isil_kthread_msg_pool_t *, isil_kthread_msg_t **);
        void	(*try_get)(isil_kthread_msg_pool_t *, isil_kthread_msg_t **);
        void	(*put)(isil_kthread_msg_pool_t *, isil_kthread_msg_t *);
        void	(*release)(isil_kthread_msg_pool_t *);
        int		(*init)(isil_kthread_msg_pool_t *);
    };

    struct isil_kthread_msg_pool{
        tcb_node_pool_t         pool_node;
        int                     msg_entry_number;
        isil_kthread_msg_t      *msg_cache_pool;
        struct isil_kthread_msg_pool_operation	*op;
    };

    typedef enum ISIL_SEND_MSG_OWNER{
        ISIL_NULL_OWNER = 0,
        ISIL_H264_ENCODE_CHAN,
        ISIL_H264_DECODE_CHAN,
        ISIL_MJPEG_ENCODE_CHAN,
        ISIL_MJPEG_DECODE_CHAN,
        ISIL_AUDIO_ENCODE_CHAN,
        ISIL_AUDIO_DECODE_CHAN,
        ISIL_VIDEO_PREVIEW_CHAN,
        ISIL_OWNER_VI,
        ISIL_OWNER_RESERVED,
    }ISIL_SEND_MSG_OWNER_E;

    struct isil_send_msg_controller_operation{
        void    (*init)(isil_send_msg_controller_t *);
        int     (*send_msg)(isil_send_msg_controller_t *, isil_chip_t *, void *context, int msg_type, int block);
        void    (*set_owner)(isil_send_msg_controller_t *, int owner, int logic_chan_id, void *owner_context);
        void    (*delete_send_msg_controller)(isil_send_msg_controller_t *);
    };

    struct isil_send_msg_controller{
        int nonblock_msg_timer_id[REQ_MSG_NUMBER];
        int delay_msg_timer_id[REQ_MSG_NUMBER];
        spinlock_t  lock;

        isil_chip_t *chip;
        void    *context;
        void    *owner_context;
        int     msg_type;
        int     msg_send_owner;
        int     logic_chan_id;

        struct isil_send_msg_controller_operation    *op;
    };

    extern void	get_msg_queue_header(isil_kthread_msg_queue_t **ptr_queue);
    extern void	init_isil_kthread_msg_queue(isil_kthread_msg_queue_t *msg_queue);
    extern void delete_all_msg_of_this_h264_encode_chan(isil_kthread_msg_queue_t *msg_queue, isil_h264_logic_encode_chan_t *encode_chan);
    extern void delete_all_msg_of_this_chip(isil_kthread_msg_queue_t *msg_queue, isil_chip_t *chip);
    extern void	remove_isil_kthread_msg_queue(isil_kthread_msg_queue_t *msg_queue);

    extern void	get_msg_pool_header(isil_kthread_msg_pool_t **msg_pool);
    extern int	init_isil_kthread_msg_pool(isil_kthread_msg_pool_t *msg_pool);
    extern int jpeg_encode_chan_gen_req_msg(isil_jpeg_logic_encode_chan_t *encode_chan, int msg_type, int block);
    extern int prev_chan_gen_req_msg(isil_prev_logic_chan_t *chan, int msg_type, int block);
    extern void	remove_isil_kthread_msg_pool(isil_kthread_msg_pool_t *msg_pool);

    extern int isil_vi_video_standard_gen_req(isil_chip_t *chip, int msg_type, int block);
    extern int	encode_chan_gen_req_msg(isil_h264_logic_encode_chan_t *encode_chan, int msg_type, int block);
    extern int  audio_chan_gen_req_msg(isil_audio_driver_t *audio_driver, int msg_type, int block);
    extern void init_isil_send_msg_controller(isil_send_msg_controller_t *send_msg_contr, int owner, int logic_chan_id, void *owner_context);

    extern void	start_isil_kthread(void);
    extern void	stop_isil_kthread(void);

#ifdef __cplusplus
}
#endif


#endif
