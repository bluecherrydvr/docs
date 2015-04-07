#include <isil5864/tc_common.h>
/*
   struct isil_chip_audio_param  def_audio_parm_en = {
   .i_bit_wide = ISIL_AUDIO_16BIT,
   .i_sample_rate = ISIL_AUDIO_8K,
   .e_audio_type = ISIL_AUDIO_PCM,
   };
   */


/*actully here we didn't discard the driver submited frame header, but user can get net payload data just by start,size*/
int  ai_buf_prepare(struct tc_queue * tq, void *priv, struct tc_buffer __user *tc)
{
    return 0;	
}

int  ai_buf_release(struct tc_queue * tq, void * priv, struct tc_buffer_kern *tb)
{
    int ret = 0;
    return ret;
}




struct tc_queue_ops    ai_queue_ops =
{
    .buf_prepare = ai_buf_prepare,
    .buf_release = ai_buf_release,	
};







int  ai_request_memory(struct file *f,unsigned long arg)
{
    int ret = 0;

    return ret;
}


int  ai_release_memory(struct file *f,unsigned long arg)
{
    return 0;
}



/*do actually release,  release everything it has*/
void tai_cleanup(void * arg)
{
    struct isil_chip_ai *tai = (struct isil_chip_ai *)arg;

    //tc_trace;
    //tc_queue_release(tai->tsd.queue);
    kfree(tai);	
}

/*
   isil_chip_ai_driver * lookup_drive_instance(struct isil_service_device *tsd)
   {
   isil_audio_driver_t *p = tsd->private_data;
   unsigned long ld = get_phchan_id_from_epobj(&tsd->epobj);
   printk("tsd->minor = %d, ld = %d encoder add = %08X\n",tsd->minor,ld,&p[ld]);

   return &p[ld];

   }
   */

/*
   static int audi_en_polling(void *p)
   {

   struct isil_chip_ai *tai = (struct isil_chip_ai *)p;
   isil_audio_packet_queue_t     *adpq;	
   unsigned long nr;

//down(&tai->sem);
adpq = &tai->driv->audio_packet_queue;	
nr = adpq->op->get_curr_queue_entry_number(adpq);
if(nr)
{
wake_up(&tai->wait_poll);
}
else
{
//printk("in %s the entry number = %d\n",__FUNCTION__,nr);	
}
//up(&tai->sem);
return 0;
}
*/




int chip_ai_open(struct isil_service_device *tsd,struct file *f)
{
    int ret = 0;
    ed_tcb_t   *aed; 
    struct isil_chip_ai *tai = container_of(tsd,struct isil_chip_ai,tsd);
    //isil_audio_driver_t * ptdr = NULL;
    //tc_trace;

    //tai->driv = lookup_drive_instance(tsd);
    //tai->tsd.ped = &tai->driv->audio_ed;
    /*	tai->timer_id = AddForFireTimerJob(12,audi_en_polling,tai);
        if(tai->timer_id == ADDJOBERROR){
        printk("in %s add timer job failed!\n",__FUNCTION__);
        ret = -1;
        return ret;
        }
        */
    if(!tai->tsd.ped){
        dbg("ai dev has no driver attached!\n");
        return -1;
    }
    aed = tai->tsd.ped;
    ret = aed->op->open(aed);
    if(ret != ISIL_OK){
        printk("in %s audio_driver open failed!\n",__FUNCTION__);
        ret = -1;
        return ret;
    }
    f->private_data = tai;
    return ret;
}



ssize_t chip_ai_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
    int offset = 0;
    return (ssize_t)offset;
}


/*this is just for test...*/
ssize_t chip_ai_write (struct file *f, const char __user *buff, size_t count, loff_t *lof)
{

    int ret = 0;

    return ret;

}

int chip_ai_ioctl (struct file *f, unsigned int cmd, unsigned long arg)
{	
    int ret = 0;
    struct isil_chip_ai *tai = (struct isil_chip_ai *)f->private_data;
    struct endpoint_tcb *ed;
    //tc_trace;
    down(&tai->sem);
    switch(cmd)
    {
        case ISIL_CODEC_CHAN_REQUEST_MEM:
            ret = ai_request_memory(f,arg);
            break;
        case ISIL_CODEC_CHAN_RELEASE_MEM:
            ret = ai_release_memory(f,arg);
            break;
        default:
            //dbg("other cmd in %s\n",__FUNCTION__);
            if(!tai->tsd.ped){
                printk("the device has no driver ed!\n");
                ret = EFAULT;
                break;
            }
            ed = tai->tsd.ped;
            ret = ed->op->ioctl(ed,cmd,arg);	
            break;
    }
    up(&tai->sem);
    return ret;
}


/*user close(fd) will call this function, it just perform close*/
int  chip_ai_release(struct file *f)
{
    int ret;
    ed_tcb_t   *aed;
    struct isil_chip_ai *tai = (struct isil_chip_ai *)f->private_data;

    tc_trace;
    down(&tai->sem);
    aed = tai->tsd.ped;
    ret = aed->op->close(aed);
    //tc_queue_reset(tai->tsd.queue);
    up(&tai->sem);

    f->private_data = NULL;
    return ret;
}



unsigned int chip_ai_poll(struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    /*
       struct isil_chip_ai *tai = (struct isil_chip_ai *)file->private_data;
       isil_audio_packet_queue_t     *adpq = &tai->driv->audio_packet_queue;	

       down(&tai->sem);
       poll_wait(file,&tai->wait_poll,wait);
       if(adpq->op->get_curr_queue_entry_number(adpq))
       {
       mask |= POLLIN | POLLRDNORM;
    //dbg("%s frame availble!\n",__FUNCTION__);
    }
    else
    {
    //printk("has no entry number will hung up!\n");	
    }
    up(&tai->sem);	
    */
    return mask; 
}




int chip_ai_init(struct isil_chip_device *tcd,struct cmd_arg *arg,struct isil_service_device **tsd)
{
    struct isil_chip_ai *tai;

    tc_trace;
    tai = kzalloc(sizeof(*tai),GFP_KERNEL);
    if(NULL == tai)
        return -ENOMEM;
    *tsd = &tai->tsd;

    tai->tsd.clean_up = tai_cleanup;

    tai->name = "tw5864-audio_input";
    tai->users = 1;
    //tai->state = NEED_INIT;
    //set_audio_param(&tai->para,&def_audio_parm_en);
    sema_init(&tai->sem, 1);
	init_timer(&tai->polling_timer);
    init_waitqueue_head(&tai->wait_poll);
    return 0;
}

void chip_ai_destory(struct isil_service_device *tsd)
{
    struct isil_chip_ai *tai;
    tc_trace;

    if(!tsd)
        return;
    tai = container_of(tsd,struct isil_chip_ai,tsd);
    //if(tai->tsd.queue)
    //        kfree(tai->tsd.queue);
    kfree(tai);
}


struct tsd_file_operations    chip_ai_fops = {
    .owner  = THIS_MODULE,
    .init   = chip_ai_init,
    .destory= chip_ai_destory,
    .open	= chip_ai_open,
    .read	= chip_ai_read,
    .write	= chip_ai_write,
    .ioctl  = chip_ai_ioctl,
    .release= chip_ai_release,
    .poll = chip_ai_poll,
};

struct isil_dev_id  isil_chip_ai_id = {
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = 0,
        .type = ISIL_AUDIO_IN,
        .key = {0},
    },
    .version = 0,
};

static struct isil_dev_table_id *tai_table = NULL;

int isil_chip_ai_init(void)
{
    tai_table = kzalloc(sizeof(struct isil_dev_table_id),GFP_KERNEL);
    if(!tai_table){
        dbg("no mem in %s\n",__FUNCTION__);
        return -ENOMEM;	
    }
    INIT_LIST_HEAD(&tai_table->list);
    tai_table->tid = &isil_chip_ai_id;
    tai_table->ops = &chip_ai_fops;
    tai_table->para = NULL;//data;
    dbg_epobj_print(&tai_table->tid->epobj);
    register_tc_device(tai_table);
    return 0;	
}

void isil_chip_ai_remove(void)
{
    if(tai_table)
        unregister_tc_device(tai_table);
    tai_table = NULL;
}

