#ifndef __SOLO6010_OSG_H
#define __SOLO6010_OSG_H

#define IOCTL_OSG_MAGIC			'o'

#define IOCTL_OSG_ON			_IO(IOCTL_OSG_MAGIC, 0)
#define IOCTL_OSG_OFF			_IO(IOCTL_OSG_MAGIC, 1)

#define IOCTL_OSG_MAXNR			2

#endif
