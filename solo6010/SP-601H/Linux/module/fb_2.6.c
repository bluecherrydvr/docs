
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

#include "solo6010.h"
#include "fb_common.h"
#include <solo6010/fb.h>

struct SOLO6010FB_PAR
{
	struct SOLO6010FB_CONTROL ctrl;

	u32 pseudo_palette[256];
};

#if 0
static ssize_t solo6010fb_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct inode *inode = file->f_dentry->d_inode;
	int fbidx = iminor(inode);
	struct fb_info *info = registered_fb[fbidx];
	int xoffset;
	int yoffset;
	int copy_size;
	char *user_mem = buf;
	unsigned int pos;

	pos = *ppos;

	xoffset = pos % (info->var.xres * 2);
	yoffset = pos / (info->var.xres * 2);

	while(count)
	{
		if(yoffset >= info->var.yres)
			break;

		copy_size = (info->var.xres * 2) - xoffset;
		if(copy_size > count)
			copy_size = count;

		copy_to_user(user_mem, info->screen_base + (yoffset * info->fix.line_length) + (xoffset * 2), copy_size);
		user_mem += copy_size;
		xoffset = 0;
		yoffset++;
		count -= copy_size;
	}

	*ppos += user_mem - buf;

	return user_mem - buf;
}

static ssize_t solo6010fb_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct inode *inode = file->f_dentry->d_inode;
	int fbidx = iminor(inode);
	struct fb_info *info = registered_fb[fbidx];
	int xoffset;
	int yoffset;
	int copy_size;
	const char *user_mem = buf;
	unsigned int pos;

	pos = *ppos;

	xoffset = pos % (info->var.xres * 2);
	yoffset = pos / (info->var.xres * 2);

	while(count)
	{
		if(yoffset >= info->var.yres)
			break;

		copy_size = (info->var.xres * 2) - xoffset;
		if(copy_size > count)
			copy_size = count;

		copy_from_user(info->screen_base + (yoffset * info->fix.line_length) + (xoffset * 2), user_mem, copy_size);
		user_mem += copy_size;
		xoffset = 0;
		yoffset++;
		count -= copy_size;
	}

	*ppos += user_mem - buf;

	return user_mem - buf;
}
#endif
static int solo6010fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = info->par;

	if(var->vmode & FB_VMODE_CONUPDATE)
	{
		var->vmode |= FB_VMODE_YWRAP;
		var->xoffset = info->var.xoffset;
		var->yoffset = info->var.yoffset;
	}

	set_resolution(&par->ctrl, var);

	if (info->fix.line_length * var->yres_virtual > VIDEOMEMSIZE)
		return -ENOMEM;

	dbg_msg("var->bits_per_pixel : %d\n", var->bits_per_pixel);

	set_color_bitfields(&par->ctrl, var);

	dbg_msg("var->bits_per_pixel : %d var->transp.length : %d\n", var->bits_per_pixel, var->transp.length);

	return 0;
}

static int solo6010fb_set_par(struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;
	struct SOLO6010FB_CONTROL *ctrl;
	struct fb_var_screeninfo *var;

	par = info->par;
	ctrl = &par->ctrl;
	var = &info->var;

	if((ctrl->byte_per_pixel == 2) && (var->transp.length == 0))
	{
		solo6010_reg_write(ctrl->solo6010, REG_P2M_CONFIG(ctrl->dma_channel),
			P2M_CSC_16BIT_565 | P2M_PCI_MASTER_MODE);
	}
	else
	{
		solo6010_reg_write(ctrl->solo6010, REG_P2M_CONFIG(ctrl->dma_channel),
			P2M_PCI_MASTER_MODE);
	}

	return 0;
}

static int solo6010fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *info)
{
	if (regno >= 256)	/* no. of hw registers */
		return 1;
	/*
	 * Program hardware... do anything you want with transp
	 */

	/* grayscale works only partially under directcolor */
	if (info->var.grayscale) {
		/* grayscale = 0.30*R + 0.59*G + 0.11*B */
		red = green = blue =
		    (red * 77 + green * 151 + blue * 28) >> 8;
	}

	/* Directcolor:
	 *   var->{color}.offset contains start of bitfield
	 *   var->{color}.length contains length of bitfield
	 *   {hardwarespecific} contains width of RAMDAC
	 *   cmap[X] is programmed to (X << red.offset) | (X << green.offset) | (X << blue.offset)
	 *   RAMDAC[X] is programmed to (red, green, blue)
	 *
	 * Pseudocolor:
	 *    uses offset = 0 && length = RAMDAC register width.
	 *    var->{color}.offset is 0
	 *    var->{color}.length contains widht of DAC
	 *    cmap is not used
	 *    RAMDAC[X] is programmed to (red, green, blue)
	 * Truecolor:
	 *    does not use DAC. Usually 3 are present.
	 *    var->{color}.offset contains start of bitfield
	 *    var->{color}.length contains length of bitfield
	 *    cmap is programmed to (red << red.offset) | (green << green.offset) |
	 *                      (blue << blue.offset) | (transp << transp.offset)
	 *    RAMDAC does not exist
	 */
#define CNVT_TOHW(val,width) ((((val)<<(width))+0x7FFF-(val))>>16)
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
		red = CNVT_TOHW(red, info->var.red.length);
		green = CNVT_TOHW(green, info->var.green.length);
		blue = CNVT_TOHW(blue, info->var.blue.length);
		transp = CNVT_TOHW(transp, info->var.transp.length);
		break;
	case FB_VISUAL_DIRECTCOLOR:
		red = CNVT_TOHW(red, 8);	/* expect 8 bit DAC */
		green = CNVT_TOHW(green, 8);
		blue = CNVT_TOHW(blue, 8);
		/* hey, there is bug in transp handling... */
		transp = CNVT_TOHW(transp, 8);
		break;
	}
#undef CNVT_TOHW
	/* Truecolor has hardware independent palette */
	if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
		u32 v;

		if (regno >= 16)
			return 1;

		v = (red << info->var.red.offset) |
		    (green << info->var.green.offset) |
		    (blue << info->var.blue.offset) |
		    (transp << info->var.transp.offset);
		switch (info->var.bits_per_pixel) {
		case 8:
			break;
		case 16:
			((u32 *) (info->pseudo_palette))[regno] = v;
			break;
		case 24:
		case 32:
			((u32 *) (info->pseudo_palette))[regno] = v;
			break;
		}
		return 0;
	}
	return 0;
}

static int solo6010fb_blank(int blank, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = (struct SOLO6010FB_PAR *)info->par;

	if(blank)
	{
		solo6010fb_off(&par->ctrl);
	}
	else
	{
		solo6010fb_on(&par->ctrl);
	}

	return 0;
}

static int solo6010fb_pan_display(struct fb_var_screeninfo *var,
			   struct fb_info *info)
{
	if (var->vmode & FB_VMODE_YWRAP) {
		if (var->yoffset < 0
		    || var->yoffset >= info->var.yres_virtual
		    || var->xoffset)
			return -EINVAL;
	} else {
		if (var->xoffset + var->xres > info->var.xres_virtual ||
		    var->yoffset + var->yres > info->var.yres_virtual)
			return -EINVAL;
	}
	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;
	if (var->vmode & FB_VMODE_YWRAP)
		info->var.vmode |= FB_VMODE_YWRAP;
	else
		info->var.vmode &= ~FB_VMODE_YWRAP;

	return 0;
}

#if 0
static int solo6010fb_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	struct SOLO6010FB_PAR *par;

	par = (struct SOLO6010FB_PAR *)info->par;

	if(cursor->set & FB_CUR_SETPOS)
	{
		info->cursor.image.dx = cursor->image.dx;
		info->cursor.image.dy = cursor->image.dy;
		solo6010_reg_write(par->solo6010, REG_VO_CURSOR_POS,
			(info->cursor.image.dx<<11) | (info->cursor.image.dy<<0));
	}

	if(info->cursor.enable)
	{
		//solo6010_cursor_on(par->solo6010);
	}
	else
	{
		//solo6010_cursor_off(par->solo6010);
	}

	return 0;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 16))
static int solo6010fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
#else
static int solo6010fb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg, struct fb_info *info)
#endif
{
	struct SOLO6010FB_PAR *par = info->par;

	return solo6010fb_ioctl_common(&par->ctrl, cmd, arg);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 16))
static int solo6010fb_mmap(struct fb_info *info, struct vm_area_struct *vma)
#else
static int solo6010fb_mmap(struct fb_info *info, struct file *file, struct vm_area_struct *vma)
#endif
{
	struct SOLO6010FB_PAR *par = (struct SOLO6010FB_PAR *)info->par;
	unsigned long from = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long pos;

	if((vma->vm_pgoff << PAGE_SHIFT) + size > info->fix.smem_len)
		return -EINVAL;

	pos = vma->vm_pgoff;

	while(size > 0)
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 10))
		if (remap_pfn_range(vma, from, page_to_pfn(par->ctrl.page[pos]), PAGE_SIZE, PAGE_SHARED))
#else
		if (remap_page_range(vma, from, page_to_pfn(par->ctrl.page[pos])<<PAGE_SHIFT, PAGE_SIZE, PAGE_SHARED))
#endif
			return -EAGAIN;

		pos++;
		from += PAGE_SIZE;
		if (size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}

	vma->vm_flags |= VM_RESERVED;
	return 0;
}

static struct fb_ops solo6010fb_ops = {
#if 0
	.fb_read = solo6010fb_read,
	.fb_write = solo6010fb_write,
#endif
	.fb_check_var = solo6010fb_check_var,
	.fb_set_par = solo6010fb_set_par,
	.fb_setcolreg = solo6010fb_setcolreg,
	.fb_blank = solo6010fb_blank,
	.fb_pan_display = solo6010fb_pan_display,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22))
	.fb_fillrect = sys_fillrect,
	.fb_copyarea = sys_copyarea,
	.fb_imageblit = sys_imageblit,
#else
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
#endif
	.fb_ioctl = solo6010fb_ioctl,
	.fb_mmap = solo6010fb_mmap,
};

int solo6010fb_init(struct SOLO6010 *solo6010)
{
	struct fb_info *info;
	struct SOLO6010FB_PAR *par;
	int retval = -ENOMEM;
	int i;

	info = framebuffer_alloc(sizeof(struct SOLO6010FB_PAR), &solo6010->pdev->dev);
	if(!info)
		return retval;

	par = (struct SOLO6010FB_PAR *)info->par;
	par->ctrl.solo6010 = solo6010;

	info->screen_base = vmalloc(VIDEOMEMSIZE);
	if(!info->screen_base)
		goto err;

	memset(info->screen_base, 0, VIDEOMEMSIZE);

	for(i=0; i<576; i++)
	{
		par->ctrl.page[i] = vmalloc_to_page(info->screen_base + (i<<PAGE_SHIFT));
		SetPageReserved(par->ctrl.page[i]);
	}

	strcpy(info->fix.id, "SOLO6010FB");
	info->fix.smem_start = (unsigned long)info->screen_base;
	info->fix.smem_len = VIDEOMEMSIZE;
	info->fix.type = FB_TYPE_PACKED_PIXELS;
	info->fix.visual = FB_VISUAL_TRUECOLOR;
	info->fix.xpanstep = 0;
	info->fix.ypanstep = 0;
	info->fix.ywrapstep = 0;
	info->fix.line_length = LINE_LENGTH;
	info->fix.mmio_start = 0;
	info->fix.mmio_len = 0;
	info->fix.accel = FB_ACCEL_NONE;

	var_set_default(&par->ctrl, &info->var);

	info->flags = FBINFO_FLAG_DEFAULT;

	info->pseudo_palette = par->pseudo_palette;

	info->fbops = &solo6010fb_ops;

	solo6010fb_alpha_rate(&par->ctrl, 10);
	solo6010fb_erase(&par->ctrl, 0, 0, info->var.xres, info->var.yres);
	solo6010fb_on(&par->ctrl);

	// set default cursor
	solo6010_cursor_off(solo6010);
	solo6010_cursor_position(solo6010, 352, 240);
	solo6010_cursor_color(solo6010, 0x80, 0x10, 0xe0);

	retval = fb_alloc_cmap(&info->cmap, 256, 0);
	if(retval < 0)
		goto err2;

	retval = register_framebuffer(info);
	if(retval < 0)
		goto err3;

	solo6010->fb = (void *)info;

	return 0;
err3:
	fb_dealloc_cmap(&info->cmap);
err2:
	vfree(info->screen_base);
err:
	framebuffer_release(info);
	return retval;
}

void solo6010fb_exit(struct SOLO6010 *solo6010)
{
	struct fb_info *info;
	struct SOLO6010FB_PAR *par;

	info = (struct fb_info *)solo6010->fb;
	par = (struct SOLO6010FB_PAR *)info->par;

	if(info)
	{
		int i;

		unregister_framebuffer(info);
		solo6010fb_off(&par->ctrl);
		fb_dealloc_cmap(&info->cmap);
		for(i=0; i<576; i++)
		{
			ClearPageReserved(par->ctrl.page[i]);
		}
		vfree(info->screen_base);
		framebuffer_release(info);
	}
}

