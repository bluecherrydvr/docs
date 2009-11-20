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

#ifndef SMODIALOG_H
#define SMODIALOG_H

#include "ui_smodialog.h"
#include "spotmonitor.h"

class DVRBoard;

class SmoDialog : public QDialog, private Ui::SmoDialog
{
    Q_OBJECT

public:
    SmoDialog(SpotMonitor *smo, QWidget *parent = 0);
    bool isOsdModified(int index) const { return m_osdModified[index]; }

private slots:
    void onOsdSelectionChanged(int);
    void onOsdTransValueChanged(int);
    void onOsdLocationComboActivated(int);
    virtual void accept();

private:
    void selectOsd(int index);
    void saveInput();

    SpotMonitor *m_smo;

    int  m_curOsd;
    bool m_osdModified[SDVR_MAX_OSD];
    bool m_osdEnabled[SDVR_MAX_OSD];
    sdvr_osd_config_ex_t m_osdConfig[SDVR_MAX_OSD];
};

#endif
