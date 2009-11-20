/********************************************************************************
** Form generated from reading ui file 'connectdialog.ui'
**
** Created: Mon Mar 30 21:53:05 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CONNECTDIALOG_H
#define UI_CONNECTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

class Ui_ConnectDialog
{
public:
    QLabel *label_3;
    QLabel *uiLabelChan;
    QComboBox *channelCombo;
    QComboBox *boardCombo;
    QLabel *uiLabelPrompt;
    QDialogButtonBox *buttonBox;
    QPushButton *uiBtnChangeAVEnc;

    void setupUi(QDialog *ConnectDialog)
    {
    ConnectDialog->setObjectName(QString::fromUtf8("ConnectDialog"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ConnectDialog->sizePolicy().hasHeightForWidth());
    ConnectDialog->setSizePolicy(sizePolicy);
    ConnectDialog->setMinimumSize(QSize(386, 192));
    ConnectDialog->setMaximumSize(QSize(386, 192));
    label_3 = new QLabel(ConnectDialog);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(7, 50, 90, 20));
    label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLabelChan = new QLabel(ConnectDialog);
    uiLabelChan->setObjectName(QString::fromUtf8("uiLabelChan"));
    uiLabelChan->setGeometry(QRect(8, 92, 90, 20));
    uiLabelChan->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    channelCombo = new QComboBox(ConnectDialog);
    channelCombo->setObjectName(QString::fromUtf8("channelCombo"));
    channelCombo->setGeometry(QRect(100, 90, 71, 21));
    boardCombo = new QComboBox(ConnectDialog);
    boardCombo->setObjectName(QString::fromUtf8("boardCombo"));
    boardCombo->setGeometry(QRect(100, 50, 71, 21));
    uiLabelPrompt = new QLabel(ConnectDialog);
    uiLabelPrompt->setObjectName(QString::fromUtf8("uiLabelPrompt"));
    uiLabelPrompt->setGeometry(QRect(20, 20, 211, 18));
    buttonBox = new QDialogButtonBox(ConnectDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(207, 140, 161, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    uiBtnChangeAVEnc = new QPushButton(ConnectDialog);
    uiBtnChangeAVEnc->setObjectName(QString::fromUtf8("uiBtnChangeAVEnc"));
    uiBtnChangeAVEnc->setGeometry(QRect(22, 142, 151, 28));
    QWidget::setTabOrder(boardCombo, channelCombo);
    QWidget::setTabOrder(channelCombo, buttonBox);

    retranslateUi(ConnectDialog);

    QSize size(386, 192);
    size = size.expandedTo(ConnectDialog->minimumSizeHint());
    ConnectDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), ConnectDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), ConnectDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(ConnectDialog);
    } // setupUi

    void retranslateUi(QDialog *ConnectDialog)
    {
    ConnectDialog->setWindowTitle(QApplication::translate("ConnectDialog", "Port Connection", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("ConnectDialog", "Board:", 0, QApplication::UnicodeUTF8));
    uiLabelChan->setText(QApplication::translate("ConnectDialog", "Channel Port:", 0, QApplication::UnicodeUTF8));
    uiLabelPrompt->setText(QApplication::translate("ConnectDialog", "Select the connection port:", 0, QApplication::UnicodeUTF8));
    uiBtnChangeAVEnc->setText(QApplication::translate("ConnectDialog", "Change A/V Encoders...", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(ConnectDialog);
    } // retranslateUi

};

namespace Ui {
    class ConnectDialog: public Ui_ConnectDialog {};
} // namespace Ui

#endif // UI_CONNECTDIALOG_H
