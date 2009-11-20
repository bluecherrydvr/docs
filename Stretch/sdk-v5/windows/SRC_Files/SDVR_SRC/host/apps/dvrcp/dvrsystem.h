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

#ifndef DVRSYSTEM_H
#define DVRSYSTEM_H

#include <QString>
#include <QDomElement>
#include <QTextStream>
#include <QVector>
#include <QList>
#include <qmessagebox.h>
#include "sdvr_sdk.h"
#include "recorder.h"
#include "decoder.h"
#include "alarmhandler.h"
#include "sensorshandler.h"
#include <QSemaphore>


class Camera;
class CPlayer;
class DVRBoard;
class SpotMonitor;
class ViewSettings;

typedef QList<Camera *> CameraList;
typedef QList<CPlayer *> PlayerList;

typedef enum {
    NEW_SYSTEM_BLANK_CNFG,
    NEW_SYSTEM_ENCODER_ONLY_CNFG,
    NEW_SYSTEM_DECODER_ONLY_CNFG,
    NEW_SYSTEM_COMBO_CNFG
} new_system_cngf_e ;


#define MAX_FOLDERS_REMEMBERED 4

// Number of miliseconds to send watchdog reset to the board.
// Currently since the watchdog timer is set at 10 seconds on the firmware
// we send the heartbeat message every 5 seconds.
#define WATCHDOG_TIMER_VALUE    (5 * 1000)

class DVRSystem
{
public:
    DVRSystem();
	~DVRSystem();

    sdvr_err_e setup();
    void getFWVersion(sdvr_firmware_ver_t  *version);

    QString name() const { return m_name; }
    void setName(const QString & name) { m_name = name; }

    bool isUseSDKRecording() {return m_bUseSDKRecording; }
    void setUseSDKRecording(bool bUserNewRecording) {m_bUseSDKRecording = bUserNewRecording; }

    bool isEnableAuthKey() {return m_bEnableAuthKey; }
    void setEnableAuthKey(bool bEnableAuthKey) {m_bEnableAuthKey = bEnableAuthKey; }

    int boardCount() const { return boardList.count(); }
    DVRBoard *board(int index) const;

    bool supportsVideoStandard(sdvr_video_std_e std) const;

    sdvr_video_std_e videoStandard() const { return m_videoStandard; }
    void setVideoStandard(sdvr_video_std_e std) { m_videoStandard = std; }

    sdvr_video_std_e deferredVideoStandard() const
    {
        return m_deferredVideoStandard;
    }

    void setDeferredVideoStandard(sdvr_video_std_e std)
    {
        m_deferredVideoStandard = std;
    }

    sdvr_audio_rate_e deferredAudioRate() const
    {
        return m_deferredAudioRate;
    }

    void setDeferredAudioRate(sdvr_audio_rate_e aRate)
    {
        m_deferredAudioRate = aRate;
    }

    QString getDeferredFirmwarePath() {return m_deferredFirmwarePath;}
    void setDeferredFirmwarePath(QString firmwarePath)
    {
        m_deferredFirmwarePath = firmwarePath ;
    }

    void setFontTablePath(QString fontTablePath) {m_fontTablePath = fontTablePath;}
    QString getFontTablePath() {return m_fontTablePath;}

    void setFontTableRange(sx_uint32 low, sx_uint32 high) {m_fontCodeRangeLow = low; m_fontCodeRangeHigh = high;}
    void getFontTableRange(sx_uint32 *low, sx_uint32 *high) {*low = m_fontCodeRangeLow; *high = m_fontCodeRangeHigh;}

    void setFontColor(sx_uint8 y_color, sx_uint8 u_color, sx_uint8 v_color)
            {m_fontColorY = y_color; m_fontColorU = u_color; m_fontColorV = v_color;}
    void getFontColor(sx_uint8 *y_color, sx_uint8 *u_color, sx_uint8 *v_color)
            {*y_color = m_fontColorY; *u_color = m_fontColorU; *v_color = m_fontColorV;}


    void setActiveFontTable(sx_uint8 id) { m_activeFontTable = id;}
    sx_uint8 getActiveFontTable() { return m_activeFontTable;}

    void setWatchDogState(uint state);
    uint getWatchDogState() const { return m_watchDogState; }

    void setWatchDogTimeout(uint timeout) {m_watchDogTimeout = timeout; }
    uint getWatchDogTimeout() const { return m_watchDogTimeout; }

    void setUpdateTimeInterval(uint updateTimeInterval) {m_updateTimeInterval = updateTimeInterval;}
    uint getUpdateTimeInterval() {return m_updateTimeInterval;}

    uint getStatFPSInterval() const { return m_statFPSInterval; }
    void setStatFPSInterval(uint statFPSInterval) { m_statFPSInterval = statFPSInterval; }

    uint getH264MaxFramesRecord() const { return m_H264MaxFramesRecord; }
    void setH264MaxFramesRecord(uint H264MaxFramesRecord) { m_H264MaxFramesRecord = H264MaxFramesRecord; }

    bool isSegmentedRecording() { return m_bSegmentedRecording;}
    void setSegmentedRecording(bool bSegmentedRecording) {m_bSegmentedRecording = bSegmentedRecording;}

    bool isSkipRecording() { return m_bSkipRecording;}
    void setSkipRecording(bool bSkipRecording) {m_bSkipRecording = bSkipRecording;}

    uint getPCItimeOut() const { return m_timeOut; }
    void setPCItimeOut(uint timeout) { m_timeOut = timeout; }

    bool getDebugEnable() const {return m_bDebugEnable; }
    void setDebugEnable(bool bEnable) {m_bDebugEnable = bEnable; }

    uint statViewInfo() const { return m_statViewInfo; }
    void setStatViewInfo(uint info) { m_statViewInfo = info; }

    uint getDebugAVFrames() const {return m_bDebugAVFrames; }
    void setDebugAVFrames(bool bAVFrames) {m_bDebugAVFrames = bAVFrames; }

    uint getDebugSDK() const {return m_bDebugSDK; }
    void setDebugSDK(bool bDebugSDK) {m_bDebugSDK = bDebugSDK; }

    bool isExtendedFeatures() {return m_bExtendedFeatures; }

    sdvr_audio_rate_e getAudioRate() { return m_audioRate; }
    void setAudioRate(sdvr_audio_rate_e audioRate) { m_audioRate = audioRate; }

    void setConfigFilePath(const QString &path) { m_configFilePath = path; }
    QString configFilePath() const { return m_configFilePath; }
    QString systemDirPath() const;

    QString getApplicationPath() const;
    QString getDefaultSystemPath(new_system_cngf_e configType) const;

    bool open(const QString &configFilePath, bool bConnect = true);
    bool open();
    bool isOpen() const;
    bool close();

    bool save(QTextStream &);
    bool save();

    bool promptVideoStandard();

    void updateSDKSettings(char *debug_file_name = NULL);
    sdvr_err_e updateOSDFontTable();

    int newId() { return ++m_lastId; }
    void setId(int id) {m_lastId = id;}

    int getNewPlayerId() { return ++m_lastPlayerId; }
    void setDecoderId(int id) {m_lastPlayerId = id;}

    bool attachCamera(Camera *, int boardIndex, int input);
    bool deleteCamera(int cameraId, bool bRemoveFromList = true);

    void removeChannelFromMonitors(sdvr_chan_handle_t chan);

    bool deletePlayer(int Id, bool bRemoveFromList = true);

    Camera *findCameraById(int id) const;
    Camera *findCameraByName(QString name) const;
    Camera *findCameraByInput(int boardIndex, int input) const;
    Camera *findCameraByHandle(sdvr_chan_handle_t handle) const;

    CPlayer *findDecoderById(int id) const;
    CPlayer *findDecoderByChanNum(int boardIndex, int chanNum) const;
    CPlayer *findDecoderByHandle(sdvr_chan_handle_t handle) const;

    ViewSettings *findMonitorByName(const QString &name);

    QString getNameFromHandle(sdvr_chan_handle_t handle);
    int getIdFromChanHandle(sdvr_chan_handle_t handle);

    QVector<int> availableVideoInputs(int boardIndex);
    QVector<int> availableBoardDecoders(int boardIndex);

    SpotMonitor *spotMonitor(int boardIndex, int port) const;
    bool isActiveSmo(ViewSettings *vs);

    Recorder &recorder() { return m_recorder; }
    AlarmHandler &alarmHandler() { return m_alarmHandler; }
    SensorsHandler &sensorsHandler() {return m_sensorsHandler; }
    SignalsHandler &signalsHandler() {return m_signalsHandler; }

    static void setStartupSystem(const QString &configFilePath);
    static void setStartupSystemList(QList<QString> list);
    static QList<QString> startupSystemList();

    static bool isStartupSystem(const QString &configFilePath);
    static QString startupSystem();

	static void setStartupFirmwarePath(const QString & fwPath);
	static QString getStartupFirmwarePath();
    static void setStartupFirmwarePathList(QList<QString> list);
    static QList<QString> getStartupFirmwarePathList();

    bool isAnyCameraRecording();

	bool isLoadFirmware() {return m_bLoadFirmware;}
	void setLoadFirmwareState(bool bLoad) {m_bLoadFirmware = bLoad;}

    void setShowDropFrames(bool bShow) {m_bShowDropFrames = bShow;}
    bool isShowDropFrames() {return m_bShowDropFrames;}

    bool isMJPEGRecStyleAVI() {return m_bMJPEGRecStyleAVI; }
    void setMJPEGRecStyleAVI(bool bMJPEGRecStyleAVI) { m_bMJPEGRecStyleAVI = bMJPEGRecStyleAVI; }

    sdvr_sdk_params_t &getSDKParams() {return m_sdk_params;}

    int getHMOPlayInterval(sdvr_video_std_e videoStandard);

    int getVideoRecDuration(sdvr_video_std_e videoStandard);

    bool isSystemClosing() { return m_bSystemClosing; }

    sx_uint8 getLastHighVSTD_FPS() {return m_lastHighVSTD_FPS;}

    QVector<DVRBoard *> boardList;
    QList<Camera *> cameraList;
    QList<CPlayer *> m_playerList;
    QList<ViewSettings *> monitorList;
    QList<SpotMonitor *> smoList;

private:
    QString m_name;
    bool m_bUseSDKRecording;
    bool m_bEnableAuthKey;
    QString m_configFilePath;
    QString m_deferredFirmwarePath;
    sdvr_video_std_e m_videoStandard;
    sdvr_video_std_e m_deferredVideoStandard;
    sdvr_audio_rate_e m_deferredAudioRate;
    uint m_availableVideoStandards;
    QString m_fontTablePath;
    sx_uint32 m_fontCodeRangeLow;
    sx_uint32 m_fontCodeRangeHigh;
    sx_uint8 m_fontColorY;
    sx_uint8 m_fontColorU;
    sx_uint8 m_fontColorV;
    sx_uint8 m_activeFontTable;
    uint m_watchDogState;
    uint m_watchDogTimeout;
    uint m_updateTimeInterval;
    uint m_statFPSInterval;
    uint m_H264MaxFramesRecord;
    uint m_timeOut;
    uint m_statViewInfo;
    bool m_bDebugEnable;
    bool m_bDebugAVFrames;
    bool m_bDebugSDK;
    bool m_bLoadFirmware;
	int  m_lastId;
    int  m_lastPlayerId;
    bool m_bShowDropFrames;
    bool m_bH264SCE;
    sdvr_audio_rate_e m_audioRate;
    bool m_bMJPEGRecStyleAVI;
    sdvr_firmware_ver_t     m_fwVersion;
    Recorder m_recorder;
    AlarmHandler m_alarmHandler;
    SensorsHandler  m_sensorsHandler;
    SignalsHandler  m_signalsHandler;
    sdvr_sdk_params_t m_sdk_params;
    sx_uint8 m_lastHighVSTD_FPS;
    bool m_bSegmentedRecording;     // If true, recorded file name will have
                                    // data/time appended to it while, recording
                                    // a limited number of frames.
    bool m_bSkipRecording;          // If ture, the encoder can be enabled but
                                    // the A/V frame will not be written to any
                                    // file.
    bool m_bExtendedFeatures;

    // A flag to indicate that we are about to close the system
    // so that the recorder thread empties its queue and the
    // decoder thread should stop sending any new frame to be
    // decoded.
    bool m_bSystemClosing;

	QDomElement toElement(QDomDocument &doc);
    bool fromElement(QDomElement docElem);
	bool camerasFromElement(QDomElement e);
    bool decodersFromElement(QDomElement e);
	bool monitorsFromElement(QDomElement e);

};

#endif /* DVRSYSTEM_H */
