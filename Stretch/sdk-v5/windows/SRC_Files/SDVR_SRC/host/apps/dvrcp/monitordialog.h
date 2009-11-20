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

#ifndef MONITORDIALOG_H
#define MONITORDIALOG_H

#include "sdvr_sdk.h"
#include "ui_monitordialog.h"

class ViewSettings;

class MonitorDialog : public QDialog, private Ui::MonitorDialog
{
    Q_OBJECT

public:
    MonitorDialog(QWidget *parent = 0);

    ViewSettings *monitorSettings() const;

    static ViewSettings *getMonitorSettings(QWidget *parent = 0);
    static QString getNewName(const QString &oldName, QWidget *parent = 0);

private slots:
    virtual void accept();
    void onDecimationComboActivated(int index);

private:
    bool isSMOSupportedResolution(sdvr_video_res_decimation_e res_decimation);

};

#endif /* MONITORDIALOG_H */
