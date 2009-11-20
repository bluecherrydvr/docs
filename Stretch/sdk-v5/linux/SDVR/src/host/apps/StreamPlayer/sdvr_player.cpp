/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * SDVR, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF SDVR, INC.
 *
 ************************************************************************/

#include "sdvr_player.h"
#include "stplayer.h"

CStretchPlayer *players[SDVR_PLAYER_MAX_PORTS];

/**********************************************************************************
    Initialize stretch dvr player.

    @Arguments@:

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_init()
{
	int i;
 
	for (i = 0; i < SDVR_PLAYER_MAX_PORTS; i++) {
		players[i] = NULL;
	}

    return SDVR_PLAYER_NOERROR;
}

/**********************************************************************************
    Release stretch dvr player resources.

    @Arguments@:

    @Returns@: void
**********************************************************************************/
void sdvr_player_release()
{
	int i;
	for (i = 0; i <SDVR_PLAYER_MAX_PORTS; i++) {
		if (players[i] != NULL) {
			delete players[i];
			players[i] = NULL;
		}
	}
}

/**********************************************************************************
    Open the file to play. 

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "filename" - The file name.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_open_file(sx_int32 port, char *filename)
{
    sdvr_player_err_e err;

    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;
   
    if (filename == NULL)
        return SDVR_PLAYER_NULL_POINTER;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer != NULL) {
        err = pPlayer->Close();
        if (err != SDVR_PLAYER_NOERROR)
            return err;
    } else {
        pPlayer = new CStretchPlayer;
        if (pPlayer == NULL)
            return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
	    players[port] = pPlayer;
    }

    return pPlayer->OpenFile(filename);
}

/**********************************************************************************
    Open the file to play. 

    @Arguments@:

        "filename" - The file name.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_open_sdp_file(char *filename)
{
    sdvr_player_err_e err;

    if (filename == NULL)
        return SDVR_PLAYER_NULL_POINTER;

    CStretchPlayer *pPlayer = players[0];
    if (pPlayer != NULL) {
        err = pPlayer->Close();
        if (err != SDVR_PLAYER_NOERROR)
            return err;
    } else {
        pPlayer = new CStretchPlayer;
        if (pPlayer == NULL)
            return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
	    players[0] = pPlayer;
    }

    return pPlayer->OpenSdpFile(filename);
}

/**********************************************************************************
    Close the file that has been opened. 

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_close_file(sx_int32 port)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->Close();
}

/**********************************************************************************
    Start playing. The display window size will be adjusted according to hwnd 
    window when it starts to play.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "hwnd" - The handle to the window where the video is displayed.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_play(sx_int32 port, sdvr_ui_hwnd_t hwnd)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    if (hwnd == NULL)
        return SDVR_PLAYER_NULL_POINTER;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->Play(hwnd);
}

/**********************************************************************************
    Stop playing. The player will rewind to the beginning of the file, and release
    resources allocated in @sdvr_player_play@.  The player will stop updating 
    the window.  To again start playing the file, call @sdvr_player_play@.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_stop(sx_int32 port)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->Stop();
}

/**********************************************************************************
   Pause or resume playing.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "pause" -  If it is true, then pause the playing, else then resume the playing.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_pause(sx_int32 port, sx_bool pause)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->Pause();
}

/**********************************************************************************
    Double the current play speed.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_double_speed(sx_int32 port)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->DoublePlaySpeed();
}

/**********************************************************************************
    Reduce the current play speed by half.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_halve_speed(sx_int32 port)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->HalvePlaySpeed();
}

/**********************************************************************************
    Seek to the relative position of the file.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "relative_pos" - The relative position of the file. It ranges from 0.0 to 1.0.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_set_play_pos(sx_int32 port, float relative_pos)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->SetPlayPos(relative_pos);
}

/**********************************************************************************
    Get current relative position of the file.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "relative_pos" - Current relative position inside the file. It ranges from 0.0 to 1.0.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_play_pos(sx_int32 port, float *relative_pos)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->GetPlayPos(relative_pos);
}

/**********************************************************************************
    Register a windows message to catch the end of file event.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "hwnd" - The handle to the window to receive this message.

        "msg" - The message defined by an application.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_register_file_end_msg(sx_int32 port, sdvr_ui_hwnd_t hwnd, sx_uint32 msg)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->RegisterFileEndMessage(hwnd, msg);
}

/**********************************************************************************
    Get total file time in milliseconds.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "time" - The duration of the file in milliseconds.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_file_time(sx_int32 port, sx_int32 *time)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->GetFileTime(time);
}

/**********************************************************************************
    Advance the player by single frame. Calls @sdvr_player_play@ to play the file normally.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_next_frame(sx_int32 port)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Back up the player by single frame . Calls @sdvr_player_play@ to play the file normally.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_prev_frame(sx_int32 port)
{
   return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Retrieves the total number of frames of the file.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "total_frames" - Total frames of the file.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_total_frames(sx_int32 port, sx_int32 *total_frames)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->GetTotalFrames(total_frames);
}

/**********************************************************************************
    Retrieves the average time per frame in milliseconds.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "avg_time_per_frame" - Average time per frame in milliseconds.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_avg_time_per_frame(sx_int32 port, double *avg_time_per_frame)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->GetAverageTimePerFrame(avg_time_per_frame);
}

/**********************************************************************************
    Retrieves the player state.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "state" - Current player state. See @sdvr_player_state_e@ for supported states.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_state(sx_int32 port, sdvr_player_state_e *state)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->GetState(state);
}

/**********************************************************************************
    Retrieves the current play back rate.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "rate" - Current play back rate.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_play_rate(sx_int32 port, float *rate)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->GetPlayRate(rate);
}


/**********************************************************************************
    Get current file time in milliseconds.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "time" - The current position in the file in milliseconds from the beginning.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_played_time(sx_int32 port, sx_int32 *time)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Specifies the new position in the file in milliseconds from the beginning;

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "time" - The position in the file in milliseconds from the beginning.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_set_current_time(sx_int32 port, sx_int32 time)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
   Retrieves number of frames that have been played.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "frames" - The current position in the file in frames from the beginning.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_played_frames(sx_int32 port, sx_int32 *frames)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
   Specifies the current position in the file in frames from the beginning

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "frame_num" - The current frame number.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_set_current_frame_num(sx_int32 port, sx_int32 frame_num)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Set volume of the sound.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "volume" - New volume requested for this sound, range 0-0XFFFF.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_set_volume(sx_int32 port, sx_uint16 volume)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
	else
        return pPlayer->SetVolume(volume);
}

/**********************************************************************************
   Retrieves the volume of the sound.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "volume" - The volume value.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_volume(sx_int32 port, sx_uint16 *volume)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
	else
        return pPlayer->GetVolume(volume);
}

/**********************************************************************************
    Get the width and height of the video.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "width" - The width of the video.

        "height" - The height of the video.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_picture_size(sx_int32 port, sx_int32 *width, sx_int32 *height)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Search the position of key frame before designated position

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "curr_pos" - The current position, it is either the time in milliseconds or the frame number specified by type.

        "type" - The search type. See @sdvr_player_search_type_e@ for supported search type.

        "frame_pos" - Points to a @sdvr_player_frame_pos_t@ structure to receive the information of a key frame.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_prev_key_frame_pos(sx_int32 port, sx_int32 curr_pos, sdvr_player_search_type_e type, sdvr_player_frame_pos_t *frame_pos)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Search the position of key frame after designated position

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "curr_pos" - The current position, it is either the time in milliseconds or the frame number specified by type.

        "type" - The search type. See @sdvr_player_search_type_e@ for supported search type.

        "frame_pos" - Points to a @sdvr_player_frame_pos_t@ structure to receive the information of a key frame.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_next_key_frame_pos(sx_int32 port, sx_int32 curr_pos, sdvr_player_search_type_e type, sdvr_player_frame_pos_t *frame_pos)
{
   return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Refresh display window.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_refresh_play(sx_int32 port)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->Refresh();
}

/**********************************************************************************
    Play a sound specified by the given channel. It stops the others automatically.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_play_sound(sx_int32 port)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->PlaySound();
}

/**********************************************************************************
    Stop the sound.

    @Arguments@:

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_stop_sound()
{
	// stop all channels' sound
	for (int i=0; i<SDVR_PLAYER_MAX_PORTS; i++) {
		CStretchPlayer *pPlayer = players[i];
		if (pPlayer != NULL)
			pPlayer->StopSound();
	}
	return SDVR_PLAYER_NOERROR;
}

/**********************************************************************************
    Plays a sound specified by the channel with share mode. It doesn¡¯t stop others.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_play_sound_share(sx_int32 port)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->PlaySound();
}

/**********************************************************************************
    Stop a sound specified by the channel. Notice : The player must use the same mode to play or stop the same sound.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_stop_sound_share(sx_int32 port)
{
    if (port >= SDVR_PLAYER_MAX_PORTS)
        return SDVR_PLAYER_INVALID_PORT;

    CStretchPlayer *pPlayer = players[port];
    if (pPlayer == NULL)
        return SDVR_PLAYER_NULL_POINTER;
    else
        return pPlayer->StopSound();
}

/**********************************************************************************
    Open a stream to play. 

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "buf_size" - Specifies the size of the buffer to allocate for storing data for the bitstream to play.  This buffer should be long enough to hold several seconds of encoded video.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_open_stream(sx_int32 port, sx_int32 buf_pool_size)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Send data to the player.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "data_buf" - Pointer to the buffer containing the data to be sent to the player.

        "data_size" - Size of the buffer.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_input_data(sx_int32 port, sx_uint8 *data_buf, sx_int32 data_size)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Read number of bytes currently used out of the buf_size byte-buffer allocated by @sdvr_player_open_stream@.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

        "num_bytes_in_buffer" - Pointer to number of bytes in the buffer that have not yet been consumed.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_get_num_bytes_in_buffer(sx_int32 port, sx_uint32 *num_bytes_in_buffer)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}

/**********************************************************************************
    Close a stream.

    @Arguments@:

        "port" - The channel number of the player. It ranges from 0 to @SDVR_PLAYER_MAX_PORTS@.

    @Returns@: @SDVR_PLAYER_NOERROR@ when successful, error messages otherwise.
**********************************************************************************/
sdvr_player_err_e sdvr_player_close_stream(sx_int32 port)
{
    return SDVR_PLAYER_NOT_IMPLEMENTED;
}



