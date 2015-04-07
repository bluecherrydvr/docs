#ifndef _IO_UTILS_H
#define _IO_UTILS_H




#ifdef __cplusplus
extern "C" {
#endif

#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include <netinet/in.h>

#define MAX_SEND_BUFF_SIZE (110592)

typedef unsigned short u_port_t;

extern int set_noblock(int fd);

extern int set_block(int fd);

extern void inetNtoa(struct in_addr * addr, char * ip, int size);

extern ssize_t r_read(int fd, void *buf, size_t size);

extern ssize_t r_write(int fd, void *buf, size_t size);



extern int u_accept(int fd, char *hostn, int hostnsize);

extern int u_connect(u_port_t port, char *hostn);

extern int u_open(u_port_t port);

extern int u_open2(u_port_t port);

extern int r_close(int fd);

extern int u_openudp(u_port_t port ,struct sockaddr_in *server) ;

extern int u_openudp2(unsigned int net_addr,u_port_t port ,struct sockaddr_in *server); 

extern int isil_writeiovall(int fd, struct iovec *iov_ptr, int nvec,
                    void* net_addr);

extern int u_recvfrom(int fd, void *buf, size_t nbytes, struct sockaddr_in *ubufp);






#ifdef __cplusplus
}
#endif //__cplusplus

#endif
