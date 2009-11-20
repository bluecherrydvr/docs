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
  This function enables the Nth encoder on a particular channel. After an encoder is
  enabled for a particular channel, the associated video encoding on 
  alarms will be activated.

  Parameters:

    "handle" - An encoding channel handle.

    "sub_chan_enc" - The encoder subchannel to enable or disable.

    "enable" - If true, the encoding is enabled; disabled otherwise.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.

  Remarks:
  
    After the encoder is enabled, both audio (if the channel has one) and video
    frames are sent.

    @NOTE: There are no audio frames for secondary encoders.@
****************************************************************************/
sdvr_err_e sdvr_enable_encoder(sdvr_chan_handle_t handle,
                               sdvr_sub_encoders_e sub_chan_enc,
                               sx_bool enable)
{
    sdvr_err_e err;
    sx_uint8 board_index = sdvr_get_board_index(handle);
    dvr_control_t cntrl_strct;
    dvr_encode_info_t encode_strct;

    sx_uint8 chan_num;
    //char msg[256];

    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER)) != SDVR_ERR_NONE ||
        (err = ssdk_validate_enc_sub_chan( handle, sub_chan_enc)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_enable_encoder_func, err);
        return err;
    }

    chan_num = sdvr_get_chan_num(handle);

    memset(&cntrl_strct,0,sizeof(cntrl_strct));

    encode_strct.job_type =  cntrl_strct.job_type = sdvr_get_chan_type(handle);
    encode_strct.job_id =  cntrl_strct.job_id = sdvr_get_chan_num(handle);
    encode_strct.stream_id = sub_chan_enc;

    /************************************************************************
            Get the existing encoder attributes so that we won't overwrite
            them when enable/disabling the encoder.
    ************************************************************************/

    ssdk_message_chan_send(handle, DVR_GET_ENCODE, &encode_strct, sizeof(encode_strct));
    ssdk_message_chan_recv(handle, DVR_REP_ENCODE, &encode_strct, 0, 0);
    if (encode_strct.status != DVR_STATUS_OK)
    {
        sdbg_msg_output_err(DVR_GET_ENCODE, encode_strct.status);

        return encode_strct.status;
    }

    /***********************************************************************
            NOTE: No attributes of the channel can be changed while the 
                  channel is running. In this case, we must temp. disable
                  the channel, make the change and re-enable it. The 
                  channel is running if either the encoder, SMO, or HMO
                  on that channel is still enabled.
             Make sure the channel is disabled
    ************************************************************************/
    if (ssdk_is_chan_running(handle))
    {
        cntrl_strct.enable = false;
        ssdk_message_chan_send(handle, DVR_SET_CONTROL, &cntrl_strct, sizeof(cntrl_strct));
        ssdk_message_chan_recv(handle, DVR_REP_CONTROL, &cntrl_strct, 0, 0);
        if (cntrl_strct.status != DVR_STATUS_OK)
		{
            sdbg_msg_output_err(DVR_SET_CONTROL, cntrl_strct.status);
			return cntrl_strct.status;
		}
    }

    /*****************************************************************************
         Set the new enable state of encoder and send the command to the firmware
    *****************************************************************************/
    encode_strct.enable = enable;
    sdbg_msg_output(sdvr_enable_encoder_func, board_index, (void*)&encode_strct);

    ssdk_message_chan_send(handle, DVR_SET_ENCODE, &encode_strct, sizeof(encode_strct));
    ssdk_message_chan_recv(handle, DVR_REP_ENCODE, &encode_strct, 0, 0);

	if ((err = encode_strct.status) == SDVR_ERR_NONE)
		ssdk_enable_enc_av( handle, 
							sub_chan_enc, 
							enable);
    else
        sdbg_msg_output_err(DVR_SET_ENCODE, encode_strct.status);

    /***************************************************************************
            If any of the SMO, Raw Video, or Encoded video where streaming
            we must restart the channel. Otherwise they will all stop.
    ****************************************************************************/
    if ((encode_strct.enable && encode_strct.status == SDVR_ERR_NONE) ||
        ssdk_is_chan_streaming(handle)) 
    {
        cntrl_strct.enable = true;
        ssdk_message_chan_send(handle, DVR_SET_CONTROL, &cntrl_strct, sizeof(cntrl_strct));
        ssdk_message_chan_recv(handle, DVR_REP_CONTROL, &cntrl_strct, 0, 0);
        err = cntrl_strct.status;
        if (cntrl_strct.status != DVR_STATUS_OK)
            sdbg_msg_output_err(DVR_SET_CONTROL, cntrl_strct.status);
    }

    sdbg_msg_output_err(sdvr_enable_encoder_func, err);
    return err;

}
   
/****************************************************************************
  This function is used to get one of:

    An encoded video frame from the encoder

    An encoded audio frame from the encoder

    A raw audio PCM frame

    A motion values frame.

  This function is called by the DVR Application to get a frame of the 
  appropriate type from the SDK. Typically, the DVR Application registers
  a callback so that it can be informed when a frame is available. A callback,
  however, is not required. This function can be called at any time
  by the DVR Application to request a particular frame type. If the frame
  is not available, an error code indicates that no frame
  is available and the buffer pointer is NULL. 

  This is the typical flow of operation in the encoding path:

    DVR Application registers a AV frame callback using 
    sdvr_set_av_frame_callback().

    When this callback function is called, the DVR Application notes
    the channels and the frame types available in a data structure.

    In a separate thread that saves encoded streams or motion values to 
    disk, the DVR application uses the information from the previous 
    step to retrieve the encoded AV frames or motion values and saves 
    them to disk.

    In a separate thread that displays live video or audio, the DVR Application
    uses the information saved earlier to retrieve the raw audio and 
    video frames, and renders them on the display or plays them. 

  When this function is called, an sdvr_av_buffer_t * pointer to a frame is
  returned.  
  The DVR Application should treat this buffer as read-only,
  and not modify any field of the structure. After the buffer is 
  used, it must be released to the SDK using sdvr_release_av_buffer().
  This release should happen as soon as possible because there are limited
  buffers in the SDK and released buffers are used to hold incoming data. 
  If the DVR Application holds a buffer for too long, frame loss could result.

  This function can be called repeatedly to retrieve frames for 
  different channels. It can be called repeatedly for the same channel and
  it will return as many frames as are available before returning 
  an error code indicating no more frames are available.
 
  @What to do with the buffer header information?@
  The sdvr_av_buffer_t structure contains a header in addition to the 
  frame buffer (payload). This header contains information about the 
  size of the frame buffer, the type of the frame, whether motion
  was detected in this frame, and so on. This information may be useful 
  later to search through the stored video file to locate 
  events quickly, e.g., frames where motion happened. We highly
  recommend that you store the header and the frame buffer. We also
  recommended that you save the header in a separate file (perhaps
  with other information) from the frame buffer file, so that you 
  can search the header file to locate
  a particular section of encoded video and then jump to that location
  in the frame buffer file. The reserved field in the 
  buffer data structure need not be saved. Also, the reserved field should 
  not be changed or else sdvr_release_av_buffer() will have trouble recycling
  the buffer.

  Parameters:

    "handle" - An encoding channel handle.

    "frame_type" - The type of frame you want.

      SDVR_FRAME_VIDEO_ENCODED_PRIMARY - For an encoded video
      frame associated with the primary encoder.
 
      SDVR_FRAME_VIDEO_ENCODED_SECONDARY - For an encoded video
      frame associated with the secondary encoder.

      SDVR_FRAME_AUDIO_ENCODED - For an encoded audio frame

      SDVR_FRAME_RAW_AUDIO - For a raw audio PCM frame

      SDVR_FRAME_MOTION_VALUES - For motion values frame.

    "frame_buffer" - The pointer to a pointer to an A/V buffer
    structure. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_av_buffer(sdvr_chan_handle_t handle, 
                              sdvr_frame_type_e frame_type, 
                              sdvr_av_buffer_t **frame_buffer)
{
    sdvr_err_e err;
    ssdk_recv_frame_type_e recv_frame_type;
	sx_uint8 chan_num = sdvr_get_chan_num(handle);
    sx_uint8 board_index = sdvr_get_board_index(handle);

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE ||
         _ssdvr_boards_info.boards[board_index].recv_channels == NULL ||
          chan_num >= _ssdvr_boards_info.boards[board_index].max_num_video_in)
    {
        return SDVR_ERR_INVALID_HANDLE;    // Bad Handle.
    }

    /*****************************************************
        Translate the frame type to queue types of the 
        channel.
        NOTE: RAW video frame are not supported and the 
              caller must call sdvr_get_yuv_buffer()
    ******************************************************/
    switch (frame_type)
    {
    case SDVR_FRAME_AUDIO_ENCODED:
        recv_frame_type = SSDK_RECV_FRAME_TYPE_ENC_AUDIO;

        break;
    case SDVR_FRAME_RAW_AUDIO:
        recv_frame_type =  SSDK_RECV_FRAME_TYPE_RAW_AUDIO;
        break;

    case SDVR_FRAME_VIDEO_ENCODED_PRIMARY:
        if (_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].enc_av_frame_queue[0].size == 0)
            return SDVR_ERR_INVALID_FRAME_TYPE;

        recv_frame_type =  SSDK_RECV_FRAME_TYPE_ENC_PRIMARY;
        break;

    case SDVR_FRAME_VIDEO_ENCODED_SECONDARY:
        if (_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].enc_av_frame_queue[1].size == 0)
            return SDVR_ERR_INVALID_FRAME_TYPE;

        recv_frame_type =  SSDK_RECV_FRAME_TYPE_ENC_SECONDARY;
        break;

    case SDVR_FRAME_MOTION_VALUES:
        recv_frame_type = SSDK_RECV_FRAME_TYPE_MOTION_VALUES;
        break;

    default:
        sdbg_msg_output_err(sdvr_get_av_buffer_func, SDVR_ERR_INVALID_FRAME_TYPE);

        return SDVR_ERR_INVALID_FRAME_TYPE;
    }
    err =  ssdk_dequeue_frame(handle, 
                              (void **)frame_buffer, 
                              recv_frame_type);
    if (err == SDVR_ERR_NONE)
        _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count.eFrameAppRcvd++;

    sdbg_msg_output_err(sdvr_get_av_buffer_func,  err);
    return err;
}
/****************************************************************************
  This function is called by the DVR Application to get a raw video frame   
  from the SDK. Typically, the DVR Application registers a callback so
  that it can be informed when a frame is available. A callback, however,
  is not required. This function can be called at any time
  by the DVR Application to request a particular frame type. If the frame
  is not available, an error code indicates that no frame
  is available and that the buffer pointer is NULL. 

  This is the typical flow of operation in an encoding path:

    The DVR Application registers an AV frame callback using 
    sdvr_set_av_frame_callback().

    When this callback function is called, the DVR Application notes
    the channels and the frame types available in a data structure.

    In a separate thread that displays live video, the DVR Application
    uses the information saved earlier to retrieve the raw 
    video frames and renders them on the display or plays them. 

  When this function is called, an sdvr_yuv_buffer_t pointer to a raw frame is
  returned.  
  The DVR Application should treat this buffer as read-only 
  and not modify any field of the structure. After the buffer is 
  used, it must be released to the SDK using sdvr_release_yuv_buffer().
  This release should happen as soon as possible because there are limited
  buffers in the SDK, and released buffers are used to hold incoming data. 
  If the DVR Application holds a buffer for too long, frame loss could result.

  This function can be called repeatedly to retrieve frames for 
  different channels. It can be called repeatedly for the same channel and
  it will return as many frames that are available before returning 
  an error code indicating no more frames are available.
 
  Parameters:

    "handle" - An encoding or decoding channel handle.

    "frame_buffer" - The pointer to a pointer to a YUV buffer structure. 
    NULL if no YUV buffer is available.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/

sdvr_err_e sdvr_get_yuv_buffer(sdvr_chan_handle_t handle, 
                              sdvr_yuv_buffer_t **frame_buffer)
{
    sdvr_err_e err;
    sx_uint8 chan_num = sdvr_get_chan_num(handle);
    sx_uint8 board_index = sdvr_get_board_index(handle);

    if ((err = ssdk_validate_chan_handle(handle, -1)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_yuv_buffer_func,  err);
        return SDVR_ERR_INVALID_HANDLE;    // Bad Handle.
    }

    err =  ssdk_dequeue_frame(handle, 
                              (void **)frame_buffer, 
                               SSDK_RECV_FRAME_TYPE_RAW_VIDEO);
    if (err == SDVR_ERR_NONE)
        ssdk_get_dbg_frame_count(handle)->yuvFrameAppRcvd++;

    sdbg_msg_output_err(sdvr_get_yuv_buffer_func,  err);

    return err;
}
/****************************************************************************
  This function is used to release an A/V frame to the SDK. 
  Buffers should be released as quickly as possible to prevent 
  frame loss. 

  Decoder buffers obtained from sdvr_alloc_av_buffer() will be released when
  you call sdvr_send_av_frame(). If you don't need to send the
  buffer after it is allocated, you must release the buffer by calling
  this function.

  @NOTE: To release a raw video YUV frame, you must call sdvr_release_yuv_buffer.@

  Parameters:

    "frame_buffer" - The pointer to a buffer obtained using
    sdvr_get_av_buffer() or sdvr_alloc_av_buffer().
    @You should not call this function if frame_buffer was
    already used in sdvr_send_av_frame().@

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_release_av_buffer(sdvr_av_buffer_t *frame_buffer)
{
    sdvr_err_e err;
    sx_uint8 board_index;
    sx_uint8 chan_num;
    sdvr_chan_type_e chan_type;
    sdvr_chan_handle_t handle;

    if ((handle = sdvr_get_buffer_channel(frame_buffer)) == INVALID_CHAN_HANDLE)
        return SDVR_ERR_INVALID_ARG;

    board_index = frame_buffer->board_id;
    chan_num = frame_buffer->channel_id;
    chan_type = frame_buffer->channel_type;

    sdbg_msg_output(sdvr_release_av_buffer_func, frame_buffer->board_id, frame_buffer);

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE)
    {
        return SDVR_ERR_INVALID_ARG;
    }

    // for decoder and output frame we send the buffer to sct with zero length
    // and will not send the buffer but will free it back.
    if (chan_type == SDVR_CHAN_TYPE_DECODER)
    {
        sct_channel_t pci_send_chan_handle;

        pci_send_chan_handle = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_send_chan_handle;
        // send buffer to board and free
        err = sct_buffer_send(pci_send_chan_handle,(void *)frame_buffer, 0);
    }
    else if (chan_type == SDVR_CHAN_TYPE_OUTPUT)
    {
        sct_channel_t pci_send_chan_handle;

        pci_send_chan_handle = _ssdvr_boards_info.boards[board_index].smo_info[chan_num].pci_send_chan_handle;
        // send buffer to board and free
        err = sct_buffer_send(pci_send_chan_handle,(void *)frame_buffer, 0);
    }
    else

    {
        ssdk_get_dbg_frame_count(handle)->eFrameAppFreed++;

        err = ssdk_release_frame(frame_buffer, true);
    }
    sdbg_msg_output_err(sdvr_release_av_buffer_func,err);
    return err;
}

/****************************************************************************
  This function is used to release a YUV frame buffer to the SDK. 
  Buffers should be released as quickly as possible to prevent 
  frame loss. 

  Parameters:

    "frame_buffer" - The pointer to a buffer obtained using
    sdvr_get_yuv_buffer().

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_release_yuv_buffer(sdvr_yuv_buffer_t *frame_buffer)
{
    sdvr_err_e err;
	ssdk_yuv_malloc_item_t *yuv_buffer = (ssdk_yuv_malloc_item_t *)frame_buffer;
    sx_uint8 board_index = frame_buffer->board_id;
    sx_uint8 chan_num = frame_buffer->channel_id;
    sdvr_chan_handle_t handle = sdvr_get_buffer_channel(frame_buffer);

    char msg[256];

	if (yuv_buffer->signiture != 0xFEEDBEEF)
    {
        sprintf(msg,"\nsdvr_release_yuv_buffer(): YUV buffer does not blong to the SDK.");
        sdbg_msg_output(sdvr_dbg_err_msg,board_index,msg);

        return SDVR_ERR_INVALID_ARG;
    }

    if ((err = ssdk_validate_chan_handle(handle, -1)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_release_yuv_buffer_func,err);
        return SDVR_ERR_INVALID_ARG;    // Bad Handle.
    }

    ssdk_get_dbg_frame_count(handle)->yuvFrameAppFreed++;

    sdbg_msg_output(sdvr_release_yuv_buffer_func, board_index, frame_buffer);

    err = ssdk_release_frame(frame_buffer, false);

    sdbg_msg_output_err(sdvr_release_yuv_buffer_func,err);

    return err;
}


