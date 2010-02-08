#ifdef _WIN32
#include <conio.h>
#include "tchar.h"
#endif

#ifdef _LINUX

#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#define _T
#define _tscanf scanf
#define Sleep usleep
#include <pthread.h>

int _kbhit(void) {

	static const int STDIN = 0;
	static bool initialized = false;

	if (! initialized) {
		// Use termios to turn off line buffering
		termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = true;
	}

	int bytesWaiting;
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}

#endif
