#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>


#include "net_api.h"
#include "io_utils.h"



#define MAX_SEND_BUFF (110592)

#define IP_MAX_LEN (32)

#define BACKLOG (1024)


int tcp_listen(const char* ip,int port ,int block)
{
    int fd ,ret = 0;
    int sock_buf_size;
    struct sockaddr_in addr;
    
    

    if(strlen(ip) >= IP_MAX_LEN) {
        return -1;
    }

    
    fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( fd < 0 ) {
		printf("listen failed, errno %d, %s.\n", errno, strerror( errno ) );
		return -1;
	}

    if( block == 0 ) {
        if(set_noblock(fd) < 0){
            printf("Set listen fd no block err.\n");
            return  -1;
        }
    }

    sock_buf_size = MAX_SEND_BUFF;
    
    int flags = 1;
    if(setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flags, sizeof( flags )) < 0 ) {
        printf("failed to set setsock to reuseaddr.\n" );
        return -1;
    }

    ret = setsockopt( fd, SOL_SOCKET, SO_SNDBUF,
               (char *)&sock_buf_size, sizeof(sock_buf_size));

    if(ret < 0) {
        printf("failed to set setsock to send buff.\n" );
        return -1;
    }


    if( setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flags, sizeof(flags) ) < 0 ) {
        printf("failed to set socket to nodelay.\n" );
        return -1;
    }

    memset(&addr, 0, sizeof( struct sockaddr_in ));
    addr.sin_family = AF_INET;
    addr.sin_port = htons( port );
    addr.sin_addr.s_addr = INADDR_ANY;

    if(*ip != '\0') {
        if(inet_aton(ip,&addr.sin_addr) == 0){
            printf("failed to convert %s to inet_addr.\n",ip);
            return -1;
        }
    }
    

   
    if(bind( fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in ) ) < 0 ) {
        printf("bind failed, errno %d, %s.\n", errno, strerror( errno ));
        return -1;
    }
    

    
    if(listen( fd ,BACKLOG) < 0){
        printf("listen failed, errno %d, %s.\n", errno, strerror( errno ));
        return -1;
    }
    
    
    return fd;
}









