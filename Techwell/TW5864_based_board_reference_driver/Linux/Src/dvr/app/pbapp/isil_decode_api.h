/*
 * isil_decode_api.h
 *
 *  Created on: 2011-6-30
 *      Author: zoucx
 */

#ifndef ISIL_DECODE_API_H_
#define ISIL_DECODE_API_H_



#ifdef __cplusplus
extern "C" {
#endif

#include "isil_messagestruct.h"
#include "isil_fm_play.h"
#include "isil_msg_mgt.h"
#include "isil_pb_mgt.h"
#include "isil_media_config.h"
#include "isil_interface.h"


typedef struct _DEC_MSG_T{
    int msg_type;
    int msg_len;
	
}DEC_MSG_T;


extern int isil_set_dec_msg_by_single_pb_mgt(single_pb_mgt *s_pb_mgt, int msg_type,int arg_len,void *arg);

extern int isil_dec_msg_set_thr_param(int fd, int msg_type,int arg_len,void *arg);


extern int isil_bind_display_window(unsigned int nChipId ,
                             unsigned int nChanId ,
                             ISIL_SUB_WINDOWS *cfg);

extern int isil_set_sub_window(unsigned int nChipId ,
                        unsigned int nChanId ,
                        ISIL_SUB_WINDOWS *cfg);

extern int isil_set_video_standard(unsigned int chipId,
                            struct ISIL_display_video_standard* cfg);

extern int isil_blind_privew_window(unsigned int chipId, ISIL_SUB_WINDOWS_BIND* WinCfg);

extern int isil_set_display_port_mode(unsigned int chipId ,
                               ISIL_DIAPLAY_PORT_MODE* cfg);

extern int isil_blind_loopback_window(unsigned int chipId, ISIL_SUB_WINDOWS_BIND* WinCfg);
extern int isil_del_display_window(unsigned int chipId,unsigned int ch);
extern int isil_remote_search_file( void * search_condition, void *buff);

extern int isil_remote_open_file_cb( void* stOpen_p, void* fileinfo);

extern int isil_remote_get_fileinfo_cb(void *opt ,void *stRFileInfo);

extern int isil_set_play_mod_param_cb( int fd, void * plyMod );

extern int isil_fm_remote_get_frame_cb(int fd,void *get_frame_opt);

extern int isil_fm_remote_stop_cb(int fd,void *ststop);

extern int isil_fm_remote_pause_cb(int fd,void *stpause);

extern int isil_fm_remote_play_cb(int fd,void *stplay);

extern int isil_fm_remote_close_cb(int fd,void *stClose);

extern int isil_set_av_sync_cb(int fd ,void *avsync);

extern int isil_dec_set_diaplay_port_mode_cb(unsigned int chipId ,void* portmode);

extern int isil_bind_file_windows_cb(unsigned int chip_id,void *win);

extern int set_decoder_avsync(unsigned int chipId,unsigned short vchan, unsigned short achan);



extern int parse_dec_msg(single_pb_mgt * s_pb_mgt ,struct ISIL_MSG_BLK *msg_node);

extern S_ISIL_SINGLE_CHAN_MGT * isil_open_dec_chn(int chipid,int chn ,enum ECHANFUNCPRO type,int enable);

extern int isil_pb_netlink_req_frames_cb(struct  query_info *pb_netlink_query);

extern int create_video_dec_chan(single_pb_mgt * s_pb_mgt);

extern int create_audio_dec_chan(single_pb_mgt * s_pb_mgt);

extern int close_video_dec_chan(single_pb_mgt * s_pb_mgt);

extern int close_audio_dec_chan(single_pb_mgt * s_pb_mgt);


#ifdef __cplusplus
}
#endif



#endif /* ISIL_DECODE_API_H_ */
