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

#define BUFFER_SIZE		48

int main(int argc, char **argv)
{
	int g723_fd;
	int in_fd;
	unsigned char *buf;

	unsigned int channel = 0;
	unsigned int bypass = 0;

	int opt;
	int opt_index;
	struct option long_opt[] =
	{
		{"device", 1, 0, 'd'},
		{0, 0, 0, 0}
	};

	while((opt = getopt_long(argc, argv, "d:", long_opt, &opt_index)) != -1)
	{
		switch(opt)
		{
			case 'd':
				strncpy(device_path, optarg, 256);
				break;

			default:
				printf("getopt_long return %d\n", opt);
		}
	}

	if(!bypass)
	{
		if(argc != (optind+1))
			return -1;

		in_fd = open(argv[optind], O_RDONLY);
		if(in_fd == -1)
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

		read_size = read(in_fd, buf, BUFFER_SIZE);
		if(read_size == 0)
			return 0;
		if(read_size <= 0)
			return -1;

		write(g723_fd, buf, 48);
	}

	return 0;
}

