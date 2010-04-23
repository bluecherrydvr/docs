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
#include <QMouseEvent>
#include <QMenu>
#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEngine>
#include "dvrsystem.h"
#include "sdvr_sdk.h"
#include "videopanel.h"
#include "viewsettings.h"
#include "mainwindow.h"
#include "camera.h"
#include "player.h"
#include "log.h"
#include "sdvr.h"



extern DVRSystem *g_DVR;

const int VideoPanel::MARGIN = 7;
const int VideoPanel::SPACING = 10;

// Custom dash drawing for the selected frame
const qreal VideoPanel::SPACE_LENGTH = 3;
const qreal VideoPanel::DASH_LENGTH = 3;

VideoPanel::VideoPanel(int hres, int vres, ViewSettings *vs, QWidget *parent)
    : QWidget(parent)
{
    QPalette qp = palette();
    QBrush  br(QColor(0,0,0));
    qp.setBrush(QPalette::Window,br);
//    setPalette(qp);
//    setBackgroundRole(QPalette::Window);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen, true);    
    m_hres = hres;
    m_vres = vres;
    m_settings = vs;

    // By default the top left grid is selected
    if (m_settings->displayCount() > 0)
        m_settings->setSelectedDisplay(0);

    // Re-adjust for the correct display aspect ration. 
    // We should display the current raw video frame into 
    // 4x3 aspect ratio. This only applies to HMO and not SMO.
    hres = sdvr::getHMOAspectRatio(hres,g_DVR->videoStandard());

    m_displayRect = vs->layout(hres, vres, MARGIN, MARGIN, SPACING);
    m_displayId.resize( vs->displayCount());
    resize(vs->size(hres, vres, MARGIN, MARGIN, SPACING));
    m_started = false;

}

VideoPanel::~VideoPanel()
{
//    stopPlaying();
}

void VideoPanel::setView(int hres, int vres, ViewSettings *vs)
{
    if (m_settings != NULL)
        stopPlaying();

    m_hres = hres;
    m_vres = vres;
    m_settings = vs;   

    m_settings->setSelectedDisplay(0);
    // Re-adjust for the correct display aspect ration. 
    // We should display the current raw video frame into 
    // 4x3 aspect ratio. This only applies to HMO and not SMO.
    hres = sdvr::getHMOAspectRatio(hres,g_DVR->videoStandard());

    m_displayRect = vs->layout(hres, vres, MARGIN, MARGIN, SPACING);
    m_displayId.resize( vs->displayCount());
    resize(vs->size(hres, vres, MARGIN, MARGIN, SPACING));
    startPlaying();

    // force updating of the screen to repaint the new grids
    update();
}

// getDecimationSize()
//      This method returns the raw video resolution decimation
//      deminssions.
QSize VideoPanel::getDecimationSize()
{
    QSize size;

    size.setWidth(m_hres);
    size.setHeight(m_vres);

    return size;

}
void VideoPanel::closeEvent ( QCloseEvent * event )
{
    stopPlaying();
    QWidget::closeEvent( event);
}

void VideoPanel::showEvent ( QShowEvent * event )
{
    QWidget::showEvent( event);
    startPlaying();
}

void VideoPanel::paintEvent (QPaintEvent * e)
{   
    sdvr_ui_region_t region;
    QRect r = e->rect();

    region.top_left_x = r.x();
    region.top_left_y = r.y();
    region.width = r.width();
    region.height = r.height();

    sdvr_ui_clear_yuv(winId(), &region);
    sdvr_ui_refresh();
    drawGrids(false);
}

void VideoPanel::mousePressEvent(QMouseEvent *event)
{
	int i;
    int nDisplays = m_settings->displayCount();
	bool bChanFound = false;

    // Select the new grid which is clicked on.
    // Remove the focus rect from the previously selected
    // grid and add it to the new one.
    for (i = 0; i < nDisplays; ++i)
    {
        if (m_displayRect[i].contains(event->x(), event->y()))
        {
            QRect rect;
			bChanFound = true;
            rect = m_displayRect[i];

            // No need to do anything, if the current grid
            // is already selected.
            if (m_settings->selectedDisplay() == i)
                break;

            m_settings->setSelectedDisplay(i);
            drawGrids();

            emit screenSelected();
            break;
        }
    }
	if (event->button() == Qt::RightButton && bChanFound)
	{
        if (m_settings->displayChannel(i) != INVALID_CHAN_HANDLE)
		{
            g_MainWindow->videoPanelMenu()->exec(event->globalPos());
		}
		else
		{
			g_MainWindow->setStatusBar("");
		}
	}
}

void VideoPanel::startPlaying()
{    
    sdvr_err_e err = SDVR_ERR_INVALID_HANDLE;

    if (m_started)
        return;
    m_started = true;
	// Start raw video streaming for all the grids
    // in the current display.
    int count = m_settings->displayCount();
    for (int i = 0; i < count; ++i)
    {
        if (m_settings->displayChannel(i) != INVALID_CHAN_HANDLE)
        {
           err = EnableChanRawVideo(m_settings->displayChannel(i), true);
           if (err != SDVR_ERR_NONE)
                Log::error(QString("Error starting HMO for '%1' [rc = %2 (%3)]")
                   .arg(g_DVR->getNameFromHandle(m_settings->displayChannel(i))).
                   arg(sdvr_get_error_text(err)).arg(err));
           {
               sdvr_ui_region_t ui_region;
               int id;
               QRect rc = m_displayRect[i];
               ui_region.top_left_x = rc.x();
               ui_region.top_left_y = rc.y();
               ui_region.width = rc.width();
               ui_region.height = rc.height();
               if (!g_MainWindow->isSkipRawVideoDisplay() &&
                   sdvr_ui_start_video_preview(m_settings->displayChannel(i),winId(),&ui_region,&id) == SDVR_UI_ERR_NONE)
                   m_displayId[i] = id;
               else
                   m_displayId[i] = -1;
           }
           if (m_settings->isEnableRawAudio())
           {
               err = EnableChanRawAudio(m_settings->displayChannel(i), true);
           }
        }
    }  
}

void VideoPanel::stopPlaying()
{
	// Stop raw video streaming
    sdvr_err_e err = SDVR_ERR_INVALID_HANDLE;
    int count = m_settings->displayCount();

    for (int i = 0; i < count; ++i)
    {
        if (m_settings->displayChannel(i) != INVALID_CHAN_HANDLE)
        {
            err = EnableChanRawVideo(m_settings->displayChannel(i), false);
            if (err != SDVR_ERR_NONE)
                Log::error(QString("Error stopping HMO for '%1' [rc = %2 (%3)]")
                   .arg(g_DVR->getNameFromHandle(m_settings->displayChannel(i))).
                   arg(sdvr_get_error_text(err)).arg(err));

            if (!g_MainWindow->isSkipRawVideoDisplay())
                sdvr_ui_stop_video_preview(m_settings->displayChannel(i),m_displayId[i]);
//			clearDisplay(i);

           if (m_settings->isEnableRawAudio())
           {
               err = EnableChanRawAudio(m_settings->displayChannel(i), false);
           }

        }
    } 
    m_started = false;
}

void VideoPanel::updateDisplayChan(sdvr_chan_handle_t newChanHandle)
{
    int curDisplay = m_settings->selectedDisplay();
    
    /***************************************************************
        In SMO displays, same channel can not be displayed in 
        multiple grids. incases the new channel is already assigned
        to a different preview region, we must first 
        remove that channel from previous preview region before 
        it can be added to the new location.
    ****************************************************************/
    if (m_settings->isSmo())
    {
        QVector<int> displaysPlayingChannel = m_settings->displaysPlayingChannel(newChanHandle);
        if (displaysPlayingChannel.count() > 0)
        {
            int display = displaysPlayingChannel.first();
            if (!g_MainWindow->isSkipRawVideoDisplay())
                sdvr_ui_stop_video_preview(newChanHandle,m_displayId[display]);
            // Clear the channel that was turned off.
            QRect rc = m_displayRect[display];
            update(rc);

            m_settings->setDisplayChannel(display, INVALID_CHAN_HANDLE);
        }
    }

    // remove the current display chan from the preview regions
    if (!g_MainWindow->isSkipRawVideoDisplay())
        sdvr_ui_stop_video_preview(m_settings->selectedDisplayChannel(),m_displayId[curDisplay]);

    // Assign the new channel handle to the current display and
    // add it to the display regions for this channle handle only if
    // we did not remove a channel from this display region.
    m_settings->setDisplayChannel(curDisplay, newChanHandle);
    if (newChanHandle != INVALID_CHAN_HANDLE)
    {
        sdvr_ui_region_t ui_region;
        int id;
        QRect rc = m_displayRect[curDisplay];
        ui_region.top_left_x = rc.x();
        ui_region.top_left_y = rc.y();
        ui_region.width = rc.width();
        ui_region.height = rc.height();
        if (!g_MainWindow->isSkipRawVideoDisplay() &&
            sdvr_ui_start_video_preview(newChanHandle, winId(), &ui_region,&id) == SDVR_UI_ERR_NONE)
           m_displayId[curDisplay] = id;
        else
           m_displayId[curDisplay] = -1;
    }
    else
    {
        // Clear the channel that was turned off.
        QRect rc = m_displayRect[curDisplay];
        update(rc);
    }
}

sdvr_err_e VideoPanel::EnableChanRawVideo(sdvr_chan_handle_t handle, bool enable)
{
    sdvr_err_e err = SDVR_ERR_WRONG_CHANNEL_TYPE;    

    if (sdvr_get_chan_type(handle) == SDVR_CHAN_TYPE_DECODER)
    {
	    CPlayer *chan =g_DVR->findDecoderByHandle(handle);
        if (chan != NULL)
           err = chan->setRawVideo(m_settings->decimation(), enable);
    }
    else
    {
	    Camera *chan =g_DVR->findCameraByHandle(handle);
        if (chan != NULL)
            err = chan->setRawVideo(m_settings->decimation(), enable);
    }   
    return err;
 }

sdvr_err_e VideoPanel::EnableChanRawAudio(sdvr_chan_handle_t handle, bool enable)
{
    sdvr_err_e err = SDVR_ERR_WRONG_CHANNEL_TYPE;    

    if (sdvr_get_chan_type(handle) == SDVR_CHAN_TYPE_DECODER)
    {
	    CPlayer *chan =g_DVR->findDecoderByHandle(handle);
        if (chan != NULL)
           err = chan->setRawAudio(enable);
    }
    else
    {
	    Camera *chan =g_DVR->findCameraByHandle(handle);
        if (chan != NULL)
            err = chan->setRawAudio(enable);
    }   
    return err;
 }

void VideoPanel::clearDisplay( int s )
{
    if ( ( s>=0) && (s < m_displayRect.size()))
    {
        sdvr_ui_region_t ui_region;
        QRect rc = m_displayRect[s];

        ui_region.top_left_x = rc.x();
        ui_region.top_left_y = rc.y();
        ui_region.width = rc.width();
        ui_region.height = rc.height();

        sdvr_ui_clear_yuv(winId(),&ui_region);
    }
}

/*
   This function reads the frame raw A/V buffer for the given
   channel handle. It display the raw video buffers on all the
   displays for on the current video panel. Additionally, it
   saves the raw A/V buffers into a file if the user is saving
   raw Audio or Video frames.

*/
bool VideoPanel::acquireBuffer(sdvr_chan_handle_t handle)
{
    sdvr_yuv_buffer_t *buf;
    sdvr_err_e err;
    err = sdvr_get_yuv_buffer(handle, &buf);

    if (err == SDVR_ERR_NONE && buf)
    {
        // Check if we should save the YUV buffers for this camera into a file
        if (g_MainWindow->getyuvSaveHandle() && (g_MainWindow->getyuvSaveHandle() == handle))
            g_MainWindow->saveYUVBuffer(buf);

        // Display this video frame in all the HMO grids that the
        // camera/player is assigned to.
#ifdef low_level_ui_display
        updateDisplay(handle, buf);
#endif

        sdvr_release_yuv_buffer(buf);
    }
 
    // Check if we should save the raw audio buffers for this camera into a file
    if (g_MainWindow->getRawAudioSaveHandle() && (g_MainWindow->getRawAudioSaveHandle() == handle))
    {
        sdvr_av_buffer_t  *av_buffer;
        if (sdvr_get_av_buffer(handle, SDVR_FRAME_RAW_AUDIO, &av_buffer) == SDVR_ERR_NONE)
        {
            g_MainWindow->saveRawAudioBuffer(av_buffer);
            sdvr_release_av_buffer(av_buffer);
        }
    }

    return true;
}

/*
  This function actually displays the raw video on the screen.
*/
#ifdef low_level_ui_display
void VideoPanel::updateDisplay(sdvr_chan_handle_t chanHandle, sdvr_yuv_buffer_t *yuv_buf)
{
    int numScreens = m_settings->displayCount();
    sdvr_ui_region_t ui_region;

    sdvr_ui_set_yuv_buffer(yuv_buf);

    for (int screen = 0; screen < numScreens; ++screen)
    {
        if (m_settings->displayChannel(screen) == chanHandle)
        {
            QRect rc = m_displayRect[screen];
            ui_region.top_left_x = rc.x();
            ui_region.top_left_y = rc.y();
            ui_region.width = rc.width();
            ui_region.height = rc.height();

            sdvr_ui_draw_yuv(winId(),&ui_region);
        }
    }
}
#endif

/*
   This function draw the grids for the current video pannel.
   It indicates the currently selected grid with dotted yellow frame.
   The non-selected grids are denoted with solid green frames.


   Remark: Due to a Qt painting problem, we should draw the
   frame using Qt function when drawGrids() is being called from
   the paintEvent(). Otherwise, we draw the frames using the
   linux or windows native calls to prevent flashing.
*/
void VideoPanel::drawGrids(bool bQtDraw)
{
//    char prompt[100];
    int numScreens = m_settings->displayCount();
    QPen myPen;
    QRect rect;
    sdvr_ui_color_key_t rgb_color;
    int pen_style;

    myPen.setWidth(1);

//    sprintf(prompt,"drawGirds %d \n",m_settings->selectedDisplay());
//    OutputDebugStringA(prompt);

    // Draw the grid
    for (int screen = 0; screen < numScreens; ++screen)
    {
        rect = m_displayRect[screen];

        if (screen == m_settings->selectedDisplay())
        {
            QVector<qreal> dashes;
//            sprintf(prompt,"draw highlightedGirds %d \n",m_settings->selectedDisplay());
//            OutputDebugStringA(prompt);

            // Draw the highlighted frame
            myPen.setColor(Qt::yellow);  // Bright yellow
            myPen.setStyle(Qt::CustomDashLine);
            dashes << DASH_LENGTH << SPACE_LENGTH;
            myPen.setDashPattern(dashes);

            rgb_color = sdvr_rgb(255,255,0); // Bright yellow
            pen_style = SDVR_UI_LS_DOT;

        }
        else
        {
            // Draw the frame
            myPen.setColor(Qt::green);  // green
            myPen.setStyle(Qt::SolidLine);
            rgb_color = sdvr_rgb(0,255,0);  // green
            pen_style = SDVR_UI_LS_SOLID;
        }

        if (bQtDraw)
        {
           QPainter painter(this);

           painter.setBackgroundMode(Qt::TransparentMode);
           painter.setPen(myPen);
           painter.drawRect(rect.x()-2, rect.y()-2, rect.width() + 2, rect.height() + 2);
        }
        else
        {
            sdvr_ui_region_t region;

            region.top_left_x = rect.x() - 2;
            region.top_left_y = rect.y() - 2;
            region.width = rect.width() + 2;
            region.height = rect.height() + 2;
            sdvr_ui_draw_frame(winId(), region, rgb_color, pen_style);
        }

    }
}

