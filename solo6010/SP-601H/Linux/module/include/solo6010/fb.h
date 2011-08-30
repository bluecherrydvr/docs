#ifndef __SOLO6010FB_H
#define __SOLO6010FB_H

#define IOCTL_FB_MAGIC	'f'

struct SOLO6010FB_RECT
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
};

#define ALPHA_OFF		(0<<30)
#define ALPHA_FORCE		(1<<30)
#define ALPHA_G0		(2<<30)
#define ALPHA_BIT15		(3<<30)
#define ALPHA_MODE(v)	((v)>>30)
#define ALPHA_VALUE(v)	((v) & 0x3f)

struct SOLO6010FB_CURSOR_POS
{
	unsigned int x;
	unsigned int y;
};

#define IOCTL_FB_FLUSH	_IOW(IOCTL_FB_MAGIC, 0, struct SOLO6010FB_RECT)
#define IOCTL_FB_ERASE	_IOW(IOCTL_FB_MAGIC, 1, struct SOLO6010FB_RECT)
#define IOCTL_FB_CURSOR_ON	_IO(IOCTL_FB_MAGIC, 2)
#define IOCTL_FB_CURSOR_OFF	_IO(IOCTL_FB_MAGIC, 3)
#define IOCTL_FB_CURSOR_POS	_IOW(IOCTL_FB_MAGIC, 4, struct SOLO6010FB_CURSOR_POS)
#define IOCTL_FB_ALPHA_RATE	_IOW(IOCTL_FB_MAGIC, 5, unsigned int)

#endif
