#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <pthread.h>
#include <curses.h>

#include "solo_disp.h"

#define PRINT_SIZE		1

#define VIDEO_HSIZE		704
#define VIDEO_VSIZE		480
#define BUFFER_SIZE		(512<<10) //(VIDEO_HSIZE * VIDEO_VSIZE * 2)
#define NR_BUFFER		128

#define DEFAULT_ENC_DEVICE	"/dev/solo6110_enc0"
#define DEFAULT_DEC_DEVICE	"/dev/solo6110_dec0"
#define DEFAULT_DISP_DEVICE	"/dev/solo6110_disp0"

static int enc_fd = -1;
static int dec_fd = -1;
static int rec_fd = -1;
static SOLO_DISP *disp = NULL;

static volatile unsigned int sb_wr = 0;
static volatile unsigned int sb_rd_dec = 0;
static volatile unsigned int sb_rd_rec = 0;
static pthread_mutex_t sb_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sb_cond = PTHREAD_COND_INITIALIZER;

static unsigned int disp_channel = 1;
static unsigned int dec_channel = 1;
static unsigned int wait_i_frame = 0xffffffff;
static int quit = 0;

static unsigned int playback_pip = 0;
static unsigned int pip_x = 64;
static unsigned int pip_y = 46;
static unsigned int zoom = 0;
static unsigned int zoom_x = 176;
static unsigned int zoom_y = 120;

static unsigned int panorama = 0;
static unsigned int panorama_win[2] = {0, 0};

//static unsigned int pause = 0;

static unsigned int data_size[16];
static unsigned int fps[16];

#define dbg_msg(str, args...)		do{mvprintw(20, 1, "%s :" str, __FUNCTION__, ##args); refresh();}while(0)
#define dbg_msg2(str, args...)		do{mvprintw(21, 1, "%s :" str, __FUNCTION__, ##args); refresh();}while(0)
#define dbg_msg3(str, args...)		do{mvprintw(22, 1, "%s :" str, __FUNCTION__, ##args); refresh();}while(0)

struct stream_buffer
{
	pthread_mutex_t mutex;
	void *__data;
	unsigned char *data;
	int size;
};

void swap_u16(unsigned short *data, unsigned int size)
{
	int i;
	unsigned short tmp;

	for(i=0; i<(size/sizeof(unsigned short)); i+=2)
	{
		tmp = data[i];
		data[i] = data[i+1];
		data[i+1] = tmp;
	}
}

void swap_u32(unsigned long *data, unsigned int size)
{
	int i;
	unsigned short tmp;

	for(i=0; i<(size/sizeof(unsigned long)); i+=2)
	{
		tmp = data[i];
		data[i] = data[i+1];
		data[i+1] = tmp;
	}
}

int stream_buffer_init(struct stream_buffer *sb, int size)
{
	if(pthread_mutex_init(&sb->mutex, NULL) != 0)
	{
		fprintf(stderr, "pthread_mutex_init error\n");
		return -1;
	}

	sb->__data = malloc(size + 16);
	if(sb->__data == NULL)
	{
		pthread_mutex_destroy(&sb->mutex);
		fprintf(stderr, "malloc error\n");
		return -1;
	}
	sb->data = (unsigned char *)(((unsigned long)sb->__data + 15) & (~15));

	return 0;
}

void stream_buffer_destroy(struct stream_buffer *sb)
{
	pthread_mutex_destroy(&sb->mutex);
	free(sb->__data);
}

static int get_option(int argc, char **argv)
{
	int opt;
	int opt_index;
	struct option long_opt[] =
	{
		{"enc", 1, 0, 'e'},
		{"dec", 1, 0, 'd'},
		{"disp", 1, 0, 'D'},
		{"channel", 1, 0, 'c'},
		{"output", 1, 0, 'o'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};

	while((opt = getopt_long(argc, argv, "e:d:D:c:o:h", long_opt, &opt_index)) != -1)
	{
		switch(opt)
		{
			case 'e':
				enc_fd = open(optarg, O_RDWR);
				if(enc_fd == -1)
				{
					fprintf(stderr, "enc(%s) - %m\n", optarg);
					return -1;
				}
				break;

			case 'd':
				dec_fd = open(optarg, O_RDWR);
				if(dec_fd == -1)
				{
					fprintf(stderr, "dec(%s) - %m\n", optarg);
					return -1;
				}
				break;

			case 'D':
				disp = solo_disp_open(optarg, MUX_MODE_1CH);
				if(!disp)
					return -1;

				break;

			case 'c':
				dec_channel = strtoul(optarg, NULL, 0);
				break;

			case 'o':
				rec_fd = open(optarg, O_CREAT | O_TRUNC | O_RDWR, 00644);
				if(rec_fd == -1)
				{
					fprintf(stderr, "out(%s) - %m\n", optarg);
					return -1;
				}
				break;

			case 'h':
				break;

			default:
				fprintf(stderr, "unknown option : %c\n", opt);
		}
	}

	return 0;
}

static int open_devices(void)
{
	if(enc_fd == -1)
	{
		enc_fd = open(DEFAULT_ENC_DEVICE, O_RDWR);
		if(enc_fd == -1)
		{
			fprintf(stderr, "enc(%s) - %m\n", DEFAULT_ENC_DEVICE);
			return -1;
		}
	}

	if(dec_fd == -1)
	{
		dec_fd = open(DEFAULT_DEC_DEVICE, O_RDWR);
		if(enc_fd == -1)
		{
			fprintf(stderr, "dec(%s) - %m\n", DEFAULT_DEC_DEVICE);
			return -1;
		}
	}

	if(!disp)
	{
		disp = solo_disp_open(DEFAULT_DISP_DEVICE, MUX_MODE_1CH);
		if(!disp)
			return -1;
	}

	return 0;
}

void print_size(unsigned char *index, unsigned int size)
{
	unsigned int channel;
	unsigned int sec;
	static unsigned int old_sec = 0;

	channel = index[36];

	if(channel < 16)
	{
		data_size[channel] += size;
		fps[channel]++;
		sec = (((unsigned int)index[20])<<0)
			| (((unsigned int)index[21])<<8)
			| (((unsigned int)index[22])<<16)
			| (((unsigned int)index[23])<<24);
		if(sec != old_sec)
		{
			unsigned int total_size = 0;

			old_sec = sec;
			for(channel=0; channel<16; channel++)
			{
				mvprintw(channel, 3, "%02d : %04dKBytes %02dfps", channel, data_size[channel]>>10, fps[channel]);
				total_size += data_size[channel];
				data_size[channel] = 0;
				fps[channel] = 0;
			}

			mvprintw(16, 2, "all : %04dKBytes", total_size>>10);

			dbg_msg2("%3d (%3d, %3d)", (NR_BUFFER + sb_wr - sb_rd_dec) % NR_BUFFER, sb_wr, sb_rd_dec);

			refresh();
		}
	}
}

void *func_enc(void *arg)
{
	struct stream_buffer *sb;

	sb = (struct stream_buffer *)arg;

	while(!quit)
	{
		unsigned char *buf;
		int read_size;
		unsigned char channel;

		pthread_mutex_lock(&sb_mutex);
		while((((NR_BUFFER + sb_wr - sb_rd_dec) % NR_BUFFER) == (NR_BUFFER - 1)) ||
			((rec_fd != -1) && (((NR_BUFFER + sb_wr - sb_rd_rec) % NR_BUFFER) == (NR_BUFFER - 1))))
		{
			pthread_cond_wait(&sb_cond, &sb_mutex);
		}
		pthread_mutex_unlock(&sb_mutex);

		buf = sb[sb_wr].data;
		read_size = read(enc_fd, buf, BUFFER_SIZE);
		if(read_size < 0)
		{
			dbg_msg("read_size : %d\n", read_size);
			continue;
		}

		print_size(buf, read_size);

		channel = buf[36];

		if(!(disp_channel & (1<<channel)))
			continue;

		if(wait_i_frame & (1<<channel))
		{
			if(buf[2] & (3<<6))	// check i-frame
				continue;
			else
				wait_i_frame &= ~(1<<channel);
		}

		sb[sb_wr].size = read_size;

		pthread_mutex_lock(&sb_mutex);
		sb_wr = (sb_wr + 1) % NR_BUFFER;
		pthread_cond_signal(&sb_cond);
		pthread_mutex_unlock(&sb_mutex);
	}

	return NULL;
}

int solo6110_decode(int fd, unsigned char *data, int size, unsigned int channel_mask)
{
	unsigned char dec_index[64];
	int channel;
	int window_id;

	channel = data[36];
	window_id = channel;

	if(channel > 16)
	{
		dbg_msg("channel - %d", channel);
		return -1;
	}

	if(playback_pip)
	{
		if(channel != 0)
			return 0;
		channel = 1;
		window_id = 1;
	}
	else if(panorama)
	{
		if(channel > 1)
			return 0;
		window_id = (channel*8) + panorama_win[channel];
		panorama_win[channel] = (panorama_win[channel] + 1) % 8;
	}
	else if(!(channel_mask & (1<<channel)))
		return 0;

	memset(dec_index, 0, 64);
	dec_index[0] = data[0];
	dec_index[1] = data[1];
	dec_index[2] = data[2] & 0xcf;
	dec_index[3] = (data[3] & 0xf0) | channel;
	dec_index[4] = data[4];
	dec_index[5] = data[5];
	//if(((data[4]>>4) == 9) && ((data[18]&0xf0) || (data[19]&0x3f)))
	dec_index[6] = 1;	// interpolation
	if(zoom)
		dec_index[6] |= 1<<2;
	dec_index[7] = (data[7] & 0xf0) | window_id;
/*
	dec_index[20] = data[20];
	dec_index[21] = data[21];
	dec_index[22] = data[22];
	dec_index[23] = data[23];
	dec_index[24] = data[24];
	dec_index[25] = data[25];
	dec_index[26] = data[26];
	dec_index[27] = data[27];
*/
	if((data[7]>>1) & 3)		// motion mode
	{
		memcpy(data+256, dec_index, 64);
		return write(fd, data+256, size-256);
	}

	memcpy(data, dec_index, 64);

	//swap_u32((unsigned short *)(data+64), size-64);

	return write(fd, data, size);
}

void *func_dec(void *arg)
{
	struct stream_buffer *sb;

	sb = (struct stream_buffer *)arg;

	while(!quit)
	{
		pthread_mutex_lock(&sb_mutex);
		while((sb_wr == sb_rd_dec) ||
			((rec_fd != -1) && (sb_rd_rec == sb_rd_dec)))
		{
			pthread_cond_wait(&sb_cond, &sb_mutex);
		}
		pthread_mutex_unlock(&sb_mutex);

		solo6110_decode(dec_fd, sb[sb_rd_dec].data, sb[sb_rd_dec].size, dec_channel & disp_channel);

		pthread_mutex_lock(&sb_mutex);
		sb_rd_dec = (sb_rd_dec + 1) % NR_BUFFER;
		pthread_cond_signal(&sb_cond);
		pthread_mutex_unlock(&sb_mutex);
	}

	return NULL;
}

void *func_rec(void *arg)
{
	struct stream_buffer *sb;
	unsigned char tmp;
	unsigned int rec_size = 0;
	unsigned int rec_cnt=0;

	sb = (struct stream_buffer *)arg;

	while(!quit)
	{
		pthread_mutex_lock(&sb_mutex);
		while(sb_wr == sb_rd_rec)
		{
			pthread_cond_wait(&sb_cond, &sb_mutex);
		}
		pthread_mutex_unlock(&sb_mutex);

		if(sb[sb_rd_rec].size > 0)
		{
			tmp = (unsigned char)(0xff & (sb[sb_rd_rec].size>>0));
			write(rec_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (sb[sb_rd_rec].size>>8));
			write(rec_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (sb[sb_rd_rec].size>>16));
			write(rec_fd, &tmp, 1);
			tmp = (unsigned char)(0xff & (sb[sb_rd_rec].size>>24));
			write(rec_fd, &tmp, 1);

			write(rec_fd, sb[sb_rd_rec].data, sb[sb_rd_rec].size);
			rec_size += 4 + sb[sb_rd_rec].size;

			rec_cnt++;
			if(rec_cnt >= 100)
			{
				rec_cnt = 0;
				dbg_msg3("%08x (wr:%3d, rd_dec:%3d, rd_rec:%3d)", rec_size, sb_wr, sb_rd_dec, sb_rd_rec);
			}
		}

		pthread_mutex_lock(&sb_mutex);
		sb_rd_rec = (sb_rd_rec + 1) % NR_BUFFER;
		pthread_cond_signal(&sb_cond);
		pthread_mutex_unlock(&sb_mutex);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	struct stream_buffer sb[NR_BUFFER+1];

	pthread_t thread_enc;
	pthread_t thread_dec;
	pthread_t thread_rec;

	int retval;
	int i;
	int key;

	retval = get_option(argc, argv);
	if(retval)
		return -1;

	retval = open_devices();
	if(retval)
		return -1;

	for(i=0; i<16; i++)
	{
		if(dec_channel & (1<<i))
			solo_disp_channel_mode(disp, i, DISP_MODE_PB);

		data_size[i] = 0;
		fps[i] = 0;
	}

	for(i=0; i<(NR_BUFFER+1); i++)
	{
		if(stream_buffer_init(&sb[i], BUFFER_SIZE) != 0)
			return -1;
	}

	retval = pthread_create(&thread_enc, NULL, func_enc, (void *)&sb);
	if(retval)
	{
		fprintf(stderr, "thread_enc - %m\n");
	}

	retval = pthread_create(&thread_dec, NULL, func_dec, (void *)&sb);
	if(retval)
	{
		fprintf(stderr, "thread_dec - %m\n");
	}

	if(rec_fd != -1)
	{
		retval = pthread_create(&thread_rec, NULL, func_rec, (void *)&sb);
		if(retval)
		{
			fprintf(stderr, "thread_rec - %m\n");
		}
	}
////
	initscr();
	refresh();
	crmode();
	keypad(stdscr, TRUE);
	noecho();

	while((key = getch()) != 'q')
	{
		switch(key)
		{
			case KEY_UP:
				if(zoom)
				{
					zoom_y = (zoom_y < 8) ? 0 : (zoom_y - 8);
					solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
					break;
				}
				else if(playback_pip)
				{
					pip_y = (pip_y < 8) ? 0 : (pip_y - 8);
					solo_disp_pip(disp, 1, pip_x, pip_y);
					dbg_msg3("pip(%3d,%3d)", pip_x, pip_y);
					break;
				}
				else if(panorama)
					break;

				//solo_disp_mux_mode(disp, (disp->mux_mode + 1) % 6);
				solo_disp_mux_mode(disp, (disp->mux_mode + 1) % 7);
				switch(disp->mux_mode)
				{
					case MUX_MODE_1CH:	disp_channel = 0x0001;	break;
					case MUX_MODE_4CH:	disp_channel = 0x000f;	break;
					case MUX_MODE_6CH:	disp_channel = 0x003f;	break;
					case MUX_MODE_8CH:	disp_channel = 0x00ff;	break;
					case MUX_MODE_9CH:	disp_channel = 0x01ff;	break;
					case MUX_MODE_16CH:	disp_channel = 0xffff;	break;
					case MUX_MODE_2CH:	disp_channel = 0x0003;	break;
				}
				wait_i_frame |= ~disp_channel;
				break;

			case KEY_DOWN:
				if(zoom)
				{
					zoom_y = (zoom_y > (240-8)) ? 240 : (zoom_y + 8);
					solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
					break;
				}
				else if(playback_pip)
				{
					pip_y = (pip_y > (480-160-8)) ? (480-160) : (pip_y + 8);
					solo_disp_pip(disp, 1, pip_x, pip_y);
					dbg_msg3("pip(%3d,%3d)", pip_x, pip_y);
					break;
				}
				else if(panorama)
					break;

				//solo_disp_mux_mode(disp, (disp->mux_mode + 5) % 6);
				solo_disp_mux_mode(disp, (disp->mux_mode + 6) % 7);
				switch(disp->mux_mode)
				{
					case MUX_MODE_1CH:	disp_channel = 0x0001;	break;
					case MUX_MODE_4CH:	disp_channel = 0x000f;	break;
					case MUX_MODE_6CH:	disp_channel = 0x003f;	break;
					case MUX_MODE_8CH:	disp_channel = 0x00ff;	break;
					case MUX_MODE_9CH:	disp_channel = 0x01ff;	break;
					case MUX_MODE_16CH:	disp_channel = 0xffff;	break;
					case MUX_MODE_2CH:	disp_channel = 0x0003;	break;
				}
				wait_i_frame |= ~disp_channel;
				break;

			case KEY_LEFT:
				if(zoom)
				{
					zoom_x = (zoom_x < 8) ? 0 : (zoom_x - 8);
					solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
				}
				else if(playback_pip)
				{
					pip_x = (pip_x < 8) ? 0 : (pip_x - 8);
					solo_disp_pip(disp, 1, pip_x, pip_y);
					dbg_msg3("pip(%3d,%3d)", pip_x, pip_y);
				}
				break;

			case KEY_RIGHT:
				if(zoom)
				{
					zoom_x = (zoom_x > (352-8)) ? 352 : (zoom_x + 8);
					solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
				}
				else if(playback_pip)
				{
					pip_x = (pip_x > (704-232-8)) ? (704-232) : (pip_x + 8);
					solo_disp_pip(disp, 1, pip_x, pip_y);
					dbg_msg3("pip(%3d,%3d)", pip_x, pip_y);
				}
				break;

			case 'p':
				if(disp->mux_mode == MUX_MODE_1CH)
				{
					playback_pip = playback_pip ? 0 : 1;
					if(playback_pip)
					{
						solo_disp_channel_mode(disp, 0, DISP_MODE_NORMAL);
						solo_disp_channel_mode(disp, 1, DISP_MODE_PB);
						//solo_disp_mux_mode(disp, MUX_MODE_1CH);
						solo_disp_pip(disp, 1, pip_x, pip_y);
					}
					else
					{
						solo_disp_pip(disp, -1, pip_x, pip_y);
						solo_disp_channel_mode(disp, 0, DISP_MODE_PB);
						if(!(disp_channel & (1<<1)))
							solo_disp_channel_mode(disp, 1, DISP_MODE_NORMAL);
					}
				}
				else if(disp->mux_mode == MUX_MODE_16CH)
				{
					panorama = panorama ? 0 : 1;
					if(panorama)
					{
						for(i=0; i<16; i++)
						{
							solo_disp_channel_mode(disp, i, DISP_MODE_PB);
						}
					}
					else
					{
						for(i=0; i<16; i++)
						{
							if(dec_channel & (1<<i))
								solo_disp_channel_mode(disp, i, DISP_MODE_PB);
							else
								solo_disp_channel_mode(disp, i, DISP_MODE_NORMAL);
						}
					}
				}
				break;
			case 'z':
				if(disp->mux_mode == MUX_MODE_1CH)
				{
					zoom = zoom ? 0 : 1;
					solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
				}
				break;
		}
	}

	endwin();
	exit(0);
////
	pthread_join(thread_enc, NULL);
	pthread_join(thread_dec, NULL);
	if(rec_fd != -1)
	{
		pthread_join(thread_rec, NULL);
		close(rec_fd);
	}

	return 0;
}
