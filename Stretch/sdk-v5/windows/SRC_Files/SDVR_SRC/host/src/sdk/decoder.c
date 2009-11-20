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

#ifdef writeyuv
static FILE *fp = NULL;
#endif
static sdvr_err_e _ssdvr_enable_decoder(sdvr_chan_handle_t handle,
                                      sx_bool enable);
/****************************************************************************
  This function enables decoding for the given decoder channel type.

  Parameters:

    "handle" - A decoder channel handle.

    "enable" - If true, the decoding is enabled, and disabled otherwise.

  Returns:

    SDVR_ERR_NONE - On success.

    SDVR_ERR_BOARD_NOT_CONNECTED - Error code if the board was never connected.

    SDVR_ERR_INVALID_BOARD - Error code if the given handle is not valid
    because the board index is bad.

    SDVR_ERR_ENCODER_NOT_ENABLED - Failed to start the encoder on the
    given channel.

    SDVR_ERR_ENCODER_NOT_DISABLED - Failed to stop the encoder on the
    given channel.

    SDVR_ERR_SDK_NO_FRAME_BUF - There is not enough buffer allocated to
    received encoded and frame buffers.
****************************************************************************/
sdvr_err_e _ssdvr_enable_decoder(sdvr_chan_handle_t handle,
                                 sx_bool enable)
{
    dvr_control_t cntrl_strct;
    dvr_decode_info_t decode_strct;

    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = sdvr_get_chan_num(handle);
    char msg[256];
    int size = sizeof(decode_strct);

    memset(&cntrl_strct,0,sizeof(cntrl_strct));

    decode_strct.job_type =  cntrl_strct.job_type = CH_GET_CHAN_TYPE(handle);
    decode_strct.job_id =  cntrl_strct.job_id = CH_GET_CHAN_NUM(handle);

    /************************************************************************
            Get the existing decoder attributes so that we won't overwrite
            them when enable/disabling the decoder.
    ************************************************************************/

    ssdk_message_chan_send(handle, DVR_GET_DECODE, &decode_strct, sizeof(decode_strct));
    ssdk_message_chan_recv(handle, DVR_REP_DECODE, &decode_strct, 0, 0);
    if (decode_strct.status != DVR_STATUS_OK)
    {
        sdbg_msg_output_err(DVR_GET_DECODE, decode_strct.status);
        return decode_strct.status;
    }

    /***********************************************************************
            NOTE: No attributes of the channel can be changed while the
                  channel is running. In this case, we must temp. disable
                  the channel, make the change and re-enable it. The
                  channel is running if either the decoder, SMO, or HMO
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
         Set the new enable state of the decoder and send the command to the firmware
    *****************************************************************************/
    decode_strct.enable = enable;
    decode_strct.width =
      _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vsize_width;

    decode_strct.height =
     _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vsize_lines;

    ssdk_message_chan_send(handle, DVR_SET_DECODE, &decode_strct, sizeof(decode_strct));
    ssdk_message_chan_recv(handle, DVR_REP_DECODE, &decode_strct, 0, 0);
    sprintf(msg,"DVR_SET_DECODE: board_index = %d chan_num = %d enable = %d status = %s\n",
        board_index,decode_strct.job_id, enable, sdvr_get_error_text(decode_strct.status));
    sdbg_msg_output(sdvr_dbg_msg, board_index, (void *)msg);

	if (decode_strct.status == SDVR_ERR_NONE)
        _ssdvr_boards_info.boards[board_index].decode_channels[decode_strct.job_id].is_dec_av_enable = enable;

    /***************************************************************************
      Re-start the channle if the decoder is enabled.
      NOTE: This behavior is different than the encoder channels which checks
      to see if SMO or HMO are running and starts the encoder.
    ****************************************************************************/
    if (decode_strct.enable && decode_strct.status == SDVR_ERR_NONE)
    {
#ifdef writeyuv
        fp = fopen("c:\\decoder.264","wb");
#endif
        cntrl_strct.enable = true;
        ssdk_message_chan_send(handle, DVR_SET_CONTROL, &cntrl_strct, sizeof(cntrl_strct));
        ssdk_message_chan_recv(handle, DVR_REP_CONTROL, &cntrl_strct, 0, 0);
        if (cntrl_strct.status != DVR_STATUS_OK)
            sdbg_msg_output_err(DVR_SET_CONTROL, cntrl_strct.status);
    }

#ifdef writeyuv
    if (!enable && fp != NULL)
        fclose(fp);
#endif
    sdbg_msg_output(sdvr_enable_decoder_func, board_index, (void *)&cntrl_strct);

    return cntrl_strct.status;
}

/****************************************************************************
  This function sets the encoded video frame size to be decoded for the given
  decoder channel type.

  You must call this function before calling sdvr_enable_decoder() to enable
  the decoder. Otherwise, the encoded video size is assumed to be the size
  of the current video standard at full resolution.

  You do not need to call this function multiple times prior to enabling the
  decoder if the video frame size has not changed.

  Parameters:

    "handle" - A decoder channel handle.

    "width" - Width in pixels of the encoded frames.

    "lines" - Number of lines in pixels of the encoded frames.

    @NOTE@: Width and lines combinations must match one of the supported
    video resolutions.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

  If you call this function while the decoder is enabled, the change
  takes affect after the decoder is re-enabled.
****************************************************************************/
sdvr_err_e sdvr_set_decoder_size(sdvr_chan_handle_t handle,
                        sx_uint16 width, sx_uint16 lines)
{

    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = sdvr_get_chan_num(handle);

    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_DECODER)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_set_decoder_size_func, err);
        return err;
    }

    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vsize_width = width;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vsize_lines = lines;

    sdbg_msg_output(sdvr_set_decoder_size_func, board_index, (void *)&handle);

    return err;
}
/****************************************************************************
  This function enables decoding using a particular channel. This function
  must be called every time a new playback session is started and ended.

  Each decoded video frames can either be displayed on the SMO by calling
  sdvr_set_smo_grid() or requested to be sent to the DVR Application for
  displaying on the host monitor by calling sdvr_stream_raw_video().

  We recommend to call sdvr_set_smo_grid() and sdvr_stream_raw_video()
  before enabling the decoder so that no decoded video is lost.

  Parameters:

    "handle" - A decoding channel handle.

    "enable" - If true, then decoding is enabled; otherwise, disabled.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_enable_decoder(sdvr_chan_handle_t handle,
                               sx_bool enable)
{
    sdvr_venc_e video_codec;
    sdvr_err_e err;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_DECODER)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_enable_decoder_func, err);
        return err;
    }

    sdbg_msg_output(sdvr_enable_decoder_func, board_index, (void*)&handle);

    /*****************************************************************************
        First we must get the video codec type of the channel to decide
        which video encoder structure to use
    *****************************************************************************/

    if ((err = ssdk_get_chan_av_codec(handle,0, &video_codec, 0)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_video_encoder_channel_params_func, err);
        return err;
    }

    /***************************************************************************
        Enable the decoding for the given channel if it is a decoder channel.
    ****************************************************************************/
    switch (video_codec)
    {
    case SDVR_VIDEO_ENC_H264:
    case SDVR_VIDEO_ENC_JPEG:

        err = _ssdvr_enable_decoder(handle, enable);
        break;
    default:
        // This is a HMO channel, there are no decoding parameters
        err = SDVR_ERR_WRONG_CHANNEL_TYPE;
        break;
    }
    sdbg_msg_output_err(sdvr_enable_decoder_func, err);

    return err;
}

/****************************************************************************
  This function is used to get a buffer from the SDK. To decode video, the
  DVR Application first needs to call this function to get an empty buffer.
  The application then copies encoded video from the disk into this buffer.
  Then it can be sent to the board for decoding using
  sdvr_send_av_frame().

  Each buffer can hold one encoded AV frame. For normal speed playback, the
  DVR Application needs to send 30 frames per second to the decoder.

  The decoded data can be retrieved from the board using sdvr_get_av_buffer()
  using the channel number that was used for decoding.

  Buffers allocated this way should be returned as quickly
  as possible to the SDK, so that the decoder is not starved of data.

  The returned buffer has the board and channel numbers, and
  other information embedded in the buffer data structure. This information
  has to be treated as read-only. Only the "payload" field in the
  sdvr_av_buffer_t structure can be modified by the DVR Application.

  @How to fill up the AV buffer?@ The buffer returned by this function
  is of type sdvr_av_buffer_t and has a header and location for the frame
  buffer (payload). The frame buffer can be filled with the encoded data
  from disk. If you stored the header for the encoded buffer (see the description
  of sdvr_get_av_buffer()), then you can copy the header into the
  header for this buffer. You have to be careful, however, not to overwrite
  the reserved field of the buffer returned by this function, as the
  reserved field is used by the SDK when you call sdvr_send_av_frame().

  @NOTE: The board index, channel number, and channel types will be
  set to match the given channel handle upon successful return.@

  Parameters:

    "handle" - A decoding channel handle.

    "frame_buffer" - A pointer to the buffer variable that is
    set when this function returns.

    "timeout" - Wait for this number of milliseconds for an available
    buffer before timing out.

  Returns

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_alloc_av_buffer_wait(sdvr_chan_handle_t handle,
        sdvr_av_buffer_t **frame_buffer, sx_uint32 timeout)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sct_channel_t pci_send_chan_handle;
    sdvr_err_e err;

    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_DECODER)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_alloc_av_buffer_func, err);
        return err;
    }

    pci_send_chan_handle = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_send_chan_handle;
    if (pci_send_chan_handle == 0)
    {
        sdbg_msg_output_err(sdvr_alloc_av_buffer_func, SDVR_ERR_CHANNEL_CLOSED);
        return SDVR_ERR_CHANNEL_CLOSED;

    }

    /************************************************************
        get the memory from sct and setup the frame header.
    *************************************************************/
    *frame_buffer = (sdvr_av_buffer_t *) sct_buffer_alloc_wait(
            pci_send_chan_handle, timeout);
    if (*frame_buffer != NULL)
    {
        (*frame_buffer)->signature = SDVR_DATA_HDR_SIG;
        (*frame_buffer)->board_id = board_index;
        (*frame_buffer)->channel_id = chan_num;
        (*frame_buffer)->channel_type = chan_type;
        (*frame_buffer)->hdr_size = sizeof(dvr_data_header_t);

        _ssdk_mutex_decode_buf_lock();

        if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].avail_decode_buf_count > 0)
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].avail_decode_buf_count--;

        _ssdk_mutex_decode_buf_unlock();

        err = SDVR_ERR_NONE;
    }
    else
        err = SDVR_ERR_BUF_NOT_AVAIL;
    return err;
}

/****************************************************************************
  Convenience function equivalent to calling sdvr_alloc_av_buffer_wait with
  timeout set to 0. Returns immediately if no empty buffer available.

  Parameters:

    "handle" - A decoding channel handle.

    "frame_buffer" - A pointer to the buffer variable that is
    set when this function returns.

  Returns

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_alloc_av_buffer(sdvr_chan_handle_t handle,
        sdvr_av_buffer_t **frame_buffer)
{
    return sdvr_alloc_av_buffer_wait(handle, frame_buffer, 0);
}

/****************************************************************************
  The buffer obtained using sdvr_alloc_av_buffer()
  is filled with encoded data by the DVR Application, and then the
  buffer is sent to the decoder for decoding using this function.

  NOTE: Sending the buffer for decoding also implicitly returns the
  buffer to the SDK so that it can be used again in a future call to
  allocate a buffer.

  Parameters:

    "frame_buffer" - Pointer to the buffer to be sent. The channel
    number, board number, and so on are already a part of the
    buffer data structure.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_send_av_frame(sdvr_av_buffer_t *frame_buffer)
{
    sx_uint8 board_index;
    sx_uint8 chan_type;
    sx_uint8 chan_num;
    sdvr_chan_handle_t  handle;
//    sx_int32 header_size;
    sct_channel_t pci_send_chan_handle;
    sdvr_err_e err;
    void * buf = &frame_buffer->payload[0];

    if ((handle = sdvr_get_buffer_channel(frame_buffer)) == INVALID_CHAN_HANDLE ||
        frame_buffer->channel_type != SDVR_CHAN_TYPE_DECODER)
        return SDVR_ERR_INVALID_ARG;

    board_index = frame_buffer->board_id;
    chan_num = frame_buffer->channel_id;
    chan_type = frame_buffer->channel_type;
//    header_size = (sx_uint8 *)(buf) - (sx_uint8*)(frame_buffer);
    //header_size = sizeof(sdvr_av_buffer_t) - sizeof(&frame_buffer->payload[0]);

    if (chan_type != SDVR_CHAN_TYPE_DECODER)
        return SDVR_ERR_WRONG_CHANNEL_TYPE;

    if (!_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_dec_av_enable)
        return SDVR_ERR_DECODER_NOT_ENABLED;

    pci_send_chan_handle = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_send_chan_handle;
    // send buffer to board and remove it from the availabe buffer list.
#ifdef writeyuv
    if (fp)
        fwrite(buf, frame_buffer->payload_size, 1, fp);
    err = sct_buffer_send(pci_send_chan_handle,(void *)frame_buffer,0);
#else
    err = sct_buffer_send(pci_send_chan_handle,(void *)frame_buffer,
        frame_buffer->payload_size + frame_buffer->hdr_size);
#endif

    return err;
}

/****************************************************************************
  This function returns the number of send buffers that are still
  available for the given decoder channel. You call this function
  to determine whether to call sdvr_alloc_av_buffer() and get
  a  buffer to send for decoding.

  You should not disable the decoder if this number is not the maximum
  number of available buffers as is defined in the SDK parameters, since
  this means the DVR firmware is not finished with decoding all the frames.

  Parameters:

    "handle" - A decoding channel handle.

  Returns:

    -1 - If the channel handle is invalid.
    Otherwise, the number of availabe decode send buffers.
****************************************************************************/
int sdvr_avail_decoder_buf_count(sdvr_chan_handle_t handle)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_err_e err;
    int buf_count;

    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_DECODER)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_avail_decoder_buf_count_func, err);
        return -1;
    }

    if (chan_type != SDVR_CHAN_TYPE_DECODER)
    {
        sdbg_msg_output_err(sdvr_avail_decoder_buf_count_func, SDVR_ERR_WRONG_CHANNEL_TYPE);
        return  -1;

    }

    _ssdk_mutex_decode_buf_lock();

    buf_count = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].avail_decode_buf_count;

    _ssdk_mutex_decode_buf_unlock();

    return buf_count;
}


