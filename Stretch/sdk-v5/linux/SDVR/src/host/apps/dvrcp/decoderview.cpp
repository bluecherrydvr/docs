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

#include "decoder.h"
#include "player.h"
#include "decoderview.h"
#include "dvrsystem.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <math.h>

extern DVRSystem *g_DVR;

DecoderView::DecoderView(DVRSystem *system, QWidget *parent)
    : QWidget(parent)
{

    m_system = system;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_playersStatTable = new QTableWidget(this);
    m_playersStatTable->setColumnCount(3);
    m_playersStatTable->setShowGrid(false);
    m_playersStatTable->horizontalHeader()->setClickable(false);
    m_playersStatTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    m_playersStatTable->setHorizontalHeaderLabels(QStringList() << tr("Decoder")
            << tr("Frame rate [FPS]")<< tr(" Bitrate [KbPS] "));
    m_playersStatTable->verticalHeader()->hide();

    m_playersStatTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_playersStatTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    layout->addWidget(m_playersStatTable, 0);

    connect(m_playersStatTable, SIGNAL(itemClicked(QTableWidgetItem *)),
            SIGNAL(itemClicked()));
}

void DecoderView::resizeEvent(QResizeEvent *)
{
    resizeColumns();
}

void DecoderView::showEvent(QShowEvent *)
{
    uint interval = m_system->getStatFPSInterval();

    resetAllStats();
    m_timer.start(interval * 1000, this);
    update();
}

void DecoderView::hideEvent(QHideEvent *)
{
    m_timer.stop();
}


void DecoderView::timerEvent(QTimerEvent *)
{
    update();
}

void DecoderView::contextMenuEvent(QContextMenuEvent * event)
{
    MainWindow *mw = static_cast<MainWindow *>(parent()->parent());
    mw->decoderViewMenu()->exec(event->globalPos());
}

void DecoderView::update()
{
    PlayerList &playerList = m_system->m_playerList;

    int row = 0;
    PlayerList::const_iterator i;
    for (i = playerList.begin(); i != playerList.end(); ++i)
    {
        CPlayer *player = *i;
        if (!player->isChannelOpen())
            continue;

        if (row == m_playersStatTable->rowCount())
        {
            m_playersStatTable->insertRow(row);
            m_playersStatTable->setRowHeight(row, 16);

            m_playersStatTable->setItem(row, 0, new QTableWidgetItem());

            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            m_playersStatTable->setItem(row, 1, item);

            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            m_playersStatTable->setItem(row, 2, item);
        }

        QTableWidgetItem *PlayerNameItem = m_playersStatTable->item(row, 0);
        PlayerNameItem->setText(player->getName());
        PlayerNameItem->setData(Qt::UserRole, player->getId());

        QTableWidgetItem *fpsItem = m_playersStatTable->item(row, 1);
        QTableWidgetItem *bpsItem = m_playersStatTable->item(row, 2);

        if (player->isDecoding())
        {
            uint bytes = player->decoder().statBytes();
            uint frames = player->decoder().statFrames();
            uint missedFrames = player->decoder().statMissedFrames();

			uint interval = m_system->getStatFPSInterval();
            uint fps = (uint) floor((double)frames / interval + 0.5);
            double kbps = (double)(bytes * 8) / (1024 * interval);

            fpsItem->setText(g_DVR->isShowDropFrames() ?
                QString("%1 (%2)").arg(fps).arg(missedFrames) :
                QString::number(fps));

            bpsItem->setText(QString("%1").arg(kbps, 0, 'f', 0));

            player->decoder().resetStats();
        }
        else
        {
            static const QString NA("---");
            fpsItem->setText(NA);
            bpsItem->setText(NA);
        }

        ++row;
    }

    while (row < m_playersStatTable->rowCount())
        m_playersStatTable->removeRow(row);

    if (m_playersStatTable->rowCount() > 0)
        resizeColumns();
}

void DecoderView::resetAllStats()
{
    PlayerList &list = m_system->m_playerList;

    for (PlayerList::iterator i = list.begin(); i != list.end(); ++i)
    {
        (*i)->decoder().resetStats();
    }
}

void DecoderView::resizeColumns()
{
    m_playersStatTable->resizeColumnsToContents();
}

PlayerList DecoderView::selectedDecoders() const
{
    PlayerList playerList;

    QList<QTableWidgetItem *> itemList = m_playersStatTable->selectedItems();
    foreach (QTableWidgetItem *item, itemList)
    {
        if (item->column() == 0)
        {
            CPlayer *player = m_system->findDecoderById(item->data(Qt::UserRole).toInt());

            Q_ASSERT(player != NULL);
            playerList.append(player);
        }
    }

    return playerList;
}

void DecoderView::updateTimerInterval(uint seconds)
{
    if (m_timer.isActive())
        m_timer.stop();

    resetAllStats();
    m_timer.start(seconds, this);
    update();
}

