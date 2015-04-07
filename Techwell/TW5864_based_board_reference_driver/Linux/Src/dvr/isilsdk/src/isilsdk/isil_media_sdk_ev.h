#ifndef  _ISIL_MEDIA_SDK_EV_H
#define _ISIL_MEDIA_SDK_EV_H


#ifdef __cplusplus
extern "C" {
#endif

#include "isil_media_config.h"
#include "isil_data_stream.h"
#include "isil_ev_task.h"

#define ALARM_BUFF_SIZE (1024)

extern S_ISIL_SINGLE_CHAN_MGT *  isil_open_enc_chn(int chipid,int chn ,enum ECHANFUNCPRO type,int enable);

extern int reg_date_rec_handle(int chipid,
                        int chn ,
                        enum ECHANFUNCPRO type,
                        int enable);

extern ISIL_AV_PACKET *handle_frame_to_av_packet(frame_t *frame,
                                          struct ISIL_EV_TASK *ev_task);

extern int create_alarm_ev(unsigned int chipid );


#ifdef __cplusplus
}
#endif //__cplusplus

#endif


