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

#include "systemdialog.h"
#include "dvrsystem.h"
#include "sdvr.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>


StartupDialog::StartupDialog(DVRSystem *system, QWidget *parent)
    : QDialog(parent)
{
    QList<QString> startupList;

    setupUi(this);

    m_system = system;

    //////////////////////////////////////////////////////////////////
    // Initialize the General Tab
    //////////////////////////////////////////////////////////////////
    connect(uiBrowseStartupFirmware, SIGNAL(clicked()), SLOT(browseStartupFirmware()));
    connect(uiBrowseSystemFolder, SIGNAL(clicked()), SLOT(browseSystemFolder()));


    // display the startup firmware and the list most recently used ones.

    uiCombboxStartupFirmware->setMaxCount(MAX_FOLDERS_REMEMBERED);
    startupList = DVRSystem::getStartupFirmwarePathList();
    foreach (QString qstrPrevStartupFolder, startupList)
    {
        uiCombboxStartupFirmware->addItem(qstrPrevStartupFolder);
    }
    uiCombboxStartupFirmware->setEditText(DVRSystem::getStartupFirmwarePath());

    // display the startup folder and the least most recently used ones.
    QFileInfo dirStartupSystem(DVRSystem::startupSystem());

    uiCombboxSystemFolder->setMaxCount(MAX_FOLDERS_REMEMBERED);
    startupList = DVRSystem::startupSystemList();
    foreach (QString qstrPrevStartupFolder, startupList)
    {
        uiCombboxSystemFolder->addItem(qstrPrevStartupFolder);
    }
    uiCombboxSystemFolder->setEditText(dirStartupSystem.path());

}
void StartupDialog::browseStartupFirmware()
{
    QString romFile;
    QString workingDir = uiCombboxStartupFirmware->lineEdit()->text();
        
    // Setup the startup directory. The startup directory is the folder
    // where the currently selected firmware file is located if no
    // firmware file is currently is selected get the directory of the
    // most recently firmware file
    if (workingDir.isEmpty())
    {
        workingDir = uiCombboxStartupFirmware->itemText(0);
    }
    romFile = QFileDialog::getOpenFileName(this,
        tr("Select a Firmware to load"), workingDir,
        tr("*.rom;;*.*"));

    if (!romFile.isEmpty())
    {
        int nSelIndex =  uiCombboxStartupFirmware->findText(romFile);
        if (nSelIndex == -1)
        {
            nSelIndex = 0;
            uiCombboxStartupFirmware->insertItem(nSelIndex,romFile);
        }
        uiCombboxStartupFirmware->setCurrentIndex(nSelIndex);
        uiCombboxStartupFirmware->setEditText(romFile);
    }
}
void StartupDialog::browseSystemFolder()
{
    QString dir;
    QString romFile;
    QString workingDir = uiCombboxSystemFolder->lineEdit()->text();
        
    // Setup the startup directory. The startup directory is the folder
    // where the currently selected system file is located if no
    // system config file is currently is selected get the directory of the
    // most recently system folder
    if (workingDir.isEmpty())
    {
        workingDir = uiCombboxSystemFolder->itemText(0);
    }
        
    dir = QFileDialog::getExistingDirectory(this,
        tr("Select System Folder"), workingDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        int nSelIndex =  uiCombboxSystemFolder->findText(dir);
        if (nSelIndex == -1)
        {
            nSelIndex = 0;
            uiCombboxSystemFolder->insertItem(nSelIndex,dir);
        }
        uiCombboxSystemFolder->setCurrentIndex(nSelIndex);
        uiCombboxSystemFolder->setEditText(dir);
    }
}


bool StartupDialog::validateSystemFolder(QString qstrSystemFolder)
{
    if (!qstrSystemFolder.isEmpty())
    {

        QDir dir(qstrSystemFolder);

        if (!dir.isAbsolute())
        {
            QMessageBox::critical(this, "Parameter Validation",
    		        "Absolute path to the system folder is required.");
            return false;
        }

        if (!dir.exists())
        {
            QMessageBox::critical(this, "Parameter Validation",
		            "The specified system folder does not exist.");
            return false;
        }

        QString filePath = dir.absoluteFilePath("config.xml");
        if (!QFile::exists(filePath))
        {
            QMessageBox::critical(this, "Parameter Validation",
		            "'config.xml' file is does not exist in the specified system folder.");
            return false;
        }
    }
    return true;
}
bool StartupDialog::validateFirmwarePath(QString fwPath)
{
    bool bRet = true;
    // If we are loading the firmware, the path to the
    // firmware must exist in aboslute path
    if (!fwPath.isEmpty())
    {
        QDir dir(fwPath);

        if (!dir.isAbsolute())
        {
            QMessageBox::critical(this, "Parameter Validation",
    		        "Absolute path to the firmware is required");
            return false;
        }

        if (!QFile::exists(fwPath))
        {
            QMessageBox::critical(this, "Parameter Validation",
    		        "The specified firmware does not exist.");
            return false;
        }
    }
    return bRet;
}

void StartupDialog::accept()
{
    if (doAccept())
    {
        QString systemPath(uiCombboxSystemFolder->lineEdit()->text());
        QString firmwarePath(uiCombboxStartupFirmware->lineEdit()->text());

        QDir systemFolder(systemPath);
        QString systemFilePath = systemFolder.absoluteFilePath("config.xml");
        QList<QString> list;
        int count;

        // Save the startup firmware path and the list of most recently
        // used ones.
        DVRSystem::setStartupFirmwarePath(firmwarePath);
        if (!firmwarePath.isEmpty())
        {
            // Make sure the firmware file that is typed in is in the
            // list.
            int nSelIndex =  uiCombboxStartupFirmware->findText(firmwarePath);
            if (nSelIndex == -1)
            {
                nSelIndex = 0;
                uiCombboxStartupFirmware->insertItem(nSelIndex,firmwarePath);
            }
        }
 
        count = uiCombboxStartupFirmware->count();
        list.clear();
        for (int i = 0; i < count; i++)
        {
            list.append(uiCombboxStartupFirmware->itemText(i));
        }
        DVRSystem::setStartupFirmwarePathList(list);


        // Save the system configuration path and the list of most recently
        // used ones.
        if (systemPath.isEmpty())
            systemFilePath.clear();
        else
        {
            // Make sure the system config file that is typed in is in the
            // list.
            int nSelIndex =  uiCombboxSystemFolder->findText(systemFilePath);
            if (nSelIndex == -1)
            {
                nSelIndex = 0;
                uiCombboxSystemFolder->insertItem(nSelIndex,systemFilePath);
            }
        }
        DVRSystem::setStartupSystem(systemFilePath);
        count = uiCombboxSystemFolder->count();
        list.clear();
        for (int i = 0; i < count; i++)
        {
            list.append(uiCombboxSystemFolder->itemText(i));
        }
        DVRSystem::setStartupSystemList(list);

        QDialog::accept();
    }
}

bool StartupDialog::doAccept()
{
    bool bRet = false;

    if (validateSystemFolder(uiCombboxSystemFolder->lineEdit()->text()))
    {
        if (validateFirmwarePath(uiCombboxStartupFirmware->lineEdit()->text()))
            bRet = true;
    }

    return bRet;
}

//
////////////////////////////// System Dialog /////////////////////////////
//
SystemDialog::SystemDialog(DVRSystem *system, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    m_system = system;

    //////////////////////////////////////////////////////////////////
    // Initialize the General Tab
    //////////////////////////////////////////////////////////////////
    sysNameEdit->setText(system->name());

    videoStdCombo->addItem("<select>", QVariant(SDVR_VIDEO_STD_NONE));
    for (sdvr_video_std_e std = SDVR_VIDEO_STD_D1_PAL;
        std <= SDVR_VIDEO_STD_4CIF_NTSC; std = (sdvr_video_std_e)(std << 1))
    {
        if (system->supportsVideoStandard(std))
            videoStdCombo->addItem(sdvr::videoStandardToString(std),
                                   QVariant(std));
    }

    // Show deferred video standard, if set
    sdvr_video_std_e curStd = system->deferredVideoStandard();
    if (curStd == SDVR_VIDEO_STD_NONE)
        curStd = system->videoStandard();

    videoStdCombo->setCurrentIndex(videoStdCombo->findData(QVariant(curStd)));

    // Setup the audio rate combobox

    uiComboAudioRate->addItem(sdvr::audioRateToString(SDVR_AUDIO_RATE_8KHZ),
                                   QVariant(SDVR_AUDIO_RATE_8KHZ));
    uiComboAudioRate->addItem(sdvr::audioRateToString(SDVR_AUDIO_RATE_16KHZ),
                                   QVariant(SDVR_AUDIO_RATE_16KHZ));

    // Show deferred audio rate, if set
    sdvr_audio_rate_e curAudioRate = system->deferredAudioRate();
    if (curAudioRate == SDVR_AUDIO_RATE_NONE)
        curAudioRate = system->getAudioRate();

    uiComboAudioRate->setCurrentIndex(uiComboAudioRate->findData(QVariant(curAudioRate)));


    if (DVRSystem::isStartupSystem(system->configFilePath()))
        openOnStartupCB->setCheckState(Qt::Checked);

    uiLEditGeneralLoadFirmware->setText(DVRSystem::getStartupFirmwarePath());
    uiLEditGeneralLoadFirmware->setReadOnly(true);
    uiLabelGeneralLoadFirmware->setEnabled(false);

    //////////////////////////////////////////////////////////////////
    // Initialize the Settings Tab
    //////////////////////////////////////////////////////////////////

    uiLEditTimeUpdateInterval->setText(QString::number(system->getUpdateTimeInterval()));
    uiLEditTimeUpdateInterval->setValidator(new QIntValidator(0, 1000, uiLEditTimeUpdateInterval));

    uiLEditSettingsTimeOut->setText(QString::number(system->getPCItimeOut()));
    uiChkBoxSettingsWatchdogTimer->setChecked(system->getWatchDogState());
    uiLEditSettingsWatchTimeout->setText(QString::number(system->getWatchDogTimeout()));
    uiLEditSettingsWatchTimeout->setValidator(new QIntValidator(1, 10, uiLEditSettingsWatchTimeout));

    uiLEditSettingsStatInterval->setText(QString::number(system->getStatFPSInterval()));

    // Do not allow changing of the recording options while
    // any camera is in recording.
    if (system->isAnyCameraRecording())
    {
        uiLabelMaxH264Frames->setEnabled(false);
        uiLEditMaxH264Frames->setEnabled(false);
        uiChkBoxSegmentRec->setEnabled(false);
        uiChkBoxMJPEGSytleAVI->setEnabled(false);
    }
    uiLEditMaxH264Frames->setText(QString::number(system->getH264MaxFramesRecord()));
    uiChkBoxSegmentRec->setChecked(system->isSegmentedRecording());
    uiChkBoxMJPEGSytleAVI->setChecked(system->isMJPEGRecStyleAVI());

    //////////////////////////////////////////////////////////////////
    // Initialize the Debug Tab
    //////////////////////////////////////////////////////////////////
    if (system->getDebugEnable())
        uiGrpSystemDebugDebug->setChecked(true);

    if (system->getDebugSDK())
        uiChkDebugSDK->setChecked(true);

    if (system->getDebugAVFrames())
        uiChkDebugAVFrames->setChecked(true);

    ////////////////////////////////////////////////////////////////////////
    // Initialize the font table tab
    ////////////////////////////////////////////////////////////////////////
    sx_uint8 y_color, u_color, v_color;
    sx_uint32 low, high;
    sdvr_firmware_ver_t  version;

    system->getFWVersion(&version);

    if (!sdvr::isMinVersion(
         version.fw_major, version.fw_minor, version.fw_revision, version.fw_build,
         3, 2, 0, 12))
    {
        uiGrpUseCustomFont->setChecked(false);
        uiGrpUseCustomFont->setEnabled(false);
    }
    else
    {
        uiGrpUseCustomFont->setChecked(system->getActiveFontTable() != 0);

        connect(uiBtnFontFileBrowse, SIGNAL(clicked()), SLOT(browseFontFile()));
        uiLEditFontFile->setText(system->getFontTablePath());

        uiLEditStartCode->setValidator(new QIntValidator(0, 65536, uiLEditStartCode));
        uiLEditEndCode->setValidator(new QIntValidator(0, 65536, uiLEditEndCode));
        system->getFontTableRange(&low, &high);
        uiLEditStartCode->setText(QString::number(low));
        uiLEditEndCode->setText(QString::number(high));

        uiLEditColor_y->setValidator(new QIntValidator(0, 255, uiLEditColor_y));
        uiLEditColor_u->setValidator(new QIntValidator(0, 255, uiLEditColor_u));
        uiLEditColor_v->setValidator(new QIntValidator(0, 255, uiLEditColor_v));
        system->getFontColor(&y_color, &u_color, &v_color);
        uiLEditColor_y->setText(QString::number(y_color));
        uiLEditColor_u->setText(QString::number(u_color));
        uiLEditColor_v->setText(QString::number(v_color));
    }
}

void SystemDialog::browseFontFile()
{
    QString bdfFile;
    QString workingDir = uiLEditFontFile->text();
        
    bdfFile = QFileDialog::getOpenFileName(this,
        tr("Select a BDF Font File"), workingDir,
        tr("*.bdf;;*.*"));

    if (!bdfFile.isEmpty())
    {
        uiLEditFontFile->setText(bdfFile);
    }
}

sdvr_video_std_e SystemDialog::videoStandard() const
{
    sdvr_video_std_e videoStd = (sdvr_video_std_e) videoStdCombo->itemData(
            videoStdCombo->currentIndex()).toInt();
    return videoStd;
}

sdvr_audio_rate_e SystemDialog::getAudioRate() const
{
    sdvr_audio_rate_e aRate = (sdvr_audio_rate_e) uiComboAudioRate->itemData(
            uiComboAudioRate->currentIndex()).toInt();

    return aRate;
}

bool SystemDialog::doAccept()
{
    bool ok;
    QList<QString> list;
    new_system_cngf_e configType = NEW_SYSTEM_BLANK_CNFG;

    if (videoStandard() == SDVR_VIDEO_STD_NONE)
    {
        QMessageBox::critical(this, "Parameter Validation",
        		"Please select video standard");
        return false;
    }

    uint uH264MaxFramesRecord = uiLEditMaxH264Frames->text().toUInt(&ok);
    if (!ok)
    {
        uH264MaxFramesRecord = 0;
    }

    // Only validate the configuration folder when
    // creating a new system.
    if (!isEditSystem()) 
    {
   	    QString path = uiCombboxSystemFolder->lineEdit()->text();
	    if (path.isEmpty())
	    {
            QMessageBox::critical(this, "Parameter Validation",
        		    "Please specify system folder");
            return false;
	    }

	    QDir dir(path);

        if (!dir.isAbsolute())
        {
            QMessageBox::critical(this, "Parameter Validation",
        		    "Absolute path to the system folder is required");
            return false;
        }

        if (!dir.exists())
	    {
		    if (QMessageBox::question(this, "Question",
				    "The system folder does not exist. Create?",
				     QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel)
				    != QMessageBox::Ok)
            return false;

		    if (!dir.mkpath(path))
		    {
	            QMessageBox::critical(this, "Parameter Validation",
	        		    tr("Could not create system folder '%1'").arg(path));
            return false;
		    }	
	    }

        QString systemFilePath = dir.absoluteFilePath("config.xml");
        if (QFile::exists(systemFilePath))
        {
		    if (QMessageBox::question(this, "Question",
			        "Configuration file 'config.xml' exists. Overwrite?",
				     QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel)
				    != QMessageBox::Ok)
            return false;
        }


        configType = (new_system_cngf_e)uiComboInitialSystem->itemData(
            uiComboInitialSystem->currentIndex()).toInt();
        QString qstrDefaultSystemPath = m_system->getDefaultSystemPath(configType);

        QFile file(systemFilePath);
        file.remove();
        if (!qstrDefaultSystemPath.isEmpty() && QFile::exists(qstrDefaultSystemPath))
        {
            // Copy the configuration file and open the system but do not
            // connect to the board or the channels until we read the
            // the new video std.
            // Since the defualt config file does not have any specific video standard
            // we must set the currently selected video standard.
            m_system->setVideoStandard(videoStandard());

            QFile::copy(qstrDefaultSystemPath,systemFilePath);
            if (!m_system->open(systemFilePath,false))
            {
                QMessageBox::critical(this, "DVRCP - Critical Error",
                    "Error in loading the default configuration system.");
                return false;
            }
        }
        else
        {
            if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
            {
                QMessageBox::critical(this, "Parameter Validation",
                    "Could not create configuration file");
                return false;
            }


            QTextStream ts(&file);
            m_system->save(ts);
            file.close();
        }

        ///////////////////////////////////////////////////////////////////
        // Save the system configuration path and the list of most recently
        // used ones.
        ///////////////////////////////////////////////////////////////////

        // Make sure the system config file that is typed in is in the list
        
        int nSelIndex =  uiCombboxSystemFolder->findText(path);
        if (nSelIndex == -1)
        {
            nSelIndex = 0;
            uiCombboxSystemFolder->insertItem(nSelIndex,path);
        }

        // Save the list of most recently used config folders.
        int count = uiCombboxSystemFolder->count();
        list.clear();
        for (int i = 0; i < count; i++)
        {
            list.append(uiCombboxSystemFolder->itemText(i));
        }
        DVRSystem::setStartupSystemList(list);

        // Save the current configuration file.
        m_system->setConfigFilePath(systemFilePath);
    }


    uint uPCItimeout = uiLEditSettingsTimeOut->text().toUInt(&ok);
    if (!ok)
    {
        QMessageBox::critical(this, "Parameter Validation",
            tr("Bad or missing PCI timeout value"));
        return false;
    }

    uint uStatFPSInterval = uiLEditSettingsStatInterval->text().toUInt(&ok);
    if (!ok)
    {
        QMessageBox::critical(this, "Parameter Validation",
            tr("Bad or missing frame rate sample range value."));
        return false;
    }
    ///////////////////////////////////////////////////////////
    // Save General Tab 

    // Use deferred video standard, if set and we are in the edit mode
    sdvr_video_std_e std = m_system->deferredVideoStandard();
    if (std == SDVR_VIDEO_STD_NONE)
        std = m_system->videoStandard();

    if (isEditSystem() && std != this->videoStandard())
    {
        if (QMessageBox::information(this, tr(""),
            "Video standard change will take effect next time the system is open",
            QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)
				!= QMessageBox::Ok)
            return false;
        m_system->setDeferredVideoStandard(this->videoStandard());
    }
    else
        m_system->setVideoStandard(videoStandard());

    // Use deferred audio rate, if set and we are in the edit mode
    sdvr_audio_rate_e audioRate = m_system->deferredAudioRate();
    if (audioRate == SDVR_AUDIO_RATE_NONE)
        audioRate = m_system->getAudioRate();

    if (isEditSystem() && audioRate != this->getAudioRate())
    {
        if (QMessageBox::information(this, tr(""),
            "The audio rate change will take effect next time the system is open",
            QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)
				!= QMessageBox::Ok)
            return false;
        m_system->setDeferredAudioRate(this->getAudioRate());
    }
    else
        m_system->setAudioRate(this->getAudioRate());

    m_system->setName(sysNameEdit->text());

    /////////////////////////////////////////////////////////////
    // Save Settings Tab
    bool bEnableWatchdog = uiChkBoxSettingsWatchdogTimer->isChecked();
    uint uWatchdogTimeout = uiLEditSettingsWatchTimeout->text().toUInt(&ok);
    uint uTimeUpdateInterval = uiLEditTimeUpdateInterval->text().toUInt(&ok);

    m_system->setUpdateTimeInterval(uTimeUpdateInterval);
    m_system->setWatchDogTimeout(uWatchdogTimeout);
    m_system->setWatchDogState(bEnableWatchdog);
    m_system->setPCItimeOut(uPCItimeout);
    m_system->setStatFPSInterval(uStatFPSInterval);
    m_system->setH264MaxFramesRecord(uH264MaxFramesRecord);
    m_system->setSegmentedRecording(uiChkBoxSegmentRec->isChecked());
    m_system->setMJPEGRecStyleAVI(uiChkBoxMJPEGSytleAVI->isChecked());

    // Save Debug Tab
    m_system->setDebugEnable(uiGrpSystemDebugDebug->isChecked());
    m_system->setDebugAVFrames(uiChkDebugAVFrames->isChecked());
    m_system->setDebugSDK(uiChkDebugSDK->isChecked());

	// Following information are saved in to the registary
    if (openOnStartupCB->checkState() == Qt::Checked)
        DVRSystem::setStartupSystem(m_system->configFilePath());
    else
        if (DVRSystem::isStartupSystem(m_system->configFilePath()))
            DVRSystem::setStartupSystem("");

    /////////////////////////////////////////////////////////////
    // Save font table Tab
    QString yColor, uColor, vColor;
    QString lowCode, highCode;

    // Currently the default custom ID that is supported is 21.
    // If the active font table is zero, it means use the
    // default font table in the firmware.
    m_system->setActiveFontTable(uiGrpUseCustomFont->isChecked() ? 21 : 0);

    m_system->setFontTablePath(uiLEditFontFile->text());

    lowCode = uiLEditStartCode->text();
    highCode = uiLEditEndCode->text();
    m_system->setFontTableRange(lowCode.toInt(), highCode.toInt());

    yColor = uiLEditColor_y->text();
    uColor = uiLEditColor_u->text();
    vColor = uiLEditColor_v->text();
    m_system->setFontColor(yColor.toInt(),uColor.toInt(),vColor.toInt());

    return true;
}
// ----------------------------- SystemNewDialog-------------------------------

SystemNewDialog::SystemNewDialog(DVRSystem *system, QWidget *parent)
    : SystemDialog(system, parent)
{
    QList<QString> startupList;
    connect(browseButton, SIGNAL(clicked()), SLOT(browse()));

    // Specify that we are in New system mode
    setEditSystem(false);
    // display the startup folder and the list most recently used ones.
    QFileInfo dirStartupSystem(DVRSystem::startupSystem());

    uiCombboxSystemFolder->setMaxCount(MAX_FOLDERS_REMEMBERED);
    startupList = DVRSystem::startupSystemList();
    foreach (QString qstrPrevStartupFolder, startupList)
    {
        uiCombboxSystemFolder->addItem(qstrPrevStartupFolder);
    }
    uiCombboxSystemFolder->setCurrentIndex(-1);

    // Show the combobox for the new dialogbox to
    // select initial system configuration
    uiLabelInitialSystem->setVisible(true);
    uiComboInitialSystem->setVisible(true);
    uiComboInitialSystem->addItem("Blank Configuration", QVariant(NEW_SYSTEM_BLANK_CNFG));
    uiComboInitialSystem->addItem("Encoder Configuration", QVariant(NEW_SYSTEM_ENCODER_ONLY_CNFG));
    // For now we don't have any default config for decoder or codec
//    uiComboInitialSystem->addItem("Decoder Configuration", QVariant(NEW_SYSTEM_DECODER_ONLY_CNFG));
//    uiComboInitialSystem->addItem("CODEC Configuration", QVariant(NEW_SYSTEM_COMBO_CNFG));
    uiComboInitialSystem->setCurrentIndex(uiComboInitialSystem->findData(QVariant(NEW_SYSTEM_BLANK_CNFG)));
}

void SystemNewDialog::browse()
{
    QString dir;
    QString romFile;
    QString workingDir = uiCombboxSystemFolder->lineEdit()->text();
        
    // Setup the startup directory. The startup directory is the folder
    // where the currently selected system file is located if no
    // system config file is currently is selected get the directory of the
    // most recently system folder
    if (workingDir.isEmpty())
    {
        workingDir = uiCombboxSystemFolder->itemText(0);
    }
        
    dir = QFileDialog::getExistingDirectory(this,
        tr("Select System Folder"), workingDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        int nSelIndex =  uiCombboxSystemFolder->findText(dir);
        if (nSelIndex == -1)
        {
            nSelIndex = 0;
            uiCombboxSystemFolder->insertItem(nSelIndex,dir);
        }
        uiCombboxSystemFolder->setCurrentIndex(nSelIndex);
        uiCombboxSystemFolder->setEditText(dir);
    }
}

void SystemNewDialog::accept()
{
    if (doAccept())
        QDialog::accept();
}
//------------------------- SystemEditDialog ----------------------------------

SystemEditDialog::SystemEditDialog(DVRSystem *system, QWidget *parent)
    : SystemDialog(system, parent)
{
    setWindowTitle("System Settings");

    // Specify that we are in edit system mode
    setEditSystem(true);

    uiCombboxSystemFolder->setEditText(system->systemDirPath());
    uiCombboxSystemFolder->lineEdit()->setReadOnly(true);
    uiLabelSystemFolder->setEnabled(false);
    browseButton->setEnabled(false);

    // When edit a system, hide the initial system config settings.
    uiLabelInitialSystem->setVisible(false);
    uiComboInitialSystem->setVisible(false);

}

void SystemEditDialog::accept()
{
    if (doAccept())
        QDialog::accept();

}

