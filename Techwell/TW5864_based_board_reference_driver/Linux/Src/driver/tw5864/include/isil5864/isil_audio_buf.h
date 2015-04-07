#ifndef	ISIL_AUDIO_BUF_H
#define	ISIL_AUDIO_BUF_H

#ifdef __cplusplus
extern "C"
{
#endif

#define AUDIO_CHAN_BUF_POOL_LEN 	(0x10000)	//64KB

#define	DeltaT_8K_16BIT_1ms_LEN		(16)
#define	DeltaT_16K_16BIT_1ms_LEN	(32)
#define	DeltaT_8K_8BIT_1ms_LEN		(8)
#define	DeltaT_16K_8BIT_1ms_LEN		(16)

    struct isil_audio_chan_buf_pool_operation{
	int     (*create)(isil_audio_chan_buf_pool_t *, unsigned long );
	void    (*release)(isil_audio_chan_buf_pool_t *);

	void    (*get_audio_section_tcb)(isil_audio_chan_buf_pool_t *, isil_audio_packet_section_t **);
	void    (*try_get_audio_section_tcb)(isil_audio_chan_buf_pool_t *, isil_audio_packet_section_t **);
	void    (*put_audio_section_tcb)(isil_audio_chan_buf_pool_t *, isil_audio_packet_section_t *);
	int     (*get_audio_section_tcb_pool_entry_number)(isil_audio_chan_buf_pool_t *);
    };

    struct isil_audio_chan_buf_pool{
	int audio_buffer_entry_order;
	u8  *audio_buffer_cache;

	u32 audio_section_tcb_number;
	tcb_node_pool_t pool_node;
	isil_audio_packet_section_t   *audio_section_tcb_cache;

	struct isil_audio_chan_buf_pool_operation *op;
    };

    struct isil_audio_packet_section_operation{
	void    (*init)(isil_audio_packet_section_t *audio_packet_section, isil_audio_chan_buf_pool_t *audio_chan_buf_pool, int id);
	void    (*reset)(isil_audio_packet_section_t *audio_packet_section);
	void    (*release)(isil_audio_packet_section_t **ptr_audio_packet_section, isil_audio_chan_buf_pool_t *audio_chan_buf_pool);
	void    (*reference)(isil_audio_packet_section_t *src_audio_packet_section, isil_audio_packet_section_t **dest_audio_packet_section);
	void    (*dma_map)(isil_audio_packet_section_t *audio_packet_section, enum dma_data_direction direction);
	void    (*dma_unmap)(isil_audio_packet_section_t *audio_packet_section, enum dma_data_direction direction);
	size_t  (*submit)(isil_audio_packet_section_t *, char __user *, size_t , loff_t *, int , isil_audio_chan_buf_pool_t *);
	int     (*copy)(isil_audio_packet_section_t *, isil_audio_packet_section_t *);
    };

#define	to_get_isil_audio_packet_section(node)	container_of(node, isil_audio_packet_section_t, packet_node)
    struct isil_audio_packet_section{
	atomic_t	ref_count;
	tcb_node_t	packet_node;
	spinlock_t	lock;

	short   id;
	short   section_size;
	short   payload_len;
	short   sample_rate;
	short   type;
	short   bit_wide;
	short   consumer_offset;
	u32     frameSerial;
	__u32   timestamp;
	isil_audio_descriptor_t   descriptor;
	ISIL_FRAME_TIME_PAD	time_pad;

	u8	*data;
	dma_addr_t  dma_addr;

	isil_audio_chan_buf_pool_t    *audio_chan_buf_pool;
	struct isil_audio_packet_section_operation    *op;
    };

    struct isil_audio_packet_queue_operation{
	void    (*get)(isil_audio_packet_queue_t *, isil_audio_packet_section_t **);
	void    (*try_get)(isil_audio_packet_queue_t *, isil_audio_packet_section_t **);
	void    (*get_tailer)(isil_audio_packet_queue_t *, isil_audio_packet_section_t **);
	void    (*try_get_tailer)(isil_audio_packet_queue_t *, isil_audio_packet_section_t **);
	void    (*put)(isil_audio_packet_queue_t *, isil_audio_packet_section_t *);
	void    (*put_header)(isil_audio_packet_queue_t *, isil_audio_packet_section_t *);

	void    (*get_curr_producer_from_pool)(isil_audio_packet_queue_t *, isil_audio_chan_buf_pool_t *audio_chan_buf_pool);
	void    (*try_get_curr_producer_from_pool)(isil_audio_packet_queue_t *, isil_audio_chan_buf_pool_t *audio_chan_buf_pool);
	void    (*put_curr_producer_into_queue)(isil_audio_packet_queue_t *);
	void    (*release_curr_producer)(isil_audio_packet_queue_t *, isil_audio_chan_buf_pool_t *audio_chan_buf_pool);
	void    (*get_curr_consumer_from_queue)(isil_audio_packet_queue_t *);
	void    (*try_get_curr_consumer_from_queue)(isil_audio_packet_queue_t *);
	void    (*release_curr_consumer)(isil_audio_packet_queue_t *, isil_audio_chan_buf_pool_t *audio_chan_buf_pool);

	int     (*get_curr_queue_entry_number)(isil_audio_packet_queue_t *);
	void    (*release)(isil_audio_packet_queue_t *, isil_audio_chan_buf_pool_t *audio_chan_buf_pool);
	void    (*init)(isil_audio_packet_queue_t *);
    };

    struct isil_audio_packet_queue{
	u32	start_timestamp, total_duration;
	tcb_node_queue_t	queue_node;
	isil_audio_packet_section_t	*curr_consumer;
	isil_audio_packet_section_t	*curr_producer;
	spinlock_t	lock;

	struct isil_audio_packet_queue_operation	*op;
    };


    extern void	init_isil_audio_packet_queue(isil_audio_packet_queue_t *audio_packet_queue);
    extern void	remove_isil_audio_packet_queue(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_chan_buf_pool_t *audio_chan_buf_pool);
    extern int	init_audio_chan_buf_pool(isil_audio_chan_buf_pool_t *audio_chan_buf_pool);
    extern void	remove_audio_chan_buf_pool(isil_audio_chan_buf_pool_t	*audio_chan_buf_pool);

#ifdef __cplusplus
}
#endif


#endif

