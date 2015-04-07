#include<stdio.h>
#include<fcntl.h>
#include<errno.h>
#include<assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <signal.h>
#include <netinet/tcp.h>


#include "io_utils.h"

#ifndef MAXBACKLOG
#define MAXBACKLOG 1024
#endif








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

void inetNtoa(struct in_addr * addr, char * ip, int size)
{
    const unsigned char *p = ( const unsigned char *) addr;
	snprintf(ip, size, "%i.%i.%i.%i", p[0], p[1], p[2], p[3]);
}




ssize_t r_read(int fd, void *buf, size_t size) {

   ssize_t retval;

   while (((retval = read(fd, buf, size)) == -1) && ((errno == EINTR) || (errno = EAGAIN))) ;
   return retval;
}


ssize_t r_write(int fd, void *buf, size_t size) {
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





static int u_ignore_sigpipe() {
   struct sigaction act;

   if (sigaction(SIGPIPE, (struct sigaction *)NULL, &act) == -1)
      return -1;
   if (act.sa_handler == SIG_DFL) {
      act.sa_handler = SIG_IGN;
      if (sigaction(SIGPIPE, &act, (struct sigaction *)NULL) == -1)
         return -1;
   }   
   return 0;
}


int u_open(u_port_t port) {
   int error;  
   struct sockaddr_in server;
   int sock;
   int i_true = 1;

   if ((u_ignore_sigpipe() == -1) ||
        ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1))
      return -1; 

   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&i_true,
                  sizeof(i_true)) == -1) {
      error = errno;
      while ((close(sock) == -1) && (errno == EINTR)); 
      errno = error;
      return -1;
   }
 
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   server.sin_port = htons((short)port);
   if ((bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1) ||
        (listen(sock, MAXBACKLOG) == -1)) {
      error = errno;
      while ((close(sock) == -1) && (errno == EINTR)); 
      errno = error;
      return -1;
   }
   return sock;
}


int u_open2(u_port_t port) {
   int error;  
   struct sockaddr_in server;
   int sock;
   int sock_buf_size;
   int i_true = 1;
   int flags = 1;

   if ((u_ignore_sigpipe() == -1) ||
        ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1))
      return -1; 

   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&i_true,
                  sizeof(i_true)) == -1) {
      error = errno;
      while ((close(sock) == -1) && (errno == EINTR)); 
      errno = error;
      return -1;
   }

    sock_buf_size = MAX_SEND_BUFF_SIZE;
    if(setsockopt( sock, 
                   SOL_SOCKET, 
                   SO_SNDBUF,
                   (char *)&sock_buf_size, 
                   sizeof(sock_buf_size)) < 0){
        while ((close(sock) == -1) && (errno == EINTR));
        return -1;
    }

    if( setsockopt( sock, 
                    IPPROTO_TCP, 
                    TCP_NODELAY, 
                    (char*)&flags, 
                    sizeof(flags) ) < 0 ) {

        while ((close(sock) == -1) && (errno == EINTR));
        return -1;
			
	}

    

 
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   server.sin_port = htons((short)port);
   if ((bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1) ||
        (listen(sock, MAXBACKLOG) == -1)) {
      error = errno;
      while ((close(sock) == -1) && (errno == EINTR)); 
      errno = error;
      return -1;
   }
   return sock;
}



int u_accept(int fd, char *hostn, int hostnsize) {
   int len = sizeof(struct sockaddr);
   struct sockaddr_in netclient;
   int retval;

   while (((retval =
           accept(fd, (struct sockaddr *)(&netclient), (socklen_t *)&len)) == -1) &&
          (errno == EINTR))
      ;  
   if ((retval == -1) || (hostn == NULL) || (hostnsize <= 0))
      return retval;
//   addr2name(netclient.sin_addr, hostn, hostnsize);
   return retval;
}


int r_close(int fd) 
{
   int retval;

   while (retval = close(fd), retval == -1 && errno == EINTR) ;
   return retval;
}



int u_openudp(u_port_t port ,struct sockaddr_in *server) 
{
   int error;
   int one = 1;
   
   int sock;

   if( !server ) {
       return -1;
   }

   if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      return -1;

   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
      error = errno;
      r_close(sock);
      errno = error;
      return -1;
   }
       
   if (port > 0) {
      server->sin_family = AF_INET;
      server->sin_addr.s_addr = htonl(INADDR_ANY);
      server->sin_port = htons((short)port);
 
      if (bind(sock, (struct sockaddr *)server, sizeof(*server)) == -1) {
         error = errno;
         r_close(sock);
         errno = error;
         return -1;
      }
   }
   return sock;
}



int u_openudp2(unsigned int net_addr,u_port_t port ,struct sockaddr_in *server) 
{
   int error;
   int one = 1;

   char* ip;

   struct in_addr in;
   
   int sock;

   if( !server ) {
       return -1;
   }

   in.s_addr = net_addr;
   ip = inet_ntoa(in);
   if(!ip) {
       return -1;
   }

   if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      return -1;


   if(inet_pton(AF_INET,ip, &server->sin_addr) <= 0){
        fprintf(stderr,"[%s] is not a valid IP address\n", ip);
        r_close(sock);
        return -1;
   }

   server->sin_addr.s_addr = inet_addr(ip);

   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
      error = errno;
      r_close(sock);
      errno = error;
      return -1;
   }
       
   if (port > 0) {
      server->sin_family = AF_INET;
      server->sin_addr.s_addr = htonl(INADDR_ANY);
      server->sin_port = htons((short)port);
 
      if (bind(sock, (struct sockaddr *)server, sizeof(*server)) == -1) {
         error = errno;
         r_close(sock);
         errno = error;
         return -1;
      }
   }
   return sock;
}



int isil_writeiovall(int fd, struct iovec *iov_ptr, int nvec,
                    void* net_addr)
{
	int  rv = 0;
    struct msghdr msg;
	
//	struct iovec* vec = iov_ptr;
	if( !net_addr )
	{
	    msg.msg_name = NULL;
        msg.msg_namelen = 0;
	}
	else
	{
        msg.msg_name = (void*)(net_addr);
        msg.msg_namelen = sizeof(struct sockaddr_in);
	}
    msg.msg_control = NULL ;
    msg.msg_controllen = 0 ;
    msg.msg_flags = 0 ;
	msg.msg_iov = &iov_ptr[0];
	msg.msg_iovlen = nvec ;
	while( (rv = sendmsg(fd, &msg, 0) < 0) \
		   && ((errno == EINTR) ||(errno == EAGAIN)));
	if(rv < 0) {
		perror( "Send to net" );
		return -1;
	}

	/* We should get here only after we write out everything */
	return 0;
}


int u_recvfrom(int fd, void *buf, size_t nbytes, struct sockaddr_in *ubufp) {

   int len;
   
   int retval;
   
   if( !ubufp || ! buf) {
       return -1;
   }

   len = sizeof (struct sockaddr_in);
   
   while (((retval = recvfrom(fd, (char *)buf, nbytes, 0, (struct sockaddr*)ubufp, (socklen_t *)(&len))) == -1) &&
           (errno == EINTR)) ;  
   return retval;
}


















