#include <linux/module.h>
#include <linux/kernel.h>
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

#include <video/fbcon.h>
#include <video/fbcon-mfb.h>
#include <video/fbcon-cfb2.h>
#include <video/fbcon-cfb4.h>
#include <video/fbcon-cfb8.h>
#include <video/fbcon-cfb16.h>
#include <video/fbcon-cfb24.h>
#include <video/fbcon-cfb32.h>

#include "solo6010.h"
#include "fb_common.h"
#include "sl_thread.h"

struct SOLO6010FB_PAR
{
	struct SOLO6010FB_CONTROL ctrl;

	struct fb_info info;
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;
	struct display disp;

	int currcon;

	u_char colreg_red[256];
	u_char colreg_green[256];
	u_char colreg_blue[256];

#ifdef FBCON_HAS_CFB16
    u16 cfb16[16];
#endif
#ifdef FBCON_HAS_CFB24
    u32 cfb24[16];
#endif
};

#define info_to_par(fb_info)	((struct SOLO6010FB_PAR *)((char *)(fb_info) - ((size_t)&((struct SOLO6010FB_PAR *)0)->info)))

static int solo6010fb_getcolreg(u_int regno, u_int *red, u_int *green, u_int *blue, u_int *transp, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = info_to_par(info);

	if (regno > 255)
		return 1;

	*red = (par->colreg_red[regno]<<8) | par->colreg_red[regno];
	*green = (par->colreg_green[regno]<<8) | par->colreg_green[regno];
	*blue = (par->colreg_blue[regno]<<8) | par->colreg_blue[regno];
	*transp = 0;

	return 0;
}

static int solo6010fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue, u_int transp, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = info_to_par(info);

	if (regno > 255)
		return 1;

	red >>= 8;
	green >>= 8;
	blue >>= 8;
	par->colreg_red[regno] = red;
	par->colreg_green[regno] = green;
	par->colreg_blue[regno] = blue;

	return 0;
}

static void do_install_cmap(int con, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = info_to_par(info);

	if(con != par->currcon)
		return;

	if(fb_display[con].cmap.len)
		fb_set_cmap(&fb_display[con].cmap, 1, solo6010fb_setcolreg, info);
	else
		fb_set_cmap(fb_default_cmap(1<<fb_display[con].var.bits_per_pixel), 1, solo6010fb_setcolreg, info);
}

static int solo6010fb_get_fix(struct fb_fix_screeninfo *fix, int con, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = info_to_par(info);
	memcpy(fix, &par->fix, sizeof(struct fb_fix_screeninfo));

	return 0;
}

static int solo6010fb_get_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = info_to_par(info);

	if (con == -1)
		var_set_default(&par->ctrl, var);
	else
		*var = fb_display[con].var;

	set_color_bitfields(&par->ctrl, var);

	return 0;
}

static int solo6010fb_set_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;
	struct display *display;

	int err, activate = var->activate;
	int res_changed;
	int bpp_changed;

	par = info_to_par(info);

	if(con >= 0)
		display = &fb_display[con];
	else
		display = info->disp;

	if(var->vmode & FB_VMODE_CONUPDATE)
	{
		var->vmode |= FB_VMODE_YWRAP;
		var->xoffset = display->var.xoffset;
		var->yoffset = display->var.yoffset;
	}

	set_resolution(&par->ctrl, var);

	if(par->fix.line_length * var->yres_virtual > par->fix.smem_len)
		return -ENOMEM;

	if(var->bits_per_pixel != 16)
		return -EINVAL;

	set_color_bitfields(&par->ctrl, var);

	if((activate & FB_ACTIVATE_MASK) == FB_ACTIVATE_NOW)
	{
		if((display->var.xres != var->xres) ||
			(display->var.yres != var->yres) ||
			(display->var.xres_virtual != var->xres_virtual) ||
			(display->var.yres_virtual != var->yres_virtual))
			res_changed = 1;
		else
			res_changed = 0;

		if(display->var.bits_per_pixel != var->bits_per_pixel)
			bpp_changed = 1;
		else
			bpp_changed = 0;

		display->var = *var;

		if(res_changed || bpp_changed || (con < 0))
		{
			display->screen_base = (char *)par->fix.smem_start;
			display->visual = par->fix.visual;
			display->type = par->fix.type;
			display->type_aux = par->fix.type_aux;
			display->ypanstep = par->fix.ypanstep;
			display->ywrapstep = par->fix.ywrapstep;
			display->line_length = par->fix.line_length;
			display->can_soft_blank = 1;
			display->inverse = 0;

			switch (var->bits_per_pixel)
			{
#ifdef FBCON_HAS_CFB16
			case 16:
				display->dispsw = &fbcon_cfb16;
				display->dispsw_data = par->cfb16;
				break;
#endif
#ifdef FBCON_HAS_CFB24
			case 24:
				display->dispsw = &fbcon_cfb24;
				display->dispsw_data = par->cfb24;
				break;
#endif
			default:
				return -EINVAL;
			}

			if(par->info.changevar)
				(*par->info.changevar)(con);

			if(bpp_changed || (con < 0))
			{
				if((err = fb_alloc_cmap(&display->cmap, 0, 0)))
					return err;

				do_install_cmap(con, info);
			}
		}
	}

	return 0;
}

static int solo6010fb_pan_display(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
	if(var->vmode & FB_VMODE_YWRAP)
	{
		if(var->yoffset < 0 || var->yoffset >= fb_display[con].var.yres_virtual || var->xoffset)
			return -EINVAL;
	}
	else
	{
		if(var->xoffset+fb_display[con].var.xres > fb_display[con].var.xres_virtual || var->yoffset+fb_display[con].var.yres > fb_display[con].var.yres_virtual)
			return -EINVAL;
	}

	fb_display[con].var.xoffset = var->xoffset;
	fb_display[con].var.yoffset = var->yoffset;

	if (var->vmode & FB_VMODE_YWRAP)
		fb_display[con].var.vmode |= FB_VMODE_YWRAP;
	else
		fb_display[con].var.vmode &= ~FB_VMODE_YWRAP;

	return 0;
}

static int solo6010fb_get_cmap(struct fb_cmap *cmap, int kspc, int con, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = info_to_par(info);

	if(con == par->currcon)
		return fb_get_cmap(cmap, kspc, solo6010fb_getcolreg, info);
	else if(fb_display[con].cmap.len)
		fb_copy_cmap(&fb_display[con].cmap, cmap, kspc ? 0 : 2);
	else
		fb_copy_cmap(fb_default_cmap(1<<fb_display[con].var.bits_per_pixel), cmap, kspc ? 0 : 2);

	return 0;
}

static int solo6010fb_set_cmap(struct fb_cmap *cmap, int kspc, int con, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;
	int err;

	par = info_to_par(info);

	if(!fb_display[con].cmap.len)
	{
		if((err = fb_alloc_cmap(&fb_display[con].cmap, 1<<fb_display[con].var.bits_per_pixel, 0)))
			return err;
	}

	if(con == par->currcon)
		return fb_set_cmap(cmap, kspc, solo6010fb_setcolreg, info);
	else
		fb_copy_cmap(cmap, &fb_display[con].cmap, kspc ? 0 : 1);

	return 0;
}

static int vfbcon_switch(int con, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par;

	par = info_to_par(info);

	if (fb_display[par->currcon].cmap.len)
		fb_get_cmap(&fb_display[par->currcon].cmap, 1, solo6010fb_getcolreg, info);

	par->currcon = con;

	do_install_cmap(con, info);

	return 0;
}

static int vfbcon_updatevar(int con, struct fb_info *info)
{
    return 0;
}

static void vfbcon_blank(int blank, struct fb_info *info)
{
}

int solo6010fb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg, int con, struct fb_info *info)
{
	struct SOLO6010FB_PAR *par = info_to_par(info);;

	return solo6010fb_ioctl_common(&par->ctrl, cmd, arg);
}

void solo6010fb_vma_open(struct vm_area_struct *vma)
{
}

void solo6010fb_vma_close(struct vm_area_struct *vma)
{
}

struct page *solo6010fb_vma_nopage(struct vm_area_struct *vma, unsigned long address, int write)
{
	struct fb_info *info;
	unsigned long offset;
	struct page *page = NOPAGE_SIGBUS;

	info = (struct fb_info *)vma->vm_private_data;

	offset = (address - vma->vm_start) + (vma->vm_pgoff << PAGE_SHIFT);
	if(offset >= VIDEOMEMSIZE)
		return page;

	page = vmalloc_to_page(info->screen_base + offset);
	get_page(page);

	return page;
}

static struct vm_operations_struct solo6010fb_vm_ops = {
	.open = solo6010fb_vma_open,
	.close = solo6010fb_vma_close,
	.nopage = solo6010fb_vma_nopage,
};

static int solo6010fb_mmap(struct fb_info *info, struct file *file, struct vm_area_struct *vma)
{
	vma->vm_ops = &solo6010fb_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = info;
	solo6010fb_vma_open(vma);
	return 0;
}

static struct fb_ops solo6010fb_ops = {
	owner: THIS_MODULE,
	fb_get_fix: solo6010fb_get_fix,
	fb_get_var: solo6010fb_get_var,
	fb_set_var: solo6010fb_set_var,
	fb_get_cmap: solo6010fb_get_cmap,
	fb_set_cmap: solo6010fb_set_cmap,
	fb_pan_display: solo6010fb_pan_display,
	fb_ioctl: solo6010fb_ioctl,
	fb_mmap: solo6010fb_mmap,
};

int solo6010fb_init(struct SOLO6010 *solo6010)
{
	struct SOLO6010FB_PAR *par;
	int i;

	par = kmalloc(sizeof(struct SOLO6010FB_PAR), GFP_KERNEL);
	if(!par)
		return -ENOMEM;

	memset(par, 0, sizeof(struct SOLO6010FB_PAR));

	par->fix.smem_start = (u_long)vmalloc(VIDEOMEMSIZE);
	if(!par->fix.smem_start)
	{
		kfree(par);
		return -ENOMEM;
	}

	memset((void *)par->fix.smem_start, 0, VIDEOMEMSIZE);

	strcpy(par->info.modename, "SOLO6010FB");
	par->info.changevar = NULL;
	par->info.node = -1;
	par->info.fbops = &solo6010fb_ops;
	par->info.disp = &par->disp;
	par->info.switch_con = &vfbcon_switch;
	par->info.updatevar = &vfbcon_updatevar;
	par->info.blank = &vfbcon_blank;
	par->info.flags = FBINFO_FLAG_DEFAULT;
	par->info.screen_base = par->fix.smem_start;

	strcpy(par->fix.id, "SOLO6010FB");
	par->fix.smem_len = VIDEOMEMSIZE;
	par->fix.type = FB_TYPE_PACKED_PIXELS;
	par->fix.type_aux = 0;
	par->fix.visual = FB_VISUAL_TRUECOLOR;
	par->fix.ywrapstep = 1;
	par->fix.xpanstep = 1;
	par->fix.ypanstep = 1;
	par->fix.line_length = LINE_LENGTH;

	par->ctrl.solo6010 = solo6010;
	for(i=0; i<576; i++)
	{
		par->ctrl.page[i] = vmalloc_to_page(par->info.screen_base + (i<<PAGE_SHIFT));
	}

	var_set_default(&par->ctrl, &par->var);
	solo6010fb_set_var(&par->var, -1, &par->info);

	solo6010fb_alpha_rate(&par->ctrl, 10);
	solo6010fb_erase(&par->ctrl, 0, 0, par->info.var.xres, par->info.var.yres);
	solo6010fb_on(&par->ctrl);

	solo6010->fb = (void *)par;

	if(register_framebuffer(&par->info) < 0)
	{
		vfree((void *)par->fix.smem_start);
		kfree(par);
		return -EINVAL;
	}

	return 0;
}

void solo6010fb_exit(struct SOLO6010 *solo6010)
{
	struct SOLO6010FB_PAR *par;

	par = (struct SOLO6010FB_PAR *)solo6010->fb;

	unregister_framebuffer(&par->info);
	vfree((void *)par->fix.smem_start);
	kfree(par);
}

