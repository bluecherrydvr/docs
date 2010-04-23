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
#ifndef VIDEOPANEL_H
#define VIDEOPANEL_H

#include <QMutex>
#include "sdvr_sdk.h"

#include <QtGui/QWidget>
#include "sdvr_ui_sdk.h"

class ViewSettings;

class VideoPanel : public QWidget
{
    Q_OBJECT

    static const int MARGIN;
    static const int SPACING;

    static const qreal SPACE_LENGTH;
    static const qreal DASH_LENGTH;


    typedef QPair<sdvr_chan_handle_t, sdvr_yuv_buffer_t *> BufferListItem;

public:
	VideoPanel(int hres, int vres, ViewSettings *vs, QWidget *parent = 0);
    ~VideoPanel();

    void setView(int hres, int vres, ViewSettings *vs);
    ViewSettings *settings() const { return m_settings; }
    QSize getDecimationSize();


    void startPlaying();
    void stopPlaying();
    void updateDisplay();
    void updateDisplayChan(sdvr_chan_handle_t newChanHandle);

    void clearDisplay( int s);

    bool acquireBuffer(sdvr_chan_handle_t handle);    

signals:
    void screenSelected();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    sdvr_err_e EnableChanRawVideo(sdvr_chan_handle_t handle, bool enable);
    sdvr_err_e EnableChanRawAudio(sdvr_chan_handle_t handle, bool enable);

#ifdef low_level_ui_display
    void updateDisplay(sdvr_chan_handle_t chanHandle, sdvr_yuv_buffer_t *yuv_buf);
#endif
    void drawGrids(bool bQtDraw = false);   
    virtual void showEvent ( QShowEvent * event );
    virtual void closeEvent( QCloseEvent * e);
private:
    int m_hres;
    int m_vres;

    QVector<QRect> m_displayRect;
    QVector<int>   m_displayId;

    ViewSettings *m_settings;
    bool    m_started;

};
#endif
