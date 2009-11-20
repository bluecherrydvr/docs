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

/****************************************************************************
  This function is used to enable streaming of raw video for a526
  channel to the host. If the channel is an encoding channel, 
  then the raw video is from the camera. If it is a decoding 
  channel, the raw video is the decoded video. 

  This function can be called multiple times to enable and disable streaming.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "res_decimation" - Specifies whether the video is streamed at
    the system-wide maximum resolution, or 1/4 (CIF), or 1/16 (QCIF), or 2CIF
    of that resolution. (NOTE: DCIF resolution is not supported).

    "frame_rate" - Specifies the frame rate at which the raw video
    frames are being sent to the host application. In general this
    value should be 30. 
    The valid range is 1 - 30.

    "enable" - If true, streaming is enabled. Disabled otherwise.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e __sdvr_stream_raw_video(sdvr_chan_handle_t handle,
								 sx_uint8 stream_id,
                                 sdvr_video_res_decimation_e res_decimation,
                                 sx_uint8 frame_rate,
                                 sx_bool enable)
{
    dvr_hmo_t cmd_strct;
    dvr_control_t cntrl_strct;
    sdvr_err_e err;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = sdvr_get_board_index(handle);
    sx_uint8 chan_num = sdvr_get_chan_num(handle);

    if (frame_rate < 1 || frame_rate > 30)
    {
        err =  SDVR_ERR_INVALID_ARG;
    }
    else if (res_decimation == SDVR_VIDEO_RES_DECIMATION_DCIF)
        err = SDVR_ERR_INVALID_RESOLUTION;
    else
    {
        err = ssdk_validate_chan_handle(handle, -1);
    }

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_stream_raw_video_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);
    cntrl_strct.job_type = cmd_strct.job_type = sdvr_get_chan_type(handle);
    cntrl_strct.job_id = cmd_strct.job_id = sdvr_get_chan_num(handle);
    // Get the existing HMO information to preserve the raw audio streaming
    ssdk_message_chan_send(handle, DVR_GET_HMO, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_HMO, &cmd_strct, 0, 0);
    if (cmd_strct.status != DVR_STATUS_OK)
    {
        sdbg_msg_output_err(DVR_GET_HMO, cmd_strct.status);
        return cmd_strct.status;
    }
    
    // Set the new raw vido parameters. Keep the existing raw audio state.
    cmd_strct.audio_enable =  _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].is_raw_audio_streaming;
    cmd_strct.enable[stream_id] = enable;
    cmd_strct.frame_rate[stream_id] = frame_rate;
    switch (res_decimation)
    {
    case SDVR_VIDEO_RES_DECIMATION_FOURTH:
        cmd_strct.output_res[stream_id] = DVR_VIDEO_RES_CIF;
        break;

    case SDVR_VIDEO_RES_DECIMATION_SIXTEENTH:
        cmd_strct.output_res[stream_id] = DVR_VIDEO_RES_QCIF;
        break;

    case SDVR_VIDEO_RES_DECIMATION_EQUAL:
        cmd_strct.output_res[stream_id] = DVR_VIDEO_RES_FULL;
        break;

    case SDVR_VIDEO_RES_DECIMATION_HALF:
        cmd_strct.output_res[stream_id] = DVR_VIDEO_RES_HALF;
        break;

    case SDVR_VIDEO_RES_DECIMATION_DCIF:
        cmd_strct.output_res[stream_id] = DVR_VIDEO_RES_DCIF;
        break;

    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF:
        cmd_strct.output_res[stream_id] = DVR_VIDEO_RES_CLASSIC_CIF;
        break;

    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF:
        cmd_strct.output_res[stream_id] = DVR_VIDEO_RES_CLASSIC_2CIF;
        break;

    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF:
        cmd_strct.output_res[stream_id] = DVR_VIDEO_RES_CLASSIC_4CIF;
        break;


    default:
        sdbg_msg_output_err(sdvr_stream_raw_video_func, SDVR_ERR_INVALID_ARG);
        return SDVR_ERR_INVALID_ARG;
    }


    /********************************************************************
           If we are enabling the raw video streaming,
           set the raw A/V streaming flag so that the callback
           function accept the raw frames; Otherwise, it is possible to 
           loose one or more of Y, U, or V buffers from the time we 
           send the enable HMO to the board before we set the streaming flag.

           NOTE: If we are disabling streaming, the flag should be set after we
           send the disable HMO to the board.
    ********************************************************************/
    if (cmd_strct.enable[stream_id])
        ssdk_enable_raw_av(handle, stream_id, true, res_decimation);

    /*****************************************************************************
             Send the command to the firmware
    *****************************************************************************/
    ssdk_message_chan_send(handle, DVR_SET_HMO, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_HMO, &cmd_strct, 0, 0);
    sdbg_msg_output(sdvr_stream_raw_video_func, board_index, (void *)&cmd_strct);

    /********************************************************************
           If there was an error to enable HMO streaming or the user wants
           to disable and existing HMO streamin,
           set the raw A/V streaming flag so that the callback
           function either reject the raw frames.
           Also release all the frames in the SDK raw frame queue.
    ********************************************************************/
    if ((cmd_strct.status == SDVR_ERR_NONE && !enable) ||
        (cmd_strct.status != SDVR_ERR_NONE && enable) )
        ssdk_enable_raw_av(handle, stream_id, false, res_decimation);

    /*********************************************************************
           In case, of decoder channel we must let the channel to be
           started by enabling the decoder itself.
           In case, of encoder or HMO only channel, we start the channel
           if the raw video is enabled. We also need to stop the channel
           if encoder, HMO, and SMO are no longer enabled.
    **********************************************************************/
    if ((cmd_strct.status == SDVR_ERR_NONE)
        && (sdvr_get_chan_type(handle) != SDVR_CHAN_TYPE_DECODER))
    {
        cmd_strct.status = ssdvr_enable_chan(handle);
    }

    return cmd_strct.status;
}

/****************************************************************************
  This function is used to enable streaming of primary raw video for a
  channel to the host. If the channel is an encoding channel, 
  then the primary raw video is from the camera. If it is a decoding 
  channel, the raw video is the decoded video. 

  This function can be called multiple times to enable and disable streaming.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "res_decimation" - Specifies whether the video is streamed at
    the system-wide maximum resolution, or 1/4 (CIF), or 1/16 (QCIF), or 2CIF
    of that resolution. (NOTE: DCIF resolution is not supported).

    "frame_rate" - Specifies the frame rate at which the raw video
    frames are being sent to the host application. In general this
    value should be 30. 
    The valid range is 1 - 30.

    "enable" - If true, streaming is enabled. Disabled otherwise.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_stream_raw_video(sdvr_chan_handle_t handle,
                                 sdvr_video_res_decimation_e res_decimation,
                                 sx_uint8 frame_rate,
                                 sx_bool enable)
{
	return __sdvr_stream_raw_video(handle, 0, res_decimation, frame_rate, enable);
}

/****************************************************************************
  This function is used to enable streaming of secondary raw video for a
  channel to the host. If the channel is an encoding channel, 
  then the secondary raw video is from the camera. Decoding
  channel is not supported for secondary raw video,

  This function can be called multiple times to enable and disable streaming.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "res_decimation" - Specifies whether the video is streamed at
    the system-wide maximum resolution, or 1/4 (CIF), or 1/16 (QCIF), or 2CIF
    of that resolution. (NOTE: DCIF resolution is not supported).

    "frame_rate" - Specifies the frame rate at which the raw video
    frames are being sent to the host application. In general this
    value should be 30. 
    The valid range is 1 - 30.

    "enable" - If true, streaming is enabled. Disabled otherwise.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_stream_raw_video_secondary(sdvr_chan_handle_t handle,
                                 sdvr_video_res_decimation_e res_decimation,
                                 sx_uint8 frame_rate,
                                 sx_bool enable)
{
	return __sdvr_stream_raw_video(handle, 1, res_decimation, frame_rate, enable);
}

/****************************************************************************
  This function is used to enable streaming of raw audio for a
  channel to the host. If the channel is an encoding channel, 
  then the raw audio is from the microphone. If it is a decoding 
  channel, the raw audio is the decoded audio.

  The raw audio can only be enabled on a channel that is streaming raw
  video. Otherwise, this function is ignored. 

  This function can be called multiple times to enable and disable streaming.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "enable" - If true, streaming is enabled; otherwise, disabled.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_stream_raw_audio(sdvr_chan_handle_t handle,
                                 sx_bool enable)
{
    dvr_hmo_t cmd_strct;
    dvr_control_t cntrl_strct;
    sdvr_err_e err;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = sdvr_get_board_index(handle);
    sx_uint8 chan_num = sdvr_get_chan_num(handle);
    sdvr_chan_type_e chan_type = sdvr_get_chan_type(handle);

    if ((err = ssdk_validate_chan_handle(handle, -1)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_stream_raw_audio_func, err);
        return err;
    }

    // Ignore this command if there is no raw video streaming.
    if (chan_type == SDVR_CHAN_TYPE_DECODER)
    {
        if (!_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_raw_av_streaming)
            return SDVR_ERR_NONE;
    }
    else
    {
		int i;
		for (i=0; i<MAX_VIDEO_ENCODING; i++)
			if (_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].is_raw_av_streaming[i])
				break;
		if (i==MAX_VIDEO_ENCODING)
			return SDVR_ERR_NONE;

        if (_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].is_raw_audio_streaming == enable)
            return SDVR_ERR_NONE;

    }
    // Get the existing HMO information to preserve the raw video paramters
    memset(&cmd_strct,0,cmd_strct_size);
    cntrl_strct.job_type = cmd_strct.job_type = sdvr_get_chan_type(handle);
    cntrl_strct.job_id = cmd_strct.job_id = chan_num;

    ssdk_message_chan_send(handle, DVR_GET_HMO, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_HMO, &cmd_strct, 0, 0);
    if (cmd_strct.status != DVR_STATUS_OK)
    {
        sdbg_msg_output_err(DVR_GET_HMO, cmd_strct.status);
        return cmd_strct.status;
    }

    // Set the new raw audio parameters. Keep the existing raw video parameters
    cmd_strct.audio_enable = enable;

    /*****************************************************************************
             Send the command to the firmware
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_HMO, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_HMO, &cmd_strct, 0, 0);
    sdbg_msg_output(sdvr_stream_raw_audio_func, board_index, (void *)&cmd_strct);

    /*********************************************************************
           In case, of decoder channel we must let the channel to be
           started by enabling the decoder itself.
           In case, of encoder or HMO only channel, we start the channel
           if the raw audio is enabled. We also need to stop the channel
           if encoder, HMO, raw audio, and SMO are no longer enabled.
    **********************************************************************/
    if ((cmd_strct.status == SDVR_ERR_NONE)
        && (sdvr_get_chan_type(handle) != SDVR_CHAN_TYPE_DECODER))
    {
        _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].is_raw_audio_streaming = enable;

        cmd_strct.status = ssdvr_enable_chan(handle);
    }

    return cmd_strct.status;
}

/****************************************************************************
  This function is used to get the current video-in image parameters.

  Parameters:

    "handle" - An encoding channel handle.

    "image_ctrl" - A pointer to sdvr_image_ctrl_t which holds the
    current video-in image parameter for the given upon successful
    return.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_video_in_params(sdvr_chan_handle_t handle, 
                                    sdvr_image_ctrl_t *image_ctrl)
{
    dvr_ioctl_t cmd_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    if (image_ctrl == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    /*
       Video-in params is only supported in Firmware version 4.0.0.0 or later
    */
    if (!ssdk_is_min_version(_ssdvr_boards_info.boards[board_index].version.fw_major,
        _ssdvr_boards_info.boards[board_index].version.fw_minor,
        _ssdvr_boards_info.boards[board_index].version.fw_revision,
        _ssdvr_boards_info.boards[board_index].version.fw_build,
        4,0,0,0))
    {
        err = SDVR_ERR_UNSUPPORTED_FIRMWARE;
    }

    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_video_in_params_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

   /*****************************************************************************
             Get the current video-in image control parameters
    *****************************************************************************/
    cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);
    cmd_strct.ioctl_code = DVR_IOCTL_CODE_IMG_CONTROL;

    ssdk_message_chan_send(handle, DVR_GET_IOCTL, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_IOCTL, &cmd_strct, 0, 0);

    sdbg_msg_output(sdvr_get_video_in_params_func, board_index, (void *)&cmd_strct);

    /* Copy the result into the caller's data structure */
    if (cmd_strct.status == SDVR_ERR_NONE)
    {
        image_ctrl->brightness = cmd_strct.u1.img_ctrl.brt;
        image_ctrl->contrast   = cmd_strct.u1.img_ctrl.cont;
        image_ctrl->hue        = cmd_strct.u1.img_ctrl.hue;
        image_ctrl->saturation = cmd_strct.u1.img_ctrl.sat;
        image_ctrl->sharpness  = cmd_strct.u1.img_ctrl.sharp;
    }

    return cmd_strct.status;
}

/****************************************************************************
  This function is used to set various video-in image control parameters.
  You may choose to change one or more of the image control parameters. This
  is accompolished by setting the approriate bit in the control parameter
  flag.

  Parameters:

    "handle" - An encoding channel handle.

    "image_ctrl_flag" - A bit field indicating which of the fields
    in image_ctrl parameter needs to be updated. Use SDVR_ICFLAG_ALL to update
    all the video-in image control parameters. Or choose specific fields
    by ORing any of the image control flags (SDVR_ICFLAG_HUE, SDVR_ICFLAG_SATURATION, SDVR_ICFLAG_BRIGHTNESS,
    SDVR_ICFLAG_CONTRAST, and SDVR_ICFLAG_SHARPNESS).

    "image_ctrl" - A pointer to sdvr_image_ctrl_t which holds the
    new video-in image parameter for the given. For each image control
    flag set in image_ctrl_flag parameter, you must have the corresponding
    value in image_ctrl data structure parameter. (See sdvr_image_ctrl_t for
    valid ranges and defualt values for each field in this structure.)

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    Changing these parameters could effect both alarms detection and
    encoding bitrate.
****************************************************************************/

sdvr_err_e sdvr_set_video_in_params(sdvr_chan_handle_t handle, 
                                    sx_uint16 image_ctrl_flag,
                                    sdvr_image_ctrl_t *image_ctrl)
{
    dvr_ioctl_t cmd_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    if (image_ctrl == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    /*
       Video-in params is only supported in Firmware version 4.0.0.0 or later
    */
    if (!ssdk_is_min_version(_ssdvr_boards_info.boards[board_index].version.fw_major,
        _ssdvr_boards_info.boards[board_index].version.fw_minor,
        _ssdvr_boards_info.boards[board_index].version.fw_revision,
        _ssdvr_boards_info.boards[board_index].version.fw_build,
        4,0,0,0))
    {
        err = SDVR_ERR_UNSUPPORTED_FIRMWARE;
    }

    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_set_video_in_params_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

   /*****************************************************************************
             Set the current video-in image control parameters
    *****************************************************************************/
    cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);
    cmd_strct.ioctl_code = DVR_IOCTL_CODE_IMG_CONTROL;
    cmd_strct.u1.img_ctrl.flags = image_ctrl_flag;
    cmd_strct.u1.img_ctrl.hue = image_ctrl->hue;
    cmd_strct.u1.img_ctrl.sat = image_ctrl->saturation;
    cmd_strct.u1.img_ctrl.brt = image_ctrl->brightness;
    cmd_strct.u1.img_ctrl.cont = image_ctrl->contrast;
    cmd_strct.u1.img_ctrl.sharp = image_ctrl->sharpness;

    ssdk_message_chan_send(handle, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_IOCTL, &cmd_strct, 0, 0);

    sdbg_msg_output(sdvr_set_video_in_params_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;

}

/****************************************************************************
  By calling this function, you can specify the 
  format by which the raw video frames should be constructed. This is a system
  wide settings that affects raw video streaming for both the decoded and
  live video frames. 
  By default all the raw video frames are YUV 4:2:0 format.

  Parameters:

    "board_index" - The number of the board to set its raw video
    streaming format.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "format" - The requested YUV format. If the given format is not
    valid, it will default to SDVR_RAWV_FORMAT_YUV_4_2_0

  Returns:

    None.

  Remarks:

  This function should only be called if the board is in idle state. This
  means there is no A/V encoding, decoding, or streaming.
****************************************************************************/
sdvr_err_e sdvr_set_yuv_format(sx_uint32 board_index, sx_uint8 format)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    /********************************************************
       Validate the parameters:
    ********************************************************/

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }


    memset(&cmd_strct,0,cmd_strct_size);

    switch (format)
    {
        /*
            YUV 4.2.2 conversion is done by the SDK currently.
            No need to send this format to the board.
            But make sure the firmware is sending YUV420.
        */
    case SDVR_RAWV_FORMAT_YUV_4_2_2:
        cmd_strct.u1.rawv_format.format = SDVR_RAWV_FORMAT_YUV_4_2_0;
        break;

    case SDVR_RAWV_FORMAT_YUV_4_2_0:
    case SDVR_RAWV_FORMAT_YVU_4_2_0:
        cmd_strct.u1.rawv_format.format = format;

        break;
    default:
        return SDVR_ERR_INVALID_ARG;
    }

    cmd_strct.ioctl_code = DVR_IOCTL_CODE_RAWV_FORMAT;
    cmd_strct.status = SDVR_ERR_NONE;

    ssdk_message_board_send(board_index, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);
    err = ssdk_message_board_recv(board_index, DVR_REP_IOCTL, &cmd_strct, 0, 0);
    if (err != SDVR_ERR_NONE)
        cmd_strct.status = (unsigned char) err & 0xff;

    if (cmd_strct.status == SDVR_ERR_NONE)
        _ssdvr_boards_info.yuv_format = format;  

    return cmd_strct.status;
}

/***********************************************************************
  This function is used to enable or disable deinterlacing on the 
  specified input channel.

  Parameters:

    "handle" – Handle to an existing encoder or raw video channel.

    "enable" – Set to True to enable deinterlacing, 
    False to disable deinterlacing.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible error codes.

  Remarks:

    Deinterlacing is enabled by default.
***********************************************************************/
sdvr_err_e
sdvr_enable_deinterlacing( sdvr_chan_handle_t handle, sx_bool enable )
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    ssdk_pci_channel_info_t *enc_chan_info;
        

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.ioctl_code = DVR_IOCTL_CODE_VPP_CONTROL;
    cmd_strct.status = SDVR_ERR_NONE;

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err ==  SDVR_ERR_NONE)
    {
        cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
        cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);
        enc_chan_info = ssdk_get_enc_chan_strct(board_index, cmd_strct.job_id);

        /*
           Update the deinterlace flag without altering any other
           VPP action flags
        */
        if (enable)
        {
            enc_chan_info->vpp_action_flags |= DVR_VPP_ACTION_DEINTERLACE;
        }
        else
        {
            enc_chan_info->vpp_action_flags &= ~DVR_VPP_ACTION_DEINTERLACE;

        }
        cmd_strct.u1.vpp_ctrl.actions = enc_chan_info->vpp_action_flags;

        ssdk_message_chan_send(handle, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);
        err = ssdk_message_chan_recv(handle, DVR_REP_IOCTL, &cmd_strct, 0, 0);
        if (err != SDVR_ERR_NONE)
            cmd_strct.status = (unsigned char) err & 0xff;

    }

    sdbg_msg_output(sdvr_enable_deinterlacing_func,board_index,&cmd_strct);

    return err;
}

/**********************************************************************
  This function is used to enable or disable Stretch-lateral 
  noise reduction filtering on the specified input channel.

  Parameters:

    "handle" – Handle to an existing encoder or raw video channel.

    "enable" – Set to True to enable noise reduction, 
    False to disable noise reduction.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible error codes.

  Remarks:

    noise reduction is enabled by default.
**********************************************************************/
sdvr_err_e
sdvr_enable_noise_reduction( sdvr_chan_handle_t handle, sx_bool enable )
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    ssdk_pci_channel_info_t *enc_chan_info;

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.ioctl_code = DVR_IOCTL_CODE_VPP_CONTROL;
    cmd_strct.status = SDVR_ERR_NONE;

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err ==  SDVR_ERR_NONE)
    {
        cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
        cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);
        enc_chan_info = ssdk_get_enc_chan_strct(board_index, cmd_strct.job_id);

        /*
           Update the noise reduction flag without altering any other
           VPP action flags
        */
        if (enable)
        {
            enc_chan_info->vpp_action_flags |= DVR_VPP_ACTION_SLATERAL;
        }
        else
        {
            enc_chan_info->vpp_action_flags &= ~DVR_VPP_ACTION_SLATERAL;
        }
        cmd_strct.u1.vpp_ctrl.actions = enc_chan_info->vpp_action_flags;

        ssdk_message_chan_send(handle, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);
        err = ssdk_message_chan_recv(handle, DVR_REP_IOCTL, &cmd_strct, 0, 0);
        if (err != SDVR_ERR_NONE)
            cmd_strct.status = (unsigned char) err & 0xff;

    }

    sdbg_msg_output(sdvr_enable_noise_reduction_func,board_index,&cmd_strct);

    return err;
}


/**********************************************************************
  This function is used to adjust the gain on the specified camera input.

  Parameters:

    "handle" – Handle to an existing encoder or raw video channel.

    "value" – Gain setting value. 
    This value will be passed through as-is to the hardware. The range
    is 0 - 255.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible error codes.

  Remarks:

    Default values for this field is hardware specific. 
**********************************************************************/
sdvr_err_e
sdvr_set_gain_mode( sdvr_chan_handle_t handle, sx_uint8 value )
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.ioctl_code = DVR_IOCTL_CODE_GAIN_MODE;
    cmd_strct.u1.gain_mode.value = value;
    cmd_strct.status = SDVR_ERR_NONE;

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err ==  SDVR_ERR_NONE)
    {
        cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
        cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);

        /**********************************************************
          Send the given value to the given register number on the
          given device ID. 
        ***********************************************************/
        ssdk_message_chan_send(handle, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);
        err = ssdk_message_chan_recv(handle, DVR_REP_IOCTL, &cmd_strct, 0, 0);
        if (err != SDVR_ERR_NONE)
            cmd_strct.status = (unsigned char) err & 0xff;

    }

    sdbg_msg_output(sdvr_set_gain_mode_func,board_index,&cmd_strct);

    return err;
}

/*******************************************************************
  This function sets the camera termination impedance value.

  Parameters:

    "handle" – Handle of the channel to be affected.

    "term" – Enumerated constant defining the termination value.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible error codes.

  Remarks:

    The default termination value is SDVR_TERM_75OHM.
*******************************************************************/
sdvr_err_e
sdvr_set_camera_termination(sdvr_chan_handle_t handle, sdvr_term_e term)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.ioctl_code = DVR_IOCTL_CODE_TERMINATION;
    cmd_strct.u1.termination.value = (sx_uint8)term;
    cmd_strct.status = SDVR_ERR_NONE;

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err ==  SDVR_ERR_NONE)
    {
        cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
        cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);

        /**********************************************************
          Send the given value to the given register number on the
          given device ID. 
        ***********************************************************/
        ssdk_message_chan_send(handle, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);
        err = ssdk_message_chan_recv(handle, DVR_REP_IOCTL, &cmd_strct, 0, 0);
        if (err != SDVR_ERR_NONE)
            cmd_strct.status = (unsigned char) err & 0xff;
    }

    sdbg_msg_output(sdvr_set_camera_termination_func,board_index,&cmd_strct);

    return err;
}


