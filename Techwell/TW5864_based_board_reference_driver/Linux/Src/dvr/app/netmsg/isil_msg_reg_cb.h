#ifndef  _ISIL_MSG_REG_CB_H
#define  _ISIL_MSG_REG_CB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_messagestruct.h"

typedef struct _MSG_FILE_DEC_REG_T{
    
    int  (*fm_remote_open_file)( void * stOpen_p , void* fileinfo);
    int  (*set_play_mod_param)( int fd, void * plyMod );
	int  (*fm_remote_search)( void * search_condition, void *buff);
    int  (*fm_remote_close)(int fd,void *stClose);
    int  (*fm_remote_play)(int fd,void *stplay);
    int  (*fm_remote_pause)(int fd,void *stpause);
    int  (*fm_remote_stop)(int fd,void *ststop);
    int  (*fm_remote_get_frame)(int fd,void *get_frame_opt);
    int  (*fm_remote_get_fileinfo)(void *opt ,void *stRFileInfo);
    int  (*fm_set_av_sync)(int fd ,void *avsync);
    int  (*fm_dec_set_diaplay_port_mode)(unsigned int chipId ,void* portmode);
    int  (*fm_bind_file_windows_cb)(unsigned int chip_id,void *win);

}MSG_FILE_DEC_REG_T;


typedef struct _MSG_PREVIEW_DEC_REG_T{

    int (*dec_preview_start)(unsigned int chipId, void* WinCfg);
    int (*dec_preview_stop)(unsigned int chipId, void  *stClose );

}MSG_PREVIEW_DEC_REG_T;



typedef struct _net_msg_reg_func{
    
    int (*set_udp_task_cb)(void);
	int (*set_udp_task_cb2)(unsigned int net_addr);
    int (*start_record_file_cb)(void *p_fm_start_rec);
	int (*stop_record_file_cb)(int chan);
}net_msg_reg_func;


static inline void net_msg_reg_set_udp_task_cb(net_msg_reg_func *net_msg_reg_t,int (*set_udp_task_cb)(void))
{
	if(net_msg_reg_t) {
		net_msg_reg_t->set_udp_task_cb = set_udp_task_cb;
	}
}

static inline void net_msg_reg_set_start_rec_cb(net_msg_reg_func *net_msg_reg_t,int (*start_record_file_cb)(void *p_fm_start_rec))
{
	if(net_msg_reg_t) {
		net_msg_reg_t->start_record_file_cb = start_record_file_cb;
	}
}


static inline void net_msg_reg_set_stop_rec_cb(net_msg_reg_func *net_msg_reg_t,int (*stop_record_file_cb)(int chan))
{
	if(net_msg_reg_t) {
		net_msg_reg_t->stop_record_file_cb = stop_record_file_cb;
	}
}


extern MSG_FILE_DEC_REG_T *get_glb_file_dec_reg_ptr( void );

extern MSG_PREVIEW_DEC_REG_T *get_glb_preview_dec_reg_t( void );


extern net_msg_reg_func *get_glb_net_msg_reg_t( void );



#ifdef __cplusplus
}
#endif

#endif
