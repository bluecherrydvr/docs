/* isil_vb_common.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright Techwell inc., 2010
 * Author: Ruixian Chen <rxchen@techwellinc.com.cn>
 *
 * vb buffer management
 */

#ifndef	_ISIL_VB_COMMON_H__
#define	_ISIL_VB_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct isil_vb_pool	    isil_vb_pool_t;

    typedef struct isil_vb_packet_tcb  isil_vb_packet_tcb_t;
    typedef struct isil_vb_packet_tcb_queue isil_vb_packet_tcb_queue_t;

    typedef struct isil_vb_frame_tcb isil_vb_frame_tcb_t;
    typedef struct isil_vb_frame_tcb_queue isil_vb_frame_tcb_queue_t;

    struct isil_vb_pool_operation{
	int (*create)(isil_vb_pool_t *pool, unsigned long size);
	void (*release)(isil_vb_pool_t *pool);

	void (*get_packet_tcb)(isil_vb_pool_t *pool, isil_vb_packet_tcb_t **packet);
	void (*try_get_packet_tcb)(isil_vb_pool_t *pool, isil_vb_packet_tcb_t **packet);
	void (*put_packet_tcb)(isil_vb_pool_t *pool, isil_vb_packet_tcb_t *packet);
	int	 (*get_packet_tcb_pool_entry_number)(isil_vb_pool_t *pool);

	void (*get_frame_tcb)(isil_vb_pool_t *pool, isil_vb_frame_tcb_t **frame);
	void (*try_get_frame_tcb)(isil_vb_pool_t *pool, isil_vb_frame_tcb_t **frame);
	void (*put_frame_tcb)(isil_vb_pool_t *pool, isil_vb_frame_tcb_t *frame);
	int  (*get_frame_tcb_pool_entry_number)(isil_vb_pool_t *pool);
    };

    struct isil_vb_pool{
	int	cache_order;
	unsigned char    *cache_buffer;

	unsigned long    vb_packet_buffer_size;
	unsigned long    vb_packet_entry_number;
	tcb_node_pool_t  vb_packet_pool_tcb;
	isil_vb_packet_tcb_t *vb_packet_entry_pool;

	unsigned long    vb_frame_entry_number;
	tcb_node_pool_t  vb_frame_pool_tcb;
	isil_vb_frame_tcb_t *vb_frame_entry_pool;

	struct isil_vb_pool_operation *op;
    };

    struct isil_vb_packet_operation
    {
	void	(*init)(isil_vb_packet_tcb_t *packet, isil_vb_pool_t *pool, int id);
	void	(*reset)(isil_vb_packet_tcb_t *packet);
	void	(*release)(isil_vb_packet_tcb_t **packet, isil_vb_pool_t *pool);
	void	(*reference)(isil_vb_packet_tcb_t *src_packet, isil_vb_packet_tcb_t **ptr_dest_packet);

	void	(*dma_map)(isil_vb_packet_tcb_t *packet, enum dma_data_direction direction);
	void	(*dma_unmap)(isil_vb_packet_tcb_t *packet, enum dma_data_direction direction);

	size_t  (*submit)(isil_vb_packet_tcb_t *packet, char __user *data, size_t count, loff_t *ppos, insert_03_state_machine_t *machine);
    };

#define	to_vb_packet_tcb(node)	container_of(node, isil_vb_packet_tcb_t, packet_node)
    struct isil_vb_packet_tcb
    {
	atomic_t	ref_count;
	spinlock_t	lock;
	tcb_node_t	packet_node;
	int		packet_id;

	int	packet_size;
	int	payload_len;
	int	consumer_len;
	int	id_of_packet_queue;

	u8	*data;
	dma_addr_t	dma_addr;

	struct isil_vb_packet_operation	*op;
    };

    struct isil_vb_packet_queue_operation{
	void	(*get)(isil_vb_packet_tcb_queue_t *, isil_vb_packet_tcb_t **);
	void	(*try_get)(isil_vb_packet_tcb_queue_t *, isil_vb_packet_tcb_t **);
	void	(*get_tailer)(isil_vb_packet_tcb_queue_t *, isil_vb_packet_tcb_t **);
	void	(*try_get_tailer)(isil_vb_packet_tcb_queue_t *, isil_vb_packet_tcb_t **);
	void	(*put)(isil_vb_packet_tcb_queue_t *, isil_vb_packet_tcb_t *);
	void	(*put_header)(isil_vb_packet_tcb_queue_t *, isil_vb_packet_tcb_t *);

	void	(*get_curr_producer_from_pool)(isil_vb_packet_tcb_queue_t *, isil_vb_pool_t *pool);
	void	(*try_get_curr_producer_from_pool)(isil_vb_packet_tcb_queue_t *, isil_vb_pool_t *pool);
	void	(*put_curr_producer_into_queue)(isil_vb_packet_tcb_queue_t *);
	void	(*release_curr_producer)(isil_vb_packet_tcb_queue_t *, isil_vb_pool_t *pool);
	void	(*get_curr_consumer_from_queue)(isil_vb_packet_tcb_queue_t *);
	void	(*try_get_curr_consumer_from_queue)(isil_vb_packet_tcb_queue_t *);
	void	(*release_curr_consumer)(isil_vb_packet_tcb_queue_t *, isil_vb_pool_t *pool);

	int     (*get_curr_queue_entry_number)(isil_vb_packet_tcb_queue_t *);
	void	(*release)(isil_vb_packet_tcb_queue_t *, isil_vb_pool_t *pool);
	void	(*init)(isil_vb_packet_tcb_queue_t *);
    };

    struct isil_vb_packet_tcb_queue{
	u32	start_timestamp, total_duration;
	tcb_node_queue_t	queue_node;
	isil_vb_packet_tcb_t	*curr_consumer;
	isil_vb_packet_tcb_t	*curr_producer;
	spinlock_t	lock;

	struct isil_vb_packet_queue_operation	*op;
    };

    struct isil_vb_frame_tcb_operation {
	void (*init)(isil_vb_frame_tcb_t *frame);
	void (*reset)(isil_vb_frame_tcb_t *frame);
	void (*release)(isil_vb_frame_tcb_t **ptr_frame, isil_vb_pool_t *pool);
	void (*reference)(isil_vb_frame_tcb_t *src_frame, isil_vb_frame_tcb_t **ptr_dest_frame);

	size_t (*submit)(isil_vb_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_vb_pool_t *pool);
    };

#define	to_vb_frame_tcb(node)	container_of(node, isil_vb_frame_tcb_t, frame_node)
    struct isil_vb_frame_tcb{
	spinlock_t	lock;
	atomic_t	ref_count;
	tcb_node_t	frame_node;

	int	        frame_type;
	h264_nal_t  nal;
	int         frame_len;
	int         consumer_frame_offset;
	__u32       timestamp;
	int         i_init_qp;
	int         i_curr_qp;
	int         i_mb_x;
	int         i_mb_y;
	int         frame_number;
	int         frame_is_err;
	u32         frame_flags;

	isil_vb_packet_tcb_queue_t	vb_packet_queue;
	isil_vb_pool_t	*pool;
	struct isil_vb_frame_tcb_operation *op;
    };

    struct isil_vb_frame_queue_operation{
	/*get frame tcb in block*/
	void	(*get)(isil_vb_frame_tcb_queue_t *, isil_vb_frame_tcb_t **);
	void	(*try_get)(isil_vb_frame_tcb_queue_t *, isil_vb_frame_tcb_t **);
	void	(*get_tailer)(isil_vb_frame_tcb_queue_t *, isil_vb_frame_tcb_t **);
	void	(*try_get_tailer)(isil_vb_frame_tcb_queue_t *, isil_vb_frame_tcb_t **);
	void	(*put)(isil_vb_frame_tcb_queue_t *, isil_vb_frame_tcb_t *);
	void	(*put_header)(isil_vb_frame_tcb_queue_t *, isil_vb_frame_tcb_t *);

	void	(*get_curr_producer_from_pool)(isil_vb_frame_tcb_queue_t *, isil_vb_pool_t *pool);
	void	(*try_get_curr_producer_from_pool)(isil_vb_frame_tcb_queue_t *, isil_vb_pool_t *pool);
	void	(*put_curr_producer_into_queue)(isil_vb_frame_tcb_queue_t *);
	void	(*release_curr_producer)(isil_vb_frame_tcb_queue_t *, isil_vb_pool_t *pool);
	void	(*get_curr_consumer_from_queue)(isil_vb_frame_tcb_queue_t *);
	void	(*try_get_curr_consumer_from_queue)(isil_vb_frame_tcb_queue_t *);
	void	(*release_curr_consumer)(isil_vb_frame_tcb_queue_t *, isil_vb_pool_t *pool);

	int	(*get_curr_queue_entry_number)(isil_vb_frame_tcb_queue_t *);
	void	(*release)(isil_vb_frame_tcb_queue_t *, isil_vb_pool_t *pool);
	void	(*init)(isil_vb_frame_tcb_queue_t *);
    };

    struct isil_vb_frame_tcb_queue{
	u32	start_timestamp, total_duration;
	tcb_node_queue_t	queue_node;
	isil_vb_frame_tcb_t	*curr_consumer;
	isil_vb_frame_tcb_t	*curr_producer;
	spinlock_t	lock;

	struct isil_vb_frame_queue_operation	*op;
    };

    void	isil_create_packet_tcb_queue(isil_vb_packet_tcb_queue_t *vb_packet_queue);
    void	isil_destroy_packet_tcb_queue(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_pool_t *pool);

    void	isil_create_frame_tcb_queue(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue);
    void	isil_destroy_frame_tcb_queue(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_pool_t *pool);

    int	isil_create_pool(isil_vb_pool_t *pool, unsigned long buf_len);
    void	isil_destroy_pool(isil_vb_pool_t *pool);

#ifdef __cplusplus
}
#endif

#endif	//_ISIL_VB_COMMON_H__

