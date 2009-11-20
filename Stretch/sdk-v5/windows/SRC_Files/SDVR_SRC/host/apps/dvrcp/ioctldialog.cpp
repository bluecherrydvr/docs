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

#include "ioctldialog.h"
#include "dvrsystem.h"
#include "mainwindow.h"
#include "sdvr.h"
#include "sdvr_sdk.h"

extern DVRSystem *g_DVR;

IOCTLDialog::IOCTLDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

//    uiLEditDeviceID->setValidator(new QIntValidator(uiLEditDeviceID,));
//    uiLEditRegister->setValidator(new QIntValidator(uiLEditRegister));
//    uiLEditValue->setValidator(new QIntValidator(uiLEditValue));

    // Generate the list of connected cameras
    foreach (Camera *cam, g_DVR->cameraList)
        uiComboCamera->addItem(cam->name(), QVariant(cam->handle()));

    if (uiComboCamera->count() > 0)
    {
        uiComboCamera->setCurrentIndex(0);

        connect(uiBtnSend, SIGNAL(clicked()),
                this, SLOT(onBtnSend()));
        connect(uiBtnRead, SIGNAL(clicked()),
                this, SLOT(onBtnRead()));


    }
    else
    {
        uiBtnSend->setEnabled(false);
        uiBtnRead->setEnabled(false);
    }


}

void IOCTLDialog::onBtnSend()
{
    bool okDeviceId;
    bool okReg;
    bool okValue;
    int curCameraIndex = uiComboCamera->currentIndex();
    sdvr_chan_handle_t CameraHandle =  
        (sdvr_chan_handle_t)uiComboCamera->itemData(curCameraIndex).toInt();
    sx_uint8 deviceID = uiLEditDeviceID->text().toInt(&okDeviceId, 16);
    sx_uint8 registerNum = uiLEditRegister->text().toInt(&okReg, 16);
    sx_uint16 value = uiLEditValue->text().toInt(&okValue, 16);
    sdvr_err_e err;

    if (!okDeviceId)
    {
        QMessageBox::critical(this, "Device I/O Control", QString("Invalid device ID."));
        return;
    }

    if (!okReg)
    {
        QMessageBox::critical(this, "Device I/O Control", QString("Invalid register number."));
        return;
    }
    if (!okValue)
    {
        QMessageBox::critical(this, "Device I/O Control", QString("Invalid register value."));
        return;
    }

    err = sdvr_write_ioctl(CameraHandle,deviceID, registerNum, value);

    if (err != SDVR_ERR_NONE)
    {
        QMessageBox::critical(this, "Device I/O Control", QString("Failed to send the value to the device. [Error = %1 ").arg(err));
    }


}
void IOCTLDialog::onBtnRead()
{
    bool okDeviceId;
    bool okReg;

    int curCameraIndex = uiComboCamera->currentIndex();
    sdvr_chan_handle_t CameraHandle =  
        (sdvr_chan_handle_t)uiComboCamera->itemData(curCameraIndex).toInt();
    sx_uint8 deviceID = uiLEditDeviceID->text().toInt(&okDeviceId, 16);
    sx_uint8 registerNum = uiLEditRegister->text().toInt(&okReg, 16);
    sx_uint16 value;
    sdvr_err_e err;

    if (!okDeviceId)
    {
        QMessageBox::critical(this, "Device I/O Control", QString("Invalid device ID."));
        return;
    }

    if (!okReg)
    {
        QMessageBox::critical(this, "Device I/O Control", QString("Invalid register number."));
        return;
    }

    err = sdvr_read_ioctl(CameraHandle,deviceID, registerNum, &value);

    if (err != SDVR_ERR_NONE)
    {
        QMessageBox::critical(this, "Device I/O Control", QString("Failed to read the value from the device. [Error = %1] ").arg(err));
    }
    else
    {
        uiLEditValue->setText(QString("%1").arg(value,2,16));
    }

}


