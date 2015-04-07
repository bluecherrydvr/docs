#ifndef  _ISIL_PB_PROCESS_H
#define _ISIL_PB_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#define TC_NETLINK_GENERIC      17

extern int tc_create_netlink_connect(void);

extern int create_dec_netlink_ev( void );


#ifdef __cplusplus
}
#endif

#endif
