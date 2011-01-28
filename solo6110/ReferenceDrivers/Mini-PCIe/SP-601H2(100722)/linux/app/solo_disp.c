#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "solo_disp.h"

const unsigned int disp_scale[] = {1, 3, 4, 5, 4, 5};
const unsigned int disp_nr_channel[] = {1, 4, 6, 8, 9, 16};
const unsigned int disp_div[] = {1, 2, 3, 4, 3, 4};

int solo_disp_pos2channel(SOLO_DISP *disp, int x, int y)
{
	int i;

	if(disp->pip_ctrl)
		return disp->pip_ctrl->channel;

	for(i=0; i<16; i++)
	{
		struct DISP_CTRL *disp_ctrl;

		disp_ctrl = &disp->disp_ctrl[i];
		if((x >= disp_ctrl->sx) && (x < disp_ctrl->ex) && (y >= disp_ctrl->sy) && (y < disp_ctrl->ey))
			return disp_ctrl->channel;
	}

	return -1;
}

void solo_disp_channel_mode(SOLO_DISP *disp, unsigned int channel, unsigned int mode)
{
	disp->disp_ctrl[channel].mode = mode;
	ioctl(disp->fd, IOCTL_DISP_CHANNEL, &disp->disp_ctrl[channel]);
}

int solo_disp_mux_mode(SOLO_DISP *disp, unsigned int mode)
{
	struct BORDER border;
	unsigned int width;
	unsigned int height;
	unsigned int i;

	if(disp->zoom)
		return -1;

	if(disp->pip_ctrl)
		return -1;

	ioctl(disp->fd, IOCTL_DISP_ERASE_ON);
	width = (disp->width / disp_div[mode]) & (~7);
	height = (disp->height / disp_div[mode]) & (~7);

	bzero(&border,sizeof(struct BORDER));
	for(i=0; i<disp_div[mode]; i++)
	{
		int j;
		border.x[i] = width * i;
		border.y[i] = height * i;
	}
	border.x[disp_div[mode]] = disp->width - 2;
	border.y[disp_div[mode]] = disp->height - 2;

	for(i=0; i<16; i++)
	{
		struct DISP_CTRL *disp_ctrl;

		disp_ctrl = &disp->disp_ctrl[i];
		if(i < disp_nr_channel[mode])
		{

			switch(mode)
			{
				case MUX_MODE_6CH:
					if(i==0)
					{
						disp_ctrl->scale = 6;
						disp_ctrl->sx = 0;
						disp_ctrl->sy = 0;
						disp_ctrl->ex = width * 2;
						disp_ctrl->ey = height * 2;
					}
					else
					{
						int j;

						switch(i)
						{
							case 1:	j = 2;	break;
							case 2:	j = 5;	break;
							case 3:	j = 6;	break;
							case 4:	j = 7;	break;
							case 5:	j = 8;	break;
						}

						disp_ctrl->scale = disp_scale[mode];
						disp_ctrl->sx = width * (j%disp_div[mode]);
						disp_ctrl->sy = height * (j/disp_div[mode]);
						disp_ctrl->ex = disp_ctrl->sx + width;
						disp_ctrl->ey = disp_ctrl->sy + height;
						border.line_mask |= 1<<(((j/disp_div[mode])*4) + (j%disp_div[mode]));
					}
					break;
				case MUX_MODE_8CH:
					if(i==0)
					{
						disp_ctrl->scale = 7;
						disp_ctrl->sx = 0;
						disp_ctrl->sy = 0;
						disp_ctrl->ex = width * 3;
						disp_ctrl->ey = height * 3;
					}
					else
					{
						int j;
						switch(i)
						{
							case 1:	j = 3;	break;
							case 2:	j = 7;	break;
							case 3:	j = 11;	break;
							case 4:	j = 12;	break;
							case 5:	j = 13;	break;
							case 6:	j = 14;	break;
							case 7:	j = 15;	break;
						}

						disp_ctrl->scale = disp_scale[mode];
						disp_ctrl->sx = width * (j%disp_div[mode]);
						disp_ctrl->sy = height * (j/disp_div[mode]);
						disp_ctrl->ex = disp_ctrl->sx + width;
						disp_ctrl->ey = disp_ctrl->sy + height;
						border.line_mask |= 1<<(((j/disp_div[mode])*4) + (j%disp_div[mode]));
					}
					break;
				default:
					disp_ctrl->scale = disp_scale[mode];
					disp_ctrl->sx = width * (i%disp_div[mode]);
					disp_ctrl->sy = height * (i/disp_div[mode]);
					disp_ctrl->ex = disp_ctrl->sx + width;
					disp_ctrl->ey = disp_ctrl->sy + height;
					border.line_mask |= 1<<(((i/disp_div[mode])*4) + (i%disp_div[mode]));
			}
		}
		else
		{
			disp_ctrl->scale = 0;
			disp_ctrl->sx = 0;
			disp_ctrl->sy = 0;
			disp_ctrl->ex = 0;
			disp_ctrl->ey = 0;
		}

		ioctl(disp->fd, IOCTL_DISP_CHANNEL, &disp->disp_ctrl[i]);
	}

	if((disp->width == 704) && ((mode == MUX_MODE_6CH) || (mode == MUX_MODE_9CH)))
	{
		i = (0<<28) | (1<<12) | (0<<0);
		ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
		i = (1<<28) | (1<<12) | (2<<0);
		ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
		i = (2<<28) | (1<<12) | (698<<0);
		ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
		i = (3<<28) | (1<<12) | (700<<0);
		ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
		border.x[1] += 4;
		border.x[2] += 4;
	}
	else
	{
		i = (0<<28) | 0;
		ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
		i = (1<<28) | 0;
		ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
		i = (2<<28) | 0;
		ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
		i = (3<<28) | 0;
		ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
	}

	ioctl(disp->fd, IOCTL_DISP_BORDER, &border);
	ioctl(disp->fd, IOCTL_DISP_ERASE_OFF);
	disp->mux_mode = mode;

	return 0;
}

int solo_disp_select_channel(SOLO_DISP *disp, int channel)
{
	struct BORDER border;
	unsigned int i;

	if(disp->zoom)
		return -1;

	if(disp->pip_ctrl)
		return -1;

	ioctl(disp->fd, IOCTL_DISP_ERASE_ON);

	bzero(&border,sizeof(struct BORDER));
	border.x[1] = disp->width - 2;
	border.y[1] = disp->height - 2;

	for(i=0; i<16; i++)
	{
		struct DISP_CTRL *disp_ctrl;

		disp_ctrl = &disp->disp_ctrl[i];
		if(disp_ctrl->channel == channel)
		{
			disp_ctrl->scale = disp_scale[0];
			disp_ctrl->sx = 0;
			disp_ctrl->sy = 0;
			disp_ctrl->ex = disp->width;
			disp_ctrl->ey = disp->height;
			border.line_mask |= 1;
		}
		else
		{
			disp_ctrl->scale = 0;
			disp_ctrl->sx = 0;
			disp_ctrl->sy = 0;
			disp_ctrl->ex = 0;
			disp_ctrl->ey = 0;
		}

		ioctl(disp->fd, IOCTL_DISP_CHANNEL, &disp->disp_ctrl[i]);
	}

	i = (0<<28) | 0;
	ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
	i = (1<<28) | 0;
	ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
	i = (2<<28) | 0;
	ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);
	i = (3<<28) | 0;
	ioctl(disp->fd, IOCTL_DISP_EXPANSION, &i);

	ioctl(disp->fd, IOCTL_DISP_BORDER, &border);
	ioctl(disp->fd, IOCTL_DISP_ERASE_OFF);
	disp->mux_mode = MUX_MODE_1CH;

	return 0;
}

int solo_disp_pip(SOLO_DISP *disp, int channel, int x, int y)
{
	struct RECTANGLE rect;

	if(disp->mux_mode != MUX_MODE_1CH)
		return -1;

	if(disp->zoom)
		return -1;

	if(channel == -1)
	{
		if(disp->pip_ctrl)
		{
			disp->pip_ctrl->mode &= ~DISP_MODE_PIP;
			disp->pip_ctrl->scale = 0;
			disp->pip_ctrl->ex = disp->pip_ctrl->sx;
			disp->pip_ctrl->ey = disp->pip_ctrl->sy;
			ioctl(disp->fd, IOCTL_DISP_CHANNEL, disp->pip_ctrl);
			rect.id = 1;
			rect.line = 0;
			rect.fill = 0;
			rect.sx = 0;
			rect.sy = 0;
			rect.ex = 0;
			rect.ey = 0;
			ioctl(disp->fd, IOCTL_DISP_RECTANGLE, &rect);
			disp->pip_ctrl = NULL;
		}
		else
			return -1;
	}
	else
	{
		struct DISP_CTRL *disp_ctrl = NULL;
		int width;
		int height;
		int i;

		for(i=0; i<16; i++)
		{
			if(disp->disp_ctrl[i].channel == channel)
			{
				disp_ctrl = &disp->disp_ctrl[i];
				break;
			}
		}

		if((disp_ctrl==NULL) || disp_ctrl->scale)
			return -1;

		width = (disp->width/3) & ~7;
		height = (disp->height/3) & ~7;

		if(x > (disp->width - width))
			x = disp->width - width;
		if(y > (disp->height - height))
			y = disp->height - height;

		x &= ~7;
		y &= ~7;

		disp_ctrl->scale = 4;
		disp_ctrl->sx = x;
		disp_ctrl->sy = y;
		disp_ctrl->ex = x + width;
		disp_ctrl->ey = y + height;
		disp_ctrl->mode |= DISP_MODE_PIP;
		ioctl(disp->fd, IOCTL_DISP_CHANNEL, disp_ctrl);

		rect.id = 1;
		rect.line = 1;
		rect.fill = 0;
		rect.sx = x;
		rect.sy = y;
		rect.ex = x + width;
		rect.ey = y + height;
		ioctl(disp->fd, IOCTL_DISP_RECTANGLE, &rect);

		disp->pip_ctrl = disp_ctrl;
	}

	return 0;
}

int solo_disp_pip_move(SOLO_DISP *disp, int dx, int dy)
{
	struct RECTANGLE rect;
	int x;
	int y;
	int width;
	int height;

	if(!disp->pip_ctrl)
		return -1;

	x = disp->pip_ctrl->sx;
	y = disp->pip_ctrl->sy;
	width = (disp->width/3) & ~7;
	height = (disp->height/3) & ~7;

	x += dx;
	y += dy;
	if(x < 0)
		x = 0;
	if(x > (disp->width-width))
		x = disp->width-width;
	if(y < 0)
		y = 0;
	if(y > (disp->height-height))
		y = disp->height-height;

	x &= ~7;
	y &= ~7;

	disp->pip_ctrl->sx = x;
	disp->pip_ctrl->sy = y;
	disp->pip_ctrl->ex = x + width;
	disp->pip_ctrl->ey = y + height;
	ioctl(disp->fd, IOCTL_DISP_CHANNEL, disp->pip_ctrl);

	rect.id = 1;
	rect.line = 1;
	rect.fill = 0;
	rect.sx = x;
	rect.sy = y;
	rect.ex = x + width;
	rect.ey = y + height;
	ioctl(disp->fd, IOCTL_DISP_RECTANGLE, &rect);
}

int solo_disp_zoom(SOLO_DISP *disp, int on, int x, int y)
{
	struct ZOOM_CTRL zoom_ctrl;
	int ret;

	if(disp->mux_mode != MUX_MODE_1CH)
		return -1;

	if(disp->pip_ctrl)
		return -1;

	zoom_ctrl.on = on;

	if(x < 0)
		zoom_ctrl.x = 0;
	else if(x > (disp->width/2))
		zoom_ctrl.x = disp->width/2;
	else
		zoom_ctrl.x = x;

	if(y < 0)
		zoom_ctrl.y = 0;
	else if(y > (disp->height/2))
		zoom_ctrl.y = disp->height/2;
	else
		zoom_ctrl.y = y;

	ret = ioctl(disp->fd, IOCTL_DISP_ZOOM, &zoom_ctrl);

	if(ret==0)
		disp->zoom = on;

	return ret;
}

int solo_disp_freeze(SOLO_DISP *disp, int on)
{
	if(on)
		return ioctl(disp->fd, IOCTL_DISP_FREEZE_ON);
	else
		return ioctl(disp->fd, IOCTL_DISP_FREEZE_OFF);
}

int solo_disp_motion_trace(SOLO_DISP *disp, int on)
{
	if(on)
		return ioctl(disp->fd, IOCTL_DISP_MOTION_TRACE_ON);
	else
		return ioctl(disp->fd, IOCTL_DISP_MOTION_TRACE_OFF);
}

SOLO_DISP *solo_disp_open(char *path, unsigned int mode)
{
	SOLO_DISP *disp;
	struct RECTANGLE rect;
	int i;
	unsigned int width;
	unsigned int height;

	disp = malloc(sizeof(SOLO_DISP));
	if(!disp)
		return NULL;

	bzero(disp, sizeof(SOLO_DISP));

	disp->fd = open(path, O_RDWR);

	ioctl(disp->fd, IOCTL_DISP_GET_XRES, &width);
	ioctl(disp->fd, IOCTL_DISP_GET_YRES, &height);
	height = height * 2;

	rect.id = 0;
	rect.line = 1;
	rect.fill = 0;
	rect.sx = 0;
	rect.sy = 0;
	rect.ex = width-2;
	rect.ey = height-2;
	ioctl(disp->fd, IOCTL_DISP_RECTANGLE, &rect);

	disp->width = width;
	disp->height = height;
	disp->mux_mode = mode;
	for(i=0; i<16; i++)
	{
		struct DISP_CTRL *disp_ctrl;

		disp_ctrl = &disp->disp_ctrl[i];
		disp_ctrl->window_id = i;
		disp_ctrl->channel = i;
		disp_ctrl->mode = DISP_MODE_NORMAL;
	}

	disp->pip_ctrl = NULL;

	solo_disp_zoom(disp, 0, 0, 0);
	solo_disp_mux_mode(disp, mode);
	return disp;
}

void solo_disp_close(SOLO_DISP *disp)
{
	solo_disp_mux_mode(disp, MUX_MODE_16CH);
	free(disp);
}

