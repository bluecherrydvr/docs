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

#ifndef ALARMHANDLER_H
#define ALARMHANDLER_H

#include "sdvr_sdk.h"
#include <QObject>
#include <QList>
#include <QMutex>

class DVRSystem;

class AlarmHandler : public QObject
{
    Q_OBJECT

    struct AlarmMessage
    {
        sdvr_chan_handle_t handle;
        sdvr_video_alarm_e alarm_type;
        sx_uint32 data;
    };

public:
    AlarmHandler(DVRSystem *system);

    void registerMessage(sdvr_chan_handle_t handle,
            sdvr_video_alarm_e alarm_type, sx_uint32 data);

    void processMessages();

public slots:
    void reset();

private:
    DVRSystem *m_system;

    QList<AlarmMessage> m_messageList;
    QMutex m_messageListMutex;
};

#endif /* ALARMHANDLER_H */
