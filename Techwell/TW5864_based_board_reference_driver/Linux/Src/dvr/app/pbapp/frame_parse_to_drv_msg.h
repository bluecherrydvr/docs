#ifndef  _FRAME_PARSE_TO_DRV_MSG_H
#define  _FRAME_PARSE_TO_DRV_MSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_fm_play.h"
#include "isil_interface.h"

extern int decode_fill_audio_buff_header(unsigned char *buff,
                                         int frameNum,
                                         PB_FRAME_INFO  *pb_frame_inf_t);


extern int decode_fill_h264_buff_header(unsigned char *buff,
                                        int frameNum,
                                        PB_FRAME_INFO  *pb_frame_inf_t,
                                        isil_h264_decode_bit_info_msg_t *bit_inf);




extern int decode_only_fill_h264_bitinfo(unsigned char *buff, unsigned int bit_info);


#ifdef __cplusplus
}
#endif

#endif
