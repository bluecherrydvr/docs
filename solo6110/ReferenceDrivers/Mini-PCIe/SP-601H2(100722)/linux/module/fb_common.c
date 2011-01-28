
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/fb.h>
#include <linux/init.h>

#include "solo6x10.h"
#include "fb_common.h"
#include <solo6x10/fb.h>

int solo6x10fb_ioctl_common(struct SOLO6x10FB_CONTROL *ctrl, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case IOCTL_FB_FLUSH:
		{
			struct SOLO6x10FB_RECT rect;

			if(copy_from_user(&rect, (void *)arg, sizeof(struct SOLO6x10FB_RECT)))
				return -EFAULT;

			rect.width = ((rect.x & 7) + rect.width + 7) & ~7;
			rect.x = rect.x & ~7;
		 	solo6x10fb_flush(ctrl, rect.x, rect.y, rect.width, rect.height);

			return 0;
		}

		case IOCTL_FB_ERASE:
		{
			struct SOLO6x10FB_RECT rect;

			if(copy_from_user(&rect, (void *)arg, sizeof(struct SOLO6x10FB_RECT)))
				return -EFAULT;

			solo6x10fb_erase(ctrl, rect.x, rect.y, rect.width, rect.height);

			return 0;
		}

		case IOCTL_FB_CURSOR_ON:
			solo6x10_cursor_on(ctrl->solo6x10);
			return 0;

		case IOCTL_FB_CURSOR_OFF:
			solo6x10_cursor_off(ctrl->solo6x10);
			return 0;

		case IOCTL_FB_CURSOR_POS:
		{
			struct SOLO6x10FB_CURSOR_POS cursor_pos;
			if(copy_from_user(&cursor_pos, (void *)arg, sizeof(struct SOLO6x10FB_CURSOR_POS)))
				return -EFAULT;
			solo6x10_cursor_position(ctrl->solo6x10, cursor_pos.x, cursor_pos.y);
			return 0;
		}

		case IOCTL_FB_ALPHA_RATE:
		{
			unsigned int alpha_rate;
			if(copy_from_user(&alpha_rate, (void *)arg, sizeof(unsigned int)))
				return -EFAULT;
			ctrl->alpha_mode = ALPHA_MODE(alpha_rate);
			solo6x10fb_alpha_rate(ctrl, ALPHA_VALUE(alpha_rate));
			return 0;
		}

		default:
			return -EINVAL;
	}

	return -EINVAL;
}

