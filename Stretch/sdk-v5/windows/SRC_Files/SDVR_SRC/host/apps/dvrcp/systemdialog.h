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

#ifndef SYSTEMDIALOG_H
#define SYSTEMDIALOG_H

#include "ui_systemdialog.h"
#include "ui_startupdialog.h"
#include "sdvr_sdk.h"


class DVRSystem;


class StartupDialog : public QDialog, protected Ui::StartupDialog
{
    Q_OBJECT

public:
    StartupDialog(DVRSystem *system, QWidget *parent = 0);

    bool doAccept();

private slots:
    void browseStartupFirmware();
    void browseSystemFolder();
    virtual void accept();

private:
    bool validateSystemFolder(QString qstrSystemFolder);
    bool validateFirmwarePath(QString fwPath);

protected:
    DVRSystem *m_system;
    bool m_bIsEditSystem;   // A flag to indicate whether we are 
                            // creating a new system or editing it.
};



//
/////////////////////////////// System Dialog ///////////////////
///
class SystemDialog : public QDialog, protected Ui::SystemDialog
{
    Q_OBJECT

public:
    SystemDialog(DVRSystem *system, QWidget *parent = 0);

    sdvr_video_std_e videoStandard() const;
    sdvr_audio_rate_e getAudioRate() const;
    bool isEditSystem() {return m_bIsEditSystem;}
    void setEditSystem(bool editSystem) {m_bIsEditSystem = editSystem;}
    bool doAccept();

private slots:
    void browseFontFile();

protected:
    DVRSystem *m_system;
    bool m_bIsEditSystem;   // A flag to indicate whether we are 
                            // creating a new system or editing it.
};


class SystemNewDialog : public SystemDialog
{
    Q_OBJECT

public:
    SystemNewDialog(DVRSystem *system, QWidget *parent = 0);

private slots:
    void browse();
    virtual void accept();

};


class SystemEditDialog : public SystemDialog
{
    Q_OBJECT

public:
    SystemEditDialog(DVRSystem *system, QWidget *parent = 0);

private slots:
    virtual void accept();
};


#endif
