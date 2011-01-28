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
#include <malloc.h>

#include <solo6x10/enc.h>

static char device_path[256] = "/dev/solo6110_enc0";
static unsigned int jpeg_mode = 0;

#define NR_CHANNEL						16

int main(int argc, char **argv)
{
	int enc_fd;
	int out_fd;
	int maxsize=1<<20;
	unsigned char *buf;
	unsigned int channel_mask = 0xffff;
	unsigned int wait_i_frame = 0xffffffff;
	int opt;
	int opt_index;
	struct option long_opt[] =
	{
		{"device", 1, 0, 'd'},
		{"jpeg", 0, 0, 'j'},
		{"channel", 1, 0, 'c'},
		{0, 0, 0, 0}
	};

	while((opt = getopt_long(argc, argv, "d:jc:", long_opt, &opt_index)) != -1)
	{
		switch(opt)
		{
			case 'd':
				strncpy(device_path, optarg, 256);
				break;

			case 'j':
				jpeg_mode = 1;
				break;

			case 'c':
				channel_mask = strtoul(optarg, NULL, 0);
				break;

			default:
				printf("getopt_long return %d\n", opt);
		}
	}

	if(argc != (optind+1))
		return -1;

	out_fd = open(argv[optind], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(out_fd == -1)
	{
		printf("open %s - %m\n", argv[optind]);
		exit(EXIT_FAILURE);
	}

	enc_fd = open(device_path, O_RDWR);
	if(enc_fd == -1)
	{
		printf("open %s - %m\n", device_path);
		exit(EXIT_FAILURE);
	}

	if(jpeg_mode)
	{
		if(ioctl(enc_fd, IOCTL_SET_JPEG_MODE) != 0)
		{
			printf("IOCTL_SET_JPEG_MODE error");
			return -1;
		}
		ioctl(enc_fd, IOCTL_CLEAR_JPEG_QUEUE);
	}
	else
	{
		ioctl(enc_fd, IOCTL_CLEAR_MPEG_QUEUE);
	}

	buf = valloc(maxsize);
	if(!buf)
	{
		printf("valloc fail\n");
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		int read_size;
		int ret_val;
		unsigned int channel;
		unsigned int vop_type;
		unsigned char tmp;

		read_size = read(enc_fd, buf, maxsize);
		if(read_size == 0)
			continue;
		if(read_size < 0)
			return -1;

		channel = buf[36];
		vop_type = (buf[2]>>6) & 0x03;
		if(wait_i_frame & (1<<channel))
		{
			if(!vop_type)
				wait_i_frame &= ~(1<<channel);
			else
				continue;
		}

		if(!(channel_mask & (1<<buf[36])))
			continue;

		if((buf[7]>>1) & 3)		// motion mode
		{
			buf[7] &= ~(3<<1);

			read_size -= 256;

			tmp = (unsigned char)(0xff & (read_size>>0));
			write(out_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (read_size>>8));
			write(out_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (read_size>>16));
			write(out_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (read_size>>24));
			write(out_fd, &tmp, 1);

			ret_val = write(out_fd, buf, 64);
			if(ret_val != 64)
			{
				printf("write(%d, %d)\n", ret_val, 64);
				return 0;
			}

			read_size -= 64;

			ret_val = write(out_fd, buf + 64 + 256, read_size);
			if(ret_val != read_size)
			{
				printf("write(%d, %d)\n", ret_val, read_size);
				return 0;
			}
		}
		else
		{
			tmp = (unsigned char)(0xff & (read_size>>0));
			write(out_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (read_size>>8));
			write(out_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (read_size>>16));
			write(out_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (read_size>>24));
			write(out_fd, &tmp, 1);

			ret_val = write(out_fd, buf, read_size);
			if(ret_val != read_size)
			{
				printf("write(%d, %d)\n", ret_val, read_size);
				return 0;
			}
		}
	}

	return 0;
}

