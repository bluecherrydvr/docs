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

#include "dvrsystemtree.h"
#include "dvrsystem.h"
#include "dvrboard.h"
#include "camera.h"
#include "player.h"
#include "mainwindow.h"
#include "spotmonitor.h"

#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>

extern DVRSystem *g_DVR;

DVRSystemTree::DVRSystemTree(QWidget *parent) : QTreeWidget(parent)
{
    //	setColumnCount(2);
    //    setHeaderLabels(QStringList() << tr("Component") << tr("Status"));
    //    header()->setResizeMode(0, QHeaderView::ResizeToContents);
    header()->hide();

    m_allBoards = new QTreeWidgetItem(QStringList("DVR Boards"), ALLBOARDS);
    //	m_allBoards->setFirstColumnSpanned(true);
    addTopLevelItem(m_allBoards);

    m_allCameras = new QTreeWidgetItem(QStringList("Cameras"), ALLCAMERAS);
    addTopLevelItem(m_allCameras);

    m_allPlayers = new QTreeWidgetItem(QStringList("Players"), ALLDECODERS);
    addTopLevelItem(m_allPlayers);
}

void DVRSystemTree::addBoardItem(DVRBoard *board)
{
    BoardItem *boardItem = new BoardItem(board);

    int videoInCount = board->videoInCount();

    QTreeWidgetItem *videoIns = new QTreeWidgetItem(
    		QStringList(QString("Video Inputs (%1)").arg(videoInCount)),
    		BOARDVIDEOINS);

    boardItem->addChild(videoIns);

    for (int i = 0; i < videoInCount; ++i)
        videoIns->addChild(new VideoInputItem(board->index(), i));

    QTreeWidgetItem *audioIns = new QTreeWidgetItem(
    		QStringList(QString("Audio Inputs (%1)").arg(board->audioInCount())),
    		BOARDAUDIOINS);

    boardItem->addChild(audioIns);

    int decoderCount = board->decodersCount();

    QTreeWidgetItem *decoders = new QTreeWidgetItem(
    		QStringList(QString("Decoders (%1)").arg(decoderCount)),
    		BOARDDECODERS);

    boardItem->addChild(decoders);

    for (int i = 0; i < decoderCount; ++i)
        decoders->addChild(new CBoardDecoderItem(board->index(), i));


    QTreeWidgetItem *sensors = new QTreeWidgetItem(
    		QStringList(QString("Sensors (%1)").arg(board->sensorsCount())),
    		BOARDSENSORS);

    boardItem->addChild(sensors);

    QTreeWidgetItem *relays = new QTreeWidgetItem(
    		QStringList(QString("Relays (%1)").arg(board->relaysCount())),
    		BOARDRELAYS);

    boardItem->addChild(relays);

    QTreeWidgetItem *allEncoders = new QTreeWidgetItem(
    		QStringList(QString("Encoders (%1)").arg(board->encodersCount())),
    		ALLENCODERS);

    boardItem->addChild(allEncoders);

    QTreeWidgetItem *allSmos = new QTreeWidgetItem(
    		QStringList(QString("Spot Monitors (%1)").arg(board->smoCount())),
    		ALLSMOS);

    for (int i = 0; i < board->smoCount(); ++i)
    {
        SpotMonitor *smo = g_DVR->spotMonitor(board->index(), i + 1);
        if (smo != NULL)
            allSmos->addChild(new SpotMonitorItem(smo));
    }

    boardItem->addChild(allSmos);

    m_allBoards->addChild(boardItem);
}

void DVRSystemTree::addCameraItem(int cameraId)
{
    m_allCameras->addChild(new CameraItem(cameraId));
}

void DVRSystemTree::removeAllCameraItems()
{
    while (m_allCameras->childCount() != 0)
        delete m_allCameras->child(0);
}

void DVRSystemTree::addDecoderItem(int decoderId)
{
    m_allPlayers->addChild(new CPlayerItem(decoderId));
}

void DVRSystemTree::removeAllDecoderItems()
{
    while (m_allPlayers->childCount() != 0)
        delete m_allPlayers->child(0);
}

void DVRSystemTree::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu *menu = NULL;
    MainWindow *mw = static_cast<MainWindow *>(parent()->parent());
    QTreeWidgetItem *curItem = currentItem();

    switch (curItem->type())
    {
    case SMO:
        menu = mw->smoMenu(
            static_cast<SpotMonitorItem *>(curItem)->spotMonitor());
        break;

    case VIDEOINPUT:
        menu = mw->cameraMenu();
        break;
    case CAMERA:
    case ALLCAMERAS:
        menu = mw->cameraMenu();
        break;
    case BOARDDECODERITEM:
        menu = mw->decoderMenu();
        break;
    case DECODER:
    case ALLDECODERS:
        menu = mw->decoderMenu();
    }

    if (menu != NULL)
        menu->exec(event->globalPos());
}

void DVRSystemTree::videoInputDataChanged()
{
	emit dataChanged(indexFromItem(m_allBoards), indexFromItem(m_allCameras));
}

void DVRSystemTree::boardDecoderDataChanged()
{
	emit dataChanged(indexFromItem(m_allBoards), indexFromItem(m_allPlayers));
}

BoardItem::BoardItem(DVRBoard *board)
    : QTreeWidgetItem(DVRSystemTree::BOARD)
{
    m_board = board;
}

QVariant BoardItem::data(int column, int role) const
{
    if (column == 0 && role == Qt::DisplayRole)
        return QVariant(QObject::tr("Board %n", 0, m_board->index() + 1));

    return QTreeWidgetItem::data(column, role);
}

VideoInputItem::VideoInputItem(int boardIndex, int input)
    : QTreeWidgetItem(DVRSystemTree::VIDEOINPUT)
{
    m_boardIndex = boardIndex;
    m_input = input;
}

QVariant VideoInputItem::data(int column, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (column == 0)
        {
            QString str = QString("#%1").arg(m_input + 1);

            Camera *cam = g_DVR->findCameraByInput(m_boardIndex, m_input);
            if (cam != NULL)
                str.append(" <-- ").append(cam->name());

            return QVariant(str);
        }
//		if (column == 1)
//		{
//			Camera *cam = m_board->connectedCamera(m_channel);
//			return QVariant((cam == NULL) ? "<not connected>" : cam->name());
//		}
	}

    return QTreeWidgetItem::data(column, role);
}

CBoardDecoderItem::CBoardDecoderItem(int boardIndex, int chan_num)
    : QTreeWidgetItem(DVRSystemTree::BOARDDECODERITEM)
{
    m_boardIndex = boardIndex;
    m_chan_num = chan_num;
}

QVariant CBoardDecoderItem::data(int column, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (column == 0)
        {
            QString str = QString("#%1").arg(m_chan_num + 1);

            CPlayer *decoder = g_DVR->findDecoderByChanNum(m_boardIndex, m_chan_num);
            if (decoder != NULL)
                str.append(" <-- ").append(decoder->getName());

            return QVariant(str);
        }
	}

    return QTreeWidgetItem::data(column, role);
}

CameraItem::CameraItem(int cameraId)
    : QTreeWidgetItem(DVRSystemTree::CAMERA)
{
    m_cameraId = cameraId;
}

QVariant CameraItem::data(int column, int role) const
{
    if (column == 0 && role == Qt::DisplayRole)
    {
        Camera *cam = g_DVR->findCameraById(m_cameraId);
        return (cam == NULL) ? "(null)" : cam->name();
    }

    return QTreeWidgetItem::data(column, role);
}

//void CameraItem::setData(int column, int role, const QVariant & value)
//{
//    if (column == 0 && role == Qt::DisplayRole)
//        m_camera->setName(value.toString());
//    else
//        QTreeWidgetItem::setData(column, role, value);
//}


CPlayerItem::CPlayerItem(int decoderId)
    : QTreeWidgetItem(DVRSystemTree::DECODER)
{
    m_decoderId = decoderId;
}

QVariant CPlayerItem::data(int column, int role) const
{
    if (column == 0 && role == Qt::DisplayRole)
    {
        CPlayer *decoder = g_DVR->findDecoderById(m_decoderId);
        return (decoder == NULL) ? "(null)" : decoder->getName();
    }

    return QTreeWidgetItem::data(column, role);
}

SpotMonitorItem::SpotMonitorItem(SpotMonitor *smo)
    : QTreeWidgetItem(DVRSystemTree::SMO)
{
    m_smo = smo;
}

QVariant SpotMonitorItem::data(int column, int role) const
{
    if (column == 0 && role == Qt::DisplayRole)
    {
        return m_smo->name();
    }

    return QTreeWidgetItem::data(column, role);
}
