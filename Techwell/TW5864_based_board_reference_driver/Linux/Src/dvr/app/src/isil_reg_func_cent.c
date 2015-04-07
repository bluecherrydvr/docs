

#include "isil_reg_func_cent.h"
#include "isil_preview_session.h"
#include "isil_decode_api.h"
#include "isil_msg_reg_cb.h"
#include "isil_handle_win_msg.h"
#include "net_parse_reg.h"
#include "isil_file_record_session.h"
#include "isil_udp.h"
#include "isil_alarm_session.h"

void init_glb_net_parse_reg_t( void )
{
	net_parse_reg_t *net_parse_reg_ptr = get_glb_net_parse_reg_t( );
	net_parse_reg_ptr->net_ctl_cb   = ISIL_ParseNetMessages;
    net_parse_reg_ptr->net_data_cb  = ISIL_ParseDataMsg;
	net_parse_reg_ptr->net_local_cb = ISIL_ParseLocalNetMessages;
	net_parse_reg_ptr->net_async_notify_cb = async_net_read_cb;
}


void init_glb_file_dec_reg_t(void)
{
	MSG_FILE_DEC_REG_T *file_dec_reg_ptr = get_glb_file_dec_reg_ptr();
	file_dec_reg_ptr->fm_bind_file_windows_cb = isil_union_bind_windows_cb;
	file_dec_reg_ptr->fm_dec_set_diaplay_port_mode = isil_dec_set_diaplay_port_mode_cb;
	file_dec_reg_ptr->fm_remote_close = isil_fm_remote_close_cb;
	file_dec_reg_ptr->fm_remote_get_fileinfo = isil_remote_get_fileinfo_cb;
	file_dec_reg_ptr->fm_remote_get_frame = isil_fm_remote_get_frame_cb;
	file_dec_reg_ptr->fm_remote_pause = isil_fm_remote_pause_cb;
	file_dec_reg_ptr->fm_remote_play = isil_fm_remote_play_cb;
	file_dec_reg_ptr->fm_remote_stop = isil_fm_remote_stop_cb;
	file_dec_reg_ptr->fm_set_av_sync = isil_set_av_sync_cb;
	file_dec_reg_ptr->fm_remote_open_file  = isil_remote_open_file_cb;
	file_dec_reg_ptr->fm_remote_search     = isil_remote_search_file; 
	file_dec_reg_ptr->fm_remote_close   = isil_union_remote_close_cb;
	file_dec_reg_ptr->set_play_mod_param = isil_set_play_mod_param_cb;
}	


void init_glb_preview_dec_reg_t( void )
{
	MSG_PREVIEW_DEC_REG_T *preview_dec_reg_ptr =get_glb_preview_dec_reg_t();
	preview_dec_reg_ptr->dec_preview_start = isil_union_bind_windows_cb;
	preview_dec_reg_ptr->dec_preview_stop  = preview_stop_cb;
}


void init_net_msg_reg( void )
{
	net_msg_reg_func *net_msg_reg_ptr = get_glb_net_msg_reg_t();
	net_msg_reg_ptr->set_udp_task_cb =  set_udp_task;
	net_msg_reg_ptr->set_udp_task_cb2 =  set_udp_task2;
	net_msg_reg_ptr->start_record_file_cb = start_record_file;
	net_msg_reg_ptr->stop_record_file_cb =  stop_file_record;
}


void glb_reg_func_init( void )
{
	init_glb_net_parse_reg_t();
	init_glb_file_dec_reg_t();
	init_glb_preview_dec_reg_t();
	init_net_msg_reg();
}
