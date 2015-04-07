#include <isil5864/tc_common.h>

struct isil_chip_audio_param_my  def_audio_parm_de = {
    .i_bit_wide = ISIL_AUDIO_16BIT,
    .i_sample_rate = ISIL_AUDIO_8K,
    .e_audio_type = ISIL_AUDIO_PCM,
};


int  de_buf_prepare(struct tc_queue * tq, void *priv,struct tc_buffer *tc)
{
    struct tc_buffer_kern *tck;
    struct isil_audio_de * tde = (struct isil_audio_de *)priv;
    struct isil_audio_chan_driver *driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    isil_audio_packet_queue_t     *adpq;
    isil_audio_packet_section_t   *as;
    isil_audio_chan_buf_pool_t    *adpool;			

    adpq = &driv->audio_packet_queue;
    adpool = &driv->audio_buf_pool;

    //tc_trace;
    do
    {
        adpq->op->get_curr_consumer_from_queue(adpq);
        //dbg("after get consumer!\n");
        as = adpq->curr_consumer;
        if(!as){
            dbg("\n we get section nothing!---------------------------------\n");
            return 0;	
        }

        //dbg("id =%d;  section_size = %d;  payload_len= %d,consumer_offset= %d timestamp =%d\n",
        //	as->id,as->section_size,as->payload_len,as->consumer_offset,as->timestamp);	

        mutex_lock(&tde->tsd.queue->lock);	
        //dbg("**********************************before wp inc =%d\n",tq->wp);
        tck = tq->bufs[tq->wp];
        /*arrive here must have freed tck!!!*/
        tck->index = as->id;
        tck->type = MEM_MMAP;
        set_tck_state(tck,STATE_READY);
        tck->isalign = ((unsigned long)as->data >> PAGE_SHIFT)? 1: 0;
        tck->off_to_page = ((unsigned long)as->data) & 0xFFF;
        tck->start = (__u32 *)as->data;
        tck->size = tde->tsd.queue->unit_size;
        queue_inc_wp(tde->tsd.queue);
        mutex_unlock(&tde->tsd.queue->lock);
        //tc_buffer_set_audio_data(tc,tck,as);

        //dbg("section start = %X, off_to_page = %X\n",tck->start,tck->off_to_page);
    }while(0);

    return 0;	
}

int  de_buf_release(struct tc_queue * tq, void * priv, struct tc_buffer_kern *tb)
{
    int ret = 0;
    struct isil_audio_de * tde = (struct isil_audio_de *)priv;
    struct isil_audio_chan_driver *driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    isil_audio_packet_queue_t     *adpq;
    isil_audio_chan_buf_pool_t    *adpool;			

    //tc_trace;
    adpq = &driv->audio_packet_queue;
    adpool = &driv->audio_buf_pool;
    adpq->op->release_curr_consumer(adpq, adpool);
    return ret;
}

struct tc_queue_ops    de_queue_ops = {
    .buf_prepare = de_buf_prepare,
    .buf_release = de_buf_release,	
};


/*
   int  request_memory(struct file *f,unsigned long arg)
   {
   int ret = 0;
   struct tc_buffer_kern *tck;
   unsigned long  *p = (unsigned long *)arg;
   struct tc_buffer *tc = (struct tc_buffer *)(*p);
   struct isil_audio_de * tde = (struct isil_audio_de *)f->private_data;
   struct tc_queue *q = tde->queue;
   unsigned long fa = 0;

   tc_trace;
   down(&tde->sem);
   while(queue_full(q)){
   if(f->f_flags & O_NONBLOCK){
   ret = -EAGAIN;	
   goto out_direct;
   }
   wait_event_interruptible(q->done, !queue_full(q));
   dbg("queue is full, we still need wait!!!!...\n");		
   }

   if(q->ops->buf_prepare)
   ret = q->ops->buf_prepare(q,tde,tc);
out_direct:
up(&tde->sem);
return ret;
}


int  release_memory(struct file *f,unsigned long arg)
{
int ret = 0;	
struct isil_audio_de * tde = (struct isil_audio_de *)f->private_data;
struct tc_queue *tq = tde->queue;
struct tc_buffer_kern * tck;	

down(&tde->sem);
if(queue_empty(tq)){
dbg("queue is empty");
ret = -EINVAL;
goto out;
}

mutex_lock(&tde->queue->lock);
tck = tq->bufs[tq->rp];
if(tq->ops->buf_release)
tq->ops->buf_release(tq,tde,tck);
queue_inc_rp(tde->queue);
set_tck_state(tck,STATE_FREE);
wake_up_interruptible(&tq->done);
mutex_unlock(&tde->queue->lock);	
out:
up(&tde->sem);
return 0;
}
*/

isil_audio_driver_t * lookup_drive_instance_de(struct isil_service_device *tsd)
{
    isil_audio_driver_t *p = tsd->private_data;
    unsigned long ld = get_phchan_id_from_epobj(&tsd->epobj);
    unsigned long ulen = sizeof(isil_audio_driver_t);
    char * q = (char *)&p[ld];
    q = q - 17*ulen;
    printk("%s ld = %lu, decoder add = %p first encoder add should be %p\n",__FUNCTION__,ld,&p[ld],q);

    return &p[ld];

}


void tde_cleanup(void *arg)
{
    struct isil_audio_de *tde = (struct isil_audio_de *)arg;

    //tc_trace;
    tc_queue_release(tde->tsd.queue);
    kfree(tde);	
}



static void audi_de_polling(unsigned long p)
{
    struct isil_audio_de *tde;// = (struct isil_audio_de *)p;
    struct isil_audio_chan_driver *driv;// = container_of(tde->tsd.ped,struct isil_audio_chan_driver, audio_ed);
    isil_audio_chan_buf_pool_t    *ap;// = &driv->audio_buf_pool;
    //isil_audio_packet_queue_t     *adpq = &driv->audio_packet_queue;	
    unsigned long nr;// = adpq->op->get_curr_queue_entry_number(adpq);

    if(0 == p){
        printk("BUG: NULL pointer in %s\n", __FUNCTION__);
        return;
    }
    tde = (struct isil_audio_de *)p;
    driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    ap = &driv->audio_buf_pool;
    nr = ap->op->get_audio_section_tcb_pool_entry_number(ap);
    //printk("in %s the entry number = %lu\n",__FUNCTION__,nr);	
    if(nr > 10)
    {
        wake_up(&tde->wait_poll);
    }
    else
    {
        //printk("in %s the entry number = %d\n",__FUNCTION__,nr);	
    }

    mod_timer(&tde->polling_timer, jiffies + 40);
    return;
}


int audio_de_open(struct isil_service_device *tsd,struct file *f)
{
    int ret = 0;
    ed_tcb_t   *aded;
    struct isil_audio_de *tde = container_of(tsd,struct isil_audio_de,tsd);
    //struct isil_audio_chan_driver *driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, audio_ed);
    //tc_trace;

    if(!tde)
        return -EINVAL;
    if(f->private_data){
        dbg("have already opened!\n");
        return 0;
    }

    if(!tde->tsd.ped){
        dbg("audio decoder has no driver attached!\n");
        ret = -1;
        return ret;
    }

	aded = tde->tsd.ped;//&driv->audio_ed;
	ret = aded->op->open(aded);
	if(ret != ISIL_OK){
		printk("in %s audio decoder driver open failed!\n",__FUNCTION__);
		return ret;
	}
	f->private_data = tde;

    tde->polling_timer.expires = jiffies + 40 * HZ;
    tde->polling_timer.data = (unsigned long)tde;
    tde->polling_timer.function = audi_de_polling;
    add_timer(&tde->polling_timer);
    return ret;
}



ssize_t audio_de_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
    return 0;
}


void watch_timestamp(unsigned long  time)
{
    static unsigned long last = 0, cnt = 0,delta = 0;

    cnt = time;
    delta = (0x10000 + cnt - last)&0xffff;
    if(delta > 100  && last != 0){
        dbg("%s long time has no new frame!\n",__FUNCTION__);
    }
    last = cnt;
}


#ifndef  NEW_HEADER
ssize_t process_logic_frame(isil_audio_packet_queue_t *adpq, isil_audio_chan_buf_pool_t *adpool, unsigned long ulen, unsigned long tlen, 
        const char __user *buff,  size_t * offset,  loff_t *lof,unsigned long time_step )
{
    int ret = 0,remain = tlen; 
    static size_t last_remain;
    isil_audio_packet_section_t   *as = NULL;
    int iof = (int)(*offset - ISIL_TOTAL_AUDIO_FRAMEHEADER_LEN);
    isil_frame_header_t           *ph = (isil_frame_header_t *)&buff[iof];
    isil_audio_frame_pad_t        *pad = (isil_audio_frame_pad_t *)ph->pad;
    //unsigned long time_stamp = ph->timeStamp;
    unsigned long icount =0;

    //dbg("ul[%d],tl[%d],off[%d]\n",ulen,tlen,*offset);
    while(remain > 0){
        icount++;
        if((as = adpq->curr_producer)){
            if( pad->sample_rate != as->sample_rate || pad->bit_wide != as->bit_wide || pad->type != as->type){
                as->payload_len = (remain > ulen)? ulen:remain;
                if(as->payload_len < ulen) printk("one logic frame is too small!\n");
                as->sample_rate = pad->sample_rate;
                as->type = pad->type;
                as->bit_wide = pad->bit_wide;
                as->timestamp = ph->timeStamp;
                watch_timestamp(ph->timeStamp);				
                if(copy_from_user(as->data,&buff[*offset],as->payload_len) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(as->payload_len < ulen)
                    goto deal_tail;
                adpq->op->put_curr_producer_into_queue(adpq);			
                *offset += as->payload_len;
                remain -= as->payload_len;							
            }
            else
            {
                if(copy_from_user(&as->data[last_remain],&buff[*offset],ulen -last_remain) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                as->payload_len = ulen;			
                adpq->op->put_curr_producer_into_queue(adpq);
                *offset += ulen -last_remain;
                remain -= ulen -last_remain;				
            }	
        }
        else
        {
            adpq->op->get_curr_producer_from_pool(adpq,adpool);
            as = adpq->curr_producer;
            if(as){
                as->payload_len = (remain > ulen)? ulen:remain;
                as->sample_rate = pad->sample_rate;
                as->type = pad->type;
                as->bit_wide = pad->bit_wide;
                as->timestamp = ph->timeStamp;////time_stamp + idx*time_step;
                watch_timestamp(ph->timeStamp);
                if(copy_from_user(as->data,&buff[*offset],as->payload_len) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(as->payload_len < ulen){		
                    goto deal_tail;
                }
                adpq->op->put_curr_producer_into_queue(adpq);		
                *offset += as->payload_len;
                remain -= as->payload_len;				
            }
            else
            {
                dbg("can not get section!!!!!!!!!!!!!!!!!!!!!!!!\n");
                ret = -ENOMEM;
                goto failed;				
            }					
        }
    }
deal_tail:
    if(remain)
        last_remain = as->payload_len;
    else
        last_remain = 0;
    //dbg("la[%d],cn[%d]\n",last_remain,icount);
    //out:
    *lof += *offset; 
    ret = tlen-remain;
    return ret;
failed:
    return ret;
}



ssize_t audio_de_write (struct file *f, const char __user *buff, size_t count, loff_t *lof)
{
    int ret = 0;//,idx = 0;
    unsigned long  total_paylen;
    struct isil_audio_de *tde = (struct isil_audio_de *)f->private_data;
    struct isil_audio_chan_driver *driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    isil_audio_packet_queue_t     *adpq = &driv->audio_packet_queue;
    isil_audio_chan_buf_pool_t    *adpool = &driv->audio_buf_pool;
    isil_frame_header_t           *ph = (isil_frame_header_t *)buff;
    //char          *phtest = NULL;
    isil_audio_frame_pad_t        *pad = (isil_audio_frame_pad_t *)ph->pad;
    //unsigned long lfhl = sizeof(isil_frame_header_t) + sizeof(isil_audio_frame_pad_t);	
    size_t   offset = 0;
    unsigned long gap = 0,pad_nr = 0;
    unsigned long unit_paylen = 384; 
    unsigned long time_delta = 20;//= calc_time_stamp_deta(pad,unit_paylen);    

    down(&tde->sem);
    //dbg("i->tck=%d\n",get_isil_system_tick());
    while(1)
    {
        offset += ISIL_TOTAL_AUDIO_FRAMEHEADER_LEN;
#ifdef ISIL_CONFIG_ADD_NAL
        total_paylen = pad->frame_size - sizeof(ext_h264_nal_bitstream_t);
#else
        total_paylen = pad->frame_size;
#endif
        //dbg("fp[%d];pp[%d];off[%d],tl[%d]\n",ph->payloadLen,pad->frame_size,offset,total_paylen);  
        ret = process_logic_frame(adpq,adpool,unit_paylen,total_paylen,buff,&offset,lof,time_delta);
        if(ret < 0){
            up(&tde->sem);
            return ret;
        }
        gap = total_paylen-ret;
        if(offset >= count-3)
            break;
        pad_nr = (offset + gap) & 0x3;
        if(gap < unit_paylen){
            if(offset + gap + pad_nr + ISIL_TOTAL_AUDIO_FRAMEHEADER_LEN >= count)
                goto out;
            ph = (isil_frame_header_t *)&buff[offset + gap + pad_nr];   
            offset += gap;
            offset += pad_nr;	
        }
        else
        {
            printk("data wrong format!!!!!!!!!!!!!!!!\n");
            up(&tde->sem);
            return -EINVAL;
        }
        pad = (isil_audio_frame_pad_t *)ph->pad;
    }
out:
    up(&tde->sem);
    //printk("o<-tk=%d\n",get_isil_system_tick());
    return (ssize_t)offset;
}

#else

static unsigned int query_space(struct file *file)
{
        unsigned int v,t;
	struct isil_audio_de *tde = (struct isil_audio_de *)file->private_data;
        struct isil_audio_chan_driver *driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
        isil_audio_chan_buf_pool_t    *ap = &driv->audio_buf_pool;

        v = ap->op->get_audio_section_tcb_pool_entry_number(ap);
        t = AUDIO_SECTION_SIZE;
        return v * t;
}



static int get_space_onnode(struct isil_audio_de *tde,  struct file *f, size_t count)
{

        while(query_space(f) < count){
                DEFINE_WAIT(wait);
                printk("litter space lefted in driver!!!!!!!!!!!1");
                up(&tde->sem);
                if(f->f_flags & O_NONBLOCK)
                        return -EAGAIN;
                printk("defore wait\n");
                prepare_to_wait(&tde->wait_poll, &wait, TASK_INTERRUPTIBLE);
                if(query_space(f) < count)
                        schedule();
                printk("finish wait for space!\n");
                finish_wait(&tde->wait_poll, &wait);
                if(signal_pending(current))
                        return -ERESTARTSYS;
                if(down_interruptible(&tde->sem))
                        return -ERESTARTSYS;
        }
        
        return 0;
}

ssize_t process_logic_frame(isil_audio_packet_queue_t *adpq, isil_audio_chan_buf_pool_t *adpool, unsigned long ulen, unsigned long tlen, 
        const char __user *buff,  size_t * offset,  loff_t *lof,unsigned long time_step, isil_audio_packet_section_t *pas )
{
    int ret = 0,remain = tlen; 
    static size_t last_remain;
    isil_audio_packet_section_t   *as = NULL;
    //unsigned long time_stamp = ph->timeStamp;
    unsigned long icount =0;
    //unsigned int  cs = 0;

    //dbg("ul[%d],tl[%d],off[%d]\n",ulen,tlen,*offset);
    while(remain > 0){
        icount++;
        if((as = adpq->curr_producer)){
            if(0/* pas->sample_rate != as->sample_rate || pas->bit_wide != as->bit_wide || pas->type != as->type*/){
                as->payload_len = (remain > ulen)? ulen:remain;
                if(as->payload_len < ulen) ISIL_DBG(ISIL_DBG_ERR,"one logic frame is too small!\n");
                as->sample_rate = pas->sample_rate;
                as->type = pas->type;
                as->bit_wide = pas->bit_wide;
                as->timestamp = pas->timestamp;
                watch_timestamp(pas->timestamp);				
                if(copy_from_user(as->data,&buff[*offset],as->payload_len) != 0){
                    ISIL_DBG(ISIL_DBG_ERR,"%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(as->payload_len < ulen)
                    goto deal_tail;
                adpq->op->put_curr_producer_into_queue(adpq);			
                *offset += as->payload_len;
                remain -= as->payload_len;
            }
            else
            {
                if(copy_from_user(&as->data[last_remain],&buff[*offset],ulen -last_remain) != 0){
                    ISIL_DBG(ISIL_DBG_ERR,"%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                as->payload_len = ulen;			
                adpq->op->put_curr_producer_into_queue(adpq);
                *offset += ulen -last_remain;
                remain -= ulen -last_remain;				
            }	
        }
        else
        {
            adpq->op->try_get_curr_producer_from_pool(adpq,adpool);
            as = adpq->curr_producer;
            if(as){
                as->payload_len = (remain > ulen)? ulen:remain;
                as->sample_rate = pas->sample_rate;
                as->type = pas->type;
                as->bit_wide = pas->bit_wide;
                as->timestamp = pas->timestamp;////time_stamp + idx*time_step;
                //as->consumer_offset = pas->consumer_offset;//////////////////////////////////*****************************
				
                watch_timestamp(pas->timestamp);
                //ISIL_DBG(ISIL_DBG_ERR,"as pl[%d],sr[%d],t[%d],bw[%d],of[%d]\n", as->payload_len, as->sample_rate, as->type, as->bit_wide, *offset);
                if(copy_from_user(as->data,&buff[*offset],as->payload_len) != 0){
                    ISIL_DBG(ISIL_DBG_ERR,"%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                /*
                                cs = compute_crc(as->data, as->payload_len);
                                if(cs != pas->consumer_offset){
                                        ISIL_DBG(ISIL_DBG_ERR,"checksum is not same$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ O[%x], N[%x],as[%p]\n", pas->consumer_offset, cs, as);												
                                }
		*/		
				
                if(as->payload_len < ulen){
                    goto deal_tail;
                }
                adpq->op->put_curr_producer_into_queue(adpq);

                *offset += as->payload_len;
                remain -= as->payload_len;				
            }
            else
            {
                ISIL_DBG(ISIL_DBG_ERR,"can not get section!!!!!!!!!!!!!!!!!!!!!!!!\n");
                ret = -ENOMEM;
                goto failed;				
            }					
        }
    }
deal_tail:
    if(remain)
        last_remain = as->payload_len;
    else
        last_remain = 0;
    *lof += *offset; 
    ret = tlen-remain;
    return ret;
failed:
    return ret;
}


int get_audio_frame_param(const char __user *buff, isil_audio_packet_section_t *pas, size_t *off, unsigned long *frame_len)
{
        isil_common_stream_header_t   *picsh = (isil_common_stream_header_t *)buff;
        isil_transparent_msg_header_t *pitmh;// = (isil_transparent_msg_header_t *)&buff[sizeof(isil_common_stream_header_t)];
        isil_audio_param_msg_t        *piapm;
        isil_frame_msg_t              *pifm;
        unsigned int i = 0, n;
        int *pint;
/*        
        char *p;
        int ix;

        p = (char *)&buff[*off];
        printk("\n\n");
        for(ix = 0; ix < 64; ix++, p++){
                if(ix % 16 == 0)
                        printk("\n[%8p]: ",p);
                printk("<%02x>,",*p);
        }
        printk("\n\n");
*/


        memset(pas, 0, sizeof(isil_audio_packet_section_t));
        if(picsh->stream_len & ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG){
                *off += sizeof(isil_common_stream_header_t);
                pitmh = (isil_transparent_msg_header_t *)&buff[sizeof(isil_common_stream_header_t)];
                n = pitmh->isil_transparent_msg_number;
                *off += sizeof(isil_transparent_msg_header_t);
                while(i < n){
                        pint = (int *)&buff[*off];
                        switch(*pint){
                                case ISIL_AUDIO_ENCODE_PARAM_MSG:
                                        piapm = (isil_audio_param_msg_t *)&buff[*off];
                                        pas->type = piapm->audio_type;
                                        pas->sample_rate = piapm->audio_sample_rate;
                                        pas->bit_wide = piapm->audio_bit_wide;
                                        *off += sizeof(isil_audio_param_msg_t);
                                        break;
                                case ISIL_AUDIO_ENCODE_FRAME_MSG:
                                        pifm = (isil_frame_msg_t *)&buff[*off];
                                        pas->timestamp = pifm->frame_timestamp;
                                        pas->payload_len = pifm->msg_len - sizeof(isil_frame_msg_t);
                                        pas->frameSerial = pifm->frame_serial;
                                        *off += sizeof(isil_frame_msg_t);
                                        /*
                                        cs = compute_crc((unsigned char *)&buff[*off + 6], pas->payload_len - 6);
                                        if(cs != pifm->frame_checksam){
                                                ISIL_DBG(ISIL_DBG_ERR,"checksum is not same$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");  
                                        }
					pas->consumer_offset = 0;
                                        pas->consumer_offset = cs;//////////////////////////////////
                                        */
                                        break;
                                default:
                                        ISIL_DBG(ISIL_DBG_ERR,"unsupported msg type right now![%d]\n",buff[*off]);
                                        goto fault;
                        }
                        i++;
                }
                *frame_len = pas->payload_len;
        }
        else{
                off += sizeof(isil_common_stream_header_t);
                *frame_len = picsh->stream_len - sizeof(isil_common_stream_header_t);
        }
        return 0;
fault:
        return -1;

}

ssize_t audio_de_write (struct file *f, const char __user *buff, size_t count, loff_t *lof)
{
    int ret = 0;//,idx = 0;
    unsigned long  total_paylen;
    struct isil_audio_de *tde = (struct isil_audio_de *)f->private_data;
    struct isil_audio_chan_driver *driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    isil_audio_packet_queue_t     *adpq = &driv->audio_packet_queue;
    isil_audio_chan_buf_pool_t    *adpool = &driv->audio_buf_pool;
    isil_audio_packet_section_t   as;
    size_t   offset = 0;
    unsigned long gap = 0,pad_nr = 0;
    unsigned long unit_paylen = 384; 
    unsigned long time_delta = 20;//= calc_time_stamp_deta(pad,unit_paylen);
    int v = 0;

    down(&tde->sem);
    v = get_space_onnode(tde, f, count);
    if(v)
        goto out;
    

    memset((char *)&as, 0, sizeof(isil_audio_packet_section_t));
    while(1)
    {
        total_paylen = 0;
        if(get_audio_frame_param(buff, &as, &offset, &total_paylen))
                return -EINVAL;

#ifdef ISIL_CONFIG_ADD_NAL
        total_paylen = total_paylen - sizeof(ext_h264_nal_bitstream_t);
        offset += sizeof(ext_h264_nal_bitstream_t);
#else
        total_paylen = total_paylen;
#endif
        //ISIL_DBG(ISIL_DBG_ERR,"b PLF off[%d],tl[%d],ul[%d]\n",offset,total_paylen,unit_paylen);  
        ret = process_logic_frame(adpq,adpool,unit_paylen,total_paylen,buff,&offset,lof,time_delta,&as);
        //ISIL_DBG(ISIL_DBG_ERR,"process logic frame returns %d\n",ret);
        if(ret < 0){
            up(&tde->sem);
            return ret;
        }
        gap = total_paylen-ret;
        if(offset >= count-3)
            break;
        pad_nr = (offset + gap) & 0x3;
        //ISIL_DBG(ISIL_DBG_ERR,"ret[%d],gap[%d], offset[%d], pad[%d]\n", ret, gap, offset,pad_nr);
        if(gap < unit_paylen){
            if(offset + gap + pad_nr + ISIL_TOTAL_AUDIO_FRAMEHEADER_LEN >= count)
                goto out;
            /*arrive here, means there are still more frames lefted*/
            offset += gap;
            offset += pad_nr;	
        }
        else
        {
            ISIL_DBG(ISIL_DBG_ERR,"data wrong format!!!!!!!!!!!!!!!!\n");
            up(&tde->sem);
            return -EINVAL;
        }
    }
out:
    up(&tde->sem);
    //printk("o<-tk=%d\n",get_isil_system_tick());
    return (ssize_t)offset;
}

#endif











int audio_de_ioctl (struct file *f, unsigned int cmd, unsigned long arg)
{	
    int ret = 0;
    struct isil_audio_de *tde = (struct isil_audio_de *)f->private_data;
    struct endpoint_tcb *ed;
    //tc_trace;

    down(&tde->sem);
    switch(cmd)
    {
        /*transfor to low level driver*/
        default:
            //dbg("other cmd in %s\n",__FUNCTION__);
            if(!tde->tsd.ped){
                printk("the device has no driver ed!\n");
                ret = EFAULT;
                break;
            }
            ed = tde->tsd.ped;
            ret = ed->op->ioctl(ed,cmd,arg);	
            break;
    }
    up(&tde->sem);
    return ret;
}



int  audio_de_release(struct file *f)
{
    int ret;
    ed_tcb_t *aded;
    struct isil_audio_de *tde = (struct isil_audio_de *)f->private_data;
    //struct isil_audio_chan_driver *driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, audio_ed);

    //tc_trace;
    down(&tde->sem);
    aded = tde->tsd.ped;
#ifdef CONFIG_SMP
    del_timer_sync(&tde->polling_timer);
#else
    del_timer(&tde->polling_timer);
#endif
    ret = aded->op->close(aded);
    tc_queue_reset(tde->tsd.queue);
    //set_tde_state(tde,CLOSED);	
    up(&tde->sem);
    f->private_data = NULL;
    return ret;
}



unsigned int audio_de_poll(struct file *file, struct poll_table_struct *wait)
{
	unsigned int mask = 0, nr = 0;
	struct isil_audio_de *tde;// = (struct isil_audio_de *)file->private_data;
        struct isil_audio_chan_driver *driv;// = container_of(tde->tsd.ped,struct isil_audio_chan_driver, audio_ed);
        isil_audio_chan_buf_pool_t    *ap;// = &driv->audio_buf_pool;
	//isil_audio_packet_queue_t     *adpq = &driv->audio_packet_queue;
	
        if(!file)
                return 0;

        tde = (struct isil_audio_de *)file->private_data;
        driv = container_of(tde->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
        ap = &driv->audio_buf_pool;

	down(&tde->sem); 
	poll_wait(file,&tde->wait_poll,wait);
        nr = ap->op->get_audio_section_tcb_pool_entry_number(ap);
	//printk("in %s ,,, audio pool entry number[%d]!\n", __FUNCTION__, nr);
	if(nr > 10)
	{
		mask |= POLLOUT | POLLRDNORM;
	}
	else
	{
		//printk("has no entry number will hung up!\n");	
	}
	up(&tde->sem);	
	return mask; 
}



int audio_de_init(struct isil_chip_device *tcd,struct cmd_arg *arg,struct isil_service_device **tsd)
{
    //int ret;
    struct isil_audio_de *tde;

    tde = kzalloc(sizeof(*tde),GFP_KERNEL);
    if(NULL == tde)
        return -ENOMEM;
    *tsd = &tde->tsd;

    tde->tsd.clean_up = tde_cleanup;
    tde->tsd.queue = kzalloc(sizeof(struct tc_queue),GFP_KERNEL);
    if(!tde->tsd.queue)
        return -ENOMEM;
    tc_queue_init(tde->tsd.queue,&de_queue_ops,NULL,512,10,tde);
    tde->name = "tw5864-audio_decoder";
    tde->users = 1;
    //tde->state = NEED_INIT;
    tc_set_audio_param(&tde->para,&def_audio_parm_de);
    //init_MUTEX(&tde->sem);
    sema_init(&tde->sem, 1);
    init_waitqueue_head(&tde->wait_poll);
    return 0;
}

void audio_de_destory(struct isil_service_device *tsd)
{
    struct isil_audio_de *tde;

    if(!tsd)
        return;

    tde = container_of(tsd,struct isil_audio_de,tsd);
    if(tde->tsd.queue)
        kfree(tde->tsd.queue);
    kfree(tde);
}


struct tsd_file_operations    audio_de_fops = {
    .owner  = THIS_MODULE,
    .init   = audio_de_init,
    .destory= audio_de_destory,
    .open	= audio_de_open,
    .read	= audio_de_read,
    .write	= audio_de_write,
    .ioctl  = audio_de_ioctl,
    .release= audio_de_release,
    .poll = audio_de_poll,
};

struct isil_dev_id  isil_audio_de_id = {
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (STREAM_TYPE_AUDIO << 16) | CODEC_AUDIO_ADPCM,
        .type = ISIL_DECODER,
        .key = {0},
    },     
    .version = 0,
};

static struct isil_dev_table_id *tde_table = NULL;

int isil_audio_de_init(void)
{
    //int ret = 0;
    tde_table = kzalloc(sizeof(struct isil_dev_table_id),GFP_KERNEL);
    if(!tde_table){
        dbg("no mem in %s\n",__FUNCTION__);
        return -ENOMEM;	
    }
    INIT_LIST_HEAD(&tde_table->list);
    tde_table->tid = &isil_audio_de_id;
    tde_table->ops = &audio_de_fops;
    tde_table->para = NULL;//data;
    register_tc_device(tde_table);
    //printk("decoder <0x%8p>\n",data);
    return 0;		
}

void isil_audio_de_remove(void)
{
    if(tde_table)
        unregister_tc_device(tde_table);
    tde_table = NULL;
}

















