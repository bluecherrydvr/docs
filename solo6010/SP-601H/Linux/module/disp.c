#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/delay.h>

#include "solo6010.h"

#include <solo6010/disp.h>

struct SOLO6010_DISP
{
	struct SOLO6010 *solo6010;
	solo6010_device_t *device;
    
    u32 disp_nr;
};

static void solo6010_disp_config(struct SOLO6010 *solo6010)
{
	solo6010_reg_write(solo6010, REG_VO_BORDER_LINE_COLOR, (0xa0<<24) | (0x88<<16) | (0xa0<<8) | 0x88);
	solo6010_reg_write(solo6010, REG_VO_BORDER_FILL_COLOR, (0x10<<24) | (0x8f<<16) | (0x10<<8) | 0x8f);

	solo6010_reg_write(solo6010, REG_VO_BKG_COLOR, (16<<24) | (128<<16) | (16<<8) | 128);

	solo6010_reg_write(solo6010, REG_VO_FMT_ENC,
		((solo6010->video_type == 0) ? 0 : VO_FMT_TYPE_PAL) |
		VO_USER_COLOR_SET_NAV |
		VO_NA_COLOR_Y(0) |
		VO_NA_COLOR_CB(0) |
		VO_NA_COLOR_CR(0));

	solo6010_reg_write(solo6010, REG_VO_ACT_H,
		VO_H_START(solo6010->vout_hstart) | VO_H_STOP(solo6010->vout_hstart + solo6010->video_hsize));

	solo6010_reg_write(solo6010, REG_VO_ACT_V,
		VO_V_START(solo6010->vout_vstart) | VO_V_STOP(solo6010->vout_vstart + solo6010->video_vsize));

	solo6010_reg_write(solo6010, REG_VO_RANHE_HV,
		VO_H_LEN(solo6010->video_hsize) | VO_V_LEN(solo6010->video_vsize));

	solo6010_reg_write(solo6010, REG_VI_WIN_SW, 5);
    solo6010_reg_write(solo6010, REG_VO_DEINTERLACE, VO_DEINTERLACE_THRESHOLD(100) | VO_DEINTERLACE_EDGE_VALUE(5));
	solo6010_reg_write(solo6010, REG_VO_DISP_CTRL, VO_DISP_ON | VO_DISP_ERASE_COUNT(12) | VO_DISP_BASE(solo6010->disp_ext_addr));
	solo6010_reg_write(solo6010, REG_VO_DISP_ERASE, VO_DISP_ERASE_ON);
}

static loff_t fops_llseek(struct file *filp, loff_t off, int whence)
{
	return -EINVAL;
}

static ssize_t fops_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6010_DISP *disp;
    struct SOLO6010 *solo6010;
    int size;

    disp = filp->private_data;
    solo6010 = disp->solo6010;

    if(solo6010->video_type == 0)
        size = 1024*480*2;
    else 
        size = 1024*576*2;

    solo6010_dma_user(solo6010, 0, 0, buf, solo6010->disp_ext_addr + size , size);

    return size;
}

static ssize_t fops_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	return -EINVAL;
}

static int fops_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct SOLO6010_DISP *disp;
	struct SOLO6010 *solo6010;

	if(_IOC_TYPE(cmd) != IOCTL_DISP_MAGIC)
		return -EINVAL;

	if(_IOC_NR(cmd) >= IOCTL_DISP_MAXNR)
		return -EINVAL;

	if(_IOC_SIZE(cmd))
	{
		if((_IOC_DIR(cmd) & _IOC_READ) && (!access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd))))
		{
			return -1;
		}

		if((_IOC_DIR(cmd) & _IOC_WRITE) && (!access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd))))
		{
			return -1;
		}
	}

	disp = filp->private_data;
	solo6010 = disp->solo6010;

	switch(cmd)
	{
		case IOCTL_DISP_ERASE_ON:
			solo6010_reg_write(solo6010, REG_VO_DISP_ERASE, VO_DISP_ERASE_ON);
			return 0;

		case IOCTL_DISP_ERASE_OFF:
			solo6010_reg_write(solo6010, REG_VO_DISP_ERASE, 0);
			return 0;

		case IOCTL_DISP_CHANNEL:
		{
			struct DISP_CTRL disp_ctrl;

			if(copy_from_user(&disp_ctrl, (struct DISP_CTRL *)arg, sizeof(struct DISP_CTRL)))
				return -EINVAL;

			if(disp_ctrl.window_id > 15)
				return -EINVAL;

			solo6010_reg_write(solo6010, VI_WIN_CTRL0(disp_ctrl.window_id),
				VI_WIN_CHANNEL(disp_ctrl.channel) |
				VI_WIN_PIP((disp_ctrl.scale && (disp_ctrl.mode & DISP_MODE_PIP)) ? 1 : 0) |
				VI_WIN_SCALE(disp_ctrl.scale) |
				VI_WIN_SX(disp_ctrl.sx) |
				VI_WIN_EX(disp_ctrl.ex));

			solo6010_reg_write(solo6010, VI_WIN_CTRL1(disp_ctrl.window_id),
				VI_WIN_SY(disp_ctrl.sy) |
				VI_WIN_EY(disp_ctrl.ey));

			if((disp_ctrl.scale==0) || (disp_ctrl.mode==0))
				solo6010_reg_write(solo6010, VI_LIVE_ON(disp_ctrl.window_id), 0);
			else if(disp_ctrl.mode & DISP_MODE_PB)
				solo6010_reg_write(solo6010, VI_LIVE_ON(disp_ctrl.window_id), 1<<1);
			else	// if((disp_ctrl.mode & DISP_MODE_NORMAL) || (disp_ctrl.mode & DISP_MODE_PIP))
				solo6010_reg_write(solo6010, VI_LIVE_ON(disp_ctrl.window_id), 1<<0);

			return 0;
		}

		case IOCTL_DISP_ZOOM:
		{
			struct ZOOM_CTRL zoom_ctrl;

			if(copy_from_user(&zoom_ctrl, (struct ZOOM_CTRL *)arg, sizeof(struct ZOOM_CTRL)))
				return -EINVAL;

			solo6010_reg_write(solo6010, REG_VO_ZOOM_CTRL,
				(zoom_ctrl.on ? (VO_ZOOM_VER_ON | VO_ZOOM_HOR_ON | VO_ZOOM_V_COMP) : 0) |
				VO_ZOOM_SX(zoom_ctrl.y) |
				VO_ZOOM_SY(zoom_ctrl.x));

			return 0;
		}

		case IOCTL_DISP_FREEZE_ON:
			solo6010_reg_write(solo6010, REG_VO_FREEZE_CTRL, VO_FREEZE_ON | VO_FREEZE_INTERPOLATION);
			return 0;

		case IOCTL_DISP_FREEZE_OFF:
			solo6010_reg_write(solo6010, REG_VO_FREEZE_CTRL, 0);
			return 0;

		case IOCTL_DISP_EXPANSION:
		{
			unsigned int param;

			if(copy_from_user(&param, (unsigned int *)arg, sizeof(unsigned int)))
				return -EINVAL;

			solo6010_reg_write(solo6010, REG_VO_EXPANSION((param>>28)&3), param & 0x1fff);

			return 0;
		}

		case IOCTL_DISP_BORDER:
		{
			int	i;
			struct BORDER border;

			if(copy_from_user(&border, (struct BORDER *)arg, sizeof(struct BORDER)))
				return -EINVAL;

			for(i=0;i<5;i++)
				solo6010_reg_write(solo6010,REG_VO_BORDER_X(i), border.x[i]/2);

			for(i=0;i<5;i++)
				solo6010_reg_write(solo6010,REG_VO_BORDER_Y(i), border.y[i]/2);

			solo6010_reg_write(solo6010,REG_VO_BORDER_LINE_MASK, border.line_mask);
			solo6010_reg_write(solo6010,REG_VO_BORDER_FILL_MASK, border.fill_mask);

			return 0;
		}

		case IOCTL_DISP_RECTANGLE:
		{
			struct RECTANGLE rect;

			if(copy_from_user(&rect, (struct RECTANGLE *)arg, sizeof(struct RECTANGLE) ))
				return -EINVAL;

			solo6010_reg_write(solo6010, REG_VO_RECTANGLE_CTRL(rect.id), (rect.line<<1) | rect.fill);
			solo6010_reg_write(solo6010, REG_VO_RECTANGLE_START(rect.id), ((rect.sx/2)<<10) | (rect.sy/2));
			solo6010_reg_write(solo6010, REG_VO_RECTANGLE_STOP(rect.id), ((rect.ex/2)<<10) | (rect.ey/2));

			return 0;
		}

		case IOCTL_DISP_MOTION_TRACE_ON:
			solo6010_reg_write(solo6010, REG_VI_MOTION_BORDER,
				VI_MOTION_Y_ADD |
				VI_MOTION_Y_VALUE(0x20) |
				VI_MOTION_CB_VALUE(0x10) |
				VI_MOTION_CR_VALUE(0x10));
			solo6010_reg_write(solo6010, REG_VI_MOTION_BAR,
				VI_MOTION_CR_ADD |
				VI_MOTION_Y_VALUE(0x10) |
				VI_MOTION_CB_VALUE(0x80) |
				VI_MOTION_CR_VALUE(0x10));
			return 0;

		case IOCTL_DISP_MOTION_TRACE_OFF:
			solo6010_reg_write(solo6010, REG_VI_MOTION_BORDER, 0);
			solo6010_reg_write(solo6010, REG_VI_MOTION_BAR, 0);
			return 0;

		case IOCTL_DISP_GET_XRES:
			if(copy_to_user((unsigned int *)arg, &solo6010->video_hsize, sizeof(unsigned int)))
				return -EFAULT;
			return 0;

		case IOCTL_DISP_GET_YRES:
			if(copy_to_user((unsigned int *)arg, &solo6010->video_vsize, sizeof(unsigned int)))
				return -EFAULT;
			return 0;

		case IOCTL_DISP_SET_NORM:
		{
			unsigned int norm;
			if(copy_from_user(&norm, (unsigned int *)arg, sizeof(unsigned int)))
				return -EINVAL;

      return 0;
		}
	}

	return -ENOTTY;
}

static int fops_open(struct inode *inode, struct file *filp)
{
	struct SOLO6010_DISP *disp;

	disp = (struct SOLO6010_DISP *)solo6010_device_data_get(inode_to_solo6010_device(inode));
	if(!disp)
		return -ENODEV;

	filp->private_data = disp;

	return 0;
}

static int fops_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations fops = {
	.owner =    THIS_MODULE,
	.llseek =   fops_llseek,
	.read =     fops_read,
	.write =    fops_write,
	.ioctl =    fops_ioctl,
	.open =     fops_open,
	.release =  fops_release,
};

int solo6010_disp_init(struct SOLO6010 *solo6010)
{
	struct SOLO6010_DISP *disp;

	disp = (struct SOLO6010_DISP *)kmalloc(sizeof(struct SOLO6010_DISP), GFP_KERNEL);
	if(disp == NULL)
		return -ENOMEM;

	memset(disp, 0, sizeof(struct SOLO6010_DISP));

	disp->solo6010 = solo6010;

	solo6010_disp_config(solo6010);

	disp->device = solo6010_device_add(&solo6010->chip, &fops, "disp", SOLO6010_DISP_DEV_OFFSET, (void *)disp);
	if(IS_ERR(disp->device))
	{
		int ret_val = PTR_ERR(disp->device);
		kfree(disp);
		return ret_val;
	}

	solo6010->disp = (struct SOLO6010_DISP *)disp;

	return 0;
}

void solo6010_disp_exit(struct SOLO6010 *solo6010)
{
	struct SOLO6010_DISP *disp;
	int i;

	disp = (struct SOLO6010_DISP *)solo6010->disp;

	solo6010_reg_write(solo6010, REG_VO_DISP_CTRL, 0);

	solo6010_reg_write(solo6010, REG_VO_ZOOM_CTRL, 0);
	solo6010_reg_write(solo6010, REG_VO_FREEZE_CTRL, 0);

	for(i=0; i<16; i++)
	{
		solo6010_reg_write(solo6010, VI_WIN_CTRL0(i), 0);
		solo6010_reg_write(solo6010, VI_WIN_CTRL1(i), 0);
		solo6010_reg_write(solo6010, VI_LIVE_ON(i), 0);
	}

	// set default border
	for(i=0; i<5; i++)
		solo6010_reg_write(solo6010,REG_VO_BORDER_X(i), 0);

	for(i=0; i<5; i++)
		solo6010_reg_write(solo6010,REG_VO_BORDER_Y(i), 0);

	solo6010_reg_write(solo6010,REG_VO_BORDER_LINE_MASK, 0);
	solo6010_reg_write(solo6010,REG_VO_BORDER_FILL_MASK, 0);

	solo6010_reg_write(solo6010, REG_VO_RECTANGLE_CTRL(0), 0);
	solo6010_reg_write(solo6010, REG_VO_RECTANGLE_START(0), 0);
	solo6010_reg_write(solo6010, REG_VO_RECTANGLE_STOP(0), 0);

	solo6010_reg_write(solo6010, REG_VO_RECTANGLE_CTRL(1), 0);
	solo6010_reg_write(solo6010, REG_VO_RECTANGLE_START(1), 0);
	solo6010_reg_write(solo6010, REG_VO_RECTANGLE_STOP(1), 0);

	solo6010_device_remove(disp->device);
	kfree(disp);
}

