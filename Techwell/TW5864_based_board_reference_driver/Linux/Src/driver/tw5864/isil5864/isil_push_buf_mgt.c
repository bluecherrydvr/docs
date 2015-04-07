#include	<isil5864/isil_common.h>

static int push_proc_read(struct seq_file *seq, void *data)
{
    isil_chip_t *chip;
    int i;
    push_buf_pool_t *pool;

    chip = (isil_chip_t *)seq->private;
    pool = &chip->push_buf_pool;

    seq_printf(seq, "%8s %8s %8s %8s %8s\n", "type", "index", "addr", "bit map", "format");

    for (i = 0; i < PREVIEW_BUF_CNT; i++) {
        push_prev_buffer_t *buf = &pool->preview_bufs[i];
        seq_printf(seq, "%8s %8d %8lx %8x", "PREVIEW", i, buf->buf_addr, buf->bit_map);
        switch (buf->fmt) {
            case ISIL_VIDEO_SIZE_D1:
                seq_printf(seq, "%8s\n", "D1");
                break;
            case ISIL_VIDEO_SIZE_CIF:
                seq_printf(seq, "%8s\n", "CIF");
                break;
            case ISIL_VIDEO_SIZE_QCIF:
                seq_printf(seq, "%8s\n", "QCIF");
                break;
            case ISIL_VIDEO_SIZE_USER:
                seq_printf(seq, "%8s\n", "USER");
                break;	
            default:
                seq_printf(seq, "invalid format type %d\n", buf->fmt);
        }
    }

    seq_printf(seq, "%8s %8s %8s %8s\n", "type", "index", "addr", "state");
    for (i = 0; i < H264_BUF_CNT; i++) {
        push_buffer_t *buf = &pool->h264_bufs[i];
        seq_printf(seq, "%8s %8d %8lx %8x\n", "H264", i, buf->buf_addr, buf->state);
    }

    for (i = 0; i < AUDIO_BUF_CNT; i++) {
        push_buffer_t *buf = &pool->audio_bufs[i];
        seq_printf(seq, "%8s %8d %8lx %8x\n", "AUDIO", i, buf->buf_addr, buf->state);
    }

    for (i = 0; i < MV_BUF_CNT; i++) {
        push_buffer_t *buf = &pool->mv_bufs[i];
        seq_printf(seq, "%8s %8d %8lx %8x\n", "MV", i, buf->buf_addr, buf->state);
    }

    for (i = 0; i < JPEG_BUF_CNT; i++) {
        push_buffer_t *buf = &pool->jpeg_bufs[i];
        seq_printf(seq, "%8s %8d %8lx %8x\n", "MJPEG", i, buf->buf_addr, buf->state);
    }

    return 0;
}

static int push_proc_write(struct file *file, const char __user *buffer,
        size_t count, loff_t *offset)
{
    return count;
}

static void audio_push_buf_release(push_buf_pool_t *pool)
{
    int i;

    pool->audio_free_cnt = 0;
    for (i = 0; i < AUDIO_BUF_CNT; i++) {
        if (pool->audio_bufs[i].buf_addr)
            free_pages(pool->audio_bufs[i].buf_addr, AUDIO_BUF_ORDER);
        pool->audio_bufs[i].state = BUF_STATE_BUSY;
    }
}

static int audio_push_buf_init(push_buf_pool_t *pool)
{
    int i;

    spin_lock_init(&pool->audio_queue_lock);
    for (i = 0; i < AUDIO_BUF_CNT; i++) {
        if ((pool->audio_bufs[i].buf_addr = __get_free_pages(GFP_KERNEL, AUDIO_BUF_ORDER)) == 0) {
            printk("audio can't get push buffer\n");
            goto out;
        }
        pool->audio_bufs[i].state = BUF_STATE_FREE;
        pool->audio_free_cnt++;
        //printk("\naudio init push buf%lx\n", pool->audio_bufs[i].buf_addr);
    }
    return 0;

out:
    audio_push_buf_release(pool);
    return -1;
}

static void h264_push_buf_release(push_buf_pool_t *pool)
{
    int i;

    pool->h264_free_cnt = 0;
    for (i = 0; i < H264_BUF_CNT; i++) {
        if (pool->h264_bufs[i].buf_addr)
            free_pages(pool->h264_bufs[i].buf_addr, H264_BUF_ORDER);
        pool->h264_bufs[i].state = BUF_STATE_BUSY;
    }
} 

static int h264_push_buf_init(push_buf_pool_t *pool)
{
    int i;

    spin_lock_init(&pool->h264_queue_lock);
    for (i = 0; i < H264_BUF_CNT; i++) {
        if ((pool->h264_bufs[i].buf_addr = __get_free_pages(GFP_KERNEL, H264_BUF_ORDER)) == 0) {
            printk("h264 can't get push buffer\n");
            goto out;
        }
        pool->h264_bufs[i].state = BUF_STATE_FREE;
        pool->h264_free_cnt++;
        //printk("\nh264 init push buf%lx\n", pool->h264_bufs[i].buf_addr);
    }
    return 0;

out:
    h264_push_buf_release(pool);
    return -1;
} 

static void mv_push_buf_release(push_buf_pool_t *pool)
{
    int i;

    pool->mv_free_cnt = 0;
    for (i = 0; i < MV_BUF_CNT; i++) {
        if (pool->mv_bufs[i].buf_addr)
            free_pages(pool->mv_bufs[i].buf_addr, MV_BUF_ORDER);
        pool->mv_bufs[i].state = BUF_STATE_BUSY;
    }
} 

static int mv_push_buf_init(push_buf_pool_t *pool)
{
    int i;

    spin_lock_init(&pool->mv_queue_lock);
    for (i = 0; i < MV_BUF_CNT; i++) {
        if ((pool->mv_bufs[i].buf_addr = __get_free_pages(GFP_KERNEL, MV_BUF_ORDER)) == 0) {
            printk("mv can't get push buffer\n");
            goto out;
        }
        pool->mv_bufs[i].state = BUF_STATE_FREE;
        pool->mv_free_cnt++;
        //printk("\nmv init push buf %lx\n", pool->mv_bufs[i].buf_addr);
    }
    return 0;

out:
    mv_push_buf_release(pool);
    return -1;
} 

static void jpeg_push_buf_release(push_buf_pool_t *pool)
{
    int i;

    pool->jpeg_free_cnt = 0;
    for (i = 0; i < JPEG_BUF_CNT; i++) {
        if (pool->jpeg_bufs[i].buf_addr)
            free_pages(pool->jpeg_bufs[i].buf_addr, JPEG_BUF_ORDER);
        pool->jpeg_bufs[i].state = BUF_STATE_BUSY;
    }
} 

static int jpeg_push_buf_init(push_buf_pool_t *pool)
{
    int i;

    spin_lock_init(&pool->jpeg_queue_lock);

    for (i = 0; i < JPEG_BUF_CNT; i++) {
        if ((pool->jpeg_bufs[i].buf_addr = __get_free_pages(GFP_KERNEL, JPEG_BUF_ORDER)) == 0) {
            printk("jpeg can't get push buffer\n");
            goto out;
        }
        pool->jpeg_bufs[i].state = BUF_STATE_FREE;
        pool->jpeg_free_cnt++;
        //printk("\njpeg init push buf %lx\n", pool->jpeg_bufs[i].buf_addr);
    }
    return 0;

out:
    jpeg_push_buf_release(pool);
    return -1;
} 

static void preview_push_buf_release(push_buf_pool_t *pool)
{
    int i;

    pool->preview_free_cnt = 0;
    for (i = 0; i < PREVIEW_BUF_CNT; i++) {
        if (pool->preview_bufs[i].buf_addr) {
            free_pages(pool->preview_bufs[i].buf_addr, PREVIEW_BUF_ORDER);
        }
    }
} 

static int preview_push_buf_init(push_buf_pool_t *pool)
{
    int i;

    spin_lock_init(&pool->preview_queue_lock);

    for (i = 0; i < PREVIEW_BUF_CNT; i++) {
        if ((pool->preview_bufs[i].buf_addr = __get_free_pages(GFP_KERNEL, PREVIEW_BUF_ORDER)) == 0) {
            printk("preview can't get push buffer\n");
            goto out;
        }
        pool->preview_bufs[i].bit_map = 0;
        pool->preview_bufs[i].fmt = ISIL_VIDEO_SIZE_USER;
        pool->preview_free_cnt++;
    }
    return 0;

out:
    preview_push_buf_release(pool);
    return -1;
} 

int push_buf_pool_init(isil_chip_t *chip)
{
    push_buf_pool_t *pool = &chip->push_buf_pool;
    int ret = 0;
    isil_proc_register_s *push_buf_mgt_proc;

    memset(pool, 0, sizeof(push_buf_pool_t));
    ret |= h264_push_buf_init(pool);
    ret |= audio_push_buf_init(pool);
    ret |= preview_push_buf_init(pool);
    ret |= mv_push_buf_init(pool);
    ret |= jpeg_push_buf_init(pool);

    push_buf_mgt_proc = &chip->push_buf_mgt_proc;
    if(!push_buf_mgt_proc->entry) {
        strcpy(push_buf_mgt_proc->name, "push_buf_mgt");
        push_buf_mgt_proc->read  = push_proc_read;
        push_buf_mgt_proc->write = push_proc_write;
        push_buf_mgt_proc->private = chip;
        isil_module_register(chip, push_buf_mgt_proc);
    }

    return ret;
}

void push_buf_pool_release(isil_chip_t *chip)
{
    push_buf_pool_t *pool = &chip->push_buf_pool;

    h264_push_buf_release(pool);
    audio_push_buf_release(pool);
    preview_push_buf_release(pool);
    mv_push_buf_release(pool);
    jpeg_push_buf_release(pool);

    isil_module_unregister(chip, &chip->push_buf_mgt_proc);
}

void *get_push_buf(int type, int fmt, isil_chip_t *chip)
{
    push_buf_pool_t *pool = &chip->push_buf_pool;
    unsigned long lock_flags;
    int i, j;

    switch (type) {
        case BUF_TYPE_H264:
            spin_lock_irqsave(&pool->h264_queue_lock, lock_flags);
            if (pool->h264_free_cnt == 0) {
                printk("no free h264 buf.\n");
                spin_unlock_irqrestore(&pool->h264_queue_lock, lock_flags);
                goto err;
            }

            for (i = 0; i < H264_BUF_CNT; i++) {
                if (pool->h264_bufs[i].state == BUF_STATE_FREE)
                    break;
            }
            if (i == H264_BUF_CNT) {
                //printk("no free h264 buf, free_cnt is wrong.\n");
                spin_unlock_irqrestore(&pool->h264_queue_lock, lock_flags);
                goto err;
            }

            pool->h264_bufs[i].state = BUF_STATE_BUSY;
            pool->h264_free_cnt--;
            spin_unlock_irqrestore(&pool->h264_queue_lock, lock_flags);
            return (void *)pool->h264_bufs[i].buf_addr;

        case BUF_TYPE_PREVIEW:
            spin_lock_irqsave(&pool->preview_queue_lock, lock_flags);
            for (i = 0; i < PREVIEW_BUF_CNT; i++) {
                if (pool->preview_bufs[i].fmt == fmt) {
                    push_prev_buffer_t *buf = &pool->preview_bufs[i];
                    switch (fmt) {
                        case ISIL_VIDEO_SIZE_D1:
                            break;

                        case ISIL_VIDEO_SIZE_CIF:
                            for (j = 0; j < CIF_CNT_PER_BUF; j++) {
                                if (!(buf->bit_map & (0xf << 4*j))) {
                                    buf->bit_map |= 0xf << 4*j;
                                    spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
                                    return (void *)(buf->buf_addr + j*CIF_PUSH_SIZE);
                                }
                            }
                            break;

                        case ISIL_VIDEO_SIZE_QCIF:
                            for (j = 0; j < QCIF_CNT_PER_BUF; j++) {
                                if (!(buf->bit_map & (0x1 << j))) {
                                    buf->bit_map |= 0x1 << j;
                                    spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
                                    return (void *)(buf->buf_addr + j*QCIF_PUSH_SIZE);
                                }
                            }
                            break;

                        default:
                            ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
                    }				
                }
            }

            for (i = 0; i < PREVIEW_BUF_CNT; i++) {
                if (pool->preview_bufs[i].fmt == ISIL_VIDEO_SIZE_USER) {
                    push_prev_buffer_t *buf = &pool->preview_bufs[i];
                    switch (fmt) {
                        case ISIL_VIDEO_SIZE_D1:
                            buf->fmt = fmt;
                            buf->bit_map = 0xffff;
                            pool->preview_free_cnt--;
                            spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
                            return (void *)buf->buf_addr;

                        case ISIL_VIDEO_SIZE_CIF:
                            buf->fmt = fmt;
                            buf->bit_map = 0xf;
                            pool->preview_free_cnt--;
                            spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
                            return (void *)buf->buf_addr;

                        case ISIL_VIDEO_SIZE_QCIF:
                            buf->fmt = fmt;
                            buf->bit_map = 0x1;
                            pool->preview_free_cnt--;
                            spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
                            return (void *)buf->buf_addr;

                        default:
                            ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
                    }				
                }
            }
            spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
            goto err;

        case BUF_TYPE_AUDIO:
            spin_lock_irqsave(&pool->audio_queue_lock, lock_flags);
            if (pool->audio_free_cnt == 0) {
                printk("no free audio buf.\n");
                spin_unlock_irqrestore(&pool->audio_queue_lock, lock_flags);
                goto err;
            }

            for (i = 0; i < AUDIO_BUF_CNT; i++) {
                if (pool->audio_bufs[i].state == BUF_STATE_FREE)
                    break;
            }
            if (i == AUDIO_BUF_CNT) {
                printk("no free h264 buf, free_cnt is wrong.\n");
                spin_unlock_irqrestore(&pool->audio_queue_lock, lock_flags);
                goto err;
            }

            pool->audio_bufs[i].state = BUF_STATE_BUSY;
            pool->audio_free_cnt--;
            spin_unlock_irqrestore(&pool->audio_queue_lock, lock_flags);
            return (void *)pool->audio_bufs[i].buf_addr;

        case BUF_TYPE_MV:
            spin_lock_irqsave(&pool->mv_queue_lock, lock_flags);
            if (pool->mv_free_cnt == 0) {
                printk("no free mv buf.\n");
                spin_unlock_irqrestore(&pool->mv_queue_lock, lock_flags);
                goto err;
            }

            for (i = 0; i < MV_BUF_CNT; i++) {
                if (pool->mv_bufs[i].state == BUF_STATE_FREE)
                    break;
            }
            if (i == MV_BUF_CNT) {
                printk("no free mv buf, free_cnt is wrong.\n");
                spin_unlock_irqrestore(&pool->mv_queue_lock, lock_flags);
                goto err;
            }

            pool->mv_bufs[i].state = BUF_STATE_BUSY;
            pool->mv_free_cnt--;
            spin_unlock_irqrestore(&pool->mv_queue_lock, lock_flags);
            return (void *)pool->mv_bufs[i].buf_addr;

        case BUF_TYPE_JPEG:
            spin_lock_irqsave(&pool->jpeg_queue_lock, lock_flags);
            if (pool->jpeg_free_cnt == 0) {
                printk("no free jpeg buf.\n");
                spin_unlock_irqrestore(&pool->jpeg_queue_lock, lock_flags);
                goto err;
            }

            for (i = 0; i < JPEG_BUF_CNT; i++) {
                if (pool->jpeg_bufs[i].state == BUF_STATE_FREE)
                    break;
            }
            if (i == JPEG_BUF_CNT) {
                printk("no free jpeg buf, free_cnt is wrong.\n");
                spin_unlock_irqrestore(&pool->jpeg_queue_lock, lock_flags);
                goto err;
            }

            pool->jpeg_bufs[i].state = BUF_STATE_BUSY;
            pool->jpeg_free_cnt--;
            spin_unlock_irqrestore(&pool->jpeg_queue_lock, lock_flags);
            return (void *)pool->jpeg_bufs[i].buf_addr;

        default:
            printk("get push buf, type err\n");
            goto err;
    }

err:
    return NULL;
}

void release_push_buf(int type, int fmt, void *p, isil_chip_t *chip)
{
    push_buf_pool_t *pool = &chip->push_buf_pool;
    unsigned long lock_flags;
    unsigned long addr = (unsigned long)p;
    int i, j;

    switch (type) {
        case BUF_TYPE_H264:
            spin_lock_irqsave(&pool->h264_queue_lock, lock_flags);
            for (i = 0; i < H264_BUF_CNT; i++) {
                if (pool->h264_bufs[i].buf_addr == addr)
                    break;
            }
            if (i == H264_BUF_CNT) {
                printk("can't find right h264 buf %p, check it out.\n", p);
                spin_unlock_irqrestore(&pool->h264_queue_lock, lock_flags);
                return;
            }			

            if (pool->h264_bufs[i].state != BUF_STATE_BUSY)
                printk("buf state isn't right, check it out.\n");
            pool->h264_bufs[i].state = BUF_STATE_FREE;
            pool->h264_free_cnt++;
            spin_unlock_irqrestore(&pool->h264_queue_lock, lock_flags);
            break;

        case BUF_TYPE_PREVIEW:
            spin_lock_irqsave(&pool->preview_queue_lock, lock_flags);
            for (i = 0; i < PREVIEW_BUF_CNT; i++) {
                if (pool->preview_bufs[i].fmt == fmt) {
                    push_prev_buffer_t *buf = &pool->preview_bufs[i];
                    switch (fmt) {
                        case ISIL_VIDEO_SIZE_D1:
                            if (buf->buf_addr == addr) {
                                if (buf->bit_map != 0xffff)
                                    printk("buf bit_map %x isn't right, check it out.\n", buf->bit_map);
                                buf->fmt = ISIL_VIDEO_SIZE_USER;
                                buf->bit_map = 0;
                                pool->preview_free_cnt++;
                                spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
                                return;
                            }				
                            break;

                        case ISIL_VIDEO_SIZE_CIF:
                            for (j = 0; j < CIF_CNT_PER_BUF; j++) {
                                if (buf->buf_addr + j*CIF_PUSH_SIZE == addr)
                                    break;
                            }
                            if (j == CIF_CNT_PER_BUF)
                                break;

                            if (!(buf->bit_map & (0xf << 4*j)))
                                printk("buf bit_map %x isn't right, j=%d.\n", buf->bit_map, j);
                            buf->bit_map &= ~(0xf << 4*j);
                            if (!(buf->bit_map & MASK_LOW16)) {
                                buf->fmt = ISIL_VIDEO_SIZE_USER;
                                buf->bit_map = 0;
                                pool->preview_free_cnt++;
                            }
                            spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
                            return;

                        case ISIL_VIDEO_SIZE_QCIF:
                            for (j = 0; j < QCIF_CNT_PER_BUF; j++) {
                                if (buf->buf_addr + j*QCIF_PUSH_SIZE == addr)
                                    break;
                            }
                            if (j == QCIF_CNT_PER_BUF)
                                break;

                            if (!(buf->bit_map & (0x1 << j)))
                                printk("buf bit_map %x isn't right, j=%d.\n", buf->bit_map, j);
                            buf->bit_map &= ~(0x1 << j);
                            if (!(buf->bit_map & MASK_LOW16)) {
                                buf->fmt = ISIL_VIDEO_SIZE_USER;
                                buf->bit_map = 0;
                                pool->preview_free_cnt++;
                            }
                            spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
                            return;

                        default:
                            ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
                    }				
                }
            }
            printk("can't find right fmt %d buf %p, check it out.\n", fmt, p);
            spin_unlock_irqrestore(&pool->preview_queue_lock, lock_flags);
            break;

        case BUF_TYPE_AUDIO:
            spin_lock_irqsave(&pool->audio_queue_lock, lock_flags);
            for (i = 0; i < AUDIO_BUF_CNT; i++) {
                if (pool->audio_bufs[i].buf_addr == addr)
                    break;
            }
            if (i == AUDIO_BUF_CNT) {
                printk("can't find right audio buf %p, check it out.\n", p);
                spin_unlock_irqrestore(&pool->audio_queue_lock, lock_flags);
                return;
            }			

            if (pool->audio_bufs[i].state != BUF_STATE_BUSY)
                printk("buf state isn't right, check it out.\n");
            pool->audio_bufs[i].state = BUF_STATE_FREE;
            pool->audio_free_cnt++;
            spin_unlock_irqrestore(&pool->audio_queue_lock, lock_flags);
            break;

        case BUF_TYPE_MV:
            spin_lock_irqsave(&pool->mv_queue_lock, lock_flags);
            for (i = 0; i < MV_BUF_CNT; i++) {
                if (pool->mv_bufs[i].buf_addr == addr)
                    break;
            }
            if (i == MV_BUF_CNT) {
                printk("can't find right mv buf %p, check it out.\n", p);
                spin_unlock_irqrestore(&pool->mv_queue_lock, lock_flags);
                return;
            }			

            if (pool->mv_bufs[i].state != BUF_STATE_BUSY)
                printk("buf state isn't right, check it out.\n");
            pool->mv_bufs[i].state = BUF_STATE_FREE;
            pool->mv_free_cnt++;
            spin_unlock_irqrestore(&pool->mv_queue_lock, lock_flags);
            break;

        case BUF_TYPE_JPEG:
            spin_lock_irqsave(&pool->jpeg_queue_lock, lock_flags);
            for (i = 0; i < JPEG_BUF_CNT; i++) {
                if (pool->jpeg_bufs[i].buf_addr == addr)
                    break;
            }
            if (i == JPEG_BUF_CNT) {
                printk("can't find right jpeg buf %p, check it out.\n", p);
                spin_unlock_irqrestore(&pool->jpeg_queue_lock, lock_flags);
                return;
            }			

            if (pool->jpeg_bufs[i].state != BUF_STATE_BUSY)
                printk("jpeg buf state isn't right, check it out.\n");
            pool->jpeg_bufs[i].state = BUF_STATE_FREE;
            pool->jpeg_free_cnt++;
            spin_unlock_irqrestore(&pool->jpeg_queue_lock, lock_flags);
            break;

        default:
            printk("get push buf, type err\n");
    }
}


