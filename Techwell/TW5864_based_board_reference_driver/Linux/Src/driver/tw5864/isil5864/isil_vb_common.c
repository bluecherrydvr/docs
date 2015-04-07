#include	<isil5864/isil_common.h>

static void	isil_vb_packet_tcb_init(isil_vb_packet_tcb_t *vb_packet_tcb, isil_vb_pool_t *pool, int id)
{
    if((vb_packet_tcb!=NULL) && (pool!=NULL)){
        tcb_node_t	*node = &vb_packet_tcb->packet_node;
        spin_lock_init(&vb_packet_tcb->lock);
        atomic_set(&vb_packet_tcb->ref_count, 0);
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, vb_packet_tcb);
        vb_packet_tcb->packet_id = id;
        vb_packet_tcb->packet_size = pool->vb_packet_buffer_size;
        vb_packet_tcb->payload_len = 0;
        vb_packet_tcb->consumer_len = 0;
#ifdef  USE_DISCRETE_BUF
        vb_packet_tcb->data = (u8*)__get_free_page(GFP_KERNEL);
        if(vb_packet_tcb->data == NULL) {
            while(1) {
                printk("%s.%d: can't get one page, only for debug\n", __FUNCTION__, __LINE__);
            }
        }
#else
        vb_packet_tcb->data = &pool->cache_buffer[pool->vb_packet_buffer_size*id];
#endif
        vb_packet_tcb->dma_addr = 0;
        vb_packet_tcb->id_of_packet_queue = 0;
    }
}

static void	isil_vb_packet_tcb_reset(isil_vb_packet_tcb_t *vb_packet_tcb)
{
    if(vb_packet_tcb!=NULL){
        atomic_set(&vb_packet_tcb->ref_count, 0);
        vb_packet_tcb->payload_len = 0;
        vb_packet_tcb->consumer_len = 0;
        vb_packet_tcb->dma_addr = 0;
        vb_packet_tcb->id_of_packet_queue = 0;
    }
}

static void	isil_vb_packet_tcb_release(isil_vb_packet_tcb_t **ptr_vb_packet_tcb, isil_vb_pool_t *pool)
{
    if((*ptr_vb_packet_tcb!=NULL) && (pool!=NULL)){
        isil_vb_packet_tcb_t	*vb_packet_tcb = *ptr_vb_packet_tcb;
        unsigned long	flags;

        spin_lock_irqsave(&vb_packet_tcb->lock, flags);
        if(atomic_read(&vb_packet_tcb->ref_count) > 1){
            atomic_dec(&vb_packet_tcb->ref_count);
        } else {
            tcb_node_t	*node;
            if(vb_packet_tcb->op != NULL){
                vb_packet_tcb->op->reset(vb_packet_tcb);
            }
            node = &vb_packet_tcb->packet_node;
            if(node->op != NULL){
                node->op->release(node, &pool->vb_packet_pool_tcb);
            }
        }
        *ptr_vb_packet_tcb = NULL;
        spin_unlock_irqrestore(&vb_packet_tcb->lock, flags);
    }
}

static void	isil_vb_packet_tcb_reference(isil_vb_packet_tcb_t *src_vb_packet_tcb, isil_vb_packet_tcb_t **ptr_dest_vb_packet_tcb)
{
    if((src_vb_packet_tcb!=NULL) && (ptr_dest_vb_packet_tcb!=NULL)){
        unsigned long	flags;

        spin_lock_irqsave(&src_vb_packet_tcb->lock, flags);
        atomic_inc(&src_vb_packet_tcb->ref_count);
        *ptr_dest_vb_packet_tcb = src_vb_packet_tcb;
        spin_unlock_irqrestore(&src_vb_packet_tcb->lock, flags);
    }
}

static void	isil_vb_packet_tcb_dma_map(isil_vb_packet_tcb_t *vb_packet_tcb, enum dma_data_direction direction)
{
    if(vb_packet_tcb != NULL) {
        if(vb_packet_tcb->data != NULL) {
            vb_packet_tcb->dma_addr = dma_map_single(NULL, vb_packet_tcb->data, vb_packet_tcb->packet_size, direction);
            if (vb_packet_tcb->dma_addr == 0){
                printk("%s.%d err\n", __FUNCTION__, __LINE__);
            }
        }
    }
}

static void	isil_vb_packet_tcb_dma_unmap(isil_vb_packet_tcb_t *vb_packet_tcb, enum dma_data_direction direction)
{
    if(vb_packet_tcb != NULL) {
        if (vb_packet_tcb->dma_addr != 0) {
            dma_unmap_single(NULL, vb_packet_tcb->dma_addr, vb_packet_tcb->packet_size, direction);
            vb_packet_tcb->dma_addr = 0;
        }
    }
}

static size_t	isil_vb_packet_tcb_submit(isil_vb_packet_tcb_t *packet, char __user *data, size_t count, loff_t *ppos, insert_03_state_machine_t *machine)
{
    loff_t  buf_offset = 0;
    char __user	*dest_data_ptr;
    char	*src_data_ptr;
    int ret = 0;
    if(packet != NULL){
        machine->transfer = packet->payload_len - packet->consumer_len;
        src_data_ptr = packet->data + packet->consumer_len;
        dest_data_ptr = data;

        ret = copy_to_user(dest_data_ptr, src_data_ptr, machine->transfer);
        if(ret != 0){
            printk("%s, %d: copy_to_user failed, left bytes %d\n", __FUNCTION__, __LINE__, ret);
        }
        buf_offset = machine->transfer;
    }
    *ppos += buf_offset;
    return (size_t)buf_offset;
}

static	struct isil_vb_packet_operation	isil_vb_packet_tcb_op = {
    .init = isil_vb_packet_tcb_init,
    .reset = isil_vb_packet_tcb_reset,
    .release = isil_vb_packet_tcb_release,
    .reference = isil_vb_packet_tcb_reference,

    .dma_map = isil_vb_packet_tcb_dma_map,
    .dma_unmap = isil_vb_packet_tcb_dma_unmap,

    .submit = isil_vb_packet_tcb_submit,
};

static void	isil_vb_packet_queue_get(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_packet_tcb_t **ptr_vb_packet_tcb)
{
    if((vb_packet_queue!=NULL) && (ptr_vb_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_vb_packet_tcb = NULL;
        queue = &vb_packet_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_vb_packet_tcb = to_vb_packet_tcb(temp_node);
            }
        }
    }
}

static void	isil_vb_packet_queue_try_get(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_packet_tcb_t **ptr_vb_packet_tcb)
{
    if((vb_packet_queue!=NULL) && (ptr_vb_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_vb_packet_tcb = NULL;
        queue = &vb_packet_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_vb_packet_tcb = to_vb_packet_tcb(temp_node);
            }
        }
    }
}

static void	isil_vb_packet_queue_get_tailer(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_packet_tcb_t **ptr_vb_packet_tcb)
{
    if((vb_packet_queue!=NULL) && (ptr_vb_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_vb_packet_tcb = NULL;
        queue = &vb_packet_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_vb_packet_tcb = to_vb_packet_tcb(temp_node);
            }
        }
    }
}

static void	isil_vb_packet_queue_try_get_tailer(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_packet_tcb_t **ptr_vb_packet_tcb)
{
    if((vb_packet_queue!=NULL) && (ptr_vb_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_vb_packet_tcb = NULL;
        queue = &vb_packet_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_vb_packet_tcb = to_vb_packet_tcb(temp_node);
            }
        }
    }
}

static void	isil_vb_packet_queue_put(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_packet_tcb_t *vb_packet_tcb)
{
    if((vb_packet_queue!=NULL) && (vb_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &vb_packet_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put(queue, &vb_packet_tcb->packet_node);
            vb_packet_tcb->id_of_packet_queue = queue->op->get_queue_curr_entry_number(queue);
        }
    }
}

static void	isil_vb_packet_queue_put_header(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_packet_tcb_t *vb_packet_tcb)
{
    if((vb_packet_queue!=NULL) && (vb_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &vb_packet_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put_header(queue, &vb_packet_tcb->packet_node);
        }
    }
}

static void	isil_vb_packet_queue_get_curr_producer_from_pool(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_pool_t *pool)
{
    if((vb_packet_queue!=NULL) && (pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&vb_packet_queue->lock, flags);
        if(vb_packet_queue->curr_producer == NULL){
            spin_unlock_irqrestore(&vb_packet_queue->lock, flags);
            if(pool->op != NULL){
                pool->op->get_packet_tcb(pool, &vb_packet_queue->curr_producer);
            }
            //if(vb_packet_queue->curr_producer != NULL){
            //vb_packet_queue->curr_producer->op->dma_map(vb_packet_queue->curr_producer, DMA_FROM_DEVICE);
            //}
            spin_lock_irqsave(&vb_packet_queue->lock, flags);
        }
        spin_unlock_irqrestore(&vb_packet_queue->lock, flags);
    }
}

static void	isil_vb_packet_queue_try_get_curr_producer_from_pool(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_pool_t *pool)
{
    if((vb_packet_queue!=NULL) && (pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&vb_packet_queue->lock, flags);
        if(vb_packet_queue->curr_producer == NULL){
            if(pool->op != NULL){
                pool->op->try_get_packet_tcb(pool, &vb_packet_queue->curr_producer);
            }
            //if(vb_packet_queue->curr_producer != NULL){
            //vb_packet_queue->curr_producer->op->dma_map(vb_packet_queue->curr_producer, DMA_FROM_DEVICE);
            //}
        }
        spin_unlock_irqrestore(&vb_packet_queue->lock, flags);
        //printk("%s: %d\n", __FUNCTION__, pool->op->get_packet_tcb_pool_entry_number(pool));
    }
}

static void	isil_vb_packet_queue_put_curr_producer_into_queue(isil_vb_packet_tcb_queue_t *vb_packet_queue)
{
    if(vb_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&vb_packet_queue->lock, flags);
        if(vb_packet_queue->curr_producer != NULL){
            //vb_packet_queue->curr_producer->op->dma_unmap(vb_packet_queue->curr_producer, DMA_FROM_DEVICE);
            if(vb_packet_queue->op != NULL){
                vb_packet_queue->op->put(vb_packet_queue, vb_packet_queue->curr_producer);
            } else {
                isil_vb_packet_queue_put(vb_packet_queue, vb_packet_queue->curr_producer);
            }
            vb_packet_queue->curr_producer = NULL;
        }
        spin_unlock_irqrestore(&vb_packet_queue->lock, flags);
    }
}

static void	isil_vb_packet_queue_release_curr_producer(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_pool_t *pool)
{
    if((vb_packet_queue!=NULL) && (pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&vb_packet_queue->lock, flags);
        if(vb_packet_queue->curr_producer != NULL){
            if(vb_packet_queue->curr_producer->op != NULL){
                vb_packet_queue->curr_producer->op->release(&vb_packet_queue->curr_producer, pool);
            } else {
                isil_vb_packet_tcb_release(&vb_packet_queue->curr_producer, pool);
            }
        }
        spin_unlock_irqrestore(&vb_packet_queue->lock, flags);
    }
}

static void	isil_vb_packet_queue_get_curr_consumer_from_queue(isil_vb_packet_tcb_queue_t *vb_packet_queue)
{
    if(vb_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&vb_packet_queue->lock, flags);
        if(vb_packet_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&vb_packet_queue->lock, flags);
            if(vb_packet_queue->op != NULL){
                vb_packet_queue->op->get(vb_packet_queue, &vb_packet_queue->curr_consumer);
            } else {
                isil_vb_packet_queue_get(vb_packet_queue, &vb_packet_queue->curr_consumer);
            }
            spin_lock_irqsave(&vb_packet_queue->lock, flags);
        }
        spin_unlock_irqrestore(&vb_packet_queue->lock, flags);
    }
}

static void	isil_vb_packet_queue_try_get_curr_consumer_from_queue(isil_vb_packet_tcb_queue_t *vb_packet_queue)
{
    if(vb_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&vb_packet_queue->lock, flags);
        if(vb_packet_queue->curr_consumer == NULL){
            if(vb_packet_queue->op != NULL){
                vb_packet_queue->op->try_get(vb_packet_queue, &vb_packet_queue->curr_consumer);
            } else {
                isil_vb_packet_queue_try_get(vb_packet_queue, &vb_packet_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&vb_packet_queue->lock, flags);
    }
}

static void	isil_vb_packet_queue_release_curr_consumer(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_pool_t *pool)
{
    if((vb_packet_queue!=NULL) && (pool!=NULL)){
        unsigned long	flags;

        //printk("%s: %d\n", __FUNCTION__, pool->op->get_packet_tcb_pool_entry_number(pool));
        spin_lock_irqsave(&vb_packet_queue->lock, flags);
        if(vb_packet_queue->curr_consumer != NULL){
            if(vb_packet_queue->curr_consumer->op != NULL){
                vb_packet_queue->curr_consumer->op->release(&vb_packet_queue->curr_consumer, pool);
            } else {
                isil_vb_packet_tcb_release(&vb_packet_queue->curr_consumer, pool);
            }
        }
        spin_unlock_irqrestore(&vb_packet_queue->lock, flags);

        //printk("%s: %d\n", __FUNCTION__, pool->op->get_packet_tcb_pool_entry_number(pool));
    }
}

static int	isil_vb_packet_queue_get_curr_queue_entry_number(isil_vb_packet_tcb_queue_t *vb_packet_queue)
{
    int	entry_number = 0;
    if(vb_packet_queue!=NULL){
        tcb_node_queue_t	*queue;
        queue = &vb_packet_queue->queue_node;
        if(queue->op != NULL){
            entry_number = queue->op->get_queue_curr_entry_number(queue);
        }
    }
    return entry_number;
}

static void	isil_vb_packet_queue_release(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_pool_t *pool)
{
    if((vb_packet_queue!=NULL) && (pool!=NULL)){
        isil_destroy_packet_tcb_queue(vb_packet_queue, pool);
    }
}

static void	isil_vb_packet_queue_init(isil_vb_packet_tcb_queue_t *vb_packet_queue)
{
    if(vb_packet_queue != NULL){
        tcb_node_queue_t	*queue;
        queue = &vb_packet_queue->queue_node;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);
        vb_packet_queue->curr_consumer = NULL;
        vb_packet_queue->curr_producer = NULL;
        vb_packet_queue->start_timestamp = 0;
        vb_packet_queue->total_duration = 0;
        spin_lock_init(&vb_packet_queue->lock);
    }
}

static struct isil_vb_packet_queue_operation	isil_vb_packet_queue_op = {
    .get = isil_vb_packet_queue_get,
    .try_get = isil_vb_packet_queue_try_get,
    .get_tailer = isil_vb_packet_queue_get_tailer,
    .try_get_tailer = isil_vb_packet_queue_try_get_tailer,
    .put = isil_vb_packet_queue_put,
    .put_header = isil_vb_packet_queue_put_header,

    .get_curr_producer_from_pool = isil_vb_packet_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool = isil_vb_packet_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue = isil_vb_packet_queue_put_curr_producer_into_queue,
    .release_curr_producer =  isil_vb_packet_queue_release_curr_producer,
    .get_curr_consumer_from_queue = isil_vb_packet_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue = isil_vb_packet_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = isil_vb_packet_queue_release_curr_consumer,

    .get_curr_queue_entry_number = isil_vb_packet_queue_get_curr_queue_entry_number,
    .release = isil_vb_packet_queue_release,
    .init = isil_vb_packet_queue_init,
};

void	isil_create_packet_tcb_queue(isil_vb_packet_tcb_queue_t *vb_packet_queue)
{
    if(vb_packet_queue != NULL){
        vb_packet_queue->op = &isil_vb_packet_queue_op;
        vb_packet_queue->op->init(vb_packet_queue);
    }
}

void	isil_destroy_packet_tcb_queue(isil_vb_packet_tcb_queue_t *vb_packet_queue, isil_vb_pool_t *pool)
{
    if((vb_packet_queue!=NULL) && (pool!=NULL)){
        if(vb_packet_queue->op != NULL){
            vb_packet_queue->op->put_curr_producer_into_queue(vb_packet_queue);
            vb_packet_queue->op->release_curr_consumer(vb_packet_queue, pool);
            while(vb_packet_queue->op->get_curr_queue_entry_number(vb_packet_queue)){
                vb_packet_queue->op->get_curr_consumer_from_queue(vb_packet_queue);
                if(vb_packet_queue->curr_consumer == NULL){
                    break;
                }
                vb_packet_queue->op->release_curr_consumer(vb_packet_queue, pool);
            }
        } else {
            isil_vb_packet_queue_put_curr_producer_into_queue(vb_packet_queue);
            isil_vb_packet_queue_release_curr_consumer(vb_packet_queue, pool);
            while(isil_vb_packet_queue_get_curr_queue_entry_number(vb_packet_queue)){
                isil_vb_packet_queue_get_curr_consumer_from_queue(vb_packet_queue);
                if(vb_packet_queue->curr_consumer == NULL){
                    break;
                }
                isil_vb_packet_queue_release_curr_consumer(vb_packet_queue, pool);
            }
        }
    }
}

static void	isil_vb_frame_tcb_nal_init(h264_nal_t *nal)
{
    if(nal != NULL){
        memset(nal, 0, sizeof(h264_nal_t));
        nal->sps_payload = nal->pps_payload = nal->slice_payload = nal->slice_head_buf;
        nal->sps_paysize = nal->pps_paysize = nal->slice_paysize = 0;
    }
}

static void	isil_vb_frame_tcb_init(isil_vb_frame_tcb_t *frame)
{
    if(frame!=NULL){
        tcb_node_t	*node = &frame->frame_node;
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, frame);

        spin_lock_init(&frame->lock);
        atomic_set(&frame->ref_count, 0);
        frame->frame_type = H264_FRAME_TYPE_IDR;
        isil_vb_frame_tcb_nal_init(&frame->nal);	
        frame->frame_len = 0;
        frame->consumer_frame_offset = 0;
        frame->timestamp = 0;
        frame->i_init_qp = DEFAULT_QP;
        frame->i_curr_qp = 0;
        frame->i_mb_x = 0;
        frame->i_mb_y = 0;
        frame->frame_number = 0;
        frame->frame_is_err = 0;
        frame->frame_flags  = 0;
        isil_create_packet_tcb_queue(&frame->vb_packet_queue);
    }
}

static void	isil_vb_frame_tcb_reset(isil_vb_frame_tcb_t *frame)
{
    if(frame != NULL){
        atomic_set(&frame->ref_count, 0);
        frame->frame_type = H264_FRAME_TYPE_IDR;
        isil_vb_frame_tcb_nal_init(&frame->nal);
        frame->frame_len = 0;
        frame->consumer_frame_offset = 0;
        frame->timestamp = 0;
        frame->i_init_qp = DEFAULT_QP;
        frame->i_curr_qp = 0;
        frame->i_mb_x = 0;
        frame->i_mb_y = 0;
        frame->frame_number = 0;
        frame->frame_is_err = 0;
        frame->frame_flags  = 0;
    }
}

static void	isil_vb_frame_tcb_release(isil_vb_frame_tcb_t **ptr_frame, isil_vb_pool_t *pool)
{
    if(ptr_frame != NULL){
        isil_vb_frame_tcb_t	*frame = *ptr_frame;
        unsigned long	flags;
        if(pool == NULL){
            pool = frame->pool;
        }
        spin_lock_irqsave(&frame->lock, flags);
        if(atomic_read(&frame->ref_count) > 1){
            atomic_dec(&frame->ref_count);
        } else {
            tcb_node_t	*node;
            if(frame->op != NULL){
                frame->op->reset(frame);
            }
            if(frame->vb_packet_queue.op != NULL){
                frame->vb_packet_queue.op->release(&frame->vb_packet_queue, pool);
            } else {
                isil_vb_packet_queue_release(&frame->vb_packet_queue, pool);
            }
            node = &frame->frame_node;
            if(node->op != NULL){
                node->op->release(node, &pool->vb_frame_pool_tcb);
            }
        }
        *ptr_frame = NULL;
        spin_unlock_irqrestore(&frame->lock, flags);
    }
}

static void	isil_vb_frame_tcb_reference(isil_vb_frame_tcb_t *src_frame, isil_vb_frame_tcb_t **ptr_dest_frame)
{
    if((src_frame!=NULL) && (ptr_dest_frame!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&src_frame->lock, flags);
        atomic_inc(&src_frame->ref_count);
        *ptr_dest_frame = src_frame;
        spin_unlock_irqrestore(&src_frame->lock, flags);
    }
}
#ifndef NEW_HEADER
static size_t	isil_vb_frame_submit(isil_vb_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_vb_pool_t *pool)
{
    isil_frame_header_t __user    *frame_header_ptr;
    isil_frame_header_t           isil_frame_head;
    isil_vb_packet_tcb_queue_t *vb_packet_queue;
    size_t  buf_size, packet_size;
    loff_t  buf_offset, packet_offset;
    insert_03_state_machine_t	state_machine;

    buf_size = count;
    buf_offset = 0;
    memset(&state_machine, 0, sizeof(insert_03_state_machine_t));
    if(frame != NULL){
        frame_header_ptr = (isil_frame_header_t*)data;
        buf_size -= sizeof(isil_frame_header_t);
        buf_offset += sizeof(isil_frame_header_t);
        memset(&isil_frame_head, 0, sizeof(isil_frame_header_t));
        isil_frame_head.codecType = ISIL_VIDEO_MJPEG_CODEC_TYPE;
        isil_frame_head.streamType = frame->frame_flags;
        isil_frame_head.frameSerial = frame->frame_number;
        isil_frame_head.frameType = frame->frame_type;
        isil_frame_head.timeStamp = frame->timestamp;
        isil_frame_head.payload_offset = sizeof(isil_frame_header_t);
        isil_frame_head.payloadLen = 0;

        vb_packet_queue = &frame->vb_packet_queue;
        while(vb_packet_queue->op->get_curr_queue_entry_number(vb_packet_queue))
        {
            vb_packet_queue->op->get_curr_consumer_from_queue(vb_packet_queue);
            packet_size = vb_packet_queue->curr_consumer->op->submit(vb_packet_queue->curr_consumer, &data[buf_offset], buf_size, &packet_offset, &state_machine);
            //printk("%s,%d: packet size %d\n", __FUNCTION__, __LINE__, packet_size);
            vb_packet_queue->op->release_curr_consumer(vb_packet_queue, pool);
            buf_size -= packet_size;
            buf_offset += packet_size;
            isil_frame_head.payloadLen += packet_size;
        }
        while(buf_offset & 3){
            __put_user(0, &data[buf_offset]);
            buf_size--;
            buf_offset++;
            isil_frame_head.payloadLen++;
        }
        if(copy_to_user(frame_header_ptr, &isil_frame_head, sizeof(isil_frame_header_t)) != 0){
            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
        }
    }
    *ppos += buf_offset;
    //printk("%s,%d: payload length %d\n", __FUNCTION__, __LINE__, isil_frame_head.payloadLen);
    return (size_t)buf_offset;
}

#else

static size_t	isil_vb_frame_submit(isil_vb_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_vb_pool_t *pool)
{
    isil_vb_packet_tcb_queue_t *vb_packet_queue;
    size_t  buf_size, packet_size = 0;
    loff_t  buf_offset, packet_offset;
    insert_03_state_machine_t	state_machine;
    struct isil_common_stream_header  icsh, *picsh;
    isil_transparent_msg_header_t  mh, *pmh;
    isil_jpeg_param_msg_t  jpm, *pjpm;
    isil_frame_msg_t  fm, *pfm;    
    char *pc, *pd;
    unsigned int expect_len,u_len;


    buf_size = count;
    buf_offset = 0;
    picsh = &icsh;
    pmh = &mh;
    pjpm = &jpm;
    pfm = &fm;

    memset(&state_machine, 0, sizeof(insert_03_state_machine_t));
    if(frame != NULL){
        buf_offset += sizeof(struct isil_common_stream_header);
        buf_size -= sizeof(struct isil_common_stream_header);
        picsh->stream_type = ISIL_MJPEG_STREAM;
        picsh->stream_len = sizeof(struct isil_common_stream_header);
        
        buf_offset += sizeof(isil_transparent_msg_header_t);
        buf_size -= sizeof(isil_transparent_msg_header_t);
        pmh->isil_transparent_msg_number = 2;///////
        picsh->stream_len += sizeof(isil_transparent_msg_header_t);
        pmh->isil_transparent_msg_total_len = sizeof(isil_transparent_msg_header_t);

        buf_offset += sizeof(isil_jpeg_param_msg_t);
        buf_size -= sizeof(isil_jpeg_param_msg_t);
        pjpm->msg_type = ISIL_MJPEG_ENCODE_PARAM_MSG;
        pjpm->msg_len  = sizeof(isil_jpeg_param_msg_t);
        pjpm->capture_type = frame->frame_flags;//////
        pjpm->i_mb_x = frame->i_mb_x;//////
        pjpm->i_mb_y = frame->i_mb_y;//////
        picsh->stream_len += sizeof(isil_jpeg_param_msg_t);
        pmh->isil_transparent_msg_total_len += sizeof(isil_jpeg_param_msg_t);

        buf_offset += sizeof(isil_frame_msg_t);
        buf_size -= sizeof(isil_frame_msg_t);
        pfm->msg_type = ISIL_MJPEG_ENCODE_FRAME_MSG;
        pfm->msg_len  = sizeof(isil_frame_msg_t);
        pfm->frame_serial = frame->frame_number;
        pfm->frame_timestamp = frame->timestamp;
        pfm->frame_checksam = 0;//compute_crc(frame->data, frame->payload_len);
        picsh->stream_len += sizeof(isil_frame_msg_t);
        pmh->isil_transparent_msg_total_len += sizeof(isil_frame_msg_t);


        expect_len = tc_align_up(frame->frame_len,4);
        if(buf_size < expect_len){
                ISIL_DBG(ISIL_DBG_ERR,"user buffer is smaller than one frame in %s\n",__FUNCTION__);
                return 0;
        }

        vb_packet_queue = &frame->vb_packet_queue;
        while(vb_packet_queue->op->get_curr_queue_entry_number(vb_packet_queue))
        {
            vb_packet_queue->op->get_curr_consumer_from_queue(vb_packet_queue);
            pd = vb_packet_queue->curr_consumer->data + vb_packet_queue->curr_consumer->consumer_len;
            u_len = vb_packet_queue->curr_consumer->payload_len - vb_packet_queue->curr_consumer->consumer_len;
            pfm->frame_checksam = compute_accumulate_crc(pfm->frame_checksam, pd, u_len);
            packet_size = vb_packet_queue->curr_consumer->op->submit(vb_packet_queue->curr_consumer, &data[buf_offset], buf_size, &packet_offset, &state_machine);
            vb_packet_queue->op->release_curr_consumer(vb_packet_queue, pool);
            buf_size -= packet_size;
            buf_offset += packet_size;
            picsh->stream_len += packet_size;
            pmh->isil_transparent_msg_total_len += packet_size;
            pfm->msg_len += packet_size;
        }
        while(buf_offset & 3){
            __put_user(0, &data[buf_offset]);
            buf_size--;
            buf_offset++;
            picsh->stream_len += packet_size;
            pmh->isil_transparent_msg_total_len += packet_size;
            pfm->msg_len += packet_size;
        }

        picsh->stream_len |= ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG;

        /*finally cope the header info*/
        pc = (char *)data;
        if(copy_to_user(pc, &icsh, sizeof(struct isil_common_stream_header)) != 0){
                ISIL_DBG(ISIL_DBG_ERR,"%s, %d:struct isil_common_stream_header copy_to_user failed\n", __FUNCTION__, __LINE__);
                return 0;
        }
        pc += sizeof(struct isil_common_stream_header);
        if(copy_to_user(pc, &mh, sizeof(isil_transparent_msg_header_t)) != 0){
                ISIL_DBG(ISIL_DBG_ERR,"%s, %d:isil_transparent_msg_header_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                return 0;
        }
        pc += sizeof(isil_transparent_msg_header_t);
        if(copy_to_user(pc, &jpm, sizeof(isil_jpeg_param_msg_t)) != 0){
                ISIL_DBG(ISIL_DBG_ERR,"%s, %d:isil_jpeg_param_msg_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                return 0;
        }
        pc += sizeof(isil_jpeg_param_msg_t);
        if(copy_to_user(pc, &fm, sizeof(isil_frame_msg_t)) != 0){
                ISIL_DBG(ISIL_DBG_ERR,"%s, %d:isil_frame_msg_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                return 0;
        }
    }else{
         ISIL_DBG(ISIL_DBG_ERR,"frame is NULL in %s\n",__FUNCTION__);
    }

    *ppos += buf_offset;
    //printk("%s,%d: payload length %d\n", __FUNCTION__, __LINE__, isil_frame_head.payloadLen);
    return (size_t)buf_offset;

}


#endif
static struct isil_vb_frame_tcb_operation	isil_vb_frame_tcb_op = {
    .init = isil_vb_frame_tcb_init,
    .reset = isil_vb_frame_tcb_reset,
    .release = isil_vb_frame_tcb_release,
    .reference = isil_vb_frame_tcb_reference,

    .submit = isil_vb_frame_submit,
};

static void	isil_vb_frame_queue_get(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_frame_tcb_t **ptr_frame)
{
    if((vb_frame_tcb_queue!=NULL) && (ptr_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_frame = NULL;
        queue = &vb_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_vb_frame_tcb(temp_node);
            }
        }
    } else {
        printk("\n%s.%d**attention**\n", __FUNCTION__, __LINE__);
    }
}

static void	isil_vb_frame_queue_try_get(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_frame_tcb_t **ptr_frame)
{
    if((vb_frame_tcb_queue!=NULL) && (ptr_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_frame = NULL;
        queue = &vb_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_vb_frame_tcb(temp_node);
            }
        }
    }
}

static void	isil_vb_frame_queue_get_tailer(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_frame_tcb_t **ptr_frame)
{
    if((vb_frame_tcb_queue!=NULL) && (ptr_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_frame = NULL;
        queue = &vb_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_vb_frame_tcb(temp_node);
            }
        }
    }
}

static void	isil_vb_frame_queue_try_get_tailer(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_frame_tcb_t **ptr_frame)
{
    if((vb_frame_tcb_queue!=NULL) && (ptr_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_frame = NULL;
        queue = &vb_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_vb_frame_tcb(temp_node);
            }
        }
    }
}

static void	isil_vb_frame_queue_put(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_frame_tcb_t *frame)
{
    if((vb_frame_tcb_queue!=NULL) && (frame!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &vb_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put(queue, &frame->frame_node);
        }
    }
}

static void	isil_vb_frame_queue_put_header(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_frame_tcb_t *frame)
{
    if((vb_frame_tcb_queue!=NULL) && (frame!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &vb_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put_header(queue, &frame->frame_node);
        }
    }
}

static void	isil_vb_frame_queue_get_curr_producer_from_pool(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_pool_t *pool)
{
    if((vb_frame_tcb_queue!=NULL) && (pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        if(vb_frame_tcb_queue->curr_producer == NULL){
            printk("%s,%d: get a jpeg frame!\n", __FUNCTION__, __LINE__);
            spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
            if(pool->op != NULL){
                pool->op->get_frame_tcb(pool, &vb_frame_tcb_queue->curr_producer);
            }
            spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        }
        spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
    }
}

static void	isil_vb_frame_queue_try_get_curr_producer_from_pool(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_pool_t *pool)
{
    if((vb_frame_tcb_queue!=NULL) && (pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        if(vb_frame_tcb_queue->curr_producer == NULL){
            if(pool->op != NULL){
                //printk("%s,%d: get a jpeg frame!\n", __FUNCTION__, __LINE__);
                pool->op->try_get_frame_tcb(pool, &vb_frame_tcb_queue->curr_producer);
            }
        }
        spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
    }
}

static void	isil_vb_frame_queue_put_curr_producer_into_queue(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue)
{
    if(vb_frame_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        if(vb_frame_tcb_queue->curr_producer != NULL){
            if(vb_frame_tcb_queue->op != NULL){
                vb_frame_tcb_queue->op->put(vb_frame_tcb_queue, vb_frame_tcb_queue->curr_producer);
            } else {
                isil_vb_frame_queue_put(vb_frame_tcb_queue, vb_frame_tcb_queue->curr_producer);
            }
            vb_frame_tcb_queue->curr_producer = NULL;
        }
        spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
    }
}

static void	isil_vb_frame_queue_release_curr_producer(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_pool_t *pool)
{
    if((vb_frame_tcb_queue!=NULL) && (pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        if(vb_frame_tcb_queue->curr_producer != NULL){
            if(vb_frame_tcb_queue->curr_producer->op != NULL){
                vb_frame_tcb_queue->curr_producer->op->release(&vb_frame_tcb_queue->curr_producer, pool);
            } else {
                isil_vb_frame_tcb_release(&vb_frame_tcb_queue->curr_producer, pool);
            }
        }
        spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
    }
}

static void	isil_vb_frame_queue_get_curr_consumer_from_queue(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue)
{
    if(vb_frame_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        if(vb_frame_tcb_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
            if(vb_frame_tcb_queue->op != NULL){
                vb_frame_tcb_queue->op->get(vb_frame_tcb_queue, &vb_frame_tcb_queue->curr_consumer);
            } else {
                isil_vb_frame_queue_get(vb_frame_tcb_queue, &vb_frame_tcb_queue->curr_consumer);
            }
            spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        }
        spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
    } else {
        printk("\n%s.%d**attention**\n", __FUNCTION__, __LINE__);
    }
}

static void	isil_vb_frame_queue_try_get_curr_consumer_from_queue(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue)
{
    if(vb_frame_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        if(vb_frame_tcb_queue->curr_consumer == NULL){
            if(vb_frame_tcb_queue->op != NULL){
                vb_frame_tcb_queue->op->try_get(vb_frame_tcb_queue, &vb_frame_tcb_queue->curr_consumer);
            } else {
                isil_vb_frame_queue_try_get(vb_frame_tcb_queue, &vb_frame_tcb_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
    }
}

static void	isil_vb_frame_queue_release_curr_consumer(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_pool_t *pool)
{
    if((vb_frame_tcb_queue!=NULL) && (pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&vb_frame_tcb_queue->lock, flags);
        if(vb_frame_tcb_queue->curr_consumer != NULL){
            if(vb_frame_tcb_queue->curr_consumer->op != NULL){
                //printk("%s,%d: release a jpeg frame!\n", __FUNCTION__, __LINE__);
                vb_frame_tcb_queue->curr_consumer->op->release(&vb_frame_tcb_queue->curr_consumer, pool);
            } else {
                isil_vb_frame_tcb_release(&vb_frame_tcb_queue->curr_consumer, pool);
            }
        }
        spin_unlock_irqrestore(&vb_frame_tcb_queue->lock, flags);
    }
}

static int	isil_vb_frame_queue_get_curr_queue_entry_number(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue)
{
    int	entry_number = 0;
    if(vb_frame_tcb_queue!=NULL){
        tcb_node_queue_t	*queue;
        queue = &vb_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            entry_number = queue->op->get_queue_curr_entry_number(queue);
        }
    }
    return entry_number;
}

static void	isil_vb_frame_queue_release(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_pool_t *pool)
{
    if((vb_frame_tcb_queue!=NULL) && (pool!=NULL)){
        isil_destroy_frame_tcb_queue(vb_frame_tcb_queue, pool);
    }
}

static void	isil_vb_frame_queue_init(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue)
{
    if(vb_frame_tcb_queue != NULL){
        tcb_node_queue_t	*queue;
        queue = &vb_frame_tcb_queue->queue_node;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);
        vb_frame_tcb_queue->curr_consumer = NULL;
        vb_frame_tcb_queue->curr_producer = NULL;
        vb_frame_tcb_queue->start_timestamp = 0;
        vb_frame_tcb_queue->total_duration = 0;
        spin_lock_init(&vb_frame_tcb_queue->lock);
    }
}

static struct isil_vb_frame_queue_operation	isil_vb_frame_queue_op = {
    .get = isil_vb_frame_queue_get,
    .try_get = isil_vb_frame_queue_try_get,
    .get_tailer = isil_vb_frame_queue_get_tailer,
    .try_get_tailer = isil_vb_frame_queue_try_get_tailer,
    .put = isil_vb_frame_queue_put,
    .put_header = isil_vb_frame_queue_put_header,

    .get_curr_producer_from_pool = isil_vb_frame_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool = isil_vb_frame_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue = isil_vb_frame_queue_put_curr_producer_into_queue,
    .release_curr_producer = isil_vb_frame_queue_release_curr_producer,
    .get_curr_consumer_from_queue = isil_vb_frame_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue = isil_vb_frame_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = isil_vb_frame_queue_release_curr_consumer,

    .get_curr_queue_entry_number = isil_vb_frame_queue_get_curr_queue_entry_number,
    .release = isil_vb_frame_queue_release,
    .init = isil_vb_frame_queue_init,
};

void	isil_create_frame_tcb_queue(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue)
{
    if(vb_frame_tcb_queue != NULL){
        vb_frame_tcb_queue->op = &isil_vb_frame_queue_op;
        vb_frame_tcb_queue->op->init(vb_frame_tcb_queue);
    }
}

void	isil_destroy_frame_tcb_queue(isil_vb_frame_tcb_queue_t *vb_frame_tcb_queue, isil_vb_pool_t *pool)
{
    if((vb_frame_tcb_queue!=NULL) && (pool!=NULL)){
        if(vb_frame_tcb_queue->op != NULL){
            vb_frame_tcb_queue->op->put_curr_producer_into_queue(vb_frame_tcb_queue);
            vb_frame_tcb_queue->op->release_curr_consumer(vb_frame_tcb_queue, pool);
            while(vb_frame_tcb_queue->op->get_curr_queue_entry_number(vb_frame_tcb_queue)){
                vb_frame_tcb_queue->op->get_curr_consumer_from_queue(vb_frame_tcb_queue);
                if(vb_frame_tcb_queue->curr_consumer == NULL){
                    break;
                }
                vb_frame_tcb_queue->op->release_curr_consumer(vb_frame_tcb_queue, pool);
            }
        } else {
            isil_vb_frame_queue_put_curr_producer_into_queue(vb_frame_tcb_queue);
            isil_vb_frame_queue_release_curr_consumer(vb_frame_tcb_queue, pool);
            while(isil_vb_frame_queue_get_curr_queue_entry_number(vb_frame_tcb_queue)){
                isil_vb_frame_queue_get_curr_consumer_from_queue(vb_frame_tcb_queue);
                if(vb_frame_tcb_queue->curr_consumer == NULL){
                    break;
                }
                isil_vb_frame_queue_release_curr_consumer(vb_frame_tcb_queue, pool);
            }
        }
    }
}


static int	isil_pool_create(isil_vb_pool_t *pool, unsigned long buf_len)
{
    int	ret = ISIL_ERR;

    if(pool != NULL){
        tcb_node_pool_t	*node_pool;
        isil_vb_packet_tcb_t	*ptr_vb_packet_tcb;
        isil_vb_frame_tcb_t	*ptr_vb_frame_tcb;
        int		i;

        pool->cache_order = get_order(buf_len);
#ifdef  USE_DISCRETE_BUF
        pool->cache_buffer = NULL;
#else
        pool->cache_buffer = (u8*)__get_free_pages(GFP_KERNEL, pool->cache_order);
        if(pool->cache_buffer == NULL) {
            pool->cache_order = 0;
            printk("can't alloc %d pages for vb buf pool\n", pool->cache_order);
            return -ENOMEM;
        }
#endif
        pool->vb_packet_entry_number = 1;
        for(i=0; i<pool->cache_order; i++){
            pool->vb_packet_entry_number <<= 1;
        }
        pool->vb_packet_buffer_size = PAGE_SIZE;
        node_pool = &pool->vb_packet_pool_tcb;
        node_pool->op = &tcb_node_pool_op;
        pool->vb_packet_entry_pool = (isil_vb_packet_tcb_t*)kmalloc(sizeof(isil_vb_packet_tcb_t)*pool->vb_packet_entry_number, GFP_KERNEL);
        if(pool->vb_packet_entry_pool == NULL){
            if(pool->cache_buffer != NULL) {
                free_pages((unsigned long)pool->cache_buffer, pool->cache_order);
            }
            pool->cache_buffer = NULL;
            pool->cache_order = 0;
            printk("can't alloc vb packet tcb\n");
            return -ENOMEM;
        }
        node_pool->op->init(node_pool, pool->vb_packet_entry_number);
        for(i=0; i<pool->vb_packet_entry_number; i++){
            ptr_vb_packet_tcb = &pool->vb_packet_entry_pool[i];
            ptr_vb_packet_tcb->op = &isil_vb_packet_tcb_op;
            ptr_vb_packet_tcb->op->init(ptr_vb_packet_tcb, pool, i);
            ptr_vb_packet_tcb->op->release(&ptr_vb_packet_tcb, pool);
        }

        pool->vb_frame_entry_number = pool->vb_packet_entry_number;
        node_pool = &pool->vb_frame_pool_tcb;
        node_pool->op = &tcb_node_pool_op;
        pool->vb_frame_entry_pool = (isil_vb_frame_tcb_t*)kmalloc(sizeof(isil_vb_frame_tcb_t)*pool->vb_frame_entry_number, GFP_KERNEL);
        if(pool->vb_frame_entry_pool == NULL){
            node_pool = &pool->vb_packet_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(pool->vb_packet_entry_pool);
            pool->vb_packet_entry_pool = NULL;
            pool->vb_packet_entry_number = 0;
            if(pool->cache_buffer != NULL) {
                free_pages((unsigned long)pool->cache_buffer, pool->cache_order);
            }
            pool->cache_buffer = NULL;
            pool->cache_order = 0;
            printk("can't alloc vb frame tcb\n");
            return -ENOMEM;
        }
        node_pool->op->init(node_pool, pool->vb_frame_entry_number);
        for(i=0; i<pool->vb_frame_entry_number; i++){
            ptr_vb_frame_tcb = &pool->vb_frame_entry_pool[i];
            ptr_vb_frame_tcb->op = &isil_vb_frame_tcb_op;
            ptr_vb_frame_tcb->pool = pool;
            ptr_vb_frame_tcb->op->init(ptr_vb_frame_tcb);
            ptr_vb_frame_tcb->op->release(&ptr_vb_frame_tcb, pool);
        }
    }
    return ret;
}

static void	isil_pool_release(isil_vb_pool_t *pool)
{
    if(pool != NULL){
        tcb_node_pool_t	*node_pool;

        node_pool = &pool->vb_frame_pool_tcb;
        if(node_pool->op != NULL){
            node_pool->op->release(node_pool);
        }
        if(pool->vb_frame_entry_pool != NULL){
            kfree(pool->vb_frame_entry_pool);
            pool->vb_frame_entry_pool = NULL;
        }
        pool->vb_frame_entry_number = 0;

        node_pool = &pool->vb_packet_pool_tcb;
        if(node_pool->op != NULL){
            node_pool->op->release(node_pool);
        }
        if(pool->vb_packet_entry_pool != NULL){
#ifdef  USE_DISCRETE_BUF
            isil_vb_packet_tcb_t	*ptr_vb_packet_tcb;
            int i;
            for(i=0; i<pool->vb_packet_entry_number; i++) {
                ptr_vb_packet_tcb = &pool->vb_packet_entry_pool[i];
                if(ptr_vb_packet_tcb->data != NULL) {
                    free_page((unsigned long)ptr_vb_packet_tcb->data);
                    ptr_vb_packet_tcb->data = NULL;
                }
            }
#endif
            kfree(pool->vb_packet_entry_pool);
            pool->vb_packet_entry_pool = NULL;
        }
        pool->vb_packet_entry_number = 0;

        if(pool->cache_buffer != NULL){
            free_pages((unsigned long)pool->cache_buffer, pool->cache_order);
            pool->cache_buffer = NULL;
        }
        pool->cache_order = 0;
    }
}

static void	isil_pool_get_vb_packet_tcb(isil_vb_pool_t *pool, isil_vb_packet_tcb_t **packet)
{
    if((pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *vb_packet_pool_tcb = &pool->vb_packet_pool_tcb;
        *packet = NULL;
        if(vb_packet_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            vb_packet_pool_tcb->op->get(vb_packet_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *packet = to_vb_packet_tcb(temp_node);
                atomic_inc(&((*packet)->ref_count));
            }
        }
    }
}

static void	isil_vb_pool_try_get_vb_packet_tcb(isil_vb_pool_t *pool, isil_vb_packet_tcb_t **packet)
{
    if((pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *vb_packet_pool_tcb = &pool->vb_packet_pool_tcb;
        *packet = NULL;
        if(vb_packet_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            vb_packet_pool_tcb->op->try_get(vb_packet_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *packet = to_vb_packet_tcb(temp_node);
                atomic_inc(&((*packet)->ref_count));
            }
        }
    }
}

static void	isil_pool_put_vb_packet_tcb(isil_vb_pool_t *pool, isil_vb_packet_tcb_t *packet)
{
    if((pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *vb_packet_pool_tcb = &pool->vb_packet_pool_tcb;
        if(vb_packet_pool_tcb->op != NULL){
            vb_packet_pool_tcb->op->put(vb_packet_pool_tcb, &packet->packet_node);
        }
    }
}

static int	isil_pool_get_vb_packet_tcb_pool_entry_number(isil_vb_pool_t *pool)
{
    int	entry_number = 0;
    if(pool != NULL){
        tcb_node_pool_t *vb_packet_pool_tcb = &pool->vb_packet_pool_tcb;
        if(vb_packet_pool_tcb->op != NULL){
            entry_number = vb_packet_pool_tcb->op->get_curr_pool_entry_number(vb_packet_pool_tcb);
        }
    }
    return entry_number;
}

static void	isil_pool_get_vb_frame_tcb(isil_vb_pool_t *pool, isil_vb_frame_tcb_t **frame)
{
    if((pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *vb_frame_pool_tcb = &pool->vb_frame_pool_tcb;
        *frame = NULL;
        if(vb_frame_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            vb_frame_pool_tcb->op->get(vb_frame_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *frame = to_vb_frame_tcb(temp_node);
                atomic_inc(&((*frame)->ref_count));
            }
        }
    }
}

static void	isil_vb_pool_try_get_vb_frame_tcb(isil_vb_pool_t *pool, isil_vb_frame_tcb_t **frame)
{
    if((pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *vb_frame_pool_tcb = &pool->vb_frame_pool_tcb;
        *frame = NULL;
        if(vb_frame_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            vb_frame_pool_tcb->op->try_get(vb_frame_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *frame = to_vb_frame_tcb(temp_node);
                atomic_inc(&((*frame)->ref_count));
            }
        }
    }
}

static void	isil_pool_put_vb_frame_tcb(isil_vb_pool_t *pool, isil_vb_frame_tcb_t *frame)
{
    if((pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *vb_frame_pool_tcb = &pool->vb_frame_pool_tcb;
        if(vb_frame_pool_tcb->op != NULL){
            vb_frame_pool_tcb->op->put(vb_frame_pool_tcb, &frame->frame_node);
        }
    }
}

static int	isil_pool_get_vb_frame_tcb_pool_entry_number(isil_vb_pool_t *pool)
{
    int	entry_number = 0;
    if(pool != NULL){
        tcb_node_pool_t *vb_frame_pool_tcb = &pool->vb_frame_pool_tcb;
        if(vb_frame_pool_tcb->op != NULL){
            entry_number = vb_frame_pool_tcb->op->get_curr_pool_entry_number(vb_frame_pool_tcb);
        }
    }
    return entry_number;
}


static struct isil_vb_pool_operation	isil_pool_op = {
    .create = isil_pool_create,
    .release = isil_pool_release,

    .get_packet_tcb = isil_pool_get_vb_packet_tcb,
    .try_get_packet_tcb = isil_vb_pool_try_get_vb_packet_tcb,
    .put_packet_tcb = isil_pool_put_vb_packet_tcb,
    .get_packet_tcb_pool_entry_number = isil_pool_get_vb_packet_tcb_pool_entry_number,

    .get_frame_tcb = isil_pool_get_vb_frame_tcb,
    .try_get_frame_tcb = isil_vb_pool_try_get_vb_frame_tcb,
    .put_frame_tcb = isil_pool_put_vb_frame_tcb,
    .get_frame_tcb_pool_entry_number = isil_pool_get_vb_frame_tcb_pool_entry_number,

};

int isil_create_pool(isil_vb_pool_t *pool, unsigned long buf_len)
{
    int	ret = ISIL_ERR;
    if(pool != NULL){
        pool->op = &isil_pool_op;
        ret = pool->op->create(pool, buf_len);
    }
    return ret;
}

void isil_destroy_pool(isil_vb_pool_t *pool)
{
    if(pool != NULL){
        if(pool->op != NULL){
            pool->op->release(pool);
        }
    }
}


