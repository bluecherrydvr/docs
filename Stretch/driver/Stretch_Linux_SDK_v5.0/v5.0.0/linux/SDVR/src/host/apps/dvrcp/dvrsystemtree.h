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

#ifndef DVRSYSTEMTREE_H
#define DVRSYSTEMTREE_H

#include <QTreeWidget>

class DVRBoard;
class SpotMonitor;

class DVRSystemTree : public QTreeWidget
{
	Q_OBJECT

public:
	// Tree item types
	static const int ALLBOARDS     = 1000;
	static const int BOARD         = 1001;
	static const int BOARDVIDEOINS = 1002;
	static const int BOARDAUDIOINS = 1003;
	static const int BOARDSENSORS  = 1004;
	static const int BOARDRELAYS   = 1005;
	static const int SMO           = 1006;
	static const int VIDEOINPUT    = 1007;
	static const int AUDIOINPUT    = 1008;
	static const int BOARDDECODERS = 1009;
    static const int BOARDDECODERITEM = 1010;

	static const int ENABLEDDECODERS = 1011;
	static const int SENSOR        = 1012;
	static const int RELAY         = 1013;

	static const int ALLCAMERAS    = 1014;
	static const int CAMERA        = 1015;
    static const int ALLDECODERS   = 1016;
    static const int DECODER       = 1017;
    static const int ALLENCODERS   = 1018;
    static const int ALLSMOS       = 1019;

	DVRSystemTree(QWidget * parent = 0);

	void addBoardItem(DVRBoard *board);
	void addCameraItem(int cameraId);
    void addDecoderItem(int decoderId);

//	bool removeCameraItem(int cameraId);
    void removeAllCameraItems();
    void removeAllDecoderItems();
	void videoInputDataChanged();
    void boardDecoderDataChanged();

protected:
	virtual void contextMenuEvent(QContextMenuEvent * event);

private:
	QTreeWidgetItem *m_allBoards;
	QTreeWidgetItem *m_allCameras;
    QTreeWidgetItem *m_allPlayers;
};


class BoardItem : public QTreeWidgetItem
{
public:
    BoardItem(DVRBoard *board);

    DVRBoard *board() { return m_board; }

    virtual QVariant data(int column, int role) const;
//    virtual void setData(int column, int role, const QVariant & value);

private:
    DVRBoard *m_board;
};

class VideoInputItem : public QTreeWidgetItem
{
public:
    VideoInputItem(int boardIndex, int input);

    int boardIndex() { return m_boardIndex; }
    int input()      { return m_input; }

    virtual QVariant data(int column, int role) const;

private:
    int m_boardIndex;
    int m_input;
};

class CBoardDecoderItem : public QTreeWidgetItem
{
public:
    CBoardDecoderItem(int boardIndex, int chan_num);

    int boardIndex() { return m_boardIndex; }
    int chan_num()      { return m_chan_num; }

    virtual QVariant data(int column, int role) const;

private:
    int m_boardIndex;
    int m_chan_num;
};

class CameraItem : public QTreeWidgetItem
{
public:
    CameraItem(int cameraId);

    int cameraId() { return m_cameraId; }

    virtual QVariant data(int column, int role) const;

private:
    int m_cameraId;
};

class CPlayerItem : public QTreeWidgetItem
{
public:
    CPlayerItem(int decoderId);

    int getDecoderId() { return m_decoderId; }

    virtual QVariant data(int column, int role) const;

private:
    int m_decoderId;
};

class SpotMonitorItem : public QTreeWidgetItem
{
public:
    SpotMonitorItem(SpotMonitor *smo);

    SpotMonitor *spotMonitor() const { return m_smo; }
    virtual QVariant data(int column, int role) const;

private:
    SpotMonitor *m_smo;
};

#endif /* DVRSYSTEMTREE_H */
