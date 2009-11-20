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

#include "monitordialog.h"
#include "viewsettings.h"
#include "dvrsystem.h"
#include "sdvr.h"
#include <QMessageBox>
#include <QInputDialog>


extern DVRSystem *g_DVR;

MonitorDialog::MonitorDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    rowsEdit->setValidator(new QIntValidator(1, 8, rowsEdit));
    colsEdit->setValidator(new QIntValidator(1, 8, colsEdit));

    decimationCombo->addItem("<select>",
        QVariant(SDVR_VIDEO_RES_DECIMATION_NONE));

    decimationCombo->addItem(
        sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_EQUAL),
        QVariant(SDVR_VIDEO_RES_DECIMATION_EQUAL));

    decimationCombo->addItem(
        sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_HALF),
        QVariant(SDVR_VIDEO_RES_DECIMATION_HALF));

    decimationCombo->addItem(
        sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_FOURTH),
        QVariant(SDVR_VIDEO_RES_DECIMATION_FOURTH));

    decimationCombo->addItem(
        sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_SIXTEENTH),
        QVariant(SDVR_VIDEO_RES_DECIMATION_SIXTEENTH));

    decimationCombo->addItem(
        sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF),
        QVariant(SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF));

    decimationCombo->addItem(
        sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF),
        QVariant(SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF));

    decimationCombo->addItem(
        sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF),
        QVariant(SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF));

    for (int i = 0; i < g_DVR->boardCount(); ++i)
        boardCombo->addItem(QString::number(i + 1));

    connect(decimationCombo, SIGNAL(activated(int)), SLOT(onDecimationComboActivated(int)));  

    smoLeftMarginEdit->setText("0");
    smoLeftMarginEdit->setValidator(new QIntValidator(0, 500, smoLeftMarginEdit));

    smoTopMarginEdit->setText("0");
    smoTopMarginEdit->setValidator(new QIntValidator(0, 500, smoTopMarginEdit));

    smoSpacingEdit->setText("0");
    smoSpacingEdit->setValidator(new QIntValidator(0, 100, smoSpacingEdit));
}

 
bool MonitorDialog::isSMOSupportedResolution(sdvr_video_res_decimation_e res_decimation)
{
    bool bRet = false;

    switch (res_decimation)
    {
    case SDVR_VIDEO_RES_DECIMATION_FOURTH: /*or  SDVR_VIDEO_RES_DECIMATION_CIF*/
    case SDVR_VIDEO_RES_DECIMATION_SIXTEENTH: /* or SDVR_VIDEO_RES_DECIMATION_QCIF*/
    case SDVR_VIDEO_RES_DECIMATION_EQUAL:
        bRet = true;
        break;
    default:
        bRet = false;
    }
    return bRet;
}

void MonitorDialog::onDecimationComboActivated(int index)
{
    sdvr_video_res_decimation_e dec = (sdvr_video_res_decimation_e)
        decimationCombo->itemData(index).toInt();

    // make sure the selected decimation is supported by the SMO
    smoGroup->setEnabled(isSMOSupportedResolution(dec));
    if (!isSMOSupportedResolution(dec))
        smoGroup->setChecked(false);
}

void MonitorDialog::accept()
{
    QString monitorName = nameEdit->text();
    if (monitorName.isEmpty())
    {
        QMessageBox::critical(this, "Input Error", "Monitor name is required");
        return;
    }

    if (g_DVR->findMonitorByName(monitorName) != NULL)
    {
        QMessageBox::critical(this, "Input Error", "Duplicate monitor name");
        return;
    }

     QString input = rowsEdit->text();
     if (input.isEmpty())
     {
         QMessageBox::critical(this, "Input Error", "Number of rows is required");
         return;
     }

     int pos;
     if (rowsEdit->validator()->validate(input, pos) != QValidator::Acceptable)
     {
         QMessageBox::critical(this, "Input Error", "Invalid number of rows");
         return;
     }

     input = colsEdit->text();
     if (input.isEmpty())
     {
         QMessageBox::critical(this, "Input Error", "Number of columns is required");
         return;
     }

     if (colsEdit->validator()->validate(input, pos) != QValidator::Acceptable)
     {
         QMessageBox::critical(this, "Input Error", "Invalid number of columns");
         return;
     }

     sdvr_video_res_decimation_e dec = (sdvr_video_res_decimation_e)
         decimationCombo->itemData(decimationCombo->currentIndex()).toInt();
     if (dec == SDVR_VIDEO_RES_DECIMATION_NONE)
     {
         QMessageBox::critical(this, "Input Error", "Resolution decimation is not specified");
         return;
     }

     if (smoGroup->isChecked())
     {
         input = smoLeftMarginEdit->text();
         if (smoLeftMarginEdit->validator()->validate(input, pos) != QValidator::Acceptable)
         {
             QMessageBox::critical(this, "Input Error", "Invalid left margin");
             return;
         }

         input = smoTopMarginEdit->text();
         if (smoTopMarginEdit->validator()->validate(input, pos) != QValidator::Acceptable)
         {
             QMessageBox::critical(this, "Input Error", "Invalid top margin");
             return;
         }
         input = smoSpacingEdit->text();
         if (smoSpacingEdit->validator()->validate(input, pos) != QValidator::Acceptable)
         {
             QMessageBox::critical(this, "Input Error", "Invalid spacing");
             return;
         }
     }

     QDialog::accept();
}

ViewSettings *MonitorDialog::monitorSettings() const
{
    ViewSettings *vs = NULL;

    if (result() == QDialog::Accepted)
    {
        QString name = nameEdit->text();
        int rows = rowsEdit->text().toInt();
        int cols = colsEdit->text().toInt();
        sdvr_video_res_decimation_e dec = (sdvr_video_res_decimation_e)
            decimationCombo->itemData(decimationCombo->currentIndex()).toInt();
        bool bEnableRawAudio = uiChkBoxEnableRawAudio->isChecked();

        vs = new ViewSettings(name, rows, cols, dec,bEnableRawAudio);
        if (smoGroup->isChecked())
        {
            vs->setSmo(true);
            vs->setSmoBoardIndex(boardCombo->currentIndex());
            vs->setSmoLeftMargin(smoLeftMarginEdit->text().toUInt());
            vs->setSmoTopMargin(smoTopMarginEdit->text().toUInt());
            vs->setSmoSpacing(smoSpacingEdit->text().toUInt());
        }

    }

    return vs;
}

ViewSettings *MonitorDialog::getMonitorSettings(QWidget *parent)
{
    MonitorDialog dialog(parent);
    if (dialog.exec() == QDialog::Accepted)
        return dialog.monitorSettings();

    return NULL;
}

QString MonitorDialog::getNewName(const QString &currentName, QWidget *parent)
{
    for (;;)
    {
        bool ok;
        QString input = QInputDialog::getText(parent, tr("Rename Monitor"),
            tr("New name:"), QLineEdit::Normal, currentName, &ok);

        if (!ok)
            break;

        if (input.isEmpty())
        {
            QMessageBox::critical(parent, "Input Error", "Monitor name is required");
        }
        else if (g_DVR->findMonitorByName(input) != NULL)
        {
            QMessageBox::critical(parent, "Input Error", "Duplicate monitor name");
        }
        else
            return input;
    }

    return QString::null;
}
