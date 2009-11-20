/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/

/************************************************************************
    PACKAGE: sdvr_player -- Stretch media player API.

    DESCRIPTION:

    SECTION: Include
    {
    #include "sdvr_player.h"
    }

    SECTION: Usage

	The Stretch Media Player can play audio/video files and streams comprised of 
	sdvr_av_buffer packets obtained from the Stretch DVR SDK.  It can also play 
	.mov files generated from sdvr_av_buffer packets using example code in the 
	Stretch DVR SDK.

	The player supports up to SDVR_PLAYER_MAX_PORTS of audio and video display.  
	Each port represents either a file or a data stream, and they operate in parallel.  
	The player supports simultaneous display of multiple video channels in multiple windows.  
	It also supports playing audio both pre-emptively, where only one of these ports may 
	play audio at a time, or shared, where the audio channels share the output.

	Before using any other API functions, an application must initialize the player with 
	sdvr_player_init().  Ports may then be initialized in two ways: for file mode, 
	sdvr_player_open_file() associates a port with a file; for streaming mode, 
	sdvr_player_open_stream() to inititialize the decoding.  Once it has initialized a port,
	the application calls sdvr_player_play() with a window handle to specify where to display the video.

	When playing from file mode, the application can call various functions to pause, seek, 
	and back up in the video stream.  When playing a stream, the player displays frames and 
	then deletes the buffers.  Players in stream mode thus have fewer features available, 
	although they can still adjust the playback speed and pause video.

	In file mode, the player stops when it reaches the end of the file, and it may call back 
	the application when this event occurs.  Playback ends in file mode when 
	sdvr_player_close_file() is called for a port playing a file, and sdvr_close_stream 
	for a streaming port.

    SUBSECTION: Pseudo code

	#include "sdvr_player.h"

	#define NETWORK_PACKET_SIZE 65536
	#define FILE_PORT 1
	#define STREAM_PORT 2
	#define STREAM_BIT_RATE 2000000

	int main()
	{
		unsigned char *bufferFromNetwork;
		int bufferSize;
		sdvr_ui_hwnd_t filePlayWindow, networkPlayWindow;
		 //  Initialization
		sdvr_player_init();
		bufferFromNetwork = malloc(NETWORK_PACKET_SIZE);
		//  [Create windows]
		 //  Open one file port and one stream port
		sdvr_player_open_file(FILE_PORT, "myfile.mov");
		sdvr_player_open_stream(STREAM_PORT, (STREAM_BIT_RATE / 8) * 10);
		 //  Start playing both channels
		sdvr_player_play(FILE_PORT, filePlayWindow);
		sdvr_player_play(STREAM_PORT, networkPlayWindow);
		while(wantToKeepPlaying)
		{
			int numBytesInBuffer;
			network_receive_frame(bufferFromNetwork, &bufferSize);
			//   Wait for buffer to have space for new data
			while(sdvr_player_get_num_bytes_in_buffer(STREAM_PORT, &numBytesInBuffer) + bufferSize < (STREAM_BIT_RATE / 8) * 10);
			//  Add data to player
			sdvr_player_stream_input_data(STREAM_PORT, bufferFromNetwork, bufferSize);
		}   
		 //  Stop playing
		sdvr_player_close_file(FILE_PORT);
		sdvr_player_close_stream(STREAM_PORT);
		sdvr_player_release();
		free(bufferFromNetwork);
		return 0;
	}
		
    SECTION: Special notes

    The current player API only works on Windows platform.  
************************************************************************/

#ifndef __SDVR_PLAYER_H__
#define __SDVR_PLAYER_H__

#include "sdvr_ui_sdk.h"

/**********************************************************************************
    Maximum number of player ports supported.
**********************************************************************************/
#define SDVR_PLAYER_MAX_PORTS   64

/**********************************************************************************
    SDVR player state.

    @SDVR_PLAYER_EMPTY@ - No file or stream has been opened.

    @SDVR_PLAYER_READY@ - Player is ready to play.

    @SDVR_PLAYER_PLAY@ - Player is playing.

    @SDVR_PLAYER_STOP@ - Player is stopped.

    @SDVR_PLAYER_PAUSE@ - Player is paused.
**********************************************************************************/
typedef enum 
{
    SDVR_PLAYER_EMPTY = 0,                  
    SDVR_PLAYER_READY,                      
    SDVR_PLAYER_PLAY,                       
    SDVR_PLAYER_STOP,                       
    SDVR_PLAYER_PAUSE                       
} sdvr_player_state_e;

/**********************************************************************************
    SDVR player frame search type.

    @SDVR_PLAYER_SEARCH_BY_FRAME_NUM@ - Search by frame number.

    @SDVR_PLAYER_SEARCH_BY_FRAME_TIME@ - Search by frame time in milli-second.
**********************************************************************************/
typedef enum {
    SDVR_PLAYER_SEARCH_BY_FRAME_NUM = 0,     
    SDVR_PLAYER_SEARCH_BY_FRAME_TIME         
} sdvr_player_search_type_e;

/**********************************************************************************
    SDVR player error code. 

    @SDVR_PLAYER_NOERROR@ - No error, operation succeeded.

    @SDVR_PLAYER_NULL_POINTER@ - Null pointer.

    @SDVR_PLAYER_ALLOC_MEMORY_ERROR@ - Memory allocation failed.

    @SDVR_PLAYER_NOT_IMPLEMENTED@ - Feature not implemented.

    @SDVR_PLAYER_INVALID_PARAM@ - Invalid input parameter.

    @SDVR_PLAYER_INVALID_PORT@ - Invalid port number.

    @SDVR_PLAYER_NOT_READY@ - Player is not ready to play.

    @SDVR_PLAYER_THREAD_ERROR@ - Create thread failed.

    @SDVR_PLAYER_SUPPORT_FILE_ONLY@ - Operation supported in file mode only.

    @SDVR_PLAYER_SUPPORT_STREAM_ONLY@ - Operation supported in streaming mode only.

    @SDVR_PLAYER_FILE_NOT_EXIST@ - Input file does not exist.

    @SDVR_PLAYER_FILE_LOAD_ERROR@ - Load file failed.

    @SDVR_PLAYER_READ_FILE_ERROR@ - Read file failed.

    @SDVR_PLAYER_FILE_NO_VIDEO@ - Input file does not have video.

    @SDVR_PLAYER_FILE_NO_AUDIO@ - Input file does not have audio.
    
    @SDVR_PLAYER_UNKNOWN_FRAME_TYPE@ - Unknown frame type.

    @SDVR_PLAYER_CREATE_DDRAW_ERROR@ - Create directdraw object failed.

    @SDVR_PLAYER_CREATE_OFFSCREEN_ERROR@ - Create offscreen surface failed.

    @SDVR_PLAYER_LOCK_SURFACE_ERROR@ - Lock surface failed.

    @SDVR_PLAYER_BLT_ERROR@ - Blt block transfer failed.

    @SDVR_PLAYER_DRAW_FRAME_ERROR@ - Draw frame failed. 

    @SDVR_PLAYER_VIDEO_DECODER_INIT_ERROR@ - Video decoder init failed.

    @SDVR_PLAYER_VIDEO_DEC_ERROR@ - Video frame decoding failed.

    @SDVR_PLAYER_AUDIO_DECODER_INIT_ERROR@ - Audio decoder init failed.

    @SDVR_PLAYER_AUDIO_DEC_ERROR@ - Audio packet decoding failed.

    @SDVR_PLAYER_SPLITTER_ERROR@ - Splitter error.

    @SDVR_PLAYER_AUDIO_RENDERER_ERROR@ - Audio renderer error.
**********************************************************************************/
typedef enum 
{
    SDVR_PLAYER_NOERROR                     = 0x0000,   // no error, operation succeeded.
    SDVR_PLAYER_NULL_POINTER,                           // null pointer.
    SDVR_PLAYER_ALLOC_MEMORY_ERROR,                     // memory allocation failed.
    SDVR_PLAYER_NOT_IMPLEMENTED,                        // feature not implemented.
    SDVR_PLAYER_INVALID_PARAM,                          // invalid input parameter.
    SDVR_PLAYER_INVALID_PORT,                           // invalid port number.
    SDVR_PLAYER_UNEXPECTED,                             // unexpected error.
    SDVR_PLAYER_NOT_READY,                              // player is not ready to play.
    SDVR_PLAYER_THREAD_ERROR,                           // create thread failed.
    SDVR_PLAYER_SUPPORT_FILE_ONLY,                      // operation supported in file mode only.
    SDVR_PLAYER_SUPPORT_STREAM_ONLY,                    // operation supported in streaming mode only.
    SDVR_PLAYER_FILE_NOT_EXIST,                         // input file does not exist.
    SDVR_PLAYER_FILE_LOAD_ERROR,                        // load file failed.
    SDVR_PLAYER_READ_FILE_ERROR,                        // read file failed.
    SDVR_PLAYER_FILE_NO_VIDEO,                          // input file does not have video.
    SDVR_PLAYER_FILE_NO_AUDIO,                          // input file does not have audio.
    SDVR_PLAYER_UNKNOWN_FRAME_TYPE,                     // unknown frame type.
    SDVR_PLAYER_CREATE_DDRAW_ERROR,                     // create directdraw object failed.
    SDVR_PLAYER_CREATE_OFFSCREEN_ERROR,                 // create offscreen surface failed.
    SDVR_PLAYER_LOCK_SURFACE_ERROR,                     // lock surface failed.
    SDVR_PLAYER_BLT_ERROR,                              // blt transfer failed.
    SDVR_PLAYER_DRAW_FRAME_ERROR,                       // draw frame failed. 
    SDVR_PLAYER_VIDEO_DECODER_INIT_ERROR,               // video decoder init failed.
    SDVR_PLAYER_VIDEO_DEC_ERROR,                        // video frame decoding failed.
    SDVR_PLAYER_CREATE_DSOUND_ERROR,                    // create directsound object failed.
    SDVR_PLAYER_DSOUND_NOT_INITIALIZED,                 // directsound object has not been initialized.
    SDVR_PLAYER_RESTORE_DSOUND_BUFFER_ERROR,            // restore directsound buffer failed.
    SDVR_PLAYER_LOCK_DSOUND_BUFFER_ERROR,               // lock directsound buffer failed.
    SDVR_PLAYER_PLAY_DSOUND_ERROR,                      // play directsound failed.
    SDVR_PLAYER_GET_DSOUND_POSITION_ERROR,              // get directsound position failed.
    SDVR_PLAYER_AUDIO_DECODER_INIT_ERROR,               // audio decoder init failed.
    SDVR_PLAYER_AUDIO_DEC_ERROR,                        // audio packet decoding failed.
	SDVR_PLAYER_RTP_ERROR,								// Error in RTP streaming stack

} sdvr_player_err_e;

/**********************************************************************************
    SDVR player frame position.

    @pos@ - Frame position in the file.

    @frame_num@ - Frame number.

    @frame_time@ - Frame timestamp in millisecond.
**********************************************************************************/
typedef struct{
    sx_int32 pos;                          // frame position in the file.
    sx_int32 frame_num;                    // frame number.
    sx_int32 frame_time;                   // frame timestamp in millisecond.
}sdvr_player_frame_pos_t;

EXTERN sdvr_player_err_e sdvr_player_init();
EXTERN void sdvr_player_release();
EXTERN sdvr_player_err_e sdvr_player_open_file(sx_int32 port, char *filename);
EXTERN sdvr_player_err_e sdvr_player_open_sdp_file(char *filename);
EXTERN sdvr_player_err_e sdvr_player_close_file(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_play(sx_int32 port, sdvr_ui_hwnd_t hwnd);
EXTERN sdvr_player_err_e sdvr_player_stop(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_pause(sx_int32 port, sx_bool pause);
EXTERN sdvr_player_err_e sdvr_player_double_speed(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_halve_speed(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_set_play_pos(sx_int32 port, float relative_pos);
EXTERN sdvr_player_err_e sdvr_player_get_play_pos(sx_int32 port, float *relative_pos);
EXTERN sdvr_player_err_e sdvr_player_register_file_end_msg(sx_int32 port, sdvr_ui_hwnd_t hwnd, sx_uint32 msg);
EXTERN sdvr_player_err_e sdvr_player_get_file_time(sx_int32 port, sx_int32 *time);
EXTERN sdvr_player_err_e sdvr_player_next_frame(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_prev_frame(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_get_total_frames(sx_int32 port, sx_int32 *total_frames);
EXTERN sdvr_player_err_e sdvr_player_get_avg_time_per_frame(sx_int32 port, double *avg_time_per_frame);
EXTERN sdvr_player_err_e sdvr_player_get_state(sx_int32 port, sdvr_player_state_e *state);
EXTERN sdvr_player_err_e sdvr_player_get_play_rate(sx_int32 port, float *rate);
EXTERN sdvr_player_err_e sdvr_player_get_played_time(sx_int32 port, sx_int32 *time);
EXTERN sdvr_player_err_e sdvr_player_set_current_time(sx_int32 port, sx_int32 time);
EXTERN sdvr_player_err_e sdvr_player_get_played_frames(sx_int32 port, sx_int32 *frames);
EXTERN sdvr_player_err_e sdvr_player_set_current_frame_num(sx_int32 port, sx_int32 frame_num);
EXTERN sdvr_player_err_e sdvr_player_set_volume(sx_int32 port, sx_uint16 volume);
EXTERN sdvr_player_err_e sdvr_player_get_volume(sx_int32 port, sx_uint16 *volume);
EXTERN sdvr_player_err_e sdvr_player_get_picture_size(sx_int32 port, sx_int32 *width, sx_int32 *height);
EXTERN sdvr_player_err_e sdvr_player_get_prev_key_frame_pos(sx_int32 port, sx_int32 curr_pos, sdvr_player_search_type_e type, sdvr_player_frame_pos_t *frame_pos);
EXTERN sdvr_player_err_e sdvr_player_get_next_key_frame_pos(sx_int32 port, sx_int32 curr_pos, sdvr_player_search_type_e type, sdvr_player_frame_pos_t *frame_pos);
EXTERN sdvr_player_err_e sdvr_player_refresh_play(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_play_sound(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_stop_sound();
EXTERN sdvr_player_err_e sdvr_player_play_sound_share(sx_int32 port);
EXTERN sdvr_player_err_e sdvr_player_stop_sound_share(sx_int32 port);

/* streaming API */
EXTERN sdvr_player_err_e sdvr_player_open_stream(sx_int32 port, sx_int32 buf_pool_size);
EXTERN sdvr_player_err_e sdvr_player_input_data(sx_int32 port, sx_uint8 *data_buf, sx_int32 data_size);
EXTERN sdvr_player_err_e sdvr_player_get_num_bytes_in_buffer(sx_int32 port, sx_uint32 *num_bytes_in_buffer);
EXTERN sdvr_player_err_e sdvr_player_close_stream(sx_int32 port);


#endif
