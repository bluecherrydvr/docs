/*用于系统相关的升级*/
#ifndef __ISIL_UPDTAE_H__
#define __ISIL_UPDTAE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "isil_type.h"
/*系统升级接口，根据类型不同，分别处理*/
void ISIL_SysUpdate(S32 UpdateType);

#ifdef __cplusplus
}
#endif

#endif

