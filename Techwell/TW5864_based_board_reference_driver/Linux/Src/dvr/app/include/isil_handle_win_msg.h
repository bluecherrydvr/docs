#ifndef  _ISIL_HANDLE_WIN_MSG_H
#define _ISIL_HANDLE_WIN_MSG_H



#ifdef __cplusplus
extern "C" {
#endif

#include "net_task.h"

extern int ISIL_ParseNetMessages(int readlen, void* buf, struct NET_TASK* task);
extern int ISIL_ParseLocalNetMessages(int readlen, void* buf, struct NET_TASK* task);
extern int ISIL_ParseDataMsg(int readlen, void* buf, struct NET_TASK* task);



#ifdef __cplusplus
}
#endif

#endif


