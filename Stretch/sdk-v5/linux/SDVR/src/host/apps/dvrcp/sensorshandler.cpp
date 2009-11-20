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
#include "sdvr.h"
#include "sensorshandler.h"
#include "dvrsystem.h"
#include "camera.h"
#include "log.h"
#include "dvrboard.h"


SensorsHandler::SensorsHandler(DVRSystem *system)
{
    m_system = system;
}

void SensorsHandler::registerMessage(sx_uint32 board_index, sx_uint32 sensor_map)
{
    SensorsMessage msg;
    msg.board_index = board_index;
    msg.sensor_map = sensor_map;

    m_messageListMutex.lock();
    m_messageList.append(msg);
    m_messageListMutex.unlock();
}

void SensorsHandler::processMessages()
{
    QList<SensorsMessage> messages;
    QString sensorMsg;

    m_messageListMutex.lock();
    if (!m_messageList.empty())
    {
        messages = m_messageList;
        m_messageList.clear();
    }
    m_messageListMutex.unlock();

    QList<SensorsMessage>::iterator i;
    for (i = messages.begin(); i != messages.end(); ++i)
    {
        sensorMsg = formatSensorMsg((*i).board_index, (*i).sensor_map);
        Log::sensors(sensorMsg);
    } // for (i = messages.begin(); i != messages.end(); ++i)
}

QString SensorsHandler::formatSensorMsg(uint board_index, uint sensorMap)
{
    QString msg = QString(" Board[%1]: ").arg(board_index);
    uint sensorMapIndex = 1;
    int sensorsCount = m_system->board(board_index)->sensorsCount();

    for (int i = 0; i <sensorsCount ; i++)
    {
        // Seperate each bank of 4 sensors
        if (!(i % 4) && (i > 0))
            msg += " | ";

        if (sensorMap & (sensorMapIndex << i))
            msg += "X";
        else 
            msg += "-";
    }

    return msg;
}


////////////////////////// Singals Hanlder Class //////////
SignalsHandler::SignalsHandler(DVRSystem *system)
{
    m_system = system;
}

void SignalsHandler::registerMessage(sx_uint32 board_index, 
                                     sdvr_signal_info_t *signal_info)
{
    SignalsMessage msg;
    msg.board_index = board_index;
    memcpy(&msg.signal_info, signal_info,sizeof(sdvr_signal_info_t));

    m_listMutex.lock();
    m_signalsList.append(msg);
    m_listMutex.unlock();
}

void SignalsHandler::processMessages()
{
    QList<SignalsMessage> messages;
    QString qstrMsg;

    m_listMutex.lock();
    if (!m_signalsList.empty())
    {
        messages = m_signalsList;
        m_signalsList.clear();
    }
    m_listMutex.unlock();

    QList<SignalsMessage>::iterator i;
    for (i = messages.begin(); i != messages.end(); ++i)
    {
        switch ((*i).signal_info.sig_type)
        {
        case SDVR_SIGNAL_RUNTIME_ERROR:
            qstrMsg = QString("Run time error signal recieved from board [%1] - channel [%2] - type [%3] - Data = %4  - Extra = %5.").
                arg((*i).board_index+1).
                arg((*i).signal_info.chan_num + 1).
                arg(sdvr::ChannelTypeToString(((*i).signal_info.chan_type))).
                arg(sdvr_get_error_text((sdvr_err_e)(*i).signal_info.data)).
                arg((*i).signal_info.extra_data);

            break;
        case SDVR_SIGNAL_FATAL_ERROR:
            qstrMsg = QString("Fatal error signal recieved from board [%1] - channel [%2] - type [%3] - Data = %4  - Extra = %5. You must exit the application.").
                arg((*i).board_index+1).
                arg((*i).signal_info.chan_num + 1).
                arg(sdvr::ChannelTypeToString(((*i).signal_info.chan_type))).
                arg(sdvr_get_error_text((sdvr_err_e)(*i).signal_info.data)).
                arg((*i).signal_info.extra_data);
            break;

        default:
            qstrMsg = QString("Uknown signal recieved from board [%1] - channel [%2] - type [%3] - Data = %4  - Extra = %5.").
                arg((*i).board_index+1).
                arg((*i).signal_info.chan_num+1).
                arg(sdvr::ChannelTypeToString(((*i).signal_info.chan_type))).
                arg(sdvr_get_error_text((sdvr_err_e)(*i).signal_info.data)).
                arg((*i).signal_info.extra_data);
            break;
        }
        Log::error(qstrMsg);
    } // for (i = messages.begin(); i != messages.end(); ++i)

}
