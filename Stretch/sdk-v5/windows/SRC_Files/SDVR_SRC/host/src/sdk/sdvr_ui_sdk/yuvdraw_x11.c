
/************************************************************************
 *
 * Copyright C 2007 Stretch, Inc. All rights reserved.  Stretch products are
 * protected under numerous U.S. and foreign patents, maskwork rights,
 * copyrights and other intellectual property laws.
 *
 * This source code and the related tools, software code and documentation, and
 * your use thereof, are subject to and governed by the terms and conditions of
 * the applicable Stretch IDE or SDK and RDK License Agreement
 * (either as agreed by you or found at www.stretchinc.com).  By using these
 * items, you indicate your acceptance of such terms and conditions between you
 * and Stretch, Inc.  In the event that you do not agree with such terms and
 * conditions, you may not use any of these items and must immediately destroy
 * any copies you have made.
 *
 ************************************************************************/
#include "sdvr_ui_sdk.h"
#include <stdlib.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include "yuvdraw.h"

#define NO_USE_SHM 0
/*
    Maximum width and height of a video frame allowed.
    The user sets this limit when calling sdvr_ui_init()
*/
sx_uint32 m_max_width = 720;
sx_uint32 m_max_height = 480;

/*
    The width and height of the current video frame being displayed.
*/
sx_uint32 m_width = 0;
sx_uint32 m_height = 0;

/*
    The default color key is white.
*/
sdvr_ui_color_key_t    m_keyColor = sdvr_rgb(255,255,255);

/*
    A flag to indicate if the video card supports color key.
    NOTE: Currently some nvida cards don't support color keys.
*/
int  m_color_key_supported = 0;
Visual  * g_visual = 0;

static Display * g_display = 0;
static int g_xv_port = -1;
static sdvr_ui_surface m_overlay_surface = NULL;

#define YV12_FORMAT 0x32315659
#define sdvr_get_r( c )    ((unsigned char)(c))
#define sdvr_get_g( c )    ((unsigned char)((c)>>8))
#define sdvr_get_b( c )    ((unsigned char)((c)>>16))

unsigned long    get_color_pixel(  sdvr_ui_color_key_t rgb_color);
static int (*X_handler)(Display *, XErrorEvent *) = NULL;

static int shm_errhandler(Display *d, XErrorEvent *e)
{
    printf("shm error %d\n",e->error_code);
    return(X_handler(d,e));
}

static int xv_errhandler(Display *d, XErrorEvent *e)
{
    printf("Xv error %d\n",e->error_code);
    return(X_handler(d,e));
}


/* create a offscreen surface */
sdvr_ui_surface create_sdvr_ui_surface( int width, int height )
{
    sdvr_ui_surface p;
    if ( g_display == 0)
    {
        return NULL;
    }
    p = (sdvr_ui_surface)malloc( sizeof (struct sdvr_ui_surface_struct_t));
    p->image = XvCreateImage(g_display,g_xv_port, YV12_FORMAT, 0, width, height);
    memset(&(p->shm),0,sizeof(XShmSegmentInfo));
    if (p->image)
    {
#if NO_USE_SHM
        p->image->data = (char *)malloc(p->image->data_size);
#else
        p->image->obdata = (XPointer)&(p->shm);
        p->shm.shmid = shmget( IPC_PRIVATE, p->image->data_size,IPC_CREAT | 0777);
        if (p->shm.shmid >= 0)
        {
            p->shm.shmaddr = (char*) shmat(p->shm.shmid,0,0);
            p->shm.readOnly = False;
            XShmAttach( g_display, &(p->shm));
            XSync(g_display,True);
            shmctl(p->shm.shmid,IPC_RMID,NULL);
            p->image->data = p->shm.shmaddr;
        }
#endif
    }
    return p;
}

void release_sdvr_ui_surface( sdvr_ui_surface p)
{
    if (p)
    {
#if NO_USE_SHM
        free(p->image->data);
#else
        XShmDetach(g_display, &(p->shm));
        shmdt(p->shm.shmaddr);
#endif
        XFree( p->image );
        free(p);
    }
}

sdvr_err_ui_e init_draw(sx_uint32 max_width, sx_uint32 max_lines)
{
    int ret = SDVR_ERR_UI_INIT;

    if ( g_display == 0)
    {
        g_display = XOpenDisplay(NULL);
    }
    if ( g_display )
    {
        int snum = XDefaultScreen(g_display);
        Visual * v = XDefaultVisual(g_display,snum);
        if ( v )
        {
            if ((v->class == TrueColor)||(v->class == DirectColor))
            {
                ret = SDVR_ERR_NONE;
                g_visual = v;
            }
            else
            {
                printf("Error: X display is not in TrueColor mode, Video display is disabled.\n");
            }
        }
    }
    if (ret == SDVR_ERR_NONE)
    {
        unsigned int ad;
        unsigned int j;
        ret = SDVR_ERR_UI_INIT;
        if (g_xv_port == -1)
        {
            XvAdaptorInfo  *ainfo;
            //grab the video port
            if ( (Success== XvQueryExtension(g_display,&j,&j,&j,&j,&j)) &&
                    (Success == XvQueryAdaptors(g_display,RootWindow(g_display,0),&ad,&ainfo)))
            {
                if ( ad > 0)
                {
                    int adnum = ad-1;
                    if ( XvGrabPort(g_display,ainfo[adnum].base_id,CurrentTime) == Success)
                    {
                        g_xv_port = ainfo[adnum].base_id;
                        ret = SDVR_ERR_NONE;
                        {
                            static const char *attr[] = { "XV_AUTOPAINT_COLORKEY", "XV_AUTOPAINT_COLOURKEY" };
                            unsigned int i;

                            XvSelectPortNotify(g_display, g_xv_port, True);
                            for ( i=0; i < sizeof(attr)/(sizeof attr[0]); ++i ) {
                                Atom a;

                                a = XInternAtom(g_display, attr[i], True);
                                if ( a != None ) {
                                    XvSetPortAttribute(g_display, g_xv_port, a, 1);
                                    XSync(g_display, True);
                                }
                            }
                            XvSelectPortNotify(g_display, g_xv_port, True);
                            X_handler = XSetErrorHandler(xv_errhandler);
                        }
                    }
                    else
                    {
                        g_xv_port = -1;
                    }
                }
                if (ainfo)
                {
                    XvFreeAdaptorInfo(ainfo);
                }
            }
            else
            {
                printf("Error: Xv extension is not supported"
                        "by current X display driver. "
                        "Video display is disabled.\n");
            }
        }
    }
    if (ret == SDVR_ERR_NONE)
    {
        m_max_width = max_width;
        m_max_height = max_lines;
        m_overlay_surface = create_sdvr_ui_surface(max_width, max_lines);
    }
    else
    {
        XCloseDisplay(g_display);
        g_display = NULL;
    }

    return ret;
}

sdvr_err_ui_e close_draw()
{
    if (g_display)
    {
        release_sdvr_ui_surface(m_overlay_surface);
        if (g_xv_port != -1)
        {
            XvUngrabPort(g_display,g_xv_port,CurrentTime);
            g_xv_port = -1;
        }
        XCloseDisplay(g_display);
    }
    g_display = 0;
    return 0;
}

/*set the buffer data*/
sdvr_err_ui_e draw_set_yuv_buffer(sdvr_ui_surface surface, char * y, char * u, char *v, unsigned int width, unsigned int height)
{
    char *p,*q;
    sx_uint32 i;
    int pitch = width;
    int lines = height;

    if (g_display == 0)
    {
        return SDVR_ERR_UI_NO_INIT;
    }

    if (surface == NULL)
    {
        surface = m_overlay_surface;
        pitch = m_max_width;
        lines = m_max_height;
    }
    if ( surface->image == NULL)
    {
        return SDVR_ERR_UI_NO_INIT;
    }
    m_width = width;
    m_height = height;
    /**********************
      Setup the Y Buffer
     ***********************/
    p = (char *)surface->image->data;
    q = y;
    for (i = 0; i< m_height;i++)
    {
        memcpy(p,q,m_width);
        p+=pitch;
        q+=m_width;
    }
    /**********************
      Setup the v Buffer
     ***********************/

    p = (char *)surface->image->data + pitch * lines;
    q = v;
    for ( i = 0; i< m_height/2; i++)
    {
        memcpy(p,q,m_width/2);
        p+=pitch/2;
        q+=m_width/2;
    }

    /**********************
      Setup the u Buffer
     ***********************/

    p = (char *)surface->image->data + pitch * lines/4*5;
    q = u;
    for ( i = 0; i< m_height/2; i++)
    {
        memcpy(p,q,m_width/2);
        p+=pitch/2;
        q+=m_width/2;
    }
    return SDVR_UI_ERR_NONE;
}

sdvr_err_ui_e draw_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region,
		sdvr_ui_surface src_surface, sdvr_ui_region_t *src_region,
		sdvr_ui_preview_cb preview_cb)
{
    GC        gc;
    XvImage  *  image;
    if ( g_display )
    {
        int snum = XDefaultScreen(g_display);
        gc = XDefaultGC(g_display,snum);
        if (gc)
        {
            int x,y,w,h;
            if ( src_surface == NULL)
            {
                image = m_overlay_surface->image;
            }
            else
            {
                image = src_surface->image;
            }
            if ( src_region == NULL)
            {
                x = y = 0;
                w = m_width;
                h = m_height;
            }
            else
            {
		        x = src_region->top_left_x;
		        y = src_region->top_left_y;
		        w = src_region->width;
		        h = src_region->height;
            }
            if ( image )
            {
                /*
                printf("display = %x\n"
                        "xv_port = %d\n"
                        "drawable = %x\n"
                        "gc = %x\n"
                        "image =%x\n"
                        "x=%d,y=%d,w=%d,h=%d\n"
                        "x=%d,y=%d,w=%d,h=%d\n",
                        g_display,g_xv_port,hwnd, gc, image ,
                        x,y,w,h,
                        region->top_left_x,region->top_left_y,region->width,region->height
                      );
                 */
#if NO_USE_SHM
                  XvPutImage(g_display,g_xv_port,hwnd, gc, image ,
                            x,y,w,h,
                            region->top_left_x,region->top_left_y,region->width,region->height);
#else
                  XvShmPutImage(g_display,g_xv_port,hwnd, gc, image ,
                            x,y,w,h,
                            region->top_left_x,region->top_left_y,region->width,region->height,
                            False);
#endif
                XSync(g_display,False);

                if (preview_cb != NULL)
                    preview_cb(g_display, hwnd);

                XSync(g_display,False);
            }
        }
        return SDVR_ERR_NONE;
    }
    else
    {
        return SDVR_ERR_UI_NO_INIT;
    }
}

unsigned long get_color_pixel(sdvr_ui_color_key_t rgb_color)
{
    /*
    Colormap    cm;
    XColor      color;
    int snum = XDefaultScreen(g_display);
    cm = XDefaultColormap(g_display,snum);
    color.red = sdvr_get_r(rgb_color);
    color.red = color.red << 8;
    color.green = sdvr_get_g(rgb_color);
    color.green = color.green << 8;
    color.blue = sdvr_get_b(rgb_color);
    color.blue = color.blue << 8;
    XAllocColor(g_display,cm,&color);
    return color.pixel;
    */
    unsigned long r = sdvr_get_r(rgb_color);
    unsigned long g = sdvr_get_g(rgb_color);
    unsigned long b = sdvr_get_b(rgb_color);
    return (r << 16) | (g <<8) | (b);
}

sdvr_err_ui_e clear_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region)
{
    GC        gc;
    int     snum;
    int kp;

    if (g_display == 0)
    {
        return SDVR_ERR_UI_NO_INIT;
    }
    kp = get_color_pixel(m_keyColor);
    snum = XDefaultScreen(g_display);
    gc = XDefaultGC(g_display,snum);
    XSetForeground(g_display,gc,kp);
    XFillRectangle(g_display,hwnd,gc,region->top_left_x,region->top_left_y,region->width,region->height);
    XSync(g_display,False);
    return SDVR_ERR_NONE;
}

void draw_set_key_color( sdvr_ui_color_key_t color_key )
{
    m_keyColor = color_key;
}

sdvr_err_ui_e draw_frame(sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t* region, sdvr_ui_color_key_t rgb_color, int line_style)
{
    GC        gc;
    XPoint xp[5];
    unsigned long pixel;
    int ls;
    int snum;

    if (g_display == 0)
    {
        return SDVR_ERR_UI_NO_INIT;
    }
    snum = XDefaultScreen(g_display);
    gc = XDefaultGC(g_display,snum);
    xp[0].x=region->top_left_x;
    xp[0].y=region->top_left_y;
    xp[1].x=region->width + xp[0].x;
    xp[1].y=xp[0].y;
    xp[2].x=xp[1].x;
    xp[2].y=xp[1].y+region->height;
    xp[3].x=xp[0].x;
    xp[3].y=xp[2].y;
    xp[4].x=xp[0].x;
    xp[4].y=xp[0].y;
    switch( line_style)
    {
        case SDVR_UI_LS_DASH:
            ls = LineDoubleDash;
            break;
        case SDVR_UI_LS_DOT:
            ls = LineOnOffDash;
            break;
        default:
            ls = LineSolid;
            break;
    }
    pixel = get_color_pixel(rgb_color);
    //printf("draw %x %x %x(%d,%d)\n",hwnd,rgb_color, pixel,xp[0].x/region->width,xp[0].y/region->height);
    if ( ls != LineSolid )
    {
        unsigned long kp = get_color_pixel(m_keyColor);
        XSetForeground(g_display,gc,kp);
        XSetLineAttributes(g_display,gc,1,LineSolid,CapNotLast,JoinMiter);
        XDrawLines(g_display,hwnd,gc,xp,5,CoordModeOrigin);
    }
    XSetForeground(g_display,gc,pixel);
    XSetLineAttributes(g_display,gc,1,ls,CapNotLast,JoinMiter);
    XDrawLines(g_display,hwnd,gc,xp,5,CoordModeOrigin);
    XSync(g_display,False);
    return SDVR_ERR_NONE;
}

