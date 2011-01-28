#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <solo6x10/enc.h>

#include "jpeg.h"

int encoder_config(int fd)
{
	struct CAP_SCALE scale;
	struct MP4E_GOPSIZE gop;
	struct MP4E_QP qp;
	struct CAP_INTERVAL intvl;

	scale.channel = 0;
	scale.value = 9;
	gop.channel = 0;
	gop.value = 30;
	qp.channel = 0;
	qp.value = 4;
	intvl.channel = 0;
	intvl.value = 0;

	ioctl(fd, IOCTL_CAP_SET_INTERVAL, &intvl);
	ioctl(fd, IOCTL_MP4E_SET_QP, &qp);
	ioctl(fd, IOCTL_MP4E_SET_GOPSIZE, &gop);
	ioctl(fd, IOCTL_CAP_SET_SCALE, &scale);

	return 0;
}

int main(int argc, char **argv)
{
	int channel=0;
	int fd;
	int out_fd;
	unsigned char *jpeg_header;
	unsigned char *jpeg_data;
	int jpeg_data_size;

	if((fd = open("/dev/solo6110_enc0", O_RDWR)) == -1)
	{
		fprintf(stderr, "open(/dev/solo6110_enc0) - %m");
		return -1;
	}

	if(ioctl(fd, IOCTL_SET_JPEG_MODE) != 0)
	{
		fprintf(stderr, "IOCTL_SET_JPEG_MODE error");
		return -1;
	}

	encoder_config(fd);

	if(argc != 2)
		return -1;

	out_fd = open(argv[1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(out_fd == -1)
	{
		fprintf(stderr, "open %s - %m\n", argv[1]);
		return -1;
	}

	if((jpeg_header = malloc(JPEG_HEADER_SIZE)) == NULL)
	{
		fprintf(stderr, "memory allocation error");
		return -1;
	}

	if((jpeg_data = valloc(1<<20)) == NULL)
	{
		fprintf(stderr, "memory allocation error");
		return -1;
	}

	do{
		if((jpeg_data_size = read(fd, jpeg_data, 1<<20)) < 0)
		{
			fprintf(stderr, "read error");
			return -1;
		}

		if((jpeg_data_size != 0) && ((jpeg_data[3] & 0x1f) != channel))
			jpeg_data_size = 0;
	}while(jpeg_data_size == 0);

	printf("jpeg_data_size : %d\n", jpeg_data_size);

	jpeg_data_size = (unsigned int)jpeg_data[18] & 0x0f;
	jpeg_data_size = (jpeg_data_size<<8) + (unsigned int)jpeg_data[17];
	jpeg_data_size = (jpeg_data_size<<8) + (unsigned int)jpeg_data[16];

	printf("jpeg_data_size : %d\n", jpeg_data_size);

	jpeg_set_header(jpeg_header,
		((unsigned int)jpeg_data[48] - 1) & 0x03,	// qp
		(unsigned int)jpeg_data[5]<<4,				// width
		(unsigned int)jpeg_data[4]<<4);				// height

	if(write(out_fd, jpeg_header, JPEG_HEADER_SIZE) != JPEG_HEADER_SIZE)
		return -1;
	if(write(out_fd, jpeg_data + 64, jpeg_data_size) != jpeg_data_size)
		return -1;

	return 0;
}
