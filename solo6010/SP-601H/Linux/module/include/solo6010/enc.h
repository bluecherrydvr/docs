#ifndef __SOLO6010_ENC_H
#define __SOLO6010_ENC_H

struct CAP_SCALE
{
	unsigned int channel;
	unsigned int value;
};

struct MP4E_GOPSIZE
{
	unsigned int channel;
	unsigned int value;
};

struct MP4E_QP
{
	unsigned int channel;
	unsigned int value;
};

struct CAP_INTERVAL
{
	unsigned int channel;
	unsigned int value;
};

struct CAP_OSD
{
	unsigned int channel;
	unsigned int on;
	unsigned long data;
};

struct RAWDATA
{
    unsigned int mode;
    unsigned int data;
};

struct WM
{
    unsigned int polynomial;
    unsigned int initialkey;
    unsigned int strength;
};

#define IOCTL_ENC_MAGIC			'e'

#define IOCTL_CAP_SET_SCALE		_IOW(IOCTL_ENC_MAGIC, 0, struct CAP_SCALE)
#define IOCTL_CAP_SET_INTERVAL	_IOW(IOCTL_ENC_MAGIC, 1, struct CAP_INTERVAL)
#define IOCTL_MP4E_SET_GOPSIZE	_IOW(IOCTL_ENC_MAGIC, 2, struct MP4E_GOPSIZE)
#define IOCTL_MP4E_SET_QP		_IOW(IOCTL_ENC_MAGIC, 3, struct MP4E_QP)

#define IOCTL_CAP_GET_SCALE		_IOWR(IOCTL_ENC_MAGIC, 4, struct CAP_SCALE)
#define IOCTL_CAP_GET_INTERVAL	_IOWR(IOCTL_ENC_MAGIC, 5, struct CAP_INTERVAL)
#define IOCTL_MP4E_GET_GOPSIZE	_IOWR(IOCTL_ENC_MAGIC, 6, struct MP4E_GOPSIZE)
#define IOCTL_MP4E_GET_QP		_IOWR(IOCTL_ENC_MAGIC, 7, struct MP4E_QP)

#define IOCTL_CAP_OSD			_IOW(IOCTL_ENC_MAGIC, 8, struct CAP_OSD)

#define IOCTL_SET_JPEG_MODE		_IO(IOCTL_ENC_MAGIC, 9)

#define IOCTL_GOP_RESET			_IOW(IOCTL_ENC_MAGIC, 10, unsigned int)
#define IOCTL_GET_RAWDATA		_IOR(IOCTL_ENC_MAGIC, 11, struct RAWDATA)
#define IOCTL_ENC_SETWM			_IOW(IOCTL_ENC_MAGIC, 12, struct WM)

#define IOCTL_ENC_MAXNR			13

#endif
