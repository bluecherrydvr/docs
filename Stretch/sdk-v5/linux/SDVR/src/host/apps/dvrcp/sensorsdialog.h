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

#ifndef SENSORSDIALOG_H
#define SENSORSDIALOG_H

#include "ui_sensorsdialog.h"
#include "sdvr_sdk.h"

class DVRBoard;

class SensorsCnfgDialog : public QDialog, private Ui::SensorsCnfgDialog
{
    Q_OBJECT

public:
    SensorsCnfgDialog(uint board_index, QWidget *parent = 0);

private slots:
    void onEnableAllClicked();
    void onDisableAllClicked();
    void onEdgeTriggerAllClicked();
    void onLevelSensitiveAllClicked();
    virtual void accept();


private:
    void initAllControls();

private:
    uint m_boardIndex;
    bool m_bEnableFirstSensorBoard;
    bool m_bEnableSecondSensorBoard;
    DVRBoard *m_board;


};

#endif
