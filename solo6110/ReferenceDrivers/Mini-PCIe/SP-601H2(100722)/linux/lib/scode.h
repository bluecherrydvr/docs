#ifndef _SCODE_CAPTURE_H_
#define _SCODE_CAPTURE_H_

#include "../module/include/solo6x10/enc.h"
#include "../module/include/solo6x10/disp.h"
#include "../module/include/solo6x10/dec.h"

#define NR_CHANNEL    4

#define MUX_MODE_1CH    0
#define MUX_MODE_4CH    1
#define MUX_MODE_16CH    5

#define NR_COL        8
#define NR_ROW        4
#define COL_WIDTH    7

#define COL_SCALE    0
#define COL_QP        1
#define COL_INTVL    2
#define COL_GOP        3
#define COL_EN_E    4
#define COL_QP_E    5
#define COL_INTVL_E    6
#define COL_GOP_E    7

typedef struct solo6x10_display
{
	int fd;
	unsigned int width;
	unsigned int height;
	unsigned int mux_mode;
	int zoom;
    int norm;
	struct DISP_CTRL disp_ctrl[16];
	struct DISP_CTRL *pip_ctrl;
} SOLO6x10_DISPLAY;

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
    unsigned int sel_col;
    unsigned int sel_row;
    struct TBL_COL col[NR_COL];
};


#endif
