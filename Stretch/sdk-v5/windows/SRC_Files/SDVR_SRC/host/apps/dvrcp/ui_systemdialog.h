/********************************************************************************
** Form generated from reading ui file 'systemdialog.ui'
**
** Created: Fri Apr 10 16:32:00 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SYSTEMDIALOG_H
#define UI_SYSTEMDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QWidget>

class Ui_SystemDialog
{
public:
    QLabel *uiLabelInitialSystem;
    QComboBox *uiComboInitialSystem;
    QDialogButtonBox *buttonBox;
    QTabWidget *uiTabsSystem;
    QWidget *uiTabSystemGeneral;
    QComboBox *uiCombboxSystemFolder;
    QPushButton *browseButton;
    QLabel *uiLabelSystemFolder;
    QLabel *label_1;
    QCheckBox *openOnStartupCB;
    QLabel *uiLabelGeneralLoadFirmware;
    QLineEdit *uiLEditGeneralLoadFirmware;
    QLabel *label_3;
    QLabel *uiLabelAudioRate;
    QComboBox *videoStdCombo;
    QComboBox *uiComboAudioRate;
    QLineEdit *sysNameEdit;
    QWidget *uiTabSystemSettings;
    QLineEdit *uiLEditSettingsStatInterval;
    QLabel *uiLabelMaxH264Frames;
    QCheckBox *uiChkBoxSegmentRec;
    QCheckBox *uiChkBoxMJPEGSytleAVI;
    QCheckBox *uiChkBoxSettingsWatchdogTimer;
    QLabel *uiLabelWatchdotTimer;
    QLineEdit *uiLEditSettingsWatchTimeout;
    QLineEdit *uiLEditMaxH264Frames;
    QLabel *label;
    QLabel *uiLabelSettingsTimeOut;
    QLineEdit *uiLEditSettingsTimeOut;
    QLabel *uilabelSettingsStatInterval;
    QLabel *uiLabelTimeUpdateInterval;
    QLineEdit *uiLEditTimeUpdateInterval;
    QWidget *uiTabSystemDebug;
    QGroupBox *uiGrpSystemDebugDebug;
    QCheckBox *uiChkDebugAVFrames;
    QCheckBox *uiChkDebugSDK;
    QWidget *uiTabSystemOSDFont;
    QGroupBox *uiGrpUseCustomFont;
    QLabel *uiLabelColor_y;
    QLineEdit *uiLEditColor_y;
    QLabel *uiLabelColor_u;
    QLineEdit *uiLEditColor_u;
    QLabel *uiLabelColor_v;
    QLabel *uiLabelFontTableFile;
    QLineEdit *uiLEditFontFile;
    QPushButton *uiBtnFontFileBrowse;
    QLineEdit *uiLEditStartCode;
    QLabel *uiLabelStartCode;
    QLabel *uiLabelEndCode;
    QLineEdit *uiLEditColor_v;
    QLineEdit *uiLEditEndCode;

    void setupUi(QDialog *SystemDialog)
    {
    SystemDialog->setObjectName(QString::fromUtf8("SystemDialog"));
    uiLabelInitialSystem = new QLabel(SystemDialog);
    uiLabelInitialSystem->setObjectName(QString::fromUtf8("uiLabelInitialSystem"));
    uiLabelInitialSystem->setEnabled(true);
    uiLabelInitialSystem->setGeometry(QRect(12, 316, 91, 20));
    uiLabelInitialSystem->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiComboInitialSystem = new QComboBox(SystemDialog);
    uiComboInitialSystem->setObjectName(QString::fromUtf8("uiComboInitialSystem"));
    uiComboInitialSystem->setEnabled(true);
    uiComboInitialSystem->setGeometry(QRect(111, 316, 151, 22));
    buttonBox = new QDialogButtonBox(SystemDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(268, 310, 171, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    uiTabsSystem = new QTabWidget(SystemDialog);
    uiTabsSystem->setObjectName(QString::fromUtf8("uiTabsSystem"));
    uiTabsSystem->setGeometry(QRect(20, 13, 421, 291));
    uiTabsSystem->setContextMenuPolicy(Qt::NoContextMenu);
    uiTabSystemGeneral = new QWidget();
    uiTabSystemGeneral->setObjectName(QString::fromUtf8("uiTabSystemGeneral"));
    uiCombboxSystemFolder = new QComboBox(uiTabSystemGeneral);
    uiCombboxSystemFolder->setObjectName(QString::fromUtf8("uiCombboxSystemFolder"));
    uiCombboxSystemFolder->setGeometry(QRect(125, 60, 211, 22));
    uiCombboxSystemFolder->setEditable(true);
    uiCombboxSystemFolder->setMaxVisibleItems(4);
    browseButton = new QPushButton(uiTabSystemGeneral);
    browseButton->setObjectName(QString::fromUtf8("browseButton"));
    browseButton->setGeometry(QRect(350, 60, 40, 21));
    uiLabelSystemFolder = new QLabel(uiTabSystemGeneral);
    uiLabelSystemFolder->setObjectName(QString::fromUtf8("uiLabelSystemFolder"));
    uiLabelSystemFolder->setGeometry(QRect(10, 61, 102, 21));
    uiLabelSystemFolder->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_1 = new QLabel(uiTabSystemGeneral);
    label_1->setObjectName(QString::fromUtf8("label_1"));
    label_1->setGeometry(QRect(10, 20, 102, 20));
    label_1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    openOnStartupCB = new QCheckBox(uiTabSystemGeneral);
    openOnStartupCB->setObjectName(QString::fromUtf8("openOnStartupCB"));
    openOnStartupCB->setGeometry(QRect(10, 231, 281, 18));
    uiLabelGeneralLoadFirmware = new QLabel(uiTabSystemGeneral);
    uiLabelGeneralLoadFirmware->setObjectName(QString::fromUtf8("uiLabelGeneralLoadFirmware"));
    uiLabelGeneralLoadFirmware->setGeometry(QRect(8, 160, 104, 17));
    uiLabelGeneralLoadFirmware->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditGeneralLoadFirmware = new QLineEdit(uiTabSystemGeneral);
    uiLEditGeneralLoadFirmware->setObjectName(QString::fromUtf8("uiLEditGeneralLoadFirmware"));
    uiLEditGeneralLoadFirmware->setGeometry(QRect(125, 156, 271, 23));
    label_3 = new QLabel(uiTabSystemGeneral);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(10, 97, 102, 20));
    label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLabelAudioRate = new QLabel(uiTabSystemGeneral);
    uiLabelAudioRate->setObjectName(QString::fromUtf8("uiLabelAudioRate"));
    uiLabelAudioRate->setGeometry(QRect(10, 128, 102, 17));
    uiLabelAudioRate->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    videoStdCombo = new QComboBox(uiTabSystemGeneral);
    videoStdCombo->setObjectName(QString::fromUtf8("videoStdCombo"));
    videoStdCombo->setGeometry(QRect(125, 97, 81, 22));
    uiComboAudioRate = new QComboBox(uiTabSystemGeneral);
    uiComboAudioRate->setObjectName(QString::fromUtf8("uiComboAudioRate"));
    uiComboAudioRate->setGeometry(QRect(125, 127, 81, 22));
    sysNameEdit = new QLineEdit(uiTabSystemGeneral);
    sysNameEdit->setObjectName(QString::fromUtf8("sysNameEdit"));
    sysNameEdit->setGeometry(QRect(125, 20, 211, 23));
    uiTabsSystem->addTab(uiTabSystemGeneral, QApplication::translate("SystemDialog", "General", 0, QApplication::UnicodeUTF8));
    uiTabSystemSettings = new QWidget();
    uiTabSystemSettings->setObjectName(QString::fromUtf8("uiTabSystemSettings"));
    uiLEditSettingsStatInterval = new QLineEdit(uiTabSystemSettings);
    uiLEditSettingsStatInterval->setObjectName(QString::fromUtf8("uiLEditSettingsStatInterval"));
    uiLEditSettingsStatInterval->setGeometry(QRect(347, 69, 49, 21));
    uiLEditSettingsStatInterval->setMaxLength(2);
    uiLabelMaxH264Frames = new QLabel(uiTabSystemSettings);
    uiLabelMaxH264Frames->setObjectName(QString::fromUtf8("uiLabelMaxH264Frames"));
    uiLabelMaxH264Frames->setGeometry(QRect(30, 100, 302, 20));
    uiLabelMaxH264Frames->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiChkBoxSegmentRec = new QCheckBox(uiTabSystemSettings);
    uiChkBoxSegmentRec->setObjectName(QString::fromUtf8("uiChkBoxSegmentRec"));
    uiChkBoxSegmentRec->setGeometry(QRect(19, 127, 231, 21));
    uiChkBoxMJPEGSytleAVI = new QCheckBox(uiTabSystemSettings);
    uiChkBoxMJPEGSytleAVI->setObjectName(QString::fromUtf8("uiChkBoxMJPEGSytleAVI"));
    uiChkBoxMJPEGSytleAVI->setGeometry(QRect(19, 153, 391, 21));
    uiChkBoxMJPEGSytleAVI->setAutoFillBackground(true);
    uiChkBoxSettingsWatchdogTimer = new QCheckBox(uiTabSystemSettings);
    uiChkBoxSettingsWatchdogTimer->setObjectName(QString::fromUtf8("uiChkBoxSettingsWatchdogTimer"));
    uiChkBoxSettingsWatchdogTimer->setGeometry(QRect(19, 220, 161, 21));
    uiLabelWatchdotTimer = new QLabel(uiTabSystemSettings);
    uiLabelWatchdotTimer->setObjectName(QString::fromUtf8("uiLabelWatchdotTimer"));
    uiLabelWatchdotTimer->setGeometry(QRect(186, 220, 151, 20));
    uiLabelWatchdotTimer->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditSettingsWatchTimeout = new QLineEdit(uiTabSystemSettings);
    uiLEditSettingsWatchTimeout->setObjectName(QString::fromUtf8("uiLEditSettingsWatchTimeout"));
    uiLEditSettingsWatchTimeout->setGeometry(QRect(347, 218, 49, 23));
    uiLEditMaxH264Frames = new QLineEdit(uiTabSystemSettings);
    uiLEditMaxH264Frames->setObjectName(QString::fromUtf8("uiLEditMaxH264Frames"));
    uiLEditMaxH264Frames->setGeometry(QRect(347, 99, 49, 21));
    label = new QLabel(uiTabSystemSettings);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(40, 177, 321, 17));
    uiLabelSettingsTimeOut = new QLabel(uiTabSystemSettings);
    uiLabelSettingsTimeOut->setObjectName(QString::fromUtf8("uiLabelSettingsTimeOut"));
    uiLabelSettingsTimeOut->setGeometry(QRect(30, 42, 302, 17));
    uiLabelSettingsTimeOut->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditSettingsTimeOut = new QLineEdit(uiTabSystemSettings);
    uiLEditSettingsTimeOut->setObjectName(QString::fromUtf8("uiLEditSettingsTimeOut"));
    uiLEditSettingsTimeOut->setGeometry(QRect(347, 40, 49, 21));
    uiLEditSettingsTimeOut->setMaxLength(2);
    uilabelSettingsStatInterval = new QLabel(uiTabSystemSettings);
    uilabelSettingsStatInterval->setObjectName(QString::fromUtf8("uilabelSettingsStatInterval"));
    uilabelSettingsStatInterval->setGeometry(QRect(30, 69, 302, 20));
    uilabelSettingsStatInterval->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLabelTimeUpdateInterval = new QLabel(uiTabSystemSettings);
    uiLabelTimeUpdateInterval->setObjectName(QString::fromUtf8("uiLabelTimeUpdateInterval"));
    uiLabelTimeUpdateInterval->setGeometry(QRect(30, 15, 307, 17));
    uiLabelTimeUpdateInterval->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditTimeUpdateInterval = new QLineEdit(uiTabSystemSettings);
    uiLEditTimeUpdateInterval->setObjectName(QString::fromUtf8("uiLEditTimeUpdateInterval"));
    uiLEditTimeUpdateInterval->setGeometry(QRect(347, 10, 49, 23));
    uiTabsSystem->addTab(uiTabSystemSettings, QApplication::translate("SystemDialog", "Settings", 0, QApplication::UnicodeUTF8));
    uiTabSystemDebug = new QWidget();
    uiTabSystemDebug->setObjectName(QString::fromUtf8("uiTabSystemDebug"));
    uiGrpSystemDebugDebug = new QGroupBox(uiTabSystemDebug);
    uiGrpSystemDebugDebug->setObjectName(QString::fromUtf8("uiGrpSystemDebugDebug"));
    uiGrpSystemDebugDebug->setGeometry(QRect(10, 10, 391, 221));
    uiGrpSystemDebugDebug->setCheckable(true);
    uiChkDebugAVFrames = new QCheckBox(uiGrpSystemDebugDebug);
    uiChkDebugAVFrames->setObjectName(QString::fromUtf8("uiChkDebugAVFrames"));
    uiChkDebugAVFrames->setGeometry(QRect(90, 27, 95, 21));
    uiChkDebugSDK = new QCheckBox(uiGrpSystemDebugDebug);
    uiChkDebugSDK->setObjectName(QString::fromUtf8("uiChkDebugSDK"));
    uiChkDebugSDK->setGeometry(QRect(20, 27, 50, 21));
    uiTabsSystem->addTab(uiTabSystemDebug, QApplication::translate("SystemDialog", "Debug", 0, QApplication::UnicodeUTF8));
    uiTabSystemOSDFont = new QWidget();
    uiTabSystemOSDFont->setObjectName(QString::fromUtf8("uiTabSystemOSDFont"));
    uiGrpUseCustomFont = new QGroupBox(uiTabSystemOSDFont);
    uiGrpUseCustomFont->setObjectName(QString::fromUtf8("uiGrpUseCustomFont"));
    uiGrpUseCustomFont->setGeometry(QRect(10, 10, 401, 161));
    uiGrpUseCustomFont->setCheckable(true);
    uiGrpUseCustomFont->setChecked(false);
    uiLabelColor_y = new QLabel(uiGrpUseCustomFont);
    uiLabelColor_y->setObjectName(QString::fromUtf8("uiLabelColor_y"));
    uiLabelColor_y->setGeometry(QRect(0, 110, 60, 20));
    uiLabelColor_y->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditColor_y = new QLineEdit(uiGrpUseCustomFont);
    uiLEditColor_y->setObjectName(QString::fromUtf8("uiLEditColor_y"));
    uiLEditColor_y->setGeometry(QRect(60, 110, 60, 23));
    uiLabelColor_u = new QLabel(uiGrpUseCustomFont);
    uiLabelColor_u->setObjectName(QString::fromUtf8("uiLabelColor_u"));
    uiLabelColor_u->setGeometry(QRect(130, 110, 60, 20));
    uiLabelColor_u->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditColor_u = new QLineEdit(uiGrpUseCustomFont);
    uiLEditColor_u->setObjectName(QString::fromUtf8("uiLEditColor_u"));
    uiLEditColor_u->setGeometry(QRect(200, 110, 60, 23));
    uiLabelColor_v = new QLabel(uiGrpUseCustomFont);
    uiLabelColor_v->setObjectName(QString::fromUtf8("uiLabelColor_v"));
    uiLabelColor_v->setGeometry(QRect(260, 110, 60, 20));
    uiLabelColor_v->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLabelFontTableFile = new QLabel(uiGrpUseCustomFont);
    uiLabelFontTableFile->setObjectName(QString::fromUtf8("uiLabelFontTableFile"));
    uiLabelFontTableFile->setGeometry(QRect(0, 30, 101, 17));
    uiLabelFontTableFile->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditFontFile = new QLineEdit(uiGrpUseCustomFont);
    uiLEditFontFile->setObjectName(QString::fromUtf8("uiLEditFontFile"));
    uiLEditFontFile->setGeometry(QRect(104, 28, 231, 23));
    uiBtnFontFileBrowse = new QPushButton(uiGrpUseCustomFont);
    uiBtnFontFileBrowse->setObjectName(QString::fromUtf8("uiBtnFontFileBrowse"));
    uiBtnFontFileBrowse->setGeometry(QRect(354, 28, 40, 25));
    uiLEditStartCode = new QLineEdit(uiGrpUseCustomFont);
    uiLEditStartCode->setObjectName(QString::fromUtf8("uiLEditStartCode"));
    uiLEditStartCode->setGeometry(QRect(102, 68, 60, 23));
    uiLabelStartCode = new QLabel(uiGrpUseCustomFont);
    uiLabelStartCode->setObjectName(QString::fromUtf8("uiLabelStartCode"));
    uiLabelStartCode->setGeometry(QRect(10, 70, 81, 20));
    uiLabelStartCode->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLabelEndCode = new QLabel(uiGrpUseCustomFont);
    uiLabelEndCode->setObjectName(QString::fromUtf8("uiLabelEndCode"));
    uiLabelEndCode->setGeometry(QRect(252, 70, 71, 17));
    uiLabelEndCode->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditColor_v = new QLineEdit(uiGrpUseCustomFont);
    uiLEditColor_v->setObjectName(QString::fromUtf8("uiLEditColor_v"));
    uiLEditColor_v->setGeometry(QRect(330, 110, 60, 23));
    uiLEditEndCode = new QLineEdit(uiGrpUseCustomFont);
    uiLEditEndCode->setObjectName(QString::fromUtf8("uiLEditEndCode"));
    uiLEditEndCode->setGeometry(QRect(330, 68, 60, 23));
    uiTabsSystem->addTab(uiTabSystemOSDFont, QApplication::translate("SystemDialog", "OSD Font", 0, QApplication::UnicodeUTF8));
    QWidget::setTabOrder(uiTabsSystem, sysNameEdit);
    QWidget::setTabOrder(sysNameEdit, uiCombboxSystemFolder);
    QWidget::setTabOrder(uiCombboxSystemFolder, browseButton);
    QWidget::setTabOrder(browseButton, videoStdCombo);
    QWidget::setTabOrder(videoStdCombo, uiComboAudioRate);
    QWidget::setTabOrder(uiComboAudioRate, uiLEditGeneralLoadFirmware);
    QWidget::setTabOrder(uiLEditGeneralLoadFirmware, openOnStartupCB);
    QWidget::setTabOrder(openOnStartupCB, uiComboInitialSystem);
    QWidget::setTabOrder(uiComboInitialSystem, buttonBox);
    QWidget::setTabOrder(buttonBox, uiLEditEndCode);
    QWidget::setTabOrder(uiLEditEndCode, uiLEditColor_y);
    QWidget::setTabOrder(uiLEditColor_y, uiLEditColor_u);
    QWidget::setTabOrder(uiLEditColor_u, uiLEditColor_v);
    QWidget::setTabOrder(uiLEditColor_v, uiLEditTimeUpdateInterval);
    QWidget::setTabOrder(uiLEditTimeUpdateInterval, uiLEditSettingsTimeOut);
    QWidget::setTabOrder(uiLEditSettingsTimeOut, uiLEditSettingsStatInterval);
    QWidget::setTabOrder(uiLEditSettingsStatInterval, uiLEditMaxH264Frames);
    QWidget::setTabOrder(uiLEditMaxH264Frames, uiChkBoxSegmentRec);
    QWidget::setTabOrder(uiChkBoxSegmentRec, uiChkBoxMJPEGSytleAVI);
    QWidget::setTabOrder(uiChkBoxMJPEGSytleAVI, uiChkBoxSettingsWatchdogTimer);
    QWidget::setTabOrder(uiChkBoxSettingsWatchdogTimer, uiLEditSettingsWatchTimeout);
    QWidget::setTabOrder(uiLEditSettingsWatchTimeout, uiChkDebugSDK);
    QWidget::setTabOrder(uiChkDebugSDK, uiChkDebugAVFrames);
    QWidget::setTabOrder(uiChkDebugAVFrames, uiLEditFontFile);
    QWidget::setTabOrder(uiLEditFontFile, uiBtnFontFileBrowse);
    QWidget::setTabOrder(uiBtnFontFileBrowse, uiLEditStartCode);

    retranslateUi(SystemDialog);

    QSize size(452, 355);
    size = size.expandedTo(SystemDialog->minimumSizeHint());
    SystemDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), SystemDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), SystemDialog, SLOT(reject()));

    uiTabsSystem->setCurrentIndex(0);


    QMetaObject::connectSlotsByName(SystemDialog);
    } // setupUi

    void retranslateUi(QDialog *SystemDialog)
    {
    SystemDialog->setWindowTitle(QApplication::translate("SystemDialog", "New System", 0, QApplication::UnicodeUTF8));
    uiLabelInitialSystem->setText(QApplication::translate("SystemDialog", "Initial System:", 0, QApplication::UnicodeUTF8));
    browseButton->setText(QApplication::translate("SystemDialog", "...", 0, QApplication::UnicodeUTF8));
    uiLabelSystemFolder->setText(QApplication::translate("SystemDialog", "System Folder:", 0, QApplication::UnicodeUTF8));
    label_1->setText(QApplication::translate("SystemDialog", "System Name:", 0, QApplication::UnicodeUTF8));
    openOnStartupCB->setText(QApplication::translate("SystemDialog", "Open this System Confiruation  on Start-up", 0, QApplication::UnicodeUTF8));
    uiLabelGeneralLoadFirmware->setText(QApplication::translate("SystemDialog", "Startup Firmware:", 0, QApplication::UnicodeUTF8));
    uiLEditGeneralLoadFirmware->setToolTip(QApplication::translate("SystemDialog", "Enter the filename of a DVR firmware to load.", 0, QApplication::UnicodeUTF8));
    uiLEditGeneralLoadFirmware->setWhatsThis(QApplication::translate("SystemDialog", "Enter  the path and name of a DVR firmware to be loaded on the startup of the application. Note that there is only one startup firmware for all the system configuration. The value of this field is kept in the registary and not the config.xml file.", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("SystemDialog", "Video Standard:", 0, QApplication::UnicodeUTF8));
    uiLabelAudioRate->setText(QApplication::translate("SystemDialog", "Audio Rate:", 0, QApplication::UnicodeUTF8));
    videoStdCombo->setToolTip(QApplication::translate("SystemDialog", "Select a video standard to be loaded next the system is opened.", 0, QApplication::UnicodeUTF8));
    uiTabsSystem->setTabText(uiTabsSystem->indexOf(uiTabSystemGeneral), QApplication::translate("SystemDialog", "General", 0, QApplication::UnicodeUTF8));
    uiLabelMaxH264Frames->setToolTip(QString());
    uiLabelMaxH264Frames->setWhatsThis(QApplication::translate("SystemDialog", "Enter maximum number of frames to record in a H.264 or AVI recorded file. Enter zero for no limit.", 0, QApplication::UnicodeUTF8));
    uiLabelMaxH264Frames->setText(QApplication::translate("SystemDialog", "Max. Number of Frames to Record:", 0, QApplication::UnicodeUTF8));
    uiChkBoxSegmentRec->setText(QApplication::translate("SystemDialog", "Create a New File on Recording.", 0, QApplication::UnicodeUTF8));
    uiChkBoxMJPEGSytleAVI->setText(QApplication::translate("SystemDialog", "Use AVI style file for MJPEG Recording. (Uncheck to use .mov)", 0, QApplication::UnicodeUTF8));
    uiChkBoxSettingsWatchdogTimer->setText(QApplication::translate("SystemDialog", "Enable Watchdog Timer", 0, QApplication::UnicodeUTF8));
    uiLabelWatchdotTimer->setText(QApplication::translate("SystemDialog", "Watchdog Timeout(sec.):", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("SystemDialog", "(There is no audio support in AVI file recording.)", 0, QApplication::UnicodeUTF8));
    uiLabelSettingsTimeOut->setText(QApplication::translate("SystemDialog", "PCI Timeout (sec.):", 0, QApplication::UnicodeUTF8));
    uilabelSettingsStatInterval->setText(QApplication::translate("SystemDialog", "Frame Rate Statistics Interval (sec.):", 0, QApplication::UnicodeUTF8));
    uiLabelTimeUpdateInterval->setText(QApplication::translate("SystemDialog", "Date/Time Update Interval (minutes):", 0, QApplication::UnicodeUTF8));
    uiTabsSystem->setTabText(uiTabsSystem->indexOf(uiTabSystemSettings), QApplication::translate("SystemDialog", "Settings", 0, QApplication::UnicodeUTF8));
    uiGrpSystemDebugDebug->setTitle(QApplication::translate("SystemDialog", "Enable Debug", 0, QApplication::UnicodeUTF8));
    uiChkDebugAVFrames->setText(QApplication::translate("SystemDialog", "A/V Frames", 0, QApplication::UnicodeUTF8));
    uiChkDebugSDK->setText(QApplication::translate("SystemDialog", "SDK", 0, QApplication::UnicodeUTF8));
    uiTabsSystem->setTabText(uiTabsSystem->indexOf(uiTabSystemDebug), QApplication::translate("SystemDialog", "Debug", 0, QApplication::UnicodeUTF8));
    uiGrpUseCustomFont->setTitle(QApplication::translate("SystemDialog", "Use Custom Font Table:", 0, QApplication::UnicodeUTF8));
    uiLabelColor_y->setText(QApplication::translate("SystemDialog", "Y-Color:", 0, QApplication::UnicodeUTF8));
    uiLabelColor_u->setText(QApplication::translate("SystemDialog", "U-Color:", 0, QApplication::UnicodeUTF8));
    uiLabelColor_v->setText(QApplication::translate("SystemDialog", "V-Color:", 0, QApplication::UnicodeUTF8));
    uiLabelFontTableFile->setText(QApplication::translate("SystemDialog", "Font File (.bdf):", 0, QApplication::UnicodeUTF8));
    uiBtnFontFileBrowse->setText(QApplication::translate("SystemDialog", "...", 0, QApplication::UnicodeUTF8));
    uiLabelStartCode->setText(QApplication::translate("SystemDialog", "Start Code:", 0, QApplication::UnicodeUTF8));
    uiLabelEndCode->setText(QApplication::translate("SystemDialog", "End Code:", 0, QApplication::UnicodeUTF8));
    uiTabsSystem->setTabText(uiTabsSystem->indexOf(uiTabSystemOSDFont), QApplication::translate("SystemDialog", "OSD Font", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(SystemDialog);
    } // retranslateUi

};

namespace Ui {
    class SystemDialog: public Ui_SystemDialog {};
} // namespace Ui

#endif // UI_SYSTEMDIALOG_H
