#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>

#include "solo_disp.h"
#include <solo6x10/vin.h>

static char device_path[128] = "/dev/solo6110_disp0";
static int display_mode = MUX_MODE_16CH;

typedef enum{R_NONE=0, R_NUM0, R_NUM1, R_NUM2, R_NUM3, R_NUM4, R_NUM5, R_NUM6, R_NUM7, R_NUM8, R_NUM9,
	R_UP, R_DOWN, R_RIGHT, R_LEFT,
	R_MENU, R_ENTER, R_PLAY, R_STOP, R_PAUSE, R_FF, R_REW, R_REC, R_MODE, R_VMOD,
	R_PIP, R_ZOOM, R_FREEZE, R_MOTION,
	R_QUIT}
	R_KEY;

static int keyboard_control_mode = 0;
static int channel_list[16]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

static struct termios initial_settings;

R_KEY keyboard_read_key(int fd)
{
	unsigned char key;
	int read_size;

	read_size = read(fd, &key, 1);
	if(read_size != 1)		return R_NONE;

	switch(key)
	{
		case '0':	return R_NUM0;
		case '1':	return R_NUM1;
		case '2':	return R_NUM2;
		case '3':	return R_NUM3;
		case '4':	return R_NUM4;
		case '5':	return R_NUM5;
		case '6':	return R_NUM6;
		case '7':	return R_NUM7;
		case '8':	return R_NUM8;
		case '9':	return R_NUM9;

		case 'm':	return R_MOTION;
		case 'f':	return R_FREEZE;
		case 'z':	return R_ZOOM;

		case 'Q':
		case 'q':	return R_QUIT;

		case 0x0a:	return R_ENTER;
		case 0x1b:
			read_size = read(fd, &key, 1);
			if(read_size != 1)		return R_NONE;

			switch(key)
			{
				case 0x5b:
					read_size = read(fd, &key, 1);
					if(read_size != 1)		return R_NONE;

					switch(key)
					{
						case 0x41:	return R_UP;
						case 0x42:	return R_DOWN;
						case 0x43:	return R_RIGHT;
						case 0x44:	return R_LEFT;
						default:
							printf("key : 0x1b 0x5b %02x\n", key);
					}

					return R_NONE;

				case 0xab:
					read_size = read(fd, &key, 1);
					if(read_size != 1)		return R_NONE;

					switch(key)
					{
						case 0xff:	return R_UP;
						default:
							printf("key : 0x1b 0xab %02x\n", key);
					}

					return R_NONE;

				case 0x2b:
					read_size = read(fd, &key, 1);
					if(read_size != 1)		return R_NONE;

					switch(key)
					{
						case 0xff:	return R_DOWN;
						case 0xf4:	return R_LEFT;
						default:
							printf("key : 0x1b 0xab %02x\n", key);
					}

					return R_NONE;

				default:
					printf("key : 0x1b %02x\n", key);
			}

			return R_NONE;

		default:
			printf("key : %02x\n", key);
	}

	return R_NONE;
}

int get_option(int argc, char **argv)
{
	int opt;
	int opt_index;
	struct option long_opt[] =
	{
		{"device", 1, 0, 'd'},
		{"mode", 1, 0, 'm'},
		{"keyboard", 0, 0, 'k'},
		{"help", 0, 0, 'H'},
		{0, 0, 0, 0}
	};

	while((opt = getopt_long(argc, argv, "d:m:k", long_opt, &opt_index)) != -1)
	{
		switch(opt)
		{
			case 'd':
				strncpy(device_path, optarg, 128);
				break;

			case 'm':
				switch(strtoul(optarg, NULL, 0))
				{
					case 1:		display_mode = MUX_MODE_1CH;	break;
					case 4:		display_mode = MUX_MODE_4CH;	break;
					case 6:		display_mode = MUX_MODE_6CH;	break;
					case 8:		display_mode = MUX_MODE_8CH;	break;
					case 9:		display_mode = MUX_MODE_9CH;	break;
					case 16:	display_mode = MUX_MODE_16CH;	break;
					case 2:		display_mode = MUX_MODE_2CH;	break;
				}
				break;

			case 'k':
				keyboard_control_mode = 1;
				break;

			case 'H':
				printf("Usage: %s [OPTION]...\n\n", argv[0]);
				printf("  -d,   --device=DEVICE_PATH                 display device path\n");
				printf("  -m,   --mode=MODE                          MODE:1,4,6,8,9,16\n");
				printf("  -k,   --keyboard                           keyboard control mode\n");
				printf("\n");
				return -1;

			default:
				printf(" getopt_long return %d\n", opt);
				return -1;
		}
	}

	while(optind < argc)
	{
		printf(" non-option : %s\n", argv[optind++]);
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct termios new_settings;
	int keyboard_fd;
	SOLO_DISP *disp;
	int motion_trace = 0;
	int freeze = 0;
	int zoom = 0;
	unsigned int zoom_x = 176;
	unsigned int zoom_y = 120;

	if(get_option(argc, argv) != 0)
		return -1;

	disp = solo_disp_open(device_path, display_mode);
	if(!disp)
		return -1;

	if(!keyboard_control_mode)
		return 0;

	keyboard_fd = fileno(stdin);
	if(keyboard_fd == -1)
	{
		printf("fileno(stdin) - %m");
		return -1;
	}

	tcgetattr(keyboard_fd, &initial_settings);
	new_settings = initial_settings;
	new_settings.c_lflag &= ~(ICANON | ECHO);
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	tcsetattr(keyboard_fd, TCSANOW, &new_settings);

	while(1)
	{
		struct pollfd ufds[1];
		int nfds;

		ufds[0].fd = keyboard_fd;
		ufds[0].events = POLLIN | POLLERR;
		nfds = 1;

		if(poll(ufds, nfds, 10000) < 0)
		{
			printf("poll - %m\n");
			continue;
		}

		if(ufds[0].revents & POLLERR)
		{
			printf("POLLERR!!!\n");
			continue;
		}

		if(ufds[0].revents & POLLIN)
		{
			switch(keyboard_read_key(keyboard_fd))
			{
				case R_UP:
					if(zoom)
					{
						zoom_y = (zoom_y < 8) ? 0 : (zoom_y - 8);
						solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
					}
					else
						solo_disp_mux_mode(disp, (disp->mux_mode + 1) % 6);
					break;
				case R_DOWN:
					if(zoom)
					{
						zoom_y = (zoom_y > (240-8)) ? 240 : (zoom_y + 8);
						solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
					}
					else
						solo_disp_mux_mode(disp, (disp->mux_mode + 5) % 6);
					break;
				case R_LEFT:
					if(zoom)
					{
						zoom_x = (zoom_x < 8) ? 0 : (zoom_x - 8);
						solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
					}
					break;
				case R_RIGHT:
					if(zoom)
					{
						zoom_x = (zoom_x > (352-8)) ? 352 : (zoom_x + 8);
						solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
					}
					break;
				case R_MOTION:
					if(!motion_trace)
						motion_trace = 1;
					else
						motion_trace = 0;
					solo_disp_motion_trace(disp, motion_trace);
					break;

				case R_FREEZE:
					if(!freeze)
						freeze = 1;
					else
						freeze = 0;
					solo_disp_freeze(disp, freeze);
					break;

				case R_ZOOM:
					if(disp->mux_mode==MUX_MODE_1CH)
					{
						zoom = zoom ? 0 : 1;
						solo_disp_zoom(disp, zoom, zoom_x, zoom_y);
					}
					break;

				case R_QUIT:
					tcsetattr(keyboard_fd, TCSANOW, &initial_settings);
					return 0;
				default:
					continue;
			}
		}
	}

	return 0;
}
