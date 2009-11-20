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
#ifndef STRETCH_YUVDRAW_H
#define STRETCH_YUVDRAW_H

#ifdef WIN32
#include <ddraw.h>
#else
#include <X11/extensions/Xvlib.h>
#include <X11/extensions/XShm.h>
#include <pthread.h>
#endif

#define MAX_CHANNEL_DRAW_INFO   32
#define INVALID_HWND    0

typedef struct _draw_info {
    sdvr_ui_hwnd_t hwnd;
    sdvr_ui_region_t rect;
	sdvr_ui_preview_cb preview_cb;
}draw_region_info_t;

#ifdef WIN32    
    typedef LPDIRECTDRAWSURFACE sdvr_ui_surface;
	typedef HANDLE				os_umutex_t;
	#define OS_UMUTEX_CREATE(m) m = CreateMutex(NULL,FALSE,NULL)
    #define OS_UMUTEX_LOCK(m)   WaitForSingleObject( m, INFINITE )
    #define OS_UMUTEX_TRY_LOCK(m)   ((WaitForSingleObject( m, 0 )==WAIT_OBJECT_0)?0:-1)
    #define OS_UMUTEX_UNLOCK(m) ReleaseMutex( m )
    #define OS_UMUTEX_DESTROY(m) CloseHandle(m) 
#else
    struct  sdvr_ui_surface_struct_t
            {
                XvImage*    image;
                XShmSegmentInfo  shm;
            };
    typedef  struct sdvr_ui_surface_struct_t * sdvr_ui_surface;
    typedef  pthread_mutex_t    os_umutex_t;
    #define OS_UMUTEX_CREATE(m) pthread_mutex_init(&(m),NULL)
    #define OS_UMUTEX_LOCK(m)   pthread_mutex_lock(&(m))
    #define OS_UMUTEX_TRY_LOCK(m)   pthread_mutex_trylock(&(m))
    #define OS_UMUTEX_UNLOCK(m) pthread_mutex_unlock(&(m))
    #define OS_UMUTEX_DESTROY(m) pthread_mutex_destroy(&(m))
#endif    

typedef struct _channel_draw_info {
    sdvr_chan_handle_t  handle;
    sdvr_ui_surface save_surface;
    int     s_width;                // widht of the video frame
    int     s_height;               // height of the video frame
    int     reg_count;              // number of regions specified.
    draw_region_info_t   di[SDVR_MAX_DRAW_REGIONS];
} channel_draw_info_t;

extern sx_uint32 m_max_width;
extern sx_uint32 m_max_height;
sdvr_ui_surface create_sdvr_ui_surface( int width, int height );
void release_sdvr_ui_surface( sdvr_ui_surface os);
sdvr_err_ui_e draw_set_yuv_buffer(sdvr_ui_surface surface, char * y, char * u, char *v, unsigned int width, unsigned int height);
sdvr_err_ui_e draw_yuv(sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region,
					   sdvr_ui_surface src_surface, sdvr_ui_region_t *src_region,
					   sdvr_ui_preview_cb preview_cb);
sdvr_err_ui_e clear_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region);
sdvr_err_ui_e draw_frame(sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region, sdvr_ui_color_key_t rgb_color, int line_style);
sdvr_err_ui_e init_draw(sx_uint32 max_width, sx_uint32 max_lines);
sdvr_err_ui_e close_draw();
void draw_set_key_color( sdvr_ui_color_key_t color_key );


#endif //STRETCH_YUVDRAW_H

