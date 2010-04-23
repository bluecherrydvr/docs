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

#ifndef SPOTMONITOR_H
#define SPOTMONITOR_H

#include <QVector>
#include <QRect>
#include <QString>
#include "sdvr_sdk.h"

class DVRBoard;
class ViewSettings;

class SpotMonitor
{
public:
    SpotMonitor(DVRBoard *board, int port);

    DVRBoard *board() const { return m_board; }
    int port() const { return m_port; }
    
    sdvr_err_e getAttributes(sdvr_smo_attribute_t *attr);

    void setName(const QString &name) { m_name = name; };
    const QString &name() const { return m_name; }

    bool isOsdEnabled(int i) const { return m_osdEnabled[i]; }
    void setOsdEnabled(int i, bool enabled) { m_osdEnabled[i] = enabled; }

    sdvr_osd_config_ex_t &osdConfig(int i) { return m_osdConfig[i]; }

    bool setView(ViewSettings *vs);
    ViewSettings *view() const { return m_view; }

    bool enableGrid(int display, bool bEnable);

    void clear();

private:
    DVRBoard *m_board;
    int m_port;

    QString m_name;

    bool m_osdEnabled[SDVR_MAX_OSD];
    sdvr_osd_config_ex_t m_osdConfig[SDVR_MAX_OSD];

    bool m_haveAttributes;
    sdvr_smo_attribute_t m_attributes;

    ViewSettings *m_view;        // Active view
    QVector<QRect> m_rectVector; // Display boundaries
};

#endif
