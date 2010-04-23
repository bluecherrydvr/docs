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

#include "boarddialog.h"
#include "dvrboard.h"
#include "sdvr.h"

BoardDialog::BoardDialog(DVRBoard * board, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    sdvr_firmware_ver_t fw_version_value  = board->getFWVersion();

    // Fill in the PCI information.
    sdvr_board_attrib_t *bd_attrib = board->getAttrib();
    sdvr_pci_attrib_t *pci_attrib = board->getPCIAttrib();

    videoInputsLabel->setText(QString::number(board->videoInCount()));
    audioInputsLabel->setText(QString::number(board->audioInCount()));
    numSensorsLabel->setText(QString::number(board->sensorsCount()));
    numRelaysLabel->setText(QString::number(board->relaysCount()));
    SMOAvailableLabel->setText(board->hasSmo() ? "Yes" : "No");


    fw_version->setText(QString("%1.%2.%3.%4").arg(fw_version_value.fw_major).
        arg(fw_version_value.fw_minor).arg(fw_version_value.fw_revision).
        arg(fw_version_value.fw_build));
    fw_build_date->setText(QString("%1/%2/%3").arg(fw_version_value.fw_build_month).
        arg(fw_version_value.fw_build_day).arg(fw_version_value.fw_build_year));
    boot_loader_version->setText(QString("%1.%2").arg(fw_version_value.bootloader_major).
        arg(fw_version_value.bootloader_minor));
    bsp_version->setText(QString("%1.%2").arg(fw_version_value.bsp_major).
        arg(fw_version_value.bsp_minor));
    dvr_board_rev->setText(QString("%1").arg(bd_attrib->board_revision));
    dvr_chip_rev->setText(QString("%1").arg(sdvr::ChipRevToString(board->getChipRev())));
    serial_no->setText(QString("%1").arg(board->getSerialNo()));


    pci_slot_no->setText(QString("%1").arg(pci_attrib->pci_slot_num));
    board_type->setText(QString("0x%1").arg(pci_attrib->board_type,0,16));
    vendor_id->setText(QString("0x%1").arg(pci_attrib->vendor_id,0,16));
    device_id->setText(QString("0x%1").arg(pci_attrib->device_id,0,16));
    sub_vendor_id->setText(QString("0x%1").arg(pci_attrib->subsystem_vendor,0,16));
    sub_system_id->setText(QString("0x%1").arg(pci_attrib->subsystem_id,0,16));


}
