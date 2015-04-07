#include <isil5864/tc_common.h>
void set_tq_state_busy(struct tc_queue *tq)
{
    struct tc_buffer_kern *tck;
    unsigned int cix = 0;

    while((tck = tq->bufs[cix]) && (STATE_READY == get_tck_state(tck))){
        set_tck_state(tck,STATE_BUSY);
        cix++;
    }	
}


int calc_data_span(struct tc_buffer_kern *tck)
{
    int s = 0,nr = tck->index,idx;
    struct tc_buffer_kern *tk;
    struct tc_queue *tq = tck->tq;
    int o;

    tk = tq->bufs[0];
    o = tk->off_to_page;
    if(o >= 4096)
        dbg("BUG: the off_to_page is too big!\n");


    for(idx = 0; idx < nr;)
    {
        tk = tq->bufs[idx];
        if(tk->tq->unit_size >= PAGE_SIZE)
        {
            s += tc_align_up(tk->size, PAGE_SIZE);
        }
        else
        {
            s += tc_align_up(tk->size,tk->tq->unit_size);
        }
        idx++;
        if(idx >= tq->buf_nr)
            break;
    }
    if(tk->tq->unit_size < PAGE_SIZE)
        s += o;

    //dbg("data_span[%d]\n",s);
    return s;
}



void tc_buffer_set_audio_data(struct tc_buffer *tc, struct tc_buffer_kern *tck,isil_audio_packet_section_t *taps, unsigned long *off)
{
    struct tc_buffer stck = {0};
    struct tc_frame_header tfh = {0};
    struct tc_audio_frame_pad   tafp = {0};
    struct tc_frame_header *ptfh;
    //unsigned long offset = tc_align_up(MIN_HEADER_LEN_AUDIO,4) * tck->index;
    unsigned long x = PAGE_SIZE - (*off + MIN_COMMON_HEAD_LEN);
    unsigned long s;
    char *dest;
    //isil_audio_descriptor_t  *ad = &taps->descriptor;


    ptfh = (struct tc_frame_header *)((char *)(tc->tc_frame_head) + *off);
    dest = (char *)tc + *off;
    s = calc_data_span(tck);

    /*tc_audio_frame_pad stuff*/
    tafp.type = taps->type;
    tafp.bit_wide = taps->bit_wide;
    tafp.sample_rate = taps->sample_rate;
    tafp.bit_rate = 0;
    tafp.frame_offset = x + s;
    tafp.frame_size = taps->payload_len;

    /*tc_frame_header stuff*/
    tfh.codecType = ISIL_AUDIO_CODEC_TYPE;
    tfh.streamType = ISIL_MASTER_BITSTREAM;
    tfh.frameSerial = taps->frameSerial;
    tfh.frameType = AUDIO_FRAME_TYPE;
    tfh.timeStamp = taps->timestamp;
    tfh.payload_offset = sizeof(struct tc_frame_header);
    tfh.payloadLen = sizeof(struct tc_audio_frame_pad) + tafp.frame_size;

    /*tc_buffer stuff*/
    stck.index = tck->index;
    stck.type = tck->type;
    stck.flag = tck->flag;
    //stck.timestap = tck->timestap;
    stck.__tc__state = tck->state;
    stck.__tc__isalign = tck->isalign;
    stck.__tc__off_to_page = tck->off_to_page;
    stck.__tc__start = tck->start;                //actually net payload data start without any header
    stck.__tc__size = taps->payload_len;          //actully the net payload len;
    memcpy(ptfh->pad,&tafp,sizeof(struct tc_audio_frame_pad));
    memcpy(ptfh,&tfh,sizeof(struct tc_frame_header));
    memcpy(dest,&stck,sizeof(struct tc_buffer));
    *off += tc_align_up(MIN_HEADER_LEN_AUDIO,4);
}



int calc_frame_size(struct tc_buffer_kern *tck)
{
    int size = 0,idx = 0;

    while(tck->sg[idx].carrier && tck->sg[idx].addr && tck->sg[idx].len){
        size += tck->sg[idx].len;
        idx++;
        if(idx >= SCATTRT_MAX_ENTRY)
            break;
    }

    //ISIL_DBG("c[%d]\n",size);
    return size;
}




void tc_buffer_set_h264_data(int is_master,struct tc_buffer *tc, struct tc_buffer_kern *tck, isil_video_frame_tcb_t *vf,unsigned long *off)
{
    struct tc_buffer stck = {0};//48
    struct tc_frame_header tfh = {0};//28
    struct tc_h264_idr_frame_pad   tvfp = {0};//32
    struct tc_frame_header *ptfh;
    h264_nal_t  *nal = &vf->nal;
    int frame_size = 0;
    unsigned long x = PAGE_SIZE - (*off + MIN_COMMON_HEAD_LEN);
    unsigned long y = PAGE_SIZE - (*off + sizeof(struct tc_buffer));
    unsigned long s;
    char *dest;
    struct tc_frame_header *tp;


    ptfh = (struct tc_frame_header *)((char *)(tc->tc_frame_head) + *off);
    dest = (char *)tc + *off;
    if(((unsigned int)ptfh) - ((unsigned int)tc) >= 4000){
        printk("\n\n&&&&&&&&&&&&&impossiable&&&&&&&&&&&&&&&&&&&&&&&&&&&&& %p, %p\n", ptfh, tc);
    }
    if(((unsigned int)dest) - ((unsigned int)tc) >= 4000){
        printk("\n\n**************impossiable**************************** %p %p\n", dest, tc);
    }
    tp = (struct tc_frame_header *)((struct tc_buffer *)dest)->tc_frame_head;
    frame_size = tck->size;
    //dbg("tc--start[%8p],off[%lu], ptfh[%8p],dest[%8p],tp[%8p]\n",tc,*off,ptfh,dest,tp);

    s = calc_data_span(tck);
    //dbg("\nnumber[%d]: x[%lu] data span[%lu]\n******************************************************\n ",tck->index,x,s);

    if(H264_FRAME_TYPE_IDR == vf->frame_type){
        /*tc_h264_idr_frame_pad_t stuff*/
        tvfp.fps = vf->fps;
        tvfp.init_qp = vf->i_init_qp;
        tvfp.log2MaxFrameNumNinus4 = vf->i_log_gop_value;
        tvfp.mb_width_minus1 = vf->i_mb_x;
        tvfp.mb_height_minus1 = vf->i_mb_y;
        tvfp.i_curr_qp = vf->i_curr_qp;

        tvfp.sps_frame_offset = x + s;
        tvfp.sps_frame_size = nal->sps_paysize;
        tvfp.pps_frame_offset = tvfp.sps_frame_offset + nal->sps_paysize;
        tvfp.pps_frame_size = nal->pps_paysize;
        tvfp.idr_frame_offset = tvfp.pps_frame_offset + nal->pps_paysize;
        tvfp.idr_frame_size = vf->frame_len;
        //dbg("sps_off [%d] sps_size[%d],pps_off[%d],pps_size[%d],idr_off[%d],idr_size[%d]\n",
        //	tvfp.sps_frame_offset,tvfp.sps_frame_size,tvfp.pps_frame_offset,
        //	tvfp.pps_frame_size,tvfp.idr_frame_offset,tvfp.idr_frame_size);
    }
    //dbg("***********************************************\n");
    /*tc_frame_header stuff*/
    tfh.codecType = ISIL_VIDEO_H264_CODEC_TYPE;
    if(is_master)
        tfh.streamType = ISIL_MASTER_BITSTREAM;
    else
        tfh.streamType = ISIL_SUB_BITSTREAM;

    tfh.frameSerial = vf->frame_number;
    tfh.frameType = vf->frame_type;
    tfh.timeStamp = vf->timestamp;
    if(H264_FRAME_TYPE_IDR == vf->frame_type){
        tfh.payload_offset = sizeof(struct tc_frame_header);
        tfh.payloadLen = sizeof(struct tc_h264_idr_frame_pad) + frame_size;                
    }
    else
    {
        tfh.payload_offset = y + s;
        tfh.payloadLen = frame_size;
    }

    /*tc_buffer stuff*/
    stck.index = tck->index;
    stck.type = tck->type;
    stck.flag = tck->flag;
    //stck.timestap = tck->timestap;
    stck.__tc__state = tck->state;
    stck.__tc__isalign = 1;
    stck.__tc__off_to_page = 0;
    stck.__tc__start = tck->sg[0].addr;
    stck.__tc__size = frame_size;

    if(H264_FRAME_TYPE_IDR == vf->frame_type)
        memcpy(ptfh->pad,&tvfp,sizeof(struct tc_h264_idr_frame_pad));
    memcpy(ptfh,&tfh,sizeof(struct tc_frame_header));
    memcpy(dest,&stck,sizeof(struct tc_buffer));

    if(H264_FRAME_TYPE_IDR == vf->frame_type)
        *off += tc_align_up(MIN_HEADER_LEN_VIDEO_IDR,4);
    else
        *off += tc_align_up(MIN_HEADER_LEN_VIDEO,4);

    //dbg("\ncodectype[%d],\nstreamtype[%d],\nframe_type[%d],\nframeSerial[%d],\ntimeStamp[%d],\npayload_offset[%d],\npayloadLen[%d]\n",
    //	tp->codecType,tp->streamType,tp->frameType,tp->frameSerial,tp->timeStamp,tp->payload_offset,tp->payloadLen);

}


void tc_buffer_set_jpeg_data(struct tc_buffer *tc, struct tc_buffer_kern *tck, struct isil_vb_frame_tcb *tvf, unsigned long *off)
{
    struct tc_buffer stck = {0};
    struct tc_frame_header tfh = {0};
    struct tc_frame_header *ptfh;
    int frame_size = 0;
    unsigned long x = PAGE_SIZE - (*off + sizeof(struct tc_buffer));
    unsigned long s;
    char *dest;
    //tc_trace;

    ptfh = (struct tc_frame_header *)(((char *)tc->tc_frame_head) + (*off));
    dest = (char *)tc + (*off);
    frame_size = tck->size;//calc_frame_size(tck);
    s = calc_data_span(tck);

    /*tc_frame_header stuff*/
    tfh.codecType = ISIL_VIDEO_MJPEG_CODEC_TYPE;
    tfh.streamType = ISIL_MASTER_BITSTREAM;
    tfh.frameSerial = tvf->frame_number;
    tfh.frameType = tvf->frame_type;
    tfh.timeStamp = tvf->timestamp;
    tfh.payload_offset = x + s;
    tfh.payloadLen = frame_size;

    /*tc_buffer stuff*/
    stck.index = tck->index;
    stck.type = tck->type;
    stck.flag = tck->flag;
    //stck.timestap = tck->timestap;
    stck.__tc__state = tck->state;
    stck.__tc__isalign = 1;
    stck.__tc__off_to_page = 0;
    stck.__tc__start = tck->sg[0].addr;
    stck.__tc__size = frame_size;
    memcpy(ptfh,&tfh,sizeof(struct tc_frame_header));
    memcpy(dest,&stck,sizeof(struct tc_buffer));
    *off += tc_align_up(MIN_HEADER_LEN_JPEG,4);
}


/*
   struct tc_buffer_kern * find_first_free_tck(struct tc_queue *tq)
   {
   struct tc_buffer_kern *tck = tq->bufs;
   unsigned long idx = 0;

   for(;idx < tq->buf_nr;idx++,tck++)
   {
   if(tck->state == STATE_FREE){
   ISIL_DBG("*********************find the freeed idx = %d\n",idx);
   return tck;
   }
   }	

   return NULL;
   }
   */



static int init_queue_bufs(struct tc_queue * tq)
{
    unsigned long idx,cidx;
    unsigned long nr;
    //struct tc_buffer_kern *tck = tq->bufs[0];

    if(NULL == tq->bufs){
        ISIL_DBG(ISIL_DBG_ERR,"the tq->bufs = NULL!\n");
        return -EINVAL;
    }

    if(0 == tq->buf_nr){
        ISIL_DBG(ISIL_DBG_ERR,"the tq->buf_nr = 0!\n");
        return -EINVAL;
    }

    nr = tc_min(tq->buf_nr,QUEUE_MAX_FRAME);
    if(tq->buf_nr > QUEUE_MAX_FRAME){
        ISIL_DBG(ISIL_DBG_ERR,"max %d bufs\n",QUEUE_MAX_FRAME);
        tq->buf_nr = QUEUE_MAX_FRAME;
    }	

    for(idx = 0;idx < nr; idx++)
    {
        tq->bufs[idx] = kzalloc(sizeof(struct tc_buffer_kern),GFP_KERNEL);
        if(NULL == tq->bufs[idx])
            goto cleanup;
        init_waitqueue_head(&tq->bufs[idx]->done);
        tq->bufs[idx]->index = idx;
        tq->bufs[idx]->tq = tq;
        set_tck_state(tq->bufs[idx],STATE_FREE);		
    }
    return 0;	
cleanup:
    for(cidx = 0; cidx < idx; cidx++)
    {
        kfree(tq->bufs[cidx]);
    }

    return -1;
}








int tc_queue_init(struct tc_queue *q, struct tc_queue_ops *ops, void * dev,
        unsigned long usize, unsigned long buf_count,void *priv)
{
    if(usize == 0 || buf_count == 0){
        ISIL_DBG(ISIL_DBG_ERR,"init with 0 size or 0 count!\n");
        return -1;	
    }

    memset(q,0,sizeof(*q));
    q->dev     = dev;
    //q->type    = type;
    q->unit_size   = usize;
    q->ops     = ops;
    q->priv_data = priv;
    q->buf_nr = buf_count;
    //mutex_init(&q->lock);
    init_waitqueue_head(&q->done);
    q->wp = 0;
    q->rp = 0;
    return 	init_queue_bufs(q);
}

void  tc_queue_reset(struct tc_queue *q)
{
    unsigned long idx = 0;
    struct tc_buffer_kern * tck;

    //tc_trace;
    //mutex_lock(&q->lock);
    /*
       while(!queue_empty(q))
       {
       q->ops->buf_release(q,q->priv_data,q->bufs[q->rp]);
       queue_inc_rp(q);
       }
       */
    //mutex_unlock(&q->lock);

    for(idx = 0; idx < q->buf_nr; idx++)
    {
        tck = q->bufs[idx];
        init_waitqueue_head(&tck->done);
        set_tck_state(tck,STATE_FREE);
    } 
    q->wp = 0;
    q->rp = 0;

}


void tc_queue_release(struct tc_queue *q)
{
    unsigned long idx = 0;
    //struct tc_buffer_kern * tck;

    //	mutex_lock(&q->lock);
    /*
       while(!queue_empty(q))
       {
       q->ops->buf_release(q,q->priv_data,q->bufs[q->rp]);
       queue_inc_rp(q);
       }
       */
    //	mutex_unlock(&q->lock);

    for(idx=0; idx < q->buf_nr; idx++)
        kfree(q->bufs[idx]);

    kfree(q);
}




