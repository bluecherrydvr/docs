#ifndef __ISIL_VIDEO_MANAGE_H__
#define __ISIL_VIDEO_MANAGE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "isil_type.h"

#define VIDEO_LOST		1
#define VIDEO_MASK 		2
#define VIDEO_SHELTER	3
#define VIDEO_MOTION	4
#define VIDEO_SIGNAL	5

S32 ISIL_SaveVideoCfg(S32 iChannel, S32 iVideoCfgType, S32 iCfgSize, void * pVideoCfg);
S32 ISIL_GetVideoCfg(S32 iChannel, S32 iVideoCfgType, S32 iCfgSize, void *pVideoCfg);

#ifdef __cplusplus
}
#endif
#endif

