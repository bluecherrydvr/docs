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

#include "decoderdialog.h"
#include "dvrsystem.h"
#include "sdvr.h"
#include <QMessageBox>
#include <QFileDialog>



DecoderDialog::DecoderDialog(CPlayer *player, DVRSystem *system, QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    m_system = system;

    m_player = player;

    ///////////////////////////////////////////////////////////////
    // Setup the grouping of radio control properly
    ///////////////////////////////////////////////////////////////
    m_btnGrpAttach.addButton(uiRadioFileName,ATTACH_GRP_FILENAME);
    m_btnGrpAttach.addButton(uiRadioCamera,ATTACH_GRP_CAMERA);

    m_btnGrpAttachVDecoder.addButton(uiRadioVDecoderH264,VDECODER_GRP_H264);
    m_btnGrpAttachVDecoder.addButton(uiRadioVDecoderMP4,VDECODER_GRP_MP4);
    m_btnGrpAttachVDecoder.addButton(uiRadioVDecoderMJPEG,VDECODER_GRP_MJPEG);

    m_btnGrpAttachADecoder.addButton(uiRadioADecoderNone,ADECODER_GRP_NONE);
    m_btnGrpAttachADecoder.addButton(uiRadioADecoderG711,ADECODER_GRP_G711);
    m_btnGrpAttachADecoder.addButton(uiRadioADecoderG726_16K,ADECODER_GRP_G726_16K);
    m_btnGrpAttachADecoder.addButton(uiRadioADecoderG726_24K,ADECODER_GRP_G726_24K);
    m_btnGrpAttachADecoder.addButton(uiRadioADecoderG726_32K,ADECODER_GRP_G726_32K);

    // Set-up the video size
    uiComboVideoSize->addItem(QString("D1-PAL 720x576")  ,QVariant(SDVR_VIDEO_SIZE_720x576));
    uiComboVideoSize->addItem(QString("D1-NTSC 720x480")  ,QVariant(SDVR_VIDEO_SIZE_720x480));
    uiComboVideoSize->addItem(QString("CIF-PAL 352x288")  ,QVariant(SDVR_VIDEO_SIZE_352x288));
    uiComboVideoSize->addItem(QString("CIF-NTSC 352x240")  ,QVariant(SDVR_VIDEO_SIZE_352x240));
    uiComboVideoSize->addItem(QString("2CIF-PAL 704x288")  ,QVariant(SDVR_VIDEO_SIZE_704x288));
    uiComboVideoSize->addItem(QString("2CIF-NTSC 704x240")  ,QVariant(SDVR_VIDEO_SIZE_704x240));
    uiComboVideoSize->addItem(QString("4CIF-PAL 704x576")  ,QVariant(SDVR_VIDEO_SIZE_704x576));
    uiComboVideoSize->addItem(QString("4CIF-NTSC 704x480")  ,QVariant(SDVR_VIDEO_SIZE_704x480));
    uiComboVideoSize->addItem(QString("QCIF-PAL 176x144")  ,QVariant(SDVR_VIDEO_SIZE_176x144));
    uiComboVideoSize->addItem(QString("QCIF-NTSC 176x112")  ,QVariant(SDVR_VIDEO_SIZE_176x112));

    uiComboVideoSize->addItem(QString("DCIF-NTSC 528x320")  ,QVariant(SDVR_VIDEO_SIZE_528x320));
    uiComboVideoSize->addItem(QString("DCIF-PAL 528x384")  ,QVariant(SDVR_VIDEO_SIZE_528x384));

    // If the player is attached show its board-id and channel
    // number but does not let the user to change it.
    // If it is not attached allow the user to connect it
    // to an available player channel.
    if (m_player->isAttached())
    {
        uiComboBoardId->addItem(QString::number(m_player->getBoardIndex() + 1), QVariant(m_player->getBoardIndex()));
		uiComboDecoderChan->addItem(QString::number(m_player->getChanNum() + 1), QVariant(m_player->getChanNum()));
    	uiComboBoardId->setCurrentIndex(0);
    	uiComboDecoderChan->setCurrentIndex(0);
    }
    else
    {
        // Initialize board combo. List all boards with disconnected input channels
        int boardCount = m_system->boardCount();
        for (int index = 0; index < boardCount; ++index)
    	    if (m_system->availableBoardDecoders(index).count() > 0)
    		    uiComboBoardId->addItem(QString::number(index + 1), QVariant(index));

        if (uiComboBoardId->count() > 0)
        {
    	    onComboBoardIdSelected(0);
    	    uiComboBoardId->setCurrentIndex(0);
    	    uiComboDecoderChan->setCurrentIndex(0);

    	    connect(uiComboBoardId, SIGNAL(currentIndexChanged(int)),
        		    this, SLOT(onComboBoardIdSelected(int)));
        }
    }
    uiGroupBoxConnect->setChecked(m_player->isAttached());
    uiLableBoardId->setEnabled(!m_player->isAttached());
    uiLabelDecoderChan->setEnabled(!m_player->isAttached());
    uiComboBoardId->setEnabled(!m_player->isAttached());
    uiComboDecoderChan->setEnabled(!m_player->isAttached());
    uiGroupBoxConnect->setEnabled(!m_player->isAttached());

    connect(uiBtnBrowse, SIGNAL(clicked()), SLOT(browse()));
}

///////////////////////////////////////////////////////////////
// Initialize controls and enable/disable as needed
///////////////////////////////////////////////////////////////

bool DecoderDialog::initDialog()
{
    uiLEditName->setText(m_playerName);

    uiLEditPath->setText(m_playerPath);

//todo: remove it
//    int videoSizeDefault = uiComboVideoSize->findData(QVariant(m_vSize));
//    uiComboVideoSize->setCurrentIndex(videoSizeDefault);
    uiComboVideoSize->setCurrentIndex(-1);
    if (!m_playerPath.isEmpty())
    {
        sdvr_video_size_e video_size;
        bool bRet = sdvr::getVideoSizeFromFile(&video_size,m_playerPath);
        if (bRet)
        {
            int videoSizeDefault = uiComboVideoSize->findData(QVariant(video_size));
            uiComboVideoSize->setCurrentIndex(videoSizeDefault);
        }
    }

//todo: remove the video size control
    uiComboVideoSize->setEnabled(false);
    uiLabelVideoSize->setEnabled(false);

    // todo: for now the only vide player we support is h.264
    uiRadioVDecoderH264->setChecked(true);

    // TODO: Only support for h.264 player

    // TODO: no audio decoding support

    // Todo: For now we only allow specifying a file path
    uiRadioFileName->setChecked(true);

    return true;
}


void DecoderDialog::browse()
{
    QString startupDir;
    QString curMovieFile = uiLEditPath->text();
    sdvr_video_size_e video_size;

    if (curMovieFile.isEmpty())
        startupDir = m_system->systemDirPath();
    else
        startupDir = QFileInfo(curMovieFile).absolutePath();

    QString movieFile = QFileDialog::getOpenFileName(this,
        tr("Select a movie file to decode"), startupDir,
        tr("*.mov"));

    if (!movieFile.isEmpty())
    {
        uiLEditPath->setText(movieFile);
        // Get the video size from the file attached to this player
        bool bRet = sdvr::getVideoSizeFromFile(&video_size,movieFile);
        if (bRet)
        {
            int videoSizeDefault = uiComboVideoSize->findData(QVariant(video_size));
            uiComboVideoSize->setCurrentIndex(videoSizeDefault);
        }
        else
            uiComboVideoSize->setCurrentIndex(-1);

    }
}

void DecoderDialog::onComboBoardIdSelected(int index)
{
	QVariant data = uiComboBoardId->itemData(index);
	QVector<int> playersList = m_system->availableBoardDecoders(data.toInt());

	uiComboDecoderChan->clear();
	foreach (int player, playersList)
		uiComboDecoderChan->addItem(QString::number(player + 1), QVariant(player));

	uiComboDecoderChan->setCurrentIndex(0);
}

void DecoderDialog::accept()
{
    QString filePath;
    QString qstrName(uiLEditName->text());
    sdvr_video_size_e video_size;

    // Each player must have a name
    if (qstrName.isEmpty())
    {
	    QMessageBox::warning(this, tr("Error"),
            "You must specify a name for the player.");
        return;
    }
    // If we are attaching a file, a file name must be given.
    if (uiRadioFileName->isChecked())
    {
        filePath = uiLEditPath->text();
        if (filePath.isEmpty())
        {
		    QMessageBox::warning(this, tr("Error"),
                "You must specify an file name to attach to the player for decoding.");
            return;
        }
        // Get the video size from the file attached to this player
        bool bRet = sdvr::getVideoSizeFromFile(&video_size,filePath);
        if (bRet)
        {
            int videoSizeDefault = uiComboVideoSize->findData(QVariant(video_size));
            uiComboVideoSize->setCurrentIndex(videoSizeDefault);
        }
        else
        {
		        QMessageBox::warning(this, tr("Error"),
                    "The attached file either does not exist or is invalid.");
                return;
        }
    }
    
    setDecoderName(qstrName);
    // todo: for now the only video format supported is h.264
    setVidoeFormat(SDVR_VIDEO_ENC_H264);
    // todo: for now audio is diabled
    setAudioFormat(SDVR_AUDIO_ENC_NONE);
    setDecoderPath(uiLEditPath->text());
    setVideoSize((sdvr_video_size_e)
        uiComboVideoSize->itemData(uiComboVideoSize->currentIndex()).toInt());
    if (uiGroupBoxConnect->isChecked())
    {
        setConnect(true);
        setBoardIndex(uiComboBoardId->itemData(uiComboBoardId->currentIndex()).toInt());
        setChanNum(uiComboDecoderChan->itemData(uiComboDecoderChan->currentIndex()).toInt());
    }
    else
    {
        setConnect(false);
        setBoardIndex(-1);
        setChanNum(-1);
    }
    m_player->setName(getDecoderName());
    m_player->setVideoFormat(getVidoeFormat());
    m_player->setAudioFormat(getAudioFormat());
    m_player->setVideoSize(getVideoSize());
    m_player->setEncodedFilePath(getDecoderPath());

    QDialog::accept();
}
