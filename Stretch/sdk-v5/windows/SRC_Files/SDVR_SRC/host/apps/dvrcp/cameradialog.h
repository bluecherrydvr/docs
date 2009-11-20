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

#ifndef CAMERADIALOG_H
#define CAMERADIALOG_H

#include "camera.h"
#include "ui_cameradialog.h"
#include "ui_camerasetupdialog.h"
#include "ui_regionsdialog.h"
#include "sdvr_sdk.h"


class Camera;


class CameraSetupDialog : public QDialog, private Ui::CameraSetupDialog
{
    Q_OBJECT

public:
    CameraSetupDialog(QWidget *parent = 0);

    sdvr_venc_e videoEncoder(uint subEnc) const;
    sdvr_aenc_e audioEncoding() const;
    void doInitDialog(Camera *camera);

private slots:
    void onGrpPrimaryVideoEncoderToggled( bool on );

};


class CameraDialog : public QDialog, private Ui::CameraDialog
{
    Q_OBJECT

public:
    CameraDialog(Camera *camera, QWidget *parent = 0);

    bool isYUVFrameRateModified() const { return m_yuvFrameRateModified; }
    bool isMotionFrequencyModified() const {return m_MotionFrequencyModified;}
    bool isNameModified() const { return m_nameModified; }
    bool isOsdModified() const  { return m_osdModified; }
    bool isMDModified() const { return m_MDModified; }
    bool isBDModified() const { return m_BDModified; }
    bool isNDModified() const { return m_NDModified; }
    bool isPRModified() const { return m_PRModified; }

    bool isMDRegionsModified() const { return m_MDRegionsModified; }
    bool isBDRegionsModified() const { return m_BDRegionsModified; }
    bool isNDRegionsModified() const { return m_NDRegionsModified; }
    bool isPRRegionsModified() const { return m_PRRegionsModified; }

    bool isVideoInModified() const { return m_videoInModified; }
    bool isDeinterlacingModified() const {return m_bDeinterlacingModified; }
    bool isNoiseReductionModified() const {return m_bNoiseReductionModified; }
    bool isGainModeModified() const {return m_bGainModeModified; }
    bool isCamTerminationModifed() const {return m_bCamTerminationModifed; }

    bool isVideoEncodingModified(uint subEnc) const { return m_videoEncModified[subEnc]; }
    bool isOnAlarmVideoEncodingModified(uint subEnc) const { return m_onAlarmVideoEncModified[subEnc]; }

private slots:
    void onBrcComboActivated(int index);
    void onAlarmBrcComboActivated(int index);
    void onComboBitRateControlMPEG4Activated(int index);
    void onComboEncOnAlarmBitRateControlMPEG4Activated(int index);
    void onAdjustEncodingOnAlarmChanged(int state);
    void onBtnGrpSubEncoderClicked(int subEnc);
    void onBtnPrivacyRegions();
    void onBtnMDRegions();
    void onBtnBDRegions();
    void onTranslucentValueChanged(int value);
    void onHueValueChanged(int value); 
    void onBrightnessValueChanged(int value); 
    void onContrastValueChanged(int value);  
    void onSaturationValueChanged(int value);
    void onSharpnessValueChanged(int nValue);
    void onGainModeValueChanged(int nGainModeValue);

    void onVideoLiveUpdateChanged(int state);
    void onChkBoxDeinterlacingChanged(int state);
    void onChkBoxNoiseReductionChanged(int state);
    void onComboCamTerminationActivated(int index);

    void onOSDItemChanged(int value);
    void onComboOSDLocActivated(int index);
    void onComboMBThresholds(int index);
    void onBtnResetAll();
    virtual void accept();


private:
    void enableTab();
    void initAllControls();
    void initGeneralTabCntrl();
    void initAlarmsTabCntrl();
    void initEncodingTabCntrl();
    void initEncodingTabData();
    void initEncodingOnAlarmTabCntrl();
    void initEncodingOnAlarmTabData();
    void initOSDInfo();
    void initVideoInTabCntrl();

    bool ValidateGeneralTab();
    bool ValidateAlarmsTabs();
    bool ValidateEncoderTab(uint SubEnc);
    bool ValidateOnAlarmEncoderTab(uint SubEnc);

    bool SaveGeneralTab();
    void SaveTempCurOSDInfo(int osdItem);
    bool SaveEncoderTab(uint SubEnc);
    bool SaveAlarmsTabs();
    bool SaveOnAlarmEncoderTab(uint SubEnc);
    bool SaveVideoInSettingsTab();

private:
    Camera *m_camera;

    // Following indecies correspond to the tab order in the
    // Camera Settings dialogbox. They need to be changed
    // in the event the order of tabs were ever changed.
	static const int TAB_NDX_GENERAL            = 0;
	static const int TAB_NDX_ENCODING           = 1;
	static const int TAB_NDX_ALARM              = 2;
	static const int TAB_NDX_ENC_ON_ALARM       = 3;
    static const int TAB_NDX_VIDEO_IN_SETTINGS  = 4;
	static const int TAB_NDX_AUDIO_ENCODER      = 5;

    bool m_nameModified;
    bool m_yuvFrameRateModified;
    bool m_MotionFrequencyModified;
    bool m_osdModified;
    bool m_MDModified;
    bool m_BDModified;
    bool m_NDModified;
    bool m_PRModified;
    bool m_MDRegionsModified;
    bool m_BDRegionsModified;
    bool m_NDRegionsModified;
    bool m_PRRegionsModified;
    bool m_videoEncModified[MAX_SUB_ENCODERS];
    bool m_audioEncModified;
    bool m_onAlarmVideoEncModified[MAX_SUB_ENCODERS];
    bool m_videoInModified;
    bool m_bDeinterlacingModified;
    bool m_bNoiseReductionModified;
    bool m_bGainModeModified;
    bool m_bCamTerminationModifed;

    bool m_bLiveUpdateVideoInSettings;

    int m_curSubEnc;
    QButtonGroup *m_btnGrpSubEncoder;
    sx_uint8 m_FWVersionMajor;
    sx_uint8 m_FWVersionMinor;
    int m_curOSDItem;
    int m_lastThresholdIndex;

    CRegion m_origPRRegions;

    bool m_osdShow[SDVR_MAX_OSD];
    QString m_osdText[SDVR_MAX_OSD];
    sdvr_location_e m_osdLoc[SDVR_MAX_OSD];
    bool m_osdDtsEnabled[SDVR_MAX_OSD];
    sdvr_dts_style_e m_osdDtsFormat[SDVR_MAX_OSD];
    sx_uint16 m_osdLocx[SDVR_MAX_OSD];
    sx_uint16 m_osdLocy[SDVR_MAX_OSD];
    sx_uint8 m_osdTranslucent[SDVR_MAX_OSD];
    sx_uint8 m_osdBrightness[SDVR_MAX_OSD];

};

#endif
