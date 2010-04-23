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
#include "dvrboard.h"
#include "sensorsdialog.h"
#include "dvrsystem.h"
#include "dvrboard.h"
#include "sdvr_sdk.h"


extern DVRSystem *g_DVR;

static const char *SENSOR_EDGE_TRIGGER_TXT = "Edge Trigger";
static const char *SENSOR_LEVEL_SENSETIVE_TXT = "Level Sensitive";
#define SENSOR_EDGE_TRIGGER     0
#define SENSOR_LEVEL_SENSETIVE  1


SensorsCnfgDialog::SensorsCnfgDialog(uint board_index, QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    m_boardIndex = board_index ;
    m_bEnableFirstSensorBoard = false;
    m_bEnableSecondSensorBoard = false;

    connect(uiBtnEnableAll, SIGNAL(clicked()), SLOT(onEnableAllClicked()));
    connect(uiBtnDisableAll, SIGNAL(clicked()), SLOT(onDisableAllClicked()));

    connect(uiBtnEdgeTriggerAll, SIGNAL(clicked()), SLOT(onEdgeTriggerAllClicked()));
    connect(uiBtnLevelSensitiveAll, SIGNAL(clicked()), SLOT(onLevelSensitiveAllClicked()));

    m_board = g_DVR->board(m_boardIndex);

    // Set the initial values for all the controls in all the tabs
    // As well as populating the combobox list data
    initAllControls();
}

void SensorsCnfgDialog::initAllControls()
{
    sx_uint32 sensor_enable_map = 0;
    sx_uint32 edge_triggered_map = 0;

    int sensorsCount;

    if (m_board != NULL)
    {
        sensorsCount = m_board->sensorsCount();

        // The system currently handles 3 different sensor configuarions:
        // A system with no sensor card connected. sensorsCount = 0
        // A system with 1 sensor card connected.  sensorsCount = 8
        // A system with 2 sensor card connected. sensorsCount = 16

        if (sensorsCount != 0)
        {
//            sensor_enable_map = m_board->sensorEnableMap();
//            edge_triggered_map = m_board->edgeTriggeredMap();
            sdvr_get_sensors(m_boardIndex, &sensor_enable_map, &edge_triggered_map);

            // At the stage we have at least on Sensor board connected.
            m_bEnableFirstSensorBoard = true;

            // Enable the Sensors on the second card
            if (sensorsCount > 8)
            {
                m_bEnableSecondSensorBoard  = true;
            }
        }
    } //if (board != NULL)

    // Enable the controls
    uiChkEnableSensor1->setEnabled(m_bEnableFirstSensorBoard);
    uiChkEnableSensor2->setEnabled(m_bEnableFirstSensorBoard);
    uiChkEnableSensor3->setEnabled(m_bEnableFirstSensorBoard);
    uiChkEnableSensor4->setEnabled(m_bEnableFirstSensorBoard);
    uiChkEnableSensor5->setEnabled(m_bEnableFirstSensorBoard);
    uiChkEnableSensor6->setEnabled(m_bEnableFirstSensorBoard);
    uiChkEnableSensor7->setEnabled(m_bEnableFirstSensorBoard);
    uiChkEnableSensor8->setEnabled(m_bEnableFirstSensorBoard);
    uiChkEnableSensor09->setEnabled(m_bEnableSecondSensorBoard);
    uiChkEnableSensor10->setEnabled(m_bEnableSecondSensorBoard);
    uiChkEnableSensor11->setEnabled(m_bEnableSecondSensorBoard);
    uiChkEnableSensor12->setEnabled(m_bEnableSecondSensorBoard);
    uiChkEnableSensor13->setEnabled(m_bEnableSecondSensorBoard);
    uiChkEnableSensor14->setEnabled(m_bEnableSecondSensorBoard);
    uiChkEnableSensor15->setEnabled(m_bEnableSecondSensorBoard);
    uiChkEnableSensor16->setEnabled(m_bEnableSecondSensorBoard);
    uiComboSensor1Trigger->setEnabled(m_bEnableFirstSensorBoard);
    uiComboSensor2Trigger->setEnabled(m_bEnableFirstSensorBoard);
    uiComboSensor3Trigger->setEnabled(m_bEnableFirstSensorBoard);
    uiComboSensor4Trigger->setEnabled(m_bEnableFirstSensorBoard);
    uiComboSensor5Trigger->setEnabled(m_bEnableFirstSensorBoard);
    uiComboSensor6Trigger->setEnabled(m_bEnableFirstSensorBoard);
    uiComboSensor7Trigger->setEnabled(m_bEnableFirstSensorBoard);
    uiComboSensor8Trigger->setEnabled(m_bEnableFirstSensorBoard);
    uiComboSensor09Trigger->setEnabled(m_bEnableSecondSensorBoard);
    uiComboSensor10Trigger->setEnabled(m_bEnableSecondSensorBoard);
    uiComboSensor11Trigger->setEnabled(m_bEnableSecondSensorBoard);
    uiComboSensor12Trigger->setEnabled(m_bEnableSecondSensorBoard);
    uiComboSensor13Trigger->setEnabled(m_bEnableSecondSensorBoard);
    uiComboSensor14Trigger->setEnabled(m_bEnableSecondSensorBoard);
    uiComboSensor15Trigger->setEnabled(m_bEnableSecondSensorBoard);
    uiComboSensor16Trigger->setEnabled(m_bEnableSecondSensorBoard);

    uiComboSensor1Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor1Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor2Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor2Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor3Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor3Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor4Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor4Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor5Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor5Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor6Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor6Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor7Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor7Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor8Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor8Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));

    uiComboSensor09Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor09Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor10Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor10Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor11Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor11Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor12Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor12Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor13Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor13Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor14Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor14Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor15Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor15Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));
    uiComboSensor16Trigger->addItem(SENSOR_EDGE_TRIGGER_TXT, QVariant(SENSOR_EDGE_TRIGGER));
    uiComboSensor16Trigger->addItem(SENSOR_LEVEL_SENSETIVE_TXT, QVariant(SENSOR_LEVEL_SENSETIVE));

    ////////////////////////////////////////////////////////////////////////
    // Initialize all the control
    uiChkEnableSensor1->setChecked(m_bEnableFirstSensorBoard   && (sensor_enable_map & 0x1));
    uiChkEnableSensor2->setChecked(m_bEnableFirstSensorBoard   && (sensor_enable_map & 0x2));
    uiChkEnableSensor3->setChecked(m_bEnableFirstSensorBoard   && (sensor_enable_map & 0x4));
    uiChkEnableSensor4->setChecked(m_bEnableFirstSensorBoard   && (sensor_enable_map & 0x8));
    uiChkEnableSensor5->setChecked(m_bEnableFirstSensorBoard   && (sensor_enable_map & 0x10));
    uiChkEnableSensor6->setChecked(m_bEnableFirstSensorBoard   && (sensor_enable_map & 0x20));
    uiChkEnableSensor7->setChecked(m_bEnableFirstSensorBoard   && (sensor_enable_map & 0x40));
    uiChkEnableSensor8->setChecked(m_bEnableFirstSensorBoard   && (sensor_enable_map & 0x80));
    uiChkEnableSensor09->setChecked(m_bEnableSecondSensorBoard && (sensor_enable_map & 0x100));
    uiChkEnableSensor10->setChecked(m_bEnableSecondSensorBoard && (sensor_enable_map & 0x200));
    uiChkEnableSensor11->setChecked(m_bEnableSecondSensorBoard && (sensor_enable_map & 0x400));
    uiChkEnableSensor12->setChecked(m_bEnableSecondSensorBoard && (sensor_enable_map & 0x800));
    uiChkEnableSensor13->setChecked(m_bEnableSecondSensorBoard && (sensor_enable_map & 0x1000));
    uiChkEnableSensor14->setChecked(m_bEnableSecondSensorBoard && (sensor_enable_map & 0x2000));
    uiChkEnableSensor15->setChecked(m_bEnableSecondSensorBoard && (sensor_enable_map & 0x4000));
    uiChkEnableSensor16->setChecked(m_bEnableSecondSensorBoard && (sensor_enable_map & 0x8000));

    uiComboSensor1Trigger->setCurrentIndex(
        (edge_triggered_map & 0x1) == 0x1 ? uiComboSensor1Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor1Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));
    uiComboSensor2Trigger->setCurrentIndex(
        (edge_triggered_map & 0x2) == 0x2 ? uiComboSensor2Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor2Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));
    uiComboSensor3Trigger->setCurrentIndex(
        (edge_triggered_map & 0x4) == 0x4 ? uiComboSensor3Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor3Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));
    uiComboSensor4Trigger->setCurrentIndex(
        (edge_triggered_map & 0x8) == 0x8 ? uiComboSensor4Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor4Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));
    uiComboSensor5Trigger->setCurrentIndex(
        (edge_triggered_map & 0x10) == 0x10 ? uiComboSensor5Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor5Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));
    uiComboSensor6Trigger->setCurrentIndex(
        (edge_triggered_map & 0x20) == 0x20 ? uiComboSensor6Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor6Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));
    uiComboSensor7Trigger->setCurrentIndex(
        (edge_triggered_map & 0x40) == 0x40 ? uiComboSensor7Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor7Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));
    uiComboSensor8Trigger->setCurrentIndex(
        (edge_triggered_map & 0x80) == 0x80 ? uiComboSensor8Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor8Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));

    uiComboSensor09Trigger->setCurrentIndex(
        (edge_triggered_map & 0x100) == 0x100 ? uiComboSensor09Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor09Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));

    uiComboSensor10Trigger->setCurrentIndex(
        (edge_triggered_map & 0x200) == 0x200 ? uiComboSensor10Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor10Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));

    uiComboSensor11Trigger->setCurrentIndex(
        (edge_triggered_map & 0x400) == 0x400 ? uiComboSensor11Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor11Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));

    uiComboSensor12Trigger->setCurrentIndex(
        (edge_triggered_map & 0x800) == 0x800 ? uiComboSensor12Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor12Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));

    uiComboSensor13Trigger->setCurrentIndex(
        (edge_triggered_map & 0x1000) == 0x1000 ? uiComboSensor13Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor13Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));

    uiComboSensor14Trigger->setCurrentIndex(
        (edge_triggered_map & 0x2000) == 0x2000 ? uiComboSensor14Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor13Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));

    uiComboSensor15Trigger->setCurrentIndex(
        (edge_triggered_map & 0x4000) == 0x4000 ? uiComboSensor15Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor15Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));

    uiComboSensor16Trigger->setCurrentIndex(
        (edge_triggered_map & 0x8000) == 0x8000 ? uiComboSensor16Trigger->findData(QVariant(SENSOR_EDGE_TRIGGER)) :
        uiComboSensor16Trigger->findData(QVariant(SENSOR_LEVEL_SENSETIVE)));
}

void SensorsCnfgDialog::accept()
{
    uint sensor_enable_map = 0;
    uint edge_triggered_map = 0;

    // Get the Sensor setting on the Sensor board 1
    if (m_bEnableFirstSensorBoard)
    {
        if (uiChkEnableSensor1->isChecked())
        {
            sensor_enable_map |= 1;

            if (uiComboSensor1Trigger->itemData(uiComboSensor1Trigger->currentIndex()).toInt()
                == SENSOR_EDGE_TRIGGER)
                edge_triggered_map |= 1;
        }
        if (uiChkEnableSensor2->isChecked())
        {
            sensor_enable_map |= 2;

            if (uiComboSensor2Trigger->itemData(uiComboSensor2Trigger->currentIndex()).toInt()
                == SENSOR_EDGE_TRIGGER)
                edge_triggered_map |= 2;
        }
        if (uiChkEnableSensor3->isChecked())
        {
            sensor_enable_map |= 4;

            if (uiComboSensor3Trigger->itemData(uiComboSensor3Trigger->currentIndex()).toInt()
                == SENSOR_EDGE_TRIGGER)
                edge_triggered_map |= 4;
        }
        if (uiChkEnableSensor4->isChecked())
        {
            sensor_enable_map |= 8;

            if (uiComboSensor4Trigger->itemData(uiComboSensor4Trigger->currentIndex()).toInt()
                == SENSOR_EDGE_TRIGGER)
                edge_triggered_map |= 8;
        }
        if (uiChkEnableSensor5->isChecked())
        {
            sensor_enable_map |= 0x10;

            if (uiComboSensor5Trigger->itemData(uiComboSensor5Trigger->currentIndex()).toInt()
                == SENSOR_EDGE_TRIGGER)
                edge_triggered_map |= 0x10;
        }
        if (uiChkEnableSensor6->isChecked())
        {
            sensor_enable_map |= 0x20;

            if (uiComboSensor6Trigger->itemData(uiComboSensor6Trigger->currentIndex()).toInt()
                == SENSOR_EDGE_TRIGGER)
                edge_triggered_map |= 0x20;
        }
        if (uiChkEnableSensor7->isChecked())
        {
            sensor_enable_map |= 0x40;

            if (uiComboSensor7Trigger->itemData(uiComboSensor7Trigger->currentIndex()).toInt()
                == SENSOR_EDGE_TRIGGER)
                edge_triggered_map |= 0x40;
        }
        if (uiChkEnableSensor8->isChecked())
        {
            sensor_enable_map |= 0x80;

            if (uiComboSensor8Trigger->itemData(uiComboSensor8Trigger->currentIndex()).toInt()
                == SENSOR_EDGE_TRIGGER)
                edge_triggered_map |= 0x80;
        }

        // Get the Sensor setting on the Sensor board 2
        if (m_bEnableSecondSensorBoard)
        {
            if (uiChkEnableSensor09->isChecked())
            {
                sensor_enable_map |= 0x100;

                if (uiComboSensor09Trigger->itemData(uiComboSensor09Trigger->currentIndex()).toInt()
                    == SENSOR_EDGE_TRIGGER)
                    edge_triggered_map |= 0x100;
            }
            if (uiChkEnableSensor10->isChecked())
            {
                sensor_enable_map |= 0x200;

                if (uiComboSensor10Trigger->itemData(uiComboSensor10Trigger->currentIndex()).toInt()
                    == SENSOR_EDGE_TRIGGER)
                    edge_triggered_map |= 0x200;
            }
            if (uiChkEnableSensor11->isChecked())
            {
                sensor_enable_map |= 0x400;

                if (uiComboSensor11Trigger->itemData(uiComboSensor11Trigger->currentIndex()).toInt()
                    == SENSOR_EDGE_TRIGGER)
                    edge_triggered_map |= 0x400;
            }
            if (uiChkEnableSensor12->isChecked())
            {
                sensor_enable_map |= 0x800;

                if (uiComboSensor12Trigger->itemData(uiComboSensor12Trigger->currentIndex()).toInt()
                    == SENSOR_EDGE_TRIGGER)
                    edge_triggered_map |= 0x800;
            }
            if (uiChkEnableSensor13->isChecked())
            {
                sensor_enable_map |= 0x1000;

                if (uiComboSensor13Trigger->itemData(uiComboSensor13Trigger->currentIndex()).toInt()
                    == SENSOR_EDGE_TRIGGER)
                    edge_triggered_map |= 0x1000;
            }
            if (uiChkEnableSensor14->isChecked())
            {
                sensor_enable_map |= 0x2000;

                if (uiComboSensor14Trigger->itemData(uiComboSensor14Trigger->currentIndex()).toInt()
                    == SENSOR_EDGE_TRIGGER)
                    edge_triggered_map |= 0x2000;
            }
            if (uiChkEnableSensor15->isChecked())
            {
                sensor_enable_map |= 0x4000;

                if (uiComboSensor15Trigger->itemData(uiComboSensor15Trigger->currentIndex()).toInt()
                    == SENSOR_EDGE_TRIGGER)
                    edge_triggered_map |= 0x4000;
            }
            if (uiChkEnableSensor16->isChecked())
            {
                sensor_enable_map |= 0x8000;

                if (uiComboSensor16Trigger->itemData(uiComboSensor16Trigger->currentIndex()).toInt()
                    == SENSOR_EDGE_TRIGGER)
                    edge_triggered_map |= 0x8000;
            }
        }
        m_board->setSensorConfig(sensor_enable_map,edge_triggered_map);
        sdvr_config_sensors(m_boardIndex,
                            sensor_enable_map,
                            edge_triggered_map);
    }
    QDialog::accept();
}

void SensorsCnfgDialog::onEnableAllClicked()
{
    // Enable all the sensors that are available

    uiChkEnableSensor1->setChecked(m_bEnableFirstSensorBoard);
    uiChkEnableSensor2->setChecked(m_bEnableFirstSensorBoard);
    uiChkEnableSensor3->setChecked(m_bEnableFirstSensorBoard);
    uiChkEnableSensor4->setChecked(m_bEnableFirstSensorBoard);
    uiChkEnableSensor5->setChecked(m_bEnableFirstSensorBoard);
    uiChkEnableSensor6->setChecked(m_bEnableFirstSensorBoard);
    uiChkEnableSensor7->setChecked(m_bEnableFirstSensorBoard);
    uiChkEnableSensor8->setChecked(m_bEnableFirstSensorBoard);
    uiChkEnableSensor09->setChecked(m_bEnableSecondSensorBoard);
    uiChkEnableSensor10->setChecked(m_bEnableSecondSensorBoard);
    uiChkEnableSensor11->setChecked(m_bEnableSecondSensorBoard);
    uiChkEnableSensor12->setChecked(m_bEnableSecondSensorBoard);
    uiChkEnableSensor13->setChecked(m_bEnableSecondSensorBoard);
    uiChkEnableSensor14->setChecked(m_bEnableSecondSensorBoard);
    uiChkEnableSensor15->setChecked(m_bEnableSecondSensorBoard);
    uiChkEnableSensor16->setChecked(m_bEnableSecondSensorBoard);

}
void SensorsCnfgDialog::onDisableAllClicked()
{
    uiChkEnableSensor1->setChecked(false);
    uiChkEnableSensor2->setChecked(false);
    uiChkEnableSensor3->setChecked(false);
    uiChkEnableSensor4->setChecked(false);
    uiChkEnableSensor5->setChecked(false);
    uiChkEnableSensor6->setChecked(false);
    uiChkEnableSensor7->setChecked(false);
    uiChkEnableSensor8->setChecked(false);
    uiChkEnableSensor09->setChecked(false);
    uiChkEnableSensor10->setChecked(false);
    uiChkEnableSensor11->setChecked(false);
    uiChkEnableSensor12->setChecked(false);
    uiChkEnableSensor13->setChecked(false);
    uiChkEnableSensor14->setChecked(false);
    uiChkEnableSensor15->setChecked(false);
    uiChkEnableSensor16->setChecked(false);

}
void SensorsCnfgDialog::onEdgeTriggerAllClicked()
{
    uiComboSensor1Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor2Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor3Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor4Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor5Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor6Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor7Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor8Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);

    uiComboSensor09Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor10Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor11Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor12Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor13Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor14Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor15Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);
    uiComboSensor16Trigger->setCurrentIndex(SENSOR_EDGE_TRIGGER);

}
void SensorsCnfgDialog::onLevelSensitiveAllClicked()
{
    uiComboSensor1Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor2Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor3Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor4Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor5Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor6Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor7Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor8Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);

    uiComboSensor09Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor10Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor11Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor12Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor13Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor14Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor15Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);
    uiComboSensor16Trigger->setCurrentIndex(SENSOR_LEVEL_SENSETIVE);

}
