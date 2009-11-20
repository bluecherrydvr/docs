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
    PACKAGE: splayer -- Stretch Media Player API.

    DESCRIPTION:

    SECTION: Include
    {
    #include "splayer.h"
    }

    SECTION: Usage

    The Stretch Media Player can play audio/video files and streams comprising of 
    sdvr_av_buffer packets obtained from the Stretch DVR SDK.

    The player supports up to SPLAYER_MAX_PORTS of audio and video display.  
    Each port represents either a file or a data stream, and they operate in parallel.  
    The player supports simultaneous display of multiple video channels in multiple windows.  
    It also supports playing audio both pre-emptively, where only one of these ports may 
    play audio at a time, or shared, where the audio channels share the output.

    Before using any other API functions, an application must initialize the player with 
    splayer_init().  Ports may then be initialized in two ways: for file mode, 
    splayer_open_file() associates a port with a file; for streaming mode, 
    splayer_open_stream() initializes the decoding.  Once it has initialized a port,
    the application calls splayer_play() with a window handle to specify where to display the video.

    When playing from file mode, the application can call various functions to pause, seek, 
    and back up in the video stream.  When playing a stream, the player displays frames and 
    then deletes the buffers.  Players in stream mode thus have fewer features available, 
    for example, pause playback.

    In file mode, the player stops when it reaches the end of the file, and it may call back 
    the application when this event occurs.  Playback ends in file mode when 
    splayer_close_file() is called for a port playing a file, and splayer_close_stream() 
    for a streaming port.

    SUBSECTION: Pseudo code

    #include "splayer.h"

    {
    #define NETWORK_PACKET_SIZE 65536
    #define FILE_PORT 1
    #define STREAM_PORT 2
    #define STREAM_BIT_RATE 2000000
    }

    {
    int main()
    {
        unsigned char *bufferFromNetwork;
        int bufferSize;
        sdvr_ui_hwnd_t filePlayWindow, networkPlayWindow;
        //  Initialization
        splayer_init();
        bufferFromNetwork = malloc(NETWORK_PACKET_SIZE);
        //  [Create windows]
        ...
        //  Open one file port and one stream port
        splayer_open_file(FILE_PORT, "myfile.raw");
        splayer_open_stream(STREAM_PORT, (STREAM_BIT_RATE / 8) * 10);
        //  Start playing both channels
        splayer_play(FILE_PORT, filePlayWindow);
        splayer_play(STREAM_PORT, networkPlayWindow);
        while(wantToKeepPlaying)
        {
            int numBytesInBuffer;
            network_receive_frame(bufferFromNetwork, &bufferSize);
            //   Wait for buffer to have space for new data
            while(splayer_get_num_bytes_in_buffer(STREAM_PORT, &numBytesInBuffer) + bufferSize < (STREAM_BIT_RATE / 8) * 10);
            //  Add data to player
            splayer_stream_input_data(STREAM_PORT, bufferFromNetwork, bufferSize);
        }   
        //  Stop playing
        splayer_close_file(FILE_PORT);
        splayer_close_stream(STREAM_PORT);
        splayer_release();
        free(bufferFromNetwork);
        return 0;
    }
    }

    SECTION: Special notes

    The current player API only supports Windows platform.  
************************************************************************/

#ifndef __SPLAYER_H__
#define __SPLAYER_H__

#include "sdvr_ui_sdk.h"

/**********************************************************************************
    Maximum number of player ports supported.
**********************************************************************************/
#define SPLAYER_MAX_PORTS   64

/**********************************************************************************
    Maximum number of display regions one player supports.
**********************************************************************************/
#define SPLAYER_MAX_DISPLAY_REGIONS   4

/**********************************************************************************
  VISIBLE: SDVR player state.

    @SPLAYER_EMPTY@ - No file or stream has been opened.

    @SPLAYER_READY@ - Player is ready to play.

    @SPLAYER_PLAY@ - Player is playing.

    @SPLAYER_STOP@ - Player is stopped.

    @SPLAYER_PAUSE@ - Player is paused.
**********************************************************************************/
typedef enum 
{
    SPLAYER_EMPTY = 0,                  
    SPLAYER_READY,                      
    SPLAYER_PLAY,                       
    SPLAYER_STOP,                       
    SPLAYER_PAUSE                       
} splayer_state_e;

/**********************************************************************************
  VISIBLE: SDVR player frame search type.

    @SPLAYER_SEARCH_BY_FRAME_NUM@ - Search by frame number.

    @SPLAYER_SEARCH_BY_FRAME_TIME@ - Search by frame time in milli-second.
**********************************************************************************/
typedef enum 
{
    SPLAYER_SEARCH_BY_FRAME_NUM = 0,     
    SPLAYER_SEARCH_BY_FRAME_TIME         
} splayer_search_type_e;

/**********************************************************************************
  VISIBLE: SDVR player error code. 

    @SPLAYER_NOERROR@ - No error, operation succeeded.

    @SPLAYER_NULL_POINTER@ - Null player pointer.

    @SPLAYER_ALLOC_MEMORY_ERROR@ - Memory allocation failed.

    @SPLAYER_NOT_IMPLEMENTED@ - Feature not implemented.

    @SPLAYER_INVALID_PARAM@ - Invalid input parameter.

    @SPLAYER_INVALID_PORT@ - Invalid port number.

    @SPLAYER_INITIALIZED@ - Player is already initialized.

    @SPLAYER_NOT_READY@ - Player is not ready.

    @SPLAYER_THREAD_ERROR@ - Creating thread failed.

    @SPLAYER_INTERNAL_ERROR@ - Player internal error.

    @SPLAYER_SUPPORT_FILE_ONLY@ - Operation supported in file mode only.

    @SPLAYER_SUPPORT_STREAM_ONLY@ - Operation supported in streaming mode only.

    @SPLAYER_OPEN_FILE_ERROR@ - Failed to open file.

    @SPLAYER_UNSUPPORTED_FILE_FORMAT@ - File format is not supported.

    @SPLAYER_READ_FILE_ERROR@ - Read file error.

    @SPLAYER_FILE_NO_VIDEO@ - Input file does not have video.

    @SPLAYER_FILE_NO_AUDIO@ - Input file does not have audio.

    @SPLAYER_VIDEO_DECODER_INIT_ERROR@ - Video decoder initialization failed.

    @SPLAYER_VIDEO_DEC_ERROR@ - Video decoding error.

    @SPLAYER_AUDIO_DECODER_INIT_ERROR@ - Audio decoder initialization failed.

    @SPLAYER_AUDIO_DEC_ERROR@ - Audio decoding error.

    @SPLAYER_RENDERER_NULL_POINTER@ - // Null renderer pointer.

    @SPLAYER_RENDERER_NO_VIDEO@ - No video renderer.

    @SPLAYER_RENDERER_NO_AUDIO@ - No audio renderer.

    @SPLAYER_DDRAW_ERROR@ - DirectDraw error. 

    @SPLAYER_DSOUND_ERROR@ - DirectSound error.

    @SPLAYER_SNAPSHOT_BUFFER_SIZE@ - Buffer size not enough to take snapshot.

**********************************************************************************/
typedef enum 
{
    SPLAYER_NOERROR = 3000,		        // no error, operation succeeded.
    // general
    SPLAYER_NULL_POINTER,               // null player pointer.
    SPLAYER_ALLOC_MEMORY_ERROR,         // memory allocation failed.
    SPLAYER_NOT_IMPLEMENTED,            // feature not implemented.
    SPLAYER_INVALID_PARAM,              // invalid input parameter.
    SPLAYER_INVALID_PORT,               // invalid port number.
    SPLAYER_INITIALIZED,                // player is already initialized.
    SPLAYER_NOT_READY,                  // player is not ready to play.
    SPLAYER_THREAD_ERROR,               // create thread failed.
    SPLAYER_INTERNAL_ERROR,             // internal error.
    // demuxer related
    SPLAYER_SUPPORT_FILE_ONLY,          // operation supported in file mode only.
    SPLAYER_SUPPORT_STREAM_ONLY,        // operation supported in streaming mode only.
    SPLAYER_OPEN_FILE_ERROR,            // input file does not exist.
    SPLAYER_UNSUPPORTED_FILE_FORMAT,    // input file format is not supported.
    SPLAYER_READ_FILE_ERROR,            // read file failed.
    SPLAYER_FILE_NO_VIDEO,              // input file does not have video.
    SPLAYER_FILE_NO_AUDIO,              // input file does not have audio.
    // decoder related
    SPLAYER_VIDEO_DECODER_INIT_ERROR,   // video decoder init failed.
    SPLAYER_VIDEO_DEC_ERROR,            // video frame decoding failed.
    SPLAYER_AUDIO_DECODER_INIT_ERROR,   // audio decoder init failed.
    SPLAYER_AUDIO_DEC_ERROR,            // audio packet decoding failed.
    // renderer related
    SPLAYER_RENDERER_NULL_POINTER,      // null renderer pointer.
    SPLAYER_RENDERER_NO_VIDEO,          // no video renderer.
    SPLAYER_RENDERER_NO_AUDIO,          // no audio renderer.
    SPLAYER_DDRAW_ERROR,                // DirectDraw error. 
    SPLAYER_DSOUND_ERROR,               // DirectSound error.
    SPLAYER_SNAPSHOT_BUFFER_SIZE,       // buffer size not enough to take snapshot.
} splayer_err_e;

/**********************************************************************************
  VISIBLE: SDVR player frame position.

    @pos@ - Frame position in the file.

    @frame_num@ - Frame number.

    @frame_time@ - Frame timestamp in millisecond.
**********************************************************************************/
typedef struct
{
    sx_int32 pos;                          // frame position in the file.
    sx_int32 frame_num;                    // frame number.
    sx_int32 frame_time;                   // frame timestamp in millisecond.
} splayer_frame_pos_t;

/**********************************************************************************
  VISIBLE: SDVR player frame type.

    @SPLAYER_FRAME_TYPE_UNKNOWN@ - Unknown type.

    @SPLAYER_FRAME_TYPE_YV12@ - Video type YV12.

    @SPLAYER_FRAME_TYPE_AUDIO_S16LE@ - Audio type signed 16-bit little endian.
**********************************************************************************/
typedef enum 
{
    SPLAYER_FRAME_TYPE_UNKNOWN = 0,     
    SPLAYER_FRAME_TYPE_YV12,         
    SPLAYER_FRAME_TYPE_AUDIO_S16LE,         
} splayer_frame_type_e;

/**********************************************************************************
  VISIBLE: SDVR player frame position.

    @buf@ - Pointer of frame buffer.

    @size@ - Size of frame buffer in byte.

    @width@ - Frame width.

    @height@ - Frame height.

    @timestamp@ - Frame timestamp.

    @type@ - Frame type.

    @id@ - Frame id.
**********************************************************************************/
typedef struct
{
    sx_uint8 *buf;                         // pointer of frame buffer.
    sx_int32 size;					       // size of frame buffer.
    sx_int32 width;                        // frame width.
    sx_int32 height;                       // frame height.
    sx_int64 timestamp;                    // frame timestamp.
    splayer_frame_type_e type;             // frame type
    sx_int32 id;                           // frame id
    sx_int32 alloc_size;                   // allocated buffer size
} splayer_frame_t;

/**********************************************************************************
  VISIBLE: SDVR player display callback function prototype.
**********************************************************************************/
typedef void (*display_callback_t)(sx_int32 port, splayer_frame_t *frame);

/**********************************************************************************
  VISIBLE: SDVR player decode callback function prototype.
**********************************************************************************/
typedef void (*decode_callback_t)(sx_int32 port, splayer_frame_t *frame);


/*******************************************************************************
   Declaration of exported dll functions.
   For example:
   {
       EXTERN void my_external_symbol(int a, double f);
   }
   This specifies that the function has C linkage so that it can be used
   when compiled with a C++ compiler.
*******************************************************************************/
#ifdef EXTERN
	#undef EXTERN
#endif

#if defined(__cplusplus)
	#define EXTERN              extern "C" __declspec(dllexport)
#else
	#define EXTERN              extern __declspec(dllexport)
#endif

/*******************************************************************************
    GROUP: General player functions.
*******************************************************************************/
EXTERN void splayer_get_version(sx_uint8 *major, sx_uint8 *minor, sx_uint8 *revision, sx_uint8 *build);
EXTERN splayer_err_e splayer_init();
EXTERN void splayer_release();

/*******************************************************************************
    GROUP: File player functions.
*******************************************************************************/
EXTERN splayer_err_e splayer_open_file(sx_int32 port, char *filename);
EXTERN splayer_err_e splayer_close_file(sx_int32 port);
EXTERN splayer_err_e splayer_play(sx_int32 port, sdvr_ui_hwnd_t hwnd);
EXTERN splayer_err_e splayer_stop(sx_int32 port);
EXTERN splayer_err_e splayer_pause(sx_int32 port, sx_bool pause);
EXTERN splayer_err_e splayer_double_speed(sx_int32 port);
EXTERN splayer_err_e splayer_halve_speed(sx_int32 port);
EXTERN splayer_err_e splayer_set_play_pos(sx_int32 port, float relative_pos);
EXTERN splayer_err_e splayer_get_play_pos(sx_int32 port, float *relative_pos);
EXTERN splayer_err_e splayer_register_file_end_msg(sx_int32 port, sdvr_ui_hwnd_t hwnd, sx_uint32 msg);
EXTERN splayer_err_e splayer_get_file_time(sx_int32 port, sx_int32 *time);
EXTERN splayer_err_e splayer_next_frame(sx_int32 port);
EXTERN splayer_err_e splayer_prev_frame(sx_int32 port);
EXTERN splayer_err_e splayer_get_total_frames(sx_int32 port, sx_int32 *total_frames);
EXTERN splayer_err_e splayer_get_avg_time_per_frame(sx_int32 port, double *avg_time_per_frame);
EXTERN splayer_err_e splayer_get_state(sx_int32 port, splayer_state_e *state);
EXTERN splayer_err_e splayer_get_play_speed(sx_int32 port, float *speed);
EXTERN splayer_err_e splayer_get_played_time(sx_int32 port, sx_int32 *time);
EXTERN splayer_err_e splayer_set_current_time(sx_int32 port, sx_int32 time);
EXTERN splayer_err_e splayer_get_played_frames(sx_int32 port, sx_int32 *frames);
EXTERN splayer_err_e splayer_set_current_frame_num(sx_int32 port, sx_int32 frame_num);
EXTERN splayer_err_e splayer_set_volume(sx_int32 port, sx_uint16 volume);
EXTERN splayer_err_e splayer_get_volume(sx_int32 port, sx_uint16 *volume);
EXTERN splayer_err_e splayer_get_picture_size(sx_int32 port, sx_int32 *width, sx_int32 *height);
EXTERN splayer_err_e splayer_get_prev_key_frame_pos(sx_int32 port, sx_int32 curr_pos, splayer_search_type_e type, splayer_frame_pos_t *frame_pos);
EXTERN splayer_err_e splayer_get_next_key_frame_pos(sx_int32 port, sx_int32 curr_pos, splayer_search_type_e type, splayer_frame_pos_t *frame_pos);
EXTERN splayer_err_e splayer_refresh_play(sx_int32 port);
EXTERN splayer_err_e splayer_play_sound(sx_int32 port);
EXTERN splayer_err_e splayer_stop_sound();
EXTERN splayer_err_e splayer_play_sound_share(sx_int32 port);
EXTERN splayer_err_e splayer_stop_sound_share(sx_int32 port);

/*******************************************************************************
    GROUP: Stream player functions.
*******************************************************************************/
EXTERN splayer_err_e splayer_open_stream(sx_int32 port, sx_int32 buf_pool_size);
EXTERN splayer_err_e splayer_input_data(sx_int32 port, sx_uint8 *data_buf, sx_int32 data_size);
EXTERN splayer_err_e splayer_get_num_bytes_in_buffer(sx_int32 port, sx_uint32 *num_bytes_in_buffer);
EXTERN splayer_err_e splayer_close_stream(sx_int32 port);

/*******************************************************************************
    GROUP: Misc functions.
*******************************************************************************/
EXTERN splayer_err_e splayer_set_display_callback(sx_int32 port, display_callback_t pCB);
EXTERN splayer_err_e splayer_take_video_snapshot(sx_int32 port, splayer_frame_t *frame);
EXTERN splayer_err_e splayer_set_decode_callback(sx_int32 port, decode_callback_t pCB);
EXTERN splayer_err_e splayer_set_display_region(sx_int32 port, sx_int32 region_num, sdvr_ui_region_t *region, sdvr_ui_hwnd_t hwnd, sx_bool enable);

#endif
