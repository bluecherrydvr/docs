/********************************************************************************
** Form generated from reading ui file 'regionsdialog.ui'
**
** Created: Fri Apr 10 16:32:11 2009
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_REGIONSDIALOG_H
#define UI_REGIONSDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>

class Ui_RegionsDialog
{
public:
    QTableWidget *uiTableRegions;
    QPushButton *uiBtnRemoveRegion;
    QPushButton *uiBtnAddRegion;
    QDialogButtonBox *buttonBox;
    QLabel *uiLabelRegions;

    void setupUi(QDialog *RegionsDialog)
    {
    RegionsDialog->setObjectName(QString::fromUtf8("RegionsDialog"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(RegionsDialog->sizePolicy().hasHeightForWidth());
    RegionsDialog->setSizePolicy(sizePolicy);
    RegionsDialog->setMinimumSize(QSize(494, 275));
    RegionsDialog->setMaximumSize(QSize(494, 275));
    uiTableRegions = new QTableWidget(RegionsDialog);
    uiTableRegions->setObjectName(QString::fromUtf8("uiTableRegions"));
    uiTableRegions->setGeometry(QRect(46, 30, 421, 131));
    uiBtnRemoveRegion = new QPushButton(RegionsDialog);
    uiBtnRemoveRegion->setObjectName(QString::fromUtf8("uiBtnRemoveRegion"));
    uiBtnRemoveRegion->setGeometry(QRect(263, 177, 100, 27));
    uiBtnAddRegion = new QPushButton(RegionsDialog);
    uiBtnAddRegion->setObjectName(QString::fromUtf8("uiBtnAddRegion"));
    uiBtnAddRegion->setGeometry(QRect(144, 177, 100, 27));
    buttonBox = new QDialogButtonBox(RegionsDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(290, 223, 191, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    uiLabelRegions = new QLabel(RegionsDialog);
    uiLabelRegions->setObjectName(QString::fromUtf8("uiLabelRegions"));
    uiLabelRegions->setGeometry(QRect(22, 6, 291, 17));
    QWidget::setTabOrder(uiTableRegions, uiBtnAddRegion);
    QWidget::setTabOrder(uiBtnAddRegion, uiBtnRemoveRegion);
    QWidget::setTabOrder(uiBtnRemoveRegion, buttonBox);

    retranslateUi(RegionsDialog);

    QSize size(494, 275);
    size = size.expandedTo(RegionsDialog->minimumSizeHint());
    RegionsDialog->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), RegionsDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), RegionsDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(RegionsDialog);
    } // setupUi

    void retranslateUi(QDialog *RegionsDialog)
    {
    RegionsDialog->setWindowTitle(QApplication::translate("RegionsDialog", "Region Definition", 0, QApplication::UnicodeUTF8));
    uiBtnRemoveRegion->setText(QApplication::translate("RegionsDialog", "Remove Region", 0, QApplication::UnicodeUTF8));
    uiBtnAddRegion->setText(QApplication::translate("RegionsDialog", "Add Region", 0, QApplication::UnicodeUTF8));
    uiLabelRegions->setText(QApplication::translate("RegionsDialog", "Regions:", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(RegionsDialog);
    } // retranslateUi

};

namespace Ui {
    class RegionsDialog: public Ui_RegionsDialog {};
} // namespace Ui

#endif // UI_REGIONSDIALOG_H
