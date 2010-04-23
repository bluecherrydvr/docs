/********************************************************************************
** Form generated from reading ui file 'ioctldialog.ui'
**
** Created: Mon Mar 30 21:53:04 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_IOCTLDIALOG_H
#define UI_IOCTLDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

class Ui_IOCTLDialog
{
public:
    QPushButton *uiBtnRead;
    QLineEdit *uiLEditDeviceID;
    QLineEdit *uiLEditRegister;
    QLineEdit *uiLEditValue;
    QPushButton *uiBtnSend;
    QPushButton *uiBtnClose;
    QLabel *uilabelCamera;
    QComboBox *uiComboCamera;
    QLabel *uilabelRegNum;
    QLabel *uilabelValue;
    QLabel *uilabelDeviceID;

    void setupUi(QDialog *IOCTLDialog)
    {
    IOCTLDialog->setObjectName(QString::fromUtf8("IOCTLDialog"));
    uiBtnRead = new QPushButton(IOCTLDialog);
    uiBtnRead->setObjectName(QString::fromUtf8("uiBtnRead"));
    uiBtnRead->setGeometry(QRect(310, 50, 75, 25));
    uiBtnRead->setAutoDefault(false);
    uiLEditDeviceID = new QLineEdit(IOCTLDialog);
    uiLEditDeviceID->setObjectName(QString::fromUtf8("uiLEditDeviceID"));
    uiLEditDeviceID->setGeometry(QRect(120, 57, 113, 23));
    uiLEditRegister = new QLineEdit(IOCTLDialog);
    uiLEditRegister->setObjectName(QString::fromUtf8("uiLEditRegister"));
    uiLEditRegister->setGeometry(QRect(120, 85, 113, 23));
    uiLEditValue = new QLineEdit(IOCTLDialog);
    uiLEditValue->setObjectName(QString::fromUtf8("uiLEditValue"));
    uiLEditValue->setGeometry(QRect(120, 113, 113, 23));
    uiBtnSend = new QPushButton(IOCTLDialog);
    uiBtnSend->setObjectName(QString::fromUtf8("uiBtnSend"));
    uiBtnSend->setGeometry(QRect(310, 20, 75, 25));
    uiBtnSend->setAutoDefault(false);
    uiBtnClose = new QPushButton(IOCTLDialog);
    uiBtnClose->setObjectName(QString::fromUtf8("uiBtnClose"));
    uiBtnClose->setGeometry(QRect(310, 110, 75, 25));
    uiBtnClose->setDefault(true);
    uilabelCamera = new QLabel(IOCTLDialog);
    uilabelCamera->setObjectName(QString::fromUtf8("uilabelCamera"));
    uilabelCamera->setGeometry(QRect(30, 20, 88, 17));
    uilabelCamera->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiComboCamera = new QComboBox(IOCTLDialog);
    uiComboCamera->setObjectName(QString::fromUtf8("uiComboCamera"));
    uiComboCamera->setGeometry(QRect(120, 18, 131, 22));
    uilabelRegNum = new QLabel(IOCTLDialog);
    uilabelRegNum->setObjectName(QString::fromUtf8("uilabelRegNum"));
    uilabelRegNum->setGeometry(QRect(7, 90, 112, 20));
    uilabelRegNum->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uilabelValue = new QLabel(IOCTLDialog);
    uilabelValue->setObjectName(QString::fromUtf8("uilabelValue"));
    uilabelValue->setGeometry(QRect(7, 118, 112, 17));
    uilabelValue->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uilabelDeviceID = new QLabel(IOCTLDialog);
    uilabelDeviceID->setObjectName(QString::fromUtf8("uilabelDeviceID"));
    uilabelDeviceID->setGeometry(QRect(7, 60, 112, 17));
    uilabelDeviceID->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    QWidget::setTabOrder(uiComboCamera, uiLEditDeviceID);
    QWidget::setTabOrder(uiLEditDeviceID, uiLEditRegister);
    QWidget::setTabOrder(uiLEditRegister, uiLEditValue);
    QWidget::setTabOrder(uiLEditValue, uiBtnSend);
    QWidget::setTabOrder(uiBtnSend, uiBtnRead);
    QWidget::setTabOrder(uiBtnRead, uiBtnClose);

    retranslateUi(IOCTLDialog);

    QSize size(400, 144);
    size = size.expandedTo(IOCTLDialog->minimumSizeHint());
    IOCTLDialog->resize(size);

    QObject::connect(uiBtnClose, SIGNAL(clicked()), IOCTLDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(IOCTLDialog);
    } // setupUi

    void retranslateUi(QDialog *IOCTLDialog)
    {
    IOCTLDialog->setWindowTitle(QApplication::translate("IOCTLDialog", "Device I/O Control", 0, QApplication::UnicodeUTF8));
    uiBtnRead->setText(QApplication::translate("IOCTLDialog", "Read", 0, QApplication::UnicodeUTF8));
    uiBtnSend->setText(QApplication::translate("IOCTLDialog", "Send", 0, QApplication::UnicodeUTF8));
    uiBtnClose->setText(QApplication::translate("IOCTLDialog", "Close", 0, QApplication::UnicodeUTF8));
    uilabelCamera->setText(QApplication::translate("IOCTLDialog", "Camera:", 0, QApplication::UnicodeUTF8));
    uilabelRegNum->setText(QApplication::translate("IOCTLDialog", "Register #(hex):", 0, QApplication::UnicodeUTF8));
    uilabelValue->setText(QApplication::translate("IOCTLDialog", "Value(hex):", 0, QApplication::UnicodeUTF8));
    uilabelDeviceID->setText(QApplication::translate("IOCTLDialog", "Device ID(hex):", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(IOCTLDialog);
    } // retranslateUi

};

namespace Ui {
    class IOCTLDialog: public Ui_IOCTLDialog {};
} // namespace Ui

#endif // UI_IOCTLDIALOG_H
