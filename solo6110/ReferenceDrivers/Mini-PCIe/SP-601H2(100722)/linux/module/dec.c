#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/poll.h>

#include "solo6x10.h"

#define SYNC_TIME_MARGIN		10
#define MAX_IMAGE_SIZE			(512<<10)
#define TIME_CONTROL_ENABLE		1
#define USER_DISP_CONTROL		1

struct EXT_BUFFER
{
	u32 addr;

	u32 channel;
#if USER_DISP_CONTROL
	u32 window_id;
	u32 scale;
#endif
	u32 vop_type;
	u32 size;
	u32 usec;
	u32 sec;

	u32 vd_idx[5];
};

struct SOLO6x10_DEC
{
	struct SOLO6x10 *solo6x10;
	solo6x10_device_t *device;

	wait_queue_head_t wq;

	unsigned int used;

	struct EXT_BUFFER *ext_buf;

	u32 wait_i_mask;
	u32 enable_mask;

	u32 error_mask;

	unsigned int nr_ext_buf;
	unsigned int ext_buf_start;
	unsigned int ext_buf_end;

	struct EXT_BUFFER *cur_buf;
	struct EXT_BUFFER *rdy_buf;

	unsigned int time_diff_sec;
	unsigned int time_diff_usec;

	unsigned int lock;
	wait_queue_head_t lock_wq;

#if USER_DISP_CONTROL
	u32 prev_scale[16];
	u32 wr_page[16];
	u32 rd_page[16];
	u32 zoom[4];
	unsigned int user_page_control;
#endif
};

unsigned int solo6x10_disp_get_scale(struct SOLO6x10 *solo6x10, unsigned int window_id);

void solo6x10_dec_lock(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_DEC *dec = (struct SOLO6x10_DEC *)solo6x10->dec;

	if(!dec)
		return;

	dec->lock = 1;
	wait_event_interruptible(dec->lock_wq, !dec->cur_buf);
	dec->ext_buf_end = dec->ext_buf_start;
}

void solo6x10_dec_unlock(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_DEC *dec = (struct SOLO6x10_DEC *)solo6x10->dec;
#if USER_DISP_CONTROL
	int full_screen = 0;
	int i;
#endif
	if(!dec)
		return;

#if USER_DISP_CONTROL
	for(i=0; i<16; i++)
	{
		if(solo6x10_disp_get_scale(solo6x10, i) == 1)
			full_screen = 1;
	}

	if(full_screen)
	{
		solo6x10_reg_write(solo6x10, REG_VD_CFG0,
			VD_CFG_NO_DISP_ERROR |
			VD_CFG_SINGLE_MODE |
			VD_CFG_USER_PAGE_CTRL |
			VD_CFG_LITTLE_ENDIAN |
			VD_CFG_ERR_LOCK |
			VD_CFG_ERR_INT_ENA |
			VD_CFG_TIME_WIDTH(15) |
			VD_CFG_DCT_INTERVAL(13));
		solo6x10_reg_write(solo6x10, 0x0248, 1);
	}
	else
	{
		solo6x10_reg_write(solo6x10, REG_VD_CFG0,
			VD_CFG_NO_DISP_ERROR |
			VD_CFG_SINGLE_MODE |
			VD_CFG_LITTLE_ENDIAN |
			VD_CFG_ERR_LOCK |
			VD_CFG_ERR_INT_ENA |
			VD_CFG_TIME_WIDTH(15) |
			VD_CFG_DCT_INTERVAL(13));
		solo6x10_reg_write(solo6x10, 0x0248, 0);
	}

	dec->user_page_control = full_screen;
#endif

	dec->lock = 0;
	wake_up(&dec->wq);
}

void solo6x10_dec_enable_pb(struct SOLO6x10 *solo6x10, unsigned int channel)
{
	struct SOLO6x10_DEC *dec = (struct SOLO6x10_DEC *)solo6x10->dec;

	if(!dec)
		return;

	dec->wait_i_mask |= 1<<channel;
	dec->enable_mask |= 1<<channel;
}

void solo6x10_dec_disable_pb(struct SOLO6x10 *solo6x10, unsigned int channel)
{
	struct SOLO6x10_DEC *dec = (struct SOLO6x10_DEC *)solo6x10->dec;

	if(!dec)
		return;

	dec->enable_mask &= ~(1<<channel);
	solo6x10_reg_write(solo6x10, REG_VO_ZOOM_CTRL, 0);
}

static void solo6x10_mp4d_config(struct SOLO6x10 *solo6x10)
{
	if(solo6x10->pdev->device == 0x6110)
	{
		unsigned int max_mb_h = (solo6x10->video_hsize/16);
		unsigned int max_mb_v = (solo6x10->video_vsize/8);

		solo6x10_reg_write(solo6x10, REG_VD_CFG0,
			VD_CFG_NO_DISP_ERROR |
			VD_CFG_SINGLE_MODE |
			VD_CFG_LITTLE_ENDIAN |
			VD_CFG_ERR_LOCK |
			VD_CFG_ERR_INT_ENA |
			VD_CFG_TIME_WIDTH(15) |
			VD_CFG_DCT_INTERVAL(13));

		solo6x10_reg_write(solo6x10, 0x0248, 0);

		solo6x10_reg_write(solo6x10, REG_VD_DEINTERLACE,
			(16<<24) |
			(3<<22) |
			VD_DEINTERLACE_THRESHOLD(100) |
			VD_DEINTERLACE_EDGE_VALUE(5));

		solo6x10_reg_write(solo6x10, REG_VD_CFG1,
			VD_CFG_MAX_MBH(max_mb_h) |
			VD_CFG_MAX_MBV(max_mb_v) |
			VD_CFG_TIMEOUT(2) |	// decode timeout (n frame)
			VD_CFG_DEC2LIVE_ADDR(solo6x10->dec2live_ext_addr));
	}
	else
	{
		solo6x10_reg_write(solo6x10, REG_VD_CFG0,
			VD_CFG_BUSY_WIAT_CODE |
			VD_CFG_BUSY_WIAT_REF |
			VD_CFG_BUSY_WIAT_RES |
			VD_CFG_BUSY_WIAT_MS |
			VD_CFG_SINGLE_MODE |
			VD_CFG_LITTLE_ENDIAN |
			VD_CFG_ERR_LOCK |
			VD_CFG_ERR_INT_ENA |
			VD_CFG_TIME_WIDTH(15) |
			VD_CFG_DCT_INTERVAL(36/solo6x10->decoder_ability));

		solo6x10_reg_write(solo6x10, REG_VD_DEINTERLACE, VD_DEINTERLACE_THRESHOLD(100) | VD_DEINTERLACE_EDGE_VALUE(5));

		solo6x10_reg_write(solo6x10, REG_VD_CFG1, solo6x10->dec2live_ext_addr>>16);
	}
}

unsigned int change_decoder_ability(struct SOLO6x10 *solo6x10, unsigned int ability)
{
	if(ability > SOLO6010_CODEC_ABILITY_MAX)
		ability = SOLO6010_CODEC_ABILITY_MAX;

	if(ability < SOLO6010_CODEC_ABILITY_MIN)
		ability = SOLO6010_CODEC_ABILITY_MIN;

	if(solo6x10->pdev->device == 0x6110)
		return 0;

	solo6x10->decoder_ability = ability;

	solo6x10_reg_write(solo6x10, REG_VD_CFG0,
		VD_CFG_BUSY_WIAT_CODE |
		VD_CFG_BUSY_WIAT_REF |
		VD_CFG_BUSY_WIAT_RES |
		VD_CFG_BUSY_WIAT_MS |
		VD_CFG_SINGLE_MODE |
		VD_CFG_LITTLE_ENDIAN |
		VD_CFG_ERR_LOCK |
		VD_CFG_ERR_INT_ENA |
		VD_CFG_TIME_WIDTH(15) |
		VD_CFG_DCT_INTERVAL(36/solo6x10->decoder_ability));

	dbg_msg("decoder ability : %d\n", solo6x10->decoder_ability);

	return solo6x10->decoder_ability;
}

#define reset_time_diff(dec, buf, chip_sec, chip_usec)		\
do{															\
	dec->time_diff_sec = chip_sec - 1 - buf->sec;			\
	dec->time_diff_usec = chip_usec + 1000000 - buf->usec;	\
}while(0)

static int intrrupt_dec_error(struct SOLO6x10_DEC *dec, u32 status0, u32 status1)
{
	struct SOLO6x10 *solo6x10 = dec->solo6x10;
	struct EXT_BUFFER *cur_buf = dec->cur_buf;
	struct EXT_BUFFER *rdy_buf = dec->rdy_buf;

	if(!cur_buf)
	{
		dbg_msg("decode error! (%08x,%08x)\n", status0, status1);
		return 0;
	}

	dbg_msg("decode error! (ch%d %08x:%08x size:%05x(%08x,%08x) diff:%d)\n",
		cur_buf->channel,
		cur_buf->sec,
		cur_buf->usec,
		cur_buf->size,
		status0,
		status1,
		(int)cur_buf->size - (int)(status1 & 0x000fffff));

	dec->wait_i_mask |= 1<<cur_buf->channel;
	dec->error_mask |= 1<<cur_buf->channel;

	if(rdy_buf)
	{
		if(rdy_buf->channel != cur_buf->channel)
		{
			solo6x10_reg_write(solo6x10, REG_VD_CTRL, VD_OPER_ON | VD_MAX_ITEM(1));
			return 1;
		}

		if(rdy_buf->vop_type == 0)
		{
			dec->wait_i_mask &= ~(1<<rdy_buf->channel);
			dec->error_mask &= ~(1<<rdy_buf->channel);
			rdy_buf->vd_idx[0] |= (1<<21);
			solo6x10_reg_write(solo6x10, REG_VD_IDX0, rdy_buf->vd_idx[0]);
			solo6x10_reg_write(solo6x10, REG_VD_CTRL, VD_OPER_ON | VD_MAX_ITEM(1));
			return 1;
		}

		dec->rdy_buf = NULL;
	}

	dec->cur_buf = NULL;

	return 0;
}

int solo6x10_decoder_interrupt(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_DEC *dec;
	u32 status0;
	u32 status1;
	struct EXT_BUFFER *ext_buf;

	dec = solo6x10->dec;

	status0 = solo6x10_reg_read(solo6x10, REG_VD_STATUS0);
	status1 = solo6x10_reg_read(solo6x10, REG_VD_STATUS1);
	solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_DECODER);

	if(solo6x10->pdev->device == 0x6110)
	{
		if(status0 & VD_STATUS0_INTR_MBSIZE_ERR)
		{
			struct EXT_BUFFER *rdy_buf = dec->rdy_buf;
			if(rdy_buf)
			{
				dbg_msg("mb_size error! (ch%d %08x:%08x size:%05x(%08x,%08x) idx1:%08x)\n",
					rdy_buf->channel, rdy_buf->sec, rdy_buf->usec, rdy_buf->size,
					status0, status1, rdy_buf->vd_idx[1]);
				dec->wait_i_mask |= 1<<rdy_buf->channel;
				dec->error_mask |= 1<<rdy_buf->channel;
				dec->rdy_buf = NULL;
			}
			else
			{
				dbg_msg("mb_size error! (%08x,%08x)\n", status0, status1);
			}
		}
		else if(status0 & VD_STATUS0_INTR_TIMEOUT)
		{
			dbg_msg("VD_STATUS0_INTR_TIMEOUT(%08x,%08x)\n", status0, status1);
			dec->wait_i_mask = 0xffff;
			dec->error_mask = 0xffff;
			dec->cur_buf = NULL;
			dec->rdy_buf = NULL;
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
			return 0;
		}
		else if(status0 & VD_STATUS0_INTR_ERR)
		{
			if(intrrupt_dec_error(dec, status0, status1))
				return 0;
		}
	}
	else if(status0 & VD_STATUS0_INTR_ERR)
	{
		if(dec->cur_buf)
			dbg_msg("decode error! (ch%d %08x:%08x size:%05x(%05x))\n",
			dec->cur_buf->channel, dec->cur_buf->sec, dec->cur_buf->usec, dec->cur_buf->size, status1 & 0x000fffff);
		else
			dbg_msg("decode error!\n");
	}

	dec->cur_buf = dec->rdy_buf;
	dec->rdy_buf = NULL;

	if(dec->lock && (!dec->cur_buf))
		wake_up(&dec->lock_wq);

get_ext_buf:
	if((dec->ext_buf_start == dec->ext_buf_end) || dec->lock)
	{
		dec->rdy_buf = NULL;
		return 0;
	}
	ext_buf = &dec->ext_buf[dec->ext_buf_start];

	if(!(dec->enable_mask & (1<<ext_buf->channel)))
	{
		dec->ext_buf_start = (dec->ext_buf_start + 1) % dec->nr_ext_buf;
		wake_up(&dec->wq);
		goto get_ext_buf;
	}

	if(dec->wait_i_mask & (1<<ext_buf->channel))
	{
		if(ext_buf->vop_type)
		{
			dec->ext_buf_start = (dec->ext_buf_start + 1) % dec->nr_ext_buf;
			wake_up(&dec->wq);
			goto get_ext_buf;
		}
		else if(solo6x10->pdev->device == 0x6110)
		{
			ext_buf->vd_idx[0] |= (1<<21);
		}
	}

#if TIME_CONTROL_ENABLE
	if(ext_buf->sec || ext_buf->usec)
	{
		u32 chip_sec;
		u32 chip_usec;
		u32 dec_sec;
		u32 dec_usec;

		chip_sec = solo6x10_reg_read(solo6x10, REG_TIMER_SEC);
		chip_usec = solo6x10_reg_read(solo6x10, REG_TIMER_USEC);

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
#endif

	if(dec->error_mask & (1<<ext_buf->channel))
	{
		dec->error_mask &= ~(1<<ext_buf->channel);
		if(solo6x10->pdev->device == 0x6110)
			ext_buf->vd_idx[0] |= (1<<21);
	}

	solo6x10_reg_write(solo6x10, REG_VD_IDX0, ext_buf->vd_idx[0]);
#if USER_DISP_CONTROL
	if(dec->user_page_control)
	{
		u32 wr_page = (dec->wr_page[ext_buf->channel] + 1) & 3;

		if(wr_page == dec->rd_page[ext_buf->channel])
			return 0;

		dec->wr_page[ext_buf->channel] = wr_page;
		solo6x10_reg_write(solo6x10, REG_VD_IDX4, VD_IDX_DEC_WR_PAGE(wr_page));
	}

	if((dec->prev_scale[ext_buf->channel] == 2) && (solo6x10_disp_get_scale(solo6x10, ext_buf->window_id) == 1))
		dec->zoom[dec->wr_page[ext_buf->channel]] = 1;
	else
		dec->zoom[dec->wr_page[ext_buf->channel]] = 0;

	dec->prev_scale[ext_buf->channel] = ext_buf->scale;
	if((ext_buf->scale == 2) && (solo6x10_disp_get_scale(solo6x10, ext_buf->window_id) == 1))
		solo6x10_reg_write(solo6x10, REG_VD_IDX1, (ext_buf->vd_idx[1] & 0x0FFFFFFF) | (1<<28));
	else
		solo6x10_reg_write(solo6x10, REG_VD_IDX1, ext_buf->vd_idx[1]);
#else
	solo6x10_reg_write(solo6x10, REG_VD_IDX1, ext_buf->vd_idx[1]);
#endif
	solo6x10_reg_write(solo6x10, REG_VD_IDX2, ext_buf->vd_idx[2]);
	solo6x10_reg_write(solo6x10, REG_VD_CODE_ADR, ext_buf->addr);
	solo6x10_reg_write(solo6x10, REG_VD_CTRL, VD_OPER_ON | VD_MAX_ITEM(1));
	dec->rdy_buf = ext_buf;

	if(dec->wait_i_mask & (1<<ext_buf->channel))
	{
		dec->wait_i_mask &= ~(1<<ext_buf->channel);
	}
	else
	{
		dec->ext_buf_start = (dec->ext_buf_start + 1) % dec->nr_ext_buf;
		wake_up(&dec->wq);
	}

	return 0;
}

int solo6x10_decoder_update(struct SOLO6x10 *solo6x10, int fi)
{
#if USER_DISP_CONTROL
	struct SOLO6x10_DEC *dec = (struct SOLO6x10_DEC *)solo6x10->dec;

	if(dec->user_page_control && fi)
	{
		int i;
		u32 rd_page;

		for(i=0; i<16; i++)
		{
			if(!(dec->enable_mask & (1<<i)))
				continue;
			rd_page = (dec->rd_page[i] + 1) & 3;
			if(rd_page == dec->wr_page[i])
				continue;
			dec->rd_page[i] = rd_page;
			solo6x10_reg_write(solo6x10, 0x0550 + (i*4), rd_page);
			solo6x10_reg_write(solo6x10, REG_VO_ZOOM_CTRL, dec->zoom[rd_page] ? VO_ZOOM_HOR_ON : 0);
		}
	}
#endif
	return 0;
}

static loff_t fops_llseek(struct file *filp, loff_t off, int whence)
{
	return -EINVAL;
}

#define DEC_BUF_READY(dec)	\
	(	(!dec->lock)	\
	&&	(dec->cur_buf != (dec->ext_buf + dec->ext_buf_end))	\
	&&	(dec->rdy_buf != (dec->ext_buf + dec->ext_buf_end))	\
	&&	(((dec->nr_ext_buf + dec->ext_buf_end - dec->ext_buf_start) % dec->nr_ext_buf) <= (dec->nr_ext_buf - 4)))

static unsigned int fops_poll(struct file *filp, poll_table *wait)
{
	struct SOLO6x10_DEC *dec;
	unsigned int mask = 0;

	dec = filp->private_data;

	poll_wait(filp, &dec->wq, wait);
	if(DEC_BUF_READY(dec))
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;

	return mask;
}

static ssize_t fops_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	return -1;
}

static ssize_t fops_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6x10_DEC *dec;
	u8 dec_index[64];
	struct EXT_BUFFER *ext_buf;
	int ret;

	dec = filp->private_data;

	if((unsigned long)buf & 0xf)
	{
		dbg_msg(" solo6x10 dec write buffer should be aligned by 16.\n");
		return -1;
	}

	while(!DEC_BUF_READY(dec))
	{
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(dec->wq, DEC_BUF_READY(dec));
		if(ret)
			return ret;
	}

	if(count < 64)
		return -1;

	if(copy_from_user(dec_index, buf, 64))
		return -EFAULT;

	ext_buf = dec->ext_buf + dec->ext_buf_end;

	ext_buf->channel = dec_index[3] & 0x0f;
#if USER_DISP_CONTROL
	ext_buf->window_id = dec_index[7] & 0x0f;
	ext_buf->scale = (dec_index[7]>>4) & 0x0f;
#endif
	ext_buf->vop_type = (dec_index[2]>>6) & 3;
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

	ext_buf->vd_idx[0] = dec_index[3];
	ext_buf->vd_idx[0] = (ext_buf->vd_idx[0]<<8) + (u32)dec_index[2];
	ext_buf->vd_idx[0] = (ext_buf->vd_idx[0]<<8) + (u32)dec_index[1];
	ext_buf->vd_idx[0] = (ext_buf->vd_idx[0]<<8) + (u32)dec_index[0];
	ext_buf->vd_idx[1] = dec_index[7];
	ext_buf->vd_idx[1] = (ext_buf->vd_idx[1]<<8) + (u32)dec_index[6];
	ext_buf->vd_idx[1] = (ext_buf->vd_idx[1]<<8) + (u32)dec_index[5];
	ext_buf->vd_idx[1] = (ext_buf->vd_idx[1]<<8) + (u32)dec_index[4];
	ext_buf->vd_idx[2] = (dec->solo6x10->dref_ext_addr + (ext_buf->channel * 0x00140000))>>16;
	solo6x10_dma_user(dec->solo6x10, 1, 1, buf + 64, ext_buf->addr, (count - 64 + 0xf) & (~0xf));

	dec->ext_buf_end = (dec->ext_buf_end + 1) % dec->nr_ext_buf;

	return count;
}

static int fops_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	return -ENOTTY;
}

static int fops_open(struct inode *inode, struct file *filp)
{
	struct SOLO6x10_DEC *dec;

	dec = (struct SOLO6x10_DEC *)solo6x10_device_data_get(inode_to_solo6x10_device(inode));
	if(!dec)
		return -ENODEV;

	if(dec->used)
		return -EBUSY;
	dec->used = 1;

	filp->private_data = dec;

	return 0;
}

static int fops_release(struct inode *inode, struct file *filp)
{
	struct SOLO6x10_DEC *dec;

	dec = filp->private_data;

	dec->ext_buf_end = dec->ext_buf_start;

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

int solo6x10_dec_init(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_DEC *dec;
	int i;

	dec = (struct SOLO6x10_DEC *)kmalloc(sizeof(struct SOLO6x10_DEC), GFP_KERNEL);
	if(dec == NULL)
		return -ENOMEM;

	memset(dec, 0, sizeof(struct SOLO6x10_DEC));

	dec->solo6x10 = solo6x10;

	solo6x10_mp4d_config(solo6x10);

	init_waitqueue_head(&dec->wq);

	dec->nr_ext_buf = solo6x10->mp4d_ext_size / MAX_IMAGE_SIZE;
	dec->ext_buf = kmalloc(dec->nr_ext_buf * sizeof(struct EXT_BUFFER), GFP_KERNEL);
	for(i=0; i<dec->nr_ext_buf; i++)
	{
		dec->ext_buf[i].channel = 0;
		dec->ext_buf[i].addr = solo6x10->mp4d_ext_addr + (MAX_IMAGE_SIZE * i);
		dec->ext_buf[i].size = 0;
	}

	dbg_msg("dec->nr_ext_buf:%d\n", dec->nr_ext_buf);

	init_waitqueue_head(&dec->lock_wq);

	dec->device = solo6x10_device_add(&solo6x10->chip, &fops, "dec", SOLO6x10_DEC_DEV_OFFSET, (void *)dec);
	if(IS_ERR(dec->device))
	{
		int ret_val = PTR_ERR(dec->device);
		kfree(dec);
		return ret_val;
	}

	solo6x10->dec = (void *)dec;

	solo6x10_enable_interrupt(dec->solo6x10, INTR_DECODER);
	solo6x10_reg_write(dec->solo6x10, REG_VD_CTRL, VD_OPER_ON);

	return 0;
}

void solo6x10_dec_exit(struct SOLO6x10 *solo6x10)
{
	struct SOLO6x10_DEC *dec;

	dec = (struct SOLO6x10_DEC *)solo6x10->dec;

	solo6x10_disable_interrupt(dec->solo6x10, INTR_DECODER);
	solo6x10_reg_write(dec->solo6x10, REG_VD_CTRL, 0);

	solo6x10_device_remove(dec->device);

	kfree(dec->ext_buf);

	kfree(dec);
}
