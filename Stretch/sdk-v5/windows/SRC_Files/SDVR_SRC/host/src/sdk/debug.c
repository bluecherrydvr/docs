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
#define STRETCH_SDVR_DEBUG_C

#include <time.h>
#include "sdvr_sdk_common.h"

/**********************************************************
  Temp buffer to hold the trace message text.
**********************************************************/
static char dbg_buf[512];


/****************************************************************************
  This function initializes the debugging module according to the given
  flags. It also opens the debugging file if write to file flag is set.
  Otherwise it closes it if the file was open and write to file flag is
  re-set.


  Parameters: 
    szDbgFile - The file name and its path to write the debuggin info to.
    flags - A bitmap flag to show what information needs to be debugged
              and tracked.

  Returns:
    SDVR_ERR_NONE - Debug module was initialized successfully
    SDVR_ERR_DBG_FILE - Failed to open the given debug file or the debug file
    was missing but the debug flag write to file is set.
****************************************************************************/
sdvr_err_e sdbg_init(char *szDbgFile, sx_uint32 flags)
{
    time_t tod;
    char *ts;
    char szDbgFWFile[256];

    m_dbgFlags = flags;

    time(&tod);
    ts = asctime(localtime(&tod));

    // In case the write flag is removed and the debug file was open
    // makes sure to close it.
    if (!IS_DBG_WRITE_FILE || !IS_DBG_FW_WRITE_FILE || szDbgFile)
        sdbg_cleanup();

    if (IS_DBG_WRITE_FILE)
    {
        if (m_dbgFileHandle && szDbgFile == NULL)
            return SDVR_ERR_NONE; // the caller only wanted to change the 
                                  // debug flag

        if (szDbgFile == NULL) {
            return SDVR_ERR_DBG_FILE; // No file was given
        }

        // open the file in overwrite mode.
        m_dbgFileHandle = fopen(szDbgFile,"w");
        if (m_dbgFileHandle == NULL)
            return SDVR_ERR_DBG_FILE; // failed to open the file.
        else
        {
            // put the time stamp on the file
            fprintf(m_dbgFileHandle,"\n=================================================\n");
            fprintf(m_dbgFileHandle,"Trace Started on %s",ts);
            fprintf(m_dbgFileHandle,"=================================================\n");
        }

    }

    if (IS_DBG_FW_WRITE_FILE)
    {
        sprintf(szDbgFWFile,"%s_fw", szDbgFile);
        
        // open the file in overwrite mode.
        m_dbgFWFileHandle = fopen(szDbgFWFile,"w");
        if (m_dbgFWFileHandle == NULL)
            return SDVR_ERR_DBG_FILE; // failed to open the file.
        else
        {
            // put the time stamp on the file
            fprintf(m_dbgFWFileHandle,"\n=================================================\n");
            fprintf(m_dbgFWFileHandle,"Trace Started on %s",ts);
            fprintf(m_dbgFWFileHandle,"=================================================\n");
        }
    }

    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function closes the debug flag if it is open.


  Parameters: 
    None.

  Returns:
    None.
****************************************************************************/
void sdbg_cleanup()
{
    time_t tod;
    char *ts;

    time(&tod);
    ts = asctime(localtime(&tod));

    // put the time stamp on the file

    if (m_dbgFileHandle)
    {
        fprintf(m_dbgFileHandle,"\n=================================================\n");
        fprintf(m_dbgFileHandle,"Trace Stopped on %s",ts);
        fprintf(m_dbgFileHandle,"=================================================\n");

        fclose(m_dbgFileHandle);
        m_dbgFileHandle = NULL;
    }

    if (m_dbgFWFileHandle)
    {
        fprintf(m_dbgFWFileHandle,"\n=================================================\n");
        fprintf(m_dbgFWFileHandle,"Trace Stopped on %s",ts);
        fprintf(m_dbgFWFileHandle,"=================================================\n");

        fclose(m_dbgFWFileHandle);
        m_dbgFWFileHandle = NULL;
    }
}

/****************************************************************************
  This function either displays the given text on the screen and/or to a
  debug file.

  Parameters: 
    szText - The debugging information to display/save
    bOutput_fw - The flag to indicate which log file to write

  Returns:
    None.
****************************************************************************/
void sdbg_output(char *szText, sx_bool bOutput_fw)
{
    void (*disp_debug_callback) (char *message);

    if (!IS_DBG_ON)
        return;     // debugging is off.

    if (bOutput_fw)
    {
        sdbg_fw_output(szText);
        return;
    }


    if (IS_DBG_DISP_OUTPUT)
    {
        disp_debug_callback = ssdvr_get_display_bebug_callback();

#ifdef WIN32        
        OutputDebugStringA(szText) ;
#endif
        /*
          While we are in the callback function we cannot perform
          any display I/O, otherwise there could be a deadlock
          between the DVR application and SCT calls
        */
        if (!ssdvr_in_callback)
        {
            // display the text on the screen
            if (disp_debug_callback) 
            {
                disp_debug_callback(szText);
            }
            else
            {
                printf("%s",szText);
            }
        }
    }
    if (IS_DBG_WRITE_FILE && m_dbgFileHandle)
    {
        // write the text in to the debugging file.
        fprintf(m_dbgFileHandle,"%s",szText);
        fflush(m_dbgFileHandle);
    }
}
void sdbg_fw_output(char *szText)
{
    if (!IS_DBG_ON || !IS_DBG_FW_OUTPUT)
        return;     // debugging is off.

    if (IS_DBG_FW_WRITE_FILE && m_dbgFWFileHandle)
    {
        // write the text in to the debugging file.
        fprintf(m_dbgFWFileHandle,"%s",szText);
        fflush(m_dbgFWFileHandle);
    }
}

sx_uint16 sdbg_get_flag()
{
    return m_dbgFlags;
}

void sdbg_msg_output_err(unsigned int msg, sdvr_err_e err)
{
    time_t tod;
    char *ts;

    if (!dbg_is_debug(msg))
        return;     // debugging is off.

    /* Make sure all the error codes have a time stamp*/
    time(&tod);
    ts = asctime(localtime(&tod));
    //remove the \n.
    ts[strlen(ts)-1] = '\0';
    sprintf(dbg_buf,"[%s] ",ts);
    sdbg_output(dbg_buf, false);

    switch (msg)
    {
    case sdvr_sdk_init_func:
        sprintf(dbg_buf,"sdvr_sdk_init(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_sdk_close_func:
        sprintf(dbg_buf,"sdvr_sdk_close(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_board_count_func:
        sprintf(dbg_buf,"sdvr_get_board_count(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_board_attributes_func:
        sprintf(dbg_buf,"sdvr_get_board_attributes(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_pci_attrib_func:
        sprintf(dbg_buf,"sdvr_get_pci_board_attrib(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_board_connect_func:
        sprintf(dbg_buf,"sdvr_board_connect(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_board_disconnect_func:
        sprintf(dbg_buf,"sdvr_board_disconnect(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_upgrade_firmware_func:
        sprintf(dbg_buf,"sdvr_upgrade_firmware(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_sdk_params_func:
        sprintf(dbg_buf,"sdvr_set_sdk_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_sdk_params_func:
        sprintf(dbg_buf,"sdvr_get_sdk_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_sdk_version_func:
        sprintf(dbg_buf,"sdvr_get_sdk_version(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_driver_version_func:
        sprintf(dbg_buf,"sdvr_get_driver_version(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_firmware_version_func:
        sprintf(dbg_buf,"sdvr_get_firmware_version(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_firmware_version_ex_func:
        sprintf(dbg_buf,"sdvr_get_firmware_version_ex(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_sensor_callback_func:
        sprintf(dbg_buf,"sdvr_set_sensor_callback(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_video_alarm_callback_func:
        sprintf(dbg_buf,"sdvr_set_video_alarm_callback(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_av_frame_callback_func:
        sprintf(dbg_buf,"sdvr_set_av_frame_callback(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_board_config_func:
        sprintf(dbg_buf,"sdvr_get_board_config(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_video_standard_func:
        sprintf(dbg_buf,"sdvr_get_video_standard(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_supported_vstd_func:
        sprintf(dbg_buf,"sdvr_get_supported_vstd(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_watchdog_state_func:
        sprintf(dbg_buf,"sdvr_set_watchdog_state(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_watchdog_state_func:
        sprintf(dbg_buf,"sdvr_get_watchdog_state(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_date_time_func:
        sprintf(dbg_buf,"sdvr_set_date_time(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_date_time_func:
        sprintf(dbg_buf,"sdvr_get_date_time(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_run_diagnostics_func:
        sprintf(dbg_buf,"sdvr_run_diagnostics(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case _ssdk_message_callback_func_func:
        sprintf(dbg_buf,"_ssdk_message_callback_func(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_create_chan_func:
        sprintf(dbg_buf,"sdvr_create_chan(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_channel_default_func:
        sprintf(dbg_buf,"sdvr_set_channel_default(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_destroy_chan_func:
        sprintf(dbg_buf,"sdvr_destroy_chan(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_video_encoder_channel_params_func:
        sprintf(dbg_buf,"sdvr_set_video_encoder_channel_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_video_encoder_channel_params_func:
        sprintf(dbg_buf,"sdvr_get_video_encoder_channel_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_alarm_video_encoder_params_func:
        sprintf(dbg_buf,"sdvr_set_alarm_video_encoder_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_alarm_video_encoder_params_func:
        sprintf(dbg_buf,"sdvr_get_alarm_video_encoder_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_audio_encoder_channel_params_func: 
        sprintf(dbg_buf,"sdvr_set_audio_encoder_channel_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_audio_encoder_channel_params_func:
        sprintf(dbg_buf,"sdvr_get_audio_encoder_channel_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_regions_map_func:
        sprintf(dbg_buf,"sdvr_set_regions_map(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_regions_func:
        sprintf(dbg_buf,"sdvr_set_regions(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_motion_value_analyzer_func:
        sprintf(dbg_buf,"sdvr_motion_value_analyzer(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_motion_value_frequency_func:
        sprintf(dbg_buf,"sdvr_set_motion_value_frequency(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_add_region_func:
        sprintf(dbg_buf,"sdvr_add_region(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_change_region_func:
        sprintf(dbg_buf,"sdvr_change_region(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_remove_region_func:
        sprintf(dbg_buf,"sdvr_remove_region(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_privacy_regions_func:
        sprintf(dbg_buf,"sdvr_get_privacy_regions(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_enable_privacy_regions_func:
        sprintf(dbg_buf,"sdvr_enable_privacy_regions(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_enable_motion_detection_func:
        sprintf(dbg_buf,"sdvr_enable_motion_detection(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_enable_motion_detection_ex_func:
        sprintf(dbg_buf,"sdvr_enable_motion_detection_ex(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_enable_blind_detection_func:
        sprintf(dbg_buf,"sdvr_enable_blind_detection(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_enable_night_detection_func:
        sprintf(dbg_buf,"sdvr_enable_night_detection(): Err = %s\n",sdvr_get_error_text(err));
        break;

   case sdvr_get_night_detection_func:
        sprintf(dbg_buf,"sdvr_get_night_detection(): Err = %s\n",sdvr_get_error_text(err));
        break;
     
    case sdvr_enable_encoder_func:
        sprintf(dbg_buf,"sdvr_enable_encoder(): Err = %s\n",sdvr_get_error_text(err));
        break;
    case sdvr_get_av_buffer_func:
        sprintf(dbg_buf,"sdvr_get_av_buffer(): Err = %s\n",sdvr_get_error_text(err));
        break;
    case sdvr_get_yuv_buffer_func:
        sprintf(dbg_buf,"sdvr_get_yuv_buffer(): Err = %s\n",sdvr_get_error_text(err));
        break;
    case sdvr_release_av_buffer_func:
        sprintf(dbg_buf,"sdvr_release_av_buffer(): Err = %s\n",sdvr_get_error_text(err));
        break;
    case sdvr_release_yuv_buffer_func:
        sprintf(dbg_buf,"sdvr_release_yuv_buffer(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_stream_raw_video_func:
        sprintf(dbg_buf,"sdvr_stream_raw_video(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_stream_raw_audio_func:
        sprintf(dbg_buf,"sdvr_stream_raw_audio(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_osd_text_func:
        sprintf(dbg_buf,"sdvr_set_osd_text(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_osd_text_func:
        sprintf(dbg_buf,"sdvr_get_osd_text(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_enable_osd_text_func:
        sprintf(dbg_buf,"sdvr_enable_osd_text(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_osd_text_show_func:
        sprintf(dbg_buf,"sdvr_osd_text_show(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_osd_text_config_ex_func:
        sprintf(dbg_buf,"sdvr_osd_text_config_ex(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_smo_grid_func:
        sprintf(dbg_buf,"sdvr_set_smo_text(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_smo_grid_func:
        sprintf(dbg_buf,"sdvr_get_smo_grid(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_smo_attributes_func:
        sprintf(dbg_buf,"sdvr_get_smo_attributes(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_avail_decoder_buf_count_func:
        sprintf(dbg_buf,"sdvr_avail_decoder_buf_count(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case DVR_SET_CONTROL:
        sprintf(dbg_buf,"DVR_SET_CONTROL: Err = %s\n",sdvr_get_error_text(err));
        break;

    case DVR_GET_CONTROL:
        sprintf(dbg_buf,"DVR_GET_CONTROL: Err = %s\n",sdvr_get_error_text(err));
        break;

    case DVR_GET_HMO:
        sprintf(dbg_buf,"DVR_GET_HMO: Err = %s\n",sdvr_get_error_text(err));
        break;

    case DVR_GET_ENCODE:
        sprintf(dbg_buf,"DVR_GET_ENCODE: Err = %s\n",sdvr_get_error_text(err));
        break;

    case DVR_GET_DECODE:
        sprintf(dbg_buf,"DVR_GET_DECODE: Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_trigger_relay_func:
        sprintf(dbg_buf,"sdvr_trigger_relay(): Err = %s\n",sdvr_get_error_text(err));
        break;
    case sdvr_enable_sensor_func:
        sprintf(dbg_buf,"sdvr_enable_sensor(): Err = %s\n",sdvr_get_error_text(err));
        break;
    case sdvr_config_sensors_func:
        sprintf(dbg_buf,"sdvr_config_sensors(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_enable_decoder_func:
        sprintf(dbg_buf,"sdvr_enable_decoder(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_decoder_size_func:
        sprintf(dbg_buf,"sdvr_set_decoder_size(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_alloc_av_buffer_func:
        sprintf(dbg_buf,"sdvr_alloc_av_buffer(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_send_av_frame_func:
        sprintf(dbg_buf,"sdvr_send_av_frame(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_chan_user_data_func:
        sprintf(dbg_buf,"sdvr_set_chan_user_data(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_chan_user_data_func:
        sprintf(dbg_buf,"sdvr_get_chan_user_data(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_osd_text_ex_func:
        sprintf(dbg_buf,"sdvr_osd_text_ex(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_start_recording_func:
        sprintf(dbg_buf,"sdvr_start_recording(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_stop_recording_func:
        sprintf(dbg_buf,"sdvr_stop_recording(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_video_in_params_func:
        sprintf(dbg_buf,"sdvr_get_video_in_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_set_video_in_params_func:
        sprintf(dbg_buf,"sdvr_set_video_in_params(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_start_video_output_func:
        sprintf(dbg_buf,"sdvr_start_video_output(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_stop_video_output_func:
        sprintf(dbg_buf,"sdvr_stop_video_output(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_get_video_output_buffer_func:
        sprintf(dbg_buf,"sdvr_get_video_output_buffer(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_send_video_output_func:
        sprintf(dbg_buf,"sdvr_send_video_output(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_clear_video_overlay_func:
        sprintf(dbg_buf,"sdvr_clear_video_overlay(): Err = %s\n",sdvr_get_error_text(err));
        break;

    default:
        sprintf(dbg_buf,"function unknown - %s (%x)  Err = %s\n",dbg_message_name(msg), msg,sdvr_get_error_text(err));
        break;

        return;
    } // switch (msg)

    sdbg_output(dbg_buf, false);
}

void sdbg_fw_msg_output(unsigned int msg, sx_uint8 board_index, void *params, sx_bool display_params)
{
    time_t tod;
    char *ts;
    sdebug_dvr_msg_header_t *msgHder = (sdebug_dvr_msg_header_t *)params;
    time(&tod);
    ts = asctime(localtime(&tod));
    //remove the \n.
    ts[strlen(ts)-1] = '\0';
    if (!IS_DBG_ON || !IS_DBG_FW_OUTPUT)
        return;     // debugging is off.

    switch (msg) {
    case DVR_SET_BOARD:      
    case DVR_SET_JOB:        
    case DVR_SET_SMO:        
    case DVR_SET_HMO:        
    case DVR_SET_OSD:  
    case DVR_SET_PR:         
    case DVR_SET_MD:         
    case DVR_SET_BD:         
    case DVR_SET_ND: 
    case DVR_SET_RELAYS:     
    case DVR_SET_SENSORS:    
    case DVR_SET_WATCHDOG:   
    case DVR_SET_CONTROL:    
    case DVR_SET_TIME:  
    case DVR_SET_ENCODE:
    case DVR_SET_ENC_ALARM:
    case DVR_SET_DECODE:
    case DVR_SET_FONT_TABLE:
    case DVR_SET_OSD_EX:
    case DVR_SET_UART:
    case DVR_SET_IOCTL:
    case DVR_SET_AUTH_KEY:
    case DVR_SET_REGIONS_MAP:
    case DVR_SET_VIDEO_OUTPUT:

        sprintf(dbg_buf,"\n<[%s] %s\n",ts,dbg_message_name(msg));
        break;

    case DVR_GET_VER_INFO:   
    case DVR_GET_CODEC_INFO: 
    case DVR_GET_BOARD:      
    case DVR_GET_JOB:        
    case DVR_GET_SMO:        
    case DVR_GET_HMO:        
    case DVR_GET_OSD:  
    case DVR_GET_PR:         
    case DVR_GET_MD:         
    case DVR_GET_BD:         
    case DVR_GET_ND:    
    case DVR_GET_RELAYS:     
    case DVR_GET_SENSORS:    
    case DVR_GET_WATCHDOG:   
    case DVR_GET_CONTROL:    
    case DVR_GET_TIME:       
    case DVR_GET_ENCODE:
    case DVR_GET_ENC_ALARM:
    case DVR_GET_DECODE:
    case DVR_GET_OSD_EX:
    case DVR_GET_UART:
    case DVR_GET_IOCTL:
    case DVR_GET_AUTH_KEY:
    case DVR_GET_SMO_ATTRIB:

    case DVR_SIG_HOST:
    case DVR_FW_INTERNAL:
        sprintf(dbg_buf,"\n<%s %s\n",ts,dbg_message_name(msg));
        break;

    case DVR_REP_VER_INFO:
    case DVR_REP_CODEC_INFO:
    case DVR_REP_BOARD:
    case DVR_REP_JOB:
    case DVR_REP_SMO:
    case DVR_REP_HMO:
    case DVR_REP_OSD:
    case DVR_REP_PR:
    case DVR_REP_MD:
    case DVR_REP_BD:
    case DVR_REP_ND:
    case DVR_REP_RELAYS:
    case DVR_REP_SENSORS:
    case DVR_REP_WATCHDOG:
    case DVR_REP_CONTROL:
    case DVR_REP_TIME:
    case DVR_REP_ENCODE:
    case DVR_REP_DECODE:
    case DVR_REP_FONT_TABLE:
    case DVR_REP_OSD_EX:
    case DVR_REP_UART:
    case DVR_REP_IOCTL:
    case DVR_REP_AUTH_KEY:
    case DVR_REP_REGIONS_MAP:
    case DVR_REP_VIDEO_OUTPUT:
    case DVR_REP_SMO_ATTRIB:

    case DVR_REP_ENC_ALARM:
        sprintf(dbg_buf,"\n<[%s] %s\n",ts,dbg_message_name(msg));
        break;

    default:
        sprintf(dbg_buf,"\n[%s] Unknown Command [%d]\n",ts,msg);
        break;
    }
    sdbg_fw_output(dbg_buf);

    /***************************************************************
       Print command specific parameters
    ***************************************************************/
    switch (msg) {

    case DVR_SET_BOARD: 
    {
        dvr_board_t *info = (dvr_board_t *)params;

        sprintf(dbg_buf,"\tid: %d\n", info->board_id);
        sdbg_fw_output(dbg_buf);

        dbg_output_board_setting(info, true);
        break;
    }
    case DVR_SET_JOB:     
    {
        dvr_job_t *info = (dvr_job_t *)params;

        sprintf(dbg_buf,"\tjob_type: %s\n", dbg_job_type_name(info->job_type));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_id: %d\n", info->job_id);
        sdbg_fw_output(dbg_buf);

        dbg_output_setup_chan(info, true);
        break;
    }
    case DVR_SET_SMO:        
    {
        dvr_smo_t *info = (dvr_smo_t *)params;

        sprintf(dbg_buf,"\tjob_type: %s\n", dbg_job_type_name(info->job_type));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_id: %d\n", info->job_id);
        sdbg_fw_output(dbg_buf);

        dbg_output_smo_params(info, true);

        break;
    }

    case DVR_SET_HMO:        
    {
        dvr_hmo_t *info = (dvr_hmo_t *)params;
        int stream_id;

        sprintf(dbg_buf,"\tjob_type: %s\n", dbg_job_type_name(info->job_type));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_id: %d\n", info->job_id);
        sdbg_fw_output(dbg_buf);

		// PROVIDEO -- TODO Print secondary raw video
        for (stream_id = 0; stream_id < 2; stream_id++) {
            sprintf(dbg_buf,"\tdecimation: %d\n", info->output_res[stream_id]);
            sdbg_fw_output(dbg_buf);

            sprintf(dbg_buf,"\tframe rate: %d\n", info->frame_rate[stream_id]);
            sdbg_fw_output(dbg_buf);

            sprintf(dbg_buf,"\tenable: %d\n", info->enable[stream_id]);
            sdbg_fw_output(dbg_buf);
        }

        break;
    }

    case DVR_SET_OSD:        
    {
        dvr_osd_t *info = (dvr_osd_t *)params;
        char title[20];

        strncpy(title, info->title, 10);

        sprintf(dbg_buf,"\tob_type: %s\n", dbg_job_type_name(info->job_type));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_id: %d\n", info->job_id);
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tposition: %s\n", dbg_osd_text_loc_text(info->position));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tformat: %s\n", dbg_osd_dts_style_text(info->format));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\ttitle: %s\n", title);
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tenable: %d\n", info->enable);
        sdbg_fw_output(dbg_buf);

        break;
    }

    case DVR_SET_OSD_EX:
    {
        dvr_osd_ex_t *osd_ex = (dvr_osd_ex_t*) params;

        sprintf(dbg_buf,"\tjob_id: %d\n", osd_ex->job_id);
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_type: %s\n", dbg_job_type_name(osd_ex->job_type));
        sdbg_fw_output(dbg_buf);

        dbg_output_osd_config_ex(osd_ex, true);

        return;

    }
    case DVR_SET_PR:
    case DVR_SET_MD:
    case DVR_SET_BD:
    case DVR_SET_ND:
        if (msgHder)
        {
#ifdef mxadisable
            sprintf(dbg_buf,"\tjob_id: %d\t\tjob_type: %s\n", 
                msgHder->job_id,
                dbg_job_type_name(msgHder->job_type));
            sdbg_fw_output(dbg_buf);
#endif
        }
        break;

    case DVR_GET_ENC_ALARM: 
    {
        dvr_enc_alarm_info_t *info = (dvr_enc_alarm_info_t *)params;
        sprintf(dbg_buf,"\t[%s] job_type: %s stream id = %d\n",ts, dbg_job_type_name(info->job_type),
            info->stream_id);
        sdbg_fw_output(dbg_buf);

        break;
    }

    case DVR_SET_ENCODE:
    case DVR_REP_ENCODE:
    {
        dvr_encode_info_t *info = (dvr_encode_info_t *)params;

        sprintf(dbg_buf,"\tjob_type: %s\n", dbg_job_type_name(info->job_type));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_id: %d\n", info->job_id);
        sdbg_fw_output(dbg_buf);

        dbg_output_venc_params(board_index, info, true);

        break;
    }
    case DVR_SET_ENC_ALARM:
    case DVR_REP_ENC_ALARM:
    {
        dvr_enc_alarm_info_t *info = (dvr_enc_alarm_info_t *)params;

        sprintf(dbg_buf,"\tjob_type: %s\n", dbg_job_type_name(info->job_type));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_id: %d\n", info->job_id);
        sdbg_fw_output(dbg_buf);

        dbg_output_venc_alarm_params(board_index,info, true);
        break;
    }

    case DVR_SET_RELAYS: 
        break;

    case DVR_SET_SENSORS:  
        break;

    case DVR_SET_WATCHDOG:   
    {
        dvr_watchdog_t *info = (dvr_watchdog_t *)params;

        sprintf(dbg_buf,"\tWatchdog Enable = : %d\n", info->enable);
        sdbg_fw_output(dbg_buf);

        break;
    }

    case DVR_SET_CONTROL:  
    {
        dvr_control_t *info = (dvr_control_t *)params;

        sprintf(dbg_buf,"\tjob_type: %s\n", dbg_job_type_name(info->job_type));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_id: %d\n", info->job_id);
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tenable: %d\n", info->enable);
        sdbg_fw_output(dbg_buf);

        break;
    }

    case DVR_SET_IOCTL:
    case DVR_REP_IOCTL:
    {
        dvr_ioctl_t *info = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"\tjob_type: %s\n", dbg_job_type_name(info->job_type));
        sdbg_fw_output(dbg_buf);

        sprintf(dbg_buf,"\tjob_id: %d\n", info->job_id);
        sdbg_fw_output(dbg_buf);

        dbg_output_ioctl_params(info,true);
        break;
    }

    case DVR_SET_TIME:  
        break;
    default:
        break;
    }

}
void sdbg_msg_output(unsigned int msg, sx_uint8 board_index, void *params)
{
    time_t tod;
    char *ts;
    time(&tod);
    ts = asctime(localtime(&tod));
    //remove the \n.
    ts[strlen(ts)-1] = '\0';;

    if (!dbg_is_debug(msg))
        return;     // debugging is off.
#ifdef mxadisable
    if (ssdvr_in_callback == 1 && msg != _ssdk_message_callback_func_func)
    {
        sprintf(dbg_buf, "\n**** [%s] Calling SDK API %d while in callback *****\n",ts, msg);
        sdbg_output(dbg_buf, false);
    }
#endif
    switch (msg)
    {
    case sdvr_dbg_err_msg:
        sprintf(dbg_buf,"\n[%s] %s\n",ts,(char*)params);
        break;

    case sdvr_dbg_msg:
        sprintf(dbg_buf,"\n[%s] %s\n",ts,(char*)params);
        break;

    case sdvr_sdk_init_func:
        sprintf(dbg_buf,"\n[%s] sdvr_sdk_init(): Err = %s\n",ts,(char*)params);
        break;

    case sdvr_sdk_close_func:
        sprintf(dbg_buf,"\n[%s] sdvr_sdk_close(): %s\n",ts,(char*)params);
        break;

    case sdvr_upgrade_firmware_func:
        sprintf(dbg_buf,"[%s] sdvr_upgrade_firmware(): %s\n",ts,(char*)params);
        break;


    case sdvr_get_board_count_func: {
        int *count = (int *)params;
        sprintf(dbg_buf,"\n[%s] sdvr_get_board_count(): count = %d\n",ts, *count);
        break;
    }
    case sdvr_get_board_attributes_func: {
        int i;
        sdvr_board_attrib_t *board_attrib = (sdvr_board_attrib_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_board_attributes()\n",ts);
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\tboard_type = 0x%x\tchip_rev = %s\tboard_revision = 0x%x\tpci_slot_num = %d\nsupported_video_stds = ",
                board_attrib->board_type,
                dbg_chip_rev_text(board_attrib->chip_revision),
                board_attrib->board_revision,
                board_attrib->pci_slot_num);
        sdbg_output(dbg_buf, false);
        for (i = SDVR_VIDEO_STD_D1_PAL; i <= SDVR_VIDEO_STD_4CIF_NTSC; i = i << 1) {
            if (i & board_attrib->supported_video_stds) {
                sprintf(dbg_buf," %s", dbg_camera_type_name(i));
                sdbg_output(dbg_buf, false);
            }
        }
        sdbg_output("\n", false);
        }
        return;

    case sdvr_get_supported_vstd_func: {
        int i;
        dvr_board_t *board_attrib = (dvr_board_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_supported_vstd()\n",ts);
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\tboard_index = %d\tstatus = %s\tsupported_video_stds = ",
                board_index, board_attrib->status);
        sdbg_output(dbg_buf, false);
        for (i = SDVR_VIDEO_STD_D1_PAL; i <= SDVR_VIDEO_STD_4CIF_NTSC; i = i << 1) {
            if (i & board_attrib->camera_info) {
                sprintf(dbg_buf," %s", dbg_camera_type_name(i));
                sdbg_output(dbg_buf, false);
            }
        }
        sdbg_output("\n", false);
        }
        return;

    case sdvr_get_pci_attrib_func: {
        sdvr_pci_attrib_t *pci_attrib = (sdvr_pci_attrib_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_pci_attrib()\n",ts);
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\tboard_type = 0x%x\tpci_slot_num = %d\n",
                pci_attrib->board_type,
                pci_attrib->pci_slot_num);
        sdbg_output(dbg_buf, false);
        sdbg_output("\n", false);
        sprintf(dbg_buf,"\tvendor_id = 0x%x\tdevice_id = 0x%x\tsubsystem_vendor = 0x%x\tsubsystem_id = 0x%x\n",
                pci_attrib->vendor_id,
                pci_attrib->device_id,
                pci_attrib->subsystem_vendor,
                pci_attrib->subsystem_id);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\tserial_no = %s\n",pci_attrib->serial_number);
        sdbg_output(dbg_buf, false);
        }
        return;


    case sdvr_board_disconnect_func: 
        sprintf(dbg_buf,"\n[%s] sdvr_board_disconnect(): \n\tboard_index = %d\n",ts, board_index);
        break;
    

    case sdvr_set_sdk_params_func: 
    {
        sdvr_sdk_params_t *sdk_params = (sdvr_sdk_params_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_set_sdk_params()\n",ts);
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\tdebug_file_name = %s\tdebug_flag = %x\n\tdec_buf_num = %d\tdec_buf_size = %d\n\tenc_buf_num = %d\traw_buf_num = %d\n\ttimeout = %d\n",
            sdk_params->debug_file_name,
            sdk_params->debug_flag,
            sdk_params->dec_buf_num,
            sdk_params->dec_buf_size,
            sdk_params->enc_buf_num,
            sdk_params->raw_buf_num,
            sdk_params->timeout);
        break;
    }

    case sdvr_get_sdk_params_func: 
    {
        sdvr_sdk_params_t *sdk_params = (sdvr_sdk_params_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_sdk_params()\n",ts);
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\tdebug_file_name = %s\tdebug_flag = %x\n\tdec_buf_num = %d\tdec_buf_size = %d\n\tenc_buf_num = %d\traw_buf_num = %d\n\ttimeout = %d\n",
            sdk_params->debug_file_name,
            sdk_params->debug_flag,
            sdk_params->dec_buf_num,
            sdk_params->dec_buf_size,
            sdk_params->enc_buf_num,
            sdk_params->raw_buf_num,
            sdk_params->timeout);
        break;
    }

    case _ssdk_message_callback_func_func:
    {
        dvr_sig_host_t *cmd_strct = (dvr_sig_host_t *)params;
        sprintf(dbg_buf,"\n[%s] _ssdk_message_callback(%s):\t Signal Recieved = %s\n",ts,
                ssdvr_in_callback == 1 ? "enter" : "exit",
                dbg_signal_text(cmd_strct->sig_type));
        sdbg_output(dbg_buf, false);

        switch (cmd_strct->sig_type) {
            case DVR_SIGNAL_MOTION_DETECTED:
            case DVR_SIGNAL_BLIND_DETECTED:
            case DVR_SIGNAL_NIGHT_DETECTED:
            case DVR_SIGNAL_VIDEO_LOST:
            case DVR_SIGNAL_VIDEO_DETECTED:
            case DVR_SIGNAL_RUNTIME_ERROR:
            case DVR_SIGNAL_FATAL_ERROR:
                sprintf(dbg_buf,"\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
                    board_index,cmd_strct->job_id,dbg_chan_type_text(cmd_strct->job_type));
                sdbg_output(dbg_buf, false);
            break;
        }
        sprintf(dbg_buf,"\t Signal Data = 0x%x\n",cmd_strct->signal_data);
        sdbg_output(dbg_buf, false);
        return;
    }

 case sdvr_get_firmware_version_func:
        {
        dvr_ver_info_t *cmd_strct = (dvr_ver_info_t *)params;
        sprintf(dbg_buf,"\n[%s]sdvr_get_firmware_version(): Status = %s\n",
            ts, sdvr_get_error_text(cmd_strct->status));
        sdbg_output(dbg_buf, false);
        if (cmd_strct->status == DVR_STATUS_OK)
        {
            sprintf(dbg_buf,"firmware version: v%d.%d.%d.%d - %d/%d/%d\n",
                cmd_strct->version_major, 
                cmd_strct->version_minor,
                cmd_strct->version_build,
                cmd_strct->version_bug,
                cmd_strct->build_year, 
                cmd_strct->build_month, 
                cmd_strct->build_day);
            sdbg_output(dbg_buf, false);
            sprintf(dbg_buf,"bootloader version: %d.%d - BSP version: %d.%d\n",
                cmd_strct->bootloader_ver_major, 
                cmd_strct->bootloader_ver_minor,
                cmd_strct->bsp_ver_major,
                cmd_strct->bsp_ver_minor);
            sdbg_output(dbg_buf, false);
            sprintf(dbg_buf,"board revision: 0x%x - chip revision: %s\n",
                cmd_strct->board_revision, 
                dbg_chip_rev_text(cmd_strct->chip_revision));
            sdbg_output(dbg_buf, false);
        }
        return;
        }

    case sdvr_get_firmware_version_ex_func:
        {
        sdvr_firmware_ver_t *version_info = (sdvr_firmware_ver_t *)params;
        sprintf(dbg_buf,"\n[%s] sdvr_get_firmware_version_ex(): \n",ts);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\tfirmware version: v%d.%d.%d.%d - %d/%d/%d\n",
            version_info->fw_major, 
            version_info->fw_minor,
            version_info->fw_revision,
            version_info->fw_build,
            version_info->fw_build_year, 
            version_info->fw_build_month,
            version_info->fw_build_day);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\tBoot-Loader version: %d.%d\n",
            version_info->bootloader_major, 
            version_info->bootloader_minor);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\tBSP version: %d.%d\n",
            version_info->bsp_major, 
            version_info->bsp_minor);
        sdbg_output(dbg_buf, false);

        return;
        }

    case sdvr_release_av_buffer_func:
        sprintf(dbg_buf,"\n[%s] sdvr_release_av_buffer(): \n",ts);
        sdbg_output(dbg_buf, false);

        dbg_output_frame_header((sdvr_av_buffer_t *)params);
        return;

    case sdvr_release_yuv_buffer_func: 
    {
        sdvr_av_buffer_t *frame = (sdvr_av_buffer_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_release_yuv_buffer(): \n",ts);
        sdbg_output(dbg_buf, false);

        dbg_output_frame_header(frame);
        return;
    }
    case DVR_SET_BOARD:
    {
        dvr_board_t *cmd_strct = (dvr_board_t *)params;
        sprintf(dbg_buf,"\n[%s] DVR_SET_BOARD-sdvr_board_connect(): Status = %s\n\tboard_index = %d\n",
                ts, sdvr_get_error_text(cmd_strct->status),cmd_strct->board_id);
        sdbg_output(dbg_buf, false);

        dbg_output_board_setting(cmd_strct, false);

        break;    
    }
    case sdvr_get_board_config_func:
    {
        dvr_board_t *cmd_strct = (dvr_board_t *)params;
        sprintf(dbg_buf,"\n[%s] sdvr_get_board_config(): Status = %s\n",ts, sdvr_get_error_text(cmd_strct->status));
        sdbg_output(dbg_buf, false);

        if (cmd_strct->status == DVR_STATUS_OK)
        {
            dbg_output_board_config(cmd_strct);
        }
        return;
    }
    case sdvr_get_video_standard_func:
    {
        dvr_board_t *cmd_strct = (dvr_board_t *)params;
        sprintf(dbg_buf,"\n[%s] sdvr_get_video_standard(): Status = %s\n\t%s\n",
                ts, sdvr_get_error_text(cmd_strct->status),
                dbg_camera_type_name(cmd_strct->camera_type));
        break;
    }

    case  sdvr_get_watchdog_state_func:
    {
        dvr_watchdog_t *cmd_strct = (dvr_watchdog_t *)params;
        sprintf(dbg_buf,"\n[%s] sdvr_get_watchdog_state(): status = %s\n\tboard_index = %d\tEnable = %d\n",
                ts, sdvr_get_error_text(cmd_strct->status),board_index,cmd_strct->enable);
        break;
    }
    case  sdvr_set_watchdog_state_func:
    {
        dvr_watchdog_t *cmd_strct = (dvr_watchdog_t *)params;
        sprintf(dbg_buf,"\n[%s] sdvr_set_watchdog_state(): status = %s\n\tboard_index = %d\tEnable = %d\n",
                ts, sdvr_get_error_text(cmd_strct->status),board_index,cmd_strct->enable);
        break;
    }

    case sdvr_create_chan_func:
    {
        dvr_job_t *cmd_strct = (dvr_job_t *)params;
        sprintf(dbg_buf,"\n[%s] sdvr_create_chan()\tstatus = %s\n\tboard_index = %d\n", 
                ts, sdvr_get_error_text(cmd_strct->status),board_index);
        sdbg_output(dbg_buf, false);

        dbg_output_setup_chan(params, false);
        return;
    }

    case  sdvr_destroy_chan_func:
    {
        dvr_job_t *cmd_strct = (dvr_job_t *)params;
        sprintf(dbg_buf,"\n[%s] sdvr_destroy_chan():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
           ts, board_index,cmd_strct->job_id,dbg_chan_type_text(cmd_strct->job_type));
        break;
    }


    case sdvr_set_video_encoder_channel_params_func:
    {
        sdvr_chan_handle_t *handle = (sdvr_chan_handle_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_set_video_encoder_channel_params():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, sdvr_get_board_index(*handle),sdvr_get_chan_num(*handle),
            dbg_chan_type_text(sdvr_get_chan_type(*handle)));
        break;
    }

    case sdvr_get_video_encoder_channel_params_func:
    {
        sdvr_chan_handle_t *handle = (sdvr_chan_handle_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_video_encoder_channel_params():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, sdvr_get_board_index(*handle),sdvr_get_chan_num(*handle),
            dbg_chan_type_text(sdvr_get_chan_type(*handle)));
        break;
    }

    case sdvr_set_alarm_video_encoder_params_func:
    {
        sdvr_chan_handle_t *handle = (sdvr_chan_handle_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_set_alarm_video_encoder_params():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, sdvr_get_board_index(*handle),sdvr_get_chan_num(*handle),
            dbg_chan_type_text(sdvr_get_chan_type(*handle)));
        break;
    }

    case sdvr_get_alarm_video_encoder_params_func:
    {
        sdvr_chan_handle_t *handle = (sdvr_chan_handle_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_alarm_video_encoder_params():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, sdvr_get_board_index(*handle),sdvr_get_chan_num(*handle),
            dbg_chan_type_text(sdvr_get_chan_type(*handle)));
        break;
    }

    case sdvr_set_channel_default_func:
    {
        dvr_job_t *cmd_strct = (dvr_job_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_set_channel_default():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type));
        break;
    }

    case sdvr_get_motion_detection_func:
    {
        sdebug_sdvr_msg_wrapper_t *cmd_strct = (sdebug_sdvr_msg_wrapper_t *)params;
        sdvr_motion_detection_t *md_strct = (sdvr_motion_detection_t *)cmd_strct->strct;
        sprintf(dbg_buf,"\n[%s] sdvr_get_motion_detection():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\tthreshold\tnum_regions = %d\n",
            ts, board_index,cmd_strct->chan_num,
            dbg_chan_type_text(cmd_strct->chan_type),
            md_strct->enable,md_strct->threshold, md_strct->num_regions);
        break;
    }
        
    case sdvr_set_regions_map_func:
    {
        dvr_regions_map_t *cmd_strct = (dvr_regions_map_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_set_regions_map(): Status = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\tregion_type = %s\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            dbg_region_type_text(cmd_strct->region_type));
        break;
    }
/*
    case sdvr_set_regions_func:
        sprintf(dbg_buf,"sdvr_set_regions(): Err = %s\n",sdvr_get_error_text(err));
        break;

    case sdvr_motion_value_analyzer_func:
        sprintf(dbg_buf,"sdvr_motion_value_analyzer(): Err = %s\n",sdvr_get_error_text(err));
        break;
*/
    case sdvr_set_motion_value_frequency_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_set_motion_value_frequency(): Status = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\tfrequency = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.motion.frequency);

        break;
    }
    case sdvr_change_region_func:
    case sdvr_add_region_func:
    {
        sdebug_sdvr_msg_wrapper_t *msg_wrapper = (sdebug_sdvr_msg_wrapper_t *)params;
        switch (msg_wrapper->func_id)
        {
        case SDVR_REGION_MOTION:
            {
            dvr_md_t *cmd_strct = (dvr_md_t *)msg_wrapper->strct;

            sprintf(dbg_buf,"\n[%s] %s(MD):\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tindex = %d\tenable = %d\n",
                ts, (msg == sdvr_add_region_func) ? "sdvr_add_region" : "sdvr_change_region_func",
                board_index,cmd_strct->job_id,
                dbg_chan_type_text(cmd_strct->job_type),
                cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
            sdbg_output(dbg_buf, false);

            sprintf(dbg_buf,"\tx_TL = %d\ty_TL = %d\tx_BR = %d\ty_BR = %d\n",
                cmd_strct->u1.reg.x_TL,cmd_strct->u1.reg.y_TL,cmd_strct->u1.reg.x_BR,cmd_strct->u1.reg.y_BR);
            break;
            }
        case SDVR_REGION_BLIND:
            {
            dvr_bd_t *cmd_strct = (dvr_bd_t *)msg_wrapper->strct;

            sprintf(dbg_buf,"\n[%s] %s(BD):\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tindex = %d\tenable = %d\n",
                ts, (msg == sdvr_add_region_func) ? "sdvr_add_region" : "sdvr_change_region_func",
                board_index,cmd_strct->job_id,
                dbg_chan_type_text(cmd_strct->job_type),
                cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
            sdbg_output(dbg_buf, false);

            sprintf(dbg_buf,"\tx_TL = %d\ty_TL = %d\tx_BR = %d\ty_BR = %d\n",
                cmd_strct->u1.reg.x_TL,cmd_strct->u1.reg.y_TL,cmd_strct->u1.reg.x_BR,cmd_strct->u1.reg.y_BR);
            break;
            }
        case SDVR_REGION_PRIVACY:
            {
            dvr_pr_t *cmd_strct = (dvr_pr_t *)msg_wrapper->strct;

            sprintf(dbg_buf,"\n[%s] %s(PR):\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tindex = %d\tenable = %d\n",
                ts, (msg == sdvr_add_region_func) ? "sdvr_add_region" : "sdvr_change_region_func",
                board_index,cmd_strct->job_id,
                dbg_chan_type_text(cmd_strct->job_type),
                cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
            sdbg_output(dbg_buf, false);

            sprintf(dbg_buf,"\tx_TL = %d\ty_TL = %d\tx_BR = %d\ty_BR = %d\n",
                cmd_strct->u1.reg.x_TL,cmd_strct->u1.reg.y_TL,cmd_strct->u1.reg.x_BR,cmd_strct->u1.reg.y_BR);
            break;
            }

        default:
            sprintf(dbg_buf,"\n[%s] %s(??[%d])\n",
                ts, (msg == sdvr_add_region_func) ? "sdvr_add_region" : "sdvr_change_region_func",
                msg_wrapper->func_id);
            break;
        }

        break;
    }
    case sdvr_remove_region_func:
    {
        sdebug_sdvr_msg_wrapper_t *msg_wrapper = (sdebug_sdvr_msg_wrapper_t *)params;
        switch (msg_wrapper->func_id)
        {
        case SDVR_REGION_MOTION:
            {
            dvr_md_t *cmd_strct = (dvr_md_t *)msg_wrapper->strct;

            sprintf(dbg_buf,"\n[%s] sdvr_remove_region(MD):\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tindex = %d\tenable = %d\n",
               ts,  board_index,cmd_strct->job_id,
                dbg_chan_type_text(cmd_strct->job_type),
                cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
            break;
            }
        case SDVR_REGION_BLIND:
            {
            dvr_bd_t *cmd_strct = (dvr_bd_t *)msg_wrapper->strct;

            sprintf(dbg_buf,"\n[%s] sdvr_remove_region(BD):\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tindex = %d\tenable = %d\n",
                ts, board_index,cmd_strct->job_id,
                dbg_chan_type_text(cmd_strct->job_type),
                cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
            break;
            }
        case SDVR_REGION_PRIVACY:
            {
            dvr_pr_t *cmd_strct = (dvr_pr_t *)msg_wrapper->strct;

            sprintf(dbg_buf,"\n[%s] sdvr_remove_region(PR):\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tindex = %d\tenable = %d\n",
                ts, board_index,cmd_strct->job_id,
                dbg_chan_type_text(cmd_strct->job_type),
                cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
            break;
            }
        default:
            sprintf(dbg_buf,"\nsdvr_remove_region(??[%d])\n",
                msg_wrapper->func_id);
            break;
        }

        break;
    }

    case sdvr_enable_motion_detection_func:
    {
        dvr_md_t *cmd_strct = (dvr_md_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_enable_motion_detection():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\tthreshold = %d\n",
            ts, board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.ctrl.enable,cmd_strct->u1.ctrl.threshold);
        break;
    }

    case sdvr_enable_motion_detection_ex_func:
    {
        dvr_regions_map_t *cmd_strct = (dvr_regions_map_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_enable_motion_detection_ex():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
                ts, board_index,cmd_strct->job_id,
                dbg_chan_type_text(cmd_strct->job_type));
        sdbg_output(dbg_buf, false);

        if (cmd_strct->opcode == DVR_REGION_OPCODE_CONFIG)
        {
            sprintf(dbg_buf,"\tthreshold1 = %d\tthreshold2 = %d\tthreshold3 = %d\tthreshold4 = %d\n",
                cmd_strct->u1.config.threshold1,
                cmd_strct->u1.config.threshold2,
                cmd_strct->u1.config.threshold3,
                cmd_strct->u1.config.threshold4);
        }
        else if (cmd_strct->opcode == DVR_REGION_OPCODE_ENABLE)
        {

            sprintf(dbg_buf,"\tenable = %d\n",cmd_strct->u1.enable.enable_flag);
        }

        break;

    }

    case sdvr_enable_blind_detection_func:
    {
        dvr_bd_t *cmd_strct = (dvr_bd_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_enable_blind_detection():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\tthreshold = %d\n",
            ts, board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.ctrl.enable,cmd_strct->u1.ctrl.threshold);
        break;
    }

    case sdvr_enable_privacy_regions_func:
    {
        dvr_pr_t *cmd_strct = (dvr_pr_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_enable_privacy_regions():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\n",
            ts, board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.ctrl.enable);
        break;
    }

    case sdvr_enable_night_detection_func:
    {
        dvr_nd_t *cmd_strct = (dvr_nd_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_enable_night_detection():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\tthreshold = %d\n",
            ts, board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->enable,cmd_strct->threshold);
        break;
    }
    case sdvr_get_night_detection_func:
    {
        dvr_nd_t *cmd_strct = (dvr_nd_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_night_detection():\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\tthreshold = %d\n",
            ts, board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->enable,cmd_strct->threshold);
        break;
    }


    case sdvr_enable_encoder_func:
    {
        dvr_encode_info_t *encode_strct = (dvr_encode_info_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_enable_encoder():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tstream_id = %d\tChan_type = %s\n\tEnable = %d\n",
            ts, sdvr_get_error_text(encode_strct->status),
            board_index,
            encode_strct->job_id,
            encode_strct->stream_id,
            dbg_chan_type_text(encode_strct->job_type),encode_strct->enable);
        break;
    }

    case sdvr_enable_decoder_func:
    {
        dvr_control_t *cmd_strct = (dvr_control_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_enable_decoder():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),cmd_strct->enable);
        break;
    }

    case sdvr_set_decoder_size_func:
    {
        sdvr_chan_handle_t *handle = (sdvr_chan_handle_t *)params;
        sx_uint8 chan_num = sdvr_get_chan_num(*handle);

        sprintf(dbg_buf,"\n[%s] sdvr_set_decoder_size():\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tWidth = %d\tLines = %d\n",
            ts, board_index,chan_num,
            dbg_chan_type_text(sdvr_get_chan_type(*handle)),
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vsize_width,
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vsize_lines);
        break;

    }
    case sdvr_stream_raw_video_func:
    {
        dvr_hmo_t *cmd_strct = (dvr_hmo_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_stream_raw_video():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\tres_decimation = %s\n\tframe rate = %d",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),cmd_strct->enable[0],
            dbg_res_decimation_text(cmd_strct->output_res[0]),cmd_strct->frame_rate);
        break;
    }
    case sdvr_stream_raw_audio_func:
    {
        dvr_hmo_t *cmd_strct = (dvr_hmo_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_stream_raw_audio():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),cmd_strct->audio_enable);
        break;

    }
    case sdvr_set_osd_text_func:
    {
        dvr_osd_t *cmd_strct = (dvr_osd_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_set_osd_text():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type));
        sdbg_output(dbg_buf, false);

        dbg_output_osd(cmd_strct);
        return;
    }

    case sdvr_get_osd_text_func:
    {
        dvr_osd_t *cmd_strct = (dvr_osd_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_osd_text():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type));
        sdbg_output(dbg_buf, false);

        dbg_output_osd(cmd_strct);
        return;
    }

    case sdvr_enable_osd_text_func:
    {
        dvr_osd_t *cmd_strct = (dvr_osd_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_enable_osd_text():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n\tEnable = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),cmd_strct->enable);
        break;
    }

    case sdvr_osd_text_show_func:
    {
        dvr_osd_ex_t *cmd_strct = (dvr_osd_ex_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_osd_text_show():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type));
        sdbg_output(dbg_buf, false);

        dbg_output_osd_config_ex(cmd_strct, false);

        return;

        break;
    }

    case sdvr_osd_config_ex_func:
    {
        dvr_osd_ex_t *cmd_strct = (dvr_osd_ex_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_osd_config_ex():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type));
        sdbg_output(dbg_buf, false);

        dbg_output_osd_config_ex(cmd_strct, false);

        return;
    }
    case sdvr_set_smo_grid_func:
    case sdvr_get_smo_grid_func:
    {
        dvr_smo_t *cmd_strct = (dvr_smo_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_%s_smo_grid():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\n",
            ts, msg == sdvr_set_smo_grid_func ? "set" : "get",
            sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type));
        sdbg_output(dbg_buf, false);

        dbg_output_smo_params(cmd_strct, false);
        return;
    }

    case sdvr_get_smo_attributes_func:
    {
        dvr_smo_attrib_t *cmd_strct = (dvr_smo_attrib_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_smo_attributes():\tStatus = %s\n\tBoard_index = %d\tSMO_port_num = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->smo_port_num);
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\twidth = %d\theight = %d\n\tvideo_formats = 0x%x\tcap_flags = 0x%x\n",
            cmd_strct->width,cmd_strct->height,
            cmd_strct->video_formats, cmd_strct->cap_flags);
        sdbg_output(dbg_buf, false);
        return;
    }

    case sdvr_start_video_output_func:
    {
        dvr_videoout_t *cmd_strct = (dvr_videoout_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_start_video_output():\tStatus = %s\n\tBoard_index = %d\tSMO_port_num = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->smo_port_num);
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\tvideo_formats = 0x%x\n",
            cmd_strct->u1.start.video_format);
        sdbg_output(dbg_buf, false);

        return;
    }
    case sdvr_stop_video_output_func:
    {
        dvr_videoout_t *cmd_strct = (dvr_videoout_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_stop_video_output():\tStatus = %s\n\tBoard_index = %d\tSMO_port_num = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->smo_port_num);
        break;
    }

    case _ssdk_av_frame_callback_func_func:
    {

        sprintf(dbg_buf,"\n[%s] _ssdk_av_frame_callback_func():\n\tBoard_index = %d\n\t%s\n",
            ts, board_index,(char *)params);
        break;

    }
    case _ssdk_av_frame_decoder_callback_func_func:
    {

        sprintf(dbg_buf,"\n[%s] _ssdk_av_frame_decoder_callback_func():\n\tBoard_index = %d\n\t%s\n",
            ts, board_index,(char *)params);
        break;

    }
    case _ssdk_queue_frame_func:
    {

        sprintf(dbg_buf,"\n[%s] _ssdk_queue_frame():\n\tBoard_index = %d\n\t%s\n",
            ts, board_index,(char *)params);
        break;

    }
    case _ssdk_dequeue_frame_func:
    {

        sprintf(dbg_buf,"\n[%s] _ssdk_dequeue_frame():\n\tBoard_index = %d\n\t%s\n",
            ts, board_index,(char *)params);
        break;
    }

    case sdvr_get_relays_func:
    case sdvr_trigger_relay_func:
    {
        dvr_relay_t *cmd_strct = (dvr_relay_t *)params;

        sprintf(dbg_buf,"\n[%s] %s:\tStatus = %s\n\tRelay Status Map = 0x%x\n",
            ts, msg == sdvr_get_relays_func ? "sdvr_get_relays()" : "sdvr_trigger_relay()",
            sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->relay_status);
        break;
    }

    case sdvr_enable_sensor_func:
    case sdvr_config_sensors_func:
    {
        dvr_sensor_t *cmd_strct = (dvr_sensor_t *)params;

        sprintf(dbg_buf,"\n[%s] %s:\tStatus = %s\tboard_index = %d\n\tSensor Status Map = 0x%x\n\tSensor enable Map = 0x%x\n\tEdge triggered Map = 0x%x\n",
            ts, msg == sdvr_enable_sensor_func ? "sdvr_enable_sensor()" : "sdvr_config_sensors()",
            sdvr_get_error_text(cmd_strct->status),
            board_index,
            cmd_strct->sensor_status,cmd_strct->sensor_enable,cmd_strct->edge_triggered);
        break;
    }

    case sdvr_get_sensors_func:
    {
        dvr_sensor_t *cmd_strct = (dvr_sensor_t *)params;

        sprintf(dbg_buf,"\n[%s] sdvr_get_sensors():\tStatus = %s\tboard_index = %d\n\tSensor Status Map = 0x%x\n\tSensor enable Map = 0x%x\n\tEdge triggered Map = 0x%x\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,
            cmd_strct->sensor_status,cmd_strct->sensor_enable,cmd_strct->edge_triggered);
        break;
    }

    case sdvr_get_video_in_params_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"[%s] sdvr_get_video_in_params():\tStatus = %s\tboard_index = %d\n\thue = 0x%x\tsaturation = 0x%x\tbrightness = 0x%x\tcontrast = 0x%x\tsharpness = 0x%x\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,
            cmd_strct->u1.img_ctrl.hue,cmd_strct->u1.img_ctrl.sat,cmd_strct->u1.img_ctrl.brt,
            cmd_strct->u1.img_ctrl.cont, cmd_strct->u1.img_ctrl.sharp);

        break;
    }

    case sdvr_set_video_in_params_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"[%s] sdvr_set_video_in_params():\tStatus = %s\tboard_index = %d\t flags = 0x%x\n\thue = 0x%x\tsaturation = 0x%x\tbrightness = 0x%x\tcontrast = 0x%x\tsharpness = 0x%x\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,
            cmd_strct->u1.img_ctrl.flags,
            cmd_strct->u1.img_ctrl.hue,cmd_strct->u1.img_ctrl.sat,cmd_strct->u1.img_ctrl.brt,
            cmd_strct->u1.img_ctrl.cont, cmd_strct->u1.img_ctrl.sharp);
        break;
    }

    case sdvr_read_ioctl_func:
    case sdvr_write_ioctl_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"[%s] sdvr_%s_ioctl():\tStatus = %s\tboard_index = %d\tChan_num = %d\tChan_type = %s\n\t device_id = %d\treg_num = %d\tvalue = %d\n",
            ts, msg == sdvr_read_ioctl_func ? "read" : "write",
            sdvr_get_error_text(cmd_strct->status),
            board_index, cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.reg.device_id,
            cmd_strct->u1.reg.reg_num,
            cmd_strct->u1.reg.val);
        break;
    }
    case sdvr_enable_deinterlacing_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"[%s] sdvr_enable_deinterlacing():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\tvpp_actions = 0x%x\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.vpp_ctrl.actions);
        break;
    }
    case sdvr_enable_noise_reduction_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"[%s] sdvr_enable_noise_reduction():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\tvpp_actions = 0x%x\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.vpp_ctrl.actions);
        break;
    }
    case sdvr_set_gain_mode_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"[%s] sdvr_set_gain_mode():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\tvalue = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.gain_mode.value);
        break;
    }
    case sdvr_set_camera_termination_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"[%s] sdvr_set_camera_termination():\tStatus = %s\n\tBoard_index = %d\tChan_num = %d\tChan_type = %s\tvalue = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,cmd_strct->job_id,
            dbg_chan_type_text(cmd_strct->job_type),
            cmd_strct->u1.termination.value);
        break;
    }

    case sdvr_set_led_func:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)params;

        sprintf(dbg_buf,"[%s] sdvr_set_led():\tStatus = %s\n\tBoard_index = %d\nled_type = %s\tled_num = %d\tenable = %d\n",
            ts, sdvr_get_error_text(cmd_strct->status),
            board_index,
            dbg_led_type_text(cmd_strct->u1.led.type),
            cmd_strct->u1.led.number,
            cmd_strct->u1.led.enable);
        break;
    }
    case DVR_SET_MD:
    case DVR_GET_MD:
    {
        dvr_md_t *cmd_strct = (dvr_md_t *)params;
        sprintf(dbg_buf,"\t%s Motion Detection Parameters:\tstatus = %s\n",
                msg == DVR_SET_MD ? "Set" : "Get",
                sdvr_get_error_text(cmd_strct->status));
        sdbg_output(dbg_buf, false);

        dbg_output_md_params(cmd_strct);
        return;
    }

    case DVR_SET_BD:
    case DVR_GET_BD:
    {
        dvr_bd_t *cmd_strct = (dvr_bd_t *)params;
        sprintf(dbg_buf,"\t%s Blind Detection Parameters:\tstatus = %s\n",
                msg == DVR_SET_BD ? "Set" : "Get",
                sdvr_get_error_text(cmd_strct->status));
        sdbg_output(dbg_buf, false);

        dbg_output_bd_params(cmd_strct);
        return;
    }
    case DVR_SET_ND:
    case DVR_GET_ND:
    {
        dvr_nd_t *cmd_strct = (dvr_nd_t *)params;
        sprintf(dbg_buf,"\t%s Night Detection Parameters:\tstatus = %s\n",
                msg == DVR_SET_ND ? "Set" : "Get",
                sdvr_get_error_text(cmd_strct->status));
        sdbg_output(dbg_buf, false);

        dbg_output_nd_params(cmd_strct);
        return;
    }

    case DVR_SET_PR:
    case DVR_GET_PR:
    {
        dvr_pr_t *cmd_strct = (dvr_pr_t *)params;
        sprintf(dbg_buf,"\t%s Privacy Region Parameters:\tstatus = %s\n",
                msg == DVR_SET_PR ? "Set" : "Get",
                sdvr_get_error_text(cmd_strct->status));
        sdbg_output(dbg_buf, false);

        dbg_output_pr_params(cmd_strct);
        return;
    }

    case DVR_SET_TIME:
    case DVR_GET_TIME:
    {
        dvr_time_t *cmd_strct = (dvr_time_t *)params;
        sprintf(dbg_buf,"[%s] DVR_%s_TIME:\tboard_index = %d status = %s\n\ttv_sec = %d",
                ts, msg == DVR_SET_TIME ? "SET" : "GET",
                board_index,
                sdvr_get_error_text(cmd_strct->status),
                cmd_strct->tv_sec);
        break;
    }

    default:
        sprintf(dbg_buf,"\n[%s] function unknown - %s (%x)  \n",ts, dbg_message_name(msg), msg);
        break;;
    } // switch (msg)

    sdbg_output(dbg_buf, false);
}


/************************************************************************
    Print the board configuration information
************************************************************************/
void dbg_output_board_config(dvr_board_t *info)
{
    int i;
    sdbg_output("  Board Info:\n", false);
    sprintf(dbg_buf,"    id: %d\n", info->board_id);
    sdbg_output(dbg_buf, false);
    sprintf(dbg_buf,"    num_cameras: %d\n", info->num_cameras);
    sdbg_output(dbg_buf, false);
    sprintf(dbg_buf,"    num_sensors: %d\n", info->num_sensors);
    sdbg_output(dbg_buf, false);
    sprintf(dbg_buf,"    num_relays: %d\n", info->num_relays);
    sdbg_output(dbg_buf, false);
    sprintf(dbg_buf,"    num_smos: %d\n", info->num_smos);
    sdbg_output(dbg_buf, false);


    sdbg_output("    camera_info:", false);
    for (i = SDVR_VIDEO_STD_D1_PAL; i <= SDVR_VIDEO_STD_4CIF_NTSC; i = i << 1) {
        if (i & info->camera_info) {
            sprintf(dbg_buf,"   %s", dbg_camera_type_name(i));
            sdbg_output(dbg_buf, false);
        }
    }
    sdbg_output("\n", false);

    sprintf(dbg_buf,"    camera_type: %s\n", dbg_camera_type_name(info->camera_type));
    sdbg_output(dbg_buf, false);

}
/************************************************************************
    Print the frame header information
************************************************************************/

void  dbg_output_frame_header(sdvr_av_buffer_t *frame_buf)
{
    sprintf(dbg_buf,"\tboard_index = %d\n", frame_buf->board_id);
    sdbg_output(dbg_buf, false);
    sprintf(dbg_buf,"\tchan_num = %d\n", frame_buf->channel_id);
    sdbg_output(dbg_buf, false);
    sprintf(dbg_buf,"\tchan_type = %s\n", dbg_chan_type_text(frame_buf->channel_type));
    sdbg_output(dbg_buf, false);
    sprintf(dbg_buf,"\tframe_type = %s\n", dbg_frame_type_text(frame_buf->frame_type));
    sdbg_output(dbg_buf, false);
}

/************************************************************************
    Print the setup channel information
************************************************************************/
void dbg_output_setup_chan(dvr_job_t *cmd_strct, sx_bool bOutput_fw)
{
    sprintf(dbg_buf,"\tchan_num: %d\n", cmd_strct->job_id);
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tchan_type: %s\n", dbg_job_type_name(cmd_strct->job_type));
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tvideo_format: %s\n", dbg_video_format_name(cmd_strct->video_format));
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tvideo_format_2: %s\n", dbg_video_format_name(cmd_strct->video_format_2));
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\taudio_format: %s\n", dbg_audio_format_name(cmd_strct->audio_format));
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tcontrol: %s\n", dbg_job_control_name(cmd_strct->control));
    sdbg_output(dbg_buf, bOutput_fw);

    // VPP mode only applies to encoder channels.
    if (cmd_strct->job_type ==  SDVR_CHAN_TYPE_ENCODER)
    {
        sprintf(dbg_buf,"\tvpp_mode: %s\n", dbg_vpp_mode_name(cmd_strct->vpp_mode));
        sdbg_output(dbg_buf, bOutput_fw);
    }

}
/************************************************************************
    Print the Motion Detection information
************************************************************************/
void   dbg_output_md_params(dvr_md_t *cmd_strct)
{
    sprintf(dbg_buf,"\t\tchan_num: %d\n", cmd_strct->job_id);
    sdbg_output(dbg_buf, false);

    sprintf(dbg_buf,"\t\tchan_type: %s\n", dbg_job_type_name(cmd_strct->job_type));
    sdbg_output(dbg_buf, false);

    sprintf(dbg_buf,"\t\top_code: %s\n", dbg_vpp_opcode_to_str(cmd_strct->op_code));
    sdbg_output(dbg_buf, false);

    switch(cmd_strct->op_code)
    {
    case DVR_VPP_OPCODE_CONTROL:
        sprintf(dbg_buf,"\t\tenable: %d\n", cmd_strct->u1.ctrl.enable);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\t\tthreshold: %d\n", cmd_strct->u1.ctrl.threshold);
        sdbg_output(dbg_buf, false);

        break;
    case DVR_VPP_OPCODE_REGIONS:
        sprintf(dbg_buf,"\t\tindex: %d\tenable = %d\n", cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\t\tx_TL = %d,  y_TL = %d,  x_BR = %d,  y_BR = %d\n", 
                cmd_strct->u1.reg.x_TL, cmd_strct->u1.reg.y_TL, cmd_strct->u1.reg.x_BR, cmd_strct->u1.reg.y_BR);
        sdbg_output(dbg_buf, false);
        break;
    default:
        break;
    }
}
/************************************************************************
    Print the Blind  Detection information
************************************************************************/
void   dbg_output_bd_params(dvr_bd_t *cmd_strct)
{
    sprintf(dbg_buf,"\t\tchan_num: %d\n", cmd_strct->job_id);
    sdbg_output(dbg_buf, false);

    sprintf(dbg_buf,"\t\tchan_type: %s\n", dbg_job_type_name(cmd_strct->job_type));
    sdbg_output(dbg_buf, false);

    sprintf(dbg_buf,"\t\top_code: %s\n", dbg_vpp_opcode_to_str(cmd_strct->op_code));
    sdbg_output(dbg_buf, false);

    switch(cmd_strct->op_code)
    {
    case DVR_VPP_OPCODE_CONTROL:
        sprintf(dbg_buf,"\t\tenable: %d\n", cmd_strct->u1.ctrl.enable);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\t\tthreshold: %d\n", cmd_strct->u1.ctrl.threshold);
        sdbg_output(dbg_buf, false);

        break;
    case DVR_VPP_OPCODE_REGIONS:
        sprintf(dbg_buf,"\t\tindex: %d\tenable = %d\n", cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\t\tx_TL = %d,  y_TL = %d,  x_BR = %d,  y_BR = %d\n", 
                cmd_strct->u1.reg.x_TL, cmd_strct->u1.reg.y_TL, cmd_strct->u1.reg.x_BR, cmd_strct->u1.reg.y_BR);
        sdbg_output(dbg_buf, false);
        break;
    default:
        break;
    }
}
/************************************************************************
    Print the Night Detection information
************************************************************************/
void   dbg_output_nd_params(dvr_nd_t *cmd_strct)
{
    sprintf(dbg_buf,"\t\tchan_num: %d\n", cmd_strct->job_id);
    sdbg_output(dbg_buf, false);

    sprintf(dbg_buf,"\t\tchan_type: %s\n", dbg_job_type_name(cmd_strct->job_type));
    sdbg_output(dbg_buf, false);

    sprintf(dbg_buf,"\t\tenable: %d\n", cmd_strct->enable);
    sdbg_output(dbg_buf, false);
    sprintf(dbg_buf,"\t\tthreshold: %d\n", cmd_strct->threshold);
    sdbg_output(dbg_buf, false);

}
/************************************************************************
    Print the Privacy regions information
************************************************************************/
void   dbg_output_pr_params(dvr_pr_t *cmd_strct)
{
    sprintf(dbg_buf,"\t\tchan_num: %d\n", cmd_strct->job_id);
    sdbg_output(dbg_buf, false);

    sprintf(dbg_buf,"\t\tchan_type: %s\n", dbg_job_type_name(cmd_strct->job_type));
    sdbg_output(dbg_buf, false);

    sprintf(dbg_buf,"\t\top_code: %s\n", dbg_vpp_opcode_to_str(cmd_strct->op_code));
    sdbg_output(dbg_buf, false);

    switch(cmd_strct->op_code)
    {
    case DVR_VPP_OPCODE_CONTROL:
        sprintf(dbg_buf,"\t\tenable: %d\n", cmd_strct->u1.ctrl.enable);
        sdbg_output(dbg_buf, false);

        break;
    case DVR_VPP_OPCODE_REGIONS:
        sprintf(dbg_buf,"\t\tindex: %d\tenable = %d\n", cmd_strct->u1.reg.index,cmd_strct->u1.reg.enable);
        sdbg_output(dbg_buf, false);
        sprintf(dbg_buf,"\t\tx_TL = %d,  y_TL = %d,  x_BR = %d,  y_BR = %d\n", 
                cmd_strct->u1.reg.x_TL, cmd_strct->u1.reg.y_TL, cmd_strct->u1.reg.x_BR, cmd_strct->u1.reg.y_BR);
        sdbg_output(dbg_buf, false);
        break;
    default:
        break;
    }
}
/************************************************************************
    Print the video encoded channel parameters
************************************************************************/
void dbg_output_venc_params(sx_uint8 board_index, dvr_encode_info_t *venc_params, sx_bool bOutput_fw)
{
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, venc_params->job_id);

    sprintf(dbg_buf,"\tstream_id = %d\n", venc_params->stream_id);
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tres_decimation = %s\n", dbg_res_decimation_text(venc_params->record_res));
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tframe_rate = %d\n", venc_params->record_frame_rate);
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tenable = %d\n", venc_params->enable);
    sdbg_output(dbg_buf, bOutput_fw);
 
    switch (enc_chan_info->sub_enc_info[venc_params->stream_id].venc_type)
    {
    case DVR_VC_FORMAT_NONE:
        break;

    case DVR_VC_FORMAT_H264:
        sprintf(dbg_buf,"\tH.264 parameters:\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tavg_bitrate = %d\n", venc_params->u1.h264.avg_bitrate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tmax_bitrate = %d\n", venc_params->u1.h264.max_bitrate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\trate_control = %s\n", dbg_bitrate_ctrl_text(venc_params->u1.h264.rate_control));
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tgop_size = %d\n", venc_params->u1.h264.gop_size);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tquality = %d\n", venc_params->u1.h264.quality);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_VC_FORMAT_MPEG4:
        sprintf(dbg_buf,"\tMPEG4 parameters:\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tavg_bitrate = %d\n", venc_params->u1.mpeg4.avg_bitrate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tmax_bitrate = %d\n", venc_params->u1.mpeg4.max_bitrate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\trate_control = %s\n", dbg_bitrate_ctrl_text(venc_params->u1.mpeg4.rate_control));
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tgop_size = %d\n", venc_params->u1.mpeg4.gop_size);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tquality = %d\n", venc_params->u1.mpeg4.quality);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_VC_FORMAT_JPEG:
        sprintf(dbg_buf,"\tMJPEG parameters:\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tquality = %d\n", venc_params->u1.jpeg.quality);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tis_image_style = %d\n", venc_params->u1.jpeg.jpeg_style);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    default:
        break;
    }
}

/************************************************************************
    Print the alarm  video encoded channel parameters
************************************************************************/
void dbg_output_venc_alarm_params(sx_uint8 board_index, dvr_enc_alarm_info_t *venc_params, sx_bool bOutput_fw)
{
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, venc_params->job_id);

    sprintf(dbg_buf,"\tstream_id = %d\n", venc_params->stream_id);
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tmin_off_seconds = %d\n", venc_params->min_off_seconds);
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tmin_on_seconds = %d\n", venc_params->min_on_seconds);
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tenable = %d\n", venc_params->enable);
    sdbg_output(dbg_buf, bOutput_fw);

 
    switch (enc_chan_info->sub_enc_info[venc_params->stream_id].venc_type)

    {
    case DVR_VC_FORMAT_NONE:
        break;

    case DVR_VC_FORMAT_H264:
        sprintf(dbg_buf,"\tH264 parameters:\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tframe_rate = %d\n", venc_params->record_frame_rate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\trate_control = %s\n", dbg_bitrate_ctrl_text(venc_params->u1.h264.rate_control));
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tavg_bitrate = %d\n", venc_params->u1.h264.avg_bitrate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tmax_bitrate = %d\n", venc_params->u1.h264.max_bitrate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tgop_size = %d\n", venc_params->u1.h264.gop_size);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tquality = %d\n", venc_params->u1.h264.quality);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_VC_FORMAT_JPEG:
        sprintf(dbg_buf,"\tMJPEG parameters:\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tframe_rate = %d\n", venc_params->record_frame_rate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tis_image_style = %d\n", venc_params->u1.jpeg.jpeg_style);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tquality = %d\n", venc_params->u1.jpeg.quality);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_VC_FORMAT_MPEG4:
        sprintf(dbg_buf,"\tMPEG4 parameters:\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tframe_rate = %d\n", venc_params->record_frame_rate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\trate_control = %s\n", dbg_bitrate_ctrl_text(venc_params->u1.mpeg4.rate_control));
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tavg_bitrate = %d\n", venc_params->u1.mpeg4.avg_bitrate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tmax_bitrate = %d\n", venc_params->u1.mpeg4.max_bitrate);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tgop_size = %d\n", venc_params->u1.mpeg4.gop_size);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\t\tquality = %d\n", venc_params->u1.mpeg4.quality);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    default:
        break;
    }
}

/************************************************************************
    Print the OSD parameters
************************************************************************/

 void dbg_output_osd(dvr_osd_t *cmd_strct)
 {
        char display_text[SDVR_MAX_OSD_TEXT + 1];

        sprintf(dbg_buf,"\tenable = %d\n", (cmd_strct->enable != 0));
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\tappend_dts = %d\n", (cmd_strct->format != DVR_OSD_DTS_NONE));
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\tdts_style = %s\n",dbg_osd_dts_style_text(cmd_strct->format));
        sdbg_output(dbg_buf, false);

        sprintf(dbg_buf,"\ttext_location = %s\n",dbg_osd_text_loc_text(cmd_strct->position));
        sdbg_output(dbg_buf, false);

        strncpy(display_text,cmd_strct->title,SDVR_MAX_OSD_TEXT);
        display_text[SDVR_MAX_OSD_TEXT] = '\0';
        sprintf(dbg_buf,"\tdisplay_text = %s\n",display_text);
        sdbg_output(dbg_buf, false);
 }

 void dbg_output_osd_config_ex(dvr_osd_ex_t *cmd_strct, sx_bool bOutput_fw)
 {
     sprintf(dbg_buf,"\tosd_id = %d\n",cmd_strct->u1.config.osd_id);
     sdbg_output(dbg_buf, bOutput_fw);

     switch (cmd_strct->op_code)
     {
     case DVR_OSD_OPCODE_CONFIG:
        sprintf(dbg_buf,"\tDVR_OSD_OPCODE_CONFIG\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\ttranslucent = %d\n",  cmd_strct->u1.config.translucent);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\ttext_location = %s\tdts_format = %s\n",dbg_osd_text_loc_text(cmd_strct->u1.config.position_ctrl),
            dbg_osd_dts_style_text(cmd_strct->u1.config.dts_format));
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tx_TL = %d\ty_TL = %d\n",
                    cmd_strct->u1.config.x_TL, cmd_strct->u1.config.y_TL);
        sdbg_output(dbg_buf, bOutput_fw);
        break;
     case DVR_OSD_OPCODE_SHOW:
        sprintf(dbg_buf,"\tDVR_OSD_OPCODE_SHOW\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tShow-State = %d\n", cmd_strct->u1.show.state);
        sdbg_output(dbg_buf, bOutput_fw);

         break;
     case DVR_OSD_OPCODE_TEXT:
        sprintf(dbg_buf,"\tDVR_OSD_OPCODE_TEXT\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tText Length = %d\n", cmd_strct->u1.text.length);
        sdbg_output(dbg_buf, bOutput_fw);

         break;
     default:
         break;
     }
 }

void dbg_output_smo_params(dvr_smo_t *cmd_strct, sx_bool bOutput_fw)
{
        sprintf(dbg_buf,"\tport_num = %d\n", cmd_strct->port_num);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\ttop_left_mb_x = %d\n", cmd_strct->position_x);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\ttop_left_mb_y = %d\n", cmd_strct->position_y);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tres_decimation = %s\n", dbg_res_decimation_text(cmd_strct->output_res));
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tdwell_time = %d\n", cmd_strct->seconds);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tenable = %d\n", cmd_strct->enable);
        sdbg_output(dbg_buf, bOutput_fw);
/*
        sprintf(dbg_buf,"\tlayer = %d\n", cmd_strct->layer);
        sdbg_output(dbg_buf, bOutput_fw);
        
        sprintf(dbg_buf,"\tresize_width = %d\n", cmd_strct->resize_width);
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tresize_height = %d\n", cmd_strct->resize_height);
        sdbg_output(dbg_buf, bOutput_fw);
*/
}

void dbg_output_ioctl_params(dvr_ioctl_t *cmd_strct,sx_bool bOutput_fw)
{

    switch (cmd_strct->ioctl_code)
    {
    case DVR_IOCTL_CODE_IMG_CONTROL:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_IMG_CONTROL\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tflags = %x\n\thue = %d saturation = %d contrast = %d sharpness = %d\n",
            cmd_strct->u1.img_ctrl.flags,cmd_strct->u1.img_ctrl.hue, 
            cmd_strct->u1.img_ctrl.sat, cmd_strct->u1.img_ctrl.cont, 
            cmd_strct->u1.img_ctrl.sat);
        sdbg_output(dbg_buf, bOutput_fw);
        break;

    case DVR_IOCTL_CODE_DECODER_REGS:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_DECODER_REGS\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tdevice_id = %d reg_num = %d val = %x\n",
            cmd_strct->u1.reg.device_id,
            cmd_strct->u1.reg.reg_num, 
            cmd_strct->u1.reg.val);
        sdbg_output(dbg_buf, bOutput_fw);
        break;

    case DVR_IOCTL_CODE_OPEN_CHAN:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_OPEN_CHAN\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tport = %d\n",
            cmd_strct->u1.chan.port);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_IOCTL_CODE_CLOSE_CHAN:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_CLOSE_CHAN\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tport = %d\n",
            cmd_strct->u1.chan.port);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_IOCTL_CODE_MOTION_FRAME_FREQ:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_MOTION_FRAME_FREQ\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tfrequency = %d\n",
            cmd_strct->u1.motion.frequency);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_IOCTL_CODE_VPP_CONTROL:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_VPP_CONTROL\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tactions = %d\n",
            cmd_strct->u1.vpp_ctrl.actions);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_IOCTL_CODE_GAIN_MODE:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_GAIN_MODE\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tvalue = %d\n",
            cmd_strct->u1.gain_mode.value);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    case DVR_IOCTL_CODE_TERMINATION:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_TERMINATION\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tvalue = %d\n",
            cmd_strct->u1.termination.value);
        sdbg_output(dbg_buf, bOutput_fw);

        break;


    case DVR_IOCTL_CODE_LED:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_LED\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\ttype = %d\tenable = %d\n",
            cmd_strct->u1.led.type,cmd_strct->u1.led.enable);
        sdbg_output(dbg_buf, bOutput_fw);

        break;


    case DVR_IOCTL_CODE_RAWV_FORMAT:
        sprintf(dbg_buf,"\tioctl_code = DVR_IOCTL_CODE_RAWV_FORMAT\n");
        sdbg_output(dbg_buf, bOutput_fw);

        sprintf(dbg_buf,"\tformat= %d\n",
            cmd_strct->u1.rawv_format.format);
        sdbg_output(dbg_buf, bOutput_fw);

        break;

    }
}

void dbg_output_board_setting(dvr_board_t *cmd_strct, sx_bool bOutput_fw)
{
    sprintf(dbg_buf,"\tvideo_std = %s\n",dbg_camera_type_name(cmd_strct->camera_type));
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\tis_skip_h64_sce = %d\n",cmd_strct->u1.h264_skip_SCE);
    sdbg_output(dbg_buf, bOutput_fw);

    sprintf(dbg_buf,"\taudio_rate = %s\n",dbg_audio_rate_to_string(cmd_strct->audio_rate));
    sdbg_output(dbg_buf, bOutput_fw);
}

void dbg_dump_frame_count()
{
    sx_uint8 board_index = 0;
    sx_uint8 chan_num;

    sdbg_output("\n**** Debug Frame Dump ********\n", false);

    for (chan_num = 0; chan_num < _ssdvr_boards_info.boards[board_index].max_num_video_in; chan_num++)
    {
        if (_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].pci_chan_handle)
        {
            sprintf(dbg_buf,">>> Encoder Channle = %d, board ID = %d <<<\n",chan_num, board_index);
            sdbg_output(dbg_buf, false);
            sprintf(dbg_buf,"\tSDK - y_u_vBufferRcvd = %d\n",
                _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.y_u_vBufferRcvd);
            sdbg_output(dbg_buf, false);
            sprintf(dbg_buf,"\ty_u_vBuffersFreed = %d  Diff y_u_v Buffer Not Freed ===> %d <===\n----------------------------------\n",
                _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.y_u_vBuffersFreed,
                (_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.y_u_vBufferRcvd ) -
                (_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.y_u_vBuffersFreed +
                _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.yuvMissedCount));

            sdbg_output(dbg_buf, false);

            sprintf(dbg_buf,"\tyuvFrameRcvd = %d  yuvFrameFreed = %d\n----------------------------------\n",
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.yuvFrameRcvd,
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.yuvFrameFreed);
            sdbg_output(dbg_buf, false);

            sprintf(dbg_buf,"\tSDK - yuvMissedCount = %d\n----------------------------------\n",
                _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.yuvMissedCount);
            sdbg_output(dbg_buf, false);


            sprintf(dbg_buf,"\tyuvFrameAppRcvd = %d  yuvFrameAppFreed = %d\n----------------------------------\n",
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.yuvFrameAppRcvd,
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.yuvFrameAppFreed);
            sdbg_output(dbg_buf, false);

            sprintf(dbg_buf,"\teFrameAppRcvd = %d  eFrameAppFreed = %d\n----------------------------------\n",
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.eFrameAppRcvd,
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.eFrameAppFreed);
            sdbg_output(dbg_buf, false);

        
            sprintf(dbg_buf,"\teFrameRcvd = %d  eFrameFreed = %d\n----------------------------------\n",
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.eFrameRcvd,
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.eFrameFreed);
            sdbg_output(dbg_buf, false);

            sprintf(dbg_buf,"\ttotalFramesRcvd = %d  totalFramesFreed = %d   Diff ===> %d <===\n----------------------------------\n",
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.totalFramesRcvd,
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.totalFramesFreed,
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.totalFramesRcvd -
            _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.totalFramesFreed);
            sdbg_output(dbg_buf, false);
        }
    }
}

char *dbg_osd_dts_style_text(dvr_osd_dts_e format)
{
    switch (format) {
    case DVR_OSD_DTS_NONE:      return "SDVR_OSD_DTS_NONE";
    case DVR_OSD_DTS_MDY_12H:   return "SDVR_OSD_DTS_MDY_12H";
    case DVR_OSD_DTS_DMY_12H:   return "SDVR_OSD_DTS_DMY_12H";
    case DVR_OSD_DTS_YMD_12H:   return "SDVR_OSD_DTS_YMD_12H";
    case DVR_OSD_DTS_MDY_24H:   return "SDVR_OSD_DTS_MDY_242H";
    case DVR_OSD_DTS_DMY_24H:   return "SDVR_OSD_DTS_DMY_24H";
    case DVR_OSD_DTS_YMD_24H:   return "SDVR_OSD_DTS_YMD_24H";
    default:                    return "unknown OSD Style";
    }
}
char *dbg_osd_text_loc_text(dvr_osd_pos_e position)
{
    switch (position) {
    case DVR_OSD_POS_TL:        return "SDVR_LOC_TOP_LEFT";
    case DVR_OSD_POS_BL:        return "SDVR_LOC_BOTTOM_LEFT";
    case DVR_OSD_POS_TR:        return "SDVR_LOC_TOP_RIGHT";
    case DVR_OSD_POS_BR:        return "SDVR_LOC_BOTTOM_RIGHT";
    case DVR_OSD_POS_CUSTOM:    return "SDVR_LOC_CUSTOM";
    default:                    return "unknown OSD Text Location";
    }
}
char *dbg_bitrate_ctrl_text(sdvr_br_control_e bitrate_ctrl)
{
    switch (bitrate_ctrl) {
    case SDVR_BITRATE_CONTROL_VBR:              return "BITRATE_CONTROL_VBR";
    case SDVR_BITRATE_CONTROL_CBR:              return "BITRATE_CONTROL_CBR";
    case SDVR_BITRATE_CONTROL_CQP:              return "BITRATE_CONTROL_CONSTANT_CQP";
    case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY: return "BITRATE_CONTROL_CONSTANT_QUALITY";
    case SDVR_BITRATE_CONTROL_NONE:
    default:
        return "BITRATE_CONTROL_NONE";
    }
}
char *dbg_chan_type_text(sdvr_chan_type_e chan_type)
{
    switch (chan_type) 
    {
        case SDVR_CHAN_TYPE_DECODER:    return ("Decoder");
        case SDVR_CHAN_TYPE_ENCODER:    return ("Encoder");
        default:                        return ("Unknown");
    }
}
char *dbg_frame_type_text(sdvr_frame_type_e frame_type)
{
    switch (frame_type) 
    {
        case SDVR_FRAME_RAW_AUDIO:      return("SDVR_FRAME_RAW_AUDIO");
        case SDVR_FRAME_H264_IDR:       return("SDVR_FRAME_H264_IDR");
        case SDVR_FRAME_H264_I:         return("SDVR_FRAME_H264_I");
        case SDVR_FRAME_H264_P:         return("SDVR_FRAME_H264_P");
        case SDVR_FRAME_H264_B:         return("SDVR_FRAME_H264_B");
        case SDVR_FRAME_H264_SPS:       return("SDVR_FRAME_H264_SPS");
        case SDVR_FRAME_H264_PPS:       return("SDVR_FRAME_H264_PPS");
        case SDVR_FRAME_JPEG:           return("SDVR_FRAME_JPEG");
        case SDVR_FRAME_G711:           return("SDVR_FRAME_G711");
        case SDVR_FRAME_MPEG4_I:        return("SDVR_FRAME_MPEG4_I");
        case SDVR_FRAME_MPEG4_P:        return("SDVR_FRAME_MPEG4_P");
        case SDVR_FRAME_MPEG4_B:        return("SDVR_FRAME_MPEG4_B");
        case SDVR_FRAME_MPEG4_VOL:      return("SDVR_FRAME_MPEG4_VOL");
        case SDVR_FRAME_RAW_VIDEO:      return("SDVR_FRAME_RAW_VIDEO");
        case SDVR_FRAME_VIDEO_ENCODED_PRIMARY:   return("SDVR_FRAME_VIDEO_ENCODED_PRIMARY");
        case SDVR_FRAME_VIDEO_ENCODED_SECONDARY: return("SDVR_FRAME_VIDEO_ENCODED_SECONDARY");
        case SDVR_FRAME_AUDIO_ENCODED:      return("SDVR_FRAME_AUDIO_ENCODED");
        default:                        return ("Unknown");
    }
} 

/************************************************************************
    Returns message name
************************************************************************/
char *dbg_message_name(dvr_message_e message)
{
    static char name[50];

    switch (message) {
    case DVR_GET_VER_INFO:      return "DVR_GET_VER_INFO";                               
    case DVR_REP_VER_INFO:      return "DVR_REP_VER_INFO";
    case DVR_GET_CODEC_INFO:    return "DVR_GET_CODEC_INFO";                             
    case DVR_REP_CODEC_INFO:    return "DVR_REP_CODEC_INFO";
    case DVR_GET_BOARD:         return "DVR_GET_BOARD";          
    case DVR_SET_BOARD:         return "DVR_SET_BOARD";          
    case DVR_REP_BOARD:         return "DVR_REP_BOARD";
    case DVR_GET_JOB:           return "DVR_GET_JOB";          
    case DVR_SET_JOB:           return "DVR_SET_JOB";          
    case DVR_REP_JOB:           return "DVR_REP_JOB";
    case DVR_GET_SMO:           return "DVR_GET_SMO";          
    case DVR_SET_SMO:           return "DVR_SET_SMO";          
    case DVR_REP_SMO:           return "DVR_REP_SMO";
    case DVR_GET_HMO:           return "DVR_GET_HMO";          
    case DVR_SET_HMO:           return "DVR_SET_HMO";          
    case DVR_REP_HMO:           return "DVR_REP_HMO";
    case DVR_GET_OSD:           return "DVR_GET_OSD";          
    case DVR_SET_OSD:           return "DVR_SET_OSD";          
    case DVR_REP_OSD:           return "DVR_REP_OSD";
    case DVR_GET_PR:            return "DVR_GET_PR";          
    case DVR_SET_PR:            return "DVR_SET_PR";          
    case DVR_REP_PR:            return "DVR_REP_PR";
    case DVR_GET_MD:            return "DVR_GET_MD";          
    case DVR_SET_MD:            return "DVR_SET_MD";          
    case DVR_REP_MD:            return "DVR_REP_MD";
    case DVR_GET_BD:            return "DVR_GET_BD";          
    case DVR_SET_BD:            return "DVR_SET_BD";          
    case DVR_REP_BD:            return "DVR_REP_BD";
    case DVR_GET_ND:            return "DVR_GET_ND";          
    case DVR_SET_ND:            return "DVR_SET_ND";          
    case DVR_REP_ND:            return "DVR_REP_ND";
    case DVR_GET_RELAYS:        return "DVR_GET_RELAYS";         
    case DVR_SET_RELAYS:        return "DVR_SET_RELAYS";         
    case DVR_REP_RELAYS:        return "DVR_REP_RELAYS";
    case DVR_GET_SENSORS:        return "DVR_GET_SENSORS";         
    case DVR_SET_SENSORS:        return "DVR_SET_SENSORS";         
    case DVR_REP_SENSORS:        return "DVR_REP_SENSORS";
    case DVR_GET_WATCHDOG:      return "DVR_GET_WATCHDOG";       
    case DVR_SET_WATCHDOG:      return "DVR_SET_WATCHDOG";       
    case DVR_REP_WATCHDOG:      return "DVR_REP_WATCHDOG";
    case DVR_GET_CONTROL:       return "DVR_GET_CONTROL";        
    case DVR_SET_CONTROL:       return "DVR_SET_CONTROL";        
    case DVR_REP_CONTROL:       return "DVR_REP_CONTROL";
    case DVR_GET_ENCODE:        return "DVR_GET_ENCODE";    
    case DVR_SET_ENCODE:        return "DVR_SET_ENCODE";    
    case DVR_REP_ENCODE:        return "DVR_REP_ENCODE";
    case DVR_GET_ENC_ALARM:     return "DVR_GET_ENC_ALARM";
    case DVR_SET_ENC_ALARM:     return "DVR_SET_ENC_ALARM";
    case DVR_REP_ENC_ALARM:     return "DVR_REP_ENC_ALARM";
    case DVR_GET_DECODE:        return "DVR_GET_DECODE";
    case DVR_SET_DECODE:        return "DVR_SET_DECODE";
    case DVR_REP_DECODE:        return "DVR_REP_DECODE";
    case DVR_GET_TIME:			return "DVR_GET_TIME";
    case DVR_SET_TIME:			return "DVR_SET_TIME";
    case DVR_REP_TIME:			return "DVR_RET_TIME";

    case DVR_SET_FONT_TABLE:    return "DVR_SET_FONT_TABLE";
    case DVR_REP_FONT_TABLE:    return "DVR_REP_FONT_TABLE";

    case DVR_GET_OSD_EX:        return "DVR_GET_OSD_EX";
    case DVR_SET_OSD_EX:        return "DVR_SET_OSD_EX";
    case DVR_REP_OSD_EX:        return "DVR_REP_OSD_EX";

    case DVR_GET_UART:          return "DVR_GET_UART";
    case DVR_SET_UART:          return "DVR_SET_UART";
    case DVR_REP_UART:          return "DVR_REP_UART";
        
    case DVR_GET_IOCTL:         return "DVR_GET_IOCTL";
    case DVR_SET_IOCTL:         return "DVR_SET_IOCTL";
    case DVR_REP_IOCTL:         return "DVR_REP_IOCTL";

    case DVR_GET_AUTH_KEY:      return "DVR_GET_AUTH_KEY";
    case DVR_SET_AUTH_KEY:      return "DVR_SET_AUTH_KEY";
    case DVR_REP_AUTH_KEY:      return "DVR_REP_AUTH_KEY";

    case DVR_SET_REGIONS_MAP:   return "DVR_SET_REGIONS_MAP";
    case DVR_REP_REGIONS_MAP:   return "DVR_REP_REGIONS_MAP";

    case DVR_SET_VIDEO_OUTPUT:  return "DVR_SET_VIDEO_OUTPUT";
    case DVR_REP_VIDEO_OUTPUT:  return "DVR_REP_VIDEO_OUTPUT";

    case DVR_GET_SMO_ATTRIB:    return "DVR_GET_SMO_ATTRIB";
    case DVR_REP_SMO_ATTRIB:    return "DVR_REP_SMO_ATTRIB";

    case DVR_SIG_HOST:          return "DVR_SIG_HOST";
    case DVR_FW_INTERNAL:       return "DVR_FW_INTERNAL";
    }

    sprintf(name,"Unknown Message [%d]",message);
    return name;
}


/**************************************************
    Returns chip revision
**************************************************/
char *dbg_chip_rev_text(dvr_chip_rev_e chip_rev)
{
    switch (chip_rev)
    {
    case SDVR_CHIP_S6100_3_REV_C:   return "SDVR_CHIP_S6100_3_REV_C";
    case SDVR_CHIP_S6105_3_REV_C:   return "SDVR_CHIP_S6105_3_REV_C";
    case SDVR_CHIP_S6106_3_REV_C:   return "SDVR_CHIP_S6106_3_REV_C";
    case SDVR_CHIP_S6100_3_REV_D:   return "SDVR_CHIP_S6100_3_REV_D";
    case SDVR_CHIP_S6105_3_REV_D:   return "SDVR_CHIP_S6105_3_REV_D";
    case SDVR_CHIP_S6106_3_REV_D:   return "SDVR_CHIP_S6106_3_REV_D";
    case SDVR_CHIP_S6100_3_REV_F:   return "SDVR_CHIP_S6100_3_REV_F";
    case SDVR_CHIP_S6105_3_REV_F:   return "SDVR_CHIP_S6105_3_REV_F";
    case SDVR_CHIP_S6106_3_REV_F:   return "SDVR_CHIP_S6106_3_REV_F";
    case SDVR_CHIP_S6100_3_UNKNOWN: return "SDVR_CHIP_S6100_3_UNKNOWN";
    case SDVR_CHIP_S6105_3_UNKNOWN: return "SDVR_CHIP_S6105_3_UNKNOWN";
    case SDVR_CHIP_S6106_3_UNKNOWN: return "SDVR_CHIP_S6106_3_UNKNOWN";
    default:            return "unknown chip revision";

    }
}

/************************************************************************
    Returns job type name.  Cannot be freed.
************************************************************************/
char *dbg_job_type_name(dvr_job_type_e job_type)
{
    switch (job_type) {
    case DVR_JOB_CAMERA_ENCODE: return "camera encode";
    case DVR_JOB_HOST_ENCODE:   return "host encode";
    case DVR_JOB_HOST_DECODE:   return "host decode";
    case DVR_JOB_VIDEO_OUTPUT:  return "video output";
    default:                    return "unknown job type";
    }
}

/************************************************************************
    Returns job type name.  Cannot be freed.
************************************************************************/
dvr_job_type_e dbg_job_type_by_name(char *job_type)
{
    if (strcmp(job_type, "camera_encode") == 0) {
        return DVR_JOB_CAMERA_ENCODE;
    } else if (strcmp(job_type, "host_encode") == 0) {
        return DVR_JOB_HOST_ENCODE;
    } else if (strcmp(job_type, "host_decode") == 0) {
        return DVR_JOB_HOST_DECODE;
    } else {
//        dbg_FAIL("Fatal error: unknown job_type\n");
        return 0;
    }
}

sdvr_chan_type_e dbg_chan_type_by_name(char *chan_type)
{
    if (strcmp(chan_type, "encoder") == 0)
        return SDVR_CHAN_TYPE_ENCODER;
    else if (strcmp(chan_type, "decoder") == 0)
        return SDVR_CHAN_TYPE_DECODER;
    else    // wrong type;
        return SDVR_CHAN_TYPE_NONE;
}

sdvr_venc_e dbg_vformat_by_name(char *vformat_name)
{
    if (strcmp(vformat_name, "h264") == 0)
        return SDVR_VIDEO_ENC_H264;
    else if (strcmp(vformat_name, "jpeg") == 0)
        return SDVR_VIDEO_ENC_JPEG;
    else if (strcmp(vformat_name, "mpeg4") == 0)
        return SDVR_VIDEO_ENC_MPEG4;
    else    // wrong type;
        return SDVR_VIDEO_ENC_NONE;
}
sdvr_aenc_e dbg_aformat_by_name(char *aformat_name)
{
    if (strcmp(aformat_name, "g711") == 0)
        return SDVR_AUDIO_ENC_G711;
    else if (strcmp(aformat_name, "g726_16k") == 0)
        return SDVR_AUDIO_ENC_G726_16K;
    else if (strcmp(aformat_name, "g726_24k") == 0)
        return SDVR_AUDIO_ENC_G726_24K;
    else if (strcmp(aformat_name, "g726_32k") == 0)
        return SDVR_AUDIO_ENC_G726_32K;
    else if (strcmp(aformat_name, "g726_48k") == 0)
        return SDVR_AUDIO_ENC_G726_48K;
    else    // wrong type;
        return SDVR_AUDIO_ENC_NONE;
}

sdvr_br_control_e dbg_rate_ctrl_by_name(char *rate_ctrl_name)
{
    if (strcmp(rate_ctrl_name, "cbr") == 0)
        return SDVR_BITRATE_CONTROL_CBR;
    else if (strcmp(rate_ctrl_name, "vbr") == 0)
        return SDVR_BITRATE_CONTROL_VBR;
    else if (strcmp(rate_ctrl_name, "constant") == 0)
        return SDVR_BITRATE_CONTROL_CONSTANT_QUALITY;
    else    
        return SDVR_BITRATE_CONTROL_NONE;
}

sdvr_video_res_decimation_e dbg_res_decimation_by_name(char *res_des_name)
{
    if (strcmp(res_des_name, "equal") == 0)
        return SDVR_VIDEO_RES_DECIMATION_EQUAL;
    else if (strcmp(res_des_name, "fourth") == 0)
        return SDVR_VIDEO_RES_DECIMATION_FOURTH;
    else if (strcmp(res_des_name, "sixteenth") == 0)
        return SDVR_VIDEO_RES_DECIMATION_SIXTEENTH;
    else    
        return SDVR_VIDEO_RES_DECIMATION_NONE;
}

 char *dbg_res_decimation_text(sdvr_video_res_decimation_e res_decimation)
{
    static char buf[256];

    switch (res_decimation)
    {

    case SDVR_VIDEO_RES_DECIMATION_NONE:
        return "SDVR_VIDEO_RES_DECIMATION_NONE";

    case SDVR_VIDEO_RES_DECIMATION_FOURTH:
        return "SDVR_VIDEO_RES_DECIMATION_FOURTH";

    case SDVR_VIDEO_RES_DECIMATION_SIXTEENTH:
        return "SDVR_VIDEO_RES_DECIMATION_SIXTEENTH";

    case SDVR_VIDEO_RES_DECIMATION_EQUAL:
        return "SDVR_VIDEO_RES_DECIMATION_EQUAL";

    case SDVR_VIDEO_RES_DECIMATION_HALF:
        return "SDVR_VIDEO_RES_DECIMATION_HALF";

    case SDVR_VIDEO_RES_DECIMATION_DCIF:
        return "SDVR_VIDEO_RES_DECIMATION_DCIF";

    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF:
        return "SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF";

    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF:
        return "SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF";

    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF:
        return "SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF";

    default:
        sprintf(buf,"unknown resolution decimation [%d]",res_decimation);
        return buf;
    }
}

/************************************************************************
    Returns video format name.  Cannot be freed.
************************************************************************/
char *dbg_video_format_name(dvr_vc_format_e video_format)
{
    switch (video_format) {
    case DVR_VC_FORMAT_NONE:    return "none";
    case DVR_VC_FORMAT_H264:    return "h264";
    case DVR_VC_FORMAT_JPEG:    return "jpeg";
    case DVR_VC_FORMAT_MPEG4:    return "mpeg4";
    default:                    return "unknown video format";
    }
}

/************************************************************************
    Returns audio format name.  Cannot be freed.
************************************************************************/
char *dbg_audio_format_name(dvr_ac_format_e audio_format)
{
    switch (audio_format) {
    case DVR_AC_FORMAT_NONE:        return "none";
    case DVR_AC_FORMAT_G711:        return "g711";
    case DVR_AC_FORMAT_G726_16K:    return "g726-16k";
    case DVR_AC_FORMAT_G726_24K:    return "g726-24k";
    case DVR_AC_FORMAT_G726_32K:    return "g726-32k";
    case DVR_AC_FORMAT_G726_48K:    return "g726-48k";
    default:                        return "unknown audio format";
    }
}

/************************************************************************
    Returns job control name.  Cannot be freed.
************************************************************************/
char *dbg_job_control_name(dvr_job_action_e control)
{
    switch (control) {
    case DVR_JOB_CREATE:    return "create";
    case DVR_JOB_DESTROY:   return "destroy";
    case DVR_JOB_RESET:     return "reset";
    default:                return "unknown job action";
    }
}

char *dbg_vpp_mode_name(dvr_vpp_mode_e vpp_mode)
{
    switch (vpp_mode) {
    case DVR_VPP_MODE_ANALYTICS:    return "alalytics";
    case DVR_VPP_MODE_SLATERAL:   return "slateral";
    default:                return "unknown VPP mode";
    }

}

/************************************************************************
    Video format name
************************************************************************/
dvr_vc_format_e dbg_video_format_by_name(char *video_format)
{
    if (strcmp(video_format, "h264") == 0) {
        return DVR_VC_FORMAT_H264;
    } else if (strcmp(video_format, "jpeg") == 0) {
        return DVR_VC_FORMAT_JPEG;
    } else if (strcmp(video_format, "mpeg4") == 0) {
        return DVR_VC_FORMAT_MPEG4;
    } else {
        return DVR_VC_FORMAT_NONE;
    }
}

/************************************************************************
    Audio format name
************************************************************************/
dvr_ac_format_e dbg_audio_format_by_name(char *audio_format)
{
    if (strcmp(audio_format, "g711") == 0) {
        return DVR_AC_FORMAT_G711;
    } else if (strcmp(audio_format, "g726-16k") == 0) {
        return DVR_AC_FORMAT_G726_16K;
    } else if (strcmp(audio_format, "g726-24k") == 0) {
        return DVR_AC_FORMAT_G726_24K;
    } else if (strcmp(audio_format, "g726-32k") == 0) {
        return DVR_AC_FORMAT_G726_32K;
    } else if (strcmp(audio_format, "g726-48k") == 0) {
        return DVR_AC_FORMAT_G726_48K;
    } else {
        return DVR_AC_FORMAT_NONE;
    }
}

/************************************************************************
    Audio rate name
************************************************************************/
char *dbg_audio_rate_to_string(sx_uint16 audio_rate)
{
    switch(audio_rate)
    {
        case DVR_AUDIO_RATE_INVALID: return "DVR_AUDIO_RATE_INVALID";
        case DVR_AUDIO_RATE_8KHZ:    return "DVR_AUDIO_RATE_8KHZ";
        case DVR_AUDIO_RATE_16KHZ:   return "DVR_AUDIO_RATE_16KHZ";
        case DVR_AUDIO_RATE_32KHZ:   return "DVR_AUDIO_RATE_32KHZ";
        default:                     return "unknown audio rate";
    }
}

/************************************************************************
    Returns job control name.  Cannot be freed.
************************************************************************/
char *dbg_camera_type_name(sx_uint16 camera_type)
{
    if (camera_type & DVR_VSTD_D1_PAL) {
        return "d1-pal";
    } else if (camera_type & DVR_VSTD_D1_NTSC) {
        return "d1-ntsc";
    } else if (camera_type & DVR_VSTD_CIF_PAL) {
        return "cif-pal";
    } else if (camera_type & DVR_VSTD_CIF_NTSC) {
        return "cif-ntsc";
    } else if (camera_type & DVR_VSTD_2CIF_PAL) {
        return "2cif-pal";
    } else if (camera_type & DVR_VSTD_2CIF_NTSC) {
        return "2cif-ntsc";
    } else if (camera_type & DVR_VSTD_4CIF_PAL) {
        return "4cif-pal";
    } else if (camera_type & DVR_VSTD_4CIF_NTSC) {
        return "4cif-ntsc";
    } else {
        return "unknown camera type";
    }
}

/************************************************************************
    Camera format name
************************************************************************/
sx_uint16 dbg_camera_type_by_name(char *camera_type)
{
    if (strcmp(camera_type, "d1-pal") == 0) {
        return DVR_VSTD_D1_PAL;
    } else if (strcmp(camera_type, "d1-ntsc") == 0) {
        return DVR_VSTD_D1_NTSC;
    } else if (strcmp(camera_type, "cif-pal") == 0) {
        return DVR_VSTD_CIF_PAL;
    } else if (strcmp(camera_type, "cif-ntsc") == 0) {
        return DVR_VSTD_CIF_NTSC;
    } else if (strcmp(camera_type, "2cif-pal") == 0) {
        return DVR_VSTD_2CIF_PAL;
    } else if (strcmp(camera_type, "2cif-ntsc") == 0) {
        return DVR_VSTD_2CIF_NTSC;
    } else if (strcmp(camera_type, "4cif-pal") == 0) {
        return DVR_VSTD_4CIF_PAL;
    } else if (strcmp(camera_type, "4cif-ntsc") == 0) {
        return DVR_VSTD_4CIF_NTSC;
    } else {
        return DVR_VSTD_UNKNOWN;
    }
}

/************************************************************************
    Job action name
************************************************************************/
dvr_job_action_e dbg_job_action_by_name(char *action)
{
    if (strcmp(action, "create") == 0) {
        return DVR_JOB_CREATE;
    } else if (strcmp(action, "destroy") == 0) {
        return DVR_JOB_DESTROY;
    } else if (strcmp(action, "reset") == 0) {
        return DVR_JOB_RESET;
    } else {
  //      dbg_FAIL("Error: unknown job action\n");
        return DVR_JOB_RESET;
    }
}

/************************************************************************
    signal name
************************************************************************/

char *dbg_signal_text(dvr_signal_type_e sig_type)
{
    static  char strPrompt[50];

    switch (sig_type) 
    {
        case DVR_SIGNAL_SENSOR_ACTIVATED:   return ("DVR_SIGNAL_SENSOR_ACTIVATED");
        case DVR_SIGNAL_MOTION_DETECTED:    return ("DVR_SIGNAL_MOTION_DETECTED");
        case DVR_SIGNAL_BLIND_DETECTED:     return ("DVR_SIGNAL_BLIND_DETECTED");
        case DVR_SIGNAL_NIGHT_DETECTED:     return ("DVR_SIGNAL_NIGHT_DETECTED");
        case DVR_SIGNAL_VIDEO_LOST:         return ("DVR_SIGNAL_VIDEO_LOST");
        case DVR_SIGNAL_VIDEO_DETECTED:     return ("DVR_SIGNAL_VIDEO_DETECTED");
        case DVR_SIGNAL_RUNTIME_ERROR:      return ("DVR_SIGNAL_RUNTIME_ERROR");
        case DVR_SIGNAL_FATAL_ERROR:        return ("DVR_SIGNAL_FATAL_ERROR");
        case DVR_SIGNAL_HEARTBEAT:          return ("DVR_SIGNAL_HEARTBEAT");
            break;
        default: 
            sprintf(strPrompt,"Unknown Signal [%d]",sig_type);
            return (strPrompt);
    }
}

char *dbg_vpp_opcode_to_str(sx_uint8 opcode)
{
    static  char strPrompt[50];

    switch (opcode)
    {
    case DVR_VPP_OPCODE_CONTROL:    return ("DVR_VPP_OPCODE_CONTROL");
    case DVR_VPP_OPCODE_REGIONS:    return ("DVR_VPP_OPCODE_REGIONS");
    default:
            sprintf(strPrompt,"Unknown opcode [%d]",opcode);
            return (strPrompt);
    }
}

sx_bool dbg_is_debug(unsigned int msg)
{
    sx_bool ret = 0;
    sx_uint32 debug_flag = _ssdk_get_debug_flag();

    // First check if the debugging is on 
    if (!IS_DBG_ON) 
        return ret;

    // If the debugging is on, check to see if it is
    // on for the class of msg.
    if (msg >= start_always_on_func && msg < end_always_on_func) 
    {
        ret = 1; // Debugging is always on for this class of functions
    }
    else if (msg >= start_sdvr_sdk_func && msg < end_sdvr_sdk_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_GENERAL_SDK) == DEBUG_FLAG_GENERAL_SDK;
    } 
    else if (msg >= start_sdvr_alarm_func && msg < end_sdvr_alarm_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_VIDEO_ALARM) == DEBUG_FLAG_VIDEO_ALARM;
    } 
    else if (msg >= start_sdvr_chan_func && msg < end_sdvr_chan_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_CHANNEL) == DEBUG_FLAG_CHANNEL;
    } 
    else if (msg >= start_sdvr_enc_func && msg < end_sdvr_enc_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_ENCODER) == DEBUG_FLAG_ENCODER;
    } 
    else if (msg >= start_sdvr_disp_func && msg < end_sdvr_disp_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_DISPLAY_OPERATIONS) == DEBUG_FLAG_DISPLAY_OPERATIONS;
    } 
    else if (msg >= start_sdvr_audio_func && msg < end_sdvr_audio_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_AUDIO_OPERATIONS) == DEBUG_FLAG_AUDIO_OPERATIONS;
    } 
    else if (msg >= start_sdvr_osd_func && msg < end_sdvr_osd_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_OSD) == DEBUG_FLAG_OSD;
    } 
    else if (msg >= start_sdvr_smo_func && msg < end_sdvr_smo_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_SMO) == DEBUG_FLAG_SMO;
    }
    else if (msg >= start_sdvr_video_frame_func && msg < end_sdvr_video_frame_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_VIDEO_FRAME) == DEBUG_FLAG_VIDEO_FRAME;
    }
    else if (msg >= start_sdvr_sensor_relay_func && msg < end_sdvr_sensor_relay_func) 
    {
        ret = (debug_flag & DEBUG_FLAG_SENSORS_RELAYS) == DEBUG_FLAG_SENSORS_RELAYS;
    }
    else if (msg >= start_sdvr_record_file && msg < end_sdvr_record_file) 
    {
        ret = (debug_flag & DEBUG_FLAG_RECORD_TO_FILE) == DEBUG_FLAG_RECORD_TO_FILE;
    }
    
    return ret;
}


char *dbg_region_type_text(sx_uint8 region_type)
{
    static  char strPrompt[50];

    switch (region_type)
    {
    case DVR_REGION_TYPE_MD:    return "DVR_REGION_TYPE_MD";
    case DVR_REGION_TYPE_BD:    return "DVR_REGION_TYPE_BD";
    case DVR_REGION_TYPE_ND:    return "DVR_REGION_TYPE_ND";
    case DVR_REGION_TYPE_PR:    return "DVR_REGION_TYPE_PR";
    default:
            sprintf(strPrompt,"Unknown region_type [%d]",region_type);
            return (strPrompt);
    }
}


char *dbg_led_type_text(sx_uint8 led_type)
{
    switch (led_type)
    {
        case DVR_LED_TYPE_RECORD: return "DVR_LED_TYPE_RECORD";
        case DVR_LED_TYPE_ALARM:  return "DVR_LED_TYPE_ALARM";
        default:                  return "unknown";
    }
}


