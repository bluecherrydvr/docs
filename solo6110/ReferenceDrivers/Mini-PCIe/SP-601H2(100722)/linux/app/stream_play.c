#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>

#include "solo_disp.h"

static char device_path[256] = "/dev/solo6110_dec0";
static char display_path[256] = "/dev/solo6110_disp0";
static int align = 64;
static int speed = 1;
static unsigned int dec_channel = 1;
static unsigned int nr_dec_channel = 0;
static int channel_switch[16];
static int in_fd = -1;

unsigned int stream_size(unsigned char *index, unsigned int align)
{
	unsigned int size;

	size = index[2] & 0x0f;
	size *= 256;
	size += index[1];
	size *= 256;
	size += index[0];

	return (align ? ((((size) + align)/align)*align) : size);
}

int main(int argc, char **argv)
{
	unsigned char *buf;
	unsigned int skip;
	unsigned int wait_i_frame = 0xffff;

	int dec_fd;
	SOLO_DISP *disp;
	unsigned int display_mode;

	int channel;

	int opt;
	int opt_index;
	struct option long_opt[] =
	{
		{"device", 1, 0, 'd'},
		{"display", 1, 0, 'D'},
		{"align", 1, 0, 'a'},
		{"speed", 1, 0, 's'},
		{"channel", 1, 0, 'c'},
		{"mode", 1, 0, 'm'},
		{0, 0, 0, 0}
	};

	while((opt = getopt_long(argc, argv, "d:D:a:s:c:", long_opt, &opt_index)) != -1)
	{
		switch(opt)
		{
			case 'd':
				strncpy(device_path, optarg, 256);
				break;

			case 'D':
				strncpy(display_path, optarg, 256);
				break;

			case 'a':
				align = strtoul(optarg, NULL, 0);
				break;

			case 's':
				speed = strtoul(optarg, NULL, 0);
				break;

			case 'c':
				dec_channel = strtoul(optarg, NULL, 0);
				break;

			default:
				printf("getopt_long return %d\n", opt);
		}
	}

	if(argc != (optind+1))
		return -1;

	in_fd = open(argv[optind], O_RDONLY);
	if(in_fd == -1)
		return -1;

	dec_fd = open(device_path, O_RDWR);
	if(dec_fd == -1)
		return -1;

	buf = malloc(1<<20);
	if(buf == NULL)
		return -1;

	buf = (unsigned char *)(((unsigned long)buf + 0x0f) & (~0xf));

	skip = speed;

	if(!dec_channel)
		dec_channel = 1;

	for(channel=0; channel<16; channel++)
	{
		if(!(dec_channel & (1<<channel)))
		{
			channel_switch[channel] = -1;
		}
		else
		{
			channel_switch[channel] = nr_dec_channel;
			nr_dec_channel++;
		}
	}

	if(nr_dec_channel == 1)
		display_mode = MUX_MODE_1CH;
	else if(nr_dec_channel <= 4)
		display_mode = MUX_MODE_4CH;
	else if(nr_dec_channel <= 9)
		display_mode = MUX_MODE_9CH;
	else
		display_mode = MUX_MODE_16CH;

	disp = solo_disp_open(display_path, display_mode);
	if(!disp)
		return -1;

	for(channel=0; channel<16; channel++)
	{
		if(channel_switch[channel] != -1)
			solo_disp_channel_mode(disp, channel_switch[channel], DISP_MODE_PB);
	}

	solo_disp_mux_mode(disp, display_mode);

	while(1)
	{
		unsigned char dec_index[64];
		unsigned char *dec_buf;
		int read_size;
		unsigned int mpeg4_size;
		off_t current_offset;
		unsigned char tmp;
		unsigned int header_size = 64;

		current_offset = lseek(in_fd, 0, SEEK_CUR);

		read(in_fd, &tmp, 1);
		read_size = ((int)tmp)<<0;
		read(in_fd, &tmp, 1);
		read_size |= ((int)tmp)<<8;
		read(in_fd, &tmp, 1);
		read_size |= ((int)tmp)<<16;
		read(in_fd, &tmp, 1);
		read_size |= ((int)tmp)<<24;

		if((read_size > (1<<20)) || (read_size <= header_size))
		{
			printf("read_size : %d\n", read_size);
			return -1;
		}

		if(read_size != read(in_fd, buf, read_size))
		{
			printf("read_size : %d\n", read_size);
			return -1;
		}

		if((buf[7]>>1) & 3)		// motion mode
		{
			header_size += 256;
		}

		mpeg4_size = read_size - header_size;

		channel = buf[36];

		if(channel > 16)
		{
			printf("channel : %d\n", channel);
			return -1;
		}

		if(!(dec_channel & (1<<channel)))
		{
			continue;
		}

		if(wait_i_frame & (1<<channel))
		{
			if(buf[2] & 0xc0)
				continue;
			else
				wait_i_frame &= ~(1<<channel);
		}

		channel = channel_switch[channel];

		memset(dec_index, 0, 64);
		dec_index[0] = buf[0];
		dec_index[1] = buf[1];
		dec_index[2] = buf[2] & 0xcf;

		skip--;
		if(skip)
		{
			dec_index[2] |= 1<<5;
		}
		else
		{
			skip = speed;
		}

		dec_index[3] = (buf[3] & 0xf0) | channel;
		dec_index[4] = buf[4];
		dec_index[5] = buf[5];
		dec_index[7] = (buf[7] & 0xf0) | channel;
#if 0
		if(speed == 1)
		{
			// time
			dec_index[20] = buf[20];
			dec_index[21] = buf[21];
			dec_index[22] = buf[22];
			dec_index[23] = buf[23];
			dec_index[24] = buf[24];
			dec_index[25] = buf[25];
			dec_index[26] = buf[26];
			dec_index[27] = buf[27];
		}
#endif
		dec_buf = buf;

		memcpy(dec_buf, dec_index, 64);

		write(dec_fd, dec_buf, header_size + mpeg4_size);
	}

	return 0;
}
