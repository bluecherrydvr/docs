/****************************************************************************\
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
\****************************************************************************/

#include <QTime>
#include "decoder.h"
#include "player.h"
#include "log.h"
#include "sdvr.h"
#include "dvrsystem.h"
#include "mainwindow.h"
#include "../../src/codecs/avmux/mp4/mp4mux.h"

extern DVRSystem *g_DVR;


CDecoder::CDecoder(CPlayer *player)
{
    m_player = player;
    m_stopRequested = true;
    m_continuous = false;
    m_buf = NULL;
    g_DVR->getFWVersion(&m_FWVersion);
    memset((void *)&m_mp4File,0, sizeof(m_mp4File));
}

void CDecoder::run()
{
    QTime time;
    sx_uint64 ts0 = 0, ts;
    sdvr_err_e sdvr_err;

    uint target_time, current_time, prev_time = 0;
    bool restart = true;

    setTerminationEnabled(true);
    m_buf = NULL;

    time.start();

    while (!m_stopRequested)
    {
        if (m_player->getPlaybackState() == PLAYER_STATE_PAUSED)
        {
            // Sleep for a second and check again
            msleep(1000);
            restart = true;
            continue;
        }

        if (m_buf == NULL)
        {
            sdvr_err = sdvr_alloc_av_buffer_wait(m_player->handle(), &m_buf, 500);
            if (sdvr_err != SDVR_ERR_NONE)
            {
                if (sdvr_err == SDVR_ERR_BUF_NOT_AVAIL)
                    continue;

                Log::info(QString("Error: failed to get buffer. [code: %1]")
                        .arg(sdvr_err));
                break;
            }
        }

        sx_int32 mux_err = getNextAVFrame(m_buf);
        if (mux_err != MP4MUX_ERR_OK)
        {
            // Check for EOF and rewind if continuous playback requested
            if (mux_err == MP4MUX_ERR_END_OF_FILE)
            {
                if (m_continuous)
                {
                    if (!rewind())
                        break;

                    if (getNextAVFrame(m_buf) != MP4MUX_ERR_OK)
                    {
                        Log::info(QString("Error: failed to get the next frame."
                            " [mp4mux code:%1]").arg(mux_err));
                        break;
                    }

                    restart = true;
                }
                else
                {
                    Log::info(QString("%1: finished playback.").arg(m_player->getName()));
                    break;
                }
            }
            else if (mux_err == MP4MUX_ERR_NO_MORE_SAMPLES)
            {
            	msleep(10);    // Sleep for 10ms and retry
            	continue;
            }
            else
            {
                Log::error(QString("Error: failed to get the next frame."
                    " [mp4mux code:%1]").arg(mux_err));
                break;
            }
        }

        ts = getAVFrameTS(m_buf);
        if (restart)
        {
            ts0 = ts;
            time.restart();
            prev_time = 0;
            restart = false;
        }
        else
        {
            // Map timestamp time to counter
            target_time = (ts - ts0) / 90;	// 90KHz clock = 90 tics/ms
            current_time = time.elapsed();

            // Time counter wraps to zero 24 hours after the last (re)start.
            // Restart if the current time is less than the previous.
            if (current_time < prev_time)
            {
                restart = true;
            }
            else
            {
                prev_time = current_time;
                if (target_time >= current_time)
                    msleep(target_time - current_time);
                else
                    ++m_statMissedFrames;
            }
        }

        // Collect the statistics
        m_statBytes += m_buf->payload_size;
        ++m_statFrames;

        // Send the frame
        sdvr_err_e send_err = sdvr_send_av_frame(m_buf);
        if (send_err != SDVR_ERR_NONE)
        {
            Log::error(QString("Error: failed to send frame for decoding. [error= %1 (%2)]")
                .arg(sdvr_get_error_text(send_err)).arg(send_err));
            break;
        }

        m_buf = NULL;
    }

    setTerminationEnabled(false);

    if (m_buf != NULL)
    {
        sdvr_release_av_buffer(m_buf);
        m_buf = NULL;
    }

    if (m_mp4File.FileHandle != 0)
    {
        mp4mux_close(m_mp4File.FileHandle);
        m_mp4File.FileHandle = 0;
    }

    // If the firmware timeout, force the channel to be disabled.
    if (sdvr_enable_decoder(m_player->handle(), false) == SDVR_DRV_ERR_MSG_RECV)
        g_MainWindow->setFirmwareTimeout(m_player->getBoardIndex());

    setTerminationEnabled(true);
}

bool CDecoder::startDecoding(const QString &fileName, bool bContinuousDecoding)
{
    m_mp4FileName = fileName.toAscii();
    if (!openAVFile())
        return false;

	sdvr_err_e err = sdvr_enable_decoder(m_player->handle(), true);

    if (err != SDVR_ERR_NONE)
    {
        // If the firmware timeout, force the channel to be disabled.
        if (err == SDVR_DRV_ERR_MSG_RECV)
            g_MainWindow->setFirmwareTimeout(m_player->getBoardIndex());

        mp4mux_close(m_mp4File.FileHandle);
    }
    else
    {
        m_continuous = bContinuousDecoding;
        m_stopRequested = false;
        m_statMissedFrames = 0;
        resetStats();
        start();
    }

    return err == SDVR_ERR_NONE;
}

bool CDecoder::stopDecoding()
{
    m_stopRequested = true;

    if (wait(1000))
        return true;

    // Try to terminate
    terminate();
    if (wait(1000))
    {
        if (m_buf != NULL)
        {
            sdvr_release_av_buffer(m_buf);
            m_buf = NULL;
        }

        if (m_mp4File.FileHandle != 0)
        {
            mp4mux_close(m_mp4File.FileHandle);
            m_mp4File.FileHandle = 0;
        }

        sdvr_enable_decoder(m_player->handle(), false);
    }

    return !isRunning();
}

bool CDecoder::openAVFile()
{
    mp4_info_t info;
    mp4_track_info_t track;

    m_mp4File.atrack = 0;
    m_mp4File.vtrack = 0;

    memset(&info, 0, sizeof(mp4_info_t));
    info.file_name = m_mp4FileName.data();
    info.mux_mode = MP4_READ_PLAY;
    info.file_mode = MODE_MOV;
    m_mp4File.FileHandle = mp4mux_open(&info);
    if (m_mp4File.FileHandle == 0) {
        Log::error(QString("Error: failed to open file '%1' for playback.")
            .arg(m_mp4FileName.data()));

        return false;
    }

    for (int i = 0; i < info.nb_tracks; ++i) {
        if (!mp4mux_get_track_info(m_mp4File.FileHandle, i, &track))
        {
            if (track.type == MP4_TRACK_AUDIO)
            {
                mp4mux_disable_track(m_mp4File.FileHandle, track.track_id);
            }
            else if (track.type == MP4_TRACK_VIDEO)
            {
                // We will play back the first H.264 video track.
                // Disable all other video tracks.
                if (m_mp4File.vtrack == 0 && track.codec_id == CODEC_ID_H264)
                    m_mp4File.vtrack = track.track_id;
                else
                    mp4mux_disable_track(m_mp4File.FileHandle, track.track_id);
            }
        }
    }

    if (m_mp4File.vtrack == 0)
    {
        mp4mux_close(m_mp4File.FileHandle);
        return false;
    }

    return true;
}

bool CDecoder::rewind()
{
    mp4mux_close(m_mp4File.FileHandle);
    return openAVFile();
}

sx_int32 CDecoder::getNextAVFrame(sdvr_av_buffer_t *buf)
{
    mp4_packet_t pkt;
    sx_int32 err = MP4MUX_ERR_OK;

    memset(&pkt, 0, sizeof(mp4_packet_t));

    // Attach buffer to the packet
    pkt.data = &buf->payload[0];
    pkt.size = g_DVR->getSDKParams().dec_buf_size;

    err = mp4mux_get_packet(m_mp4File.FileHandle, &pkt);
    if (err == MP4MUX_ERR_OK)
    {
        buf->payload_size = pkt.size;
        if (pkt.track_id == m_mp4File.atrack)
        {
            buf->frame_type = SDVR_FRAME_G711;
        }
        else if (pkt.track_id == m_mp4File.vtrack)
        {
            // Overwrite with start code
            pkt.data[0] = pkt.data[1] = pkt.data[2] = 0;
            pkt.data[3] = 1;

            // TODO: get the video frame type from the packet
            if (pkt.flags & PKT_FLAG_KEY_FRAME)
                buf->frame_type = SDVR_FRAME_H264_IDR;
            else if (pkt.flags & PKT_FLAG_CONFIG_DATA)
                buf->frame_type = SDVR_FRAME_H264_SPS;
            else
                buf->frame_type = SDVR_FRAME_H264_P; // Not a key frame...

            // The following OSD functionality only exist in 3.2.x.x or higher of firmware
            if (sdvr::isMinVersion(m_FWVersion.fw_major, m_FWVersion.fw_minor, 0, 0, 3, 2, 0, 0))
            {
                buf->timestamp = (sx_uint32)(pkt.dts & 0xffffffff);
                buf->timestamp_high = (sx_uint32)(pkt.dts >> 32);
            }
            else
            {
                buf->timestamp = pkt.dts;
                buf->timestamp_high = 0;
            }
        }
    }

    return err;
}

sx_uint64 CDecoder::getAVFrameTS(sdvr_av_buffer_t *av_frame)
{
    return (((sx_uint64)(av_frame->timestamp_high)) << 32) + av_frame->timestamp;
}
