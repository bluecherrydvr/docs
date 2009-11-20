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
#include <QVariant>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include "viewsettings.h"
#include "camera.h"
#include "dvrsystem.h"
#include "mainwindow.h"
#include "recorder.h"
#include "sdvr.h"
#include "log.h"
#include "videopanel.h"
#include "dvrboard.h"



extern DVRSystem *g_DVR;

static const char *TN_CAMERA = "camera";
static const char *TN_CAMID = "id";
static const char *TN_CAMNAME = "name";
static const char *TN_VPPMODE = "vppmode";

static const char *TN_ATTACH = "attach";
static const char *TN_BOARD = "board";
static const char *TN_INPUT = "input";

static const char *TN_YUVFRAMERATE = "yuvframerate";

static const char *TN_VIDEOENC = "videoencoding";
static const char *TN_FORMAT = "format";
static const char *TN_SUBENC = "subenc";
static const char *TN_DEFAULT = "default";
static const char *TN_FRAMERATE = "framerate";
static const char *TN_DECIMATION = "decimation";
static const char *TN_MOTION_VALUE_FREQ = "motion_value_freq";

// Encoder types
static const char *TN_H264 = "h264";
static const char *TN_JPEG = "jpeg";
static const char *TN_MPEG4 = "mpeg4";

// Encoder parameters
static const char *TN_ENCODER_GOPSIZE = "gopsize";
static const char *TN_ENCODER_BRC = "bitratecontrol";
static const char *TN_ENCODER_AVGBR = "avgbitrate";
static const char *TN_ENCODER_MAXBR = "maxbitrate";
static const char *TN_ENCODER_QUALITY = "quality";

static const char *TN_ONALARM = "onalarm";
static const char *TN_ONALARMENABLED = "enabled";
static const char *TN_MINONTIME = "minontime";
static const char *TN_MINOFFTIME = "minofftime";

static const char *TN_AUDIOENC = "audioencoding";

static const char *TN_OSD = "osd";
static const char *TN_OSDID = "id";
static const char *TN_OSDENABLED = "enabled";
static const char *TN_OSDTEXT = "text";
static const char *TN_OSDLOC = "location";
static const char *TN_OSDDTSENABLED = "dtsenabled";
static const char *TN_OSDDTSFORMAT = "dtsformat";
static const char *TC_OSDTRANSLUCENT = "translucent";
static const char *TC_OSDLOCX = "x_topleft";
static const char *TC_OSDLOCY = "y_topleft";
static const char *TC_OSDSHOW = "show";

static const char *TN_VPARAMS = "video_in_params";
static const char *TC_VPARAMS_HUE = "hue";
static const char *TC_VPARAMS_CONTRASTS = "contrast";
static const char *TC_VPARAMS_SATURATION = "saturation";
static const char *TC_VPARAMS_BRIGHTNESS = "brightness";
static const char *TC_VPARAMS_SHARPNESS = "sharpness";
static const char *TC_VPARAMS_DEINTERLACING = "deinterlacing";
static const char *TC_VPARAMS_NOISEREDUCTION = "noise_reduction";
static const char *TC_VPARAMS_GAINMODE = "gain_mode";
static const char *TC_VPARAMS_CAM_TERMINATION = "cam_termination";

static const char *TN_MD = "motiondetection";
static const char *TN_BD = "blinddetection";
static const char *TN_ND = "nightdetection";
static const char *TN_ENABLED = "enabled";
static const char *TN_THRESHOLD = "threshold";
static const char *TN_THRESHOLD1 = "threshold1";
static const char *TN_THRESHOLD2 = "threshold2";
static const char *TN_THRESHOLD3 = "threshold3";

static const char *TN_REGION = "region";
static const char *AN_REGLAYERS = "layers";
static const char *AN_REGVIDEOSTD = "videostandard";

static const char *TN_PRIVACY = "privacy";


Camera::Camera()
{
    m_id = 0;
    m_boardIndex = -1;
    m_input = -1;
    m_handle = 0;
    setMotionValueFreq(0);
    sdvr_video_res_decimation_e res;

    // In DVRCP we are defaulting to Stretch-lateral-filter mode.
    // But the user can change this to analytics from the config file
    // until we add this field in the user interface.
    setVPPMode(SDVR_VPP_MODE_SLATERAL);

    setAudioFormat(SDVR_AUDIO_ENC_NONE);

    int stdFrameRate = sdvr::frameRate(g_DVR->videoStandard());

    setYUVFrameRate(stdFrameRate);
    for (int subEnc = 0; subEnc < MAX_SUB_ENCODERS; subEnc++)
    {
        setVideoFormat(subEnc, SDVR_VIDEO_ENC_NONE);

        setFrameRate(subEnc,stdFrameRate);
        if (subEnc == SDVR_ENC_PRIMARY)
            res = SDVR_VIDEO_RES_DECIMATION_EQUAL;
        else
            res = SDVR_VIDEO_RES_DECIMATION_QCIF;

        setDecimation(subEnc,SDVR_VIDEO_RES_DECIMATION_EQUAL);

        setH264GopSize(subEnc,15);
        setH264BitrateControl(subEnc,SDVR_BITRATE_CONTROL_CBR);
        setH264AvgBitrate(subEnc,2000);
        setH264MaxBitrate(subEnc,4000);
        setH264Quality(subEnc,50);

        setJpegImageStyle(subEnc,false);
        setJpegQuality(subEnc,50);

        setMPEG4GopSize(subEnc,15);
        setMPEG4BitrateControl(subEnc,SDVR_BITRATE_CONTROL_CBR);
        setMPEG4AvgBitrate(subEnc,2000);
        setMPEG4MaxBitrate(subEnc,4000);
        setMPEG4Quality(subEnc,3);

        setAdjustEncodingOnAlarmEnabled(subEnc,false);
        setOnAlarmFrameRate(subEnc,stdFrameRate);
        setOnAlarmMinOnTime(subEnc,60);
        setOnAlarmMinOffTime(subEnc,30);

        setOnAlarmH264GopSize(subEnc,15);
        setOnAlarmH264BitrateControl(subEnc,SDVR_BITRATE_CONTROL_CBR);
        setOnAlarmH264AvgBitrate(subEnc,2000);
        setOnAlarmH264MaxBitrate(subEnc,4000);
        setOnAlarmH264Quality(subEnc,50);
        setonAlarmJpegQuality(subEnc,50);

        setOnAlarmMPEG4GopSize(subEnc,15);
        setOnAlarmMPEG4BitrateControl(subEnc,SDVR_BITRATE_CONTROL_CBR);
        setOnAlarmMPEG4AvgBitrate(subEnc,2000);
        setOnAlarmMPEG4MaxBitrate(subEnc,4000);
        setOnAlarmMPEG4Quality(subEnc,50);

        m_recording[subEnc] = false;
        m_lastStatTimeStamp[subEnc] = 0;
        m_firstStatTimeStamp[subEnc] = 0;
        m_dropFrameCount[subEnc] = 0;
        m_lastAudioTimeStamp = 0;
        m_lastAudioSize = 0;
        m_AccuAudioDelta = 0;
    }

    setOsdEnabled(true);
    for (int osdItem = 0; osdItem < SDVR_MAX_OSD; osdItem++)
    {
        setOsdLocation(osdItem, SDVR_LOC_BOTTOM_RIGHT);
        setOsdDtsEnabled(osdItem, false);
        setOsdDtsFormat(osdItem, SDVR_OSD_DTS_MDY_12H);
        setOsdCustomLoc(osdItem, 0, 0);
        setOsdShow(osdItem, false);
        setOsdTranslucent(osdItem, 180);
    }


    setMDEnabled(false);
    for (int Nth = 0; Nth < MAX_NLAYERS; Nth++)
        setMDThreshold(Nth,20);

    setBDEnabled(false);
    setBDThreshold(60);

    setNDEnabled(false);
    setNDThreshold(40);

    setPREnabled(false);

    setMDTriggered(false);
    setBDTriggered(false);
    setNDTriggered(false);
    setVLTriggered(false);
    setVDTriggered(false);

    // Default Video-in parameters
    setVParamHue(128);
    setVParamContrast(92);
    setVParamSaturation(128);
    setVParamBrightness(128);
    setVParamSharpness(1);

    setVParamsDeinterlacing(true);
    setVParamNoiseReduction(true);
    setVParamGainMode(128);
    setVParamCamTermination(SDVR_TERM_75OHM);

    m_rawVideoEnabled = false;
    m_smoGridEnabled = false;
    m_rawAudioEnabled = false;
}
Camera::~Camera()
{
}

Camera::Camera(Camera &other, bool bDuplicate)
{
    *this = other;
    // In case of duplcate we make sure
    // the two camera are identical
    if (bDuplicate)
    {
        m_handle = other.handle();
        attach(other.boardIndex(), other.input());
    }
}

Camera & Camera::operator=(Camera & other)
{
    if (this == &other)
        return *this;

    setId(other.id());
    setName(other.name());
    setYUVFrameRate(other.getYUVFrameRate());
    setMotionValueFreq(other.getMotionValueFreq());
    setAudioFormat(other.audioFormat());
    for (int subEnc = 0; subEnc < MAX_SUB_ENCODERS; subEnc++)
    {
        setVideoFormat(subEnc,other.videoFormat(subEnc));
        setDecimation(subEnc,other.decimation(subEnc));
        setFrameRate(subEnc,other.frameRate(subEnc));

        setH264GopSize(subEnc,other.h264GopSize(subEnc));
        setH264BitrateControl(subEnc,other.h264BitrateControl(subEnc));
        setH264AvgBitrate(subEnc,other.h264AvgBitrate(subEnc));
        setH264MaxBitrate(subEnc,other.h264MaxBitrate(subEnc));
        setH264Quality(subEnc,other.h264Quality(subEnc));
        setJpegImageStyle(subEnc,other.jpegIsImageStyle(subEnc));
        setJpegQuality(subEnc,other.jpegQuality(subEnc));
        setMPEG4GopSize(subEnc,other.MPEG4GopSize(subEnc));
        setMPEG4BitrateControl(subEnc,other.MPEG4BitrateControl(subEnc));
        setMPEG4AvgBitrate(subEnc,other.MPEG4AvgBitrate(subEnc));
        setMPEG4MaxBitrate(subEnc,other.MPEG4MaxBitrate(subEnc));
        setMPEG4Quality(subEnc,other.MPEG4Quality(subEnc));

        setAdjustEncodingOnAlarmEnabled(subEnc,other.isAdjustEncodingOnAlarmEnabled(subEnc));
        setOnAlarmFrameRate(subEnc,other.onAlarmFrameRate(subEnc));
        setOnAlarmMinOnTime(subEnc,other.onAlarmMinOnTime(subEnc));
        setOnAlarmMinOffTime(subEnc,other.onAlarmMinOffTime(subEnc));

        setOnAlarmH264GopSize(subEnc,other.onAlarmH264GopSize(subEnc));
        setOnAlarmH264BitrateControl(subEnc,other.onAlarmH264BitrateControl(subEnc));
        setOnAlarmH264AvgBitrate(subEnc,other.onAlarmH264AvgBitrate(subEnc));
        setOnAlarmH264MaxBitrate(subEnc,other.onAlarmH264MaxBitrate(subEnc));
        setOnAlarmH264Quality(subEnc,other.onAlarmH264Quality(subEnc));
        setonAlarmJpegQuality(subEnc,other.getOnAlarmJpegQuality(subEnc));
        setOnAlarmMPEG4GopSize(subEnc,other.onAlarmH264GopSize(subEnc));
        setOnAlarmMPEG4BitrateControl(subEnc,other.onAlarmH264BitrateControl(subEnc));
        setOnAlarmMPEG4AvgBitrate(subEnc,other.onAlarmH264AvgBitrate(subEnc));
        setOnAlarmMPEG4MaxBitrate(subEnc,other.onAlarmH264MaxBitrate(subEnc));
        setOnAlarmMPEG4Quality(subEnc,other.onAlarmH264Quality(subEnc));

        m_recording[subEnc] = other.m_recording[subEnc];

    }

    // save the osd data for all the osd items
    setOsdEnabled(other.isOsdEnabled());

    int osdMax = 1;
    sx_uint16 x,y;
    sdvr_firmware_ver_t  version;
    g_DVR->getFWVersion(&version);

    // The following OSD functionality only exist in 3.2.x.x or higher of firmware
    if (sdvr::isMinVersion(version.fw_major,version.fw_minor, 0 , 0, 3, 2, 0, 0))
        osdMax = SDVR_MAX_OSD;
    for (int osdItem = 0; osdItem < osdMax; osdItem++)
    {
        setOsdText(osdItem, other.osdText(osdItem));
        setOsdLocation(osdItem, other.osdLocation(osdItem));
        setOsdDtsEnabled(osdItem, other.isOsdDtsEnabled(osdItem));
        setOsdDtsFormat(osdItem, other.osdDtsFormat(osdItem));
        other.getOsdCustomLoc(osdItem, &x, &y);
        setOsdCustomLoc(osdItem, x, y);
        setOsdTranslucent(osdItem, other.getOsdTranslucent(osdItem));
        setOsdShow(osdItem, other.isOsdShow(osdItem));
    }

    setMDEnabled(other.isMDEnabled());

    for (int Nth = 0; Nth < MAX_NLAYERS; Nth++)
        setMDThreshold(Nth,other.MDThreshold(Nth));

    setMDRegion(other.MDRegion());

    setBDEnabled(other.isBDEnabled());
    setBDThreshold(other.BDThreshold());
    setBDRegion(other.BDRegion());

	setPREnabled(other.isPREnabled());
    setPRRegion(other.PRRegion());

    setNDEnabled(other.isNDEnabled());
    setNDThreshold(other.NDThreshold());

    setVParamHue(other.getVParamHue());
    setVParamContrast(other.getVParamContrast());
    setVParamSaturation(other.getVParamSaturation());
    setVParamBrightness(other.getVParamBrightness());
    setVParamSharpness(other.getVParamSharpness());

    setVParamsDeinterlacing(other.isVParamsDeinterlacing());
    setVParamNoiseReduction(other.isVParamNoiseReduction());
    setVParamGainMode(other.getVParamGainMode());
    setVParamCamTermination(other.getVParamCamTermination());

    m_rawVideoEnabled = other.m_rawVideoEnabled;
    m_smoGridEnabled = other.m_smoGridEnabled;

    return *this;
}

bool Camera::attach(int boardIndex, int input)
{
    if (isAttached())
        return false;

    m_boardIndex = boardIndex;
    m_input = input;
    return true;
}

bool Camera::isAttached() const
{
    return m_boardIndex != -1 && m_input != -1;
}

bool Camera::detach()
{
    if (isAttached())
    {
        m_boardIndex = -1;
        m_handle = 0;
        m_input = -1;
        return true;
    }

    return false;
}

sdvr_err_e Camera::applyAllParameters()
{
    sdvr_err_e rc;
    sdvr_firmware_ver_t  version;
    g_DVR->getFWVersion(&version);

    if (videoFormat(SDVR_ENC_PRIMARY) != SDVR_VIDEO_ENC_NONE)
    {
        g_MainWindow->setStatusBar(QString("Setting the primary encoder parameters for %1...").arg(name()));

        rc = applyVideoEncoding(SDVR_ENC_PRIMARY);
        Log::chk_err(QString("%1: Failed to set the primary encoder parameters.").arg(name()),rc, true, m_boardIndex);

        if (rc == SDVR_DRV_ERR_MSG_RECV)
            return rc;

        g_MainWindow->setStatusBar(QString("Setting the alarm recording parameters of primary encoder for %1...").arg(name()));

        rc = applyOnAlarmVideoEncoding(SDVR_ENC_PRIMARY);
        Log::chk_err(QString("%1: Failed to set the alarm recording parameters for primary encoder.").arg(name()),rc, true,m_boardIndex);

        if (rc == SDVR_DRV_ERR_MSG_RECV)
            return rc;
    }

    if (videoFormat(SDVR_ENC_SECONDARY) != SDVR_VIDEO_ENC_NONE)
    {
        g_MainWindow->setStatusBar(QString("Setting the secondary encoder parameters for %1...").arg(name()));

        rc = applyVideoEncoding(SDVR_ENC_SECONDARY);
        Log::chk_err(QString("%1: Failed to set the secondary encoder parameters.").arg(name()),
            rc, true,m_boardIndex);

        if (rc == SDVR_DRV_ERR_MSG_RECV)
            return rc;

        g_MainWindow->setStatusBar(QString("Setting the alarm recording parameters of secondary encoder for %1...").arg(name()));

        rc = applyOnAlarmVideoEncoding(SDVR_ENC_SECONDARY);
        Log::chk_err(QString("%1: Failed to set the alarm recording parameters for secondary encoder.").arg(name()),
                    rc, true,m_boardIndex);

        if (rc == SDVR_DRV_ERR_MSG_RECV)
            return rc;

    }

    g_MainWindow->setStatusBar(QString("Setting the OSD parameters for %1...").arg(name()));

    // In Analytics we don't support new style OSD, in order
    // to reduce confussion, OSD is turned off.
    if (getVPPMode() == SDVR_VPP_MODE_ANALYTICS)
    {
        if (isOsdEnabled())
        {
            Log::info(QString("%1: VPP mode is set to analytics. Turning off OSD.").arg(name()));
            setOsdEnabled(false);
        }
    }
    rc = applyOsd();
    Log::chk_err(QString("%1: Failed to set the OSD parameters.").arg(name()),
        rc, true,m_boardIndex);

    if (rc == SDVR_DRV_ERR_MSG_RECV)
        return rc;

    g_MainWindow->setStatusBar(QString("Setting the Motion Detection parameters for %1...").arg(name()));
    rc = applyMD();
    Log::chk_err(QString("%1: Failed to set the Motion Detection parameters.").arg(name()),
        rc, true, m_boardIndex);

    if (rc == SDVR_DRV_ERR_MSG_RECV)
        return rc;

    g_MainWindow->setStatusBar(QString("Setting the Motion Detection regions for %1...").arg(name()));
    rc = applyMDRegions();
    Log::chk_err(QString("%1: Failed to set the Motion Detection regions.").arg(name()),
        rc, true, m_boardIndex);

    if (rc == SDVR_DRV_ERR_MSG_RECV)
        return rc;

    g_MainWindow->setStatusBar(QString("Setting the Blind Detection parameters for %1...").arg(name()));
    rc = applyBD();
    Log::chk_err(QString("%1: Failed to set the Blindg Detection parameters.").arg(name()),
        rc, true,m_boardIndex);

    if (rc == SDVR_DRV_ERR_MSG_RECV)
        return rc;

    g_MainWindow->setStatusBar(QString("Setting the Blind Detection regions for %1...").arg(name()));
    rc = applyBDRegions();
    Log::chk_err(QString("%1: Failed to set the Blindg Detection regions.").arg(name()),
        rc, true,m_boardIndex);

    if (rc == SDVR_DRV_ERR_MSG_RECV)
        return rc;

    g_MainWindow->setStatusBar(QString("Setting the Privacy Blocking parameters for %1...").arg(name()));
    rc = applyPR();
    Log::chk_err(QString("%1: Failed to set the Privacy Blocking parameters.").arg(name()),
        rc, true,m_boardIndex);

    if (rc == SDVR_DRV_ERR_MSG_RECV)
        return rc;

    g_MainWindow->setStatusBar(QString("Setting the Privacy Blocking regions for %1...").arg(name()));
    rc = applyPRRegions();
    Log::chk_err(QString("%1: Failed to set the Privacy Blocking regions.").arg(name()),
        rc, true,m_boardIndex);

    if (rc == SDVR_DRV_ERR_MSG_RECV)
        return rc;

    g_MainWindow->setStatusBar(QString("Setting the Night Detection parameters for %1...").arg(name()));
    rc = applyND();
    Log::chk_err(QString("%1: Failed to set the Night Detection parameters.").arg(name()),
        rc, true, m_boardIndex);

    if (rc == SDVR_DRV_ERR_MSG_RECV)
        return rc;

    // video-in parameter change is only supported in 4.0.0.0 or higher of firmware
    if (sdvr::isMinVersion(version.fw_major, 0 , 0 , 0, 4, 0, 0, 0))
    {
        g_MainWindow->setStatusBar(QString("Setting the Video-In Image parameters for %1...").arg(name()));
        rc = applyVideoInSettings();
        Log::chk_err(QString("%1: Failed to set the Video-In Image parameters.").arg(name()),
            rc, true, m_boardIndex);
    }
    return rc;
}

bool Camera::openChannel()
{
    QString qstrMsg;
    DVRBoard *board;
    if (!isAttached() || isChannelOpen())
        return false;

    qstrMsg = QString("Creating %1...").arg(name());
    g_MainWindow->setStatusBar(qstrMsg);

    sdvr_err_e rc = SDVR_DRV_ERR_MSG_RECV;

    if (!g_MainWindow->isFirmwareTimeout(m_boardIndex))
    {
        sdvr_chan_def_t chan_def;
        chan_def.board_index = boardIndex();
        chan_def.chan_num = input();
        chan_def.chan_type = SDVR_CHAN_TYPE_ENCODER;

        // Re-set the video encoders to NONE if the DVR board
        // does not support encoding.
        // In the future we should get this information per board
        board = g_DVR->board(boardIndex());
        if (board->encodersCount() == 0)
        {
            setVideoFormat(SDVR_ENC_PRIMARY,SDVR_VIDEO_ENC_NONE);
            setVideoFormat(SDVR_ENC_SECONDARY,SDVR_VIDEO_ENC_NONE);
        }

        chan_def.video_format_primary = videoFormat(SDVR_ENC_PRIMARY);
        chan_def.video_format_secondary = videoFormat(SDVR_ENC_SECONDARY);

        chan_def.audio_format = audioFormat();
        chan_def.vpp_mode = getVPPMode();  // SDVR_VPP_MODE_SLATERAL;
        rc = sdvr_create_chan(&chan_def, &m_handle);
    }
    if (rc != SDVR_ERR_NONE)
    {
        if (rc == SDVR_DRV_ERR_MSG_RECV)
            g_MainWindow->setFirmwareTimeout(m_boardIndex);

        Log::error(QString("%1: Failed to create camera. Channel = %2  Board Index = %3. Err = %5 [%4]")
                   .arg(name()).arg(input()).arg(boardIndex() + 1).arg(sdvr_get_error_text(rc)).arg(rc));
        return false;
    }
    if (rc != SDVR_ERR_NONE)
        m_handle = 0;

    return rc == SDVR_ERR_NONE;
}

bool Camera::closeChannel()
{
    QString qstrPrompt;
    if (isChannelOpen())
    {
        sdvr_err_e rc = SDVR_DRV_ERR_MSG_RECV;

        for (unsigned int subEnc = SDVR_ENC_PRIMARY; subEnc < MAX_SUB_ENCODERS; subEnc++)
        {
            if (isRecording(subEnc))
                stopRecording(subEnc);
        }
        // Disable the SMO if it was enabled so that we can close the channel.
        if (isSmoGridEnabled())
            setSmoGrid(0,0,SDVR_VIDEO_RES_DECIMATION_NONE,0,false);

        // Disable the HMO if it was enabled so that we can close the channel.
        if (isRawVideoEnabled())
            setRawVideo(SDVR_VIDEO_RES_DECIMATION_NONE,false);

        // Disable the raw audio if it was enabled so that we can close the channel.
        if (isRawAudioEnabled())
            setRawAudio(false);

        g_MainWindow->setStatusBar(QString("Destroying %1...").arg(name()));

        rc = sdvr_destroy_chan(m_handle);

        qstrPrompt = QString("%1: Failed to destroy camera.").arg(name());
        Log::chk_err(qstrPrompt, rc, true,m_boardIndex);

        m_handle = 0;
        return rc == SDVR_ERR_NONE;
    }

    return false;
}

sdvr_err_e Camera::setRawVideo(sdvr_video_res_decimation_e dec, bool enabled)
{
    sdvr_err_e err = SDVR_DRV_ERR_MSG_RECV;
    QString qstrPrompt;

    if (enabled)
        g_MainWindow->setStatusBar(QString("%1: Starting HMO...").arg(name()));
    else
        g_MainWindow->setStatusBar(QString("%1: Stopping HMO...").arg(name()));


    err = sdvr_stream_raw_video(m_handle, dec, getYUVFrameRate(), enabled);
    if (err == SDVR_ERR_NONE)
    {
        m_rawVideoEnabled = enabled;
    }

    // Close the YUV file if we are saving the YUV buffer
    // for this camera and we are stopping the raw video streaming
    if ((enabled == false) && g_MainWindow->getyuvSaveHandle() &&
        (g_MainWindow->getyuvSaveHandle() == m_handle))
        g_MainWindow->closeYUVFile();


    // If the firmware timeout, force the channel to be disabled.
    if (err == SDVR_DRV_ERR_MSG_RECV)
    {
        m_rawVideoEnabled = false;
    }

    qstrPrompt = QString("%1: Failed to %2 raw video.").arg(m_name).arg(enabled ? "enable" : "disable");
    Log::chk_err(qstrPrompt,err, true, m_boardIndex);

    return err;
}

sdvr_err_e Camera::setRawAudio(bool enabled)
{
	sdvr_err_e err = SDVR_DRV_ERR_MSG_RECV;

	if (enabled)
        g_MainWindow->setStatusBar(QString("%1: Starting raw audio streaming...").arg(name()));
	else
        g_MainWindow->setStatusBar(QString("%1: Stopping raw audio streaming...").arg(name()));


    if (!g_MainWindow->isFirmwareTimeout(m_boardIndex))
        err = sdvr_stream_raw_audio(m_handle, enabled);
    if (err == SDVR_ERR_NONE)
    {
    	m_rawAudioEnabled = enabled;
    }

    // Close the raw audio file if we are saving the raw audio buffer
    // for this Camera and we are stopping the raw audio streaming
    if ((enabled == false) && g_MainWindow->getRawAudioSaveHandle() &&
        (g_MainWindow->getRawAudioSaveHandle() == m_handle))
        g_MainWindow->closeRawAudioFile();


    // If the firmware timeout, force the channel to be disabled.
    if (err == SDVR_DRV_ERR_MSG_RECV) {
        m_rawAudioEnabled = false;
    }

    Log::chk_err(QString("%1: Failed to %2 raw audio streaming.").arg(name()).arg(enabled ? "start" : "stop"),err, true, m_boardIndex);

    return err;
}

bool Camera::isRawAudioEnabled() 
{ 
    return m_rawAudioEnabled; 
}

bool Camera::isAVStreaming()
{
    bool ret;
    
    ret = isRawVideoEnabled() || isSmoGridEnabled() ||
          isRecording(SDVR_ENC_PRIMARY) || isRecording(SDVR_ENC_SECONDARY) ||
          isRawAudioEnabled();

    return ret;

}


sdvr_err_e Camera::setSmoGrid(int x, int y, sdvr_video_res_decimation_e dec,
                              int dwellTime, bool enabled)
{
    sdvr_smo_grid_t smo_grid;
    // make sure x,y are on even boundaries
    smo_grid.top_left_mb_x = (x / 2) * 2;
    smo_grid.top_left_mb_y = (y / 2) * 2;
    smo_grid.res_decimation = dec;
    smo_grid.dwell_time = dwellTime;
    smo_grid.enable = enabled;

    sdvr_err_e err = sdvr_set_smo_grid(m_handle, &smo_grid);
    if (err == SDVR_ERR_NONE)
    {
        m_smoGridEnabled = enabled;
    }
    Log::chk_err(QString("%1: Failed to set SMO grid.").arg(name()),
        err, true,m_boardIndex);

    return err;
}
// recordFilePath()
//      This method returns the recording file format for the
//      current camera depends on its video encoder format.
//      It also returns the YUV file name for the camera if
//      it asked for.
//
// Parameters:
//
//      bAppendTimeDate - If true, appends the current time and
//                        date to the file name.
//      bYUVFileFormat - Indicates whether to return the encoded
//                       file name if false or YUV if set to true.
//                       This parameter is defaulted to false.
//
QString Camera::recordFilePath(uint subEnc, bool bAppendTimeDate, bool bYUVFileFormat) const
{
    QString qstrPath;
    QString qstrTempPath;
    const char *ext = "yuv";
    if (!bYUVFileFormat)
    {
        switch (videoFormat(subEnc))
        {
        case SDVR_VIDEO_ENC_H264:
#ifdef elementary_stream
            ext = "h264";
#else
            ext = "mov";
#endif
            break;
        case SDVR_VIDEO_ENC_MPEG4:
#ifdef elementary_stream
            ext = "mp4";
#else
            ext = "mov";
#endif
            break;

        case SDVR_VIDEO_ENC_JPEG:
            if (jpegIsImageStyle(subEnc))
                ext = "jpg";
            else
            {
                if (g_DVR->isMJPEGRecStyleAVI())
                    ext = "avi";
                else
                    ext = "mov";
            }
            break;

        case SDVR_VIDEO_ENC_NONE:
        default:
            ext = "???";

        }
        QString qstrCurDateTime = "";
        if (bAppendTimeDate)
        {
            qstrCurDateTime = QDateTime::currentDateTime().toString(" yy_MM_dd hh_mm_ss");
            qstrTempPath = QString("%1/%1%2").arg(name()).arg(qstrCurDateTime);
        }
        else
        {
            qstrTempPath = QString("%1").arg(name());
        }
        // We append "-2" to the file for the secondary encoder.
        qstrPath = QString("%1/%2%3.%4").arg(g_DVR->systemDirPath())
            .arg(qstrTempPath).arg(subEnc == SDVR_ENC_PRIMARY ? "" : "-2").arg(ext);
        return qstrPath;

    }
    else
    {

        QSize size = g_MainWindow->videoPanel()->getDecimationSize();
        int width = size.width();
        int height = size.height();

        return QString("%1/%2_%3x%4.%5").arg(g_DVR->systemDirPath())
            .arg(name()).arg(width).arg(height).arg(ext);

    }
}

bool Camera::startRecording(uint subEnc)
{
    sdvr_err_e err = SDVR_DRV_ERR_MSG_RECV;
    QString qstrPrompt;
    QString qstrFileName;

    if (videoFormat(subEnc) == SDVR_VIDEO_ENC_NONE ||
        !isChannelOpen() || isRecording(subEnc))
        return false;


    qstrFileName = recordFilePath(subEnc,g_DVR->isSegmentedRecording());
    QByteArray baFile(qstrFileName.toAscii());

//todo mxa: must remove JPEG special handling once avmux supports
//      saving of JPEG
    if (g_DVR->isUseSDKRecording())
    {
        switch (videoFormat(subEnc))
        {
        case SDVR_VIDEO_ENC_H264:
        case SDVR_VIDEO_ENC_MPEG4:
            if (sdvr_start_recording(handle(), (sdvr_sub_encoders_e)subEnc, baFile.data()) != SDVR_ERR_NONE)
                return false;
            break;
        case SDVR_VIDEO_ENC_JPEG:

            if (!g_DVR->recorder().startRecording(subEnc, this, qstrFileName))
                return false;
            break;
        default:
            return false;
        }
    }
    else
    {
        QString qstrPath;

        if (g_DVR->isSegmentedRecording())
        {
            qstrPath = QString("%1/%2").arg(g_DVR->systemDirPath()).arg(name());

	        QDir dir(qstrPath);

            if (!dir.exists())
	        {
		        dir.mkpath(qstrPath);
            }
        }

        if (!g_DVR->recorder().startRecording(subEnc, this, qstrFileName))
            return false;

    }
    if (subEnc == SDVR_ENC_PRIMARY)
        g_MainWindow->setStatusBar(QString("Starting primary encoder on %1...").arg(name()));
    else
        g_MainWindow->setStatusBar(QString("Starting secondary encoder on %1...").arg(name()));

    m_firstStatTimeStamp[subEnc] = 0;
    m_lastStatTimeStamp[subEnc] = 0;

    m_lastAudioTimeStamp = 0;
    m_lastAudioSize = 0;
    m_AccuAudioDelta = 0;

    m_dropFrameCount[subEnc] = 0;

    if (!g_MainWindow->isFirmwareTimeout(m_boardIndex))
        err = sdvr_enable_encoder(handle(), (sdvr_sub_encoders_e)subEnc, true);
    // If the firmware timeout, force the channel to be disabled.
    if (err == SDVR_DRV_ERR_MSG_RECV) {
        g_MainWindow->setFirmwareTimeout(m_boardIndex);
    }

    if (err != SDVR_ERR_NONE)
    {
        QString qstrPrompt = QString("%1: Failed to enable encoder. Error [%2-%3]")
            .arg(name()).arg(sdvr_get_error_text(err)).arg(err);
        // Report, recover and return
        Log::error(qstrPrompt);
        g_DVR->recorder().stopRecording(subEnc, this);
        g_MainWindow->setStatusBar(qstrPrompt);
        return false;
    }

    m_recording[subEnc] = true;


    if (subEnc == SDVR_ENC_PRIMARY)
        qstrPrompt = QString("%1: Started primary encoder.").arg(name());
    else
        qstrPrompt = QString("%1: Started secondary encoder.").arg(name());

    if (g_DVR->getH264MaxFramesRecord())
    {
        QString qstrExtraPrompt;
        qstrPrompt = QString("%1 Limited to %2 frames.").arg(qstrPrompt).arg(g_DVR->getH264MaxFramesRecord());
    }

    if (g_DVR->isSkipRecording())
    {
        QString qstrExtraPrompt;
        qstrPrompt = QString("%1 Skipping recording to file.").arg(qstrPrompt);
    }

    Log::info(qstrPrompt);
    g_MainWindow->setStatusBar(qstrPrompt);

    return true;
}

bool Camera::stopRecording(uint subEnc, bool bInterrupted)
{
    sdvr_err_e err = SDVR_DRV_ERR_MSG_RECV;
    QString qstrPrompt;

    if (!isRecording(subEnc))
        return false;

    // Get rid of the old SPS/PPS frame that were cached.
    m_SPScache.clear();
    m_PPScache.clear();

    if (subEnc == SDVR_ENC_PRIMARY)
        g_MainWindow->setStatusBar(QString("Stopping primary encoder on %1...").arg(name()));
    else
        g_MainWindow->setStatusBar(QString("Stopping secondary encoder on %1...").arg(name()));

    err = sdvr_enable_encoder(handle(),(sdvr_sub_encoders_e)subEnc, false);
    // If the firmware timeout, force the channel to be disabled.
    if (err == SDVR_DRV_ERR_MSG_RECV) {
        g_MainWindow->setFirmwareTimeout(m_boardIndex);
    }

//todo mxa: must remove JPEG special handling once avmux supports
//      saving of JPEG
    if (g_DVR->isUseSDKRecording())
    {

        switch (videoFormat(subEnc))
        {
        case SDVR_VIDEO_ENC_H264:
        case SDVR_VIDEO_ENC_MPEG4:
            if (sdvr_stop_recording(handle(),(sdvr_sub_encoders_e)subEnc) != SDVR_ERR_NONE)
                return false;
            break;
        case SDVR_VIDEO_ENC_JPEG:

            g_DVR->recorder().stopRecording(subEnc, this);
            break;
        default:
            return false;
        }
    }
    else
    {
        g_DVR->recorder().stopRecording(subEnc, this);
    }
    m_recording[subEnc] = false;
    if (err == SDVR_ERR_NONE)
    {
        if (subEnc == SDVR_ENC_PRIMARY)
            qstrPrompt = QString("%1: Stopped primary encoder.").arg(name());
        else
            qstrPrompt = QString("%1: Stopped secondary encoder.").arg(name());

        Log::info(qstrPrompt);
        g_MainWindow->setStatusBar(qstrPrompt);
    }

    if (g_DVR->isShowDropFrames())
    {
        qstrPrompt = QString("Accu. Audio = %1").arg(getAccuAduioDelta());
//        Log::info(qstrPrompt);
    }

    // If we are stopping due to an interrupt that we reached
    // the maximum frames allowed to record, we must re-start the recording
    if (bInterrupted)
        startRecording(subEnc);

    return true;
}

bool Camera::canPlayBack(uint subEnc) const
{
    QFileInfo info(recordFilePath(subEnc,false));
    return info.exists() && info.isReadable();
}

bool Camera::fromElement(QDomElement camElem)
{
    bool ok = true;

    bool converted;
    setId(camElem.attribute(TN_CAMID, "").toInt(&converted));
    if (!converted)
    {
        Log::error("Bad or missing camera ID");
        ok = false;
    }

    QDomElement e = camElem.firstChildElement();
    while (!e.isNull())
    {
        if (e.tagName() == TN_CAMNAME)
        {
            setName(e.text());
        }
        else if (e.tagName() == TN_VPPMODE)
        {
            setVPPMode(sdvr::stringToVPPMode(e.text()));
        }
        else if (e.tagName() == TN_YUVFRAMERATE)
        {
            setYUVFrameRate(e.text().toInt());
        }
        else if (e.tagName() == TN_MOTION_VALUE_FREQ)
        {
            bool converted;
            m_motionValueFreq = e.text().toInt(&converted);
            if (!converted)
            {
                m_motionValueFreq = 0;
            }
        }
        else if (e.tagName() == TN_VPARAMS)
        {
            // Add all the video-in parameters

            QDomElement vParamsHueElem = e.firstChildElement(TC_VPARAMS_HUE);
            if (!vParamsHueElem.isNull())
                setVParamHue(QVariant(vParamsHueElem.text()).toInt());

            QDomElement vParamsContrastElem = e.firstChildElement(TC_VPARAMS_CONTRASTS);
            if (!vParamsContrastElem.isNull())
                setVParamContrast(QVariant(vParamsContrastElem.text()).toInt());

            QDomElement vParamsSaturationElem = e.firstChildElement(TC_VPARAMS_SATURATION);
            if (!vParamsSaturationElem.isNull())
                setVParamSaturation(QVariant(vParamsSaturationElem.text()).toInt());

            QDomElement vParamsBrightnessElem = e.firstChildElement(TC_VPARAMS_BRIGHTNESS);
            if (!vParamsBrightnessElem.isNull())
                setVParamBrightness(QVariant(vParamsBrightnessElem.text()).toInt());

            QDomElement vParamsSharpnessElem = e.firstChildElement(TC_VPARAMS_SHARPNESS);
            if (!vParamsSharpnessElem.isNull())
                setVParamSharpness(QVariant(vParamsSharpnessElem.text()).toInt());

            QDomElement vParamsDeinterlacingElem = e.firstChildElement(TC_VPARAMS_DEINTERLACING);
            if (!vParamsDeinterlacingElem.isNull())
                setVParamsDeinterlacing(QVariant(vParamsDeinterlacingElem.text()).toBool());

            QDomElement vParamsNoiseReductionElem = e.firstChildElement(TC_VPARAMS_NOISEREDUCTION);
            if (!vParamsNoiseReductionElem.isNull())
                setVParamNoiseReduction(QVariant(vParamsNoiseReductionElem.text()).toBool());

            QDomElement vParamsGainModeElem = e.firstChildElement(TC_VPARAMS_GAINMODE);
            if (!vParamsGainModeElem.isNull())
                setVParamGainMode(QVariant(vParamsGainModeElem.text()).toInt());

            QDomElement vParamsCamTerminationElem = e.firstChildElement(TC_VPARAMS_CAM_TERMINATION);
            if (!vParamsCamTerminationElem.isNull())
                setVParamCamTermination(sdvr::StringToCamTermination(vParamsCamTerminationElem.text()));

        }
        else if (e.tagName() == TN_VIDEOENC)
        {
            uint subEnc = e.attribute(TN_SUBENC).toInt(&converted);
            if (!converted)
            {
                // For backware compatibilities if subencoder
                // is missing we mark it as primary
                subEnc = SDVR_ENC_PRIMARY;
            }
            setVideoFormat(subEnc, sdvr::stringToVideoEnc(e.attribute(TN_FORMAT)));
            if (videoFormat(subEnc) != SDVR_VIDEO_ENC_NONE)
            {
                QDomElement defaultElem = e.firstChildElement(TN_DEFAULT);
                if (!defaultElem.isNull())
                {
                    QDomElement frameRateElem = defaultElem.firstChildElement(TN_FRAMERATE);
                    if (!frameRateElem.isNull())
                    {
                        int nFrameRate = frameRateElem.text().toInt();
                        nFrameRate = sdvr::verifyFrameRate(g_DVR->videoStandard(), nFrameRate);
                        setFrameRate(subEnc, nFrameRate);
                    }

                    QDomElement decElem = defaultElem.firstChildElement(TN_DECIMATION);
                    if (!decElem.isNull())
                        setDecimation(subEnc, sdvr::stringToDecimation(decElem.text()));

                    switch (videoFormat(subEnc))
                    {
                        case SDVR_VIDEO_ENC_H264:
                        {
                            QDomElement h264Elem = defaultElem.firstChildElement(TN_H264);
                            if (!h264Elem.isNull())
                            {
                                QDomElement gopSizeElem = h264Elem.firstChildElement(TN_ENCODER_GOPSIZE);
                                if (!gopSizeElem.isNull())
                                    setH264GopSize(subEnc, gopSizeElem.text().toUInt());

                                QDomElement brcElem = h264Elem.firstChildElement(TN_ENCODER_BRC);
                                if (!brcElem.isNull())
                                    setH264BitrateControl(subEnc, sdvr::stringToBrc(brcElem.text()));

                                QDomElement avgBitrateElem = h264Elem.firstChildElement(TN_ENCODER_AVGBR);
                                if (!avgBitrateElem.isNull())
                                    setH264AvgBitrate(subEnc, avgBitrateElem.text().toUInt());

                                QDomElement maxBitrateElem = h264Elem.firstChildElement(TN_ENCODER_MAXBR);
                                if (!maxBitrateElem.isNull())
                                    setH264MaxBitrate(subEnc, maxBitrateElem.text().toUInt());

                                QDomElement qualityElem = h264Elem.firstChildElement(TN_ENCODER_QUALITY);
                                if (!qualityElem.isNull())
                                    setH264Quality(subEnc, qualityElem.text().toUInt());
                            }
                            break;
                        }
                        case SDVR_VIDEO_ENC_JPEG:
                        {
                            QDomElement jpegElem = defaultElem.firstChildElement(TN_JPEG);
                            if (!jpegElem.isNull())
                            {
                                setJpegImageStyle(subEnc, false); // AVI or JPEG image style is only supported for DVR

                                QDomElement qualityElem = jpegElem.firstChildElement(TN_ENCODER_QUALITY);
                                if (!qualityElem.isNull())
                                    setJpegQuality(subEnc, qualityElem.text().toUInt());
                            }
                            break;
                        }
                        case SDVR_VIDEO_ENC_MPEG4:
                        {
                            QDomElement MPEG4Elem = defaultElem.firstChildElement(TN_MPEG4);
                            if (!MPEG4Elem.isNull())
                            {
                                QDomElement gopSizeElem = MPEG4Elem.firstChildElement(TN_ENCODER_GOPSIZE);
                                if (!gopSizeElem.isNull())
                                    setMPEG4GopSize(subEnc, gopSizeElem.text().toUInt());

                                QDomElement brcElem = MPEG4Elem.firstChildElement(TN_ENCODER_BRC);
                                if (!brcElem.isNull())
                                    setMPEG4BitrateControl(subEnc, sdvr::stringToBrc(brcElem.text()));

                                QDomElement avgBitrateElem = MPEG4Elem.firstChildElement(TN_ENCODER_AVGBR);
                                if (!avgBitrateElem.isNull())
                                    setMPEG4AvgBitrate(subEnc, avgBitrateElem.text().toUInt());

                                QDomElement maxBitrateElem = MPEG4Elem.firstChildElement(TN_ENCODER_MAXBR);
                                if (!maxBitrateElem.isNull())
                                    setMPEG4MaxBitrate(subEnc, maxBitrateElem.text().toUInt());

                                QDomElement qualityElem = MPEG4Elem.firstChildElement(TN_ENCODER_QUALITY);
                                if (!qualityElem.isNull())
                                    setMPEG4Quality(subEnc, qualityElem.text().toUInt());
                            }
                            break;
                        }
                        default:
                            break;
                    } //switch (videoFormat(subEnc)
                } // if (!defaultElem.isNull())

                //////////////////////////////////////////////////////////////////////////
                //   On Alarm Parameters
                //////////////////////////////////////////////////////////////////////////
                QDomElement onAlarmElem = e.firstChildElement(TN_ONALARM);
                if (!onAlarmElem.isNull())
                {
                    QDomElement enabledElem = onAlarmElem.firstChildElement(TN_ONALARMENABLED);
                    if (!enabledElem.isNull())
                        setAdjustEncodingOnAlarmEnabled(subEnc, QVariant(enabledElem.text()).toBool());

                    QDomElement minOnElem = onAlarmElem.firstChildElement(TN_MINONTIME);
                    if (!minOnElem.isNull())
                        setOnAlarmMinOnTime(subEnc, minOnElem.text().toUInt());

                    QDomElement minOffElem = onAlarmElem.firstChildElement(TN_MINOFFTIME);
                    if (!minOffElem.isNull())
                        setOnAlarmMinOffTime(subEnc, minOffElem.text().toUInt());

                    QDomElement frameRateElem = onAlarmElem.firstChildElement(TN_FRAMERATE);
                    if (!frameRateElem.isNull())
                    {
                        int nFrameRate = frameRateElem.text().toInt();
                        nFrameRate = sdvr::verifyFrameRate(g_DVR->videoStandard(), nFrameRate);
                        setOnAlarmFrameRate(subEnc, nFrameRate);
                    }

                    // codec specific on alarm parameters
                    switch (videoFormat(subEnc))
                    {
                        case SDVR_VIDEO_ENC_H264:
                            {
                            QDomElement h264Elem = onAlarmElem.firstChildElement(TN_H264);
                            if (!h264Elem.isNull())
                            {
                                QDomElement gopSizeElem = h264Elem.firstChildElement(TN_ENCODER_GOPSIZE);
                                if (!gopSizeElem.isNull())
                                    setOnAlarmH264GopSize(subEnc, gopSizeElem.text().toUInt());

                                QDomElement brcElem = h264Elem.firstChildElement(TN_ENCODER_BRC);
                                if (!brcElem.isNull())
                                    setOnAlarmH264BitrateControl(subEnc, sdvr::stringToBrc(brcElem.text()));

                                QDomElement avgBitrateElem = h264Elem.firstChildElement(TN_ENCODER_AVGBR);
                                if (!avgBitrateElem.isNull())
                                    setOnAlarmH264AvgBitrate(subEnc, avgBitrateElem.text().toUInt());

                                QDomElement maxBitrateElem = h264Elem.firstChildElement(TN_ENCODER_MAXBR);
                                if (!maxBitrateElem.isNull())
                                    setOnAlarmH264MaxBitrate(subEnc, maxBitrateElem.text().toUInt());

                                QDomElement qualityElem = h264Elem.firstChildElement(TN_ENCODER_QUALITY);
                                if (!qualityElem.isNull())
                                    setOnAlarmH264Quality(subEnc, qualityElem.text().toUInt());
                            }
                            break;
                            }
                        case SDVR_VIDEO_ENC_JPEG:
                            {
                            QDomElement jpegElem = onAlarmElem.firstChildElement(TN_JPEG);
                            if (!jpegElem.isNull())
                            {
                                QDomElement qualityElem = jpegElem.firstChildElement(TN_ENCODER_QUALITY);
                                if (!qualityElem.isNull())
                                    setonAlarmJpegQuality(subEnc, qualityElem.text().toUInt());
                            }
                            break;
                            }
                        case SDVR_VIDEO_ENC_MPEG4:
                            {
                            QDomElement MPEG4Elem = onAlarmElem.firstChildElement(TN_MPEG4);
                            if (!MPEG4Elem.isNull())
                            {
                                QDomElement gopSizeElem = MPEG4Elem.firstChildElement(TN_ENCODER_GOPSIZE);
                                if (!gopSizeElem.isNull())
                                    setOnAlarmMPEG4GopSize(subEnc, gopSizeElem.text().toUInt());

                                QDomElement brcElem = MPEG4Elem.firstChildElement(TN_ENCODER_BRC);
                                if (!brcElem.isNull())
                                    setOnAlarmMPEG4BitrateControl(subEnc, sdvr::stringToBrc(brcElem.text()));

                                QDomElement avgBitrateElem = MPEG4Elem.firstChildElement(TN_ENCODER_AVGBR);
                                if (!avgBitrateElem.isNull())
                                    setOnAlarmMPEG4AvgBitrate(subEnc, avgBitrateElem.text().toUInt());

                                QDomElement maxBitrateElem = MPEG4Elem.firstChildElement(TN_ENCODER_MAXBR);
                                if (!maxBitrateElem.isNull())
                                    setOnAlarmMPEG4MaxBitrate(subEnc, maxBitrateElem.text().toUInt());

                                QDomElement qualityElem = MPEG4Elem.firstChildElement(TN_ENCODER_QUALITY);
                                if (!qualityElem.isNull())
                                    setOnAlarmMPEG4Quality(subEnc, qualityElem.text().toUInt());
                            }
                            break;
                            }

                        default:
                            break;
                    } //switch (videoFormat())
                }
            }
        }
        else if (e.tagName() == TN_AUDIOENC)
        {
            setAudioFormat(sdvr::stringToAudioEnc(e.text()));
        }
        else if (e.tagName() == TN_ATTACH)
        {
            int boardIndex = e.attribute(TN_BOARD, "-1").toInt();
            int input = e.attribute(TN_INPUT, "-1").toInt();
            attach(boardIndex, input);
        }
        else if (e.tagName() == TN_OSD)
        {
            int osd_id = e.attribute(TN_OSDID).toInt(&converted);
            if (!converted)
            {
                // For backware compatibilities if osd id
                // is missing we assume id 0
                osd_id = 0;
            }

            QDomElement osdEnabledElem = e.firstChildElement(TN_OSDENABLED);
            if (!osdEnabledElem.isNull())
                setOsdEnabled(QVariant(osdEnabledElem.text()).toBool());

            QDomElement osdTextElem = e.firstChildElement(TN_OSDTEXT);
            if (!osdTextElem.isNull())
                setOsdText(osd_id, osdTextElem.text());

            sx_uint16 osdTLx, osdTLy;
            QDomElement osdLocElem = e.firstChildElement(TN_OSDLOC);
            if (!osdLocElem.isNull())
                setOsdLocation(osd_id, sdvr::stringToOsdLocation(osdLocElem.text()));

            QDomElement osdTLxElem = e.firstChildElement(TC_OSDLOCX);
            if (!osdTLxElem.isNull())
                osdTLx = osdTLxElem.text().toInt();
            else
                osdTLx = 0;

            QDomElement osdTLyElem = e.firstChildElement(TC_OSDLOCY);
            if (!osdTLyElem.isNull())
                osdTLy = osdTLyElem.text().toInt();
            else
                osdTLy = 0;

            setOsdCustomLoc(osd_id, osdTLx, osdTLy);

            QDomElement dtsEnabledElem = e.firstChildElement(TN_OSDDTSENABLED);
            if (!dtsEnabledElem.isNull())
                setOsdDtsEnabled(osd_id, QVariant(dtsEnabledElem.text()).toBool());

            QDomElement dtsFormatElem = e.firstChildElement(TN_OSDDTSFORMAT);
            if (!dtsFormatElem.isNull())
                setOsdDtsFormat(osd_id, sdvr::stringToDtsFormat(dtsFormatElem.text()));

            QDomElement translucentElem = e.firstChildElement(TC_OSDTRANSLUCENT);
            if (!translucentElem.isNull())
                setOsdTranslucent(osd_id, translucentElem.text().toInt());
            else
                setOsdTranslucent(osd_id, 120);

            QDomElement showElem = e.firstChildElement(TC_OSDSHOW);
            // For backward compatibility the show attribute of the
            // first osd id is true if the attribute does not exist.
            bool bShow = (osd_id == 0);
            if (!showElem.isNull())
                bShow = showElem.text().toInt();

            // Only send OSD show if it is enabled. Otherwise by
            // default it is not being shown.
            if (bShow)
                setOsdShow(osd_id, bShow);
        }
        else if (e.tagName() == TN_MD)
        {
            QDomElement mdEnabledElem = e.firstChildElement(TN_ENABLED);
            if (!mdEnabledElem.isNull())
                setMDEnabled(QVariant(mdEnabledElem.text()).toBool());

            QDomElement mdThreshElem = e.firstChildElement(TN_THRESHOLD);
            if (!mdThreshElem.isNull())
                setMDThreshold(0,mdThreshElem.text().toInt());

            QDomElement mdThresh1Elem = e.firstChildElement(TN_THRESHOLD1);
            if (!mdThresh1Elem.isNull())
                setMDThreshold(1,mdThresh1Elem.text().toInt());

            QDomElement mdThresh2Elem = e.firstChildElement(TN_THRESHOLD2);
            if (!mdThresh2Elem.isNull())
                setMDThreshold(2,mdThresh2Elem.text().toInt());

            QDomElement mdThresh3Elem = e.firstChildElement(TN_THRESHOLD3);
            if (!mdThresh3Elem.isNull())
                setMDThreshold(3,mdThresh3Elem.text().toInt());

	        QDomElement mdRegionElem = e.firstChildElement(TN_REGION);
			if (!mdRegionElem.isNull())
				m_MDRegion.fromElement(mdRegionElem);
		}
        else if (e.tagName() == TN_BD)
        {
            QDomElement bdEnabledElem = e.firstChildElement(TN_ENABLED);
            if (!bdEnabledElem.isNull())
                setBDEnabled(QVariant(bdEnabledElem.text()).toBool());

            QDomElement bdThreshElem = e.firstChildElement(TN_THRESHOLD);
            if (!bdThreshElem.isNull())
                setBDThreshold(bdThreshElem.text().toInt());

			QDomElement bdRegionElem = e.firstChildElement(TN_REGION);
			if (!bdRegionElem.isNull())
				m_BDRegion.fromElement(bdRegionElem);
		}
        else if (e.tagName() == TN_ND)
        {
            QDomElement ndEnabledElem = e.firstChildElement(TN_ENABLED);
            if (!ndEnabledElem.isNull())
                setNDEnabled(QVariant(ndEnabledElem.text()).toBool());

            QDomElement ndThreshElem = e.firstChildElement(TN_THRESHOLD);
            if (!ndThreshElem.isNull())
                setNDThreshold(ndThreshElem.text().toInt());
        }
        else if (e.tagName() == TN_PRIVACY)
        {
            QDomElement prEnabledElem = e.firstChildElement(TN_ENABLED);
            if (!prEnabledElem.isNull())
                setPREnabled(QVariant(prEnabledElem.text()).toBool());

	        QDomElement prRegionElem = e.firstChildElement(TN_REGION);
			if (!prRegionElem.isNull())
				m_PRRegion.fromElement(prRegionElem);
        }
        else
        {
            Log::info(QString("Unexpected element '%1' in %2 is ignored")
            .arg(e.tagName()).arg(camElem.tagName()));
            ok = false;
        }

        e = e.nextSiblingElement();
    }

    return ok;
}

QDomElement Camera::toElement(QDomDocument &doc)
{
    QDomElement camElem = doc.createElement(TN_CAMERA);
    camElem.setAttribute(TN_CAMID, id());

    QDomElement nameElem = doc.createElement(TN_CAMNAME);
    nameElem.appendChild(doc.createTextNode(name()));
    camElem.appendChild(nameElem);

    QDomElement vppElem = doc.createElement(TN_VPPMODE);
    vppElem.appendChild(doc.createTextNode(sdvr::vppModeToString(getVPPMode())));
    camElem.appendChild(vppElem);

    QDomElement yuvFrameRateElem = doc.createElement(TN_YUVFRAMERATE);
    yuvFrameRateElem.appendChild(doc.createTextNode(QString::number(getYUVFrameRate())));
    camElem.appendChild(yuvFrameRateElem);

    QDomElement motionValueFreqElem = doc.createElement(TN_MOTION_VALUE_FREQ);
    motionValueFreqElem.appendChild(doc.createTextNode(QString::number(m_motionValueFreq)));
    camElem.appendChild(motionValueFreqElem);

    // Add all the video-in parameters
    QDomElement videoParamsElem = doc.createElement(TN_VPARAMS);

    QDomElement vParamsHueElem = doc.createElement(TC_VPARAMS_HUE);
    vParamsHueElem.appendChild(doc.createTextNode(QString::number(getVParamHue())));
    videoParamsElem.appendChild(vParamsHueElem);

    QDomElement vParamsContrastElem = doc.createElement(TC_VPARAMS_CONTRASTS);
    vParamsContrastElem.appendChild(doc.createTextNode(QString::number(getVParamContrast())));
    videoParamsElem.appendChild(vParamsContrastElem);

    QDomElement vParamsSaturationElem = doc.createElement(TC_VPARAMS_SATURATION);
    vParamsSaturationElem.appendChild(doc.createTextNode(QString::number(getVParamSaturation())));
    videoParamsElem.appendChild(vParamsSaturationElem);

    QDomElement vParamsBrightnessElem = doc.createElement(TC_VPARAMS_BRIGHTNESS);
    vParamsBrightnessElem.appendChild(doc.createTextNode(QString::number(getVParamBrightness())));
    videoParamsElem.appendChild(vParamsBrightnessElem);

    QDomElement vParamsSharpnessElem = doc.createElement(TC_VPARAMS_SHARPNESS);
    vParamsSharpnessElem.appendChild(doc.createTextNode(QString::number(getVParamSharpness())));
    videoParamsElem.appendChild(vParamsSharpnessElem);

    QDomElement vParamsDeinterlacingElem = doc.createElement(TC_VPARAMS_DEINTERLACING);
    vParamsDeinterlacingElem.appendChild(doc.createTextNode(QVariant(isVParamsDeinterlacing()).toString()));
    videoParamsElem.appendChild(vParamsDeinterlacingElem);

    QDomElement vParamsNoiseReductionElem = doc.createElement(TC_VPARAMS_NOISEREDUCTION);
    vParamsNoiseReductionElem.appendChild(doc.createTextNode(QVariant(isVParamNoiseReduction()).toString()));
    videoParamsElem.appendChild(vParamsNoiseReductionElem);

    QDomElement vParamsGainModeElem = doc.createElement(TC_VPARAMS_GAINMODE);
    vParamsGainModeElem.appendChild(doc.createTextNode(QString::number(getVParamGainMode())));
    videoParamsElem.appendChild(vParamsGainModeElem);

    QDomElement vParamsCamTerminationElem = doc.createElement(TC_VPARAMS_CAM_TERMINATION);
    vParamsCamTerminationElem.appendChild(doc.createTextNode(sdvr::camTerminationToString(getVParamCamTermination())));
    videoParamsElem.appendChild(vParamsCamTerminationElem);


    camElem.appendChild(videoParamsElem);

    // Add all the video encoder parameters for each sub encoder
    for (int subEnc = 0; subEnc < MAX_SUB_ENCODERS; subEnc++)
    {

        if (videoFormat(subEnc) != SDVR_VIDEO_ENC_NONE)
        {
            QDomElement videoEncElem = doc.createElement(TN_VIDEOENC);
            videoEncElem.setAttribute(TN_FORMAT, sdvr::videoEncToString(videoFormat(subEnc)));
            videoEncElem.setAttribute(TN_SUBENC, subEnc);

            QDomElement defaultElem = doc.createElement(TN_DEFAULT);

            QDomElement frameRateElem = doc.createElement(TN_FRAMERATE);
            frameRateElem.appendChild(doc.createTextNode(QString::number(frameRate(subEnc))));
            defaultElem.appendChild(frameRateElem);

            QDomElement decElem = doc.createElement(TN_DECIMATION);
            decElem.appendChild(doc.createTextNode(sdvr::decimationToString(decimation(subEnc))));
            defaultElem.appendChild(decElem);

            switch (videoFormat(subEnc))
            {
                case SDVR_VIDEO_ENC_H264:
                {
                    QDomElement h264Elem = doc.createElement(TN_H264);

                    QDomElement gopSizeElem = doc.createElement(TN_ENCODER_GOPSIZE);
                    gopSizeElem.appendChild(doc.createTextNode(QString::number(h264GopSize(subEnc))));
                    h264Elem.appendChild(gopSizeElem);

                    QDomElement brcElem = doc.createElement(TN_ENCODER_BRC);
                    brcElem.appendChild(
                        doc.createTextNode(sdvr::brcToString(h264BitrateControl(subEnc))));
                    h264Elem.appendChild(brcElem);

                    QDomElement avgBitrateElem = doc.createElement(TN_ENCODER_AVGBR);
                    avgBitrateElem.appendChild(
                        doc.createTextNode(QString::number(h264AvgBitrate(subEnc))));
                    h264Elem.appendChild(avgBitrateElem);

                    QDomElement maxBitrateElem = doc.createElement(TN_ENCODER_MAXBR);
                    maxBitrateElem.appendChild(
                        doc.createTextNode(QString::number(h264MaxBitrate(subEnc))));
                    h264Elem.appendChild(maxBitrateElem);

                    QDomElement qualityElem = doc.createElement(TN_ENCODER_QUALITY);
                    qualityElem.appendChild(
                        doc.createTextNode(QString::number(h264Quality(subEnc))));
                    h264Elem.appendChild(qualityElem);

                    defaultElem.appendChild(h264Elem);
                    break;
                }
                case SDVR_VIDEO_ENC_JPEG:
                {
                    QDomElement jpegElem = doc.createElement(TN_JPEG);

                    QDomElement qualityElem = doc.createElement(TN_ENCODER_QUALITY);
                    qualityElem.appendChild(doc.createTextNode(QString::number(jpegQuality(subEnc))));
                    jpegElem.appendChild(qualityElem);

                    defaultElem.appendChild(jpegElem);
                    break;
                }
                case SDVR_VIDEO_ENC_MPEG4:
                {
                    QDomElement MPEG4Elem = doc.createElement(TN_MPEG4);

                    QDomElement gopSizeElem = doc.createElement(TN_ENCODER_GOPSIZE);
                    gopSizeElem.appendChild(doc.createTextNode(QString::number(MPEG4GopSize(subEnc))));
                    MPEG4Elem.appendChild(gopSizeElem);

                    QDomElement brcElem = doc.createElement(TN_ENCODER_BRC);
                    brcElem.appendChild(
                        doc.createTextNode(sdvr::brcToString(MPEG4BitrateControl(subEnc))));
                    MPEG4Elem.appendChild(brcElem);

                    QDomElement avgBitrateElem = doc.createElement(TN_ENCODER_AVGBR);
                    avgBitrateElem.appendChild(
                        doc.createTextNode(QString::number(MPEG4AvgBitrate(subEnc))));
                    MPEG4Elem.appendChild(avgBitrateElem);

                    QDomElement maxBitrateElem = doc.createElement(TN_ENCODER_MAXBR);
                    maxBitrateElem.appendChild(
                        doc.createTextNode(QString::number(MPEG4MaxBitrate(subEnc))));
                    MPEG4Elem.appendChild(maxBitrateElem);

                    QDomElement qualityElem = doc.createElement(TN_ENCODER_QUALITY);
                    qualityElem.appendChild(
                        doc.createTextNode(QString::number(MPEG4Quality(subEnc))));
                    MPEG4Elem.appendChild(qualityElem);

                    defaultElem.appendChild(MPEG4Elem);
                    break;
                }
                default:
                    break;
            } //switch (videoFormat(subEnc)

            videoEncElem.appendChild(defaultElem);

            //////////////////////////////////////////////////////////////////////////
            //   On Alarm Parameters
            //////////////////////////////////////////////////////////////////////////

            QDomElement onAlarmElem = doc.createElement(TN_ONALARM);
            QDomElement onAlarmEnabledElem = doc.createElement(TN_ONALARMENABLED);

            onAlarmEnabledElem.appendChild(
                doc.createTextNode(QVariant(isAdjustEncodingOnAlarmEnabled(subEnc)).toString()));
            onAlarmElem.appendChild(onAlarmEnabledElem);

            QDomElement minOnElem = doc.createElement(TN_MINONTIME);
            minOnElem.appendChild(doc.createTextNode(QString::number(onAlarmMinOnTime(subEnc))));
            onAlarmElem.appendChild(minOnElem);

            QDomElement minOffElem = doc.createElement(TN_MINOFFTIME);
            minOffElem.appendChild(doc.createTextNode(QString::number(onAlarmMinOffTime(subEnc))));
            onAlarmElem.appendChild(minOffElem);

            QDomElement onAlarmFrameRateElem = doc.createElement(TN_FRAMERATE);
            onAlarmFrameRateElem.appendChild(doc.createTextNode(QString::number(onAlarmFrameRate(subEnc))));
            onAlarmElem.appendChild(onAlarmFrameRateElem);

            // codec specific on alarm parameters
            switch (videoFormat(subEnc))
            {
                case SDVR_VIDEO_ENC_H264:
                {
                    QDomElement h264Elem = doc.createElement(TN_H264);

                    QDomElement gopSizeElem = doc.createElement(TN_ENCODER_GOPSIZE);
                    gopSizeElem.appendChild(doc.createTextNode(QString::number(onAlarmH264GopSize(subEnc))));
                    h264Elem.appendChild(gopSizeElem);

                    QDomElement brcElem = doc.createElement(TN_ENCODER_BRC);
                    brcElem.appendChild(
                        doc.createTextNode(sdvr::brcToString(onAlarmH264BitrateControl(subEnc))));
                    h264Elem.appendChild(brcElem);

                    QDomElement avgBitrateElem = doc.createElement(TN_ENCODER_AVGBR);
                    avgBitrateElem.appendChild(
                        doc.createTextNode(QString::number(onAlarmH264AvgBitrate(subEnc))));
                    h264Elem.appendChild(avgBitrateElem);

                    QDomElement maxBitrateElem = doc.createElement(TN_ENCODER_MAXBR);
                    maxBitrateElem.appendChild(
                        doc.createTextNode(QString::number(onAlarmH264MaxBitrate(subEnc))));
                    h264Elem.appendChild(maxBitrateElem);

                    QDomElement qualityElem = doc.createElement(TN_ENCODER_QUALITY);
                    qualityElem.appendChild(
                        doc.createTextNode(QString::number(onAlarmH264Quality(subEnc))));
                    h264Elem.appendChild(qualityElem);

                    onAlarmElem.appendChild(h264Elem);
                    break;
                }
                case SDVR_VIDEO_ENC_JPEG:
                {
                    QDomElement jpegElem = doc.createElement(TN_JPEG);

                    QDomElement qualityElem = doc.createElement(TN_ENCODER_QUALITY);
                    qualityElem.appendChild(doc.createTextNode(QString::number(getOnAlarmJpegQuality(subEnc))));
                    jpegElem.appendChild(qualityElem);

                    onAlarmElem.appendChild(jpegElem);
                    break;
                }
                case SDVR_VIDEO_ENC_MPEG4:
                {
                    QDomElement MPEG4Elem = doc.createElement(TN_MPEG4);

                    QDomElement gopSizeElem = doc.createElement(TN_ENCODER_GOPSIZE);
                    gopSizeElem.appendChild(doc.createTextNode(QString::number(onAlarmMPEG4GopSize(subEnc))));
                    MPEG4Elem.appendChild(gopSizeElem);

                    QDomElement brcElem = doc.createElement(TN_ENCODER_BRC);
                    brcElem.appendChild(
                        doc.createTextNode(sdvr::brcToString(onAlarmMPEG4BitrateControl(subEnc))));
                    MPEG4Elem.appendChild(brcElem);

                    QDomElement avgBitrateElem = doc.createElement(TN_ENCODER_AVGBR);
                    avgBitrateElem.appendChild(
                        doc.createTextNode(QString::number(onAlarmMPEG4AvgBitrate(subEnc))));
                    MPEG4Elem.appendChild(avgBitrateElem);

                    QDomElement maxBitrateElem = doc.createElement(TN_ENCODER_MAXBR);
                    maxBitrateElem.appendChild(
                        doc.createTextNode(QString::number(onAlarmMPEG4MaxBitrate(subEnc))));
                    MPEG4Elem.appendChild(maxBitrateElem);

                    QDomElement qualityElem = doc.createElement(TN_ENCODER_QUALITY);
                    qualityElem.appendChild(
                        doc.createTextNode(QString::number(onAlarmMPEG4Quality(subEnc))));
                    MPEG4Elem.appendChild(qualityElem);

                    onAlarmElem.appendChild(MPEG4Elem);
                    break;
                }
                default:
                    break;
            } // switch (videoFormat(subEnc))

            videoEncElem.appendChild(onAlarmElem);

            camElem.appendChild(videoEncElem);
        }
    } //for (int subEnc = 0; subEnc < MAX_SUB_ENCODERS; subEnc++)

    QDomElement audioEncElem = doc.createElement(TN_AUDIOENC);
    audioEncElem.appendChild(
        doc.createTextNode(sdvr::audioEncToString(audioFormat())));
    camElem.appendChild(audioEncElem);

    if (isAttached())
    {
        QDomElement attachElem = doc.createElement(TN_ATTACH);
        attachElem.setAttribute(TN_BOARD, boardIndex());
        attachElem.setAttribute(TN_INPUT, input());
        camElem.appendChild(attachElem);
    }

    // OSD
    for (sx_uint8 osd_id = 0; osd_id < SDVR_MAX_OSD; osd_id++)
    {
        QDomElement osdElem = doc.createElement(TN_OSD);
        osdElem.setAttribute(TN_OSDID,QString::number(osd_id));

        QDomElement osdEnabledElem = doc.createElement(TN_OSDENABLED);
        osdEnabledElem.appendChild(
                doc.createTextNode(QVariant(isOsdEnabled()).toString()));
        osdElem.appendChild(osdEnabledElem);

        QDomElement osdTextElem = doc.createElement(TN_OSDTEXT);
        osdTextElem.appendChild(doc.createTextNode(osdText(osd_id)));
        osdElem.appendChild(osdTextElem);

        QDomElement osdLocElem = doc.createElement(TN_OSDLOC);
        osdLocElem.appendChild(doc.createTextNode(
                sdvr::osdLocationToString(osdLocation(osd_id))));
        osdElem.appendChild(osdLocElem);

        sx_uint16 x, y;
        getOsdCustomLoc(osd_id, &x, &y);
        QDomElement xLocElem = doc.createElement(TC_OSDLOCX);
        xLocElem.appendChild(doc.createTextNode(QString::number(x)));
        osdElem.appendChild(xLocElem);

        QDomElement yLocElem = doc.createElement(TC_OSDLOCY);
        yLocElem.appendChild(doc.createTextNode(QString::number(y)));
        osdElem.appendChild(yLocElem);

        QDomElement dtsEnabledElem = doc.createElement(TN_OSDDTSENABLED);
        dtsEnabledElem.appendChild(
                doc.createTextNode(QVariant(isOsdDtsEnabled(osd_id)).toString()));
        osdElem.appendChild(dtsEnabledElem);

        QDomElement dtsFormatElem = doc.createElement(TN_OSDDTSFORMAT);
        dtsFormatElem.appendChild(doc.createTextNode(
                sdvr::dtsFormatToString(osdDtsFormat(osd_id))));
        osdElem.appendChild(dtsFormatElem);

        QDomElement translucentElem = doc.createElement(TC_OSDTRANSLUCENT);
        translucentElem.appendChild(doc.createTextNode(
                        QString::number(getOsdTranslucent(osd_id))));
        osdElem.appendChild(translucentElem);

        QDomElement showElem = doc.createElement(TC_OSDSHOW);
        bool bShow = isOsdShow(osd_id);
        QString qstrShow = QString::number(bShow);
        showElem.appendChild(doc.createTextNode(qstrShow));

        osdElem.appendChild(showElem);

        camElem.appendChild(osdElem);
    }

    // Motion detection
    QDomElement mdElem = doc.createElement(TN_MD);
    QDomElement mdEnabledElem = doc.createElement(TN_ENABLED);
    mdEnabledElem.appendChild(
            doc.createTextNode(QVariant(isMDEnabled()).toString()));
    mdElem.appendChild(mdEnabledElem);

    QDomElement mdThreshElem = doc.createElement(TN_THRESHOLD);
    mdThreshElem.appendChild(
            doc.createTextNode(QString::number(MDThreshold(0))));
    mdElem.appendChild(mdThreshElem);

    QDomElement mdThresh1Elem = doc.createElement(TN_THRESHOLD1);
    mdThresh1Elem.appendChild(
            doc.createTextNode(QString::number(MDThreshold(1))));
    mdElem.appendChild(mdThresh1Elem);

    QDomElement mdThresh2Elem = doc.createElement(TN_THRESHOLD2);
    mdThresh2Elem.appendChild(
            doc.createTextNode(QString::number(MDThreshold(2))));
    mdElem.appendChild(mdThresh2Elem);

    QDomElement mdThresh3Elem = doc.createElement(TN_THRESHOLD3);
    mdThresh3Elem.appendChild(
            doc.createTextNode(QString::number(MDThreshold(3))));
    mdElem.appendChild(mdThresh3Elem);

	if (MDRegion().isInitialized())
	{
		QDomElement mdRegionElem = MDRegion().toElement(doc);
		mdElem.appendChild(mdRegionElem);
	}

	camElem.appendChild(mdElem);

    // Blind detection
    QDomElement bdElem = doc.createElement(TN_BD);
    QDomElement bdEnabledElem = doc.createElement(TN_ENABLED);
    bdEnabledElem.appendChild(
            doc.createTextNode(QVariant(isBDEnabled()).toString()));
    bdElem.appendChild(bdEnabledElem);

    QDomElement bdThreshElem = doc.createElement(TN_THRESHOLD);
    bdThreshElem.appendChild(
            doc.createTextNode(QString::number(BDThreshold())));
    bdElem.appendChild(bdThreshElem);

	if (BDRegion().isInitialized())
	{
		QDomElement bdRegionElem = BDRegion().toElement(doc);
		bdElem.appendChild(bdRegionElem);
	}

    camElem.appendChild(bdElem);

    // Night detection
    QDomElement ndElem = doc.createElement(TN_ND);
    QDomElement ndEnabledElem = doc.createElement(TN_ENABLED);
    ndEnabledElem.appendChild(
            doc.createTextNode(QVariant(isNDEnabled()).toString()));
    ndElem.appendChild(ndEnabledElem);

    QDomElement ndThreshElem = doc.createElement(TN_THRESHOLD);
    ndThreshElem.appendChild(
            doc.createTextNode(QString::number(NDThreshold())));
    ndElem.appendChild(ndThreshElem);

    camElem.appendChild(ndElem);

	// Privacy
    QDomElement prElem = doc.createElement(TN_PRIVACY);
    QDomElement prEnabledElem = doc.createElement(TN_ENABLED);
    prEnabledElem.appendChild(
            doc.createTextNode(QVariant(isPREnabled()).toString()));
    prElem.appendChild(prEnabledElem);

	if (PRRegion().isInitialized())
	{
		QDomElement prRegionElem = PRRegion().toElement(doc);
		prElem.appendChild(prRegionElem);
	}

	camElem.appendChild(prElem);

    return camElem;
}

sdvr_err_e Camera::applyYUVFrameRate()
{
    sdvr_err_e err = SDVR_ERR_NONE;
    // Force the frame rate change if the
    // raw video is streaming.
    if (isRawVideoEnabled())
        err = setRawVideo(g_MainWindow->currentMonitorSettings()->decimation(), true);

    return err;
}

sdvr_err_e Camera::applyMotionFrequency()
{
    sdvr_err_e err = SDVR_ERR_NONE;

    // Set the frequency of sending the motion value frame.
    // This value applies to all the board.
    err = sdvr_set_motion_value_frequency(handle(), getMotionValueFreq());

    return err;
}
sdvr_err_e Camera::applyOsd()
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 osdItem;
    sdvr_firmware_ver_t  version;

    g_DVR->getFWVersion(&version);


    if (isOsdEnabled())
    {
        sdvr_osd_config_ex_t osd_text_config;
        for (osdItem = 0; osdItem < SDVR_MAX_OSD && err == SDVR_ERR_NONE; osdItem++)
        {

            if (isOsdDtsEnabled(osdItem))
                osd_text_config.dts_format = osdDtsFormat(osdItem);
            else
                osd_text_config.dts_format = SDVR_OSD_DTS_NONE;
            osd_text_config.translucent = getOsdTranslucent(osdItem);
            osd_text_config.location_ctrl = osdLocation(osdItem);
            getOsdCustomLoc(osdItem, &osd_text_config.top_left_x,
                            &osd_text_config.top_left_y);
            osd_text_config.text_len = osdText(osdItem).length();
            memcpy(&osd_text_config.text,
                osdText(osdItem).data(),
                osd_text_config.text_len * 2);

            err =  sdvr_osd_text_config_ex(m_handle,
                                      osdItem,
                                      &osd_text_config);
        }
    }
    if (err == SDVR_ERR_NONE)
    {
        for (osdItem = 0; osdItem < SDVR_MAX_OSD && err == SDVR_ERR_NONE; osdItem++)
        {
            err = sdvr_osd_text_show(m_handle, osdItem,
                             isOsdShow(osdItem) && isOsdEnabled());
            // skip all invalid OSD ID.
            if (err == SDVR_ERR_INVALID_OSD_ID)
                err = SDVR_ERR_NONE;
        }
    }
    return err;
}

sdvr_err_e Camera::applyVideoEncoding(uint subEnc)
{
    sdvr_video_enc_chan_params_t params;
    params.frame_rate = frameRate(subEnc);
    int nMaxAllowedFrameRate = sdvr::verifyFrameRate(g_DVR->videoStandard(), params.frame_rate);
    if (params.frame_rate > nMaxAllowedFrameRate)
    {
        params.frame_rate = nMaxAllowedFrameRate;
        setFrameRate(subEnc, nMaxAllowedFrameRate);
    }

    params.res_decimation = decimation(subEnc);

    switch (videoFormat(subEnc))
    {
        case  SDVR_VIDEO_ENC_H264:
        {
            params.encoder.h264.gop = h264GopSize(subEnc);
            params.encoder.h264.bitrate_control = h264BitrateControl(subEnc);
            params.encoder.h264.avg_bitrate = h264AvgBitrate(subEnc);
            params.encoder.h264.max_bitrate = h264MaxBitrate(subEnc);
            params.encoder.h264.quality = h264Quality(subEnc);
            break;
        }
        case SDVR_VIDEO_ENC_JPEG:
        {
            params.encoder.jpeg.is_image_style = jpegIsImageStyle(subEnc);
            params.encoder.jpeg.quality = jpegQuality(subEnc);
            break;
        }
        case  SDVR_VIDEO_ENC_MPEG4:
        {
            params.encoder.mpeg4.gop = MPEG4GopSize(subEnc);
            params.encoder.mpeg4.bitrate_control = MPEG4BitrateControl(subEnc);
            params.encoder.mpeg4.avg_bitrate = MPEG4AvgBitrate(subEnc);
            params.encoder.mpeg4.max_bitrate = MPEG4MaxBitrate(subEnc);
            params.encoder.mpeg4.quality = MPEG4Quality(subEnc);
            break;
        }

        default:
            return SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
    } // switch (videoFormat(subEnc))

    return sdvr_set_video_encoder_channel_params(handle(), (sdvr_sub_encoders_e) subEnc, &params);
}

sdvr_err_e Camera::applyOnAlarmVideoEncoding(uint subEnc)
{
    sdvr_alarm_video_enc_params_t params;

    params.enable = isAdjustEncodingOnAlarmEnabled(subEnc);
    params.min_on_seconds = onAlarmMinOnTime(subEnc);
    params.min_off_seconds = onAlarmMinOffTime(subEnc);
    params.frame_rate = onAlarmFrameRate(subEnc);
    int nMaxAllowedFrameRate = sdvr::verifyFrameRate(g_DVR->videoStandard(), params.frame_rate);
    if (params.frame_rate > nMaxAllowedFrameRate)
    {
        params.frame_rate = nMaxAllowedFrameRate;
        setOnAlarmFrameRate(subEnc, nMaxAllowedFrameRate);
    }

    switch (videoFormat(subEnc))
    {
        case SDVR_VIDEO_ENC_JPEG:
        {
            // For now is_image_style is alwasy 0 which means AVI Motion JPEG
            params.encoder.jpeg.is_image_style = 0;
            params.encoder.jpeg.quality = getOnAlarmJpegQuality(subEnc);
            break;
        }
        case SDVR_VIDEO_ENC_H264:
        {
            params.encoder.h264.gop = onAlarmH264GopSize(subEnc);
            params.encoder.h264.bitrate_control = onAlarmH264BitrateControl(subEnc);
            params.encoder.h264.avg_bitrate = onAlarmH264AvgBitrate(subEnc);
            params.encoder.h264.max_bitrate = onAlarmH264MaxBitrate(subEnc);
            params.encoder.h264.quality = onAlarmH264Quality(subEnc);
            break;
        }
        case SDVR_VIDEO_ENC_MPEG4:
        {
            params.encoder.mpeg4.gop = onAlarmMPEG4GopSize(subEnc);
            params.encoder.mpeg4.bitrate_control = onAlarmMPEG4BitrateControl(subEnc);
            params.encoder.mpeg4.avg_bitrate = onAlarmMPEG4AvgBitrate(subEnc);
            params.encoder.mpeg4.max_bitrate = onAlarmMPEG4MaxBitrate(subEnc);
            params.encoder.mpeg4.quality = onAlarmMPEG4Quality(subEnc);
            break;
        }

        default:
            return SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT;
    }

    return sdvr_set_alarm_video_encoder_params(handle(), (sdvr_sub_encoders_e) subEnc, &params);
}

sdvr_err_e Camera::applyMD()
{
//    sdvr_err_e err = sdvr_enable_motion_detection(m_handle,isMDEnabled(),MDThreshold());
    sdvr_err_e err = sdvr_enable_motion_detection_ex(m_handle,isMDEnabled(),
                                                    MDThreshold(0),
                                                    MDThreshold(1),
                                                    MDThreshold(2),
                                                    MDThreshold(3));

    if (err != SDVR_ERR_NONE)
        Log::error(QString("%1: Failed to %2 Motion Detection . Error = %3[%4]")
        .arg(name()).arg(isMDEnabled() ? "Enable" : "Disable").arg(sdvr_get_error_text(err)).arg(err));

    return err;

}

sdvr_err_e Camera::applyMDRegions()
{
    sdvr_err_e err = SDVR_ERR_NONE;

    if (m_MDRegion.isInitialized())
    {
        m_MDRegion.migrate(g_DVR->videoStandard());
        err = sdvr_set_regions_map(m_handle, SDVR_REGION_MOTION,
                                   (sx_uint8 *)m_MDRegion.data());
        if (err != SDVR_ERR_NONE)
            Log::error(QString("%1: Failed to save Motion Detection regions . Error = %2 [%3]")
            .arg(name()).arg(sdvr_get_error_text(err)).arg(err));
    }

    return err;
}

sdvr_err_e Camera::applyBD()
{
    sdvr_err_e err = sdvr_enable_blind_detection(m_handle,isBDEnabled(),BDThreshold());

    if (err != SDVR_ERR_NONE)
        Log::error(QString("%1: Failed to %2 blind dectection. Error = %3[%4]")
        .arg(name()).arg(isBDEnabled() ? "Enable" : "Disable").arg(sdvr_get_error_text(err)).arg(err));

    return err;
}

sdvr_err_e Camera::applyBDRegions()
{
    sdvr_err_e err = SDVR_ERR_NONE;

    if (m_BDRegion.isInitialized())
    {
        m_BDRegion.migrate(g_DVR->videoStandard());
        err = sdvr_set_regions_map(m_handle, SDVR_REGION_BLIND,
                                   (sx_uint8 *)m_BDRegion.data());

        if (err != SDVR_ERR_NONE)
            Log::error(QString("%1: Failed to save Blind Detection regions . Error = %2 [%3]")
            .arg(name()).arg(sdvr_get_error_text(err)).arg(err));
    }

    return err;
}


sdvr_err_e Camera::applyPR()
{
    sdvr_err_e err = sdvr_enable_privacy_regions(m_handle,isPREnabled());

    if (err != SDVR_ERR_NONE)
        Log::error(QString("%1: Failed to %2 privacy regions. Error = %3[%4]")
        .arg(name()).arg(isPREnabled() ? "Enable" : "Disable").arg(sdvr_get_error_text(err)).arg(err));

    return err;
}

sdvr_err_e Camera::applyPRRegions()
{
    sdvr_err_e err = SDVR_ERR_NONE;

    if (m_PRRegion.isInitialized())
    {
        m_PRRegion.migrate(g_DVR->videoStandard());
        err = sdvr_set_regions_map(m_handle, SDVR_REGION_PRIVACY,
                                   (sx_uint8 *)m_PRRegion.data());

        if (err != SDVR_ERR_NONE)
            Log::error(QString("%1: Failed to save privacy blocking regions . Error = %2 [%3]")
            .arg(name()).arg(sdvr_get_error_text(err)).arg(err));
    }

    return err;
}

sdvr_err_e Camera::applyND() const
{
    return sdvr_enable_night_detection(m_handle, isNDEnabled(),NDThreshold());
}

//////////////////////////////////////////////////////////
// Send all video-In settings that is not -1 to the board
//
sdvr_err_e Camera::applyVideoInHCSBSSettings(int nHue, int nContrast,
                                        int nSaturation, int nBrightness,
                                        int nSharpness)
{
    sdvr_err_e err;
    sdvr_image_ctrl_t image_ctrl;
    sx_uint16 flags = 0;

    if (nHue >= 0)
    {
        flags |= SDVR_ICFLAG_HUE;
        image_ctrl.hue = (sx_uint8)nHue;
    }
    else
        image_ctrl.hue = 0;

    if (nContrast >= 0)
    {
        flags |= SDVR_ICFLAG_CONTRAST;
        image_ctrl.contrast = (sx_uint8)nContrast;
    }
    else
        image_ctrl.contrast = 0;

    if (nSaturation >= 0)
    {
        flags |= SDVR_ICFLAG_SATURATION;
        image_ctrl.saturation = (sx_uint8)nSaturation;
    }
    else
        image_ctrl.saturation = 0;

    if (nBrightness >= 0)
    {
        flags |= SDVR_ICFLAG_BRIGHTNESS;
        image_ctrl.brightness = (sx_uint8)nBrightness;
    }
    else
        image_ctrl.brightness = 0;

    if (nSharpness >= 0)
    {
        flags |= SDVR_ICFLAG_SHARPNESS;
        image_ctrl.sharpness = (sx_uint8)nSharpness;
    }
    else
        image_ctrl.sharpness = 0;

    err = sdvr_set_video_in_params(handle(), flags,
                                   &image_ctrl);

    return err;
}

//////////////////////////////////////////////////////
// Send all the current video-In settings to the board
//
sdvr_err_e Camera::applyVideoInSettings()
{
    sdvr_err_e err;
    sdvr_image_ctrl_t image_ctrl;

    image_ctrl.hue = getVParamHue();
    image_ctrl.contrast = getVParamContrast();
    image_ctrl.saturation = getVParamSaturation();
    image_ctrl.brightness = getVParamBrightness();
    image_ctrl.sharpness = getVParamSharpness();
    err = sdvr_set_video_in_params(handle(), SDVR_ICFLAG_ALL,
                                   &image_ctrl);

    if (err == SDVR_ERR_NONE)
        err = sdvr_enable_deinterlacing(handle(),isVParamsDeinterlacing());
    if (err == SDVR_ERR_NONE)
        err = sdvr_enable_noise_reduction(handle(),isVParamNoiseReduction());
    if (err == SDVR_ERR_NONE)
        err = sdvr_set_gain_mode(handle(),getVParamGainMode());
    if (err == SDVR_ERR_NONE)
        err = sdvr_set_camera_termination(handle(),getVParamCamTermination());

    return err;

}

bool Camera::jpegIsImageStyle(uint subEnc) const
{
    return m_jpegImageStyle[subEnc];
}
sx_uint8 Camera::getYUVFrameRate()
{
    int stdFrameRate = sdvr::frameRate(g_DVR->videoStandard());
    if (m_yuvFrameRate > stdFrameRate)
        m_yuvFrameRate = stdFrameRate;

    return m_yuvFrameRate;
}

void Camera::setYUVFrameRate(sx_uint8 rate)
{
    int stdFrameRate = sdvr::frameRate(g_DVR->videoStandard());
    if (rate > stdFrameRate)
        rate = stdFrameRate;
    m_yuvFrameRate = rate;
}


void Camera::savePictureInfoFrame(sdvr_av_buffer_t *buf)
{
    switch (buf->frame_type)
    {
    case SDVR_FRAME_H264_SPS:
    {
        QByteArray bufData((const char *)buf, buf->payload_size + (&buf->payload[0] - (sx_uint8 *)buf));
        if (!m_SPScache.isEmpty())
            m_SPScache.clear();
        m_SPScache = bufData;

        break;
    }
    case SDVR_FRAME_H264_PPS:
    {
        QByteArray bufData((const char *)buf, buf->payload_size + (&buf->payload[0] - (sx_uint8 *)buf));
        if (!m_PPScache.isEmpty())
            m_PPScache.clear();
        m_PPScache = bufData;

        break;
    }
    case SDVR_FRAME_MPEG4_VOL:
    {
        QByteArray bufData((const char *)buf, buf->payload_size + (&buf->payload[0] - (sx_uint8 *)buf));
        if (!m_VOLcache.isEmpty())
            m_VOLcache.clear();
        m_VOLcache = bufData;

        break;
    }

    default:
        break;
    }
}
sdvr_av_buffer_t *Camera::getPictureInfoFrame(sdvr_frame_type_e frame_type)
{
    sdvr_av_buffer_t *avBuf = NULL;

    switch (frame_type)
    {
    case SDVR_FRAME_H264_SPS:
    {
        if (!m_SPScache.isEmpty())
            avBuf =  (sdvr_av_buffer_t *)m_SPScache.data();
        break;
    }
    case SDVR_FRAME_H264_PPS:
    {
        if (!m_PPScache.isEmpty())
            avBuf = (sdvr_av_buffer_t *)m_PPScache.data();

        break;
    }
    case SDVR_FRAME_MPEG4_VOL:
    {
        if (!m_VOLcache.isEmpty())
            avBuf = (sdvr_av_buffer_t *)m_VOLcache.data();

        break;
    }

    default:
        break;
    }

    return avBuf;
}

/************************** Class Region ******************************/

CRegion::CRegion()
{
	clear();
}

CRegion::CRegion(sdvr_video_std_e std, int layers)
{
	init(std, layers);
}

CRegion::CRegion(CRegion &other)
{
    *this = other;
}

void CRegion::clear()
{
	m_videoStd = SDVR_VIDEO_STD_NONE;
	m_layers = 0;
	m_rows = m_cols = 0;
	m_frameSize.setWidth(0);
	m_frameSize.setHeight(0);
	m_array.clear();
}

CRegion & CRegion::operator=(CRegion & other)
{
    if (this != &other)
    {
        m_videoStd = other.m_videoStd;
        m_frameSize = other.m_frameSize;
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        m_layers = other.m_layers;
        m_array = other.m_array;
    }
    return *this;
}

bool CRegion::init(sdvr_video_std_e std, int layers)
{
	if (std != SDVR_VIDEO_STD_NONE && 0 < layers && layers <= MAX_NLAYERS)
	{
		m_videoStd = std;
		m_frameSize = sdvr::frameSize(std, SDVR_VIDEO_RES_DECIMATION_EQUAL);
		m_rows = frameHeight() / BLOCK_SIZE;
		m_cols = frameWidth() / BLOCK_SIZE;
		m_layers = layers;
		m_array.fill(0, m_rows * m_cols);
		return true;
	}

	return false;
}

void CRegion::setBlock(int row, int col, int layer, bool enabled)
{
	QByteRef ref = m_array[row * cols() + col];
	if (enabled)
		ref = ref | (1 << layer);
	else
		ref = ref & ~(1 << layer);
}

bool CRegion::block(int row, int col, int layer) const
{
	return (m_array[row * cols() + col] & (1 << layer)) != 0;
}

bool CRegion::migrate(sdvr_video_std_e otherVideoStd)
{
    if (m_videoStd == SDVR_VIDEO_STD_NONE)
        return false;

    if (m_videoStd == otherVideoStd)
        return true;

    CRegion reg(otherVideoStd, m_layers);

    int minRows = qMin(reg.rows(), rows());
    int minCols = qMin(reg.cols(), cols());

    for (int x = 0; x < minCols; ++x)
        for (int y = 0; y < minRows; ++y)
            for (int n = 0; n < m_layers; ++n)
                reg.setBlock(y, x, n, block(y, x, n));

    *this = reg;
    return true;
}

bool CRegion::isEmpty() const
{
	if (videoStd() != SDVR_VIDEO_STD_NONE)
		foreach (uchar ch, m_array)
			if (ch != 0)
				return false;

	return true;
}

QDomElement CRegion::toElement(QDomDocument &doc)
{
    QDomElement regElem = doc.createElement(TN_REGION);
	regElem.setAttribute(AN_REGVIDEOSTD, sdvr::videoStandardToString(videoStd()));
    regElem.setAttribute(AN_REGLAYERS, layers());

	QString data;
	data.reserve(m_array.size());
	for (int i = 0; i < m_array.size(); ++i)
		data.append(QString::number(m_array[i], 16));
	regElem.appendChild(doc.createTextNode(data));

	return regElem;
}

bool CRegion::fromElement(QDomElement e)
{
	// Modify the region only in case if there are no errors
	sdvr_video_std_e std = sdvr::stringToVideoStandard(e.attribute(AN_REGVIDEOSTD));
	int n_layers = e.attribute(AN_REGLAYERS).toInt();
	if (std == SDVR_VIDEO_STD_NONE || n_layers == 0)
    {
        Log::error("Bad region attributes. Region ignored.");
        return false;
    }

	QString data(e.text());
	QSize fs = sdvr::frameSize(std, SDVR_VIDEO_RES_DECIMATION_EQUAL);

	if (data.size() != (fs.height() / BLOCK_SIZE) * (fs.width() / BLOCK_SIZE))
    {
        Log::error("Region data size mismatch. Region ignored.");
        return false;
    }

	// Verify data
	for (int i = 0; i < data.size(); ++i)
	{
		QChar ch = data[i].toUpper();
		if (!ch.isDigit() && (ch < 'A' || ch > 'F'))
		{
			Log::error("Illegal region data. Hexadecimal digits expected. "
					   "Region ignored");
			return false;
		}
	}

	init(std, n_layers);

	for (int i = 0; i < m_array.size(); ++i)
	{
		bool ok;
		m_array[i] = QString(data[i]).toUInt(&ok, 16);
	}

	return true;
}
