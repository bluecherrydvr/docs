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

#include <QDateTime>
#include "dvrboard.h"
#include "mainwindow.h"
#include "log.h"
extern MainWindow *g_MainWindow;

DVRBoard::DVRBoard(int boardIndex)
{
    m_boardIndex = boardIndex;

    memset(&m_board_attrib, 0, sizeof(m_board_attrib));
    memset(&m_pci_attrib, 0, sizeof(m_pci_attrib));
    memset(&m_fwVersion, 0, sizeof(m_fwVersion));

    m_videoInCount = 0;
    m_audioInCount = 0;
    m_sensorsCount = 0;
    m_relaysCount = 0;
    m_decodersCount = 0;
    m_encodersCount = 0;
    m_smoCount = 0;
    // by default all the sensors are disabled and are edge triggred
    m_sensor_enable_map = 0x00000000;
    m_edge_triggered_map = 0x0000FFFF;

    m_hasSmo = false;
    m_bFirmwareTimeout = false;

    m_timeLastUpdated = 0;
}

DVRBoard::~DVRBoard()
{
//    qDeleteAll(m_smoList);
}

void DVRBoard::setAttrib(sdvr_board_attrib_t *attrib)
{
    memcpy(&m_board_attrib, attrib, sizeof(m_board_attrib));
}

void DVRBoard::setPCIAttrib(sdvr_pci_attrib_t *attrib)
{
    memcpy(&m_pci_attrib, attrib, sizeof(m_pci_attrib));
}
void DVRBoard::setConfig(sdvr_board_config_t *config)
{
    m_videoInCount = config->num_cameras_supported;
    m_decodersCount = config->num_decoders_supported;
    m_audioInCount = config->num_microphones_supported;
    m_sensorsCount = config->num_sensors;
    m_relaysCount  = config->num_relays;
    m_hasSmo       = config->has_smo;
    m_smoCount     = config->num_smos;
    m_encodersCount = config->num_encoders_supported;
}

void DVRBoard::setSensorConfig(uint sensor_enable_map, uint edge_triggered_map)
{
    m_sensor_enable_map = sensor_enable_map;
    m_edge_triggered_map = edge_triggered_map;
}

void DVRBoard::setFWVersion(sdvr_firmware_ver_t *fwVer)
{
    if (fwVer)
        memcpy(&m_fwVersion, fwVer, sizeof(m_fwVersion));
}

void DVRBoard::setFirmwareTimeout() 
{ 
    m_bFirmwareTimeout = true;
}

sdvr_err_e DVRBoard::updateDateTime()
{
    sdvr_err_e err;
    QDateTime qtCurTime = QDateTime::currentDateTime();
    // NOTE: we must use UTC as oppose to localtime otherwise
    //       the time is off after day light saving time.
    qtCurTime.setTimeSpec(Qt::UTC);
    m_timeLastUpdated = qtCurTime.toTime_t();

#ifdef disable
    // If the board time drifts away more than two seconds
    // from the PC time, alert the user.
    time_t boardTime;

    if (sdvr_get_date_time(m_boardIndex,  &boardTime) == SDVR_ERR_NONE)
    {
        QDateTime qtBoardTime;
        QString qstrLog;

        qtBoardTime.setTime_t(boardTime);
        qtBoardTime.setTimeSpec(Qt::LocalTime);
        if ((boardTime - m_timeLastUpdated) > 2)
        {
            qstrLog = QString("Board ID %1: Setting time. Board Time %2  New Time %3").
                arg(m_boardIndex + 1).arg(qtBoardTime.toString()).arg(qtCurTime.toString());
            Log::info(qstrLog);
        }
    }
#endif        
    err = sdvr_set_date_time(m_boardIndex, m_timeLastUpdated);

    return err;
}



