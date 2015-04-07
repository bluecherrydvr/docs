#include	<isil5864/isil_common.h>

static void isil_audio_packet_section_init(isil_audio_packet_section_t *audio_packet_section, isil_audio_chan_buf_pool_t *audio_chan_buf_pool, int id)
{
    if((audio_packet_section!=NULL) && (audio_chan_buf_pool!=NULL)){
        tcb_node_t  *node;
        atomic_set(&audio_packet_section->ref_count, 0);
        node = &audio_packet_section->packet_node;
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, audio_packet_section);
        spin_lock_init(&audio_packet_section->lock);
        audio_packet_section->id = id;
        audio_packet_section->section_size = AUDIO_SECTION_SIZE;
        audio_packet_section->payload_len = 0;
        audio_packet_section->sample_rate = ISIL_AUDIO_8000;
        audio_packet_section->type = ISIL_AUDIO_PCM;
        audio_packet_section->bit_wide = ISIL_AUDIO_16BIT;
        audio_packet_section->consumer_offset = 0;
        audio_packet_section->frameSerial = 0;
        audio_packet_section->data = audio_chan_buf_pool->audio_buffer_cache+(audio_packet_section->section_size*id);
        audio_packet_section->dma_addr = 0;
        init_isil_audio_descriptor(&audio_packet_section->descriptor, &audio_packet_section->data[audio_packet_section->section_size-sizeof(audio_section_descriptor_t)]);
    }
}

static void isil_audio_packet_section_reset(isil_audio_packet_section_t *audio_packet_section)
{
    if(audio_packet_section != NULL){
        atomic_set(&audio_packet_section->ref_count, 0);
        audio_packet_section->payload_len = 0;
        audio_packet_section->consumer_offset = 0;
        audio_packet_section->section_size = AUDIO_SECTION_SIZE;
        audio_packet_section->sample_rate = ISIL_AUDIO_8000;
        audio_packet_section->type = ISIL_AUDIO_PCM;
        audio_packet_section->bit_wide = ISIL_AUDIO_16BIT;
        audio_packet_section->frameSerial = 0;
        init_isil_audio_descriptor(&audio_packet_section->descriptor, &audio_packet_section->data[audio_packet_section->section_size-sizeof(audio_section_descriptor_t)]);
    }
}

static void isil_audio_packet_section_release(isil_audio_packet_section_t **ptr_audio_packet_section, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if((ptr_audio_packet_section!=NULL)){
        isil_audio_packet_section_t   *audio_packet_section = *ptr_audio_packet_section;
        unsigned long	flags;

        if(audio_chan_buf_pool == NULL){
            audio_chan_buf_pool = audio_packet_section->audio_chan_buf_pool;
        }
        spin_lock_irqsave(&audio_packet_section->lock, flags);
        if(atomic_read(&audio_packet_section->ref_count) > 1){
            atomic_dec(&audio_packet_section->ref_count);
        } else {
            tcb_node_t	*node;
            if(audio_packet_section->op != NULL){
                audio_packet_section->op->reset(audio_packet_section);
            }
            node = &audio_packet_section->packet_node;
            if(node->op != NULL){
                node->op->release(node, &audio_chan_buf_pool->pool_node);
            }
        }
        *ptr_audio_packet_section = NULL;
        spin_unlock_irqrestore(&audio_packet_section->lock, flags);
    }
}

static void isil_audio_packet_section_reference(isil_audio_packet_section_t *src_audio_packet_section, isil_audio_packet_section_t **dest_audio_packet_section)
{
    if((src_audio_packet_section!=NULL) && (dest_audio_packet_section!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&src_audio_packet_section->lock, flags);
        atomic_inc(&src_audio_packet_section->ref_count);
        *dest_audio_packet_section = src_audio_packet_section;
        spin_unlock_irqrestore(&src_audio_packet_section->lock, flags);
    }
}

static void isil_audio_packet_section_dma_map(isil_audio_packet_section_t *audio_packet_section, enum dma_data_direction direction)
{
    if(audio_packet_section != NULL) {
        if(audio_packet_section->data != NULL) {
            audio_packet_section->dma_addr = dma_map_single(NULL, audio_packet_section->data, audio_packet_section->section_size, direction);
            if (audio_packet_section->dma_addr == 0){
                printk("%s.%d err\n", __FUNCTION__, __LINE__);
            }
        }
    }
}

static void isil_audio_packet_section_dma_unmap(isil_audio_packet_section_t *audio_packet_section, enum dma_data_direction direction)
{
    if(audio_packet_section != NULL) {
        if (audio_packet_section->dma_addr != 0) {
            dma_unmap_single(NULL, audio_packet_section->dma_addr, audio_packet_section->section_size, direction);
            audio_packet_section->dma_addr = 0;
        }
    }
}


#ifndef NEW_HEADER
static size_t   isil_audio_packet_section_submit(isil_audio_packet_section_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    isil_frame_header_t __user    *frame_header_ptr;
    isil_frame_header_t           isil_frame_head;
    isil_audio_frame_pad_t        *audio_frame_pad_ptr;
    isil_audio_frame_pad_t        audio_frame_pad;
    size_t  buf_size;
    loff_t  buf_offset;
    unsigned int expect_len = sizeof(isil_frame_header_t) + sizeof(isil_audio_frame_pad_t) + frame->payload_len;// + sizeof(ISIL_FRAME_TIME_PAD);

    buf_size = count;
    buf_offset = 0;
    expect_len = tc_align_up(expect_len,4);

    if((frame != NULL) && (buf_size >= expect_len)){
        frame_header_ptr = (isil_frame_header_t*)data;
        buf_size -= sizeof(isil_frame_header_t);
        buf_offset += sizeof(isil_frame_header_t);

        memset(&isil_frame_head, 0, sizeof(isil_frame_header_t));
        isil_frame_head.codecType = ISIL_AUDIO_CODEC_TYPE;
        isil_frame_head.streamType = masterOrSubFlag;
        isil_frame_head.frameSerial = frame->frameSerial;
        isil_frame_head.frameType = AUDIO_FRAME_TYPE;
        isil_frame_head.timeStamp = frame->timestamp;
        isil_frame_head.payload_offset = sizeof(isil_frame_header_t);
        isil_frame_head.payloadLen = 0;

        audio_frame_pad_ptr = (isil_audio_frame_pad_t*)frame_header_ptr->pad;
        buf_size -= sizeof(isil_audio_frame_pad_t);
        buf_offset += sizeof(isil_audio_frame_pad_t);
        isil_frame_head.payloadLen += sizeof(isil_audio_frame_pad_t);

        memset(&audio_frame_pad, 0, sizeof(isil_audio_frame_pad_t));
        audio_frame_pad.type = frame->type;
        audio_frame_pad.bit_wide = frame->bit_wide;
        audio_frame_pad.sample_rate = frame->sample_rate;
        audio_frame_pad.bit_rate = 0;
        audio_frame_pad.frame_offset = sizeof(isil_audio_frame_pad_t);
        copy_to_user(audio_frame_pad_ptr->nal, frame->data, frame->payload_len);
        audio_frame_pad.frame_size = frame->payload_len;
        isil_frame_head.payloadLen += audio_frame_pad.frame_size;
        buf_size -= audio_frame_pad.frame_size;
        buf_offset += audio_frame_pad.frame_size;

        copy_to_user(frame_header_ptr->pad, (char*)&audio_frame_pad, sizeof(isil_audio_frame_pad_t));
        copy_to_user(frame_header_ptr, &isil_frame_head, sizeof(isil_frame_header_t));
        //printk("----->%x\n", frame->frameSerial);
    }
    *ppos += buf_offset;
    return (size_t)buf_offset;
}


#else


static size_t   isil_audio_packet_section_submit(isil_audio_packet_section_t *frame, char __user *data, size_t count, loff_t *ppos, int masterOrSubFlag, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    struct isil_common_stream_header  icsh, *picsh;
    isil_transparent_msg_header_t  mh, *pmh;
    isil_audio_param_msg_t  apm, *papm;
    isil_frame_msg_t  fm, *pfm;    
    size_t  buf_size;
    loff_t  buf_offset = 0;
    char *pc;
    unsigned int expect_len;


    buf_size = count;
    buf_offset = 0;
    picsh = &icsh;
    pmh = &mh;
    papm = &apm;
    pfm = &fm;

    if(frame != NULL){
        buf_offset += sizeof(struct isil_common_stream_header);
        buf_size -= sizeof(struct isil_common_stream_header);
        picsh->stream_type = ISIL_AUDIO_STREAM;
        picsh->stream_len = sizeof(struct isil_common_stream_header);

        buf_offset += sizeof(isil_transparent_msg_header_t);
        buf_size -= sizeof(isil_transparent_msg_header_t);
        pmh->isil_transparent_msg_number = 2;///////
        picsh->stream_len += sizeof(isil_transparent_msg_header_t);
        pmh->isil_transparent_msg_total_len = sizeof(isil_transparent_msg_header_t);

        buf_offset += sizeof(isil_audio_param_msg_t);
        buf_size -= sizeof(isil_audio_param_msg_t);
        papm->msg_type = ISIL_AUDIO_ENCODE_PARAM_MSG;
        papm->msg_len  = sizeof(isil_audio_param_msg_t);
        papm->audio_type = frame->type;
        papm->audio_sample_rate = frame->sample_rate;
        papm->audio_bit_wide = frame->bit_wide;
        picsh->stream_len += sizeof(isil_audio_param_msg_t);
        pmh->isil_transparent_msg_total_len += sizeof(isil_audio_param_msg_t);

        buf_offset += sizeof(isil_frame_msg_t);
        buf_size -= sizeof(isil_frame_msg_t);
        pfm->msg_type = ISIL_AUDIO_ENCODE_FRAME_MSG;
        pfm->msg_len  = sizeof(isil_frame_msg_t);
        pfm->frame_serial = frame->frameSerial;
        pfm->frame_timestamp = frame->timestamp;
        pfm->frame_checksam = compute_crc(frame->data, frame->payload_len);
        picsh->stream_len += sizeof(isil_frame_msg_t);
        pmh->isil_transparent_msg_total_len += sizeof(isil_frame_msg_t);


        expect_len = tc_align_up(frame->payload_len,4);
        if(buf_size < expect_len){
            ISIL_DBG(ISIL_DBG_ERR,"user buffer is smaller than one frame in %s\n",__FUNCTION__);
            return 0;
        }
        if(copy_to_user(&data[buf_offset], frame->data, frame->payload_len)){
            ISIL_DBG(ISIL_DBG_ERR,"copy data failed! in %s\n", __FUNCTION__);
            return 0;
        }
        buf_offset += frame->payload_len;
        pfm->msg_len += frame->payload_len;
        picsh->stream_len += frame->payload_len;
        pmh->isil_transparent_msg_total_len += frame->payload_len;
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
        if(copy_to_user(pc, &apm, sizeof(isil_audio_param_msg_t)) != 0){
            ISIL_DBG(ISIL_DBG_ERR,"%s, %d:isil_audio_param_msg_t copy_to_user failed\n", __FUNCTION__, __LINE__);
            return 0;
        }
        pc += sizeof(isil_audio_param_msg_t);
        if(copy_to_user(pc, &fm, sizeof(isil_frame_msg_t)) != 0){
            ISIL_DBG(ISIL_DBG_ERR,"%s, %d:isil_frame_msg_t copy_to_user failed\n", __FUNCTION__, __LINE__);
            return 0;
        }
    }else{

        ISIL_DBG(ISIL_DBG_ERR,"frame is NULL in %s\n",__FUNCTION__);

    }
    *ppos += buf_offset;
    return (size_t)buf_offset;
}




#endif

static int isil_audio_packet_section_copy(isil_audio_packet_section_t *src, isil_audio_packet_section_t *dst)
{
    if(src && dst) {
        isil_audio_descriptor_t   *src_descriptor, *dst_descriptor;

        dst->payload_len        = src->payload_len;
        dst->consumer_offset    = src->consumer_offset;
        dst->sample_rate        = src->sample_rate;
        dst->type               = src->type;
        dst->bit_wide           = src->bit_wide;
        dst->frameSerial        = src->frameSerial;

        dst->timestamp          = src->timestamp;

        src_descriptor          = &src->descriptor;
        dst_descriptor          = &dst->descriptor;
        //*dst_descriptor->descriptor = *src_descriptor->descriptor;
        memcpy(&dst->time_pad, &src->time_pad, sizeof(ISIL_FRAME_TIME_PAD));
        memcpy(&dst->data, &src->data, src->section_size);

        return ISIL_OK;
    }

    return -EINVAL;
}

static struct isil_audio_packet_section_operation	isil_audio_packet_section_op = {
    .init = isil_audio_packet_section_init,
    .reset = isil_audio_packet_section_reset,
    .release = isil_audio_packet_section_release,
    .reference = isil_audio_packet_section_reference,

    .dma_map = isil_audio_packet_section_dma_map,
    .dma_unmap = isil_audio_packet_section_dma_unmap,

    .submit = isil_audio_packet_section_submit,
    .copy   = isil_audio_packet_section_copy,
};

static void isil_audio_packet_queue_get(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_packet_section_t **ptr_audio_packet_section)
{
    if((audio_packet_queue!=NULL) && (ptr_audio_packet_section!=NULL)){
        tcb_node_queue_t    *node_queue;
        *ptr_audio_packet_section = NULL;
        node_queue = &audio_packet_queue->queue_node;
        if(node_queue->op != NULL){
            tcb_node_t	*temp_node;
            node_queue->op->get(node_queue, &temp_node);
            if(temp_node != NULL){
                *ptr_audio_packet_section = to_get_isil_audio_packet_section(temp_node);
            }
        }
    }
}

static void isil_audio_packet_queue_try_get(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_packet_section_t **ptr_audio_packet_section)
{
    if((audio_packet_queue!=NULL) && (ptr_audio_packet_section!=NULL)){
        tcb_node_queue_t	*node_queue;
        *ptr_audio_packet_section = NULL;
        node_queue = &audio_packet_queue->queue_node;
        if(node_queue->op != NULL){
            tcb_node_t	*temp_node;
            node_queue->op->try_get(node_queue, &temp_node);
            if(temp_node != NULL){
                *ptr_audio_packet_section = to_get_isil_audio_packet_section(temp_node);
            }
        }
    }
}

static void isil_audio_packet_queue_get_tailer(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_packet_section_t **ptr_audio_packet_section)
{
    if((audio_packet_queue!=NULL) && (ptr_audio_packet_section!=NULL)){
        tcb_node_queue_t	*node_queue;
        *ptr_audio_packet_section = NULL;
        node_queue = &audio_packet_queue->queue_node;
        if(node_queue->op != NULL){
            tcb_node_t	*temp_node;
            node_queue->op->get_tailer(node_queue, &temp_node);
            if(temp_node != NULL){
                *ptr_audio_packet_section = to_get_isil_audio_packet_section(temp_node);
            }
        }
    }
}

static void isil_audio_packet_queue_try_get_tailer(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_packet_section_t **ptr_audio_packet_section)
{
    if((audio_packet_queue!=NULL) && (ptr_audio_packet_section!=NULL)){
        tcb_node_queue_t	*node_queue;
        *ptr_audio_packet_section = NULL;
        node_queue = &audio_packet_queue->queue_node;
        if(node_queue->op != NULL){
            tcb_node_t	*temp_node;
            node_queue->op->try_get_tailer(node_queue, &temp_node);
            if(temp_node != NULL){
                *ptr_audio_packet_section = to_get_isil_audio_packet_section(temp_node);
            }
        }
    }
}

static void isil_audio_packet_queue_put(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_packet_section_t *audio_packet_section)
{
    if((audio_packet_queue!=NULL) && (audio_packet_section!=NULL)){
        tcb_node_queue_t	*node_queue;
        node_queue = &audio_packet_queue->queue_node;
        if(node_queue->op != NULL){
            node_queue->op->put(node_queue, &audio_packet_section->packet_node);
        }
    }
}

static void	isil_audio_packet_queue_put_header(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_packet_section_t *audio_packet_section)
{
    if((audio_packet_queue!=NULL) && (audio_packet_section!=NULL)){
        tcb_node_queue_t	*node_queue;
        node_queue = &audio_packet_queue->queue_node;
        if(node_queue->op != NULL){
            node_queue->op->put_header(node_queue, &audio_packet_section->packet_node);
        }
    }
}

static void isil_audio_packet_queue_get_curr_producer_from_pool(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if((audio_packet_queue!=NULL) && (audio_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&audio_packet_queue->lock, flags);
        if(audio_packet_queue->curr_producer == NULL){
            if(audio_chan_buf_pool->op != NULL){
                spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
                audio_chan_buf_pool->op->get_audio_section_tcb(audio_chan_buf_pool, &audio_packet_queue->curr_producer);
                /*if(audio_packet_queue->curr_producer != NULL){
                  audio_packet_queue->curr_producer->op->dma_map(audio_packet_queue->curr_producer, DMA_FROM_DEVICE);
                  }*/
                spin_lock_irqsave(&audio_packet_queue->lock, flags);
            }
        }
        spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
    }
}

static void isil_audio_packet_queue_try_get_curr_producer_from_pool(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if((audio_packet_queue!=NULL) && (audio_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&audio_packet_queue->lock, flags);
        if(audio_packet_queue->curr_producer == NULL){
            if(audio_chan_buf_pool->op != NULL){
                audio_chan_buf_pool->op->try_get_audio_section_tcb(audio_chan_buf_pool, &audio_packet_queue->curr_producer);
            }
            /*if(audio_packet_queue->curr_producer != NULL){
              audio_packet_queue->curr_producer->op->dma_map(audio_packet_queue->curr_producer, DMA_FROM_DEVICE);
              }*/
        }
        spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
    }
}

static void isil_audio_packet_queue_put_curr_producer_into_queue(isil_audio_packet_queue_t *audio_packet_queue)
{
    if(audio_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&audio_packet_queue->lock, flags);
        if(audio_packet_queue->curr_producer != NULL){
            //audio_packet_queue->curr_producer->op->dma_unmap(audio_packet_queue->curr_producer, DMA_FROM_DEVICE);
            if(audio_packet_queue->op != NULL){
                audio_packet_queue->op->put(audio_packet_queue, audio_packet_queue->curr_producer);
            } else {
                isil_audio_packet_queue_put(audio_packet_queue, audio_packet_queue->curr_producer);
            }
            audio_packet_queue->curr_producer = NULL;
        }
        spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
    }
}

static void isil_audio_packet_queue_release_curr_producer(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if((audio_packet_queue!=NULL) && (audio_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&audio_packet_queue->lock, flags);
        if(audio_packet_queue->curr_producer != NULL){
            //audio_packet_queue->curr_producer->op->dma_unmap(audio_packet_queue->curr_producer, DMA_FROM_DEVICE);
            if(audio_packet_queue->curr_producer->op != NULL){
                audio_packet_queue->curr_producer->op->release(&audio_packet_queue->curr_producer, audio_chan_buf_pool);
            } else {
                isil_audio_packet_section_release(&audio_packet_queue->curr_producer, audio_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
    }
}

static void isil_audio_packet_queue_get_curr_consumer_from_queue(isil_audio_packet_queue_t *audio_packet_queue)
{
    if(audio_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&audio_packet_queue->lock, flags);
        if(audio_packet_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
            if(audio_packet_queue->op != NULL){
                audio_packet_queue->op->get(audio_packet_queue, &audio_packet_queue->curr_consumer);
            } else {
                isil_audio_packet_queue_get(audio_packet_queue, &audio_packet_queue->curr_consumer);
            }
            spin_lock_irqsave(&audio_packet_queue->lock, flags);
        }
        spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
    }
}

static void isil_audio_packet_queue_try_get_curr_consumer_from_queue(isil_audio_packet_queue_t *audio_packet_queue)
{
    if(audio_packet_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&audio_packet_queue->lock, flags);
        if(audio_packet_queue->curr_consumer == NULL){
            if(audio_packet_queue->op != NULL){
                audio_packet_queue->op->try_get(audio_packet_queue, &audio_packet_queue->curr_consumer);
            } else {
                isil_audio_packet_queue_try_get(audio_packet_queue, &audio_packet_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
    }
}

static void isil_audio_packet_queue_release_curr_consumer(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if((audio_packet_queue!=NULL) && (audio_chan_buf_pool!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&audio_packet_queue->lock, flags);
        if(audio_packet_queue->curr_consumer != NULL){
            if(audio_packet_queue->curr_consumer->op != NULL){
                audio_packet_queue->curr_consumer->op->release(&audio_packet_queue->curr_consumer, audio_chan_buf_pool);
            } else {
                isil_audio_packet_section_release(&audio_packet_queue->curr_consumer, audio_chan_buf_pool);
            }
        }
        spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
    }
}

static int  isil_audio_packet_queue_get_curr_queue_entry_number(isil_audio_packet_queue_t *audio_packet_queue)
{
    int	entry_number = 0;
    if(audio_packet_queue != NULL){
        tcb_node_queue_t	*node_queue;
        node_queue = &audio_packet_queue->queue_node;
        if(node_queue->op != NULL){
            entry_number = node_queue->op->get_queue_curr_entry_number(node_queue);
        }
    }
    return entry_number;
}

static void isil_audio_packet_queue_release(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if((audio_packet_queue!=NULL) && (audio_chan_buf_pool!=NULL)){
        remove_isil_audio_packet_queue(audio_packet_queue, audio_chan_buf_pool);
    }
}

static void isil_audio_packet_queue_init(isil_audio_packet_queue_t *audio_packet_queue)
{
    if(audio_packet_queue != NULL){
        tcb_node_queue_t	*node_queue;
        audio_packet_queue->start_timestamp = 0;
        audio_packet_queue->total_duration = 0;
        node_queue = &audio_packet_queue->queue_node;
        node_queue->op = &tcb_node_queue_op;
        node_queue->op->init(node_queue);
        audio_packet_queue->curr_consumer = NULL;
        audio_packet_queue->curr_producer = NULL;
        spin_lock_init(&audio_packet_queue->lock);
    }
}

static struct isil_audio_packet_queue_operation	isil_audio_packet_queue_op = {
    .get = isil_audio_packet_queue_get,
    .try_get = isil_audio_packet_queue_try_get,
    .get_tailer = isil_audio_packet_queue_get_tailer,
    .try_get_tailer = isil_audio_packet_queue_try_get_tailer,
    .put = isil_audio_packet_queue_put,
    .put_header = isil_audio_packet_queue_put_header,

    .get_curr_producer_from_pool = isil_audio_packet_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool = isil_audio_packet_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue = isil_audio_packet_queue_put_curr_producer_into_queue,
    .release_curr_producer = isil_audio_packet_queue_release_curr_producer,
    .get_curr_consumer_from_queue = isil_audio_packet_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue = isil_audio_packet_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = isil_audio_packet_queue_release_curr_consumer,

    .get_curr_queue_entry_number = isil_audio_packet_queue_get_curr_queue_entry_number,
    .release = isil_audio_packet_queue_release,
    .init = isil_audio_packet_queue_init,
};

void    init_isil_audio_packet_queue(isil_audio_packet_queue_t *audio_packet_queue)
{
    if(audio_packet_queue != NULL){
        audio_packet_queue->op = &isil_audio_packet_queue_op;
        audio_packet_queue->op->init(audio_packet_queue);
    }
}

void    remove_isil_audio_packet_queue(isil_audio_packet_queue_t *audio_packet_queue, isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if((audio_packet_queue!=NULL) && (audio_chan_buf_pool!=NULL)){
        if(audio_packet_queue->op != NULL){
            audio_packet_queue->op->put_curr_producer_into_queue(audio_packet_queue);
            audio_packet_queue->op->release_curr_consumer(audio_packet_queue, audio_chan_buf_pool);
            while(audio_packet_queue->op->get_curr_queue_entry_number(audio_packet_queue)){
                audio_packet_queue->op->get_curr_consumer_from_queue(audio_packet_queue);
                if(audio_packet_queue->curr_consumer == NULL){
                    break;
                }
                audio_packet_queue->op->release_curr_consumer(audio_packet_queue, audio_chan_buf_pool);
            }
        } else {
            isil_audio_packet_queue_put_curr_producer_into_queue(audio_packet_queue);
            isil_audio_packet_queue_release_curr_consumer(audio_packet_queue, audio_chan_buf_pool);
            while(isil_audio_packet_queue_get_curr_queue_entry_number(audio_packet_queue)){
                isil_audio_packet_queue_get_curr_consumer_from_queue(audio_packet_queue);
                if(audio_packet_queue->curr_consumer == NULL){
                    break;
                }
                isil_audio_packet_queue_release_curr_consumer(audio_packet_queue, audio_chan_buf_pool);
            }
        }
    }
}


static int  create_isil_audio_pool(isil_audio_chan_buf_pool_t *audio_chan_buf_pool, unsigned long buf_len)
{
    int	ret = ISIL_ERR;
    if(audio_chan_buf_pool != NULL){
        tcb_node_pool_t	*node_pool;
        isil_audio_packet_section_t   *ptr_audio_section_tcb;
        int i;

        audio_chan_buf_pool->audio_buffer_entry_order = get_order(buf_len);
        audio_chan_buf_pool->audio_buffer_cache = (u8*)__get_free_pages(GFP_KERNEL, audio_chan_buf_pool->audio_buffer_entry_order);
        if(audio_chan_buf_pool->audio_buffer_cache == NULL) {
            audio_chan_buf_pool->audio_buffer_entry_order = 0;
            printk("can't alloc %d pages for audio buf pool\n", audio_chan_buf_pool->audio_buffer_entry_order);
            return ret;
        }

        audio_chan_buf_pool->audio_section_tcb_number = (buf_len/AUDIO_SECTION_SIZE);
        node_pool = &audio_chan_buf_pool->pool_node;
        node_pool->op = &tcb_node_pool_op;
        audio_chan_buf_pool->audio_section_tcb_cache = (isil_audio_packet_section_t*)kmalloc(sizeof(isil_audio_packet_section_t)*audio_chan_buf_pool->audio_section_tcb_number, GFP_KERNEL);
        if(audio_chan_buf_pool->audio_section_tcb_cache == NULL){
            free_pages((unsigned long)audio_chan_buf_pool->audio_buffer_cache, audio_chan_buf_pool->audio_buffer_entry_order);
            audio_chan_buf_pool->audio_buffer_cache = NULL;
            audio_chan_buf_pool->audio_buffer_entry_order = 0;
            printk("can't alloc audio buf section tcb\n");
            return ret;
        }
        node_pool->op->init(node_pool, audio_chan_buf_pool->audio_section_tcb_number);
        for(i=0; i<audio_chan_buf_pool->audio_section_tcb_number; i++){
            ptr_audio_section_tcb = &audio_chan_buf_pool->audio_section_tcb_cache[i];
            ptr_audio_section_tcb->op = &isil_audio_packet_section_op;
            ptr_audio_section_tcb->audio_chan_buf_pool = audio_chan_buf_pool;
            ptr_audio_section_tcb->op->init(ptr_audio_section_tcb, audio_chan_buf_pool, i);
            ptr_audio_section_tcb->op->release(&ptr_audio_section_tcb, audio_chan_buf_pool);
        }
        ret = ISIL_OK;
    }
    return ret;
}

static void release_isil_audio_pool(isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if(audio_chan_buf_pool != NULL){
        tcb_node_pool_t	*pool_node;
        pool_node = &audio_chan_buf_pool->pool_node;
        if(pool_node->op != NULL){
            pool_node->op->release(pool_node);
        }
        if(audio_chan_buf_pool->audio_section_tcb_cache != NULL){
            kfree(audio_chan_buf_pool->audio_section_tcb_cache);
            audio_chan_buf_pool->audio_section_tcb_cache = NULL;
        }
        audio_chan_buf_pool->audio_section_tcb_number = 0;

        if(audio_chan_buf_pool->audio_buffer_cache != NULL){
            free_pages((unsigned long)audio_chan_buf_pool->audio_buffer_cache, audio_chan_buf_pool->audio_buffer_entry_order);
            audio_chan_buf_pool->audio_buffer_cache = NULL;
        }
        audio_chan_buf_pool->audio_buffer_entry_order = 0;
    }
}

static void isil_audio_chan_buf_pool_get_audio_section_tcb(isil_audio_chan_buf_pool_t *audio_chan_buf_pool, isil_audio_packet_section_t **ptr_audio_packet_section)
{
    if((audio_chan_buf_pool!=NULL) && (ptr_audio_packet_section!=NULL)){
        tcb_node_pool_t *pool_node = &audio_chan_buf_pool->pool_node;
        *ptr_audio_packet_section = NULL;
        if(pool_node->op != NULL){
            tcb_node_t	*temp_node;
            pool_node->op->get(pool_node, &temp_node);
            if(temp_node != NULL){
                *ptr_audio_packet_section = to_get_isil_audio_packet_section(temp_node);
                atomic_inc(&((*ptr_audio_packet_section)->ref_count));
            }
        }
    }
}

static void isil_audio_chan_buf_pool_try_get_audio_section_tcb(isil_audio_chan_buf_pool_t *audio_chan_buf_pool, isil_audio_packet_section_t **ptr_audio_packet_section)
{
    if((audio_chan_buf_pool!=NULL) && (ptr_audio_packet_section!=NULL)){
        tcb_node_pool_t *pool_node = &audio_chan_buf_pool->pool_node;
        *ptr_audio_packet_section = NULL;
        if(pool_node->op != NULL){
            tcb_node_t	*temp_node;
            pool_node->op->try_get(pool_node, &temp_node);
            if(temp_node != NULL){
                *ptr_audio_packet_section = to_get_isil_audio_packet_section(temp_node);
                atomic_inc(&((*ptr_audio_packet_section)->ref_count));
            }
        }
    }
}

static void isil_audio_chan_buf_pool_put_audio_section_tcb(isil_audio_chan_buf_pool_t *audio_chan_buf_pool, isil_audio_packet_section_t *audio_packet_section)
{
    if((audio_chan_buf_pool!=NULL) && (audio_packet_section!=NULL)){
        tcb_node_pool_t *pool_node = &audio_chan_buf_pool->pool_node;
        if(pool_node->op != NULL){
            pool_node->op->put(pool_node, &audio_packet_section->packet_node);
        }
    }
}

static int  isil_audio_chan_buf_pool_get_audio_section_tcb_pool_entry_number(isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    int	entry_number = 0;
    if(audio_chan_buf_pool!=NULL){
        tcb_node_pool_t *pool_node = &audio_chan_buf_pool->pool_node;
        if(pool_node->op != NULL){
            entry_number = pool_node->op->get_curr_pool_entry_number(pool_node);
        }
    }
    return entry_number;
}

static struct isil_audio_chan_buf_pool_operation	isil_audio_chan_buf_pool_op = {
    .create = create_isil_audio_pool,
    .release = release_isil_audio_pool,

    .get_audio_section_tcb = isil_audio_chan_buf_pool_get_audio_section_tcb,
    .try_get_audio_section_tcb = isil_audio_chan_buf_pool_try_get_audio_section_tcb,
    .put_audio_section_tcb = isil_audio_chan_buf_pool_put_audio_section_tcb,
    .get_audio_section_tcb_pool_entry_number = isil_audio_chan_buf_pool_get_audio_section_tcb_pool_entry_number,
};

int init_audio_chan_buf_pool(isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    int	ret = ISIL_ERR;
    if(audio_chan_buf_pool != NULL){
        audio_chan_buf_pool->op = &isil_audio_chan_buf_pool_op;
        ret = audio_chan_buf_pool->op->create(audio_chan_buf_pool, AUDIO_CHAN_BUF_POOL_LEN);
    }
    return ret;
}

void    remove_audio_chan_buf_pool(isil_audio_chan_buf_pool_t *audio_chan_buf_pool)
{
    if(audio_chan_buf_pool != NULL){
        if(audio_chan_buf_pool->op != NULL){
            printk("%s.%d: %d\n", __FUNCTION__, __LINE__, audio_chan_buf_pool->op->get_audio_section_tcb_pool_entry_number(audio_chan_buf_pool));
            audio_chan_buf_pool->op->release(audio_chan_buf_pool);
        } else {
            release_isil_audio_pool(audio_chan_buf_pool);
        }
    }
}

