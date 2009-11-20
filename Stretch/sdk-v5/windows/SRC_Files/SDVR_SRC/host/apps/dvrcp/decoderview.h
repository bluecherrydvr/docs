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

#ifndef DECODEVIEW_H
#define DECODEVIEW_H

#include <QWidget>
#include <QBasicTimer>
#include "dvrsystem.h"

class QTableWidget;

class DecoderView : public QWidget
{
    Q_OBJECT

public:
    DecoderView(DVRSystem *system, QWidget *parent = 0);
    PlayerList selectedDecoders() const;
	void updateTimerInterval(uint seconds);

private slots:
    void update();

signals:
    void itemClicked();

protected:
    virtual void resizeEvent(QResizeEvent * event);
	virtual void timerEvent(QTimerEvent *event);
    virtual void showEvent(QShowEvent * event);
    virtual void hideEvent(QHideEvent * event);
	virtual void contextMenuEvent(QContextMenuEvent * event);

private:
    void resetAllStats();
    void resizeColumns();

    DVRSystem *m_system;
    QTableWidget *m_playersStatTable;

    QBasicTimer m_timer;
};

#endif
