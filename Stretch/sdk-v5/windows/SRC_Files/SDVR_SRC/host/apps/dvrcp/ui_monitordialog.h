/********************************************************************************
** Form generated from reading ui file 'monitordialog.ui'
**
** Created: Fri Apr 10 16:32:06 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MONITORDIALOG_H
#define UI_MONITORDIALOG_H

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

class Ui_MonitorDialog
{
public:
    QLabel *label_2;
    QLabel *label_1;
    QCheckBox *uiChkBoxEnableRawAudio;
    QComboBox *decimationCombo;
    QLineEdit *rowsEdit;
    QLineEdit *colsEdit;
    QLabel *label_4;
    QLabel *label_3;
    QLineEdit *nameEdit;
    QGroupBox *smoGroup;
    QComboBox *boardCombo;
    QLineEdit *smoLeftMarginEdit;
    QLineEdit *smoTopMarginEdit;
    QLineEdit *smoSpacingEdit;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_5;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *MonitorDialog)
    {
    MonitorDialog->setObjectName(QString::fromUtf8("MonitorDialog"));
    label_2 = new QLabel(MonitorDialog);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setGeometry(QRect(20, 80, 131, 20));
    label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_1 = new QLabel(MonitorDialog);
    label_1->setObjectName(QString::fromUtf8("label_1"));
    label_1->setGeometry(QRect(14, 37, 91, 21));
    label_1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    uiChkBoxEnableRawAudio = new QCheckBox(MonitorDialog);
    uiChkBoxEnableRawAudio->setObjectName(QString::fromUtf8("uiChkBoxEnableRawAudio"));
    uiChkBoxEnableRawAudio->setGeometry(QRect(19, 201, 131, 21));
    decimationCombo = new QComboBox(MonitorDialog);
    decimationCombo->setObjectName(QString::fromUtf8("decimationCombo"));
    decimationCombo->setGeometry(QRect(160, 80, 71, 22));
    rowsEdit = new QLineEdit(MonitorDialog);
    rowsEdit->setObjectName(QString::fromUtf8("rowsEdit"));
    rowsEdit->setGeometry(QRect(160, 120, 71, 20));
    colsEdit = new QLineEdit(MonitorDialog);
    colsEdit->setObjectName(QString::fromUtf8("colsEdit"));
    colsEdit->setGeometry(QRect(160, 160, 71, 20));
    label_4 = new QLabel(MonitorDialog);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setGeometry(QRect(58, 160, 92, 21));
    label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_3 = new QLabel(MonitorDialog);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(58, 120, 92, 21));
    label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    nameEdit = new QLineEdit(MonitorDialog);
    nameEdit->setObjectName(QString::fromUtf8("nameEdit"));
    nameEdit->setGeometry(QRect(110, 37, 121, 23));
    smoGroup = new QGroupBox(MonitorDialog);
    smoGroup->setObjectName(QString::fromUtf8("smoGroup"));
    smoGroup->setGeometry(QRect(250, 30, 191, 191));
    smoGroup->setFocusPolicy(Qt::TabFocus);
    smoGroup->setCheckable(true);
    smoGroup->setChecked(false);
    boardCombo = new QComboBox(smoGroup);
    boardCombo->setObjectName(QString::fromUtf8("boardCombo"));
    boardCombo->setGeometry(QRect(101, 30, 71, 22));
    smoLeftMarginEdit = new QLineEdit(smoGroup);
    smoLeftMarginEdit->setObjectName(QString::fromUtf8("smoLeftMarginEdit"));
    smoLeftMarginEdit->setGeometry(QRect(101, 70, 71, 20));
    smoTopMarginEdit = new QLineEdit(smoGroup);
    smoTopMarginEdit->setObjectName(QString::fromUtf8("smoTopMarginEdit"));
    smoTopMarginEdit->setGeometry(QRect(101, 110, 71, 20));
    smoSpacingEdit = new QLineEdit(smoGroup);
    smoSpacingEdit->setObjectName(QString::fromUtf8("smoSpacingEdit"));
    smoSpacingEdit->setGeometry(QRect(101, 150, 71, 20));
    label_6 = new QLabel(smoGroup);
    label_6->setObjectName(QString::fromUtf8("label_6"));
    label_6->setGeometry(QRect(20, 70, 70, 17));
    label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_7 = new QLabel(smoGroup);
    label_7->setObjectName(QString::fromUtf8("label_7"));
    label_7->setGeometry(QRect(20, 110, 70, 21));
    label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_8 = new QLabel(smoGroup);
    label_8->setObjectName(QString::fromUtf8("label_8"));
    label_8->setGeometry(QRect(20, 150, 70, 21));
    label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_5 = new QLabel(smoGroup);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setGeometry(QRect(20, 30, 70, 21));
    label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    buttonBox = new QDialogButtonBox(MonitorDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(270, 240, 171, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    QWidget::setTabOrder(nameEdit, decimationCombo);
    QWidget::setTabOrder(decimationCombo, rowsEdit);
    QWidget::setTabOrder(rowsEdit, colsEdit);
    QWidget::setTabOrder(colsEdit, smoGroup);
    QWidget::setTabOrder(smoGroup, boardCombo);
    QWidget::setTabOrder(boardCombo, smoLeftMarginEdit);
    QWidget::setTabOrder(smoLeftMarginEdit, smoTopMarginEdit);
    QWidget::setTabOrder(smoTopMarginEdit, smoSpacingEdit);
    QWidget::setTabOrder(smoSpacingEdit, buttonBox);

    retranslateUi(MonitorDialog);

    QSize size(470, 279);
    size = size.expandedTo(MonitorDialog->minimumSizeHint());
    MonitorDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), MonitorDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), MonitorDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(MonitorDialog);
    } // setupUi

    void retranslateUi(QDialog *MonitorDialog)
    {
    MonitorDialog->setWindowTitle(QApplication::translate("MonitorDialog", "Monitor Settings", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("MonitorDialog", "Resolution decimation:", 0, QApplication::UnicodeUTF8));
    label_1->setText(QApplication::translate("MonitorDialog", "Monitor name:", 0, QApplication::UnicodeUTF8));
    uiChkBoxEnableRawAudio->setText(QApplication::translate("MonitorDialog", "Enable Raw Audio", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("MonitorDialog", "Columns [1-8]:", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("MonitorDialog", "Rows [1-8]:", 0, QApplication::UnicodeUTF8));
    smoGroup->setTitle(QApplication::translate("MonitorDialog", "Spot monitor", 0, QApplication::UnicodeUTF8));
    label_6->setText(QApplication::translate("MonitorDialog", "Left margin:", 0, QApplication::UnicodeUTF8));
    label_7->setText(QApplication::translate("MonitorDialog", "Top margin:", 0, QApplication::UnicodeUTF8));
    label_8->setText(QApplication::translate("MonitorDialog", "Spacing:", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("MonitorDialog", "Board:", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MonitorDialog);
    } // retranslateUi

};

namespace Ui {
    class MonitorDialog: public Ui_MonitorDialog {};
} // namespace Ui

#endif // UI_MONITORDIALOG_H
