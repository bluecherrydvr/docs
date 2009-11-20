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

#ifndef SMOYUVPLAYER_H
#define SMOYUVPLAYER_H

#include "ui_smoyuvplayer.h"
#include <sdvr_sdk.h>

#include <QFile>

class SpotMonitor;
class SmoYuvPlayerThread;

class SmoYuvPlayer : public QWidget, private Ui::SmoYuvPlayer
{
    Q_OBJECT

public:
    enum State { STOPPED, PAUSED, PLAYING };

    SmoYuvPlayer(SpotMonitor *smo, QWidget *parent = 0);
    virtual ~SmoYuvPlayer();

    const SpotMonitor *spotMonitor() const { return m_smo; }

    void setState(State state);
    State state() { return m_state; }

    sdvr_err_e startVideoOutput();
    sdvr_err_e stopVideoOutput();

private slots:
    void onBrowseButtonClicked();
    void onPlayButtonClicked();
    void onPauseButtonClicked();
    void onStopButtonClicked();

    void sendNextFrame();
    void onThreadFinished();

private:
    SpotMonitor *m_smo;
    SmoYuvPlayerThread *m_sender;
    QFile m_file;
    State m_state;
    sdvr_chan_handle_t m_videoOutHandle;
};

#endif /* SMOYUVPLAYER_H */
