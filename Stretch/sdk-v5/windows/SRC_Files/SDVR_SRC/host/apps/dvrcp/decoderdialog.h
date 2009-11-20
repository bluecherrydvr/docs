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

#ifndef DECODERDIALOG_H
#define DECODERDIALOG_H

#include "ui_decoderdialog.h"
#include "sdvr_sdk.h"
#include "player.h"


class DVRSystem;


class DecoderDialog : public QDialog, private Ui::DecoderDialog
{
    Q_OBJECT

public:
    DecoderDialog(CPlayer *player, DVRSystem *system, QWidget *parent = 0);

    // Call this routine before showing the dialogbox to
    // initialize all the controls.
    bool initDialog();

    bool isNewPlayer() {return m_bIsNewPlayer;}
    void setNewPlayer(bool bIsNewPlayer) {m_bIsNewPlayer = bIsNewPlayer;}

    void setDecoderName(QString name) {m_playerName = name;}
    QString getDecoderName() {return m_playerName;}

    void setBoardIndex(int nBoardId) {m_nBoardId = nBoardId;}
    int getBoardIndex() {return m_nBoardId;}

    void setChanNum(int nChanNum) {m_nChanNum = nChanNum;}
    int  getChanNum() {return m_nChanNum;}

    void setDecoderPath(QString path) {m_playerPath = path;}
    QString getDecoderPath() {return m_playerPath;}

    sdvr_venc_e getVidoeFormat() {return m_vFormat;}
    void setVidoeFormat(sdvr_venc_e vFormat) {m_vFormat = vFormat;}

    sdvr_aenc_e getAudioFormat() {return m_aFormat;}
    void setAudioFormat(sdvr_aenc_e aFormat) {m_aFormat = aFormat;}

    sdvr_video_size_e getVideoSize() {return m_vSize;}
    void setVideoSize(sdvr_video_size_e videoSize) {m_vSize = videoSize;}

    void  setConnect(const bool connect) {m_bConnect = connect;}
    bool  getConnect() const {return m_bConnect;}

private slots:
//    void onBrcComboActivated(int index);
//    void onAdjustEncodingOnAlarmChanged(int state);
    virtual void accept();
    void browse();
    void onComboBoardIdSelected(int index);



private:
    static const int ATTACH_GRP_FILENAME     = 1000;
    static const int ATTACH_GRP_CAMERA       = 1001;
    static const int VDECODER_GRP_H264       = 1002;
    static const int VDECODER_GRP_MP4        = 1003;
    static const int VDECODER_GRP_MJPEG      = 1004;
    static const int ADECODER_GRP_NONE       = 1005;
    static const int ADECODER_GRP_G711       = 1006;
    static const int ADECODER_GRP_G726_16K   = 1007;
    static const int ADECODER_GRP_G726_24K   = 1008;
    static const int ADECODER_GRP_G726_32K   = 1009;


private:
    CPlayer  *m_player;                 // The player about to be created or
                                        // change its attribute.
    DVRSystem *m_system;
    bool m_bIsNewPlayer;               // A flag to indicate if we are creating
                                        // a new player or editing an existing one.

    //////////////////////////////////////////////////////
    // Dialog field values
    //////////////////////////////////////////////////////
    QString m_playerName;
    int m_nBoardId;                     // The board Id which this player is create on
    int m_nChanNum;                   // The channel Id for which the player is
                                        // assigned on the board.
    QString m_playerPath;
    bool m_bConnect;            // A flag indicating to connect to 
                                // a player channel.

    sdvr_venc_e m_vFormat;
    sdvr_aenc_e m_aFormat;
    sdvr_video_size_e m_vSize;
    /////////////////////////////////////////////////////

    QButtonGroup m_btnGrpAttach;
    QButtonGroup m_btnGrpAttachVDecoder;
    QButtonGroup m_btnGrpAttachADecoder;

};

#endif
