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

#ifndef LOG_H
#define LOG_H

#include <QWidget>
#include <QEvent>
#include "sdvr_sdk.h"
#include "mainwindow.h"

class QTextEdit;
class QLabel;
class QPushButton;

class Log : public QWidget
{
    Q_OBJECT

    ~Log();

public:
    static Log *Instance();
    static void info(const QString &msg);
    static void error(const QString &msg);
    static void alarm(const QString &msg);
    static void sensors(const QString &msg);
    static void chk_err(QString qstrMsg, sdvr_err_e rc, bool bUpdateStatusBar, int boardIndex);
    static void clearText();
    static void toggleInfoDisplay();

signals:
    void alarmReset();

protected:
    Log(QWidget *parent = 0);

    virtual void customEvent(QEvent *event);

protected slots:
    void resetAlarm();

private:
    QTextEdit *m_logView;
    QLabel *m_alarmIndicator;
    QPushButton *m_resetAlarmButton;
    bool m_bDisplayInfo;

    static Log *m_instance;
    static QString timePrefix();
};

#endif
