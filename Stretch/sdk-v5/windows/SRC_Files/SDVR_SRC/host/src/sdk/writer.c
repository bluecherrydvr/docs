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
  This function saves the encoded audio and video frames for any of the
  sub-encoders of the specified encoder channel into the given file name.
  The generated file is in .mov format. 
 
  Calling this function does not affect the current enable state of the encoder.
  After calling this function, you must call sdvr_enable_encoder() to
  enable the subencoder of the given encoder channel. 
  
  Calling this function multiple times consecutively results in writing the 
  audio and video frames into the new file without re-starting the encoder. 
  Hence, there will not be any lose of frames. This function ensures the 
  new recorded file starts with an I-frame. @NOTE: Not implemented in this
  release.@

  Call sdvr_stop_recording() to stop saving of  encoded frames. 
  Additionally, if you need to stop the encoder, you must
  call sdvr_enable_encoder().
 
  @NOTE: This function is not supported and maybe changed in the next
  release.@

  Parameters:

    "handle" - The channel handle for which to record audio and video
    frames.

    "sub_chan_enc" - The encoder subchannel for which to save its encoded audio
    and video frames.

    "file_name" - The full file name path to record audio/video frames.

    @NOTE: This function always truncates the given file before
    recording new audio and video frames.@

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:
 
  Even though this function is called to save the encoded frames,
  the DVR Application is still notified by the callback function as the audio and
  video frames arrive. You can choose to ignore these frames or call
  sdvr_get_av_buffer() to get the frames and perform additional processing.

****************************************************************************/
sdvr_err_e sdvr_start_recording(sdvr_chan_handle_t handle,
                               sdvr_sub_encoders_e sub_chan_enc,
                               char *file_name)
{
    mp4_track_info_t track;
    mp4_info_t info;
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    ssdvr_size_t frameSize;
    sdvr_err_e err;

    ssdk_pci_channel_info_t *enc_chan_info;
    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER)) != SDVR_ERR_NONE ||
        (err = ssdk_validate_enc_sub_chan( handle, sub_chan_enc)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_start_recording_func, err);
        return err;
    }

    if (file_name == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
        sdbg_msg_output_err(sdvr_start_recording_func, err);
        return err;

    }

    enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);
    _ssdk_mutex_chan_rec_lock(handle);

    /*
       Make sure this sub-encoder is not currently in recording.
    */
    if (enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.rec_state != SSDK_REC_STATE_STOP)
    {
        err = SDVR_ERR_CAMERA_IN_REC;
        _ssdk_mutex_chan_rec_unlock(handle);
        sdbg_msg_output_err(sdvr_start_recording_func, err);
        return err;
    }

    info.file_name = file_name;
    info.mux_mode = MP4_RECORD;
    info.file_mode = MODE_MOV;
    enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.fh = mp4mux_open(&info);
    if (enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.fh == 0) {
        err = SDVR_ERR_OPEN_REC_FILE;
        _ssdk_mutex_chan_rec_unlock(handle);
        sdbg_msg_output_err(sdvr_start_recording_func, err);
        return err;
    }

    // Add Video Track to mp4 file
    memset(&track, 0, sizeof(mp4_track_info_t));
    track.type = MP4_TRACK_VIDEO;

//todo: add support for MJPEG
    track.codec_id = (enc_chan_info->sub_enc_info[sub_chan_enc].venc_type == SDVR_VIDEO_ENC_H264) ? CODEC_ID_H264 : CODEC_ID_MPEG4;
    // Timescale is based on 90k Hz clock.
    track.timescale = 90000; 
    frameSize = get_frame_size(_ssdvr_boards_info.boards[board_index].video_std ,enc_chan_info->sub_enc_info[sub_chan_enc].res_decimation);
    track.width = frameSize.width;
    track.height = frameSize.height;
    track.sample_size = 0;
    track.language = 0;
    track.default_duration = ssdk_get_video_rec_duration(_ssdvr_boards_info.boards[board_index].video_std);
    track.default_size = 0; // variable frame size

    if (mp4mux_add_track(enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.fh , &track) == MP4MUX_ERR_OK)
        enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.vtrack = track.track_id;

    else
    {
//        Log::error(QString("Error: failed to add video track for file '%1'.")
//            .arg(fileName));
        mp4mux_close(enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.fh);
        _ssdk_mutex_chan_rec_unlock(handle);
        err = SDVR_ERR_FAILED_ADD_VIDEO_TRACK;
        sdbg_msg_output(sdvr_dbg_err_msg,board_index,"Error: failed to add video track for file\n");
//        sdbg_msg_output_err(sdvr_start_recording_func, err);
        return err;
    }

    // Add audio Track to mp4 file if this channel has audio.
    // NOTE: Only the primary sub-encoder has audio.
    if (enc_chan_info->aenc_type != SDVR_AUDIO_ENC_NONE &&
        sub_chan_enc == SDVR_ENC_PRIMARY)
    {
        sx_int8  mp4mux_lang[4] = {'e', 'n', 'g', '\0'};
        memset(&track, 0, sizeof(mp4_track_info_t));
        track.type = MP4_TRACK_AUDIO;
        track.codec_id = CODEC_ID_PCM_MULAW;
        // Audio sampling rate
        if (_ssdvr_boards_info.boards[board_index].audio_rate == SDVR_AUDIO_RATE_16KHZ)
             track.timescale = 16000;
        else
            track.timescale =  8000;; 

        track.channels = 2; // a stereo recording
        track.sample_size = 1 * track.channels;
        track.default_duration = 2560 / track.channels; // packet size in bytes / channels
        track.default_size = 1 * track.channels; // one byte per channel for PCM

        track.language = mp4mux_iso639_to_lang(mp4mux_lang, 1);

        // add track
        if (mp4mux_add_track(enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.fh, &track) == MP4MUX_ERR_OK)
            enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.atrack = track.track_id;

        else
        {
//            Log::error(QString("Error: failed to add audio track for file '%1'.")
//                .arg(fileName));
        mp4mux_close(enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.fh);

        _ssdk_mutex_chan_rec_unlock(handle);
        err = SDVR_ERR_FAILED_ADD_AUDIO_TRACK;
        sdbg_msg_output_err(sdvr_start_recording_func, err);
        return err;
        }
    }

    enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.rec_state = SSDK_REC_STATE_START;
    _ssdk_mutex_chan_rec_unlock(handle);

    return SDVR_ERR_NONE;
}

/****************************************************************************
  Call this function to stop saving encoded audio and video frames for
  the subencoder of the specified encoder channel that is recording.
 
  Calling this function does not affect the current enable state of the encoder.
  After calling this function, you must call sdvr_enable_encoder() to
  stop the subencoder of the given encoder channel.

  @NOTE: This function is not supported and maybe changed in the next
  release.@

  Parameters:

    "handle" - The channel handle for which to stop recording audio/video
    frames.

    "sub_chan_enc" - The encoder subchannel to stop recording.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

****************************************************************************/
sdvr_err_e sdvr_stop_recording(sdvr_chan_handle_t handle,
                               sdvr_sub_encoders_e sub_chan_enc)
{
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sdvr_err_e err;

    ssdk_pci_channel_info_t *enc_chan_info;
    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER)) != SDVR_ERR_NONE ||
        (err = ssdk_validate_enc_sub_chan( handle, sub_chan_enc)) != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_stop_recording_func, err);
        return err;
    }
    _ssdk_mutex_chan_rec_lock(handle);

    enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    /*
	 Check if there is any recording for the given sub-encoder.
    */
    if (enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.rec_state != SSDK_REC_STATE_START)
    {
        _ssdk_mutex_chan_rec_unlock(handle);
        return SDVR_ERR_NONE;
    }

    enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.rec_state = SSDK_REC_STATE_STOP_PENDING;

    // Close the recorded file.
    mp4mux_close(enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.fh);

    memset(&enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info,0,
        sizeof(enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info));

    enc_chan_info->sub_enc_info[sub_chan_enc].mp4_file_info.rec_state = SSDK_REC_STATE_STOP;

    _ssdk_mutex_chan_rec_unlock(handle);

    return SDVR_ERR_NONE;
}

/****************************************************************************
    This function writes the given video frame to a .mov file if recording is
    enabled for the corresponding sub-encoder of the given encoder handle.
    
    NOTE: This function currently only saves MPEG4 and H.264 frames.
****************************************************************************/
sdvr_err_e ssdvr_write_Enc_VFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf)
{
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sdvr_err_e err;
    mp4_packet_t pkt_v;
    ssdk_pci_channel_info_t *enc_chan_info;

    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER)) != SDVR_ERR_NONE ||
        (err = ssdk_validate_enc_sub_chan( handle, buf->stream_id)) != SDVR_ERR_NONE)
    {
        return err;
    }
    _ssdk_mutex_chan_rec_lock(handle);

    enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    /*
        Check to see if we need to record the video frames
        for this channel into a .mov file.
    */
    if (enc_chan_info->sub_enc_info[buf->stream_id].mp4_file_info.rec_state != SSDK_REC_STATE_STOP)
    {
        int err;
        sx_uint64 *ptr64TS;

        memset(&pkt_v, 0, sizeof(mp4_packet_t));
        pkt_v.data = buf->payload;
        pkt_v.size = buf->payload_size;
        pkt_v.sample_count = 1;      // set always to 1

        /* 
           The 64 bit timestamp only exist in 3.2.x.x or higher of firmware 
        */
        if (ssdk_is_min_version(_ssdvr_boards_info.boards[board_index].version.fw_major,
            _ssdvr_boards_info.boards[board_index].version.fw_minor,
            _ssdvr_boards_info.boards[board_index].version.fw_revision,
            _ssdvr_boards_info.boards[board_index].version.fw_build,
            3,2,0,0))
        {
            ptr64TS = (sx_uint64 *)&buf->timestamp;
            pkt_v.pts = pkt_v.dts =  *ptr64TS; //buf->timestamp;  //(*i).mp4File.dts;
        }
        else
        {
            pkt_v.pts = pkt_v.dts =  buf->timestamp;  //(*i).mp4File.dts;
        }


        if (enc_chan_info->sub_enc_info[buf->stream_id].venc_type == SDVR_VIDEO_ENC_H264)
        {
            pkt_v.flags = (buf->frame_type == SDVR_FRAME_H264_IDR) ||
                      (buf->frame_type == SDVR_FRAME_H264_I);
        }
        else
        {
            pkt_v.flags = (buf->frame_type == SDVR_FRAME_MPEG4_I);
        }
        err = mp4mux_put_packet(enc_chan_info->sub_enc_info[buf->stream_id].mp4_file_info.fh, 
                    enc_chan_info->sub_enc_info[buf->stream_id].mp4_file_info.vtrack, &pkt_v);
        if (err != MP4MUX_ERR_OK)
        {
//                    Log::error(QString("Error: Failed to add a video packet into mov file for camera %1. Error = [%2]")
//                        .arg((*i).camera->name()).arg(err));
        }
    } //if (enc_chan_info->sub_enc_info[buf->stream_id].mp4_file_info.rec_state != SSDK_REC_STATE_STOP)
    else
        err = SDVR_ERR_NONE;

    _ssdk_mutex_chan_rec_unlock(handle);

    return err;
}

/*
    This function writes the given audio frame to .mov file if recording is
    enabled for the corresponding sub-encoder of the given encoder handle.
    
    NOTE: The audio will only be save for the primary encoder and only for
    H.264 and MPEG4
*/

sdvr_err_e ssdvr_write_Enc_AFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf)
{
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sdvr_err_e err;
    mp4_packet_t pkt_a;
    ssdk_pci_channel_info_t *enc_chan_info;

    if ((err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER)) != SDVR_ERR_NONE ||
        (err = ssdk_validate_enc_sub_chan( handle, buf->stream_id)) != SDVR_ERR_NONE)
    {
        return err;
    }
    _ssdk_mutex_chan_rec_lock(handle);

    enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    /*
        Check to see if we need to record the video frames
        for this channel into a .mov file. 
        Note: The audio can only be save on the primary stream

        NOTE: TODO: Currently we are saving audio frame only with
              h.264 & MPEG4 video frames. The audio will be ignored for
              Any other video codec types.

    */
    if (enc_chan_info->sub_enc_info[buf->stream_id].mp4_file_info.rec_state != SSDK_REC_STATE_STOP &&
        buf->stream_id == SDVR_ENC_PRIMARY && 
        (enc_chan_info->sub_enc_info[buf->stream_id].venc_type == SDVR_VIDEO_ENC_H264 ||
        enc_chan_info->sub_enc_info[buf->stream_id].venc_type == SDVR_VIDEO_ENC_MPEG4))
    {
        sdvr_aenc_e  aenc = SDVR_AUDIO_ENC_G711;
        sx_uint64 * ptr64TS;

        err = SDVR_ERR_NONE;

        switch (enc_chan_info->aenc_type) 
        {
        case SDVR_AUDIO_ENC_G711:
        {
            memset(&pkt_a, 0, sizeof(mp4_packet_t));
            pkt_a.data = buf->payload;
            pkt_a.size = buf->payload_size;


            /* 
               The 64 bit timestamp only exist in 3.2.x.x or higher of firmware 
            */
            if (ssdk_is_min_version(_ssdvr_boards_info.boards[board_index].version.fw_major,
                _ssdvr_boards_info.boards[board_index].version.fw_minor,
                _ssdvr_boards_info.boards[board_index].version.fw_revision,
                _ssdvr_boards_info.boards[board_index].version.fw_build,
                3,2,0,0))
            {
                ptr64TS = (sx_uint64 *)&buf->timestamp;
                pkt_a.pts = pkt_a.dts =  *ptr64TS; //buf->timestamp; 
            }
            else
            {
                pkt_a.pts = pkt_a.dts =  buf->timestamp;  
            }

            // The payload contains 2560 samples of 8 bytes. The payload_size
            // was already re-asjusted in the SDK when it was eNcoded.
            pkt_a.sample_count = buf->payload_size;

            pkt_a.flags = 0;        // This field is unused for audio.
            err = mp4mux_put_packet(enc_chan_info->sub_enc_info[buf->stream_id].mp4_file_info.fh, 
                    enc_chan_info->sub_enc_info[buf->stream_id].mp4_file_info.atrack, &pkt_a);
            if (err != MP4MUX_ERR_OK)
            {
//                    Log::error(QString("Error: Failed to add a video packet into mov file for camera %1. Error = [%2]")
//                        .arg((*i).camera->name()).arg(err));
            }

            break;
        }

        case SDVR_AUDIO_ENC_NONE:
        default:
            // err = not supported
            break;
        }
	}
    else
        err = SDVR_ERR_NONE;

    _ssdk_mutex_chan_rec_unlock(handle);
    return err;
}
