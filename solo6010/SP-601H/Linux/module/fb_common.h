#ifndef SOLO6010FB_COMMON_H
#define SOLO6010FB_COMMON_H

#define LINE_LENGTH			PAGE_SIZE
#define EXT_LINE_SHIFT		11
#define EXT_LINE_LEHGTH		(1<<EXT_LINE_SHIFT)

#define VIDEOMEMSIZE	(LINE_LENGTH * 576)

struct SOLO6010FB_CONTROL
{
	struct SOLO6010 *solo6010;
	u32 on;
	u32 alpha_mode;
	u32 alpha_rate;

	int dma_channel;
	int byte_per_pixel;

	struct page *page[576];
};

static inline void set_resolution(struct SOLO6010FB_CONTROL *ctrl, struct fb_var_screeninfo *var)
{
	if(!var->xres)
		var->xres = ctrl->solo6010->video_hsize;
	if(!var->yres)
		var->yres = ctrl->solo6010->video_vsize * 2;
	if(var->xres_virtual > ctrl->solo6010->video_hsize)
		var->xres_virtual = ctrl->solo6010->video_hsize;
	if(var->yres_virtual > (ctrl->solo6010->video_vsize * 2))
		var->yres_virtual = ctrl->solo6010->video_vsize * 2;
	if(var->xres > (var->xres_virtual - var->xoffset))
		var->xres = (var->xres_virtual - var->xoffset);
	if(var->yres > (var->yres_virtual - var->yoffset))
		var->yres = (var->yres_virtual - var->yoffset);
}

static inline void set_color_bitfields(struct SOLO6010FB_CONTROL *ctrl, struct fb_var_screeninfo *var)
{
	if((var->bits_per_pixel != 16) && (var->bits_per_pixel != 24))
		var->bits_per_pixel = 16;

	switch(var->bits_per_pixel)
	{
		case 24:
			var->red.offset = 16;
			var->red.length = 8;
			var->green.offset = 8;
			var->green.length = 8;
			var->blue.offset = 0;
			var->blue.length = 8;
			var->transp.offset = 0;
			var->transp.length = 0;

			ctrl->byte_per_pixel = 3;
			break;
		case 16:
		default:
			var->bits_per_pixel = 16;
			if (var->transp.length) {
				var->red.offset = 10;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 5;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 15;
				var->transp.length = 1;
			} else {	/* RGB 565 */
				var->red.offset = 11;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 6;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 0;
			}

			ctrl->byte_per_pixel = 2;
			break;
	}

	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;
	var->transp.msb_right = 0;
}

static inline void var_set_default(struct SOLO6010FB_CONTROL *ctrl, struct fb_var_screeninfo *var)
{
	memset(var, 0, sizeof(struct fb_var_screeninfo));
	var->xres = ctrl->solo6010->video_hsize;
	var->yres = ctrl->solo6010->video_vsize*2;
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres;
	var->xoffset = 0;
	var->yoffset = 0;

	var->bits_per_pixel = 16;
	set_color_bitfields(ctrl, var);

	var->activate = FB_ACTIVATE_NOW;

	var->height = -1;
	var->width = -1;

	var->sync = FB_SYNC_BROADCAST;
	var->vmode = FB_VMODE_INTERLACED;
}

#define solo6010fb_control(solo6010, on, alpha_rate)	\
	solo6010_reg_write(solo6010, REG_OSG_CONFIG,		\
		(on ? VO_OSG_ON : 0) |							\
		VO_OSG_ALPHA_RATE(alpha_rate) |					\
		VO_OSG_ALPHA_BG_RATE(63 - alpha_rate) |			\
		VO_OSG_BASE(solo6010->osg_ext_addr))

static inline void solo6010fb_on(struct SOLO6010FB_CONTROL *ctrl)
{
	ctrl->on = 1;
	solo6010_reg_write(ctrl->solo6010, REG_OSG_CONFIG,
		(ctrl->on ? VO_OSG_ON : 0) |
		VO_OSG_ALPHA_RATE(ctrl->alpha_rate) |
		VO_OSG_ALPHA_BG_RATE(63 - ctrl->alpha_rate) |
		VO_OSG_BASE(ctrl->solo6010->osg_ext_addr));
}

static inline void solo6010fb_off(struct SOLO6010FB_CONTROL *ctrl)
{
	ctrl->on = 0;
	solo6010_reg_write(ctrl->solo6010, REG_OSG_CONFIG,
		(ctrl->on ? VO_OSG_ON : 0) |
		VO_OSG_ALPHA_RATE(ctrl->alpha_rate) |
		VO_OSG_ALPHA_BG_RATE(63 - ctrl->alpha_rate) |
		VO_OSG_BASE(ctrl->solo6010->osg_ext_addr));
}

static inline void solo6010fb_alpha_rate(struct SOLO6010FB_CONTROL *ctrl, unsigned int alpha_rate)
{
	ctrl->alpha_rate = (alpha_rate > 63) ? 63 : alpha_rate;
	solo6010_reg_write(ctrl->solo6010, REG_OSG_CONFIG,
		(ctrl->on ? VO_OSG_ON : 0) |
		VO_OSG_ALPHA_RATE(ctrl->alpha_rate) |
		VO_OSG_ALPHA_BG_RATE(63 - ctrl->alpha_rate) |
		VO_OSG_BASE(ctrl->solo6010->osg_ext_addr));
}

static inline void solo6010fb_flush(struct SOLO6010FB_CONTROL *ctrl, int x, int y, int width, int height)
{
	struct page *page;
	int i;

 	for(i=y; i<(y+height); i++)
	{
		page = ctrl->page[i];
		solo6010_osg_dma_page(ctrl->solo6010, ctrl->dma_channel, ctrl->alpha_mode, ctrl->byte_per_pixel, page,
			x * ctrl->byte_per_pixel, ctrl->solo6010->osg_ext_addr + (i<<EXT_LINE_SHIFT) + (x<<1), width);
	}
}

static inline void solo6010fb_erase(struct SOLO6010FB_CONTROL *ctrl, int x, int y, int width, int height)
{
	unsigned long buf;
	int i;

	buf = __get_free_page(GFP_KERNEL);
	memset((void *)buf, 0, PAGE_SIZE);

 	for(i=y; i<(y+height); i++)
	{
		solo6010_p2m_dma(ctrl->solo6010, 0, 1, (void *)buf,
			ctrl->solo6010->osg_ext_addr + (i<<EXT_LINE_SHIFT) + (x<<1), width * ctrl->byte_per_pixel);
	}

	free_page(buf);
}

static inline void solo6010_cursor_color(struct SOLO6010 *solo6010, u8 Y1, u8 Y2, u8 Y3)
{
	solo6010_reg_write(solo6010, REG_VO_CURSOR_CLR, (Y1<<24) | (0x80<<16) | (Y2<<8) | 0x80);
	solo6010_reg_write(solo6010, REG_VO_CURSOR_CLR2, (Y3<<8) | 0x80);
}

static inline void solo6010_cursor_position(struct SOLO6010 *solo6010, u32 x, u32 y)
{
	solo6010_reg_write(solo6010, REG_VO_CURSOR_POS, (x<<11) | (y<<0));
}

static inline void solo6010_cursor_on(struct SOLO6010 *solo6010)
{
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 0), CURSOR_MASK(1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 1), CURSOR_MASK(1,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 2), CURSOR_MASK(1,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 3), CURSOR_MASK(1,3,3,3,1,0,0,0,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 4), CURSOR_MASK(1,3,3,3,3,1,0,0,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 5), CURSOR_MASK(1,3,3,3,3,3,1,0,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 6), CURSOR_MASK(1,3,3,3,3,3,3,1,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 7), CURSOR_MASK(1,3,3,3,3,3,3,3,1,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 8), CURSOR_MASK(1,3,3,3,3,3,3,3,3,1,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK( 9), CURSOR_MASK(1,3,3,3,3,3,3,3,3,3,1,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(10), CURSOR_MASK(1,3,3,3,3,3,3,1,1,1,1,1,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(11), CURSOR_MASK(1,3,3,3,1,3,3,1,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(12), CURSOR_MASK(1,3,3,1,1,3,3,1,0,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(13), CURSOR_MASK(1,3,1,0,0,1,3,3,1,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(14), CURSOR_MASK(1,1,0,0,0,1,3,3,1,0,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(15), CURSOR_MASK(1,0,0,0,0,0,1,3,3,1,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(16), CURSOR_MASK(0,0,0,0,0,0,1,3,3,1,0,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(17), CURSOR_MASK(0,0,0,0,0,0,0,1,3,3,1,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(18), CURSOR_MASK(0,0,0,0,0,0,0,1,3,3,1,0,0,0,0,0));
	solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(19), CURSOR_MASK(0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0));
}

static inline void solo6010_cursor_off(struct SOLO6010 *solo6010)
{
	int i;

	for(i=0; i<20; i++)
		solo6010_reg_write(solo6010, REG_VO_CURSOR_MASK(i), 0);
}

int solo6010fb_ioctl_common(struct SOLO6010FB_CONTROL *ctrl, unsigned int cmd, unsigned long arg);

#endif

