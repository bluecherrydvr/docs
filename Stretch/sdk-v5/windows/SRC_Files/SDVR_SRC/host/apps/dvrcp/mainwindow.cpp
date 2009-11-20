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
#define MAINWINDOW_CPP 1

#include <QDockWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QMenuBar>
#include <QScrollArea>
#include <QSettings>
#include <QFileDialog>
#include <QStatusBar>
#include <QCloseEvent>
#include <QProcess>
#include <QShortcut>

#include "mainwindow.h"
#include "viewsettings.h"
#include "dvrsystem.h"
#include "dvrboard.h"
#include "camera.h"
#include "player.h"
#include "spotmonitor.h"
#include "dvrsystemtree.h"
#include "cameradialog.h"
#include "boarddialog.h"
#include "connectdialog.h"
#include "monitordialog.h"
#include "systemdialog.h"
#include "ioctldialog.h"
#include "smodialog.h"

#include "aboutdialog.h"
#include "recorder.h"
#include "recview.h"
#include "statviewdialog.h"
#include "decoderview.h"
#include "log.h"
#include "sdvr_sdk.h"
#include "sdvr_ui_sdk.h"
#include "sdvr.h"
#include "relaysdialog.h"
#include "sensorsdialog.h"
#include "decoderdialog.h"
#include "videopanel.h"
#include "smoyuvplayer.h"

sx_uint8 version_major = 5;
sx_uint8 version_minor = 0;
sx_uint8 version_revision = 1;
sx_uint8 version_build = 0;


DVRSystem *g_DVR;
MainWindow *g_MainWindow;
VideoPanel *g_videoPanel;
QApplication *g_App;
bool g_disable_hmo = false;

void av_frame_callback(sdvr_chan_handle_t handle,
                       sdvr_frame_type_e frame_type, sx_bool);

void video_alarm_callback(sdvr_chan_handle_t handle,
                          sdvr_video_alarm_e alarm_type, sx_uint32 data);

void sensors_callback(sx_uint32 board_index, sx_uint32 sensors_map);

void signals_callback(sx_uint32 board_index,
                      sdvr_signal_info_t * signal_info);

void disp_debug_callback(char *message);


const char *MainWindow::TITLE = "H264 DVR Control Panel";
const QString MainWindow::SMO_OFF_ACTION_TEXT = tr("Off");
const QString MainWindow::SMO_PROPS_ACTION_TEXT = tr("Properties...");
const QString MainWindow::SMO_PLAYBACK_ACTION_TEXT = tr("Playback...");

MainWindow::MainWindow()
{
    m_recViewMenu = 0;
    m_playerViewMenu = 0;
    m_videoPanelMenu = 0;

    m_bFirmwareTimeout = false;
    m_bContinuousDecoding = false;
    m_bDisableWatchDogTimer = false;

    m_yuvSaveChanHandle = 0;
    m_rawAudioSaveChanHandle = 0;

    m_bSetYUV422Convert = false;
    m_bSetYUV420Convert = true;
    m_bSetYVU420Convert = false;

    m_bSkipRawVideoDisplay = false;

    m_logDock = new QDockWidget("System Log", this);
    m_logDock->setAllowedAreas(Qt::BottomDockWidgetArea);
//    m_logDock->setFloating(true);
    m_logDock->setGeometry(100, 100, 640, 240);
    m_logDock->setWidget(Log::Instance());
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);

    // Central widget
    QWidget * centralWidget = new QWidget();
    QVBoxLayout *centralWidgetLayout = new QVBoxLayout(centralWidget);
    centralWidgetLayout->setMargin(0);
    centralWidgetLayout->setSpacing(0);
    centralWidget->setLayout(centralWidgetLayout);

    m_monitorSelector = new QComboBox();
    m_monitorSelector->setEnabled(false);
    m_monitorSelector->setFixedSize(150, 20);

    m_channelSelector = new QComboBox();
    m_channelSelector->setEnabled(false);
    m_channelSelector->setFixedSize(150, 20);

    QWidget *selectorBox = new QWidget();
    QHBoxLayout *selectorBoxLayout = new QHBoxLayout();
    selectorBoxLayout->setMargin(2);
    selectorBoxLayout->setSpacing(10);
    selectorBoxLayout->addSpacing(10);
    selectorBoxLayout->addWidget(new QLabel("Monitor:"));
    selectorBoxLayout->addWidget(m_monitorSelector);
    selectorBoxLayout->addSpacing(30);
    selectorBoxLayout->addWidget(new QLabel("Display:"));
    selectorBoxLayout->addWidget(m_channelSelector);
    selectorBox->setLayout(selectorBoxLayout);

    QToolBar *videoPanelToolBar = new QToolBar();
    videoPanelToolBar->addWidget(selectorBox);

    centralWidgetLayout->addWidget(videoPanelToolBar, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setBackgroundRole(QPalette::Dark);
    centralWidgetLayout->addWidget(m_scrollArea, 1);

    setCentralWidget(centralWidget);

    createStatusBar();

    QSettings settings("H264", "DVRCP");
    restoreGeometry(settings.value("main_window/geometry").toByteArray());

    setTitle();

    // We send a message to ourself to load the startup system. This is
    // required in order for us to be able to show loading status information on
    // the statusbar as the system is being loaded.
    connect(this, SIGNAL(loadStartupSystem()), this, SLOT(onLoadStartupSystem()),Qt::QueuedConnection);

    emit loadStartupSystem();
}

MainWindow::~MainWindow()
{
    if (m_recViewMenu) delete m_recViewMenu;
    if (m_playerViewMenu) delete m_playerViewMenu;
    if (m_videoPanelMenu) delete m_videoPanelMenu;
    delete g_DVR;
    setStatusBar("Shutting down the application...");
    sdvr_sdk_close();
    sdvr_ui_close();
}

void MainWindow::onSystemNew()
{
    SystemNewDialog dialog(g_DVR, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString saveCnfgFile = g_DVR->configFilePath();
        if (g_DVR->open())
        {
            doAfterSystemOpen();
        }
    }
}

void MainWindow::onSystemOpen()
{
    QList<QString> startupList = DVRSystem::startupSystemList();
    QString workingDir;

    if (startupList.count())
        workingDir = startupList[0];

    QString configFile = QFileDialog::getOpenFileName(this,
        tr("Select a System Configuration File"), workingDir,
        tr("*.xml;;*.*"));

    if (configFile == QString::null)
        return;

    if (!openSystem(configFile))
        onSystemClose();
}

bool MainWindow::openSystem(const QString &configFile)
{

    Log::info(QString("Reading configuration file %1").arg(configFile));
    bool ok = g_DVR->open(configFile);
    if (ok)
    {
        doAfterSystemOpen();
    }
    else
    {
        QMessageBox::warning(this, tr("Open System Error"),
            "Error(s) reading configuration file or connecting to the DVR board(s).\nSee log for details.");
    }

    return ok;
}

void MainWindow::doAfterSystemOpen()
{
    sdvr_err_e err;
    int maxBoardCount = g_DVR->boardCount();

    int frame_rate = sdvr::frameRate(g_DVR->videoStandard());

    // We only need to set the video timer interval when
    // we create a new system or open an existing time since
    // the video standard can not be change while editing.
    m_videoTimer.setInterval(1000 / frame_rate / 8);

    foreach (Camera *cam, g_DVR->cameraList)
        m_sysTree->addCameraItem(cam->id());

    foreach (CPlayer *decoder, g_DVR->m_playerList)
        m_sysTree->addDecoderItem(decoder->getId());

    // Setup SMO playback
    QTabWidget *smoYuvPlayerTabWidget = NULL;
    foreach (SpotMonitor *smo, g_DVR->smoList)
    {
        sdvr_smo_attribute_t attr;
        sdvr_err_e err = smo->getAttributes(&attr);
        if (err == SDVR_ERR_NONE)
        {
            if ((attr.cap_flags & SDVR_SMO_CAP_OUTPUT) != 0)
            {
                if (smoYuvPlayerTabWidget == NULL)
                    smoYuvPlayerTabWidget = new QTabWidget();

                smoYuvPlayerTabWidget->addTab(new SmoYuvPlayer(smo), smo->name());
            }
        }
        else
        {
            Log::error(QString("Error getting attributes for '%1' port %2: %3")
                .arg(smo->name()).arg(smo->port()).arg(sdvr_get_error_text(err)));
        }
    }

    if (smoYuvPlayerTabWidget == NULL)
    {
        QLabel *label = new QLabel("There is no YUV playback capable spot monitor.");
        label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        m_smoYuvPlayerDock->setWidget(label);
    }
    else
        m_smoYuvPlayerDock->setWidget(smoYuvPlayerTabWidget);


    g_DVR->updateSDKSettings();

    statusBar()->showMessage(tr("Updating the OSD font table..."));
    err = g_DVR->updateOSDFontTable();
    Log::chk_err(QString("Failed to load OSD font table."),err,true,-1);

    // Set the same watchdog timer for all the boards
    for (int board_index = 0; board_index < maxBoardCount; board_index++)
        sdvr_set_watchdog_state_ex(board_index,g_DVR->getWatchDogState(),
                                   g_DVR->getWatchDogTimeout() * 1000);
    if (g_DVR->getWatchDogState())
        m_watchDogTimer.start((g_DVR->getWatchDogTimeout() / 2) * 1000);//(WATCHDOG_TIMER_VALUE);
    else
        m_watchDogTimer.stop();

    // Set the interval of time to update date/time on all the boards
    // This is needed to keep the boards time in sync with the PC.
    // Note: the upate time is in minutes
    if (g_DVR->getUpdateTimeInterval())
        m_updateBoardTimeTimer.start(g_DVR->getUpdateTimeInterval() * 60000);
    else
        m_updateBoardTimeTimer.stop();

    if (m_recView)
    {
        if (m_recView->isVisible())
            m_recView->updateTimerInterval(g_DVR->getStatFPSInterval() * 1000);
        m_recView->selectInfo(g_DVR->statViewInfo());
    }

    if (m_decoderView && m_decoderView->isVisible())
        m_decoderView->updateTimerInterval(g_DVR->getStatFPSInterval() * 1000);

    updateMonitorSelector();
    setTitle();
}

extern "C" void dbg_dump_frame_count();

void MainWindow::onSystemEdit()
{
    SystemEditDialog dialog(g_DVR, this);

    if (false)
        dbg_dump_frame_count();

    if (dialog.exec() == QDialog::Accepted)
    {
        sdvr_err_e err;
        int maxBoardCount = g_DVR->boardCount();
        g_DVR->save();
        g_DVR->updateSDKSettings();

        statusBar()->showMessage(tr("Updating the OSD font table..."));
        err = g_DVR->updateOSDFontTable();
        Log::chk_err("Failed to load OSD font table.",err, true, -1);

        // Set the interval of time to update date/time on all the boards
        // This is needed to keep the boards time in sync with the PC.
        // Note: the upate time is in minutes
        if (g_DVR->getUpdateTimeInterval())
            m_updateBoardTimeTimer.start(g_DVR->getUpdateTimeInterval() * 60000);
        else
            m_updateBoardTimeTimer.stop();

        // Set the same watchdog timer for all the boards
        for (int board_index = 0; board_index < maxBoardCount; board_index++)
            sdvr_set_watchdog_state_ex(board_index,g_DVR->getWatchDogState(),g_DVR->getWatchDogTimeout() * 1000);

        if (g_DVR->getWatchDogState())
            m_watchDogTimer.start((g_DVR->getWatchDogTimeout() / 2) * 1000);//(WATCHDOG_TIMER_VALUE);
        else
            m_watchDogTimer.stop();

        if (m_recView && m_recView->isVisible())
            m_recView->updateTimerInterval(g_DVR->getStatFPSInterval() * 1000);
        if (m_decoderView && m_decoderView->isVisible())
            m_decoderView->updateTimerInterval(g_DVR->getStatFPSInterval() * 1000);

        setTitle();
    }
}

void MainWindow::onSystemClose()
{
    setVideoPanel(NULL);

    // Make sure the SMO for all the boards are off otherwise
    // channel close will fail.
    foreach (SpotMonitor *smo, g_DVR->smoList)
        smo->clear();

    m_sysTree->removeAllCameraItems();
    m_sysTree->removeAllDecoderItems();
    g_DVR->close();

    updateChannelSelector();
    updateMonitorSelector();

    setTitle();
}

void MainWindow::onSaveSystemConfiguration()
{
    if (g_DVR->isOpen())
    {
        g_DVR->save();
        statusBar()->showMessage(tr("System configuration saved."));
    }
}
void MainWindow::onDBGDisableHMO()
{
    g_disable_hmo = !g_disable_hmo;
}

void MainWindow::onSystemMenuAboutToShow()
{
    if (g_DVR->isOpen())
    {
        newSystemAct->setEnabled(false);
        openSystemAct->setEnabled(false);
        editSystemAct->setEnabled(true);
        closeSystemAct->setEnabled(true);
        saveSystemAct->setEnabled(true);
    }
    else
    {
        newSystemAct->setEnabled(true);
        openSystemAct->setEnabled(true);
        editSystemAct->setEnabled(false);
        closeSystemAct->setEnabled(false);
        saveSystemAct->setEnabled(false);
    }
}

void MainWindow::onContinuousDecoding()
{
    m_bContinuousDecoding = !m_bContinuousDecoding;
}

void MainWindow::onMonitorSelected(int index)
{
//    setView(((QAction *)sender())->data().toInt());
    if (index == 0)
        setVideoPanel(NULL);
    else
        setView(m_monitorSelector->itemText(index));

    updateChannelSelector();
}

void MainWindow::onInterruptRecording(uint subEnc, int nCamId)
{
    Camera *cam;
    cam = g_DVR->findCameraById(nCamId);
    if (cam)
        cam->stopRecording(subEnc, true);
}

void MainWindow::setView(const QString &name)
{
    ViewSettings *vs = g_DVR->findMonitorByName(name);
    Q_ASSERT(vs != NULL);

    VideoPanel *panel = videoPanel();

    if (panel != NULL && panel->settings() == vs)
        return;

    QSize size = sdvr::frameSize(g_DVR->videoStandard(), vs->decimation());

    {
        setVideoPanel(NULL);
        panel = new VideoPanel(size.width(), size.height(), vs, this);
        connect(panel, SIGNAL(screenSelected()), SLOT(onDisplaySelected()));
        setVideoPanel(panel);
    }

    updateChannelSelector();
    onDisplaySelected();
}

void MainWindow::onDisplaySelected()
{
    ViewSettings *ms = currentMonitorSettings();
    if (ms != NULL)
    {
        sdvr_chan_handle_t handle;
        handle = ms->selectedDisplayChannel();
        if (handle == INVALID_CHAN_HANDLE)
            handle = 0;  // channel was not found, set it to off
        int index = m_channelSelector->findData(QVariant(handle));
        m_channelSelector->setCurrentIndex(index);
    }
}

void MainWindow::onChannelSelected(int index)
{
    ViewSettings *ms = currentMonitorSettings();
    VideoPanel *panel = videoPanel();
    sdvr_err_e err = SDVR_ERR_NONE;

    Q_ASSERT(ms != NULL);

    sdvr_chan_handle_t selChanHandle =  (sdvr_chan_handle_t)m_channelSelector->itemData(index).toInt();

    int curDisplay = ms->selectedDisplay();
    sdvr_chan_handle_t curChanHandle = ms->displayChannel(curDisplay);

    if (selChanHandle == curChanHandle)
        return;

    if ((curChanHandle != INVALID_CHAN_HANDLE) && (curChanHandle != 0))
    {
        if (ms->displaysPlayingChannel(curChanHandle).count() == 1)
        {
            if (sdvr_get_chan_type(curChanHandle) == SDVR_CHAN_TYPE_DECODER)
            {
                CPlayer *player = g_DVR->findDecoderByHandle(curChanHandle);
                err = player->setRawVideo(ms->decimation(), false);
            }
            else
            {
                Camera *cam = g_DVR->findCameraByHandle(curChanHandle);
                err = cam->setRawVideo(ms->decimation(), false);
                // Disable the raw audio if it was enabled.
                if (cam->isRawAudioEnabled())
                    cam->setRawAudio(false);
            }
        }

        foreach (SpotMonitor *smo, g_DVR->smoList)
            if (smo->view() == ms)
                smo->enableGrid(curDisplay, false);
    }

    if (selChanHandle == INVALID_CHAN_HANDLE || selChanHandle == 0)
    {
        panel->updateDisplayChan(INVALID_CHAN_HANDLE);

    }
    else
    {
        QVector<int> displaysPlayingChannel = ms->displaysPlayingChannel(selChanHandle);

        if (displaysPlayingChannel.count() == 0)
        {
            if (sdvr_get_chan_type(selChanHandle) == SDVR_CHAN_TYPE_DECODER)
            {
                CPlayer *player = g_DVR->findDecoderByHandle(selChanHandle);
                err = player->setRawVideo(ms->decimation(), true);
            }
            else
            {
                Camera *cam = g_DVR->findCameraByHandle(selChanHandle);
                err = cam->setRawVideo(ms->decimation(), true);
                if (ms->isEnableRawAudio())
                    cam->setRawAudio(true);
            }

        }
        // Only add the channel if we could enable the raw video.
        if (err == SDVR_ERR_NONE)
        {
            panel->updateDisplayChan(selChanHandle);

            foreach (SpotMonitor *smo, g_DVR->smoList)
                if (smo->view() == ms)
                    smo->enableGrid(curDisplay, true);
        }
        else
        {
            // The channel was not added, re-set it to OFF
            m_channelSelector->setCurrentIndex(0);
        }
    }
}

void MainWindow::onTreeItemActivated(QTreeWidgetItem *item, int)
{
    int itemType = item->type();
    if (itemType == DVRSystemTree::CAMERA ||
        itemType == DVRSystemTree::VIDEOINPUT)
    {
        onCameraEdit();
    }
    else if (itemType == DVRSystemTree::BOARD)
    {
        BoardItem *boardItem = (BoardItem *) item;
        BoardDialog dialog(boardItem->board(), this);
        dialog.exec();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("H264", "DVRCP");
    settings.beginGroup("main_window");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

    setVideoPanel(NULL);

    // Make sure the SMO for all the boards are off otherwise
    // channel close will fail.
    foreach (SpotMonitor *smo, g_DVR->smoList)
        smo->clear();

    if (g_DVR->isOpen())
        g_DVR->close();

    event->accept();

    //if (maybeSave()) {
    //    writeSettings();
    //    event->accept();
    //} else {
    //    event->ignore();
    //}
}

void MainWindow::onCameraMenuAboutToShow()
{
    bool enableStartPrimary = false;
    bool enableStopPrimary = false;
    bool enableStartSecondary = false;
    bool enableStopSecondary = false;

    newCameraAct->setEnabled(g_DVR->isOpen());
    editCameraAct->setEnabled(false);
    connectCameraAct->setVisible(false);
    connectCameraAct->setEnabled(false);
    disconnectCameraAct->setVisible(false);
    deleteCameraAct->setEnabled(false);
    startPrimaryRecordingAct->setEnabled(false);
    stopPrimaryRecordingAct->setEnabled(false);
    startBothRecordingAct->setEnabled(false);
    startSecondaryRecordingAct->setEnabled(false);
    stopSecondaryRecordingAct->setEnabled(false);
    stopBothRecordingAct->setEnabled(false);
    m_startRecordMenu->setEnabled(false);
    m_stopRecordMenu->setEnabled(false);

    playBackAct->setVisible(false);

    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();
    Camera *cam = NULL;
    if (selItems.isEmpty())
        return;

    QTreeWidgetItem *item = selItems.first();

    if (item == NULL)
        return;

    connectCameraAct->setVisible(item->type() == DVRSystemTree::CAMERA);
    deleteCameraAct->setVisible(item->type() == DVRSystemTree::CAMERA);

    cam = GetCameraFromContextMenu(selItems.first());
    if (cam == NULL)
        return;


    if (item->type() == DVRSystemTree::CAMERA ||
        item->type() == DVRSystemTree::VIDEOINPUT)
    {
        editCameraAct->setEnabled(true);

        if (cam->isChannelOpen())
        {
            // if camera is connected

            disconnectCameraAct->setVisible(true);
            connectCameraAct->setVisible(false);

            if (cam->isAVStreaming() )
            {
                disconnectCameraAct->setEnabled(false);
                deleteCameraAct->setEnabled(false);
            }
            else
            {
                disconnectCameraAct->setEnabled(true);
                deleteCameraAct->setEnabled(item->type() == DVRSystemTree::CAMERA);
            }

            if (cam->videoFormat(SDVR_ENC_PRIMARY) != SDVR_VIDEO_ENC_NONE)
            {
                if (cam->isRecording(SDVR_ENC_PRIMARY))
                    enableStopPrimary = true;
                else
                    enableStartPrimary = true;

                if (cam->videoFormat(SDVR_ENC_SECONDARY) != SDVR_VIDEO_ENC_NONE)
                {
                    if (cam->isRecording(SDVR_ENC_SECONDARY))
                        enableStopSecondary = true;
                    else
                        enableStartSecondary = true;
                }
            }
        }
        else
        {
            // If camera is not connected

            connectCameraAct->setEnabled(item->type() == DVRSystemTree::CAMERA);
            deleteCameraAct->setEnabled(item->type() == DVRSystemTree::CAMERA);

        }

        if (cam->videoFormat(SDVR_ENC_PRIMARY) != SDVR_VIDEO_ENC_NONE)
        {
            m_startRecordMenu->setEnabled(enableStartPrimary || enableStartSecondary);
            m_stopRecordMenu->setEnabled(enableStopPrimary || enableStopSecondary);

            startPrimaryRecordingAct->setEnabled(enableStartPrimary);
            stopPrimaryRecordingAct->setEnabled(enableStopPrimary);

            startSecondaryRecordingAct->setEnabled(enableStartSecondary);
            stopSecondaryRecordingAct->setEnabled(enableStopSecondary);

            stopBothRecordingAct->setEnabled(enableStopPrimary && enableStopSecondary);
            startBothRecordingAct->setEnabled(enableStartPrimary && enableStartSecondary);
            playBackAct->setVisible(true);
            playBackAct->setEnabled(cam->canPlayBack(SDVR_ENC_PRIMARY));
        }
    }
}

void MainWindow::onCameraNew()
{
    CameraSetupDialog setupDialog(this);
    if (setupDialog.exec() == QDialog::Accepted)
    {
        int newId = g_DVR->newId();
        Camera *cam = new Camera();

        // Set a default camera name. Also the OSD text
        // is defaulted to the camera name.
        cam->setName(QString("Camera %1").arg(newId));
        cam->setOsdText(0,cam->name());
        cam->setAudioFormat(setupDialog.audioEncoding());
        cam->setVideoFormat(SDVR_ENC_PRIMARY, setupDialog.videoEncoder(SDVR_ENC_PRIMARY));
        cam->setVideoFormat(SDVR_ENC_SECONDARY, setupDialog.videoEncoder(SDVR_ENC_SECONDARY));

        CameraDialog dialog(cam, this);
        if (dialog.exec() == QDialog::Accepted)
        {
            cam->setId(newId);
            g_DVR->cameraList.append(cam);
            m_sysTree->addCameraItem(newId);
        }
        else
        {
            // Since the Camera was not created, re-use
            // the Id for the future cameras.
            g_DVR->setId(newId);
            delete cam;
        }
    }
}

Camera *MainWindow::GetCameraFromContextMenu(QTreeWidgetItem *item)
{
    Camera *cam = NULL;
    int cameraId;

    if (item->type() == DVRSystemTree::CAMERA)
    {
        cameraId = ((CameraItem *)item)->cameraId();
        cam = g_DVR->findCameraById(cameraId);
    }
    else if (item->type() == DVRSystemTree::VIDEOINPUT)
    {
        VideoInputItem * videoInput = (VideoInputItem *)item;
        cam = g_DVR->findCameraByInput(
                videoInput->boardIndex(), videoInput->input());
    }

    return cam;
}


void MainWindow::onCameraEdit()
{
    Camera *cam = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();
    if (selItems.isEmpty())
        return;

    cam = GetCameraFromContextMenu(selItems.first());
    if (cam != NULL)
        doCameraEdit(cam);
}

void MainWindow::onCameraDelete()
{
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();
    QTreeWidgetItem *itemFirst;

    if (selItems.isEmpty())
        return;

    itemFirst = selItems.first();

    // Ignore delete if the selected item is not a camera
    if (itemFirst->type() == DVRSystemTree::CAMERA)
    {
        if (g_DVR->deleteCamera(((CameraItem *)itemFirst)->cameraId()))
        {
            delete itemFirst;
            updateChannelSelector();
        }
    } //if (item->type() == DVRSystemTree::CAMERA)
}

void MainWindow::onCameraConnect()
{
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();
    QTreeWidgetItem *itemFirst;

    if (selItems.isEmpty())
        return;

    itemFirst = selItems.first();

    // Ignore camera connect if the selected item is not a camera
    if (itemFirst->type() != DVRSystemTree::CAMERA)
        return;

    int cameraId = ((CameraItem *) selItems.first())->cameraId();
    Camera *cam = g_DVR->findCameraById(cameraId);
    // If the camera does not exist or is already
    // connected, we should not re-connect.
    if (cam == NULL || cam->isAttached())
        return;

    ConnectDialog dialog(SDVR_CHAN_TYPE_ENCODER, cameraId, g_DVR,this);
    if (dialog.exec() == QDialog::Accepted)
    {
        sdvr_venc_e venc;
        sdvr_aenc_e aenc;

        cam->attach(dialog.boardIndex(), dialog.channel());

        // Set the new A/V encoder formats and VPP Mode.
        aenc = dialog.audioFormat();
        cam->setAudioFormat(aenc);
        venc = dialog.videoFormat(SDVR_ENC_PRIMARY);
        cam->setVideoFormat(SDVR_ENC_PRIMARY, venc);
        venc = dialog.videoFormat(SDVR_ENC_SECONDARY);
        cam->setVideoFormat(SDVR_ENC_SECONDARY, venc);

        if (cam->openChannel())
            cam->applyAllParameters();
        else
            cam->detach();

        updateChannelSelector();
    }
}

void MainWindow::onCameraDisconnect()
{
    Camera *cam = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();

    if (selItems.isEmpty())
        return;

    cam = GetCameraFromContextMenu(selItems.first());

    if (cam != NULL && cam->isAttached())
    {
        g_DVR->removeChannelFromMonitors(cam->handle());
        cam->closeChannel();
        cam->detach();

        m_sysTree->videoInputDataChanged();
        updateChannelSelector();
    }
}

void MainWindow::onPrimaryStartRecording()
{
    doOnStartRecording(SDVR_ENC_PRIMARY);
}

void MainWindow::onPrimaryStopRecording()
{
    doOnStopRecording(SDVR_ENC_PRIMARY);
}
void MainWindow::onSecondaryStartRecording()
{
    doOnStartRecording(SDVR_ENC_SECONDARY);
}

void MainWindow::onSecondaryStopRecording()
{
    doOnStopRecording(SDVR_ENC_SECONDARY,-1);
}

void MainWindow::onBothStartRecording()
{
    doOnStartRecording(SDVR_ENC_PRIMARY,SDVR_ENC_SECONDARY);
}

void MainWindow::onBothStopRecording()
{
    doOnStopRecording(SDVR_ENC_PRIMARY,SDVR_ENC_SECONDARY);
}

void MainWindow::doOnStartRecording(int sub1Enc, int sub2Enc)
{
    Camera *cam = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();

    if (selItems.isEmpty())
        return;

    cam = GetCameraFromContextMenu(selItems.first());

    if (cam)
    {
        if (sub1Enc != -1)
            cam->startRecording(sub1Enc);
        if (sub2Enc != -1)
            cam->startRecording(sub2Enc);
    }
}

void MainWindow::doOnStopRecording(int sub1Enc, int sub2Enc)
{
    Camera *cam = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();

    if (selItems.isEmpty())
        return;

    cam = GetCameraFromContextMenu(selItems.first());

    if (cam)
    {
        if (sub1Enc != -1)
            cam->stopRecording(sub1Enc);
        if (sub2Enc != -1)
            cam->stopRecording(sub2Enc);
    }
}

void MainWindow::onPlayBack()
{
    Camera *cam = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();

    if (selItems.isEmpty())
        return;

    cam = GetCameraFromContextMenu(selItems.first());
//todo: Add support for both primary and secondary
    if (cam)
        playBack(SDVR_ENC_PRIMARY, cam);
}


void MainWindow::onRecViewPrimaryStartRecording()
{
    doOnRecViewStartRecording(SDVR_ENC_PRIMARY);
}

void MainWindow::onRecViewPrimaryStopRecording()
{
    doOnRecViewStopRecording(SDVR_ENC_PRIMARY);
}
void MainWindow::onRecViewSecondaryStartRecording()
{
    doOnRecViewStartRecording(SDVR_ENC_SECONDARY);
}

void MainWindow::onRecViewSecondaryStopRecording()
{
    doOnRecViewStopRecording(SDVR_ENC_SECONDARY,-1);
}

void MainWindow::onRecViewBothStartRecording()
{
    doOnRecViewStartRecording(SDVR_ENC_PRIMARY,SDVR_ENC_SECONDARY);
}

void MainWindow::onRecViewBothStopRecording()
{
    doOnRecViewStopRecording(SDVR_ENC_PRIMARY,SDVR_ENC_SECONDARY);
}

void MainWindow::doOnRecViewStartRecording(int sub1Enc, int sub2Enc)
{
    CameraList camList = m_recView->selectedCameras();

    foreach (Camera *cam, camList)
    {
        if (sub1Enc != -1)
            cam->startRecording(sub1Enc);
        if (sub2Enc != -1)
            cam->startRecording(sub2Enc);

    }
}

void MainWindow::doOnRecViewStopRecording(int sub1Enc, int sub2Enc)
{
    CameraList camList = m_recView->selectedCameras();

    foreach (Camera *cam, camList)
    {
        if (sub1Enc != -1)
            cam->stopRecording(sub1Enc);
        if (sub2Enc != -1)
            cam->stopRecording(sub2Enc);
    }
}

void MainWindow::onRecViewPlayBack()
{
    CameraList camList = m_recView->selectedCameras();
//todo: Add support for both primary and secondary

    if (camList.count() == 1)
        playBack(SDVR_ENC_PRIMARY, camList.first());
}

void MainWindow::onRecViewCameraEdit()
{
    CameraList camList = m_recView->selectedCameras();
    if (camList.count() != 1)
    {
        setStatusBar("The properties of only one camera can be edited.");
        return;
    }
    Camera *cam = camList.first();
    if (cam != NULL)
        doCameraEdit(cam);
}

void MainWindow::onPlayerViewStartDecoding()
{

    PlayerList playerList = m_decoderView->selectedDecoders();

    foreach (CPlayer *player, playerList)
        player->startDecoding(continuousDecodingAct->isChecked());

}

void MainWindow::onPlayerViewStopDecoding()
{

    PlayerList playerList = m_decoderView->selectedDecoders();

    foreach (CPlayer *player, playerList)
        player->stopDecoding();
}

void MainWindow::onPlayerViewPauseResumeDecoding()
{
    playerRunningState_e curState;
    PlayerList playerList = m_decoderView->selectedDecoders();

    foreach (CPlayer *player, playerList)
    {
        curState =  player->getPlaybackState();
        if (curState != PLAYER_STATE_STOPPED)
        {
            player->setPlaybackState(curState != PLAYER_STATE_PAUSED ? PLAYER_STATE_PAUSED : PLAYER_STATE_PLAYBACK);
        }
    }
}

void MainWindow::onPlayerViewEditProperties()
{
    PlayerList playerList = m_decoderView->selectedDecoders();
    if (playerList.count() != 1)
    {
        setStatusBar("The properties of only one Player can be edited.");
        return;
    }
    CPlayer *player = playerList.first();
    if (player != NULL)
        doPlayerEditProperties(player);
}


void MainWindow::onPlayerNew()
{
    CPlayer *player = new CPlayer;

    int newDecoderId = g_DVR->getNewPlayerId();
    DecoderDialog dlgDecoder(player, g_DVR,this);

    dlgDecoder.setWindowTitle("New Player");
    dlgDecoder.setNewPlayer(true);
    dlgDecoder.setDecoderName(QString("Player %1").arg(newDecoderId));
    dlgDecoder.setVideoSize(SDVR_VIDEO_SIZE_720x480);
    dlgDecoder.initDialog();

    if (dlgDecoder.exec() == QDialog::Accepted)
    {
        player->setId(newDecoderId);

        g_DVR->m_playerList.append(player);
        m_sysTree->addDecoderItem(newDecoderId);
        if (dlgDecoder.getConnect())
        {
            // Decoder needs to be connected
            player->attach(dlgDecoder.getBoardIndex(), dlgDecoder.getChanNum());
            if (!player->openChannel())
                player->detach();
            updateChannelSelector();
        }
    }
    else
    {
        delete player;

        // Since the decoder was not created, re-use
        // the Id for the future cameras.
        g_DVR->setDecoderId(newDecoderId);
    }
}

void MainWindow::onPlayerDelete()
{
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();
    QTreeWidgetItem *itemFirst;
    CPlayer *player;
    if (selItems.isEmpty())
        return;

    itemFirst = selItems.first();
    player = GetDecoderFromContextMenu(itemFirst);

    // Ignore delete if the selected item is not a player
    if (player)
    {
        if (g_DVR->deletePlayer(player->getId()))
        {
            delete itemFirst;
            updateChannelSelector();
        }
    }
}

void MainWindow::onPlayerConnect()
{
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();
    QTreeWidgetItem *itemFirst;

    if (selItems.isEmpty())
        return;

    itemFirst = selItems.first();

    // Ignore the connect if the selected item is not a player
    if (itemFirst->type() != DVRSystemTree::DECODER)
        return;

    int playerId = ((CPlayerItem *) selItems.first())->getDecoderId();
    CPlayer *player = g_DVR->findDecoderById(playerId);

    if (player && !player->isAttached())
    {
        ConnectDialog dialog(SDVR_CHAN_TYPE_DECODER, playerId, g_DVR,this);
        if (dialog.exec() == QDialog::Accepted)
        {
            player->attach(dialog.boardIndex(), dialog.channel());
            if (!player->openChannel())
                player->detach();
            m_sysTree->boardDecoderDataChanged();

            updateChannelSelector();
        }
    } //if (player && !player->isAttached())
}

void MainWindow::onPlayerDisconnect()
{
    CPlayer *player = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();

    if (selItems.isEmpty())
        return;

    player = GetDecoderFromContextMenu(selItems.first());

    if (player != NULL && player->isAttached())
    {
        g_DVR->removeChannelFromMonitors(player->handle());
        player->closeChannel();
        player->detach();

        m_sysTree->boardDecoderDataChanged();
        updateChannelSelector();
    }
}
void MainWindow::onPlayerEditProperties()
{
    CPlayer *player = NULL;

    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();
    if (selItems.isEmpty())
        return;

    player = GetDecoderFromContextMenu(selItems.first());
    if (player == NULL)
        return;

    doPlayerEditProperties(player);
}

bool MainWindow::doPlayerEditProperties(CPlayer *player)
{
    bool ret = true;
    DecoderDialog dlgDecoder(player, g_DVR,this);

    dlgDecoder.setNewPlayer(false);
    dlgDecoder.setWindowTitle("Edit Decoder");

    dlgDecoder.setDecoderName(player->getName());
    dlgDecoder.setVidoeFormat(player->getVideoFormat());
    // todo: for now audio is disabled
    dlgDecoder.setAudioFormat(SDVR_AUDIO_ENC_NONE);
    dlgDecoder.setDecoderPath(player->getEncodedFilePath());

    dlgDecoder.setVideoSize(player->getVideoSize());

    bool bOldConnectState = player->isAttached();
    dlgDecoder.initDialog();
    if (dlgDecoder.exec() == QDialog::Accepted)
    {
        bool bNewConnectState = dlgDecoder.getConnect();
        if (bOldConnectState != bNewConnectState)
        {
            if (bOldConnectState)
            {
                // Decoder was connected but now should be disconnect
                g_DVR->removeChannelFromMonitors(player->handle());
                if ((ret = player->closeChannel()))
                    ret = player->detach();
            }
            else
            {
                // Decoder needs to be connected
                player->attach(dlgDecoder.getBoardIndex(), dlgDecoder.getChanNum());
                if (!(ret = player->openChannel()))
                    player->detach();

            }
            if (ret)
            {
                m_sysTree->boardDecoderDataChanged();
                updateChannelSelector();
            }
        }
    } //if (dlgDecoder.exec() == QDialog::Accepted)

    return ret;
}

void MainWindow::onPlayerStartDecoding()
{
    CPlayer *decoder = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();

    if (selItems.isEmpty())
        return;

    decoder = GetDecoderFromContextMenu(selItems.first());

    if (decoder)
        decoder->startDecoding(continuousDecodingAct->isChecked());
}

void MainWindow::onPlayerStopDecoding()
{
    CPlayer *decoder = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();

    if (selItems.isEmpty())
        return;

    decoder = GetDecoderFromContextMenu(selItems.first());

    if (decoder)
        decoder->stopDecoding();
}
void MainWindow::onPlayerPauseResumeDecoding()
{
    playerRunningState_e curState;
    CPlayer *player = NULL;
    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();

    if (selItems.isEmpty())
        return;

    player = GetDecoderFromContextMenu(selItems.first());
    if (player)
    {
        curState =  player->getPlaybackState();
        if (curState != PLAYER_STATE_STOPPED)
        {
            player->setPlaybackState(curState != PLAYER_STATE_PAUSED ? PLAYER_STATE_PAUSED : PLAYER_STATE_PLAYBACK);
        }
    }
}

void MainWindow::onVideoPanelPrimaryStartRecording()
{
    doOnVideoPanelStartRecording(SDVR_ENC_PRIMARY, -1);
}

void MainWindow::onVideoPanelPrimaryStopRecording()
{
    doOnVideoPanelStopRecording(SDVR_ENC_PRIMARY, -1);
}
void MainWindow::onVideoPanelSecondaryStartRecording()
{
    doOnVideoPanelStartRecording(SDVR_ENC_SECONDARY, -1);
}

void MainWindow::onVideoPanelSecondaryStopRecording()
{
    doOnVideoPanelStopRecording(SDVR_ENC_SECONDARY, -1);
}

void MainWindow::onVideoPanelBothStartRecording()
{
    doOnVideoPanelStartRecording(SDVR_ENC_PRIMARY,SDVR_ENC_SECONDARY);
}
void MainWindow::onVideoPanelBothStopRecording()
{
    doOnVideoPanelStopRecording(SDVR_ENC_PRIMARY,SDVR_ENC_SECONDARY);
}

void MainWindow::doOnVideoPanelStartRecording(int sub1Enc, int sub2Enc)
{
    Camera *cam;
    VideoPanel *panel = videoPanel();

    cam = g_DVR->findCameraByHandle(panel->settings()->selectedDisplayChannel());
    if (cam)
    {
        if (sub1Enc != -1)
            cam->startRecording(sub1Enc);
        if (sub2Enc != -1)
            cam->startRecording(sub2Enc);
    }
}

void MainWindow::doOnVideoPanelStopRecording(int sub1Enc, int sub2Enc)
{
    Camera *cam;
    VideoPanel *panel = videoPanel();

    cam = g_DVR->findCameraByHandle(panel->settings()->selectedDisplayChannel());
    if (cam)
    {
        if (sub1Enc != -1)
            cam->stopRecording(sub1Enc);
        if (sub2Enc != -1)
            cam->stopRecording(sub2Enc);
    }
}

void MainWindow::onVideoPanelStartPlayer()
{
    CPlayer *player;
    VideoPanel *panel = videoPanel();

    player = g_DVR->findDecoderByHandle(panel->settings()->selectedDisplayChannel());
    if (player)
        player->startDecoding(continuousDecodingAct->isChecked());
}

void MainWindow::onVideoPanelStopPlayer()
{
    CPlayer *player;
    VideoPanel *panel = videoPanel();

    player = g_DVR->findDecoderByHandle(panel->settings()->selectedDisplayChannel());
    if (player)
        player->stopDecoding();
}

void MainWindow::onVideoPanelPauseResumePlayer()
{
    CPlayer *player;
    VideoPanel *panel = videoPanel();
    playerRunningState_e curState;

    player = g_DVR->findDecoderByHandle(panel->settings()->selectedDisplayChannel());
    if (player)
    {
        curState =  player->getPlaybackState();
        if (curState != PLAYER_STATE_STOPPED)
        {
            player->setPlaybackState(curState != PLAYER_STATE_PAUSED ? PLAYER_STATE_PAUSED : PLAYER_STATE_PLAYBACK);
        }
    }

}
void MainWindow::onVideoPanelPlayBack()
{
    Camera *cam;
    VideoPanel *panel = videoPanel();

    cam = g_DVR->findCameraByHandle(panel->settings()->selectedDisplayChannel());

//todo: Add support for both primary and secondary
    if (cam)
        playBack(SDVR_ENC_PRIMARY, cam);
}

// onVideoPanelYUVRecording --
//
// This method is one of the context menu on the video panel window.
// It starts or stops saving of the YUV buffers in a file.
// At every time only one channel can save its YUV buffers. This
// decission was made for performance reasons.
//
void MainWindow::onVideoPanelYUVRecording()
{
    VideoPanel *panel = videoPanel();
    sdvr_chan_handle_t chanHandle;

    chanHandle = panel->settings()->selectedDisplayChannel();
    // A pannel was selected with no assoicated camera
    if (chanHandle == INVALID_CHAN_HANDLE)
        return;
    //The current video panel does not correspond
    //to the camera that we are saving its raw video frames.
    if (m_yuvSaveChanHandle && chanHandle != m_yuvSaveChanHandle)
        return;

    // toggle the current recording state
    if (m_yuvSaveChanHandle)
    {
        closeYUVFile();
    }
    else
    {
        QString deviceName = g_DVR->getNameFromHandle(chanHandle);
        QString fileName = recordFilePath(deviceName);
        QString menuText;

        m_qFileyuvSaveFile.setFileName(fileName);
        if (!m_qFileyuvSaveFile.open(QIODevice::WriteOnly))
        {
            Log::error(QString("Error: failed to open file '%1' for recording. Error = [%2]")
                .arg(fileName).arg(m_qFileyuvSaveFile.errorString()));

            return;
        }
        m_yuvSaveChanHandle = chanHandle;
        menuText = QString("Stop Saving %1 YUV Buffers").arg(deviceName);
        videoPanelSaveYUVFramesAct->setText(menuText);
    }
}

// onVideoPanelRawAudioRecording --
//
// This method is one of the context menu on the video panel window.
// It starts or stops saving of the raw audio buffers in a file.
// At every time only one channel can save its raw audio buffers. This
// decission was made for performance reasons.
//
void MainWindow::onVideoPanelRawAudioRecording()
{
    VideoPanel *panel = videoPanel();
    sdvr_chan_handle_t chanHandle;

    chanHandle = panel->settings()->selectedDisplayChannel();
    // A pannel was selected with no assoicated camera
    if (chanHandle == INVALID_CHAN_HANDLE)
        return;
    //The current video panel does not correspond
    //to the camera that we are saving its raw video frames.
    if (m_rawAudioSaveChanHandle && chanHandle != m_rawAudioSaveChanHandle)
        return;

    // toggle the current recording state
    if (m_rawAudioSaveChanHandle)
    {
//mxa removeit        sdvr_stream_raw_audio(m_rawAudioSaveChanHandle, false);
        closeRawAudioFile();
    }
    else
    {
        QString deviceName = g_DVR->getNameFromHandle(chanHandle);
        QString fileName = rawAudioFilePath(deviceName);
        QString menuText;

        m_qFileRawAudioSaveFile.setFileName(fileName);
        if (!m_qFileRawAudioSaveFile.open(QIODevice::WriteOnly))
        {
            Log::error(QString("Error: failed to open file '%1' for recording. Error = [%2]")
                .arg(fileName).arg(m_qFileRawAudioSaveFile.errorString()));

            return;
        }
        m_rawAudioSaveChanHandle = chanHandle;

//mxa removeit        sdvr_stream_raw_audio(m_rawAudioSaveChanHandle, true);

        menuText = QString("Stop Saving %1 Raw Audio Buffers").arg(deviceName);
        videoPanelSaveRawAudioFramesAct->setText(menuText);
    }
}
void MainWindow::onVideoPanelEnableRawAudio()
{
    Camera *cam;
    VideoPanel *panel = videoPanel();

    cam = g_DVR->findCameraByHandle(panel->settings()->selectedDisplayChannel());

    if (cam)
    {
        cam->setRawAudio(!cam->isRawAudioEnabled());
    }
}

void MainWindow::onRelays()
{
// todo: must read the current boardIndex
    uint boardIndex = 0;

    RelaysDialog dialog(boardIndex, this);
    if (dialog.exec() == QDialog::Accepted)
    {
    }
}
void MainWindow::onSensorConfig()
{
// todo: must read the current boardIndex
    uint boardIndex = 0;

    SensorsCnfgDialog dialog(boardIndex, this);
    if (dialog.exec() == QDialog::Accepted)
    {
    }
}

void MainWindow::onDisableWatchDog()
{

    m_bDisableWatchDogTimer = !m_bDisableWatchDogTimer;
    disableWatchDogAct->setChecked(m_bDisableWatchDogTimer);

    if (m_bDisableWatchDogTimer || g_DVR->getWatchDogState())
        m_watchDogTimer.stop();
    else
    {
        m_watchDogTimer.start((g_DVR->getWatchDogTimeout() / 2) * 1000);//(WATCHDOG_TIMER_VALUE);
    }

}

void MainWindow::onSetYUV422Convert()
{
    QString qstrPrompt;
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint32 board_count = g_DVR->boardCount();

    if (m_bSetYUV422Convert)
        return;         // Ignore if it is already set

    for (sx_uint32 index = 0; index < board_count; ++index)
    {
        err = sdvr_set_yuv_format(index, SDVR_RAWV_FORMAT_YUV_4_2_2);
        if (err != SDVR_ERR_NONE)
        {
            qstrPrompt = QString("Failed to YUV 4-2-2 raw video format on board %1.").arg(index + 1);
            Log::chk_err(qstrPrompt,err, true, index);
            break;
        }

    }

    if (err == SDVR_ERR_NONE)
    {
        m_bSetYUV422Convert = true;
        m_bSetYUV420Convert = false;
        m_bSetYVU420Convert = false;
        setYUV422VideoAct->setChecked(m_bSetYUV422Convert);
        setYUV420VideoAct->setChecked(m_bSetYUV420Convert);
        setYVU420VideoAct->setChecked(m_bSetYVU420Convert);
    }
}

void MainWindow::onSetYUV420Convert()
{
    QString qstrPrompt;
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint32 board_count = g_DVR->boardCount();

    if (m_bSetYUV420Convert)
        return;         // Ignore if it is already set

    for (sx_uint32 index = 0; index < board_count; ++index)
    {
        err = sdvr_set_yuv_format(index, SDVR_RAWV_FORMAT_YUV_4_2_0);
        if (err != SDVR_ERR_NONE)
        {
            qstrPrompt = QString("Failed to YVU 4-2-0 raw video format on board %1.").arg(index + 1);
            Log::chk_err(qstrPrompt,err, true, index);
            break;
        }

    }

    if (err == SDVR_ERR_NONE)
    {
        m_bSetYUV422Convert = false;
        m_bSetYUV420Convert = true;
        m_bSetYVU420Convert = false;
        setYUV422VideoAct->setChecked(m_bSetYUV422Convert);
        setYUV420VideoAct->setChecked(m_bSetYUV420Convert);
        setYVU420VideoAct->setChecked(m_bSetYVU420Convert);
    }

}
void MainWindow::onSetYVU420Convert()
{
    QString qstrPrompt;
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint32 board_count = g_DVR->boardCount();

    if (m_bSetYVU420Convert)
        return;         // Ignore if it is already set

    for (sx_uint32 index = 0; index < board_count; ++index)
    {
        err = sdvr_set_yuv_format(index, SDVR_RAWV_FORMAT_YVU_4_2_0);
        if (err != SDVR_ERR_NONE)
        {
            qstrPrompt = QString("Failed to YUV 4-2-0 raw video format on board %1.").arg(index + 1);
            Log::chk_err(qstrPrompt,err, true, index);
            break;
        }

    }

    if (err == SDVR_ERR_NONE)
    {
        m_bSetYUV422Convert = false;
        m_bSetYUV420Convert = false;
        m_bSetYVU420Convert = true;
        setYUV422VideoAct->setChecked(m_bSetYUV422Convert);
        setYUV420VideoAct->setChecked(m_bSetYUV420Convert);
        setYVU420VideoAct->setChecked(m_bSetYVU420Convert);
    }

}

void MainWindow::onSetSkipRawVideoDisplay()
{
    m_bSkipRawVideoDisplay = !m_bSkipRawVideoDisplay;
}

void MainWindow::onIOCTLTest()
{
    IOCTLDialog dialog(this);
    dialog.exec();
}


void MainWindow::onVideoPanelCameraEdit()
{
    Camera *cam;
    VideoPanel *panel = videoPanel();

    cam = g_DVR->findCameraByHandle(panel->settings()->selectedDisplayChannel());
    if (cam != NULL)
        doCameraEdit(cam);
}

bool MainWindow::doCameraEdit(Camera *cam)
{
    bool bRet = false;
    sdvr_err_e err;
    int retry = 0;

    if (cam == NULL)
        return bRet;

    Camera origCam(*cam, true); // duplicate the original camera incase
                                // we need to re-store the settings.
    err = SDVR_ERR_NONE;
    CameraDialog dialog(cam, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        if (dialog.isNameModified())
            updateChannelSelector();

        if (cam->isChannelOpen())
        {
            do {
                if (dialog.isYUVFrameRateModified())
                {
                    err = cam->applyYUVFrameRate();
                    Log::chk_err(QString("%1: Failed to save YUV frame rate.").arg(cam->name()),err,true,cam->boardIndex());
                }
                if (dialog.isMotionFrequencyModified())
                {
                    err = cam->applyMotionFrequency();
                    Log::chk_err(QString("%1: Failed to save motion frequency rate.").arg(cam->name()),err,true,cam->boardIndex());
                }

                if (dialog.isOsdModified())
                {
                    err = cam->applyOsd();
                    Log::chk_err(QString("%1: Failed to save OSD parameters.").arg(cam->name()),err,true,cam->boardIndex());
                }

                if (dialog.isVideoEncodingModified(SDVR_ENC_PRIMARY))
                {
                    err = cam->applyVideoEncoding(SDVR_ENC_PRIMARY);
                    Log::chk_err(QString("%1: Failed to save primary video encoder parameters.").arg(cam->name()),err,true,cam->boardIndex());
                }

                if (dialog.isVideoEncodingModified(SDVR_ENC_SECONDARY))
                {
                    err = cam->applyVideoEncoding(SDVR_ENC_SECONDARY);
                    Log::chk_err(QString("%1: Failed to save secondary video encoder parameters.").arg(cam->name()),err,true,cam->boardIndex());
                }

                if (dialog.isOnAlarmVideoEncodingModified(SDVR_ENC_PRIMARY))
                {
                    err = cam->applyOnAlarmVideoEncoding(SDVR_ENC_PRIMARY);
                    Log::chk_err(QString("%1: Failed to save primary alaram video encoder parameters.").arg(cam->name()),err,true,cam->boardIndex());

                }

                if (dialog.isOnAlarmVideoEncodingModified(SDVR_ENC_SECONDARY))
                {
                    err = cam->applyOnAlarmVideoEncoding(SDVR_ENC_SECONDARY);
                    Log::chk_err(QString("1: Failed to save secondary alaram video encoder parameters.").arg(cam->name()),err,true,cam->boardIndex());

                }

                if (dialog.isMDModified())
                {
                    err = cam->applyMD();
                    Log::chk_err(QString("%1: Failed to save motion detection parameters.").arg(cam->name()),err,true,cam->boardIndex());
                }

                if (dialog.isMDRegionsModified())
                {
                    err = cam->applyMDRegions();
                    Log::chk_err(QString("%1: Failed to save motion detection regions.").arg(cam->name()),err,true,cam->boardIndex());
                }

                if (dialog.isBDModified())
                {
                    err = cam->applyBD();
                    Log::chk_err(QString("%1: Failed to save blind detection parameters.").arg(cam->name()),err,true,cam->boardIndex());

                }
                if (dialog.isBDRegionsModified())
                {
                    err = cam->applyBDRegions();
                    Log::chk_err(QString("%1: Failed to save blind detection regions.").arg(cam->name()),err,true,cam->boardIndex());
                }

                if (dialog.isNDModified())
                {
                    err = cam->applyND();
                    Log::chk_err(QString("%1: Failed to save night detection parameters.").arg(cam->name()),err,true,cam->boardIndex());

                }

                if (dialog.isPRModified())
                {
                    err = cam->applyPR();
                    Log::chk_err(QString("%1: Failed to save privacy blocking parameters.").arg(cam->name()),err,true,cam->boardIndex());

                }

                if (dialog.isPRRegionsModified())
                {
                    err = cam->applyPRRegions();
                    Log::chk_err(QString("%1: Failed to save privacy blocking regions.").arg(cam->name()),err,true,cam->boardIndex());

                }


                if (dialog.isVideoInModified())
                {
                    err = cam->applyVideoInSettings();
                    Log::chk_err(QString("%1: Failed to save video-in parameters.").arg(cam->name()),err,true,cam->boardIndex());
                }
                if (dialog.isDeinterlacingModified())
                {
                    err = sdvr_enable_deinterlacing(cam->handle(),cam->isVParamsDeinterlacing());
                    Log::chk_err(QString("%1: Failed to save video de-interlacing.").arg(cam->name()),err,true,cam->boardIndex());
                }
                if (dialog.isNoiseReductionModified())
                {
                    err = sdvr_enable_noise_reduction(cam->handle(),cam->isVParamNoiseReduction());
                    Log::chk_err(QString("%1: Failed to save video noise reduction.").arg(cam->name()),err,true,cam->boardIndex());
                }
                if (dialog.isGainModeModified())
                {
                    err = sdvr_set_gain_mode(cam->handle(),cam->getVParamGainMode());
                    Log::chk_err(QString("%1: Failed to save camera gain mode.").arg(cam->name()),err,true,cam->boardIndex());
                }
                if (dialog.isCamTerminationModifed())
                {
                    err = sdvr_set_camera_termination(cam->handle(),cam->getVParamCamTermination());
                    Log::chk_err(QString("%1: Failed to save camera termination.").arg(cam->name()),err,true,cam->boardIndex());
                }

                /*********************************************************
                 In case there was any error saving the camera parameters
                 other than FW timeout, then we must re-store the old
                 camera parameters that were changed.
                *********************************************************/
                if (err == SDVR_ERR_NONE || err == SDVR_DRV_ERR_MSG_RECV || ++retry > 1)
                    break;

                // re-store the old parameters of the camera
                *cam = origCam;
                Log::info(QString("%1: Restoring all camera parameters.").arg(cam->name()));

            } while (retry < 2);
        }
        bRet = (err == SDVR_ERR_NONE) && (retry < 2);
    }
    else
    {
        // If any of the video In setting were modified,
        // cancel it by re-storing the old settings.
        if (dialog.isVideoInModified())
            err = origCam.applyVideoInSettings();
#ifdef mxadisable
        // If we used to have some privacy blocking defined prior
        // getting into the the Regions Dialog box and the user
        // cancel his changes, we must re-set the old privacy regions
        // to the board.
        if (!origCam.PRRegion().isEmpty())
        {
            err =  sdvr_set_regions_map(cam->handle(),
                                    SDVR_REGION_PRIVACY,
                                    (sx_uint8*)origCam.PRRegion().data());
        }
#endif
    }

    if (!bRet)
    {
        // re-store the old parameters of the camera
        *cam = origCam;

    } // if (!bRet)

    return bRet;
}
void MainWindow::onVideoPanelPlayerEdit()
{
    CPlayer *player;
    VideoPanel *panel = videoPanel();

    player = g_DVR->findDecoderByHandle(panel->settings()->selectedDisplayChannel());
    if (player == NULL)
        return;

    DecoderDialog dlgDecoder(player, g_DVR,this);

    dlgDecoder.setNewPlayer(false);
    dlgDecoder.setWindowTitle("Edit Decoder");

    dlgDecoder.setDecoderName(player->getName());
    dlgDecoder.setVidoeFormat(player->getVideoFormat());
    // todo: for now audio is disabled
    dlgDecoder.setAudioFormat(SDVR_AUDIO_ENC_NONE);
    dlgDecoder.setDecoderPath(player->getEncodedFilePath());

    dlgDecoder.setVideoSize(player->getVideoSize());

    dlgDecoder.initDialog();
    if (dlgDecoder.exec() == QDialog::Accepted)
    {
    } //if (dlgDecoder.exec() == QDialog::Accepted)
}

void MainWindow::onSelectStatData()
{
    StatViewDialog dialog(m_recView, this);
    if (dialog.exec() == QDialog::Accepted)
    	// Save the selection in the system object too.
    	g_DVR->setStatViewInfo(m_recView->selectedInfo());
}

void MainWindow::onNewMonitor()
{
    ViewSettings *ms = MonitorDialog::getMonitorSettings(this);
    if (ms != NULL)
    {
        g_DVR->monitorList.append(ms);

        updateMonitorSelector();

        m_monitorSelector->setCurrentIndex(g_DVR->monitorList.count());
        onMonitorSelected(g_DVR->monitorList.count());
    }
}

void MainWindow::onDeleteMonitor()
{
    int curIndex = m_monitorSelector->currentIndex();
    if (curIndex > 0)
    {
        QString monitorName = m_monitorSelector->currentText();
        ViewSettings *ms = g_DVR->findMonitorByName(monitorName);

        Q_ASSERT(ms != NULL);
        Q_ASSERT(ms == currentMonitorSettings());

        // Clear all SMOs currently showing this monitor
        foreach (SpotMonitor *smo, g_DVR->smoList)
            if (smo->view() == ms)
                smo->clear();

        setVideoPanel(NULL);
        g_DVR->monitorList.removeAll(ms);
        updateMonitorSelector();

        delete ms;
    }
}

void MainWindow::onRenameMonitor()
{
    int curIndex = m_monitorSelector->currentIndex();
    if (curIndex > 0)
    {
        QString curName = m_monitorSelector->currentText();
        ViewSettings *ms = g_DVR->findMonitorByName(curName);

        Q_ASSERT(ms != NULL);
        Q_ASSERT(ms == currentMonitorSettings());

        QString newName = MonitorDialog::getNewName(curName, this);
        if (!newName.isEmpty())
        {
            ms->setName(newName);
            m_monitorSelector->setItemText(curIndex, newName);
        }

    }
}

void MainWindow::onClearLogText()
{
    Log::clearText();
}
void MainWindow::onSkipLogInfo()
{
    Log::toggleInfoDisplay();
}

void MainWindow::onViewMenuAboutToShow()
{
    newMonitorAct->setEnabled(g_DVR->isOpen());

    bool monitorSelected = (m_monitorSelector->currentIndex() > 0);
    deleteMonitorAct->setEnabled(monitorSelected);
    renameMonitorAct->setEnabled(monitorSelected);

    clearLogAct->setEnabled(m_logDock->isVisible());
    skipLogInfoAct->setEnabled(m_logDock->isVisible());

    if (g_DVR->isOpen())
        foreach (SpotMonitor *smo, g_DVR->smoList)
            smoMenu(smo);
    else
    {
        for (int boardIndex = 0; boardIndex < g_DVR->boardCount(); boardIndex++)
        {
            QMenu *menu = m_smoMenu[boardIndex];

            if (menu != NULL)
                menu->setDisabled(true);
//                menu->clear();
        }
    }
}

void MainWindow::onToolsMenuAboutToShow()
{
    bool monitorSelected = (m_monitorSelector->currentIndex() > 0);

    relaysAct->setEnabled(g_DVR->isOpen() && (g_DVR->boardCount() > 0) );
    sensorCnfgAct->setEnabled(g_DVR->isOpen() && (g_DVR->boardCount() > 0) );

    // Adjust all the extended feature menus
    if (g_DVR->isExtendedFeatures())
    {
        setYUV422VideoAct->setVisible(true);
        setYUV420VideoAct->setVisible(true);
        setYVU420VideoAct->setVisible(true);
        setYUV422VideoAct->setChecked(m_bSetYUV422Convert);
        setYUV420VideoAct->setChecked(m_bSetYUV420Convert);
        setYVU420VideoAct->setChecked(m_bSetYVU420Convert);

        setSkipRawVideoDisplayAct->setVisible(true);
        setSkipRawVideoDisplayAct->setEnabled(!monitorSelected);
    }

}

void MainWindow::onRecViewMenuAboutToShow()
{
    bool enableStartPrimary = false;
    bool enableStopPrimary = false;
    bool enableStartSecondary = false;
    bool enableStopSecondary = false;

    CameraList list = m_recView->selectedCameras();

    for (CameraList::const_iterator i = list.begin(); i != list.end(); ++i)
    {
        if ((*i)->videoFormat(SDVR_ENC_PRIMARY) != SDVR_VIDEO_ENC_NONE)
        {
            if ((*i)->isRecording(SDVR_ENC_PRIMARY))
                enableStopPrimary = true;
            else
                enableStartPrimary = true;

            if ((*i)->videoFormat(SDVR_ENC_SECONDARY) != SDVR_VIDEO_ENC_NONE)
            {
                if ((*i)->isRecording(SDVR_ENC_SECONDARY))
                    enableStopSecondary = true;
                else
                    enableStartSecondary = true;
            }
        }
    }

    m_recViewStartRecordMenu->setEnabled(enableStartPrimary || enableStartSecondary);
    m_recViewStopRecordMenu->setEnabled(enableStopPrimary || enableStopSecondary);

    recViewStartPrimaryRecordingAct->setEnabled(enableStartPrimary);
    recViewStopPrimaryRecordingAct->setEnabled(enableStopPrimary);

    recViewStartSecondaryRecordingAct->setEnabled(enableStartSecondary);
    recViewStopSecondaryRecordingAct->setEnabled(enableStopSecondary);

    recViewStopBothRecordingAct->setEnabled(enableStopPrimary && enableStopSecondary);
    recViewStartBothRecordingAct->setEnabled(enableStartPrimary && enableStartSecondary);


    recViewPlayBackAct->setEnabled(
        list.count() == 1 && list.first()->canPlayBack(SDVR_ENC_PRIMARY));

    recViewEditCameraAct->setEnabled(list.count() == 1);
}

void MainWindow::onPlayerViewMenuAboutToShow()
{
    bool enableStart = false;
    bool enableStop = false;
    bool enablePauseResume = false;
    bool bResume = false;
    bool bPause = false;

    PlayerList list = m_decoderView->selectedDecoders();

    // Check all the players to see if they are all only in
    // pause mode or resume mode. Or they are in the mix mode
    // which in this case the menu item say Pause/Resume to
    // invert the current state of the selected players
    for (PlayerList::const_iterator player = list.begin(); player != list.end(); ++player)
    {
        if ((*player)->isDecoding())
        {
            enableStop = true;
            enablePauseResume = true;
            if ((*player)->getPlaybackState() == PLAYER_STATE_PAUSED)
            {
                bResume = true;
            }
            else
            {
                bPause = true;
            }
        }
        else
        {
            enableStart = true;
        }
    }

    if (bPause && bResume)
        playerViewPauseResumeDecodingAct->setText("Toggel Pause/Resume Playback");
    else if (bResume)
        playerViewPauseResumeDecodingAct->setText("Resume Playback");
    else
        playerViewPauseResumeDecodingAct->setText("Pause Playback");

    playerViewStartDecodingAct->setEnabled(enableStart);
    playerViewStopDecodingAct->setEnabled(enableStop);
    playerViewPauseResumeDecodingAct->setEnabled(enablePauseResume);
    continuousDecodingAct->setVisible(true);
    continuousDecodingAct->setEnabled(!list.isEmpty());

    playerViewEditPropertiesAct->setEnabled(list.count() == 1 && enableStart);
}
void MainWindow::onPlayerMenuAboutToShow()
{
    playerNewAct->setEnabled(g_DVR->isOpen());
    playerEditPropertiesAct->setEnabled(false);
    playerDeleteAct->setEnabled(false);
    playerStartDecodingAct->setEnabled(false);
    playerStopDecodingAct->setEnabled(false);
    playerPauseResumeDecodingAct->setEnabled(false);
    playerConnectAct->setVisible(false);
    playerDisconnectAct->setVisible(false);
    continuousDecodingAct->setEnabled(true);

    QList<QTreeWidgetItem *> selItems = m_sysTree->selectedItems();
    CPlayer *decoder = NULL;
    if (selItems.isEmpty())
        return;

    QTreeWidgetItem *item = selItems.first();

    if (item == NULL)
        return;

    playerDeleteAct->setVisible(item->type() == DVRSystemTree::DECODER);

    decoder = GetDecoderFromContextMenu(selItems.first());
    if (decoder == NULL)
        return;

    if (item->type() == DVRSystemTree::DECODER ||
        item->type() == DVRSystemTree::BOARDDECODERITEM)
    {
        playerEditPropertiesAct->setEnabled(true);
        playerDeleteAct->setEnabled(item->type() == DVRSystemTree::DECODER);

        if (decoder->isChannelOpen())
        {
            // if decoder is connected
            continuousDecodingAct->setEnabled(true);
            playerDisconnectAct->setVisible(true);
            playerDisconnectAct->setEnabled(!decoder->isDecoding());
            playerEditPropertiesAct->setEnabled(!decoder->isDecoding());

            if (decoder->isDecoding())
            {
                playerStopDecodingAct->setEnabled(true);
                playerPauseResumeDecodingAct->setEnabled(true);
                if (decoder->getPlaybackState() == PLAYER_STATE_PAUSED)
                {
                    playerPauseResumeDecodingAct->setText("Resume Playback");
                }
                else
                {
                    playerPauseResumeDecodingAct->setText("Pause Playback");
                }
            }
            else
            {
                playerStartDecodingAct->setEnabled(true);
                playerPauseResumeDecodingAct->setText("Pause Playback");
            }
        }
        else
        {
            // If decoder is not connected
            playerConnectAct->setVisible(true);
            playerConnectAct->setEnabled(item->type() == DVRSystemTree::DECODER);
        }
    }
}

CPlayer *MainWindow::GetDecoderFromContextMenu(QTreeWidgetItem *item)
{
    CPlayer *decoder = NULL;
    int decoderId;

    if (item->type() == DVRSystemTree::DECODER)
    {
        decoderId = ((CPlayerItem *)item)->getDecoderId();
        decoder = g_DVR->findDecoderById(decoderId);
    }
    else if (item->type() == DVRSystemTree::BOARDDECODERITEM)
    {
        CBoardDecoderItem * boardDecoderItem = (CBoardDecoderItem *)item;
        decoder = g_DVR->findDecoderByChanNum(
                boardDecoderItem->boardIndex(), boardDecoderItem->chan_num());
    }
    return decoder;
}
void MainWindow::onVideoPanelMenuAboutToShow()
{
    bool enableRawAudio = false;
    bool enableStartPrimary = false;
    bool enableStopPrimary = false;
    bool enableStartSecondary = false;
    bool enableStopSecondary = false;

    bool enablePlayback = false;
    bool enableEdit = false;
    bool enableSaveYUV = false;
    bool enableSaveRawAudio = false;
    bool enableDecodingStop = false;
    bool enableDecodingPauseResume = false;
    bool enableDecodingStart = false;
    CPlayer *player;
    Camera *cam = NULL;
    sdvr_chan_type_e chan_type;
    VideoPanel *panel = videoPanel();
    sdvr_chan_handle_t chanHandle;

    if (!panel)
        return;

    videoPanelPlayBackAct->setVisible(false);

    m_videoPanelStartRecordMenu->setEnabled(false);
    m_videoPanelStopRecordMenu->setEnabled(false);

    videoPanelStartPrimaryRecordingAct->setVisible(false);
    videoPanelStopPrimaryRecordingAct->setVisible(false);
    videoPanelStartSecondaryRecordingAct->setVisible(false);
    videoPanelStopSecondaryRecordingAct->setVisible(false);
    videoPanelStartBothRecordingAct->setVisible(false);
    videoPanelStopBothRecordingAct->setVisible(false);

    videoPanelStartDecodingAct->setVisible(false);
    videoPanelStopDecodingAct->setVisible(false);
    videoPanelPauseResumeDecodingAct->setVisible(false);
    continuousDecodingAct->setVisible(false);

    videoPanelEditCameraAct->setVisible(false);
    videoPanelEditPlayerAct->setVisible(false);
    continuousDecodingAct->setEnabled(false);

    videoPanelEnableRawAudioAct->setEnabled(false);
    videoPanelEnableRawAudioAct->setChecked(false);

    chanHandle = panel->settings()->selectedDisplayChannel();
    chan_type = sdvr_get_chan_type(chanHandle);
    if (chan_type == SDVR_CHAN_TYPE_ENCODER)
    {
        cam = g_DVR->findCameraByHandle(chanHandle);
        if (cam != NULL && cam->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_NONE)
            chan_type = SDVR_CHAN_TYPE_NONE;
    }

    switch (chan_type)
    {
    case SDVR_CHAN_TYPE_DECODER:

        videoPanelStartDecodingAct->setVisible(true);
        videoPanelStopDecodingAct->setVisible(true);
        videoPanelEditPlayerAct->setVisible(true);
        videoPanelPauseResumeDecodingAct->setVisible(true);
        continuousDecodingAct->setVisible(true);
        continuousDecodingAct->setEnabled(true);

        player = g_DVR->findDecoderByHandle(chanHandle);
        if (player)
        {
            if (player->isDecoding())
            {
                enableSaveYUV = true;
                enableDecodingStop = true;
                enableDecodingPauseResume = true;
                if (player->getPlaybackState() == PLAYER_STATE_PAUSED)
                {
                    videoPanelPauseResumeDecodingAct->setText("Resume Playback");
                }
                else
                {
                    videoPanelPauseResumeDecodingAct->setText("Pause Playback");
                }
                // We can only save YUV for one camera at a time.
                enableSaveYUV = (m_yuvSaveChanHandle == 0) ||
                 (m_yuvSaveChanHandle && (chanHandle == m_yuvSaveChanHandle));

            }
            else
            {
                enableDecodingStart = true;
                videoPanelPauseResumeDecodingAct->setText("Pause Playback");
            }
            enableEdit = !player->isDecoding();
        }
        videoPanelStartDecodingAct->setEnabled(enableDecodingStart);
        videoPanelStopDecodingAct->setEnabled(enableDecodingStop);
        videoPanelPauseResumeDecodingAct->setEnabled(enableDecodingPauseResume);
        videoPanelEditPlayerAct->setEnabled(enableEdit);

        break;
    case SDVR_CHAN_TYPE_ENCODER:

        videoPanelStartPrimaryRecordingAct->setVisible(true);
        videoPanelStopPrimaryRecordingAct->setVisible(true);
        videoPanelStartSecondaryRecordingAct->setVisible(true);
        videoPanelStopSecondaryRecordingAct->setVisible(true);
        videoPanelStartBothRecordingAct->setVisible(true);
        videoPanelStopBothRecordingAct->setVisible(true);

        videoPanelEditCameraAct->setVisible(true);
        videoPanelPlayBackAct->setVisible(true);
        if (cam)
        {
            if (cam->isRecording(SDVR_ENC_PRIMARY))
                enableStopPrimary = true;
            else
                enableStartPrimary = (cam->videoFormat(SDVR_ENC_PRIMARY) != SDVR_VIDEO_ENC_NONE);

            if (cam->videoFormat(SDVR_ENC_SECONDARY) != SDVR_VIDEO_ENC_NONE)
            {
                if (cam->isRecording(SDVR_ENC_SECONDARY))
                    enableStopSecondary = true;
                else
                    enableStartSecondary = true;
            }
            enablePlayback = cam->canPlayBack(SDVR_ENC_PRIMARY);
            enableEdit = true;
            // We can only save YUV for one camera at a time.
            enableSaveYUV = (m_yuvSaveChanHandle == 0) ||
             (m_yuvSaveChanHandle && (chanHandle == m_yuvSaveChanHandle));

            enableRawAudio = true;

            // We can only save raw audio for one camera at a time.
            if (cam->isRawAudioEnabled())
            {
                enableSaveRawAudio = (m_rawAudioSaveChanHandle == 0) ||
                 (m_rawAudioSaveChanHandle && (chanHandle == m_rawAudioSaveChanHandle));
            }
            videoPanelEnableRawAudioAct->setChecked(cam->isRawAudioEnabled());

        }

        m_videoPanelStartRecordMenu->setEnabled(enableStartPrimary || enableStartSecondary);
        m_videoPanelStopRecordMenu->setEnabled(enableStopPrimary || enableStopSecondary);

        videoPanelStartPrimaryRecordingAct->setEnabled(enableStartPrimary);
        videoPanelStopPrimaryRecordingAct->setEnabled(enableStopPrimary);
        videoPanelStartSecondaryRecordingAct->setEnabled(enableStartSecondary);
        videoPanelStopSecondaryRecordingAct->setEnabled(enableStopSecondary);
        videoPanelStopBothRecordingAct->setEnabled(enableStopPrimary && enableStopSecondary);
        videoPanelStartBothRecordingAct->setEnabled(enableStartPrimary && enableStartSecondary);


        videoPanelEditCameraAct->setEnabled(enableEdit);
        videoPanelPlayBackAct->setEnabled(enablePlayback);
        break;

    case SDVR_CHAN_TYPE_NONE:
        videoPanelEditCameraAct->setVisible(true);

        enableEdit = true;

        enableSaveYUV = (m_yuvSaveChanHandle == 0) ||
         (m_yuvSaveChanHandle && (chanHandle == m_yuvSaveChanHandle));

        if (cam)
        {
            enableRawAudio = true;

            // We can only save raw audio for one camera at a time.
            if (cam->isRawAudioEnabled())
            {
                // We can only save raw audio for one camera at a time.
                enableSaveRawAudio = (m_rawAudioSaveChanHandle == 0) ||
                 (m_rawAudioSaveChanHandle && (chanHandle == m_rawAudioSaveChanHandle));
            }
            videoPanelEnableRawAudioAct->setChecked(cam->isRawAudioEnabled());
        }

        videoPanelEditCameraAct->setEnabled(enableEdit);
        break;

    default:
        break;
    }

    videoPanelSaveYUVFramesAct->setEnabled(enableSaveYUV);
    videoPanelSaveRawAudioFramesAct->setEnabled(enableSaveRawAudio);
    videoPanelEnableRawAudioAct->setEnabled(enableRawAudio);
}

void MainWindow::onSmoMenuTriggered(QAction *act)
{
    // Data is a pointer to the associated SpotMonitor object
    SpotMonitor *smo = static_cast<SpotMonitor *>(act->data().value<void *>());

    if (act->text() == SMO_OFF_ACTION_TEXT)
    {
        smo->clear();
    }
    else if (act->text() == SMO_PROPS_ACTION_TEXT)
    {
        editSmoProperties(smo);
    }
    else if (act->text() == SMO_PLAYBACK_ACTION_TEXT)
    {
        // Show the SMO player dock widget
        m_smoYuvPlayerDock->show();

        // Select the tab corresponding to the SMO
        QWidget *dockWidget = m_smoYuvPlayerDock->widget();
        if (dockWidget->inherits("QTabWidget"))
        {
            QTabWidget *tabWidget = (QTabWidget *) dockWidget;
            for (int i = 0; i < tabWidget->count(); ++i)
            {
                SmoYuvPlayer *player = (SmoYuvPlayer *) tabWidget->widget(i);
                if (player->spotMonitor() == smo)
                {
                    tabWidget->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
    else
    {
        ViewSettings *vs = g_DVR->findMonitorByName(act->text());
        if (vs != NULL && vs->isSmo())
        {
            smo->setView(vs);
        }
    }
}

void MainWindow::editSmoProperties(SpotMonitor *smo)
{
    SmoDialog dialog(smo, this);

    if (dialog.exec() == QDialog::Accepted)
    {
        sdvr_err_e err = SDVR_ERR_NONE;
        for (int i = 0; i < SDVR_MAX_OSD && err == SDVR_ERR_NONE; ++i)
        {
            if (dialog.isOsdModified(i))
            {
                if (smo->isOsdEnabled(i))
                {
                    err = sdvr_smo_osd_text_config(smo->board()->index(),
                        smo->port(), i, &(smo->osdConfig(i)));
                }

                if (err == SDVR_ERR_NONE)
                {
                    err = sdvr_smo_osd_text_show(smo->board()->index(),
                        smo->port(), i, smo->isOsdEnabled(i));
                }
            }
        }
    }
}

void MainWindow::about()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::createActions()
{
    newSystemAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newSystemAct->setStatusTip(tr("Configure a new system"));
    connect(newSystemAct, SIGNAL(triggered()), this, SLOT(onSystemNew()));

    openSystemAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openSystemAct->setStatusTip(tr("Open a previously configured system"));
    connect(openSystemAct, SIGNAL(triggered()), this, SLOT(onSystemOpen()));

    editSystemAct = new QAction(tr("&Edit..."), this);
    editSystemAct->setStatusTip(tr("View/Edit system properties"));
    connect(editSystemAct, SIGNAL(triggered()), this, SLOT(onSystemEdit()));

    closeSystemAct = new QAction(tr("&Close"), this);
    closeSystemAct->setStatusTip(tr("Close the currently open system"));
    connect(closeSystemAct, SIGNAL(triggered()), this, SLOT(onSystemClose()));

    saveSystemAct = new QAction(tr("&Save System"), this);
    saveSystemAct->setShortcut(tr("Ctrl+S"));
    saveSystemAct->setStatusTip(tr("Save the current system configuration"));
    connect(saveSystemAct, SIGNAL(triggered()), this, SLOT(onSaveSystemConfiguration()));

//    dbgDisableHMO = new QAction(tr("&Disable HMO"), this);
//    dbgDisableHMO->setShortcut(tr("Ctrl+Y"));
//    connect(dbgDisableHMO, SIGNAL(triggered()), this, SLOT(onDBGDisableHMO()));
    connect(new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_Y), this),
        SIGNAL(activated()), SLOT(onDBGDisableHMO()));


    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    continuousDecodingAct = new QAction(tr("Continuous Playback"), this);
    continuousDecodingAct->setCheckable(true);
    connect(continuousDecodingAct, SIGNAL(triggered()), this, SLOT(onContinuousDecoding()));

    newCameraAct = new QAction(tr("New Camera"), this);
    newCameraAct->setShortcut(tr("Ctrl+N"));
    newCameraAct->setStatusTip(tr("Define new camera"));
    connect(newCameraAct, SIGNAL(triggered()), this, SLOT(onCameraNew()));

    deleteCameraAct = new QAction(tr("Delete Camera"), this);
    deleteCameraAct->setShortcut(tr("Ctrl+D"));
    deleteCameraAct->setStatusTip(tr("Delete selected camera"));
    connect(deleteCameraAct, SIGNAL(triggered()), this, SLOT(onCameraDelete()));

    editCameraAct = new QAction(tr("Edit Properties..."), this);
    editCameraAct->setShortcut(tr("Ctrl+E"));
    editCameraAct->setStatusTip(tr("Edit camera properties"));
    connect(editCameraAct, SIGNAL(triggered()), this, SLOT(onCameraEdit()));

    connectCameraAct = new QAction(tr("Connect..."), this);
    connectCameraAct->setShortcut(tr("Ctrl+O"));

    connectCameraAct->setStatusTip(tr("Connect camera to video input"));
    connect(connectCameraAct, SIGNAL(triggered()), this, SLOT(onCameraConnect()));

    disconnectCameraAct = new QAction(tr("Disconnect"), this);
    disconnectCameraAct->setShortcut(tr("Ctrl+T"));

    disconnectCameraAct->setStatusTip(tr("Disconnect camera from video input"));
    connect(disconnectCameraAct, SIGNAL(triggered()),
            this, SLOT(onCameraDisconnect()));

    startPrimaryRecordingAct = new QAction(tr("Start Primary Recording"), this);
    connect(startPrimaryRecordingAct, SIGNAL(triggered()),
            SLOT(onPrimaryStartRecording()));

    stopPrimaryRecordingAct = new QAction(tr("Stop Primary Recording"), this);
    connect(stopPrimaryRecordingAct, SIGNAL(triggered()),
            SLOT(onPrimaryStopRecording()));

    startSecondaryRecordingAct = new QAction(tr("Start Secondary Recording"), this);
    connect(startSecondaryRecordingAct, SIGNAL(triggered()),
            SLOT(onSecondaryStartRecording()));

    stopSecondaryRecordingAct = new QAction(tr("Stop Secondary  Recording"), this);
    connect(stopSecondaryRecordingAct, SIGNAL(triggered()),
            SLOT(onSecondaryStopRecording()));

    startBothRecordingAct = new QAction(tr("Start All Recording"), this);
    connect(startBothRecordingAct, SIGNAL(triggered()),
            SLOT(onBothStartRecording()));

    stopBothRecordingAct = new QAction(tr("Stop All  Recording"), this);
    connect(stopBothRecordingAct, SIGNAL(triggered()),
            SLOT(onBothStopRecording()));

    playBackAct = new QAction(tr("External Playback Player"), this);
    connect(playBackAct, SIGNAL(triggered()), this, SLOT(onPlayBack()));


    //viewSettingsActGroup = new QActionGroup(this);
    //for (int i = 0; i < m_viewSettings.size(); ++i)
    //{
    //    QAction *act = new QAction(m_viewSettings[i]->name(), this);
    //    act->setCheckable(true);
    //    act->setData(QVariant(i));
    //    connect(act, SIGNAL(triggered()), this, SLOT(changeView()));
    //    viewSettingsActGroup->addAction(act);
    //}

    viewLogAct = m_logDock->toggleViewAction();
    viewTreeAct = m_sysNavigator->toggleViewAction();
    viewRecViewAct = m_recViewDock->toggleViewAction();
    viewDecoderViewAct = m_DecoderViewDock->toggleViewAction();
    viewSmoYuvPlayerAct = m_smoYuvPlayerDock->toggleViewAction();

    selectStatData = new QAction(tr("System Status Info..."), this);
    connect(selectStatData, SIGNAL(triggered()), SLOT(onSelectStatData()));

    newMonitorAct = new QAction(tr("New Monitor..."), this);
    newMonitorAct->setEnabled(false);
    connect(newMonitorAct, SIGNAL(triggered()), this, SLOT(onNewMonitor()));

    deleteMonitorAct = new QAction(tr("Delete Monitor"), this);
    deleteMonitorAct->setEnabled(false);
    connect(deleteMonitorAct, SIGNAL(triggered()), this, SLOT(onDeleteMonitor()));

    renameMonitorAct = new QAction(tr("Rename Monitor..."), this);
    renameMonitorAct->setEnabled(false);
    connect(renameMonitorAct, SIGNAL(triggered()), this, SLOT(onRenameMonitor()));

    clearLogAct = new QAction(tr("Clear System Log Text"), this);
    connect(clearLogAct, SIGNAL(triggered()), this, SLOT(onClearLogText()));

    skipLogInfoAct = new QAction(tr("Skip System Log Info"), this);
    skipLogInfoAct->setCheckable(true);
    connect(skipLogInfoAct, SIGNAL(triggered()), this, SLOT(onSkipLogInfo()));


    recViewStartPrimaryRecordingAct = new QAction(tr("Start Primary Recording"), this);
    connect(recViewStartPrimaryRecordingAct, SIGNAL(triggered()),
            SLOT(onRecViewPrimaryStartRecording()));

    recViewStopPrimaryRecordingAct = new QAction(tr("Stop Primary Recording"), this);
    connect(recViewStopPrimaryRecordingAct, SIGNAL(triggered()),
            SLOT(onRecViewPrimaryStopRecording()));

    recViewStartSecondaryRecordingAct = new QAction(tr("Start Secondary Recording"), this);
    connect(recViewStartSecondaryRecordingAct, SIGNAL(triggered()),
            SLOT(onRecViewSecondaryStartRecording()));

    recViewStopSecondaryRecordingAct = new QAction(tr("Stop Secondary  Recording"), this);
    connect(recViewStopSecondaryRecordingAct, SIGNAL(triggered()),
            SLOT(onRecViewSecondaryStopRecording()));

    recViewStartBothRecordingAct = new QAction(tr("Start All Recording"), this);
    connect(recViewStartBothRecordingAct, SIGNAL(triggered()),
            SLOT(onRecViewBothStartRecording()));

    recViewStopBothRecordingAct = new QAction(tr("Stop All  Recording"), this);
    connect(recViewStopBothRecordingAct, SIGNAL(triggered()),
            SLOT(onRecViewBothStopRecording()));

    recViewPlayBackAct = new QAction(tr("External Playback"), this);
    connect(recViewPlayBackAct, SIGNAL(triggered()),
            SLOT(onRecViewPlayBack()));

    recViewEditCameraAct = new QAction(tr("Edit Properties..."), this);
    recViewEditCameraAct->setStatusTip(tr("Edit camera properties"));
    connect(recViewEditCameraAct, SIGNAL(triggered()), this, SLOT(onRecViewCameraEdit()));

    playerViewStartDecodingAct = new QAction(tr("Start Playback"), this);
    connect(playerViewStartDecodingAct, SIGNAL(triggered()),
            SLOT(onPlayerViewStartDecoding()));

    playerViewStopDecodingAct = new QAction(tr("Stop Playback"), this);
    connect(playerViewStopDecodingAct, SIGNAL(triggered()),
            SLOT(onPlayerViewStopDecoding()));

    playerViewPauseResumeDecodingAct = new QAction(tr("Pause/Resume"), this);
    connect(playerViewPauseResumeDecodingAct, SIGNAL(triggered()),
            SLOT(onPlayerViewPauseResumeDecoding()));

    playerViewEditPropertiesAct = new QAction(tr("Edit Properties..."), this);
    connect(playerViewEditPropertiesAct, SIGNAL(triggered()),
            SLOT(onPlayerViewEditProperties()));


    playerNewAct = new QAction(tr("New Player..."), this);
    playerNewAct->setShortcut(tr("Ctrl+P"));
    connect(playerNewAct, SIGNAL(triggered()),
            SLOT(onPlayerNew()));
    playerDeleteAct = new QAction(tr("Delete Player"), this);
    connect(playerDeleteAct, SIGNAL(triggered()),
            SLOT(onPlayerDelete()));
    playerConnectAct = new QAction(tr("Connect..."), this);
    playerConnectAct->setShortcut(tr("Ctrl+G"));

    connect(playerConnectAct, SIGNAL(triggered()),
            SLOT(onPlayerConnect()));
    playerDisconnectAct = new QAction(tr("Disconnect"), this);
    playerDisconnectAct->setShortcut(tr("Ctrl+I"));

    connect(playerDisconnectAct, SIGNAL(triggered()),
            SLOT(onPlayerDisconnect()));

    playerStartDecodingAct = new QAction(tr("Start Playback"), this);
    connect(playerStartDecodingAct, SIGNAL(triggered()),
            SLOT(onPlayerStartDecoding()));
    playerStopDecodingAct = new QAction(tr("Stop Playback"), this);
    connect(playerStopDecodingAct, SIGNAL(triggered()),
            SLOT(onPlayerStopDecoding()));
    playerPauseResumeDecodingAct = new QAction(tr("Pause Playback"), this);
    connect(playerPauseResumeDecodingAct, SIGNAL(triggered()),
            SLOT(onPlayerPauseResumeDecoding()));

    playerEditPropertiesAct = new QAction(tr("Edit Properties..."), this);
    playerEditPropertiesAct->setStatusTip(tr("Edit decoder properties"));
    connect(playerEditPropertiesAct, SIGNAL(triggered()),
            SLOT(onPlayerEditProperties()));

    videoPanelStartPrimaryRecordingAct = new QAction(tr("Start Primary Recording"), this);
    connect(videoPanelStartPrimaryRecordingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelPrimaryStartRecording()));

    videoPanelStopPrimaryRecordingAct = new QAction(tr("Stop Primary Recording"), this);
    connect(videoPanelStopPrimaryRecordingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelPrimaryStopRecording()));

    videoPanelStartSecondaryRecordingAct = new QAction(tr("Start Secondary Recording"), this);
    connect(videoPanelStartSecondaryRecordingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelSecondaryStartRecording()));

    videoPanelStopSecondaryRecordingAct = new QAction(tr("Stop Secondary  Recording"), this);
    connect(videoPanelStopSecondaryRecordingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelSecondaryStopRecording()));

    videoPanelStartBothRecordingAct = new QAction(tr("Start All Recording"), this);
    connect(videoPanelStartBothRecordingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelBothStartRecording()));

    videoPanelStopBothRecordingAct = new QAction(tr("Stop All  Recording"), this);
    connect(videoPanelStopBothRecordingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelBothStopRecording()));

    videoPanelStartDecodingAct = new QAction(tr("Start Playback"), this);
    connect(videoPanelStartDecodingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelStartPlayer()));

    videoPanelStopDecodingAct = new QAction(tr("Stop Playback"), this);
    connect(videoPanelStopDecodingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelStopPlayer()));

    videoPanelPauseResumeDecodingAct = new QAction(tr("Pause Playback"), this);
    connect(videoPanelPauseResumeDecodingAct, SIGNAL(triggered()),
            SLOT(onVideoPanelPauseResumePlayer()));

    videoPanelPlayBackAct = new QAction(tr("External Playback"), this);
    connect(videoPanelPlayBackAct, SIGNAL(triggered()),
            SLOT(onVideoPanelPlayBack()));

    videoPanelSaveYUVFramesAct = new QAction(tr("Save YUV Buffers"), this);
    connect(videoPanelSaveYUVFramesAct, SIGNAL(triggered()),
            SLOT(onVideoPanelYUVRecording()));

    videoPanelSaveRawAudioFramesAct = new QAction(tr("Save Raw Audio Buffers"), this);
    connect(videoPanelSaveRawAudioFramesAct, SIGNAL(triggered()),
            SLOT(onVideoPanelRawAudioRecording()));

    videoPanelEnableRawAudioAct = new QAction(tr("Enable Raw Audio"), this);
    videoPanelEnableRawAudioAct->setCheckable(true);
    connect(videoPanelEnableRawAudioAct, SIGNAL(triggered()), this, SLOT(onVideoPanelEnableRawAudio()));

    videoPanelEditCameraAct = new QAction(tr("Edit Properties..."), this);
    videoPanelEditCameraAct->setStatusTip(tr("Edit camera properties"));
    connect(videoPanelEditCameraAct, SIGNAL(triggered()), this, SLOT(onVideoPanelCameraEdit()));

    videoPanelEditPlayerAct = new QAction(tr("Edit Properties..."), this);
    videoPanelEditPlayerAct->setStatusTip(tr("Edit player properties"));
    connect(videoPanelEditPlayerAct, SIGNAL(triggered()), this, SLOT(onVideoPanelPlayerEdit()));

    relaysAct = new QAction(tr("&Trigger Relays..."), this);
    relaysAct->setStatusTip(tr("Set/Clear Relays"));
    connect(relaysAct, SIGNAL(triggered()), this, SLOT(onRelays()));

    sensorCnfgAct = new QAction(tr("&Sensors Configuration..."), this);
    sensorCnfgAct->setStatusTip(tr("Enable/Disable Sensors and set its trigger method"));
    connect(sensorCnfgAct, SIGNAL(triggered()), this, SLOT(onSensorConfig()));

    disableWatchDogAct = new QAction(tr("&Disable Watchdog Reset"), this);
    disableWatchDogAct->setStatusTip(tr("Enable/Disable sending watchdog reset to the firmware."));
    disableWatchDogAct->setCheckable(true);
    connect(disableWatchDogAct, SIGNAL(triggered()), this, SLOT(onDisableWatchDog()));

    setYUV422VideoAct = new QAction(tr("&YUV4-2-2 Video"), this);
    setYUV422VideoAct->setCheckable(true);
    setYUV422VideoAct->setVisible(false);
    connect(setYUV422VideoAct, SIGNAL(triggered()), this, SLOT(onSetYUV422Convert()));

    setYUV420VideoAct = new QAction(tr("&YUV4-2-0 Video"), this);
    setYUV420VideoAct->setCheckable(true);
    setYUV420VideoAct->setVisible(false);
    connect(setYUV420VideoAct, SIGNAL(triggered()), this, SLOT(onSetYUV420Convert()));

    setYVU420VideoAct = new QAction(tr("&YVU4-2-0 Video"), this);
    setYVU420VideoAct->setCheckable(true);
    setYVU420VideoAct->setVisible(false);
    connect(setYVU420VideoAct, SIGNAL(triggered()), this, SLOT(onSetYVU420Convert()));


    setSkipRawVideoDisplayAct = new QAction(tr("Skip Raw Video Display"), this);
    setSkipRawVideoDisplayAct->setCheckable(true);
    setSkipRawVideoDisplayAct->setVisible(false);
    connect(setSkipRawVideoDisplayAct, SIGNAL(triggered()), this, SLOT(onSetSkipRawVideoDisplay()));

    ioctlTestAct = new QAction(tr("Device I/O Control..."), this);
    connect(ioctlTestAct, SIGNAL(triggered()), this, SLOT(onIOCTLTest()));


    aboutAct = new QAction(tr("&About..."), this);
    aboutAct->setStatusTip(tr("Show About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

}

void MainWindow::createMenus()
{
    m_systemMenu = menuBar()->addMenu(tr("&System"));
    m_systemMenu->addAction(newSystemAct);
    m_systemMenu->addAction(openSystemAct);
    m_systemMenu->addAction(editSystemAct);
    m_systemMenu->addAction(saveSystemAct);
    m_systemMenu->addAction(closeSystemAct);
    m_systemMenu->addSeparator();
    m_systemMenu->addAction(exitAct);

    m_cameraMenu = menuBar()->addMenu(tr("&Camera"));
    m_cameraMenu->setSeparatorsCollapsible(true);
    m_cameraMenu->addAction(newCameraAct);
    m_cameraMenu->addAction(deleteCameraAct);
    m_cameraMenu->addAction(connectCameraAct);
    m_cameraMenu->addAction(disconnectCameraAct);
    m_cameraMenu->addSeparator();

    m_startRecordMenu = m_cameraMenu->addMenu(tr("Start Recording"));
    m_startRecordMenu->addAction(startPrimaryRecordingAct);
    m_startRecordMenu->addAction(startSecondaryRecordingAct);
    m_startRecordMenu->addAction(startBothRecordingAct);

    m_stopRecordMenu = m_cameraMenu->addMenu(tr("Stop Recording"));
    m_stopRecordMenu->addAction(stopPrimaryRecordingAct);
    m_stopRecordMenu->addAction(stopSecondaryRecordingAct);
    m_stopRecordMenu->addAction(stopBothRecordingAct);

#ifdef WIN32
    m_cameraMenu->addAction(playBackAct);
#endif
    m_cameraMenu->addSeparator();
    m_cameraMenu->addAction(editCameraAct);

    m_playerMenu = menuBar()->addMenu(tr("&Player"));
    m_playerMenu->addAction(playerNewAct);
    m_playerMenu->addAction(playerDeleteAct);
    m_playerMenu->addSeparator();
    m_playerMenu->addAction(playerStartDecodingAct);
    m_playerMenu->addAction(playerStopDecodingAct);
    m_playerMenu->addAction(playerPauseResumeDecodingAct);
    m_playerMenu->addSeparator();
    m_playerMenu->addAction(playerEditPropertiesAct);
    m_playerMenu->addSeparator();
    m_playerMenu->addAction(continuousDecodingAct);
    m_playerMenu->addSeparator();
    m_playerMenu->addAction(playerConnectAct);
    m_playerMenu->addAction(playerDisconnectAct);


    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->setSeparatorsCollapsible(true);
    m_viewMenu->addAction(viewTreeAct);
    m_viewMenu->addAction(viewLogAct);
    m_viewMenu->addAction(viewDecoderViewAct);
    m_viewMenu->addAction(viewSmoYuvPlayerAct);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(viewRecViewAct);
    m_viewMenu->addAction(selectStatData);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(newMonitorAct);
    m_viewMenu->addAction(deleteMonitorAct);
    m_viewMenu->addAction(renameMonitorAct);

    m_viewMenu->addSeparator();

    for (int i = 0; i < g_DVR->smoList.count(); ++i)
    {
        SpotMonitor *smo = g_DVR->smoList[i];
        m_smoMenu[i] = new QMenu(smo->name());
        m_smoMenu[i]->setSeparatorsCollapsible(true);
        connect(m_smoMenu[i], SIGNAL(triggered(QAction *)),
                SLOT(onSmoMenuTriggered(QAction *)));

        m_viewMenu->addMenu(m_smoMenu[i]);
    }

    m_viewMenu->addSeparator();
    m_viewMenu->addAction(clearLogAct);
    m_viewMenu->addAction(skipLogInfoAct);

    m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
    m_toolsMenu->addAction(relaysAct);
    m_toolsMenu->addAction(sensorCnfgAct);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction(disableWatchDogAct);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction(setYUV422VideoAct);
    m_toolsMenu->addAction(setYUV420VideoAct);
    m_toolsMenu->addAction(setYVU420VideoAct);

    m_toolsMenu->addAction(setSkipRawVideoDisplayAct);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction(ioctlTestAct);

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(aboutAct);

    m_recViewMenu = new QMenu();
    m_recViewStartRecordMenu = m_recViewMenu->addMenu(tr("Start Recording"));
    m_recViewStartRecordMenu->addAction(recViewStartPrimaryRecordingAct);
    m_recViewStartRecordMenu->addAction(recViewStartSecondaryRecordingAct);
    m_recViewStartRecordMenu->addAction(recViewStartBothRecordingAct);

    m_recViewStopRecordMenu = m_recViewMenu->addMenu(tr("Stop Recording"));
    m_recViewStopRecordMenu->addAction(recViewStopPrimaryRecordingAct);
    m_recViewStopRecordMenu->addAction(recViewStopSecondaryRecordingAct);
    m_recViewStopRecordMenu->addAction(recViewStopBothRecordingAct);

    m_recViewMenu->addSeparator();
#ifdef WIN32
    m_recViewMenu->addAction(recViewPlayBackAct);
    m_recViewMenu->addSeparator();
#endif
    m_recViewMenu->addAction(recViewEditCameraAct);


    m_playerViewMenu = new QMenu();
    m_playerViewMenu->addAction(playerViewStartDecodingAct);
    m_playerViewMenu->addAction(playerViewStopDecodingAct);
    m_playerViewMenu->addAction(playerViewPauseResumeDecodingAct);
    m_playerViewMenu->addSeparator();
    m_playerViewMenu->addAction(playerViewEditPropertiesAct);
    m_playerViewMenu->addSeparator();
    m_playerViewMenu->addAction(continuousDecodingAct);

    m_videoPanelMenu = new QMenu();
    m_videoPanelStartRecordMenu = m_videoPanelMenu->addMenu(tr("Start Recording"));
    m_videoPanelStartRecordMenu->addAction(videoPanelStartPrimaryRecordingAct);
    m_videoPanelStartRecordMenu->addAction(videoPanelStartSecondaryRecordingAct);
    m_videoPanelStartRecordMenu->addAction(videoPanelStartBothRecordingAct);

    m_videoPanelStopRecordMenu = m_videoPanelMenu->addMenu(tr("Stop Recording"));
    m_videoPanelStopRecordMenu->addAction(videoPanelStopPrimaryRecordingAct);
    m_videoPanelStopRecordMenu->addAction(videoPanelStopSecondaryRecordingAct);
    m_videoPanelStopRecordMenu->addAction(videoPanelStopBothRecordingAct);

    m_videoPanelMenu->addAction(videoPanelStartDecodingAct);
    m_videoPanelMenu->addAction(videoPanelStopDecodingAct);
    m_videoPanelMenu->addAction(videoPanelPauseResumeDecodingAct);
    m_videoPanelMenu->addSeparator();
#ifdef WIN32
    m_videoPanelMenu->addAction(videoPanelPlayBackAct);
    m_videoPanelMenu->addSeparator();
#endif
    m_videoPanelMenu->addAction(videoPanelEditCameraAct);
    m_videoPanelMenu->addAction(videoPanelEditPlayerAct);
    m_videoPanelMenu->addSeparator();
    m_videoPanelMenu->addAction(continuousDecodingAct);
    m_videoPanelMenu->addSeparator();
    m_videoPanelMenu->addAction(videoPanelSaveYUVFramesAct);
    m_videoPanelMenu->addAction(videoPanelEnableRawAudioAct);
    m_videoPanelMenu->addAction(videoPanelSaveRawAudioFramesAct);

}

QMenu *MainWindow::smoMenu(SpotMonitor *smo)
{
    sdvr_smo_attribute_t attr;
    QAction *offAct, *act;
    QMenu *menu = m_smoMenu[g_DVR->smoList.indexOf(smo)];

    Q_ASSERT(menu != NULL);
    menu->setEnabled(true);
    menu->clear();

    sdvr_err_e err = smo->getAttributes(&attr);
    if (err != SDVR_ERR_NONE)
    {
        Log::error(QString("Error getting attributes for '%1' port %2: %3")
                .arg(smo->name()).arg(smo->port())
                .arg(sdvr_get_error_text(err)));
        return menu;
    }

    if ((attr.cap_flags & (SDVR_SMO_CAP_TILING | SDVR_SMO_CAP_ANALOG)) != 0)
    {
        offAct = new QAction(SMO_OFF_ACTION_TEXT, menu);
        offAct->setCheckable(true);
        offAct->setData(qVariantFromValue((void *) smo));
        menu->addAction(offAct);
        menu->addSeparator();

        int boardIndex = smo->board()->index();
        bool activeSmoFound = false;
        foreach (ViewSettings *vs, g_DVR->monitorList)
        {
            if (vs->isSmo() && vs->smoBoardIndex() == boardIndex)
            {
                act = new QAction(vs->name(), menu);
                act->setCheckable(true);
                act->setData(qVariantFromValue((void *) smo));
                if (g_DVR->isActiveSmo(vs))
                {
                    act->setChecked(true);
                    activeSmoFound = true;
                }

                menu->addAction(act);
            }
        }

        if (!activeSmoFound)
            offAct->setChecked(true);
    }

    menu->addSeparator();

    if ((attr.cap_flags & SDVR_SMO_CAP_OUTPUT) != 0)
    {
        act = new QAction(SMO_PLAYBACK_ACTION_TEXT, menu);
        act->setData(qVariantFromValue((void *) smo));
        menu->addAction(act);
    }

    menu->addSeparator();

    if ((attr.cap_flags & SDVR_SMO_CAP_OSD) != 0)
    {
        act = new QAction(SMO_PROPS_ACTION_TEXT, menu);
        act->setData(qVariantFromValue((void *) smo));
        menu->addAction(act);
    }

    return menu;
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setTitle()
{
    if (g_DVR != NULL && g_DVR->isOpen())
        setWindowTitle(QString("%1 - %2").arg(g_DVR->name()).arg(TITLE));
    else
        setWindowTitle(TITLE);
}

/*
void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("System"));
    fileToolBar->addAction(newSystemAct);
    fileToolBar->addAction(openSystemAct);
    fileToolBar->addAction(editSystemAct);
}
*/
/*
void MainWindow::readSettings()
{
    QSettings settings("Trolltech", "Application Example");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void MainWindow::writeSettings()
{
    QSettings settings("Trolltech", "Application Example");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}
*/
void MainWindow::updateChannelSelector()
{
    QSize playerSize;
    QSize videoPanelSize;
    m_channelSelector->clear();

    ViewSettings *ms = currentMonitorSettings();
    if (ms == NULL)
    {
        m_channelSelector->setEnabled(false);
        return;
    }

    // Generate the list of connected cameras
    // If current monitor is SMO, filter by the board index
    QList<Camera *> connectedCameras;
    foreach (Camera *cam, g_DVR->cameraList)
        if (cam->isChannelOpen())
            if (!ms->isSmo() || ms->smoBoardIndex() == cam->boardIndex())
                connectedCameras.append(cam);


    m_channelSelector->addItem(tr("Off"), QVariant(0));
    foreach (Camera *cam, connectedCameras)
        m_channelSelector->addItem(cam->name(), QVariant(cam->handle()));

    // Generate the list of connected Players
    // If current monitor is SMO, filter by the board index
    QList<CPlayer *> connectedPlayers;
    foreach (CPlayer *player, g_DVR->m_playerList)
        if (player->isChannelOpen())
            if (!ms->isSmo() || ms->smoBoardIndex() == player->getBoardIndex())
                connectedPlayers.append(player);

    if (connectedPlayers.count())
    {
        if (m_channelSelector->count() == 0)
            m_channelSelector->addItem(tr("Off"), QVariant(0));

        foreach (CPlayer *player, connectedPlayers)
        {
            m_channelSelector->addItem(player->getName(), QVariant(player->handle()));
        }

    }

    // Enable the channel selector combobox if we have at least one
    // Camera or Player that is connected
    if (connectedCameras.count() == 0 &&connectedPlayers.count() == 0 )
    {
        m_channelSelector->setEnabled(false);
    }
    else
    {
        m_channelSelector->setEnabled(true);
        onDisplaySelected();
    }
}

void MainWindow::updateMonitorSelector()
{
    m_monitorSelector->clear();
    if (g_DVR->monitorList.count() == 0)
        m_monitorSelector->setEnabled(false);
    else
    {
        m_monitorSelector->addItem(tr("Off"));
        foreach (ViewSettings *vs, g_DVR->monitorList)
            m_monitorSelector->addItem(vs->name());

        m_monitorSelector->setEnabled(true);
    }

    updateChannelSelector();
}

VideoPanel *MainWindow::videoPanel() const
{
    return (VideoPanel *) m_scrollArea->widget();
}

void MainWindow::setVideoPanel(VideoPanel *panel)
{
    if (g_videoPanel != NULL)
    {
        g_videoPanel->close();
        delete g_videoPanel;
        g_videoPanel  = NULL;
        m_scrollArea->takeWidget();
    }

    if (panel != NULL)
    {
        m_scrollArea->setWidget(panel);
        g_videoPanel  = panel;
    }
}

ViewSettings *MainWindow::currentMonitorSettings() const
{
    VideoPanel *vp = videoPanel();
    return (vp != NULL) ? vp->settings(): NULL;
}

bool MainWindow::playBack(uint subEnc, Camera *cam)
{
    if (cam == NULL || !cam->canPlayBack(SDVR_ENC_PRIMARY))
        return false;

    if (cam->isRecording(SDVR_ENC_PRIMARY))
        cam->stopRecording(SDVR_ENC_PRIMARY);

    if (!QProcess::startDetached("explorer",
        QStringList() << QDir::toNativeSeparators(cam->recordFilePath(subEnc,false))))
    {
        QMessageBox::critical(this, tr("Error"),
            "Error starting playback process");
        return false;
    }

    return true;
}

void MainWindow::onVideoTimer()
{
    g_DVR->alarmHandler().processMessages();
    g_DVR->sensorsHandler().processMessages();
    g_DVR->signalsHandler().processMessages();
}

void MainWindow::onWatchDogTimer()
{
    int maxBoardCount = g_DVR->boardCount();
    bool bEnable = g_DVR->getWatchDogState();

    // Set the same watchdog timer for all the boards
    for (int board_index = 0; board_index < maxBoardCount; board_index++)
        sdvr_set_watchdog_state_ex(board_index,bEnable,g_DVR->getWatchDogTimeout() * 1000);
}

void MainWindow::onUpdateBoardTimeTimer()
{
    int maxBoardCount = g_DVR->boardCount();

    // update the time for all the boards
    for (int board_index = 0; board_index < maxBoardCount; board_index++)
        g_DVR->board(board_index)->updateDateTime();
}

void MainWindow::setStatusBar(const QString &msg)
{
    statusBar()->showMessage(msg);
//	Log::info(msg);
}

void MainWindow::onLoadStartupSystem()
{
    sdvr_err_e rc;
//    sdvr_sdk_params_t sdk_params;
    QString log_file_path;
    QDir directoryObj;
    bool bLoadSystem = true;
    QString startupSystem;
    QString systemDir;
    sx_uint8 major, minor, revision, build;
    major =  minor = revision = build = 0;

    // Prompt the user to select config file and firmware file
    //////////////////////////////////////////////////////////
    StartupDialog dialog(g_DVR, this);
    if (dialog.exec() != QDialog::Accepted)
        exit(-1);

    startupSystem = DVRSystem::startupSystem();

    // We use the directory where the system config file
    // is located to create the SDK debug log file.
    // If no config file is set, we use the current directory.
    if (startupSystem.isEmpty())
    {
        bLoadSystem = false;
        systemDir = QDir::currentPath();
    }
    else
        systemDir = QFileInfo(startupSystem).absolutePath();

    statusBar()->showMessage("Searching for SDVR PCIe boards...");

    if ((rc = sdvr_sdk_init()) != SDVR_ERR_NONE)
    {
        sdvr_get_driver_version(0,&major, &minor, &revision, &build);

        QMessageBox::critical(NULL, "DVRCP Initialization Error",
            QString("Application failed to find any PCIe boards and initialize the SDK. Error Code [%1-%2].\nCurrent Driver Version = %3.%4.%5.%6")
            .arg(sdvr_get_error_text(rc)).arg((int)rc).
            arg(major).arg(minor).arg(revision).arg(build));

        exit(1);
    }

    // We just assume the largest video size is PAL_D1 720x576
    if (sdvr_ui_init(720,576) != SDVR_UI_ERR_NONE)
    {
        QMessageBox::warning(NULL, "DVRCP UI Initialization Error",
            QString("Application failed to initialize the video card. Makes sure the DirectDraw acceleration is enabled.\nIn Windows XP, type dxdiag from run menu, go to 'Display' Tab."));
    }
    // Set the backgroung color of each window to black
    sdvr_ui_set_key_color(sdvr_rgb(0,0,0));
    sdvr_get_driver_version(0,&major, &minor, &revision, &build);

    statusBar()->showMessage("Initializing system Navigator...");

    g_DVR = new DVRSystem();

    statusBar()->showMessage("Reading the DVR board attributes...");

    if ((rc = g_DVR->setup()) != SDVR_ERR_NONE)
    {
        sx_uint8 sdk_major, sdk_minor, sdk_revision, sdk_build;
        sdk_major =  sdk_minor = sdk_revision = sdk_build = 0;

        sdvr_get_sdk_version(&sdk_major, &sdk_minor,
                             &sdk_revision,& sdk_build);

        QMessageBox::critical(NULL, "DVRCP Initialization Error",
            QString("Application failed to communicate with any PCIe boards. Error Code [%1-%2].\nCurrent Driver Version = %3.%4.%5.%6\nCurrent SDK Version: %7.%8.%9.%10")
            .arg(sdvr_get_error_text(rc)).arg((int)rc).
            arg(major).arg(minor).arg(revision).arg(build).
            arg(sdk_major).arg(sdk_minor).arg(sdk_revision).arg(sdk_build));

        exit(1);
    }

    statusBar()->showMessage("Initializing the SDK parameters...");
    QString debugFileName(QString("%1/sdk.log").arg(systemDir));
    QByteArray baDebugFileName(debugFileName.toAscii());
    g_DVR->updateSDKSettings(baDebugFileName.data());

    sdvr_set_display_debug(disp_debug_callback);
    sdvr_set_av_frame_callback(av_frame_callback);
    sdvr_set_video_alarm_callback(video_alarm_callback);
    sdvr_set_sensor_callback(sensors_callback);
    sdvr_set_signals_callback(signals_callback);

    statusBar()->showMessage("Setting up the UI widgets...");

    g_videoPanel  = NULL;

    m_sysTree = new DVRSystemTree();
    connect(m_sysTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            this, SLOT(onTreeItemActivated(QTreeWidgetItem *, int)));

    m_sysNavigator = new QDockWidget("System Navigator", this);
    m_sysNavigator->setAllowedAreas(Qt::LeftDockWidgetArea |
                                    Qt::RightDockWidgetArea);
    m_sysNavigator->setWidget(m_sysTree);
    addDockWidget(Qt::LeftDockWidgetArea, m_sysNavigator);

    m_recView = new RecView(g_DVR);
    m_recViewDock = new QDockWidget("System Status", this);
    m_recViewDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_recViewDock->setWidget(m_recView);
    addDockWidget(Qt::LeftDockWidgetArea, m_recViewDock);

 //todo removeit   tabifyDockWidget(m_sysNavigator,m_recViewDock);

    m_decoderView = new DecoderView(g_DVR);
    m_DecoderViewDock = new QDockWidget("Decoding", this);
    m_DecoderViewDock->setAllowedAreas(Qt::LeftDockWidgetArea |
                                   Qt::RightDockWidgetArea);
    m_DecoderViewDock->setWidget(m_decoderView);
    addDockWidget(Qt::LeftDockWidgetArea, m_DecoderViewDock);

    m_smoYuvPlayerDock = new QDockWidget("SMO Playback", this);
    m_smoYuvPlayerDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, m_smoYuvPlayerDock);
    m_smoYuvPlayerDock->setVisible(false);

    connect(&g_DVR->recorder(), SIGNAL(interruptRecording(uint, int)),SLOT(onInterruptRecording(uint, int)));

    // To avoid ambiguity between the stop recording action in the main menu
    // and stop recording action in the recording view context menu clear the
    // system tree selection when an item in the recording view is clicked.
    connect(m_recView, SIGNAL(itemClicked()), m_sysTree, SLOT(clearSelection()));

    int boardCount = g_DVR->boardCount();
    for (int i = 0; i < boardCount; ++i)
        m_sysTree->addBoardItem(g_DVR->board(i));

    createActions();
    createMenus();

    connect(m_systemMenu, SIGNAL(aboutToShow()), SLOT(onSystemMenuAboutToShow()));
    connect(m_cameraMenu, SIGNAL(aboutToShow()), SLOT(onCameraMenuAboutToShow()));
    connect(m_viewMenu, SIGNAL(aboutToShow()), SLOT(onViewMenuAboutToShow()));
    connect(m_toolsMenu, SIGNAL(aboutToShow()), SLOT(onToolsMenuAboutToShow()));

    connect(m_recViewMenu, SIGNAL(aboutToShow()), SLOT(onRecViewMenuAboutToShow()));
    connect(m_playerViewMenu, SIGNAL(aboutToShow()), SLOT(onPlayerViewMenuAboutToShow()));
    connect(m_playerMenu, SIGNAL(aboutToShow()), SLOT(onPlayerMenuAboutToShow()));

    connect(m_videoPanelMenu, SIGNAL(aboutToShow()), SLOT(onVideoPanelMenuAboutToShow()));
    connect(m_channelSelector, SIGNAL(activated(int)), SLOT(onChannelSelected(int)));
    connect(m_monitorSelector, SIGNAL(activated(int)), SLOT(onMonitorSelected(int)));

    connect(Log::Instance(), SIGNAL(alarmReset()),
            &g_DVR->alarmHandler(), SLOT(reset()));

    connect(&m_videoTimer, SIGNAL(timeout()), this, SLOT(onVideoTimer()));
    connect(&m_watchDogTimer, SIGNAL(timeout()), this, SLOT(onWatchDogTimer()));
    connect(&m_updateBoardTimeTimer, SIGNAL(timeout()), this, SLOT(onUpdateBoardTimeTimer()));

    int timerInterval = g_DVR->getHMOPlayInterval(g_DVR->videoStandard());
    m_videoTimer.setInterval(timerInterval);

    if (g_DVR->getWatchDogState())
        m_watchDogTimer.start((g_DVR->getWatchDogTimeout() / 2) * 1000);//(WATCHDOG_TIMER_VALUE);
    else
        m_watchDogTimer.stop();

    m_videoTimer.start();

    // Load the start up system if one was specified
    if (bLoadSystem)
    {
        if (!openSystem(startupSystem))
            onSystemClose();
    }

    statusBar()->showMessage("Ready");
}

void MainWindow::showEvent(QShowEvent *)
{
    // NOTE: The recording statistics timer get set/reset
    //       in ShowEvent() handler. Every time a new
    //       system is loaded we have to make sure the
    //       timer interval changes according to the new settings.
    if (g_DVR && m_recView && m_recView->isVisible())
        m_recView->updateTimerInterval(g_DVR->getStatFPSInterval() * 1000);
    if (g_DVR && m_decoderView && m_decoderView->isVisible())
        m_decoderView->updateTimerInterval(g_DVR->getStatFPSInterval() * 1000);
}

// Sets the firmware timed out flag on the given board index.
// Also globally remember that a firmware in one of the boards
// has timedout.
void MainWindow::setFirmwareTimeout(int boardIndex)
{
    QString qstrMsg;
    DVRBoard *boardPtr = g_DVR->board(boardIndex);

    if (boardPtr)
    {
        boardPtr->setFirmwareTimeout();
    }

    m_bFirmwareTimeout = true;
}

// The function returns whether the firmware on
// the given board index has timed out.
bool MainWindow::isFirmwareTimeout(int boardIndex)
{
    // If the given boardIndex does not exist we assume
    // the firmware on the board is timed out.
    bool bRet = true;
    DVRBoard *boardPtr = g_DVR->board(boardIndex);

    if (boardPtr)
    {
        bRet = boardPtr->isFirmwareTimeout();
    }

    return bRet;
}

QString MainWindow::recordFilePath(QString name)
{
    QSize size = videoPanel()->getDecimationSize();
    int width = size.width();
    int height = size.height();

    return QString("%1/%2_%3x%4.yuv").arg(g_DVR->systemDirPath())
        .arg(name).arg(width).arg(height);
}

void MainWindow::saveYUVBuffer(sdvr_yuv_buffer_t *buf)
{
    QMutexLocker locker(&m_YUVSaveMutex);

     if (!m_qFileyuvSaveFile.isOpen())
     {
        return;
     }

    if (m_qFileyuvSaveFile.write((const char *)buf->y_data, buf->y_data_size)!= buf->y_data_size)
    {
        Log::error(QString("Error: Saving Y buffer. Write file error (code=%1)").arg(m_qFileyuvSaveFile.errorString()));
        m_qFileyuvSaveFile.close();
        m_yuvSaveChanHandle = 0;
        return;
    }
    if (m_qFileyuvSaveFile.write((const char *)buf->u_data, buf->u_data_size)!= buf->u_data_size)
    {
        Log::error(QString("Error: Saving U buffer. Write file error (code=%1)").arg(m_qFileyuvSaveFile.errorString()));
        m_qFileyuvSaveFile.close();
        m_yuvSaveChanHandle = 0;
        return;
    }
    if (m_qFileyuvSaveFile.write((const char *)buf->v_data, buf->v_data_size)!= buf->v_data_size)
    {
        Log::error(QString("Error: Saving V buffer. Write file error (code=%1)").arg(m_qFileyuvSaveFile.errorString()));
        m_qFileyuvSaveFile.close();
        m_yuvSaveChanHandle = 0;
        return;
    }
}

void MainWindow::closeYUVFile()
{
    if (m_yuvSaveChanHandle)
    {
        QString menuText;
        QMutexLocker locker(&m_YUVSaveMutex);

        m_qFileyuvSaveFile.close();
        m_yuvSaveChanHandle = 0;
        menuText = QString("Save YUV Buffers");
        videoPanelSaveYUVFramesAct->setText(menuText);

    }
}


QString MainWindow::rawAudioFilePath(QString name)
{
    return QString("%1/%2.pcm").arg(g_DVR->systemDirPath()).arg(name);
}

void MainWindow::saveRawAudioBuffer(sdvr_av_buffer_t *buf)
{

     if (!m_qFileRawAudioSaveFile.isOpen())
        return;

    if (m_qFileRawAudioSaveFile.write((const char *)buf->payload, buf->payload_size)!= buf->payload_size)
    {
        Log::error(QString("Error: Saving raw audio buffer. Write file error (code=%1)").arg(m_qFileRawAudioSaveFile.errorString()));
        m_qFileRawAudioSaveFile.close();
        m_rawAudioSaveChanHandle = 0;
        return;
    }
}
void MainWindow::closeRawAudioFile()
{
    if (m_rawAudioSaveChanHandle && m_qFileRawAudioSaveFile.isOpen())
    {
        QString menuText;

        m_qFileRawAudioSaveFile.close();
        m_rawAudioSaveChanHandle = 0;
        menuText = QString("Save Raw Audio Buffers");
        videoPanelSaveRawAudioFramesAct->setText(menuText);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//              C A L L    B A C K       F U N C T I O N
////////////////////////////////////////////////////////////////////////////////////////////////
void av_frame_callback(sdvr_chan_handle_t handle,
                       sdvr_frame_type_e frame_type, sx_bool is_primary)
{
    sdvr_av_buffer_t  *av_buffer;
    sdvr_yuv_buffer_t *yuv_buffer;
    sdvr_err_e err;

    // Don't receive any new frame while we are trying to close the system
    if (g_DVR->isSystemClosing())
    {
        if (frame_type == SDVR_FRAME_RAW_VIDEO)
        {
            if (sdvr_get_yuv_buffer(handle, &yuv_buffer) == SDVR_ERR_NONE)
                sdvr_release_yuv_buffer(yuv_buffer);
        }
        else
        {
            if (sdvr_get_av_buffer(handle,
                is_primary ? SDVR_FRAME_VIDEO_ENCODED_PRIMARY : SDVR_FRAME_VIDEO_ENCODED_SECONDARY,
                &av_buffer) == SDVR_ERR_NONE)
            {
                sdvr_release_av_buffer(av_buffer);
            }
        }
        return;
    }

    switch (frame_type)
    {
    case SDVR_FRAME_AUDIO_ENCODED:
        if (g_DVR->isUseSDKRecording())
            break;
        if (sdvr_get_av_buffer(handle, frame_type, &av_buffer) == SDVR_ERR_NONE)
            g_DVR->recorder().enqueue(handle, av_buffer);
        break;

    case SDVR_FRAME_RAW_AUDIO:
        // Check if we should save the raw audio buffers for this camera into a file
        if (g_MainWindow->getRawAudioSaveHandle() && (g_MainWindow->getRawAudioSaveHandle() == handle))
        {
            if (sdvr_get_av_buffer(handle, SDVR_FRAME_RAW_AUDIO, &av_buffer) == SDVR_ERR_NONE)
            {
                g_MainWindow->saveRawAudioBuffer(av_buffer);
                sdvr_release_av_buffer(av_buffer);
            }
        }
        break;

    case SDVR_FRAME_RAW_VIDEO:

    	yuv_buffer = NULL;
        err = sdvr_get_yuv_buffer(handle, &yuv_buffer);

        if (err == SDVR_ERR_NONE && yuv_buffer != NULL)
        {
#ifdef low_level_ui_display
            // Display this video frame in all the HMO grids that
            // the camera/player is assigned to.
            if (!g_disable_hmo && g_videoPanel != NULL)
            	g_videoPanel->updateDisplay(handle, yuv_buffer);
#endif

            g_MainWindow->statView()->collectStats(handle, yuv_buffer);

            // Check if we should save the YUV buffers for this camera into a file
            if (g_MainWindow->getyuvSaveHandle() && (g_MainWindow->getyuvSaveHandle() == handle))
                g_MainWindow->saveYUVBuffer(yuv_buffer);

            sdvr_release_yuv_buffer(yuv_buffer);
        }

        break;

    case SDVR_FRAME_H264_IDR:
    case SDVR_FRAME_H264_I:
    case SDVR_FRAME_H264_P:
    case SDVR_FRAME_H264_B:
    case SDVR_FRAME_H264_SPS:
    case SDVR_FRAME_H264_PPS:
    case SDVR_FRAME_MPEG4_I:
    case SDVR_FRAME_MPEG4_P:
    case SDVR_FRAME_MPEG4_VOL:
// todo: for now we only handle writing of h.264 and MPEG4
//       once AVMUX supports JPEG move the next case up
//       and delete the call to enqueue()
    if (g_DVR->isUseSDKRecording())
        break;

    case SDVR_FRAME_JPEG:

        if (sdvr_get_av_buffer(handle,
            is_primary ? SDVR_FRAME_VIDEO_ENCODED_PRIMARY : SDVR_FRAME_VIDEO_ENCODED_SECONDARY,
            &av_buffer) == SDVR_ERR_NONE)
            g_DVR->recorder().enqueue(handle, av_buffer);
        break;

    case SDVR_FRAME_MOTION_VALUES:
        // Just get the motion values and throw it away
        // TODO: In the future add processing of motion values
        if (sdvr_get_av_buffer(handle, frame_type,&av_buffer) == SDVR_ERR_NONE)
        {
#ifdef mxa_disable
            sdvr_mb_region_t regions_list[2];
            memset(&regions_list[0],0, sizeof(regions_list[2]));
            regions_list[0].alarm_flag = 0;
            regions_list[0].upper_left_x = 5;
            regions_list[0].upper_left_y = 12;
            regions_list[0].lower_right_x = 17;
            regions_list[0].lower_right_y = 30;
            regions_list[1].alarm_flag = 0;
            regions_list[1].upper_left_x = 0;
            regions_list[1].upper_left_y = 20;
            regions_list[1].lower_right_x = 30;
            regions_list[1].lower_right_y = 33;

            sdvr_motion_value_analyzer((sx_uint8 *)&av_buffer->payload[0],
                                (sdvr_video_std_e)g_DVR->videoStandard(),
                                regions_list,
                                2,
                                18);
#endif
             sdvr_release_av_buffer(av_buffer);
        }

        break;

    default:
//        Log::error(QString("Un-expected frame type [%1] was received. Board ID = %d Channel = %d Type = %d ts = %d").arg(frame_type)
//            .arg(sdvr_get_board_index(handle)).arg(sdvr_get_chan_num(handle)).arg(sdvr_get_chan_type(handle).arg(av_buffer->timestamp)));
        break;
    }
}

void video_alarm_callback(sdvr_chan_handle_t handle,
                          sdvr_video_alarm_e alarm_type, sx_uint32 data)
{
    g_DVR->alarmHandler().registerMessage(handle, alarm_type, data);
}

void sensors_callback(sx_uint32 board_index, sx_uint32 sensors_map)
{
    g_DVR->sensorsHandler().registerMessage(board_index, sensors_map);
}

void signals_callback(sx_uint32 board_index,
                      sdvr_signal_info_t *signal_info)
{
    g_DVR->signalsHandler().registerMessage(board_index, signal_info);
}

void disp_debug_callback(char *message)
{
    Log::info(message);
}
