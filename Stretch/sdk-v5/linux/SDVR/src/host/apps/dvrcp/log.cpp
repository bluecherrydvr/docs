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

#include "log.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QApplication>

class LogEvent : public QEvent
{
public:
    enum MessageType { INFO, ERR, ALARM, SENSOR, SDVR_ERR };

    LogEvent(MessageType msgType, const QString &msg);
    LogEvent(MessageType msgType, const QString &msg, sdvr_err_e sdvrErr,
        int boardIndex, bool updateStatusBar = false);

    MessageType messageType() { return m_msgType; }
    const QString &message() { return m_msg; }
    sdvr_err_e sdvrErr() { return m_sdvrErr; }
    int boardIndex()  { return m_boardIndex; }
    bool updateStatusBar() { return m_updateStatusBar; }

private:
    MessageType m_msgType;
    QString m_msg;
    sdvr_err_e m_sdvrErr;
    int m_boardIndex;
    bool m_updateStatusBar;
};


Log *Log::m_instance = NULL;  // Initialize pointer

Log *Log::Instance()
{
    if (m_instance == NULL)
        m_instance = new Log();

    return m_instance;
}

Log::~Log()
{
    m_instance = NULL;
}

Log::Log(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_bDisplayInfo = true;

    QToolBar *toolBar = new QToolBar();
//    toolBar->layout()->setAlignment(Qt::AlignRight);

    m_alarmIndicator = new QLabel();
    m_alarmIndicator->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
    m_alarmIndicator->setPixmap(QPixmap(":/images/alarm.png"));
    m_alarmIndicator->setEnabled(false);
    toolBar->addWidget(m_alarmIndicator);
    toolBar->addSeparator();

    QPushButton *m_resetAlarmButton = new QPushButton("Reset Alarm");
    m_resetAlarmButton->setFixedHeight(24);
    connect(m_resetAlarmButton, SIGNAL(clicked()), SLOT(resetAlarm()));
    toolBar->addWidget(m_resetAlarmButton);

    layout->addWidget(toolBar, 0);

    m_logView = new QTextEdit();

    m_logView->document()->setMaximumBlockCount(100);
    m_logView->setLineWrapMode(QTextEdit::NoWrap);
    m_logView->setFontFamily("Courier");
    m_logView->setReadOnly(true);

    layout->addWidget(m_logView, 1);

//    QAction *resetAction = new QAction(QIcon(":/images/alarm.bmp"), tr("&Reset"), this);
//    QAction *resetAction = new QAction(tr("&Reset"), this);
//    resetAction->setStatusTip(tr("Reset Alarm"));
//    connect(resetAction, SIGNAL(triggered()), this, SLOT(resetAlarm()));
//    toolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
////    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//    toolBar->addAction(resetAction);

//    QWidget *w = toolBar->widgetForAction(resetAction);
//    toolBar->setIconSize(QSize(64, 24));
//    w->setIconSize(QSize(64, 24));
}

void Log::customEvent(QEvent *event)
{
    if (event->type() != QEvent::User)
    {
        event->ignore();
        return;
    }

    LogEvent *e = static_cast<LogEvent *>(event);

    switch (e->messageType())
    {
    case LogEvent::INFO:
        m_logView->setTextColor(QColor(0, 0, 0));
        m_logView->append(timePrefix() + e->message());
        break;

    case LogEvent::ERR:
        m_logView->setTextColor(QColor(192, 0, 0));
        m_logView->append(timePrefix()+ e->message());
        break;

    case LogEvent::ALARM:
        m_logView->setTextColor(QColor(192, 0, 0));
        m_logView->append(timePrefix()+ "ALARM: " + e->message());
        m_alarmIndicator->setEnabled(true);
        QApplication::beep();
        break;

    case LogEvent::SENSOR:
        m_logView->setTextColor(QColor(192, 0, 0));
        m_logView->append(timePrefix()+ "SENSORS: " + e->message());
        break;

    case LogEvent::SDVR_ERR:
        {
            QString msg;

            if (e->sdvrErr() == SDVR_ERR_NONE)
                msg = "Ready";
            else if (e->sdvrErr() == SDVR_DRV_ERR_MSG_RECV)
                msg = QString("Firmware timeout on board %1. You must exit.").arg(e->boardIndex());
            else
            {
                msg = QString("%1 Error [%2]").arg(e->message()).arg(sdvr_get_error_text(e->sdvrErr()));
                m_logView->setTextColor(QColor(192, 0, 0));
                m_logView->append(timePrefix()+ msg);
            }

            if (e->updateStatusBar())
                g_MainWindow->setStatusBar(msg);
        }
    }

    if (e->messageType() != LogEvent::INFO)
    {
        QWidget *p = static_cast<QWidget *>(parent());
        if (p->isHidden())
            p->show();
    }

    event->accept();
}

void Log::info(const QString &msg)
{
    if (Instance()->m_bDisplayInfo)
    {
        QCoreApplication::postEvent(Instance(),
            new LogEvent(LogEvent::INFO, msg));
    }
}

void Log::error(const QString &msg)
{
    QCoreApplication::postEvent(Instance(),
        new LogEvent(LogEvent::ERR, msg));
}

void Log::alarm(const QString &msg)
{
    QCoreApplication::postEvent(Instance(),
        new LogEvent(LogEvent::ALARM, msg));
}

void Log::sensors(const QString &msg)
{
    QCoreApplication::postEvent(Instance(),
        new LogEvent(LogEvent::SENSOR, msg));
}

void Log::chk_err(QString msg, sdvr_err_e sdvrErr, bool updateStatusBar, int boardIndex)
{
    // Logging should not be combined with other operations!
    // TODO: Remove when possible.
    if (sdvrErr == SDVR_DRV_ERR_MSG_RECV && boardIndex >= 0)
        g_MainWindow->setFirmwareTimeout(boardIndex);

    QCoreApplication::postEvent(Instance(),
        new LogEvent(LogEvent::SDVR_ERR, msg, sdvrErr, boardIndex, updateStatusBar));
}

void Log::clearText()
{
    Instance()->m_logView->clear();
}

void Log::toggleInfoDisplay() 
{
    Log *log = Instance();
    log->m_bDisplayInfo = !log->m_bDisplayInfo;
}


QString Log::timePrefix()
{
    return QDateTime::currentDateTime().toString("[dd/MM/yy hh:mm:ss]  ");
}

void Log::resetAlarm()
{
    m_alarmIndicator->setEnabled(false);
    info("Alarm reset");
    emit alarmReset();
}

LogEvent::LogEvent(MessageType msgType, const QString &msg)
    : QEvent(QEvent::User)
{
    m_msgType = msgType;
    m_msg = msg;
    m_sdvrErr = SDVR_ERR_NONE;
    m_boardIndex = 0;
    m_updateStatusBar = false;
}

LogEvent::LogEvent(MessageType msgType, const QString &msg, sdvr_err_e sdvrErr,
        int boardIndex, bool updateStatusBar)
    : QEvent(QEvent::User)
{
    m_msgType = msgType;
    m_msg = msg;
    m_sdvrErr = sdvrErr;
    m_boardIndex = boardIndex;
    m_updateStatusBar = updateStatusBar;
}
