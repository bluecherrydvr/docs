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

#include <QVariant>
#include <QFileInfo>
#include "viewsettings.h"
#include "camera.h"
#include "player.h"
#include "dvrsystem.h"
#include "mainwindow.h"
#include "sdvr.h"
#include "log.h"
#include "videopanel.h"


extern DVRSystem *g_DVR;

static const char *TN_PLAYER = "player";
static const char *TN_PLAYERID = "id";
static const char *TN_PLAYERNAME = "name";

static const char *TN_ATTACH = "attach";
static const char *TN_BOARD = "board";
static const char *TN_CHANNUM = "channum";

static const char *TN_ATTACHEDCAMID = "attachedcamid";
static const char *TN_ATTACHEDFILE = "attachedfile";
static const char *TN_VIDEODECODER = "videodecoding";
static const char *TN_AUDIODECODER = "audioformat";
static const char *TN_FORMAT = "format";
static const char *TN_VIDEOSIZE = "videosize";

CPlayer::CPlayer() :
    m_decoder(this)
{
    m_id = 0;
    m_boardIndex = -1;
    m_chanNum = -1;
    m_handle = 0;

    // todo: for now the RAW video frame rate is hard coded to the
    // frame rate of the current video standard.
    int stdFrameRate = sdvr::frameRate(g_DVR->videoStandard());
    setYUVFrameRate(stdFrameRate);

    setVideoFormat(SDVR_VIDEO_ENC_NONE);
    setAudioFormat(SDVR_AUDIO_ENC_NONE);

    m_cameraAttached = NULL;
    m_cameraAttachedId = -1;

    m_rawVideoEnabled = false;

    m_playbackState = PLAYER_STATE_STOPPED;
}

bool CPlayer::attachEncodedFile(QString path,
        sdvr_venc_e vFormat, sdvr_aenc_e aFormat)
{
    m_cameraAttached = NULL;
    m_cameraAttachedId = -1;
    m_encodedFileAttached = path;
    setVideoFormat(vFormat);
    setAudioFormat(aFormat);

    return true;
}

bool CPlayer::attachCamera(Camera *c)
{
    m_cameraAttached = c;
    m_cameraAttachedId = m_cameraAttached->id();
//todo: for now we are only attaching from the primary encoder
//     must add the secondary encoder.
    setVideoFormat(m_cameraAttached->videoFormat(SDVR_ENC_PRIMARY));
    setAudioFormat(m_cameraAttached->audioFormat());
// todo: set the file path from the video format;

    return true;
}

bool CPlayer::attach(int boardIndex, int chanNum)
{
    if (isAttached())
        return false;

    m_boardIndex = boardIndex;
    m_chanNum = chanNum;
    return true;
}

bool CPlayer::isAttached() const
{
    return m_boardIndex != -1 && m_chanNum != -1;
}

bool CPlayer::detach()
{
    if (isAttached())
    {
        m_boardIndex = -1;
        m_chanNum = -1;
        return true;
    }
    return false;
}

bool CPlayer::openChannel()
{
    QString qstrMsg;
    bool bRet;
    sdvr_video_size_e video_size;

    if (!isAttached() || isChannelOpen() || m_encodedFileAttached.isEmpty())
        return false;

    qstrMsg = QString("Creating %1...").arg(getName());
    g_MainWindow->setStatusBar(qstrMsg);

    sdvr_err_e rc = SDVR_DRV_ERR_MSG_RECV;

    if (!g_MainWindow->isFirmwareTimeout(m_boardIndex))
    {
        sdvr_chan_def_t chan_def;
        chan_def.board_index = getBoardIndex();
        chan_def.chan_num = getChanNum();
        chan_def.chan_type = SDVR_CHAN_TYPE_DECODER;
        chan_def.video_format_primary = getVideoFormat();
        // There is no secondary decoder
        chan_def.video_format_secondary = SDVR_VIDEO_ENC_NONE;
        chan_def.audio_format = getAudioFormat();

        // Get the video size from the file attached to this player
        bRet = sdvr::getVideoSizeFromFile(&video_size,getEncodedFilePath());
        if (bRet)
        {
            setVideoSize(video_size);
            rc = sdvr_create_chan(&chan_def, &m_handle);
            if (rc == SDVR_ERR_NONE)
            {
                QSize vSize = sdvr::VSizeToWidthHeight(video_size);
                sdvr_set_decoder_size(m_handle,vSize.width(),vSize.height());
            }
        }
        else
        {
            QString qstrPrompt = QString("%1: Failed to create player.").arg(getName());
            Log::error(QString("Error: Bad or missing file '%1' attached to player %2")
                .arg(getEncodedFilePath()).arg(getName()));

            g_MainWindow->setStatusBar(qstrPrompt);
            return false;
        }
    }

    if (rc != SDVR_ERR_NONE)
    {
        if (rc == SDVR_DRV_ERR_MSG_RECV)
            g_MainWindow->setFirmwareTimeout(m_boardIndex);

        Log::error(QString("%1: Failed to create player. Decoder Channel = %2 Board Id = %3. Error = %4 [%5]")
                   .arg(getName()).arg(getChanNum()).arg(getBoardIndex() + 1).arg(sdvr_get_error_text(rc)).arg(rc));
        return false;
    }

    g_MainWindow->setStatusBar("Ready");
    return true;
}

bool CPlayer::closeChannel()
{
    if (isChannelOpen())
    {
        sdvr_err_e rc = SDVR_DRV_ERR_MSG_RECV;

        if (isDecoding())
            stopDecoding();

        g_MainWindow->setStatusBar(QString("Destroying %1...").arg(getName()));

        rc = sdvr_destroy_chan(m_handle);

        if (rc == SDVR_DRV_ERR_MSG_RECV)
        {
            g_MainWindow->setFirmwareTimeout(m_boardIndex);
            g_MainWindow->setStatusBar("The firmware timeout. You must exit.");
        }
        else
            g_MainWindow->setStatusBar("Ready");

        m_handle = 0;
        return true;
    }

    return false;
}

sdvr_err_e CPlayer::applyYUVFrameRate()
{
    sdvr_err_e err = SDVR_ERR_NONE;
    // Force the frame rate change if the
    // raw video is streaming.
    if (isRawVideoEnabled())
        err = setRawVideo(g_MainWindow->currentMonitorSettings()->decimation(), true);

    return err;
}
sdvr_err_e CPlayer::setRawVideo(sdvr_video_res_decimation_e dec, bool enabled)
{
	sdvr_err_e err = SDVR_DRV_ERR_MSG_RECV;

	if (enabled)
        g_MainWindow->setStatusBar(QString("Starting HMO for %1...").arg(getName()));
	else
        g_MainWindow->setStatusBar(QString("Stopping HMO for %1...").arg(getName()));


    if (!g_MainWindow->isFirmwareTimeout(m_boardIndex))
        err = sdvr_stream_raw_video(m_handle, dec, getYUVFrameRate(), enabled);
    if (err == SDVR_ERR_NONE)
    {
    	m_rawVideoEnabled = enabled;
    }

    // Close the YUV file if we are saving the YUV buffer
    // for this camera and we are stopping the raw video streaming
    if ((enabled == false) && g_MainWindow->getyuvSaveHandle() &&
        (g_MainWindow->getyuvSaveHandle() == m_handle))
        g_MainWindow->closeYUVFile();


    // If the firmware timeout, force the channel to be disabled.
    if (err == SDVR_DRV_ERR_MSG_RECV) {
        m_rawVideoEnabled = false;
    }

    Log::chk_err(QString("Failed to %1 the HMO.").arg(enabled ? "start" : "stop"),err, true, m_boardIndex);

    return err;
}

sdvr_err_e CPlayer::setRawAudio(bool enabled)
{
	sdvr_err_e err = SDVR_DRV_ERR_MSG_RECV;

	if (enabled)
        g_MainWindow->setStatusBar(QString("%1: Starting raw audio streaming...").arg(getName()));
	else
        g_MainWindow->setStatusBar(QString("%1: Stopping raw audio streaming...").arg(getName()));


    if (!g_MainWindow->isFirmwareTimeout(m_boardIndex))
        err = sdvr_stream_raw_audio(m_handle, enabled);
    if (err == SDVR_ERR_NONE)
    {
    	m_rawVideoEnabled = enabled;
    }

    // Close the raw audio file if we are saving the raw audio buffer
    // for this player and we are stopping the raw audio streaming
    if ((enabled == false) && g_MainWindow->getRawAudioSaveHandle() &&
        (g_MainWindow->getRawAudioSaveHandle() == m_handle))
        g_MainWindow->closeRawAudioFile();


    // If the firmware timeout, force the channel to be disabled.
    if (err == SDVR_DRV_ERR_MSG_RECV) {
        m_rawVideoEnabled = false;
    }

    Log::chk_err(QString("%1: Failed to %2 raw audio streaming.").arg(getName()).arg(enabled ? "start" : "stop"),err, true, m_boardIndex);

    return err;
}

sdvr_err_e CPlayer::setSmoGrid(int x, int y, sdvr_video_res_decimation_e dec,
                              int dwellTime, bool enabled)
{
    sdvr_smo_grid_t smo_grid;

    smo_grid.top_left_mb_x = x;
    smo_grid.top_left_mb_y = y;
    smo_grid.res_decimation = dec;
    smo_grid.dwell_time = dwellTime;
    smo_grid.enable = enabled;

    sdvr_err_e err = sdvr_set_smo_grid(m_handle, &smo_grid);
    if (err == SDVR_ERR_NONE)
    	m_smoGridEnabled = enabled;

    return err;
}

bool CPlayer::startDecoding(bool bContinuousDecoding)
{
    QString enableInfo;
    bool bRet;
    sdvr_video_size_e video_size;

    if (getVideoFormat() != SDVR_VIDEO_ENC_H264 ||
        !isChannelOpen() || isDecoding() || g_MainWindow->isFirmwareTimeout(m_boardIndex))
        return false;


    g_MainWindow->setStatusBar(QString("Starting decoder for %1...").arg(getName()));

    m_missedBufCount = 0;

    // Before enabling the decoder make sure the video size
    // was not changed
    bRet = sdvr::getVideoSizeFromFile(&video_size,getEncodedFilePath());
    if (bRet)
    {
        setVideoSize(video_size);
        QSize vSize = sdvr::VSizeToWidthHeight(video_size);
        sdvr_set_decoder_size(m_handle,vSize.width(),vSize.height());
    }
    m_decoder.startDecoding(getEncodedFilePath(), bContinuousDecoding);

    if (isDecoding())
    {
        setPlaybackState(PLAYER_STATE_PLAYBACK);

        enableInfo = QString("%1: Started decoder.").arg(getName());
        Log::info(enableInfo);
    }
    else
    {
        enableInfo = QString("%1: Failed to start decoder.").arg(getName());
        Log::error(enableInfo);
        // If for any reason decoder failed to be started for any
        // player, we must stop continous decoding for all.
        g_MainWindow->setContinuousDecoding( false );
    }
    g_MainWindow->setStatusBar(enableInfo);

    return isDecoding();
}

bool CPlayer::stopDecoding()
{
    if (!isDecoding())
        return false;

    g_MainWindow->setStatusBar(QString("Stopping decoder for %1...").arg(getName()));

    if (m_decoder.stopDecoding())
    {
        setPlaybackState(PLAYER_STATE_STOPPED);

        Log::info(QString("%1: Stopped decoder.").arg(getName()));
        g_MainWindow->setStatusBar(QString("%1: Stopped decoder.").arg(getName()));
    }
    else
    {
        g_MainWindow->setStatusBar(QString("%1: Failed to stop decoder.").arg(getName()));
        // If for any reason decoder failed to be stopped for any
        // player, we must stop continous decoding for all.
        g_MainWindow->setContinuousDecoding( false );
    }

    return true;
}



bool CPlayer::fromElement(QDomElement decElem)
{
	bool ok = true;
	bool converted;
	setId(decElem.attribute(TN_PLAYERID, "").toInt(&converted));
	if (!converted)
	{
		Log::error("Bad or missing player ID");
		ok = false;
	}

	QDomElement e = decElem.firstChildElement();
	while (!e.isNull())
	{
		if (e.tagName() == TN_PLAYERNAME)
		{
			setName(e.text());
		}
		else if (e.tagName() == TN_ATTACH)
		{
			int boardIndex = e.attribute(TN_BOARD, "-1").toInt();
			int input = e.attribute(TN_CHANNUM, "-1").toInt();
            attach(boardIndex, input);
        }

        else if (e.tagName() == TN_VIDEODECODER)
        {
            setVideoFormat(sdvr::stringToVideoEnc(e.attribute(TN_FORMAT)));

            if (getVideoFormat() != SDVR_VIDEO_ENC_NONE)
            {
                QDomElement attachedCamIdElem = e.firstChildElement(TN_ATTACHEDCAMID);
                if (!attachedCamIdElem.isNull())
                {
                    int nCamId = attachedCamIdElem.text().toInt();
                    if (nCamId != -1)
                    {
                        //  check to see if the cammera ID is still valid if not
                        //  set it to -1
                        if (g_DVR->findCameraById(nCamId) == NULL)
                            nCamId = -1;
                    }
                    setCameraAttachedId(nCamId);
                }
                QDomElement attachedFileElem = e.firstChildElement(TN_ATTACHEDFILE);
                setEncodedFilePath(attachedFileElem.text());

                QDomElement videoSizeElem = e.firstChildElement(TN_VIDEOSIZE);
                setVideoSize((sdvr_video_size_e)videoSizeElem.text().toInt());

            }
        }
        else if (e.tagName() == TN_AUDIODECODER)
        {
            setAudioFormat(sdvr::stringToAudioEnc(e.text()));
        }

        e = e.nextSiblingElement();
    } //while (!e.isNull())

    return ok;
}

QDomElement CPlayer::toElement(QDomDocument &doc)
{
	QDomElement decElem = doc.createElement(TN_PLAYER);
	decElem.setAttribute(TN_PLAYERID, getId());

	QDomElement nameElem = doc.createElement(TN_PLAYERNAME);
	nameElem.appendChild(doc.createTextNode(getName()));
	decElem.appendChild(nameElem);

    QDomElement videoEncElem = doc.createElement(TN_VIDEODECODER);
    videoEncElem.setAttribute(TN_FORMAT, sdvr::videoEncToString(getVideoFormat()));

    QDomElement attachedCamIdElem = doc.createElement(TN_ATTACHEDCAMID);
    attachedCamIdElem.appendChild(doc.createTextNode(QString::number(getCameraAttachedId())));
    videoEncElem.appendChild(attachedCamIdElem);

    QDomElement attachedFileElem = doc.createElement(TN_ATTACHEDFILE);
    attachedFileElem.appendChild(doc.createTextNode(getEncodedFilePath()));
    videoEncElem.appendChild(attachedFileElem);

    QDomElement videoSizeElem = doc.createElement(TN_VIDEOSIZE);
    videoSizeElem.appendChild(doc.createTextNode(QString::number(getVideoSize())));
    videoEncElem.appendChild(videoSizeElem);
    decElem.appendChild(videoEncElem);

    QDomElement audioDecElem = doc.createElement(TN_AUDIODECODER);
    audioDecElem.appendChild(
        doc.createTextNode(sdvr::audioEncToString(getAudioFormat())));
    decElem.appendChild(audioDecElem);

    if (isAttached())
    {
        QDomElement attachElem = doc.createElement(TN_ATTACH);
        attachElem.setAttribute(TN_BOARD, getBoardIndex());
        attachElem.setAttribute(TN_CHANNUM, getChanNum());
        decElem.appendChild(attachElem);
    }

    return decElem;
}

void CPlayer::setVideoSize(sdvr_video_size_e vSize)
{
    m_vSize = vSize;
}
sdvr_video_size_e CPlayer::getVideoSize()
{
    return m_vSize;
}

