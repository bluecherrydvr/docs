#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <solo6x10/vin.h>

int main(int argc, char **argv)
{
	char device[1024] = "/dev/solo6110_vin0";
	int fd;

	unsigned int threshold = 0xffffffff;
	unsigned int mosaic = 0;

	unsigned int channel = 0;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int width = 44;
	unsigned int height = 30;

	int opt;
	int opt_index;
	struct option long_opt[] = {
		{"device", 1, 0, 'd'},
		{"channel", 1, 0, 'c'},
		{"threshold", 1, 0, 't'},
		{"motion", 0, 0, 'm'},
		{"x", 1, 0, 'x'},
		{"y", 1, 0, 'y'},
		{"width", 1, 0, 'w'},
		{"height", 1, 0, 'h'},
		{0, 0, 0, 0}
	};

	int i;

	while((opt = getopt_long(argc, argv, "d:c:t:mx:y:w:h:", long_opt, &opt_index)) != -1)
	{
		switch(opt)
		{
			case 'd':
				strcpy(device, optarg);
				break;
			case 'c':
				channel = strtoul(optarg, NULL, 0);
				break;
			case 't':
				threshold = strtoul(optarg, NULL, 0);
				break;
			case 'm':
				mosaic = 1;
				break;
			case 'x':
				x = strtoul(optarg, NULL, 0);
				break;
			case 'y':
				y = strtoul(optarg, NULL, 0);
				break;
			case 'w':
				width = strtoul(optarg, NULL, 0);
				break;
			case 'h':
				height = strtoul(optarg, NULL, 0);
				break;
		}
	}

	if((threshold != 0xffffffff) && mosaic)
		return -1;

	fd = open(device, O_RDWR);
	if(fd == -1)
		return -1;

	if(threshold != 0xffffffff)
	{
		struct SOLO6x10_VIN_MOTION_THRESHOLD ctrl;
		unsigned short *threshold_table;

		threshold_table = malloc(ctrl.width * ctrl.height * 2);

		for(i=0; i<(ctrl.width * ctrl.height); i++)
			threshold_table[i] = threshold;

		ctrl.channel = channel;
		ctrl.x = x;
		ctrl.y = y;
		ctrl.width = width;
		ctrl.height = height;
		ctrl.ptr = (unsigned long)threshold_table;

		return ioctl(fd, IOCTL_VIN_MOTION_THRESHOLD, &ctrl);
	}

	if(mosaic)
	{
		struct SOLO6x10_VIN_MOSAIC ctrl;

		ctrl.channel = channel;
		ctrl.sx = x;
		ctrl.sy = y;
		ctrl.ex = x+width;
		ctrl.ey = y+height;

		return ioctl(fd, IOCTL_VIN_MOSAIC, &ctrl);
	}

	return -1;
}
