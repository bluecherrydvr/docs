#ifndef   _ISIL_ENC_TO_DEC_LOOPBACK_H
#define   _ISIL_ENC_TO_DEC_LOOPBACK_H

#ifdef __cplusplus
extern "C" {
#endif

extern int loopback_start( unsigned int chipid, unsigned int enc_chan_id,  void* win);

extern int loopback_stop( unsigned int enc_chan_id );



#ifdef __cplusplus
}
#endif

#endif
