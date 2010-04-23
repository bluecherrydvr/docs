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
#include <stdio.h>
#include "camera.h"
#include "log.h"
#include "sdvr.h"
#include "dvrsystem.h"
#include "../../src/codecs/avmux/mp4/mp4mux.h"

extern unsigned util_crc32(void *buf, int length, unsigned seed);

extern DVRSystem *g_DVR;
static int dts;
Recorder::Recorder()
{
    dts = 0;

    for (uint subEnc = 0; subEnc < MAX_SUB_ENCODERS; subEnc++)
        m_statsEnabled[subEnc] = false;
}

Recorder::~Recorder()
{
}

void Recorder::terminate()
{
    m_exit = true;
    m_semNext.release(); 
}

void Recorder::run()
{
    QueueItem item;
    m_exit = false;
    while (true)
    {
        m_semNext.acquire();
        if (m_exit)
        {
            break;
        }

        m_queueMutex.lock();
        if (!m_bufferQueue.isEmpty())
        {
            item = m_bufferQueue.dequeue();
            m_queueMutex.unlock();

            writeBuffer(item.first, item.second);
            sdvr_release_av_buffer(item.second);

        }
        else
            m_queueMutex.unlock();

    }
}

void Recorder::flushFrameBufQueue()
{

    m_queueMutex.lock();

    while (!m_bufferQueue.isEmpty())
    {
        QueueItem item = m_bufferQueue.dequeue();

        writeBuffer(item.first, item.second);
        sdvr_release_av_buffer(item.second);
    }

    m_queueMutex.unlock();
}

void Recorder::enqueue(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf)
{
    m_queueMutex.lock();
    m_bufferQueue.enqueue(QueueItem(handle, buf));
    m_queueMutex.unlock();

    m_semNext.release();
}

void Recorder::writeBuffer(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf)
{
    sx_uint8 frame_type;

    sdvr_get_buffer_frame_type(buf, &frame_type);

    switch (frame_type)
    {
    case SDVR_FRAME_AUDIO_ENCODED:
        writeEncAudioFrames(handle, buf);
        break;

    case SDVR_FRAME_H264_IDR:
    case SDVR_FRAME_H264_I:
    case SDVR_FRAME_H264_P:
    case SDVR_FRAME_H264_B:
    case SDVR_FRAME_H264_SPS:
    case SDVR_FRAME_H264_PPS:
    case SDVR_FRAME_JPEG:
    case SDVR_FRAME_MPEG4_I:
    case SDVR_FRAME_MPEG4_P:
    case SDVR_FRAME_MPEG4_VOL:

        writeEncVideoFrames(handle, buf);
        break;
    default:
        break;
    }
}
void Recorder::writeEncVideoFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf)
{
    m_statTableMutex.lock();
    sdvr_venc_e venc;
    sx_int32 err;
    sx_uint8 frame_type;
    Camera *camPtr = NULL;

    sdvr_get_buffer_frame_type(buf, &frame_type);

	StatList::iterator i;
	for (i = m_statTable[buf->stream_id].begin(); i != m_statTable[buf->stream_id].end(); ++i)
        if ((*i).camera->handle() == handle)
        {
            camPtr = (*i).camera;
        	break;
        }

    if (camPtr == NULL)
    {
	    m_statTableMutex.unlock();
	    return;
    }


	if (i != m_statTable[buf->stream_id].end() && camPtr->isRecording(buf->stream_id)&&
        !(*i).bIsStopped)
	{
        venc = camPtr->videoFormat(buf->stream_id);
        switch (venc)
        {
        case SDVR_VIDEO_ENC_JPEG:
            // save the mjpeg frames into an avi file.
            if (g_DVR->isMJPEGRecStyleAVI())
            {
                if (camPtr->jpegIsImageStyle(buf->stream_id) == false)
                {
                    // Honor the max number of frames to record as long it is less
                    // than what is allowed in the AVI file.
                    if (g_DVR->getH264MaxFramesRecord() &&
                        (*i).aviFile.maxFramesAllowed > g_DVR->getH264MaxFramesRecord())
                        (*i).aviFile.maxFramesAllowed = g_DVR->getH264MaxFramesRecord();

				    // since we pre allocate a buffer to hold the jpeg
				    // image indecies, we should check to not record
				    // more than we can save the image indecies.
				    if ((*i).aviFile.totalFramesWrtn < (*i).aviFile.maxFramesAllowed )
                    {
                        if (!g_DVR->isSkipRecording())
					        avi_add(&(*i).aviFile, buf->payload, buf->payload_size);
                    }
				    else if ((*i).aviFile.totalFramesWrtn == (*i).aviFile.maxFramesAllowed)
				    {
					    (*i).bIsStopped = true;
					    emit interruptRecording(buf->stream_id, camPtr->id());
				    }
                }
                break;
            }
        // Saving of the jpeg image style into elemantry stream file.
        // no support for jpeg image style.
#ifdef elementary_stream
            break;
#endif
        case SDVR_VIDEO_ENC_H264:
        case SDVR_VIDEO_ENC_MPEG4:
        {
            // Cache SPS and PPS to be used in the fragmented
            // file recording.
            // Every new file needs to start with SPS and PPS since
            // the encoder only sends these information once it is
            // started, we have to save SPS/PPS frame and write it
            // into the new file before add any frames to it.
            // This condition does not apply in MJPEG recording.
          if (venc != SDVR_VIDEO_ENC_JPEG && g_DVR->isSegmentedRecording())
                camPtr->savePictureInfoFrame(buf);
            
#ifdef elementary_stream
            FILE *m_fpElementary = (*i).m_fpElementary;

#ifdef rawav_packet
            // video packet
            RawHeader hdr;
            hdr.sync_word = 0x66667273;
            hdr.header_size = BYTESWAP_16(18);
            hdr.stream_id = 0x20;
            hdr.version = 0;
            hdr.flags = 0x1;
            if (isKeyFrame(buf->frame_type))
                hdr.flags |= 0x10;
            hdr.flags = BYTESWAP_32(hdr.flags);
            hdr.payload_size = BYTESWAP_32(buf->payload_size);
            hdr.timestamp = BYTESWAP_32(buf->timestamp);
            hdr.timestamp <<= 32;
            hdr.timestamp += BYTESWAP_32(buf->timestamp_high);
            fwrite(&hdr.sync_word, 1, 4, m_fpElementary);
            fwrite(&hdr.header_size, 1, 2, m_fpElementary);
            fwrite(&hdr.stream_id, 1, 1, m_fpElementary);
            fwrite(&hdr.version, 1, 1, m_fpElementary);
            fwrite(&hdr.flags, 1, 4, m_fpElementary);
            fwrite(&hdr.payload_size, 1, 4, m_fpElementary);
            fwrite(&hdr.timestamp, 1, 8, m_fpElementary);
            fwrite(buf->payload,sizeof(sx_uint8),buf->payload_size,m_fpElementary);
#else
            if (fwrite(buf->payload,sizeof(sx_uint8),buf->payload_size,m_fpElementary) != buf->payload_size)
            {
                Log::error(QString("Error: write file error"));
                m_statTable[buf->stream_id].erase(i);
                fclose(m_fpElementary);
            }
#endif
#else
            // Stop recording. If we have recorded the maximum number of
            // frame allowed. 
            // NOTE: Maximum of zero means there is no restriction.
            // In the event we are recording a limited number of
            // frames and segmented recording, we start the new file
            // only if a key frame is arrived.
            if (g_DVR->getH264MaxFramesRecord() != 0 && 
                (*i).totalFramesWrtn > g_DVR->getH264MaxFramesRecord() &&
                (!g_DVR->isSegmentedRecording() ||
                (g_DVR->isSegmentedRecording() && 
                isKeyFrame(frame_type))))
            {
                if (!g_DVR->isSegmentedRecording())
                {
                    (*i).bIsStopped = true;

				    emit interruptRecording(buf->stream_id, camPtr->id());
                }
                else
                {
                    Camera *cam = (*i).camera;
                    uint subEnc = (*i).m_subEnc;

//mxa               close current recording file.
                    doStopRecording(i,cam, subEnc);

//                  open the new file name
                    QString qstrFileName = cam->recordFilePath(subEnc,g_DVR->isSegmentedRecording());

                    if (doOpenRecording(cam, subEnc, qstrFileName, &(*i).mp4File, 
                        &(*i).aviFile, &(*i).m_fpElementary))
                    {
                        // In case of MPEG4 and H.264, add SPS/PPS. Make sure to reset the time
                        // stamp to be the same as the latest IDR, otherwise
                        // AVMUX gets out of synch.
                        err = -1;
                        if (venc != SDVR_VIDEO_ENC_JPEG)
                        {
                            sdvr_av_buffer_t *sps_ppsBuf = cam->getPictureInfoFrame(SDVR_FRAME_H264_SPS);
                            if (sps_ppsBuf)
                            {
                                sps_ppsBuf->timestamp = buf->timestamp;
                                sps_ppsBuf->timestamp_high = buf->timestamp_high;
                                err = doPutMP4Packet(i, sps_ppsBuf);
                            }
                            if (err != MP4MUX_ERR_OK)
                            {
                                Log::error(QString("Error: Failed to add a SPS video packet into mov file for camera %1. Error = [%2]")
                                    .arg(cam->name()).arg(err));
                            }
                            sps_ppsBuf = cam->getPictureInfoFrame(SDVR_FRAME_H264_PPS);
                            err = -2;
                            if (sps_ppsBuf)
                            {
                                sps_ppsBuf->timestamp = buf->timestamp;
                                sps_ppsBuf->timestamp_high = buf->timestamp_high;
                                err = doPutMP4Packet(i, sps_ppsBuf);
                            }
                            if (err != MP4MUX_ERR_OK)
                            {
                                Log::error(QString("Error: Failed to add a PPS video packet into mov file for camera %1. Error = [%2]")
                                    .arg(cam->name()).arg(err));
                            }
                        }
                        // set a flag to start saving on the next keyframe.
 //                       (*i).bKeyFrameOnly = true;

                        // Now add the key frame buffer
                        err = doPutMP4Packet(i, buf);
                        if (err != MP4MUX_ERR_OK)
                        {
                            Log::error(QString("Error: Failed to add a video packet into mov file for camera %1. Error = [%2]")
                                .arg(cam->name()).arg(err));
                        }
                        // reset the number of frames written to the file to be
                        // used for the limited recording. All other statistics information
                        // should remain the same.
                        (*i).totalFramesWrtn = 0;

                        // Mark the camera is recording.
	                    (*i).bIsStopped = false;

                    }
                }
            }
            else
            {
                // Check if we MUST start recording on the key frame.
                // This is the case if we are in the segmented recording and we just
                // opened a new file.
                if ((*i).bKeyFrameOnly && !isKeyFrame(frame_type))
                    break;

                (*i).bKeyFrameOnly = false;
                err = doPutMP4Packet(i, buf);
                if (err != MP4MUX_ERR_OK)
                {
                    Log::error(QString("Error: Failed to add a video packet into mov file for camera %1. Error = [%2]")
                        .arg(camPtr->name()).arg(err));
                }

                // Increment the number of frames written to the file to be
                // used for the limited recording
                (*i).totalFramesWrtn++;
            }
#endif
            break;
        }

        case SDVR_VIDEO_ENC_NONE:
        default:
            break;
        }

        if (m_statsEnabled[buf->stream_id])
        {
            // Skip the MPEG4 VOL frames for calculating Frame Rates. It
            // Only contains the header information. 
            if (frame_type != SDVR_FRAME_MPEG4_VOL)
            {
                bool bCountFrame = true;
                sx_uint64 frameInterval = 0;
                sx_uint64 longTimeStamp = (sx_uint64)buf->timestamp | ((sx_uint64)buf->timestamp_high << 32);
                int nNumDropFrames = 0;

                (*i).lastTimeStamp = camPtr->getLastStatTimeStamp(buf->stream_id);
                (*i).firstTimeStamp = camPtr->getFirstStatTimeStamp(buf->stream_id);

                if ((*i).lastTimeStamp)
                {
                    int stdFrameRate = sdvr::frameRate(g_DVR->videoStandard());
                    frameInterval = longTimeStamp - (*i).lastTimeStamp;
                    // We can only calculate the dropped frame rate for the maximum
                    // FPS for either NTSC or PAL.
                    // For NTSC 30 FPS, the frame interval is approximately 3337
                    // and for PAL 25 FPS is 4000 assuming 100Mhz Clock

                    nNumDropFrames = (frameInterval - 200) / (stdFrameRate == 30 ? 3337 : 4000);
                    bCountFrame = frameInterval;
                    if (g_DVR->isShowDropFrames() && frameInterval && nNumDropFrames)
                    {
                        QString qstrMsg;
                        camPtr->incDropFrameCount(buf->stream_id, nNumDropFrames);
                        qstrMsg = QString("Dropped video frame on camera %1. Frame Interval = %2").arg(camPtr->name()).arg(frameInterval);
//                        Log::error(qstrMsg);
                    }
                }

                if (!(*i).firstTimeStamp)
                {
                    camPtr->setFirstStatTimeStamp(buf->stream_id, longTimeStamp);
                }

                if (bCountFrame)
                {
                    (*i).bytes  += buf->payload_size;
                    (*i).frames += 1;
                }
                camPtr->setLastStatTimeStamp(buf->stream_id, longTimeStamp);
            } // if (frame_type != SDVR_FRAME_MPEG4_VOL)
        } // if (m_statsEnabled[buf->stream_id])
	} //if (i != m_statTable[buf->stream_id].end() && camPtr->isRecording(buf->stream_id)&& ...

	m_statTableMutex.unlock();
}
void Recorder::writeEncAudioFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf)
{
    QMutexLocker locker(&m_statTableMutex);

    sdvr_aenc_e  aenc = SDVR_AUDIO_ENC_G711;
    sdvr_firmware_ver_t  version;
    Camera *camPtr = NULL;

    g_DVR->getFWVersion(&version);


	StatList::iterator i;

    // For audio there is only one encoder
    uint subEnc = 0;
	for (i = m_statTable[subEnc].begin(); i != m_statTable[subEnc].end(); ++i)
        if ((*i).camera->handle() == handle)
        {
            camPtr = (*i).camera;
        	break;
        }
    if (camPtr == NULL)
	    return;

    // NOTE: TODO: Currently we are saving audio frame only when
    //       saving into MOV file format. The audio will be ignored for
    //       any other file format.
	if (camPtr && camPtr->isRecording(buf->stream_id) &&
        (((camPtr->videoFormat(buf->stream_id) == SDVR_VIDEO_ENC_H264 ||
        camPtr->videoFormat(buf->stream_id) == SDVR_VIDEO_ENC_MPEG4)) ||
        (camPtr->videoFormat(buf->stream_id) == SDVR_VIDEO_ENC_JPEG &&
         !g_DVR->isMJPEGRecStyleAVI())))

	{
//todo: need to change this to use real audioformat
        switch (aenc) //camPtr->audioFormat())
        {
        case SDVR_AUDIO_ENC_G711:
        {
            mp4_packet_t pkt_a;

            memset(&pkt_a, 0, sizeof(mp4_packet_t));
            pkt_a.data = buf->payload;
            pkt_a.size = buf->payload_size;

            if (camPtr->getLastAudioTimeStamp())
            {
                int delta;
                QString qstrMsg;

                delta = buf->timestamp - camPtr->getLastAudioTimeStamp();
                camPtr->setAccuAudioDelta(delta - 2560);
                if (g_DVR->isShowDropFrames() && delta > 2650)
                {
                   qstrMsg = QString("Dropped audio frame on camera %1. Frame Interval = %2").arg(camPtr->name()).arg(delta);
//                   Log::error(qstrMsg);
                }
            }
            camPtr->setLastAudioTimeStamp(buf->timestamp);
            camPtr->setLastAudioSize(buf->payload_size);

#ifdef rawav_packet
            FILE *m_fpElementary = (*i).m_fpElementary;

            RawHeader hdr;
            hdr.sync_word = 0x66667273;
            hdr.header_size = BYTESWAP_16(18);
            hdr.stream_id = 0x40;
            hdr.version = 0;
            hdr.flags = BYTESWAP_32(0x1);
            hdr.payload_size = BYTESWAP_32(buf->payload_size);
            hdr.timestamp = BYTESWAP_32(buf->timestamp);
            hdr.timestamp <<= 32;
            hdr.timestamp += BYTESWAP_32(buf->timestamp_high);
            fwrite(&hdr.sync_word, 1, 4, m_fpElementary);
            fwrite(&hdr.header_size, 1, 2, m_fpElementary);
            fwrite(&hdr.stream_id, 1, 1, m_fpElementary);
            fwrite(&hdr.version, 1, 1, m_fpElementary);
            fwrite(&hdr.flags, 1, 4, m_fpElementary);
            fwrite(&hdr.payload_size, 1, 4, m_fpElementary);
            fwrite(&hdr.timestamp, 1, 8, m_fpElementary);
            fwrite(buf->payload,sizeof(sx_uint8),buf->payload_size,m_fpElementary);
#else
            // The 64 bit timestamp only exist in 3.2.x.x or higher of firmware
            if (sdvr::isMinVersion(version.fw_major,version.fw_minor, 0 , 0, 3, 2, 0, 0))
            {
                pkt_a.pts = pkt_a.dts = (sx_uint64)buf->timestamp | ((sx_uint64)buf->timestamp_high << 32);
            }
            else
            {
                pkt_a.pts = pkt_a.dts =  buf->timestamp;  //(*i).mp4File.dts;
            }

            // The payload contains 2560 samples of 8 bytes. The payload_size
            // was already re-asjusted in the SDK when it was decoded.
            pkt_a.sample_count = buf->payload_size;

            pkt_a.flags = 0;        // This field is unused for audio.
            if (!g_DVR->isSkipRecording())
                mp4mux_put_packet((*i).mp4File.FileHandle, (*i).mp4File.atrack, &pkt_a);
#endif
            break;
        }

        case SDVR_AUDIO_ENC_NONE:
        default:
            break;
        }
	}

}

bool Recorder::doOpenRecording(Camera *cam, uint subEnc, const QString &fileName,
                               mp4File_t *mp4File,
                               aviFile_t *aviFile, FILE **fpElementary)
{
    mp4_track_info_t track;
    mp4_info_t info;
    QSize imageSize = sdvr::frameSize(g_DVR->videoStandard(), cam->decimation(subEnc));
    QByteArray baFile(fileName.toAscii());
    sdvr_venc_e venc;
    int vHeight;

    *fpElementary = NULL;

    memset(&info, 0, sizeof(info));
    mp4File->vtrack = -1;
    mp4File->atrack = -1;
    mp4File->FileHandle = 0;

    aviFile->frameLen = 0;
    // TODO: For now we are limiting each avi file to 
    //       1 Hr worth of frames.
    aviFile->maxFramesAllowed = 30 * 60 * 60;
    aviFile->totalFramesWrtn = 0;

    if (g_DVR->isSkipRecording())
        return true;

    venc = cam->videoFormat(subEnc);
    switch (venc)
    {
    case SDVR_VIDEO_ENC_JPEG:
        // save the mjpeg frames into an avi file.
        if (g_DVR->isMJPEGRecStyleAVI())
        {
            if (cam->jpegIsImageStyle(subEnc) == false)
            {
                int scale  = 1;     // We assume scale is always 1

                if (!avi_open(aviFile,fileName.toAscii(), scale, cam->frameRate(subEnc), 
                     imageSize.width(), imageSize.height(), aviFile->maxFramesAllowed))
                {
                    Log::error(QString("Error: failed to open file '%1' for recording.")
                        .arg(fileName));

                    return false;
                }
            }
            break;
        }
#ifdef elementary_stream
    // Saving of the jpeg image style into elemantry stream file
    // is not supported.
        break;
#endif
    case SDVR_VIDEO_ENC_H264:
    case SDVR_VIDEO_ENC_MPEG4:
    {
#ifdef elementary_stream
        *fpElementary = fopen(baFile.data(), "wb");

        if (*fpElementary == NULL)
        {
            Log::error(QString("Error: failed to open file '%1' for recording ")
                .arg(fileName));

            return false;
        }
        // Move to the end of the file to append the data
        fseek(*fpElementary,0,SEEK_END);
#ifdef rawav_packet
        // SDT packet
        unsigned char payload[12] = {0, 0, 0, 0, 0x20, 0x3, 0, 0, 0x40, 0x80, 0, 0};
        RawHeader hdr;
        hdr.sync_word = 0x66667273;
        hdr.header_size = BYTESWAP_16(10);
        hdr.stream_id = 1;
        hdr.version = 0;
        hdr.flags = BYTESWAP_32(0);
        hdr.payload_size = BYTESWAP_32(sizeof(payload));
        fwrite(&hdr, 1, 16, *fpElementary);
        fwrite(payload, 1, sizeof(payload), *fpElementary);
#endif
#else
        info.file_name = baFile.data();
        info.mux_mode = MP4_RECORD;
        info.file_mode = MODE_MOV;
        // Timescale is based on 90k Hz clock.
        mp4File->FileHandle = mp4mux_open(&info);
        if (mp4File->FileHandle == 0) {
            Log::error(QString("Error: failed to open file '%1' for recording.")
                .arg(fileName));

            return false;
        }

        // Add Video Track to mp4 file
        memset(&track, 0, sizeof(mp4_track_info_t));
        track.type = MP4_TRACK_VIDEO;

        // Translate the video encoder with the CODEC ID for .mov file.
        switch (venc)
        {
        case SDVR_VIDEO_ENC_MPEG4:
            track.codec_id =  CODEC_ID_MPEG4;           
            break;

        case SDVR_VIDEO_ENC_JPEG:
            // Use JPEG ID. The motion jpeg will not work;
            track.codec_id =  CODEC_ID_JPEG;           
            break;

        case  SDVR_VIDEO_ENC_H264:
        default:
            track.codec_id =  CODEC_ID_H264;           
            break;

        }
        // Timescale is based on 90k Hz clock.
        track.timescale = 90000; //30000; 
        track.width = imageSize.width();
        // Make sure that the video height is multiple of 16;
        vHeight = imageSize.height();
        vHeight /=  16;
        vHeight *= 16;
        track.height = vHeight;
        track.sample_size = 0;
        track.language = 0;
        track.default_duration = g_DVR->getVideoRecDuration(g_DVR->videoStandard()); // for NTSC
        track.default_size = 0; // variable frame size

        if (mp4mux_add_track(mp4File->FileHandle, &track) == MP4MUX_ERR_OK)
            mp4File->vtrack = track.track_id;

        else
        {
            Log::error(QString("Error: failed to add video track for file '%1'.")
                .arg(fileName));
            mp4mux_close(mp4File->FileHandle);

            return false;
        }
        // Add audio Track to mp4 file
        sx_int8  mp4mux_lang[4] = {'e', 'n', 'g', '\0'};
        memset(&track, 0, sizeof(mp4_track_info_t));
        track.type = MP4_TRACK_AUDIO;
        track.codec_id = CODEC_ID_PCM_MULAW;
        // Audio sampling rate
        if (g_DVR->getAudioRate() == SDVR_AUDIO_RATE_16KHZ)
             track.timescale = 16000;
        else
            track.timescale =  8000;; 
        track.channels = 2; // a stereo recording
        track.sample_size = 1 * track.channels;
        track.default_duration = 2560 / track.channels; // packet size in bytes / channels
        track.default_size = 1 * track.channels; // one byte per channel for PCM

        track.language = mp4mux_iso639_to_lang(mp4mux_lang, 1);

        // add track
        if (mp4mux_add_track(mp4File->FileHandle, &track) == MP4MUX_ERR_OK)
            mp4File->atrack = track.track_id;

        else
        {
            Log::error(QString("Error: failed to add audio track for file '%1'.")
                .arg(fileName));
            mp4mux_close(mp4File->FileHandle);

            return false;
        }
#endif
        break;
    }
    case SDVR_VIDEO_ENC_NONE:
    default:
        return false;

    }

    return true;
}
bool Recorder::startRecording(uint subEnc, Camera *cam, const QString &fileName)
{
    mp4File_t mp4File;
    FILE *fpElementary = NULL;
    aviFile_t aviFile;
    bool bRet = false;

    QMutexLocker locker(&m_statTableMutex);

	// Check if the channel is already being recorded
	StatList::iterator i;
	for (i = m_statTable[subEnc].begin(); i != m_statTable[subEnc].end(); ++i)
        if ((*i).camera == cam)
        	break;

    // If could not find the camera in the recording list,
    // try to open and initialize the recording files approporiately
	if (i == m_statTable[subEnc].end())
    {
        bRet = doOpenRecording(cam, subEnc, fileName, &mp4File, &aviFile, &fpElementary);
        if (bRet)
        {
            m_statTable[subEnc].append(stat_t(cam, subEnc, fpElementary, aviFile,mp4File));
        }
    }
    return bRet;
}

bool Recorder::doStopRecording(StatList::iterator &i, Camera *cam, uint subEnc)
{
    bool bRet = true;

    // Mark the camera is interrupted to stop.
	(*i).bIsStopped = true;

    if (g_DVR->isSkipRecording())
        return true;

    switch (cam->videoFormat(subEnc))
    {
    case SDVR_VIDEO_ENC_JPEG:
        if (g_DVR->isMJPEGRecStyleAVI())
        {
            if (cam->jpegIsImageStyle(subEnc) == false)
                avi_close(&(*i).aviFile);
            break;
        }
    case SDVR_VIDEO_ENC_H264:
    case SDVR_VIDEO_ENC_MPEG4:
#ifdef elementary_stream
        // saving of the jpeg image style into elemantry stream file
        // is not supported
        if (cam->videoFormat(subEnc) != SDVR_VIDEO_ENC_JPEG)
            fclose((*i).m_fpElementary);
#else
        mp4mux_close((*i).mp4File.FileHandle);
#endif
        break;

    case SDVR_VIDEO_ENC_NONE:
    default:
        bRet = false;
    }

    return bRet;
}

bool Recorder::stopRecording(uint subEnc, Camera *cam)
{
    bool bRet = false;
    QMutexLocker locker(&m_statTableMutex);

	// Check if the channel is already being recorded
	StatList::iterator i;
	for (i = m_statTable[subEnc].begin(); i != m_statTable[subEnc].end(); ++i)
        if ((*i).camera == cam)
        	break;

	if (i != m_statTable[subEnc].end())
	{
        if ((bRet = doStopRecording(i, cam, subEnc)) == true)
        {
    		m_statTable[subEnc].erase(i);
        }
	}

	return bRet;
}

sx_int32 Recorder::doPutMP4Packet(StatList::iterator &i, sdvr_av_buffer_t *buf)
{
    sx_int32 err = 0;
    mp4_packet_t pkt_v;
    sdvr_firmware_ver_t  version;
    sx_uint8 frame_type;

    g_DVR->getFWVersion(&version);

    memset(&pkt_v, 0, sizeof(mp4_packet_t));
    sdvr_av_buf_payload(buf,&pkt_v.data, (sx_uint32 *)&pkt_v.size);
    pkt_v.sample_count = 1;      // set always to 1

    // 64 bit timestamp only exists in 3.2.x.x or higher of firmware
    if (sdvr::isMinVersion(version.fw_major,version.fw_minor, 0 , 0, 3, 2, 0, 0))
    {
        sdvr_get_buffer_timestamp(buf,(sx_uint64 *)&pkt_v.dts);
        pkt_v.pts = pkt_v.dts;
    }
    else
    {
        pkt_v.pts = pkt_v.dts =  buf->timestamp;  //(*i).mp4File.dts;
    }
    sdvr_get_buffer_frame_type(buf,&frame_type);
    if (isKeyFrame(frame_type))
        pkt_v.flags |= PKT_FLAG_KEY_FRAME;

    if (!g_DVR->isSkipRecording())
        err = mp4mux_put_packet((*i).mp4File.FileHandle, (*i).mp4File.vtrack, &pkt_v);

    return err;
}

Recorder::StatList Recorder::takeStats(uint subEnc)
{
    QMutexLocker locker(&m_statTableMutex);

    StatList result;

	StatList::iterator i;
	for (i = m_statTable[subEnc].begin(); i != m_statTable[subEnc].end(); ++i)
    {
        result.append(*i);
        (*i).lastTimeStamp = (*i).firstTimeStamp = (*i).bytes = (*i).frames = 0;
        (*i).camera->setLastStatTimeStamp(subEnc,0);
        (*i).camera->setFirstStatTimeStamp(subEnc,0);

    }

    return result;
}

void Recorder::enableStats(uint subEnc)
{
    QMutexLocker locker(&m_statTableMutex);

	StatList::iterator i;
	for (i = m_statTable[subEnc].begin(); i != m_statTable[subEnc].end(); ++i)
    {
        (*i).lastTimeStamp = (*i).firstTimeStamp = (*i).bytes = (*i).frames = 0;
    }
    m_statsEnabled[subEnc] = true;
}

void Recorder::disableStats(uint subEnc)
{
    m_statsEnabled[subEnc] = false;
}

bool Recorder::isKeyFrame(sdvr_frame_type_e frame_type)
{
    switch (frame_type)
    {
    case SDVR_FRAME_H264_IDR:
    //case SDVR_FRAME_H264_I: //I is not a key frame.
    case SDVR_FRAME_MPEG4_I:
    case SDVR_FRAME_JPEG:
        return true;

    default:
        return false;
    }
}
bool Recorder::isPictureInfoFrame(sdvr_frame_type_e frame_type)
{
    switch (frame_type)
    {
    case SDVR_FRAME_H264_SPS:
    case SDVR_FRAME_H264_PPS:
    case SDVR_FRAME_MPEG4_VOL:
        return true;

    default:
        return false;
    }

}
