/********************************************************************************
** Form generated from reading ui file 'decoderdialog.ui'
**
** Created: Fri Apr 10 16:32:09 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_DECODERDIALOG_H
#define UI_DECODERDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>

class Ui_DecoderDialog
{
public:
    QDialogButtonBox *buttonBox;
    QRadioButton *uiRadioFileName;
    QLabel *uiLabelAudioDecoder;
    QRadioButton *uiRadioADecoderNone;
    QRadioButton *uiRadioADecoderG711;
    QRadioButton *uiRadioADecoderG726_16K;
    QRadioButton *uiRadioADecoderG726_24K;
    QRadioButton *uiRadioADecoderG726_32K;
    QRadioButton *uiRadioVDecoderMP4;
    QRadioButton *uiRadioVDecoderMJPEG;
    QRadioButton *uiRadioCamera;
    QLabel *uiLabelCameraName;
    QComboBox *uiComboCameras;
    QRadioButton *uiRadioVDecoderH264;
    QLabel *uiLabelPath;
    QPushButton *uiBtnBrowse;
    QLineEdit *uiLEditPath;
    QLabel *uiLabelVideoSize;
    QComboBox *uiComboVideoSize;
    QLabel *uiLabelVideoDecoder;
    QLineEdit *uiLEditName;
    QLabel *uiLabelAttach;
    QLabel *uiLabelName;
    QGroupBox *uiGroupBoxConnect;
    QLabel *uiLableBoardId;
    QLabel *uiLabelDecoderChan;
    QComboBox *uiComboBoardId;
    QComboBox *uiComboDecoderChan;

    void setupUi(QDialog *DecoderDialog)
    {
    DecoderDialog->setObjectName(QString::fromUtf8("DecoderDialog"));
    buttonBox = new QDialogButtonBox(DecoderDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(260, 390, 341, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    uiRadioFileName = new QRadioButton(DecoderDialog);
    uiRadioFileName->setObjectName(QString::fromUtf8("uiRadioFileName"));
    uiRadioFileName->setGeometry(QRect(50, 80, 95, 21));
    uiLabelAudioDecoder = new QLabel(DecoderDialog);
    uiLabelAudioDecoder->setObjectName(QString::fromUtf8("uiLabelAudioDecoder"));
    uiLabelAudioDecoder->setEnabled(false);
    uiLabelAudioDecoder->setGeometry(QRect(70, 170, 100, 20));
    uiLabelAudioDecoder->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiRadioADecoderNone = new QRadioButton(DecoderDialog);
    uiRadioADecoderNone->setObjectName(QString::fromUtf8("uiRadioADecoderNone"));
    uiRadioADecoderNone->setEnabled(false);
    uiRadioADecoderNone->setGeometry(QRect(190, 170, 56, 21));
    uiRadioADecoderG711 = new QRadioButton(DecoderDialog);
    uiRadioADecoderG711->setObjectName(QString::fromUtf8("uiRadioADecoderG711"));
    uiRadioADecoderG711->setEnabled(false);
    uiRadioADecoderG711->setGeometry(QRect(260, 170, 56, 21));
    uiRadioADecoderG726_16K = new QRadioButton(DecoderDialog);
    uiRadioADecoderG726_16K->setObjectName(QString::fromUtf8("uiRadioADecoderG726_16K"));
    uiRadioADecoderG726_16K->setEnabled(false);
    uiRadioADecoderG726_16K->setGeometry(QRect(330, 170, 61, 21));
    uiRadioADecoderG726_24K = new QRadioButton(DecoderDialog);
    uiRadioADecoderG726_24K->setObjectName(QString::fromUtf8("uiRadioADecoderG726_24K"));
    uiRadioADecoderG726_24K->setEnabled(false);
    uiRadioADecoderG726_24K->setGeometry(QRect(420, 170, 82, 21));
    uiRadioADecoderG726_32K = new QRadioButton(DecoderDialog);
    uiRadioADecoderG726_32K->setObjectName(QString::fromUtf8("uiRadioADecoderG726_32K"));
    uiRadioADecoderG726_32K->setEnabled(false);
    uiRadioADecoderG726_32K->setGeometry(QRect(510, 170, 81, 21));
    uiRadioVDecoderMP4 = new QRadioButton(DecoderDialog);
    uiRadioVDecoderMP4->setObjectName(QString::fromUtf8("uiRadioVDecoderMP4"));
    uiRadioVDecoderMP4->setEnabled(false);
    uiRadioVDecoderMP4->setGeometry(QRect(280, 140, 61, 21));
    uiRadioVDecoderMJPEG = new QRadioButton(DecoderDialog);
    uiRadioVDecoderMJPEG->setObjectName(QString::fromUtf8("uiRadioVDecoderMJPEG"));
    uiRadioVDecoderMJPEG->setEnabled(false);
    uiRadioVDecoderMJPEG->setGeometry(QRect(360, 140, 71, 21));
    uiRadioCamera = new QRadioButton(DecoderDialog);
    uiRadioCamera->setObjectName(QString::fromUtf8("uiRadioCamera"));
    uiRadioCamera->setEnabled(false);
    uiRadioCamera->setGeometry(QRect(50, 200, 95, 21));
    uiLabelCameraName = new QLabel(DecoderDialog);
    uiLabelCameraName->setObjectName(QString::fromUtf8("uiLabelCameraName"));
    uiLabelCameraName->setEnabled(false);
    uiLabelCameraName->setGeometry(QRect(70, 230, 100, 20));
    uiLabelCameraName->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiComboCameras = new QComboBox(DecoderDialog);
    uiComboCameras->setObjectName(QString::fromUtf8("uiComboCameras"));
    uiComboCameras->setEnabled(false);
    uiComboCameras->setGeometry(QRect(190, 230, 141, 22));
    uiRadioVDecoderH264 = new QRadioButton(DecoderDialog);
    uiRadioVDecoderH264->setObjectName(QString::fromUtf8("uiRadioVDecoderH264"));
    uiRadioVDecoderH264->setGeometry(QRect(190, 140, 71, 21));
    uiLabelPath = new QLabel(DecoderDialog);
    uiLabelPath->setObjectName(QString::fromUtf8("uiLabelPath"));
    uiLabelPath->setGeometry(QRect(70, 110, 100, 17));
    uiLabelPath->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiBtnBrowse = new QPushButton(DecoderDialog);
    uiBtnBrowse->setObjectName(QString::fromUtf8("uiBtnBrowse"));
    uiBtnBrowse->setGeometry(QRect(510, 100, 75, 25));
    uiLEditPath = new QLineEdit(DecoderDialog);
    uiLEditPath->setObjectName(QString::fromUtf8("uiLEditPath"));
    uiLEditPath->setGeometry(QRect(190, 100, 311, 23));
    uiLEditPath->setMaxLength(256);
    uiLabelVideoSize = new QLabel(DecoderDialog);
    uiLabelVideoSize->setObjectName(QString::fromUtf8("uiLabelVideoSize"));
    uiLabelVideoSize->setGeometry(QRect(30, 270, 110, 20));
    uiLabelVideoSize->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiComboVideoSize = new QComboBox(DecoderDialog);
    uiComboVideoSize->setObjectName(QString::fromUtf8("uiComboVideoSize"));
    uiComboVideoSize->setGeometry(QRect(150, 270, 131, 22));
    uiLabelVideoDecoder = new QLabel(DecoderDialog);
    uiLabelVideoDecoder->setObjectName(QString::fromUtf8("uiLabelVideoDecoder"));
    uiLabelVideoDecoder->setGeometry(QRect(70, 140, 100, 20));
    uiLabelVideoDecoder->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLEditName = new QLineEdit(DecoderDialog);
    uiLEditName->setObjectName(QString::fromUtf8("uiLEditName"));
    uiLEditName->setGeometry(QRect(81, 20, 113, 23));
    uiLEditName->setMaxLength(20);
    uiLabelAttach = new QLabel(DecoderDialog);
    uiLabelAttach->setObjectName(QString::fromUtf8("uiLabelAttach"));
    uiLabelAttach->setGeometry(QRect(20, 60, 53, 17));
    uiLabelName = new QLabel(DecoderDialog);
    uiLabelName->setObjectName(QString::fromUtf8("uiLabelName"));
    uiLabelName->setGeometry(QRect(20, 20, 51, 17));
    uiLabelName->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    uiGroupBoxConnect = new QGroupBox(DecoderDialog);
    uiGroupBoxConnect->setObjectName(QString::fromUtf8("uiGroupBoxConnect"));
    uiGroupBoxConnect->setGeometry(QRect(20, 310, 321, 80));
    uiGroupBoxConnect->setCheckable(true);
    uiGroupBoxConnect->setChecked(false);
    uiLableBoardId = new QLabel(uiGroupBoxConnect);
    uiLableBoardId->setObjectName(QString::fromUtf8("uiLableBoardId"));
    uiLableBoardId->setGeometry(QRect(20, 20, 110, 17));
    uiLableBoardId->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiLabelDecoderChan = new QLabel(uiGroupBoxConnect);
    uiLabelDecoderChan->setObjectName(QString::fromUtf8("uiLabelDecoderChan"));
    uiLabelDecoderChan->setGeometry(QRect(20, 50, 110, 20));
    uiLabelDecoderChan->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiComboBoardId = new QComboBox(uiGroupBoxConnect);
    uiComboBoardId->setObjectName(QString::fromUtf8("uiComboBoardId"));
    uiComboBoardId->setGeometry(QRect(140, 20, 81, 22));
    uiComboDecoderChan = new QComboBox(uiGroupBoxConnect);
    uiComboDecoderChan->setObjectName(QString::fromUtf8("uiComboDecoderChan"));
    uiComboDecoderChan->setGeometry(QRect(140, 50, 81, 22));
    QWidget::setTabOrder(uiLEditName, uiComboDecoderChan);
    QWidget::setTabOrder(uiComboDecoderChan, uiRadioFileName);
    QWidget::setTabOrder(uiRadioFileName, uiLEditPath);
    QWidget::setTabOrder(uiLEditPath, uiRadioVDecoderH264);
    QWidget::setTabOrder(uiRadioVDecoderH264, uiRadioVDecoderMP4);
    QWidget::setTabOrder(uiRadioVDecoderMP4, uiRadioVDecoderMJPEG);
    QWidget::setTabOrder(uiRadioVDecoderMJPEG, uiRadioADecoderNone);
    QWidget::setTabOrder(uiRadioADecoderNone, uiRadioADecoderG711);
    QWidget::setTabOrder(uiRadioADecoderG711, uiRadioADecoderG726_16K);
    QWidget::setTabOrder(uiRadioADecoderG726_16K, uiRadioADecoderG726_24K);
    QWidget::setTabOrder(uiRadioADecoderG726_24K, uiRadioADecoderG726_32K);
    QWidget::setTabOrder(uiRadioADecoderG726_32K, uiRadioCamera);
    QWidget::setTabOrder(uiRadioCamera, uiComboCameras);
    QWidget::setTabOrder(uiComboCameras, buttonBox);

    retranslateUi(DecoderDialog);

    QSize size(615, 438);
    size = size.expandedTo(DecoderDialog->minimumSizeHint());
    DecoderDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), DecoderDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), DecoderDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(DecoderDialog);
    } // setupUi

    void retranslateUi(QDialog *DecoderDialog)
    {
    DecoderDialog->setWindowTitle(QApplication::translate("DecoderDialog", "Decoder Properties", 0, QApplication::UnicodeUTF8));
    uiRadioFileName->setText(QApplication::translate("DecoderDialog", "File Name", 0, QApplication::UnicodeUTF8));
    uiLabelAudioDecoder->setText(QApplication::translate("DecoderDialog", "Audio Decoder:", 0, QApplication::UnicodeUTF8));
    uiRadioADecoderNone->setWhatsThis(QApplication::translate("DecoderDialog", "Select if the given file name to decode is mp4 file format without any audio.", 0, QApplication::UnicodeUTF8));
    uiRadioADecoderNone->setText(QApplication::translate("DecoderDialog", "None", 0, QApplication::UnicodeUTF8));
    uiRadioADecoderG711->setWhatsThis(QApplication::translate("DecoderDialog", "Select if the given file name to decode is mp4 file format and the video is encoded as H.264 with audio encoded as G711.", 0, QApplication::UnicodeUTF8));
    uiRadioADecoderG711->setText(QApplication::translate("DecoderDialog", "G711", 0, QApplication::UnicodeUTF8));
    uiRadioADecoderG726_16K->setText(QApplication::translate("DecoderDialog", "G726-16K", 0, QApplication::UnicodeUTF8));
    uiRadioADecoderG726_24K->setText(QApplication::translate("DecoderDialog", "G726-24K", 0, QApplication::UnicodeUTF8));
    uiRadioADecoderG726_32K->setText(QApplication::translate("DecoderDialog", "G726-32K", 0, QApplication::UnicodeUTF8));
    uiRadioVDecoderMP4->setText(QApplication::translate("DecoderDialog", "MP4", 0, QApplication::UnicodeUTF8));
    uiRadioVDecoderMJPEG->setText(QApplication::translate("DecoderDialog", "MJPEG", 0, QApplication::UnicodeUTF8));
    uiRadioCamera->setText(QApplication::translate("DecoderDialog", "Camera", 0, QApplication::UnicodeUTF8));
    uiLabelCameraName->setText(QApplication::translate("DecoderDialog", "Camera Name:", 0, QApplication::UnicodeUTF8));
    uiRadioVDecoderH264->setWhatsThis(QApplication::translate("DecoderDialog", "Select if the given file name to decode is mp4 file format and the video is encoded as H.264", 0, QApplication::UnicodeUTF8));
    uiRadioVDecoderH264->setText(QApplication::translate("DecoderDialog", "H264", 0, QApplication::UnicodeUTF8));
    uiLabelPath->setText(QApplication::translate("DecoderDialog", "Path:", 0, QApplication::UnicodeUTF8));
    uiBtnBrowse->setText(QApplication::translate("DecoderDialog", "Browse...", 0, QApplication::UnicodeUTF8));
    uiLEditPath->setWhatsThis(QApplication::translate("DecoderDialog", "Specify a .mov file to decode. The .mov file must have been  created by DVRCP application.", 0, QApplication::UnicodeUTF8));
    uiLabelVideoSize->setText(QApplication::translate("DecoderDialog", "Video Size:", 0, QApplication::UnicodeUTF8));
    uiComboVideoSize->setWhatsThis(QApplication::translate("DecoderDialog", "Selecting \"Any\" lets the firmware to choose where the decoder is created on. Select 1-64 to force which chip the decoder is created on. Each 16 channel range corresponds to a chip on the DVR board.", 0, QApplication::UnicodeUTF8));
    uiLabelVideoDecoder->setText(QApplication::translate("DecoderDialog", "Video Decoder:", 0, QApplication::UnicodeUTF8));
    uiLabelAttach->setText(QApplication::translate("DecoderDialog", "Attach:", 0, QApplication::UnicodeUTF8));
    uiLabelName->setText(QApplication::translate("DecoderDialog", "Name:", 0, QApplication::UnicodeUTF8));
    uiGroupBoxConnect->setTitle(QApplication::translate("DecoderDialog", "Connect", 0, QApplication::UnicodeUTF8));
    uiLableBoardId->setText(QApplication::translate("DecoderDialog", "Board Index:", 0, QApplication::UnicodeUTF8));
    uiLabelDecoderChan->setText(QApplication::translate("DecoderDialog", "Decoder Channel:", 0, QApplication::UnicodeUTF8));
    uiComboDecoderChan->setWhatsThis(QApplication::translate("DecoderDialog", "Selecting \"Any\" lets the firmware to choose where the decoder is created on. Select 1-64 to force which chip the decoder is created on. Each 16 channel range corresponds to a chip on the DVR board.", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(DecoderDialog);
    } // retranslateUi

};

namespace Ui {
    class DecoderDialog: public Ui_DecoderDialog {};
} // namespace Ui

#endif // UI_DECODERDIALOG_H
