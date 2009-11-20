/********************************************************************************
** Form generated from reading ui file 'statviewdialog.ui'
**
** Created: Fri Apr 10 16:32:01 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_STATVIEWDIALOG_H
#define UI_STATVIEWDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>

class Ui_StatViewDialog
{
public:
    QFrame *frame;
    QCheckBox *recStatCheckBox;
    QCheckBox *bitRateCheckBox;
    QCheckBox *encFpsCheckBox;
    QCheckBox *rawFpsCheckBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *StatViewDialog)
    {
    StatViewDialog->setObjectName(QString::fromUtf8("StatViewDialog"));
    frame = new QFrame(StatViewDialog);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setGeometry(QRect(20, 20, 251, 261));
    frame->setFrameShape(QFrame::Box);
    frame->setFrameShadow(QFrame::Sunken);
    recStatCheckBox = new QCheckBox(frame);
    recStatCheckBox->setObjectName(QString::fromUtf8("recStatCheckBox"));
    recStatCheckBox->setGeometry(QRect(20, 20, 151, 23));
    bitRateCheckBox = new QCheckBox(frame);
    bitRateCheckBox->setObjectName(QString::fromUtf8("bitRateCheckBox"));
    bitRateCheckBox->setGeometry(QRect(20, 110, 84, 23));
    encFpsCheckBox = new QCheckBox(frame);
    encFpsCheckBox->setObjectName(QString::fromUtf8("encFpsCheckBox"));
    encFpsCheckBox->setGeometry(QRect(20, 50, 171, 23));
    rawFpsCheckBox = new QCheckBox(frame);
    rawFpsCheckBox->setObjectName(QString::fromUtf8("rawFpsCheckBox"));
    rawFpsCheckBox->setGeometry(QRect(20, 80, 201, 23));
    buttonBox = new QDialogButtonBox(StatViewDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(290, 20, 81, 241));
    buttonBox->setOrientation(Qt::Vertical);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    retranslateUi(StatViewDialog);

    QSize size(389, 300);
    size = size.expandedTo(StatViewDialog->minimumSizeHint());
    StatViewDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), StatViewDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), StatViewDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(StatViewDialog);
    } // setupUi

    void retranslateUi(QDialog *StatViewDialog)
    {
    StatViewDialog->setWindowTitle(QApplication::translate("StatViewDialog", "System Status Info", 0, QApplication::UnicodeUTF8));
    recStatCheckBox->setText(QApplication::translate("StatViewDialog", "Recording status", 0, QApplication::UnicodeUTF8));
    bitRateCheckBox->setText(QApplication::translate("StatViewDialog", "Bit rate", 0, QApplication::UnicodeUTF8));
    encFpsCheckBox->setText(QApplication::translate("StatViewDialog", "Encoded frame rate", 0, QApplication::UnicodeUTF8));
    rawFpsCheckBox->setText(QApplication::translate("StatViewDialog", "Raw video frame rate", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(StatViewDialog);
    } // retranslateUi

};

namespace Ui {
    class StatViewDialog: public Ui_StatViewDialog {};
} // namespace Ui

#endif // UI_STATVIEWDIALOG_H
