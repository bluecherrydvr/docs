/********************************************************************************
** Form generated from reading ui file 'aboutdialog.ui'
**
** Created: Mon Mar 30 21:53:05 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <Qt3Support/Q3MimeSourceFactory>

class Ui_AboutDialog
{
public:
    QLabel *label_1;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *sdk_version;
    QLabel *uiLabelLogFilePath;
    QLineEdit *uiLEditLogFilePath;
    QPushButton *okBtn;
    QLabel *label;
    QLabel *dvrcp_version;
    QGroupBox *groupBox;
    QLabel *label_10;
    QLabel *dvr_chip_rev;
    QLabel *label_9;
    QLabel *dvr_board_type_rev;
    QLabel *label_7;
    QLabel *bsp_version;
    QLabel *label_6;
    QLabel *boot_loader_version;
    QLabel *label_8;
    QLabel *fw_build_date;
    QLabel *label_5;
    QLabel *label_4;
    QLabel *fw_version;
    QLabel *driver_version;

    void setupUi(QDialog *AboutDialog)
    {
    AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(AboutDialog->sizePolicy().hasHeightForWidth());
    AboutDialog->setSizePolicy(sizePolicy);
    AboutDialog->setMinimumSize(QSize(448, 350));
    AboutDialog->setMaximumSize(QSize(448, 350));
    label_1 = new QLabel(AboutDialog);
    label_1->setObjectName(QString::fromUtf8("label_1"));
    label_1->setGeometry(QRect(190, 12, 231, 29));
    QFont font;
    font.setPointSize(14);
    label_1->setFont(font);
    label_1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_2 = new QLabel(AboutDialog);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setGeometry(QRect(190, 47, 141, 20));
    label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_3 = new QLabel(AboutDialog);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(240, 67, 91, 20));
    label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    sdk_version = new QLabel(AboutDialog);
    sdk_version->setObjectName(QString::fromUtf8("sdk_version"));
    sdk_version->setGeometry(QRect(338, 67, 100, 20));
    sdk_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    uiLabelLogFilePath = new QLabel(AboutDialog);
    uiLabelLogFilePath->setObjectName(QString::fromUtf8("uiLabelLogFilePath"));
    uiLabelLogFilePath->setGeometry(QRect(10, 265, 91, 20));
    uiLabelLogFilePath->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditLogFilePath = new QLineEdit(AboutDialog);
    uiLEditLogFilePath->setObjectName(QString::fromUtf8("uiLEditLogFilePath"));
    uiLEditLogFilePath->setGeometry(QRect(110, 263, 321, 23));
    uiLEditLogFilePath->setReadOnly(true);
    okBtn = new QPushButton(AboutDialog);
    okBtn->setObjectName(QString::fromUtf8("okBtn"));
    okBtn->setGeometry(QRect(180, 318, 80, 25));
    label = new QLabel(AboutDialog);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(120, 294, 301, 16));
    label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    dvrcp_version = new QLabel(AboutDialog);
    dvrcp_version->setObjectName(QString::fromUtf8("dvrcp_version"));
    dvrcp_version->setGeometry(QRect(338, 47, 100, 20));
    dvrcp_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    groupBox = new QGroupBox(AboutDialog);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    groupBox->setGeometry(QRect(161, 83, 271, 171));
    label_10 = new QLabel(groupBox);
    label_10->setObjectName(QString::fromUtf8("label_10"));
    label_10->setGeometry(QRect(10, 142, 121, 20));
    label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    dvr_chip_rev = new QLabel(groupBox);
    dvr_chip_rev->setObjectName(QString::fromUtf8("dvr_chip_rev"));
    dvr_chip_rev->setGeometry(QRect(140, 142, 100, 20));
    dvr_chip_rev->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    label_9 = new QLabel(groupBox);
    label_9->setObjectName(QString::fromUtf8("label_9"));
    label_9->setGeometry(QRect(10, 121, 121, 20));
    label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    dvr_board_type_rev = new QLabel(groupBox);
    dvr_board_type_rev->setObjectName(QString::fromUtf8("dvr_board_type_rev"));
    dvr_board_type_rev->setGeometry(QRect(140, 121, 121, 20));
    dvr_board_type_rev->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    label_7 = new QLabel(groupBox);
    label_7->setObjectName(QString::fromUtf8("label_7"));
    label_7->setGeometry(QRect(10, 101, 121, 20));
    label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    bsp_version = new QLabel(groupBox);
    bsp_version->setObjectName(QString::fromUtf8("bsp_version"));
    bsp_version->setGeometry(QRect(140, 101, 100, 20));
    bsp_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    label_6 = new QLabel(groupBox);
    label_6->setObjectName(QString::fromUtf8("label_6"));
    label_6->setGeometry(QRect(10, 81, 121, 20));
    label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    boot_loader_version = new QLabel(groupBox);
    boot_loader_version->setObjectName(QString::fromUtf8("boot_loader_version"));
    boot_loader_version->setGeometry(QRect(140, 81, 100, 20));
    boot_loader_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    label_8 = new QLabel(groupBox);
    label_8->setObjectName(QString::fromUtf8("label_8"));
    label_8->setGeometry(QRect(10, 60, 121, 20));
    label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    fw_build_date = new QLabel(groupBox);
    fw_build_date->setObjectName(QString::fromUtf8("fw_build_date"));
    fw_build_date->setGeometry(QRect(140, 60, 100, 17));
    fw_build_date->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    label_5 = new QLabel(groupBox);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setGeometry(QRect(10, 41, 121, 20));
    label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_4 = new QLabel(groupBox);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setGeometry(QRect(9, 21, 121, 20));
    label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    fw_version = new QLabel(groupBox);
    fw_version->setObjectName(QString::fromUtf8("fw_version"));
    fw_version->setGeometry(QRect(140, 41, 100, 20));
    fw_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    driver_version = new QLabel(groupBox);
    driver_version->setObjectName(QString::fromUtf8("driver_version"));
    driver_version->setGeometry(QRect(140, 21, 100, 20));
    driver_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

    retranslateUi(AboutDialog);

    QSize size(448, 350);
    size = size.expandedTo(AboutDialog->minimumSizeHint());
    AboutDialog->resize(size);

    QObject::connect(okBtn, SIGNAL(clicked()), AboutDialog, SLOT(accept()));

    QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
    AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "Stretch IDE", 0, QApplication::UnicodeUTF8));
    label_1->setText(QApplication::translate("AboutDialog", "Digital Video Recorder", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("AboutDialog", "Control Panel Version:", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("AboutDialog", "SDK Version:", 0, QApplication::UnicodeUTF8));
    sdk_version->setText(QString());
    uiLabelLogFilePath->setText(QApplication::translate("AboutDialog", "Log Files Path:", 0, QApplication::UnicodeUTF8));
    okBtn->setText(QApplication::translate("AboutDialog", "&OK", 0, QApplication::UnicodeUTF8));
    okBtn->setShortcut(QApplication::translate("AboutDialog", "Alt+O", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("AboutDialog", "Copyright (c) 2008 Stretch, Inc. All Rights Reserved.", 0, QApplication::UnicodeUTF8));
    dvrcp_version->setText(QString());
    groupBox->setTitle(QApplication::translate("AboutDialog", "Board 1 Information:", 0, QApplication::UnicodeUTF8));
    label_10->setText(QApplication::translate("AboutDialog", "Chip Revision:", 0, QApplication::UnicodeUTF8));
    dvr_chip_rev->setText(QString());
    label_9->setText(QApplication::translate("AboutDialog", "DVR Board Type-Rev:", 0, QApplication::UnicodeUTF8));
    dvr_board_type_rev->setText(QString());
    label_7->setText(QApplication::translate("AboutDialog", "BSP Version:", 0, QApplication::UnicodeUTF8));
    bsp_version->setText(QString());
    label_6->setText(QApplication::translate("AboutDialog", "Boot Loader Version:", 0, QApplication::UnicodeUTF8));
    boot_loader_version->setText(QString());
    label_8->setText(QApplication::translate("AboutDialog", "Firmware Build Date:", 0, QApplication::UnicodeUTF8));
    fw_build_date->setText(QString());
    label_5->setText(QApplication::translate("AboutDialog", "Firmware Version:", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("AboutDialog", "PCIe Driver Version:", 0, QApplication::UnicodeUTF8));
    fw_version->setText(QString());
    driver_version->setText(QString());
    Q_UNUSED(AboutDialog);
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

#endif // UI_ABOUTDIALOG_H
