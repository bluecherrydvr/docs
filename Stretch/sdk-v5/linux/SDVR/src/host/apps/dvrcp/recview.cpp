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

#include "recview.h"
#include "dvrsystem.h"
#include "camera.h"
#include "mainwindow.h"
#include <math.h>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include "log.h"
#include "sdvr.h"


RecView::RecView(DVRSystem *system, QWidget *parent)
    : QWidget(parent)
{

    m_system = system;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_statTable = new QTableWidget(this);
    m_statTable->setColumnCount(5);
    m_statTable->setShowGrid(false);
    m_statTable->horizontalHeader()->setClickable(false);
    m_statTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    m_statTable->setHorizontalHeaderLabels(QStringList() << tr("Camera")
        << tr("Recording\nStatus") << tr("Encoded\nFrame Rate [FPS]")
        << tr(" Bitrate [KbPS]") << tr("Raw Video\nFrame Rate [FPS]"));
    m_statTable->verticalHeader()->hide();

    m_statTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    layout->addWidget(m_statTable, 0);

    connect(m_statTable, SIGNAL(itemClicked(QTableWidgetItem *)),
            SIGNAL(itemClicked()));
}

void RecView::resizeEvent(QResizeEvent *)
{
    resizeColumns();
}

void RecView::showEvent(QShowEvent *)
{
    uint interval = m_system->getStatFPSInterval();

    m_system->recorder().enableStats(SDVR_ENC_PRIMARY);
    m_system->recorder().enableStats(SDVR_ENC_SECONDARY);

    m_timer.start(interval * 1000, this);
    update();
}

void RecView::hideEvent(QHideEvent *)
{
    m_timer.stop();
    m_system->recorder().disableStats(SDVR_ENC_PRIMARY);
    m_system->recorder().disableStats(SDVR_ENC_SECONDARY);

    m_statListMutex.lock();
    m_statList.clear();
    m_statListMutex.unlock();
}

void RecView::timerEvent(QTimerEvent *)
{
    update();
}

void RecView::contextMenuEvent(QContextMenuEvent * event)
{
    MainWindow *mw = static_cast<MainWindow *>(parent()->parent());
    mw->recViewMenu()->exec(event->globalPos());
}

//void RecView::update()
//{
//    // Showing the FPS is only possible if we are recording
//    // to file not using the SDK recording.
//    if (m_system->isShowFPS() && !m_system->isUseSDKRecording())
//        updateFPS();
//    else
//        updateNoFPS();
//}

void RecView::selectInfo(uint info)
{
	if (info & REC_STATUS)
    	m_statTable->showColumn(1);
	else
    	m_statTable->hideColumn(1);

	if (info & ENCODED_FPS)
    	m_statTable->showColumn(2);
	else
    	m_statTable->hideColumn(2);

	if (info & BITRATE)
    	m_statTable->showColumn(3);
	else
    	m_statTable->hideColumn(3);

	if (info & RAW_VIDEO_FPS)
    	m_statTable->showColumn(4);
	else
    	m_statTable->hideColumn(4);
}

uint RecView::selectedInfo()
{
	uint info = 0;
	if (!m_statTable->isColumnHidden(1))
		info |= REC_STATUS;

	if (!m_statTable->isColumnHidden(2))
		info |= ENCODED_FPS;

	if (!m_statTable->isColumnHidden(3))
		info |= BITRATE;

	if (!m_statTable->isColumnHidden(4))
		info |= RAW_VIDEO_FPS;

	return info;
}

void RecView::update()
{
    Recorder::StatList statListPrimary(m_system->recorder().takeStats(SDVR_ENC_PRIMARY));
    Recorder::StatList statListSecondary(m_system->recorder().takeStats(SDVR_ENC_SECONDARY));

    uint interval = m_system->getStatFPSInterval();
    if (interval == 0)
    	interval = 1; // The minimum interval is 1 sec

    int row = 0;
    CameraList::const_iterator i;
    CameraList &camList = m_system->cameraList;

    for (i = camList.begin(); i != camList.end(); ++i)
    {
        Camera *cam = *i;
        if (!cam->isChannelOpen())
            continue;

        // Find primary encoder statistics for the camera
        Recorder::StatList::const_iterator si;
        const Recorder::stat_t *statPrimary = NULL;
        for (si = statListPrimary.begin(); si != statListPrimary.end(); ++si)
        {
            if (si->camera == cam)
            {
                statPrimary = &(*si);
                break;
            }
        }

        // Find secondary encoder statistics for the camera
        const Recorder::stat_t *statSecondary = NULL;
        for (si = statListSecondary.begin(); si != statListSecondary.end(); ++si)
        {
            if (si->camera == cam)
            {
                statSecondary = &(*si);
                break;
            }
        }

        // Add the statistics for the current camera to the recording table
        if (row == m_statTable->rowCount())
        {
            m_statTable->insertRow(row);
            m_statTable->setRowHeight(row, 16);

            m_statTable->setItem(row, 0, new QTableWidgetItem());

            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            m_statTable->setItem(row, 1, item);

            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            m_statTable->setItem(row, 2, item);

            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            m_statTable->setItem(row, 3, item);

            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            m_statTable->setItem(row, 4, item);
        }

        QTableWidgetItem *cameraNameItem = m_statTable->item(row, 0);
        cameraNameItem->setText(cam->name());
        cameraNameItem->setData(Qt::UserRole, cam->id());

        if (!m_statTable->isColumnHidden(1))
        {
        	if (cam->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_NONE)
        	{
        		m_statTable->item(row, 1)->setText("N/A");
        	}
        	else
        	{
        		QString primStatus("Stopped");
				QString secStatus("Stopped");

				if (statPrimary && (statPrimary->lastTimeStamp
						|| m_system->isUseSDKRecording()))
					primStatus = "Recording";
				else if (cam->isRecording(SDVR_ENC_PRIMARY))
					primStatus = "waiting";

				if (statSecondary && (statSecondary->lastTimeStamp
						|| m_system->isUseSDKRecording()))
					secStatus = "Recording";
				else if (cam->isRecording(SDVR_ENC_SECONDARY))
					secStatus = "waiting";

				m_statTable->item(row, 1)->setText(QString("%1 - %2").
						arg(primStatus).arg(secStatus));
        	}
        }

        if (!m_statTable->isColumnHidden(2))
        {
        	if (cam->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_NONE)
        	{
                m_statTable->item(row, 2)->setText("N/A");
        	}
        	else
			{
            	uint fps;
            	QString primFPS("---");
            	QString secFPS("---");

				if (statPrimary && (statPrimary->lastTimeStamp || m_system->isUseSDKRecording()))
				{
					fps = (uint) floor((double)statPrimary->frames / interval + 0.5);
//					fps = (uint) ((double) (statPrimary->frames
//							+ (interval - 1)) / interval);
					primFPS = QString::number(fps);

					if (m_system->isShowDropFrames())
					{
						// We only show the drop frames for NTSC/PAL at max fps.
						if (cam->frameRate(SDVR_ENC_PRIMARY)
								== sdvr::frameRate(m_system->videoStandard()))
							primFPS += QString("[%1]").arg(
									cam->getDropFrameCount(SDVR_ENC_PRIMARY));
						else
							primFPS += "[??]";
					}
				}
				else if (cam->isRecording(SDVR_ENC_PRIMARY))
					primFPS = "waiting";

				if (statSecondary && (statSecondary->lastTimeStamp ||
						m_system->isUseSDKRecording()))
				{
					fps = (uint) floor((double)statSecondary->frames / interval + 0.5);
//					fps = (uint)((double) (statSecondary->frames +
//							(interval - 1)) / interval);
					secFPS = QString::number(fps);

					if (m_system->isShowDropFrames())
					{
						// We only show the drop frames for NTSC/PAL at max fps.
						if (cam->frameRate(SDVR_ENC_SECONDARY)
								== sdvr::frameRate(m_system->videoStandard()))
							secFPS += QString("[%1]").arg(
									cam->getDropFrameCount(SDVR_ENC_SECONDARY));
						else
							secFPS += "[??]";
					}
				}
				else if (cam->isRecording(SDVR_ENC_SECONDARY))
					secFPS = "waiting";

				m_statTable->item(row, 2)->setText(
						QString("%1/%2").arg(primFPS).arg(secFPS));
			}
        }

        if (!m_statTable->isColumnHidden(3))
        {
        	if (cam->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_NONE)
        	{
                m_statTable->item(row, 3)->setText("N/A");
        	}
        	else
			{
				double kbps;
				QString primBPS("----");
				QString secBPS("----");

				if (statPrimary && (statPrimary->lastTimeStamp
						|| m_system->isUseSDKRecording()))
				{
					kbps = (double)(statPrimary->bytes * 8)	/ (1024 * interval);
					primBPS = QString::number(kbps, 'f', 0);
				}
				else if (cam->isRecording(SDVR_ENC_PRIMARY))
					primBPS = "waiting";

				if (statSecondary && (statSecondary->lastTimeStamp
						|| m_system->isUseSDKRecording()))
				{
					kbps = (double)(statSecondary->bytes * 8) / (1024*interval);
					secBPS = QString::number(kbps, 'f', 0);
				}
				else if (cam->isRecording(SDVR_ENC_SECONDARY))
					secBPS = "waiting";

				m_statTable->item(row, 3)->setText(
						QString("%1 / %2").arg(primBPS).arg(secBPS));
			}
        }

        if (!m_statTable->isColumnHidden(4))
        {
        	if (cam->isRawVideoEnabled())
        	{
                QString qstrFPS;
                sx_uint32 droppedCount = 0;
                sx_uint32 rawFrameOutofSeqCounter = 0;
                sx_uint32 rawFrameMissedFrameCounter = 0;
        		m_statListMutex.lock();

				ChanStats &stats = getChanStats(cam->handle(), 0);
				uint fps = (uint) floor((double) stats.rawFrameCounter
						/ interval + 0.5);
				stats.rawFrameCounter = 0;
                droppedCount = stats.rawFrameDropCounter;
                rawFrameOutofSeqCounter = stats.rawFrameOutofSeqCounter;
                rawFrameMissedFrameCounter = stats.rawFrameMissedFrameCounter;
				m_statListMutex.unlock();

                if (m_system->isShowDropFrames())
                    qstrFPS = QString("%1[%2-%3-%4]").arg(fps).arg(droppedCount).arg(rawFrameOutofSeqCounter).
                    arg(rawFrameMissedFrameCounter);
                else
                    qstrFPS = QString("%1").arg(fps);

				m_statTable->item(row, 4)->setText(qstrFPS);
        	}
        	else
        	{
				m_statTable->item(row, 4)->setText("---");
        	}
        }

        ++row;
    }

    while (row < m_statTable->rowCount())
        m_statTable->removeRow(row);

    if (m_statTable->rowCount() > 0)
    	resizeColumns();
}
/*
void RecView::updateFPS()
{
    Recorder::StatList statListPrimary(m_system->recorder().takeStats(SDVR_ENC_PRIMARY));
    Recorder::StatList statListSecondary(m_system->recorder().takeStats(SDVR_ENC_SECONDARY));

    CameraList &camList = m_system->cameraList;

    int row = 0;
    CameraList::const_iterator i;
    QString qstrPrimaryFPS;
    QString qstrSecondaryFPS;
    QString qstrPrimaryDropFrames;
    QString qstrSecondaryDropFrames;
    QString qstrPrimaryBPS;
    QString qstrSecondaryBPS;
    Recorder::StatList::const_iterator si;
    // The minimum interval to calculate the statistics is 1 seconds
    uint   interval = m_system->getStatFPSInterval() > 0 ? m_system->getStatFPSInterval() : 1;
    uint   fps ;
    double kbps;


    for (i = camList.begin(); i != camList.end(); ++i)
    {
        Camera *cam = *i;
        if (!cam->isChannelOpen() || cam->videoFormat(SDVR_ENC_PRIMARY) == SDVR_VIDEO_ENC_NONE)
            continue;

        // Find primary encoder statistics for the camera
        const Recorder::stat_t *statPrimary = NULL;
        for (si = statListPrimary.begin(); si != statListPrimary.end(); ++si)
        {
            if (si->camera == cam)
            {
                statPrimary = &(*si);
                break;
            }
        }

        // Find secondary encoder statistics for the camera
        const Recorder::stat_t *statSecondary = NULL;
        for (si = statListSecondary.begin(); si != statListSecondary.end(); ++si)
        {
            if (si->camera == cam)
            {
                statSecondary = &(*si);
                break;
            }
        }
        qstrPrimaryFPS   = "waiting";
        qstrSecondaryFPS = "waiting";
        qstrPrimaryBPS   = "waiting";
        qstrSecondaryBPS = "waiting";

        // Add the statistics for the current camera to the recording table
        if (row == m_statTable->rowCount())
        {
            m_statTable->insertRow(row);
            m_statTable->setRowHeight(row, 16);

            m_statTable->setItem(row, 0, new QTableWidgetItem());

            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            m_statTable->setItem(row, 1, item);

            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            m_statTable->setItem(row, 2, item);
        }

        QTableWidgetItem *cameraNameItem = m_statTable->item(row, 0);
        cameraNameItem->setText(cam->name());
        cameraNameItem->setData(Qt::UserRole, cam->id());

        QTableWidgetItem *fpsItem = m_statTable->item(row, 1);
        QTableWidgetItem *bpsItem = m_statTable->item(row, 2);

        if (statPrimary && statPrimary->lastTimeStamp)
        {
            fps  = (uint)((double)(statPrimary->frames + (interval - 1)) / interval);

            kbps = (double)(statPrimary->bytes * 8) / (1024 * interval);
            if (m_system->isShowDropFrames())
            {
                // We only show the drop frames for NTSC/PAL at max fps.
                if (cam->frameRate(SDVR_ENC_PRIMARY) == sdvr::frameRate(m_system->videoStandard()))
                    qstrPrimaryFPS = QString("%1[%2]").arg(fps).arg(cam->getDropFrameCount(SDVR_ENC_PRIMARY));
                else
                    qstrPrimaryFPS = QString("%1[??]").arg(fps);

            }
            else
                qstrPrimaryFPS = QString("%1").arg(fps);

            qstrPrimaryBPS = QString("%1").arg(kbps, 0, 'f', 0);
        }
        else
        {
            if (!cam->isRecording(SDVR_ENC_PRIMARY))
            {
                qstrPrimaryFPS = "---";
                qstrPrimaryBPS = "----";
            }
        }

        interval = m_system->getStatFPSInterval() > 0 ? m_system->getStatFPSInterval() : 1;
    	if (statSecondary)
        {
            fps  = (statSecondary->frames + interval - 1) / interval;
            kbps = (double)(statSecondary->bytes * 8) / (1024 * interval);
            if (m_system->isShowDropFrames())
            {
                // We only show the drop frames for NTSC/PAL at max fps.
                if (cam->frameRate(SDVR_ENC_SECONDARY) == sdvr::frameRate(m_system->videoStandard()))
                    qstrSecondaryFPS = QString("%1[%2]").arg(fps).arg(cam->getDropFrameCount(SDVR_ENC_SECONDARY));
                else
                    qstrSecondaryFPS = QString("%1[??]").arg(fps);

            }
            else
                qstrSecondaryFPS = QString("%1").arg(fps);

            qstrSecondaryBPS = QString("%1").arg(kbps, 0, 'f', 0);
        }
        else
        {
            if (!cam->isRecording(SDVR_ENC_SECONDARY))
            {
                qstrSecondaryFPS = "---";
                qstrSecondaryBPS = "----";
            }

        }
        fpsItem->setText(QString("%1/%2").arg(qstrPrimaryFPS).arg(qstrSecondaryFPS));
        bpsItem->setText(QString("%1/%2").arg(qstrPrimaryBPS).arg(qstrSecondaryBPS));

        ++row;
    }

    while (row < m_statTable->rowCount())
        m_statTable->removeRow(row);

    if (m_statTable->rowCount() > 0)
        resizeColumns();
}
*/
void RecView::resizeColumns()
{
//todo: remove this
#ifdef disable
    m_statTable->resizeColumnToContents(1);
    m_statTable->resizeColumnToContents(2);
    m_statTable->setColumnWidth(0, width() -
        m_statTable->columnWidth(1) - m_statTable->columnWidth(2) - 2);
#endif
    m_statTable->resizeColumnsToContents();

}

CameraList RecView::selectedCameras() const
{
    CameraList camList;

    QList<QTableWidgetItem *> itemList = m_statTable->selectedItems();
    foreach (QTableWidgetItem *item, itemList)
    {
        if (item->column() == 0)
        {
            Camera *cam = m_system->findCameraById(item->data(Qt::UserRole).toInt());

            Q_ASSERT(cam != NULL);
            camList.append(cam);
        }
    }

    return camList;
}

void RecView::updateTimerInterval(uint seconds)
{
    if (m_timer.isActive())
    {
        m_timer.stop();
        m_system->recorder().disableStats(SDVR_ENC_PRIMARY);
        m_system->recorder().disableStats(SDVR_ENC_SECONDARY);

    }
    m_system->recorder().enableStats(SDVR_ENC_PRIMARY);
    m_system->recorder().enableStats(SDVR_ENC_SECONDARY);

    m_timer.start(seconds, this);
    update();
}

RecView::ChanStats &RecView::getChanStats(sdvr_chan_handle_t handle, uint subChan)
{
	QList<ChanStats>::iterator i;
	for (i = m_statList.begin(); i != m_statList.end(); ++i)
	{
		ChanStats &stats = (*i);
		if (stats.handle == handle && stats.subChan == subChan)
			return stats;
	}

	ChanStats stats;
	memset(&stats, 0, sizeof(ChanStats));
	stats.handle = handle;
	stats.subChan = subChan;
	m_statList.append(stats);

	return m_statList.last();
}

void RecView::collectStats(sdvr_chan_handle_t,
		sdvr_av_buffer_t *)
{
}

void RecView::collectStats(sdvr_chan_handle_t handle,
		sdvr_yuv_buffer_t * yuv_buffer)
{
    sx_uint32 rawFrameSeqNumber;
    sx_uint32 rawFrameFrameNumber;

	// Called by the frame callback thread. We have to lock.
	QMutexLocker locker(&m_statListMutex);

	// Return if the viewer is not visible.
	if (!isVisible())
		return;

	ChanStats &stats = getChanStats(handle, 0);
	++stats.rawFrameCounter;
    sdvr_av_buf_sequence( (void *)yuv_buffer, 
                           &rawFrameSeqNumber,
                           &rawFrameFrameNumber, 
                           &stats.rawFrameDropCounter);

    // Skip the first frame Sequence number since the
    // default is zero and the number that comes from the
    // board is also zero. This causes the first frame + 1
    // always to fail.
    if ((stats.rawFrameSeqNumber < rawFrameSeqNumber) &&
        (stats.rawFrameSeqNumber + 1 != rawFrameSeqNumber))
        ++stats.rawFrameOutofSeqCounter;
    stats.rawFrameSeqNumber = rawFrameSeqNumber;

    if ((stats.rawFrameFrameNumber < rawFrameFrameNumber) &&
        (stats.rawFrameFrameNumber + 1 != rawFrameFrameNumber))
        ++stats.rawFrameMissedFrameCounter;
    stats.rawFrameFrameNumber = rawFrameFrameNumber;
}
