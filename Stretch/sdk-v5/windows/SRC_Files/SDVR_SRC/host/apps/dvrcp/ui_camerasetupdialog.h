/********************************************************************************
** Form generated from reading ui file 'camerasetupdialog.ui'
**
** Created: Fri Apr 10 16:32:11 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CAMERASETUPDIALOG_H
#define UI_CAMERASETUPDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>

class Ui_CameraSetupDialog
{
public:
    QGroupBox *audioGroup;
    QRadioButton *G726Radio;
    QRadioButton *G711Radio;
    QDialogButtonBox *buttonBox;
    QGroupBox *uiGrpSecondaryVideoEncoder;
    QRadioButton *uiRadioSecondaryEncJPEG;
    QRadioButton *uiRadioSecondaryEncH264;
    QRadioButton *uiRadioSecondaryEncMPEG4;
    QGroupBox *uiGrpPrimaryVideoEncoder;
    QRadioButton *uiRadioPrimaryEncJPEG;
    QRadioButton *uiRadioPrimaryEncH264;
    QRadioButton *uiRadioPrimaryEncMPEG4;

    void setupUi(QDialog *CameraSetupDialog)
    {
    CameraSetupDialog->setObjectName(QString::fromUtf8("CameraSetupDialog"));
    CameraSetupDialog->setEnabled(true);
    audioGroup = new QGroupBox(CameraSetupDialog);
    audioGroup->setObjectName(QString::fromUtf8("audioGroup"));
    audioGroup->setEnabled(true);
    audioGroup->setGeometry(QRect(345, 20, 131, 122));
    audioGroup->setCheckable(true);
    audioGroup->setChecked(true);
    G726Radio = new QRadioButton(audioGroup);
    G726Radio->setObjectName(QString::fromUtf8("G726Radio"));
    G726Radio->setGeometry(QRect(30, 60, 61, 21));
    G711Radio = new QRadioButton(audioGroup);
    G711Radio->setObjectName(QString::fromUtf8("G711Radio"));
    G711Radio->setEnabled(true);
    G711Radio->setGeometry(QRect(30, 30, 61, 21));
    G711Radio->setCheckable(true);
    G711Radio->setChecked(true);
    buttonBox = new QDialogButtonBox(CameraSetupDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(286, 153, 191, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    uiGrpSecondaryVideoEncoder = new QGroupBox(CameraSetupDialog);
    uiGrpSecondaryVideoEncoder->setObjectName(QString::fromUtf8("uiGrpSecondaryVideoEncoder"));
    uiGrpSecondaryVideoEncoder->setGeometry(QRect(20, 86, 321, 60));
    uiGrpSecondaryVideoEncoder->setCheckable(true);
    uiGrpSecondaryVideoEncoder->setChecked(false);
    uiRadioSecondaryEncJPEG = new QRadioButton(uiGrpSecondaryVideoEncoder);
    uiRadioSecondaryEncJPEG->setObjectName(QString::fromUtf8("uiRadioSecondaryEncJPEG"));
    uiRadioSecondaryEncJPEG->setGeometry(QRect(88, 30, 61, 21));
    uiRadioSecondaryEncH264 = new QRadioButton(uiGrpSecondaryVideoEncoder);
    uiRadioSecondaryEncH264->setObjectName(QString::fromUtf8("uiRadioSecondaryEncH264"));
    uiRadioSecondaryEncH264->setGeometry(QRect(18, 30, 61, 21));
    uiRadioSecondaryEncH264->setChecked(true);
    uiRadioSecondaryEncMPEG4 = new QRadioButton(uiGrpSecondaryVideoEncoder);
    uiRadioSecondaryEncMPEG4->setObjectName(QString::fromUtf8("uiRadioSecondaryEncMPEG4"));
    uiRadioSecondaryEncMPEG4->setGeometry(QRect(155, 30, 71, 21));
    uiGrpPrimaryVideoEncoder = new QGroupBox(CameraSetupDialog);
    uiGrpPrimaryVideoEncoder->setObjectName(QString::fromUtf8("uiGrpPrimaryVideoEncoder"));
    uiGrpPrimaryVideoEncoder->setGeometry(QRect(20, 20, 321, 60));
    uiGrpPrimaryVideoEncoder->setCheckable(true);
    uiGrpPrimaryVideoEncoder->setChecked(true);
    uiRadioPrimaryEncJPEG = new QRadioButton(uiGrpPrimaryVideoEncoder);
    uiRadioPrimaryEncJPEG->setObjectName(QString::fromUtf8("uiRadioPrimaryEncJPEG"));
    uiRadioPrimaryEncJPEG->setGeometry(QRect(88, 30, 60, 18));
    uiRadioPrimaryEncH264 = new QRadioButton(uiGrpPrimaryVideoEncoder);
    uiRadioPrimaryEncH264->setObjectName(QString::fromUtf8("uiRadioPrimaryEncH264"));
    uiRadioPrimaryEncH264->setGeometry(QRect(18, 30, 60, 18));
    uiRadioPrimaryEncH264->setChecked(true);
    uiRadioPrimaryEncMPEG4 = new QRadioButton(uiGrpPrimaryVideoEncoder);
    uiRadioPrimaryEncMPEG4->setObjectName(QString::fromUtf8("uiRadioPrimaryEncMPEG4"));
    uiRadioPrimaryEncMPEG4->setGeometry(QRect(155, 30, 71, 21));
    QWidget::setTabOrder(uiRadioPrimaryEncH264, uiRadioPrimaryEncJPEG);
    QWidget::setTabOrder(uiRadioPrimaryEncJPEG, uiRadioPrimaryEncMPEG4);
    QWidget::setTabOrder(uiRadioPrimaryEncMPEG4, uiRadioSecondaryEncH264);
    QWidget::setTabOrder(uiRadioSecondaryEncH264, uiRadioSecondaryEncJPEG);
    QWidget::setTabOrder(uiRadioSecondaryEncJPEG, uiRadioSecondaryEncMPEG4);
    QWidget::setTabOrder(uiRadioSecondaryEncMPEG4, G711Radio);
    QWidget::setTabOrder(G711Radio, G726Radio);
    QWidget::setTabOrder(G726Radio, buttonBox);

    retranslateUi(CameraSetupDialog);

    QSize size(491, 193);
    size = size.expandedTo(CameraSetupDialog->minimumSizeHint());
    CameraSetupDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), CameraSetupDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), CameraSetupDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(CameraSetupDialog);
    } // setupUi

    void retranslateUi(QDialog *CameraSetupDialog)
    {
    CameraSetupDialog->setWindowTitle(QApplication::translate("CameraSetupDialog", "Camera Setup", 0, QApplication::UnicodeUTF8));
    audioGroup->setTitle(QApplication::translate("CameraSetupDialog", "Audio Encoding", 0, QApplication::UnicodeUTF8));
    G726Radio->setText(QApplication::translate("CameraSetupDialog", "G726", 0, QApplication::UnicodeUTF8));
    G711Radio->setText(QApplication::translate("CameraSetupDialog", "G711", 0, QApplication::UnicodeUTF8));
    uiGrpSecondaryVideoEncoder->setTitle(QApplication::translate("CameraSetupDialog", "Secondary Video Encoder", 0, QApplication::UnicodeUTF8));
    uiRadioSecondaryEncJPEG->setText(QApplication::translate("CameraSetupDialog", "MJPEG", 0, QApplication::UnicodeUTF8));
    uiRadioSecondaryEncH264->setText(QApplication::translate("CameraSetupDialog", "H.264", 0, QApplication::UnicodeUTF8));
    uiRadioSecondaryEncMPEG4->setText(QApplication::translate("CameraSetupDialog", "MPEG4", 0, QApplication::UnicodeUTF8));
    uiGrpPrimaryVideoEncoder->setTitle(QApplication::translate("CameraSetupDialog", "Primary Video Encoder", 0, QApplication::UnicodeUTF8));
    uiRadioPrimaryEncJPEG->setText(QApplication::translate("CameraSetupDialog", "MJPEG", 0, QApplication::UnicodeUTF8));
    uiRadioPrimaryEncH264->setText(QApplication::translate("CameraSetupDialog", "H.264", 0, QApplication::UnicodeUTF8));
    uiRadioPrimaryEncMPEG4->setText(QApplication::translate("CameraSetupDialog", "MPEG4", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(CameraSetupDialog);
    } // retranslateUi

};

namespace Ui {
    class CameraSetupDialog: public Ui_CameraSetupDialog {};
} // namespace Ui

#endif // UI_CAMERASETUPDIALOG_H
