#ifndef	ISIL_H264_VIDEO_BUF_H
#define	ISIL_H264_VIDEO_BUF_H

#ifdef __cplusplus
extern "C"
{
#endif

#define	VIDEO_MASTER_CHAN_BUF_POOL_LEN	(0x80000)	//512KB
#define	VIDEO_SUB_CHAN_BUF_POOL_LEN	(0x20000)	//128KB
#define	MotionVectorBitFlag_BUF_LEN     (64)

    struct insert_03_state_machine{
	int	tran_state;
	int	transfer;
    };


    struct isil_video_chan_buf_pool_operation{
	int     (*create)(isil_video_chan_buf_pool_t *video_chan_buf_pool, unsigned long );
	void    (*release)(isil_video_chan_buf_pool_t *video_chan_buf_pool);

	void    (*get_video_packet_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_packet_tcb_t **packet);
	void    (*try_get_video_packet_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_packet_tcb_t **packet);
	void    (*put_video_packet_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_packet_tcb_t *packet);
	int     (*get_video_packet_tcb_pool_entry_number)(isil_video_chan_buf_pool_t *video_chan_buf_pool);

	void    (*get_video_frame_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_frame_tcb_t **frame);
	void    (*try_get_video_frame_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_frame_tcb_t **frame);
	void    (*put_video_frame_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_frame_tcb_t *frame);
	int     (*get_video_frame_tcb_pool_entry_number)(isil_video_chan_buf_pool_t *video_chan_buf_pool);

#ifdef  MV_MODULE
	void    (*get_video_mv_packet_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_packet_tcb_t **packet);
	void    (*try_get_video_mv_packet_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_packet_tcb_t **packet);
	void    (*put_video_mv_packet_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_packet_tcb_t *packet);
	int     (*get_video_mv_packet_tcb_pool_entry_number)(isil_video_chan_buf_pool_t *video_chan_buf_pool);

	void    (*get_video_mv_frame_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_frame_tcb_t **frame);
	void    (*try_get_video_mv_frame_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_frame_tcb_t **frame);
	void    (*put_video_mv_frame_tcb)(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_frame_tcb_t *frame);
	int     (*get_video_mv_frame_tcb_pool_entry_number)(isil_video_chan_buf_pool_t *video_chan_buf_pool);
#endif
    };

    struct isil_video_chan_buf_pool{
	int	cache_order;
	unsigned char	*cache_buffer;

	unsigned long	video_packet_buffer_size;
	unsigned long	video_packet_entry_number;
	tcb_node_pool_t	video_packet_pool_tcb;
	isil_video_packet_tcb_t	*video_packet_entry_pool;

	unsigned long	video_frame_entry_number;
	tcb_node_pool_t	video_frame_pool_tcb;
	isil_video_frame_tcb_t	*video_frame_entry_pool;
#ifdef  MV_MODULE
	unsigned long	mv_cache_order;
	unsigned char	*mv_cache_buffer;

	unsigned long	mv_packet_buffer_size;
	unsigned long	mv_packet_entry_number;
	tcb_node_pool_t	mv_packet_pool_tcb;
	isil_video_mv_packet_tcb_t	*mv_packet_entry_pool;

	unsigned long	mv_frame_entry_number;
	tcb_node_pool_t	mv_frame_pool_tcb;
	isil_video_mv_frame_tcb_t	*mv_frame_entry_pool;
#endif
	struct isil_video_chan_buf_pool_operation	*op;
    };

    struct isil_video_packet_operation
    {
	void    (*init)(isil_video_packet_tcb_t *packet, isil_video_chan_buf_pool_t *pool, int id);
	void    (*reset)(isil_video_packet_tcb_t *packet);
	void    (*release)(isil_video_packet_tcb_t **packet, isil_video_chan_buf_pool_t *pool);
	void    (*reference)(isil_video_packet_tcb_t *src_packet, isil_video_packet_tcb_t **ptr_dest_packet);

	void    (*dma_map)(isil_video_packet_tcb_t *packet, enum dma_data_direction direction);
	void    (*dma_unmap)(isil_video_packet_tcb_t *packet, enum dma_data_direction direction);

	size_t  (*submit)(isil_video_packet_tcb_t *packet, char __user *data, size_t count, loff_t *ppos, h264_nal_t *nal);
	size_t  (*slicent_discard)(isil_video_packet_tcb_t *packet, char __user *data, size_t count, loff_t *ppos, h264_nal_t *nal);
	int     (*copy)(isil_video_packet_tcb_t *, isil_video_packet_tcb_t *);
    };

#define	to_video_packet_buf_tcb(node)	container_of(node, isil_video_packet_tcb_t, packet_node)
    struct isil_video_packet_tcb
    {
	atomic_t    ref_count;
	spinlock_t  lock;
	tcb_node_t  packet_node;
	int         packet_id;

	int	packet_size;
	int	payload_len;
	int	consumer_len;
	int	id_of_packet_queue;

	u8	*data;
	dma_addr_t	dma_addr;

	struct isil_video_packet_operation	*op;
    };

    struct isil_video_packet_queue_operation{
	void    (*get)(isil_video_packet_tcb_queue_t *, isil_video_packet_tcb_t **);
	void    (*try_get)(isil_video_packet_tcb_queue_t *, isil_video_packet_tcb_t **);
	void    (*get_tailer)(isil_video_packet_tcb_queue_t *, isil_video_packet_tcb_t **);
	void    (*try_get_tailer)(isil_video_packet_tcb_queue_t *, isil_video_packet_tcb_t **);
	void    (*put)(isil_video_packet_tcb_queue_t *, isil_video_packet_tcb_t *);
	void    (*put_header)(isil_video_packet_tcb_queue_t *, isil_video_packet_tcb_t *);

	void    (*get_curr_producer_from_pool)(isil_video_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void    (*try_get_curr_producer_from_pool)(isil_video_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void    (*put_curr_producer_into_queue)(isil_video_packet_tcb_queue_t *);
	void    (*release_curr_producer)(isil_video_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void    (*get_curr_consumer_from_queue)(isil_video_packet_tcb_queue_t *);
	void    (*try_get_curr_consumer_from_queue)(isil_video_packet_tcb_queue_t *);
	void    (*release_curr_consumer)(isil_video_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);

	int     (*get_curr_queue_entry_number)(isil_video_packet_tcb_queue_t *);
	void    (*release)(isil_video_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void    (*init)(isil_video_packet_tcb_queue_t *);
    };

    struct isil_video_packet_tcb_queue{
	u32	start_timestamp, total_duration;
	tcb_node_queue_t	queue_node;
	isil_video_packet_tcb_t	*curr_consumer;
	isil_video_packet_tcb_t	*curr_producer;
	spinlock_t	lock;

	struct isil_video_packet_queue_operation	*op;
    };

    struct isil_video_frame_tcb_operation {
	void    (*init)(isil_video_frame_tcb_t *frame);
	void    (*reset)(isil_video_frame_tcb_t *frame);
	void    (*release)(isil_video_frame_tcb_t **ptr_frame, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void    (*reference)(isil_video_frame_tcb_t *src_frame, isil_video_frame_tcb_t **ptr_dest_frame);

	size_t  (*submit)(isil_video_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	int     (*copy)(isil_video_frame_tcb_t *, isil_video_frame_tcb_t *);
    };


#define	to_video_frame_tcb(node)	container_of(node, isil_video_frame_tcb_t, frame_node)
    struct isil_video_frame_tcb{
	spinlock_t	lock;
	atomic_t	ref_count;
	tcb_node_t	frame_node;

	int	        frame_type;
	h264_nal_t  nal;
	int         frame_len;
	int         consumer_frame_offset;
	__u32       timestamp;
	int         duration;
	int         i_init_qp;
	int         i_curr_qp;
	int         i_mb_x;
	int         i_mb_y;
	int         i_log_gop_value;
	int         slice_type;
	int         frame_number;
	int         fps;
	int         frame_is_err;
	u32         in_ddr_addr;
	int         b_is_display;
	int         b_is_gop_end;
	//int         i_rebuild_id;
	int         master_or_sub;
	int         sen_type;
	int         need_large_mbdelay;
	int         encodeSize;
	u32         checksum;

#if (defined(DEBUG_HELPER))
	u32         i_slice_num;
#endif
	ISIL_FRAME_TIME_PAD	time_pad;

	isil_video_packet_tcb_queue_t	video_packet_queue;
	isil_video_chan_buf_pool_t	*video_chan_buf_pool;
	struct isil_video_frame_tcb_operation	*op;
    };

    struct isil_video_frame_queue_operation{
	void	(*get)(isil_video_frame_tcb_queue_t *, isil_video_frame_tcb_t **);
	void	(*try_get)(isil_video_frame_tcb_queue_t *, isil_video_frame_tcb_t **);
	void	(*get_tailer)(isil_video_frame_tcb_queue_t *, isil_video_frame_tcb_t **);
	void	(*try_get_tailer)(isil_video_frame_tcb_queue_t *, isil_video_frame_tcb_t **);
	void	(*put)(isil_video_frame_tcb_queue_t *, isil_video_frame_tcb_t *);
	void	(*put_header)(isil_video_frame_tcb_queue_t *, isil_video_frame_tcb_t *);

	void	(*get_curr_producer_from_pool)(isil_video_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*try_get_curr_producer_from_pool)(isil_video_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*put_curr_producer_into_queue)(isil_video_frame_tcb_queue_t *);
	void	(*release_curr_producer)(isil_video_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*get_curr_consumer_from_queue)(isil_video_frame_tcb_queue_t *);
	void	(*try_get_curr_consumer_from_queue)(isil_video_frame_tcb_queue_t *);
	void	(*release_curr_consumer)(isil_video_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void    (*put_curr_consumer_into_queue_header)(isil_video_frame_tcb_queue_t *);

	int     (*get_curr_queue_entry_number)(isil_video_frame_tcb_queue_t *);
	void	(*release)(isil_video_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*init)(isil_video_frame_tcb_queue_t *);
    };

    struct isil_video_frame_tcb_queue{
	u32	start_timestamp, total_duration;
	tcb_node_queue_t	queue_node;
	isil_video_frame_tcb_t	*curr_consumer;
	isil_video_frame_tcb_t	*curr_producer;
	spinlock_t	lock;

	struct isil_video_frame_queue_operation	*op;
    };

#ifdef  MV_MODULE
    struct isil_video_mv_packet_tcb_operation{
	void    (*init)(isil_video_mv_packet_tcb_t *packet, isil_video_chan_buf_pool_t *pool, int id);
	void    (*reset)(isil_video_mv_packet_tcb_t *packet);
	void    (*release)(isil_video_mv_packet_tcb_t **ptr_packet, isil_video_chan_buf_pool_t *pool);
	void    (*reference)(isil_video_mv_packet_tcb_t *src_packet, isil_video_mv_packet_tcb_t **ptr_dest_packet);

	void    (*dma_map)(isil_video_mv_packet_tcb_t *packet, enum dma_data_direction direction);
	void    (*dma_unmap)(isil_video_mv_packet_tcb_t *packet, enum dma_data_direction direction);

	size_t  (*submit)(isil_video_mv_packet_tcb_t *packet, char __user *data, size_t count, loff_t *ppos);
    };

#define	to_get_isil_video_mv_packet_tcb(node)	container_of(node, isil_video_mv_packet_tcb_t, mv_packet_node)
    struct isil_video_mv_packet_tcb{
	spinlock_t	lock;
	atomic_t	ref_count;
	tcb_node_t	mv_packet_node;

	__u8		*data;
	__u32		data_size;
	__u8		*mvVector_buf;
	__u32		mvVector_len;
	dma_addr_t	dma_addr;

	struct isil_video_mv_packet_tcb_operation	*op;
    };

    struct isil_video_mv_packet_tcb_queue_operation{
	void	(*get)(isil_video_mv_packet_tcb_queue_t *, isil_video_mv_packet_tcb_t **);
	void	(*try_get)(isil_video_mv_packet_tcb_queue_t *, isil_video_mv_packet_tcb_t **);
	void	(*get_tailer)(isil_video_mv_packet_tcb_queue_t *, isil_video_mv_packet_tcb_t **);
	void	(*try_get_tailer)(isil_video_mv_packet_tcb_queue_t *, isil_video_mv_packet_tcb_t **);
	void	(*put)(isil_video_mv_packet_tcb_queue_t *, isil_video_mv_packet_tcb_t *);
	void	(*put_header)(isil_video_mv_packet_tcb_queue_t *, isil_video_mv_packet_tcb_t *);

	void	(*get_curr_producer_from_pool)(isil_video_mv_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*try_get_curr_producer_from_pool)(isil_video_mv_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*put_curr_producer_into_queue)(isil_video_mv_packet_tcb_queue_t *);
	void	(*release_curr_producer)(isil_video_mv_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*get_curr_consumer_from_queue)(isil_video_mv_packet_tcb_queue_t *);
	void	(*try_get_curr_consumer_from_queue)(isil_video_mv_packet_tcb_queue_t *);
	void	(*release_curr_consumer)(isil_video_mv_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);

	int     (*get_curr_queue_entry_number)(isil_video_mv_packet_tcb_queue_t *);
	void	(*release)(isil_video_mv_packet_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*init)(isil_video_mv_packet_tcb_queue_t *);
    };

    struct isil_video_mv_packet_tcb_queue{
	u32	start_timestamp, total_duration;
	tcb_node_queue_t	queue_node;
	isil_video_mv_packet_tcb_t	*curr_consumer;
	isil_video_mv_packet_tcb_t	*curr_producer;
	spinlock_t	lock;

	struct isil_video_mv_packet_tcb_queue_operation	*op;
    };

    struct isil_video_mv_frame_tcb_operation{
	void    (*init)(isil_video_mv_frame_tcb_t *frame);
	void    (*reset)(isil_video_mv_frame_tcb_t *frame);
	void    (*release)(isil_video_mv_frame_tcb_t **ptr_frame, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void    (*reference)(isil_video_mv_frame_tcb_t *src_frame, isil_video_mv_frame_tcb_t **ptr_dest_frame);

	size_t  (*submit)(isil_video_mv_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_video_chan_buf_pool_t *video_chan_buf_pool);
    };

#define	to_get_isil_video_mv_frame_tcb(node)	container_of(node, isil_video_mv_frame_tcb_t, mv_frame_node)
    struct isil_video_mv_frame_tcb{
	spinlock_t	lock;
	atomic_t	ref_count;
	tcb_node_t	mv_frame_node;
	__u8    mvFlag_buf[MotionVectorBitFlag_BUF_LEN];
	__u32   mvFlag_len;
	__u32   mvBuf_payLoadLen;
	__u32   timestamp;
	int     frame_number;
	int     frame_is_err;

	isil_video_mv_packet_tcb_queue_t	mv_queue;
	isil_video_chan_buf_pool_t	*video_chan_buf_pool;
	struct isil_video_mv_frame_tcb_operation	*op;
    };

    struct isil_video_mv_frame_tcb_queue_operation{
	void	(*get)(isil_video_mv_frame_tcb_queue_t *, isil_video_mv_frame_tcb_t **);
	void	(*try_get)(isil_video_mv_frame_tcb_queue_t *, isil_video_mv_frame_tcb_t **);
	void	(*get_tailer)(isil_video_mv_frame_tcb_queue_t *, isil_video_mv_frame_tcb_t **);
	void	(*try_get_tailer)(isil_video_mv_frame_tcb_queue_t *, isil_video_mv_frame_tcb_t **);
	void	(*put)(isil_video_mv_frame_tcb_queue_t *, isil_video_mv_frame_tcb_t *);
	void	(*put_header)(isil_video_mv_frame_tcb_queue_t *, isil_video_mv_frame_tcb_t *);

	void	(*get_curr_producer_from_pool)(isil_video_mv_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*try_get_curr_producer_from_pool)(isil_video_mv_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*put_curr_producer_into_queue)(isil_video_mv_frame_tcb_queue_t *);
	void	(*release_curr_producer)(isil_video_mv_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*get_curr_consumer_from_queue)(isil_video_mv_frame_tcb_queue_t *);
	void	(*try_get_curr_consumer_from_queue)(isil_video_mv_frame_tcb_queue_t *);
	void	(*release_curr_consumer)(isil_video_mv_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);

	int		(*get_curr_queue_entry_number)(isil_video_mv_frame_tcb_queue_t *);
	void	(*release)(isil_video_mv_frame_tcb_queue_t *, isil_video_chan_buf_pool_t *video_chan_buf_pool);
	void	(*init)(isil_video_mv_frame_tcb_queue_t *);
    };

    struct isil_video_mv_frame_tcb_queue{
	u32	start_timestamp, total_duration;
	tcb_node_queue_t	queue_node;
	isil_video_mv_frame_tcb_t	*curr_consumer;
	isil_video_mv_frame_tcb_t	*curr_producer;
	spinlock_t	lock;

	struct isil_video_mv_frame_tcb_queue_operation	*op;
    };
#endif

    extern void	init_isil_video_packet_tcb_queue(isil_video_packet_tcb_queue_t *video_packet_queue);
    extern void	remove_isil_video_packet_tcb_queue(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool);
    extern void	init_isil_video_frame_tcb_queue(isil_video_frame_tcb_queue_t *video_frame_tcb_queue);
    extern void	remove_isil_video_frame_tcb_queue(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool);
#ifdef  MV_MODULE
    extern void	init_isil_video_mv_packet_tcb_queue(isil_video_mv_packet_tcb_queue_t *video_mv_packet_queue);
    extern void	remove_isil_video_mv_packet_tcb_queue(isil_video_mv_packet_tcb_queue_t *video_mv_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool);
    extern void	init_isil_video_mv_frame_tcb_queue(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue);
    extern void	remove_isil_video_mv_frame_tcb_queue(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool);
#endif
    extern int	init_isil_video_chan_buf_pool(isil_video_chan_buf_pool_t *video_chan_buf_pool, unsigned long buf_len);
    extern void	remove_isil_video_chan_buf_pool(isil_video_chan_buf_pool_t *video_chan_buf_pool);

#ifdef __cplusplus
}
#endif


#endif	//ISIL_H264_VIDEO_BUF_H

