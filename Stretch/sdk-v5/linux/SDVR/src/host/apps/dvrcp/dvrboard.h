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

#ifndef DVRBOARD_H
#define DVRBOARD_H

#include <QObject>
#include <QVector>
#include "sdvr_sdk.h"

class Camera;

class DVRBoard : public QObject
{
    Q_OBJECT

public:
    DVRBoard(int boardIndex);
    ~DVRBoard();

    void setAttrib(sdvr_board_attrib_t *attrib);
    sdvr_board_attrib_t *getAttrib() {return &m_board_attrib;}
    sdvr_pci_attrib_t *getPCIAttrib() {return &m_pci_attrib;}
    void setPCIAttrib(sdvr_pci_attrib_t *attrib);

    void setSerialNo(char *serial_no) {m_serial_no = QString(serial_no);}
    QString getSerialNo() {return m_serial_no;}

    void setConfig(sdvr_board_config_t *config);

    int index() const { return m_boardIndex; }

    uint videoInCount() const { return m_videoInCount; }
    uint decodersCount() const {return m_decodersCount; }
    uint encodersCount() const {return m_encodersCount; }
    uint audioInCount() const { return m_audioInCount; }
    uint sensorsCount() const { return m_sensorsCount; }
    uint relaysCount() const  { return m_relaysCount; }
    uint sensorEnableMap() const { return m_sensor_enable_map; }
    uint edgeTriggeredMap() const { return m_edge_triggered_map;}
    void setSensorConfig(uint sensor_enable_map, uint edge_triggered_map);
    sx_uint32 getBoardType() const {return m_board_attrib.board_type;}
    sdvr_chip_rev_e getChipRev() const {return m_board_attrib.chip_revision;}
    bool hasSmo() const      { return m_hasSmo; }
    int smoCount() const { return m_smoCount; }
    sdvr_firmware_ver_t getFWVersion() { return m_fwVersion; }
    void setFWVersion(sdvr_firmware_ver_t *fwVer);
    bool isFirmwareTimeout() { return m_bFirmwareTimeout; }
    void setFirmwareTimeout();
    sdvr_err_e updateDateTime();
    int getTimeLastUpdated() {return m_timeLastUpdated;}

private:
    int m_boardIndex;

    sdvr_board_attrib_t m_board_attrib;
    sdvr_pci_attrib_t m_pci_attrib;
    QString m_serial_no;
    sdvr_firmware_ver_t m_fwVersion;

    uint m_videoInCount;
    uint m_decodersCount;
    uint m_encodersCount;
    uint m_audioInCount;
    uint m_sensorsCount;
    uint m_relaysCount;
    bool m_hasSmo;
    int  m_smoCount;
    uint m_sensor_enable_map;
    uint m_edge_triggered_map;
    bool m_bFirmwareTimeout;

    uint m_timeLastUpdated;             // The time stamp that was send to the DVR board
                                        // on the last call to sdvr_set_date_time

    sdvr_video_std_e m_videoStandards;
};

#endif
