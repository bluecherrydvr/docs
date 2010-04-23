/*****************************************************************************
*  Copyright C 2007 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
*****************************************************************************/


#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include "mp4mux.h"
#include "internal.h"

/****************************************************************************
  This function is used to open an mp4 file format for reading or writting A/V 
  frames. (See mp4_mux_mode_t for all the different modes of MP4 file.)
  It returns a handle to the openned file which needs to be used by other
  functions to add or read A/V track to the file.

  This function supports H.264, MPEG4, MJPEG video CODECS as well as G.711 audio CODEC.
  Currently, only
  QuickTime .mov file format is supported in this release. Other than QuickTime,
  .mov file format can be played back by such players as Elecard or VLC.

  Before calling this function, you must initialize the 'mp4_open_info' 
  parameter data structure to zero and only set the required parameters. 
  (See below for the required fields.)

  Parameters:

    @mp4_open_info@ - The pointer to a structure to indicate the file name,
    open mode (read or write), and file type.

    To open a file to read A/V frames for playback, you must initialize the 
    following fields:

        mp4_open_info.file_name = <full path to the .mov file to open>;

        mp4_open_info.mux_mode = MP4_READ_PLAY;

        mp4_open_info.file_mode = MODE_MOV; // This is the only mode supported.

    To open a file to write A/V frames for recording, you must initialize the 
    following fields:

        mp4_open_info.file_name = <full path to the .mov file to open>;

        mp4_open_info.mux_mode = MP4_RECORD;

        mp4_open_info.file_mode = MODE_MOV; // This is the only mode supported.


  Returns:

    A handle to the file that is opened if the return value is greater than zero.

    If zero, check mp4_open_info->error for the error code.
****************************************************************************/
mp4_handle mp4mux_open(mp4_info_t *mp4_open_info)
{
    sx_int32 oflag, err;
    fstat_t st;
    mp4_context_t *ctx;

    if (mp4_open_info == NULL)
        return 0;

    ctx = malloc(sizeof(mp4_context_t));
    if (ctx == NULL) {
        mp4_open_info->error = MP4MUX_ERR_MEMORY;
        return 0;
    }
    memset(ctx, 0, sizeof(mp4_context_t));

    // copy file name
    ctx->file_name = (char *)malloc(strlen(mp4_open_info->file_name) + 1);
    if (ctx->file_name == NULL) {
        free(ctx);
        mp4_open_info->error = MP4MUX_ERR_MEMORY;
        return 0;
    }
    strcpy(ctx->file_name, mp4_open_info->file_name);

    // initialization
    ctx->mux_mode = mp4_open_info->mux_mode;
    ctx->mode = mp4_open_info->file_mode;
    ctx->timescale = mp4_open_info->timescale;
    ctx->bs_buf_size = mp4_open_info->bs_buf_size;
    ctx->log = mp4_open_info->log;

    switch (ctx->mux_mode) {
    case MP4_RECORD:
        ctx->version = SMFF_VERSION;
#ifdef  STRETCH_TARGET
        ctx->fd = (sx_int32) FS_FOpen (ctx->file_name, "ab+");
        oflag = oflag; // avoid compiler warning
#else
        // check if the file is already exist
        if (sx_stat(ctx->file_name, &st) == 0) {
            // delete the file
            if ((err = remove(ctx->file_name)) != 0) {
                // can not delete file
                mp4_open_info->error = MP4MUX_ERR_FILE_EXIST;
                mp4_freep((void *) &ctx->file_name);
                free(ctx);
                return 0;
            }
        }

#ifdef WIN32
        // open file
        oflag = O_BINARY | O_RDWR | O_CREAT | O_TRUNC;
        ctx->fd = sx_open(ctx->file_name, oflag, _S_IREAD | _S_IWRITE);
#else
        // open file
        oflag = O_RDWR | O_CREAT | O_TRUNC;
        ctx->fd = sx_open(ctx->file_name, oflag, 0644);
#endif
#endif
        if (ctx->fd == -1) {
            mp4_open_info->error = MP4MUX_ERR_OPEN_FILE;
            mp4_freep((void *) &ctx->file_name);
            free(ctx);
            return 0;
        }
        ctx->sys_time = time(NULL);
        ctx->flush_period = DEFAULT_FLASH_PERIOD;
        SET_FLAGS(ctx->flags, MP4_FLAG_APPEND_MOOV);
        mp4_open_info->error = mp4_create(ctx);
        break;

    case MP4_READ_INFO:
#ifdef  STRETCH_TARGET
        ctx->fd = (sx_int32) FS_FOpen (ctx->file_name, "r");
        oflag = oflag;
#else
#ifdef WIN32
        oflag = O_BINARY | O_RDONLY;
#else
        oflag = O_RDONLY;
#endif
        ctx->fd = sx_open(ctx->file_name, oflag);
#endif
        if (ctx->fd == -1) {
            mp4_open_info->error = MP4MUX_ERR_OPEN_FILE;
            mp4_freep((void *) &ctx->file_name);
            free(ctx);
            return 0;
        }
        mp4_open_info->error = mp4_read_info(ctx);
        // copy useful information
        mp4_open_info->nb_tracks = ctx->nb_tracks;
        mp4_open_info->timescale = ctx->timescale;
        mp4_open_info->duration = ctx->duration;
        break;

    case MP4_READ_PLAY:
#ifdef  STRETCH_TARGET
        ctx->fd = (sx_int32) FS_FOpen (ctx->file_name, "r");
        oflag = oflag;
#else
#ifdef WIN32
        oflag = O_BINARY | O_RDONLY;
#else
        oflag = O_RDONLY;
#endif
        ctx->fd = sx_open(ctx->file_name, oflag);
#endif
        if (ctx->fd == -1) {
            mp4_open_info->error = MP4MUX_ERR_OPEN_FILE;
            mp4_freep((void *) &ctx->file_name);
            free(ctx);
            return 0;
        }
        SET_FLAGS(ctx->flags, MP4_FLAG_PARSE_SAMPLE_TABLE);
        mp4_open_info->error = mp4_read_open(ctx);
        // copy useful information
        mp4_open_info->nb_tracks = ctx->nb_tracks;
        mp4_open_info->timescale = ctx->timescale;
        mp4_open_info->duration = ctx->duration;
        ctx->play_speed = 1.0;
        break;

    case MP4_AUTHORIZE:
#ifdef WIN32
        oflag = O_BINARY | O_RDWR;
#else
        oflag = O_RDWR;
#endif
        // open file
        ctx->fd = sx_open(ctx->file_name, oflag);
        if (ctx->fd == -1) {
            mp4_open_info->error = MP4MUX_ERR_OPEN_FILE;
            mp4_freep((void *) &ctx->file_name);
            free(ctx);
            return 0;
        }
        break;

    default:
        mp4_open_info->error = MP4MUX_ERR_MUX_MODE;
        return 0;
    }

    return (mp4_handle) ctx;
}
/****************************************************************************
  This function is used to close the mp4 file format which was opened by
  calling mp4mux_open().

  You must call this function after you are done reading or writing
  A/V frames. 

  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open().

  Returns:

    MP4MUX_ERR_OK if success. Otherwise, an error code.

  Remarks:

    The generated .mov can not be played back by any third party
    players if this function is not called at the end of recording. Yet, it can be
    openned and played back by the Stretch mp4mux APIs.
****************************************************************************/

sx_int32 mp4mux_close(mp4_handle file_handle)
{
    mp4_context_t *ctx = (mp4_context_t *) file_handle;

    // release resources
    switch (ctx->mux_mode) {
    case MP4_RECORD:
        mp4_write_close(ctx);
        break;

    case MP4_READ_INFO:
    case MP4_READ_PLAY:
        mp4_read_close(ctx);
        break;

    case MP4_AUTHORIZE:
        mp4_authorize_close(ctx);
        break;
    }

#ifdef STRETCH_TARGET
    FS_FClose ((FS_FILE*)ctx->fd);
#else
    sx_close(ctx->fd);
#endif

    mp4_freep((void *) &ctx->file_name);
    free(ctx);
    return MP4MUX_ERR_OK;
}

/****************************************************************************
  After you open an MP4 file for recording, you must setup the file to 
  support adding audio or video packets. This function adds audio or
  video tracks to the MP4 file.

  You must call this function separately for every track. This means if you 
  are recording both audio and video frames. You must call this function
  twice, once to add the audio track another to add video track.

  After A/V tracks are setup, you can add audio or video packets to each 
  track by calling mp4mux_put_packet().

  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open().

    @track_info@ - The track information data structure. Most of the field
    in this data structure are constants. You must initialize this data
    structure to zero before changing its fields and calling mp4mux_add_track.

    Following are the fields within this data structure that are needed to 
    be initialized for adding video track:

        track_info.type = MP4_TRACK_VIDEO;

        track_info.codec_id = <CODEC_ID_H264, CODEC_ID_MPEG4, or CODEC_ID_JPEG>

        Video timescale is based on 90k Hz clock.

        track_info.timescale = 90000; 

        track_info.width = imageSize.width();

        Make sure that the video height is multiple of 16;

        vHeight = imageSize.height();

        vHeight /=  16;

        vHeight *= 16;

        track.height = vHeight;

        track_info.default_duration = <3003 for NTSC> or <3600 for PAL>

    Following are the fields within this data structure that are needed to 
    be initialized for adding audio track:

        sx_int8  mp4mux_lang[4] = {'e', 'n', 'g', '\0'};
        track_info.type = MP4_TRACK_AUDIO;

        Currently the SDK encodes audio in CODEC_ID_PCM_MULAW

        track_info.codec_id = CODEC_ID_PCM_MULAW;

        track_info.timescale = 8000; Audio sampling rate

        track_info.channels = 2;  stereo recording

        track_info.sample_size = 1 * track.channels;

        Default audio packet duration is packet size (2560) in bytes divided 
        by channels

        track_info.default_duration = 2560 / track.channels; 

        track_info.default_size = 1 * track.channels; // one byte per channel for PCM

        track.language = mp4mux_iso639_to_lang(mp4mux_lang, 1);

  Returns:

    MP4MUX_ERR_OK if success. Otherwise, an error code.

    @On successful return, track_info.track_id contains the ID of this new
    track. You must save this ID as it is needed in calls to mp4mux_put_packet()
    to add A/V frames to the MP4 file.@
****************************************************************************/
sx_int32 mp4mux_add_track(mp4_handle file_handle, mp4_track_info_t *track_info)
{
    mp4_context_t *ctx = (mp4_context_t *) file_handle;
    return mp4_add_track(ctx, track_info);
}

/****************************************************************************
  Call this function to add A/V frames to the tracks in the MP4 file.

  You must use the track ID returned from mp4mux_add_track() in order
  to add the corresponding A/V frames.

  Each video key frame needs to be flaged as you add video frames.
  (i.e., SDVR_FRAME_H264_IDR, SDVR_FRAME_H264_I, SDVR_FRAME_MPEG4_I, 
  SDVR_FRAME_JPEG)


  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open().

    @track_id@ - The track ID returned from mp4mux_add_track() for the
    current audio or frame type that is being added.

    @av_pkt@ - The audio or video packet to be added. Each A/V packet in
    addition to the A/V frame data has other information that is needed
    to be set before calling mp4mux_put_packet(). 
    Initialize this data structure to zero before changing any of its fields.

    Following is a list of
    fields in mp4_packet_t data structure that must be initialized when adding
    video frames:

        av_pkt.data = buf->payload; // the encoded video frame data

        av_pkt.size = buf->payload_size; // the size of encoded video frame

        av_pkt.sample_count = 1;      // set always to 1
        av_pkt.dts =  buf->timestamp; // the frame timestamp

        av_pkt.pts = av_pkt.dts = (sx_uint64)buf->timestamp | ((sx_uint64)buf->timestamp_high << 32);

        You must set the av_pkt.flags to true for every video key frame

        {
            if (venc == SDVR_VIDEO_ENC_H264)
            {
                if (buf->frame_type == SDVR_FRAME_H264_IDR) ||
                          (buf->frame_type == SDVR_FRAME_H264_I))
                      av_pkt.flags |= PKT_FLAG_KEY_FRAME;

            }
            else if (venc == SDVR_VIDEO_ENC_MPEG4)
            {
                if (buf->frame_type == SDVR_FRAME_MPEG4_I)
                    av_pkt.flags |= PKT_FLAG_KEY_FRAME;
            }
            else // All MJPEG frames are key frame
            {
                av_pkt.flags |= PKT_FLAG_KEY_FRAME;
            }
        }

    Following is a list of
    fields in mp4_packet_t data structure that must be initialized when adding
    audio frames:

        av_pkt.data = buf->payload; // the encoded audio frame data

        av_pkt.size = buf->payload_size; // the size of encoded audio frame

        av_pkt.sample_count = buf->payload_size; // The payload contains 2560 
        samples of 8 bytes.

        av_pkt.pts = av_pkt.dts = (sx_uint64)buf->timestamp | ((sx_uint64)buf->timestamp_high << 32);

  Returns:

    MP4MUX_ERR_OK if success. Otherwise, an error code.
****************************************************************************/
sx_int32 mp4mux_put_packet(mp4_handle hdl, sx_uint32 track_id, mp4_packet_t *av_pkt)
{
    mp4_context_t *ctx = (mp4_context_t *) hdl;
    sx_int32 err;
    // av sync processing
    if ((err = mp4_av_sync(ctx, track_id, av_pkt)) != MP4MUX_ERR_OK)
        return err;
    return mp4_put_packet(ctx, track_id, av_pkt);
}

/****************************************************************************
  Call this function to set the period of flushing during recording in unit
  of MP4_TIMESCALE defined in internal.h. 

  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open().

    @period@ - Period of flushing in unit of MP4_TIMESCALE.

  Returns:

    MP4MUX_ERR_OK if success. Otherwise, an error code.

  Remarks:

    Internally the minimal flushing period is one video GOP regardless 
    of this setting.
****************************************************************************/
EXTERN sx_int32 mp4mux_set_flush_period(mp4_handle hdl, sx_int64 period)
{
    mp4_context_t *ctx = (mp4_context_t *) hdl;
    if (ctx == NULL)
        return MP4MUX_ERR_INVALID_HANDLE;
    ctx->flush_period = period;
    return MP4MUX_ERR_OK;
}

/****************************************************************************
  Call this function to get the number of tracks, timescale, and the duration
  of longest track in the MP4 file.

  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open().

    @mp4_file_info@ - Up on return, this data structure is filled
    with the MP4 file information.

  Returns:

    MP4MUX_ERR_OK if success. Otherwise, an error code.
****************************************************************************/
sx_int32 mp4mux_get_file_info(mp4_handle hdl, mp4_info_t *mp4_file_info)
{
    mp4_context_t *ctx = (mp4_context_t *) hdl;

    // copy useful information
    mp4_file_info->nb_tracks = ctx->nb_tracks;
    mp4_file_info->timescale = ctx->timescale;
    mp4_file_info->duration = ctx->duration;
    return MP4MUX_ERR_OK;
}

/****************************************************************************
  Call this function to get information about a given track inside the
  MP4 file.

  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open().

    @track_id@ - ID of the track to get its information. This ID
    was returned from mp4mux_add_track().

    @trk_info@ - Up on return, this data structure is filled
    with track information.

  Returns:

    MP4MUX_ERR_OK if success. Otherwise, an error code.
****************************************************************************/
sx_int32 mp4mux_get_track_info(mp4_handle hdl, sx_int32 track_id, mp4_track_info_t *trk_info)
{
    mp4_context_t *ctx = (mp4_context_t *) hdl;

    if (track_id >= MP4_MAX_TRACKS)
        return MP4MUX_ERR_TRACK_NUMBER;
    if (ctx->tracks[track_id] == NULL)
        return MP4MUX_ERR_INVALID_TRACK;
    memcpy(trk_info, &ctx->tracks[track_id]->info, sizeof(mp4_track_info_t));
    return MP4MUX_ERR_OK;
}

/****************************************************************************
  As you open an MP4 file for reading, you may not be interested in either
  audio or video track which is recorded in the file. Call this function 
  to disable a specific track. Once a track is disabled, the calls to
  mp4mux_get_packet() skips over those track, resulting a better performance.

  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open().

    @track_id@ - ID of the track to be disabled.

  Returns:

    MP4MUX_ERR_OK if success. 

    MP4MUX_ERR_INVALID_TRACK - Error code if the track ID is invalid.
****************************************************************************/
sx_int32 mp4mux_disable_track(mp4_handle hdl, sx_int32 track_id)
{
    sx_int32 i;
    mp4_context_t *ctx = (mp4_context_t *) hdl;
    for (i=0; i<ctx->nb_tracks; i++) {
        if (ctx->tracks[i]->info.track_id == track_id) {
            SET_FLAGS(ctx->tracks[i]->flags, TRACK_FLAG_DISABLED);
            return MP4MUX_ERR_OK;
        }
    }
    return MP4MUX_ERR_INVALID_TRACK;
}

/****************************************************************************
  Call this function to retreive the next the A/V frame from all tracks that
  have not been disabled. The direction of the next returning frame is 
  deteremined by the call to mp4mux_set_play_speed(). 
  If mp4mux_set_play_speed() is never called,
  A/V frames are retrieved in a forward sequence for all the consecutive A/V frames.
  No audio frames is returned if the speed parameter passed to 
  mp4mux_set_play_speed() is any value other than 1.

  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open() which was
    openned with mux_mode field set to MP4_READ_PLAY.

    @av_pkt@ - A pointer to the data which will be filled with audio
    or video frame packet. (See mp4_packet_t structure for the definition
    of the fields.)

  Returns:

    MP4MUX_ERR_OK if success. 
    
    MP4MUX_ERR_END_OF_FILE - Error code if we reached to the end of the file.

    Otherwise, an error condition while reading the next frame.
****************************************************************************/
sx_int32 mp4mux_get_packet(mp4_handle hdl, mp4_packet_t *av_pkt)
{
    mp4_context_t *ctx = (mp4_context_t *) hdl;
    sx_int32 err;

    // handling fragment mode play
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_FRAGMENT_PLAY)) {
    }

    // do seek in trick mode
    if (CHK_FLAGS(ctx->flags, MP4_FLAG_TRICK_MODE)) {
        if ((err = mp4_read_seek_trick_mode(ctx)) != MP4MUX_ERR_OK)
            return err;
    }

    return mp4_get_packet(ctx, av_pkt);
}

/****************************************************************************
  Call this function to repair a recorded MP4 file. If during the recording,
  there is a power failure or your DVR Application crashed, the generated
  MP4 file can not be read by third party application. In most cases but not
  all, the corrupted Mp4 file can be fixed by calling this function.

  Before calling this function, you must open the file by calling mp4mux_open()
  and set the mux_mode field to MP4_ATHORIZE. Call mp4mux_close() after calling
  this function.


  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open() which was
    openned with mux_mode field set to MP4_ATHORIZE.

  Returns:

    MP4MUX_ERR_OK if success. Otherwise, an error code.
****************************************************************************/
sx_int32 mp4mux_authorize(mp4_handle hdl)
{
    error_code_t err;
    mp4_context_t *ctx = (mp4_context_t *) hdl;
    sx_int32 oflag;
    offset_t smff_pos;

    // read file to check syntax
    SET_FLAGS(ctx->flags, MP4_FLAG_REPAIR);
    err = mp4_read_open(ctx);
    smff_pos = ctx->smff_pos;
    mp4_read_close(ctx);

    // check if file is already authorized
    if (err != MP4MUX_ERR_OK)
        return err;
    if (CHK_FLAGS(ctx->smff_flags, SMFF_FLAG_CLOSING_MOOV))
        return MP4MUX_ERR_OK;

    // check if found mdat
//    if (!CHK_FLAGS(ctx->flags, MP4_FLAG_FOUND_MDAT))
//        return MP4MUX_ERR_PARSE_ERROR;

    // open file again for authorizing
#ifdef WIN32
    oflag = O_BINARY | O_RDWR;
#else
    oflag = O_RDWR;
#endif
    // open file
    ctx->fd = sx_open(ctx->file_name, oflag);
    if (ctx->fd == -1)
        return MP4MUX_ERR_OPEN_FILE;

    if ((err = mp4_write_open(ctx)) != MP4MUX_ERR_OK)
        return err;

    ctx->smff_pos = smff_pos;
    err = mp4_authorize(ctx);
    return err;
}

/****************************************************************************
  As MP4 file is being played backed, you may need to playback in forward, 
  backward,  or slow motion. Call this function to specidy the direction and
  speed of getting next frame as you call mp4mux_get_packet().

  Parameters:

    @file_handle@ - The file handle returned by mp4mux_open() which was
    openned with mux_mode field set to MP4_READ_PLAY.

    @speed@ - This field specify the direction and the number of frames
    to skip in order to the next frame for playback.

    A positive value indicates going forward withing the file, whereas, a
    negative value means go backward.

    1 means normal playback going forward. 

    Any other value means skip that many I-frame to the direction of the 
    playback.

  Returns:

    MP4MUX_ERR_OK if success. Otherwise, an error code.
****************************************************************************/
sx_int32 mp4mux_set_play_speed(mp4_handle hdl, float speed)
{
    mp4_context_t *ctx = (mp4_context_t *) hdl;

    ctx->play_speed = speed;
    if (ctx->play_speed != 1.0)
        SET_FLAGS(ctx->flags, MP4_FLAG_TRICK_MODE);
    if (ctx->play_speed < 0.0)
        SET_FLAGS(ctx->seek_flags, MP4_SEEK_FLAG_BACKWARD);
    SET_FLAGS(ctx->seek_flags, MP4_SEEK_INITIAL);

    return MP4MUX_ERR_OK;
}

sx_int32 mp4mux_play_seek(mp4_handle hdl, sx_int32 track_id, sx_int64 timestamp, sx_uint32 flags)
{
    mp4_context_t *ctx = (mp4_context_t *) hdl;
    if (ctx->mux_mode != MP4_READ_PLAY)
        return MP4MUX_ERR_MUX_MODE;
    return mp4_read_seek(ctx, track_id, timestamp, flags);
}
