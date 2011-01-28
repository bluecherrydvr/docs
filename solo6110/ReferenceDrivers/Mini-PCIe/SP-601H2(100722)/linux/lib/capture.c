#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>  
#include <unistd.h>
#include <error.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/videodev.h>
#include <sys/types.h>
#include <fcntl.h>

#include "scode.h"
#include "../module/include/solo6x10/enc.h"
#include "../module/include/solo6x10/disp.h"
#include "../module/include/solo6x10/dec.h"

static char enc_device_path[256] = "/dev/solo6110_enc0";
static char disp_device_path[256] = "/dev/solo6110_disp0";
static char dec_device_path[256] = "/dev/solo6110_dec0";

const unsigned int disp_scale[] = {1, 3, 4, 5, 4, 5};
const unsigned int disp_nr_channel[] = {1, 4, 6, 8, 9, 16};
const unsigned int disp_div[] = {1, 2, 3, 4, 3, 4};


void enc_para_set(struct ENC_TBL *enc_tbl, int channel, int scale, int qp, int intvl, int gop, int mask)
{
	struct CAP_SCALE sscale;
	struct MP4E_GOPSIZE sgop;
	struct MP4E_QP sqp;
	struct CAP_INTERVAL sintvl;
	sscale.channel = channel;
	sscale.value = (channel<16) ? scale : 0;
	sgop.channel = channel;
	sgop.value = gop;
	sqp.channel = channel;
	sqp.value = qp;
	sintvl.channel = channel;
	sintvl.value = intvl;

    if(mask&0x4)
    	ioctl(enc_tbl->fd, IOCTL_CAP_SET_INTERVAL, &sintvl);

    if(mask&0x2)
        ioctl(enc_tbl->fd, IOCTL_MP4E_SET_QP, &sqp);

    if(mask&0x8)
        ioctl(enc_tbl->fd, IOCTL_MP4E_SET_GOPSIZE, &sgop);

    if(mask&0x1)
    	ioctl(enc_tbl->fd, IOCTL_CAP_SET_SCALE, &sscale);
}

void enc_para_get(struct ENC_TBL *enc_tbl, int channel, int *scale, int *qp, int *intvl, int *gop)
{
	struct CAP_SCALE sscale;
	struct MP4E_GOPSIZE sgop;
	struct MP4E_QP sqp;
	struct CAP_INTERVAL sintvl;
	sscale.channel = channel;
	sgop.channel = channel;
	sqp.channel = channel;
	sintvl.channel = channel;
    
	ioctl(enc_tbl->fd, IOCTL_CAP_GET_INTERVAL, &sintvl);
	ioctl(enc_tbl->fd, IOCTL_MP4E_GET_QP, &sqp);
	ioctl(enc_tbl->fd, IOCTL_MP4E_GET_GOPSIZE, &sgop);
	ioctl(enc_tbl->fd, IOCTL_CAP_GET_SCALE, &sscale);

    *scale = sscale.value;
    *qp = sqp.value;
    *intvl = sintvl.value;
    *gop = sgop.value;
}

void enc_chan_preset(struct ENC_TBL *enc_tbl, int key, int channel)
{
	struct CAP_SCALE scale;
	struct MP4E_GOPSIZE gop;
	struct MP4E_QP qp;
	struct CAP_INTERVAL intvl;

    if(key==3)
    {
    	scale.channel = channel;
    	scale.value = (channel<4) ? 9 : 0;
    	gop.channel = channel;
    	gop.value = 30;
    	qp.channel = channel;
    	qp.value = 4;
    	intvl.channel = channel;
    	intvl.value = 0;
    }
    else if(key==1)
    {
    	scale.channel = channel;
    	scale.value = (channel<16) ? 2 : 0;
    	gop.channel = channel;
    	gop.value = 30;
    	qp.channel = channel;
    	qp.value = 4;
    	intvl.channel = channel;
    	intvl.value = 1;
    }
    else
    {
    	scale.channel = channel;
    	scale.value = 0;
    	gop.channel = channel;
    	gop.value = 30;
    	qp.channel = channel;
    	qp.value = 4;
    	intvl.channel = channel;
    	intvl.value = 1;
    }
    
	ioctl(enc_tbl->fd, IOCTL_CAP_SET_INTERVAL, &intvl);
	ioctl(enc_tbl->fd, IOCTL_MP4E_SET_QP, &qp);
	ioctl(enc_tbl->fd, IOCTL_MP4E_SET_GOPSIZE, &gop);
	ioctl(enc_tbl->fd, IOCTL_CAP_SET_SCALE, &scale);

	scale.channel = channel+16;
	scale.value = 0;
	gop.channel = channel+16;
	gop.value = 30;
	qp.channel = channel+16;
	qp.value = 4;
	intvl.channel = channel+16;
	intvl.value = 0;

	ioctl(enc_tbl->fd, IOCTL_CAP_SET_INTERVAL, &intvl);
	ioctl(enc_tbl->fd, IOCTL_MP4E_SET_QP, &qp);
	ioctl(enc_tbl->fd, IOCTL_MP4E_SET_GOPSIZE, &gop);
	ioctl(enc_tbl->fd, IOCTL_CAP_SET_SCALE, &scale);
}

void enc_set_wm(struct ENC_TBL *enc_tbl, int poly, int key, int str)
{
    struct WM watermark;
    watermark.polynomial = poly;
    watermark.initialkey = key;
    watermark.strength   = str;

    ioctl(enc_tbl->fd, IOCTL_ENC_SETWM, &watermark);
}

int enc_tbl_init(struct ENC_TBL *enc_tbl)
{
	char str[COL_WIDTH+1];
	int channel;

	enc_tbl->fd = open(enc_device_path, O_RDWR);
	if(enc_tbl->fd == -1)
		return -1;

	return 0;
}

void enc_tbl_exit(struct ENC_TBL *enc_tbl)
{
    close(enc_tbl->fd);
}

int solo6x10_display_motion_trace(SOLO6x10_DISPLAY *disp, int on)
{
	if(on)
		return ioctl(disp->fd, IOCTL_DISP_MOTION_TRACE_ON);
	else
		return ioctl(disp->fd, IOCTL_DISP_MOTION_TRACE_OFF);
}

void solo6x10_display_channel_mode(SOLO6x10_DISPLAY *disp, unsigned int channel, unsigned int mode)
{
	disp->disp_ctrl[channel].mode = mode;
	ioctl(disp->fd, IOCTL_DISP_CHANNEL, &disp->disp_ctrl[channel]);
}

int solo6x10_display_zoom(SOLO6x10_DISPLAY *disp, int on, int x, int y)
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

int solo6x10_display_mux_mode(SOLO6x10_DISPLAY *disp, unsigned int mode)
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
			disp_ctrl->scale = disp_scale[mode];
			disp_ctrl->sx = width * (i%disp_div[mode]);
			disp_ctrl->sy = height * (i/disp_div[mode]);
			disp_ctrl->ex = disp_ctrl->sx + width;
			disp_ctrl->ey = disp_ctrl->sy + height;
			border.line_mask |= 1<<(((i/disp_div[mode])*4) + (i%disp_div[mode]));
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

SOLO6x10_DISPLAY *solo6x10_display_open(const char *path, unsigned int mode)
{
	SOLO6x10_DISPLAY *disp;
	struct RECTANGLE rect;
	int i;
	unsigned int width;
	unsigned int height;

	disp = malloc(sizeof(SOLO6x10_DISPLAY));
	if(!disp)
		return NULL;

	bzero(disp, sizeof(SOLO6x10_DISPLAY));

	disp->fd = open(path, O_RDWR);
	if(disp->fd == -1)
	{
		free(disp);
		return NULL;
	}

	ioctl(disp->fd, IOCTL_DISP_GET_XRES, &width);
	ioctl(disp->fd, IOCTL_DISP_GET_YRES, &height);
	height = height * 2;

    if(height==480)
        disp->norm = 0;
    else if(height==576)
        disp->norm = 1;        

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
	solo6x10_display_zoom(disp, 0, 0, 0);
	solo6x10_display_mux_mode(disp, mode);
	return disp;
}



