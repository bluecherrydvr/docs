/********************************************************************************
** Form generated from reading ui file 'boarddialog.ui'
**
** Created: Mon Mar 30 21:53:05 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_BOARDDIALOG_H
#define UI_BOARDDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

class Ui_BoardDialog
{
public:
    QGroupBox *uiGrpBoxBrdVersions;
    QLabel *fw_version;
    QLabel *label_12;
    QLabel *label_14;
    QLabel *label_8;
    QLabel *label_10;
    QLabel *dvr_board_rev;
    QLabel *label_13;
    QLabel *fw_build_date;
    QLabel *boot_loader_version;
    QLabel *dvr_chip_rev;
    QLabel *label_serial_no;
    QLabel *bsp_version;
    QLabel *serial_no;
    QLabel *label_6;
    QGroupBox *uiGrpBoxBrdConfig;
    QLabel *videoInputsLabel;
    QLabel *audioInputsLabel;
    QLabel *label_9;
    QLabel *numSensorsLabel;
    QLabel *label_11;
    QLabel *numRelaysLabel;
    QLabel *label;
    QLabel *SMOAvailableLabel;
    QLabel *label_7;
    QLabel *label_5;
    QGroupBox *uiGrpBoxPciBoardInfo;
    QLabel *label_slot_num;
    QLabel *pci_slot_no;
    QLabel *label_board_type;
    QLabel *board_type;
    QLabel *label_vendor_id;
    QLabel *vendor_id;
    QLabel *label_sub_vendor_id;
    QLabel *sub_vendor_id;
    QLabel *label_device_id;
    QLabel *device_id;
    QLabel *label_sub_system_id;
    QLabel *sub_system_id;
    QPushButton *closeButton;

    void setupUi(QDialog *BoardDialog)
    {
    BoardDialog->setObjectName(QString::fromUtf8("BoardDialog"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(13));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(BoardDialog->sizePolicy().hasHeightForWidth());
    BoardDialog->setSizePolicy(sizePolicy);
    BoardDialog->setMinimumSize(QSize(535, 360));
    BoardDialog->setMaximumSize(QSize(535, 360));
    uiGrpBoxBrdVersions = new QGroupBox(BoardDialog);
    uiGrpBoxBrdVersions->setObjectName(QString::fromUtf8("uiGrpBoxBrdVersions"));
    uiGrpBoxBrdVersions->setGeometry(QRect(260, 10, 261, 178));
    fw_version = new QLabel(uiGrpBoxBrdVersions);
    fw_version->setObjectName(QString::fromUtf8("fw_version"));
    fw_version->setGeometry(QRect(140, 20, 100, 20));
    fw_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    label_12 = new QLabel(uiGrpBoxBrdVersions);
    label_12->setObjectName(QString::fromUtf8("label_12"));
    label_12->setGeometry(QRect(10, 41, 121, 20));
    label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_14 = new QLabel(uiGrpBoxBrdVersions);
    label_14->setObjectName(QString::fromUtf8("label_14"));
    label_14->setGeometry(QRect(10, 62, 121, 20));
    label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_8 = new QLabel(uiGrpBoxBrdVersions);
    label_8->setObjectName(QString::fromUtf8("label_8"));
    label_8->setGeometry(QRect(10, 83, 121, 20));
    label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_10 = new QLabel(uiGrpBoxBrdVersions);
    label_10->setObjectName(QString::fromUtf8("label_10"));
    label_10->setGeometry(QRect(10, 103, 121, 20));
    label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    dvr_board_rev = new QLabel(uiGrpBoxBrdVersions);
    dvr_board_rev->setObjectName(QString::fromUtf8("dvr_board_rev"));
    dvr_board_rev->setGeometry(QRect(140, 103, 100, 20));
    dvr_board_rev->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    label_13 = new QLabel(uiGrpBoxBrdVersions);
    label_13->setObjectName(QString::fromUtf8("label_13"));
    label_13->setGeometry(QRect(10, 123, 121, 20));
    label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    fw_build_date = new QLabel(uiGrpBoxBrdVersions);
    fw_build_date->setObjectName(QString::fromUtf8("fw_build_date"));
    fw_build_date->setGeometry(QRect(140, 43, 100, 17));
    fw_build_date->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    boot_loader_version = new QLabel(uiGrpBoxBrdVersions);
    boot_loader_version->setObjectName(QString::fromUtf8("boot_loader_version"));
    boot_loader_version->setGeometry(QRect(140, 62, 100, 20));
    boot_loader_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    dvr_chip_rev = new QLabel(uiGrpBoxBrdVersions);
    dvr_chip_rev->setObjectName(QString::fromUtf8("dvr_chip_rev"));
    dvr_chip_rev->setGeometry(QRect(140, 123, 100, 20));
    dvr_chip_rev->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    label_serial_no = new QLabel(uiGrpBoxBrdVersions);
    label_serial_no->setObjectName(QString::fromUtf8("label_serial_no"));
    label_serial_no->setGeometry(QRect(10, 143, 121, 20));
    label_serial_no->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    bsp_version = new QLabel(uiGrpBoxBrdVersions);
    bsp_version->setObjectName(QString::fromUtf8("bsp_version"));
    bsp_version->setGeometry(QRect(140, 83, 100, 20));
    bsp_version->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    serial_no = new QLabel(uiGrpBoxBrdVersions);
    serial_no->setObjectName(QString::fromUtf8("serial_no"));
    serial_no->setGeometry(QRect(140, 143, 100, 20));
    label_6 = new QLabel(uiGrpBoxBrdVersions);
    label_6->setObjectName(QString::fromUtf8("label_6"));
    label_6->setGeometry(QRect(10, 20, 121, 20));
    label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiGrpBoxBrdConfig = new QGroupBox(BoardDialog);
    uiGrpBoxBrdConfig->setObjectName(QString::fromUtf8("uiGrpBoxBrdConfig"));
    uiGrpBoxBrdConfig->setGeometry(QRect(10, 10, 241, 178));
    videoInputsLabel = new QLabel(uiGrpBoxBrdConfig);
    videoInputsLabel->setObjectName(QString::fromUtf8("videoInputsLabel"));
    videoInputsLabel->setGeometry(QRect(190, 23, 21, 16));
    audioInputsLabel = new QLabel(uiGrpBoxBrdConfig);
    audioInputsLabel->setObjectName(QString::fromUtf8("audioInputsLabel"));
    audioInputsLabel->setGeometry(QRect(190, 43, 21, 16));
    label_9 = new QLabel(uiGrpBoxBrdConfig);
    label_9->setObjectName(QString::fromUtf8("label_9"));
    label_9->setGeometry(QRect(20, 63, 141, 18));
    numSensorsLabel = new QLabel(uiGrpBoxBrdConfig);
    numSensorsLabel->setObjectName(QString::fromUtf8("numSensorsLabel"));
    numSensorsLabel->setGeometry(QRect(190, 63, 21, 16));
    label_11 = new QLabel(uiGrpBoxBrdConfig);
    label_11->setObjectName(QString::fromUtf8("label_11"));
    label_11->setGeometry(QRect(20, 83, 141, 18));
    numRelaysLabel = new QLabel(uiGrpBoxBrdConfig);
    numRelaysLabel->setObjectName(QString::fromUtf8("numRelaysLabel"));
    numRelaysLabel->setGeometry(QRect(190, 83, 21, 16));
    label = new QLabel(uiGrpBoxBrdConfig);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(20, 103, 101, 18));
    SMOAvailableLabel = new QLabel(uiGrpBoxBrdConfig);
    SMOAvailableLabel->setObjectName(QString::fromUtf8("SMOAvailableLabel"));
    SMOAvailableLabel->setGeometry(QRect(190, 103, 41, 16));
    label_7 = new QLabel(uiGrpBoxBrdConfig);
    label_7->setObjectName(QString::fromUtf8("label_7"));
    label_7->setGeometry(QRect(20, 43, 161, 18));
    label_5 = new QLabel(uiGrpBoxBrdConfig);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setGeometry(QRect(20, 23, 161, 18));
    uiGrpBoxPciBoardInfo = new QGroupBox(BoardDialog);
    uiGrpBoxPciBoardInfo->setObjectName(QString::fromUtf8("uiGrpBoxPciBoardInfo"));
    uiGrpBoxPciBoardInfo->setGeometry(QRect(10, 194, 241, 150));
    label_slot_num = new QLabel(uiGrpBoxPciBoardInfo);
    label_slot_num->setObjectName(QString::fromUtf8("label_slot_num"));
    label_slot_num->setGeometry(QRect(20, 21, 111, 20));
    label_slot_num->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    pci_slot_no = new QLabel(uiGrpBoxPciBoardInfo);
    pci_slot_no->setObjectName(QString::fromUtf8("pci_slot_no"));
    pci_slot_no->setGeometry(QRect(150, 21, 80, 20));
    label_board_type = new QLabel(uiGrpBoxPciBoardInfo);
    label_board_type->setObjectName(QString::fromUtf8("label_board_type"));
    label_board_type->setGeometry(QRect(20, 40, 111, 20));
    label_board_type->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    board_type = new QLabel(uiGrpBoxPciBoardInfo);
    board_type->setObjectName(QString::fromUtf8("board_type"));
    board_type->setGeometry(QRect(150, 40, 80, 20));
    label_vendor_id = new QLabel(uiGrpBoxPciBoardInfo);
    label_vendor_id->setObjectName(QString::fromUtf8("label_vendor_id"));
    label_vendor_id->setGeometry(QRect(20, 60, 111, 20));
    label_vendor_id->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    vendor_id = new QLabel(uiGrpBoxPciBoardInfo);
    vendor_id->setObjectName(QString::fromUtf8("vendor_id"));
    vendor_id->setGeometry(QRect(150, 60, 80, 20));
    label_sub_vendor_id = new QLabel(uiGrpBoxPciBoardInfo);
    label_sub_vendor_id->setObjectName(QString::fromUtf8("label_sub_vendor_id"));
    label_sub_vendor_id->setGeometry(QRect(20, 100, 111, 20));
    label_sub_vendor_id->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    sub_vendor_id = new QLabel(uiGrpBoxPciBoardInfo);
    sub_vendor_id->setObjectName(QString::fromUtf8("sub_vendor_id"));
    sub_vendor_id->setGeometry(QRect(150, 100, 80, 20));
    label_device_id = new QLabel(uiGrpBoxPciBoardInfo);
    label_device_id->setObjectName(QString::fromUtf8("label_device_id"));
    label_device_id->setGeometry(QRect(20, 80, 111, 20));
    label_device_id->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    device_id = new QLabel(uiGrpBoxPciBoardInfo);
    device_id->setObjectName(QString::fromUtf8("device_id"));
    device_id->setGeometry(QRect(150, 80, 80, 20));
    label_sub_system_id = new QLabel(uiGrpBoxPciBoardInfo);
    label_sub_system_id->setObjectName(QString::fromUtf8("label_sub_system_id"));
    label_sub_system_id->setGeometry(QRect(20, 120, 111, 20));
    label_sub_system_id->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    sub_system_id = new QLabel(uiGrpBoxPciBoardInfo);
    sub_system_id->setObjectName(QString::fromUtf8("sub_system_id"));
    sub_system_id->setGeometry(QRect(150, 120, 80, 20));
    closeButton = new QPushButton(BoardDialog);
    closeButton->setObjectName(QString::fromUtf8("closeButton"));
    closeButton->setGeometry(QRect(440, 323, 75, 24));
    closeButton->setDefault(true);

    retranslateUi(BoardDialog);

    QSize size(535, 360);
    size = size.expandedTo(BoardDialog->minimumSizeHint());
    BoardDialog->resize(size);

    QObject::connect(closeButton, SIGNAL(clicked()), BoardDialog, SLOT(accept()));

    QMetaObject::connectSlotsByName(BoardDialog);
    } // setupUi

    void retranslateUi(QDialog *BoardDialog)
    {
    BoardDialog->setWindowTitle(QApplication::translate("BoardDialog", "Board Information", 0, QApplication::UnicodeUTF8));
    uiGrpBoxBrdVersions->setTitle(QApplication::translate("BoardDialog", "DVR Board Versions:", 0, QApplication::UnicodeUTF8));
    fw_version->setText(QString());
    label_12->setText(QApplication::translate("BoardDialog", "Firmware Build Date:", 0, QApplication::UnicodeUTF8));
    label_14->setText(QApplication::translate("BoardDialog", "Boot Loader Version:", 0, QApplication::UnicodeUTF8));
    label_8->setText(QApplication::translate("BoardDialog", "BSP Version:", 0, QApplication::UnicodeUTF8));
    label_10->setText(QApplication::translate("BoardDialog", "DVR Board Revision:", 0, QApplication::UnicodeUTF8));
    dvr_board_rev->setText(QString());
    label_13->setText(QApplication::translate("BoardDialog", "Chip Revision:", 0, QApplication::UnicodeUTF8));
    fw_build_date->setText(QString());
    boot_loader_version->setText(QString());
    dvr_chip_rev->setText(QString());
    label_serial_no->setText(QApplication::translate("BoardDialog", "Serial No.:", 0, QApplication::UnicodeUTF8));
    bsp_version->setText(QString());
    serial_no->setText(QString());
    label_6->setText(QApplication::translate("BoardDialog", "Firmware Version:", 0, QApplication::UnicodeUTF8));
    uiGrpBoxBrdConfig->setTitle(QApplication::translate("BoardDialog", "DVR Board Configuration:", 0, QApplication::UnicodeUTF8));
    videoInputsLabel->setText(QString());
    audioInputsLabel->setText(QString());
    label_9->setText(QApplication::translate("BoardDialog", "Number of sensors:", 0, QApplication::UnicodeUTF8));
    numSensorsLabel->setText(QString());
    label_11->setText(QApplication::translate("BoardDialog", "Number of relays:", 0, QApplication::UnicodeUTF8));
    numRelaysLabel->setText(QString());
    label->setText(QApplication::translate("BoardDialog", "SMO available:", 0, QApplication::UnicodeUTF8));
    SMOAvailableLabel->setText(QString());
    label_7->setText(QApplication::translate("BoardDialog", "Number of audio inputs:", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("BoardDialog", "Number of video inputs:", 0, QApplication::UnicodeUTF8));
    uiGrpBoxPciBoardInfo->setTitle(QApplication::translate("BoardDialog", "PCIe Board Information:", 0, QApplication::UnicodeUTF8));
    label_slot_num->setText(QApplication::translate("BoardDialog", "PCI Slot No.:", 0, QApplication::UnicodeUTF8));
    pci_slot_no->setText(QApplication::translate("BoardDialog", "pci slot", 0, QApplication::UnicodeUTF8));
    label_board_type->setText(QApplication::translate("BoardDialog", "Board Type:", 0, QApplication::UnicodeUTF8));
    board_type->setText(QApplication::translate("BoardDialog", "boardtype", 0, QApplication::UnicodeUTF8));
    label_vendor_id->setText(QApplication::translate("BoardDialog", "Vendor ID:", 0, QApplication::UnicodeUTF8));
    vendor_id->setText(QApplication::translate("BoardDialog", "vendor_id", 0, QApplication::UnicodeUTF8));
    label_sub_vendor_id->setText(QApplication::translate("BoardDialog", "Sub Vendor ID:", 0, QApplication::UnicodeUTF8));
    sub_vendor_id->setText(QApplication::translate("BoardDialog", "sub_vendor_id", 0, QApplication::UnicodeUTF8));
    label_device_id->setText(QApplication::translate("BoardDialog", "Device ID:", 0, QApplication::UnicodeUTF8));
    device_id->setText(QApplication::translate("BoardDialog", "Device ID", 0, QApplication::UnicodeUTF8));
    label_sub_system_id->setText(QApplication::translate("BoardDialog", "Sub System ID:", 0, QApplication::UnicodeUTF8));
    sub_system_id->setText(QApplication::translate("BoardDialog", "Sub system ID", 0, QApplication::UnicodeUTF8));
    closeButton->setText(QApplication::translate("BoardDialog", "Close", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(BoardDialog);
    } // retranslateUi

};

namespace Ui {
    class BoardDialog: public Ui_BoardDialog {};
} // namespace Ui

#endif // UI_BOARDDIALOG_H
