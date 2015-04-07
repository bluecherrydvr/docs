#ifndef   _ISIL_NET_SEND_SESSION_H
#define   _ISIL_NET_SEND_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

extern int send_enc_data_to_client_cb(struct NET_TASK *ev_task ,void *src);

extern int new_send_enc_data_to_client_cb(struct NET_TASK *ev_task ,void *src);



#ifdef __cplusplus
}
#endif

#endif
