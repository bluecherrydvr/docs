/************************************************************************
 *
 * Copyright C 2007 Stretch, Inc. All rights reserved.  Stretch products are
 * protected under numerous U.S. and foreign patents, maskwork rights,
 * copyrights and other intellectual property laws.  
 *
 * This source code and the related tools, software code and documentation, and
 * your use thereof, are subject to and governed by the terms and conditions of
 * the applicable Stretch IDE or SDK and RDK License Agreement
 * (either as agreed by you or found at www.stretchinc.com).  By using these
 * items, you indicate your acceptance of such terms and conditions between you
 * and Stretch, Inc.  In the event that you do not agree with such terms and
 * conditions, you may not use any of these items and must immediately destroy
 * any copies you have made.
 *
 ************************************************************************/

#ifndef STRETCH_SDVR_DEBUG_H
#define STRETCH_SDVR_DEBUG_H

/*************************************************************************
  A wrapper structure to the sdvr structure to hold the channel ID and type
**************************************************************************/
typedef struct {
    sx_uint32 func_id;
    sx_uint8 chan_num;
    sx_uint8 chan_type;
    void *strct;
} sdebug_sdvr_msg_wrapper_t;

typedef struct {
    sx_uint8 status;
    sx_uint8 chan_num;
    sx_uint8 chan_type;
} sdebug_dvr_msg_header_t;

#ifdef STRETCH_SDVR_DEBUG_C
#define DBG_EXTERN

static sx_uint32  m_dbgFlags = 0;            // Currently set debugging flags
static FILE      *m_dbgFileHandle = NULL ;  // The debug file handle
static FILE      *m_dbgFWFileHandle = NULL ;  // The debug file handle to the
										      // Firmware messages sent

#define IS_DBG_ON ((m_dbgFlags & DEBUG_FLAG_DEBUGGING_ON) == DEBUG_FLAG_DEBUGGING_ON)
#define IS_DBG_DISP_OUTPUT ((m_dbgFlags & DEBUG_FLAG_OUTPUT_TO_SCREEN) == DEBUG_FLAG_OUTPUT_TO_SCREEN)
#define IS_DBG_FW_OUTPUT ((m_dbgFlags & DEBUG_FLAG_FW_WRITE_TO_FILE) == DEBUG_FLAG_FW_WRITE_TO_FILE)
#define IS_DBG_WRITE_FILE ((m_dbgFlags & DEBUG_FLAG_WRITE_TO_FILE) == DEBUG_FLAG_WRITE_TO_FILE)
#define IS_DBG_FW_WRITE_FILE ((m_dbgFlags & DEBUG_FLAG_FW_WRITE_TO_FILE) == DEBUG_FLAG_FW_WRITE_TO_FILE)


static void sdbg_output(char *szText, sx_bool bOutput_fw);
static void sdbg_fw_output(char *szText);
#else
#if defined(__cplusplus)
   #define DBG_EXTERN              extern "C"
#else
   #define DBG_EXTERN              extern
#endif

#endif /* STRETCH_SDVR_DEBUG_C */


/*******************************************************************
   Defines to specify each SDVR function name
********************************************************************/
enum sdvr_function_names {
    // Following functions will be in the log file
    // as long as debugging is on.
    start_always_on_func = 2000,
    sdvr_get_sdk_version_func,
    sdvr_get_driver_version_func,
    sdvr_get_firmware_version_func,
    sdvr_dbg_err_msg,
    sdvr_get_firmware_version_ex_func,
    end_always_on_func,

    //sdk functions
    start_sdvr_sdk_func = end_always_on_func,
    sdvr_sdk_init_func = start_sdvr_sdk_func,
    sdvr_sdk_close_func,
    sdvr_get_board_count_func,
    sdvr_get_board_attributes_func,
    sdvr_get_pci_attrib_func,
    sdvr_board_connect_func,
    sdvr_board_disconnect_func,
    sdvr_upgrade_firmware_func,
    sdvr_set_sdk_params_func,
    sdvr_get_sdk_params_func,
    sdvr_set_sensor_callback_func,
    _ssdk_message_callback_func_func,
    sdvr_set_video_alarm_callback_func,
    sdvr_set_av_frame_callback_func,
    sdvr_get_board_config_func,
    sdvr_get_video_standard_func,
    sdvr_set_watchdog_state_func,
    sdvr_get_watchdog_state_func,
    sdvr_set_date_time_func,
    sdvr_get_date_time_func,
    sdvr_run_diagnostics_func,
	sdvr_dbg_msg,
    sdvr_get_supported_vstd_func,
    sdvr_write_ioctl_func,
    sdvr_read_ioctl_func,
    sdvr_enable_deinterlacing_func,
    sdvr_enable_noise_reduction_func,
    sdvr_set_gain_mode_func,
    sdvr_set_camera_termination_func,
    end_sdvr_sdk_func,

    // video frame functions
    start_sdvr_video_frame_func = end_sdvr_sdk_func,
    _ssdk_av_frame_callback_func_func,
    _ssdk_av_frame_decoder_callback_func_func,
    _ssdk_queue_frame_func,
    _ssdk_dequeue_frame_func,
    sdvr_get_av_buffer_func,
    sdvr_get_yuv_buffer_func,
    sdvr_release_av_buffer_func,
    sdvr_release_yuv_buffer_func,

    end_sdvr_video_frame_func,

    // Alarm related functions
    start_sdvr_alarm_func = end_sdvr_video_frame_func,
    sdvr_add_region_func,
    sdvr_change_region_func,
    sdvr_remove_region_func,
    sdvr_enable_motion_detection_func,
    sdvr_enable_blind_detection_func,
    sdvr_enable_night_detection_func,
    sdvr_enable_privacy_regions_func,
    sdvr_get_night_detection_func,
    sdvr_get_motion_detection_func,
    sdvr_get_blind_detection_func,
    sdvr_get_privacy_regions_func,
    sdvr_set_regions_map_func,
    sdvr_set_regions_func,
    sdvr_motion_value_analyzer_func,
    sdvr_set_motion_value_frequency_func,
    sdvr_enable_motion_detection_ex_func,
    end_sdvr_alarm_func,

    // channel functions
    start_sdvr_chan_func = end_sdvr_alarm_func,
    sdvr_create_chan_func = start_sdvr_chan_func, 
    sdvr_set_channel_default_func,
    sdvr_destroy_chan_func,
    sdvr_set_chan_user_data_func,
    sdvr_get_chan_user_data_func,
    end_sdvr_chan_func,

    // encoding functions
    start_sdvr_enc_func = end_sdvr_chan_func,
    sdvr_enable_encoder_func = start_sdvr_enc_func,
    sdvr_set_video_encoder_channel_params_func,
    sdvr_get_video_encoder_channel_params_func,
    sdvr_set_audio_encoder_channel_params_func, 
    sdvr_get_audio_encoder_channel_params_func,
    sdvr_set_alarm_video_encoder_params_func,
    sdvr_get_alarm_video_encoder_params_func,
    sdvr_enable_decoder_func,
    sdvr_set_decoder_size_func,
    sdvr_avail_decoder_buf_count_func,
    sdvr_alloc_av_buffer_func,
    sdvr_send_av_frame_func,
    end_sdvr_enc_func,

    // Display  functions
    start_sdvr_disp_func = end_sdvr_enc_func,
    sdvr_stream_raw_video_func,
    sdvr_get_video_in_params_func,
    sdvr_set_video_in_params_func,
    end_sdvr_disp_func,

    // Audion Functions
    start_sdvr_audio_func = end_sdvr_disp_func,
    sdvr_stream_raw_audio_func,
    end_sdvr_audio_func,

    // OSD functions
    start_sdvr_osd_func = end_sdvr_audio_func,
    sdvr_set_osd_text_func,
    sdvr_get_osd_text_func,
    sdvr_enable_osd_text_func,
    sdvr_osd_text_show_func,
    sdvr_osd_config_ex_func,
    sdvr_osd_text_ex_func,
    sdvr_osd_text_config_ex_func,
    end_sdvr_osd_func,

    // SMO functions
    start_sdvr_smo_func = end_sdvr_osd_func,
    sdvr_set_smo_grid_func,
    sdvr_get_smo_grid_func,
    sdvr_get_smo_attributes_func,
    sdvr_start_video_output_func,
    sdvr_stop_video_output_func,
    sdvr_get_video_output_buffer_func,
    sdvr_send_video_output_func,
    sdvr_clear_video_overlay_func,
    end_sdvr_smo_func,

    start_sdvr_sensor_relay_func  = end_sdvr_smo_func,
    sdvr_trigger_relay_func, 
    sdvr_enable_sensor_func,
    sdvr_config_sensors_func,
    sdvr_set_led_func,
    sdvr_get_relays_func,
    sdvr_get_sensors_func,
    end_sdvr_sensor_relay_func,

    // Record to file
    start_sdvr_record_file= end_sdvr_sensor_relay_func,
    sdvr_start_recording_func,
    sdvr_stop_recording_func,
    end_sdvr_record_file
};

DBG_EXTERN sdvr_err_e sdbg_init(char *szDbgFile, sx_uint32 flags);
DBG_EXTERN void sdbg_cleanup();
DBG_EXTERN sx_uint16 sdbg_get_flag();
DBG_EXTERN void sdbg_msg_output(unsigned int msg, sx_uint8 board_index, void *params);
DBG_EXTERN void sdbg_msg_output_err(unsigned int msg, sdvr_err_e err);
DBG_EXTERN void sdbg_fw_msg_output(unsigned int msg, sx_uint8 board_index, void *params, sx_bool display_params);

DBG_EXTERN void dbg_output_channel(dvr_job_t *info);
DBG_EXTERN void dbg_output_board_config(dvr_board_t *info);
DBG_EXTERN void dbg_output_osd(dvr_osd_t *cmd_strct);
DBG_EXTERN void dbg_output_osd_config_ex(dvr_osd_ex_t *cmd_strct, sx_bool bOutput_fw);
DBG_EXTERN void dbg_output_smo_params(dvr_smo_t *cmd_strct, sx_bool bOutput_fw);
DBG_EXTERN void dbg_output_ioctl_params(dvr_ioctl_t *cmd_strct,sx_bool output_fw);
DBG_EXTERN void dbg_output_board_setting(dvr_board_t *cmd_strct,sx_bool bOutput_fw);

DBG_EXTERN void dbg_dump_frame_count();

DBG_EXTERN void dbg_output_setup_chan(dvr_job_t *cmd_strct, sx_bool bOutput_fw);
DBG_EXTERN void dbg_output_venc_params(sx_uint8 board_index, dvr_encode_info_t *venc_params, sx_bool bOutput_fw);
DBG_EXTERN void dbg_output_venc_alarm_params(sx_uint8 board_index, dvr_enc_alarm_info_t *venc_params, sx_bool bOutput_fw);
DBG_EXTERN void dbg_output_md_params(dvr_md_t *cmd_strct);
DBG_EXTERN void dbg_output_bd_params(dvr_bd_t *cmd_strct);
DBG_EXTERN void dbg_output_nd_params(dvr_nd_t *cmd_strct);
DBG_EXTERN void dbg_output_pr_params(dvr_pr_t *cmd_strct);
DBG_EXTERN void dbg_output_frame_header(sdvr_av_buffer_t *frame_buf);

DBG_EXTERN sx_bool dbg_is_debug(unsigned int msg);


/* utility functions to convert various IDs to mnemonics for displaying pruposes */
DBG_EXTERN char *dbg_chip_rev_text(dvr_chip_rev_e chip_rev);
DBG_EXTERN char *dbg_message_name(dvr_message_e message);
DBG_EXTERN char *dbg_err_text(sdvr_err_e err);
DBG_EXTERN char *dbg_job_type_name(dvr_job_type_e job_type);
DBG_EXTERN dvr_job_type_e dbg_job_type_by_name(char *job_type);
DBG_EXTERN char *dbg_video_format_name(dvr_vc_format_e video_format);
DBG_EXTERN char *dbg_audio_format_name(dvr_ac_format_e audio_format);
DBG_EXTERN char *dbg_job_control_name(dvr_job_action_e control);
DBG_EXTERN char *dbg_vpp_mode_name(dvr_vpp_mode_e vpp_mode);
DBG_EXTERN dvr_vc_format_e dbg_video_format_by_name(char *video_format);
DBG_EXTERN dvr_ac_format_e dbg_audio_format_by_name(char *audio_format);
DBG_EXTERN char *dbg_camera_type_name(sx_uint16 camera_type);
DBG_EXTERN sx_uint16 dbg_camera_type_by_name(char *camera_type);
DBG_EXTERN char *dbg_chan_type_text(sdvr_chan_type_e chan_type);
DBG_EXTERN char *dbg_frame_type_text(sdvr_frame_type_e frame_type);
DBG_EXTERN char *dbg_bitrate_ctrl_text(sdvr_br_control_e bitrate_ctrl);
DBG_EXTERN sdvr_chan_type_e dbg_chan_type_by_name(char *chan_type);
DBG_EXTERN sdvr_venc_e dbg_vformat_by_name(char *vformat_name);
DBG_EXTERN sdvr_aenc_e dbg_aformat_by_name(char *aformat_name);
DBG_EXTERN sdvr_br_control_e dbg_rate_ctrl_by_name(char *rate_ctrl_name);
DBG_EXTERN sdvr_video_res_decimation_e dbg_res_decimation_by_name(char *res_des_name);
DBG_EXTERN char *dbg_res_decimation_text(sdvr_video_res_decimation_e res_decimation);
DBG_EXTERN char *dbg_osd_dts_style_text(dvr_osd_dts_e format);
DBG_EXTERN char *dbg_osd_text_loc_text(dvr_osd_pos_e position);
DBG_EXTERN char *dbg_signal_text(dvr_signal_type_e sig_type);
DBG_EXTERN char *dbg_vpp_opcode_to_str(sx_uint8 opcode);
DBG_EXTERN char *dbg_region_type_text(sx_uint8 region_type);
DBG_EXTERN char *dbg_led_type_text(sx_uint8 led_type);
DBG_EXTERN char *dbg_audio_rate_to_string(sx_uint16 audio_rate);

#endif /* STRETCH_SDVR_DEBUG_H */


