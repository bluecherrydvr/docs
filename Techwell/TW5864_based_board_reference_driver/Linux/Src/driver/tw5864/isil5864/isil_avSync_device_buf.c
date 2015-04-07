#include	<isil5864/isil_common.h>

static void	avSync_frame_tcb_init(avSync_frame_tcb_t *frame)
{
    if((frame!=NULL)){
        tcb_node_t  *node = &frame->frame_node;
        spin_lock_init(&frame->lock);
        atomic_set(&frame->ref_count, 0);
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, frame);
        frame->curr_submit_tcb = CURR_SUBMIT_IS_264_MASTER_FRAME;
        frame->h264_master_frame = NULL;
#ifdef  MV_MODULE
        frame->h264_master_mv_frame = NULL;
#endif
        frame->h264_sub_frame = NULL;
#ifdef  MV_MODULE
        frame->h264_sub_mv_frame = NULL;
#endif
#ifdef MJPEG_MODULE
        frame->jpeg_frame = NULL;
#endif
        frame->audio_encode_frame = NULL;
        frame->audio_decode_frame = NULL;
    }
}

static void	avSync_frame_tcb_reset(avSync_frame_tcb_t *frame)
{
    if((frame!=NULL)){
        atomic_set(&frame->ref_count, 0);
        frame->curr_submit_tcb = CURR_SUBMIT_IS_264_MASTER_FRAME;
    }
}

static void	avSync_frame_tcb_release(avSync_frame_tcb_t **ptr_frame, avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if((*ptr_frame!=NULL) && (video_device_buf_pool!=NULL)){
        avSync_frame_tcb_t	*avSync_frame = *ptr_frame;
        unsigned long	flags;

        spin_lock_irqsave(&avSync_frame->lock, flags);
        if(atomic_read(&avSync_frame->ref_count) > 1){
            atomic_dec(&avSync_frame->ref_count);
        } else {
            tcb_node_t	*node;
            if(avSync_frame->op != NULL){
                avSync_frame->op->reset(avSync_frame);
            }
            if(avSync_frame->h264_master_frame != NULL){
                avSync_frame->h264_master_frame->op->release(&avSync_frame->h264_master_frame, NULL);
            }
#ifdef  MV_MODULE
            if(avSync_frame->h264_master_mv_frame != NULL){
                avSync_frame->h264_master_mv_frame->op->release(&avSync_frame->h264_master_mv_frame, NULL);
            }
#endif
            if(avSync_frame->h264_sub_frame != NULL){
                avSync_frame->h264_sub_frame->op->release(&avSync_frame->h264_sub_frame, NULL);
            }
#ifdef  MV_MODULE
            if(avSync_frame->h264_sub_mv_frame != NULL){
                avSync_frame->h264_sub_mv_frame->op->release(&avSync_frame->h264_sub_mv_frame, NULL);
            }
#endif
#ifdef MJPEG_MODULE
            if(avSync_frame->jpeg_frame != NULL){
                avSync_frame->jpeg_frame->op->release(&avSync_frame->jpeg_frame, NULL);
            }
#endif
            if(avSync_frame->audio_encode_frame != NULL){
                avSync_frame->audio_encode_frame->op->release(&avSync_frame->audio_encode_frame, NULL);
            }
            if(avSync_frame->audio_decode_frame != NULL){
                avSync_frame->audio_decode_frame->op->release(&avSync_frame->audio_decode_frame, NULL);
            }
            node = &avSync_frame->frame_node;
            if(node->op != NULL){
                node->op->release(node, &video_device_buf_pool->avSync_frame_pool_node);
            }
            *ptr_frame = NULL;
        }
        spin_unlock_irqrestore(&avSync_frame->lock, flags);
    }
}

static void avSync_frame_tcb_reference(avSync_frame_tcb_t *src_frame, avSync_frame_tcb_t **ptr_dest_frame)
{
    if((src_frame!=NULL) && (ptr_dest_frame!=NULL)){
        unsigned long	flags;

        spin_lock_irqsave(&src_frame->lock, flags);
        atomic_inc(&src_frame->ref_count);
        *ptr_dest_frame = src_frame;
        spin_unlock_irqrestore(&src_frame->lock, flags);
    }
}

static size_t avSync_frame_tcb_submit(avSync_frame_tcb_t *avSync_frame, char __user *data, size_t count, loff_t *ppos, int *frame_end, isil_avSync_dev_t *dev)
{
    ssize_t frame_size = 0;
    if((data!=NULL) && (count>0)){
        isil_video_frame_tcb_t    *h264_master_frame = avSync_frame->h264_master_frame;
#ifdef  MV_MODULE
        isil_video_mv_frame_tcb_t *h264_master_mv_frame = avSync_frame->h264_master_mv_frame;
#endif
        isil_video_frame_tcb_t    *h264_sub_frame = avSync_frame->h264_sub_frame;
#ifdef  MV_MODULE
        isil_video_mv_frame_tcb_t *h264_sub_mv_frame = avSync_frame->h264_sub_mv_frame;
#endif
#ifdef MJPEG_MODULE
        isil_vb_frame_tcb_t         *jpeg_frame = avSync_frame->jpeg_frame;
        isil_vb_pool_t                *jpeg_buf_pool;
#endif
        isil_audio_packet_section_t   *audio_encode_frame = avSync_frame->audio_encode_frame;
        isil_video_chan_buf_pool_t    *video_encode_chan_buf_pool;
        isil_audio_chan_buf_pool_t    *audio_buf_pool = &dev->audio_encode_driver->audio_buf_pool;
        size_t  buf_size;
        loff_t  buf_offset, frame_offset;

        *frame_end = 0;
        buf_size = count;
        buf_offset = 0;
        if(h264_master_frame != NULL){
            video_encode_chan_buf_pool = &dev->h264_master_encode_driver->encode_chan_buf_pool;
            frame_offset = 0;
            h264_master_frame->op->submit(h264_master_frame, &data[buf_offset], buf_size, &frame_offset, ISIL_MASTER_BITSTREAM, video_encode_chan_buf_pool);
            h264_master_frame->op->release(&avSync_frame->h264_master_frame, video_encode_chan_buf_pool);
            buf_size -= frame_offset;
            buf_offset += frame_offset;
        }
#ifdef  MV_MODULE
        if(h264_master_mv_frame != NULL){
            video_encode_chan_buf_pool = &dev->h264_master_encode_driver->encode_chan_buf_pool;
            frame_offset = 0;
            h264_master_mv_frame->op->submit(h264_master_mv_frame, &data[buf_offset], buf_size, &frame_offset, ISIL_MASTER_BITSTREAM, video_encode_chan_buf_pool);
            h264_master_mv_frame->op->release(&avSync_frame->h264_master_mv_frame, video_encode_chan_buf_pool);
            buf_size -= frame_offset;
            buf_offset += frame_offset;
        }
#endif
        if(h264_sub_frame != NULL){
            video_encode_chan_buf_pool = &dev->h264_sub_encode_driver->encode_chan_buf_pool;
            frame_offset = 0;
            h264_sub_frame->op->submit(h264_sub_frame, &data[buf_offset], buf_size, &frame_offset, ISIL_SUB_BITSTREAM, video_encode_chan_buf_pool);
            h264_sub_frame->op->release(&avSync_frame->h264_sub_frame, video_encode_chan_buf_pool);
            buf_size -= frame_offset;
            buf_offset += frame_offset;
        }
#ifdef  MV_MODULE
        if(h264_sub_mv_frame != NULL){
            video_encode_chan_buf_pool = &dev->h264_sub_encode_driver->encode_chan_buf_pool;
            frame_offset = 0;
            h264_sub_mv_frame->op->submit(h264_sub_mv_frame, &data[buf_offset], buf_size, &frame_offset, ISIL_SUB_BITSTREAM, video_encode_chan_buf_pool);
            h264_sub_mv_frame->op->release(&avSync_frame->h264_sub_mv_frame, video_encode_chan_buf_pool);
            buf_size -= frame_offset;
            buf_offset += frame_offset;
        }
#endif
#ifdef MJPEG_MODULE
        if(jpeg_frame != NULL){
            jpeg_buf_pool = &dev->jpeg_encode_driver->pool;
            frame_offset = 0;
            jpeg_frame->op->submit(jpeg_frame, &data[buf_offset], buf_size, &frame_offset, ISIL_MASTER_BITSTREAM, jpeg_buf_pool);
            jpeg_frame->op->release(&avSync_frame->jpeg_frame, jpeg_buf_pool);
            buf_size -= frame_offset;
            buf_offset += frame_offset;
        }
#endif
        if(audio_encode_frame != NULL){
            audio_buf_pool = &dev->audio_encode_driver->audio_buf_pool;
            frame_offset = 0;
            audio_encode_frame->op->submit(audio_encode_frame, &data[buf_offset], buf_size, &frame_offset, ISIL_MASTER_BITSTREAM, audio_buf_pool);

            if(dev->audio_encode_driver->audio_logic_chan_id == ISIL_AUDIO_IN_CHAN0_ID)
            {
                isil_audio_driver_t       *audio_decode_chan_driver;
                isil_audio_packet_queue_t *audio_decode_packet_queue;

                dev->audio_encode_driver->chip_audio->op->get_audio_chan_driver(dev->audio_encode_driver->chip_audio, ISIL_AUDIO_OUT_SPEAKER_ID, &audio_decode_chan_driver);
                audio_decode_packet_queue = &audio_decode_chan_driver->audio_packet_queue;
                audio_decode_packet_queue->op->try_get_curr_producer_from_pool(audio_decode_packet_queue, &audio_decode_chan_driver->audio_buf_pool);
                if(audio_decode_packet_queue->curr_producer != NULL){
                    isil_audio_packet_section_t   *audio_decode_section = audio_decode_packet_queue->curr_producer;
                    audio_decode_section->payload_len = audio_encode_frame->payload_len;
                    audio_decode_section->sample_rate = audio_encode_frame->sample_rate;
                    audio_decode_section->type = audio_encode_frame->type;
                    audio_decode_section->bit_wide = audio_encode_frame->bit_wide;
                    audio_decode_section->timestamp = audio_encode_frame->timestamp;
                    memcpy(audio_decode_section->data, audio_encode_frame->data, audio_decode_section->payload_len);
                    audio_decode_packet_queue->op->put_curr_producer_into_queue(audio_decode_packet_queue);
                }
            }

            audio_encode_frame->op->release(&avSync_frame->audio_encode_frame, audio_buf_pool);
            buf_size -= frame_offset;
            buf_offset += frame_offset;
        }
        *frame_end = 1;
        *ppos = buf_offset;
        frame_size = buf_offset;
    }
    return frame_size;
}

static struct avSync_frame_tcb_operation	avSync_frame_tcb_op = {
    .init = avSync_frame_tcb_init,
    .reset = avSync_frame_tcb_reset,
    .release = avSync_frame_tcb_release,
    .reference = avSync_frame_tcb_reference,

    .submit = avSync_frame_tcb_submit,
};

static void	avSync_frame_queue_get(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_tcb_t **ptr_avSync_frame)
{
    if((avSync_frame_queue!=NULL) && (ptr_avSync_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_avSync_frame = NULL;
        queue = &avSync_frame_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_avSync_frame = to_get_avSync_frame_tcb(temp_node);
            }
        }
    }
}

static void	avSync_frame_queue_try_get(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_tcb_t **ptr_avSync_frame)
{
    if((avSync_frame_queue!=NULL) && (ptr_avSync_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_avSync_frame = NULL;
        queue = &avSync_frame_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_avSync_frame = to_get_avSync_frame_tcb(temp_node);
            }
        }
    }
}

static void	avSync_frame_queue_get_tailer(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_tcb_t **ptr_avSync_frame)
{
    if((avSync_frame_queue!=NULL) && (ptr_avSync_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_avSync_frame = NULL;
        queue = &avSync_frame_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_avSync_frame = to_get_avSync_frame_tcb(temp_node);
            }
        }
    }
}

static void	avSync_frame_queue_try_get_tailer(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_tcb_t **ptr_avSync_frame)
{
    if((avSync_frame_queue!=NULL) && (ptr_avSync_frame!=NULL)){
        tcb_node_queue_t	*queue;
        *ptr_avSync_frame = NULL;
        queue = &avSync_frame_queue->queue_node;
        if(queue->op != NULL){
            tcb_node_t	*temp_node;
            queue->op->try_get_tailer(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_avSync_frame = to_get_avSync_frame_tcb(temp_node);
            }
        }
    }
}

static void	avSync_frame_queue_put(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_tcb_t *avSync_frame)
{
    if((avSync_frame_queue!=NULL) && (avSync_frame!=NULL)){
        tcb_node_queue_t	*queue;
        queue = &avSync_frame_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put(queue, &avSync_frame->frame_node);
        }
    }
}

static void	avSync_frame_queue_put_header(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_tcb_t *avSync_frame)
{
    if((avSync_frame_queue!=NULL) && (avSync_frame!=NULL)){
        tcb_node_queue_t    *queue;
        queue = &avSync_frame_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put_header(queue, &avSync_frame->frame_node);
        }
    }
}

static void	avSync_frame_queue_get_curr_producer_from_pool(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if((avSync_frame_queue!=NULL) && (video_device_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        if(avSync_frame_queue->curr_producer == NULL){
            spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
            if(video_device_buf_pool->op != NULL){
                video_device_buf_pool->op->get_avSync_frame_tcb(video_device_buf_pool, &avSync_frame_queue->curr_producer);
            }
            spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        }
        spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
    }
}

static void	avSync_frame_queue_try_get_curr_producer_from_pool(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if((avSync_frame_queue!=NULL) && (video_device_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        if(avSync_frame_queue->curr_producer == NULL){
            if(video_device_buf_pool->op != NULL){
                video_device_buf_pool->op->try_get_avSync_frame_tcb(video_device_buf_pool, &avSync_frame_queue->curr_producer);
            }
        }
        spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
    }
}

static void avSync_frame_queue_put_curr_producer_into_queue(avSync_frame_queue_t *avSync_frame_queue)
{
    if(avSync_frame_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        if(avSync_frame_queue->curr_producer != NULL){
            if(avSync_frame_queue->op != NULL){
                avSync_frame_queue->op->put(avSync_frame_queue, avSync_frame_queue->curr_producer);
            } else {
                avSync_frame_queue_put(avSync_frame_queue, avSync_frame_queue->curr_producer);
            }
            avSync_frame_queue->curr_producer = NULL;
        }
        spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
    }
}

static void	avSync_frame_queue_release_curr_producer(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if((avSync_frame_queue!=NULL) && (video_device_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        if(avSync_frame_queue->curr_producer != NULL){
            if(avSync_frame_queue->curr_producer->op != NULL){
                avSync_frame_queue->curr_producer->op->release(&avSync_frame_queue->curr_producer, video_device_buf_pool);
            } else {
                avSync_frame_tcb_release(&avSync_frame_queue->curr_producer, video_device_buf_pool);
            }
        }
        spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
    }
}

static void	avSync_frame_queue_get_curr_consumer_from_queue(avSync_frame_queue_t *avSync_frame_queue)
{
    if(avSync_frame_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        if(avSync_frame_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
            if(avSync_frame_queue->op != NULL){
                avSync_frame_queue->op->get(avSync_frame_queue, &avSync_frame_queue->curr_consumer);
            } else {
                avSync_frame_queue_get(avSync_frame_queue, &avSync_frame_queue->curr_consumer);
            }
            spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        }
        spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
    }
}

static void	avSync_frame_queue_try_get_curr_consumer_from_queue(avSync_frame_queue_t *avSync_frame_queue)
{
    if(avSync_frame_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        if(avSync_frame_queue->curr_consumer == NULL){
            if(avSync_frame_queue->op != NULL){
                avSync_frame_queue->op->try_get(avSync_frame_queue, &avSync_frame_queue->curr_consumer);
            } else {
                avSync_frame_queue_try_get(avSync_frame_queue, &avSync_frame_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
    }
}

static void	avSync_frame_queue_release_curr_consumer(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if((avSync_frame_queue!=NULL) && (video_device_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&avSync_frame_queue->lock, flags);
        if(avSync_frame_queue->curr_consumer != NULL){
            if(avSync_frame_queue->curr_consumer->op != NULL){
                avSync_frame_queue->curr_consumer->op->release(&avSync_frame_queue->curr_consumer, video_device_buf_pool);
            } else {
                avSync_frame_tcb_release(&avSync_frame_queue->curr_consumer, video_device_buf_pool);
            }
            avSync_frame_queue->curr_consumer = NULL;
        }
        spin_unlock_irqrestore(&avSync_frame_queue->lock, flags);
    }
}

static int	avSync_frame_queue_get_curr_queue_entry_number(avSync_frame_queue_t *avSync_frame_queue)
{
    int	entry_number = 0;
    if(avSync_frame_queue!=NULL){
        tcb_node_queue_t	*queue;
        queue = &avSync_frame_queue->queue_node;
        if(queue->op != NULL){
            entry_number = queue->op->get_queue_curr_entry_number(queue);
        }
    }
    return entry_number;
}

static void	avSync_frame_queue_release(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if((avSync_frame_queue!=NULL) && (video_device_buf_pool!=NULL)){
        remove_avSync_frame_queue(avSync_frame_queue, video_device_buf_pool);
    }
}

static void	avSync_frame_queue_init(avSync_frame_queue_t *avSync_frame_queue)
{
    if(avSync_frame_queue != NULL){
        tcb_node_queue_t	*queue;
        queue = &avSync_frame_queue->queue_node;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);
        avSync_frame_queue->curr_consumer = NULL;
        avSync_frame_queue->curr_producer = NULL;
        avSync_frame_queue->start_timestamp = 0;
        avSync_frame_queue->total_duration = 0;
        spin_lock_init(&avSync_frame_queue->lock);
    }
}

static struct avSync_frame_queue_operation	avSync_frame_queue_op = {
    .get = avSync_frame_queue_get,
    .try_get = avSync_frame_queue_try_get,
    .get_tailer = avSync_frame_queue_get_tailer,
    .try_get_tailer = avSync_frame_queue_try_get_tailer,
    .put = avSync_frame_queue_put,
    .put_header = avSync_frame_queue_put_header,

    .get_curr_producer_from_pool = avSync_frame_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool = avSync_frame_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue = avSync_frame_queue_put_curr_producer_into_queue,
    .release_curr_producer = avSync_frame_queue_release_curr_producer,
    .get_curr_consumer_from_queue = avSync_frame_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue = avSync_frame_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = avSync_frame_queue_release_curr_consumer,

    .get_curr_queue_entry_number = avSync_frame_queue_get_curr_queue_entry_number,
    .release = avSync_frame_queue_release,
    .init = avSync_frame_queue_init,
};

void	init_avSync_frame_queue(avSync_frame_queue_t *avSync_frame_queue)
{
    if(avSync_frame_queue != NULL){
        avSync_frame_queue->op = &avSync_frame_queue_op;
        avSync_frame_queue->op->init(avSync_frame_queue);
    }
}

void	remove_avSync_frame_queue(avSync_frame_queue_t *avSync_frame_queue, avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if((avSync_frame_queue!=NULL) && (video_device_buf_pool!=NULL)){
        if(avSync_frame_queue->op != NULL){
            avSync_frame_queue->op->put_curr_producer_into_queue(avSync_frame_queue);
            avSync_frame_queue->op->release_curr_consumer(avSync_frame_queue, video_device_buf_pool);
            while(avSync_frame_queue->op->get_curr_queue_entry_number(avSync_frame_queue)){
                avSync_frame_queue->op->get_curr_consumer_from_queue(avSync_frame_queue);
                if(avSync_frame_queue->curr_consumer == NULL){
                    break;
                }
                avSync_frame_queue->op->release_curr_consumer(avSync_frame_queue, video_device_buf_pool);
            }
        } else {
            avSync_frame_queue_put_curr_producer_into_queue(avSync_frame_queue);
            avSync_frame_queue_release_curr_consumer(avSync_frame_queue, video_device_buf_pool);
            while(avSync_frame_queue_get_curr_queue_entry_number(avSync_frame_queue)){
                avSync_frame_queue_get_curr_consumer_from_queue(avSync_frame_queue);
                if(avSync_frame_queue->curr_consumer == NULL){
                    break;
                }
                avSync_frame_queue_release_curr_consumer(avSync_frame_queue, video_device_buf_pool);
            }
        }
        printk("avSync pool entry number: %d\n", video_device_buf_pool->avSync_frame_pool_node.op->get_curr_pool_entry_number(&video_device_buf_pool->avSync_frame_pool_node));
    }
}

static int	avSync_frame_buf_pool_create(avSync_frame_buf_pool_t *video_device_buf_pool, unsigned long buf_size)
{
    int	ret = ISIL_ERR;

    if(video_device_buf_pool != NULL){
        tcb_node_pool_t	*node_pool;
        avSync_frame_tcb_t	*acSync_frame;
        int	i;
        video_device_buf_pool->avSync_frame_entry_number = buf_size/PAGE_SIZE;
        node_pool = &video_device_buf_pool->avSync_frame_pool_node;
        node_pool->op = &tcb_node_pool_op;
        video_device_buf_pool->avSync_frame_pool = (avSync_frame_tcb_t*)kmalloc(sizeof(avSync_frame_tcb_t)*video_device_buf_pool->avSync_frame_entry_number, GFP_KERNEL);
        if(video_device_buf_pool->avSync_frame_pool == NULL){
            printk("%s.%d: can't avSync tcb pool\n", __FUNCTION__, __LINE__);
            return ret;
        }
        node_pool->op->init(node_pool, video_device_buf_pool->avSync_frame_entry_number);
        for(i=0; i<video_device_buf_pool->avSync_frame_entry_number; i++){
            acSync_frame = &video_device_buf_pool->avSync_frame_pool[i];
            acSync_frame->op = &avSync_frame_tcb_op;
            acSync_frame->op->init(acSync_frame);
            acSync_frame->op->release(&acSync_frame, video_device_buf_pool);
        }
        ret = ISIL_OK;
    }
    return ret;
}

static void	avSync_frame_buf_pool_release(avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if(video_device_buf_pool != NULL){
        tcb_node_pool_t	*node_pool;
        node_pool = &video_device_buf_pool->avSync_frame_pool_node;
        node_pool->op->release(node_pool);
        if(video_device_buf_pool->avSync_frame_pool != NULL){
            kfree(video_device_buf_pool->avSync_frame_pool);
            video_device_buf_pool->avSync_frame_pool = 0;
        }
        video_device_buf_pool->avSync_frame_entry_number = 0;
    }
}

static void	avSync_frame_buf_pool_get_avSync_frame_tcb(avSync_frame_buf_pool_t *video_device_buf_pool, avSync_frame_tcb_t **ptr_frame)
{
    if((video_device_buf_pool!=NULL) && (ptr_frame!=NULL)){
        tcb_node_pool_t *pool_node = &video_device_buf_pool->avSync_frame_pool_node;
        *ptr_frame = NULL;
        if(pool_node->op != NULL){
            tcb_node_t	*temp_node;
            pool_node->op->get(pool_node, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_get_avSync_frame_tcb(temp_node);
                atomic_inc(&((*ptr_frame)->ref_count));
            }
        }
    }
}

static void	avSync_frame_buf_pool_try_get_avSync_frame_tcb(avSync_frame_buf_pool_t *video_device_buf_pool, avSync_frame_tcb_t **ptr_frame)
{
    if((video_device_buf_pool!=NULL) && (ptr_frame!=NULL)){
        tcb_node_pool_t *pool_node = &video_device_buf_pool->avSync_frame_pool_node;
        *ptr_frame = NULL;
        if(pool_node->op != NULL){
            tcb_node_t	*temp_node;
            pool_node->op->try_get(pool_node, &temp_node);
            if(temp_node != NULL){
                *ptr_frame = to_get_avSync_frame_tcb(temp_node);
                atomic_inc(&((*ptr_frame)->ref_count));
            }
        }
    }
}

static void	avSync_frame_buf_pool_put_avSync_frame_tcb(avSync_frame_buf_pool_t *video_device_buf_pool, avSync_frame_tcb_t *frame)
{
    if((video_device_buf_pool!=NULL) && (frame!=NULL)){
        tcb_node_pool_t *pool_node = &video_device_buf_pool->avSync_frame_pool_node;
        if(pool_node->op != NULL){
            pool_node->op->put(pool_node, &frame->frame_node);
        }
    }
}

static int	avSync_frame_buf_pool_get_avSync_frame_tcb_pool_entry_number(avSync_frame_buf_pool_t *video_device_buf_pool)
{
    int	ret = 0;
    if(video_device_buf_pool != NULL){
        tcb_node_pool_t *pool_node = &video_device_buf_pool->avSync_frame_pool_node;
        ret = pool_node->op->get_curr_pool_entry_number(pool_node);
    }
    return ret;
}

struct avSync_frame_buf_pool_operation	avSync_frame_buf_pool_op = {
    .create = avSync_frame_buf_pool_create,
    .release = avSync_frame_buf_pool_release,

    .get_avSync_frame_tcb = avSync_frame_buf_pool_get_avSync_frame_tcb,
    .try_get_avSync_frame_tcb = avSync_frame_buf_pool_try_get_avSync_frame_tcb,
    .put_avSync_frame_tcb = avSync_frame_buf_pool_put_avSync_frame_tcb,
    .get_avSync_frame_tcb_pool_entry_number = avSync_frame_buf_pool_get_avSync_frame_tcb_pool_entry_number,
};

int	init_avSync_frame_buf_pool(avSync_frame_buf_pool_t *video_device_buf_pool)
{
    int	ret = ISIL_ERR;
    if(video_device_buf_pool != NULL){
        video_device_buf_pool->op = &avSync_frame_buf_pool_op;
        ret = video_device_buf_pool->op->create(video_device_buf_pool, VIDEO_MASTER_CHAN_BUF_POOL_LEN);
    }
    return ret;
}

void	remove_avSync_frame_buf_pool(avSync_frame_buf_pool_t *video_device_buf_pool)
{
    if(video_device_buf_pool != NULL){
        video_device_buf_pool->op->release(video_device_buf_pool);
    }
}


