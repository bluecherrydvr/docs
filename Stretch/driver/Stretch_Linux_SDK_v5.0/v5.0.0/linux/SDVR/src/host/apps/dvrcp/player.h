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

#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QDomElement>
#include <QRect>
#include "decoder.h"
#include "sdvr_sdk.h"

class Camera;

typedef enum _playerRunningState_e {
    PLAYER_STATE_STOPPED,
    PLAYER_STATE_PLAYBACK,
    PLAYER_STATE_PAUSED,
	PLAYER_STATE_STOP_PENDING
}playerRunningState_e;
class CPlayer
{
public:
    CPlayer();

    void setId(int id) { m_id = id; }
    int getId() const { return m_id; }

    void setName(const QString &name) { m_name = name; }
    const QString &getName() const { return m_name; }

    bool attach(int boardIndex, int chanNum);
    bool detach();
    bool isAttached() const;

    int getBoardIndex() const { return m_boardIndex; }
    int getChanNum() const { return m_chanNum; }


    // Note: We can either attach an encoded file name to be
    // decoded or Camera
    bool attachEncodedFile(QString path, 
        sdvr_venc_e vFormat, sdvr_aenc_e aFormat = SDVR_AUDIO_ENC_NONE); // Use DVRSystem::attach
    bool attachCamera(Camera *c);

    const Camera *getCameraAttached() const {return m_cameraAttached;}


    bool openChannel();
    bool closeChannel();
    bool isChannelOpen() const { return m_handle != 0; }

    sdvr_chan_handle_t handle() const { return m_handle; }

    // setVideoFormat() and setAudioFormat() are ignored if
    // a camera is attached
    void setVideoFormat(sdvr_venc_e format) { m_videoFormat = format; }
    sdvr_venc_e getVideoFormat() const { return m_videoFormat; }

    void setAudioFormat(sdvr_aenc_e format) { m_audioFormat = format; }
    sdvr_aenc_e getAudioFormat() const { return m_audioFormat; }


    void setVideoSize(sdvr_video_size_e vSize);
    sdvr_video_size_e getVideoSize();

    void setCameraAttachedId(int id) { m_cameraAttachedId = id;}
    int  getCameraAttachedId() {return m_cameraAttachedId;}

    // Raw Video methods
    void setYUVFrameRate(sx_uint8 rate)  { m_yuvFrameRate = rate; }
    sx_uint8 getYUVFrameRate() const   { return m_yuvFrameRate; }

    sdvr_err_e applyYUVFrameRate();

    sdvr_err_e setRawVideo(sdvr_video_res_decimation_e dec, bool enabled);
    bool isRawVideoEnabled() const { return m_rawVideoEnabled; }

    sdvr_err_e setRawAudio(bool enabled);
    bool isRawAudioEnabled() const { return m_rawAudioEnabled; }

    sdvr_err_e setSmoGrid(int x, int y, sdvr_video_res_decimation_e dec,
                         int dwellTime, bool enabled);
    bool isSmoGridEnabled() const { return m_smoGridEnabled; }

    bool startDecoding(bool bContinuousDecoding = false);
    bool stopDecoding();
    bool isDecoding() const { return m_decoder.isRunning(); }
    CDecoder &decoder() { return m_decoder; }

    QString getEncodedFilePath() const {return m_encodedFileAttached;}
    void setEncodedFilePath(QString filePath) {m_encodedFileAttached = filePath;}

    bool fromElement(QDomElement e);
    QDomElement toElement(QDomDocument &doc);

    playerRunningState_e getPlaybackState() {return m_playbackState;}
    void setPlaybackState(playerRunningState_e state) {m_playbackState = state;}

    void incMissedBufCount() {m_missedBufCount++;}
    uint getMissedBufCount() {return m_missedBufCount;}

private:
	int m_id;           // The internal decoder ID used in the xml file.
    int m_boardIndex;
    int m_chanNum;       // The channel ID to create the decoder on. 
                                   
    sdvr_chan_handle_t m_handle;

    sdvr_venc_e m_videoFormat;
    sdvr_aenc_e m_audioFormat;

    sdvr_video_size_e m_vSize;

    QString m_name;

    Camera *m_cameraAttached;
    int m_cameraAttachedId;
    QString m_encodedFileAttached;

    bool m_rawVideoEnabled;
    bool m_rawAudioEnabled;

    bool m_smoGridEnabled;

    // Raw Video parameters
    sx_uint8 m_yuvFrameRate;

    playerRunningState_e m_playbackState;

    unsigned int m_missedBufCount;

    CDecoder m_decoder;
};

#endif /* CAMERA_H */
