#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <curses.h>
#include <string.h>
#define _GNU_SOURCE
#include <getopt.h>

#include <solo6x10/enc.h>

#define NR_CHANNEL	16

#define NR_COL		8
#define NR_ROW		16
#define COL_WIDTH	7

#define COL_SCALE	0
#define COL_QP		1
#define COL_INTVL	2
#define COL_GOP		3
#define COL_EN_E	4
#define COL_QP_E	5
#define COL_INTVL_E	6
#define COL_GOP_E	7

static char device_path[256] = "/dev/solo6110_enc0";

static void scale_inc(unsigned int *scale)
{
	switch(*scale)
	{
		case 0:
			*scale = 5;
			break;
		case 5:
			*scale = 2;
			break;
		case 2:
			*scale = 1;
			break;
		case 1:
			*scale = 9;
			break;
	}
}

static void scale_dec(unsigned int *scale)
{
	switch(*scale)
	{
		case 9:
			*scale = 1;
			break;
		case 1:
			*scale = 2;
			break;
		case 2:
			*scale = 5;
			break;
		case 5:
			*scale = 0;
			break;
	}
}

static void scale2str(unsigned int scale, char *str)
{
	switch(scale)
	{
		case 9:
			strncpy(str, "704x480", COL_WIDTH+1);
			break;
		case 1:
			strncpy(str, "704x240", COL_WIDTH+1);
			break;
		case 2:
			strncpy(str, "352x240", COL_WIDTH+1);
			break;
		case 5:
			strncpy(str, "176x112", COL_WIDTH+1);
			break;
		case 0:
			strncpy(str, "  off  ", COL_WIDTH+1);
			break;
	}
}

static void qp_inc(unsigned int *qp)
{
	if(*qp < 31)
		(*qp)++;
}

static void qp_dec(unsigned int *qp)
{
	if(*qp > 1)
		(*qp)--;
}

static void qp2str(unsigned int qp, char *str)
{
	snprintf(str, COL_WIDTH+1, "   %02d  ", qp);
}

static void intvl_inc(unsigned int *intvl)
{
	if(*intvl < 511)
		(*intvl)++;
}

static void intvl_dec(unsigned int *intvl)
{
	if(*intvl > 0)
		(*intvl)--;
}

static void intvl2str(unsigned int intvl, char *str)
{
	snprintf(str, COL_WIDTH+1, "  %03d  ", intvl);
}

static void gop_inc(unsigned int *gop)
{
	if(*gop < 255)
		(*gop)++;
}

static void gop_dec(unsigned int *gop)
{
	if(*gop > 1)
		(*gop)--;
}

static void en_e2str(unsigned int en_e, char *str)
{
	snprintf(str, COL_WIDTH+1, "  %s  ", en_e ? " on" : "off");
}

static void en_e_inc(unsigned int *en_e)
{
	*en_e = 1;
}

static void en_e_dec(unsigned int *en_e)
{
	*en_e = 0;
}

static void gop2str(unsigned int gop, char *str)
{
	snprintf(str, COL_WIDTH+1, "  %03d  ", gop);
}

struct TBL_COL
{
	char *name;
	void (*inc)(unsigned int *value);
	void (*dec)(unsigned int *value);
	void (*row2str)(unsigned int value, char *str);
	unsigned int rows[NR_ROW];
};

struct ENC_TBL
{
	int fd;
	WINDOW *win;
	unsigned int sel_col;
	unsigned int sel_row;
	struct TBL_COL col[NR_COL];
};

void enc_tbl_refresh(struct ENC_TBL *enc_tbl)
{
	char str[COL_WIDTH+1];
	unsigned int col;
	unsigned int row;

	wattroff(enc_tbl->win, A_STANDOUT);
	for(col=0; col<NR_COL; col++)
	{
		mvwprintw(enc_tbl->win, 1, (col*7)+8, "%s", enc_tbl->col[col].name);
		for(row=0; row<16; row++)
		{
			enc_tbl->col[col].row2str(enc_tbl->col[col].rows[row], str);
			mvwprintw(enc_tbl->win, row+2, ((col+1)*COL_WIDTH)+1, "%s", str);
		}
	}

	wattron(enc_tbl->win, A_STANDOUT);
	enc_tbl->col[enc_tbl->sel_col].row2str(enc_tbl->col[enc_tbl->sel_col].rows[enc_tbl->sel_row], str);
	mvwprintw(enc_tbl->win, enc_tbl->sel_row+2, ((enc_tbl->sel_col+1)*COL_WIDTH)+1, "%s", str);

	wrefresh(enc_tbl->win);
}

void enc_tbl_get(struct ENC_TBL *enc_tbl)
{
	int channel;
	struct CAP_SCALE scale;
	struct MP4E_GOPSIZE gop;
	struct MP4E_QP qp;
	struct CAP_INTERVAL intvl;

	for(channel=0; channel<NR_CHANNEL; channel++)
	{
		scale.channel = channel;
		gop.channel = channel;
		qp.channel = channel;
		intvl.channel = channel;

		ioctl(enc_tbl->fd, IOCTL_CAP_GET_INTERVAL, &intvl);
		ioctl(enc_tbl->fd, IOCTL_MP4E_GET_QP, &qp);
		ioctl(enc_tbl->fd, IOCTL_MP4E_GET_GOPSIZE, &gop);
		ioctl(enc_tbl->fd, IOCTL_CAP_GET_SCALE, &scale);

		enc_tbl->col[COL_INTVL].rows[channel] = intvl.value;
		enc_tbl->col[COL_QP].rows[channel] = qp.value;
		enc_tbl->col[COL_GOP].rows[channel] = gop.value;
		enc_tbl->col[COL_SCALE].rows[channel] = scale.value;

		scale.channel = channel+16;
		gop.channel = channel+16;
		qp.channel = channel+16;
		intvl.channel = channel+16;

		ioctl(enc_tbl->fd, IOCTL_CAP_GET_INTERVAL, &intvl);
		ioctl(enc_tbl->fd, IOCTL_MP4E_GET_QP, &qp);
		ioctl(enc_tbl->fd, IOCTL_MP4E_GET_GOPSIZE, &gop);
		ioctl(enc_tbl->fd, IOCTL_CAP_GET_SCALE, &scale);

		enc_tbl->col[COL_INTVL_E].rows[channel] = intvl.value;
		enc_tbl->col[COL_QP_E].rows[channel] = qp.value;
		enc_tbl->col[COL_GOP_E].rows[channel] = gop.value;
		enc_tbl->col[COL_EN_E].rows[channel] = scale.value;
	}
}

void enc_tbl_set(struct ENC_TBL *enc_tbl)
{
	int channel;
	struct CAP_SCALE scale;
	struct MP4E_GOPSIZE gop;
	struct MP4E_QP qp;
	struct CAP_INTERVAL intvl;

	for(channel=0; channel<NR_CHANNEL; channel++)
	{
		scale.channel = channel;
		scale.value = enc_tbl->col[COL_SCALE].rows[channel];
		gop.channel = channel;
		gop.value = enc_tbl->col[COL_GOP].rows[channel];
		qp.channel = channel;
		qp.value = enc_tbl->col[COL_QP].rows[channel];
		intvl.channel = channel;
		intvl.value = enc_tbl->col[COL_INTVL].rows[channel];

		ioctl(enc_tbl->fd, IOCTL_CAP_SET_INTERVAL, &intvl);
		ioctl(enc_tbl->fd, IOCTL_MP4E_SET_QP, &qp);
		ioctl(enc_tbl->fd, IOCTL_MP4E_SET_GOPSIZE, &gop);
		ioctl(enc_tbl->fd, IOCTL_CAP_SET_SCALE, &scale);

		scale.channel = channel+16;
		scale.value = enc_tbl->col[COL_EN_E].rows[channel];
		gop.channel = channel+16;
		gop.value = enc_tbl->col[COL_GOP_E].rows[channel];
		qp.channel = channel+16;
		qp.value = enc_tbl->col[COL_QP_E].rows[channel];
		intvl.channel = channel+16;
		intvl.value = enc_tbl->col[COL_INTVL_E].rows[channel];

		ioctl(enc_tbl->fd, IOCTL_CAP_SET_INTERVAL, &intvl);
		ioctl(enc_tbl->fd, IOCTL_MP4E_SET_QP, &qp);
		ioctl(enc_tbl->fd, IOCTL_MP4E_SET_GOPSIZE, &gop);
		ioctl(enc_tbl->fd, IOCTL_CAP_SET_SCALE, &scale);
	}
}

int enc_tbl_preset(struct ENC_TBL *enc_tbl, int key)
{
	int channel;

	for(channel=0; channel<NR_CHANNEL; channel++)
	{
		switch(key)
		{
			case '1':
				enc_tbl->col[COL_INTVL].rows[channel] = 1;
				enc_tbl->col[COL_QP].rows[channel] = 4;
				enc_tbl->col[COL_GOP].rows[channel] = 30;
				enc_tbl->col[COL_SCALE].rows[channel] = 2;

				enc_tbl->col[COL_INTVL_E].rows[channel] = 1;
				enc_tbl->col[COL_QP_E].rows[channel] = 4;
				enc_tbl->col[COL_GOP_E].rows[channel] = 30;
				enc_tbl->col[COL_EN_E].rows[channel] = 0;
				break;
			case '2':
				enc_tbl->col[COL_INTVL].rows[channel] = 1;
				enc_tbl->col[COL_QP].rows[channel] = 4;
				enc_tbl->col[COL_GOP].rows[channel] = 30;
				enc_tbl->col[COL_SCALE].rows[channel] = (channel<8) ? 1 : 0;

				enc_tbl->col[COL_INTVL_E].rows[channel] = 1;
				enc_tbl->col[COL_QP_E].rows[channel] = 4;
				enc_tbl->col[COL_GOP_E].rows[channel] = 30;
				enc_tbl->col[COL_EN_E].rows[channel] = 0;
				break;
			case '3':
				enc_tbl->col[COL_INTVL].rows[channel] = 0;
				enc_tbl->col[COL_QP].rows[channel] = 4;
				enc_tbl->col[COL_GOP].rows[channel] = 30;
				enc_tbl->col[COL_SCALE].rows[channel] = (channel<4) ? 9 : 0;

				enc_tbl->col[COL_INTVL_E].rows[channel] = 0;
				enc_tbl->col[COL_QP_E].rows[channel] = 4;
				enc_tbl->col[COL_GOP_E].rows[channel] = 30;
				enc_tbl->col[COL_EN_E].rows[channel] = 0;
				break;
			case '0':
				enc_tbl->col[COL_INTVL].rows[channel] = 1;
				enc_tbl->col[COL_QP].rows[channel] = 4;
				enc_tbl->col[COL_GOP].rows[channel] = 30;
				enc_tbl->col[COL_SCALE].rows[channel] = 0;

				enc_tbl->col[COL_INTVL_E].rows[channel] = 1;
				enc_tbl->col[COL_QP_E].rows[channel] = 4;
				enc_tbl->col[COL_GOP_E].rows[channel] = 30;
				enc_tbl->col[COL_EN_E].rows[channel] = 0;
				break;
		}
	}
}

int enc_tbl_init(struct ENC_TBL *enc_tbl)
{
	char str[COL_WIDTH+1];
	int channel;

	enc_tbl->fd = open(device_path, O_RDWR);
	if(enc_tbl->fd == -1)
		return -1;

	enc_tbl->win = newwin((NR_ROW+1)+2, ((NR_COL+1)*COL_WIDTH)+2, 1, 1);
	if(enc_tbl->win == NULL)
	{
		close(enc_tbl->fd);
		return -1;
	}

	enc_tbl->sel_col = 0;
	enc_tbl->sel_row = 0;

	enc_tbl->col[COL_SCALE].name = " SCALE ";
	enc_tbl->col[COL_SCALE].inc = scale_inc;
	enc_tbl->col[COL_SCALE].dec = scale_dec;
	enc_tbl->col[COL_SCALE].row2str = scale2str;

	enc_tbl->col[COL_QP].name = "   QP  ";
	enc_tbl->col[COL_QP].inc = qp_inc;
	enc_tbl->col[COL_QP].dec = qp_dec;
	enc_tbl->col[COL_QP].row2str = qp2str;

	enc_tbl->col[COL_INTVL].name = " INTVL ";
	enc_tbl->col[COL_INTVL].inc = intvl_inc;
	enc_tbl->col[COL_INTVL].dec = intvl_dec;
	enc_tbl->col[COL_INTVL].row2str = intvl2str;

	enc_tbl->col[COL_GOP].name = "  GOP  ";
	enc_tbl->col[COL_GOP].inc = gop_inc;
	enc_tbl->col[COL_GOP].dec = gop_dec;
	enc_tbl->col[COL_GOP].row2str = gop2str;

	enc_tbl->col[COL_EN_E].name = "  EN_E ";
#if 0
	enc_tbl->col[COL_EN_E].inc = en_e_inc;
	enc_tbl->col[COL_EN_E].dec = en_e_dec;
	enc_tbl->col[COL_EN_E].row2str = en_e2str;
#else
	enc_tbl->col[COL_EN_E].inc = scale_inc;
	enc_tbl->col[COL_EN_E].dec = scale_dec;
	enc_tbl->col[COL_EN_E].row2str = scale2str;
#endif

	enc_tbl->col[COL_QP_E].name = "  QP_E ";
	enc_tbl->col[COL_QP_E].inc = qp_inc;
	enc_tbl->col[COL_QP_E].dec = qp_dec;
	enc_tbl->col[COL_QP_E].row2str = qp2str;

	enc_tbl->col[COL_INTVL_E].name = "INTVL_E";
	enc_tbl->col[COL_INTVL_E].inc = intvl_inc;
	enc_tbl->col[COL_INTVL_E].dec = intvl_dec;
	enc_tbl->col[COL_INTVL_E].row2str = intvl2str;

	enc_tbl->col[COL_GOP_E].name = " GOP_E ";
	enc_tbl->col[COL_GOP_E].inc = gop_inc;
	enc_tbl->col[COL_GOP_E].dec = gop_dec;
	enc_tbl->col[COL_GOP_E].row2str = gop2str;

	box(enc_tbl->win, '|', '-');

	mvwprintw(enc_tbl->win, 1, 1, "CHANNEL");
	for(channel=0; channel<16; channel++)
	{
		mvwprintw(enc_tbl->win, channel+2, 4, "%02d", channel);
	}

	enc_tbl_get(enc_tbl);

	enc_tbl_refresh(enc_tbl);

	return 0;
}

void enc_tbl_exit(struct ENC_TBL *enc_tbl)
{
	delwin(enc_tbl->win);
}

int enc_table_move(struct ENC_TBL *enc_tbl, unsigned int col, unsigned int row)
{
	char str[COL_WIDTH+1];

	wattroff(enc_tbl->win, A_STANDOUT);
	enc_tbl->col[enc_tbl->sel_col].row2str(enc_tbl->col[enc_tbl->sel_col].rows[enc_tbl->sel_row], str);
	mvwprintw(enc_tbl->win, enc_tbl->sel_row+2, ((enc_tbl->sel_col+1)*COL_WIDTH)+1, "%s", str);

	enc_tbl->sel_col = col;
	enc_tbl->sel_row = row;

	wattron(enc_tbl->win, A_STANDOUT);
	enc_tbl->col[enc_tbl->sel_col].row2str(enc_tbl->col[enc_tbl->sel_col].rows[enc_tbl->sel_row], str);
	mvwprintw(enc_tbl->win, enc_tbl->sel_row+2, ((enc_tbl->sel_col+1)*COL_WIDTH)+1, "%s", str);

	wrefresh(enc_tbl->win);
}

int main(int argc, char **argv)
{
	struct ENC_TBL enc_tbl;
	int key;

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

	initscr();
	refresh();
	crmode();
	keypad(stdscr, TRUE);
	noecho();

	mvprintw(20, 1, "1. 352x240 16channel");
	mvprintw(21, 1, "2. 704x240  8channel");
	mvprintw(22, 1, "3. 704x480  4channel");
	mvprintw(23, 1, "0. Off");

	mvprintw(20, 40, "S. Set config to device");
	mvprintw(21, 40, "G. Get config from device");
	mvprintw(22, 40, "Q. Quit");

	if(enc_tbl_init(&enc_tbl) != 0)
	{
		endwin();
		return -1;
	}

	while((key = getch()) != 'q')
	{
		switch(key)
		{
			case KEY_UP:
				if(enc_tbl.sel_row > 0)
					enc_table_move(&enc_tbl, enc_tbl.sel_col, enc_tbl.sel_row-1);
				break;
			case KEY_DOWN:
				if(enc_tbl.sel_row < (NR_ROW-1))
					enc_table_move(&enc_tbl, enc_tbl.sel_col, enc_tbl.sel_row+1);
				break;
			case KEY_LEFT:
				if(enc_tbl.sel_col > 0)
					enc_table_move(&enc_tbl, enc_tbl.sel_col-1, enc_tbl.sel_row);
				break;
			case KEY_RIGHT:
				if(enc_tbl.sel_col < (NR_COL-1))
					enc_table_move(&enc_tbl, enc_tbl.sel_col+1, enc_tbl.sel_row);
				break;
			case '+':
			case 'e':
				enc_tbl.col[enc_tbl.sel_col].inc(&enc_tbl.col[enc_tbl.sel_col].rows[enc_tbl.sel_row]);
				enc_table_move(&enc_tbl, enc_tbl.sel_col, enc_tbl.sel_row);
				break;
			case '-':
			case 'd':
				enc_tbl.col[enc_tbl.sel_col].dec(&enc_tbl.col[enc_tbl.sel_col].rows[enc_tbl.sel_row]);
				enc_table_move(&enc_tbl, enc_tbl.sel_col, enc_tbl.sel_row);
				break;
			case 'S':
			case 's':
			case 10:	// enter
				enc_tbl_set(&enc_tbl);
				break;
			case 'G':
			case 'g':
				enc_tbl_get(&enc_tbl);
				enc_tbl_refresh(&enc_tbl);
				break;
			case '1':
			case '2':
			case '3':
			case '0':
				enc_tbl_preset(&enc_tbl, key);
				enc_tbl_refresh(&enc_tbl);
				break;
			default:
				//mvprintw(0, 0, "key : %04d", key);
				wrefresh(enc_tbl.win);
		}
	}

	enc_tbl_exit(&enc_tbl);

	endwin();

	return 0;
}
