#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/poll.h>

#include "solo6010.h"

#define SYNC_TIME_MARGIN		10
#define MAX_IMAGE_SIZE			(512<<10)

struct EXT_BUFFER
{
	u32 addr;

	u32 channel;
	u32 size;
	u32 usec;
	u32 sec;
};

struct SOLO6010_DEC
{
	struct SOLO6010 *solo6010;
	solo6010_device_t *device;

	wait_queue_head_t wq;

	unsigned int used;

	struct EXT_BUFFER *ext_buf;
	unsigned int nr_ext_buf;
	unsigned int ext_buf_start;
	unsigned int ext_buf_end;

	unsigned long dma_buf;
	unsigned long dma_buf_phys;

	struct EXT_BUFFER *cur_buf;
	struct EXT_BUFFER *rdy_buf;

	unsigned int time_diff_sec;
	unsigned int time_diff_usec;
};

static void solo6010_mp4d_cnofig(struct SOLO6010 *solo6010)
{
	solo6010_reg_write(solo6010, REG_VD_CFG0,
		VD_CFG_BUSY_WIAT_CODE |
		VD_CFG_BUSY_WIAT_REF |
		VD_CFG_BUSY_WIAT_RES |
		VD_CFG_BUSY_WIAT_MS |
//		VD_CFG_USER_PAGE_CTRL |
//		VD_CFG_SINGLE_MODE |
		VD_CFG_LITTLE_ENDIAN |
		VD_CFG_ERR_LOCK |
		VD_CFG_ERR_INT_ENA |
		VD_CFG_TIME_WIDTH(15) |
		VD_CFG_DCT_INTERVAL(36/solo6010->decoder_ability));

	solo6010_reg_write(solo6010, REG_VD_DEINTERLACE, VD_DEINTERLACE_THRESHOLD(100) | VD_DEINTERLACE_EDGE_VALUE(5));

	solo6010_reg_write(solo6010, REG_VD_CFG1, solo6010->dec2live_ext_addr>>16);
}

unsigned int change_decoder_ability(struct SOLO6010 *solo6010, unsigned int ability)
{
	if(ability > SOLO6010_CODEC_ABILITY_MAX)
		ability = SOLO6010_CODEC_ABILITY_MAX;

	if(ability < SOLO6010_CODEC_ABILITY_MIN)
		ability = SOLO6010_CODEC_ABILITY_MIN;

	solo6010->decoder_ability = ability;

	solo6010_reg_write(solo6010, REG_VD_CFG0,
		VD_CFG_BUSY_WIAT_CODE |
		VD_CFG_BUSY_WIAT_REF |
		VD_CFG_BUSY_WIAT_RES |
		VD_CFG_BUSY_WIAT_MS |
//		VD_CFG_USER_PAGE_CTRL |
//		VD_CFG_SINGLE_MODE |
		VD_CFG_LITTLE_ENDIAN |
		VD_CFG_ERR_LOCK |
		VD_CFG_ERR_INT_ENA |
		VD_CFG_TIME_WIDTH(15) |
		VD_CFG_DCT_INTERVAL(36/solo6010->decoder_ability));

	dbg_msg("decoder ability : %d\n", solo6010->decoder_ability);

	return solo6010->decoder_ability;
}

#define reset_time_diff(dec, buf, chip_sec, chip_usec)		\
do{															\
	dec->time_diff_sec = chip_sec - 1 - buf->sec;			\
	dec->time_diff_usec = chip_usec + 1000000 - buf->usec;	\
}while(0)

int solo6010_decoder_interrupt(struct SOLO6010 *solo6010)
{
	struct SOLO6010_DEC *dec;
	u32 status0;
	u32 status1;
	struct EXT_BUFFER *ext_buf;

	dec = solo6010->dec;

	status0 = solo6010_reg_read(solo6010, REG_VD_STATUS0);
	status1 = solo6010_reg_read(solo6010, REG_VD_STATUS1);

	if(status0 & VD_STATUS0_INTR_ERR)
	{
		if(dec->cur_buf)
			dbg_msg("decode error! (ch%d %08x:%08x size:%05x(%05x))\n",
			dec->cur_buf->channel, dec->cur_buf->sec, dec->cur_buf->usec, dec->cur_buf->size, status1 & 0x000fffff);
		else
			dbg_msg("decode error!\n");
	}

	solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_DECODER);

	dec->cur_buf = dec->rdy_buf;

	if(dec->ext_buf_start == dec->ext_buf_end)
	{
		dec->rdy_buf = NULL;
		return 0;
	}

	ext_buf = &dec->ext_buf[dec->ext_buf_start];

	if(ext_buf->sec || ext_buf->usec)
	{
		u32 chip_sec;
		u32 chip_usec;
		u32 dec_sec;
		u32 dec_usec;

		chip_sec = solo6010_reg_read(solo6010, REG_TIMER_SEC);
		chip_usec = solo6010_reg_read(solo6010, REG_TIMER_USEC);

		dec_sec = chip_sec - 2 - dec->time_diff_sec;
		dec_usec = chip_usec + 2000000 - dec->time_diff_usec;

		dec_sec += dec_usec/1000000;
		dec_usec = dec_usec%1000000;

		if(ext_buf->sec > dec_sec)
		{
			if(ext_buf->sec > (dec_sec + SYNC_TIME_MARGIN))
				reset_time_diff(dec, ext_buf, chip_sec, chip_usec);
			return 0;
		}
		else if((ext_buf->sec == dec_sec) && (ext_buf->usec > dec_usec))
			return 0;

		if((ext_buf->sec + SYNC_TIME_MARGIN) < dec_sec)
		{
			reset_time_diff(dec, ext_buf, chip_sec, chip_usec);
		}
	}

	solo6010_reg_write(solo6010, REG_VD_CODE_ADR, ext_buf->addr);
	solo6010_reg_write(solo6010, REG_VD_CTRL, VD_OPER_ON | VD_MAX_ITEM(1));
	dec->rdy_buf = ext_buf;

	dec->ext_buf_start = (dec->ext_buf_start + 1) % dec->nr_ext_buf;
	wake_up(&dec->wq);

	return 0;
}

static loff_t fops_llseek(struct file *filp, loff_t off, int whence)
{
	return -EINVAL;
}

static unsigned int fops_poll(struct file *filp, poll_table *wait)
{
	struct SOLO6010_DEC *dec;
	unsigned int mask = 0;

	dec = filp->private_data;

	poll_wait(filp, &dec->wq, wait);
	if(((dec->nr_ext_buf + dec->ext_buf_end - dec->ext_buf_start) % dec->nr_ext_buf) <= (dec->nr_ext_buf - 3))
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;

	return mask;
}

static ssize_t fops_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6010_DEC *dec;
    struct SOLO6010 *solo6010;
    int size = 0x240000;

    dec = filp->private_data;
    solo6010 = dec->solo6010;

    solo6010_dma_user(solo6010, 1, 1, buf, solo6010->dec2live_ext_addr, size);

    return size;
}

static ssize_t fops_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6010_DEC *dec;
	u8 *dec_index;
	struct EXT_BUFFER *ext_buf;
	unsigned int channel;
	unsigned int ref_addr;
	int ret;

	dec = filp->private_data;

	while(((dec->nr_ext_buf + dec->ext_buf_end - dec->ext_buf_start) % dec->nr_ext_buf) > (dec->nr_ext_buf - 4))
	{
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(dec->wq, ((dec->nr_ext_buf + dec->ext_buf_end - dec->ext_buf_start) % dec->nr_ext_buf) <= (dec->nr_ext_buf - 4));
		if(ret)
			return ret;
	}

	if(count < 64)
		return -1;

	if(copy_from_user((char *)dec->dma_buf, buf, 64))
		return -EFAULT;

	dec_index = (u8 *)dec->dma_buf;
	ext_buf = dec->ext_buf + dec->ext_buf_end;

	channel = dec_index[3] & 0x0f;
	ref_addr = (dec->solo6010->dref_ext_addr + (channel * 0x00140000))>>16;
	dec_index[8] = ref_addr % 256;
	dec_index[9] = ref_addr / 256;

	solo6010_p2m_dma(dec->solo6010, 1, 1, (void *)dec->dma_buf, ext_buf->addr, 64);
	solo6010_dma_user(dec->solo6010, 1, 1, buf + 64, ext_buf->addr + 64, (count - 64 + 7) & (~7));

	ext_buf->channel = channel;

	ext_buf->size = dec_index[2] & 0x0f;
	ext_buf->size = (ext_buf->size<<8) + dec_index[1];
	ext_buf->size = (ext_buf->size<<8) + dec_index[0];

	ext_buf->sec = dec_index[23];
	ext_buf->sec = (ext_buf->sec<<8) + dec_index[22];
	ext_buf->sec = (ext_buf->sec<<8) + dec_index[21];
	ext_buf->sec = (ext_buf->sec<<8) + dec_index[20];

	ext_buf->usec = dec_index[27];
	ext_buf->usec = (ext_buf->usec<<8) + dec_index[26];
	ext_buf->usec = (ext_buf->usec<<8) + dec_index[25];
	ext_buf->usec = (ext_buf->usec<<8) + dec_index[24];

	dec->ext_buf_end = (dec->ext_buf_end + 1) % dec->nr_ext_buf;

	return count;
}

static int fops_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	return -ENOTTY;
}

static int fops_open(struct inode *inode, struct file *filp)
{
	struct SOLO6010_DEC *dec;

	dec = (struct SOLO6010_DEC *)solo6010_device_data_get(inode_to_solo6010_device(inode));
	if(!dec)
		return -ENODEV;

	if(dec->used)
		return -EBUSY;

	dec->used = 1;

	dec->dma_buf = __get_free_pages(GFP_KERNEL, 1);
	if(!dec->dma_buf)
	{
		dec->used = 0;
		return -ENOMEM;
	}

	filp->private_data = dec;

	solo6010_enable_interrupt(dec->solo6010, INTR_DECODER);
	solo6010_reg_write(dec->solo6010, REG_VD_CTRL, VD_OPER_ON);

	return 0;
}

static int fops_release(struct inode *inode, struct file *filp)
{
	struct SOLO6010_DEC *dec;

	dec = filp->private_data;

	solo6010_reg_write(dec->solo6010, REG_VD_CTRL, 0);
	solo6010_disable_interrupt(dec->solo6010, INTR_DECODER);

	dec->ext_buf_end = dec->ext_buf_start;

	free_pages(dec->dma_buf, 1);

	dec->used = 0;

	return 0;
}

static struct file_operations fops = {
	.owner =    THIS_MODULE,
	.llseek =   fops_llseek,
	.poll =		fops_poll,
	.read =     fops_read,
	.write =    fops_write,
	.ioctl =    fops_ioctl,
	.open =     fops_open,
	.release =  fops_release,
};

int solo6010_dec_init(struct SOLO6010 *solo6010)
{
	struct SOLO6010_DEC *dec;
	int i;

	dec = (struct SOLO6010_DEC *)kmalloc(sizeof(struct SOLO6010_DEC), GFP_KERNEL);
	if(dec == NULL)
		return -ENOMEM;

	memset(dec, 0, sizeof(struct SOLO6010_DEC));

	dec->solo6010 = solo6010;

	solo6010_mp4d_cnofig(solo6010);

	init_waitqueue_head(&dec->wq);

	dec->nr_ext_buf = solo6010->mp4d_ext_size / MAX_IMAGE_SIZE;
	dec->ext_buf = kmalloc(dec->nr_ext_buf * sizeof(struct EXT_BUFFER), GFP_KERNEL);
	for(i=0; i<dec->nr_ext_buf; i++)
	{
		dec->ext_buf[i].channel = 0;
		dec->ext_buf[i].addr = solo6010->mp4d_ext_addr + (MAX_IMAGE_SIZE * i);
		dec->ext_buf[i].size = 0;
	}

	dbg_msg("dec->nr_ext_buf:%d\n", dec->nr_ext_buf);

	dec->device = solo6010_device_add(&solo6010->chip, &fops, "dec", SOLO6010_DEC_DEV_OFFSET, (void *)dec);
	if(IS_ERR(dec->device))
	{
		int ret_val = PTR_ERR(dec->device);
		kfree(dec);
		return ret_val;
	}

	solo6010->dec = (void *)dec;

	return 0;
}

void solo6010_dec_exit(struct SOLO6010 *solo6010)
{
	struct SOLO6010_DEC *dec;

	dec = (struct SOLO6010_DEC *)solo6010->dec;

	solo6010_device_remove(dec->device);

	kfree(dec->ext_buf);

	kfree(dec);
}

