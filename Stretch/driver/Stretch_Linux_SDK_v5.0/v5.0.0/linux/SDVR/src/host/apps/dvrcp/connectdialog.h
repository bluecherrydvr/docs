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

#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include "camera.h"
#include "ui_connectdialog.h"
#include "sdvr_sdk.h"


class DVRSystem;


class ConnectDialog : public QDialog, private Ui::ConnectDialog
{
    Q_OBJECT

public:
    ConnectDialog(__sdvr_chan_type_e chanType,  int chanId, DVRSystem *system,QWidget *parent = 0);

    int boardIndex();
    int channel();
    void setVideoFormat(uint subEnc, sdvr_venc_e format) {m_videoFormat[subEnc] = format; }
    sdvr_venc_e videoFormat(uint subEnc) const { return m_videoFormat[subEnc];}

    void setAudioFormat(sdvr_aenc_e format) { m_audioFormat = format; }
    sdvr_aenc_e audioFormat() const { return m_audioFormat; }

private slots:
	void onBoardSelected(int index); 
    void onBtnChangeAVEnc();

private:
    __sdvr_chan_type_e m_chanType;
    DVRSystem *m_system;
    sdvr_venc_e m_videoFormat[MAX_SUB_ENCODERS];
    sdvr_aenc_e m_audioFormat;
    Camera *m_camera;


};

#endif /* CONNECTDIALOG_H */
