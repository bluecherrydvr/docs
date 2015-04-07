#include <isil5864/tc_common.h>


void print_jpeg_frame_header(char * pt)
{
    unsigned long idx =0;
    for(;idx < 7; idx++) 
        dbg("<0x%02X>",*pt++);
    dbg("\n");
}


int jpeg_gather_scatter_info(isil_vb_frame_tcb_t * vjf,struct tc_buffer_kern *tck)
{
    isil_vb_packet_tcb_queue_t *vpq = &vjf->vb_packet_queue;
    isil_vb_packet_tcb_t  *vp = NULL;
    int idx = 0,ret = 0;

    while(vpq->op->get_curr_queue_entry_number(vpq)){
        vpq->op->get_curr_consumer_from_queue(vpq);
        vp = vpq->curr_consumer;
        if(vp){
            vp->op->reference(vp,(isil_vb_packet_tcb_t **)&tck->sg[idx].carrier);
            tck->sg[idx].addr = (__u32 *)vp->data;
            tck->sg[idx].len = vp->payload_len;
            ret += vp->payload_len;
            //if(0 == idx)
            //        print_jpeg_frame_header((char *)vp->data);
            vpq->op->release_curr_consumer(vpq,vjf->pool);
            idx++;
            if(idx >= SCATTRT_MAX_ENTRY)
                break;
        }
        else 
        {
            dbg("can not gather all packages for one frame!\n");
            break;
        }
    }
    return ret;
}

/*actully here we didn't discard the driver submited frame header, but user can get net payload data just by start,size*/
int  vje_buf_prepare(struct tc_queue * tq, void *priv, struct tc_buffer __user *tc)
{
    struct tc_buffer_kern *tck,*tmp;
    struct isil_jpeg_en * tje = (struct isil_jpeg_en *)priv;
    struct isil_jpeg_logic_encode_chan *driv = container_of(tje->tsd.ped,struct isil_jpeg_logic_encode_chan, opened_logic_chan_ed);
    isil_vb_frame_tcb_queue_t *vfq = &driv->jpeg_frame_queue;
    isil_vb_frame_tcb_t *vjf = NULL;
    int buffer_need = PAGE_SIZE;
    unsigned long	nr_ready,nr_header,idx,us = tc->__tc__size;
    struct tc_header_buf  *thb = NULL;
    unsigned long off = 0;
    unsigned long nosense;       

    //tc_trace;
    tmp = tq->bufs[0];
    if((STATE_READY == get_tck_state(tmp)) && (0 != tmp->last_prepare)){
        tc->__tc__nr_frame = 1;
        /*last gathered data were not copy by user*/
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
        nr_header = tc_round_low(thb->unit_size,MIN_HEADER_LEN_JPEG);

retry:
		nr_ready = vfq->op->get_curr_queue_entry_number(vfq);
		//printk("header pool number [%d] in %s\n",thb_pool->op->get_hb_pool_entry_number(thb_pool),__FUNCTION__);
        nr_ready = tc_min(nr_ready,nr_header);
        nr_ready = tc_min(nr_ready, tq->buf_nr);
        tc->__tc__nr_frame = 0;
        for(idx = 0; idx < nr_ready; idx++){
            vjf = NULL;
            if(!vfq->curr_consumer){
                vfq->op->try_get_curr_consumer_from_queue(vfq);
                if(!vfq->curr_consumer){
                    dbg("consumer is NULL %s;\n",__FUNCTION__);
                    tc->__tc__size = buffer_need;  // tell user how many data are ready
                    if(PAGE_SIZE != tc->__tc__size){
                        return 0;
                    }
                    else{
                        thb_pool->op->put_header_buf(thb_pool,thb);
                        return -EAGAIN;
                    }

                }
            }
            if(tje->discard){
                vfq->op->release_curr_consumer(vfq,&driv->pool);
                thb_pool->op->put_header_buf(thb_pool,thb);
                tje->discard = 0;
                goto retry;
            }

            vfq->curr_consumer->op->reference(vfq->curr_consumer,&vjf);
            vfq->op->release_curr_consumer(vfq,&driv->pool);

            tck = tq->bufs[idx];
            //dbg("-%d-%d\n",vjf->frame_number,vjf->frame_len);
            tck->index = idx;
            tck->type = MEM_MMAP;
            tck->thb = thb;
            tck->holder = vjf;
            tck->size = vjf->frame_len;//jpeg_gather_scatter_info(vjf,tck);

            if((buffer_need + tc_align_up(tck->size,PAGE_SIZE)) > us){
                if(0 == idx){
                    tc->__tc__size = buffer_need + tc_align_up(tck->size,PAGE_SIZE);
                    tc->__tc__nr_frame = 1;
                    set_tck_state(tck,STATE_READY);
                    nosense = jpeg_gather_scatter_info(vjf,tck);   
                    tc_buffer_set_jpeg_data((struct tc_buffer *)(thb->data),tck,vjf,&off);
                    tmp->last_prepare = tc->__tc__size;
                    dbg("user buffer is too smaller!\n");
                    return -ENOMEM;
                }
                else{
                    vjf->op->reference(vjf,&vfq->curr_consumer);
                    vjf->op->release(&vjf,&driv->pool);
                    tck->holder = NULL;
                    return 0;
                }
            }
            buffer_need += tc_align_up(tck->size,PAGE_SIZE);
            tc->__tc__size = buffer_need;
            set_tck_state(tck,STATE_READY);
            nosense = jpeg_gather_scatter_info(vjf,tck);   
            tc_buffer_set_jpeg_data((struct tc_buffer *)(thb->data),tck,vjf,&off);
            tc->__tc__nr_frame++;
        }
    }
    return 0;
}

int  vje_buf_release(struct tc_queue * tq, void * priv, struct tc_buffer_kern *tb)
{
    int ret = 0,idx,cn = 0;
    struct isil_jpeg_en * tje = (struct isil_jpeg_en *)priv;
    struct isil_jpeg_logic_encode_chan *driv = container_of(tje->tsd.ped,struct isil_jpeg_logic_encode_chan, opened_logic_chan_ed);
    isil_vb_pool_t    *vjpool = &driv->pool;			
    isil_vb_packet_tcb_t  *vp = NULL;
    isil_vb_frame_tcb_t *vf = NULL; 
    struct tc_buffer_kern *tck = tq->bufs[0];


    /*release the header buffer page*/
    if(!tck->thb || !tck->holder){
        dbg("invalid parameters in tck! -->%s\n",__FUNCTION__);
        return -EINVAL;
    }
    thb_pool->op->put_header_buf(thb_pool,tck->thb);

    while((tck = tq->bufs[cn]) && (STATE_BUSY == get_tck_state(tck))){
        idx = 0;
        /*first release all packets in one frame*/
        while(tck->sg[idx].carrier && tck->sg[idx].addr && tck->sg[idx].len){
            vp = (isil_vb_packet_tcb_t *)tck->sg[idx].carrier;
            vp->op->release(&vp,vjpool);
            memset(&tck->sg[idx],0,sizeof(struct tc_scatter));
            idx++;
            if(idx >= SCATTRT_MAX_ENTRY)
                break;
        }
        /*then release frmae */
        vf = (isil_vb_frame_tcb_t *)tck->holder;
        vf->op->release(&vf,vjpool);
        set_tck_state(tck,STATE_FREE);             
        cn++;
        if(cn >= tq->buf_nr)
            break;
    }

    return ret;
}

struct tc_queue_ops    vje_queue_ops =
{
    .buf_prepare = vje_buf_prepare,
    .buf_release = vje_buf_release,	
};


int jpeg_en_request_memory(struct file *f,unsigned long arg)
{
    int ret = 0;
    struct tc_buffer *tc = (struct tc_buffer *)arg;
    struct isil_jpeg_en * tje = (struct isil_jpeg_en *)f->private_data;
    struct tc_queue *q = tje->tsd.queue;
    //unsigned long fa = 0;
    //tc_trace;

    down(&tje->sem);
#ifdef STATIC_COUNTING
    //get_start((struct isil_service_device *)&tje->tsd);
#endif
    if(!q->ops){
        dbg("tq has no ops!\n");
        ret = EINVAL;
        goto out;
    }
    if(!q->ops->buf_prepare){
        dbg("tq->ops has no buf_prepare!\n");
        ret = EINVAL;
        goto out;  
    }
    ret = q->ops->buf_prepare(q,tje,tc);
out:
    up(&tje->sem);
    return ret;
}


int  jpeg_en_release_memory(struct file *f,unsigned long arg)
{
    int ret = 0;	
    struct isil_jpeg_en * tje = (struct isil_jpeg_en *)f->private_data;
    struct tc_queue *tq = tje->tsd.queue;
    struct tc_buffer_kern * tck;	

    down(&tje->sem);
#ifdef STATIC_COUNTING
    //get_end((struct isil_service_device *)&tje->tsd);
#endif
    tck = tq->bufs[0];
    if(!tq->ops || !tq->ops->buf_release){
        dbg("no ops or no buf_release!\n");
        ret = EINVAL;
        goto out;

    }

    ret = tq->ops->buf_release(tq,tje,tck);
    //queue_inc_rp(tq);
    //set_tck_state(tck,STATE_FREE);
    //wake_up_interruptible(&tq->done);	
out:
    up(&tje->sem);
    return ret;
}



/*do actually release,  release everything it has*/
void tje_cleanup(void * arg)
{
    struct isil_jpeg_en *tje = (struct isil_jpeg_en *)arg;

    tc_trace;
    tc_queue_release(tje->tsd.queue);
    kfree(tje);	
    return;	
}

/*
   struct isil_jpeg_logic_encode_chan * lookup_jpeg_drive_instance(struct isil_service_device *tsd)
   {
   struct isil_jpeg_logic_encode_chan *p;
   unsigned long ld;

//tc_trace;
p = tsd->private_data;
ld = get_phchan_id_from_epobj(&tsd->epobj);
return (struct isil_jpeg_logic_encode_chan *)&p[ld]; 
}
*/

void tc_jpeg_en_polling(unsigned long p)
{
    struct isil_jpeg_en *tje = (struct isil_jpeg_en *)p;
    struct isil_jpeg_logic_encode_chan *driv = container_of(tje->tsd.ped,struct isil_jpeg_logic_encode_chan, opened_logic_chan_ed);
    isil_vb_frame_tcb_queue_t   *vfq = &driv->jpeg_frame_queue;
    isil_jpeg_encode_control_t  *jpeg_encode_control;
    unsigned long nr;

    jpeg_encode_control = &driv->encode_control;
    jpeg_encode_control->op->flush_curr_consumer_frame(jpeg_encode_control);
    nr = vfq->op->get_curr_queue_entry_number(vfq);
    if(nr)
    {
        wake_up(&tje->wait_poll);
    }
    else
    {
        //printk("in %s the entry number = %d\n",__FUNCTION__,nr);	
    }
    mod_timer(&tje->polling_timer, jiffies + 40);
    return;
}

int jpeg_en_open(struct isil_service_device *tsd,struct file *f)
{
    int ret = 0;
    ed_tcb_t *jped;
    struct isil_jpeg_en *tje = container_of(tsd,struct isil_jpeg_en,tsd);
    struct isil_jpeg_logic_encode_chan *driv = container_of(tje->tsd.ped,struct isil_jpeg_logic_encode_chan, opened_logic_chan_ed);
    //tc_trace;

    /*
       driv = lookup_jpeg_drive_instance(tsd);
       if(!driv){
       dbg("bad register, check para!\n");
       return -EINVAL;
       }
       tje->tsd.ped = &driv->opened_logic_chan_ed;
       */
    if(!tje->tsd.ped){
        dbg("jpeg has no driver attached!\n");
        return -1;
    }
    jped = tje->tsd.ped;
    ret = jped->op->open(&driv->opened_logic_chan_ed);
    if(ret != ISIL_OK){
        printk("in %s audio_driver open failed!\n",__FUNCTION__);
        ret = -1;
        return ret;
    }
    f->private_data = tje;
    tje->polling_timer.expires = jiffies + 40;
    tje->polling_timer.data = (unsigned long)tje;
    tje->polling_timer.function = tc_jpeg_en_polling;
    add_timer(&tje->polling_timer);
    return ret;
}


#ifndef NEW_HEADER
ssize_t jpeg_en_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
    ssize_t offset = 0;
    struct isil_jpeg_en *tje = (struct isil_jpeg_en *)f->private_data;
    struct isil_jpeg_logic_encode_chan *driv = container_of(tje->tsd.ped,struct isil_jpeg_logic_encode_chan, opened_logic_chan_ed);
    isil_vb_pool_t *vjpool = &driv->pool;
    isil_vb_frame_tcb_queue_t *vjq = &driv->jpeg_frame_queue;
    isil_vb_frame_tcb_t *vjf = NULL;
    isil_jpeg_encode_control_t  *jpeg_encode_control;
    size_t cn = count;
    size_t min =  sizeof(isil_frame_header_t) + vjpool->vb_packet_buffer_size;
    loff_t frame_offset;

    //tc_trace;
    down(&tje->sem);
    jpeg_encode_control = &driv->encode_control;
    while(cn > min)
    {
        jpeg_encode_control->op->flush_curr_consumer_frame(jpeg_encode_control);
        vjq->op->try_get_curr_consumer_from_queue(vjq);
        vjf = vjq->curr_consumer;
        if(vjf){
            frame_offset = 0;
            vjf->op->submit(vjf,&buff[offset],cn,&frame_offset,ISIL_MASTER_BITSTREAM,vjpool);
            vjq->op->release_curr_consumer(vjq,vjpool);
            offset += frame_offset;
            cn -= frame_offset;
            /*since inside of the submit have been do padding, so we believe here is aligned */
        }
        else
        {
            //dbg("can not get cunsumer in %s\n",__FUNCTION__);
            goto out;
        }	
    }
out:
    up(&tje->sem);
    *lof = offset;
    //dbg("read %d bytes\n",offset);
    return offset;
}

#else

ssize_t jpeg_en_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
    ssize_t offset = 0;
    struct isil_jpeg_en *tje = (struct isil_jpeg_en *)f->private_data;
    struct isil_jpeg_logic_encode_chan *driv = container_of(tje->tsd.ped,struct isil_jpeg_logic_encode_chan, opened_logic_chan_ed);
    isil_vb_pool_t *vjpool = &driv->pool;
    isil_vb_frame_tcb_queue_t *vjq = &driv->jpeg_frame_queue;
    isil_vb_frame_tcb_t *vjf = NULL;
    isil_jpeg_encode_control_t  *jpeg_encode_control;
    size_t cn = count;
    size_t min =  sizeof(struct isil_common_stream_header) + sizeof(struct isil_transparent_msg_header) + sizeof(isil_jpeg_param_msg_t) + vjpool->vb_packet_buffer_size;
    loff_t frame_offset;

    //tc_trace;
    down(&tje->sem);
    jpeg_encode_control = &driv->encode_control;
    while(cn > min)
    {
        jpeg_encode_control->op->flush_curr_consumer_frame(jpeg_encode_control);
        vjq->op->try_get_curr_consumer_from_queue(vjq);
        vjf = vjq->curr_consumer;
        if(vjf){
            frame_offset = 0;
            vjf->op->submit(vjf,&buff[offset],cn,&frame_offset,ISIL_MASTER_BITSTREAM,vjpool);
            vjq->op->release_curr_consumer(vjq,vjpool);
            offset += frame_offset;
            cn -= frame_offset;
            /*since inside of the submit have been do padding, so we believe here is aligned */
#if 0
        u32 len, size;
        struct file *jpg_file = NULL;
        mm_segment_t fs;
        char *jpg_buf = NULL;
        char *pc;
        isil_frame_msg_t *fm;
        static int no = 0;
        char name[64] = {0};

        pc = (char *)buff;
        pc += sizeof(isil_common_stream_header_t) + sizeof(isil_transparent_msg_header_t) + sizeof(isil_jpeg_param_msg_t);
        fm = (isil_frame_msg_t *)pc; 
        
        jpg_buf = (char*)__get_free_pages(GFP_KERNEL, get_order(680 * 480));
        if(!jpg_buf) {
            printk("no free buffer\n");
            goto out;
        }
        snprintf(name,sizeof(name),"/dev/shm/jpg-%05d",no++);

        jpg_file = filp_open(name, O_CREAT | O_RDWR, 655);
        if(IS_ERR(jpg_file))
        {
            printk("open file \"%s\" faild\n", name);
            return IS_ERR(jpg_file);
        }
        if(copy_from_user(jpg_buf, fm->frame_payload, fm->msg_len - sizeof(isil_frame_msg_t)))
                ISIL_DBG(ISIL_DBG_ERR,"**********************\n");
        printk("before getfs\n");
        fs=get_fs();
        set_fs(KERNEL_DS);
        printk("before fwrite\n");
        len = jpg_file->f_op->write(jpg_file, jpg_buf, 680 * 480, &(jpg_file->f_pos));
        set_fs(fs);
        printk("after write\n");
        if(jpg_buf)
            free_pages((unsigned long)jpg_buf, get_order(680 * 480));
        filp_close(jpg_file, NULL);  
#endif
        }
        else
        {
            //dbg("can not get cunsumer in %s\n",__FUNCTION__);
            goto out;
        }

        break;   //just for one frame
    }
out:
    up(&tje->sem);
    *lof = offset;
    //dbg("read %d bytes\n",offset);
    return offset;
}


#endif


/*this is just for test...*/
ssize_t jpeg_en_write (struct file *f, const char __user *buff, size_t count, loff_t *lof)
{
    return 0;
}

int jpeg_en_ioctl (struct file *f, unsigned int cmd, unsigned long arg)
{	
    int ret = 0;
    struct isil_jpeg_en *tje = (struct isil_jpeg_en *)f->private_data;
    struct endpoint_tcb *ed;
    struct tc_queue *tq = tje->tsd.queue;
    struct tc_buffer_kern * tck;	

    //tc_trace;
    down(&tje->sem);
    switch(cmd)
    {
        case ISIL_CODEC_CHAN_REQUEST_MEM:
            ret = jpeg_en_request_memory(f,arg);
            break;
        case ISIL_CODEC_CHAN_RELEASE_MEM:
            ret = jpeg_en_release_memory(f,arg);
            break;
        case ISIL_CODEC_CHAN_DISCAED_FRAME:
            tje->discard = 1;
            set_tq_state_busy(tq);
            tck = tq->bufs[0];
            ret = tq->ops->buf_release(tq,tje,tck);
            if(ret)
                printk("discard frame ---release failed! in %s\n",__FUNCTION__);
            break;
        default:
            //dbg("other cmd in %s\n",__FUNCTION__);
            if(!tje->tsd.ped){
                printk("the device has no driver ed!\n");
                ret = EFAULT;
                break;
            }
            ed = tje->tsd.ped;
            ret = ed->op->ioctl(ed,cmd,arg);	
            break;
    }
    up(&tje->sem);
    return ret;
}



/*user close(fd) will call this function, it just perform close*/
int  jpeg_en_release(struct file *f)
{
    int ret;
    ed_tcb_t *jped;
    struct isil_jpeg_en *tje = (struct isil_jpeg_en *)f->private_data;
    //struct isil_jpeg_logic_encode_chan *driv = container_of(tje->tsd.ped,struct isil_jpeg_logic_encode_chan, opened_logic_chan_ed);

    tc_trace;
    down(&tje->sem);
#ifdef CONFIG_SMP
    del_timer_sync(&tje->polling_timer);
#else
    del_timer(&tje->polling_timer);
#endif
	tc_queue_reset(tje->tsd.queue);
	jped = tje->tsd.ped;
	ret = jped->op->close(jped);
    up(&tje->sem);
    f->private_data = NULL;        	
    return ret;
}



unsigned int jpeg_en_poll(struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    struct isil_jpeg_en *tje = (struct isil_jpeg_en *)file->private_data;
    struct isil_jpeg_logic_encode_chan *driv = container_of(tje->tsd.ped,struct isil_jpeg_logic_encode_chan, opened_logic_chan_ed);
    isil_vb_frame_tcb_queue_t *vfq = &driv->jpeg_frame_queue;

    down(&tje->sem);
    //if(gb_pf)
    //        dbg("poll--tje[%8p],thread[%d]\n",tje,current->pid);
    poll_wait(file,&tje->wait_poll,wait);
    if(vfq->op->get_curr_queue_entry_number(vfq))
        mask |= POLLIN | POLLRDNORM;
    up(&tje->sem);	
    return mask; 
}




int jpeg_en_init(struct isil_chip_device *tcd,struct cmd_arg *arg,struct isil_service_device **tsd)
{
    //int ret;
    struct isil_jpeg_en *tje;

    //tc_trace;
    tje = kzalloc(sizeof(*tje),GFP_KERNEL);
    if(NULL == tje)
        return -ENOMEM;
    *tsd = &tje->tsd;

    tje->tsd.clean_up = tje_cleanup;
    tje->tsd.queue = kzalloc(sizeof(struct tc_queue),GFP_KERNEL);
    if(NULL == tje->tsd.queue)
        return -ENOMEM;

    tc_queue_init(tje->tsd.queue,&vje_queue_ops,NULL,512*1024,10,tje);
    tje->name = "tw5864_jpeg_encoder";
    tje->users = 1;
    //tje->state = NEED_INIT;
    //set_audio_param(&tje->para,&def_audio_parm_en);
    //init_MUTEX(&tje->sem);
    sema_init(&tje->sem, 1);
	init_timer(&tje->polling_timer);
    init_waitqueue_head(&tje->wait_poll);
    return 0;
}

void jpeg_en_destory(struct isil_service_device *tsd)
{
    struct isil_jpeg_en *tje;
    tc_trace;

    if(!tsd)
        return;
    tje = container_of(tsd,struct isil_jpeg_en,tsd);
    if(tje->tsd.queue)
        kfree(tje->tsd.queue);
    kfree(tje);
}



struct tsd_file_operations    jpeg_en_fops = {
    .owner  = THIS_MODULE,
    .init   = jpeg_en_init,
    .destory= jpeg_en_destory,
    .open	= jpeg_en_open,
    .read	= jpeg_en_read,
    .write	= jpeg_en_write,
    .ioctl  = jpeg_en_ioctl,
    .release= jpeg_en_release,
    .poll = jpeg_en_poll,
};

struct isil_dev_id  isil_jpeg_en_id = {
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (STREAM_TYPE_MAJOR << 16) | CODEC_VIDEO_MJPG,
        .type = ISIL_ENCODER,
        .key = {0},
    },
    .version = 0,
};




static struct isil_dev_table_id *tje_table = NULL;



int isil_jpeg_en_init(void)
{
    int ret = 0; 
    tje_table = kzalloc(sizeof(struct isil_dev_table_id),GFP_KERNEL);
    if(!tje_table){
        dbg("no mem in %s\n",__FUNCTION__);
        return -ENOMEM;	
    }
    INIT_LIST_HEAD(&tje_table->list);
    tje_table->tid = &isil_jpeg_en_id;
    tje_table->ops = &jpeg_en_fops;
    tje_table->para = NULL;// data;
    //dbg_epobj_print(&tje_table->tid->epobj);
    ret = register_tc_device(tje_table);
    return ret;		
}




void isil_jpeg_en_remove(void)
{
    if(tje_table)
        unregister_tc_device(tje_table);
    tje_table = NULL;
}

