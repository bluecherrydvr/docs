#include<stdio.h>
#include<fcntl.h>
#include<errno.h>
#include<assert.h>
#include <stdlib.h>
#include <unistd.h>





#include "isil_io_utils.h"


int set_noblock(int fd)
{
    int flags;
    flags = fcntl( fd, F_GETFL );
	if( flags < 0 )
        return flags;
    flags |= O_NONBLOCK;
	if( fcntl( fd, F_SETFL, flags ) < 0 ){
        printf("Set fd[%d] no block err.\n",fd);
        return -1;
    }
    return 0 ;
}

int set_block(int fd)
{
    int flags;

	flags = fcntl( fd, F_GETFL );
	if( flags < 0 )
        return flags;

	flags &= ~O_NONBLOCK;
	if( fcntl( fd, F_SETFL, flags ) < 0 ){
          printf("Set fd[%d] no block err.\n",fd);
          return -1;
    }
    return 0;
}






ssize_t r_read(int fd, char *buf, size_t size) {

   ssize_t retval;

   while (((retval = read(fd, buf, size)) == -1) && ((errno == EINTR) || (errno = EAGAIN))) ;
   return retval;
}


ssize_t r_write(int fd, char *buf, size_t size) {
   char *bufp;
   size_t bytestowrite;
   ssize_t byteswritten;
   size_t totalbytes;

   for (bufp = buf, bytestowrite = size, totalbytes = 0;
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




















