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

#ifndef RECVIEW_H
#define RECVIEW_H

#include <QWidget>
#include <QBasicTimer>
#include <QMutex>
#include "sdvr_sdk.h"

class QTableWidget;
class DVRSystem;
class Camera;

class RecView : public QWidget
{
    Q_OBJECT

public:
	struct ChanStats
	{
		sdvr_chan_handle_t handle;
		uint subChan;
		uint frameCounter;
		uint rawFrameCounter;
		uint bitCounter;
        sx_uint32 rawFrameDropCounter;
        sx_uint32 rawFrameSeqNumber;
        sx_uint32 rawFrameFrameNumber;
        sx_uint32 rawFrameOutofSeqCounter;
        sx_uint32 rawFrameMissedFrameCounter;
	};

	enum InfoBit {
		REC_STATUS = 1 << 0,
		ENCODED_FPS = 1 << 1,
		BITRATE = 1 << 2,
		RAW_VIDEO_FPS = 1 << 3
	};

	RecView(DVRSystem *system, QWidget *parent = 0);
    QList<Camera *> selectedCameras() const;
	void updateTimerInterval(uint seconds);
	void selectInfo(uint info);
	uint selectedInfo();

	void collectStats(sdvr_chan_handle_t handle, sdvr_av_buffer_t *av_buffer);
	void collectStats(sdvr_chan_handle_t handle, sdvr_yuv_buffer_t *yuv_buffer);

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
	RecView::ChanStats &getChanStats(sdvr_chan_handle_t handle, uint subChan);

	void resizeColumns();

    DVRSystem *m_system;
    QTableWidget *m_statTable;

    QBasicTimer m_timer;

    QList<ChanStats> m_statList;
    QMutex m_statListMutex;
};

#endif
