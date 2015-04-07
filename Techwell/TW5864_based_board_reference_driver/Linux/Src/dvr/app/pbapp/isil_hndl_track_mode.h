#ifndef  _ISIL_HNDL_TRACK_MODE_H
#define  _ISIL_HNDL_TRACK_MODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_interface.h"
#include "isil_pb_mgt.h"

extern int dec_normal_read_file(single_pb_mgt * s_pb_mgt,void *arg);

extern int dec_frame_forward_read_file(single_pb_mgt * s_pb_mgt,void *arg);
extern int dev_drv_need_frame_handle(single_pb_mgt * s_pb_mgt,struct  query_info *query);


#ifdef __cplusplus
}
#endif

#endif
