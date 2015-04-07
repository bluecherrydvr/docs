#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "isil_fm_utils.h"


#ifdef BIG_BYTES_ORDER


int swap_two_bytes(unsigned short int *small)
{
	unsigned char tmp[2];

	//fprintf(stderr, "before convert small is : %x\n ", *small);
	tmp[0] = (*small) >>8;
	tmp[1] = (*small)&0xff;

	*small = (tmp[1]<<8) | (tmp[0]);

	//fprintf(stderr, "after convert small is : %x\n ", *small);
	return 0;
}


int swap_three_bytes(unsigned int *small)
{
	unsigned char tmp[3];

	//fprintf(stderr, "before convert small is : %x\n ", *small);
	tmp[0] = (*small) >>16;
	tmp[1] = (*small) >>8;
	tmp[2] = (*small)&0xff;

	*small =(tmp[2]<<16)|(tmp[1]<<8)|tmp[0];

	//fprintf(stderr, "after convert small is : %x\n ", *small);
	return 0;
}

int swap_four_bytes(unsigned int *small)
{
	unsigned char tmp[4];

	//fprintf(stderr, "before convert small is : %x\n ", *small);
	tmp[0] = (*small) >>24;
	tmp[1] = (*small) >>16;
	tmp[2] = (*small) >>8;
	tmp[3] = (*small)&0xff;

	*small =(tmp[3]<<24)|(tmp[2]<<16)|(tmp[1]<<8)|tmp[0];

	//fprintf(stderr, "after convert small is : %x\n ", *small);
	return 0;
}
#else


int swap_two_bytes(unsigned short int *small)
{
	small = small;
	return 0;
}


int swap_three_bytes(unsigned int *small)
{
	small = small;
	return 0;
}

int swap_four_bytes(unsigned int *small)
{
	small = small;
	return 0;
}
#endif


int ISIL_GetSysTime(S_REC_DATE_TIME* date_time)
{
	struct tm tbuffer;
	time_t current_time;

    memset(&tbuffer, 0x00, sizeof(struct tm));
    memset(&current_time, 0x00, sizeof(time_t));

	current_time = time(NULL);
	if(localtime_r(&current_time, &tbuffer) == NULL)
	{
		fprintf(stderr, "localtime_r fail\n");
		return -1;
	}

	date_time->Date.Year		= 	(1900+tbuffer.tm_year);
	//fprintf(stderr, "year:%d\n",date_time->Date.Year);
	date_time->Date.Month		= 	(1+tbuffer.tm_mon);
	//fprintf(stderr, "Month:%d\n",date_time->Date.Month);
	date_time->Date.day		= 	 tbuffer.tm_mday;
	//fprintf(stderr, "day:%d\n",date_time->Date.day);

	date_time->Time.Hour		= 	tbuffer.tm_hour;
	//fprintf(stderr, "Hour:%d\n",date_time->Time.Hour);
	date_time->Time.Minute		= 	tbuffer.tm_min;
	//fprintf(stderr, "Minute:%d\n",date_time->Time.Minute);
	date_time->Time.Second	= 	tbuffer.tm_sec;
	//fprintf(stderr, "Second:%d\n",date_time->Time.Second);

	return 0;
}

ssize_t read_r(int fd, void *buf, size_t size)
{

   ssize_t retval;

   while (((retval = read(fd, buf, size)) == -1) && ((errno == EINTR) || (errno = EAGAIN))) ;
   return retval;
}


ssize_t write_r(int fd, void *buf, size_t size)
{
   char *bufp;
   size_t bytestowrite;
   ssize_t byteswritten;
   size_t totalbytes;

   for (bufp = (char *)buf, bytestowrite = size, totalbytes = 0;
        bytestowrite > 0;
        bufp += byteswritten, bytestowrite -= byteswritten) {
      byteswritten = write(fd, bufp, bytestowrite);
      if ((byteswritten) == -1 && (errno != EINTR))
         return -1;
      if (byteswritten == -1)
         byteswritten = 0;
      totalbytes += byteswritten;
   }
   return totalbytes;
}


