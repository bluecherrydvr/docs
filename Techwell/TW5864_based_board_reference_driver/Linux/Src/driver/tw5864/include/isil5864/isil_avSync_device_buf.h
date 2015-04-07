#ifndef	ISIL_AVSYNC_DEVICE_BUF_H
#define	ISIL_AVSYNC_DEVICE_BUF_H

#ifdef __cplusplus
extern "C"
{
#endif

    struct avSync_frame_buf_pool_operation{
	int		(*create)(avSync_frame_buf_pool_t *video_device_buf_pool, unsigned long );
	void	(*release)(avSync_frame_buf_pool_t *video_device_buf_pool);

	void	(*get_avSync_frame_tcb)(avSync_frame_buf_pool_t *video_device_buf_pool, avSync_frame_tcb_t **ptr_frame);
	void	(*try_get_avSync_frame_tcb)(avSync_frame_buf_pool_t *video_device_buf_pool, avSync_frame_tcb_t **ptr_frame);
	void	(*put_avSync_frame_tcb)(avSync_frame_buf_pool_t *video_device_buf_pool, avSync_frame_tcb_t *frame);
	int		(*get_avSync_frame_tcb_pool_entry_number)(avSync_frame_buf_pool_t *video_device_buf_pool);
    };

    struct avSync_frame_buf_pool{
	unsigned long	avSync_frame_entry_number;
	tcb_node_pool_t	avSync_frame_pool_node;
	avSync_frame_tcb_t	*avSync_frame_pool;

	struct avSync_frame_buf_pool_operation	*op;
    };

    struct avSync_frame_tcb_operation{
	void	(*init)(avSync_frame_tcb_t *frame);
	void	(*reset)(avSync_frame_tcb_t *frame);
	void	(*release)(avSync_frame_tcb_t **ptr_frame, avSync_frame_buf_pool_t *video_device_buf_pool);
	void	(*reference)(avSync_frame_tcb_t *src_frame, avSync_frame_tcb_t **ptr_dest_frame);

	size_t	(*submit)(avSync_frame_tcb_t *avSync_frame, char __user *data, size_t count, loff_t *ppos, int *frame_end, isil_avSync_dev_t *dev);
    };

#define	CURR_SUBMIT_IS_264_MASTER_FRAME     (0)
#define	CURR_SUBMIT_IS_264_MASTER_MV_FRAME  (1)
#define	CURR_SUBMIT_IS_264_SUB_FRAME        (2)
#define	CURR_SUBMIT_IS_264_SUB_MV_FRAME     (3)
#define	CURR_SUBMIT_IS_AUDIO_FRAME          (4)
#define	CURR_SUBMIT_IS_JPEG_FRAME           (5)
#define	to_get_avSync_frame_tcb(node)       container_of(node, avSync_frame_tcb_t, frame_node)
    struct avSync_frame_tcb{
	atomic_t	ref_count;
	spinlock_t	lock;
	tcb_node_t	frame_node;

	int	curr_submit_tcb;
	isil_video_frame_tcb_t    *h264_master_frame;
#ifdef  MV_MODULE
	isil_video_mv_frame_tcb_t *h264_master_mv_frame;
#endif
	isil_video_frame_tcb_t    *h264_sub_frame;
#ifdef  MV_MODULE
	isil_video_mv_frame_tcb_t *h264_sub_mv_frame;
#endif
	isil_audio_packet_section_t   *audio_encode_frame;
	isil_audio_packet_section_t   *audio_decode_frame;
#ifdef MJPEG_MODULE
	isil_vb_frame_tcb_t       *jpeg_frame;
#endif
	struct avSync_frame_tcb_operation	*op;
    };

    struct avSync_frame_queue_operation{
	void    (*get)(avSync_frame_queue_t *, avSync_frame_tcb_t **);
	void    (*try_get)(avSync_frame_queue_t *, avSync_frame_tcb_t **);
	void    (*get_tailer)(avSync_frame_queue_t *, avSync_frame_tcb_t **);
	void    (*try_get_tailer)(avSync_frame_queue_t *, avSync_frame_tcb_t **);
	void    (*put)(avSync_frame_queue_t *, avSync_frame_tcb_t *);
	void    (*put_header)(avSync_frame_queue_t *, avSync_frame_tcb_t *);

	void    (*get_curr_producer_from_pool)(avSync_frame_queue_t *, avSync_frame_buf_pool_t *video_device_buf_pool);
	void    (*try_get_curr_producer_from_pool)(avSync_frame_queue_t *, avSync_frame_buf_pool_t *video_device_buf_pool);
	void    (*put_curr_producer_into_queue)(avSync_frame_queue_t *);
	void    (*release_curr_producer)(avSync_frame_queue_t *, avSync_frame_buf_pool_t *video_device_buf_pool);
	void    (*get_curr_consumer_from_queue)(avSync_frame_queue_t *);
	void    (*try_get_curr_consumer_from_queue)(avSync_frame_queue_t *);
	void    (*release_curr_consumer)(avSync_frame_queue_t *, avSync_frame_buf_pool_t *video_device_buf_pool);

	int     (*get_curr_queue_entry_number)(avSync_frame_queue_t *);
	void    (*release)(avSync_frame_queue_t *, avSync_frame_buf_pool_t *video_device_buf_pool);
	void    (*init)(avSync_frame_queue_t *);
    };

    struct avSync_frame_queue{
	u32	start_timestamp, total_duration;
	tcb_node_queue_t    queue_node;
	avSync_frame_tcb_t  *curr_consumer;
	avSync_frame_tcb_t	*curr_producer;
	spinlock_t	lock;

	struct avSync_frame_queue_operation	*op;
    };

    extern void	init_avSync_frame_queue(avSync_frame_queue_t *avSync_frame_queue);
    extern void	remove_avSync_frame_queue(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_buf_pool_t *video_device_buf_pool);
    extern int	init_avSync_frame_buf_pool(avSync_frame_buf_pool_t *video_device_buf_pool);
    extern void	remove_avSync_frame_buf_pool(avSync_frame_buf_pool_t *video_device_buf_pool);


#ifdef __cplusplus
}
#endif

#endif	//ISIL_AVSYNC_DEVICE_BUF_H

