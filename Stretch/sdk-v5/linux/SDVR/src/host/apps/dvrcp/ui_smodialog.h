/********************************************************************************
** Form generated from reading ui file 'smodialog.ui'
**
** Created: Mon Mar 30 21:53:04 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SMODIALOG_H
#define UI_SMODIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

class Ui_SmoDialog
{
public:
    QVBoxLayout *vboxLayout;
    QFrame *frame;
    QGridLayout *gridLayout;
    QHBoxLayout *hboxLayout;
    QComboBox *osdNumberCombo;
    QSpacerItem *spacerItem;
    QLabel *osdNumberLabel;
    QLabel *label_1;
    QLabel *label;
    QLineEdit *osdTextEdit;
    QLabel *label_4;
    QHBoxLayout *hboxLayout1;
    QLabel *osdLocXLabel;
    QLineEdit *osdLocXEdit;
    QLabel *osdLocYLabel;
    QLineEdit *osdLocYEdit;
    QHBoxLayout *hboxLayout2;
    QComboBox *osdLocationCombo;
    QSpacerItem *spacerItem1;
    QLabel *label_2;
    QCheckBox *osdEnabledCheckBox;
    QHBoxLayout *hboxLayout3;
    QComboBox *osdDtsFormatCombo;
    QSpacerItem *spacerItem2;
    QLabel *osdTransLabel;
    QHBoxLayout *hboxLayout4;
    QSlider *osdTransSlider;
    QLabel *osdTransValueLabel;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SmoDialog)
    {
    SmoDialog->setObjectName(QString::fromUtf8("SmoDialog"));
    vboxLayout = new QVBoxLayout(SmoDialog);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    frame = new QFrame(SmoDialog);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setFrameShape(QFrame::Box);
    frame->setFrameShadow(QFrame::Sunken);
    gridLayout = new QGridLayout(frame);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    osdNumberCombo = new QComboBox(frame);
    osdNumberCombo->setObjectName(QString::fromUtf8("osdNumberCombo"));
    osdNumberCombo->setMaximumSize(QSize(50, 16777215));

    hboxLayout->addWidget(osdNumberCombo);

    spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);


    gridLayout->addLayout(hboxLayout, 0, 1, 1, 1);

    osdNumberLabel = new QLabel(frame);
    osdNumberLabel->setObjectName(QString::fromUtf8("osdNumberLabel"));
    osdNumberLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(osdNumberLabel, 0, 0, 1, 1);

    label_1 = new QLabel(frame);
    label_1->setObjectName(QString::fromUtf8("label_1"));
    label_1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(label_1, 2, 0, 1, 1);

    label = new QLabel(frame);
    label->setObjectName(QString::fromUtf8("label"));
    label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(label, 1, 0, 1, 1);

    osdTextEdit = new QLineEdit(frame);
    osdTextEdit->setObjectName(QString::fromUtf8("osdTextEdit"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(osdTextEdit->sizePolicy().hasHeightForWidth());
    osdTextEdit->setSizePolicy(sizePolicy);
    osdTextEdit->setMinimumSize(QSize(0, 0));

    gridLayout->addWidget(osdTextEdit, 2, 1, 1, 1);

    label_4 = new QLabel(frame);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(label_4, 5, 0, 1, 1);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(5);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    osdLocXLabel = new QLabel(frame);
    osdLocXLabel->setObjectName(QString::fromUtf8("osdLocXLabel"));
    osdLocXLabel->setEnabled(false);
    osdLocXLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout1->addWidget(osdLocXLabel);

    osdLocXEdit = new QLineEdit(frame);
    osdLocXEdit->setObjectName(QString::fromUtf8("osdLocXEdit"));
    osdLocXEdit->setEnabled(false);
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(osdLocXEdit->sizePolicy().hasHeightForWidth());
    osdLocXEdit->setSizePolicy(sizePolicy1);
    osdLocXEdit->setMaximumSize(QSize(50, 16777215));
    osdLocXEdit->setMaxLength(4);

    hboxLayout1->addWidget(osdLocXEdit);

    osdLocYLabel = new QLabel(frame);
    osdLocYLabel->setObjectName(QString::fromUtf8("osdLocYLabel"));
    osdLocYLabel->setEnabled(false);
    osdLocYLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout1->addWidget(osdLocYLabel);

    osdLocYEdit = new QLineEdit(frame);
    osdLocYEdit->setObjectName(QString::fromUtf8("osdLocYEdit"));
    osdLocYEdit->setEnabled(false);
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(osdLocYEdit->sizePolicy().hasHeightForWidth());
    osdLocYEdit->setSizePolicy(sizePolicy2);
    osdLocYEdit->setMaximumSize(QSize(50, 16777215));
    osdLocYEdit->setMaxLength(4);

    hboxLayout1->addWidget(osdLocYEdit);


    gridLayout->addLayout(hboxLayout1, 4, 1, 1, 1);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    osdLocationCombo = new QComboBox(frame);
    osdLocationCombo->setObjectName(QString::fromUtf8("osdLocationCombo"));
    osdLocationCombo->setMinimumSize(QSize(100, 0));
    osdLocationCombo->setMaximumSize(QSize(150, 16777215));
    osdLocationCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    hboxLayout2->addWidget(osdLocationCombo);

    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout2->addItem(spacerItem1);


    gridLayout->addLayout(hboxLayout2, 3, 1, 1, 1);

    label_2 = new QLabel(frame);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(label_2, 3, 0, 1, 1);

    osdEnabledCheckBox = new QCheckBox(frame);
    osdEnabledCheckBox->setObjectName(QString::fromUtf8("osdEnabledCheckBox"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(osdEnabledCheckBox->sizePolicy().hasHeightForWidth());
    osdEnabledCheckBox->setSizePolicy(sizePolicy3);

    gridLayout->addWidget(osdEnabledCheckBox, 1, 1, 1, 1);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    osdDtsFormatCombo = new QComboBox(frame);
    osdDtsFormatCombo->setObjectName(QString::fromUtf8("osdDtsFormatCombo"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(0));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(osdDtsFormatCombo->sizePolicy().hasHeightForWidth());
    osdDtsFormatCombo->setSizePolicy(sizePolicy4);
    osdDtsFormatCombo->setMinimumSize(QSize(100, 0));
    osdDtsFormatCombo->setMaximumSize(QSize(150, 16777215));
    osdDtsFormatCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    hboxLayout3->addWidget(osdDtsFormatCombo);

    spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout3->addItem(spacerItem2);


    gridLayout->addLayout(hboxLayout3, 5, 1, 1, 1);

    osdTransLabel = new QLabel(frame);
    osdTransLabel->setObjectName(QString::fromUtf8("osdTransLabel"));
    osdTransLabel->setEnabled(true);
    osdTransLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(osdTransLabel, 6, 0, 1, 1);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(6);
    hboxLayout4->setMargin(5);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    osdTransSlider = new QSlider(frame);
    osdTransSlider->setObjectName(QString::fromUtf8("osdTransSlider"));
    osdTransSlider->setEnabled(true);
    osdTransSlider->setMaximum(255);
    osdTransSlider->setOrientation(Qt::Horizontal);
    osdTransSlider->setTickPosition(QSlider::NoTicks);

    hboxLayout4->addWidget(osdTransSlider);

    osdTransValueLabel = new QLabel(frame);
    osdTransValueLabel->setObjectName(QString::fromUtf8("osdTransValueLabel"));
    osdTransValueLabel->setEnabled(true);
    osdTransValueLabel->setMinimumSize(QSize(30, 0));
    osdTransValueLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

    hboxLayout4->addWidget(osdTransValueLabel);


    gridLayout->addLayout(hboxLayout4, 6, 1, 1, 1);


    vboxLayout->addWidget(frame);

    buttonBox = new QDialogButtonBox(SmoDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    vboxLayout->addWidget(buttonBox);

    QWidget::setTabOrder(osdEnabledCheckBox, osdTextEdit);
    QWidget::setTabOrder(osdTextEdit, osdLocationCombo);
    QWidget::setTabOrder(osdLocationCombo, osdLocXEdit);
    QWidget::setTabOrder(osdLocXEdit, osdLocYEdit);
    QWidget::setTabOrder(osdLocYEdit, osdDtsFormatCombo);
    QWidget::setTabOrder(osdDtsFormatCombo, osdTransSlider);
    QWidget::setTabOrder(osdTransSlider, buttonBox);

    retranslateUi(SmoDialog);

    QSize size(400, 300);
    size = size.expandedTo(SmoDialog->minimumSizeHint());
    SmoDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), SmoDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), SmoDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(SmoDialog);
    } // setupUi

    void retranslateUi(QDialog *SmoDialog)
    {
    SmoDialog->setWindowTitle(QApplication::translate("SmoDialog", "Dialog", 0, QApplication::UnicodeUTF8));
    osdNumberLabel->setText(QApplication::translate("SmoDialog", "OSD number:", 0, QApplication::UnicodeUTF8));
    label_1->setText(QApplication::translate("SmoDialog", "OSD text:", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("SmoDialog", "OSD enabled:", 0, QApplication::UnicodeUTF8));
    osdTextEdit->setText(QString());
    label_4->setText(QApplication::translate("SmoDialog", "Time stamp:", 0, QApplication::UnicodeUTF8));
    osdLocXLabel->setText(QApplication::translate("SmoDialog", "Top left x:", 0, QApplication::UnicodeUTF8));
    osdLocYLabel->setText(QApplication::translate("SmoDialog", "Top left y:", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("SmoDialog", "Text location:", 0, QApplication::UnicodeUTF8));
    osdEnabledCheckBox->setText(QString());
    osdTransLabel->setText(QApplication::translate("SmoDialog", "Text translucency:", 0, QApplication::UnicodeUTF8));
    osdTransValueLabel->setText(QApplication::translate("SmoDialog", "0", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(SmoDialog);
    } // retranslateUi

};

namespace Ui {
    class SmoDialog: public Ui_SmoDialog {};
} // namespace Ui

#endif // UI_SMODIALOG_H
