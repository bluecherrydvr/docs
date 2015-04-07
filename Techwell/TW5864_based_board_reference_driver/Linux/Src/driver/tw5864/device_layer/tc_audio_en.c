#include <isil5864/tc_common.h>

struct isil_chip_audio_param_my  def_audio_parm_en = {
    .i_bit_wide = ISIL_AUDIO_16BIT,
    .i_sample_rate = ISIL_AUDIO_8K,
    .e_audio_type = ISIL_AUDIO_ADPCM_32K,
};


int if_exist(unsigned long *base, unsigned long len, unsigned long key)
{
    unsigned long *p = base;

    while(p != base+len){
        if(key == *p++)
            return 1;
    }
    return 0;
}


/*actully here we didn't discard the driver submited frame header, but user can get net payload data just by start,size*/
int  ae_buf_prepare(struct tc_queue * tq, void *priv, struct tc_buffer __user *tc)
{
    struct tc_buffer_kern *tck,*tmp;
    struct isil_audio_en * tae = (struct isil_audio_en *)priv;
    isil_audio_packet_queue_t     *adpq;
    isil_audio_packet_section_t   *as;
    isil_audio_chan_buf_pool_t    *adpool;
    unsigned long nr_ready,nr_header,buffer_need = PAGE_SIZE,us = tc->__tc__size;
	struct tc_header_buf *thb = NULL;
    int idx,la = 0;
    unsigned long offset = 0,ns = 0;
    struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);

    adpq = &driv->audio_packet_queue;
    adpool = &driv->audio_buf_pool;

    //tc_trace;
    tmp = tq->bufs[0];
    if((STATE_READY == get_tck_state(tmp)) && (0 != tmp->last_prepare)){
        /*last gathered data were not copy by user*/
        tc->__tc__nr_frame = 1;
        if(tmp->last_prepare > us){
            tc->__tc__size = tmp->last_prepare;
            return -ENOMEM;                        
        }
        tc->__tc__size = tmp->last_prepare;
        tmp->last_prepare = 0;
    }
    else
    {
		thb_pool->op->get_header_buf(thb_pool,&thb);
        if(!thb){
            dbg("get header_buffer nothing in %s!\n",__FUNCTION__);
            return -EAGAIN;         
        }
        nr_header = tc_round_low(thb->unit_size,MIN_HEADER_LEN_AUDIO);
retry:
		nr_ready = adpq->op->get_curr_queue_entry_number(adpq);
		//printk("header pool number [%d] in %s\n",thb_pool->op->get_hb_pool_entry_number(thb_pool),__FUNCTION__);
        nr_ready = tc_min(nr_ready,nr_header);
        nr_ready = tc_min(nr_ready,tq->buf_nr);
        tc->__tc__nr_frame = 0;
        for(idx = 0; idx < nr_ready; idx++){
            as = NULL;
            if(!adpq->curr_consumer){
                adpq->op->try_get_curr_consumer_from_queue(adpq);
                if(!adpq->curr_consumer){
                    dbg("get audio packet nothing! in %s!\n",__FUNCTION__);
                    tc->__tc__size = buffer_need;
                    if(PAGE_SIZE ==tc->__tc__size)
                    {
                        thb_pool->op->put_header_buf(thb_pool,thb);
                        return -EAGAIN;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
            if(tae->discard){
                adpq->op->release_curr_consumer(adpq,adpool);
                tae->discard = 0;
                goto retry;
            }
            adpq->curr_consumer->op->reference(adpq->curr_consumer,&as);
            adpq->op->release_curr_consumer(adpq,adpool);
            tck = tq->bufs[idx];
            tck->index = idx;
            tck->type = MEM_MMAP;
            tck->thb = thb;
            tck->holder = as; 
            tck->isalign = (((unsigned long)as->data) & 0xFFF)? 0: 1;
            tck->off_to_page = ((unsigned long)as->data) & 0xFFF;
            tck->start = (__u32 *)as->data;
            tck->size = as->payload_len;

            la = tck->off_to_page - la; 
            if(la < 0)
                ns = tc_align_up(tck->size,PAGE_SIZE);	
            else
                ns = tc_align_up(tck->size,as->section_size);

            if(buffer_need + ns > us){
                if(0 == idx){
                    tc->__tc__size = buffer_need + ns;
                    tc->__tc__nr_frame = 1;
                    set_tck_state(tck,STATE_READY);
                    tc_buffer_set_audio_data((struct tc_buffer *)(thb->data),tck,as,&offset);
                    tmp->last_prepare = tc->__tc__size;
                    //dbg("user buffer is too smaller %d!\n",tc->__tc__size);
                    return -ENOMEM;                                        
                }
                else{
                    /*return to adpq->curr_consumer*/
                    as->op->reference(as,&adpq->curr_consumer);
                    as->op->release(&as,adpool);
                    tck->holder = NULL;
                    //dbg("out from here,tc->__tc__size = %d\n",tc->__tc__size);
                    return 0;
                }
            }
            la = tck->off_to_page;
            buffer_need += ns;
            tc->__tc__size = buffer_need;
            set_tck_state(tck,STATE_READY);
            tc_buffer_set_audio_data((struct tc_buffer *)(thb->data),tck,as,&offset);
            tc->__tc__nr_frame++;
        }
    }
    return 0;	
}

int  ae_buf_release(struct tc_queue * tq, void * priv, struct tc_buffer_kern *tb)
{
    int ret = 0,nr = 0;
    struct isil_audio_en * tae = (struct isil_audio_en *)priv;
    isil_audio_packet_section_t   *as = NULL;
    struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    isil_audio_chan_buf_pool_t    *adpool = &driv->audio_buf_pool;			
    struct tc_buffer_kern *tck = tb;

    //tc_trace;
    if(!adpool){
        dbg("chan_buf_pool is NULL!\n");
        return -EINVAL;
    }

    if(!tck->thb || !tck->holder){
        dbg("invalid parameters in tck! -->%s\n",__FUNCTION__);
        return -EINVAL;
    }
    thb_pool->op->put_header_buf(thb_pool,tck->thb);

    while((tck = tq->bufs[nr]) && (STATE_BUSY == get_tck_state(tck))){
        as = (isil_audio_packet_section_t *)tck->holder;
        as->op->release(&as,adpool);
        set_tck_state(tck,STATE_FREE);             
        nr++;
        if(nr >= tq->buf_nr)
            break;
    }	
    //printk("abr->o  id[%d]\n",driv->audio_logic_chan_id);
    return ret;
}




struct tc_queue_ops    ae_queue_ops =
{
    .buf_prepare = ae_buf_prepare,
    .buf_release = ae_buf_release,	
};

int  request_memory(struct file *f,unsigned long arg)
{
    int ret = 0;
    struct tc_buffer *tc = (struct tc_buffer *)arg;
    struct isil_audio_en * tae = (struct isil_audio_en *)f->private_data;
    struct tc_queue *q = tae->tsd.queue;


    down(&tae->sem);
#ifdef STATIC_COUNTING
    //get_start((struct isil_service_device *)&tae->tsd);
#endif
    /*
       while(queue_full(q)){
       if(f->f_flags & O_NONBLOCK){
       ret = -EAGAIN;	
       goto out_direct;
       }
       dbg("we are here!\n");
       wait_event_interruptible(q->done, !queue_full(q));
       dbg("queue is full, we still need wait!!!!...\n");		
       }
       */
    if(!q->ops){
        dbg("tq has no ops!\n");
        ret = -EINVAL;
        goto out;
    }
    if(!q->ops->buf_prepare){
        dbg("tq->ops has no buf_prepare!\n");
        ret = -EINVAL;
        goto out;               
    }
    ret = q->ops->buf_prepare(q,tae,tc);
out:
    up(&tae->sem);
    return ret;
}

/*
   int  release_memory(struct file *f,unsigned long arg)
   {
   int ret = 0;	
   struct isil_audio_en * tae = (struct isil_audio_en *)f->private_data;
   struct tc_queue *tq = tae->tsd.queue;
   struct tc_buffer_kern * tck;	

   down(&tae->sem);
   if(queue_empty(tq)){
   dbg("queue is empty");
   ret = -EINVAL;
   goto out;
   }

   mutex_lock(&tq->lock);
   tck = tq->bufs[tq->rp];
   if(tq->ops->buf_release)
   tq->ops->buf_release(tq,tae,tck);
   queue_inc_rp(tq);
   set_tck_state(tck,STATE_FREE);
   wake_up_interruptible(&tq->done);
   mutex_unlock(&tq->lock);	
out:
up(&tae->sem);
return 0;
}
*/


int  release_memory(struct file *f,unsigned long arg)
{
    int ret = 0;	
    struct isil_audio_en * tae = (struct isil_audio_en *)f->private_data;
    struct tc_queue *tq = tae->tsd.queue;
    struct tc_buffer_kern * tck;	

    down(&tae->sem);
#ifdef STATIC_COUNTING
    //get_end((struct isil_service_device *)&tae->tsd);
#endif

    tck = tq->bufs[0];
    if(!tq->ops || !tq->ops->buf_release){
        dbg("no ops or no buf_release!\n");
        ret = -EINVAL;
        goto out;
    }
    ret = tq->ops->buf_release(tq,tae,tck);  
out:
    up(&tae->sem);
    return 0;
}






/*do actually release,  release everything it has*/
void tae_cleanup(void * arg)
{
    struct isil_audio_en *tae = (struct isil_audio_en *)arg;

    //tc_trace;
    tc_queue_release(tae->tsd.queue);
    kfree(tae);	
}


isil_audio_driver_t * lookup_drive_instance(struct isil_service_device *tsd)
{
    isil_audio_driver_t *p = tsd->private_data;
    unsigned long ld = get_phchan_id_from_epobj(&tsd->epobj);
    printk("tsd->minor = %d, ld = %lu encoder add = %8p\n",tsd->minor,ld,&p[ld]);

    return &p[ld];

}


static void audi_en_polling(unsigned long p)
{
    struct isil_audio_en *tae = (struct isil_audio_en *)p;
    isil_audio_packet_queue_t     *adpq;	
    unsigned long nr;
    struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);

    flush_audio_encode_curr_consumer_frame(driv);
    adpq = &driv->audio_packet_queue;	
    nr = adpq->op->get_curr_queue_entry_number(adpq);
    if(nr)
    {
        wake_up(&tae->wait_poll);
    }
    else
    {
        //printk("in %s the entry number = %d\n",__FUNCTION__,nr);	
    }
    mod_timer(&tae->polling_timer, jiffies + 40);
    return;
}




int audio_en_open(struct isil_service_device *tsd,struct file *f)
{
    int ret = 0;
    ed_tcb_t   *aed; 
    struct isil_audio_en *tae = container_of(tsd,struct isil_audio_en,tsd);
    //struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, audio_ed);
    //tc_trace;

    if(!tae->tsd.ped){
        dbg("audio dev has no driver attached!\n");
        ret = -1;
        return ret;
    }
	aed = tae->tsd.ped;//&driv->opened_logic_chan_ed;
    ret = aed->op->open(aed);
    if(ret != ISIL_OK){
        printk("in %s audio_driver open failed!\n",__FUNCTION__);
        ret = -1;
        return ret;
    }
    f->private_data = tae;

    tae->polling_timer.expires = jiffies + 40;
    tae->polling_timer.data = (unsigned long)tae;
    tae->polling_timer.function = audi_en_polling;
    add_timer(&tae->polling_timer);
    return ret;
}

#ifndef  NEW_HEADER

ssize_t audio_en_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
    int offset = 0;
    struct isil_audio_en *tae = (struct isil_audio_en *)f->private_data;
    struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    unsigned long paylen = get_payload_len(&tae->para);
    unsigned long queue_entry = 0;
    size_t cn = count,min =  paylen + sizeof(isil_frame_header_t) + sizeof(isil_audio_frame_pad_t);
    char __user *p = buff;
    isil_audio_packet_queue_t     *adpq = &driv->audio_packet_queue;
    isil_audio_chan_buf_pool_t    *adpool = &driv->audio_buf_pool;
    isil_frame_header_t	    *frame_head = NULL;
    isil_audio_packet_section_t   *as = NULL;
    loff_t  frame_offset;
    isil_audio_packet_section_t   *another = NULL;

    down(&tae->sem);
    //dbg("cn = %d,min = %d\n", cn,min);
    while(cn >= min){
        flush_audio_encode_curr_consumer_frame(driv);
        //dbg("encoder add = <%8p>, queue_entry number = %lu\n",driv,queue_entry);
        queue_entry = adpq->op->get_curr_queue_entry_number(adpq);
        adpq->op->try_get_curr_consumer_from_queue(adpq);
        as = adpq->curr_consumer;
        if(!as){
            //if(f->f_flags & O_NONBLOCK)
            //printk("no consumer!\n");
            goto out;
        }
        else{
            /*here we already get a section, so we can directly put to user*/
            as->op->reference(as,&another);
            adpq->op->release_curr_consumer(adpq,adpool);
            if(another){

                //printk("offset = %d\n",offset);
                frame_offset = 0;
                another->op->submit(another,&buff[offset],cn,&frame_offset,ISIL_MASTER_BITSTREAM,adpool);
                another->op->release(&another,adpool);
                //adpq->op->release_curr_consumer(adpq,adpool);
                frame_head = (isil_frame_header_t *)&buff[offset];
                offset += frame_offset;
                /*
                p = (char *)&buff[offset];
                while((unsigned long)p & 0x3){
                    //printk("doing padding!\n");
                    __put_user(0, p++);
                    frame_head->payloadLen++;
                    frame_offset += 1;
                    offset++;
                }
                */
                //printk("*******before cn = %d, frame_offset = %d\n",cn,frame_offset);
                if(cn >= frame_offset)
                    cn = cn -frame_offset;
                else
                    goto out;
            }
        }
    }
out:
    up(&tae->sem);
    *lof = offset; 
    //dbg("read %d bytes\n",offset);
    return (ssize_t)offset;
}

#else

ssize_t audio_en_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
    int offset = 0;
    struct isil_audio_en *tae = (struct isil_audio_en *)f->private_data;
    struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    unsigned long paylen = get_payload_len(&tae->para);
    unsigned long queue_entry = 0;
    size_t cn = count,min =  paylen + sizeof(struct isil_common_stream_header) + sizeof(struct isil_transparent_msg_header) + sizeof(struct isil_audio_param_msg);
    char __user *p = buff;
    isil_audio_packet_queue_t     *adpq = &driv->audio_packet_queue;
    isil_audio_chan_buf_pool_t    *adpool = &driv->audio_buf_pool;
    //isil_frame_header_t	    *frame_head = NULL;
    isil_audio_packet_section_t   *as = NULL;
    loff_t  frame_offset;
    //isil_audio_packet_section_t   *another = NULL;// why we use another for reference??

    down(&tae->sem);
    //dbg("cn = %d,min = %d\n", cn,min);
    while(cn >= min){
        flush_audio_encode_curr_consumer_frame(driv);
        queue_entry = adpq->op->get_curr_queue_entry_number(adpq);
        adpq->op->try_get_curr_consumer_from_queue(adpq);
        as = adpq->curr_consumer;
        if(!as){
            //if(f->f_flags & O_NONBLOCK)
            //printk("no consumer!\n");
            goto out;
        }
        else{
            /*here we already get a section, so we can directly put to user*/
                frame_offset = 0;
                //ISIL_DBG(ISIL_DBG_ERR,"in %d frame--as [%p]\n", __LINE__, as);
                as->op->submit(as,&buff[offset],cn,&frame_offset,ISIL_MASTER_BITSTREAM,adpool);
                adpq->op->release_curr_consumer(adpq, adpool);
                
                offset += frame_offset;
                goto out;//just for one frame
                p = (char *)&buff[offset];
                while((unsigned long)p & 0x3){
                    __put_user(0, p++);
                    frame_offset += 1;
                    offset++;
                }
                //printk("*******before cn = %d, frame_offset = %d\n",cn,frame_offset);
                if(cn >= frame_offset)
                    cn = cn -frame_offset;
                else
                    goto out;
        }
    }
out:
    up(&tae->sem);
    *lof = offset; 
    //dbg("read %d bytes\n",offset);
    return (ssize_t)offset;
}

#endif
/*this is just for test...*/
ssize_t audio_en_write (struct file *f, const char __user *buff, size_t count, loff_t *lof)
{

    int ret = 0,idx = 0,remain = count;
    unsigned long nr = 0,net_paylen;
    struct isil_audio_en *tae = (struct isil_audio_en *)f->private_data;
    struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    isil_audio_packet_queue_t     *adpq;
    isil_audio_chan_buf_pool_t    *adpool;	
    isil_audio_packet_section_t   *as;

    adpq = &driv->audio_packet_queue;
    adpool = &driv->audio_buf_pool;
    down(&tae->sem);

    net_paylen = get_payload_len(&tae->para);

    nr = tc_round_up(count,net_paylen);

    for(; idx < nr; idx++,remain = count -(idx >> 9)){
        adpq->op->get_curr_producer_from_pool(adpq,adpool);
        as = adpq->curr_producer;
        if(as){
            as->payload_len = (remain > 512)? 512:remain;
            /*here we already get a section, so we can directly copy user data to buffer 'data'*/
            if(copy_from_user(as->data, buff,count)){
                dbg("copy from user failed!\n");
                ret = -EFAULT;
                goto failed_out;
            }

            adpq->op->put_curr_producer_into_queue(adpq);
        }
        else{
            dbg("can not get section!\n");
            goto out;
        }
    }
out:
    ret = count-remain;
failed_out:
    up(&tae->sem);
    return ret;

}

int audio_en_ioctl (struct file *f, unsigned int cmd, unsigned long arg)
{	
    int ret = 0;
    struct isil_audio_en *tae = (struct isil_audio_en *)f->private_data;
    struct endpoint_tcb *ed;
    struct tc_queue *tq = tae->tsd.queue;
    struct tc_buffer_kern * tck;	
    //tc_trace;

    down(&tae->sem);
    switch(cmd)
    {

        case ISIL_CODEC_CHAN_REQUEST_MEM:
            ret = request_memory(f,arg);
            break;
        case ISIL_CODEC_CHAN_RELEASE_MEM:
            ret = release_memory(f,arg);
            break;
        case ISIL_CODEC_CHAN_DISCAED_FRAME:
            tae->discard = 1;
            set_tq_state_busy(tq);
            tck = tq->bufs[0];
            ret = tq->ops->buf_release(tq,tae,tck);
            if(ret)
                printk("discard frame ---release failed! in %s\n",__FUNCTION__);
            break;
        default:
            //dbg("other cmd in %s\n",__FUNCTION__);
            if(!tae->tsd.ped){
                printk("the device has no driver ed!\n");
                ret = EFAULT;
                break;
            }
            ed = tae->tsd.ped;
            ret = ed->op->ioctl(ed,cmd,arg);	
            break;
    }
    up(&tae->sem);
    return ret;
}


/*user close(fd) will call this function, it just perform close*/
int  audio_en_release(struct file *f)
{
    int ret;
    ed_tcb_t   *aed;
    struct isil_audio_en *tae = (struct isil_audio_en *)f->private_data;
    struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);

    //tc_trace;
    down(&tae->sem);
#ifdef CONFIG_SMP
    del_timer_sync(&tae->polling_timer);
#else
    del_timer(&tae->polling_timer);
#endif

    aed = &driv->opened_logic_chan_ed;
    ret = aed->op->close(aed);
    tc_queue_reset(tae->tsd.queue);
    up(&tae->sem);

    f->private_data = NULL;
    return ret;
}



unsigned int audio_en_poll(struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    struct isil_audio_en *tae = (struct isil_audio_en *)file->private_data;
    struct isil_audio_chan_driver *driv = container_of(tae->tsd.ped,struct isil_audio_chan_driver, opened_logic_chan_ed);
    isil_audio_packet_queue_t     *adpq = &driv->audio_packet_queue;	

    down(&tae->sem);
    poll_wait(file,&tae->wait_poll,wait);
    if(adpq->op->get_curr_queue_entry_number(adpq))
    {
        mask |= POLLIN | POLLRDNORM;
        //dbg("%s frame availble!\n",__FUNCTION__);
    }
    else
    {
        //printk("has no entry number will hung up!\n");	
    }
    up(&tae->sem);	
    return mask; 
}




int audio_en_init(struct isil_chip_device *tcd,struct cmd_arg *arg,struct isil_service_device **tsd)
{
    struct isil_audio_en *tae;

    //tc_trace;
    tae = kzalloc(sizeof(*tae),GFP_KERNEL);
    if(NULL == tae)
        return -ENOMEM;
    *tsd = &tae->tsd;

    tae->tsd.clean_up = tae_cleanup;
    tae->tsd.queue = kzalloc(sizeof(struct tc_queue),GFP_KERNEL);
    if(NULL == tae->tsd.queue)
        return -ENOMEM;

    tc_queue_init(tae->tsd.queue,&ae_queue_ops,NULL,512,10,tae);
    tae->name = "tw5864-audio_encoder";
    tae->users = 1;
    //tae->state = NEED_INIT;
    tc_set_audio_param(&tae->para,&def_audio_parm_en);
    //init_MUTEX(&tae->sem);
    sema_init(&tae->sem, 1);
	init_timer(&tae->polling_timer);
    init_waitqueue_head(&tae->wait_poll);
    return 0;
}

void audio_en_destory(struct isil_service_device *tsd)
{
    struct isil_audio_en *tae;
    //tc_trace;

    if(!tsd)
        return;
    tae = container_of(tsd,struct isil_audio_en,tsd);
    if(tae->tsd.queue)
        kfree(tae->tsd.queue);
    kfree(tae);
}


struct tsd_file_operations    audio_en_fops = {
    .owner  = THIS_MODULE,
    .init   = audio_en_init,
    .destory= audio_en_destory,
    .open	= audio_en_open,
    .read	= audio_en_read,
    .write	= audio_en_write,
    .ioctl  = audio_en_ioctl,
    .release= audio_en_release,
    .poll = audio_en_poll,
};

struct isil_dev_id  isil_audio_en_id = {
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (STREAM_TYPE_AUDIO << 16) | CODEC_AUDIO_ADPCM,
        .type = ISIL_ENCODER,
        .key = {0},
    },
    .version = 0,
};

static struct isil_dev_table_id *tae_table = NULL;

int isil_audio_en_init(void)
{
    tae_table = kzalloc(sizeof(struct isil_dev_table_id),GFP_KERNEL);
    if(!tae_table){
        dbg("no mem in %s\n",__FUNCTION__);
        return -ENOMEM;	
    }
    INIT_LIST_HEAD(&tae_table->list);
    tae_table->tid = &isil_audio_en_id;
    tae_table->ops = &audio_en_fops;
    tae_table->para = NULL;
    //dbg_epobj_print(&tae_table->tid->epobj);
    register_tc_device(tae_table);
    //printk("encoder <0x%8p>\n",data);
    return 0;	
}

void isil_audio_en_remove(void)
{
    if(tae_table)
        unregister_tc_device(tae_table);
    tae_table = NULL;
}











