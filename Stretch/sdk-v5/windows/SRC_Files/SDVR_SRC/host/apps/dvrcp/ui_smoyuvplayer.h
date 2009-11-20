/********************************************************************************
** Form generated from reading ui file 'smoyuvplayer.ui'
**
** Created: Fri Apr 10 16:32:01 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SMOYUVPLAYER_H
#define UI_SMOYUVPLAYER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

class Ui_SmoYuvPlayer
{
public:
    QGroupBox *ControlGroupBox;
    QWidget *widget;
    QHBoxLayout *hboxLayout;
    QPushButton *PlayButton;
    QPushButton *PauseButton;
    QPushButton *StopButton;
    QSpacerItem *spacerItem;
    QCheckBox *ContinuousCheckBox;
    QGroupBox *GroupBox;
    QWidget *layoutWidget_2;
    QHBoxLayout *hboxLayout1;
    QLabel *label;
    QLineEdit *FilePathEdit;
    QPushButton *BrowseButton;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLineEdit *FrameRateEdit;
    QLabel *label_3;
    QComboBox *FormatCombo;

    void setupUi(QWidget *SmoYuvPlayer)
    {
    SmoYuvPlayer->setObjectName(QString::fromUtf8("SmoYuvPlayer"));
    SmoYuvPlayer->setMinimumSize(QSize(370, 230));
    SmoYuvPlayer->setWindowIcon(QIcon());
    ControlGroupBox = new QGroupBox(SmoYuvPlayer);
    ControlGroupBox->setObjectName(QString::fromUtf8("ControlGroupBox"));
    ControlGroupBox->setGeometry(QRect(10, 160, 351, 61));
    widget = new QWidget(ControlGroupBox);
    widget->setObjectName(QString::fromUtf8("widget"));
    widget->setGeometry(QRect(21, 11, 264, 42));
    hboxLayout = new QHBoxLayout(widget);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    PlayButton = new QPushButton(widget);
    PlayButton->setObjectName(QString::fromUtf8("PlayButton"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(PlayButton->sizePolicy().hasHeightForWidth());
    PlayButton->setSizePolicy(sizePolicy);
    PlayButton->setMinimumSize(QSize(40, 40));
    PlayButton->setMaximumSize(QSize(40, 40));
    PlayButton->setIcon(QIcon(QString::fromUtf8(":/images/play.png")));
    PlayButton->setIconSize(QSize(24, 24));

    hboxLayout->addWidget(PlayButton);

    PauseButton = new QPushButton(widget);
    PauseButton->setObjectName(QString::fromUtf8("PauseButton"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(PauseButton->sizePolicy().hasHeightForWidth());
    PauseButton->setSizePolicy(sizePolicy1);
    PauseButton->setMinimumSize(QSize(40, 40));
    PauseButton->setMaximumSize(QSize(40, 40));
    PauseButton->setIcon(QIcon(QString::fromUtf8(":/images/pause.png")));
    PauseButton->setIconSize(QSize(24, 24));

    hboxLayout->addWidget(PauseButton);

    StopButton = new QPushButton(widget);
    StopButton->setObjectName(QString::fromUtf8("StopButton"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(StopButton->sizePolicy().hasHeightForWidth());
    StopButton->setSizePolicy(sizePolicy2);
    StopButton->setMinimumSize(QSize(40, 40));
    StopButton->setMaximumSize(QSize(40, 40));
    StopButton->setIcon(QIcon(QString::fromUtf8(":/images/stop.png")));
    StopButton->setIconSize(QSize(20, 20));

    hboxLayout->addWidget(StopButton);

    spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    ContinuousCheckBox = new QCheckBox(widget);
    ContinuousCheckBox->setObjectName(QString::fromUtf8("ContinuousCheckBox"));

    hboxLayout->addWidget(ContinuousCheckBox);

    GroupBox = new QGroupBox(SmoYuvPlayer);
    GroupBox->setObjectName(QString::fromUtf8("GroupBox"));
    GroupBox->setGeometry(QRect(10, 10, 351, 141));
    layoutWidget_2 = new QWidget(GroupBox);
    layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
    layoutWidget_2->setGeometry(QRect(20, 20, 311, 31));
    hboxLayout1 = new QHBoxLayout(layoutWidget_2);
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label = new QLabel(layoutWidget_2);
    label->setObjectName(QString::fromUtf8("label"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(5));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy3);
    label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout1->addWidget(label);

    FilePathEdit = new QLineEdit(layoutWidget_2);
    FilePathEdit->setObjectName(QString::fromUtf8("FilePathEdit"));

    hboxLayout1->addWidget(FilePathEdit);

    BrowseButton = new QPushButton(layoutWidget_2);
    BrowseButton->setObjectName(QString::fromUtf8("BrowseButton"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(BrowseButton->sizePolicy().hasHeightForWidth());
    BrowseButton->setSizePolicy(sizePolicy4);
    BrowseButton->setMinimumSize(QSize(32, 0));
    BrowseButton->setMaximumSize(QSize(32, 16777215));

    hboxLayout1->addWidget(BrowseButton);

    layoutWidget = new QWidget(GroupBox);
    layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
    layoutWidget->setGeometry(QRect(20, 60, 191, 65));
    gridLayout = new QGridLayout(layoutWidget);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    label_2 = new QLabel(layoutWidget);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy5);
    label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(label_2, 0, 0, 1, 1);

    FrameRateEdit = new QLineEdit(layoutWidget);
    FrameRateEdit->setObjectName(QString::fromUtf8("FrameRateEdit"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(FrameRateEdit->sizePolicy().hasHeightForWidth());
    FrameRateEdit->setSizePolicy(sizePolicy6);
    FrameRateEdit->setMaximumSize(QSize(100, 16777215));

    gridLayout->addWidget(FrameRateEdit, 0, 1, 1, 1);

    label_3 = new QLabel(layoutWidget);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(0));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy7);
    label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(label_3, 1, 0, 1, 1);

    FormatCombo = new QComboBox(layoutWidget);
    FormatCombo->setObjectName(QString::fromUtf8("FormatCombo"));
    QSizePolicy sizePolicy8(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy8.setHorizontalStretch(0);
    sizePolicy8.setVerticalStretch(0);
    sizePolicy8.setHeightForWidth(FormatCombo->sizePolicy().hasHeightForWidth());
    FormatCombo->setSizePolicy(sizePolicy8);
    FormatCombo->setMaximumSize(QSize(100, 16777215));

    gridLayout->addWidget(FormatCombo, 1, 1, 1, 1);


    retranslateUi(SmoYuvPlayer);

    QSize size(370, 230);
    size = size.expandedTo(SmoYuvPlayer->minimumSizeHint());
    SmoYuvPlayer->resize(size);


    QMetaObject::connectSlotsByName(SmoYuvPlayer);
    } // setupUi

    void retranslateUi(QWidget *SmoYuvPlayer)
    {
    SmoYuvPlayer->setWindowTitle(QApplication::translate("SmoYuvPlayer", "Form", 0, QApplication::UnicodeUTF8));
    ControlGroupBox->setTitle(QString());
    PlayButton->setText(QString());
    PauseButton->setText(QString());
    StopButton->setText(QString());
    ContinuousCheckBox->setText(QApplication::translate("SmoYuvPlayer", "Continuous", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("SmoYuvPlayer", "YUV File:", 0, QApplication::UnicodeUTF8));
    BrowseButton->setText(QApplication::translate("SmoYuvPlayer", "...", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("SmoYuvPlayer", "Frame rate:", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("SmoYuvPlayer", "Format:", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(SmoYuvPlayer);
    } // retranslateUi

};

namespace Ui {
    class SmoYuvPlayer: public Ui_SmoYuvPlayer {};
} // namespace Ui

#endif // UI_SMOYUVPLAYER_H
