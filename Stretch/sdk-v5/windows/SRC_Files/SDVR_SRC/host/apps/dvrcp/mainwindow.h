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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "sdvr_sdk.h"
#include <QMainWindow>
#include <QTimer>
#include <QFile>
#include <QMutex>


class QMenu;
class QAction;
class QScrollArea;
class QComboBox;
class QTreeWidgetItem;
class Camera;
class CPlayer;
class DVRSystemTree;
class VideoPanel;
class ViewSettings;
class SpotMonitor;
class RecView;
class DecoderView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    static const char *TITLE;
    static const QString SMO_OFF_ACTION_TEXT;
    static const QString SMO_PROPS_ACTION_TEXT;
    static const QString SMO_PLAYBACK_ACTION_TEXT;

public:
    MainWindow();
    ~MainWindow();

    QMenu *cameraMenu() { return m_cameraMenu; }
    QMenu *decoderMenu() {return m_playerMenu;}
    QMenu *smoMenu(SpotMonitor *smo);
    QMenu *recViewMenu() { return m_recViewMenu; }
    QMenu *decoderViewMenu() {return m_playerViewMenu; }
    QMenu *videoPanelMenu() {return m_videoPanelMenu; }
    QMenu *toolsMenu() {return m_toolsMenu; }
    void setStatusBar(const QString &msg);
    bool isAnyFirmwareTimeout() { return m_bFirmwareTimeout; }
    bool isFirmwareTimeout(int boardIndex);
    void setFirmwareTimeout(int boardIndex);
    ViewSettings *currentMonitorSettings() const;
    QString recordFilePath(QString name);
    void saveYUVBuffer(sdvr_yuv_buffer_t *buf);
    void closeYUVFile();
    sdvr_chan_handle_t getyuvSaveHandle() {return m_yuvSaveChanHandle;}

    QString rawAudioFilePath(QString name);
    void saveRawAudioBuffer(sdvr_av_buffer_t *buf);
    void closeRawAudioFile();
    sdvr_chan_handle_t getRawAudioSaveHandle() {return m_rawAudioSaveChanHandle;}


    bool isSkipRawVideoDisplay() {return m_bSkipRawVideoDisplay;}

    VideoPanel *videoPanel() const;
    void setContinuousDecoding(bool bContinuousDecoding) {m_bContinuousDecoding = bContinuousDecoding;}

    RecView *statView() const { return m_recView; }

signals:
    void loadStartupSystem();
protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *);

private slots:
    void onVideoTimer();
    void onWatchDogTimer();
    void onUpdateBoardTimeTimer();


/////////////////////////////////////////////////////////////////////
// Menu item handlers
////////////////////////////////////////////////////////////////////
    void onSystemNew();
    void onSystemOpen();
    void onSystemEdit();
    void onSystemClose();
    void onCameraNew();
    void onCameraEdit();
    void onCameraDelete();
    void onCameraConnect();
    void onCameraDisconnect();
    void onPrimaryStartRecording();
    void onPrimaryStopRecording();
    void onSecondaryStartRecording();
    void onSecondaryStopRecording();
    void onBothStartRecording();
    void onBothStopRecording();
    void onPlayBack();

    void onRecViewPrimaryStartRecording();
    void onRecViewPrimaryStopRecording();
    void onRecViewSecondaryStartRecording();
    void onRecViewSecondaryStopRecording();
    void onRecViewBothStartRecording();
    void onRecViewBothStopRecording();
    void onRecViewPlayBack();
    void onRecViewCameraEdit();
    void onPlayerNew();
    void onPlayerDelete();
    void onPlayerConnect();
    void onPlayerDisconnect();
    void onPlayerStartDecoding();
    void onPlayerStopDecoding();
    void onPlayerPauseResumeDecoding();
    void onPlayerEditProperties();

    void onPlayerViewStartDecoding();
    void onPlayerViewStopDecoding();
    void onPlayerViewPauseResumeDecoding();
    void onPlayerViewEditProperties();

    void onVideoPanelPrimaryStartRecording();
    void onVideoPanelPrimaryStopRecording();
    void onVideoPanelSecondaryStartRecording();
    void onVideoPanelSecondaryStopRecording();
    void onVideoPanelBothStartRecording();
    void onVideoPanelBothStopRecording();
    void onVideoPanelStartPlayer();
    void onVideoPanelStopPlayer();
    void onVideoPanelPauseResumePlayer();
    void onVideoPanelPlayBack();
    void onVideoPanelYUVRecording();
    void onVideoPanelRawAudioRecording();
    void onVideoPanelEnableRawAudio();
    void onVideoPanelCameraEdit();
    void onVideoPanelPlayerEdit();
    void onSelectStatData();
    void onNewMonitor();
    void onDeleteMonitor();
    void onRenameMonitor();
    void onClearLogText();
    void onSkipLogInfo();
    void onRelays();
    void onSensorConfig();
    void onDisableWatchDog();
    void onSetYUV422Convert();
    void onSetYUV420Convert();
    void onSetYVU420Convert();

    void onSetSkipRawVideoDisplay();
    void onIOCTLTest();
    void about();
    void onDisplaySelected();
    void onMonitorSelected(int index);
    void onChannelSelected(int index);
    void onContinuousDecoding();
/////////////////////////////////////////////////////////////////////
// About To Show menu handlers
////////////////////////////////////////////////////////////////////
    void onSystemMenuAboutToShow();
    void onCameraMenuAboutToShow();
    void onViewMenuAboutToShow();
    void onToolsMenuAboutToShow();
    void onRecViewMenuAboutToShow();
    void onPlayerViewMenuAboutToShow();
    void onPlayerMenuAboutToShow();
    void onVideoPanelMenuAboutToShow();

    void onSmoMenuTriggered(QAction *act);
    void updateChannelSelector();
    void updateMonitorSelector();
    void onSaveSystemConfiguration();
    void onDBGDisableHMO();
	void onInterruptRecording(uint subEnc, int nCamId);
    void onTreeItemActivated(QTreeWidgetItem *item, int column);
    void onLoadStartupSystem();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void setTitle();

    bool openSystem(const QString &configFilePath);
    void doAfterSystemOpen();

    void setView(const QString &name);

    void setVideoPanel(VideoPanel *vp);

    bool playBack(uint subEnc, Camera *cam);

    bool doCameraEdit(Camera *cam);
    void doOnStartRecording(int sub1Enc, int sub2Enc = -1);
    void doOnStopRecording(int sub1Enc, int sub2Enc = -1);

    void doOnRecViewStartRecording(int sub1Enc, int sub2Enc = -1);
    void doOnRecViewStopRecording(int sub1Enc, int sub2Enc = -1);
    void doOnVideoPanelStartRecording(int sub1Enc, int sub2Enc);
    void doOnVideoPanelStopRecording(int sub1Enc, int sub2Enc);
    bool doPlayerEditProperties(CPlayer *player);

    void editSmoProperties(SpotMonitor *smo);

    Camera *GetCameraFromContextMenu(QTreeWidgetItem *item);
    CPlayer *GetDecoderFromContextMenu(QTreeWidgetItem *item);

    bool    m_bFirmwareTimeout;
    bool    m_bContinuousDecoding;          // If this flas is true, the
                                            // player will keep decoding the
                                            // file until the player is manually stopped.

    bool    m_bDisableWatchDogTimer;        // A flag to indicate whether to send the
                                            // watchdog timer message to the firmware.
                                            // This is for testing this feature.

    bool m_bSetYUV422Convert;
    bool m_bSetYUV420Convert;
    bool m_bSetYVU420Convert;

    bool m_bSkipRawVideoDisplay;
    sdvr_chan_handle_t m_yuvSaveChanHandle; // the camera to save its YUV frames
    QFile m_qFileyuvSaveFile;               // File handle to save YUV frames.
                                            // NOTE: Only one camera at a time can
                                            // save it YUV frames.

    sdvr_chan_handle_t m_rawAudioSaveChanHandle; // the camera to save its raw audio frames
    QFile m_qFileRawAudioSaveFile;          // File handle to save raw audio frames.
                                            // NOTE: Only one camera at a time can
                                            // save it raw audio frames.

    QMutex m_YUVSaveMutex;

    QScrollArea *m_scrollArea;
    QDockWidget *m_logDock;
    QDockWidget *m_sysNavigator;
    QDockWidget *m_recViewDock;
    QDockWidget *m_DecoderViewDock;
    QDockWidget *m_smoYuvPlayerDock;
    DVRSystemTree *m_sysTree;
    RecView *m_recView;
    DecoderView *m_decoderView;

    QComboBox *m_channelSelector;
    QComboBox *m_monitorSelector;

    QTimer m_videoTimer;
    QTimer m_watchDogTimer;
    QTimer m_updateBoardTimeTimer;

    QMenu *m_systemMenu;
    QMenu *m_cameraMenu;
    QMenu *m_viewMenu;
    QMenu *m_smoMenu[16];
    QMenu *m_toolsMenu;
    QMenu *m_recViewMenu;
    QMenu *m_playerViewMenu;
    QMenu *m_playerMenu;
    QMenu *m_videoPanelMenu;
    QMenu *m_helpMenu;
    QMenu *m_recViewStartRecordMenu;
    QMenu *m_recViewStopRecordMenu;
    QMenu *m_videoPanelStartRecordMenu;
    QMenu *m_videoPanelStopRecordMenu;
    QMenu *m_startRecordMenu;
    QMenu *m_stopRecordMenu;


//    QToolBar *fileToolBar;

    QAction *newSystemAct;
    QAction *openSystemAct;
    QAction *editSystemAct;
    QAction *saveSystemAct;
    QAction *dbgDisableHMO;
    QAction *closeSystemAct;
    QAction *exitAct;

    QAction *newCameraAct;
    QAction *deleteCameraAct;
    QAction *editCameraAct;
    QAction *connectCameraAct;
    QAction *disconnectCameraAct;
    QAction *startPrimaryRecordingAct;
    QAction *startSecondaryRecordingAct;
    QAction *startBothRecordingAct;
    QAction *stopPrimaryRecordingAct;
    QAction *stopSecondaryRecordingAct;
    QAction *stopBothRecordingAct;

    QAction *playBackAct;
    QAction *viewTreeAct;
    QAction *viewLogAct;
    QAction *viewRecViewAct;
    QAction *selectStatData;
    QAction *viewDecoderViewAct;
    QAction *viewSmoYuvPlayerAct;
	QAction *videoPanelViewAct;
    QAction *newMonitorAct;
    QAction *deleteMonitorAct;
    QAction *renameMonitorAct;
    QAction *clearLogAct;
    QAction *skipLogInfoAct;
    QAction *ioctlTestAct;

    QAction *recViewStartPrimaryRecordingAct;
    QAction *recViewStopPrimaryRecordingAct;
    QAction *recViewStartSecondaryRecordingAct;
    QAction *recViewStopSecondaryRecordingAct;
    QAction *recViewStartBothRecordingAct;
    QAction *recViewStopBothRecordingAct;
    QAction *recViewPlayBackAct;
    QAction *recViewEditCameraAct;

    QAction *playerViewStartDecodingAct;
    QAction *playerViewStopDecodingAct;
    QAction *playerViewPauseResumeDecodingAct;
    QAction *playerViewEditPropertiesAct;

    QAction *playerNewAct;
    QAction *playerDeleteAct;
    QAction *playerConnectAct;
    QAction *playerDisconnectAct;
    QAction *playerStartDecodingAct;
    QAction *playerStopDecodingAct;
    QAction *playerPauseResumeDecodingAct;
    QAction *playerEditPropertiesAct;

    QAction *videoPanelStartPrimaryRecordingAct;
    QAction *videoPanelStopPrimaryRecordingAct;
    QAction *videoPanelStartSecondaryRecordingAct;
    QAction *videoPanelStopSecondaryRecordingAct;
    QAction *videoPanelStartBothRecordingAct;
    QAction *videoPanelStopBothRecordingAct;
    QAction *videoPanelStartDecodingAct;
    QAction *videoPanelStopDecodingAct;
    QAction *videoPanelPauseResumeDecodingAct;
    QAction *videoPanelPlayBackAct;
    QAction *videoPanelSaveYUVFramesAct;
    QAction *videoPanelSaveRawAudioFramesAct;
    QAction *videoPanelEnableRawAudioAct;
    QAction *videoPanelEditCameraAct;
    QAction *videoPanelEditPlayerAct;

    QAction *continuousDecodingAct;
    QAction *relaysAct;
    QAction *sensorCnfgAct;
    QAction *disableWatchDogAct;
    QAction *setYUV422VideoAct;
    QAction *setYVU420VideoAct;
    QAction *setYUV420VideoAct;

    QAction *setSkipRawVideoDisplayAct;

    QAction *aboutAct;
};

#ifndef MAINWINDOW_CPP
extern MainWindow *g_MainWindow;
extern QApplication *g_App;
#endif

#endif
