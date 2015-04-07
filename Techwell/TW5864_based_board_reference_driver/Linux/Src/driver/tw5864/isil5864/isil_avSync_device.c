#include	<isil5864/isil_common.h>

static ssize_t	avSync_device_read(struct file *file, char __user *data, size_t count, loff_t *ppos)
{
    isil_avSync_dev_t     *dev;
    avSync_frame_queue_t    *avSync_frame_queue;
    ssize_t ret = 0;
    int	end_flags=0;

    dev = (isil_avSync_dev_t*)file->private_data;
    if (dev == NULL) {
        printk("[%s]:video_dev is null\n", __FUNCTION__);
        return -ENODEV;
    }

    down(&dev->sem);
    if(atomic_read(&dev->opened_flags)) {
        avSync_frame_queue = &dev->avSync_frame_queue;
        if(avSync_frame_queue->curr_consumer != NULL){
            ret = avSync_frame_queue->curr_consumer->op->submit(avSync_frame_queue->curr_consumer, data, count, ppos, &end_flags, dev);
        } else {
            if(avSync_frame_queue->op->get_curr_queue_entry_number(avSync_frame_queue)){
                avSync_frame_queue->op->try_get_curr_consumer_from_queue(avSync_frame_queue);
            }
            if(avSync_frame_queue->curr_consumer != NULL){
                ret = avSync_frame_queue->curr_consumer->op->submit(avSync_frame_queue->curr_consumer, data, count, ppos, &end_flags, dev);
                //printk("%d: %d\n", __LINE__, ret);
            }
        }
        if(end_flags){
            avSync_frame_queue->op->release_curr_consumer(avSync_frame_queue, &dev->device_buf_pool);
        }
    } else {
        printk("%s:%s have not been opened\n", __FUNCTION__, dev->name);
    }
    up(&dev->sem);
    return ret;
}

static unsigned int avSync_device_poll(struct file *file, struct poll_table_struct *wait)
{
    isil_avSync_dev_t	*dev;
    unsigned int mask = 0;

    dev = (isil_avSync_dev_t*)file->private_data;
    if (dev == NULL) {
        printk("[%s]:video_dev is null\n", __FUNCTION__);
        return -ENODEV;
    }
    down(&dev->sem);
    if(atomic_read(&dev->opened_flags)) {
        poll_wait(file, &dev->wait_poll, wait);
        if(dev->avSync_frame_queue.op->get_curr_queue_entry_number(&dev->avSync_frame_queue)){
            mask = POLLIN | POLLRDNORM;
        }
    } else {
        printk("%s:%s have not been opened\n", __FUNCTION__, dev->name);
        mask = POLLOUT | POLLWRNORM | POLLIN | POLLRDNORM;
    }
    up(&dev->sem);
    return mask;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static long avSync_device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int avSync_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    isil_avSync_dev_t	*dev;
    int	ret = 0;

    dev = (isil_avSync_dev_t*)file->private_data;
    if (dev == NULL) {
        printk("[%s]:video_dev is null\n", __FUNCTION__);
        return -ENODEV;
    }
    ISIL_DBG(ISIL_DBG_DEBUG, "IO_TYPE '%c': NR %d\n", _IOC_TYPE(cmd), _IOC_NR(cmd));

    down(&dev->sem);
    if(atomic_read(&dev->opened_flags)) {
        ed_tcb_t *ed;
        switch(cmd) {
            case DVM_CODEC_GET_VIDEO_ENCODER_PARAM:
            case DVM_CODEC_SET_VIDEO_ENCODER_PARAM:
                if(dev->h264_sub_encode_driver) {
                    ed = &dev->h264_sub_encode_driver->opened_logic_chan_ed;
                    ret = ed->op->ioctl(ed, cmd, arg);
                }else{
                    ret = -ENODEV;
                }
                break;
            case ISIL_CHIP_AUDIO_ENCODE_PARAM_SET:
            case ISIL_CHIP_AUDIO_ENCODE_PARAM_GET:
                if(dev->audio_encode_driver) {
                    ed = &dev->audio_encode_driver->opened_logic_chan_ed;
                    ret = ed->op->ioctl(ed, cmd, arg);
                }else{
                    ret = -ENODEV;
                }
                break;
            case ISIL_CHIP_AUDIO_DECODE_PARAM_SET:
            case ISIL_CHIP_AUDIO_DECODE_PARAM_GET:
                if(dev->audio_decode_driver) {
                    ed = &dev->audio_decode_driver->opened_logic_chan_ed;
                    ret = ed->op->ioctl(ed, cmd, arg);
                }else{
                    ret = -ENODEV;
                }
                break;
            case ISIL_MJPEG_ENCODE_PARAM_SET:
            case ISIL_MJPEG_ENCODE_PARAM_GET:
                if(dev->jpeg_encode_driver) {
                    ed = &dev->jpeg_encode_driver->opened_logic_chan_ed;
                    ret = ed->op->ioctl(ed, cmd, arg);
                }else{
                    ret = -ENODEV;
                }
                break;
            default:
                //ret = -ENODEV;
                ret = 0;
        }
    } else {
        printk("%s:%s have not been opened\n", __FUNCTION__, dev->name);
    }
    up(&dev->sem);
    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static int avSync_device_release(struct file *file)
#else
static int avSync_device_release(struct inode *inode, struct file *file)
#endif
{
    isil_avSync_dev_t	*dev;

    dev = (isil_avSync_dev_t*)file->private_data;
    if (dev == NULL) {
        printk("[%s]:video_dev is null\n", __FUNCTION__);
        return -ENODEV;
    }
    down(&dev->sem);
    if(atomic_read(&dev->opened_flags)){
        ed_tcb_t *ed;
        if(dev->h264_master_encode_driver != NULL){
            ed = &dev->h264_master_encode_driver->opened_logic_chan_ed;
            printk("\n\n%s.%d: start close master video chan_%d\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
            ed->op->close(ed);
            printk("%s.%d: close master video chan_%d over\n\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
        }

        if(dev->h264_sub_encode_driver != NULL){
            ed = &dev->h264_sub_encode_driver->opened_logic_chan_ed;
            printk("\n\n%s.%d: start close sub video chan_%d\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
            ed->op->close(ed);
            printk("%s.%d: close sub video chan_%d over\n\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
        }

        if(dev->jpeg_encode_driver != NULL){
            ed = &dev->jpeg_encode_driver->opened_logic_chan_ed;
            printk("%s.%d: start close jpeg!\n", __FUNCTION__, __LINE__);
            ed->op->close(&dev->jpeg_encode_driver->opened_logic_chan_ed);
            printk("%s,%d:jpeg closed!\n", __FUNCTION__, __LINE__);
        }
        if(dev->audio_encode_driver != NULL){
            ed = &dev->audio_encode_driver->opened_logic_chan_ed;
            printk("\n\n%s.%d: start close audio encode chan_%d\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
            ed->op->close(ed);
            printk("%s.%d: close audio encode chan_%d over\n\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
        }
        if(dev->audio_decode_driver != NULL){
            ed = &dev->audio_decode_driver->opened_logic_chan_ed;
            printk("\n\n%s.%d: start close audio decode chan_%d\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
            ed->op->close(ed);
            printk("%s.%d: close audio decode chan_%d over\n\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
        }
        printk("%s,%d: release avSync frame queue!\n", __FUNCTION__, __LINE__);
        remove_avSync_frame_queue(&dev->avSync_frame_queue, &dev->device_buf_pool);
        file->private_data = NULL;
        atomic_set(&dev->opened_flags, 0);
        {
            printk("\n\n<<------%s.%d: close device_%d over------>>\n", __FUNCTION__, __LINE__, dev->dev_inode_id);
            if(dev->h264_master_encode_driver != NULL){
                ed = &dev->h264_master_encode_driver->opened_logic_chan_ed;
                printk("master video_packet_pool_tcb = %d\n", dev->h264_master_encode_driver->encode_chan_buf_pool.video_packet_pool_tcb.op->get_curr_pool_entry_number(&dev->h264_master_encode_driver->encode_chan_buf_pool.video_packet_pool_tcb));
                printk("master video_frame_pool_tcb = %d\n", dev->h264_master_encode_driver->encode_chan_buf_pool.video_frame_pool_tcb.op->get_curr_pool_entry_number(&dev->h264_master_encode_driver->encode_chan_buf_pool.video_frame_pool_tcb));
                printk("master state %d\n", ed->op->get_state(ed));
            }
            if(dev->h264_sub_encode_driver != NULL){
                ed = &dev->h264_sub_encode_driver->opened_logic_chan_ed;
                printk("sub video_packet_pool_tcb = %d\n", dev->h264_sub_encode_driver->encode_chan_buf_pool.video_packet_pool_tcb.op->get_curr_pool_entry_number(&dev->h264_sub_encode_driver->encode_chan_buf_pool.video_packet_pool_tcb));
                printk("sub video_frame_pool_tcb = %d\n", dev->h264_sub_encode_driver->encode_chan_buf_pool.video_frame_pool_tcb.op->get_curr_pool_entry_number(&dev->h264_sub_encode_driver->encode_chan_buf_pool.video_frame_pool_tcb));
                printk("sub state %d\n", ed->op->get_state(ed));        
            }
            if(dev->jpeg_encode_driver != NULL){

            }
            if(dev->audio_encode_driver != NULL){
                printk("encode audio_queue = %d\n", dev->audio_encode_driver->audio_packet_queue.op->get_curr_queue_entry_number(&dev->audio_encode_driver->audio_packet_queue));
                printk("encode audio_packet_pool_tcb = %d\n", dev->audio_encode_driver->audio_buf_pool.op->get_audio_section_tcb_pool_entry_number(&dev->audio_encode_driver->audio_buf_pool));
            }
            if(dev->audio_decode_driver != NULL){
                printk("decode audio_queue = %d\n", dev->audio_decode_driver->audio_packet_queue.op->get_curr_queue_entry_number(&dev->audio_decode_driver->audio_packet_queue));
                printk("decode audio_packet_pool_tcb = %d\n", dev->audio_decode_driver->audio_buf_pool.op->get_audio_section_tcb_pool_entry_number(&dev->audio_decode_driver->audio_buf_pool));
            }
            printk("\n");
        }
    } else {
        printk("%s:%s have not been opened\n", __FUNCTION__, dev->name);
    }
    up(&dev->sem);
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static int avSync_device_open(struct file *file)
{
    return 0;
}
#else
static int avSync_device_open(struct inode *inode, struct file *file)
{
    isil_avSync_dev_t	*dev;
    int minor = iminor(inode);

    ISIL_DBG(ISIL_DBG_INFO, "open minor = %d\n", minor);
    get_isil_avSync_device(&dev, minor);
    if (dev == NULL) {
        ISIL_DBG(ISIL_DBG_ERR, "video_dev is null\n");
        return -ENODEV;
    } else {
        down(&dev->sem);
        if(atomic_read(&dev->opened_flags)){
            printk("%s:%s have been opened\n", __FUNCTION__, dev->name);
        } else {
            ed_tcb_t *ed;
            atomic_set(&dev->opened_flags, 1);
            //if(minor == 0) 
            {
                if(dev->h264_master_encode_driver != NULL){
                    ed = &dev->h264_master_encode_driver->opened_logic_chan_ed;
                    ed->op->open(ed);
                }

                if(dev->h264_sub_encode_driver != NULL){
                    ed = &dev->h264_sub_encode_driver->opened_logic_chan_ed;
                    ed->op->open(ed);
                }

                if(dev->jpeg_encode_driver != NULL){
                    ed = &dev->jpeg_encode_driver->opened_logic_chan_ed;
                    ed->op->open(ed);
                }
                if(dev->audio_encode_driver != NULL){
                    ed = &dev->audio_encode_driver->opened_logic_chan_ed;
                    ed->op->open(ed);
                }
                if(dev->audio_decode_driver != NULL){
                    ed = &dev->audio_decode_driver->opened_logic_chan_ed;
                    ed->op->open(ed);
                }
            }

            file->private_data = dev;
        }
        up(&dev->sem);
    }

    ISIL_DBG(ISIL_DBG_INFO, "open successful\n");

    return 0;
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static const struct v4l2_file_operations avSync_device_fops = {
#else
static const struct file_operations avSync_device_fops = {
#endif
    .owner      = THIS_MODULE,
    .read       = avSync_device_read,
    .poll       = avSync_device_poll,
    .ioctl      = avSync_device_ioctl,
    .release    = avSync_device_release,
    .open       = avSync_device_open,
};

static void	v4l2device_release(struct video_device *vfd)
{
    printk("no support %s op\n", __FUNCTION__);
}

static int  video_device_init(isil_avSync_dev_t *dev)
{
    struct video_device *vfd;

    vfd = &dev->vfd;
    strncpy(vfd->name, dev->name, strlen(dev->name)+1);
    vfd->fops = &avSync_device_fops;
    vfd->minor = dev->dev_inode_id;
    vfd->release = v4l2device_release;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)
    vfd->type = VID_TYPE_CAPTURE;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 24)
    vfd->hardware   = 0;
#endif
    vfd->vidioc_querycap = NULL;
    vfd->vidioc_enum_fmt_cap = NULL;
    vfd->vidioc_g_fmt_cap = NULL;
    vfd->vidioc_try_fmt_cap = NULL;
    vfd->vidioc_s_fmt_cap = NULL;
    vfd->vidioc_reqbufs = NULL;
    vfd->vidioc_querybuf = NULL;
    vfd->vidioc_qbuf = NULL;
    vfd->vidioc_dqbuf = NULL;
    vfd->vidioc_s_std = NULL;
    vfd->vidioc_enum_input = NULL;
    vfd->vidioc_g_input = NULL;
    vfd->vidioc_s_input = NULL;
    vfd->vidioc_queryctrl = NULL;
    vfd->vidioc_g_ctrl = NULL;
    vfd->vidioc_s_ctrl = NULL;
    vfd->vidioc_streamon = NULL;
    vfd->vidioc_streamoff = NULL;
#ifdef CONFIG_VIDEO_V4L1_COMPAT
    vfd->vidiocgmbuf = NULL;
#endif //CONFIG_VIDEO_V4L1_COMPAT
#else
    vfd->vfl_type  = VFL_TYPE_GRABBER;
    vfd->ioctl_ops = NULL;
#endif //LINUX_VERSION_CODE
    return 0;
};

int init_isil_avSync_dev(isil_avSync_dev_t *dev, int bus_id, int chip_id, int inode_id, int id)
{
    int	ret = ISIL_ERR;
    if(dev != NULL){
        dev->bus_id = bus_id;
        dev->chip_id = chip_id;
        dev->dev_inode_id = inode_id;
        dev->id = id;
        sprintf(dev->name, "isil_avSync_dev_%d", dev->dev_inode_id);
        video_device_init(dev);
        //init_MUTEX(&dev->sem);
        sema_init(&dev->sem, 1);
        atomic_set(&dev->opened_flags, 0);
        init_waitqueue_head(&dev->wait_poll);

        dev->h264_master_encode_driver = NULL;
        dev->h264_sub_encode_driver = NULL;
        dev->jpeg_encode_driver = NULL;
        dev->audio_encode_driver = NULL;
        dev->audio_decode_driver = NULL;

        ret = init_avSync_frame_buf_pool(&dev->device_buf_pool);
        if(ret == ISIL_OK){
            init_avSync_frame_queue(&dev->avSync_frame_queue);
            ISIL_DBG(ISIL_DBG_DEBUG, "register video device minor %d\n", dev->dev_inode_id);
#if defined(X86_PLATFORM) || defined(HISILICON_PLATFORM)
            ret = 0;
#elif defined(POWERPC_PLATFORM)
            ret = video_register_device_index(&dev->vfd, VFL_TYPE_GRABBER, dev->dev_inode_id, dev->dev_inode_id);
#endif
            if(ret) {
                ISIL_DBG(ISIL_DBG_ERR, "register video device failed %d\n", ret);
            }
        }
    }
    return ret;
}

void    remove_isil_avSync_dev(isil_avSync_dev_t *dev)
{
    if(dev != NULL){
        if(dev->h264_master_encode_driver != NULL){
            remove_isil_h264_encode_chan(dev->h264_master_encode_driver);
        }
        if(dev->h264_sub_encode_driver != NULL){
            remove_isil_h264_encode_chan(dev->h264_sub_encode_driver);
        }
        if(dev->jpeg_encode_driver != NULL){
            remove_isil_jpeg_encode_chan(dev->jpeg_encode_driver);
        }
        if(dev->audio_encode_driver != NULL){
            remove_isil_audio_encode_chan(dev->audio_encode_driver);
        }
        remove_avSync_frame_queue(&dev->avSync_frame_queue, &dev->device_buf_pool);
        remove_avSync_frame_buf_pool(&dev->device_buf_pool);
#if defined(X86_PLATFORM) || defined(HISILICON_PLATFORM)
#else
        video_unregister_device(&dev->vfd);
#endif
    }
}

void	bind_av_device_and_driver(isil_avSync_dev_t *isil_device_chan, isil_h264_logic_encode_chan_t *h264_master_encode_logic_chan, isil_h264_logic_encode_chan_t *h264_sub_encode_logic_chan, isil_jpeg_logic_encode_chan_t *jpeg_logic_chan, isil_audio_driver_t *audio_encode_driver)
{
    if(isil_device_chan != NULL){
        isil_device_chan->h264_master_encode_driver = h264_master_encode_logic_chan;
        isil_device_chan->h264_sub_encode_driver = h264_sub_encode_logic_chan;
        isil_device_chan->jpeg_encode_driver = jpeg_logic_chan;
        isil_device_chan->audio_encode_driver = audio_encode_driver;
    }
    if(h264_master_encode_logic_chan != NULL){
        h264_master_encode_logic_chan->isil_device_chan = isil_device_chan;
    }
    if(h264_sub_encode_logic_chan != NULL){
        h264_sub_encode_logic_chan->isil_device_chan = isil_device_chan;
    }
    if(jpeg_logic_chan){
        jpeg_logic_chan->isil_device_chan = isil_device_chan;
    }
    if(audio_encode_driver != NULL){
        audio_encode_driver->isil_device_chan = isil_device_chan;
    }
}

void	unbind_av_device_and_driver(isil_avSync_dev_t *isil_device_chan, isil_h264_logic_encode_chan_t *h264_master_encode_logic_chan, isil_h264_logic_encode_chan_t *h264_sub_encode_logic_chan, isil_jpeg_logic_encode_chan_t *jpeg_logic_chan, isil_audio_driver_t *audio_encode_driver)
{
    if(isil_device_chan != NULL){
        isil_device_chan->h264_master_encode_driver = NULL;
        isil_device_chan->h264_sub_encode_driver = NULL;
        isil_device_chan->audio_encode_driver = NULL;
    }
    if(h264_master_encode_logic_chan != NULL){
        h264_master_encode_logic_chan->isil_device_chan = NULL;
    }
    if(h264_sub_encode_logic_chan != NULL){
        h264_sub_encode_logic_chan->isil_device_chan = NULL;
    }
    if(jpeg_logic_chan){
        jpeg_logic_chan->isil_device_chan = NULL;
    }
    if(audio_encode_driver != NULL){
        audio_encode_driver->isil_device_chan = NULL;
    }
}


