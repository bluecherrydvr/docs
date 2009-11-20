#include <QPainter>
#include <QPaintEvent>
#include <QColormap>
#include <QX11Info>
#include <QApplication>
#include <QMessageBox>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "videowidget.h"
int       VideoWidget::m_xv_port = -1;
bool      VideoWidget::b_color_setted = 0;
#define YV12_FORMAT 0x32315659

#define NO_USE_SHM 0
VideoWidget::VideoWidget(QWidget *parent):
    QWidget( parent )
{
    //set background color to black
    if ( !b_color_setted )
    {
        QPalette pl = QApplication::palette();
        pl.setColor(QPalette::Normal,QPalette::Dark,Qt::black);
        pl.setColor(QPalette::Active,QPalette::Dark,Qt::black);
        pl.setColor(QPalette::Inactive,QPalette::Dark,Qt::black);
        pl.setColor(QPalette::Disabled,QPalette::Dark,Qt::black);
        QApplication::setPalette( pl,"VideoWidget" );
        b_color_setted = 1;
    }
    images = 0;
    m_count = 0;
    rect_x = -1;
    rect_y = -1;
    //m_gc = 0;
    //m_display = 0;
    m_display = QX11Info::display();
    XGCValues gcv;
    m_gc = XCreateGC(m_display,winId(),GCGraphicsExposures,&gcv);
}

VideoWidget::~VideoWidget()
{
    releaseVideoBuffer();
    closeVideo();
    if (m_gc != 0)
    {
        XFreeGC( m_display, m_gc );
        m_gc = 0;
    }
}

void VideoWidget::setVideoSize(int w, int h )
{
    v_width = w;
    v_height = h;
}

void VideoWidget::setBufferCount(int c )
{
    releaseVideoBuffer();
    closeVideo();
    rect_x = -1;
    rect_y = -1;
    m_count = c;
    openVideo();
    createVideoBuffer();
    update();
}

int VideoWidget::writeVideoBuffer(int n, char * y, char * u, char* v)
{
    if ( images && ( n>=0) && ( n< m_count))
    {
        char * dst = images[n]->data;
        int     size = v_width*v_height;
        memcpy(dst,y,size);
        dst += size;
        size /=4;
        memcpy(dst,v,size);
        dst+=size;
        memcpy(dst,u,size);
    }
    return 0 ;
}
void VideoWidget::clearVideoBuffer( int n)
{
    if ( images && ( n>=0) && ( n< m_count))
    {
        char * dst = images[n]->data;
        int     size = v_width*v_height;
        memset(dst,0,size);
        dst += size;
        size /=4;
        memset(dst,-128,size);
        dst+=size;
        memset(dst,-128,size);
    }
}

int VideoWidget::createVideoBuffer()
{
    int i;
    if ( m_xv_port == -1)
        return 0;
    images = new XvImage*[m_count];
    m_shm = new XShmSegmentInfo[m_count];
    for (i=0;i<m_count;i++)
    {
        images[i] = XvCreateImage(m_display,m_xv_port, YV12_FORMAT, 0, v_width, v_height);
        memset(&(m_shm[i]),0,sizeof(XShmSegmentInfo));
        if (images[i])
        {
#if NO_USE_SHM        
            images[i]->data = (char *)malloc(images[i]->data_size);
#else        
            images[i]->obdata = (XPointer)&(m_shm[i]);
            m_shm[i].shmid = shmget( IPC_PRIVATE, images[i]->data_size,IPC_CREAT | 0777);
            if (m_shm[i].shmid >= 0)
            {
                m_shm[i].shmaddr = (char*) shmat(m_shm[i].shmid,0,0);
                m_shm[i].readOnly = False;
                XShmAttach( m_display, &(m_shm[i]));
                XSync(m_display,True);
                shmctl(m_shm[i].shmid,IPC_RMID,NULL);
                images[i]->data = m_shm[i].shmaddr;
            }
#endif        
        }
    }
    return 0;
}

int VideoWidget::releaseVideoBuffer()
{
    if (images )
    {
        int i;
        for (i=0;i<m_count;i++)
        {
#if NO_USE_SHM
            free(images[i]->data);
#else        
            XShmDetach(m_display, &(m_shm[i]));
            shmdt(m_shm[i].shmaddr);
#endif        
            XFree( images[i] );
        }
        delete[] images;
        delete[] m_shm;
        m_count = 0;
        images = 0;
        m_shm = 0;
    }
    return 0;
}

void VideoWidget::updateHighlightRect(int x, int y)
{
        if ( (rect_x != x ) || (rect_y != y))
        {
            drawRect( rect_x-1,rect_y-1,rect_x+v_width,rect_y+v_height,Qt::black);
        }
        rect_x = x;
        rect_y = y;
        drawRect(rect_x-1,rect_y-1,rect_x+v_width,rect_y+v_height,Qt::green);
}

void VideoWidget::drawRect(int x1,int y1,int x2,int y2, QColor c)
{
    if ( m_gc !=0 )
    {
        QColormap cm = QColormap::instance();
        int pixel = cm.pixel(c);
        XPoint xp[5];
        xp[0].x=x1;
        xp[0].y=y1;
        xp[1].x=x1;
        xp[1].y=y2;
        xp[2].x=x2;
        xp[2].y=y2;
        xp[3].x=x2;
        xp[3].y=y1;
        xp[4].x=x1;
        xp[4].y=y1;
        XSetForeground(m_display,m_gc,pixel);
        XDrawLines(m_display,winId(),m_gc,xp,5,CoordModeOrigin);
    }
}

void VideoWidget::updateVideoRegion(int n, int x , int y)
{
    if ( (m_xv_port >=0) && (m_gc != 0) && ( n>=0) && (n < m_count))
    {
        XvShmPutImage(m_display,m_xv_port,winId(), m_gc, images[n] ,0,0,v_width,v_height,x,y,v_width,v_height,False);
    }
}

int VideoWidget::openVideo()
{
    int ret = 0;
    //m_display = XOpenDisplay(NULL);
    if ( m_display )
    {
        unsigned int ad;
        unsigned int j;

        if (m_xv_port == -1)
        {
            XvAdaptorInfo  *ainfo;
            //grab the video port
            if ( (Success== XvQueryExtension(m_display,&j,&j,&j,&j,&j)) && 
                    (Success == XvQueryAdaptors(m_display,RootWindow(m_display,0),&ad,&ainfo)))
            {
                if ( ad > 0)
                {
                    int adnum = ad-1;
                    if ( XvGrabPort(m_display,ainfo[adnum].base_id,CurrentTime) == Success)
                    {
                        m_xv_port = ainfo[adnum].base_id;
                        ret = 1;
                        {
                            static const char *attr[] = { "XV_AUTOPAINT_COLORKEY", "XV_AUTOPAINT_COLOURKEY" };
                            unsigned int i;

                            XvSelectPortNotify(m_display, m_xv_port, True);
                            for ( i=0; i < sizeof(attr)/(sizeof attr[0]); ++i ) {
                                Atom a;

                                a = XInternAtom(m_display, attr[i], True);
                                if ( a != None ) {
                                    XvSetPortAttribute(m_display, m_xv_port, a, 1);
                                    XSync(m_display, True);
                                }
                            }
                            XvSelectPortNotify(m_display, m_xv_port, True);
                        }
                    }
                    else
                    {
                        m_xv_port = -1;
                        m_gc = 0;
                    }
                }
                if (ainfo)
                {
                    XvFreeAdaptorInfo(ainfo);
                }
            }
        }
        else
        {
            QMessageBox::warning( this, "DVRCP","XV extention is not supported by current X server. "
                    "You will not be able to see the HMO video."
                    );
        }
    }

    return ret;
}

int VideoWidget::closeVideo()
{
    /*
    if (m_gc != 0)
    {
        XFreeGC( m_display, m_gc );
        m_gc = 0;
    }
    */
    if (m_xv_port != -1)
    {
        XvUngrabPort(m_display,m_xv_port,CurrentTime);
        m_xv_port = -1;
    }
    return 0;
}
