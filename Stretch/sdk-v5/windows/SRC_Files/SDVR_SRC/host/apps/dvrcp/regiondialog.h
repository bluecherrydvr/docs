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

#ifndef REGIONDIALOG_H
#define REGIONDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QDomElement>
#include <sdvr_sdk.h>

#ifdef Q_OS_LINUX
#include <QX11Info>
#include <X11/Xlib.h>
#endif

class Camera;
class CRegion;

class DrawingArea : public QWidget
{
	Q_OBJECT

public:
	static const int DRAW  = 1;
	static const int ERASE = 2;

	DrawingArea(CRegion *reg, QWidget *parent = 0);
	~DrawingArea();

	int drawingMode() { return m_mode; }
	void load(); // Load from the region
	void save(); // Save the drawing into the region

#ifdef WIN32
	void drawRegion(HDC hdc);
#else
	void drawRegion(Display *dpy, WId win);
#endif

public slots:
	void setDrawingMode(int mode) { m_mode = mode; }
	void setDrawingLayer(int layer);
	void showLayer(int layer);
	void hideLayer(int layer);
	void fillAll();
	void clear();

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
	void drawLineTo(const QPoint &endPoint);

	QVector<QImage *> m_layers;
	QVector<bool> m_showLayer;
	int m_drawLayer;

	int m_mode;

	bool m_modified;
	bool m_scribbling;
	QPoint m_lastPoint;
	CRegion *m_region;

#ifdef Q_OS_LINUX
	QX11Info m_x11Info;
	Pixmap stip[4];
#endif
};


class RegionDialog : public QDialog
{
	Q_OBJECT

public:
	RegionDialog(Camera *cam, CRegion *reg, QWidget *parent = 0);

private slots:
	void showStateChanged(int state);

protected:
	virtual void showEvent(QShowEvent *);
	virtual void closeEvent(QCloseEvent *);
	virtual void hideEvent(QHideEvent *);

private slots:
	virtual void accept();

private:
	bool playVideo();
	void stopVideo();

	DrawingArea *m_drawingArea;
	QVector<QCheckBox *> m_showCheckBoxes;

	Camera *m_camera;
	int m_previewId;
};

#endif /* REGIONDIALOG_H */
