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

#include "smoyuvplayer.h"
#include "spotmonitor.h"
#include "dvrsystem.h"
#include "dvrboard.h"
#include "log.h"
#include "sdvr.h"
#include <QThread>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>

extern DVRSystem *g_DVR;


class SmoYuvPlayerThread : public QThread
{
public:
    SmoYuvPlayerThread(SmoYuvPlayer *player);

    void setInterval(int interval) { m_interval = interval; }
    void stop();

protected:
    virtual void run();

private:
    SmoYuvPlayer *m_player;
    int m_interval;  // milliseconds
};

SmoYuvPlayer::SmoYuvPlayer(SpotMonitor *smo, QWidget *parent) : QWidget(parent)
{
    int nFrameRate;
    setupUi(this);

    m_smo = smo;
    m_sender = new SmoYuvPlayerThread(this);
    m_videoOutHandle = INVALID_CHAN_HANDLE;

    FormatCombo->addItem("4:2:0");

    nFrameRate = 30; //sdvr::frameRate(g_DVR->videoStandard());
    FrameRateEdit->setValidator(new QIntValidator(1,
        nFrameRate, FrameRateEdit));
    FrameRateEdit->setText(QString::number(nFrameRate));

    connect(BrowseButton, SIGNAL(clicked()), SLOT(onBrowseButtonClicked()));
    connect(PlayButton, SIGNAL(clicked()), SLOT(onPlayButtonClicked()));
    connect(PauseButton, SIGNAL(clicked()), SLOT(onPauseButtonClicked()));
    connect(StopButton, SIGNAL(clicked()), SLOT(onStopButtonClicked()));

    connect(m_sender, SIGNAL(finished()), SLOT(onThreadFinished()));
    connect(m_sender, SIGNAL(terminated()), SLOT(onThreadFinished()));

    setState(STOPPED);
}

SmoYuvPlayer::~SmoYuvPlayer()
{
    m_sender->stop();
    delete m_sender;
}

void SmoYuvPlayer::setState(State state)
{
    m_state = state;

    switch (state)
    {
    case STOPPED:
        PlayButton->setEnabled(true);
        PauseButton->setEnabled(false);
        StopButton->setEnabled(false);
        break;

    case PAUSED:
        PlayButton->setEnabled(true);
        PauseButton->setEnabled(false);
        StopButton->setEnabled(true);
        break;

    case PLAYING:
        PlayButton->setEnabled(false);
        PauseButton->setEnabled(true);
        StopButton->setEnabled(true);
    }

    GroupBox->setEnabled(m_state == STOPPED);
}

void SmoYuvPlayer::onPlayButtonClicked()
{
    if (state() == PAUSED)
    {
        setState(PLAYING);
        return;
    }

    if (state() != STOPPED)
        return;

    Q_ASSERT(!m_sender->isRunning());

    bool ok;
    sdvr_video_std_e videoStd = g_DVR->videoStandard();
    uint stdFrameRate = sdvr::frameRate(videoStd);
    uint frameRate = FrameRateEdit->text().toUInt(&ok);
    if (!ok || frameRate <= 0 || frameRate > stdFrameRate)
    {
        QMessageBox::critical(this, tr("Error"), tr("Illegal frame rate"));
        return;
    }

    QString filePath = FilePathEdit->text();
    if (filePath.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"),
                tr("YUV file path is required"));
        return;
    }

    m_file.setFileName(filePath);
    if (!m_file.open(QFile::ReadOnly))
    {
        QMessageBox::critical(this, tr("Error"),
                QString("%1 : %2").arg(m_file.fileName()).arg(m_file.errorString()));
        return;
    }

    setState(PLAYING);
    m_sender->setInterval(1000/frameRate);
    m_sender->start();
    if (!m_sender->isRunning())
        setState(STOPPED);
}

void SmoYuvPlayer::onBrowseButtonClicked()
{
    QString openFile = QFileDialog::getOpenFileName(this, "Choose YUV file",
            g_DVR->systemDirPath());

    if (!openFile.isNull())
        FilePathEdit->setText(openFile);
}

void SmoYuvPlayer::onPauseButtonClicked()
{
    if (state() == PLAYING)
        setState(PAUSED);
}

void SmoYuvPlayer::onStopButtonClicked()
{
    m_sender->stop();
}

void SmoYuvPlayer::sendNextFrame()
{
    sx_uint32 rc;
    sdvr_av_buffer_t *buffer;
    sdvr_err_e status;
    sx_uint8 *payload;
    sx_uint32 payloadSize;

    if (state() != PLAYING)
        return;

read: 
    status = sdvr_get_video_output_buffer(m_videoOutHandle, &buffer);
    // If the buffer is not available just try again later.
    // We should not stop playback.
    if (status == SDVR_ERR_BUF_NOT_AVAIL)
        return;

    if (status != SDVR_ERR_NONE && status != SDVR_ERR_BUF_NOT_AVAIL)
    {
        Log::error(QString("Failed to get video output buffer : %1")
            .arg(sdvr_get_error_text(status)));

        setState(STOPPED);
        m_sender->stop();
        return;
    }
    sdvr_av_buf_payload(buffer, &payload, &payloadSize);
//    rc = m_file.read((char *)&buffer->payload[0], buffer->payload_size);
    rc = m_file.read((char *)payload, payloadSize);
    if (rc == payloadSize) 
    {
        sdvr_send_video_output(m_videoOutHandle, buffer);
    }
    else
    {
        if (rc == 0)
        {
            if (ContinuousCheckBox->isChecked())
            {
                sdvr_release_av_buffer(buffer);
                stopVideoOutput();
                startVideoOutput();

                m_file.reset();
                if (!m_file.atEnd()) // Prevent infinite loop on empty files
                    goto read;
            }
            else
            {
                Log::info("Finished SMO YUV playback");
            }
        }
        else if (rc == (sx_uint32)-1)
        {
            Log::error(QString("SMO YUV player : %1").arg(m_file.errorString()));
        }

        sdvr_release_av_buffer(buffer);
        setState(STOPPED);
        m_sender->stop();
    }
}

void SmoYuvPlayer::onThreadFinished()
{
    stopVideoOutput();

    if (m_file.isOpen())
        m_file.close();

    setState(STOPPED);
}

sdvr_err_e SmoYuvPlayer::startVideoOutput()
{
    sdvr_err_e status = SDVR_ERR_NONE;

    if (m_videoOutHandle == INVALID_CHAN_HANDLE)
        status = sdvr_start_video_output(m_smo->board()->index(), m_smo->port(),
                                SDVR_RAWV_FORMAT_YUV_4_2_0,
                                &m_videoOutHandle);

    return status;
}

sdvr_err_e SmoYuvPlayer::stopVideoOutput()
{
    sdvr_err_e status = SDVR_ERR_NONE;
    if (m_videoOutHandle != INVALID_CHAN_HANDLE)
    {
        status = sdvr_stop_video_output(m_videoOutHandle);
        m_videoOutHandle = INVALID_CHAN_HANDLE;
    }

    return status;
}

SmoYuvPlayerThread::SmoYuvPlayerThread(SmoYuvPlayer *player)
{
    m_player = player;
}

void SmoYuvPlayerThread::run()
{
    QTimer timer;

    sdvr_err_e status = m_player->startVideoOutput();
    if (status != SDVR_ERR_NONE)
    {
        Log::error(QString("Failed to start YUV playback : %1").arg(sdvr_get_error_text(status)));
        return;
    }

    connect(&timer, SIGNAL(timeout()), m_player, SLOT(sendNextFrame()));
    timer.start(m_interval);
    exec();
}

void SmoYuvPlayerThread::stop()
{
    quit();
    if (!wait(m_interval * 3))
        terminate();
}
