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

#include "sdvr_sdk_common.h"

sx_bool _is_SMO_supported_resolution(sdvr_video_res_decimation_e res_decimation)
{
    sx_bool bRet = false;

    switch (res_decimation)
    {
    case SDVR_VIDEO_RES_DECIMATION_FOURTH: /*or  SDVR_VIDEO_RES_DECIMATION_CIF*/
    case SDVR_VIDEO_RES_DECIMATION_SIXTEENTH: /* or SDVR_VIDEO_RES_DECIMATION_QCIF*/
    case SDVR_VIDEO_RES_DECIMATION_EQUAL:
        bRet = true;
        break;
    default:
        bRet = false;
    }
    return bRet;
}

static sdvr_err_e _set_smo_port_grid(sdvr_chan_handle_t  handle,
                              sx_uint8 smo_port_num,
                             sdvr_smo_grid_t *smo_grid)
{
    sdvr_err_e err;
    dvr_smo_t cmd_strct;

    dvr_control_t cntrl_strct;
    sx_uint8 board_index = sdvr_get_board_index(handle);
    int cmd_strct_size = sizeof(cmd_strct);
    sx_bool is_smo_enable;

    if (board_index == 0xFF)
        err = SDVR_ERR_INVALID_CHANNEL;
    else if (smo_grid == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (!smo_port_num || 
        smo_port_num > _ssdvr_boards_info.boards[board_index].max_num_smos)
        err = SDVR_ERR_SMO_PORT_NUM;
    else if (!_is_SMO_supported_resolution(smo_grid->res_decimation))
        err = SDVR_ERR_INVALID_RESOLUTION;
    else
        err = ssdk_validate_board_ndx(board_index);

    if (err !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_set_smo_grid_func,err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cntrl_strct.job_type = cmd_strct.job_type = sdvr_get_chan_type(handle);
    cntrl_strct.job_id = cmd_strct.job_id = sdvr_get_chan_num(handle);
    cmd_strct.port_num = smo_port_num;

    if (cmd_strct.job_type != SDVR_CHAN_TYPE_DECODER)
    {
        is_smo_enable = 
            _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct.job_id].is_smo_enable;
    }
    else
    {
        is_smo_enable = 
           _ssdvr_boards_info.boards[board_index].decode_channels[cmd_strct.job_id].is_smo_enable;
    }

    /***************************************************
       No need to do anything if the caller is stopping 
       an SMO which is already stopped.
    ****************************************************/
    if (!is_smo_enable && smo_grid->enable == false)
    {
        return SDVR_ERR_NONE;
    }

    // Make sure the x,y coordinates are even number
    if ((smo_grid->top_left_mb_x % 2) || (smo_grid->top_left_mb_y % 2))
        return SDVR_ERR_ODD_SMO_COORDINATES;

    /************************************************************************
            Since the firmware currently can not handle moving of an smo
            grid that is enabled from one location to another, we must
            always make sure the SMO grid is disabled before we change its
            location. Not doing so, will cause an error in the firmware 
            which indicates same SMO grid can not be displayed in to two 
            different location.
    ************************************************************************/
    if (is_smo_enable)
    {
        cmd_strct.enable = false;
        ssdk_message_chan_send(handle, DVR_SET_SMO, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_chan_recv(handle, DVR_REP_SMO, &cmd_strct, 0, 0);
        sdbg_msg_output(sdvr_set_smo_grid_func, board_index, (void *)&cmd_strct);
    }

    /******************************************************************
        set the new SMO grid string, its most top left coordinate,
        the resolution decimation, and its enable state. Then send this 
        information to the firmware.
        NOTE: There is no need to send SMO message if we are disabling.
              Since, if we get to this point the SMO is disabled by
              now.
    ******************************************************************/
    if (smo_grid->enable)
    {
        cmd_strct.port_num = smo_port_num;
        cmd_strct.position_x = smo_grid->top_left_mb_x;
        cmd_strct.position_y = smo_grid->top_left_mb_y;
        cmd_strct.output_res = smo_grid->res_decimation;
        cmd_strct.seconds = smo_grid->dwell_time;
        cmd_strct.enable = smo_grid->enable;

        ssdk_message_chan_send(handle, DVR_SET_SMO, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_chan_recv(handle, DVR_REP_SMO, &cmd_strct, 0, 0);
        sdbg_msg_output(sdvr_set_smo_grid_func, board_index, (void *)&cmd_strct);
    }

    /*********************************************************************
           Change the status of channel running.
           If nothing is streaming but the channel was running previously,
           we must stop the channel.

           If the SMO got enabled but nothing was streaming previously,
           then we must start the channel to see video frames.
    **********************************************************************/

    if (cmd_strct.status == SDVR_ERR_NONE)
    {
        if (ssdk_is_chan_running(handle) != ssdk_is_chan_streaming(handle))
        {
            cntrl_strct.enable = ssdk_is_chan_streaming(handle);
            ssdk_message_chan_send(handle, DVR_SET_CONTROL, &cntrl_strct, sizeof(cntrl_strct));
            ssdk_message_chan_recv(handle, DVR_REP_CONTROL, &cntrl_strct, 0, 0);
            if (cntrl_strct.status != DVR_STATUS_OK)
                sdbg_msg_output_err(DVR_SET_CONTROL, cntrl_strct.status);
        }
    }
    return cmd_strct.status;
}

static sdvr_err_e _get_smo_port_grid(sdvr_chan_handle_t  handle,
                                sx_uint8 smo_port_num,
                                sdvr_smo_grid_t *smo_grid)
{
    sdvr_err_e err;
    dvr_smo_t cmd_strct;
    sx_uint8 board_index = sdvr_get_board_index(handle);
    int cmd_strct_size = sizeof(cmd_strct);

    if (board_index == 0xFF)
        err = SDVR_ERR_INVALID_CHANNEL;
    else if (smo_grid == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (!smo_port_num || 
        smo_port_num > _ssdvr_boards_info.boards[board_index].max_num_smos)
        err = SDVR_ERR_SMO_PORT_NUM;

    else
        err = ssdk_validate_board_ndx(board_index);

    if (err !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_get_smo_grid_func,err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = sdvr_get_chan_type(handle);
    cmd_strct.job_id = sdvr_get_chan_num(handle);
    cmd_strct.port_num = smo_port_num;

    ssdk_message_chan_send(handle, DVR_GET_SMO, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_chan_recv(handle, DVR_REP_SMO, &cmd_strct, 0, 0);

    if (cmd_strct.status == DVR_STATUS_OK )
    {
        smo_grid->top_left_mb_x = cmd_strct.position_x;
        smo_grid->top_left_mb_y  = cmd_strct.position_y ;
        smo_grid->res_decimation = cmd_strct.output_res;
        smo_grid->dwell_time = cmd_strct.seconds;
        smo_grid->enable = cmd_strct.enable;
    }

    sdbg_msg_output(sdvr_get_smo_grid_func, board_index,(void *)&cmd_strct);

    return cmd_strct.status;
}
/****************************************************************************
  This function is used to set the Spot Monitor Output (SMO) grid pattern. 

  You can divide  the SMO into different grid patterns. Each tile of the
  SMO grid is defined by its left top most coordinate in pixels, and by the
  decimation of its original  video resolution.

  The grid pattern on a spot monitor is flexible and can be defined
  in various patterns (such as 4x4, 3x3, or 8x8 grids) as it applies to the
  resolution of the connected monitor.

  Each tile of the grid is described by an sdvr_smo_grid_t data structure. 

  Each tile can either be enabled or disabled. If you have more than one 
  channel in one tile position, you must also specify a dwell time, that is,
  the amount of time each channel is displayed before switching to the 
  next channel. If you only have one channel assigned to a tile, that 
  channel is permanently displayed while it is enabled.

  @NOTE: You can include encoder and decoder channels in the SMO tile. For
  encoder channels, the live video is displayed. For decoder channels, 
  the decoded (playback) video is displayed. @

  You can assign multiple channels to a tile, but a specific channel cannot 
  be assigned to different tiles.

  Assuming your SMO is an NTSC monitor of 720x480 resolution, and you want
  to specify 16 tiles arranged in a 4x4 grid pattern, then each tile is
  180x120. The first left top most grid is (0,0) the next one is (180,120), and so on.
  The resolution decimation for each grid is 1/4 of the camera resolution.

  @NOTE: The top left coordinates of each tile must be an even number (i.e.,
  coordinates (0,5) or (1,2) are invalid).@

  Parameters:

    "handle" - An encode or decode video channel handle whose output is
    displayed on this grid. See section "Spot Monitor Output (SMO)" for
    restriction in using decode video channel handle.

    "smo_grid" - A pointer structure defining one grid on an SMO. 
    Supported SMO resolution decimations are SDVR_VIDEO_RES_DECIMATION_EQUAL,
    SDVR_VIDEO_RES_DECIMATION_CIF, and SDVR_VIDEO_RES_DECIMATION_QCIF.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:
  
    The SMO grid is not displayed at all if it does not fit completely 
    on the screen.

    Call sdvr_set_smo_grid_ex() if your system has more than one SMO 
    monitor and you are interested to
    define the SMO grids for a specific spot monitor display.

    This call will be removed after release 5.x.x.x. Please start using
    sdvr_set_smo_grid_ex();
****************************************************************************/
sdvr_err_e sdvr_set_smo_grid(sdvr_chan_handle_t  handle,
                             sdvr_smo_grid_t *smo_grid)
{
    sdvr_err_e err;

    /* 
      We are keeping this function for backward compatiblity
      which means it always only applies to first SMO.
    */
    err = _set_smo_port_grid(handle,1, smo_grid);
    return err;

}

/****************************************************************************
  This function is used to get the current SMO grid configuration.

  Parameters:

    "handle" - An encode or decode video channel handle whose output is
    displayed on this grid.

    "smo_grid" - A pointer structure defining one grid on an SMO. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    This call will be removed after release 5.x.x.x. Please start using
    sdvr_get_smo_grid_ex().
****************************************************************************/
sdvr_err_e sdvr_get_smo_grid(sdvr_chan_handle_t  handle,
                             sdvr_smo_grid_t *smo_grid)
{
    sdvr_err_e err;

    /* 
      We are keeping this function for backward compatiblity
      which means it always only applies to first SMO.
    */
    err = _get_smo_port_grid(handle,1, smo_grid);
    return err;

}

/****************************************************************************
  This function is used to set the Spot Monitor Output (SMO) grid pattern
  for a specific SMO display.

  You can divide  the SMO into different grid patterns. Each tile of the
  SMO grid is defined by its left top most coordinate in pixels, and by the
  decimation of its original  video resolution.

  The grid pattern on a spot monitor is flexible and can be defined
  in various patterns (such as 4x4, 3x3, or 8x8 grids) as it applies to the
  resolution of the connected monitor.

  Each tile of the grid is described by an sdvr_smo_grid_t data structure. 

  Each tile can either be enabled or disabled. If you have more than one 
  channel in one tile position, you must also specify a dwell time, that is,
  the amount of time each channel is displayed before switching to the 
  next channel. If you only have one channel assigned to a tile, that 
  channel is permanently displayed while it is enabled.

  @NOTE: You can include encoder and decoder channels in the SMO tile. For
  encoder channels, the live video is displayed. For decoder channels, 
  the decoded (playback) video is displayed. @

  You can assign multiple channels to a tile, but a specific channel cannot 
  be assigned to different tiles.

  Assuming your SMO is an NTSC monitor of 720x480 resolution, and you want
  to specify 16 tiles arranged in a 4x4 grid pattern, then each tile is
  180x120. The first left top most grid is (0,0) the next one is (180,120), and so on.
  The resolution decimation for each grid is 1/4 of the camera resolution.

  @NOTE: The top left coordinates of each tile must be an even number (i.e.,
  coordinates (0,5) or (1,2) are invalid).@

  Parameters:

    "handle" - An encode or decode video channel handle whose output is
    displayed on this grid. See section "Spot Monitor Output (SMO)" for
    restriction in using decode video channel handle.

    "port_num" - The SMO port number where the first SMO display is
    one (1).

    "smo_grid" - A pointer structure defining one grid on an SMO. 
    Supported SMO resolution decimations are SDVR_VIDEO_RES_DECIMATION_EQUAL,
    SDVR_VIDEO_RES_DECIMATION_CIF, and SDVR_VIDEO_RES_DECIMATION_QCIF.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:
  
    The SMO grid is not displayed at all if it does not fit completely 
    on the screen.
****************************************************************************/
sdvr_err_e sdvr_set_smo_grid_ex(sdvr_chan_handle_t  handle,
                                        sx_uint8 port_num,
                                        sdvr_smo_grid_t *smo_grid)
{
    sdvr_err_e err;

    /* 
      We are keeping the function for backward compatiblity
      which means it always only applies to first SMO.
    */
    err = _set_smo_port_grid(handle, port_num, smo_grid);
    return err;

}
 
/****************************************************************************
  This function is used to get the current SMO grid configuration for a given
  SMO display .

  Parameters:

    "handle" - An encode or decode video channel handle whose output is
    displayed on this grid.

    "port_num" - The SMO port number where the first SMO display is
    one (1).

    "smo_grid" - A pointer structure defining one grid on an SMO. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

****************************************************************************/
sdvr_err_e sdvr_get_smo_grid_ex(sdvr_chan_handle_t  handle,
                                        sx_uint8 port_num,
                                        sdvr_smo_grid_t *smo_grid)
{
    sdvr_err_e err = SDVR_ERR_NONE;

    err = _get_smo_port_grid(handle, port_num, smo_grid);
    return err;

}

/**********************************************************************
  This function returns various attributes assoicated to the requested
  Spot Montior Out (SMO). Call this function to find out the SMO
  screen size.

  Parameters:

    "board_index" - Zero based index of the DVR board to 
    query its SMO information.

    "port_num" - The video out port number starting from port number 1.

    "smo_attrib" - A structure to hold the attribute of the 
    requested SMO port.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible
    error codes.
**********************************************************************/

sdvr_err_e sdvr_get_smo_attributes(sx_uint32 board_index,
                                  sx_uint8 port_num,
                                  sdvr_smo_attribute_t *smo_attrib)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_smo_attrib_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    if (smo_attrib == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (!port_num || port_num > MAX_NUM_SMO)
        err = SDVR_ERR_SMO_PORT_NUM;
    else if ((err = ssdk_validate_board_ndx(board_index)) == SDVR_ERR_NONE)
    {
        if (port_num > _ssdvr_boards_info.boards[board_index].max_num_smos)
            err = SDVR_ERR_SMO_PORT_NUM;
    }

    if (err !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_get_smo_attributes_func,err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.smo_port_num = port_num;

    ssdk_message_board_send(board_index, DVR_GET_SMO_ATTRIB, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_SMO_ATTRIB, &cmd_strct, 0, 0);

    if (cmd_strct.status == DVR_STATUS_OK )
    {
        smo_attrib->video_formats = cmd_strct.video_formats;
        smo_attrib->width  = cmd_strct.width ;
        smo_attrib->height = cmd_strct.height;
        smo_attrib->cap_flags = cmd_strct.cap_flags;

        // Remember the smo widht and height for this board.
        // It will be used in calculating of the send buffer size.
        _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].width = cmd_strct.width;
        _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].height = cmd_strct.height;

        sdbg_msg_output(sdvr_get_smo_attributes_func, board_index,(void *)&cmd_strct);
    }
    else
        sdbg_msg_output_err(sdvr_get_smo_attributes_func,err);

    return cmd_strct.status;

}

/**********************************************************************
  This function initializes the video output port on the specified board.

  Parameters:

    "board_index" - Index of the board to send the video.

    "port_num" - The video out port number starting from port number 1.

    "video_format" - The raw video format that is going to be sent to the
    firmware.

    "handle" - Pointer to storage for the output channel handle, filled in on
    return.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible
    error codes.
**********************************************************************/
sdvr_err_e
sdvr_start_video_output(sx_uint32 board_index, 
                        sx_uint8 port_num,
                        sdvr_rawv_formats_e video_format,
                        sdvr_chan_handle_t *handle)
{
    dvr_videoout_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sdvr_err_e err = SDVR_ERR_BOARD_NOT_CONNECTED;
    sx_uint32 sct_buf_size;
    sx_uint32 y_buf_size;
    sx_uint32 u_buf_size;
    sx_uint32 v_buf_size;
    sx_uint16 width;
    sx_uint16 height;
    int len = 0;
    // The PCI port number to send data to the FW is the
    // same number as the 1st 16 bit of channel handle.
    sct_channel_t pci_send_chan_handle = NULL;

    if (handle == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (!port_num || port_num > MAX_NUM_SMO)
        err = SDVR_ERR_SMO_PORT_NUM;
    else if ((err = ssdk_validate_board_ndx(board_index)) == SDVR_ERR_NONE)
    {
        if (port_num > _ssdvr_boards_info.boards[board_index].max_num_smos)
            err = SDVR_ERR_SMO_PORT_NUM;
    }


    if (err !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_start_video_output_func,err);
        return err;
    }


    /*
       Tell the firmware to open the given SCT port ID to receive raw video
       frame to display on SMO.
    */
    memset(&cmd_strct,0,cmd_strct_size);
    cmd_strct.opcode = DVR_VOUT_OPCODE_START;
    /* Firmware expect a 1 based port number */
    cmd_strct.smo_port_num = port_num; 
    cmd_strct.u1.start.video_format = video_format;
    *handle = ssdk_create_chan_handle(board_index, SDVR_CHAN_TYPE_OUTPUT, port_num - 1);
    cmd_strct.u1.start.sct_port_id = *handle & 0x0000FFFF;


    /*********************************************************
        Calculate the buffer size according to the video format and
        the size of SMO screen width and height.
    **********************************************************/
    /*
        Makes sure the SMO widht and height are available.
        Only get the SMO width and height on this board if it never
        was read from the board. It is needed to calculate the 
        send buffer size.

        NOTE: The SMO widht and height are saved inside the call
             to sdvr_get_smo_attributes() that is why we are not
             saving after returning from this function/
    */
    if (!_ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].width &&
        !_ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].height)
    {
        sdvr_smo_attribute_t smo_attrib;
        sdvr_get_smo_attributes(board_index, port_num,&smo_attrib);
    }

    width = _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].width;
    height = _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].height;

    if (video_format == SDVR_RAWV_FORMAT_YUV_4_2_2)
    {
        y_buf_size = width * height;
        u_buf_size = (width / 2) * height;
        v_buf_size = u_buf_size;

    }
    else // SDVR_RAWV_FORMAT_YUV_4_2_0 or anything else for now.
    {
        y_buf_size = width * height;
        u_buf_size = (width / 2) * (height / 2);
        v_buf_size = u_buf_size;
    }
    sct_buf_size = y_buf_size + u_buf_size + v_buf_size + DVR_DATA_HDR_LEN + (1024 * 5);
    _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].y_buf_size = y_buf_size;
    _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].u_buf_size = u_buf_size;
    _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].v_buf_size = v_buf_size;


    ssdk_message_board_send(board_index, DVR_SET_VIDEO_OUTPUT, &cmd_strct, sizeof(cmd_strct));
    /*
        we must try to open a SCT channel to send raw video.
    */

    err = sct_channel_connect( _ssdvr_boards_info.boards[board_index].handle,
                 cmd_strct.u1.start.sct_port_id,
                 SSDK_SMO_SEND_BUF_COUNT,   // number of send buffers to allocate
                 sct_buf_size,    // the send buffer size
                 &pci_send_chan_handle);
    if (err == SDVR_ERR_NONE)
    {
        err = ssdk_message_board_recv(board_index, DVR_REP_VIDEO_OUTPUT, &cmd_strct, 0, 0);
        if (err != SDVR_ERR_NONE || cmd_strct.smo_port_num != SDVR_ERR_NONE)
        {
            if (err != SDVR_ERR_NONE)
            {
                err = cmd_strct.status;

                // If there was any error, close the sct channel and stop.
                sct_channel_close(pci_send_chan_handle);
            }
            else 
            {
                _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].sdk_chan_handle = 
                    *handle;
                _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].pci_send_chan_handle = 
                    pci_send_chan_handle;
                _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].video_format = video_format;
                _ssdvr_boards_info.boards[board_index].smo_info[port_num - 1].avail_send_buf_count =
                    SSDK_SMO_SEND_BUF_COUNT;

                ssdk_register_sdk_chan_callback(pci_send_chan_handle, *handle);

            }
        }
    }
    sdbg_msg_output_err(sdvr_start_video_output_func,err);

    return err;
}

/**********************************************************************
  This function shuts down the video output port on the specified board.

  Parameters:

    "handle" - The handle of the output channel to close.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible
    error codes.
**********************************************************************/
sdvr_err_e
sdvr_stop_video_output(sdvr_chan_handle_t handle)
{
    dvr_videoout_t cmd_strct;
    sx_uint8 board_index;
    sx_uint8 port_num;
    int cmd_strct_size = sizeof(cmd_strct);
    sdvr_err_e err = SDVR_ERR_BOARD_NOT_CONNECTED;
    int len = 0;
    // The PCI port number to send data to the FW is the
    // same number as the 1st 16 bit of channel handle.
    sct_channel_t pci_send_chan_handle = NULL;

    err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_OUTPUT);

    if (err !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_stop_video_output_func,err);
        return err;
    }

    board_index = CH_GET_BOARD_ID(handle);

    port_num = CH_GET_CHAN_NUM(handle);

    /*
       Tell the firmware to close the given SCT port ID to stop 
       receiving SMO raw video frames from the host.
    */
    memset(&cmd_strct,0,cmd_strct_size);
    cmd_strct.opcode = DVR_VOUT_OPCODE_STOP;
    cmd_strct.smo_port_num = port_num + 1;

    ssdk_message_board_send(board_index, DVR_SET_VIDEO_OUTPUT, &cmd_strct, sizeof(cmd_strct));
            
    err = sct_channel_close(_ssdvr_boards_info.boards[board_index].smo_info[port_num].pci_send_chan_handle);

    if (err == SDVR_ERR_NONE)
    {
        _ssdvr_boards_info.boards[board_index].smo_info[port_num].sdk_chan_handle = 0;
        _ssdvr_boards_info.boards[board_index].smo_info[port_num].pci_send_chan_handle = 0; 
        _ssdvr_boards_info.boards[board_index].smo_info[port_num].video_format = 0;
        _ssdvr_boards_info.boards[board_index].smo_info[port_num].avail_send_buf_count = 0;
        _ssdvr_boards_info.boards[board_index].smo_info[port_num].y_buf_size = 0;
        _ssdvr_boards_info.boards[board_index].smo_info[port_num].u_buf_size = 0;
        _ssdvr_boards_info.boards[board_index].smo_info[port_num].v_buf_size = 0;

        err = ssdk_message_board_recv(board_index, DVR_REP_VIDEO_OUTPUT, &cmd_strct, 0, 0);
        if (err != SDVR_ERR_NONE)
        {
            err = cmd_strct.status;
        }
    }

    sdbg_msg_output_err(sdvr_stop_video_output_func,err);

    return err;
}

/**********************************************************************
  Get an empty buffer to hold the raw video frame for the purpose of video 
  output. This can be filled in and sent to the board 
  for display. The raw video frame should be copied to the payload field
  of this buffer in the compact form. In case your raw video frame is
  YUV format, you must copy Y plane followed by U plane followed by V
  plane at the payload field of this buffer.

  Parameters:

    "handle" - Handle of the output channel. This is the handle that
    was created and returned by the call to sdvr_start_video_output().

    "frame_buffer" - Pointer to storage that will hold a pointer to the 
    allocated buffer on successful return.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible
    error codes.

  Remarks:

    The size of this buffer varies depending on the the video format
    specified when sdvr_start_video_output() was called and the SMO
    video size. To get the SMO video size call sdvr_get_smo_attributes().
**********************************************************************/
sdvr_err_e
sdvr_get_video_output_buffer(sdvr_chan_handle_t handle, sdvr_av_buffer_t **frame_buffer)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 smo_port = CH_GET_CHAN_NUM(handle);
    sct_channel_t pci_send_chan_handle;
    sdvr_err_e err;
    
    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_OUTPUT)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_video_output_buffer_func, err);
        return err;    
    }
   
    pci_send_chan_handle = _ssdvr_boards_info.boards[board_index].smo_info[smo_port].pci_send_chan_handle;
    if (pci_send_chan_handle == 0)
    {
        sdbg_msg_output_err(sdvr_get_video_output_buffer_func, SDVR_ERR_CHANNEL_CLOSED);
        return SDVR_ERR_CHANNEL_CLOSED;    

    }
   
    /************************************************************
        get the memory from sct and setup the frame header.
    *************************************************************/
    *frame_buffer = (sdvr_av_buffer_t *)sct_buffer_alloc( pci_send_chan_handle );
    if (*frame_buffer != NULL)
    {
        (*frame_buffer)->signature = SDVR_DATA_HDR_SIG;
        (*frame_buffer)->board_id = board_index;
        (*frame_buffer)->channel_id = smo_port;
        (*frame_buffer)->channel_type = chan_type;
        (*frame_buffer)->hdr_size = DVR_DATA_HDR_LEN;
        (*frame_buffer)->yuv_format = 
            _ssdvr_boards_info.boards[board_index].smo_info[smo_port].video_format;
        (*frame_buffer)->y_data_size = 
            _ssdvr_boards_info.boards[board_index].smo_info[smo_port].y_buf_size;
        (*frame_buffer)->u_data_size = 
            _ssdvr_boards_info.boards[board_index].smo_info[smo_port].u_buf_size;
        (*frame_buffer)->v_data_size = 
            _ssdvr_boards_info.boards[board_index].smo_info[smo_port].v_buf_size;

        (*frame_buffer)->payload_size = (*frame_buffer)->y_data_size +
            (*frame_buffer)->u_data_size + (*frame_buffer)->v_data_size;

        (*frame_buffer)->y_data_offset = DVR_DATA_HDR_LEN;
        (*frame_buffer)->u_data_offset = (*frame_buffer)->y_data_offset + (*frame_buffer)->y_data_size;
        (*frame_buffer)->v_data_offset = (*frame_buffer)->u_data_offset + (*frame_buffer)->u_data_size;


        _ssdk_mutex_decode_buf_lock(); 

        if (_ssdvr_boards_info.boards[board_index].smo_info[smo_port].avail_send_buf_count > 0)
            _ssdvr_boards_info.boards[board_index].smo_info[smo_port].avail_send_buf_count--;

        _ssdk_mutex_decode_buf_unlock(); 
    
        err = SDVR_ERR_NONE;
    }
    else
        err = SDVR_ERR_BUF_NOT_AVAIL;
    return err;
}

/**********************************************************************
  Send one raw compact video frame to the video output.
  Each frame resolution is assumed to be 4CIF based on the current system
  video standard. 

  Parameters:

    "handle" - Handle of the output channel. This is the handle that
    was created and returned by the call to sdvr_start_video_output().

    "frame_buffer" - Pointer to a sdvr_av_buffer_t object that has 
    been filled in with the a raw video frame.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible
    error codes.

  Remarks:

    This buffer must have been allocated by the call to
    sdvr_get_video_output_buffer().
    The raw video frame should have been copied to the payload field
    of this buffer in the compacted raw video format. 
    In case your raw video frame is YUV format, you must copy Y 
    plane followed by U plane followed by V
    plane at the payload field of this buffer.

**********************************************************************/
sdvr_err_e
sdvr_send_video_output(sdvr_chan_handle_t handle, sdvr_av_buffer_t *frame_buffer)
{
    sx_uint8 board_index;
    sx_uint8 chan_type;
    sx_uint8 smo_port;
    sdvr_chan_handle_t  buf_handle;
    sct_channel_t pci_send_chan_handle;
    sdvr_err_e err;
    void * buf = &frame_buffer->payload[0];

    if (frame_buffer == NULL ||
        (buf_handle = sdvr_get_buffer_channel(frame_buffer)) == INVALID_CHAN_HANDLE ||
        frame_buffer->channel_type != SDVR_CHAN_TYPE_OUTPUT ||
        handle != buf_handle)
    {
        err = SDVR_ERR_INVALID_ARG;
        sdbg_msg_output_err(sdvr_send_video_output_func, err);
        return err;
    }

    board_index = frame_buffer->board_id;
    smo_port = frame_buffer->channel_id;
    chan_type = frame_buffer->channel_type;

    pci_send_chan_handle = _ssdvr_boards_info.boards[board_index].smo_info[smo_port].pci_send_chan_handle;
    if (pci_send_chan_handle)
    {
        /*
            Send buffer to board and remove it from the availabe buffer list.
            NOTE: The channel_id which is the smo port number is expected to
                  be one based .
        */
        frame_buffer->channel_id++;
        err = sct_buffer_send(pci_send_chan_handle,(void *)frame_buffer, 
            frame_buffer->payload_size + frame_buffer->hdr_size);
        frame_buffer->channel_id--;
     }
    else
    {
        err = SDVR_ERR_CHANNEL_CLOSED;
    }
    sdbg_msg_output_err(sdvr_send_video_output_func, err);
    return err ;    
}


