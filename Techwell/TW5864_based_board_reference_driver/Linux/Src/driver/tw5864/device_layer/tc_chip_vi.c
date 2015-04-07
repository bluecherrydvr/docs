#include <isil5864/tc_common.h>
/*
   struct isil_chip_audio_param  def_audio_parm_en = {
   .i_bit_wide = ISIL_AUDIO_16BIT,
   .i_sample_rate = ISIL_AUDIO_8K,
   .e_audio_type = ISIL_AUDIO_PCM,
   };
   */


/*actully here we didn't discard the driver submited frame header, but user can get net payload data just by start,size*/
int  vi_buf_prepare(struct tc_queue * tq, void *priv, struct tc_buffer __user *tc)
{
    return 0;	
}

int  vi_buf_release(struct tc_queue * tq, void * priv, struct tc_buffer_kern *tb)
{
    int ret = 0;
    return ret;
}




struct tc_queue_ops    vi_queue_ops =
{
    .buf_prepare = vi_buf_prepare,
    .buf_release = vi_buf_release,	
};







int  vi_request_memory(struct file *f,unsigned long arg)
{
    int ret = 0;

    return ret;
}


int  vi_release_memory(struct file *f,unsigned long arg)
{
    return 0;
}



/*do actually release,  release everything it has*/
void tvi_cleanup(void * arg)
{
    struct isil_chip_vi *tvi = (struct isil_chip_vi *)arg;

    tc_trace;
    //tc_queue_release(tvi->tsd.queue);
    kfree(tvi);	
}

/*
   isil_chip_vi_driver * lookup_drive_instance(struct isil_service_device *tsd)
   {
   isil_audio_driver_t *p = tsd->private_data;
   unsigned long ld = get_phchan_id_from_epobj(&tsd->epobj);
   printk("tsd->minor = %d, ld = %d encoder add = %08X\n",tsd->minor,ld,&p[ld]);

   return &p[ld];

   }
   */


static void tc_vi_polling(unsigned long p)
{
	struct isil_chip_vi *tvi = (struct isil_chip_vi *)p;
	unsigned long nr;
	isil_msg_pool_t *msg_pool;
    isil_msg_queue_t *msg_queue;
    isil_chip_vi_driver_t *driv = container_of(tvi->tsd.ped, isil_chip_vi_driver_t, opened_vi_ed);

    msg_pool = &driv->chip->chip_driver->msg_pool;
    msg_queue = &driv->msg_queue;

	nr = msg_queue->op->get_curr_queue_entry_number(msg_queue);
	if(nr)
	{
		wake_up(&tvi->wait_poll);
	}
	else
	{
		//printk("in %s the entry number = %d\n",__FUNCTION__,nr);	
	}
	mod_timer(&tvi->polling_timer, jiffies + 40);

	return;
}





int chip_vi_open(struct isil_service_device *tsd,struct file *f)
{
    int ret = 0;
    ed_tcb_t   *vid; 
    struct isil_chip_vi *tvi = container_of(tsd,struct isil_chip_vi,tsd);
    //isil_audio_driver_t * ptdr = NULL;
    tc_trace;
	if(!tvi->tsd.ped){
			dbg("ai dev has no driver attached!\n");
			return -1;
	}
	vid = tvi->tsd.ped;
	ret = vid->op->open(vid);
	if(ret != ISIL_OK){
		printk("in %s audio_driver open failed!\n",__FUNCTION__);
		ret = -1;
		return ret;
	}
	f->private_data = tvi;
	tvi->polling_timer.expires = jiffies + 40 * HZ;
    tvi->polling_timer.data = (unsigned long)tvi;
    tvi->polling_timer.function = tc_vi_polling;
    add_timer(&tvi->polling_timer);
        return ret;
}



ssize_t chip_vi_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
	struct isil_chip_vi *tvi = (struct isil_chip_vi *)f->private_data;
	int offset = 0,remain = count,safe_max = count - 50;
	unsigned long rv;
	isil_common_stream_header_t icsh,*picsh = &icsh;
	isil_transparent_msg_header_t itmh, *pitmh = &itmh;
    isil_msg_pool_t *msg_pool;
    isil_msg_queue_t *msg_queue;
    isil_msg_tcb_t *msg_tcb;
    isil_chip_vi_driver_t *driv = container_of(tvi->tsd.ped, isil_chip_vi_driver_t, opened_vi_ed);
    char *pc;

    msg_pool = &driv->chip->chip_driver->msg_pool;
    msg_queue = &driv->msg_queue;

    picsh->stream_type = ISIL_VIDEO_MSG_ALARM_GROUP;
    picsh->stream_len = sizeof(isil_common_stream_header_t);
    offset += sizeof(isil_common_stream_header_t);

    picsh->stream_len += sizeof(isil_transparent_msg_header_t);
    pitmh->isil_transparent_msg_total_len = sizeof(isil_transparent_msg_header_t);
    pitmh->isil_transparent_msg_number = 0;
    offset += sizeof(isil_transparent_msg_header_t);

    while(offset <= safe_max){
    	msg_queue->op->try_get_curr_consumer_from_queue(msg_queue);
    	if(!msg_queue->curr_consumer){
    		printk("can not get consumer ! %s...%d\n",__FUNCTION__, __LINE__);
    		goto out;
    	}
    	msg_tcb = msg_queue->curr_consumer;
    	rv = msg_tcb->op->submit(msg_tcb, (char __user *)&buff[offset], remain, lof);
        msg_queue->op->release_curr_consumer(msg_queue, msg_pool);
    	offset += rv;
    	remain -= rv;
    	picsh->stream_len += rv;
    	pitmh->isil_transparent_msg_total_len += rv;
    	pitmh->isil_transparent_msg_number++;
    }
out:
    picsh->stream_len |= ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG;
    pc = &buff[0];
    if(copy_to_user(pc, &icsh, sizeof(isil_common_stream_header_t)) != 0){
    	printk("copy error in %s...%d\n", __FUNCTION__, __LINE__);
    	return 0;
    }
    pc += sizeof(isil_common_stream_header_t);
    if(copy_to_user(pc, &itmh, sizeof(isil_transparent_msg_header_t)) != 0){
    	printk("copy error in %s...%d\n", __FUNCTION__, __LINE__);
    	return 0;
    }
	return (ssize_t)offset;
}


/*this is just for test...*/
ssize_t chip_vi_write (struct file *f, const char __user *buff, size_t count, loff_t *lof)
{

    int ret = 0;

    return ret;

}

int chip_vi_ioctl (struct file *f, unsigned int cmd, unsigned long arg)
{	
    int ret = 0;
    struct isil_chip_vi *tvi = (struct isil_chip_vi *)f->private_data;
    struct endpoint_tcb *ed;
    //tc_trace;

    down(&tvi->sem);
    switch(cmd)
    {
        case ISIL_CODEC_CHAN_REQUEST_MEM:
            ret = vi_request_memory(f,arg);
            break;
        case ISIL_CODEC_CHAN_RELEASE_MEM:
            ret = vi_release_memory(f,arg);
            break;
        default:
            //dbg("other cmd in %s\n",__FUNCTION__);
            if(!tvi->tsd.ped){
                printk("the device has no driver ed!\n");
                ret = -EFAULT;
                break;
            }
            ed = tvi->tsd.ped;
            ret = ed->op->ioctl(ed,cmd,arg);	
            break;
    }
    up(&tvi->sem);
    return ret;
}


/*user close(fd) will call this function, it just perform close*/
int  chip_vi_release(struct file *f)
{
    int ret;
    ed_tcb_t   *vid;
    struct isil_chip_vi *tvi = (struct isil_chip_vi *)f->private_data;

    tc_trace;
    down(&tvi->sem);
#ifdef CONFIG_SMP
    del_timer_sync(&tvi->polling_timer);
#else
    del_timer(&tvi->polling_timer);
#endif

	vid = tvi->tsd.ped;
	ret = vid->op->close(vid);
	//tc_queue_reset(tvi->tsd.queue);
    up(&tvi->sem);

    f->private_data = NULL;
    return ret;
}



unsigned int chip_vi_poll(struct file *file, struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	struct isil_chip_vi *tvi = (struct isil_chip_vi *)file->private_data;
	isil_msg_pool_t *msg_pool;
    isil_msg_queue_t *msg_queue;
    isil_chip_vi_driver_t *driv = container_of(tvi->tsd.ped, isil_chip_vi_driver_t, opened_vi_ed);

    msg_pool = &driv->chip->chip_driver->msg_pool;
    msg_queue = &driv->msg_queue;

	down(&tvi->sem);
	poll_wait(file,&tvi->wait_poll,wait);
	if(msg_queue->op->get_curr_queue_entry_number(msg_queue))
	{
		mask |= POLLIN | POLLRDNORM;
		//dbg("%s frame availble!\n",__FUNCTION__);
	}
	else
	{
		//printk("has no entry number will hung up!\n");	
	}
	up(&tvi->sem);	

    return mask; 
}




int chip_vi_init(struct isil_chip_device *tcd,struct cmd_arg *arg,struct isil_service_device **tsd)
{
    int ret=0;
    struct isil_chip_vi *tvi;

    tc_trace;
    tvi = kzalloc(sizeof(*tvi),GFP_KERNEL);
    if(NULL == tvi)
        return -ENOMEM;
    *tsd = &tvi->tsd;

    tvi->tsd.clean_up = tvi_cleanup;
    /*	tvi->tsd.queue = kzalloc(sizeof(struct tc_queue),GFP_KERNEL);
        if(NULL == tvi->tsd.queue)
        return -ENOMEM;

        tc_queue_init(tvi->tsd.queue,&vi_queue_ops,NULL,512,10,tvi);
        */
    tvi->name = "tw5864-audio_input";
    tvi->users = 1;
    //tvi->state = NEED_INIT;
    //set_audio_param(&tvi->para,&def_audio_parm_en);
    sema_init(&tvi->sem, 1);
	init_timer(&tvi->polling_timer);
	init_waitqueue_head(&tvi->wait_poll);
    return ret;
}

void chip_vi_destory(struct isil_service_device *tsd)
{
    struct isil_chip_vi *tvi;
    tc_trace;

    if(!tsd)
        return;
    tvi = container_of(tsd,struct isil_chip_vi,tsd);
    //if(tvi->tsd.queue)
    //        kfree(tvi->tsd.queue);
    kfree(tvi);
}


struct tsd_file_operations    chip_vi_fops = {
    .owner  = THIS_MODULE,
    .init   = chip_vi_init,
    .destory= chip_vi_destory,
    .open	= chip_vi_open,
    .read	= chip_vi_read,
    .write	= chip_vi_write,
    .ioctl  = chip_vi_ioctl,
    .release= chip_vi_release,
    .poll = chip_vi_poll,
};

struct isil_dev_id  isil_chip_vi_id = {
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = 0,
        .type = ISIL_VIDEO_IN,
        .key = {0},
    },
    .version = 0,
};

static struct isil_dev_table_id *tvi_table = NULL;

int isil_chip_vi_init(void)
{
    tvi_table = kzalloc(sizeof(struct isil_dev_table_id),GFP_KERNEL);
    if(!tvi_table){
        dbg("no mem in %s\n",__FUNCTION__);
        return -ENOMEM;	
    }
    INIT_LIST_HEAD(&tvi_table->list);
    tvi_table->tid = &isil_chip_vi_id;
    tvi_table->ops = &chip_vi_fops;
    tvi_table->para = NULL;//data;
    dbg_epobj_print(&tvi_table->tid->epobj);
    register_tc_device(tvi_table);
    return 0;	
}

void isil_chip_vi_remove(void)
{
    if(tvi_table)
        unregister_tc_device(tvi_table);
    tvi_table = NULL;
}
