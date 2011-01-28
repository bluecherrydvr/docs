#ifndef _SOLO6x10_VIN_H
#define _SOLO6x10_VIN_H

#define IOCTL_VIN_MAGIC 'v'

struct SOLO6x10_VIN_SWITCH
{
	unsigned int src;
	unsigned int dst;
};

struct SOLO6x10_VIN_MOSAIC
{
	unsigned int channel;
	unsigned int sx;
	unsigned int ex;
	unsigned int sy;
	unsigned int ey;
};

struct SOLO6x10_VIN_MOTION_THRESHOLD
{
	unsigned int channel;
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned long ptr;
};

#define IOCTL_VIN_SWITCH			_IOW(IOCTL_VIN_MAGIC, 0, struct SOLO6x10_VIN_SWITCH)
#define IOCTL_VIN_MOSAIC			_IOW(IOCTL_VIN_MAGIC, 1, struct SOLO6x10_VIN_MOSAIC)
#define IOCTL_VIN_MOTION_THRESHOLD	_IOW(IOCTL_VIN_MAGIC, 2, struct SOLO6x10_VIN_MOTION_THRESHOLD)

#define IOCTL_VIN_MAXNR		3

#endif
