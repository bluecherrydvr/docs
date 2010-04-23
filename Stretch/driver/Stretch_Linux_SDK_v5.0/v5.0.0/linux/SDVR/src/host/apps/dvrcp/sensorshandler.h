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

#ifndef SENSORSHANDLER_H
#define SENSORSHANDLER_H

#include "sdvr_sdk.h"
#include <QObject>
#include <QList>
#include <QMutex>

class DVRSystem;

class SensorsHandler : public QObject
{
    Q_OBJECT

    struct SensorsMessage
    {
        sx_uint32   board_index;
        sx_uint32   sensor_map;
    };

public:
    SensorsHandler(DVRSystem *system);

    void registerMessage(sx_uint32 board_index, sx_uint32 sensor_map);

    void processMessages();

private:
    QString formatSensorMsg(uint board_index, uint sensorMap);
    
private:
    DVRSystem *m_system;

    QList<SensorsMessage> m_messageList;
    QMutex m_messageListMutex;
};

////////////////////////// Singals Hanlder Class //////////
class SignalsHandler : public QObject
{
    Q_OBJECT

    struct SignalsMessage
    {
        sx_uint32   board_index;
        sdvr_signal_info_t signal_info;
    };

public:
    SignalsHandler(DVRSystem *system);

    void registerMessage(sx_uint32 board_index, sdvr_signal_info_t *signal_info);

    void processMessages();

private:
    
private:
    DVRSystem *m_system;

    QList<SignalsMessage> m_signalsList;
    QMutex m_listMutex;
};
#endif /* SENSORSHANDLER_H */
