#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/poll.h>

#include "solo6010.h"

#include <solo6010/vin.h>

#define PROGRESSIVE_HSIZE		1280
#define PROGRESSIVE_VSIZE		1024

#define MOTION_THRESHOLD_WIDTH	64
#define MOTION_THRESHOLD_HEIGHT	64
#define MOTION_THRESHOLD_SIZE	8192

#define DEFAULT_MOTION_THRESHOLD	0x0300

struct MOTION_EVENT
{
	u32 detected;
	u32 sec;
	u32 usec;
};

struct SOLO6010_VIN
{
	struct SOLO6010 *solo6010;
	solo6010_device_t *device;

	unsigned int matrix_switch[16];
	unsigned int input_enable;
	u32 fmt_cfg;

	struct MOTION_EVENT motion_event[16];
	wait_queue_head_t motion_wq;

	unsigned long dma_buf;
};

int solo6010_motion_interrupt(struct SOLO6010 *solo6010)
{
	struct SOLO6010_VIN *vin = (struct SOLO6010_VIN *)solo6010->vin;
	u32 motion_status;
	u32 sec;
	u32 usec;
	int i;

	solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_MOTION);

	sec = solo6010_reg_read(solo6010, REG_TIMER_SEC);
	usec = solo6010_reg_read(solo6010, REG_TIMER_USEC);

	motion_status = solo6010_reg_read(solo6010, REG_VI_MOT_STATUS);
	for(i=0; i<16; i++)
	{
		if((motion_status & (1<<i)) && (!vin->motion_event[i].detected))
		{
			vin->motion_event[i].detected = 1;
			vin->motion_event[i].sec = sec;
			vin->motion_event[i].usec = usec;
		}
	}
	wake_up(&vin->motion_wq);

	return 0;
}

static void solo6010_vin_config(struct SOLO6010_VIN *vin)
{
	struct SOLO6010 *solo6010;

	solo6010 = vin->solo6010;

	solo6010_reg_write(solo6010, REG_VI_ACT_I_P,
		VI_H_START(solo6010->vin_hstart) |
		VI_V_START(solo6010->vin_vstart) |
		VI_V_STOP(solo6010->vin_vstart + solo6010->video_vsize));

	solo6010_reg_write(solo6010, REG_VI_ACT_I_S,
		VI_H_START(solo6010->vout_hstart) |
		VI_V_START(solo6010->vout_vstart) |
		VI_V_STOP(solo6010->vout_vstart + solo6010->video_vsize));

	solo6010_reg_write(solo6010, REG_VI_ACT_P,
		VI_H_START(0) |
		VI_V_START(1) |
		VI_V_STOP(PROGRESSIVE_VSIZE));

	solo6010_reg_write(solo6010, REG_VI_CH_FORMAT, VI_FD_SEL_MASK(0) | VI_PROG_MASK(0));

	vin->fmt_cfg = 0/*VI_FMT_CHECK_VCOUNT | VI_FMT_CHECK_HCOUNT*/;
	solo6010_reg_write(solo6010, REG_VI_FMT_CFG, vin->fmt_cfg);

	vin->input_enable = 0xffff;
	solo6010_reg_write(solo6010, REG_VI_CH_ENA, vin->input_enable);

	solo6010_reg_write(solo6010, REG_VI_PAGE_SW, 2);

	// PB live data reader
	// internal playback video controller configuration
	if(solo6010->video_type == 0)
	{
		solo6010_reg_write(solo6010, VI_PB_CONFIG, VI_PB_USER_MODE);
		solo6010_reg_write(solo6010, VI_PB_RANGE_HV, VI_PB_HSIZE(858) | VI_PB_VSIZE(246));
		solo6010_reg_write(solo6010, VI_PB_ACT_H, VI_PB_HSTART(16) | VI_PB_HSTOP(16+720));
		solo6010_reg_write(solo6010, VI_PB_ACT_V, VI_PB_VSTART(4) | VI_PB_VSTOP(4+240));
	}
	else
	{
		solo6010_reg_write(solo6010, VI_PB_CONFIG, VI_PB_USER_MODE | VI_PB_PAL);
		solo6010_reg_write(solo6010, VI_PB_RANGE_HV, VI_PB_HSIZE(864) | VI_PB_VSIZE(294));
		solo6010_reg_write(solo6010, VI_PB_ACT_H, VI_PB_HSTART(16) | VI_PB_HSTOP(16+720));
		solo6010_reg_write(solo6010, VI_PB_ACT_V, VI_PB_VSTART(4) | VI_PB_VSTOP(4+288));
	}
}

static int solo6010_vin_motion_config(struct SOLO6010_VIN *vin)
{
	struct SOLO6010 *solo6010;
	u16 *buf;
	int i;

	solo6010 = vin->solo6010;

	if((buf = (u16 *)__get_free_pages(GFP_KERNEL, get_order(MOTION_THRESHOLD_SIZE))) == NULL)
		return -ENOMEM;

	memset(buf, 0, (8<<10));

	// clear motion flag area
	solo6010_p2m_dma(solo6010, 0, 1, buf, solo6010->motion_ext_addr, MOTION_THRESHOLD_SIZE);
	solo6010_p2m_dma(solo6010, 0, 1, buf, solo6010->motion_ext_addr + MOTION_THRESHOLD_SIZE, MOTION_THRESHOLD_SIZE);

	// clear working cache
	for(i=0; i<16; i++)
	{
		solo6010_p2m_dma(solo6010, 0, 1, buf, solo6010->motion_ext_addr + (2*MOTION_THRESHOLD_SIZE) + (i*2*MOTION_THRESHOLD_SIZE) + MOTION_THRESHOLD_SIZE, MOTION_THRESHOLD_SIZE);
	}

	for(i=0; i<(MOTION_THRESHOLD_WIDTH*MOTION_THRESHOLD_HEIGHT); i++)
	{
		buf[i] = DEFAULT_MOTION_THRESHOLD;
	}

	// set default treshold table
	for(i=0; i<16; i++)
	{
		solo6010_p2m_dma(solo6010, 0, 1, buf, solo6010->motion_ext_addr + (2*MOTION_THRESHOLD_SIZE) + (i*2*MOTION_THRESHOLD_SIZE), MOTION_THRESHOLD_SIZE);
	}

	free_pages((unsigned long)buf, get_order(MOTION_THRESHOLD_SIZE));

	solo6010_reg_write(solo6010, REG_VI_MOT_ADR, VI_MOTION_EN(0xffff) | (solo6010->motion_ext_addr>>16));
	solo6010_reg_write(solo6010, REG_VI_MOT_CTRL,
		VI_MOTION_FRAME_COUNT(3) |
		VI_MOTION_SAMPLE_LENGTH(solo6010->video_hsize/16) |
		//VI_MOTION_INTR_START_STOP |
		VI_MOTION_SAMPLE_COUNT(10));

	solo6010_reg_write(solo6010, REG_VI_MOTION_BORDER, 0);
	solo6010_reg_write(solo6010, REG_VI_MOTION_BAR, 0);

	solo6010_enable_interrupt(solo6010, INTR_MOTION);

	return 0;
}

static inline int motion_is_detected(struct SOLO6010_VIN *vin)
{
	int i;

	for(i=0; i<16; i++)
	{
		if(vin->motion_event[i].detected)
			return 1;
	}

	return 0;
}

static unsigned int fops_poll(struct file *filp, poll_table *wait)
{
	struct SOLO6010_VIN *vin;
	unsigned int mask = 0;

	vin = filp->private_data;

	poll_wait(filp, &vin->motion_wq, wait);
	if(motion_is_detected(vin))
		mask |= POLLIN | POLLRDNORM | POLLRDBAND;

	return mask;
}

static ssize_t fops_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6010_VIN *vin;
	struct SOLO6010 *solo6010;

	int ret = 0;
	u32 i;

	vin = (struct SOLO6010_VIN *)filp->private_data;
	solo6010 = vin->solo6010;

	while(!motion_is_detected(vin))
	{
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(vin->motion_wq, motion_is_detected(vin));
		if(ret)
			return ret;
	}

	for(i=0; i<16; i++)
	{
		if(vin->motion_event[i].detected)
		{
			if(count < (12 + 512))
				break;

			solo6010_p2m_dma(solo6010, 0, 0, vin->dma_buf, solo6010->motion_ext_addr + (i * 512), 512);
			solo6010_reg_write(solo6010, REG_VI_MOT_CLEAR, 1<<i);
			#if 0
			vin->motion_event[i].detected = 0;

			dbg_msg("ch%02d (%08x:%08x)\n", i, vin->motion_event[i].sec, vin->motion_event[i].usec);
			ret++;
			#else
			copy_to_user(buf, &i, 4);
			buf += 4;
			copy_to_user(buf, &vin->motion_event[i].sec, 4);
			buf += 4;
			copy_to_user(buf, &vin->motion_event[i].usec, 4);
			buf += 4;
			copy_to_user(buf, vin->dma_buf, 512);
			buf += 512;
			ret += 12 + 512;
			count -= 12 + 256;

			vin->motion_event[i].detected = 0;
			#endif
		}
	}

	return ret;
}

static int fops_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct SOLO6010_VIN *vin;
	struct SOLO6010 *solo6010;

	if(_IOC_TYPE(cmd) != IOCTL_VIN_MAGIC)
		return -EINVAL;

	if(_IOC_NR(cmd) >= IOCTL_VIN_MAXNR)
		return -EINVAL;

	vin = filp->private_data;
	solo6010 = vin->solo6010;

	switch(cmd)
	{
		case IOCTL_VIN_SWITCH:
		{
			struct SOLO6010_VIN_SWITCH vin_switch;

			if(copy_from_user(&vin_switch, (void *)arg, sizeof(struct SOLO6010_VIN_SWITCH)))
				return -EFAULT;

			if((vin_switch.src > 16) || (vin_switch.dst > 15))
				return -EINVAL;

			vin->matrix_switch[vin_switch.dst] = vin_switch.src;

			if(vin_switch.dst < 6)
			{
				solo6010_reg_write(solo6010, REG_VI_CH_SWITCH_0,
					(vin->matrix_switch[0]<<0) |
					(vin->matrix_switch[1]<<5) |
					(vin->matrix_switch[2]<<10) |
					(vin->matrix_switch[3]<<15) |
					(vin->matrix_switch[4]<<20) |
					(vin->matrix_switch[5]<<25));
			}
			else if(vin_switch.dst < 12)
			{
				solo6010_reg_write(solo6010, REG_VI_CH_SWITCH_1,
					(vin->matrix_switch[6]<<0) |
					(vin->matrix_switch[7]<<5) |
					(vin->matrix_switch[8]<<10) |
					(vin->matrix_switch[9]<<15) |
					(vin->matrix_switch[10]<<20) |
					(vin->matrix_switch[11]<<25));
			}
			else
			{
				solo6010_reg_write(solo6010, REG_VI_CH_SWITCH_2,
					(vin->matrix_switch[12]<<0) |
					(vin->matrix_switch[13]<<5) |
					(vin->matrix_switch[14]<<10) |
					(vin->matrix_switch[15]<<15));
			}

			return 0;
		}

		case IOCTL_VIN_MOSAIC:
		{
			struct SOLO6010_VIN_MOSAIC vin_mosaic;

			if(copy_from_user(&vin_mosaic, (void *)arg, sizeof(struct SOLO6010_VIN_MOSAIC)))
				return -EFAULT;

			if(vin_mosaic.channel > 15)
				return -EINVAL;

			solo6010_reg_write(solo6010, REG_VI_MOSAIC(vin_mosaic.channel),
				VI_MOSAIC_SX(vin_mosaic.sx/8) |
				VI_MOSAIC_EX(vin_mosaic.ex/8) |
				VI_MOSAIC_SY(vin_mosaic.sy/8) |
				VI_MOSAIC_EY(vin_mosaic.ey/8));

			return 0;
		}

		case IOCTL_VIN_MOTION_THRESHOLD:
		{
			struct SOLO6010_VIN_MOTION_THRESHOLD threshold;
			u16 *threshold_table;
			u16 *threshold_table_user;
			int i;

			if(copy_from_user(&threshold, (void *)arg, sizeof(struct SOLO6010_VIN_MOTION_THRESHOLD)))
				return -EFAULT;

			if((threshold.x >= MOTION_THRESHOLD_WIDTH) || (threshold.y >= MOTION_THRESHOLD_HEIGHT))
				return -EINVAL;

			if(threshold.x + threshold.width > MOTION_THRESHOLD_WIDTH)
				threshold.width = MOTION_THRESHOLD_WIDTH - threshold.x;

			if(threshold.y + threshold.height > MOTION_THRESHOLD_HEIGHT)
				threshold.height = MOTION_THRESHOLD_HEIGHT - threshold.y;

			if((threshold_table = (u16 *)__get_free_pages(GFP_KERNEL, get_order(MOTION_THRESHOLD_SIZE))) == NULL)
				return -ENOMEM;

			threshold_table_user = (u16 *)threshold.ptr;

			solo6010_p2m_dma(solo6010, 0, 0, threshold_table,
				solo6010->motion_ext_addr + (MOTION_THRESHOLD_SIZE*2) + (threshold.channel*MOTION_THRESHOLD_SIZE*2), MOTION_THRESHOLD_SIZE);

			for(i=0; i<threshold.height; i++)
			{
				if(copy_from_user(threshold_table + ((threshold.y + i) * MOTION_THRESHOLD_WIDTH) + threshold.x,
					threshold_table_user + (i * threshold.width), threshold.width * 2))
					return -EFAULT;
			}

			solo6010_p2m_dma(solo6010, 0, 1, threshold_table,
				solo6010->motion_ext_addr + (MOTION_THRESHOLD_SIZE*2) + (threshold.channel*MOTION_THRESHOLD_SIZE*2), MOTION_THRESHOLD_SIZE);

			free_pages((unsigned long)threshold_table, get_order(MOTION_THRESHOLD_SIZE));

			return 0;
		}
	}

	return -ENOTTY;
}

static int fops_open(struct inode *inode, struct file *filp)
{
	struct SOLO6010_VIN *vin;

	vin = (struct SOLO6010_VIN *)solo6010_device_data_get(inode_to_solo6010_device(inode));
	if(!vin)
		return -ENODEV;

	filp->private_data = vin;

	return 0;
}

static int fops_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations fops = {
	.owner =    THIS_MODULE,
	.poll =		fops_poll,
	.read =     fops_read,
	.ioctl =    fops_ioctl,
	.open =     fops_open,
	.release =  fops_release,
};

#define VIN_CLOCK_DELAY		3
int solo6010_vin_init(struct SOLO6010 *solo6010)
{
	struct SOLO6010_VIN *vin;
	int i;

	vin = (struct SOLO6010_VIN *)kmalloc(sizeof(struct SOLO6010_VIN), GFP_KERNEL);
	if(vin == NULL)
		return -ENOMEM;

	memset(vin, 0, sizeof(struct SOLO6010_VIN));

	vin->solo6010 = solo6010;

	for(i=0; i<16; i++)
	{
		vin->matrix_switch[i] = i;
	}

	init_waitqueue_head(&vin->motion_wq);
	for(i=0; i<16; i++)
	{
		vin->motion_event[i].detected = 0;
	}

	vin->dma_buf = __get_free_pages(GFP_KERNEL, 1);
	if(!vin->dma_buf)
	{
		kfree(vin);
		return -ENOMEM;
	}


	solo6010_reg_write(solo6010, REG_VI_CH_SWITCH_0,
		(vin->matrix_switch[0]<<0) |
		(vin->matrix_switch[1]<<5) |
		(vin->matrix_switch[2]<<10) |
		(vin->matrix_switch[3]<<15) |
		(vin->matrix_switch[4]<<20) |
		(vin->matrix_switch[5]<<25));

	solo6010_reg_write(solo6010, REG_VI_CH_SWITCH_1,
		(vin->matrix_switch[6]<<0) |
		(vin->matrix_switch[7]<<5) |
		(vin->matrix_switch[8]<<10) |
		(vin->matrix_switch[9]<<15) |
		(vin->matrix_switch[10]<<20) |
		(vin->matrix_switch[11]<<25));

	solo6010_reg_write(solo6010, REG_VI_CH_SWITCH_2,
		(vin->matrix_switch[12]<<0) |
		(vin->matrix_switch[13]<<5) |
		(vin->matrix_switch[14]<<10) |
		(vin->matrix_switch[15]<<15));

	solo6010_reg_write(solo6010, REG_SYS_VCLK,
		SYS_CONFIG_VOUT_CLK_SELECT(2) |
		SYS_CONFIG_VIN1415_DELAY(VIN_CLOCK_DELAY) |
		SYS_CONFIG_VIN1213_DELAY(VIN_CLOCK_DELAY) |
		SYS_CONFIG_VIN1011_DELAY(VIN_CLOCK_DELAY) |
		SYS_CONFIG_VIN0809_DELAY(VIN_CLOCK_DELAY) |
		SYS_CONFIG_VIN0607_DELAY(VIN_CLOCK_DELAY) |
		SYS_CONFIG_VIN0405_DELAY(VIN_CLOCK_DELAY) |
		SYS_CONFIG_VIN0203_DELAY(VIN_CLOCK_DELAY) |
		SYS_CONFIG_VIN0001_DELAY(VIN_CLOCK_DELAY));

	solo6010->vin = (void *)vin;
	solo6010_vin_config(vin);
	solo6010_vin_motion_config(vin);

	vin->device = solo6010_device_add(&solo6010->chip, &fops, "vin", SOLO6010_VIN_DEV_OFFSET, (void *)vin);
	if(IS_ERR(vin->device))
	{
		int ret_val = PTR_ERR(vin->device);

		solo6010_disable_interrupt(solo6010, INTR_MOTION);
		solo6010->vin = NULL;
		free_pages(vin->dma_buf, 1);
		kfree(vin);
		return ret_val;
	}

	return 0;
}

void solo6010_vin_exit(struct SOLO6010 *solo6010)
{
	struct SOLO6010_VIN *vin;

	vin = (struct SOLO6010_VIN *)solo6010->vin;

	solo6010_device_remove(vin->device);
	free_pages(vin->dma_buf, 1);
	kfree(vin);
}

