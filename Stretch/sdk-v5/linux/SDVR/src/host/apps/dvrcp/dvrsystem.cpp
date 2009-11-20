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

#include "dvrsystem.h"
#include "dvrboard.h"
#include "camera.h"
#include "player.h"
#include "spotmonitor.h"
#include "viewsettings.h"
#include "sdvr.h"
#include "log.h"
#include "recview.h"
#include "mainwindow.h"
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QApplication>

// Registry keys
static const char *KEY_STARTUP_SYSTEM = "startup_system";
static const char *KEY_STARTUP_SYSTEM_LIST = "startup_system_%1";
static const char *KEY_FIRMWARE_PATH = "firmware_path";
static const char *KEY_FIRMWARE_PATH_LIST = "firmware_path_%1";

//static const char *KEY_LOAD_FW_ON_STARTUP = "load_fw_on_startup";

// XML tag names
static const char *TN_DOC = "dvrsystem";
static const char *TN_LASTID = "lastid";
static const char *TN_ENABLE_AUTH_KEY = "enable_auth_key";
static const char *TN_USE_SDK_RECORDING = "usesdkrecording";
static const char *TN_LASTPLAYERID = "lastplayerid";
static const char *TN_SYSNAME = "name";
static const char *TN_VIDEOSTANDARD = "videostandard";
static const char *TN_WATCHDOGSTATE = "watchdogstate";
static const char *TN_WATCHDOGTIMEOUT = "watchdogtimeout";
static const char *TN_UPDATETIMEINTERVAL = "update_time_interval";
static const char *TN_TIMEOUT = "timeout";
static const char *TN_ENABLE_H264_SCE = "enableh264sce";
static const char *TN_AUDIO_RATE = "audio_rate";

static const char *TN_H264MAXFRAMESRECORD = "h264maxframesrecord";
static const char *TN_SEGMENTED_RECORING = "segmentedrecording";
static const char *TN_MJPEG_REC_STYLE_AVI = "mjpeg_rec_style_avi";
static const char *TN_SKIP_RECORING = "skipdrecording";

static const char *TN_DEBUG_AV_FRAMES = "debugavframes";
static const char *TN_DEBUG_SDK = "debugsdkenable";
static const char *TN_DEBUG_ENABLE = "debugenable";
static const char *TN_DEBUG_SHOW_DROP_FRAMES = "debugshowdropframes";
static const char *TN_EXTENDED_FEATURES = "extended_features";
static const char *TN_STATVIEWINFO = "statviewinfo";
static const char *TN_STAT_FPS_INTERVAL= "statfpsinterval";
static const char *TN_CAMERALIST = "cameralist";
static const char *TN_CAMERA = "camera";
static const char *TN_PLAYERLIST = "playerlist";
static const char *TN_PLAYER = "player";
static const char *TN_MONITORLIST = "monitorlist";
static const char *TN_FONTTABLE = "fonttable";
static const char *TN_FONTPATH = "fontpath";
static const char *TN_FONTCODERANGE= "fontcoderange";
static const char *TN_FONTCODELOW= "low";
static const char *TN_FONTCODEHIGH= "high";
static const char *TN_FONTCOLOR= "fontcolor";
static const char *TN_FONTCOLORY= "y";
static const char *TN_FONTCOLORU= "u";
static const char *TN_FONTCOLORV= "v";
static const char *TN_FONTTABLE_ACTIVE ="fonttableactive";

DVRSystem::DVRSystem()
    : m_alarmHandler(this), m_sensorsHandler(this), m_signalsHandler(this)
{
    setVideoStandard(SDVR_VIDEO_STD_NONE);
    setDeferredVideoStandard(SDVR_VIDEO_STD_NONE);
    m_lastHighVSTD_FPS = 0;
    m_bUseSDKRecording = false;
    m_bEnableAuthKey = false;
    m_bExtendedFeatures = false;

    setWatchDogState(0);
    setWatchDogTimeout(10);
    setUpdateTimeInterval(5);

    setH264MaxFramesRecord(0);
    setDebugEnable(true);
    setDebugSDK(true);
    setShowDropFrames(false);
    setStatViewInfo(RecView::REC_STATUS | RecView::BITRATE);

    // By default Start Code Encoded is on to be done
    // on the host side.
    m_bH264SCE = true;

    m_audioRate = SDVR_AUDIO_RATE_8KHZ;
    m_deferredAudioRate = SDVR_AUDIO_RATE_NONE;

    m_bSystemClosing = false;

    setActiveFontTable(0);  // Use the default font table.
    setFontTablePath("");
    setFontTableRange(0,65536);
    setFontColor(255,128,128);

    // By default we don't debug A/V frames. It uses lots of
    // harddisk space
    setDebugAVFrames(false);
    m_lastId = 0;
    m_lastPlayerId = 0;


    // The default Frame Rate calculation sampling is 3 seconds
    m_statFPSInterval = 3;

    // The default PCI driver timeout is 10 seconds
    m_timeOut = 10;

    memset(&m_fwVersion, 0 , sizeof(m_fwVersion));

    setLoadFirmwareState(false);

    m_bSegmentedRecording = false;

    m_bSkipRecording = false;
    m_bMJPEGRecStyleAVI = true;
}

DVRSystem::~DVRSystem()
{
    if (m_recorder.isRunning())
    {
        m_recorder.terminate();
        m_recorder.wait();
    }

    qDeleteAll(monitorList);
    qDeleteAll(cameraList);
    qDeleteAll(m_playerList);
    qDeleteAll(smoList);
    qDeleteAll(boardList);
}

void DVRSystem::getFWVersion(sdvr_firmware_ver_t *version)
{
    if (version)
    {
        memcpy(version, &m_fwVersion, sizeof(sdvr_firmware_ver_t));
    }
}

sdvr_err_e DVRSystem::setup()
{
    sdvr_firmware_ver_t fwVersion;
    QString startupFirmware = DVRSystem::getStartupFirmwarePath();
    sdvr_err_e rc;

    // We must load the new firmware if a firmware path is specified
    setLoadFirmwareState(!startupFirmware.isEmpty());

    m_availableVideoStandards = 0;

    int totalSmoCount = 0;
    int boardCount = sdvr_get_board_count();
    for (int index = 0; index < boardCount; ++index)
    {
        DVRBoard *board = new DVRBoard(index);
        sdvr_board_attrib_t attrib;
        sdvr_pci_attrib_t PCIAttrib;

        // Check to see if a startup firmware was specified to load,
        // if so we must load this firmware to all the DVR boards.
        while (isLoadFirmware())
        {
            QByteArray baFWPath(startupFirmware.toAscii());
            if ((rc = sdvr_upgrade_firmware(index, baFWPath.data())) != SDVR_ERR_NONE)
            {
                int ret = QMessageBox::critical(NULL, "Stretch-DVRCP - Initialization Error",
                QString("DVRCP failed to load the specified firmware %1. Error Code [%2-%3].\nDo you want to specify a new firmware?")
                .arg(startupFirmware).arg(sdvr_get_error_text(rc)).arg((int)rc),QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes)
                {
                    startupFirmware = QFileDialog::getOpenFileName(NULL,
                        "Select a Firmware to load", startupFirmware,
                        "*.rom;;*.*");

                    if (startupFirmware.isEmpty())
                        exit(1);
                    else
                        DVRSystem::setStartupFirmwarePath(startupFirmware);
                }
                else
                    exit(1);
            }
            else
                break;

        }

        g_MainWindow->setStatusBar(QString("Getting board attributes for board #%1...").arg(index+1));
        rc = sdvr_get_board_attributes(index, &attrib);
        if (rc != SDVR_ERR_NONE)
        {
            delete board;
            return rc;
        }
        board->setAttrib(&attrib);

        g_MainWindow->setStatusBar(QString("Getting PCI attributes for board #%1...").arg(index+1));
        rc = sdvr_get_pci_attrib(index, &PCIAttrib);
        if (rc != SDVR_ERR_NONE)
        {
            delete board;
            return rc;
        }
        board->setPCIAttrib(&PCIAttrib);
        board->setSerialNo((char *)&PCIAttrib.serial_number[0]);

        g_MainWindow->setStatusBar(QString("Getting configuration for board #%1...").arg(index+1));
        sdvr_board_config_t config;
        rc = sdvr_get_board_config(index, &config);
        if (rc != SDVR_ERR_NONE)
        {
            delete board;
            return rc;
        }

        g_MainWindow->setStatusBar(QString("Getting firmware version for board #%1...").arg(index+1));
        memset(&fwVersion, 0, sizeof(fwVersion));
        sdvr_get_firmware_version_ex(index, &fwVersion);
        board->setFWVersion(&fwVersion);

        if (index == 0)
            m_availableVideoStandards = attrib.supported_video_stds;
        else
            m_availableVideoStandards &= attrib.supported_video_stds;

        board->setConfig(&config);

        boardList.append(board);

        for (int i = 0; i < board->smoCount(); ++i)
        {
            SpotMonitor *smo = new SpotMonitor(board, i + 1);
            smo->setName(QString("Spot Monitor %1").arg(++totalSmoCount));
            smoList.append(smo);
        }
    }

    // We already loaded the firmware there is no need to load
    // one on the connect
    setLoadFirmwareState(false);

    m_recorder.start();

    return SDVR_ERR_NONE;
}

DVRBoard *DVRSystem::board(int index) const
{
    if (0 <= index && index < boardCount())
        return boardList[index];

    return NULL;
}

bool DVRSystem::open(const QString &configFilePath, bool bConnect)
{
    QFile file(configFilePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        Log::error(QString("Error opening file %1").arg(configFilePath));
        return false;
    }

    QDomDocument doc("sdvr");

    QString msg;
    int errorLine = 0, errorColumn = 0;
    if (!doc.setContent(&file, &msg, &errorLine, &errorColumn))
    {
        Log::error(QString("%1 at line %2 column %3").arg(msg)
                          .arg(errorLine).arg(errorColumn));
        file.close();
        return false;
    }

    file.close();

    fromElement(doc.documentElement());

    if (videoStandard() == SDVR_VIDEO_STD_NONE)
    {
        Log::error(QString("video standard definition is missing in the configuration file."));
        return false;
    }

    if (deferredVideoStandard() != SDVR_VIDEO_STD_NONE)
    {
        setVideoStandard(deferredVideoStandard());
        setDeferredVideoStandard(SDVR_VIDEO_STD_NONE);
    }

    if (deferredAudioRate() != SDVR_AUDIO_RATE_NONE)
    {
        setAudioRate(deferredAudioRate());
        setDeferredAudioRate(SDVR_AUDIO_RATE_NONE);
    }

    setConfigFilePath(configFilePath);
    if (bConnect)
        return open();
    return true;
}

bool DVRSystem::open()
{
    QString startupFirmware = DVRSystem::getStartupFirmwarePath();
    sdvr_err_e rc;
    sdvr_firmware_ver_t fwVersion;
    // Connect to each board
    int bc = boardCount();

    for (int index = 0; index < bc; ++index)
    {
        if (isLoadFirmware())
        {
            QByteArray baFWPath(startupFirmware.toAscii());
            if ((rc = sdvr_upgrade_firmware(index, baFWPath.data())) != SDVR_ERR_NONE)
            {
                sdvr_get_firmware_version_ex(index, &m_fwVersion);
                Log::error(QString("Failed to load the firmware to board #%1. [Error = %2]").
                    arg(index+1).arg(sdvr_get_error_text(rc)));
                Log::error(QString("Boot-loader Versions: %1.%2 BSP Versions: %3.%4").
                    arg(m_fwVersion.bootloader_major).arg(m_fwVersion.bootloader_minor).
                    arg(m_fwVersion.bsp_major).arg(m_fwVersion.bsp_minor));
                return false;
            }
        }

        // Enable/Disable authentication key based on the user request.
        sdvr_enable_auth_key(isEnableAuthKey());

        sdvr_board_settings_t board_settings;
        board_settings.video_std = videoStandard();
        board_settings.is_h264_SCE = m_bH264SCE;
        board_settings.audio_rate = m_audioRate;

        rc = sdvr_board_connect_ex(index, &board_settings);
        // If the video standard was changed, allow the user
        // to select a new video standard and try to connect again
        if (rc == SDVR_FRMW_ERR_WRONG_CAMERA_TYPE)
        {
            if (promptVideoStandard())
            {
                board_settings.video_std = videoStandard();
                rc = sdvr_board_connect_ex(index, &board_settings);
            }
        }
        if (rc != SDVR_ERR_NONE)
        {
            Log::error(QString("Failed to connect to board %1 [Error = %2]")
                       .arg(index + 1).arg(sdvr_get_error_text(rc)));

            // Disconnect all connected boards
            while (--index >= 0)
                sdvr_board_disconnect(index);

            return false;
        }

        // Save the version information for all the connected boards
        // This is needed to display in the board information dialogbox
        if (rc == SDVR_ERR_NONE)
        {
            memset(&fwVersion, 0, sizeof(fwVersion));
            sdvr_get_firmware_version_ex(index, &fwVersion);
            board(index)->setFWVersion(&fwVersion);
        }

        board(index)->updateDateTime();
    }

    // Save the firmware version for backward compatibility feature
    if ((rc = sdvr_get_firmware_version_ex(0, &m_fwVersion)) != SDVR_ERR_NONE)
    {
        Log::error(QString("Failed to read the firmware version on board 1. [Error = %1]")
                   .arg(sdvr_get_error_text(rc)));

    }

    // Connect cameras
    QList<Camera *>::iterator ci;
    for (ci = cameraList.begin(); ci != cameraList.end();)
    {
        // Make sure all the cameras with the highest FPS keep
        // the same relative FPS if video standard was changed in the config file.
        (*ci)->setFrameRate(SDVR_ENC_PRIMARY,sdvr::verifyFrameRate(m_videoStandard, (*ci)->frameRate(SDVR_ENC_PRIMARY)));
        (*ci)->setFrameRate(SDVR_ENC_SECONDARY,sdvr::verifyFrameRate(m_videoStandard, (*ci)->frameRate(SDVR_ENC_SECONDARY)));

        (*ci)->setOnAlarmFrameRate(SDVR_ENC_PRIMARY,sdvr::verifyFrameRate(m_videoStandard, (*ci)->onAlarmFrameRate(SDVR_ENC_PRIMARY)));
        (*ci)->setOnAlarmFrameRate(SDVR_ENC_SECONDARY,sdvr::verifyFrameRate(m_videoStandard, (*ci)->onAlarmFrameRate(SDVR_ENC_SECONDARY)));

        (*ci)->setYUVFrameRate(sdvr::verifyFrameRate(m_videoStandard, (*ci)->getYUVFrameRate()));

        if ((*ci)->isAttached() && !(*ci)->openChannel())
        {
            Log::info(QString("Error opening channel for camera '%1'").arg((*ci)->name()));
            // If there was an error with connecting the camera,
            // detatch the camera but keep it.
            deleteCamera((*ci)->id(),false);
            (*ci)->detach();
        }
        else
        {
            if ((*ci)->isAttached())
                (*ci)->applyAllParameters();
             ++ci;
        }
    }

    // Connect decoders
    // NOTE: Decoders must be created after encoders

    QList<CPlayer *>::iterator pi;
    for (pi = m_playerList.begin(); pi != m_playerList.end(); )
    {
        if ((*pi)->isAttached() && !(*pi)->openChannel())
        {
            Log::info(QString("Error opening channel for decoder '%1'").arg((*pi)->getName()));
            // If there was an error with connecting the player,
            // detatch the player but keep it.
            deletePlayer((*pi)->getId(),false);
            (*pi)->detach();
        }
        else
            ++pi;
    }
    // After we created all the Camera and Players, then we must
    // resolve the channel handle for any display that was attached
    // to an HMO or SMO montior
    foreach (ViewSettings *vs, monitorList)
    {
        vs->setChannels();
    }

    return true;
}

/*
   This function prompt the user to select a new video standard
   since the current one is not supported by the firmware.

*/
bool DVRSystem::promptVideoStandard()
{
    bool ok;
    QStringList supportedVideoStds;
    QString videoStd;
    QString qstrprompt =
        QString("Currently selected video standard %1 is incorrect.\nPlease select a new one.").arg(sdvr::videoStandardToString(videoStandard()));

    for (sdvr_video_std_e std = SDVR_VIDEO_STD_D1_PAL;
        std <= SDVR_VIDEO_STD_4CIF_NTSC; std = (sdvr_video_std_e)(std << 1))
    {
        if (supportsVideoStandard(std))
            supportedVideoStds += sdvr::videoStandardToString(std);
    }

    videoStd = QInputDialog::getItem(NULL,"Video Standard Selection",
        qstrprompt, supportedVideoStds,0,false, &ok);
    if (ok)
    {
        setVideoStandard(sdvr::stringToVideoStandard(videoStd));
    }
    return ok;
}

bool DVRSystem::isOpen() const
{
    return !m_configFilePath.isEmpty();
}

bool DVRSystem::close()
{
    QString startupFirmware = DVRSystem::getStartupFirmwarePath();

    if (!isOpen())
        return false;

    // Signal the recorder thread not to accept any new frames.
    // Also make sure the frame buffer queue is empty.
    // This is needed otherwise it is possible that we get a
    // blue screen due to accessing a memory buffer which
    // destroyed because we are closing all the cameras.
    m_bSystemClosing = true;
    recorder().flushFrameBufQueue();

    save();

    qDeleteAll(monitorList);
    monitorList.clear();

    QList<Camera *>::const_iterator ci;
    for (ci = cameraList.begin(); ci != cameraList.end(); ++ci)
    {
        // Stop all the recordings before closing the encoder channel.
        for (uint subEnc = 0; subEnc < MAX_SUB_ENCODERS; subEnc++)
        {
            if ((*ci)->isRecording(subEnc))
                (*ci)->stopRecording(subEnc);
        }
        (*ci)->closeChannel();
    }

    qDeleteAll(cameraList);
    cameraList.clear();

    QList<CPlayer *>::const_iterator di;
    for (di = m_playerList.begin(); di != m_playerList.end(); ++di)
    {
        if ((*di)->isDecoding())
            (*di)->stopDecoding();

        (*di)->closeChannel();
    }
    qDeleteAll(m_playerList);
    m_playerList.clear();


    int bc = boardCount();

    for (int index = 0; index < bc; ++index)
        sdvr_board_disconnect(index);

    // We must load the new firmware on the next board connect
    // if a startup firmware path is specified
    setLoadFirmwareState(!startupFirmware.isEmpty());

    m_configFilePath.clear();

    m_name.clear();
    setVideoStandard(SDVR_VIDEO_STD_NONE);
    setDeferredVideoStandard(SDVR_VIDEO_STD_NONE);
    setDeferredAudioRate(SDVR_AUDIO_RATE_NONE);
    setWatchDogState(0);
    setWatchDogTimeout(10);
    m_lastId = 0;
    m_lastPlayerId = 0;
    m_bSystemClosing = false;

    return true;
}

QString DVRSystem::systemDirPath() const
{
    if (!isOpen())
        return QString::null;

    return QFileInfo(m_configFilePath).absolutePath();
}

QString DVRSystem::getApplicationPath() const
{
    return g_App->applicationDirPath();
}

//////////////////////////////////////////////////////////////////
// This method return the full path to the default configuration
// file for the requested system configuration.
//
// If the requested system configuration is blank or is invalid,
// it return and empty QString.
//
QString DVRSystem::getDefaultSystemPath(new_system_cngf_e configType) const
{
    QDir appPath;
    QString qstrDefaultSystemPath;
    QString qstrAppPath = getApplicationPath();
    QString qstrDefaultSystemFile;
    QDir qdirAppPath(qstrAppPath);
    QString qstrDefConfigName;

    switch (configType)
    {
    case NEW_SYSTEM_BLANK_CNFG:
        return qstrDefaultSystemPath;

    case NEW_SYSTEM_ENCODER_ONLY_CNFG:
        qstrDefConfigName = "config_enc.def";
        break;
    case NEW_SYSTEM_DECODER_ONLY_CNFG:
        qstrDefConfigName = "config_dec.def";
        break;
    case NEW_SYSTEM_COMBO_CNFG:
        qstrDefConfigName = "config_codec.def";

        break;
    default:
        return qstrDefaultSystemPath;

    }
    qstrDefaultSystemPath = qdirAppPath.absoluteFilePath(qstrDefConfigName);
    if (!QFile::exists(qstrDefaultSystemPath))
        qstrDefaultSystemPath.clear();

    return qstrDefaultSystemPath;
}

Camera *DVRSystem::findCameraById(int id) const
{
    QList<Camera *>::const_iterator i;
    for (i = cameraList.begin(); i != cameraList.end(); ++i)
        if ((*i)->id() == id)
            return *i;

    return NULL;
}

Camera *DVRSystem::findCameraByName(QString name) const
{
    QList<Camera *>::const_iterator i;
    for (i = cameraList.begin(); i != cameraList.end(); ++i)
        if ((*i)->name() == name)
            return *i;

    return NULL;
}

Camera *DVRSystem::findCameraByInput(int boardIndex, int input) const
{
    QList<Camera *>::const_iterator i;
    for (i = cameraList.begin(); i != cameraList.end(); ++i)
        if ((*i)->isAttached() &&
            (*i)->boardIndex() == boardIndex && (*i)->input() == input)
            return *i;

    return NULL;
}

Camera *DVRSystem::findCameraByHandle(sdvr_chan_handle_t handle) const
{
    QList<Camera *>::const_iterator i;
    for (i = cameraList.begin(); i != cameraList.end(); ++i)
        if ((*i)->isChannelOpen() && (*i)->handle() == handle)
            return *i;

    return NULL;
}

CPlayer *DVRSystem::findDecoderById(int id) const
{
    QList<CPlayer *>::const_iterator i;
    for (i = m_playerList.begin(); i != m_playerList.end(); ++i)
        if ((*i)->getId() == id)
            return *i;

    return NULL;
}

CPlayer *DVRSystem::findDecoderByChanNum(int boardIndex, int chanNum) const
{
    QList<CPlayer *>::const_iterator i;
    for (i = m_playerList.begin(); i != m_playerList.end(); ++i)
        if ((*i)->isAttached() &&
            (*i)->getBoardIndex() == boardIndex && (*i)->getChanNum() == chanNum)
            return *i;

    return NULL;
}

CPlayer *DVRSystem::findDecoderByHandle(sdvr_chan_handle_t handle) const
{
    QList<CPlayer *>::const_iterator i;
    for (i = m_playerList.begin(); i != m_playerList.end(); ++i)
        if ((*i)->isChannelOpen() && (*i)->handle() == handle)
            return *i;

    return NULL;
}

ViewSettings *DVRSystem::findMonitorByName(const QString &name)
{
    QList<ViewSettings *>::const_iterator i;
    for (i = monitorList.begin(); i != monitorList.end(); ++i)
        if ((*i)->name() == name)
            return *i;

    return NULL;
}

QString DVRSystem::getNameFromHandle(sdvr_chan_handle_t handle)
{
    sdvr_chan_type_e chan_type = sdvr_get_chan_type(handle);
    QString name = "null";

    switch (chan_type)
    {
    case SDVR_CHAN_TYPE_DECODER:
    {
        CPlayer *player = findDecoderByHandle(handle);
        if (player != NULL)
        {
            name = player->getName();
        }
        break;
    }
    case SDVR_CHAN_TYPE_NONE:
    case SDVR_CHAN_TYPE_ENCODER:
    {
        Camera *cam = findCameraByHandle(handle);
        if (cam != NULL)
        {
            name = cam->name();
        }
        break;
    }
    default:
        break;
    }
    return name;
}

int DVRSystem::getIdFromChanHandle(sdvr_chan_handle_t handle)
{
    int id = -1;
    CPlayer *player;
    Camera *cam;

    switch (sdvr_get_chan_type(handle))
    {
    case SDVR_CHAN_TYPE_DECODER:
        player = findDecoderByHandle(handle);
        if (player != NULL)
        {
            id = player->getId();
        }
        break;
    case SDVR_CHAN_TYPE_ENCODER:
    case SDVR_CHAN_TYPE_NONE:
        cam = findCameraByHandle(handle);
        if (cam != NULL)
        {
            id = cam->id();
        }
        break;
    default:
        id = -1;
        break;
    }
    return id;
}

bool DVRSystem::attachCamera(Camera *cam, int boardIndex, int input)
{
    if (findCameraByInput(boardIndex, input) == NULL)
        return cam->attach(boardIndex, input);

    return false;
}

bool DVRSystem::deleteCamera(int camId, bool bRemoveFromList)
{
    Camera *cam = findCameraById(camId);
    if (cam == NULL)
        return false;

    removeChannelFromMonitors(cam->handle());

    // Stop all the recordings before closing the encoder channel.
    for (uint subEnc = 0; subEnc < MAX_SUB_ENCODERS; subEnc++)
    {
        if (cam->isRecording(subEnc))
            cam->stopRecording(subEnc);
    }

    cam->closeChannel();

    if (bRemoveFromList)
    {
        cameraList.removeAll(cam);
        delete cam;
    }

    return true;
}

bool DVRSystem::deletePlayer(int Id, bool bRemoveFromList)
{
    CPlayer *player = findDecoderById(Id);
    if (player == NULL)
        return false;

    removeChannelFromMonitors(player->handle());

    if (player->isDecoding())
        player->stopDecoding();

    player->closeChannel();

    if (bRemoveFromList)
    {
        m_playerList.removeAll(player);
        delete player;
    }

    return true;
}

void DVRSystem::removeChannelFromMonitors(sdvr_chan_handle_t chan)
{
    foreach (ViewSettings *vs, monitorList)
    {
        int count = vs->displayCount();
        for (int i = 0; i < count; ++i)
            if (vs->displayChannel(i) == chan)
                vs->setDisplayChannel(i, INVALID_CHAN_HANDLE);
    }
}

QVector<int> DVRSystem::availableVideoInputs(int boardIndex)
{
    QVector<int> inputs;
    DVRBoard *b = board(boardIndex);
    if (b != NULL)
    {
        int videoInCount = b->videoInCount();
        for (int i = 0; i < videoInCount; ++i)
            if (findCameraByInput(boardIndex, i) == NULL)
                inputs.append(i);
    }

    return inputs;
}

QVector<int> DVRSystem::availableBoardDecoders(int boardIndex)
{
    QVector<int> decoders;
    DVRBoard *b = board(boardIndex);
    if (b != NULL)
    {
        int decodersCount = b->decodersCount();
        for (int i = 0; i < decodersCount; ++i)
            if (findDecoderByChanNum(boardIndex, i) == NULL)
                decoders.append(i);
    }
    return decoders;
}

void DVRSystem::setWatchDogState(uint state)
{
    m_watchDogState = state;
    if (isOpen())
    {
    }
}

void DVRSystem::updateSDKSettings(char *debug_file_name)
{
    // Set the new Debug and PCI timeout SDK Parameter.
    sdvr_get_sdk_params(&m_sdk_params);

    m_sdk_params.debug_flag =  DEBUG_FLAG_ALL - DEBUG_FLAG_OUTPUT_TO_SCREEN;

    if (!getDebugEnable())
        m_sdk_params.debug_flag -= DEBUG_FLAG_DEBUGGING_ON;
    else
        m_sdk_params.debug_flag |= DEBUG_FLAG_DEBUGGING_ON;

    if (!getDebugSDK())
        m_sdk_params.debug_flag -= DEBUG_FLAG_GENERAL_SDK;
    else
        m_sdk_params.debug_flag |= DEBUG_FLAG_GENERAL_SDK;

    if (!getDebugAVFrames())
        m_sdk_params.debug_flag -= DEBUG_FLAG_VIDEO_FRAME;
    else
        m_sdk_params.debug_flag |= DEBUG_FLAG_VIDEO_FRAME;

    if (debug_file_name)
        m_sdk_params.debug_file_name = debug_file_name;

    m_sdk_params.timeout = getPCItimeOut();
    sdvr_set_sdk_params(&m_sdk_params);
}

sdvr_err_e DVRSystem::updateOSDFontTable()
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sdvr_font_table_t font_desc;
    QString fontFile = getFontTablePath();
    QByteArray baFontFile(fontFile.toAscii());

    if (!sdvr::isMinVersion(
         m_fwVersion.fw_major, m_fwVersion.fw_minor, m_fwVersion.fw_revision, m_fwVersion.fw_build,
         3, 2, 0, 13))
        return SDVR_ERR_UNSUPPORTED_FIRMWARE;

    if (getActiveFontTable() != 0)
    {
        font_desc.font_file = baFontFile.data();
        font_desc.font_table_id = 8;
        font_desc.font_table_format = SDVR_FT_FORMAT_BDF;
        getFontTableRange(&font_desc.start_font_code,&font_desc.end_font_code);
        getFontColor(&font_desc.color_y,&font_desc.color_u,&font_desc.color_v);

        if (font_desc.font_file)
            err = sdvr_osd_set_font_table(&font_desc);
    }
    else
    {
        // set the active font table to be default on the firmware.
        err = sdvr_osd_use_font_table(0);
    }
    return err;
}



bool DVRSystem::supportsVideoStandard(sdvr_video_std_e std) const
{
    return (m_availableVideoStandards & std) != 0;
}

SpotMonitor *DVRSystem::spotMonitor(int boardIndex, int port) const
{
    QList<SpotMonitor *>::const_iterator i;
    for (i = smoList.begin(); i != smoList.end(); ++i)
        if ((*i)->board()->index() == boardIndex && (*i)->port() == port)
            return *i;

    return NULL;
}

bool DVRSystem::isActiveSmo(ViewSettings *vs)
{
    if (vs != NULL)
    {
        QList<SpotMonitor *>::const_iterator i;
        for (i = smoList.begin(); i != smoList.end(); ++i)
            if ((*i)->view() == vs)
                return true;
    }

    return false;
}

bool DVRSystem::fromElement(QDomElement docElem)
{
    bool ok = true;

    if (docElem.tagName() == TN_DOC)
    {
        QDomElement e = docElem.firstChildElement();
        while (!e.isNull())
        {
            if (e.tagName() == TN_SYSNAME)
            {
                setName(e.text());
            }
            else if (e.tagName() == TN_ENABLE_AUTH_KEY)
            {
                m_bEnableAuthKey = QVariant(e.text()).toBool();
            }
            else if (e.tagName() == TN_EXTENDED_FEATURES)
            {
                m_bExtendedFeatures = QVariant(e.text()).toBool();
            }
            else if (e.tagName() == TN_USE_SDK_RECORDING)
            {
                m_bUseSDKRecording = QVariant(e.text()).toBool();
            }
            else if (e.tagName() == TN_VIDEOSTANDARD)
            {
                sdvr_video_std_e vs = sdvr::stringToVideoStandard(e.text());
                if (vs == SDVR_VIDEO_STD_NONE)
                {
                    Log::error(QString("Config file err: Unknown video standard '%1'. Default to D1 NTSC.").arg(e.text()));
                    vs = SDVR_VIDEO_STD_D1_NTSC;
                }

                setVideoStandard(vs);
                m_lastHighVSTD_FPS = sdvr::frameRate(vs);
            }
            else if (e.tagName() == TN_LASTID)
            {
                bool converted;
                m_lastId = e.text().toInt(&converted);
                if (!converted)
                {
                    Log::error("Config file err: Config file err: Illegal [lastid]");
                    ok = false;
                }
            }
            else if (e.tagName() == TN_LASTPLAYERID)
            {
                bool converted;
                m_lastPlayerId = e.text().toInt(&converted);
                if (!converted)
                {
                    Log::error("Config file err: Config file err: Illegal [lastdecodeid]");
                    ok = false;
                }
            }
            else if (e.tagName() == TN_WATCHDOGSTATE)
            {
                bool converted;
                m_watchDogState = e.text().toInt(&converted);
                if (!converted)
                {
                    m_watchDogState = 0;
                }
            }
            else if (e.tagName() == TN_WATCHDOGTIMEOUT)
            {
                bool converted;
                m_watchDogTimeout = e.text().toInt(&converted);
                if (!converted || m_watchDogTimeout == 0)
                {
                    m_watchDogTimeout = 10;
                }
            }
            else if (e.tagName() == TN_UPDATETIMEINTERVAL)
            {
                bool converted;
                m_updateTimeInterval = e.text().toInt(&converted);
                if (!converted || m_updateTimeInterval == 0)
                {
                    m_updateTimeInterval = 5;
                }

            }
            else if (e.tagName() == TN_STAT_FPS_INTERVAL)
            {
                bool converted;
                m_statFPSInterval = e.text().toInt(&converted);
                // The default Frame Rate calculation sampling is 3 seconds
                if (m_statFPSInterval == 0 || !converted)
                    m_statFPSInterval = 3;
            }
            else if (e.tagName() == TN_TIMEOUT)
            {
                bool converted;
                m_timeOut = e.text().toInt(&converted);
                if (!converted)
                {
                    m_timeOut = 5;
                }
            }
            else if (e.tagName() == TN_H264MAXFRAMESRECORD)
            {
                bool converted;
                m_H264MaxFramesRecord = e.text().toInt(&converted);
                if (!converted)
                {
                    m_H264MaxFramesRecord = 0;
                }
            }
            else if (e.tagName() == TN_SEGMENTED_RECORING)
            {
                m_bSegmentedRecording = QVariant(e.text()).toBool();
            }
            else if (e.tagName() == TN_SKIP_RECORING)
            {
                m_bSkipRecording = QVariant(e.text()).toBool();
            }
            else if (e.tagName() == TN_MJPEG_REC_STYLE_AVI)
            {
                m_bMJPEGRecStyleAVI = QVariant(e.text()).toBool();
            }
            else if (e.tagName() == TN_STATVIEWINFO)
            {
                bool converted;
                m_statViewInfo = e.text().toInt(&converted);
                if (!converted)
                {
                    setStatViewInfo(RecView::REC_STATUS | RecView::BITRATE);
                }
            }
            else if (e.tagName() == TN_DEBUG_ENABLE)
            {
                bool converted;
                m_bDebugEnable = (e.text().toInt(&converted) != 0);
                if (!converted)
                {
                    m_bDebugEnable = false;
                }
            }
            else if (e.tagName() == TN_ENABLE_H264_SCE)
            {
                bool converted;
                m_bH264SCE = (e.text().toInt(&converted) != 0);
                if (!converted)
                {
                    m_bH264SCE = false;
                }
            }
            else if (e.tagName() == TN_AUDIO_RATE)
            {
                m_audioRate = sdvr::stringToAudioRate(e.text());
                if (m_audioRate == SDVR_AUDIO_RATE_NONE)
                {
                    m_audioRate = SDVR_AUDIO_RATE_8KHZ;
                }
            }

            else if (e.tagName() == TN_DEBUG_SDK)
            {
                bool converted;
                m_bDebugSDK = (e.text().toInt(&converted) != 0);
                if (!converted)
                {
                    m_bDebugSDK = false;
                }
            }
            else if (e.tagName() == TN_DEBUG_SHOW_DROP_FRAMES)
            {
                bool converted;
                m_bShowDropFrames = (e.text().toInt(&converted) != 0);
                if (!converted)
                {
                    m_bShowDropFrames = false;
                }
            }
            else if (e.tagName() == TN_DEBUG_AV_FRAMES)
            {
                bool converted;
                m_bDebugAVFrames = (e.text().toInt(&converted) != 0);
                if (!converted)
                {
                    m_bDebugAVFrames = false;
                }
            }
            else if (e.tagName() == TN_FONTTABLE_ACTIVE)
            {
                bool converted;
                m_activeFontTable = e.text().toInt(&converted);
                if (!converted)
                {
                    m_activeFontTable = 0;
                }
            }
            else if (e.tagName() == TN_FONTTABLE)
            {
                bool converted;
                QDomElement fontElem = e.firstChildElement();
                while (!fontElem.isNull())
                {
                    if (fontElem.tagName() == TN_FONTPATH)
                    {
                        QString path = fontElem.text();
                        setFontTablePath(path);
                    }
                    else if (fontElem.tagName() == TN_FONTCODERANGE)
                    {
                        uint lowCode = fontElem.attribute(TN_FONTCODELOW).toInt(&converted);
                        if (!converted)
                            lowCode = 0;
                        uint highCode = fontElem.attribute(TN_FONTCODEHIGH).toInt(&converted);
                        if (!converted || highCode > 65536)
                            highCode = 65536;
                        setFontTableRange(lowCode,highCode);
                    }
                    else if (fontElem.tagName() == TN_FONTCOLOR)
                    {
                        uint yColor = fontElem.attribute(TN_FONTCOLORY).toInt(&converted);
                        if (!converted || yColor > 255)
                            yColor = 255;
                        uint uColor = fontElem.attribute(TN_FONTCOLORU).toInt(&converted);
                        if (!converted || uColor > 255)
                            uColor = 126;
                        uint vColor = fontElem.attribute(TN_FONTCOLORV).toInt(&converted);
                        if (!converted || vColor > 255)
                            vColor = 126;
                        setFontColor(yColor,uColor, vColor);
                    }
                    fontElem = fontElem.nextSiblingElement();
                } //while (!fontElem.isNull())
            }
            else if (e.tagName() == TN_CAMERALIST)
            {
                if (!camerasFromElement(e))
                    ok = false;
            }
            else if (e.tagName() == TN_PLAYERLIST)
            {
                if (!decodersFromElement(e))
                    ok = false;
            }
            else if (e.tagName() == TN_MONITORLIST)
            {
                if (!monitorsFromElement(e))
                    ok = false;
            }

            e = e.nextSiblingElement();
        }
    }
    else
    {
        Log::info(QString("Config file err: Unknown top level element '%1'").arg(docElem.tagName()));
        ok = false;
    }

    return ok;
}

bool DVRSystem::camerasFromElement(QDomElement camListElem)
{
    bool ok = true;

    QDomElement e = camListElem.firstChildElement();
    while (!e.isNull())
    {
        if (e.tagName() == TN_CAMERA)
        {
            Camera *cam = new Camera();
            if (!cam->fromElement(e))
                ok = false;

            cameraList.append(cam);
        }
        else
        {
            Log::info(QString("Unexpected element '%1' in %2 is ignored")
                      .arg(e.tagName()).arg(camListElem.tagName()));
        }

        e = e.nextSiblingElement();
    }

    return ok;
}
bool DVRSystem::decodersFromElement(QDomElement decListElem)
{
    bool ok = true;

    QDomElement e = decListElem.firstChildElement();
    while (!e.isNull())
    {
        if (e.tagName() == TN_PLAYER)
        {
            CPlayer *decoder = new CPlayer();
            if (!decoder->fromElement(e))
                ok = false;

            m_playerList.append(decoder);
        }
        else
        {
            Log::info(QString("Unexpected element '%1' in %2 is ignored")
                      .arg(e.tagName()).arg(decListElem.tagName()));
        }

        e = e.nextSiblingElement();
    }

    return ok;
}

bool DVRSystem::monitorsFromElement(QDomElement monitorListElem)
{
    bool ok = true;

    QDomElement e = monitorListElem.firstChildElement();
    while (!e.isNull())
    {
        ViewSettings *vs = ViewSettings::fromElement(e);
        if (vs == NULL)
            ok = false;
        else
            monitorList.append(vs);

        e = e.nextSiblingElement();
    }

    return ok;
}

bool DVRSystem::save()
{
    if (!isOpen())
        return false;

    g_MainWindow->setStatusBar("Saving the configuration file...");

    QString qstrBackupFile = QString("%1.bak").arg(m_configFilePath);
    QFile file(m_configFilePath);

    // Backup the config file before overwriting it.
    if (QFile::exists(qstrBackupFile))
        QFile::remove(qstrBackupFile);
    QFile::rename(m_configFilePath, qstrBackupFile);

    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        Log::error("Could not open system configuration file for writing");
        return false;
    }

    QTextStream ts(&file);
    save(ts);
    file.close();

    g_MainWindow->setStatusBar("Ready");
    return true;
}

bool DVRSystem::save(QTextStream & s)
{
    QDomImplementation domImpl;
    QDomDocument doc(domImpl.createDocumentType("sdvr", QString(), "sdvr.dtd"));
    doc.appendChild(doc.createProcessingInstruction("xml",
            "version='1.0' encoding='ISO-8859-1'"));
    doc.appendChild(toElement(doc));
    s << doc << endl;
    return true;
}

QDomElement DVRSystem::toElement(QDomDocument &doc)
{
    QDomElement docElem = doc.createElement(TN_DOC);

    QDomElement lastIdElem = doc.createElement(TN_LASTID);
    lastIdElem.appendChild(doc.createTextNode(QString::number(m_lastId)));
    docElem.appendChild(lastIdElem);

    QDomElement useNewRercordingElem = doc.createElement(TN_USE_SDK_RECORDING);
    useNewRercordingElem.appendChild(doc.createTextNode(QVariant(m_bUseSDKRecording).toString()));
    docElem.appendChild(useNewRercordingElem);

    QDomElement enableAuthKeyElem = doc.createElement(TN_ENABLE_AUTH_KEY);
    enableAuthKeyElem.appendChild(doc.createTextNode(QVariant(m_bEnableAuthKey).toString()));
    docElem.appendChild(enableAuthKeyElem);

    QDomElement extendedFeaturesElem = doc.createElement(TN_EXTENDED_FEATURES);
    extendedFeaturesElem.appendChild(doc.createTextNode(QVariant(m_bExtendedFeatures).toString()));
    docElem.appendChild(extendedFeaturesElem);

    QDomElement lastDecodeIdElem = doc.createElement(TN_LASTPLAYERID);
    lastDecodeIdElem.appendChild(doc.createTextNode(QString::number(m_lastPlayerId)));
    docElem.appendChild(lastDecodeIdElem);

    if (!name().isEmpty())
    {
        QDomElement nameElem = doc.createElement(TN_SYSNAME);
        nameElem.appendChild(doc.createTextNode(name()));
        docElem.appendChild(nameElem);
    }

    // Save the deferred video standard, if set
    sdvr_video_std_e std = deferredVideoStandard();
    if (std == SDVR_VIDEO_STD_NONE)
        std = videoStandard();

    if (std != SDVR_VIDEO_STD_NONE)
    {
        QDomElement vstdElem = doc.createElement(TN_VIDEOSTANDARD);
        QDomText vstdText = doc.createTextNode(sdvr::videoStandardToString(std));
        vstdElem.appendChild(vstdText);
        docElem.appendChild(vstdElem);
    }

    QDomElement wdsElem = doc.createElement(TN_WATCHDOGSTATE);
    wdsElem.appendChild(doc.createTextNode(QString::number(m_watchDogState)));
    docElem.appendChild(wdsElem);

    QDomElement wdTimeoutElem = doc.createElement(TN_WATCHDOGTIMEOUT);
    wdTimeoutElem.appendChild(doc.createTextNode(QString::number(m_watchDogTimeout)));
    docElem.appendChild(wdTimeoutElem);

    QDomElement updateTimeIntervalElem = doc.createElement(TN_UPDATETIMEINTERVAL);
    updateTimeIntervalElem.appendChild(doc.createTextNode(QString::number(m_updateTimeInterval)));
    docElem.appendChild(updateTimeIntervalElem);

    QDomElement wfpsIntervalElem = doc.createElement(TN_STAT_FPS_INTERVAL);
    wfpsIntervalElem.appendChild(doc.createTextNode(QString::number(m_statFPSInterval)));
    docElem.appendChild(wfpsIntervalElem);

    QDomElement timeOutElem = doc.createElement(TN_TIMEOUT);
    timeOutElem.appendChild(doc.createTextNode(QString::number(m_timeOut)));
    docElem.appendChild(timeOutElem);

    QDomElement h264MaxFramesRecordElem = doc.createElement(TN_H264MAXFRAMESRECORD);
    h264MaxFramesRecordElem.appendChild(doc.createTextNode(QString::number(m_H264MaxFramesRecord)));
    docElem.appendChild(h264MaxFramesRecordElem);

    QDomElement  segmentedRercordingElem = doc.createElement(TN_SEGMENTED_RECORING);
    segmentedRercordingElem.appendChild(doc.createTextNode(QVariant(m_bSegmentedRecording).toString()));
    docElem.appendChild(segmentedRercordingElem);

    QDomElement SkipRecordingElem = doc.createElement(TN_SKIP_RECORING);
    SkipRecordingElem.appendChild(doc.createTextNode(QVariant(m_bSkipRecording).toString()));
    docElem.appendChild(SkipRecordingElem);

    QDomElement MJPEGRecStyleAVIElem = doc.createElement(TN_MJPEG_REC_STYLE_AVI);
    MJPEGRecStyleAVIElem.appendChild(doc.createTextNode(QVariant(m_bMJPEGRecStyleAVI).toString()));
    docElem.appendChild(MJPEGRecStyleAVIElem);

    QDomElement enableH264SCE = doc.createElement(TN_ENABLE_H264_SCE);
    enableH264SCE.appendChild(doc.createTextNode(QString::number(m_bH264SCE)));
    docElem.appendChild(enableH264SCE);


    // Save the deferred audio rate, if set
    sdvr_audio_rate_e aRate = deferredAudioRate();
    if (aRate == SDVR_AUDIO_RATE_NONE)
        aRate = getAudioRate();

    if (aRate != SDVR_AUDIO_RATE_NONE)

    {
        QDomElement audioRateElem = doc.createElement(TN_AUDIO_RATE);
        audioRateElem.appendChild(doc.createTextNode(sdvr::audioRateToString(aRate)));
        docElem.appendChild(audioRateElem);
    }

    QDomElement debugEnable = doc.createElement(TN_DEBUG_ENABLE);
    debugEnable.appendChild(doc.createTextNode(QString::number(m_bDebugEnable)));
    docElem.appendChild(debugEnable);

    QDomElement debugSDKElem = doc.createElement(TN_DEBUG_SDK);
    debugSDKElem.appendChild(doc.createTextNode(QString::number(m_bDebugSDK)));
    docElem.appendChild(debugSDKElem);

    QDomElement StatViewElem = doc.createElement(TN_STATVIEWINFO);
    StatViewElem.appendChild(doc.createTextNode(QString::number(m_statViewInfo)));
    docElem.appendChild(StatViewElem);

    QDomElement debugShowDropFramesElem = doc.createElement(TN_DEBUG_SHOW_DROP_FRAMES);
    debugShowDropFramesElem.appendChild(doc.createTextNode(QString::number(m_bShowDropFrames)));
    docElem.appendChild(debugShowDropFramesElem);

    QDomElement debugAVFramesElem = doc.createElement(TN_DEBUG_AV_FRAMES);
    debugAVFramesElem.appendChild(doc.createTextNode(QString::number(m_bDebugAVFrames)));
    docElem.appendChild(debugAVFramesElem);

    //////////////////////////////////////////////////////////////////////
    // Add the font table information
    /////////////////////////////////////////////////////////////////////
    QDomElement activeFontTableElem = doc.createElement(TN_FONTTABLE_ACTIVE);
    activeFontTableElem.appendChild(doc.createTextNode(QString::number(m_activeFontTable)));
    docElem.appendChild(activeFontTableElem);

    QDomElement fontTableElem = doc.createElement(TN_FONTTABLE);

    QDomElement fontPathElem = doc.createElement(TN_FONTPATH);
    fontPathElem.appendChild(doc.createTextNode(getFontTablePath()));
    fontTableElem.appendChild(fontPathElem);

    sx_uint32 lowCode, highCode;
    getFontTableRange(&lowCode, &highCode);

    QDomElement fontCodeRangeElem = doc.createElement(TN_FONTCODERANGE);
    fontCodeRangeElem.setAttribute(TN_FONTCODELOW, QString::number(lowCode));
    fontCodeRangeElem.setAttribute(TN_FONTCODEHIGH, QString::number(highCode));
    fontTableElem.appendChild(fontCodeRangeElem);

    sx_uint8 y_color, u_color, v_color;
    getFontColor(&y_color, &u_color, &v_color);
    QDomElement fontColorElem = doc.createElement(TN_FONTCOLOR);
    fontColorElem.setAttribute(TN_FONTCOLORY, QString::number(y_color));
    fontColorElem.setAttribute(TN_FONTCOLORU, QString::number(u_color));
    fontColorElem.setAttribute(TN_FONTCOLORV, QString::number(v_color));
    fontTableElem.appendChild(fontColorElem);

    docElem.appendChild(fontTableElem);

    //////////////////////////////////////////////////////////////
    // Add camera list
    //////////////////////////////////////////////////////////////
    QDomElement camListElem = doc.createElement(TN_CAMERALIST);
    foreach (Camera *cam, cameraList)
    {
        QDomElement camElem = cam->toElement(doc);
        camListElem.appendChild(camElem);
    }
    docElem.appendChild(camListElem);

    QDomElement decListElem = doc.createElement(TN_PLAYERLIST);
    foreach (CPlayer *decoder, m_playerList)
    {
        QDomElement decoderElem = decoder->toElement(doc);
        decListElem.appendChild(decoderElem);
    }
    docElem.appendChild(decListElem);

    QDomElement monitorListElem = doc.createElement(TN_MONITORLIST);
    foreach (ViewSettings *vs, monitorList)
    {
        QDomElement vsElem = vs->toElement(doc);
        monitorListElem.appendChild(vsElem);
    }
    docElem.appendChild(monitorListElem);

    return docElem;
}

bool DVRSystem::isStartupSystem(const QString & configFilePath)
{
    if (!configFilePath.isEmpty())
    {
        QSettings settings("Stretch", "DVRCP");
        QVariant value = settings.value(KEY_STARTUP_SYSTEM);
        if (value.isValid() && (value.toString() == configFilePath))
            return true;
    }

    return false;
}

bool DVRSystem::isAnyCameraRecording()
{
    QList<Camera *>::const_iterator cam;
    for (cam = cameraList.begin(); cam != cameraList.end(); ++cam)
    {
        if ((*cam)->isChannelOpen() &&
            ((*cam)->isRecording(SDVR_ENC_PRIMARY) ||
            (*cam)->isRecording(SDVR_ENC_SECONDARY)))
            return true;
    }
    return false;
}

void DVRSystem::setStartupSystem(const QString & configFilePath)
{
    QSettings settings("Stretch", "DVRCP");
    settings.setValue(KEY_STARTUP_SYSTEM, configFilePath);
}

QString DVRSystem::startupSystem()
{
    QSettings settings("Stretch", "DVRCP");
    return settings.value(KEY_STARTUP_SYSTEM, QString()).toString();
}

void DVRSystem::setStartupSystemList(QList<QString> list)
{
    int count = list.count();
    QString qstrRegKey;
    QSettings settings("Stretch", "DVRCP");
    for (int i = 0; i < count; i++)
    {
        qstrRegKey = QString(KEY_STARTUP_SYSTEM_LIST).arg(i);
        settings.setValue(qstrRegKey, list[i]);
    }
}

QList<QString> DVRSystem::startupSystemList()
{
    QList<QString> list;
    QString qstrRegKey;
    QSettings settings("Stretch", "DVRCP");
    QString qstrNthPath;

    for (int i = 0; i < MAX_FOLDERS_REMEMBERED; i++)
    {
        qstrRegKey = QString(KEY_STARTUP_SYSTEM_LIST).arg(i);
        qstrNthPath = settings.value(qstrRegKey, "").toString();
        if (qstrNthPath.isEmpty())
            break;
        else
            list.append(qstrNthPath);
    }
    return list;
}


void DVRSystem::setStartupFirmwarePath(const QString & fwPath)
{
    QSettings settings("Stretch", "DVRCP");
    settings.setValue(KEY_FIRMWARE_PATH, fwPath);
}

QString DVRSystem::getStartupFirmwarePath()
{
    QSettings settings("Stretch", "DVRCP");
    return settings.value(KEY_FIRMWARE_PATH, QString()).toString();
}


void DVRSystem::setStartupFirmwarePathList(QList<QString> list)
{
    int count = list.count();
    QString qstrRegKey;
    QSettings settings("Stretch", "DVRCP");
    for (int i = 0; i < count; i++)
    {
        qstrRegKey = QString(KEY_FIRMWARE_PATH_LIST).arg(i);
        settings.setValue(qstrRegKey, list[i]);
    }
}

QList<QString> DVRSystem::getStartupFirmwarePathList()
{
    QList<QString> list;
    QString qstrRegKey;
    QSettings settings("Stretch", "DVRCP");
    QString qstrNthPath;

    for (int i = 0; i < MAX_FOLDERS_REMEMBERED; i++)
    {
        qstrRegKey = QString(KEY_FIRMWARE_PATH_LIST).arg(i);
        qstrNthPath = settings.value(qstrRegKey, "").toString();
        if (qstrNthPath.isEmpty())
            break;
        else
            list.append(qstrNthPath);
    }
    return list;
}
/////////////////////////////////////////////////////////////
// This function returns the number of miliseconds needed to
// pause between in HMO display in order to achieve the
// maximum number of frame displayed per second for the
// given video standard.
//
// Parameters:
//       videoStandard -- The video standard to get its real time
//                        video display per seconds
//
// Return:
//      number of milli-seconds to pause between each frame to
//      achieve the maximum video display per seconds for the given
//      video standard.
//
int DVRSystem::getHMOPlayInterval(sdvr_video_std_e videoStandard)
{

    switch (videoStandard)
    {
        case SDVR_VIDEO_STD_NONE:
        case SDVR_VIDEO_STD_D1_PAL:
        case SDVR_VIDEO_STD_CIF_PAL:
        case SDVR_VIDEO_STD_2CIF_PAL:
        case SDVR_VIDEO_STD_4CIF_PAL:
            return 40;      // 25 frame a second

        case SDVR_VIDEO_STD_D1_NTSC:
        case SDVR_VIDEO_STD_CIF_NTSC:
        case SDVR_VIDEO_STD_2CIF_NTSC:
        case SDVR_VIDEO_STD_4CIF_NTSC:
            return 33;      // 30 frame a second

        default:
            return 33;      // 30 frame a second
    }
}

/////////////////////////////////////////////////////////////
// This function returns the number of miliseconds needed to
// play each recorded video frame during the playback to
// acheive the maximum frame rate (30 fps for nts and 25 for pal).
//
// The duration is based on 90K Hz clock
//
// Parameters:
//       videoStandard -- The video standard to get its real time
//                        video display per seconds
//
// Return:
//      The time based on 90K Hz clock to playback each video frame to
//      achieve the maximum video display per seconds for the given
//      video standard.
//
int DVRSystem::getVideoRecDuration(sdvr_video_std_e videoStandard)
{

    switch (videoStandard)
    {
        case SDVR_VIDEO_STD_NONE:
        case SDVR_VIDEO_STD_D1_PAL:
        case SDVR_VIDEO_STD_CIF_PAL:
        case SDVR_VIDEO_STD_2CIF_PAL:
        case SDVR_VIDEO_STD_4CIF_PAL:
            return 3600;      // 25 frame a second

        case SDVR_VIDEO_STD_D1_NTSC:
        case SDVR_VIDEO_STD_CIF_NTSC:
        case SDVR_VIDEO_STD_2CIF_NTSC:
        case SDVR_VIDEO_STD_4CIF_NTSC:
            return 3003;      // 30 frame a second

        default:
            return 3003;      // 30 frame a second
    }
}
