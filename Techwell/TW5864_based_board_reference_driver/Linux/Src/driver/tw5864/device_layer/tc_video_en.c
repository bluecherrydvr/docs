#include <isil5864/tc_common.h>


void print_frame_header(char * pt)
{
    unsigned long idx =0;
    for(;idx < 48; idx++){
        if(0 == (idx%16))
            dbg("\n\n");
        dbg("<0x%02X>,",*pt++);
    }
    dbg("\n\n");
}


int gather_scatter_info(isil_video_frame_tcb_t * vf,struct tc_buffer_kern *tck)
{
    isil_video_packet_tcb_queue_t *vpq = &vf->video_packet_queue;
    isil_video_packet_tcb_t  *vp = NULL;
    int idx = 0, ret = 0;
    //static icn = 0;

    while(vpq->op->get_curr_queue_entry_number(vpq)){
        vpq->op->try_get_curr_consumer_from_queue(vpq);
        vp = vpq->curr_consumer;
        if(vp){
            vp->op->reference(vp,(isil_video_packet_tcb_t **)&tck->sg[idx].carrier);
            tck->sg[idx].addr = (__u32 *)vp->data;
            tck->sg[idx].len = vp->payload_len;
            ret += vp->payload_len;
            if(!vf->video_chan_buf_pool)
                dbg("BUG: pool is NULL! %d\n",__LINE__);

            vpq->op->release_curr_consumer(vpq,vf->video_chan_buf_pool);
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

/*actully here we didn't discard the driver submited frame header, but user can get net payload data just by start,size
 * return (1):0,tc->__tc__size!=0             we have video_frame prepared.
 *        (2):ENOMEM, tc->__tc__size!=0       the user buffer is to small.smaller than 1 logic frame size
 *        (3):EAGAIN                    can not get header_buf,or get not get even one frame from driver.
 */
int  ve_buf_prepare(struct tc_queue * tq, void *priv, struct tc_buffer __user *tc)
{
    struct tc_buffer_kern *tck,*tmp;
    struct isil_video_en * tve = (struct isil_video_en *)priv;
    struct isil_h264_logic_encode_chan *driv = container_of(tve->tsd.ped,struct isil_h264_logic_encode_chan, opened_logic_chan_ed);
    isil_video_frame_tcb_queue_t *vfq = &driv->encode_frame_queue;
    isil_video_frame_tcb_t *vf = NULL;
    int is_master = 0,buffer_need = PAGE_SIZE;
    unsigned long	nr_ready,nr_header,idx,us = tc->__tc__size;
    struct tc_header_buf  *thb = NULL;
    unsigned long off = 0,nosense;

    tmp = tq->bufs[0];
    is_master = (get_stream_from_epobj(&tve->tsd.epobj) == STREAM_TYPE_MAJOR)? 1: 0;
    if((STATE_READY == get_tck_state(tmp)) && (0 != tmp->last_prepare)){
        /*last gathered data were not copy by user*/
        tc->__tc__nr_frame = 1;
        if(tmp->last_prepare > us){
            tc->__tc__size = tmp->last_prepare;
            return -ENOMEM;                        
        }
        tc->__tc__size = tmp->last_prepare;
        tmp->last_prepare = 0;
        printk("last buffer is too smaller!!!!!!!!!!!!!!!!!!!\n");
    }
    else
    {
		thb_pool->op->get_header_buf(thb_pool,&thb);
		if(!thb){
			dbg("get header_buffer nothing in %s!\n",__FUNCTION__);
			return -EAGAIN;         
		}
		nr_header = tc_round_low(thb->unit_size,MIN_HEADER_LEN_VIDEO);
retry:
		nr_ready = vfq->op->get_curr_queue_entry_number(vfq);
        nr_ready = tc_min(nr_ready,nr_header);
        nr_ready = tc_min(nr_ready, tq->buf_nr);
        tc->__tc__nr_frame = 0;
		for(idx = 0; idx < nr_ready; idx++){
            vf = NULL;
            if(!vfq->curr_consumer){
                vfq->op->try_get_curr_consumer_from_queue(vfq);
                if(!vfq->curr_consumer){
                    dbg("consumer is NULL %s;\n",__FUNCTION__);
                    tc->__tc__size = buffer_need;  //tell user how many data are ready
                    if(PAGE_SIZE != tc->__tc__size){
                        return 0;
                    }
                    else{
                        thb_pool->op->put_header_buf(thb_pool,thb);
                        return -EAGAIN;
                    }
                }
            }
            /*
               printk("phy_id [%d]\n",driv->phy_slot_id);
               h264_decode_logic_chan = &driv->chip->h264_decode_logic_chan[driv->phy_slot_id];
               isil_h264_send_frame_to_vdec(h264_decode_logic_chan, driv, vfq->curr_consumer);
               printk("in %s ... %d\n", __FUNCTION__, __LINE__);
               */
            if(tve->discard){
                if(H264_FRAME_TYPE_IDR != vfq->curr_consumer->frame_type)
                {
                    printk("discard frame %p \n",vfq->curr_consumer);
                    vfq->op->release_curr_consumer(vfq,&driv->encode_chan_buf_pool);
                    goto retry;
                }
                else
                {
                    tve->discard = 0;	
                }
            }

            vfq->curr_consumer->op->reference(vfq->curr_consumer,&vf);
            vfq->op->release_curr_consumer(vfq,&driv->encode_chan_buf_pool);
            tck = tq->bufs[idx];
            if(get_tck_state(tck) == STATE_READY){
                dbg("BUG:%s,%d\n",__FUNCTION__,__LINE__);
                                thb_pool->op->put_header_buf(thb_pool,thb);
                return -EINVAL;
            }

            tck->index = idx;
            tck->type = MEM_MMAP;
            tck->thb = thb;
			tck->holder = vf;
			if(H264_FRAME_TYPE_IDR == vf->frame_type)
				tck->size = vf->frame_len + vf->nal.sps_paysize + vf->nal.pps_paysize;
			else
				tck->size = vf->frame_len;

            if((buffer_need + tc_align_up(tck->size,PAGE_SIZE)) > us){
                if(0 == idx){
                    tc->__tc__size = buffer_need + tc_align_up(tck->size,PAGE_SIZE);
                    tc->__tc__nr_frame = 1;
                    set_tck_state(tck,STATE_READY);
                    nosense = gather_scatter_info(vf,tck);
                    tc_buffer_set_h264_data(is_master,(struct tc_buffer *)(thb->data),tck,vf,&off);
                    tmp->last_prepare = tc->__tc__size;
                    printk("buf is smaller!!!!!!!!!!!!!!!!!!\n");
                    return -ENOMEM;
                }
                else{                                  
                    vf->op->reference(vf,&vfq->curr_consumer);
                    vf->op->release(&vf,&driv->encode_chan_buf_pool);
                    tck->holder = NULL;
                    return 0;
                }					
            }
            buffer_need += tc_align_up(tck->size,PAGE_SIZE);
            tc->__tc__size = buffer_need;
            set_tck_state(tck,STATE_READY);
            nosense = gather_scatter_info(vf,tck);
            tc_buffer_set_h264_data(is_master,(struct tc_buffer *)(thb->data),tck,vf,&off);
            tc->__tc__nr_frame++;
            //printk("E!");
        }
    }
	return 0;
}




int  ve_buf_release(struct tc_queue * tq, void * priv, struct tc_buffer_kern *tb)
{
    int ret = 0,idx,cn = 0;
    struct isil_video_en * tve = (struct isil_video_en *)priv;
    struct isil_h264_logic_encode_chan *driv = container_of(tve->tsd.ped,struct isil_h264_logic_encode_chan, opened_logic_chan_ed);
    isil_video_chan_buf_pool_t    *vdpool = &driv->encode_chan_buf_pool;
    isil_video_frame_tcb_t *vf = NULL;			
    isil_video_packet_tcb_t  *vp = NULL;
    struct tc_buffer_kern *tck = tq->bufs[0];

    /*release the header buffer page*/
    if(!tck->thb || !tck->holder){
        dbg("invalid parameters in tck! -->%s\n",__FUNCTION__);
        return -EINVAL;
    }
    thb_pool->op->put_header_buf(thb_pool,tck->thb);

    while((tck = tq->bufs[cn]) && (STATE_BUSY == get_tck_state(tck))){
        idx = 0;
        //first release all video packet in one frame
        while(tck->sg[idx].carrier && tck->sg[idx].addr && tck->sg[idx].len){
            vp = (isil_video_packet_tcb_t *)tck->sg[idx].carrier;
            if(!vp->op || !vp->op->release){
                printk("BUG: in %s  %d----@@@\n",__FUNCTION__,__LINE__);
                return -EINVAL;
            }
            if(!vdpool){
                dbg("encode_chan_buf_pool is NULL!\n");
                return -EINVAL;
            }
            vp->op->release(&vp,vdpool);
            memset(&tck->sg[idx],0,sizeof(struct tc_scatter));
            idx++;
            if(idx >= SCATTRT_MAX_ENTRY)
                break;
        }
        // then release the video frame
        vf = (isil_video_frame_tcb_t *)tck->holder;
        if(!vf->op || !vf->op->release){
            printk("BUG: in %s  %d----@@@\n",__FUNCTION__,__LINE__);
            return -EINVAL;
        }
        vf->op->release(&vf,vdpool);
        set_tck_state(tck,STATE_FREE);             
        cn++;
        if(cn >= tq->buf_nr)
            break;
    }
    return ret;
}

struct tc_queue_ops    ve_queue_ops =
{
    .buf_prepare = ve_buf_prepare,
    .buf_release = ve_buf_release,	
};


static int  h264_en_request_memory(struct file *f,unsigned long arg)
{
    int ret = 0;
    //struct tc_buffer_kern *tck;
    struct tc_buffer *tc = (struct tc_buffer *)arg;
    struct isil_video_en * tve = (struct isil_video_en *)f->private_data;
    struct tc_queue *q = tve->tsd.queue;
    //unsigned long fa = 0;
    //tc_trace;

    down(&tve->sem);
#ifdef STATIC_COUNTING
    get_loop_start((struct isil_service_device *)&tve->tsd);
#endif
    if(!q->ops){
        ISIL_DBG(ISIL_DBG_ERR,"tq has no ops!\n");
        ret = EINVAL;
        goto out;
    }
    if(!q->ops->buf_prepare){
        ISIL_DBG(ISIL_DBG_ERR,"tq->ops has no buf_prepare!\n");
        ret = EINVAL;
        goto out;               
    }
    ret = q->ops->buf_prepare(q,tve,tc);
#ifdef STATIC_COUNTING
    calc_ic_req_time((struct isil_service_device *)&tve->tsd);
#endif        
out:
    up(&tve->sem);
    //printk("<---out");
    return ret;
}





int  h264_en_release_memory(struct file *f,unsigned long arg)
{
    int ret = 0;	
    struct isil_video_en * tve = (struct isil_video_en *)f->private_data;
    struct tc_queue *tq = tve->tsd.queue;
    struct tc_buffer_kern * tck;	

    down(&tve->sem);
#ifdef STATIC_COUNTING
    get_ic_rel_start((struct isil_service_device *)&tve->tsd);
#endif
    tck = tq->bufs[0];
    if(!tq->ops || !tq->ops->buf_release){
        ISIL_DBG(ISIL_DBG_ERR,"no ops or no buf_release!\n");
        ret = EINVAL;
        goto out;

    }

    ret = tq->ops->buf_release(tq,tve,tck);
    //queue_inc_rp(tq);
    //set_tck_state(tck,STATE_FREE);
    //wake_up_interruptible(&tq->done);
#ifdef STATIC_COUNTING
    calc_ic_rel_time((struct isil_service_device *)&tve->tsd);
    get_loop_end((struct isil_service_device *)&tve->tsd);
#endif	
out:
    up(&tve->sem);
    return ret;
}



/*do actually release,  release everything it has*/
void tve_cleanup(void * arg)
{
    struct isil_video_en *tve = (struct isil_video_en *)arg;

    tc_trace;
    tc_queue_release(tve->tsd.queue);
    kfree(tve);	
    return;	
}


isil_h264_logic_encode_chan_t * lookup_video_drive_instance(struct isil_service_device *tsd)
{
    isil_h264_logic_encode_chan_t *p = tsd->private_data;
    unsigned long ld = get_phchan_id_from_epobj(&tsd->epobj);

    //ISIL_DBG("%s------> ld = %d \n",__FUNCTION__,ld);

    return &p[ld];

}


static void  video_h264_en_polling(unsigned long p)
{
    struct isil_video_en *tve = (struct isil_video_en *)p;
    struct isil_h264_logic_encode_chan *driv = container_of(tve->tsd.ped,struct isil_h264_logic_encode_chan, opened_logic_chan_ed);
    isil_video_frame_tcb_queue_t *vfq = &driv->encode_frame_queue;
    isil_h264_encode_control_t	*encode_control;
    unsigned long nr;

    encode_control = &driv->encode_control;
    encode_control->op->flush_curr_consumer_frame(encode_control);
    nr = vfq->op->get_curr_queue_entry_number(vfq);
    if(nr)
    {    
        wake_up(&tve->wait_poll);
    }
    else
    {
    }
    mod_timer(&tve->polling_timer, jiffies + 40);
    return;
}

int videoh264_en_open(struct isil_service_device *tsd,struct file *f)
{
    int ret = 0;
    ed_tcb_t   *veed;
    struct isil_video_en *tve = container_of(tsd,struct isil_video_en,tsd);
    //struct isil_h264_logic_encode_chan *driv = container_of(tve->tsd.ped,struct isil_h264_logic_encode_chan, opened_logic_chan_ed);


    if(!tve->tsd.ped){
        dbg("video dev has no driver attached!\n");
        ret = -1;
        return ret;
    }

	veed = tve->tsd.ped;//&driv->opened_logic_chan_ed;
	ret = veed->op->open(veed);
	if(ret != ISIL_OK){
		printk("in %s audio_driver open failed!\n",__FUNCTION__);
		ret = -1;
		return ret;
	}
	f->private_data = tve;

    tve->polling_timer.expires = jiffies + 40;
    tve->polling_timer.data = (unsigned long)tve;
    tve->polling_timer.function = video_h264_en_polling;
    add_timer(&tve->polling_timer);
    return ret;
}


#ifndef  NEW_HEADER
ssize_t videoh264_en_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
    ssize_t offset = 0;
    struct isil_video_en *tve = (struct isil_video_en *)f->private_data;
    struct isil_h264_logic_encode_chan *driv = container_of(tve->tsd.ped,struct isil_h264_logic_encode_chan, opened_logic_chan_ed);
    isil_h264_encode_control_t	*encode_control = &driv->encode_control;
    isil_video_chan_buf_pool_t *vdpool = &driv->encode_chan_buf_pool;
    isil_video_frame_tcb_queue_t *vfq = &driv->encode_frame_queue;
    isil_video_frame_tcb_t *vf = NULL;
    size_t cn = count;
    size_t min = sizeof(isil_frame_header_t) + vdpool->video_packet_buffer_size;
    loff_t frame_offset;

    //tc_trace;
    down(&tve->sem);
    while(cn > min)
    {
        encode_control->op->flush_curr_consumer_frame(encode_control);
        vfq->op->try_get_curr_consumer_from_queue(vfq);
        vf = vfq->curr_consumer;
        if(vf){
            frame_offset = 0;
            vf->op->submit(vf,&buff[offset],cn,&frame_offset,((driv->type == ISIL_MASTER_BITSTREAM)?ISIL_MASTER_BITSTREAM:ISIL_SUB_BITSTREAM),vdpool);
            vfq->op->release_curr_consumer(vfq,vdpool);
            offset += frame_offset;
            cn -= frame_offset;
            /*since inside of the submit have been do padding, so we believe here is aligned */
        }
        else
        {
            //ISIL_DBG(ISIL_DBG_ERR,"can not get cunsumer in %s\n",__FUNCTION__);
            goto out;
        }	
    }
out:
    up(&tve->sem);
    *lof = offset;
    //ISIL_DBG(ISIL_DBG_ERR,"read %d bytes\n",offset);
    return offset;
}
#else

ssize_t videoh264_en_read(struct file *f,char __user *buff,size_t count, loff_t * lof)
{
    ssize_t offset = 0;
    struct isil_video_en *tve = (struct isil_video_en *)f->private_data;
    struct isil_h264_logic_encode_chan *driv = container_of(tve->tsd.ped,struct isil_h264_logic_encode_chan, opened_logic_chan_ed);
    isil_h264_encode_control_t	*encode_control = &driv->encode_control;
    isil_video_chan_buf_pool_t *vdpool = &driv->encode_chan_buf_pool;
    isil_video_frame_tcb_queue_t *vfq = &driv->encode_frame_queue;
    isil_video_frame_tcb_t *vf = NULL;
    size_t cn = count, tt = 0,v;
    size_t min = sizeof(struct isil_common_stream_header) + sizeof(struct isil_transparent_msg_header) + sizeof(struct isis_h264_encode_frame_type_msg)+ vdpool->video_packet_buffer_size;
    loff_t frame_offset;
#ifdef  MV_MODULE	
    isil_video_mv_frame_tcb_queue_t *mfq;
    isil_video_mv_frame_tcb_t *mf;
#endif

    //tc_trace;
    down(&tve->sem);
    while(cn > min)
    {
        encode_control->op->flush_curr_consumer_frame(encode_control);
        vfq->op->try_get_curr_consumer_from_queue(vfq);
        vf = vfq->curr_consumer;
        if(vf){
            frame_offset = 0;
	        v = vf->op->submit(vf,&buff[offset],cn,&frame_offset,((driv->type == ISIL_MASTER_BITSTREAM)?ISIL_MASTER_BITSTREAM:ISIL_SUB_BITSTREAM),vdpool);
            if(v == 0){
            	offset = 0;
            	goto out;
            }
	        vfq->op->release_curr_consumer(vfq,vdpool);
            offset += frame_offset;
            break;//just break out, since we now just support one frame copy
            //cn -= frame_offset;
#ifdef MV_MODULE
            mfq = &driv->encode_mv_frame_queue;
            if(mfq->op->get_curr_queue_entry_number(mfq)){
                ISIL_DBG(ISIL_DBG_ERR,"has the mv data &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&**************************#####$$$@@@@@@@\n");
                mfq->op->try_get_curr_consumer_from_queue(mfq);
                mf = mfq->curr_consumer;
                if(mf){
                    mv_offset = 0;
                    mf->op->submit(mf, &buff[offset], cn, &mv_offset, ((driv->type == ISIL_MASTER_BITSTREAM)?ISIL_MASTER_BITSTREAM:ISIL_SUB_BITSTREAM), vdpool);
                    mfq->op->release_curr_consumer(mfq,vdpool);
                    offset += mv_offset;
                    cn -= mv_offset;

                    pc = (char *)buff;
                    csh = (isil_common_stream_header_t *)pc;
                    csh->stream_len += mv_offset;
                    pc = (char *)&buff[sizeof(isil_common_stream_header_t)];
                    tmh = (isil_transparent_msg_header_t *)pc;
                    tmh->isil_transparent_msg_number += 2;
                    tmh->isil_transparent_msg_total_len += mv_offset;
                }
            }
#endif			
            /*since inside of the submit have been do padding, so we believe here is aligned */
        }
        break;//just for one frame only
    }
out:
    up(&tve->sem);
    *lof = offset;
    //ISIL_DBG(ISIL_DBG_ERR,"read %d bytes\n",offset);
    return offset;
}
#endif

/*this is just for test...*/
ssize_t videoh264_en_write (struct file *f, const char __user *buff, size_t count, loff_t *lof)
{
    return 0;
}
int videoh264_en_ioctl (struct file *f, unsigned int cmd, unsigned long arg)
{	
    int ret = 0;
    struct isil_video_en *tve = (struct isil_video_en *)f->private_data;
    struct endpoint_tcb *ed;
    struct tc_queue *tq = tve->tsd.queue;
    struct tc_buffer_kern * tck;	

    down(&tve->sem);
    switch(cmd)
    {
        case ISIL_CODEC_CHAN_REQUEST_MEM:
            ret = h264_en_request_memory(f,arg);
            break;
        case ISIL_CODEC_CHAN_RELEASE_MEM:
            ret = h264_en_release_memory(f,arg);
            break;
        case ISIL_CODEC_CHAN_DISCAED_FRAME:
            tve->discard = 1;
            set_tq_state_busy(tq);
            tck = tq->bufs[0];
            ret = tq->ops->buf_release(tq,tve,tck);
            if(ret)
                printk("discard frame ---release failed!\n");
            break;
        default:
            //ISIL_DBG(ISIL_DBG_ERR,"other cmd in %s\n",__FUNCTION__);
            if(!tve->tsd.ped){
                printk("the device has no driver ed!\n");
                ret = EFAULT;
                break;
            }
            ed = tve->tsd.ped;
            ret = ed->op->ioctl(ed,cmd,arg);
            break;
    }
    up(&tve->sem);
    //printk("ioctl out!\n");
    return ret;
}



/*user close(fd) will call this function, it just perform close*/
int  videoh264_en_release(struct file *f)
{
    int ret = 0;
    ed_tcb_t *veed;
    struct isil_video_en *tve = (struct isil_video_en *)f->private_data;
    //struct isil_h264_logic_encode_chan *driv = container_of(tve->tsd.ped,struct isil_h264_logic_encode_chan, opened_logic_chan_ed);

    tc_trace;
#ifdef CONFIG_SMP
    del_timer_sync(&tve->polling_timer);
#else
    del_timer(&tve->polling_timer);
#endif
	down(&tve->sem);
	tc_queue_reset(tve->tsd.queue);
	veed = tve->tsd.ped;
    ret = veed->op->close(veed);
    up(&tve->sem);
    f->private_data = NULL;        	
    return ret;
}



unsigned int videoh264_en_poll(struct file *file, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    struct isil_video_en *tve = (struct isil_video_en *)file->private_data;
    struct isil_h264_logic_encode_chan *driv = container_of(tve->tsd.ped,struct isil_h264_logic_encode_chan, opened_logic_chan_ed);
    isil_video_frame_tcb_queue_t *vfq = &driv->encode_frame_queue;


    down(&tve->sem);
    //if(gb_pf)
    //        dbg("poll--tve[%8p],thread[%d]\n",tve,current->pid);
    poll_wait(file,&tve->wait_poll,wait);
    if(vfq->op->get_curr_queue_entry_number(vfq))
        mask |= POLLIN | POLLRDNORM;
    up(&tve->sem);	
    return mask; 
}

int videoh264_en_init(struct isil_chip_device *tcd,struct cmd_arg *arg,struct isil_service_device **tsd)
{
    //int ret;
    struct isil_video_en *tve;

    tc_trace;
    tve = kzalloc(sizeof(*tve),GFP_KERNEL);
    if(NULL == tve)
        return -ENOMEM;
    dbg("init_tve[%8p]\n",tve);
    *tsd = &tve->tsd;

    tve->tsd.clean_up = tve_cleanup;
    tve->name = "tw5864-h264master_encoder";
    //tve->users = 1;
    //tve->state = NEED_INIT;
    //set_audio_param(&tve->para,&def_audio_parm_en);
    //init_MUTEX(&tve->sem);
    sema_init(&tve->sem, 1);
	init_timer(&tve->polling_timer);
    init_waitqueue_head(&tve->wait_poll);
    /*
       rz = request_tdm_resource(tr,TDM_SUB_EN_ORIGNAL,0);
       if(!rz)
       return -ENOMEM;

       reate_end_section(rz, , 0, -1, 0x8000, 4, NULL,tr);
       */
    tve->tsd.queue = kzalloc(sizeof(struct tc_queue),GFP_KERNEL);
    if(NULL == tve->tsd.queue){
        return -ENOMEM;
    }
    return tc_queue_init(tve->tsd.queue,&ve_queue_ops,NULL,512*1024,10,tve);
}

void videoh264_en_destory(struct isil_service_device *tsd)
{
    struct isil_video_en *tve;
    tc_trace;

    if(!tsd)
        return;
    tve = container_of(tsd,struct isil_video_en,tsd);
    if(tve->tsd.queue)
        kfree(tve->tsd.queue);
    kfree(tve);
}



int tc_video_en_read(char *page, char **start, off_t off,int count, int *eof, void *data)
{
    int len = 0;
    struct isil_dev_id  *id = (struct isil_dev_id  *)data;
    struct list_head  *p,*q,*tmp,*lh;
    struct isil_chip_device *tcd;
    struct isil_service_device *tsd;
    int type = ISIL_ENCODER - 1;


    lock_chipdev();
    len += sprintf(page + len,"%10s %10s %10s %10s %10s\n","req_time", "map_time", "rel_time", "min", "max");

    lh = get_chipdev_listhead();
    list_for_each(p,lh){
        tcd = container_of(p,struct isil_chip_device, list);
        tmp = &tcd->tsd_list[type];
        list_for_each(q,tmp){
            tsd = container_of(q,struct isil_service_device,list);
            if(epobj_euqal(&tsd->epobj,&id->epobj)){
#ifdef  STATIC_COUNTING
                len += sprintf(page + len, "%10u %10u %10u %10u %10u\n", tsd->b_itval.ic_req_time, tsd->b_itval.ic_map_time,
                        tsd->b_itval.ic_rel_time, tsd->b_itval.min, tsd->b_itval.max);
#endif
            }
        }
    }
    unlock_chipdev();

    return len;
}










struct tsd_file_operations    videoh264_en_fops = {
    .owner  = THIS_MODULE,
    .init   = videoh264_en_init,
    .destory= videoh264_en_destory,
    .open	= videoh264_en_open,
    .read	= videoh264_en_read,
    .write	= videoh264_en_write,
    .ioctl  = videoh264_en_ioctl,
    .release= videoh264_en_release,
    .poll = videoh264_en_poll,
};

struct isil_dev_id  isil_videoh264_en_id = {
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (STREAM_TYPE_MAJOR << 16) | CODEC_VIDEO_H264,
        .type = ISIL_ENCODER,
        .key = {0},
    },
    .version = 0,
};



struct isil_dev_id  isil_videoh264s_en_id = {
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (STREAM_TYPE_MINOR << 16)  | CODEC_VIDEO_H264,
        .type = ISIL_ENCODER,
        .key = {0},
    },
    .version = 0,
};


static struct isil_dev_table_id *tve_table = NULL;
static struct isil_dev_table_id *tve_sub_table = NULL;
//static isil_proc_register_s *tc_video = NULL;
//static isil_proc_register_s *tc_sub_video = NULL;


int isil_videoh264_en_init(void)
{
    int ret = 0;
    //struct isil_h264_logic_encode_chan  *h264 = (struct isil_h264_logic_encode_chan  *)data;
    //isil_chip_t *chip = container_of(h264,isil_chip_t,h264_master_encode_logic_chan);


    tve_table = kzalloc(sizeof(struct isil_dev_table_id),GFP_KERNEL);
    if(!tve_table){
        ISIL_DBG(ISIL_DBG_ERR,"no mem in %s\n",__FUNCTION__);
        goto err;	
    }
    INIT_LIST_HEAD(&tve_table->list);
    tve_table->tid = &isil_videoh264_en_id;
    tve_table->ops = &videoh264_en_fops;
    tve_table->para = NULL;//data;
    //ISIL_DBG_epobj_print(&tve_table->tid->epobj);
    ret = register_tc_device(tve_table);
    if(ret)
        goto err1;
    /*
       tc_video = kzalloc(sizeof(*tc_video),GFP_KERNEL);
       if(!tc_video)
       goto err1;
       strcpy(tc_video->name,"tc_video_en");
       tc_video->read = tc_video_en_read;
       tc_video->write = NULL;
       tc_video->private = &isil_videoh264_en_id;
       ret = isil_module_register(chip,tc_video);
       if(ret)
       goto err1;
       */                
    return ret;

err1:
    //if(tc_video)
    //        kfree(tc_video);
    isil_videoh264_en_remove();        
err:
    if(tve_table)
        kfree(tve_table);

    return ret;		
}

void isil_videoh264_en_remove(void)
{
    tc_trace;
    //if(tc_video && data)
    //        isil_module_unregister((isil_chip_t *)data,tc_video);
    if(tve_table)
        unregister_tc_device(tve_table);
    tve_table = NULL;
}

int isil_videoh264s_en_init(void)
{
    int ret = 0; 
    tve_sub_table = kzalloc(sizeof(struct isil_dev_table_id),GFP_KERNEL);
    if(!tve_sub_table){
        ISIL_DBG(ISIL_DBG_ERR,"no mem in %s\n",__FUNCTION__);
        return -ENOMEM;	
    }
    INIT_LIST_HEAD(&tve_sub_table->list);
    tve_sub_table->tid = &isil_videoh264s_en_id;
    tve_sub_table->ops = &videoh264_en_fops;
    tve_sub_table->para = NULL;//data;
    //ISIL_DBG_epobj_print(&tve_sub_table->tid->epobj);
    ret = register_tc_device(tve_sub_table);
    return ret;		
}
void isil_videoh264s_en_remove(void)
{
    if(tve_sub_table)
        unregister_tc_device(tve_sub_table);
    tve_sub_table = NULL;
}
