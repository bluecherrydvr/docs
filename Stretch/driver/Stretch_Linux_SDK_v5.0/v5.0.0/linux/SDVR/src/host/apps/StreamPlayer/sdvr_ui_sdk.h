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
#ifndef SDVR_UI_SDK_H
#define SDVR_UI_SDK_H

#ifdef WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#endif
#include "sdvr_sdk.h"

/****************************************************************************
  PACKAGE: Stretch DVR Display SDK (sdvr_ui_sdk)

  DESCRIPTION:

  SECTION: Include
  {
  #include "sdvr_ui_sdk.h"
  }

  SECTION: Introduction
  In conjunction with the Stretch DVR SDK, there is a UI SDK library that lets you
  display raw video frames on the display monitor.

  This is an optional library that can be added to your Stretch DVR Application
  development environment.  If you are developing an embedded  DVR
  Application or choose to use a different
  display API, you may choose not to include this library.

  This document describes the Application Programming Interface (API)
  implemented in the UI SDK.

  The SDK provides the ability to:

    Display the specified raw video frame into one or more regions
    within a given window.

    Display a raw video frame in a rectangle within a display window.

    Clear the contents of a rectangle within a display window.

  SECTION: Linking with UI SDK API in MS Window Platform
  This library uses DirectX functions to draw directly into the
  video buffer. As result, ddraw.lib and dxguid.lib must be added to your development environment.
  (ddraw.lib and ddraw.dll are shipped as part of MSVC++ SDK.) Additionally, you need to
  add "<program files>/Microsoft DirectX SDK (March 2008)\Lib\x86" to your library path and
  "<program files>/Microsoft DirectX SDK (March 2008)\Include" to your include path.
  Lastly, make sure WIN32 is added to your C/C++ preprocessor defintions.

  You can choose to link with UI SDK API statically or dynamically.

  1) Statically Linking - You must add sdvr_sdk.lib and sdvr_ui_sdk.lib to your
  library dependecies.

  2) Dynamically Linking - To do so, you must add sdvr_ui_sdk_dll.lib
  and sdvr_sdk_dll.lib to your library dependecies. Additionally, you must
  include sdvr_ui_sdk_dll.dll and sdvr_sdk_dll.dll in the same directory as
  your DVR Application.

  SECTION: Linking with UI SDK API in Linux Platform
  This library requires an X Server that supports Xv extensions
  to display live video. As result, in addition to sdvr_sdk.lib and sdvr_ui_sdk.lib,
  you must include the following libraries to your  development environment:

  1) CentOS 5 - libXv-devel
  2) Ubuntau 7.10 - libxv-dev

  SECTION: Using the UI SDK API
  This section provides the steps required to display raw video frames on
  the display screen.

    You must provide the SDK the largest video frame size that would ever be
    displayed. This size is passed as parameters to sdvr_ui_init(). This function
    must be called before any of the display functions can be called and should only
    be called once within your application.

    The color key defines one specific color that should not
    be drawn to the  render surface.
    After initializing the UI SDK, you can specify the color key by calling
    sdvr_ui_set_key_color().  This is an optional step. The SDK uses white
    as the default color key.

    There are two different methods that you can choose in order to display raw
    video frames.

    Method 1: This method is used if you don't need to access the YUV buffers to peform any video
    analytics, save into a file, or any other operations on raw video frames. To
    start raw video display on a specific region within a window, call
    sdvr_ui_start_video_preview(). To stop raw video display, call
    sdvr_ui_stop_video_preview(). In cases where you are interested in displaying
    the raw video frames from one channel in different regions within a preview window,
    you should call sdvr_ui_start_video_preview() multiple times for each region.
    Once sdvr_ui_start_video_preview() is called for a specific
    encoder or decoder channel, the SDK starts displaying any raw video frame received
    from the DVR board into all the regions within the given window handle. You direct
    the SDK to stop displaying raw video for each region by calling sdvr_ui_stop_video_preview().
    It is also possible to get notificatioin with every video frame by setting a
    preview callback. With the preview callback you can put additional graphics
    over the video, such as highlighting some regions, and so on. To set the preview
    callback call sdvr_ui_set_preview_callback().

    Method 2: This method is used if you need to have more control over displaying the video frames.
    To display YUV frames, get the YUV buffers by calling sdvr_get_yuv_buffer().
    In general, you can either poll this function or wait to be called back in your
    video frame callback. After you aquire the YUV buffer, set the
    buffer by calling sdvr_ui_set_yuv_buffer() for displaying. Once a YUV buffer is
    set to be displayed, you can call sdvr_ui_draw_yuv() to draw the buffer
    into a specific region within a display window. There is no need to call
    sdvr_ui_set_yuv_buffer() multiple times to display the same
    video frame in different regions of the screen.

  SECTION: Important Restrictions
  Note the following restrictions when using the UI SDK. These
  restrictions, apart from those explicitly noted, are not permanent
  and will be removed in future versions of the SDK.

    You must call sdvr_ui_refresh() in your application's paint message handler
    if your application can be covered by any other applications
    while there are still video frames displaying. This function only applies
    when using Method 2 for display. (See section "Using the UI SDK API")
    This is a permanent restriction.

    The screen must not be locked while video is being displayed. Screen
    savers must not be activated while video is being displayed. Either of
    these actions can cause the display to stop refreshing. If this happens,
    the application must be shut down and restarted.

    There are only four display regions per channel
    when using sdvr_ui_start_video_preview() to display raw video frames.

    Dynamic linking of the display and board SDK libraries is only
    available in the MS Windows enviornment.

    DirectDraw acceleration is required to be enabled on your video card.

  SECTION: Example of a callback function:
  {
    void av_frame_callback(sdvr_chan_handle_t handle,
       sdvr_frame_type_e frame_type, sx_bool is_primary)
    {
        sdvr_av_buffer_t  *av_buffer;
        sdvr_yuv_buffer_t *yuv_buf;
        switch (frame_type)
        {
        case SDVR_FRAME_AUDIO_ENCODED:
            if (sdvr_get_av_buffer(handle, SDVR_FRAME_AUDIO_ENCODED,
                                   &av_buffer) == SDVR_ERR_NONE)
            {
                  <add the buffer to an audio queue to be processed from
                  a different thread>
                  <The buffer should be released from that thread when
                  you processed the audio frame.>
            }
            break;
        case SDVR_FRAME_RAW_VIDEO:
            if (sdvr_get_yuv_buffer(handle, &yuv_buf) == SDVR_ERR_NONE)
            {
                sdvr_ui_region_t ui_region;

                // set the YUV buffer so that we can display it within
                // any rectangle in any window handle.

                sdvr_ui_set_yuv_buffer(yuv_buf);

                // following code checks to see where this new
                // channel handle needs to be displayed.
                for (int screen = 0; screen < numScreens; ++screen)
                {
                    if (m_settings->displayChannel(screen) == handle)
                    {
                        sdvr_ui_draw_yuv(m_displayRect[screen].hWnd,&m_displayRect[screen].region);
                    }
                }

                // Once you are done displaying the frame,
                // you must release it.
                sdvr_release_yuv_buffer(yuv_buf);
            }
            break;
        case SDVR_FRAME_H264_IDR:
        case SDVR_FRAME_H264_I:
        case SDVR_FRAME_H264_P:
        case SDVR_FRAME_H264_B:
        case SDVR_FRAME_H264_SPS:
        case SDVR_FRAME_H264_PPS:
        case SDVR_FRAME_JPEG:
        case SDVR_FRAME_MPEG4_I:
        case SDVR_FRAME_MPEG4_P:
        case SDVR_FRAME_MPEG4_VOL:
            if (sdvr_get_av_buffer(handle,
              is_primary ? SDVR_FRAME_VIDEO_ENCODED_PRIMARY :
              SDVR_FRAME_VIDEO_ENCODED_SECONDARY, &av_buffer) == SDVR_ERR_NONE)
            {
                  <add the buffer to a video encoded queue to be processed
                  from a different thread.>
                  <The buffer should be released from that thread when
                  you processed the video frame.>
            }
            break;
        }
        NOTE: Any buffer that is not retrieved(i.e. no call to the
              "sdvr_get_xxx_buffer") will be freed by the SDK once
              the frame queue is full.
    }

  }
*****************************************************************************/

/****************************************************************************
  VISIBLE: Typedef for the errors returned by the UI SDK.

    SDVR_ERR_UI_INIT - Error code for failure to initialize the UI.

    SDVR_ERR_UI_INVALID_PARAM - Error code if the any of the parameters
    passed to any of the SDVR UI function is invalid.

    SDVR_ERR_UI_SET_BUFFER - Error code for failure to lock the draw surface.

    SDVR_ERR_UI_DRAW_YUV - Error code for failure to draw the surface.

    SDVR_ERR_UI_INVALID_YUV_BUF - Error code if the given YUV buffer is
    not valid.

    SDVR_ERR_UI_VIDEO_SIZE - Error code if the YUV buffer size is larger
    than the maximum supported video frame size.

    SDVR_ERR_UI_NO_BUFFER - Error code if no buffer was set before calling
    sdvr_ui_draw_yuv().

    SDVR_ERR_UI_MAX_PREVIEW_REGIONS - Error code if no more preview regions
    can be added.

    SDVR_ERR_UI_DRAWINFO_FULL - Error code if the maximum number preview
    channel is reached.

    SDVR_ERR_UI_CHANNEL_NOT_START - Error code if the preview was not started
    for the specified channel handle.

    SDVR_ERR_UI_NO_INIT - Error code if sdvr_ui_init() was not called prior
    calling this API.

    SDVR_ERR_UI_MEMORY - Error code if failed to allocate enough memory for
    the internal data structures.

****************************************************************************/
typedef enum _sdvr_err_ui_e {
    SDVR_ERR_UI_INIT = 3000,
    SDVR_ERR_UI_INVALID_PARAM,
    SDVR_ERR_UI_SET_BUFFER,
    SDVR_ERR_UI_DRAW_YUV,
    SDVR_ERR_UI_INVALID_YUV_BUF,
    SDVR_ERR_UI_VIDEO_SIZE,
    SDVR_ERR_UI_NO_BUFFER,
    SDVR_ERR_UI_MAX_PREVIEW_REGIONS,
    SDVR_ERR_UI_DRAWINFO_FULL,
    SDVR_ERR_UI_CHANNEL_NOT_START,
    SDVR_ERR_UI_NO_INIT,
    SDVR_ERR_UI_MEMORY
} sdvr_err_ui_e;
/****************************************************************************
  SDVR_UI_ERR_NONE - Error coder if no error.
****************************************************************************/
#define SDVR_UI_ERR_NONE  ((sdvr_err_ui_e)SDVR_ERR_NONE)

/************************************************************************
    VISIBLE: It is possible to display raw video frames for each encoder
    or decoder channel in multiple regions within a given window handle.
    SDVR_MAX_DRAW_REGIONS defines the maximum number of display regions
    that can be added to the display preview regions.
************************************************************************/
#define SDVR_MAX_DRAW_REGIONS   4

/************************************************************************
    VISIBLE: This data structure is used to define a display region.
    A display region is the area within a window in which the video frame
    is displayed. The top left corner of a window is specified by (0,0).
    Each window can include many of such regions.

        @top_left_x@ - X-coordinate of the upper left corner.

        @top_left_y@ - Y-coordinate of the upper left corner.

        @width@ - The width of display region within the given window.

        @height@ - The height of display region  within the given window.
************************************************************************/
typedef struct _sdvr_ui_region_t {
    sx_uint32 top_left_x;
    sx_uint32 top_left_y;
    sx_uint32 width;
    sx_uint32 height;
} sdvr_ui_region_t;

/************************************************************************
    VISIBLE: The window handle for where to display the raw video frames. In
    MS Windows environment  this is HWND.
************************************************************************/
#ifdef WIN32
typedef HWND sdvr_ui_hwnd_t;
#else
typedef Drawable sdvr_ui_hwnd_t;
#endif

/************************************************************************
    VISIBLE: The color key defines one specific color, for example white,
    which should not be drawn to the render surface. The color key usually
    is defined by the RGB macro.
************************************************************************/
#ifdef WIN32
typedef DWORD sdvr_ui_color_key_t;
#else
typedef unsigned long sdvr_ui_color_key_t;
#endif

/************************************************************************
    VISIBLE: A macro to construct an RGB color scheme.
************************************************************************/
#ifdef WIN32
#define sdvr_rgb(r,g,b)    RGB(r,g,b)
#else
#define sdvr_rgb(r,g,b)    (((unsigned long)(unsigned char)(r))|(((unsigned long)(unsigned char)(g)) << 8)|(((unsigned long)(unsigned char)(b)) << 16))
#endif

/************************************************************************
    VISIBLE: Preview callback function type.
************************************************************************/
#ifdef WIN32
typedef void (*sdvr_ui_preview_cb)(HDC);
#else
typedef void (*sdvr_ui_preview_cb)(Display *dpy, Drawable d);
#endif

/* Pen Styles */
/************************************************************************
    VISIBLE: The line style to use to draw a frame around a
    region.

    @SDVR_UI_LS_SOLID@ - Use solid lines.  ____

    @SDVR_UI_LS_DASH@ - Use dashed lines.  -----

    @SDVR_UI_LS_DOT@ - Use dotted lines.   .....
************************************************************************/
#define SDVR_UI_LS_SOLID            0       /* _______  */
#define SDVR_UI_LS_DASH             1       /* -------  */
#define SDVR_UI_LS_DOT              2       /* .......  */


/*******************************************************************************
   A necessary evil introduced for C++ compatibility.  C source files must
   not declare a function "extern"; instead, they must declare the function
   "EXTERN".  For example:
   {
       EXTERN void my_external_symbol(int a, double f);
   }
   This specifies that the function has C linkage so that it can be used
   when compiled with a C++ compiler.
*******************************************************************************/
#if defined(__cplusplus)
   #define EXTERN              extern "C"
#else
   #define EXTERN              extern
#endif

EXTERN sdvr_err_ui_e sdvr_ui_init(sx_uint32 max_width, sx_uint32 max_lines);
EXTERN sdvr_err_ui_e sdvr_ui_close();
EXTERN void sdvr_ui_set_key_color( sdvr_ui_color_key_t color_key);
EXTERN sdvr_err_ui_e sdvr_ui_set_yuv_buffer(sdvr_yuv_buffer_t *yuv_buffer);
EXTERN sdvr_err_ui_e sdvr_ui_draw_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region);
EXTERN sdvr_err_ui_e sdvr_ui_clear_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region);
EXTERN sdvr_err_ui_e sdvr_ui_draw_frame(sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t region, sdvr_ui_color_key_t rgb_color, int line_style);
EXTERN void sdvr_ui_version(sx_uint8 *major, sx_uint8 *minor, sx_uint8 *revision, sx_uint8 *build);
EXTERN sdvr_err_ui_e sdvr_ui_start_video_preview(sdvr_chan_handle_t handle,
                                                 sdvr_ui_hwnd_t wnd_handle,
                                                 sdvr_ui_region_t *region,
                                                 int *preview_id);
EXTERN sdvr_err_ui_e sdvr_ui_stop_video_preview( sdvr_chan_handle_t h_channel_handle,
                                                 int preview_id);

EXTERN sdvr_err_ui_e sdvr_ui_set_preview_callback(sdvr_chan_handle_t h_channel_handle,
												  int preview_id,
												  sdvr_ui_preview_cb cb);
EXTERN void sdvr_ui_refresh();

#endif /* SDVR_UI_SDK_H */



