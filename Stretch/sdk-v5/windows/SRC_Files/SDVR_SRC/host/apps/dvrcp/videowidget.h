
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

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <X11/Xlib.h>
#include <X11/extensions/Xvlib.h>
#include <X11/extensions/XShm.h>

class VideoWidget : public QWidget
{
    Q_OBJECT

    public:        
    VideoWidget(QWidget *parent = 0);
    ~VideoWidget();
    void setVideoSize(int w, int h);
    void setBufferCount( int c );
    int writeVideoBuffer(int n, char * y,char* u,char* v);
    void clearVideoBuffer(int n);
    void updateVideoRegion(int n,int x, int y);
    void updateHighlightRect(int x, int y);

    protected:
    int createVideoBuffer();
    int releaseVideoBuffer();
    int openVideo();
    int closeVideo();
    void drawRect(int x1,int y1,int x2,int y2, QColor c);

    int v_width;
    int v_height;
    int rect_x;
    int rect_y;

    private:
static    int       m_xv_port;
static    bool      b_color_setted;
    XvImage  * *images;
    GC       m_gc;
    XShmSegmentInfo    *m_shm;
    int     m_count;
    Display *   m_display;
};
#endif
