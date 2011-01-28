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
#include <sys/poll.h>

#include <solo6x10/fb.h>

int main(int argc, char **argv)
{
	char device[1024] = "/dev/fb0";
	int erase = 0;
	int auto_flush = 0;
	unsigned int alpha_rate = 10;
	struct SOLO6x10FB_RECT rect = {
		.x = 0,
		.y = 0,
		.width = 704,
		.height = 480,
	};

	int fd;

	int opt;
	int opt_index;
	struct option long_opt[] = {
		{"device", 1, 0, 'd'},
		{"erase", 0, 0, 'e'},
		{"auto", 0, 0, 'a'},
		{"x", 1, 0, 'x'},
		{"y", 1, 0, 'y'},
		{"width", 1, 0, 'w'},
		{"height", 1, 0, 'h'},
		{"alpha_rate", 1, 0, 'r'},
		{0, 0, 0, 0}
	};

	while((opt = getopt_long(argc, argv, "d:eax:y:w:h:r:", long_opt, &opt_index)) != -1)
	{
		switch(opt)
		{
			case 'd':
				strcpy(device, optarg);
				break;
			case 'e':
				erase = 1;
				break;
			case 'a':
				auto_flush = 1;
				break;
			case 'x':
				rect.x = strtoul(optarg, NULL, 0);
				break;
			case 'y':
				rect.y = strtoul(optarg, NULL, 0);
				break;
			case 'w':
				rect.width = strtoul(optarg, NULL, 0);
				break;
			case 'h':
				rect.height = strtoul(optarg, NULL, 0);
				break;
			case 'r':
				alpha_rate = strtoul(optarg, NULL, 0);
				break;
		}
	}

	fd = open(device, O_RDWR);
	if(fd == -1)
		return -1;

	if(alpha_rate)
		alpha_rate |= ALPHA_FORCE;

	ioctl(fd, IOCTL_FB_ALPHA_RATE, &alpha_rate);

	if(erase)
		ioctl(fd, IOCTL_FB_ERASE, &rect);
	else
		ioctl(fd, IOCTL_FB_FLUSH, &rect);

	if(auto_flush)
	{
		for(;;)
		{
			poll(NULL, 0, 10);
			ioctl(fd, IOCTL_FB_FLUSH, &rect);
		}
	}

	return 0;
}
