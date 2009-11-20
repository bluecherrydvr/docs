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

#include "dvrsystem.h"
#include "dvrboard.h"
#include "connectdialog.h"
#include "cameradialog.h"

#include <QList>

extern DVRSystem *g_DVR;

ConnectDialog::ConnectDialog(__sdvr_chan_type_e chanType,  int chanId, DVRSystem *system, QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    m_system = system;
    m_chanType = chanType;
    m_camera = NULL;
    // Initialize board combo. List all boards with disconnected channels

    int boardCount = m_system->boardCount();
    bool bAddBoard;

    for (int index = 0; index < boardCount; ++index)
    {
        switch (m_chanType)
        {
        case SDVR_CHAN_TYPE_DECODER:
            bAddBoard = (m_system->availableBoardDecoders(index).count() > 0);
            uiLabelChan->setText("Decoder Port:");
            uiLabelPrompt->setText(QString("Connect to the Decoder Port:"));

            break;
        case SDVR_CHAN_TYPE_ENCODER:
        {
            m_camera = m_system->findCameraById(chanId);
            bAddBoard = (m_system->availableVideoInputs(index).count() > 0);
            uiLabelPrompt->setText(QString("Connect to the Video-In Port:"));

            uiLabelChan->setText("Video-In Port:");

            if (m_camera)
            {
                setAudioFormat(m_camera->audioFormat());
                setVideoFormat(SDVR_ENC_PRIMARY, m_camera->videoFormat(SDVR_ENC_PRIMARY));
                setVideoFormat(SDVR_ENC_SECONDARY, m_camera->videoFormat(SDVR_ENC_SECONDARY));

            }
            connect(uiBtnChangeAVEnc, SIGNAL(clicked()),
                this, SLOT(onBtnChangeAVEnc()));

        }
        break;
        default:
            bAddBoard = false;
        }
        if (bAddBoard)
            boardCombo->addItem(QString::number(index + 1), QVariant(index));
    }

    // By default we select the first board in the combobox
    if (boardCombo->count() > 0)
    {
        onBoardSelected(0);
        boardCombo->setCurrentIndex(0);
        channelCombo->setCurrentIndex(0);

        connect(boardCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onBoardSelected(int)));
    }

    // Changing of A/V encoder is only allowed if we
    // are connecting cameras 
    uiBtnChangeAVEnc->setVisible(m_camera != NULL);

}

void ConnectDialog::onBtnChangeAVEnc()
{
    Camera origCam(*m_camera, true); // duplicate the original camera 
                                // so that we can change its encoder
                                // information to match what was selected
                                // within this dialogbox.
    origCam.setVideoFormat(SDVR_ENC_PRIMARY,videoFormat(SDVR_ENC_PRIMARY));
    origCam.setVideoFormat(SDVR_ENC_SECONDARY,videoFormat(SDVR_ENC_SECONDARY));
    origCam.setAudioFormat(audioFormat());

    CameraSetupDialog setupDialog(this);
    setupDialog.doInitDialog(&origCam);
    if (setupDialog.exec() == QDialog::Accepted)
    {
        sdvr_venc_e venc;
        sdvr_aenc_e aenc;

        aenc = setupDialog.audioEncoding();
        setAudioFormat(aenc);
        venc = setupDialog.videoEncoder(SDVR_ENC_PRIMARY);
        setVideoFormat(SDVR_ENC_PRIMARY, venc);
        venc = setupDialog.videoEncoder(SDVR_ENC_SECONDARY);
        setVideoFormat(SDVR_ENC_SECONDARY, venc);
    }
}
void ConnectDialog::onBoardSelected(int index)
{
    QVariant boardId = boardCombo->itemData(index);
    QVector<int> chanAvailList = (m_chanType == SDVR_CHAN_TYPE_DECODER) ? 
        m_system->availableBoardDecoders(boardId.toInt()): m_system->availableVideoInputs(boardId.toInt());

    // Changing of the encoder can only be done
    // on DVR boards that support encoding.
    bool bAVEncEnable = false;
    DVRBoard *board;
    board = m_system->board(index);
    if (board)
        bAVEncEnable = (board->encodersCount() != 0);

    uiBtnChangeAVEnc->setEnabled(bAVEncEnable);

    channelCombo->clear();
    foreach (int chan, chanAvailList)
        channelCombo->addItem(QString::number(chan + 1), QVariant(chan));

    channelCombo->setCurrentIndex(0);
}

int ConnectDialog::boardIndex()
{
    return boardCombo->itemData(boardCombo->currentIndex()).toInt();
}

int ConnectDialog::channel()
{
    return channelCombo->itemData(channelCombo->currentIndex()).toInt();
}





