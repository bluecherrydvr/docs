#ifndef __SOLO6010_DISP_H
#define __SOLO6010_DISP_H

#define DISP_MODE_NORMAL			(1<<0)
#define DISP_MODE_PIP				(1<<1)
#define DISP_MODE_PB				(1<<2)

struct DISP_CTRL
{
	unsigned int window_id;
	unsigned int channel;
	unsigned int scale;
	unsigned int sx;
	unsigned int sy;
	unsigned int ex;
	unsigned int ey;
	unsigned int mode;
};

struct ZOOM_CTRL
{
	unsigned int on;
	unsigned int x;
	unsigned int y;
};

struct BORDER
{
	unsigned int x[5];
	unsigned int y[5];
	unsigned int line_mask;
	unsigned int fill_mask;
};

struct RECTANGLE
{
	unsigned int id;
	unsigned int sx;
	unsigned int sy;
	unsigned int ex;
	unsigned int ey;
	unsigned int line;
	unsigned int fill;
};

struct CURSOR
{
	unsigned int x;
	unsigned int y;
};

struct CURSOR_MASK
{
	unsigned int mask[20];
};

#define IOCTL_DISP_MAGIC		'd'

#define IOCTL_DISP_ERASE_ON		_IO(IOCTL_DISP_MAGIC, 0)
#define IOCTL_DISP_ERASE_OFF	_IO(IOCTL_DISP_MAGIC, 1)
#define IOCTL_DISP_CHANNEL		_IOW(IOCTL_DISP_MAGIC, 2, struct DISP_CTRL)
#define IOCTL_DISP_ZOOM			_IOW(IOCTL_DISP_MAGIC, 3, struct ZOOM_CTRL)
#define IOCTL_DISP_FREEZE_ON	_IO(IOCTL_DISP_MAGIC, 4)
#define IOCTL_DISP_FREEZE_OFF	_IO(IOCTL_DISP_MAGIC, 5)
#define IOCTL_DISP_EXPANSION	_IOW(IOCTL_DISP_MAGIC, 6, unsigned int)

#define IOCTL_DISP_BORDER		_IOW(IOCTL_DISP_MAGIC, 7, struct BORDER)
#define IOCTL_DISP_RECTANGLE	_IOW(IOCTL_DISP_MAGIC, 8, struct RECTANGLE)

#define IOCTL_DISP_MOTION_TRACE_ON	_IO(IOCTL_DISP_MAGIC, 9)
#define IOCTL_DISP_MOTION_TRACE_OFF	_IO(IOCTL_DISP_MAGIC, 10)

#define IOCTL_DISP_GET_XRES		_IOR(IOCTL_DISP_MAGIC, 11, unsigned int)
#define IOCTL_DISP_GET_YRES		_IOR(IOCTL_DISP_MAGIC, 12, unsigned int)	// field size
#define IOCTL_DISP_SET_NORM	    _IOW(IOCTL_DISP_MAGIC, 13, unsigned int)
#define IOCTL_DISP_MAXNR		14

#endif
