
#include <solo6x10/disp.h>

#define MUX_MODE_1CH	0
#define MUX_MODE_4CH	1
#define MUX_MODE_6CH	2
#define MUX_MODE_8CH	3
#define MUX_MODE_9CH	4
#define MUX_MODE_16CH	5
#define MUX_MODE_2CH	6

typedef struct solo_disp
{
	int fd;
	unsigned int width;
	unsigned int height;
	unsigned int mux_mode;
	int zoom;
	struct DISP_CTRL disp_ctrl[16];
	struct DISP_CTRL *pip_ctrl;
} SOLO_DISP;

int solo_disp_pos2channel(SOLO_DISP *disp, int x, int y);
void solo_disp_channel_mode(SOLO_DISP *disp, unsigned int channel, unsigned int mode);
int solo_disp_mux_mode(SOLO_DISP *disp, unsigned int mode);
int solo_disp_select_channel(SOLO_DISP *disp, int channel);
int solo_disp_zoom(SOLO_DISP *disp, int on, int x, int y);
int solo_disp_motion_trace(SOLO_DISP *disp, int on);

SOLO_DISP *solo_disp_open(char *path, unsigned int mode);
void solo_disp_close(SOLO_DISP *disp);

