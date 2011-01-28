#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <fcntl.h>

static char device_path[256] = "/dev/solo6110_g723_0";

#define BUFFER_SIZE		(48*16)

int main(int argc, char **argv)
{
	int g723_fd;
	int out_fd;
	unsigned char *buf;

	unsigned int channel = 0;
	unsigned int bypass = 0;

	int opt;
	int opt_index;
	struct option long_opt[] =
	{
		{"device", 1, 0, 'd'},
		{"bypass", 0, 0, 'b'},
		{"channel", 1, 0, 'c'},
		{0, 0, 0, 0}
	};

	while((opt = getopt_long(argc, argv, "d:bc:", long_opt, &opt_index)) != -1)
	{
		switch(opt)
		{
			case 'd':
				strncpy(device_path, optarg, 256);
				break;

			case 'b':
				bypass = 1;
				break;

			case 'c':
				channel = strtoul(optarg, NULL, 0);
				break;

			default:
				printf("getopt_long return %d\n", opt);
		}
	}

	if(!bypass)
	{
		if(argc != (optind+1))
			return -1;

		out_fd = open(argv[optind], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if(out_fd == -1)
		{
			printf(" open %s - %m\n", argv[optind]);
			exit(EXIT_FAILURE);
		}
	}

	g723_fd = open(device_path, O_RDWR);
	if(g723_fd == -1)
		return -1;

	buf = malloc(BUFFER_SIZE);

	while(1)
	{
		int read_size;

		read_size = read(g723_fd, buf, BUFFER_SIZE);
		if(read_size == 0)
			continue;
		if(read_size < 0)
			return -1;

		if(!bypass)
		{
			write(out_fd, buf + (channel*48), 48);
		}
		else
		{
			write(g723_fd, buf + (channel*48), 48);
		}
	}

	return 0;
}

