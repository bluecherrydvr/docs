#ifndef  ISIL_DEV_FILE_MGT_H
#define ISIL_DEV_FILE_MGT_H

#include "isil_dev_file_st.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ISIL_VIDEO_IN_SUBNAME "video_in"
#define ISIL_AUDIO_IN_SUBNAME "audio_in"
#define ISIL_VIDEO_OUT_SUBNAME "video_out"
#define ISIL_AUDIO_OUT_SUBNAME "audio_out"
#define ISIL_ENCODE_SUBNAME   "encode"
#define ISIL_DECODE_SUBNAME   "decode"
#define ISIL_CONTROL_SUBNAME   "control"

#define DIVSYMBOL '-'
#define ISIL_DEV_NAME   "dev"

extern int search_sys_video_node(file_list * parentNode);

extern int init_sys_dev_inf_tree( void );


#ifdef __cplusplus
}
#endif //__cplusplus

#endif
