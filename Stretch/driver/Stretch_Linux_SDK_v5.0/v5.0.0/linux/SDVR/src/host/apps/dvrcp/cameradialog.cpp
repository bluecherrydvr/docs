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

#include "cameradialog.h"
#include "camera.h"
#include "sdvr.h"
#include "dvrsystem.h"
#include "videopanel.h"
#include "regiondialog.h"

#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>

extern DVRSystem *g_DVR;
extern VideoPanel *g_videoPanel;

CameraDialog::CameraDialog(Camera *camera, QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    m_camera = camera;

    sdvr_firmware_ver_t  version;
    g_DVR->getFWVersion(&version);
    m_FWVersionMajor = version.fw_major;
    m_FWVersionMinor = version.fw_minor;

    // Group the sub-encoder choices together so that we can toggle
    // between them easier.
    m_curSubEnc = SDVR_ENC_PRIMARY;
    uiRadioSubEncPrimary->setChecked(true);

    m_btnGrpSubEncoder = new QButtonGroup;

    m_btnGrpSubEncoder->addButton(uiRadioSubEncPrimary,SDVR_ENC_PRIMARY);
    m_btnGrpSubEncoder->addButton(uiRadioSubEncSecondary,SDVR_ENC_SECONDARY);

    // Setup all the signals
    connect(m_btnGrpSubEncoder, SIGNAL(buttonClicked(int)),
        SLOT(onBtnGrpSubEncoderClicked(int)));
    connect(uiBtnPrivacyRegions, SIGNAL(clicked()), SLOT(onBtnPrivacyRegions()));
    connect(uiBtnMotionRegions, SIGNAL(clicked()), SLOT(onBtnMDRegions()));
    connect(uiBtnBlindRegions, SIGNAL(clicked()), SLOT(onBtnBDRegions()));

    connect(uiHSliderTranslucent, SIGNAL(valueChanged(int)), SLOT(onTranslucentValueChanged(int)));  
    connect(uiSpinBoxOSDNum, SIGNAL(valueChanged(int)), SLOT(onOSDItemChanged(int)));  
    connect(osdLocationCombo, SIGNAL(activated(int)), SLOT(onComboOSDLocActivated(int)));  
   
    connect(uiHSliderHue, SIGNAL(valueChanged(int)), SLOT(onHueValueChanged(int)));  
    connect(uiHSliderBrightness, SIGNAL(valueChanged(int)), SLOT(onBrightnessValueChanged(int)));  
    connect(uiHSliderContrast, SIGNAL(valueChanged(int)), SLOT(onContrastValueChanged(int)));  
    connect(uiHSliderSaturation, SIGNAL(valueChanged(int)), SLOT(onSaturationValueChanged(int)));
    connect(uiHSliderSharpness, SIGNAL(valueChanged(int)), SLOT(onSharpnessValueChanged(int)));  

    connect(uiChkBoxDeinterlacing, SIGNAL(stateChanged(int)), SLOT(onChkBoxDeinterlacingChanged(int)));
    connect(uiChkBoxNoiseReduction, SIGNAL(stateChanged(int)), SLOT(onChkBoxNoiseReductionChanged(int)));
    connect(uiComboBoxCamTermination, SIGNAL(activated(int)), SLOT(onComboCamTerminationActivated(int)));  
    connect(uiHSliderGainMode, SIGNAL(valueChanged(int)), SLOT(onGainModeValueChanged(int)));  

    connect(uiChkBoxLiveUpdate, SIGNAL(stateChanged(int)), SLOT(onVideoLiveUpdateChanged(int)));
    connect(uiBtnResetAll, SIGNAL(clicked()), SLOT(onBtnResetAll()));

    connect(uiComboMDThresholds, SIGNAL(activated(int)), SLOT(onComboMBThresholds(int)));
    
    motionThresholdEdit->setValidator(
            new QIntValidator(0, 255, motionThresholdEdit));
    motionThresholdEdit->setText(QString::number(m_camera->MDThreshold(0)));

    m_nameModified = false;
    m_yuvFrameRateModified = false;
    m_MotionFrequencyModified = false;
    m_osdModified = false;
    m_MDModified  = false;
    m_BDModified  = false;
    m_NDModified  = false;
    m_PRModified  = false;

    m_MDRegionsModified = false;
    m_BDRegionsModified = false;
    m_NDRegionsModified = false;
    m_PRRegionsModified = false;

    m_curOSDItem = -1;

    for (uint subEnc = SDVR_ENC_PRIMARY; subEnc < MAX_SUB_ENCODERS; subEnc++)
    {
        m_videoEncModified[subEnc] = false;
        m_onAlarmVideoEncModified[subEnc] = false;
    }
    m_videoInModified = false;
    m_bDeinterlacingModified = false;
    m_bNoiseReductionModified = false;
    m_bGainModeModified = false;
    m_bCamTerminationModifed = false;

    m_bLiveUpdateVideoInSettings = true;
    m_audioEncModified = false;


    // We must save the original Privacy Regions in order
    // to decide if we need to clear/restore every time
    // we get into and out of Privacy Region dialogbox.
    // See onBtnPrivacyRegions() for explanation.
    m_origPRRegions = m_camera->PRRegion();

    // Enable tabs and controls for the current camera encoder
    enableTab();

    // Set the initial values for all the controls in all the tabs
    // As well as populating the combobox list data
    initAllControls();

}
void CameraDialog::enableTab()
{
    bool bEnableVideoInTab = false;

    // video-in parameter change is only supported in 4.0.0.0 or higher of firmware
    if (sdvr::isMinVersion(m_FWVersionMajor,m_FWVersionMinor, 0 , 0, 4, 0, 0, 0))
        bEnableVideoInTab = true;

    uiTabSettings->setTabEnabled(TAB_NDX_ENCODING, m_camera->videoFormat(m_curSubEnc) != SDVR_VIDEO_ENC_NONE); 
    uiTabSettings->setTabEnabled(TAB_NDX_ENC_ON_ALARM, m_camera->videoFormat(m_curSubEnc) != SDVR_VIDEO_ENC_NONE);
    uiTabSettings->setTabEnabled(TAB_NDX_AUDIO_ENCODER, m_camera->audioFormat() != SDVR_AUDIO_ENC_NONE);
    uiTabSettings->setTabEnabled(TAB_NDX_VIDEO_IN_SETTINGS, bEnableVideoInTab);

    // Enable controls on Encoding on Alarm Tab
    uiGrpEncOnAlarmMjpeg->setVisible(m_camera->videoFormat(m_curSubEnc) == SDVR_VIDEO_ENC_JPEG);
    uiGrpEncOnAlarmH264->setVisible(m_camera->videoFormat(m_curSubEnc) == SDVR_VIDEO_ENC_H264);
    uiGrpEncOnAlarmMPEG4->setVisible(m_camera->videoFormat(m_curSubEnc) == SDVR_VIDEO_ENC_MPEG4);

    // Enable controls on  Encoding Tab
    H264Group->setVisible(m_camera->videoFormat(m_curSubEnc) == SDVR_VIDEO_ENC_H264);
    JPEGGroup->setVisible(m_camera->videoFormat(m_curSubEnc) == SDVR_VIDEO_ENC_JPEG);
    uiGrpEncMPEG4->setVisible(m_camera->videoFormat(m_curSubEnc) == SDVR_VIDEO_ENC_MPEG4);
}

void CameraDialog::initAllControls()
{

    // Disabled the secondary encoder if one is not created
    uiRadioSubEncSecondary->setEnabled(m_camera->videoFormat(SDVR_ENC_SECONDARY) != SDVR_VIDEO_ENC_NONE);
    
    initGeneralTabCntrl();

    initAlarmsTabCntrl();

    if (m_camera->videoFormat(m_curSubEnc) != SDVR_VIDEO_ENC_NONE)
    {
        initEncodingTabCntrl();
        initEncodingOnAlarmTabCntrl();

        initEncodingTabData();
        initEncodingOnAlarmTabData();
    }

    initVideoInTabCntrl();
}

void CameraDialog::initVideoInTabCntrl()
{
    int index;

    // video-in parameter change is only supported in 4.0.0.0 or higher of firmware
    if (!sdvr::isMinVersion(m_FWVersionMajor,m_FWVersionMinor, 0 , 0, 4, 0, 0, 0))
        return;

    uiLabelHueValue->setText(QString("%1").arg(m_camera->getVParamHue()));
    uiHSliderHue->setValue(m_camera->getVParamHue());

    uiLabelBrightnessValue->setText(QString("%1").arg(m_camera->getVParamBrightness()));
    uiHSliderBrightness->setValue(m_camera->getVParamBrightness());

    uiLabelContrastValue->setText(QString("%1").arg(m_camera->getVParamContrast()));
    uiHSliderContrast->setValue(m_camera->getVParamContrast());

    uiLabelSaturationValue->setText(QString("%1").arg(m_camera->getVParamSaturation()));
    uiHSliderSaturation->setValue(m_camera->getVParamSaturation());

    uiLabelSharpnessValue->setText(QString("%1").arg(m_camera->getVParamSharpness()));
    uiHSliderSharpness->setValue(m_camera->getVParamSharpness());


    uiChkBoxDeinterlacing->setChecked(m_camera->isVParamsDeinterlacing());
    uiChkBoxNoiseReduction->setChecked(m_camera->isVParamNoiseReduction());

    uiLabelGainModeValue->setText(QString("%1").arg(m_camera->getVParamGainMode()));
    uiHSliderGainMode->setValue(m_camera->getVParamGainMode());

    uiComboBoxCamTermination->addItem(sdvr::camTerminationToString(SDVR_TERM_75OHM),
			QVariant(SDVR_TERM_75OHM));
    uiComboBoxCamTermination->addItem(sdvr::camTerminationToString(SDVR_TERM_HIGH_IMPEDANCE),
    		QVariant(SDVR_TERM_HIGH_IMPEDANCE));
    index = uiComboBoxCamTermination->findData(QVariant(m_camera->getVParamCamTermination()));
    uiComboBoxCamTermination->setCurrentIndex(index);

    // Live update is only availabe if the camera is connected.
    uiChkBoxLiveUpdate->setEnabled(m_camera->isChannelOpen());
    uiChkBoxLiveUpdate->setChecked(m_bLiveUpdateVideoInSettings && 
                                   m_camera->isChannelOpen());
}

void CameraDialog::initGeneralTabCntrl()
{
    int osdTextLen = SDVR_MAX_OSD_TEXT;

    if (!m_camera->name().isEmpty())
    {
        setWindowTitle(QString("%1 - %2").arg(m_camera->name()).arg(windowTitle()));
        cameraNameEdit->setText(m_camera->name());
    }

    uiSpinBoxRawVideoFrameRate->setRange(1,30);
    uiSpinBoxRawVideoFrameRate->setValue(m_camera->getYUVFrameRate());

    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_TOP_LEFT),
			QVariant(SDVR_LOC_TOP_LEFT));
    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_TOP_RIGHT),
    		QVariant(SDVR_LOC_TOP_RIGHT));
    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_BOTTOM_LEFT),
    		QVariant(SDVR_LOC_BOTTOM_LEFT));
    osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_BOTTOM_RIGHT),
    		QVariant(SDVR_LOC_BOTTOM_RIGHT));

    // remember the original OSD information for all the osd items.
    // This is needed so that we can rotate among different osd items
    // and compare its value to see if OSD info was changed. Also this
    // allows us to discard any osd info that was changed and the user
    // clicks on Cancel
    initOSDInfo();

    // The following OSD functionality only exist in 3.2.x.x or higher of firmware
    if (sdvr::isMinVersion(m_FWVersionMajor,m_FWVersionMinor, 0 , 0, 3, 2, 0, 0))
    {
        osdLocationCombo->addItem(sdvr::osdLocationToString(SDVR_LOC_CUSTOM),
    		    QVariant(SDVR_LOC_CUSTOM));

        uiSpinBoxOSDNum->setMaximum(2);
        uiLabelTranslucent->setEnabled(true);
        uiLabelTranslucentValue->setEnabled(true);
        uiHSliderTranslucent->setEnabled(true);
        uiLEditTLx->setValidator(new QIntValidator(0, 3000, uiLEditTLx));
        uiLEditTLy->setValidator(new QIntValidator(0, 3000, uiLEditTLy));

        osdTextLen = SDVR_MAX_OSD_EX_TEXT;
    }

    osdDtsFormatCombo->addItem(sdvr::dtsFormatToString(SDVR_OSD_DTS_DEBUG),
    		QVariant(SDVR_OSD_DTS_DEBUG));
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

    // OSD is not supported in VPP analytics mode.
    if (m_camera->getVPPMode() == SDVR_VPP_MODE_ANALYTICS)
    {
        osdGroup->setChecked(false);
        osdGroup->setEnabled(false);
    }
    else
    {
        osdGroup->setEnabled(true);
        osdGroup->setChecked(m_camera->isOsdEnabled());
    }
    osdTextEdit->setMaxLength(osdTextLen);
    onOSDItemChanged(1);
}

void CameraDialog::initAlarmsTabCntrl()
{
    motionDetectionGroup->setChecked(m_camera->isMDEnabled());


    uiComboMDThresholds->addItem(
		    "Layer 1 Threshold",
		    QVariant(m_camera->MDThreshold(0)));
    uiComboMDThresholds->addItem(
		    "Layer 2 Threshold",
		    QVariant(m_camera->MDThreshold(1)));
    uiComboMDThresholds->addItem(
		    "Layer 3 Threshold",
		    QVariant(m_camera->MDThreshold(2)));
    uiComboMDThresholds->addItem(
		    "Layer 4 Threshold",
		    QVariant(m_camera->MDThreshold(3)));
    
    m_lastThresholdIndex = 0;
    uiComboMDThresholds->setCurrentIndex(0);
    motionThresholdEdit->setValidator(
            new QIntValidator(0, 255, motionThresholdEdit));
    motionThresholdEdit->setText(QString::number(m_camera->MDThreshold(0)));

    blindDetectionGroup->setChecked(m_camera->isBDEnabled());
    blindThresholdEdit->setValidator(
            new QIntValidator(0, 255, blindThresholdEdit));
    blindThresholdEdit->setText(QString::number(m_camera->BDThreshold()));

    nightDetectionGroup->setChecked(m_camera->isNDEnabled());
    nightThresholdEdit->setValidator(
            new QIntValidator(0, 255, nightThresholdEdit));
    nightThresholdEdit->setText(QString::number(m_camera->NDThreshold()));

    privacyProtectionGroup->setChecked(m_camera->isPREnabled());

    uiLEditMotionValueFreq->setText(QString::number(m_camera->getMotionValueFreq()));
    uiLEditMotionValueFreq->setValidator(new QIntValidator(0, 255, uiLEditMotionValueFreq));

}

///////////////////////////////////////////////////////////////////////
// This method intializes the encoder data for the currently selected
// sub-encoder.
//
void CameraDialog::initEncodingTabData()
{
    // Set the encoder independent data for the current sub-encoder.
    frameRateEdit->setText(QString::number(m_camera->frameRate(m_curSubEnc)));
    decimationCombo->setCurrentIndex(
    		decimationCombo->findData(QVariant(m_camera->decimation(m_curSubEnc))));


    // Set the encoder specific data for the current sub-encoder.
    switch (m_camera->videoFormat(m_curSubEnc))
    {
    case SDVR_VIDEO_ENC_H264:
    {
        GOPSizeEdit->setText(QString::number(m_camera->h264GopSize(m_curSubEnc)));

        sdvr_br_control_e brc = m_camera->h264BitrateControl(m_curSubEnc);
        bitRateControlCombo->setCurrentIndex(bitRateControlCombo->findData(brc));

        avgBitRateEdit->setText(QString::number(m_camera->h264AvgBitrate(m_curSubEnc)));
        maxBitRateEdit->setText(QString::number(m_camera->h264MaxBitrate(m_curSubEnc)));

        h264QualityEdit->setText(QString::number(m_camera->h264Quality(m_curSubEnc)));

        onBrcComboActivated(bitRateControlCombo->currentIndex());
        break;
    }
    case SDVR_VIDEO_ENC_JPEG:
        jpegQualityEdit->setText(QString::number(m_camera->jpegQuality(m_curSubEnc)));
        break;
    case SDVR_VIDEO_ENC_MPEG4:
    {
        uiLEditEncGOPMPEG4->setText(QString::number(m_camera->MPEG4GopSize(m_curSubEnc)));

        sdvr_br_control_e brc = m_camera->MPEG4BitrateControl(m_curSubEnc);
        uiComboBitRateControlMPEG4->setCurrentIndex(uiComboBitRateControlMPEG4->findData(brc));

        uiLEditAvgBitRateMPEG4->setText(QString::number(m_camera->MPEG4AvgBitrate(m_curSubEnc)));
        uiLEditMaxBitRateMPEG4->setText(QString::number(m_camera->MPEG4MaxBitrate(m_curSubEnc)));

        uiLEditQualityMPEG4->setText(QString::number(m_camera->MPEG4Quality(m_curSubEnc)));

        onComboBitRateControlMPEG4Activated(uiComboBitRateControlMPEG4->currentIndex());
        break;
    }

    default:
        break;
    }
}
////////////////////////////////////////////////////////////////////
// This method initilaizes all the edit controls, comboboxes, etc
// that are used in the encoder tab for all the encoders that are
// used in either the secondary or primary tab.
//
void CameraDialog::initEncodingTabCntrl()
{
    // Initialize the common fields for all the encoders
    frameRateEdit->setValidator(new QIntValidator(1,
        sdvr::frameRate(g_DVR->videoStandard()), frameRateEdit));

    decimationCombo->addItem(
		    sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_EQUAL),
		    QVariant(SDVR_VIDEO_RES_DECIMATION_EQUAL));
    decimationCombo->addItem(
		    sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_FOURTH),
		    QVariant(SDVR_VIDEO_RES_DECIMATION_FOURTH));
    decimationCombo->addItem(
		    sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_SIXTEENTH),
		    QVariant(SDVR_VIDEO_RES_DECIMATION_SIXTEENTH));
    decimationCombo->addItem(
		    sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_HALF),
		    QVariant(SDVR_VIDEO_RES_DECIMATION_HALF));
    decimationCombo->addItem(
		    sdvr::decimationToString(SDVR_VIDEO_RES_DECIMATION_DCIF),
		    QVariant(SDVR_VIDEO_RES_DECIMATION_DCIF));

    bool bEnable = true;
    decimationCombo->setEnabled(bEnable);

    // Initialize the enocder specific fiels. 
    // NOTE: We have the initialize the encoder whether it is used for
    //       primary or secondary encoding.
    if (m_camera->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_H264 ||
        m_camera->videoFormat(SDVR_ENC_SECONDARY) == SDVR_VIDEO_ENC_H264)
    {

        bitRateControlCombo->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_VBR),
            QVariant(SDVR_BITRATE_CONTROL_VBR));

        bitRateControlCombo->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_CBR),
            QVariant(SDVR_BITRATE_CONTROL_CBR));

        bitRateControlCombo->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_CONSTANT_QUALITY),
            QVariant(SDVR_BITRATE_CONTROL_CONSTANT_QUALITY));

        GOPSizeEdit->setValidator(new QIntValidator(0, 100, GOPSizeEdit));
        avgBitRateEdit->setValidator(new QIntValidator(0, 10000, avgBitRateEdit));
        maxBitRateEdit->setValidator(new QIntValidator(0, 10000, maxBitRateEdit));
        h264QualityEdit->setValidator(new QIntValidator(0, 100, h264QualityEdit));

        connect(bitRateControlCombo, SIGNAL(activated(int)),
            SLOT(onBrcComboActivated(int)));

    }
    if (m_camera->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_JPEG ||
        m_camera->videoFormat(SDVR_ENC_SECONDARY) == SDVR_VIDEO_ENC_JPEG)
    {
        jpegQualityEdit->setValidator(
            new QIntValidator(1, 99, jpegQualityEdit));
    }
    if (m_camera->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_MPEG4 ||
        m_camera->videoFormat(SDVR_ENC_SECONDARY) == SDVR_VIDEO_ENC_MPEG4)
    {

        uiComboBitRateControlMPEG4->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_VBR),
            QVariant(SDVR_BITRATE_CONTROL_VBR));

        uiComboBitRateControlMPEG4->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_CBR),
            QVariant(SDVR_BITRATE_CONTROL_CBR));

        uiComboBitRateControlMPEG4->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_CONSTANT_QUALITY),
            QVariant(SDVR_BITRATE_CONTROL_CONSTANT_QUALITY));

        uiLEditEncGOPMPEG4->setValidator(new QIntValidator(0, 100, uiLEditEncGOPMPEG4));
        uiLEditAvgBitRateMPEG4->setValidator(new QIntValidator(0, 10000, uiLEditAvgBitRateMPEG4));
        uiLEditMaxBitRateMPEG4->setValidator(new QIntValidator(0, 10000, uiLEditMaxBitRateMPEG4));
        uiLEditQualityMPEG4->setValidator(new QIntValidator(0, 100, uiLEditQualityMPEG4));

        connect(uiComboBitRateControlMPEG4, SIGNAL(activated(int)),
            SLOT(onComboBitRateControlMPEG4Activated(int)));
    }
}
///////////////////////////////////////////////////////////////////////
// This method intializes the encod on alarm data for the currently selected
// sub-encoder.
void CameraDialog::initEncodingOnAlarmTabData()
{
    onAlarmEncodingCheck->setChecked(m_camera->isAdjustEncodingOnAlarmEnabled(m_curSubEnc));
    onAdjustEncodingOnAlarmChanged(onAlarmEncodingCheck->checkState());

    onAlarmMinOnEdit->setText(QString::number(m_camera->onAlarmMinOnTime(m_curSubEnc)));
    onAlarmMinOffEdit->setText(QString::number(m_camera->onAlarmMinOffTime(m_curSubEnc)));

    onAlarmFrameRateEdit->setText(QString::number(m_camera->onAlarmFrameRate(m_curSubEnc)));

    switch (m_camera->videoFormat(m_curSubEnc))
    {
    case SDVR_VIDEO_ENC_H264:
    {
        sdvr_br_control_e brc;

        onAlarmGOPSizeEdit->setText(QString::number(m_camera->onAlarmH264GopSize(m_curSubEnc)));

        brc = m_camera->onAlarmH264BitrateControl(m_curSubEnc);
        onAlarmBitRateControlCombo->setCurrentIndex(
            onAlarmBitRateControlCombo->findData(brc));

        onAlarmAvgBitRateEdit->setText(QString::number(m_camera->onAlarmH264AvgBitrate(m_curSubEnc)));
        onAlarmMaxBitRateEdit->setText(QString::number(m_camera->onAlarmH264MaxBitrate(m_curSubEnc)));

        onAlarmH264QualityEdit->setText(QString::number(m_camera->onAlarmH264Quality(m_curSubEnc)));
        onAlarmBrcComboActivated(onAlarmBitRateControlCombo->currentIndex());
        break;
    } 
    case SDVR_VIDEO_ENC_JPEG:
        uiLEditEncOnAlarmMjpegQuality->setText(QString::number(m_camera->getOnAlarmJpegQuality(m_curSubEnc)));
        break;
    case SDVR_VIDEO_ENC_MPEG4:
    {
        sdvr_br_control_e brc;

        uiLEditEncOnAlarmGOPMPEG4->setText(QString::number(m_camera->onAlarmMPEG4GopSize(m_curSubEnc)));

        brc = m_camera->onAlarmMPEG4BitrateControl(m_curSubEnc);
        uiComboEncOnAlarmBitRateControlMPEG4->setCurrentIndex(
            uiComboEncOnAlarmBitRateControlMPEG4->findData(brc));

        uiLEditEncOnAlarmAvgBitRateMPEG4->setText(QString::number(m_camera->onAlarmMPEG4AvgBitrate(m_curSubEnc)));
        uiLEditEncOnAlarmMaxBitRateMPEG4->setText(QString::number(m_camera->onAlarmMPEG4MaxBitrate(m_curSubEnc)));

        uiLEditEncOnAlarmQualityMPEG4->setText(QString::number(m_camera->onAlarmMPEG4Quality(m_curSubEnc)));
        onComboEncOnAlarmBitRateControlMPEG4Activated(uiComboEncOnAlarmBitRateControlMPEG4->currentIndex());
        break;
    } 

    default:
        break;
    }
}
////////////////////////////////////////////////////////////////////
// This method initilaizes all the edit controls, comboboxes, etc
// that are used in the Encode on Alarm tab for all the encoders that are
// used in either the secondary or primary encoder.
//
void CameraDialog::initEncodingOnAlarmTabCntrl()
{
    connect(onAlarmEncodingCheck, SIGNAL(stateChanged(int)),
        SLOT(onAdjustEncodingOnAlarmChanged(int)));
    onAlarmFrameRateEdit->setValidator(new QIntValidator(1,
        sdvr::frameRate(g_DVR->videoStandard()), onAlarmFrameRateEdit));


    // Setup all the encoder specific controls for both the primary
    // and secondary encoders
    if (m_camera->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_H264 ||
        m_camera->videoFormat(SDVR_ENC_SECONDARY) == SDVR_VIDEO_ENC_H264)
    {
        onAlarmBitRateControlCombo->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_VBR),
            QVariant(SDVR_BITRATE_CONTROL_VBR));

        onAlarmBitRateControlCombo->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_CBR),
            QVariant(SDVR_BITRATE_CONTROL_CBR));

        onAlarmBitRateControlCombo->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_CONSTANT_QUALITY),
            QVariant(SDVR_BITRATE_CONTROL_CONSTANT_QUALITY));

        onAlarmGOPSizeEdit->setValidator(new QIntValidator(0, 100, onAlarmGOPSizeEdit));
        onAlarmAvgBitRateEdit->setValidator(new QIntValidator(0, 10000, onAlarmAvgBitRateEdit));
        onAlarmMaxBitRateEdit->setValidator(new QIntValidator(0, 10000, onAlarmMaxBitRateEdit));

        onAlarmH264QualityEdit->setValidator(new QIntValidator(0, 100, onAlarmH264QualityEdit));

        connect(onAlarmBitRateControlCombo, SIGNAL(activated(int)),
            SLOT(onAlarmBrcComboActivated(int)));

    } 
    if (m_camera->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_JPEG ||
            m_camera->videoFormat(SDVR_ENC_SECONDARY) == SDVR_VIDEO_ENC_JPEG)
    {
        uiLEditEncOnAlarmMjpegQuality->setValidator(
            new QIntValidator(1, 99, uiLEditEncOnAlarmMjpegQuality));
    }
    if (m_camera->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_MPEG4 ||
        m_camera->videoFormat(SDVR_ENC_SECONDARY) == SDVR_VIDEO_ENC_MPEG4)
    {
        uiComboEncOnAlarmBitRateControlMPEG4->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_VBR),
            QVariant(SDVR_BITRATE_CONTROL_VBR));

        uiComboEncOnAlarmBitRateControlMPEG4->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_CBR),
            QVariant(SDVR_BITRATE_CONTROL_CBR));

        uiComboEncOnAlarmBitRateControlMPEG4->addItem(
            sdvr::brcToString(SDVR_BITRATE_CONTROL_CONSTANT_QUALITY),
            QVariant(SDVR_BITRATE_CONTROL_CONSTANT_QUALITY));

        uiLEditEncOnAlarmGOPMPEG4->setValidator(new QIntValidator(0, 100, uiLEditEncOnAlarmGOPMPEG4));
        uiLEditEncOnAlarmAvgBitRateMPEG4->setValidator(new QIntValidator(0, 10000, uiLEditEncOnAlarmAvgBitRateMPEG4));
        uiLEditEncOnAlarmMaxBitRateMPEG4->setValidator(new QIntValidator(0, 10000, uiLEditEncOnAlarmMaxBitRateMPEG4));

        uiLEditEncOnAlarmQualityMPEG4->setValidator(new QIntValidator(0, 100, uiLEditEncOnAlarmQualityMPEG4));

        connect(uiComboEncOnAlarmBitRateControlMPEG4, SIGNAL(activated(int)),
            SLOT(onComboEncOnAlarmBitRateControlMPEG4Activated(int)));

    } 
}

void CameraDialog::onBrcComboActivated(int index)
{
    sdvr_br_control_e brc =
        (sdvr_br_control_e) bitRateControlCombo->itemData(index).toInt();

    switch (brc)
    {
    case SDVR_BITRATE_CONTROL_VBR:
        avgBitRateEdit->setEnabled(true);
        maxBitRateEdit->setEnabled(true);
        h264QualityEdit->setEnabled(false);
        break;

    case SDVR_BITRATE_CONTROL_CBR:
        avgBitRateEdit->setEnabled(true);
        maxBitRateEdit->setEnabled(false);
        h264QualityEdit->setEnabled(false);
        break;

    case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
        avgBitRateEdit->setEnabled(false);
        maxBitRateEdit->setEnabled(true);
        h264QualityEdit->setEnabled(true);
        break;

    default:
        break;
    }
}

void CameraDialog::onAlarmBrcComboActivated(int index)
{
    sdvr_br_control_e brc =
        (sdvr_br_control_e) onAlarmBitRateControlCombo->itemData(index).toInt();

    switch (brc)
    {
    case SDVR_BITRATE_CONTROL_VBR:
        onAlarmAvgBitRateEdit->setEnabled(true);
        onAlarmMaxBitRateEdit->setEnabled(true);
        onAlarmH264QualityEdit->setEnabled(false);
        break;

    case SDVR_BITRATE_CONTROL_CBR:
        onAlarmAvgBitRateEdit->setEnabled(true);
        onAlarmMaxBitRateEdit->setEnabled(false);
        onAlarmH264QualityEdit->setEnabled(false);
        break;

    case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
        onAlarmAvgBitRateEdit->setEnabled(false);
        onAlarmMaxBitRateEdit->setEnabled(true);
        onAlarmH264QualityEdit->setEnabled(true);
        break;

    default:
        break;
    }
}

void CameraDialog::onComboBitRateControlMPEG4Activated(int index)
{
    sdvr_br_control_e brc =
        (sdvr_br_control_e) uiComboBitRateControlMPEG4->itemData(index).toInt();

    switch (brc)
    {
    case SDVR_BITRATE_CONTROL_VBR:
        uiLEditAvgBitRateMPEG4->setEnabled(true);
        uiLEditMaxBitRateMPEG4->setEnabled(true);
        uiLEditQualityMPEG4->setEnabled(false);
        break;

    case SDVR_BITRATE_CONTROL_CBR:
        uiLEditAvgBitRateMPEG4->setEnabled(true);
        uiLEditMaxBitRateMPEG4->setEnabled(false);
        uiLEditQualityMPEG4->setEnabled(false);
        break;

    case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
        uiLEditAvgBitRateMPEG4->setEnabled(false);
        uiLEditMaxBitRateMPEG4->setEnabled(true);
        uiLEditQualityMPEG4->setEnabled(true);
        break;

    default:
        break;
    }
}

void CameraDialog::onComboEncOnAlarmBitRateControlMPEG4Activated(int index)
{
    sdvr_br_control_e brc =
        (sdvr_br_control_e) uiComboEncOnAlarmBitRateControlMPEG4->itemData(index).toInt();

    switch (brc)
    {
    case SDVR_BITRATE_CONTROL_VBR:
        uiLEditEncOnAlarmAvgBitRateMPEG4->setEnabled(true);
        uiLEditEncOnAlarmMaxBitRateMPEG4->setEnabled(true);
        uiLEditEncOnAlarmQualityMPEG4->setEnabled(false);
        break;

    case SDVR_BITRATE_CONTROL_CBR:
        uiLEditEncOnAlarmAvgBitRateMPEG4->setEnabled(true);
        uiLEditEncOnAlarmMaxBitRateMPEG4->setEnabled(false);
        uiLEditEncOnAlarmQualityMPEG4->setEnabled(false);
        break;

    case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
        uiLEditEncOnAlarmAvgBitRateMPEG4->setEnabled(false);
        uiLEditEncOnAlarmMaxBitRateMPEG4->setEnabled(true);
        uiLEditEncOnAlarmQualityMPEG4->setEnabled(true);
        break;

    default:
        break;
    }
}
void CameraDialog::onAdjustEncodingOnAlarmChanged(int state)
{
    bool enabled = (state == Qt::Checked);
    onAlarmMinOnEdit->setEnabled(enabled);
    onAlarmMinOffEdit->setEnabled(enabled);
    onAlarmFrameRateEdit->setEnabled(enabled);
    switch (m_camera->videoFormat(m_curSubEnc))
    {
        case SDVR_VIDEO_ENC_H264:
            uiGrpEncOnAlarmH264->setEnabled(enabled);
            break;
        case SDVR_VIDEO_ENC_JPEG:
            uiGrpEncOnAlarmMjpeg->setEnabled(enabled);
            break;
        case SDVR_VIDEO_ENC_MPEG4:
            uiGrpEncOnAlarmMPEG4->setEnabled(enabled);
            break;
        default:
            break;
    }
}
void CameraDialog::onBtnPrivacyRegions()
{
    sdvr_err_e err;

	if (g_videoPanel != NULL)
	{
		g_videoPanel->stopPlaying();
		g_videoPanel->hide();
	}

    if (m_camera->PRRegion().isInitialized())
        m_camera->PRRegion().migrate(g_DVR->videoStandard());
    else
        m_camera->PRRegion().init(g_DVR->videoStandard(), 1);

    if (!m_origPRRegions.isEmpty())
    {
        // If there was some existing privacy blocking we need
        // to remove it from the video frames. Otherwise, as we
        // erase MB from privacy region we see the old privacy
        // blocking MB in the video frame.
        // NOTE: We need to reset the OLD privacy blocking
        // if the dialogbox is being canceled.
        CRegion emptyPRRegion(g_DVR->videoStandard(), 1);

        err =  sdvr_set_regions_map(m_camera->handle(), SDVR_REGION_PRIVACY,
                                    (sx_uint8*)emptyPRRegion.data());
    }

	RegionDialog dialog(m_camera, &m_camera->PRRegion(), this);
	dialog.setWindowTitle(
		QString("%1 : Privacy Protection Region").arg(m_camera->name()));
	if (dialog.exec() == QDialog::Accepted)
	{
        m_PRRegionsModified = true;
	}

    // NOTE: We need to reset the OLD privacy blocking
    // if cleared until the user actually accepts the changes.
    if (!m_origPRRegions.isEmpty())
    {
        err =  sdvr_set_regions_map(m_camera->handle(), SDVR_REGION_PRIVACY,
                                    (sx_uint8*)m_origPRRegions.data());
    }

	if (g_videoPanel != NULL)
	{
		g_videoPanel->show();
		g_videoPanel->startPlaying();
	}
}

void CameraDialog::onBtnMDRegions()
{
	if (g_videoPanel != NULL)
	{
		g_videoPanel->stopPlaying();
		g_videoPanel->hide();
	}

    if (m_camera->MDRegion().isInitialized())
        m_camera->MDRegion().migrate(g_DVR->videoStandard());
    else
		m_camera->MDRegion().init(g_DVR->videoStandard(), 4);

	RegionDialog dialog(m_camera, &m_camera->MDRegion(), this);
	dialog.setWindowTitle(
		QString("%1 : Motion Detection Region").arg(m_camera->name()));

	if (dialog.exec() == QDialog::Accepted)
	{
        m_MDRegionsModified = true;
	}

	if (g_videoPanel != NULL)
	{
		g_videoPanel->show();
		g_videoPanel->startPlaying();
	}
}

void CameraDialog::onBtnBDRegions()
{
	if (g_videoPanel != NULL)
	{
		g_videoPanel->stopPlaying();
		g_videoPanel->hide();
	}

	if (m_camera->BDRegion().isInitialized())
        m_camera->BDRegion().migrate(g_DVR->videoStandard());
    else
		m_camera->BDRegion().init(g_DVR->videoStandard(), 1);

	RegionDialog dialog(m_camera, &m_camera->BDRegion(), this);
	dialog.setWindowTitle(
		QString("%1 : Blind Detection Region").arg(m_camera->name()));
	if (dialog.exec() == QDialog::Accepted)
	{
        m_BDRegionsModified = true;
	}

	if (g_videoPanel != NULL)
	{
		g_videoPanel->show();
		g_videoPanel->startPlaying();
	}
}

void CameraDialog::onBtnGrpSubEncoderClicked(int subEnc)
{
    bool bFixChange = true;
    // make sure the previouse encoder settings are valid
    if (!ValidateEncoderTab(m_curSubEnc) || !ValidateOnAlarmEncoderTab(m_curSubEnc))
    {
        // todo: there was an error in the previous encoders
        //       prompt the user if you should lose them
        if (bFixChange)
        {
            if (m_curSubEnc == SDVR_ENC_PRIMARY)
                uiRadioSubEncPrimary->setChecked(true);
            else if (m_curSubEnc == SDVR_ENC_SECONDARY)
                uiRadioSubEncSecondary->setChecked(true);
            return;
        }
    }

    // save the previous seetings
    SaveEncoderTab(m_curSubEnc);
    SaveOnAlarmEncoderTab(m_curSubEnc);

    // re-set the encoder tabs for the new encoder settings
    m_curSubEnc = subEnc;
    initEncodingTabData();
    initEncodingOnAlarmTabData();
    enableTab();
}

void CameraDialog::onTranslucentValueChanged(int value)
{
    uiLabelTranslucentValue->setText(QString("%1").arg(value));
}
void CameraDialog::onHueValueChanged(int nHueValue)
{
    uiLabelHueValue->setText(QString("%1").arg(nHueValue));
    if (m_bLiveUpdateVideoInSettings)
    {
        m_camera->applyVideoInHCSBSSettings(nHueValue, -1, -1, -1, -1);
        // We must indicate the video-in settings was
        // changed, so that if the user cancels the dialogbox
        // we can change them back.
        m_videoInModified = true;
    }

}
void CameraDialog::onBrightnessValueChanged(int nBrightnessValue)
{
    uiLabelBrightnessValue->setText(QString("%1").arg(nBrightnessValue));
    if (m_bLiveUpdateVideoInSettings)
    {
        m_camera->applyVideoInHCSBSSettings(-1, -1, -1, nBrightnessValue, -1);
        // We must indicate the video-in settings was
        // changed, so that if the user cancels the dialogbox
        // we can change them back.
        m_videoInModified = true;
    }


}
void CameraDialog::onContrastValueChanged(int nContrastValue)
{
    uiLabelContrastValue->setText(QString("%1").arg(nContrastValue));
    if (m_bLiveUpdateVideoInSettings)
    {
        m_camera->applyVideoInHCSBSSettings(-1, nContrastValue, -1, -1, -1);
        // We must indicate the video-in settings was
        // changed, so that if the user cancels the dialogbox
        // we can change them back.
        m_videoInModified = true;
    }

}
void CameraDialog::onSaturationValueChanged(int nSaturationValue)
{
    uiLabelSaturationValue->setText(QString("%1").arg(nSaturationValue));
    if (m_bLiveUpdateVideoInSettings)
    {
        m_camera->applyVideoInHCSBSSettings(-1, -1, nSaturationValue, -1, -1);
        // We must indicate the video-in settings was
        // changed, so that if the user cancels the dialogbox
        // we can change them back.
        m_videoInModified = true;
    }
}

void CameraDialog::onSharpnessValueChanged(int nSharpnessValue)
{
    uiLabelSharpnessValue->setText(QString("%1").arg(nSharpnessValue));
    if (m_bLiveUpdateVideoInSettings)
    {
        m_camera->applyVideoInHCSBSSettings(-1, -1, -1, -1, nSharpnessValue);
        // We must indicate the video-in settings was
        // changed, so that if the user cancels the dialogbox
        // we can change them back.
        m_videoInModified = true;
    }

}


void CameraDialog::onOSDItemChanged(int value)
{
    int locationIndex;
    int dtsIndex;
    sdvr_location_e location;
    sdvr_dts_style_e dts;

    // Save the previous OSD item data
    if (m_curOSDItem != -1)
    {
        SaveTempCurOSDInfo(m_curOSDItem);
    }
    m_curOSDItem = value - 1;

    // Update the OSD information for the currently 
    // selected item
    
    osdTextEdit->setText(m_osdText[m_curOSDItem]);
    location = m_osdLoc[m_curOSDItem];
    locationIndex = osdLocationCombo->findData(QVariant(location));
    osdLocationCombo->setCurrentIndex(locationIndex);
    onComboOSDLocActivated(locationIndex);
    uiLEditTLx->setText(QString("%1").arg(m_osdLocx[m_curOSDItem]));
    uiLEditTLy->setText(QString("%1").arg(m_osdLocy[m_curOSDItem]));


    dts = m_osdDtsFormat[m_curOSDItem];
    dtsIndex = osdDtsFormatCombo->findData(QVariant(dts));
    osdDtsFormatCombo->setCurrentIndex(dtsIndex);

    osdDtsGroup->setChecked(m_osdDtsEnabled[m_curOSDItem]);

    uiLabelTranslucentValue->setText(QString("%1").arg(m_osdTranslucent[m_curOSDItem]));
    uiHSliderTranslucent->setValue(m_osdTranslucent[m_curOSDItem]);
    uiChkBoxOSDShow->setChecked(m_osdShow[m_curOSDItem]);
}


void CameraDialog::onComboOSDLocActivated(int index)
{
    bool bEnabled = (index == SDVR_LOC_CUSTOM);

    uiLableTLx->setEnabled(bEnabled);
    uiLEditTLx->setEnabled(bEnabled);
    uiLableTLy->setEnabled(bEnabled);
    uiLEditTLy->setEnabled(bEnabled);

}

void CameraDialog::onComboMBThresholds(int index)
{
    int newThreshold;
    int prevThreshold;
    
    // Save the  threshold value of the previous
    // threshold layer.
    prevThreshold = motionThresholdEdit->text().toInt();
    uiComboMDThresholds->setItemData(m_lastThresholdIndex,
		    QVariant(prevThreshold));


    // Set the new threshold value for the currently selected
    // threshold layer.
    newThreshold =  uiComboMDThresholds->itemData(index).toInt();
    motionThresholdEdit->setText(QString::number(newThreshold));

    m_lastThresholdIndex = index;

}
void CameraDialog::accept()
{

    if (!ValidateEncoderTab(m_curSubEnc) || !ValidateOnAlarmEncoderTab(m_curSubEnc) ||
        !ValidateGeneralTab() || !ValidateAlarmsTabs())
        return;

    SaveGeneralTab();
    SaveEncoderTab(m_curSubEnc);
    SaveAlarmsTabs();
    SaveOnAlarmEncoderTab(m_curSubEnc);

    // video-in parameter change is only supported in 4.0.0.0 or higher of firmware
    if (sdvr::isMinVersion(m_FWVersionMajor,m_FWVersionMinor, 0 , 0, 4, 0, 0, 0))
        SaveVideoInSettingsTab();

    QDialog::accept();
}

bool CameraDialog::SaveVideoInSettingsTab()
{
    Camera result(*m_camera); // Create a temporary camera object
    int nCurHueValue = uiLabelHueValue->text().toInt();
    int nCurBrightnessValue = uiLabelBrightnessValue->text().toInt();
    int nCurContrastValue = uiLabelContrastValue->text().toInt();
    int nCurSaturationValue = uiLabelSaturationValue->text().toInt();
    int nCurSharpnessValue = uiLabelSharpnessValue->text().toInt();
    bool bDeinterlacing = (uiChkBoxDeinterlacing->checkState() == Qt::Checked);
    bool bNoiseReduction = (uiChkBoxNoiseReduction->checkState() == Qt::Checked);
    sx_uint8 nGainMode = (sx_uint8)uiLabelGainModeValue->text().toInt();
    int index = uiComboBoxCamTermination->currentIndex();
    sdvr_term_e term = (sdvr_term_e)uiComboBoxCamTermination->itemData(index).toInt();

    m_videoInModified = (m_camera->getVParamHue() != nCurHueValue) ||
        (m_camera->getVParamBrightness() != nCurBrightnessValue) ||
        (m_camera->getVParamContrast() != nCurContrastValue) ||
        (m_camera->getVParamSaturation() != nCurSaturationValue) ||
        (m_camera->getVParamSharpness() != nCurSharpnessValue);

    m_bDeinterlacingModified = (m_camera->isVParamsDeinterlacing()!= bDeinterlacing);
    if (m_bDeinterlacingModified)
    { 
        m_camera->setVParamsDeinterlacing(bDeinterlacing);
    }
    m_bNoiseReductionModified = (m_camera->isVParamNoiseReduction() != bNoiseReduction);
    if (m_bNoiseReductionModified)
    {
        m_camera->setVParamNoiseReduction(bNoiseReduction);
    }
    m_bGainModeModified = (m_camera->getVParamGainMode() != nGainMode);
    if (m_bGainModeModified)
    {
        m_camera->setVParamGainMode(nGainMode);
    }
    m_bCamTerminationModifed = (m_camera->getVParamCamTermination() != term);
    if (m_bCamTerminationModifed)
    {
        m_camera->setVParamCamTermination(term);
    }

    // We must send the video-in settings to the board if
    // 1) Any setting is changed but live update is not set
    // 2) Any setting is changed and live update is set but
    //    the user is caneceling the changes
    if (m_videoInModified)
    {
        m_camera->setVParamHue(nCurHueValue);
        m_camera->setVParamBrightness(nCurBrightnessValue);
        m_camera->setVParamContrast(nCurContrastValue);
        m_camera->setVParamSaturation(nCurSaturationValue);
        m_camera->setVParamSharpness(nCurSharpnessValue);
    }

    // Since we already sent the video-in settings to the board
    // there is no need to send them again.
    if (m_bLiveUpdateVideoInSettings)
    {
        m_videoInModified = false;
        m_bDeinterlacingModified = false;
        m_bNoiseReductionModified = false;
        m_bGainModeModified = false;
        m_bCamTerminationModifed = false;
    }
    return true;
}

bool CameraDialog::ValidateGeneralTab()
{
    Camera *foundCamera;
    QString name(QString(cameraNameEdit->text()).trimmed());

    if (name.isEmpty())
    {
        QMessageBox::critical(this, "Input Error", "Camera name is required");
        return false;
    }

    foundCamera = g_DVR->findCameraByName(name);

    // Make sure no other camera with the same name exist.
    if (foundCamera != NULL)
    {
        if (m_camera->id() != foundCamera->id())
        {
            QMessageBox::critical(this, "Input Error", "Camera name must be unique in order to prevent any conflicts in recorded file names.");
            return false;

        }
    }
    return true;
}

bool CameraDialog::SaveGeneralTab()
{
    Camera result(*m_camera); // Create a temporary camera object

    result.setName(QString(cameraNameEdit->text()).trimmed());
    m_nameModified = m_camera->name() != result.name();

    result.setYUVFrameRate(uiSpinBoxRawVideoFrameRate->value());
    m_yuvFrameRateModified = m_camera->getYUVFrameRate() != result.getYUVFrameRate();

    int osdMax = 1;
    sx_uint16 result_x, result_y, camera_x, camera_y;

    // First read the current osd item data
    SaveTempCurOSDInfo(m_curOSDItem);

    // Get the data for all the osd items into the result camera
    // The following OSD functionality only exist in 3.2.x.x or higher of firmware
    if (sdvr::isMinVersion(m_FWVersionMajor,m_FWVersionMinor, 0 , 0, 3, 2, 0, 0))
        osdMax = SDVR_MAX_OSD;
    for (int osdItem = 0; osdItem < osdMax; osdItem++)
    {
        result.setOsdText(osdItem, m_osdText[osdItem]);
        result.setOsdLocation(osdItem, m_osdLoc[osdItem]);
        result.setOsdDtsEnabled(osdItem, m_osdDtsEnabled[osdItem]);
        result.setOsdDtsFormat(osdItem, m_osdDtsFormat[osdItem]);
        result.setOsdCustomLoc(osdItem, m_osdLocx[osdItem], m_osdLocy[osdItem]);
        result.setOsdTranslucent(osdItem, m_osdTranslucent[osdItem]);
        result.setOsdShow(osdItem, m_osdShow[osdItem]);  
    }
    result.setOsdEnabled(osdGroup->isChecked());

    // Compare if any of the data for any of the osd items were changed.
    m_osdModified = (m_camera->isOsdEnabled() != result.isOsdEnabled());

    for (int osdItem = 0; osdItem < osdMax && !m_osdModified; osdItem++)
    {
        result.getOsdCustomLoc(osdItem, &result_x, &result_y);
        m_camera->getOsdCustomLoc(osdItem, &camera_x, &camera_y);

		m_osdModified =	(m_osdModified
                    || m_camera->osdText(osdItem) != result.osdText(osdItem)
					|| m_camera->osdLocation(osdItem) != result.osdLocation(osdItem)
					|| m_camera->isOsdDtsEnabled(osdItem) != result.isOsdDtsEnabled(osdItem)
					|| m_camera->osdDtsFormat(osdItem) != result.osdDtsFormat(osdItem)
                    || m_camera->getOsdTranslucent(osdItem) != result.getOsdTranslucent(osdItem)
                    || m_camera->isOsdShow(osdItem) != result.isOsdShow(osdItem)
                    || camera_x != result_x
                    || camera_y != result_y );
    }

    *m_camera = result;
    return true;
}
void CameraDialog::SaveTempCurOSDInfo(int osdItem)
{
        m_osdText[osdItem] = osdTextEdit->text();
        m_osdLoc[osdItem] = (sdvr_location_e)
    		osdLocationCombo->itemData(osdLocationCombo->currentIndex()).toInt();
        m_osdDtsEnabled[osdItem] = osdDtsGroup->isChecked();
        m_osdDtsFormat[osdItem] =(sdvr_dts_style_e)
    		osdDtsFormatCombo->itemData(osdDtsFormatCombo->currentIndex()).toInt();
        m_osdLocx[osdItem] = uiLEditTLx->text().toInt();
        m_osdLocy[osdItem] = uiLEditTLy->text().toInt();
        m_osdTranslucent[osdItem] = uiHSliderTranslucent->value();
        m_osdShow[osdItem] = uiChkBoxOSDShow->isChecked();  
}


void CameraDialog::initOSDInfo()
{
    int osdMax = 1;

    // The following OSD functionality only exist in 3.2.x.x or higher of firmware
    if (sdvr::isMinVersion(m_FWVersionMajor,m_FWVersionMinor, 0 , 0, 3, 2, 0, 0))
        osdMax = SDVR_MAX_OSD;
    for (int osdItem = 0; osdItem < osdMax; osdItem++)
    {
        m_osdText[osdItem] = m_camera->osdText(osdItem);
        m_osdLoc[osdItem] = m_camera->osdLocation(osdItem);
        m_osdDtsEnabled[osdItem] = m_camera->isOsdDtsEnabled(osdItem);
        m_osdDtsFormat[osdItem] = m_camera->osdDtsFormat(osdItem);
        m_camera->getOsdCustomLoc(osdItem, &m_osdLocx[osdItem], &m_osdLocy[osdItem]);
        m_osdTranslucent[osdItem] = m_camera->getOsdTranslucent(osdItem);
        m_osdShow[osdItem] = m_camera->isOsdShow(osdItem);  
    }
}


bool CameraDialog::ValidateAlarmsTabs()
{
    QString input;

    if (motionDetectionGroup->isChecked())
    {
        input = motionThresholdEdit->text();
        if (input.isEmpty())
        {
            QMessageBox::critical(this, "Input Error",
                    "Motion detection threshold is required");
            return false;
        }

        int pos;
        if (motionThresholdEdit->validator()->validate(input, pos) !=
            QValidator::Acceptable)
        {
            QMessageBox::critical(this, "Input Error",
                    "Invalid motion detection threshold");
            return false;
        }
    }

    if (blindDetectionGroup->isChecked())
    {
        input = blindThresholdEdit->text();
        if (input.isEmpty())
        {
            QMessageBox::critical(this, "Input Error",
                    "Blind detection threshold is required");
            return false;
        }

        int pos;
        if (blindThresholdEdit->validator()->validate(input, pos) !=
            QValidator::Acceptable)
        {
            QMessageBox::critical(this, "Input Error",
                    "Invalid blind detection threshold");
            return false;
        }
    }

    if (nightDetectionGroup->isChecked())
    {
        input = nightThresholdEdit->text();
        if (input.isEmpty())
        {
            QMessageBox::critical(this, "Input Error",
                    "Night detection threshold is required");
            return false;
        }

        int pos;
        if (nightThresholdEdit->validator()->validate(input, pos) !=
            QValidator::Acceptable)
        {
            QMessageBox::critical(this, "Input Error",
                    "Invalid night detection threshold");
            return false;
        }

    }
    return true;
}

bool CameraDialog::SaveAlarmsTabs()
{
    Camera result(*m_camera); // Create a temporary camera object
    QString input;
    bool ok;

    uint uMotionValueFreq = uiLEditMotionValueFreq->text().toUInt(&ok);
    if (!ok)
    {
        uMotionValueFreq = 0;
    }
    result.setMotionValueFreq(uMotionValueFreq);

    result.setMDEnabled(motionDetectionGroup->isChecked());
    if (motionDetectionGroup->isChecked())
    {
        int threshold;

        // Make sure the last threshold value was saved
        threshold = motionThresholdEdit->text().toInt();
        uiComboMDThresholds->setItemData(m_lastThresholdIndex,
		        QVariant(threshold));

        // Read all the MD threshold for all the layers.
        for (int Nth = 0; Nth < MAX_NLAYERS; Nth++)
        {
            threshold =  uiComboMDThresholds->itemData(Nth).toInt();
            result.setMDThreshold(Nth,threshold);
        }
    }

    result.setBDEnabled(blindDetectionGroup->isChecked());
    if (blindDetectionGroup->isChecked())
    {
        input = blindThresholdEdit->text();
        result.setBDThreshold(input.toInt());
    }

    result.setNDEnabled(nightDetectionGroup->isChecked());
    if (nightDetectionGroup->isChecked())
    {
        input = nightThresholdEdit->text();
        result.setNDThreshold(input.toInt());
    }

    result.setPREnabled(privacyProtectionGroup->isChecked());

    m_MDModified = m_MDModified ||
                   result.isMDEnabled() != m_camera->isMDEnabled() ||
                   result.MDThreshold(0) != m_camera->MDThreshold(0) ||
                   result.MDThreshold(1) != m_camera->MDThreshold(1) ||
                   result.MDThreshold(2) != m_camera->MDThreshold(2) ||
                   result.MDThreshold(3) != m_camera->MDThreshold(3);
    
    m_BDModified = m_BDModified ||
                   result.isBDEnabled() != m_camera->isBDEnabled() ||
                   result.BDThreshold() != m_camera->BDThreshold();

    m_NDModified = result.isNDEnabled() != m_camera->isNDEnabled() ||
                   result.NDThreshold() != m_camera->NDThreshold();

    m_PRModified = m_PRModified ||
                   result.isPREnabled() != m_camera->isPREnabled();

    m_MotionFrequencyModified = m_MotionFrequencyModified ||
                   result.getMotionValueFreq() != m_camera->getMotionValueFreq();

    *m_camera = result;

    return true;
}
bool CameraDialog::SaveEncoderTab(uint subEnc)
{
    Camera result(*m_camera); // Create a temporary camera object
    int num;
    unsigned int uNum;
    QString input;
    bool ok;

    if (m_camera->videoFormat(subEnc) != SDVR_VIDEO_ENC_NONE)
    {
        // Save the none-encoder specific parameters

        result.setDecimation(subEnc,(sdvr_video_res_decimation_e)
            decimationCombo->itemData(decimationCombo->currentIndex()).toInt());

        if (frameRateEdit->isModified())
        {
            input = frameRateEdit->text();
            num = input.toInt();
            result.setFrameRate(subEnc, num);
        }

        m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
            result.decimation(subEnc) != m_camera->decimation(subEnc) ||
            result.frameRate(subEnc) != m_camera->frameRate(subEnc);

        m_camera->setDecimation(subEnc, result.decimation(subEnc));
        m_camera->setFrameRate(subEnc, result.frameRate(subEnc));


        // Save the encoder specific parameters
        switch (m_camera->videoFormat(subEnc))
        {
        case SDVR_VIDEO_ENC_H264:
        {
            result.setH264GopSize(subEnc, GOPSizeEdit->text().toUInt(&ok));

            sdvr_br_control_e brc = (sdvr_br_control_e)
                bitRateControlCombo->itemData(bitRateControlCombo->currentIndex()).toInt();
            result.setH264BitrateControl(subEnc, brc);

            m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                result.h264GopSize(subEnc) != m_camera->h264GopSize(subEnc) ||
                result.h264BitrateControl(subEnc) != m_camera->h264BitrateControl(subEnc);

            m_camera->setH264GopSize(subEnc, result.h264GopSize(subEnc));
            m_camera->setH264BitrateControl(subEnc, result.h264BitrateControl(subEnc));

            switch (brc)
            {
            case SDVR_BITRATE_CONTROL_CBR:
            case SDVR_BITRATE_CONTROL_VBR:

                uNum = avgBitRateEdit->text().toUInt(&ok);
                m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                    uNum != m_camera->h264AvgBitrate(subEnc);
                m_camera->setH264AvgBitrate(subEnc, uNum);


                if (brc == SDVR_BITRATE_CONTROL_VBR)
                {
                    uNum =  maxBitRateEdit->text().toUInt(&ok);
                    m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                        uNum != m_camera->h264MaxBitrate(subEnc);
                    m_camera->setH264MaxBitrate(subEnc, uNum);
                }
                break;

            case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
                input = h264QualityEdit->text();
                uNum = input.toUInt();

                m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                    uNum != m_camera->h264Quality(subEnc);
                m_camera->setH264Quality(subEnc, uNum);

                uNum =  maxBitRateEdit->text().toUInt(&ok);
                m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                    uNum != m_camera->h264MaxBitrate(subEnc);
                m_camera->setH264MaxBitrate(subEnc, uNum);

                break;
            default:
                break;
            } //switch (brc)

            break;
        }
        case  SDVR_VIDEO_ENC_JPEG:
        
            input = jpegQualityEdit->text();
            uNum = input.toUInt();
//            result.setJpegImageStyle(subEnc, false);    // For now the only image style supported is
                                                // AVI MJPEG or still JPEG which is 0


            m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                uNum != m_camera->jpegQuality(subEnc);
            m_camera->setJpegQuality(subEnc,uNum);

            break;

        case SDVR_VIDEO_ENC_MPEG4:
        {
            result.setMPEG4GopSize(subEnc, uiLEditEncGOPMPEG4->text().toUInt(&ok));

            sdvr_br_control_e brc = (sdvr_br_control_e)
                uiComboBitRateControlMPEG4->itemData(uiComboBitRateControlMPEG4->currentIndex()).toInt();
            result.setMPEG4BitrateControl(subEnc, brc);

            m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                result.MPEG4GopSize(subEnc) != m_camera->MPEG4GopSize(subEnc) ||
                result.MPEG4BitrateControl(subEnc) != m_camera->MPEG4BitrateControl(subEnc);

            m_camera->setMPEG4GopSize(subEnc, result.MPEG4GopSize(subEnc));
            m_camera->setMPEG4BitrateControl(subEnc, result.MPEG4BitrateControl(subEnc));

            switch (brc)
            {
            case SDVR_BITRATE_CONTROL_CBR:
            case SDVR_BITRATE_CONTROL_VBR:

                uNum = uiLEditAvgBitRateMPEG4->text().toUInt(&ok);
                m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                    uNum != m_camera->MPEG4AvgBitrate(subEnc);
                m_camera->setMPEG4AvgBitrate(subEnc, uNum);


                if (brc == SDVR_BITRATE_CONTROL_VBR)
                {
                    uNum =  uiLEditMaxBitRateMPEG4->text().toUInt(&ok);
                    m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                        uNum != m_camera->MPEG4MaxBitrate(subEnc);
                    m_camera->setMPEG4MaxBitrate(subEnc, uNum);
                }
                break;

            case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
                input = uiLEditQualityMPEG4->text();
                uNum = input.toUInt();

                m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                    uNum != m_camera->MPEG4Quality(subEnc);
                m_camera->setMPEG4Quality(subEnc, uNum);

                uNum =  uiLEditMaxBitRateMPEG4->text().toUInt(&ok);
                m_videoEncModified[subEnc] = m_videoEncModified[subEnc] ||
                    uNum != m_camera->MPEG4MaxBitrate(subEnc);
                m_camera->setMPEG4MaxBitrate(subEnc, uNum);

                break;
            default:
                break;
            } //switch (brc)

            break;
        }

        default:
            break;
        }
    } // if (m_camera->videoFormat(subEnc) != SDVR_VIDEO_ENC_NONE)

    return true;
}

bool CameraDialog::ValidateEncoderTab(uint subEnc)
{
    QString input;
    bool ok;
    int num;

    // if either no encoder is assigned for the given sub-encoder,
    // No need to validate the parameters
    if (m_camera->videoFormat(subEnc) == SDVR_VIDEO_ENC_NONE)
    {
        return true;
    }

    // validate none-encoder specific parameters
    if (frameRateEdit->isModified())
    {
        input = frameRateEdit->text();
        if (input.isEmpty())
        {
            QMessageBox::critical(this, "Input Error", "Frame rate is required");
            return false;
        }

        int pos;
        if (frameRateEdit->validator()->validate(input, pos) != QValidator::Acceptable)
        {
            QMessageBox::critical(this, "Input Error", "Invalid frame rate");
            return false;
        }

    }

    // validate encoder specific parameters

    switch (m_camera->videoFormat(subEnc))
    {
    case SDVR_VIDEO_ENC_H264:
    {
        GOPSizeEdit->text().toUInt(&ok);
        if (!ok)
        {
            QMessageBox::critical(this, "Input Error", "Invalid or empty H.264 GOP size");
            return false;
        }

        sdvr_br_control_e brc = (sdvr_br_control_e)
            bitRateControlCombo->itemData(bitRateControlCombo->currentIndex()).toInt();

        switch (brc)
        {
        case SDVR_BITRATE_CONTROL_CBR:
        case SDVR_BITRATE_CONTROL_VBR:
            avgBitRateEdit->text().toUInt(&ok);
            if (!ok)
            {
                QMessageBox::critical(this, "Input Error",
                    "Invalid or empty H.264 average bit rate");
                return false;
            }
            if (brc == SDVR_BITRATE_CONTROL_VBR)
            {
                maxBitRateEdit->text().toUInt(&ok);
                if (!ok)
                {
                    QMessageBox::critical(this, "Input Error",
                        "Invalid or empty H.264 maximum bit rate");
                    return false;
                }
            }
            break;
        case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
           input = h264QualityEdit->text();
           num = input.toUInt(&ok);
            if (input.isEmpty() || !ok || (num > 100))
            {
                QMessageBox::critical(this, "Input Error", "H.264 quality must be a number between 0 - 100.");
                return false;
            }

            int pos;
            if (h264QualityEdit->validator()->validate(input, pos) != QValidator::Acceptable)
            {
                QMessageBox::critical(this, "Input Error", "Invalid H.264 quality");
                return false;
            }
            maxBitRateEdit->text().toUInt(&ok);
            if (!ok)
            {
                QMessageBox::critical(this, "Input Error",
                    "Invalid or empty H.264 maximum bit rate");
                return false;
            }
            break;
        default:
            break;
        } //switch (brc)
        break;
    }
    case  SDVR_VIDEO_ENC_JPEG:
    {
        input = jpegQualityEdit->text();

        num = input.toInt(&ok);
        if (input.isEmpty() || !ok || (num < 1 || num > 99) )
        {
            QMessageBox::critical(this, "Input Error", "'MJPEG quality' must be between  1 - 99.");
            return false;
        }
        int pos;
        if (jpegQualityEdit->validator()->validate(input, pos) != QValidator::Acceptable)
        {
            QMessageBox::critical(this, "Input Error", "JPEG 'quality' must in the range of 1 - 99.");
            return false;
        }
        break;
    }
    case SDVR_VIDEO_ENC_MPEG4:
    {
        uiLEditEncGOPMPEG4->text().toUInt(&ok);
        if (!ok)
        {
            QMessageBox::critical(this, "Input Error", "Invalid or empty MPEG4 GOP size");
            return false;
        }

        sdvr_br_control_e brc = (sdvr_br_control_e)
            uiComboBitRateControlMPEG4->itemData(uiComboBitRateControlMPEG4->currentIndex()).toInt();

        switch (brc)
        {
        case SDVR_BITRATE_CONTROL_CBR:
        case SDVR_BITRATE_CONTROL_VBR:
            uiLEditAvgBitRateMPEG4->text().toUInt(&ok);
            if (!ok)
            {
                QMessageBox::critical(this, "Input Error",
                    "Invalid or empty MPEG4 average bit rate");
                return false;
            }
            if (brc == SDVR_BITRATE_CONTROL_VBR)
            {
                uiLEditMaxBitRateMPEG4->text().toUInt(&ok);
                if (!ok)
                {
                    QMessageBox::critical(this, "Input Error",
                        "Invalid or empty MPEG4 maximum bit rate");
                    return false;
                }
            }
            break;
        case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
           input = uiLEditQualityMPEG4->text();
           num = input.toUInt(&ok);
            if (input.isEmpty() || !ok || (num > 100))
            {
                QMessageBox::critical(this, "Input Error", "MPEG4 quality must be a number between 0 - 100.");
                return false;
            }

            int pos;
            if (uiLEditQualityMPEG4->validator()->validate(input, pos) != QValidator::Acceptable)
            {
                QMessageBox::critical(this, "Input Error", "Invalid MPEG4 quality");
                return false;
            }
            uiLEditMaxBitRateMPEG4->text().toUInt(&ok);
            if (!ok)
            {
                QMessageBox::critical(this, "Input Error",
                    "Invalid or empty MPEG4 maximum bit rate");
                return false;
            }
            break;
        default:
            break;
        } //switch (brc)
        break;
    }

    default:
        break;
    }

    return true;
}

bool CameraDialog::SaveOnAlarmEncoderTab(uint subEnc)
{
    Camera result(*m_camera); // Create a temporary camera object
    int num;
    uint uNum;
    QString input;
    bool ok;

    if (m_camera->videoFormat(subEnc) != SDVR_VIDEO_ENC_NONE)
    {
        result.setAdjustEncodingOnAlarmEnabled(subEnc, onAlarmEncodingCheck->isChecked());
        m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
            result.isAdjustEncodingOnAlarmEnabled(subEnc) != m_camera->isAdjustEncodingOnAlarmEnabled(subEnc);

        m_camera->setAdjustEncodingOnAlarmEnabled(subEnc, onAlarmEncodingCheck->isChecked());
        // NOTE: Eventhough the encoding on alarm might be disabled
        //       it is possible to change parameter after disabling the on alarm encoder

        // Save none-encoder specific parameters
        if (onAlarmFrameRateEdit->isModified())
        {
            input = onAlarmFrameRateEdit->text();
            result.setOnAlarmFrameRate(subEnc, input.toInt());
        }

        if (onAlarmMinOnEdit->isModified())
        {
            input = onAlarmMinOnEdit->text();
            num = input.toUInt(&ok);
            result.setOnAlarmMinOnTime(subEnc, num);
        }

        if (onAlarmMinOffEdit->isModified())
        {
            input = onAlarmMinOffEdit->text();
            num = input.toUInt(&ok);
            result.setOnAlarmMinOffTime(subEnc, num);
        }
        
        m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
            result.onAlarmMinOnTime(subEnc) != m_camera->onAlarmMinOnTime(subEnc) ||
            result.onAlarmMinOffTime(subEnc) != m_camera->onAlarmMinOffTime(subEnc) ||
            result.onAlarmFrameRate(subEnc) != m_camera->onAlarmFrameRate(subEnc);

        m_camera->setOnAlarmFrameRate(subEnc, result.onAlarmFrameRate(subEnc));
        m_camera->setOnAlarmMinOnTime(subEnc, result.onAlarmMinOnTime(subEnc));
        m_camera->setOnAlarmMinOffTime(subEnc, result.onAlarmMinOffTime(subEnc));

        // Save encoder specific parameters

        switch (m_camera->videoFormat(subEnc))
        {
        case SDVR_VIDEO_ENC_H264:
        {
            uNum = onAlarmGOPSizeEdit->text().toUInt(&ok);
            result.setOnAlarmH264GopSize(subEnc,uNum);

            sdvr_br_control_e brc = (sdvr_br_control_e) onAlarmBitRateControlCombo->itemData(
                onAlarmBitRateControlCombo->currentIndex()).toInt();
            result.setOnAlarmH264BitrateControl(subEnc, brc);

            m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                result.onAlarmH264GopSize(subEnc) != m_camera->onAlarmH264GopSize(subEnc) ||
                result.onAlarmH264BitrateControl(subEnc) != m_camera->onAlarmH264BitrateControl(subEnc);
  
            m_camera->setOnAlarmH264GopSize(subEnc,result.onAlarmH264GopSize(subEnc));
            m_camera->setOnAlarmH264BitrateControl(subEnc, result.onAlarmH264BitrateControl(subEnc));


            switch (brc)
            {
            case SDVR_BITRATE_CONTROL_CBR:
            case SDVR_BITRATE_CONTROL_VBR:
                uNum = onAlarmAvgBitRateEdit->text().toUInt(&ok);

                m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                    result.onAlarmH264AvgBitrate(subEnc) != m_camera->onAlarmH264AvgBitrate(subEnc);
                m_camera->setOnAlarmH264AvgBitrate(subEnc, uNum);


                if (brc == SDVR_BITRATE_CONTROL_VBR)
                {
                    uNum = onAlarmMaxBitRateEdit->text().toUInt(&ok);

                    m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                        uNum != m_camera->onAlarmH264MaxBitrate(subEnc);
                    m_camera->setOnAlarmH264MaxBitrate(subEnc, uNum);
                }
                break;

            case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
                input = onAlarmH264QualityEdit->text();
                uNum = input.toUInt();

                m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                    uNum != m_camera->onAlarmH264Quality(subEnc);
                m_camera->setOnAlarmH264Quality(subEnc, uNum);

                uNum = onAlarmMaxBitRateEdit->text().toUInt(&ok);

                m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                    uNum != m_camera->onAlarmH264MaxBitrate(subEnc);
                m_camera->setOnAlarmH264MaxBitrate(subEnc, uNum);

                break;
            default:
                break;
            } //  switch (brc)
            break;
        }
        case  SDVR_VIDEO_ENC_JPEG:
        
            input = uiLEditEncOnAlarmMjpegQuality->text();
            uNum = input.toUInt();

            m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                uNum != m_camera->getOnAlarmJpegQuality(subEnc);
            m_camera->setonAlarmJpegQuality(subEnc, uNum);

            break;

        case SDVR_VIDEO_ENC_MPEG4:
        {
            uNum = uiLEditEncOnAlarmGOPMPEG4->text().toUInt(&ok);
            result.setOnAlarmMPEG4GopSize(subEnc,uNum);

            sdvr_br_control_e brc = (sdvr_br_control_e) uiComboEncOnAlarmBitRateControlMPEG4->itemData(
                uiComboEncOnAlarmBitRateControlMPEG4->currentIndex()).toInt();
            result.setOnAlarmMPEG4BitrateControl(subEnc, brc);

            m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                result.onAlarmMPEG4GopSize(subEnc) != m_camera->onAlarmMPEG4GopSize(subEnc) ||
                result.onAlarmMPEG4BitrateControl(subEnc) != m_camera->onAlarmMPEG4BitrateControl(subEnc);
  
            m_camera->setOnAlarmMPEG4GopSize(subEnc,result.onAlarmMPEG4GopSize(subEnc));
            m_camera->setOnAlarmMPEG4BitrateControl(subEnc, result.onAlarmMPEG4BitrateControl(subEnc));


            switch (brc)
            {
            case SDVR_BITRATE_CONTROL_CBR:
            case SDVR_BITRATE_CONTROL_VBR:
                uNum = uiLEditEncOnAlarmAvgBitRateMPEG4->text().toUInt(&ok);

                m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                    result.onAlarmMPEG4AvgBitrate(subEnc) != m_camera->onAlarmMPEG4AvgBitrate(subEnc);
                m_camera->setOnAlarmMPEG4AvgBitrate(subEnc, uNum);


                if (brc == SDVR_BITRATE_CONTROL_VBR)
                {
                    uNum = uiLEditEncOnAlarmMaxBitRateMPEG4->text().toUInt(&ok);

                    m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                        uNum != m_camera->onAlarmMPEG4MaxBitrate(subEnc);
                    m_camera->setOnAlarmMPEG4MaxBitrate(subEnc, uNum);
                }
                break;

            case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
                input = uiLEditEncOnAlarmQualityMPEG4->text();
                uNum = input.toUInt();

                m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                    uNum != m_camera->onAlarmMPEG4Quality(subEnc);
                m_camera->setOnAlarmMPEG4Quality(subEnc, uNum);

                uNum = uiLEditEncOnAlarmMaxBitRateMPEG4->text().toUInt(&ok);

                m_onAlarmVideoEncModified[subEnc] = m_onAlarmVideoEncModified[subEnc] ||
                    uNum != m_camera->onAlarmMPEG4MaxBitrate(subEnc);
                m_camera->setOnAlarmMPEG4MaxBitrate(subEnc, uNum);

                break;
            default:
                break;

            } //  switch (brc)
            break;
        }

        default:
            break;
        }
    } // if (m_camera->videoFormat(subEnc) != SDVR_VIDEO_ENC_NONE)

    // Save new the new on Alarm Parameters
    return true;
}

bool CameraDialog::ValidateOnAlarmEncoderTab(uint subEnc)
{
    QString input;
    bool ok;
    int num;

    // if either no encoder is assigned for the given sub-encoder or
    // On alarm encoding is disabled. No need to validate the parameters
    if (m_camera->videoFormat(subEnc) == SDVR_VIDEO_ENC_NONE ||
        !onAlarmEncodingCheck->isChecked())
    {
        return true;
    }

    // validate none-encoder specific parameters
    if (onAlarmFrameRateEdit->isModified())
    {
        input = onAlarmFrameRateEdit->text();

        if (input.isEmpty())
        {
            QMessageBox::critical(this, "Input Error", "On alarm frame rate is required");
            return false;
        }

        int pos;
        if (onAlarmFrameRateEdit->validator()->validate(input, pos) != QValidator::Acceptable)
        {
            QMessageBox::critical(this, "Input Error", "Invalid on alarm frame rate");
            return false;
        }


        input = frameRateEdit->text();
        if (input.isEmpty())
        {
            QMessageBox::critical(this, "Input Error", "Frame rate is required");
            return false;
        }
    }

    if (onAlarmMinOnEdit->isModified())
    {
        input = onAlarmMinOnEdit->text();
        if (input.isEmpty())
        {
            QMessageBox::critical(this, "Input Error", "Minimum ON time is required");
            return false;
        }
        input.toUInt(&ok);
        if (!ok)
        {
            QMessageBox::critical(this, "Input Error", "Minimum ON time is required");
            return false;
        }
    }

    if (onAlarmMinOffEdit->isModified())
    {
        input = onAlarmMinOffEdit->text();
        if (input.isEmpty())
        {
            QMessageBox::critical(this, "Input Error", "Minimum OFF time is required");
            return false;
        }

        if (!ok)
        {
            QMessageBox::critical(this, "Input Error", "Minimum OFF time is required");
            return false;
        }
    }
    
    // validate encoder specific parameters

    switch (m_camera->videoFormat(subEnc))
    {
        case SDVR_VIDEO_ENC_H264:
        {
            sdvr_br_control_e brc;

            onAlarmGOPSizeEdit->text().toUInt(&ok);
            if (!ok)
            {
                QMessageBox::critical(this, "Input Error", "Invalid or empty H.264 on alarm GOP size");
                return false;
            }

            brc = (sdvr_br_control_e) onAlarmBitRateControlCombo->itemData(
                onAlarmBitRateControlCombo->currentIndex()).toInt();

            switch (brc)
            {
            case SDVR_BITRATE_CONTROL_CBR:
            case SDVR_BITRATE_CONTROL_VBR:
                onAlarmAvgBitRateEdit->text().toUInt(&ok);
                if (!ok)
                {
                    QMessageBox::critical(this, "Input Error",
                        "Invalid or empty H.264 on alarm average bit rate");
                    return false;
                }

                if (brc == SDVR_BITRATE_CONTROL_VBR)
                {
                    onAlarmMaxBitRateEdit->text().toUInt(&ok);
                    if (!ok)
                    {
                        QMessageBox::critical(this, "Input Error",
                            "Invalid or empty H.264 on alarm maximum bit rate");
                        return false;
                    }
                }
                break;

            case  SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
                input = onAlarmH264QualityEdit->text();
                num = input.toInt(&ok);
                if (input.isEmpty() || !ok || (num > 100))
                {
                   QMessageBox::critical(this, "Input Error", "On alarm quality is required. A number between 0 - 100");
                   return false;
                }

                int pos;
                if (onAlarmH264QualityEdit->validator()->validate(input, pos) != QValidator::Acceptable)
                {
                    QMessageBox::critical(this, "Input Error", "Invalid on alarm H.264 quality");
                    return false;
                }
                onAlarmMaxBitRateEdit->text().toUInt(&ok);
                if (!ok)
                {
                    QMessageBox::critical(this, "Input Error",
                        "Invalid or empty H.264 on alarm maximum bit rate");
                    return false;
                }

                break;

            default:
                break;
            } //switch (brc)

            break;
        }
        case  SDVR_VIDEO_ENC_JPEG:
        
            input = uiLEditEncOnAlarmMjpegQuality->text();
            num = input.toInt(&ok);
            if (input.isEmpty() || !ok || (num < 1 || num > 99) )
            {
                QMessageBox::critical(this, "Input Error", "Encoding on Alarm 'MJPEG quality' must be between  1 - 99.");
                return false;
            }
            break;

        case SDVR_VIDEO_ENC_MPEG4:
        {
            sdvr_br_control_e brc;

            uiLEditEncOnAlarmGOPMPEG4->text().toUInt(&ok);
            if (!ok)
            {
                QMessageBox::critical(this, "Input Error", "Invalid or empty MPEG4 on alarm GOP size");
                return false;
            }

            brc = (sdvr_br_control_e) uiComboEncOnAlarmBitRateControlMPEG4->itemData(
                uiComboEncOnAlarmBitRateControlMPEG4->currentIndex()).toInt();

            switch (brc)
            {
            case SDVR_BITRATE_CONTROL_CBR:
            case SDVR_BITRATE_CONTROL_VBR:
                uiLEditEncOnAlarmAvgBitRateMPEG4->text().toUInt(&ok);
                if (!ok)
                {
                    QMessageBox::critical(this, "Input Error",
                        "Invalid or empty MPEG4 on alarm average bit rate");
                    return false;
                }

                if (brc == SDVR_BITRATE_CONTROL_VBR)
                {
                    uiLEditEncOnAlarmMaxBitRateMPEG4->text().toUInt(&ok);
                    if (!ok)
                    {
                        QMessageBox::critical(this, "Input Error",
                            "Invalid or empty MPEG4 on alarm maximum bit rate");
                        return false;
                    }
                }
                break;

            case  SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
                input = uiLEditEncOnAlarmQualityMPEG4->text();
                num = input.toInt(&ok);
                if (input.isEmpty() || !ok || (num > 100))
                {
                   QMessageBox::critical(this, "Input Error", "On alarm quality is required. A number between 0 - 100");
                   return false;
                }

                int pos;
                if (uiLEditEncOnAlarmQualityMPEG4->validator()->validate(input, pos) != QValidator::Acceptable)
                {
                    QMessageBox::critical(this, "Input Error", "Invalid on alarm MPEG4 quality");
                    return false;
                }
                uiLEditEncOnAlarmMaxBitRateMPEG4->text().toUInt(&ok);
                if (!ok)
                {
                    QMessageBox::critical(this, "Input Error",
                        "Invalid or empty MPEG4 on alarm maximum bit rate");
                    return false;
                }

                break;

            default:
                break;
            } //switch (brc)

            break;
        }

        default:
            break;
    } // switch (m_camera->videoFormat(subEnc))

    // Save new the new on Alarm Parameters
    return true;
}

void CameraDialog::onChkBoxDeinterlacingChanged(int state)
{
    sdvr_err_e err;

    err = sdvr_enable_deinterlacing(m_camera->handle(), (state == Qt::Checked));
}
void CameraDialog::onChkBoxNoiseReductionChanged(int state)
{
    sdvr_err_e err;

    err = sdvr_enable_noise_reduction(m_camera->handle(), (state == Qt::Checked));
}

void CameraDialog::onComboCamTerminationActivated(int index)
{
    sdvr_err_e err;
    sdvr_term_e term = (sdvr_term_e)uiComboBoxCamTermination->itemData(index).toInt();

    err = sdvr_set_camera_termination(m_camera->handle(), term);
}

void CameraDialog::onGainModeValueChanged(int nGainModeValue)
{
    uiLabelGainModeValue->setText(QString("%1").arg(nGainModeValue));
    if (m_bLiveUpdateVideoInSettings)
    {
        sdvr_err_e err;

        err = sdvr_set_gain_mode(m_camera->handle(), nGainModeValue);

        // We must indicate the video-in settings was
        // changed, so that if the user cancels the dialogbox
        // we can change them back.
        m_videoInModified = true;
    }

}
void CameraDialog::onVideoLiveUpdateChanged(int state)
{
    m_bLiveUpdateVideoInSettings =  (state == Qt::Checked);

}

void CameraDialog::onBtnResetAll()
{
    // Default Video-in parameters
    uiHSliderHue->setValue(128);
    uiHSliderBrightness->setValue(128);
    uiHSliderContrast->setValue(92);
    uiHSliderSaturation->setValue(128);
    uiHSliderSharpness->setValue(1);

    uiChkBoxDeinterlacing->setChecked(true);
    uiChkBoxNoiseReduction->setChecked(true);
    uiLabelGainModeValue->setText("128");
    uiHSliderGainMode->setValue(128);
    int index = uiComboBoxCamTermination->findData(QVariant(SDVR_TERM_75OHM));
    uiComboBoxCamTermination->setCurrentIndex(index);
}
//--------------------------- CameraSetupDialog ------------------------------

CameraSetupDialog::CameraSetupDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
//todo: for now we only support G711
    G726Radio->setEnabled(false);
    connect(uiGrpPrimaryVideoEncoder, SIGNAL(toggled( bool)), SLOT(onGrpPrimaryVideoEncoderToggled( bool )));
}

void CameraSetupDialog::doInitDialog(Camera *cam)
{
    sdvr_venc_e vEncPrimary = cam->videoFormat(SDVR_ENC_PRIMARY);
    sdvr_venc_e vEncSecondary = cam->videoFormat(SDVR_ENC_SECONDARY);
    sdvr_aenc_e aEnc = cam->audioFormat();

    // Setup the current primary encoder if any
    uiGrpPrimaryVideoEncoder->setChecked(vEncPrimary !=  SDVR_VIDEO_ENC_NONE);
    switch (vEncPrimary)
    {
        case SDVR_VIDEO_ENC_H264:
            uiRadioPrimaryEncH264->setChecked(true);
            break;
        case SDVR_VIDEO_ENC_JPEG:
            uiRadioPrimaryEncJPEG->setChecked(true);
            break;
        case SDVR_VIDEO_ENC_MPEG4:
            uiRadioPrimaryEncMPEG4->setChecked(true);
            break;
        case SDVR_VIDEO_ENC_NONE:
        default:
            uiRadioPrimaryEncH264->setChecked(false);
            uiRadioPrimaryEncJPEG->setChecked(false);
            uiRadioPrimaryEncMPEG4->setChecked(false);
            // If no Primary encoder is specified,
            // we can not have Secondary encoder or Audio encoder.
            onGrpPrimaryVideoEncoderToggled(false);
            return;
    }

    // Setup the current secondary encoder if any
    uiGrpSecondaryVideoEncoder->setChecked(vEncSecondary !=  SDVR_VIDEO_ENC_NONE);
    switch (vEncSecondary)
    {
        case SDVR_VIDEO_ENC_H264:
            uiRadioSecondaryEncH264->setChecked(true);
            break;
        case SDVR_VIDEO_ENC_JPEG:
            uiRadioSecondaryEncJPEG->setChecked(true);
            break;
        case SDVR_VIDEO_ENC_MPEG4:
            uiRadioSecondaryEncMPEG4->setChecked(true);
            break;
        case SDVR_VIDEO_ENC_NONE:
        default:
            uiRadioSecondaryEncH264->setChecked(false);
            uiRadioSecondaryEncJPEG->setChecked(false);
            uiRadioSecondaryEncMPEG4->setChecked(false);
            break;
    }

    // Setup the current audio encoder
    audioGroup->setChecked(aEnc != SDVR_AUDIO_ENC_NONE);
    switch (aEnc)
    {
        case  SDVR_AUDIO_ENC_G711:
            G711Radio->setChecked(true);
        break;

        case SDVR_AUDIO_ENC_G726_16K:
        case SDVR_AUDIO_ENC_G726_24K:
        case SDVR_AUDIO_ENC_G726_32K:
        case SDVR_AUDIO_ENC_G726_48K:
        default:
            G711Radio->setChecked(false);
            break;
    }
}

sdvr_venc_e CameraSetupDialog::videoEncoder(uint subEnc) const
{
    sdvr_venc_e enc = SDVR_VIDEO_ENC_NONE;
    switch (subEnc)
    {
    case SDVR_ENC_PRIMARY:
        if (uiGrpPrimaryVideoEncoder->isChecked())
        {
            if (uiRadioPrimaryEncH264->isChecked())
                enc = SDVR_VIDEO_ENC_H264;
            else if (uiRadioPrimaryEncJPEG->isChecked())
                enc = SDVR_VIDEO_ENC_JPEG;
            else if (uiRadioPrimaryEncMPEG4->isChecked())
                enc =SDVR_VIDEO_ENC_MPEG4;
        }
        break;
    case SDVR_ENC_SECONDARY:
        if (uiGrpSecondaryVideoEncoder->isChecked())
        {
            if (uiRadioSecondaryEncH264->isChecked())
                enc = SDVR_VIDEO_ENC_H264;
            else if (uiRadioSecondaryEncJPEG->isChecked())
                enc = SDVR_VIDEO_ENC_JPEG;
            else if (uiRadioSecondaryEncMPEG4->isChecked())
                enc =SDVR_VIDEO_ENC_MPEG4;
        }
        break;

    default:
        enc = SDVR_VIDEO_ENC_NONE;
        break;
    }

    return enc;
}

sdvr_aenc_e CameraSetupDialog::audioEncoding() const
{
    if (!audioGroup->isChecked())
        return SDVR_AUDIO_ENC_NONE;

    return G711Radio->isChecked() ? SDVR_AUDIO_ENC_G711 : SDVR_AUDIO_ENC_G726_16K;
}

void CameraSetupDialog::onGrpPrimaryVideoEncoderToggled( bool on )
{
    // Audio and secondary encoding are only available if
    // Primary encoder is selected.
    if (!on)
    {
        uiGrpSecondaryVideoEncoder->setChecked(false);
        audioGroup->setChecked(false);
    }
    uiGrpSecondaryVideoEncoder->setEnabled(on);
    audioGroup->setEnabled(on);
}
