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

#include "aboutdialog.h"
#include "dvrboard.h"
#include "dvrsystem.h"
#include "mainwindow.h"
#include "sdvr.h"
#include <QPainter>

extern sx_uint8 version_major, version_minor, version_revision, version_build;
extern DVRSystem *g_DVR;

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(QPixmap(":/images/about.png")));
    palette.setColor(foregroundRole(), Qt::black);
    setPalette(palette);

    sx_uint8 major, minor, revision, build;
    sdvr_firmware_ver_t fw_version_value;
    sdvr_sdk_params_t sdk_params;

    dvrcp_version->setText(QString("%1.%2.%3.%4").arg(version_major)
            .arg(version_minor).arg(version_revision).arg(version_build));

    sdvr_get_sdk_version(&major, &minor, &revision, &build);
    sdk_version->setText(QString("%1.%2.%3.%4").arg(major).arg(minor).arg(revision).arg(build));

    sdvr_get_driver_version(0,&major, &minor, &revision, &build);
    driver_version->setText(QString("%1.%2.%3.%4").arg(major).arg(minor).arg(revision).arg(build));


    g_DVR->getFWVersion(&fw_version_value);
    if (major || minor || revision || build)
    {
        fw_version->setText(QString("%1.%2.%3.%4").arg(fw_version_value.fw_major).
            arg(fw_version_value.fw_minor).arg(fw_version_value.fw_revision).
            arg(fw_version_value.fw_build));
        fw_build_date->setText(QString("%1/%2/%3").arg(fw_version_value.fw_build_month).
            arg(fw_version_value.fw_build_day).arg(fw_version_value.fw_build_year));
        boot_loader_version->setText(QString("%1.%2").arg(fw_version_value.bootloader_major).
            arg(fw_version_value.bootloader_minor));
        bsp_version->setText(QString("%1.%2").arg(fw_version_value.bsp_major).
            arg(fw_version_value.bsp_minor));
        dvr_board_type_rev->setText(QString("0x%1 Rev_%2%3").arg(g_DVR->board(0)->getAttrib()->board_type,0,16).
            arg(g_DVR->board(0)->getAttrib()->board_revision).
            arg(sdvr::BoardSubRevToString(g_DVR->board(0)->getAttrib()->board_sub_rev)));

        dvr_chip_rev->setText(QString("%1").arg(sdvr::ChipRevToString(g_DVR->board(0)->getChipRev())));
    }
    else
    {
        fw_version->setText(QString("_._._._"));
        g_MainWindow->setStatusBar("Failed to read the firmware version.");
        dvr_board_type_rev->setText("");
        dvr_chip_rev->setText("");

    }

    sdvr_get_sdk_params(&sdk_params);
    uiLEditLogFilePath->setText(sdk_params.debug_file_name);

}
