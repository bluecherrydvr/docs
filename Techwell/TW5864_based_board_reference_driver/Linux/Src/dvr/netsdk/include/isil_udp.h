#ifndef  _ISIL_UDP_H
#define  _ISIL_UDP_H


#ifdef __cplusplus
extern "C" {
#endif

#define UDP_BASE_PORT       10100

#define UDP_MAX_PORT_NUM    128


extern int set_udp_task( void );

extern int set_udp_task2( unsigned int net_addr );

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
