#include	<isil5864/isil_common.h>


static void isil_video_packet_tcb_init(isil_video_packet_tcb_t *video_packet_tcb, isil_video_chan_buf_pool_t *video_chan_buf_pool, int id)
{
    if((video_packet_tcb!=NULL) && (video_chan_buf_pool!=NULL)){
        tcb_node_t  *node = &video_packet_tcb->packet_node;
        spin_lock_init(&video_packet_tcb->lock);
        atomic_set(&video_packet_tcb->ref_count, 0);
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, video_packet_tcb);
        video_packet_tcb->packet_id = id;
        video_packet_tcb->packet_size = video_chan_buf_pool->video_packet_buffer_size;
        video_packet_tcb->payload_len = 0;
        video_packet_tcb->consumer_len = 0;
#ifdef  USE_DISCRETE_BUF
        video_packet_tcb->data = (u8*)__get_free_page(GFP_KERNEL);
        if(video_packet_tcb->data == NULL) {
            while(1) {
                printk("%s.%d: can't get one page, only for debug\n", __FUNCTION__, __LINE__);
            }
        }
#else
        video_packet_tcb->data = &video_chan_buf_pool->cache_buffer[video_chan_buf_pool->video_packet_buffer_size*id];
#endif
        video_packet_tcb->dma_addr = 0;
        video_packet_tcb->id_of_packet_queue = 0;
    }
}

static void isil_video_packet_tcb_reset(isil_video_packet_tcb_t *video_packet_tcb)
{
    if(video_packet_tcb!=NULL){
        atomic_set(&video_packet_tcb->ref_count, 0);
        video_packet_tcb->payload_len = 0;
        video_packet_tcb->consumer_len = 0;
        video_packet_tcb->dma_addr = 0;
        video_packet_tcb->id_of_packet_queue = 0;
    }
}

static void isil_video_packet_tcb_release(isil_video_packet_tcb_t **ptr_video_packet_tcb, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((*ptr_video_packet_tcb!=NULL) && (video_chan_buf_pool!=NULL)){
        isil_video_packet_tcb_t	*video_packet_tcb = *ptr_video_packet_tcb;
        unsigned long	flags;

        spin_lock_irqsave(&video_packet_tcb->lock, flags);
        if(atomic_read(&video_packet_tcb->ref_count) > 1){
            atomic_dec(&video_packet_tcb->ref_count);
        } else {
            tcb_node_t	*node;
            if(video_packet_tcb->op != NULL){
                video_packet_tcb->op->reset(video_packet_tcb);
            }
            node = &video_packet_tcb->packet_node;
            if(node->op != NULL){
                node->op->release(node, &video_chan_buf_pool->video_packet_pool_tcb);
            }
        }
        *ptr_video_packet_tcb = NULL;
        spin_unlock_irqrestore(&video_packet_tcb->lock, flags);
    }
}

static void isil_video_packet_tcb_reference(isil_video_packet_tcb_t *src_video_packet_tcb, isil_video_packet_tcb_t **ptr_dest_video_packet_tcb)
{
    if((src_video_packet_tcb!=NULL) && (ptr_dest_video_packet_tcb!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&src_video_packet_tcb->lock, flags);
        atomic_inc(&src_video_packet_tcb->ref_count);
        *ptr_dest_video_packet_tcb = src_video_packet_tcb;
        spin_unlock_irqrestore(&src_video_packet_tcb->lock, flags);
    }
}

static void isil_video_packet_tcb_dma_map(isil_video_packet_tcb_t *video_packet_tcb, enum dma_data_direction direction)
{
    if(video_packet_tcb != NULL) {
        if(video_packet_tcb->data != NULL) {
            video_packet_tcb->dma_addr = dma_map_single(NULL, video_packet_tcb->data, video_packet_tcb->packet_size, direction);
            if (video_packet_tcb->dma_addr == 0){
                printk("%s.%d err\n", __FUNCTION__, __LINE__);
            }
        }
    }
}

static void isil_video_packet_tcb_dma_unmap(isil_video_packet_tcb_t *video_packet_tcb, enum dma_data_direction direction)
{
    if(video_packet_tcb != NULL) {
        if (video_packet_tcb->dma_addr != 0) {
            dma_unmap_single(NULL, video_packet_tcb->dma_addr, video_packet_tcb->packet_size, direction);
            video_packet_tcb->dma_addr = 0;
        }
    }
}

static size_t   isil_video_packet_tcb_submit(isil_video_packet_tcb_t *packet, char __user *data, size_t count, loff_t *ppos, h264_nal_t *nal)
{
    loff_t  buf_offset = 0;
    if((packet != NULL) && (count >= packet->payload_len) && (packet->payload_len <= packet->packet_size)){
        if(copy_to_user(data, packet->data, packet->payload_len) != 0){
            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
        }
        buf_offset = packet->payload_len;
    } else {
        printk("\n\n BAD:copy parameter is wrong!!%s.%d\n\n", __FUNCTION__, __LINE__);
    }
    *ppos += buf_offset;
    return (size_t)buf_offset;
}


static size_t  isil_video_packet_tcb_slicent_discard(isil_video_packet_tcb_t *packet, char __user *data, size_t count, loff_t *ppos, h264_nal_t *nal)
{
    loff_t  buf_offset = 0;
    if((packet != NULL) && (count>=packet->payload_len) && (packet->payload_len <= packet->packet_size)){
        buf_offset = packet->payload_len;
    } else {
        printk("\n\n BAD:copy parameter is wrong!!%s.%d\n\n", __FUNCTION__, __LINE__);
    }
    *ppos += buf_offset;
    return (size_t)buf_offset;

}

static int isil_video_packet_tcb_copy(isil_video_packet_tcb_t *src, isil_video_packet_tcb_t *dst)
{
    if(src && dst) {
        dst->consumer_len = src->consumer_len;
        dst->payload_len  = src->payload_len;
        memcpy(dst->data, src->data, src->payload_len);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static	struct isil_video_packet_operation	isil_video_packet_tcb_op = {
    .init = isil_video_packet_tcb_init,
    .reset = isil_video_packet_tcb_reset,
    .release = isil_video_packet_tcb_release,
    .reference = isil_video_packet_tcb_reference,

    .dma_map = isil_video_packet_tcb_dma_map,
    .dma_unmap = isil_video_packet_tcb_dma_unmap,

    .submit = isil_video_packet_tcb_submit,
    .slicent_discard = isil_video_packet_tcb_slicent_discard,
    .copy   = isil_video_packet_tcb_copy,
};

static void isil_video_packet_queue_get(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_packet_tcb_t **ptr_video_packet_tcb)
{
    if((video_packet_queue!=NULL) && (ptr_video_packet_tcb!=NULL)){
        tcb_node_queue_t    *queue;
        *ptr_video_packet_tcb = NULL;
        queue = &video_packet_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t  *temp_node;
            queue->op->get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_packet_tcb = to_video_packet_buf_tcb(temp_node);
            }
        }
    }
}

static void isil_video_packet_queue_try_get(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_packet_tcb_t **ptr_video_packet_tcb)
{
    if((video_packet_queue!=NULL) && (ptr_video_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_packet_tcb = NULL;
        queue = &video_packet_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t  *temp_node;
            queue->op->try_get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_packet_tcb = to_video_packet_buf_tcb(temp_node);
            }
        }
    }
}

static void isil_video_packet_queue_get_tailer(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_packet_tcb_t **ptr_video_packet_tcb)
{
    if((video_packet_queue!=NULL) && (ptr_video_packet_tcb!=NULL)){
        tcb_node_queue_t    *queue;
        *ptr_video_packet_tcb = NULL;
        queue = &video_packet_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_packet_tcb = to_video_packet_buf_tcb(temp_node);
            }
        }
    }
}

static void isil_video_packet_queue_try_get_tailer(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_packet_tcb_t **ptr_video_packet_tcb)
{
    if((video_packet_queue!=NULL) && (ptr_video_packet_tcb!=NULL)){
        tcb_node_queue_t    *queue;
        *ptr_video_packet_tcb = NULL;
        queue = &video_packet_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_packet_tcb = to_video_packet_buf_tcb(temp_node);
            }
        }
    }
}

static void isil_video_packet_queue_put(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_packet_tcb_t *video_packet_tcb)
{
    if((video_packet_queue!=NULL) && (video_packet_tcb!=NULL)){
        tcb_node_queue_t    *queue;
        queue = &video_packet_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put(queue, &video_packet_tcb->packet_node);
            video_packet_tcb->id_of_packet_queue = queue->op->get_queue_curr_entry_number(queue);
        }
    }
}

static void isil_video_packet_queue_put_header(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_packet_tcb_t *video_packet_tcb)
{
    if((video_packet_queue!=NULL) && (video_packet_tcb!=NULL)){
        tcb_node_queue_t    *queue;
        queue = &video_packet_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put_header(queue, &video_packet_tcb->packet_node);
        }
    }
}

static void isil_video_packet_queue_get_curr_producer_from_pool(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_packet_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_packet_queue->lock, flags);
        if(video_packet_queue->curr_producer == NULL){
            spin_unlock_irqrestore(&video_packet_queue->lock, flags);
            if(video_chan_buf_pool->op != NULL){
                video_chan_buf_pool->op->get_video_packet_tcb(video_chan_buf_pool, &video_packet_queue->curr_producer);
            }
            /*if(video_packet_queue->curr_producer != NULL){
              video_packet_queue->curr_producer->op->dma_map(video_packet_queue->curr_producer, DMA_FROM_DEVICE);
              }*/
            spin_lock_irqsave(&video_packet_queue->lock, flags);
        }
        spin_unlock_irqrestore(&video_packet_queue->lock, flags);
    }
}

static void isil_video_packet_queue_try_get_curr_producer_from_pool(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_packet_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long   flags;
        spin_lock_irqsave(&video_packet_queue->lock, flags);
        if(video_packet_queue->curr_producer == NULL) {
            if(video_chan_buf_pool->op != NULL){
                video_chan_buf_pool->op->try_get_video_packet_tcb(video_chan_buf_pool, &video_packet_queue->curr_producer);
            }
            /*if(video_packet_queue->curr_producer != NULL) {
              video_packet_queue->curr_producer->op->dma_map(video_packet_queue->curr_producer, DMA_FROM_DEVICE);
              }*/
        }
        spin_unlock_irqrestore(&video_packet_queue->lock, flags);
    }
}

static void isil_video_packet_queue_put_curr_producer_into_queue(isil_video_packet_tcb_queue_t *video_packet_queue)
{
    if(video_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_packet_queue->lock, flags);
        if(video_packet_queue->curr_producer != NULL){
            //video_packet_queue->curr_producer->op->dma_unmap(video_packet_queue->curr_producer, DMA_FROM_DEVICE);
            if(video_packet_queue->op != NULL){
                video_packet_queue->op->put(video_packet_queue, video_packet_queue->curr_producer);
            } else {
                isil_video_packet_queue_put(video_packet_queue, video_packet_queue->curr_producer);
            }
            video_packet_queue->curr_producer = NULL;
        }
        spin_unlock_irqrestore(&video_packet_queue->lock, flags);
    }
}

static void isil_video_packet_queue_release_curr_producer(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_packet_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_packet_queue->lock, flags);
        if(video_packet_queue->curr_producer != NULL){
            if(video_packet_queue->curr_producer->op != NULL){
                video_packet_queue->curr_producer->op->release(&video_packet_queue->curr_producer, video_chan_buf_pool);
            } else {
                isil_video_packet_tcb_release(&video_packet_queue->curr_producer, video_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&video_packet_queue->lock, flags);
    }
}

static void isil_video_packet_queue_get_curr_consumer_from_queue(isil_video_packet_tcb_queue_t *video_packet_queue)
{
    if(video_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_packet_queue->lock, flags);
        if(video_packet_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&video_packet_queue->lock, flags);
            if(video_packet_queue->op != NULL){
                video_packet_queue->op->get(video_packet_queue, &video_packet_queue->curr_consumer);
            } else {
                isil_video_packet_queue_get(video_packet_queue, &video_packet_queue->curr_consumer);
            }
            spin_lock_irqsave(&video_packet_queue->lock, flags);
        }
        spin_unlock_irqrestore(&video_packet_queue->lock, flags);
    }
}

static void isil_video_packet_queue_try_get_curr_consumer_from_queue(isil_video_packet_tcb_queue_t *video_packet_queue)
{
    if(video_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_packet_queue->lock, flags);
        if(video_packet_queue->curr_consumer == NULL){
            if(video_packet_queue->op != NULL){
                video_packet_queue->op->try_get(video_packet_queue, &video_packet_queue->curr_consumer);
            } else {
                isil_video_packet_queue_try_get(video_packet_queue, &video_packet_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&video_packet_queue->lock, flags);
    }
}

static void isil_video_packet_queue_release_curr_consumer(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_packet_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_packet_queue->lock, flags);
        if(video_packet_queue->curr_consumer != NULL){
            if(video_packet_queue->curr_consumer->op != NULL){
                video_packet_queue->curr_consumer->op->release(&video_packet_queue->curr_consumer, video_chan_buf_pool);
            } else {
                isil_video_packet_tcb_release(&video_packet_queue->curr_consumer, video_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&video_packet_queue->lock, flags);
    }
}

static int  isil_video_packet_queue_get_curr_queue_entry_number(isil_video_packet_tcb_queue_t *video_packet_queue)
{
    int entry_number = 0;
    tcb_node_queue_t    *queue;
    queue = &video_packet_queue->queue_node;
    if(queue->op != NULL){
        entry_number = queue->op->get_queue_curr_entry_number(queue);
    }
    return entry_number;
}

static void isil_video_packet_queue_release(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_packet_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        remove_isil_video_packet_tcb_queue(video_packet_queue, video_chan_buf_pool);
    }
}

static void isil_video_packet_queue_init(isil_video_packet_tcb_queue_t *video_packet_queue)
{
    if(video_packet_queue != NULL){
        tcb_node_queue_t    *queue;
        queue = &video_packet_queue->queue_node;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);
        video_packet_queue->curr_consumer = NULL;
        video_packet_queue->curr_producer = NULL;
        video_packet_queue->start_timestamp = 0;
        video_packet_queue->total_duration = 0;
        spin_lock_init(&video_packet_queue->lock);
    }
}

static struct isil_video_packet_queue_operation   isil_video_packet_queue_op = {
    .get = isil_video_packet_queue_get,
    .try_get = isil_video_packet_queue_try_get,
    .get_tailer = isil_video_packet_queue_get_tailer,
    .try_get_tailer = isil_video_packet_queue_try_get_tailer,
    .put = isil_video_packet_queue_put,
    .put_header = isil_video_packet_queue_put_header,

    .get_curr_producer_from_pool = isil_video_packet_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool = isil_video_packet_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue = isil_video_packet_queue_put_curr_producer_into_queue,
    .release_curr_producer =  isil_video_packet_queue_release_curr_producer,
    .get_curr_consumer_from_queue = isil_video_packet_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue = isil_video_packet_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = isil_video_packet_queue_release_curr_consumer,

    .get_curr_queue_entry_number = isil_video_packet_queue_get_curr_queue_entry_number,
    .release = isil_video_packet_queue_release,
    .init = isil_video_packet_queue_init,
};

void    init_isil_video_packet_tcb_queue(isil_video_packet_tcb_queue_t *video_packet_queue)
{
    if(video_packet_queue != NULL){
        video_packet_queue->op = &isil_video_packet_queue_op;
        video_packet_queue->op->init(video_packet_queue);
    }
}

void    remove_isil_video_packet_tcb_queue(isil_video_packet_tcb_queue_t *video_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_packet_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        if(video_packet_queue->op != NULL){
            video_packet_queue->op->put_curr_producer_into_queue(video_packet_queue);
            video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
            while(video_packet_queue->op->get_curr_queue_entry_number(video_packet_queue)){
                video_packet_queue->op->get_curr_consumer_from_queue(video_packet_queue);
                if(video_packet_queue->curr_consumer == NULL){
                    break;
                }
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
            }
        } else {
            isil_video_packet_queue_put_curr_producer_into_queue(video_packet_queue);
            isil_video_packet_queue_release_curr_consumer(video_packet_queue, video_chan_buf_pool);
            while(isil_video_packet_queue_get_curr_queue_entry_number(video_packet_queue)){
                isil_video_packet_queue_get_curr_consumer_from_queue(video_packet_queue);
                if(video_packet_queue->curr_consumer == NULL){
                    break;
                }
                isil_video_packet_queue_release_curr_consumer(video_packet_queue, video_chan_buf_pool);
            }
        }
    }
}

static void isil_video_frame_tcb_nal_init(h264_nal_t *nal)
{
    if(nal != NULL){
        nal->i_ref_idc = 0;
        nal->i_type = 0;
        nal->i_temp_bitalign_number = 0;
        nal->i_temp_bitalign_content = 0;
        memset(nal, 0, sizeof(h264_nal_t));
        nal->sps_payload = nal->pps_payload = nal->slice_payload = nal->slice_head_buf = NULL;
        nal->sps_paysize = nal->pps_paysize = nal->slice_paysize = 0;
    }
}

static void isil_video_frame_tcb_init(isil_video_frame_tcb_t *frame)
{
    if(frame!=NULL){
        tcb_node_t	*node = &frame->frame_node;
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, frame);

        spin_lock_init(&frame->lock);
        atomic_set(&frame->ref_count, 0);
        frame->frame_type = H264_FRAME_TYPE_IDR;
        isil_video_frame_tcb_nal_init(&frame->nal);
        frame->frame_len = 0;
        frame->consumer_frame_offset = 0;
        frame->timestamp = 0;
        frame->duration = 0;
        frame->i_init_qp = DEFAULT_QP;
        frame->i_curr_qp = 0;
        frame->i_mb_x = 0;
        frame->i_mb_y = 0;
        frame->i_log_gop_value = 0;
        frame->slice_type = NAL_SLICE_IDR;
        frame->fps = 0;
        frame->frame_number = 0;
        frame->frame_is_err = 0;
        init_isil_video_packet_tcb_queue(&frame->video_packet_queue);
    }
}

static void isil_video_frame_tcb_reset(isil_video_frame_tcb_t *frame)
{
    if(frame != NULL){
        atomic_set(&frame->ref_count, 0);
        frame->frame_type = H264_FRAME_TYPE_IDR;
        isil_video_frame_tcb_nal_init(&frame->nal);
        frame->frame_len = 0;
        frame->consumer_frame_offset = 0;
        frame->timestamp = 0;
        frame->duration = 0;
        frame->i_init_qp = DEFAULT_QP;
        frame->i_curr_qp = 0;
        frame->i_mb_x = 0;
        frame->i_mb_y = 0;
        frame->i_log_gop_value = 0;
        frame->slice_type = NAL_SLICE_IDR;
        frame->fps = 0;
        frame->frame_number = 0;
        frame->frame_is_err = 0;
    }
}

static void isil_video_frame_tcb_release(isil_video_frame_tcb_t **ptr_frame, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if(ptr_frame != NULL){
        isil_video_frame_tcb_t    *frame = *ptr_frame;
        unsigned long	flags;
        if(video_chan_buf_pool == NULL){
            video_chan_buf_pool = frame->video_chan_buf_pool;
        }
        spin_lock_irqsave(&frame->lock, flags);
        if(atomic_read(&frame->ref_count) > 1){
            atomic_dec(&frame->ref_count);
        } else {
            tcb_node_t	*node;
            if(frame->op != NULL){
                frame->op->reset(frame);
            }
            if(frame->video_packet_queue.op != NULL){
                frame->video_packet_queue.op->release(&frame->video_packet_queue, video_chan_buf_pool);
            } else {
                isil_video_packet_queue_release(&frame->video_packet_queue, video_chan_buf_pool);
            }
            node = &frame->frame_node;
            if(node->op != NULL){
                node->op->release(node, &video_chan_buf_pool->video_frame_pool_tcb);
            }
        }
        *ptr_frame = NULL;
        spin_unlock_irqrestore(&frame->lock, flags);
    }
}

static void isil_video_frame_tcb_reference(isil_video_frame_tcb_t *src_frame, isil_video_frame_tcb_t **ptr_dest_frame)
{
    if((src_frame!=NULL) && (ptr_dest_frame!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&src_frame->lock, flags);
        atomic_inc(&src_frame->ref_count);
        *ptr_dest_frame = src_frame;
        spin_unlock_irqrestore(&src_frame->lock, flags);
    }
}

#ifndef  NEW_HEADER
static size_t	isil_video_frame_tcb_submit(isil_video_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    isil_frame_header_t __user    *frame_header_ptr;
    isil_frame_header_t           isil_frame_head;
    isil_h264_idr_frame_pad_t __user  *idr_pad_ptr;
    isil_h264_idr_frame_pad_t     idr_frame_pad;
    isil_video_packet_tcb_queue_t *video_packet_queue;
    h264_nal_t  *nal;
    size_t  buf_size, packet_size;
    loff_t  buf_offset, packet_offset;
    unsigned int expect_len = sizeof(isil_frame_header_t) + sizeof(isil_h264_idr_frame_pad_t) + frame->frame_len;

    buf_size = count;
    buf_offset = 0;
    expect_len = tc_align_up(expect_len,4);

    if(frame != NULL){
        nal = &frame->nal;
        frame_header_ptr = (isil_frame_header_t*)data;
        buf_size -= sizeof(isil_frame_header_t);
        buf_offset += sizeof(isil_frame_header_t);
        memset(&isil_frame_head, 0, sizeof(isil_frame_header_t));
        isil_frame_head.codecType = ISIL_VIDEO_H264_CODEC_TYPE;
        isil_frame_head.streamType = masterOrSubFlag;
        isil_frame_head.frameSerial = frame->frame_number;
        isil_frame_head.frameType = frame->frame_type;
        isil_frame_head.timeStamp = frame->timestamp;
        isil_frame_head.payload_offset = sizeof(isil_frame_header_t);
        isil_frame_head.payloadLen = 0;
        switch(frame->frame_type) {
            case H264_FRAME_TYPE_IDR:
                if(count >= expect_len){
                    idr_pad_ptr = (isil_h264_idr_frame_pad_t*)frame_header_ptr->pad;
                    buf_size -= sizeof(isil_h264_idr_frame_pad_t);
                    buf_offset += sizeof(isil_h264_idr_frame_pad_t);
                    isil_frame_head.payloadLen += sizeof(isil_h264_idr_frame_pad_t);

                    memset(&idr_frame_pad, 0, sizeof(isil_h264_idr_frame_pad_t));
                    idr_frame_pad.fps = frame->fps;
                    idr_frame_pad.init_qp = frame->i_init_qp;
                    idr_frame_pad.log2MaxFrameNumNinus4 = frame->i_log_gop_value;
                    idr_frame_pad.mb_width_minus1 = frame->i_mb_x;
                    idr_frame_pad.mb_height_minus1 = frame->i_mb_y;
                    idr_frame_pad.i_curr_qp = frame->i_curr_qp;

                    idr_frame_pad.sps_frame_offset = sizeof(isil_h264_idr_frame_pad_t);
                    idr_frame_pad.sps_frame_size = nal->sps_paysize;
                    idr_frame_pad.pps_frame_offset = idr_frame_pad.sps_frame_offset + nal->sps_paysize;
                    idr_frame_pad.pps_frame_size = nal->pps_paysize;
                    idr_frame_pad.idr_frame_offset = idr_frame_pad.pps_frame_offset + nal->pps_paysize;
                    idr_frame_pad.idr_frame_size = 0;

                    video_packet_queue = &frame->video_packet_queue;
                    while(video_packet_queue->op->get_curr_queue_entry_number(video_packet_queue)){
                        video_packet_queue->op->get_curr_consumer_from_queue(video_packet_queue);
                        packet_size = video_packet_queue->curr_consumer->op->submit(video_packet_queue->curr_consumer, &data[buf_offset], buf_size, &packet_offset, nal);
                        video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                        buf_size -= packet_size;
                        buf_offset += packet_size;
                        isil_frame_head.payloadLen += packet_size;
                    }
                    while(buf_offset & 3){
                        __put_user(0, &data[buf_offset]);
                        buf_size--;
                        buf_offset++;
                        idr_frame_pad.idr_frame_size++;
                        isil_frame_head.payloadLen++;
                    }
                    idr_frame_pad.idr_frame_size += frame->frame_len;
                    if(copy_to_user(frame_header_ptr->pad, (char*)&idr_frame_pad, sizeof(isil_h264_idr_frame_pad_t)) != 0){
                        printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                    }
                } else {
                    buf_offset = 0;
                }
                break;
            case H264_FRAME_TYPE_I:
            case H264_FRAME_TYPE_P:
                if(count >= (sizeof(isil_frame_header_t)+sizeof(isil_h264_idr_frame_pad_t)+frame->frame_len)){
                    video_packet_queue = &frame->video_packet_queue;
                    while(video_packet_queue->op->get_curr_queue_entry_number(video_packet_queue)){
                        video_packet_queue->op->get_curr_consumer_from_queue(video_packet_queue);
                        packet_size = video_packet_queue->curr_consumer->op->submit(video_packet_queue->curr_consumer, &data[buf_offset], buf_size, &packet_offset, nal);
                        video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
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
                } else {
                    buf_offset = 0;
                }
                break;
            default:
                printk("%s.%d: err 264 stream type\n", __FUNCTION__, __LINE__);
                break;
        }
        if(buf_offset){
            if(copy_to_user(frame_header_ptr, &isil_frame_head, sizeof(isil_frame_header_t)) != 0){
                printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
            }
        }
    }
    *ppos += buf_offset;
    return (size_t)buf_offset;
}
#else

static size_t	isil_video_frame_tcb_submit(isil_video_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    struct isil_common_stream_header  icsh, *picsh;
    isil_transparent_msg_header_t  mh, *pmh;
    isil_h264_encode_msg_frame_type_t  emft_sps = {0}, emft_pps = {0}, emft_slice = {0}, *pemft;
    isil_frame_msg_t  fm_sps = {0}, fm_pps = {0}, fm_slice = {0}, *pfm;
    isil_video_packet_tcb_queue_t  *video_packet_queue;
    h264_nal_t  *nal;
    size_t  buf_size, packet_size, l;
    loff_t  buf_offset, packet_offset;
    char *pc;
    unsigned int expect_len;// frame->frame_len is not include sps_paysize and pps_paysize
    struct isil_h264_logic_encode_chan *encode_chan;

    if(!data || !ppos || !video_chan_buf_pool)
    	goto exception;

    encode_chan = container_of(video_chan_buf_pool,struct isil_h264_logic_encode_chan, encode_chan_buf_pool);
    buf_size = count;
    buf_offset = 0;
    picsh = &icsh;
    pmh = &mh;

    if(frame){
        if(frame->frame_type == H264_FRAME_TYPE_I || frame->frame_type == H264_FRAME_TYPE_P){
            pemft = &emft_slice;
            pfm = &fm_slice;
            nal = &frame->nal;

            buf_offset += sizeof(struct isil_common_stream_header);
            buf_size -= sizeof(struct isil_common_stream_header);
            picsh->stream_type = ISIL_H264_STERAM;
            picsh->stream_len = sizeof(struct isil_common_stream_header);

            buf_offset += sizeof(isil_transparent_msg_header_t);
            buf_size -= sizeof(isil_transparent_msg_header_t);
            pmh->isil_transparent_msg_number = 2;///////
            picsh->stream_len += sizeof(isil_transparent_msg_header_t);
            pmh->isil_transparent_msg_total_len = sizeof(isil_transparent_msg_header_t);

            buf_offset += sizeof(isil_h264_encode_msg_frame_type_t);
            buf_size -= sizeof(isil_h264_encode_msg_frame_type_t);
            pemft->msg_type = ISIL_H264_ENCODE_FRAME_TYPE;
            pemft->msg_len = sizeof(isil_h264_encode_msg_frame_type_t);
            pemft->frame_type = frame->frame_type + 2;
            pemft->i_mb_x = frame->i_mb_x;
            pemft->i_mb_y = frame->i_mb_y;
            picsh->stream_len += sizeof(isil_h264_encode_msg_frame_type_t);
            pmh->isil_transparent_msg_total_len += sizeof(isil_h264_encode_msg_frame_type_t);

            buf_offset += sizeof(isil_frame_msg_t);
            buf_size -= sizeof(isil_frame_msg_t);
            pfm->msg_type = ISIL_H264_ENCODE_FRAME_MSG;
            pfm->msg_len  = sizeof(isil_frame_msg_t);
            pfm->frame_serial = frame->frame_number;
            pfm->frame_timestamp = frame->timestamp;
            pfm->frame_checksam = 0;
            picsh->stream_len += sizeof(isil_frame_msg_t);
            pmh->isil_transparent_msg_total_len += sizeof(isil_frame_msg_t);

            //ISIL_DBG(ISIL_DBG_ERR,"frame_number[%d]\n",frame->frame_number);

            expect_len = tc_align_up(frame->frame_len, 4);
            if(expect_len <= buf_size){
                video_packet_queue = &frame->video_packet_queue;
                if(video_packet_queue->op && video_packet_queue->op->get_curr_queue_entry_number){
                while(video_packet_queue->op->get_curr_queue_entry_number(video_packet_queue)){
                    video_packet_queue->op->get_curr_consumer_from_queue(video_packet_queue);
                    pfm->frame_checksam = compute_accumulate_crc(pfm->frame_checksam, video_packet_queue->curr_consumer->data, video_packet_queue->curr_consumer->payload_len);
						if(encode_chan->if_slience_discard){
							printk("%s, %d slicent discard! frame->number:[%d]\n", __FUNCTION__, __LINE__, frame->frame_number);
							if(video_packet_queue->curr_consumer->op && video_packet_queue->curr_consumer->op->slicent_discard){
							    packet_size = video_packet_queue->curr_consumer->op->slicent_discard(video_packet_queue->curr_consumer, &data[buf_offset], buf_size, &packet_offset, nal);
							}else{
								printk("met NULL pointer in %s,,,%d frame->number:[%d]\n", __FUNCTION__, __LINE__, frame->frame_number);
								goto exception;
							}
						}else{
							if(video_packet_queue->curr_consumer->op && video_packet_queue->curr_consumer->op->submit){
								packet_size = video_packet_queue->curr_consumer->op->submit(video_packet_queue->curr_consumer, &data[buf_offset], buf_size, &packet_offset, nal);
							}else{
								printk("met NULL pointer in %s,,,%d frame->number:[%d]\n", __FUNCTION__, __LINE__, frame->frame_number);
								goto exception;
							}
						}
                    video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                    buf_size -= packet_size;
                    buf_offset += packet_size;
                    picsh->stream_len += packet_size;
                    pmh->isil_transparent_msg_total_len += packet_size;
                    pfm->msg_len += packet_size;
					}
					if(!encode_chan->if_slience_discard){
						while(buf_offset & 3){
							__put_user(0, &data[buf_offset]);
							buf_size--;
							buf_offset++;
							picsh->stream_len++;
							pmh->isil_transparent_msg_total_len++;
							pfm->msg_len++;
						}
					}else{
						//if the slience discard set, we may discard this frame, return 0 to user
						goto exception;
					}
                }else{
					printk("met NULL pointer in %s,,,%d\n", __FUNCTION__, __LINE__);
                	goto exception;
                }
            }else{
                printk("buffer size is smaller than one frame!!!");
                encode_chan->if_slience_discard = 1;
                goto exception;
            }

            picsh->stream_len |= ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG;
            picsh->stream_timestamp = frame->timestamp;

            /*finally we copy the header info to the head of the user buffer*/
            pc = (char *)data;
            if(copy_to_user(pc, &icsh, sizeof(struct isil_common_stream_header)) != 0){
                printk("%s, %d:struct isil_common_stream_header copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto exception;
            }
            pc += sizeof(struct isil_common_stream_header);
            if(copy_to_user(pc, &mh, sizeof(isil_transparent_msg_header_t)) != 0){
                printk("%s, %d:isil_transparent_msg_header_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto exception;
            }
            pc += sizeof(isil_transparent_msg_header_t);
            if(copy_to_user(pc, &emft_slice, sizeof(isil_h264_encode_msg_frame_type_t)) != 0){
                printk("%s, %d:isil_h264_encode_msg_frame_type_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto exception;
            }
            pc += sizeof(isil_h264_encode_msg_frame_type_t);
            if(copy_to_user(pc, &fm_slice, sizeof(isil_frame_msg_t)) != 0){
                printk("%s, %d:isil_frame_msg_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto exception;
            }
        }else if(frame->frame_type == H264_FRAME_TYPE_IDR){
        	if(encode_chan->if_slience_discard)
        		encode_chan->if_slience_discard  = 0;

            pemft = &emft_sps;
            pfm = &fm_sps;
            nal = &frame->nal;
            buf_offset += sizeof(struct isil_common_stream_header);
            buf_size -= sizeof(struct isil_common_stream_header);
            picsh->stream_type = ISIL_H264_STERAM;
            picsh->stream_len = sizeof(struct isil_common_stream_header);

            buf_offset += sizeof(isil_transparent_msg_header_t);
            buf_size -= sizeof(isil_transparent_msg_header_t);
            pmh->isil_transparent_msg_number = 6;///////
            picsh->stream_len += sizeof(isil_transparent_msg_header_t);
            pmh->isil_transparent_msg_total_len = sizeof(isil_transparent_msg_header_t);

            /*sps frame*/
            pc = (char *)&data[buf_offset];
            buf_offset += sizeof(isil_h264_encode_msg_frame_type_t);
            buf_size -= sizeof(isil_h264_encode_msg_frame_type_t);
            pemft->msg_type = ISIL_H264_ENCODE_FRAME_TYPE;
            pemft->msg_len = sizeof(isil_h264_encode_msg_frame_type_t);
            pemft->frame_type = ISIL_SPS_FRAME;
            pemft->i_mb_x = frame->i_mb_x;
            pemft->i_mb_y = frame->i_mb_y;
            picsh->stream_len += sizeof(isil_h264_encode_msg_frame_type_t);
            pmh->isil_transparent_msg_total_len += sizeof(isil_h264_encode_msg_frame_type_t);

            buf_offset += sizeof(isil_frame_msg_t);
            buf_size -= sizeof(isil_frame_msg_t);
            pfm->msg_type = ISIL_H264_ENCODE_FRAME_MSG;
            pfm->msg_len  = sizeof(isil_frame_msg_t);
            pfm->frame_serial = frame->frame_number;
            pfm->frame_timestamp = frame->timestamp;
            pfm->frame_checksam = 0;
            picsh->stream_len += sizeof(isil_frame_msg_t);
            pmh->isil_transparent_msg_total_len += sizeof(isil_frame_msg_t);

            video_packet_queue = &frame->video_packet_queue;
            if(!video_packet_queue->op || !video_packet_queue->op->get_curr_consumer_from_queue){
				printk("met NULL pointer in %s,,,%d\n", __FUNCTION__, __LINE__);
				goto exception;
            }
            video_packet_queue->op->get_curr_consumer_from_queue(video_packet_queue);
            if(NULL == video_packet_queue->curr_consumer){
                printk("BUG! video frame has no packet!");
                goto exception;
            }
            pfm->frame_checksam = compute_crc(video_packet_queue->curr_consumer->data, nal->sps_paysize);
            if(copy_to_user(&data[buf_offset],video_packet_queue->curr_consumer->data, nal->sps_paysize)){
                printk("%s, %d:sps copy_to_user failed\n", __FUNCTION__, __LINE__);
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                goto exception;
            }
            pfm->msg_len += nal->sps_paysize;
            if(copy_to_user(pc, &emft_sps, sizeof(isil_h264_encode_msg_frame_type_t)) != 0){
                printk("%s, %d:isil_h264_encode_msg_frame_type_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                goto exception;
            }
            pc += sizeof(isil_h264_encode_msg_frame_type_t);
            if(copy_to_user(pc, &fm_sps, sizeof(isil_frame_msg_t)) != 0){
                printk("%s, %d:isil_frame_msg_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                goto exception;
            }      


            /*pps frame*/
            buf_offset += nal->sps_paysize;
            buf_size -= nal->sps_paysize;
            picsh->stream_len += nal->sps_paysize;
            pmh->isil_transparent_msg_total_len += nal->sps_paysize;
            pemft = &emft_pps;
            pfm = &fm_pps;
            pc = (char *)&data[buf_offset];

            buf_offset += sizeof(isil_h264_encode_msg_frame_type_t);
            buf_size -= sizeof(isil_h264_encode_msg_frame_type_t);
            pemft->msg_type = ISIL_H264_ENCODE_FRAME_TYPE;
            pemft->msg_len = sizeof(isil_h264_encode_msg_frame_type_t);
            pemft->frame_type = ISIL_PPS_FRAME;
            pemft->i_mb_x = frame->i_mb_x;
            pemft->i_mb_y = frame->i_mb_y;
            picsh->stream_len += sizeof(isil_h264_encode_msg_frame_type_t);
            pmh->isil_transparent_msg_total_len += sizeof(isil_h264_encode_msg_frame_type_t);

            buf_offset += sizeof(isil_frame_msg_t);
            buf_size -= sizeof(isil_frame_msg_t);
            pfm->msg_type = ISIL_H264_ENCODE_FRAME_MSG;
            pfm->msg_len  = sizeof(isil_frame_msg_t);
            pfm->frame_serial = frame->frame_number;
            pfm->frame_timestamp = frame->timestamp;
            pfm->frame_checksam = 0;
            picsh->stream_len += sizeof(isil_frame_msg_t);
            pmh->isil_transparent_msg_total_len += sizeof(isil_frame_msg_t);

            pfm->frame_checksam = compute_crc(&video_packet_queue->curr_consumer->data[nal->sps_paysize], nal->pps_paysize);
            if(copy_to_user(&data[buf_offset],&video_packet_queue->curr_consumer->data[nal->sps_paysize], nal->pps_paysize)){
                printk("%s, %d:pps copy_to_user failed\n", __FUNCTION__, __LINE__);
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                goto exception;
            }
            pfm->msg_len += nal->pps_paysize;
            if(copy_to_user(pc, &emft_pps, sizeof(isil_h264_encode_msg_frame_type_t)) != 0){
                printk("%s, %d:isil_h264_encode_msg_frame_type_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                goto exception;
            }
            pc += sizeof(isil_h264_encode_msg_frame_type_t);
            if(copy_to_user(pc, &fm_pps, sizeof(isil_frame_msg_t)) != 0){
                printk("%s, %d:isil_frame_msg_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                goto exception;
            }

            /*IDR slice frame*/
            buf_offset += nal->pps_paysize;
            buf_size -= nal->pps_paysize;
            picsh->stream_len += nal->pps_paysize;
            pmh->isil_transparent_msg_total_len += nal->pps_paysize;
            pemft = &emft_slice;
            pfm = &fm_slice;
            pc = (char *)&data[buf_offset];

            buf_offset += sizeof(isil_h264_encode_msg_frame_type_t);
            buf_size -= sizeof(isil_h264_encode_msg_frame_type_t);
            pemft->msg_type = ISIL_H264_ENCODE_FRAME_TYPE;
            pemft->msg_len = sizeof(isil_h264_encode_msg_frame_type_t);
            pemft->frame_type = ISIL_IDR_FRAME;
            pemft->i_mb_x = frame->i_mb_x;
            pemft->i_mb_y = frame->i_mb_y;
            picsh->stream_len += sizeof(isil_h264_encode_msg_frame_type_t);
            pmh->isil_transparent_msg_total_len += sizeof(isil_h264_encode_msg_frame_type_t);

            buf_offset += sizeof(isil_frame_msg_t);
            buf_size -= sizeof(isil_frame_msg_t);
            pfm->msg_type = ISIL_H264_ENCODE_FRAME_MSG;
            pfm->msg_len  = sizeof(isil_frame_msg_t);
            pfm->frame_serial = frame->frame_number;
            pfm->frame_timestamp = frame->timestamp;
            pfm->frame_checksam = 0;
            picsh->stream_len += sizeof(isil_frame_msg_t);
            pmh->isil_transparent_msg_total_len += sizeof(isil_frame_msg_t);

            expect_len = tc_align_up(frame->frame_len, 4);
            if(expect_len > buf_size){
                printk("no enough buffer lefted in function %s!\n", __FUNCTION__);
                encode_chan->if_slience_discard  = 1;
            }

            /*copy the rest of the first packet, then release it*/
            l = video_packet_queue->curr_consumer->payload_len - (nal->sps_paysize + nal->pps_paysize);
            pfm->frame_checksam = compute_crc(&video_packet_queue->curr_consumer->data[nal->sps_paysize + nal->pps_paysize], l);
            if(copy_to_user(&data[buf_offset],&video_packet_queue->curr_consumer->data[nal->sps_paysize + nal->pps_paysize], l)){
                printk("%s, %d:pps copy_to_user failed\n", __FUNCTION__, __LINE__);
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                goto exception;
            }
            if(!video_packet_queue->op || !video_packet_queue->op->release_curr_consumer){
            	printk("BUG: previous is ok, but now met NULL pointer! in %s,,line %d\n", __FUNCTION__, __LINE__);
            	goto exception;
            }
            video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
            buf_offset += l;
            pfm->msg_len += l;
            picsh->stream_len += l;
            pmh->isil_transparent_msg_total_len += l;

            /*copey the rest packets*/
            if(!video_packet_queue->op || !video_packet_queue->op->get_curr_queue_entry_number){
            	printk("BUG: previous is ok, but now met NULL pointer! in %s,,line %d\n", __FUNCTION__, __LINE__);
            	goto exception;
            }
            while(video_packet_queue->op->get_curr_queue_entry_number(video_packet_queue)){
                video_packet_queue->op->get_curr_consumer_from_queue(video_packet_queue);
                pfm->frame_checksam = compute_accumulate_crc(pfm->frame_checksam, video_packet_queue->curr_consumer->data, video_packet_queue->curr_consumer->payload_len);
                if(encode_chan->if_slience_discard){
                	if(!video_packet_queue->curr_consumer->op || !video_packet_queue->curr_consumer->op->slicent_discard){
                    	printk("BUG: previous is ok, but now met NULL pointer! in %s,,line %d\n", __FUNCTION__, __LINE__);
                    	goto exception;
                	}
                	packet_size = video_packet_queue->curr_consumer->op->slicent_discard(video_packet_queue->curr_consumer, &data[buf_offset], buf_size, &packet_offset, nal);
                }else{
                	if(!video_packet_queue->curr_consumer->op || !video_packet_queue->curr_consumer->op->submit){
                    	printk("BUG: previous is ok, but now met NULL pointer! in %s,,line %d\n", __FUNCTION__, __LINE__);
                    	goto exception;
                	}
                    packet_size = video_packet_queue->curr_consumer->op->submit(video_packet_queue->curr_consumer, &data[buf_offset], buf_size, &packet_offset, nal);
                }
                if(!video_packet_queue->op || !video_packet_queue->op->release_curr_consumer){
                	printk("BUG: previous is ok, but now met NULL pointer! in %s,,line %d\n", __FUNCTION__, __LINE__);
                	goto exception;
                }
                video_packet_queue->op->release_curr_consumer(video_packet_queue, video_chan_buf_pool);
                buf_size -= packet_size;
                buf_offset += packet_size;
                picsh->stream_len += packet_size;
                pmh->isil_transparent_msg_total_len += packet_size;
                pfm->msg_len += packet_size;
            }
            if(!encode_chan->if_slience_discard){
                while(buf_offset & 3){
                    __put_user(0, &data[buf_offset]);
                    buf_size--;
                    buf_offset++;
                    picsh->stream_len++;
                    pmh->isil_transparent_msg_total_len++;
                    pfm->msg_len++;
                }
            }else{
                goto exception;
            }

            picsh->stream_len |= ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG;
            if(copy_to_user(pc, &emft_slice, sizeof(isil_h264_encode_msg_frame_type_t)) != 0){
                printk("%s, %d:isil_h264_encode_msg_frame_type_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto exception;
            }
            pc += sizeof(isil_h264_encode_msg_frame_type_t);
            if(copy_to_user(pc, &fm_slice, sizeof(isil_frame_msg_t)) != 0){
                printk("%s, %d:isil_frame_msg_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto exception;
            }

            pc = (char *)data;
            if(copy_to_user(pc, &icsh, sizeof(struct isil_common_stream_header)) != 0){
                printk("%s, %d:struct isil_common_stream_header copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto exception;
            }
            pc += sizeof(struct isil_common_stream_header);
            if(copy_to_user(pc, &mh, sizeof(isil_transparent_msg_header_t)) != 0){
                printk("%s, %d:isil_transparent_msg_header_t copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto exception;
            }
        }
    }else{
        printk("frame is NULL in %s\n",__FUNCTION__);
    }

    *ppos += buf_offset;
    return (size_t)buf_offset;
exception:
    *ppos = 0;
    return 0;
}
#endif



static int isil_video_frame_tcb_copy(isil_video_frame_tcb_t *src, isil_video_frame_tcb_t *dst)
{
    if(src && dst) {
        u32 entry_number;
        int i;
        h264_nal_t *src_nal, *dst_nal;
        isil_video_chan_buf_pool_t *video_chan_buf_pool;
        isil_video_packet_tcb_queue_t	*src_packet_queue, *dst_packet_queue;

        src_packet_queue = &src->video_packet_queue;
        dst_packet_queue = &dst->video_packet_queue;

        video_chan_buf_pool = dst->video_chan_buf_pool;
        entry_number = src_packet_queue->op->get_curr_queue_entry_number(src_packet_queue);
        for(i = 0; i < entry_number; i++) {
            isil_video_packet_tcb_t *src_packet, *dst_packet;

            video_chan_buf_pool->op->try_get_video_packet_tcb(video_chan_buf_pool, &dst_packet);
            src_packet_queue->op->try_get(src_packet_queue, &src_packet);
            if(src_packet && dst_packet) {
                src_packet->op->copy(src_packet, dst_packet);
                dst_packet_queue->op->put(dst_packet_queue, dst_packet);
            }else{
                dst->frame_is_err = 1;
            }
            src_packet_queue->op->put(src_packet_queue, src_packet);
        }

        dst->b_is_display           = src->b_is_display;
        dst->b_is_gop_end           = src->b_is_gop_end;
        dst->checksum               = src->checksum;
        dst->consumer_frame_offset  = src->consumer_frame_offset;
        dst->fps                    = src->fps;
        if(dst->frame_is_err == 0) {
            dst->frame_is_err           = src->frame_is_err;
        }
        dst->frame_len              = src->frame_len;
        dst->frame_number           = src->frame_number;
        dst->frame_type             = src->frame_type;
        dst->i_curr_qp              = src->i_curr_qp;
        dst->i_init_qp              = src->i_init_qp;
        dst->i_log_gop_value        = src->i_log_gop_value;
        dst->i_mb_x                 = src->i_mb_x;
        dst->i_mb_y                 = src->i_mb_y;
        dst->in_ddr_addr            = src->in_ddr_addr;
        dst->slice_type             = src->slice_type;
        dst->in_ddr_addr            = src->in_ddr_addr;
        dst->timestamp              = src->timestamp;
        dst->duration               = src->duration;
        memcpy(&dst->time_pad, &src->time_pad, sizeof(ISIL_FRAME_TIME_PAD));

        src_nal = &src->nal;
        dst_nal = &dst->nal;
        dst_nal->header_align_number     = src_nal->header_align_number;
        dst_nal->i_end_bitalign_number   = src_nal->i_end_bitalign_number;
        dst_nal->i_ref_idc               = src_nal->i_ref_idc;
        dst_nal->i_temp_bitalign_content = src_nal->i_temp_bitalign_content;
        dst_nal->i_temp_bitalign_number  = src_nal->i_temp_bitalign_number;
        dst_nal->i_type                  = src_nal->i_type;
        dst_nal->pps_paysize             = src_nal->pps_paysize;
        dst_nal->slice_paysize           = src_nal->slice_paysize;
        dst_nal->sps_paysize             = src_nal->sps_paysize;
        dst_nal->slice_head_buf          = NULL;
        dst_nal->sps_payload             = NULL;
        dst_nal->pps_payload             = NULL;

        if(dst->frame_is_err) {
            return ISIL_ERR;
        }
        return ISIL_OK;
    }

    return ISIL_ERR;
}
static struct isil_video_frame_tcb_operation  isil_video_frame_tcb_op = {
    .init = isil_video_frame_tcb_init,
    .reset = isil_video_frame_tcb_reset,
    .release = isil_video_frame_tcb_release,
    .reference = isil_video_frame_tcb_reference,
    .submit = isil_video_frame_tcb_submit,
    .copy   = isil_video_frame_tcb_copy,
};

static void isil_video_frame_queue_get(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_frame_tcb_t **ptr_frame)
{
    if((video_frame_tcb_queue!=NULL) && (ptr_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_frame = NULL;
        queue = &video_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_video_frame_tcb(temp_node);
            }
        }
    } else {
        printk("\n%s.%d**attention**\n", __FUNCTION__, __LINE__);
    }
}

static void isil_video_frame_queue_try_get(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_frame_tcb_t **ptr_frame)
{
    if((video_frame_tcb_queue!=NULL) && (ptr_frame!=NULL)){
        tcb_node_queue_t    *queue;
        *ptr_frame = NULL;
        queue = &video_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_video_frame_tcb(temp_node);
            }
        }
    }
}

static void isil_video_frame_queue_get_tailer(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_frame_tcb_t **ptr_frame)
{
    if((video_frame_tcb_queue!=NULL) && (ptr_frame!=NULL)){
        tcb_node_queue_t    *queue;
        *ptr_frame = NULL;
        queue = &video_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_video_frame_tcb(temp_node);
            }
        }
    }
}

static void isil_video_frame_queue_try_get_tailer(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_frame_tcb_t **ptr_frame)
{
    if((video_frame_tcb_queue!=NULL) && (ptr_frame!=NULL)){
        tcb_node_queue_t    *queue;
        *ptr_frame = NULL;
        queue = &video_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_video_frame_tcb(temp_node);
            }
        }
    }
}

static void isil_video_frame_queue_put(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_frame_tcb_t *frame)
{
    if((video_frame_tcb_queue!=NULL) && (frame!=NULL)){
        tcb_node_queue_t    *queue;
        queue = &video_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put(queue, &frame->frame_node);
        }
    }
}

static void isil_video_frame_queue_put_header(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_frame_tcb_t *frame)
{
    if((video_frame_tcb_queue!=NULL) && (frame!=NULL)){
        tcb_node_queue_t    *queue;
        queue = &video_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put_header(queue, &frame->frame_node);
        }
    }
}

static void isil_video_frame_queue_get_curr_producer_from_pool(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        if(video_frame_tcb_queue->curr_producer == NULL){
            spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
            if(video_chan_buf_pool->op != NULL){
                video_chan_buf_pool->op->get_video_frame_tcb(video_chan_buf_pool, &video_frame_tcb_queue->curr_producer);
            }
            spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        }
        spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
    }
}

static void isil_video_frame_queue_try_get_curr_producer_from_pool(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        if(video_frame_tcb_queue->curr_producer == NULL){
            if(video_chan_buf_pool->op != NULL){
                video_chan_buf_pool->op->try_get_video_frame_tcb(video_chan_buf_pool, &video_frame_tcb_queue->curr_producer);
            }
        }
        spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
    }
}

static void isil_video_frame_queue_put_curr_producer_into_queue(isil_video_frame_tcb_queue_t *video_frame_tcb_queue)
{
    if(video_frame_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        if(video_frame_tcb_queue->curr_producer != NULL){
            if(video_frame_tcb_queue->op != NULL){
                video_frame_tcb_queue->op->put(video_frame_tcb_queue, video_frame_tcb_queue->curr_producer);
            } else {
                isil_video_frame_queue_put(video_frame_tcb_queue, video_frame_tcb_queue->curr_producer);
            }
            video_frame_tcb_queue->curr_producer = NULL;
        }
        spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
    }
}

static void isil_video_frame_queue_release_curr_producer(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        if(video_frame_tcb_queue->curr_producer != NULL){
            if(video_frame_tcb_queue->curr_producer->op != NULL){
                video_frame_tcb_queue->curr_producer->op->release(&video_frame_tcb_queue->curr_producer, video_chan_buf_pool);
            } else {
                isil_video_frame_tcb_release(&video_frame_tcb_queue->curr_producer, video_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
    }
}

static void isil_video_frame_queue_get_curr_consumer_from_queue(isil_video_frame_tcb_queue_t *video_frame_tcb_queue)
{
    if(video_frame_tcb_queue != NULL){
        unsigned long	flags;

        spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        if(video_frame_tcb_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
            if(video_frame_tcb_queue->op != NULL){
                video_frame_tcb_queue->op->get(video_frame_tcb_queue, &video_frame_tcb_queue->curr_consumer);
            } else {
                isil_video_frame_queue_get(video_frame_tcb_queue, &video_frame_tcb_queue->curr_consumer);
            }
            spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        }
        spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
        //printk("get-->%d, %d, %x\n", video_frame_tcb_queue->op->get_curr_queue_entry_number(video_frame_tcb_queue), video_frame_tcb_queue->curr_consumer->timestamp, video_frame_tcb_queue->curr_consumer);
    } else {
        printk("\n%s.%d**attention**\n", __FUNCTION__, __LINE__);
    }
}

static void isil_video_frame_queue_try_get_curr_consumer_from_queue(isil_video_frame_tcb_queue_t *video_frame_tcb_queue)
{
    if(video_frame_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        if(video_frame_tcb_queue->curr_consumer == NULL){
            if(video_frame_tcb_queue->op != NULL){
                video_frame_tcb_queue->op->try_get(video_frame_tcb_queue, &video_frame_tcb_queue->curr_consumer);
            } else {
                isil_video_frame_queue_try_get(video_frame_tcb_queue, &video_frame_tcb_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
    }
}

static void isil_video_frame_queue_release_curr_consumer(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        if(video_frame_tcb_queue->curr_consumer != NULL){
            if(video_frame_tcb_queue->curr_consumer->op != NULL){
                video_frame_tcb_queue->curr_consumer->op->release(&video_frame_tcb_queue->curr_consumer, video_chan_buf_pool);
            } else {
                isil_video_frame_tcb_release(&video_frame_tcb_queue->curr_consumer, video_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
    }
}

static void isil_video_frame_queue_put_curr_consumer_into_queue_header(isil_video_frame_tcb_queue_t *video_frame_tcb_queue)
{
    if(video_frame_tcb_queue != NULL){
        unsigned long flags;
        spin_lock_irqsave(&video_frame_tcb_queue->lock, flags);
        if(video_frame_tcb_queue->curr_consumer != NULL){
            if(video_frame_tcb_queue->op != NULL){
                video_frame_tcb_queue->op->put_header(video_frame_tcb_queue, video_frame_tcb_queue->curr_consumer);
            } else {
                isil_video_frame_queue_put_header(video_frame_tcb_queue, video_frame_tcb_queue->curr_consumer);
            }
            video_frame_tcb_queue->curr_consumer = NULL;
        }
        spin_unlock_irqrestore(&video_frame_tcb_queue->lock, flags);
    }
}

static int  isil_video_frame_queue_get_curr_queue_entry_number(isil_video_frame_tcb_queue_t *video_frame_tcb_queue)
{
    int	entry_number = 0;
    if(video_frame_tcb_queue!=NULL){
        tcb_node_queue_t	*queue;
        queue = &video_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            entry_number = queue->op->get_queue_curr_entry_number(queue);
        }
    }
    return entry_number;
}

static void isil_video_frame_queue_release(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        remove_isil_video_frame_tcb_queue(video_frame_tcb_queue, video_chan_buf_pool);
    }
}

static void isil_video_frame_queue_init(isil_video_frame_tcb_queue_t *video_frame_tcb_queue)
{
    if(video_frame_tcb_queue != NULL){
        tcb_node_queue_t    *queue;
        queue = &video_frame_tcb_queue->queue_node;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);
        video_frame_tcb_queue->curr_consumer = NULL;
        video_frame_tcb_queue->curr_producer = NULL;
        video_frame_tcb_queue->start_timestamp = 0;
        video_frame_tcb_queue->total_duration = 0;
        spin_lock_init(&video_frame_tcb_queue->lock);
    }
}

static struct isil_video_frame_queue_operation	isil_video_frame_queue_op = {
    .get = isil_video_frame_queue_get,
    .try_get = isil_video_frame_queue_try_get,
    .get_tailer = isil_video_frame_queue_get_tailer,
    .try_get_tailer = isil_video_frame_queue_try_get_tailer,
    .put = isil_video_frame_queue_put,
    .put_header = isil_video_frame_queue_put_header,

    .get_curr_producer_from_pool = isil_video_frame_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool = isil_video_frame_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue = isil_video_frame_queue_put_curr_producer_into_queue,
    .release_curr_producer = isil_video_frame_queue_release_curr_producer,
    .get_curr_consumer_from_queue = isil_video_frame_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue = isil_video_frame_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = isil_video_frame_queue_release_curr_consumer,
    .put_curr_consumer_into_queue_header = isil_video_frame_queue_put_curr_consumer_into_queue_header,

    .get_curr_queue_entry_number = isil_video_frame_queue_get_curr_queue_entry_number,
    .release = isil_video_frame_queue_release,
    .init = isil_video_frame_queue_init,
};

void    init_isil_video_frame_tcb_queue(isil_video_frame_tcb_queue_t *video_frame_tcb_queue)
{
    if(video_frame_tcb_queue != NULL){
        video_frame_tcb_queue->op = &isil_video_frame_queue_op;
        video_frame_tcb_queue->op->init(video_frame_tcb_queue);
    }
}

void    remove_isil_video_frame_tcb_queue(isil_video_frame_tcb_queue_t *video_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        if(video_frame_tcb_queue->op != NULL){
            video_frame_tcb_queue->op->put_curr_producer_into_queue(video_frame_tcb_queue);
            video_frame_tcb_queue->op->release_curr_consumer(video_frame_tcb_queue, video_chan_buf_pool);
            while(video_frame_tcb_queue->op->get_curr_queue_entry_number(video_frame_tcb_queue)){
                video_frame_tcb_queue->op->get_curr_consumer_from_queue(video_frame_tcb_queue);
                if(video_frame_tcb_queue->curr_consumer == NULL){
                    break;
                }
                video_frame_tcb_queue->op->release_curr_consumer(video_frame_tcb_queue, video_chan_buf_pool);
            }
        } else {
            isil_video_frame_queue_put_curr_producer_into_queue(video_frame_tcb_queue);
            isil_video_frame_queue_release_curr_consumer(video_frame_tcb_queue, video_chan_buf_pool);
            while(isil_video_frame_queue_get_curr_queue_entry_number(video_frame_tcb_queue)){
                isil_video_frame_queue_get_curr_consumer_from_queue(video_frame_tcb_queue);
                if(video_frame_tcb_queue->curr_consumer == NULL){
                    break;
                }
                isil_video_frame_queue_release_curr_consumer(video_frame_tcb_queue, video_chan_buf_pool);
            }
        }
    }
}

#ifdef  MV_MODULE
static void isil_video_mv_packet_tcb_init(isil_video_mv_packet_tcb_t *packet, isil_video_chan_buf_pool_t *pool, int id)
{
    if((packet!=NULL) && (pool!=NULL)){
        tcb_node_t  *node = &packet->mv_packet_node;
        spin_lock_init(&packet->lock);
        atomic_set(&packet->ref_count, 0);
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, packet);
        packet->data_size = pool->mv_packet_buffer_size;
#ifdef  USE_DISCRETE_BUF
        packet->data = (u8*)__get_free_page(GFP_KERNEL);
        if(packet->data == NULL) {
            while(1) {
                printk("%s.%d: can't get one page, only for debug\n", __FUNCTION__, __LINE__);
            }
        }
#else
        packet->data = &pool->mv_cache_buffer[pool->mv_packet_buffer_size*id];
#endif
        packet->mvVector_buf = packet->data;
        packet->mvVector_len = 0;
        packet->dma_addr = 0;
    }
}

static void	isil_video_mv_packet_tcb_reset(isil_video_mv_packet_tcb_t *packet)
{
    if(packet!=NULL){
        packet->mvVector_buf = packet->data;
        packet->mvVector_len = 0;
        packet->dma_addr = 0;
    }
}

static void	isil_video_mv_packet_tcb_release(isil_video_mv_packet_tcb_t **ptr_packet, isil_video_chan_buf_pool_t *pool)
{
    if((*ptr_packet!=NULL) && (pool!=NULL)){
        isil_video_mv_packet_tcb_t	*packet = *ptr_packet;
        unsigned long	flags;

        spin_lock_irqsave(&packet->lock, flags);
        if(atomic_read(&packet->ref_count) > 1){
            atomic_dec(&packet->ref_count);
        } else {
            tcb_node_t	*node;
            if(packet->op != NULL){
                packet->op->reset(packet);
            }
            node = &packet->mv_packet_node;
            if(node->op != NULL){
                node->op->release(node, &pool->mv_packet_pool_tcb);
            }
            *ptr_packet = NULL;
        }
        spin_unlock_irqrestore(&packet->lock, flags);
    }
}

static void	isil_video_mv_packet_tcb_reference(isil_video_mv_packet_tcb_t *src_packet, isil_video_mv_packet_tcb_t **ptr_dest_packet)
{
    if((src_packet!=NULL) && (ptr_dest_packet!=NULL)){
        unsigned long	flags;

        spin_lock_irqsave(&src_packet->lock, flags);
        atomic_inc(&src_packet->ref_count);
        *ptr_dest_packet = src_packet;
        spin_unlock_irqrestore(&src_packet->lock, flags);
    }
}

static void	isil_video_mv_packet_tcb_dma_map(isil_video_mv_packet_tcb_t *packet, enum dma_data_direction direction)
{
    if(packet != NULL) {
        if(packet->data != NULL) {
            packet->dma_addr = dma_map_single(NULL, packet->data, packet->data_size, direction);
            if (packet->dma_addr == 0){
                printk("dma_map_single err\n");
            }
        }
    }
}

static void	isil_video_mv_packet_tcb_dma_unmap(isil_video_mv_packet_tcb_t *packet, enum dma_data_direction direction)
{
    if(packet != NULL) {
        if (packet->dma_addr != 0) {
            dma_unmap_single(NULL, packet->dma_addr, packet->data_size, direction);
            packet->dma_addr = 0;
        }
    }
}

static size_t  isil_video_mv_packet_tcb_submit(isil_video_mv_packet_tcb_t *packet, char __user *data, size_t count, loff_t *ppos)
{
    loff_t  buf_offset = 0;
    if(packet != NULL){
        if(copy_to_user(data, packet->mvVector_buf, packet->mvVector_len)){
            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
        }
        buf_offset = packet->mvVector_len;
    }
    *ppos += buf_offset;
    return (size_t)buf_offset;
}

static struct isil_video_mv_packet_tcb_operation	isil_video_mv_packet_tcb_op = {
    .init = isil_video_mv_packet_tcb_init,
    .reset = isil_video_mv_packet_tcb_reset,
    .release = isil_video_mv_packet_tcb_release,
    .reference = isil_video_mv_packet_tcb_reference,

    .dma_map = isil_video_mv_packet_tcb_dma_map,
    .dma_unmap = isil_video_mv_packet_tcb_dma_unmap,

    .submit = isil_video_mv_packet_tcb_submit,
};

static void	isil_video_mv_packet_tcb_queue_get(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_mv_packet_tcb_t **ptr_video_mv_packet_tcb)
{
    if((video_mv_packet_tcb_queue!=NULL) && (ptr_video_mv_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_mv_packet_tcb = NULL;
        queue = &video_mv_packet_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_mv_packet_tcb = to_get_isil_video_mv_packet_tcb(temp_node);
            }
        }
    }
}

static void	isil_video_mv_packet_tcb_queue_try_get(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_mv_packet_tcb_t **ptr_video_mv_packet_tcb)
{
    if((video_mv_packet_tcb_queue!=NULL) && (ptr_video_mv_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_mv_packet_tcb = NULL;
        queue = &video_mv_packet_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_mv_packet_tcb = to_get_isil_video_mv_packet_tcb(temp_node);
            }
        }
    }
}

static void	isil_video_mv_packet_tcb_queue_get_tailer(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_mv_packet_tcb_t **ptr_video_mv_packet_tcb)
{
    if((video_mv_packet_tcb_queue!=NULL) && (ptr_video_mv_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_mv_packet_tcb = NULL;
        queue = &video_mv_packet_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_mv_packet_tcb = to_get_isil_video_mv_packet_tcb(temp_node);
            }
        }
    }
}

static void	isil_video_mv_packet_tcb_queue_try_get_tailer(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_mv_packet_tcb_t **ptr_video_mv_packet_tcb)
{
    if((video_mv_packet_tcb_queue!=NULL) && (ptr_video_mv_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_mv_packet_tcb = NULL;
        queue = &video_mv_packet_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_mv_packet_tcb = to_get_isil_video_mv_packet_tcb(temp_node);
            }
        }
    }
}

static void	isil_video_mv_packet_tcb_queue_put(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_mv_packet_tcb_t *video_mv_packet_tcb)
{
    if((video_mv_packet_tcb_queue!=NULL) && (video_mv_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &video_mv_packet_tcb_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put(queue, &video_mv_packet_tcb->mv_packet_node);
        }
    }
}

static void	isil_video_mv_packet_tcb_queue_put_header(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_mv_packet_tcb_t *video_mv_packet_tcb)
{
    if((video_mv_packet_tcb_queue!=NULL) && (video_mv_packet_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &video_mv_packet_tcb_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put_header(queue, &video_mv_packet_tcb->mv_packet_node);
        }
    }
}

static void	isil_video_mv_packet_tcb_queue_get_curr_producer_from_pool(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_packet_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        if(video_mv_packet_tcb_queue->curr_producer == NULL){
            spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
            if(video_chan_buf_pool->op != NULL){
                video_chan_buf_pool->op->get_video_mv_packet_tcb(video_chan_buf_pool, &video_mv_packet_tcb_queue->curr_producer);
            }
            spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        }
        spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_packet_tcb_queue_try_get_curr_producer_from_pool(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_packet_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        if(video_mv_packet_tcb_queue->curr_producer == NULL){
            if(video_chan_buf_pool->op != NULL){
                video_chan_buf_pool->op->try_get_video_mv_packet_tcb(video_chan_buf_pool, &video_mv_packet_tcb_queue->curr_producer);
            }
        }
        spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_packet_tcb_queue_put_curr_producer_into_queue(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue)
{
    if(video_mv_packet_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        if(video_mv_packet_tcb_queue->curr_producer != NULL){
            if(video_mv_packet_tcb_queue->op != NULL){
                video_mv_packet_tcb_queue->op->put(video_mv_packet_tcb_queue, video_mv_packet_tcb_queue->curr_producer);
            } else {
                isil_video_mv_packet_tcb_queue_put(video_mv_packet_tcb_queue, video_mv_packet_tcb_queue->curr_producer);
            }
            video_mv_packet_tcb_queue->curr_producer = NULL;
        }
        spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_packet_tcb_queue_release_curr_producer(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_packet_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        if(video_mv_packet_tcb_queue->curr_producer != NULL){
            if(video_mv_packet_tcb_queue->curr_producer->op != NULL){
                video_mv_packet_tcb_queue->curr_producer->op->release(&video_mv_packet_tcb_queue->curr_producer, video_chan_buf_pool);
            } else {
                isil_video_mv_packet_tcb_release(&video_mv_packet_tcb_queue->curr_producer, video_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_packet_tcb_queue_get_curr_consumer_from_queue(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue)
{
    if(video_mv_packet_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        if(video_mv_packet_tcb_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
            if(video_mv_packet_tcb_queue->op != NULL){
                video_mv_packet_tcb_queue->op->get(video_mv_packet_tcb_queue, &video_mv_packet_tcb_queue->curr_consumer);
            } else {
                isil_video_mv_packet_tcb_queue_get(video_mv_packet_tcb_queue, &video_mv_packet_tcb_queue->curr_consumer);
            }
            spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        }
        spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_packet_tcb_queue_try_get_curr_consumer_from_queue(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue)
{
    if(video_mv_packet_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        if(video_mv_packet_tcb_queue->curr_consumer == NULL){
            if(video_mv_packet_tcb_queue->op != NULL){
                video_mv_packet_tcb_queue->op->try_get(video_mv_packet_tcb_queue, &video_mv_packet_tcb_queue->curr_consumer);
            } else {
                isil_video_mv_packet_tcb_queue_try_get(video_mv_packet_tcb_queue, &video_mv_packet_tcb_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_packet_tcb_queue_release_curr_consumer(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_packet_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_packet_tcb_queue->lock, flags);
        if(video_mv_packet_tcb_queue->curr_consumer != NULL){
            if(video_mv_packet_tcb_queue->curr_consumer->op != NULL){
                video_mv_packet_tcb_queue->curr_consumer->op->release(&video_mv_packet_tcb_queue->curr_consumer, video_chan_buf_pool);
            } else {
                isil_video_mv_packet_tcb_release(&video_mv_packet_tcb_queue->curr_consumer, video_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&video_mv_packet_tcb_queue->lock, flags);
    }
}

static int	isil_video_mv_packet_tcb_queue_get_curr_queue_entry_number(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue)
{
    int	entry_number = 0;
    if(video_mv_packet_tcb_queue!=NULL){
        tcb_node_queue_t	*queue;
        queue = &video_mv_packet_tcb_queue->queue_node;
        if(queue->op != NULL){
            entry_number = queue->op->get_queue_curr_entry_number(queue);
        }
    }
    return entry_number;
}

static void	isil_video_mv_packet_tcb_queue_release(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_packet_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        remove_isil_video_mv_packet_tcb_queue(video_mv_packet_tcb_queue, video_chan_buf_pool);
    }
}

static void	isil_video_mv_packet_tcb_queue_init(isil_video_mv_packet_tcb_queue_t *video_mv_packet_tcb_queue)
{
    if(video_mv_packet_tcb_queue != NULL){
        tcb_node_queue_t	*queue;
        queue = &video_mv_packet_tcb_queue->queue_node;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);
        video_mv_packet_tcb_queue->curr_consumer = NULL;
        video_mv_packet_tcb_queue->curr_producer = NULL;
        video_mv_packet_tcb_queue->start_timestamp = 0;
        video_mv_packet_tcb_queue->total_duration = 0;
        spin_lock_init(&video_mv_packet_tcb_queue->lock);
    }
}

static struct isil_video_mv_packet_tcb_queue_operation	isil_video_mv_packet_tcb_queue_op = {
    .get = isil_video_mv_packet_tcb_queue_get,
    .try_get = isil_video_mv_packet_tcb_queue_try_get,
    .get_tailer = isil_video_mv_packet_tcb_queue_get_tailer,
    .try_get_tailer = isil_video_mv_packet_tcb_queue_try_get_tailer,
    .put = isil_video_mv_packet_tcb_queue_put,
    .put_header = isil_video_mv_packet_tcb_queue_put_header,

    .get_curr_producer_from_pool = isil_video_mv_packet_tcb_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool = isil_video_mv_packet_tcb_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue = isil_video_mv_packet_tcb_queue_put_curr_producer_into_queue,
    .release_curr_producer = isil_video_mv_packet_tcb_queue_release_curr_producer,
    .get_curr_consumer_from_queue = isil_video_mv_packet_tcb_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue = isil_video_mv_packet_tcb_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = isil_video_mv_packet_tcb_queue_release_curr_consumer,

    .get_curr_queue_entry_number = isil_video_mv_packet_tcb_queue_get_curr_queue_entry_number,
    .release = isil_video_mv_packet_tcb_queue_release,
    .init = isil_video_mv_packet_tcb_queue_init,
};

void	init_isil_video_mv_packet_tcb_queue(isil_video_mv_packet_tcb_queue_t *video_mv_packet_queue)
{
    if(video_mv_packet_queue != NULL){
        video_mv_packet_queue->op = &isil_video_mv_packet_tcb_queue_op;
        video_mv_packet_queue->op->init(video_mv_packet_queue);
    }
}

void	remove_isil_video_mv_packet_tcb_queue(isil_video_mv_packet_tcb_queue_t *video_mv_packet_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_packet_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        if(video_mv_packet_queue->op != NULL){
            video_mv_packet_queue->op->put_curr_producer_into_queue(video_mv_packet_queue);
            video_mv_packet_queue->op->release_curr_consumer(video_mv_packet_queue, video_chan_buf_pool);
            while(video_mv_packet_queue->op->get_curr_queue_entry_number(video_mv_packet_queue)){
                video_mv_packet_queue->op->get_curr_consumer_from_queue(video_mv_packet_queue);
                if(video_mv_packet_queue->curr_consumer == NULL){
                    break;
                }
                video_mv_packet_queue->op->release_curr_consumer(video_mv_packet_queue, video_chan_buf_pool);
            }
        } else {
            isil_video_mv_packet_tcb_queue_put_curr_producer_into_queue(video_mv_packet_queue);
            isil_video_mv_packet_tcb_queue_release_curr_consumer(video_mv_packet_queue, video_chan_buf_pool);
            while(isil_video_mv_packet_tcb_queue_get_curr_queue_entry_number(video_mv_packet_queue)){
                isil_video_mv_packet_tcb_queue_get_curr_consumer_from_queue(video_mv_packet_queue);
                if(video_mv_packet_queue->curr_consumer == NULL){
                    break;
                }
                isil_video_mv_packet_tcb_queue_release_curr_consumer(video_mv_packet_queue, video_chan_buf_pool);
            }
        }
    }
}

static void	isil_video_mv_frame_tcb_init(isil_video_mv_frame_tcb_t *frame)
{
    if(frame!=NULL){
        tcb_node_t	*node = &frame->mv_frame_node;
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, frame);

        spin_lock_init(&frame->lock);
        atomic_set(&frame->ref_count, 0);
        memset(frame->mvFlag_buf, 0, MotionVectorBitFlag_BUF_LEN);
        frame->mvFlag_len = 0;
        frame->mvBuf_payLoadLen = 0;
        frame->timestamp = 0;
        frame->frame_number = 0;
        init_isil_video_mv_packet_tcb_queue(&frame->mv_queue);
    }
}

static void	isil_video_mv_frame_tcb_reset(isil_video_mv_frame_tcb_t *frame)
{
    if(frame != NULL){
        atomic_set(&frame->ref_count, 0);
        memset(frame->mvFlag_buf, 0, MotionVectorBitFlag_BUF_LEN);
        frame->mvFlag_len = 0;
        frame->mvBuf_payLoadLen = 0;
        frame->timestamp = 0;
        frame->frame_number = 0;
    }
}

static void	isil_video_mv_frame_tcb_release(isil_video_mv_frame_tcb_t **ptr_frame, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((ptr_frame!=NULL)){
        isil_video_mv_frame_tcb_t	*frame = *ptr_frame;
        unsigned long	flags;
        if(video_chan_buf_pool == NULL){
            video_chan_buf_pool = frame->video_chan_buf_pool;
        }
        spin_lock_irqsave(&frame->lock, flags);
        if(atomic_read(&frame->ref_count) > 1){
            atomic_dec(&frame->ref_count);
        } else {
            tcb_node_t	*node;
            if(frame->op != NULL){
                frame->op->reset(frame);
            }
            if(frame->mv_queue.op != NULL){
                frame->mv_queue.op->release(&frame->mv_queue, video_chan_buf_pool);
            } else {
                isil_video_mv_packet_tcb_queue_release(&frame->mv_queue, video_chan_buf_pool);
            }
            node = &frame->mv_frame_node;
            if(node->op != NULL){
                node->op->release(node, &video_chan_buf_pool->mv_frame_pool_tcb);
            }
            *ptr_frame = NULL;
        }
        spin_unlock_irqrestore(&frame->lock, flags);
    }
}

static void	isil_video_mv_frame_tcb_reference(isil_video_mv_frame_tcb_t *src_frame, isil_video_mv_frame_tcb_t **ptr_dest_frame)
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
static size_t  isil_video_mv_frame_tcb_submit(isil_video_mv_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    return 0;
}
#else

static size_t  isil_video_mv_frame_tcb_submit(isil_video_mv_frame_tcb_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
        isil_frame_msg_t  fm_mvf = {0}, fm_mvd = {0}, *pfm;
        isil_video_mv_packet_tcb_queue_t *mvpq;
        h264_nal_t  *nal;
        size_t  buf_size, packet_size, l;
        loff_t  buf_offset, packet_offset;
        char *pc;
        unsigned int expect_len;

        buf_size = count;
        buf_offset = 0;

        if(frame){
                pfm = &fm_mvf;
                pc = (char *)data;
                buf_offset += sizeof(isil_frame_msg_t);
                buf_size -= sizeof(isil_frame_msg_t);
                pfm->msg_type = ISIL_H264_MVFLAG_FRAME_MSG;
                pfm->msg_len  = sizeof(isil_frame_msg_t);
                pfm->frame_serial = frame->frame_number;
                pfm->frame_timestamp = frame->timestamp;
                pfm->frame_checksam = 0;

                ISIL_DBG(ISIL_DBG_ERR,"frame_number[%d]\n",frame->frame_number);

                expect_len = tc_align_up(frame->mvBuf_payLoadLen + frame->mvFlag_len + sizeof(isil_frame_msg_t), 4);
                if(expect_len > buf_size)
                        return -ENOMEM;
                copy_to_user(&data[buf_offset], frame->mvFlag_buf,frame->mvFlag_len);
                pfm->msg_len += frame->mvFlag_len;
                buf_offset += frame->mvFlag_len;
                buf_size -= frame->mvFlag_len;


                if(copy_to_user(pc, &pfm, sizeof(isil_frame_msg_t)) != 0){
                        printk("%s, %d:isil_frame_msg_t(mvflag) copy_to_user failed\n", __FUNCTION__, __LINE__);
                        return 0;
                }

                pfm = &fm_mvd;
                pc = (char *)&data[buf_offset];
                buf_offset += sizeof(isil_frame_msg_t);
                buf_size -= sizeof(isil_frame_msg_t);
                pfm->msg_type = ISIL_H264_MVD_FRAME_MSG;
                pfm->msg_len  = sizeof(isil_frame_msg_t);
                pfm->frame_serial = frame->frame_number;
                pfm->frame_timestamp = frame->timestamp;
                pfm->frame_checksam = 0;                

                mvpq = &frame->mv_queue;
                while(mvpq->op->get_curr_queue_entry_number(mvpq)){
                        mvpq->op->get_curr_consumer_from_queue(mvpq);
                        pfm->frame_checksam = compute_accumulate_crc(pfm->frame_checksam, mvpq->curr_consumer->mvVector_buf, mvpq->curr_consumer->mvVector_len);
                        packet_size = mvpq->curr_consumer->op->submit(mvpq->curr_consumer, &data[buf_offset], buf_size, &packet_offset);
                        mvpq->op->release_curr_consumer(mvpq, video_chan_buf_pool);
                        buf_size -= packet_size;
                        buf_offset += packet_size;
                        pfm->msg_len += packet_size;

                }
                while(buf_offset & 3){
                        __put_user(0, &data[buf_offset]);
                        buf_size--;
                        buf_offset++;
                        pfm->msg_len++;
                }

                if(copy_to_user(pc, pfm, sizeof(isil_frame_msg_t)) != 0){
                        printk("%s, %d:isil_frame_msg_t(mvd) copy_to_user failed\n", __FUNCTION__, __LINE__);
                        return 0;
                }
        }else{
                printk("frame is NULL in %s\n",__FUNCTION__);
        }

        *ppos += buf_offset;
        return (size_t)buf_offset;
}
#endif

static struct isil_video_mv_frame_tcb_operation	isil_video_mv_frame_tcb_op = {
    .init = isil_video_mv_frame_tcb_init,
    .reset = isil_video_mv_frame_tcb_reset,
    .release = isil_video_mv_frame_tcb_release,
    .reference = isil_video_mv_frame_tcb_reference,
    .submit = isil_video_mv_frame_tcb_submit,
};

static void	isil_video_mv_frame_tcb_queue_get(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_mv_frame_tcb_t **ptr_video_mv_frame_tcb)
{
    if((video_mv_frame_tcb_queue!=NULL) && (ptr_video_mv_frame_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_mv_frame_tcb = NULL;
        queue = &video_mv_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_mv_frame_tcb = to_get_isil_video_mv_frame_tcb(temp_node);
            }
        }
    }
}

static void	isil_video_mv_frame_tcb_queue_try_get(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_mv_frame_tcb_t **ptr_video_mv_frame_tcb)
{
    if((video_mv_frame_tcb_queue!=NULL) && (ptr_video_mv_frame_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_mv_frame_tcb = NULL;
        queue = &video_mv_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_mv_frame_tcb = to_get_isil_video_mv_frame_tcb(temp_node);
            }
        }
    }
}

static void	isil_video_mv_frame_tcb_queue_get_tailer(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_mv_frame_tcb_t **ptr_video_mv_frame_tcb)
{
    if((video_mv_frame_tcb_queue!=NULL) && (ptr_video_mv_frame_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_mv_frame_tcb = NULL;
        queue = &video_mv_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_mv_frame_tcb = to_get_isil_video_mv_frame_tcb(temp_node);
            }
        }
    }
}

static void	isil_video_mv_frame_tcb_queue_try_get_tailer(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_mv_frame_tcb_t **ptr_video_mv_frame_tcb)
{
    if((video_mv_frame_tcb_queue!=NULL) && (ptr_video_mv_frame_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_video_mv_frame_tcb = NULL;
        queue = &video_mv_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_video_mv_frame_tcb = to_get_isil_video_mv_frame_tcb(temp_node);
            }
        }
    }
}

static void	isil_video_mv_frame_tcb_queue_put(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_mv_frame_tcb_t *video_mv_frame_tcb)
{
    if((video_mv_frame_tcb_queue!=NULL) && (video_mv_frame_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &video_mv_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put(queue, &video_mv_frame_tcb->mv_frame_node);
        }
    }
}

static void	isil_video_mv_frame_tcb_queue_put_header(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_mv_frame_tcb_t *video_mv_frame_tcb)
{
    if((video_mv_frame_tcb_queue!=NULL) && (video_mv_frame_tcb!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &video_mv_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put_header(queue, &video_mv_frame_tcb->mv_frame_node);
        }
    }
}

static void	isil_video_mv_frame_tcb_queue_get_curr_producer_from_pool(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        if(video_mv_frame_tcb_queue->curr_producer == NULL){
            spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
            if(video_chan_buf_pool->op != NULL){
                video_chan_buf_pool->op->get_video_mv_frame_tcb(video_chan_buf_pool, &video_mv_frame_tcb_queue->curr_producer);
            }
            spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        }
        spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_frame_tcb_queue_try_get_curr_producer_from_pool(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        if(video_mv_frame_tcb_queue->curr_producer == NULL){
            if(video_chan_buf_pool->op != NULL){
                video_chan_buf_pool->op->try_get_video_mv_frame_tcb(video_chan_buf_pool, &video_mv_frame_tcb_queue->curr_producer);
            }
        }
        spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_frame_tcb_queue_put_curr_producer_into_queue(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue)
{
    if(video_mv_frame_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        if(video_mv_frame_tcb_queue->curr_producer != NULL){
            if(video_mv_frame_tcb_queue->op != NULL){
                video_mv_frame_tcb_queue->op->put(video_mv_frame_tcb_queue, video_mv_frame_tcb_queue->curr_producer);
            } else {
                isil_video_mv_frame_tcb_queue_put(video_mv_frame_tcb_queue, video_mv_frame_tcb_queue->curr_producer);
            }
            video_mv_frame_tcb_queue->curr_producer = NULL;
        }
        spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_frame_tcb_queue_release_curr_producer(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        if(video_mv_frame_tcb_queue->curr_producer != NULL){
            if(video_mv_frame_tcb_queue->curr_producer->op != NULL){
                video_mv_frame_tcb_queue->curr_producer->op->release(&video_mv_frame_tcb_queue->curr_producer, video_chan_buf_pool);
            } else {
                isil_video_mv_frame_tcb_release(&video_mv_frame_tcb_queue->curr_producer, video_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_frame_tcb_queue_get_curr_consumer_from_queue(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue)
{
    if(video_mv_frame_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        if(video_mv_frame_tcb_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
            if(video_mv_frame_tcb_queue->op != NULL){
                video_mv_frame_tcb_queue->op->get(video_mv_frame_tcb_queue, &video_mv_frame_tcb_queue->curr_consumer);
            } else {
                isil_video_mv_frame_tcb_queue_get(video_mv_frame_tcb_queue, &video_mv_frame_tcb_queue->curr_consumer);
            }
            spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        }
        spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_frame_tcb_queue_try_get_curr_consumer_from_queue(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue)
{
    if(video_mv_frame_tcb_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        if(video_mv_frame_tcb_queue->curr_consumer == NULL){
            if(video_mv_frame_tcb_queue->op != NULL){
                video_mv_frame_tcb_queue->op->try_get(video_mv_frame_tcb_queue, &video_mv_frame_tcb_queue->curr_consumer);
            } else {
                isil_video_mv_frame_tcb_queue_try_get(video_mv_frame_tcb_queue, &video_mv_frame_tcb_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
    }
}

static void	isil_video_mv_frame_tcb_queue_release_curr_consumer(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&video_mv_frame_tcb_queue->lock, flags);
        if(video_mv_frame_tcb_queue->curr_consumer != NULL){
            if(video_mv_frame_tcb_queue->curr_consumer->op != NULL){
                video_mv_frame_tcb_queue->curr_consumer->op->release(&video_mv_frame_tcb_queue->curr_consumer, video_chan_buf_pool);
            } else {
                isil_video_mv_frame_tcb_release(&video_mv_frame_tcb_queue->curr_consumer, video_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&video_mv_frame_tcb_queue->lock, flags);
    }
}

static int	isil_video_mv_frame_tcb_queue_get_curr_queue_entry_number(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue)
{
    int	entry_number = 0;
    if(video_mv_frame_tcb_queue!=NULL){
        tcb_node_queue_t	*queue;
        queue = &video_mv_frame_tcb_queue->queue_node;
        if(queue->op != NULL){
            entry_number = queue->op->get_queue_curr_entry_number(queue);
        }
    }
    return entry_number;
}

static void	isil_video_mv_frame_tcb_queue_release(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        remove_isil_video_mv_frame_tcb_queue(video_mv_frame_tcb_queue, video_chan_buf_pool);
    }
}

static void	isil_video_mv_frame_tcb_queue_init(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue)
{
    if(video_mv_frame_tcb_queue != NULL){
        tcb_node_queue_t	*queue;
        queue = &video_mv_frame_tcb_queue->queue_node;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);
        video_mv_frame_tcb_queue->curr_consumer = NULL;
        video_mv_frame_tcb_queue->curr_producer = NULL;
        video_mv_frame_tcb_queue->start_timestamp = 0;
        video_mv_frame_tcb_queue->total_duration = 0;
        spin_lock_init(&video_mv_frame_tcb_queue->lock);
    }
}

static struct isil_video_mv_frame_tcb_queue_operation	isil_video_mv_frame_tcb_queue_op = {
    .get = isil_video_mv_frame_tcb_queue_get,
    .try_get = isil_video_mv_frame_tcb_queue_try_get,
    .get_tailer = isil_video_mv_frame_tcb_queue_get_tailer,
    .try_get_tailer = isil_video_mv_frame_tcb_queue_try_get_tailer,
    .put = isil_video_mv_frame_tcb_queue_put,
    .put_header = isil_video_mv_frame_tcb_queue_put_header,

    .get_curr_producer_from_pool = isil_video_mv_frame_tcb_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool = isil_video_mv_frame_tcb_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue = isil_video_mv_frame_tcb_queue_put_curr_producer_into_queue,
    .release_curr_producer = isil_video_mv_frame_tcb_queue_release_curr_producer,
    .get_curr_consumer_from_queue = isil_video_mv_frame_tcb_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue = isil_video_mv_frame_tcb_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = isil_video_mv_frame_tcb_queue_release_curr_consumer,

    .get_curr_queue_entry_number = isil_video_mv_frame_tcb_queue_get_curr_queue_entry_number,
    .release = isil_video_mv_frame_tcb_queue_release,
    .init = isil_video_mv_frame_tcb_queue_init,
};

void	init_isil_video_mv_frame_tcb_queue(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue)
{
    if(video_mv_frame_tcb_queue != NULL){
        video_mv_frame_tcb_queue->op = &isil_video_mv_frame_tcb_queue_op;
        video_mv_frame_tcb_queue->op->init(video_mv_frame_tcb_queue);
    }
}

void	remove_isil_video_mv_frame_tcb_queue(isil_video_mv_frame_tcb_queue_t *video_mv_frame_tcb_queue, isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if((video_mv_frame_tcb_queue!=NULL) && (video_chan_buf_pool!=NULL)){
        if(video_mv_frame_tcb_queue->op != NULL){
            video_mv_frame_tcb_queue->op->put_curr_producer_into_queue(video_mv_frame_tcb_queue);
            video_mv_frame_tcb_queue->op->release_curr_consumer(video_mv_frame_tcb_queue, video_chan_buf_pool);
            while(video_mv_frame_tcb_queue->op->get_curr_queue_entry_number(video_mv_frame_tcb_queue)){
                video_mv_frame_tcb_queue->op->get_curr_consumer_from_queue(video_mv_frame_tcb_queue);
                if(video_mv_frame_tcb_queue->curr_consumer == NULL){
                    break;
                }
                video_mv_frame_tcb_queue->op->release_curr_consumer(video_mv_frame_tcb_queue, video_chan_buf_pool);
            }
        } else {
            isil_video_mv_frame_tcb_queue_put_curr_producer_into_queue(video_mv_frame_tcb_queue);
            isil_video_mv_frame_tcb_queue_release_curr_consumer(video_mv_frame_tcb_queue, video_chan_buf_pool);
            while(isil_video_mv_frame_tcb_queue_get_curr_queue_entry_number(video_mv_frame_tcb_queue)){
                isil_video_mv_frame_tcb_queue_get_curr_consumer_from_queue(video_mv_frame_tcb_queue);
                if(video_mv_frame_tcb_queue->curr_consumer == NULL){
                    break;
                }
                isil_video_mv_frame_tcb_queue_release_curr_consumer(video_mv_frame_tcb_queue, video_chan_buf_pool);
            }
        }
    }
}
#endif

static int	isil_video_chan_buf_pool_create(isil_video_chan_buf_pool_t *video_chan_buf_pool, unsigned long buf_len)
{
    int	ret = ISIL_ERR;

    if(video_chan_buf_pool != NULL){
        tcb_node_pool_t	*node_pool;
        isil_video_packet_tcb_t   *ptr_video_packet_tcb;
        isil_video_frame_tcb_t    *ptr_video_frame_tcb;
#ifdef  MV_MODULE
        isil_video_mv_packet_tcb_t    *ptr_mv_packet_tcb;
        isil_video_mv_frame_tcb_t     *ptr_mv_frame_tcb;
#endif
        int i;

        video_chan_buf_pool->cache_order = get_order(buf_len);
#ifdef  USE_DISCRETE_BUF
        video_chan_buf_pool->cache_buffer = NULL;
#else
        video_chan_buf_pool->cache_buffer = (u8*)__get_free_pages(GFP_KERNEL, video_chan_buf_pool->cache_order);
        if(video_chan_buf_pool->cache_buffer == NULL) {
            video_chan_buf_pool->cache_order = 0;
            printk("can't alloc %d pages for video buf pool\n", video_chan_buf_pool->cache_order);
            return -ENOMEM;
        }
#endif

        video_chan_buf_pool->video_packet_entry_number = 1;
        for(i=0; i<video_chan_buf_pool->cache_order; i++){
            video_chan_buf_pool->video_packet_entry_number <<= 1;
        }
        video_chan_buf_pool->video_packet_buffer_size = PAGE_SIZE;
        node_pool = &video_chan_buf_pool->video_packet_pool_tcb;
        node_pool->op = &tcb_node_pool_op;
        video_chan_buf_pool->video_packet_entry_pool = (isil_video_packet_tcb_t*)kmalloc(sizeof(isil_video_packet_tcb_t)*video_chan_buf_pool->video_packet_entry_number, GFP_KERNEL);
        if(video_chan_buf_pool->video_packet_entry_pool == NULL){
            if(video_chan_buf_pool->cache_buffer != NULL) {
                free_pages((unsigned long)video_chan_buf_pool->cache_buffer, video_chan_buf_pool->cache_order);
            }
            video_chan_buf_pool->cache_buffer = NULL;
            video_chan_buf_pool->cache_order = 0;
            printk("can't alloc video packet tcb\n");
            return -ENOMEM;
        }
        node_pool->op->init(node_pool, video_chan_buf_pool->video_packet_entry_number);
        for(i=0; i<video_chan_buf_pool->video_packet_entry_number; i++){
            ptr_video_packet_tcb = &video_chan_buf_pool->video_packet_entry_pool[i];
            ptr_video_packet_tcb->op = &isil_video_packet_tcb_op;
            ptr_video_packet_tcb->op->init(ptr_video_packet_tcb, video_chan_buf_pool, i);
            ptr_video_packet_tcb->op->release(&ptr_video_packet_tcb, video_chan_buf_pool);
        }

        video_chan_buf_pool->video_frame_entry_number = video_chan_buf_pool->video_packet_entry_number;
        node_pool = &video_chan_buf_pool->video_frame_pool_tcb;
        node_pool->op = &tcb_node_pool_op;
        video_chan_buf_pool->video_frame_entry_pool = (isil_video_frame_tcb_t*)kmalloc(sizeof(isil_video_frame_tcb_t)*video_chan_buf_pool->video_frame_entry_number, GFP_KERNEL);
        if(video_chan_buf_pool->video_frame_entry_pool == NULL){
            node_pool = &video_chan_buf_pool->video_packet_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(video_chan_buf_pool->video_packet_entry_pool);
            video_chan_buf_pool->video_packet_entry_pool = NULL;
            video_chan_buf_pool->video_packet_entry_number = 0;
            if(video_chan_buf_pool->cache_buffer != NULL){
                free_pages((unsigned long)video_chan_buf_pool->cache_buffer, video_chan_buf_pool->cache_order);
            }
            video_chan_buf_pool->cache_buffer = NULL;
            video_chan_buf_pool->cache_order = 0;
            printk("can't alloc video frame tcb\n");
            return -ENOMEM;
        }
        node_pool->op->init(node_pool, video_chan_buf_pool->video_frame_entry_number);
        for(i=0; i<video_chan_buf_pool->video_frame_entry_number; i++){
            ptr_video_frame_tcb = &video_chan_buf_pool->video_frame_entry_pool[i];
            ptr_video_frame_tcb->op = &isil_video_frame_tcb_op;
            ptr_video_frame_tcb->video_chan_buf_pool = video_chan_buf_pool;
            ptr_video_frame_tcb->op->init(ptr_video_frame_tcb);
            ptr_video_frame_tcb->op->release(&ptr_video_frame_tcb, video_chan_buf_pool);
        }

#ifdef  MV_MODULE
        video_chan_buf_pool->mv_cache_order = video_chan_buf_pool->cache_order;
#ifdef  USE_DISCRETE_BUF
        video_chan_buf_pool->mv_cache_buffer = NULL;
#else
        video_chan_buf_pool->mv_cache_buffer = (u8*)__get_free_pages(GFP_KERNEL, video_chan_buf_pool->mv_cache_order);
        if(video_chan_buf_pool->mv_cache_buffer == NULL){
            node_pool = &video_chan_buf_pool->video_frame_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(video_chan_buf_pool->video_frame_entry_pool);
            video_chan_buf_pool->video_frame_entry_pool = NULL;
            video_chan_buf_pool->video_frame_entry_number = 0;
            node_pool = &video_chan_buf_pool->video_packet_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(video_chan_buf_pool->video_packet_entry_pool);
            video_chan_buf_pool->video_packet_entry_pool = NULL;
            video_chan_buf_pool->video_packet_entry_number = 0;
            if(video_chan_buf_pool->cache_buffer != NULL) {
                free_pages((unsigned long)video_chan_buf_pool->cache_buffer, video_chan_buf_pool->cache_order);
            }
            video_chan_buf_pool->cache_buffer = NULL;
            video_chan_buf_pool->cache_order = 0;
            printk("can't alloc page for mv\n");
            return -ENOMEM;
        }
#endif

        video_chan_buf_pool->mv_packet_entry_number = 1;
        for(i=0; i<video_chan_buf_pool->mv_cache_order; i++){
            video_chan_buf_pool->mv_packet_entry_number <<= 1;
        }
        video_chan_buf_pool->mv_packet_buffer_size = PAGE_SIZE;
        node_pool = &video_chan_buf_pool->mv_packet_pool_tcb;
        node_pool->op = &tcb_node_pool_op;
        video_chan_buf_pool->mv_packet_entry_pool = (isil_video_mv_packet_tcb_t*)kmalloc(sizeof(isil_video_mv_packet_tcb_t)*video_chan_buf_pool->mv_packet_entry_number, GFP_KERNEL);
        if(video_chan_buf_pool->mv_packet_entry_pool == NULL){
            node_pool = &video_chan_buf_pool->video_frame_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(video_chan_buf_pool->video_frame_entry_pool);
            video_chan_buf_pool->video_frame_entry_pool = NULL;
            video_chan_buf_pool->video_frame_entry_number = 0;
            node_pool = &video_chan_buf_pool->video_packet_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(video_chan_buf_pool->video_packet_entry_pool);
            video_chan_buf_pool->video_packet_entry_pool = NULL;
            video_chan_buf_pool->video_packet_entry_number = 0;
            if(video_chan_buf_pool->cache_buffer != NULL) {
                free_pages((unsigned long)video_chan_buf_pool->cache_buffer, video_chan_buf_pool->cache_order);
            }
            video_chan_buf_pool->cache_buffer = NULL;
            video_chan_buf_pool->cache_order = 0;
            if(video_chan_buf_pool->mv_cache_buffer != NULL) {
                free_pages((unsigned long)video_chan_buf_pool->mv_cache_buffer, video_chan_buf_pool->mv_cache_order);
            }
            video_chan_buf_pool->mv_cache_buffer = NULL;
            video_chan_buf_pool->mv_cache_order = 0;
            printk("can't alloc mv packet tcb\n");
            return -ENOMEM;
        }
        node_pool->op->init(node_pool, video_chan_buf_pool->mv_packet_entry_number);
        for(i=0; i<video_chan_buf_pool->mv_packet_entry_number; i++){
            ptr_mv_packet_tcb = &video_chan_buf_pool->mv_packet_entry_pool[i];
            ptr_mv_packet_tcb->op = &isil_video_mv_packet_tcb_op;
            ptr_mv_packet_tcb->op->init(ptr_mv_packet_tcb, video_chan_buf_pool, i);
            ptr_mv_packet_tcb->op->release(&ptr_mv_packet_tcb, video_chan_buf_pool);
        }

        video_chan_buf_pool->mv_frame_entry_number = video_chan_buf_pool->mv_packet_entry_number;
        node_pool = &video_chan_buf_pool->mv_frame_pool_tcb;
        node_pool->op = &tcb_node_pool_op;
        video_chan_buf_pool->mv_frame_entry_pool = (isil_video_mv_frame_tcb_t*)kmalloc(sizeof(isil_video_mv_frame_tcb_t)*video_chan_buf_pool->mv_frame_entry_number, GFP_KERNEL);
        if(video_chan_buf_pool->mv_frame_entry_pool == NULL){
            node_pool = &video_chan_buf_pool->video_frame_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(video_chan_buf_pool->video_frame_entry_pool);
            video_chan_buf_pool->video_frame_entry_pool = NULL;
            video_chan_buf_pool->video_frame_entry_number = 0;
            node_pool = &video_chan_buf_pool->video_packet_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(video_chan_buf_pool->video_packet_entry_pool);
            video_chan_buf_pool->video_packet_entry_pool = NULL;
            video_chan_buf_pool->video_packet_entry_number = 0;
            if(video_chan_buf_pool->cache_buffer != NULL) {
                free_pages((unsigned long)video_chan_buf_pool->cache_buffer, video_chan_buf_pool->cache_order);
            }
            video_chan_buf_pool->cache_buffer = NULL;
            video_chan_buf_pool->cache_order = 0;
            node_pool = &video_chan_buf_pool->mv_packet_pool_tcb;
            node_pool->op->release(node_pool);
            kfree(video_chan_buf_pool->mv_packet_entry_pool);
            video_chan_buf_pool->mv_packet_entry_pool = NULL;
            video_chan_buf_pool->mv_packet_entry_number = 0;
            if(video_chan_buf_pool->mv_cache_buffer != NULL) {
                free_pages((unsigned long)video_chan_buf_pool->mv_cache_buffer, video_chan_buf_pool->mv_cache_order);
            }
            video_chan_buf_pool->mv_cache_buffer = NULL;
            video_chan_buf_pool->mv_cache_order = 0;
            printk("can't alloc mv frame tcb\n");
            return -ENOMEM;
        }
        node_pool->op->init(node_pool, video_chan_buf_pool->mv_frame_entry_number);
        for(i=0; i<video_chan_buf_pool->mv_frame_entry_number; i++){
            ptr_mv_frame_tcb = &video_chan_buf_pool->mv_frame_entry_pool[i];
            ptr_mv_frame_tcb->op = &isil_video_mv_frame_tcb_op;
            ptr_mv_frame_tcb->video_chan_buf_pool = video_chan_buf_pool;
            ptr_mv_frame_tcb->op->init(ptr_mv_frame_tcb);
            ptr_mv_frame_tcb->op->release(&ptr_mv_frame_tcb, video_chan_buf_pool);
        }
#endif
        ret = ISIL_OK;
    }
    return ret;
}

static void	isil_video_chan_buf_pool_release(isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if(video_chan_buf_pool != NULL){
        tcb_node_pool_t	*node_pool;

        node_pool = &video_chan_buf_pool->video_frame_pool_tcb;
        if(node_pool->op != NULL){
            node_pool->op->release(node_pool);
        }
        if(video_chan_buf_pool->video_frame_entry_pool != NULL){
            kfree(video_chan_buf_pool->video_frame_entry_pool);
            video_chan_buf_pool->video_frame_entry_pool = NULL;
        }
        video_chan_buf_pool->video_frame_entry_number = 0;

        node_pool = &video_chan_buf_pool->video_packet_pool_tcb;
        if(node_pool->op != NULL){
            node_pool->op->release(node_pool);
        }
        if(video_chan_buf_pool->video_packet_entry_pool != NULL){
#ifdef  USE_DISCRETE_BUF
            isil_video_packet_tcb_t   *ptr_video_packet_tcb;
            int i;
            for(i=0; i<video_chan_buf_pool->video_packet_entry_number; i++) {
                ptr_video_packet_tcb = &video_chan_buf_pool->video_packet_entry_pool[i];
                if(ptr_video_packet_tcb->data != NULL) {
                    free_page((unsigned long)ptr_video_packet_tcb->data);
                    ptr_video_packet_tcb->data = NULL;
                }
            }
#endif
            kfree(video_chan_buf_pool->video_packet_entry_pool);
            video_chan_buf_pool->video_packet_entry_pool = NULL;
        }
        video_chan_buf_pool->video_packet_entry_number = 0;

        if(video_chan_buf_pool->cache_buffer != NULL){
            free_pages((unsigned long)video_chan_buf_pool->cache_buffer, video_chan_buf_pool->cache_order);
            video_chan_buf_pool->cache_buffer = NULL;
        }
        video_chan_buf_pool->cache_order = 0;

#ifdef  MV_MODULE
        node_pool = &video_chan_buf_pool->mv_frame_pool_tcb;
        if(node_pool->op != NULL){
            node_pool->op->release(node_pool);
        }
        if(video_chan_buf_pool->mv_frame_entry_pool != NULL){
            kfree(video_chan_buf_pool->mv_frame_entry_pool);
            video_chan_buf_pool->mv_frame_entry_pool = NULL;
        }
        video_chan_buf_pool->mv_frame_entry_number = 0;

        node_pool = &video_chan_buf_pool->mv_packet_pool_tcb;
        if(node_pool->op != NULL){
            node_pool->op->release(node_pool);
        }
        if(video_chan_buf_pool->mv_packet_entry_pool != NULL){
#ifdef  USE_DISCRETE_BUF
            isil_video_mv_packet_tcb_t    *ptr_mv_packet_tcb;
            int i;
            for(i=0; i<video_chan_buf_pool->mv_packet_entry_number; i++) {
                ptr_mv_packet_tcb = &video_chan_buf_pool->mv_packet_entry_pool[i];
                if(ptr_mv_packet_tcb->data != NULL) {
                    free_page((unsigned long)ptr_mv_packet_tcb->data);
                    ptr_mv_packet_tcb->data = NULL;
                }
            }
#endif
            kfree(video_chan_buf_pool->mv_packet_entry_pool);
            video_chan_buf_pool->mv_packet_entry_pool = NULL;
        }
        video_chan_buf_pool->mv_packet_entry_number = 0;

        if(video_chan_buf_pool->mv_cache_buffer != NULL){
            free_pages((unsigned long)video_chan_buf_pool->mv_cache_buffer, video_chan_buf_pool->mv_cache_order);
            video_chan_buf_pool->mv_cache_buffer = NULL;
        }
        video_chan_buf_pool->mv_cache_order = 0;
#endif
    }
}

static void isil_video_chan_buf_pool_get_video_packet_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_packet_tcb_t **packet)
{
    if((video_chan_buf_pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *video_packet_pool_tcb = &video_chan_buf_pool->video_packet_pool_tcb;
        *packet = NULL;
        if(video_packet_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            video_packet_pool_tcb->op->get(video_packet_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *packet = to_video_packet_buf_tcb(temp_node);
                atomic_inc(&((*packet)->ref_count));
            }
        }
    }
}

static void isil_video_chan_buf_pool_try_get_video_packet_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_packet_tcb_t **packet)
{
    if((video_chan_buf_pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *video_packet_pool_tcb = &video_chan_buf_pool->video_packet_pool_tcb;
        *packet = NULL;
        if(video_packet_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            video_packet_pool_tcb->op->try_get(video_packet_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *packet = to_video_packet_buf_tcb(temp_node);
                atomic_inc(&((*packet)->ref_count));
            }
        }
    }
}

static void isil_video_chan_buf_pool_put_video_packet_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_packet_tcb_t *packet)
{
    if((video_chan_buf_pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *video_packet_pool_tcb = &video_chan_buf_pool->video_packet_pool_tcb;
        if(video_packet_pool_tcb->op != NULL){
            video_packet_pool_tcb->op->put(video_packet_pool_tcb, &packet->packet_node);
        }
    }
}

static int	isil_video_chan_buf_pool_get_video_packet_tcb_pool_entry_number(isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    int	entry_number = 0;
    if(video_chan_buf_pool != NULL){
        tcb_node_pool_t *video_packet_pool_tcb = &video_chan_buf_pool->video_packet_pool_tcb;
        if(video_packet_pool_tcb->op != NULL){
            entry_number = video_packet_pool_tcb->op->get_curr_pool_entry_number(video_packet_pool_tcb);
        }
    }
    return entry_number;
}

static void	isil_video_chan_buf_pool_get_video_frame_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_frame_tcb_t **frame)
{
    if((video_chan_buf_pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *video_frame_pool_tcb = &video_chan_buf_pool->video_frame_pool_tcb;
        *frame = NULL;
        if(video_frame_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            video_frame_pool_tcb->op->get(video_frame_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *frame = to_video_frame_tcb(temp_node);
                atomic_inc(&((*frame)->ref_count));
            }
        }
    }
}

static void	isil_video_chan_buf_pool_try_get_video_frame_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_frame_tcb_t **frame)
{
    if((video_chan_buf_pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *video_frame_pool_tcb = &video_chan_buf_pool->video_frame_pool_tcb;
        *frame = NULL;
        if(video_frame_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            video_frame_pool_tcb->op->try_get(video_frame_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *frame = to_video_frame_tcb(temp_node);
                atomic_inc(&((*frame)->ref_count));
            }
        }
    }
}

static void	isil_video_chan_buf_pool_put_video_frame_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_frame_tcb_t *frame)
{
    if((video_chan_buf_pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *video_frame_pool_tcb = &video_chan_buf_pool->video_frame_pool_tcb;
        if(video_frame_pool_tcb->op != NULL){
            video_frame_pool_tcb->op->put(video_frame_pool_tcb, &frame->frame_node);
        }
    }
}

static int	isil_video_chan_buf_pool_get_video_frame_tcb_pool_entry_number(isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    int	entry_number = 0;
    if(video_chan_buf_pool != NULL){
        tcb_node_pool_t *video_frame_pool_tcb = &video_chan_buf_pool->video_frame_pool_tcb;
        if(video_frame_pool_tcb->op != NULL){
            entry_number = video_frame_pool_tcb->op->get_curr_pool_entry_number(video_frame_pool_tcb);
        }
    }
    return entry_number;
}
#ifdef  MV_MODULE
static void	isil_video_chan_buf_pool_get_video_mv_packet_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_packet_tcb_t **packet)
{
    if((video_chan_buf_pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *video_mv_packet_pool_tcb = &video_chan_buf_pool->mv_packet_pool_tcb;
        *packet = NULL;
        if(video_mv_packet_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            video_mv_packet_pool_tcb->op->get(video_mv_packet_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *packet = to_get_isil_video_mv_packet_tcb(temp_node);
                atomic_inc(&((*packet)->ref_count));
            }
        }
    }
}

static void	isil_video_chan_buf_pool_try_get_video_mv_packet_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_packet_tcb_t **packet)
{
    if((video_chan_buf_pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *video_mv_packet_pool_tcb = &video_chan_buf_pool->mv_packet_pool_tcb;
        *packet = NULL;
        if(video_mv_packet_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            video_mv_packet_pool_tcb->op->try_get(video_mv_packet_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *packet = to_get_isil_video_mv_packet_tcb(temp_node);
                atomic_inc(&((*packet)->ref_count));
            }
        }
    }
}

static void	isil_video_chan_buf_pool_put_video_mv_packet_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_packet_tcb_t *packet)
{
    if((video_chan_buf_pool!=NULL) && (packet!=NULL)){
        tcb_node_pool_t *video_mv_packet_pool_tcb = &video_chan_buf_pool->mv_packet_pool_tcb;
        if(video_mv_packet_pool_tcb->op != NULL){
            video_mv_packet_pool_tcb->op->put(video_mv_packet_pool_tcb, &packet->mv_packet_node);
        }
    }
}

static int	isil_video_chan_buf_pool_get_video_mv_packet_tcb_pool_entry_number(isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    int	entry_number = 0;
    if(video_chan_buf_pool != NULL){
        tcb_node_pool_t *video_mv_packet_pool_tcb = &video_chan_buf_pool->mv_packet_pool_tcb;
        if(video_mv_packet_pool_tcb->op != NULL){
            entry_number = video_mv_packet_pool_tcb->op->get_curr_pool_entry_number(video_mv_packet_pool_tcb);
        }
    }
    return entry_number;
}

static void	isil_video_chan_buf_pool_get_video_mv_frame_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_frame_tcb_t **frame)
{
    if((video_chan_buf_pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *video_mv_frame_pool_tcb = &video_chan_buf_pool->mv_frame_pool_tcb;
        *frame = NULL;
        if(video_mv_frame_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            video_mv_frame_pool_tcb->op->get(video_mv_frame_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *frame = to_get_isil_video_mv_frame_tcb(temp_node);
                atomic_inc(&((*frame)->ref_count));
            }
        }
    }
}

static void	isil_video_chan_buf_pool_try_get_video_mv_frame_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_frame_tcb_t **frame)
{
    if((video_chan_buf_pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *video_mv_frame_pool_tcb = &video_chan_buf_pool->mv_frame_pool_tcb;
        *frame = NULL;
        if(video_mv_frame_pool_tcb->op != NULL){
            tcb_node_t	*temp_node;
            video_mv_frame_pool_tcb->op->try_get(video_mv_frame_pool_tcb, &temp_node);
            if(temp_node != NULL){
                *frame = to_get_isil_video_mv_frame_tcb(temp_node);
                atomic_inc(&((*frame)->ref_count));
            }
        }
    }
}

static void	isil_video_chan_buf_pool_put_video_mv_frame_tcb(isil_video_chan_buf_pool_t *video_chan_buf_pool, isil_video_mv_frame_tcb_t *frame)
{
    if((video_chan_buf_pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *video_mv_frame_pool_tcb = &video_chan_buf_pool->mv_frame_pool_tcb;
        if(video_mv_frame_pool_tcb->op != NULL){
            video_mv_frame_pool_tcb->op->put(video_mv_frame_pool_tcb, &frame->mv_frame_node);
        }
    }
}

static int	isil_video_chan_buf_pool_get_video_mv_frame_tcb_pool_entry_number(isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    int	entry_number = 0;
    if(video_chan_buf_pool != NULL){
        tcb_node_pool_t *video_mv_frame_pool_tcb = &video_chan_buf_pool->mv_frame_pool_tcb;
        if(video_mv_frame_pool_tcb->op != NULL){
            entry_number = video_mv_frame_pool_tcb->op->get_curr_pool_entry_number(video_mv_frame_pool_tcb);
        }
    }
    return entry_number;
}
#endif

static struct isil_video_chan_buf_pool_operation	isil_video_chan_buf_pool_op = {
    .create = isil_video_chan_buf_pool_create,
    .release = isil_video_chan_buf_pool_release,

    .get_video_packet_tcb = isil_video_chan_buf_pool_get_video_packet_tcb,
    .try_get_video_packet_tcb = isil_video_chan_buf_pool_try_get_video_packet_tcb,
    .put_video_packet_tcb = isil_video_chan_buf_pool_put_video_packet_tcb,
    .get_video_packet_tcb_pool_entry_number = isil_video_chan_buf_pool_get_video_packet_tcb_pool_entry_number,

    .get_video_frame_tcb = isil_video_chan_buf_pool_get_video_frame_tcb,
    .try_get_video_frame_tcb = isil_video_chan_buf_pool_try_get_video_frame_tcb,
    .put_video_frame_tcb = isil_video_chan_buf_pool_put_video_frame_tcb,
    .get_video_frame_tcb_pool_entry_number = isil_video_chan_buf_pool_get_video_frame_tcb_pool_entry_number,
#ifdef  MV_MODULE
    .get_video_mv_packet_tcb = isil_video_chan_buf_pool_get_video_mv_packet_tcb,
    .try_get_video_mv_packet_tcb = isil_video_chan_buf_pool_try_get_video_mv_packet_tcb,
    .put_video_mv_packet_tcb = isil_video_chan_buf_pool_put_video_mv_packet_tcb,
    .get_video_mv_packet_tcb_pool_entry_number = isil_video_chan_buf_pool_get_video_mv_packet_tcb_pool_entry_number,

    .get_video_mv_frame_tcb = isil_video_chan_buf_pool_get_video_mv_frame_tcb,
    .try_get_video_mv_frame_tcb = isil_video_chan_buf_pool_try_get_video_mv_frame_tcb,
    .put_video_mv_frame_tcb = isil_video_chan_buf_pool_put_video_mv_frame_tcb,
    .get_video_mv_frame_tcb_pool_entry_number = isil_video_chan_buf_pool_get_video_mv_frame_tcb_pool_entry_number,
#endif
};

int	init_isil_video_chan_buf_pool(isil_video_chan_buf_pool_t *video_chan_buf_pool, unsigned long buf_len)
{
    int	ret = ISIL_ERR;
    if(video_chan_buf_pool != NULL){
        video_chan_buf_pool->op = &isil_video_chan_buf_pool_op;
        ret = video_chan_buf_pool->op->create(video_chan_buf_pool, buf_len);
    }
    return ret;
}

void	remove_isil_video_chan_buf_pool(isil_video_chan_buf_pool_t *video_chan_buf_pool)
{
    if(video_chan_buf_pool != NULL){
        if(video_chan_buf_pool->op != NULL){
            video_chan_buf_pool->op->release(video_chan_buf_pool);
        }
    }
}
