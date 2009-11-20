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

#include "relaysdialog.h"
#include "dvrsystem.h"
#include "dvrboard.h"
#include "sdvr_sdk.h"


extern DVRSystem *g_DVR;


RelaysDialog::RelaysDialog(uint board_index, QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    m_boardIndex = board_index ;
    m_bEnableFirstRelayBoard = false;
    m_bEnableSecondRelayBoard = false;

    connect(uiBtnApply, SIGNAL(clicked()), SLOT(onApplyClicked()));

    // Set the initial values for all the controls in all the tabs
    // As well as populating the combobox list data
    initAllControls();
}

void RelaysDialog::initAllControls()
{
    DVRBoard *board;

    int relaysCount;
    sx_uint32 relay_status = 0;
    sx_uint32 relay_pos = 1;

    board = g_DVR->board(m_boardIndex);
    if (board != NULL)
    {
        relaysCount = board->relaysCount();

        // The system currently handles 3 different relay configuarions:
        // A system with no relay card connected. RelaysCount = 0
        // A system with 1 relay card connected.  RelaysCount = 8
        // A system with 2 relay card connected. RelaysCount = 16

        if (relaysCount != 0)
        {

            sdvr_get_relays(m_boardIndex, &relay_status);

            // At the stage we have at least one relay board connected.
            m_bEnableFirstRelayBoard = true;

            // Enable the relays on the second card
            m_bEnableSecondRelayBoard = (relaysCount > 8);

            m_bEnableThirdRelayBoard = (relaysCount > 16);
        }
    } //if (board != NULL)

    // Enable the controls
    uiChkRelay1->setEnabled(m_bEnableFirstRelayBoard);
    uiChkRelay2->setEnabled(m_bEnableFirstRelayBoard);
    uiChkRelay3->setEnabled(m_bEnableFirstRelayBoard);
    uiChkRelay4->setEnabled(m_bEnableFirstRelayBoard);
    uiChkRelay5->setEnabled(m_bEnableFirstRelayBoard);
    uiChkRelay6->setEnabled(m_bEnableFirstRelayBoard);
    uiChkRelay7->setEnabled(m_bEnableFirstRelayBoard);
    uiChkRelay8->setEnabled(m_bEnableFirstRelayBoard);
    uiChkRelay09->setEnabled(m_bEnableSecondRelayBoard);
    uiChkRelay10->setEnabled(m_bEnableSecondRelayBoard);
    uiChkRelay11->setEnabled(m_bEnableSecondRelayBoard);
    uiChkRelay12->setEnabled(m_bEnableSecondRelayBoard);
    uiChkRelay13->setEnabled(m_bEnableSecondRelayBoard);
    uiChkRelay14->setEnabled(m_bEnableSecondRelayBoard);
    uiChkRelay15->setEnabled(m_bEnableSecondRelayBoard);
    uiChkRelay16->setEnabled(m_bEnableSecondRelayBoard);

    uiChkRelay17->setVisible(m_bEnableThirdRelayBoard);
    uiChkRelay17->setEnabled(m_bEnableThirdRelayBoard);

    // Set the current relays status
    uiChkRelay1->setChecked((relay_status & (relay_pos << 0)) != 0);
    uiChkRelay2->setChecked((relay_status & (relay_pos << 1)) != 0);
    uiChkRelay3->setChecked((relay_status & (relay_pos << 2)) != 0);
    uiChkRelay4->setChecked((relay_status & (relay_pos << 3)) != 0);
    uiChkRelay5->setChecked((relay_status & (relay_pos << 4)) != 0);
    uiChkRelay6->setChecked((relay_status & (relay_pos << 5)) != 0);
    uiChkRelay7->setChecked((relay_status & (relay_pos << 6)) != 0);
    uiChkRelay8->setChecked((relay_status & (relay_pos << 7)) != 0);
    uiChkRelay09->setChecked((relay_status & (relay_pos << 8)) != 0);
    uiChkRelay10->setChecked((relay_status & (relay_pos << 9)) != 0);
    uiChkRelay11->setChecked((relay_status & (relay_pos << 10)) != 0);
    uiChkRelay12->setChecked((relay_status & (relay_pos << 11)) != 0);
    uiChkRelay13->setChecked((relay_status & (relay_pos << 12)) != 0);
    uiChkRelay14->setChecked((relay_status & (relay_pos << 13)) != 0);
    uiChkRelay15->setChecked((relay_status & (relay_pos << 14)) != 0);
    uiChkRelay16->setChecked((relay_status & (relay_pos << 15)) != 0);

    uiChkRelay17->setChecked((relay_status & (relay_pos << 16)) != 0);
}

void RelaysDialog::onApplyClicked()
{
    // Get the relay setting on the relay board 1
    if (m_bEnableFirstRelayBoard)
    {
        sdvr_trigger_relay(m_boardIndex,0,uiChkRelay1->isChecked());
        sdvr_trigger_relay(m_boardIndex,1,uiChkRelay2->isChecked());
        sdvr_trigger_relay(m_boardIndex,2,uiChkRelay3->isChecked());
        sdvr_trigger_relay(m_boardIndex,3,uiChkRelay4->isChecked());
        sdvr_trigger_relay(m_boardIndex,4,uiChkRelay5->isChecked());
        sdvr_trigger_relay(m_boardIndex,5,uiChkRelay6->isChecked());
        sdvr_trigger_relay(m_boardIndex,6,uiChkRelay7->isChecked());
        sdvr_trigger_relay(m_boardIndex,7,uiChkRelay8->isChecked());

        // Get the relay setting on the relay board 2
        if (m_bEnableSecondRelayBoard)
        {
            sdvr_trigger_relay(m_boardIndex,8,uiChkRelay09->isChecked());
            sdvr_trigger_relay(m_boardIndex,9,uiChkRelay10->isChecked());
            sdvr_trigger_relay(m_boardIndex,10,uiChkRelay11->isChecked());
            sdvr_trigger_relay(m_boardIndex,11,uiChkRelay12->isChecked());
            sdvr_trigger_relay(m_boardIndex,12,uiChkRelay13->isChecked());
            sdvr_trigger_relay(m_boardIndex,13,uiChkRelay14->isChecked());
            sdvr_trigger_relay(m_boardIndex,14,uiChkRelay15->isChecked());
            sdvr_trigger_relay(m_boardIndex,15,uiChkRelay16->isChecked());
        }

        if (m_bEnableThirdRelayBoard)
        {
            sdvr_trigger_relay(m_boardIndex,16,uiChkRelay17->isChecked());
        }
    }
}


