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

#include "smodialog.h"
#include "sdvr.h"


SmoDialog::SmoDialog(SpotMonitor *smo, QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    m_smo = smo;

    for (int i = 0; i < SDVR_MAX_OSD; ++i)
    {
        m_osdModified[i] = false;
        m_osdEnabled[i] = smo->isOsdEnabled(i);
        m_osdConfig[i] = smo->osdConfig(i);
    }

    osdNumberCombo->addItem("  1");
    osdNumberCombo->addItem("  2");

    osdTextEdit->setMaxLength(SDVR_MAX_OSD_EX_TEXT);

    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_TOP_LEFT),
        QVariant(SDVR_LOC_TOP_LEFT));
    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_TOP_RIGHT),
        QVariant(SDVR_LOC_TOP_RIGHT));
    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_BOTTOM_LEFT),
        QVariant(SDVR_LOC_BOTTOM_LEFT));
    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_BOTTOM_RIGHT),
        QVariant(SDVR_LOC_BOTTOM_RIGHT));
    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_CUSTOM),
        QVariant(SDVR_LOC_CUSTOM));

    osdLocXEdit->setValidator(new QIntValidator(0, 3000, osdLocXEdit));
    osdLocYEdit->setValidator(new QIntValidator(0, 3000, osdLocYEdit));

    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_NONE),
        QVariant(SDVR_OSD_DTS_NONE));
    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_MDY_12H),
        QVariant(SDVR_OSD_DTS_MDY_12H));
    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_DMY_12H),
        QVariant(SDVR_OSD_DTS_DMY_12H));
    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_YMD_12H),
        QVariant(SDVR_OSD_DTS_YMD_12H));
    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_MDY_24H),
        QVariant(SDVR_OSD_DTS_MDY_24H));
    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_DMY_24H),
        QVariant(SDVR_OSD_DTS_DMY_24H));
    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_YMD_24H),
        QVariant(SDVR_OSD_DTS_YMD_24H));
    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_DEBUG),
        QVariant(SDVR_OSD_DTS_DEBUG));

    connect(osdNumberCombo, SIGNAL(activated(int)),
        SLOT(onOsdSelectionChanged(int)));
    connect(osdLocationCombo, SIGNAL(activated(int)),
        SLOT(onOsdLocationComboActivated(int)));
    connect(osdTransSlider, SIGNAL(valueChanged(int)),
        SLOT(onOsdTransValueChanged(int)));

    selectOsd(0);
    onOsdLocationComboActivated(osdLocationCombo->currentIndex());

    setWindowTitle(QString("%1 - Properties").arg(smo->name()));
}

void SmoDialog::selectOsd(int index)
{
    osdEnabledCheckBox->setChecked(m_osdEnabled[index]);

    osdTextEdit->setText(QString((QChar *)(&m_osdConfig[index].text[0]),
            m_osdConfig[index].text_len));

    osdLocationCombo->setCurrentIndex(
        osdLocationCombo->findData(QVariant(m_osdConfig[index].location_ctrl)));

    osdLocXEdit->setText(QString::number(m_osdConfig[index].top_left_x));
    osdLocYEdit->setText(QString::number(m_osdConfig[index].top_left_y));

    osdDtsFormatCombo->setCurrentIndex(
        osdDtsFormatCombo->findData(QVariant(m_osdConfig[index].dts_format)));

    osdTransSlider->setValue(m_osdConfig[index].translucent);

    onOsdLocationComboActivated(osdLocationCombo->currentIndex());
    onOsdTransValueChanged(osdTransSlider->value());

    m_curOsd = index;
}

void SmoDialog::saveInput()
{
    m_osdEnabled[m_curOsd] = osdEnabledCheckBox->isChecked();

    QString text = osdTextEdit->text();
    m_osdConfig[m_curOsd].text_len = text.length();
    memcpy(&m_osdConfig[m_curOsd].text, text.data(), text.length() * 2);

    m_osdConfig[m_curOsd].location_ctrl = (sdvr_location_e)
        osdLocationCombo->itemData(osdLocationCombo->currentIndex()).toInt();

    m_osdConfig[m_curOsd].top_left_x = osdLocXEdit->text().toUInt();
    m_osdConfig[m_curOsd].top_left_y = osdLocYEdit->text().toUInt();

    m_osdConfig[m_curOsd].dts_format = (sdvr_dts_style_e)
        osdDtsFormatCombo->itemData(osdDtsFormatCombo->currentIndex()).toInt();

    m_osdConfig[m_curOsd].translucent = osdTransSlider->value();
}

void SmoDialog::onOsdSelectionChanged(int num)
{
    saveInput();
    selectOsd(num);
}

void SmoDialog::onOsdLocationComboActivated(int index)
{
    bool enabled = osdLocationCombo->itemData(index) == QVariant(SDVR_LOC_CUSTOM);
    osdLocXEdit->setEnabled(enabled);
    osdLocXLabel->setEnabled(enabled);
    osdLocYEdit->setEnabled(enabled);
    osdLocYLabel->setEnabled(enabled);
}

void SmoDialog::onOsdTransValueChanged(int value)
{
    osdTransValueLabel->setText(QString::number(value));
}

void SmoDialog::accept()
{
    saveInput();

    for (int i = 0; i < SDVR_MAX_OSD; ++i)
    {
        sdvr_osd_config_ex_t &cfg = m_smo->osdConfig(i);

        m_osdModified[i] =
            m_smo->isOsdEnabled(i) != m_osdEnabled[i] ||
            cfg.text_len != m_osdConfig[i].text_len ||
            memcmp(cfg.text, m_osdConfig[i].text, cfg.text_len * 2) != 0 ||
            cfg.location_ctrl != m_osdConfig[i].location_ctrl ||
            cfg.top_left_x != m_osdConfig[i].top_left_x ||
            cfg.top_left_y != m_osdConfig[i].top_left_y ||
            cfg.dts_format != m_osdConfig[i].dts_format ||
            cfg.translucent != m_osdConfig[i].translucent;

        if (m_osdModified[i])
        {
            m_smo->setOsdEnabled(i, m_osdEnabled[i]);
            m_smo->osdConfig(i) = m_osdConfig[i];
        }
    }

    QDialog::accept();
}
