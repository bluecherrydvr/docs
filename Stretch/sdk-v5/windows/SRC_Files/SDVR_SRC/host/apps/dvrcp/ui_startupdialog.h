/********************************************************************************
** Form generated from reading ui file 'startupdialog.ui'
**
** Created: Fri Apr 10 16:32:00 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_STARTUPDIALOG_H
#define UI_STARTUPDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

class Ui_StartupDialog
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *label_2;
    QGroupBox *uiGrpStartupSystem;
    QLabel *uiLableSystemFolder;
    QPushButton *uiBrowseSystemFolder;
    QLabel *uiLabelStartupFirmware;
    QPushButton *uiBrowseStartupFirmware;
    QLabel *label_3;
    QLabel *label;
    QComboBox *uiCombboxSystemFolder;
    QComboBox *uiCombboxStartupFirmware;

    void setupUi(QDialog *StartupDialog)
    {
    StartupDialog->setObjectName(QString::fromUtf8("StartupDialog"));
    buttonBox = new QDialogButtonBox(StartupDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(270, 223, 171, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    label_2 = new QLabel(StartupDialog);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setGeometry(QRect(30, 30, 381, 41));
    label_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    label_2->setWordWrap(true);
    uiGrpStartupSystem = new QGroupBox(StartupDialog);
    uiGrpStartupSystem->setObjectName(QString::fromUtf8("uiGrpStartupSystem"));
    uiGrpStartupSystem->setGeometry(QRect(10, 10, 431, 201));
    uiLableSystemFolder = new QLabel(uiGrpStartupSystem);
    uiLableSystemFolder->setObjectName(QString::fromUtf8("uiLableSystemFolder"));
    uiLableSystemFolder->setGeometry(QRect(20, 61, 102, 21));
    uiLableSystemFolder->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiBrowseSystemFolder = new QPushButton(uiGrpStartupSystem);
    uiBrowseSystemFolder->setObjectName(QString::fromUtf8("uiBrowseSystemFolder"));
    uiBrowseSystemFolder->setGeometry(QRect(380, 61, 41, 21));
    uiLabelStartupFirmware = new QLabel(uiGrpStartupSystem);
    uiLabelStartupFirmware->setObjectName(QString::fromUtf8("uiLabelStartupFirmware"));
    uiLabelStartupFirmware->setGeometry(QRect(10, 165, 104, 17));
    uiLabelStartupFirmware->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiBrowseStartupFirmware = new QPushButton(uiGrpStartupSystem);
    uiBrowseStartupFirmware->setObjectName(QString::fromUtf8("uiBrowseStartupFirmware"));
    uiBrowseStartupFirmware->setGeometry(QRect(380, 165, 41, 21));
    label_3 = new QLabel(uiGrpStartupSystem);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(20, 100, 391, 51));
    label_3->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    label_3->setWordWrap(true);
    label = new QLabel(uiGrpStartupSystem);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(20, 20, 381, 41));
    label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    label->setWordWrap(true);
    uiCombboxSystemFolder = new QComboBox(uiGrpStartupSystem);
    uiCombboxSystemFolder->setObjectName(QString::fromUtf8("uiCombboxSystemFolder"));
    uiCombboxSystemFolder->setGeometry(QRect(130, 60, 241, 22));
    uiCombboxSystemFolder->setEditable(true);
    uiCombboxSystemFolder->setMaxVisibleItems(4);
    uiCombboxSystemFolder->setInsertPolicy(QComboBox::InsertAtTop);
    uiCombboxStartupFirmware = new QComboBox(uiGrpStartupSystem);
    uiCombboxStartupFirmware->setObjectName(QString::fromUtf8("uiCombboxStartupFirmware"));
    uiCombboxStartupFirmware->setGeometry(QRect(130, 164, 241, 22));
    uiCombboxStartupFirmware->setEditable(true);
    uiCombboxStartupFirmware->setMaxVisibleItems(4);
    uiCombboxStartupFirmware->setInsertPolicy(QComboBox::InsertAtTop);
    QWidget::setTabOrder(uiCombboxSystemFolder, uiBrowseSystemFolder);
    QWidget::setTabOrder(uiBrowseSystemFolder, uiCombboxStartupFirmware);
    QWidget::setTabOrder(uiCombboxStartupFirmware, uiBrowseStartupFirmware);
    QWidget::setTabOrder(uiBrowseStartupFirmware, buttonBox);

    retranslateUi(StartupDialog);

    QSize size(448, 264);
    size = size.expandedTo(StartupDialog->minimumSizeHint());
    StartupDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), StartupDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), StartupDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(StartupDialog);
    } // setupUi

    void retranslateUi(QDialog *StartupDialog)
    {
    StartupDialog->setWindowTitle(QApplication::translate("StartupDialog", "Select DVRCP System", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("StartupDialog", "Select an existing system configuration folder to initialize the DVRCP. Leave it blank to create a new system configuration.", 0, QApplication::UnicodeUTF8));
    uiGrpStartupSystem->setTitle(QApplication::translate("StartupDialog", "Choose a firmware and system configuration:", 0, QApplication::UnicodeUTF8));
    uiLableSystemFolder->setWhatsThis(QString());
    uiLableSystemFolder->setText(QApplication::translate("StartupDialog", "System folder:", 0, QApplication::UnicodeUTF8));
    uiBrowseSystemFolder->setText(QApplication::translate("StartupDialog", "...", 0, QApplication::UnicodeUTF8));
    uiLabelStartupFirmware->setWhatsThis(QString());
    uiLabelStartupFirmware->setText(QApplication::translate("StartupDialog", "Startup Firmware:", 0, QApplication::UnicodeUTF8));
    uiBrowseStartupFirmware->setText(QApplication::translate("StartupDialog", "...", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("StartupDialog", "Select a romable firmware file to load on to the DVR board for this session. Leave it blank to use the exsiting firmware flashed on the DVR board.", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("StartupDialog", "Select an existing system configuration folder to initialize the DVRCP. Leave it blank to create a new system configuration.", 0, QApplication::UnicodeUTF8));
    uiCombboxSystemFolder->setWhatsThis(QApplication::translate("StartupDialog", "Select an existing system configuration folder to initialize the DVRCP. Leave it blank to create a new system configuration.", 0, QApplication::UnicodeUTF8));
    uiCombboxStartupFirmware->setWhatsThis(QApplication::translate("StartupDialog", "Select a romable firmware file to load on to the DVR board for this session. Leave it blank to use the exsiting firmware flashed on the DVR board.", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(StartupDialog);
    } // retranslateUi

};

namespace Ui {
    class StartupDialog: public Ui_StartupDialog {};
} // namespace Ui

#endif // UI_STARTUPDIALOG_H
