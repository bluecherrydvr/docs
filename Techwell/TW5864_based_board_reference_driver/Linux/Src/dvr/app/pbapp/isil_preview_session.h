#ifndef  _ISIL_PRIVIEW_SESSION_H
#define  _ISIL_PRIVIEW_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_messagestruct.h"
#include "isil_fm_play.h"

extern  int preview_start_cb(unsigned int chipId, void* WinCfg);

extern int preview_stop_cb(unsigned int chipId, void  *stClose );

extern int isil_union_remote_close_cb(int fd,void *stClose);

extern int isil_union_bind_windows_cb(unsigned int chip_id,void *win);

extern int loopback_bind_win(unsigned int chipId, void* WinCfg);

#ifdef __cplusplus
}
#endif

#endif
