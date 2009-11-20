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
#include <dvr_common.h>
#include <sct.h>
#include "sdk.h"
#include "sdvr_ui_sdk_version.h"
#include "yuvdraw.h"

/* A flag to indicate if sdvr_ui_init() was called. */
static int  m_init_ok = 0;

/* Number of channel currently displaying. */
static int  m_num_disp_chan = 0;

os_umutex_t g_draw_lock;


/*************************************************************************
                    Internal Functions
*************************************************************************/
static int _add_draw_info( channel_draw_info_t * pdi, sdvr_ui_hwnd_t wnd_handle, sdvr_ui_region_t *region);
static void _alloc_save_surface( channel_draw_info_t * pdi, int width, int height);
static void _stop_all_displays();

int _add_draw_info( channel_draw_info_t * pdi, sdvr_ui_hwnd_t wnd_handle, sdvr_ui_region_t *region)
{
    int i;
    if (pdi->reg_count >= SDVR_MAX_DRAW_REGIONS)
    {
        return -1;
    }
    for(i = 0; i< SDVR_MAX_DRAW_REGIONS; i++)
    {
        if (pdi->di[i].hwnd == INVALID_HWND)
        {
            pdi->di[i].hwnd = wnd_handle;
            pdi->di[i].rect = *region;
            pdi->reg_count++;
            break;
        }
    }
    if ( i< SDVR_MAX_DRAW_REGIONS)
    {
        return i;
    }
    else
        return -1;
}

void _alloc_save_surface( channel_draw_info_t * pdi, int width, int height)
{
    if (pdi->save_surface)
    {
        if ( (pdi->s_width != width) || (pdi->s_height != height) )
        {
            release_sdvr_ui_surface(pdi->save_surface);
            pdi->save_surface = create_sdvr_ui_surface(width,height);
            pdi->s_width = width;
            pdi->s_height = height;
        }
    }
    else
    {
        pdi->save_surface = create_sdvr_ui_surface(width,height);
        pdi->s_width = width;
        pdi->s_height = height;
    }
}

/****************************************************************************
    This function checks to see if any channel is displaying its video,
    if so, it delete all the display data structure.

    NOTE: This function does not actaully disable the raw video streaming.
****************************************************************************/
void _stop_all_displays()
{
    int chan_num;
    int board_index;
    sdvr_err_e err;
    sdvr_chan_handle_t handle;
    channel_draw_info_t * pdi;
    int max_enc_chan_num;
    int max_dec_chan_num;

    if (!m_init_ok || !m_num_disp_chan)
        return;

    OS_UMUTEX_LOCK(g_draw_lock);
    for (board_index = 0; board_index < MAX_PCI_BOARDS && m_num_disp_chan; board_index++)
    {
        err = ssdk_validate_board_ndx(board_index);
        if (err == SDVR_ERR_INVALID_BOARD)
            break;  // We reached to the end of DVR installed boards
        if (err == SDVR_ERR_BOARD_NOT_CONNECTED)
            continue;   // This board is not connected

        /* Stop all the encoder and HMO-only channels displays*/
        max_enc_chan_num = ssdk_get_max_chan_num(board_index, SDVR_CHAN_TYPE_ENCODER);

        for (chan_num=0; chan_num < max_enc_chan_num &&
            m_num_disp_chan; chan_num++)
        {
            handle = ssdk_create_chan_handle(board_index,
                                     SDVR_CHAN_TYPE_ENCODER,
                                     chan_num);
            err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER);
            if (err == SDVR_ERR_INVALID_CHAN_HANDLE)
                continue;

            pdi = (channel_draw_info_t *)ssdk_get_ui_context(handle);

            if (pdi)
            {
                if (pdi->save_surface)
                    release_sdvr_ui_surface(pdi->save_surface);
//               free((void *)pdi);
                ssdk_free_ui_context((void *)pdi);
                ssdk_set_ui_callback( handle,NULL,NULL);
                m_num_disp_chan--;
            }
        } //for (chan_num=0; i < MAX_CHANNEL_DRAW_INFO; chan_num++)

        /* Stop all the decoder channels displays*/
        max_dec_chan_num = ssdk_get_max_chan_num(board_index, SDVR_CHAN_TYPE_DECODER);

        for (chan_num=0; chan_num < max_dec_chan_num &&
             m_num_disp_chan; chan_num++)
        {
            handle = ssdk_create_chan_handle(board_index,
                                     SDVR_CHAN_TYPE_DECODER,
                                     chan_num);
            err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_DECODER) ;
            if (err == SDVR_ERR_INVALID_CHAN_HANDLE)
                continue;

            pdi = (channel_draw_info_t *)ssdk_get_ui_context(handle);

            if (pdi)
            {
                if (pdi->save_surface)
                    release_sdvr_ui_surface(pdi->save_surface);
//                free((void *)pdi);
                ssdk_free_ui_context((void *)pdi);
                ssdk_set_ui_callback( handle,NULL,NULL);
                m_num_disp_chan--;
            }
        } //for (chan_num=0; i < MAX_CHANNEL_DRAW_INFO; chan_num++)

    } //for (board_index = 0; board_index < MAX_PCI_BOARDS; board_index++)
    OS_UMUTEX_UNLOCK(g_draw_lock);
}


/****************************************************************************
  This function initializes video surface and sets up the maximum supported
  video frame size. (i.e., sdvr_ui_init(720, 480) sets the largest video frame
  size in your system as NTSC D1.)

  This function must be called before any of the display functions
  and should be called only once per session.

  Parameters:

    @max_width@ - The maximum width video frame that will ever be displayed.

    @max_lines@ - The maximum number of lines in a video frame that will ever
    be displayed.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_ui_e sdvr_ui_init(sx_uint32 max_width, sx_uint32 max_lines)
{
    sdvr_err_ui_e err;

    OS_UMUTEX_CREATE(g_draw_lock);

    err = init_draw(max_width,max_lines);
    m_init_ok = (err == SDVR_UI_ERR_NONE);
	return err;
}

/****************************************************************************
  This function closes the UI SDK and frees up the system
  resources used by it. You must call this function prior to exiting
  your DVR Application for a clean shutdown of your system.
  No other UI API function calls, except sdvr_ui_init(), are allowed after
  this function is called.

  Parameters:

    None.

  Returns:

    SDVR_ERR_NONE - On success.
****************************************************************************/

sdvr_err_ui_e sdvr_ui_close()
{
    if (!m_init_ok)
        return SDVR_ERR_UI_NO_INIT;

    _stop_all_displays();

    OS_UMUTEX_DESTROY(g_draw_lock);
	return close_draw();
}


/****************************************************************************
  The color key defines one specific color that should not be drawn to the
  render surface. The color key is used to set the background window color
  when clearing a region. The default color key is white.

  @NOTE: Color key is not supported by all the video cards.@

  Parameters:

    @color_key@ - The new color key. It is usually defined by the sdvr_rgb macro.


  Returns:

    None.
****************************************************************************/
void sdvr_ui_set_key_color( sdvr_ui_color_key_t color_key)
{
    if (!m_init_ok)
        return;

    draw_set_key_color( color_key);
}

/****************************************************************************
  Before a video frame can be displayed, you must specify a raw video frame in
  the format of 4-2-0. The same buffer is used every time sdvr_ui_draw_yuv() is
  called. This means that the same video frame can be displayed in multiple window regions.
  To display the next video frame, you must call this function again with a new
  yuv_buffer. In general you get YUV buffers by calling sdvr_get_yuv_buffer()
  in the video frame callback, as you are notified when YUV buffers arrive.

  Parameters:

    @yuv_buffer@ - A pointer to a YUV buffer structure.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_ui_e sdvr_ui_set_yuv_buffer(sdvr_yuv_buffer_t *yuv_buffer)
{
    sdvr_chan_handle_t handle;
    ssdk_yuv_malloc_item_t *interanl_yuv_buffer = (ssdk_yuv_malloc_item_t *)yuv_buffer;

    /*******************************************
       Validate the input parameters
    ********************************************/
    if (!m_init_ok)
        return SDVR_ERR_UI_NO_INIT;

    if (!yuv_buffer)
        return SDVR_ERR_UI_INVALID_YUV_BUF;

    if (interanl_yuv_buffer->signiture != 0xFEEDBEEF)
    {
        return SDVR_ERR_UI_INVALID_YUV_BUF;
    }

    handle = sdvr_get_buffer_channel(yuv_buffer);

    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
        return SDVR_ERR_UI_INVALID_YUV_BUF;
 
	return draw_set_yuv_buffer(NULL,yuv_buffer->y_data,yuv_buffer->u_data,
        yuv_buffer->v_data,yuv_buffer->active_width,yuv_buffer->active_height);
}

/****************************************************************************
  This function displays a YUV video frame that was previously set
  by sdvr_ui_set_yuv_buffer() in the given region within the given
  window handle.

  The specified region must be within the maximum video frame width and
  lines that were set by the call to sdvr_ui_init().

  Parameters:

    @hwnd@ - The window handle to display the video frame.

    @region@ - The rectangle within the given window handle in which to
    display the video frame.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_ui_e sdvr_ui_draw_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region )
{
    if (!m_init_ok)
        return SDVR_ERR_UI_NO_INIT;

    return draw_yuv(hwnd,region,NULL,NULL, NULL);
}

/****************************************************************************
  This function uses the previously defined color key that was set by
  calling sdvr_ui_set_key_color() to clear the contents of the given
  rectangle within the given window handle.

  The specified region must be within the maximum video frame width and
  lines that were set by the call to sdvr_ui_init().

  Parameters:

    @hwnd@ - The window handle to clear the video frame.

    @region@ - The rectangle within the given window handle to
    clear.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_ui_e sdvr_ui_clear_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region)
{
    sdvr_err_ui_e err;

    if (!m_init_ok)
        return SDVR_ERR_UI_NO_INIT;

    OS_UMUTEX_LOCK(g_draw_lock);
    err = clear_yuv(hwnd,region);
    OS_UMUTEX_UNLOCK(g_draw_lock);
    return err;
}

/****************************************************************************
  Use this function to draw a frame around the given region within the
  given window.

  You can specify the frame color or the line style.

  Parameters:

    @hwnd@ - The current window handle.

    @region@ - The rectangle within the given window handle around which to
    draw a frame.

    @rgb_color@ - The line frame color.

    @line_style@ - The line frame style. (i.e., SDVR_UI_LS_SOLID)

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_ui_e sdvr_ui_draw_frame(sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t region, sdvr_ui_color_key_t rgb_color, int line_style)
{
    sdvr_err_ui_e err;

    if (!m_init_ok)
        return SDVR_ERR_UI_NO_INIT;

    OS_UMUTEX_LOCK(g_draw_lock);
    err = draw_frame(hwnd,&region,rgb_color,line_style);
    OS_UMUTEX_UNLOCK(g_draw_lock);
    return err;
}

/****************************************************************************
  This function returns the display SDK version.

  Stretch follows the convention of using four numbers for version
  control. A change in the major number indicates major changes to
  functionality, a change in the minor number indicates minor
  changes to functionality, and a change in the revision number indicates
  significant bug fixes that were introduced in the minor change functionality.
  A change to the build number indicates only bug fixes that do not
  change functionality.

  This function can be called before or after sdvr_ui_init();

  Parameters:

    "major" - Pointer to a variable that will hold the major version of the
    display SDK when this function returns.

    "minor" - Pointer to a variable that will hold the minor version of
    the display SDK when this function returns.

    "revision" - Pointer to a variable that will hold the revision version of the
    display SDK when this function returns.

    "build" - Pointer to a variable that will hold the build or bug fix
    version of the display SDK when this function returns.

  Returns:

    Nothing.
****************************************************************************/
void sdvr_ui_version(sx_uint8 *major,
                          sx_uint8 *minor,
                          sx_uint8 *revision,
                          sx_uint8 *build)
{
    if (major != NULL && minor != NULL && revision != NULL && build != NULL)
    {
        *major = SDVR_UI_SDK_VERSION_MAJOR;
        *minor = SDVR_UI_SDK_VERSION_MINOR;
        *revision = SDVR_UI_SDK_VERSION_REVISION;
        *build = SDVR_UI_SDK_VERSION_BUILD;
    }
    return;
}

void ui_callback(sdvr_chan_handle_t handle, dvr_data_header_t *yuvHdr, void * context)
{
    channel_draw_info_t * pdi = (channel_draw_info_t *) context;
    sx_uint8 board_index;
    sx_uint8 chan_num;

    chan_num = CH_GET_CHAN_NUM(handle);
    board_index = CH_GET_BOARD_ID(handle);

    if ( !pdi ||
        yuvHdr == 0 || !m_init_ok ||
        (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE) ||
        (handle != pdi->handle))
        return;

    if ( OS_UMUTEX_TRY_LOCK ( g_draw_lock ) == 0)
    {
        if (pdi->reg_count > 0 )
        {
            sx_uint8 *yBuffer = (sx_uint8 *)yuvHdr + yuvHdr->y_data_offset;
            sx_uint8 *uBuffer = (sx_uint8 *)yuvHdr + yuvHdr->u_data_offset;
            sx_uint8 *vBuffer = (sx_uint8 *)yuvHdr + yuvHdr->v_data_offset;

            int i;
            int drawn = 0;
            {
                _alloc_save_surface(pdi,yuvHdr->active_width,yuvHdr->active_height);
            }

            draw_set_yuv_buffer(pdi->save_surface,yBuffer,uBuffer,vBuffer,pdi->s_width,pdi->s_height);

            for( i=0; (i< SDVR_MAX_DRAW_REGIONS) && (drawn <pdi->reg_count); i++)
            {
                if (pdi->di[i].hwnd != INVALID_HWND)
                {
					draw_yuv(pdi->di[i].hwnd, &(pdi->di[i].rect),
							pdi->save_surface, NULL, pdi->di[i].preview_cb);
                    drawn++;
                }
            }
        }
        OS_UMUTEX_UNLOCK(g_draw_lock);
    } //if ( OS_UMUTEX_TRY_LOCK ( g_draw_lock ) == 0)
}

/****************************************************************************
  You can request the SDK to display video frames directly into one or more
  regions within a given window handle. Each consecutive call to this function
  causes
  the given display region within the given window handle to be added to the
  preview lists. There is a unique preview ID returned for each region that
  can be used to stop the preview on it by calling sdvr_ui_stop_video_preview().

  This function does not enable video streaming. You must call sdvr_stream_raw_video()
  to enable streaming of the raw video frames for the specific encoder or decoder
  channel. Then, if this function is called, the corresponding video frame will be
  displayed in the given regions.

  Parameters:

    @handle@ - An encoder or decoder channel to display its raw video frames.

    @wnd_handle@ - The window handle to display the video frames.

    @region@ - The rectangle within the given window handle to
    display the video frames. You can add up to SDVR_MAX_DRAW_REGIONS
    regions by calling this function multiple times. Each region
    displays the same raw video frame for the given encoder or
    decoder handle.

    @preview_id@ - A pointer to the variable holding the ID for the region
    that is added successfully.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remark:

    You still can call sdvr_get_yuv_buffer() to retrieve raw video frames for
    further processing if it is needed.

    @NOTE:@ : For decoder channels, you must send each encoded frame to
    the board to decode them to receive raw video frames.
****************************************************************************/
sdvr_err_ui_e sdvr_ui_start_video_preview(sdvr_chan_handle_t handle,
                                         sdvr_ui_hwnd_t wnd_handle,
                                         sdvr_ui_region_t *region,
                                         int *preview_id)
{
    int id;
    channel_draw_info_t * pdi;
    int chan_num = CH_GET_CHAN_NUM(handle);
    int board_index = CH_GET_BOARD_ID(handle);

    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE ||
        preview_id == NULL || region == NULL ||
        wnd_handle == 0)
        return SDVR_ERR_UI_INVALID_PARAM;

    if (!m_init_ok)
        return SDVR_ERR_UI_NO_INIT;

    /*
       If the no preview was started for this channel,
       set-up the UI callback.
    */
    pdi = (channel_draw_info_t *)ssdk_get_ui_context(handle);

    if (!pdi)
    {
//        pdi = (channel_draw_info_t *)malloc(sizeof(channel_draw_info_t));
        pdi = (channel_draw_info_t *)ssdk_malloc_ui_context(sizeof(channel_draw_info_t));
        if (pdi == NULL)
            return SDVR_ERR_UI_MEMORY;
        m_num_disp_chan++;
        pdi->handle = handle;
        {
            int j;
            pdi->reg_count = 0;
            pdi->save_surface = 0;
            pdi->s_width = 0;
            pdi->s_height = 0;
            for(j=0; j<SDVR_MAX_DRAW_REGIONS; ++j)
            {
                pdi->di[j].hwnd = INVALID_HWND;
				pdi->di[j].preview_cb = NULL;
            }
        }

        ssdk_set_ui_callback( handle,ui_callback,pdi);
    }

    OS_UMUTEX_LOCK(g_draw_lock);
    id = _add_draw_info(pdi,wnd_handle,region);
    OS_UMUTEX_UNLOCK(g_draw_lock);

	if (id < 0)
		return SDVR_ERR_UI_MAX_PREVIEW_REGIONS;

	*preview_id = id;
	return SDVR_ERR_NONE;
}

/****************************************************************************
  This function stops previewing of the raw video frames of the given encoder
  or decoder channel for the given region ID.

  Parameters:

    @handle@ - An encoder or decoder channel to display its raw video frames.

    @preview_id@ - The ID of the region to stop raw video frame previewing.
    This ID was returned from the call sdvr_ui_start_video_preview().

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remark:

    Calling this function will not stop the actual raw video streaming.
    Call sdvr_stream_raw_video to stop raw video streaming.
****************************************************************************/
sdvr_err_ui_e sdvr_ui_stop_video_preview(sdvr_chan_handle_t handle,
										 int preview_id)
{
    channel_draw_info_t * pdi;
    sdvr_err_ui_e err = SDVR_UI_ERR_NONE;
    int chan_num = CH_GET_CHAN_NUM(handle);
    int board_index = CH_GET_BOARD_ID(handle);

    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
        return SDVR_ERR_UI_INVALID_PARAM;

    if (!m_init_ok)
        return SDVR_ERR_UI_NO_INIT;

//    pdi = &m_chan_di[board_index][chan_num];
    pdi = (channel_draw_info_t *)ssdk_get_ui_context(handle);
    if (!pdi)
    {
        return SDVR_ERR_UI_CHANNEL_NOT_START;
    }
    OS_UMUTEX_LOCK(g_draw_lock);
    if (pdi->reg_count <= 0)
    {
        err = SDVR_ERR_UI_CHANNEL_NOT_START;
    }
    else if (preview_id < 0 || preview_id >= SDVR_MAX_DRAW_REGIONS)
    {
        err = SDVR_ERR_UI_CHANNEL_NOT_START;
    }
    else if (pdi->di[preview_id].hwnd == INVALID_HWND )
    {
        err = SDVR_ERR_UI_CHANNEL_NOT_START;
    }
    else
    {
        pdi->di[preview_id].hwnd = INVALID_HWND;
        pdi->reg_count--;
        if ( pdi->reg_count == 0)
        {
            if (pdi->save_surface)
                release_sdvr_ui_surface(pdi->save_surface);
//            free((void *)pdi);
            ssdk_free_ui_context((void *)pdi);
            ssdk_set_ui_callback( handle,NULL,NULL);
            m_num_disp_chan--;
        }
    }
    OS_UMUTEX_UNLOCK(g_draw_lock);
    return err;
}

/****************************************************************************
  Sets the preview callback function. The video preview must be started before
  calling this function. The preview callback function is executed with every video
  frame, making it possible to modify the content of the frame.
  
  Parameters:

    @handle@ - An encoder or decoder channel to display its raw video frames.

    @preview_id@ - The ID of the region for which the callback is set.

    @preview_cb@ - Pointer to the callback function or NULL to clear.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Callback function on Windows:

      void (*preview_cb)(HDC hdc);

    Executed before the frame is diplayed. Device context handle can be used
    for drawing on the frame. Do not release the device context. It is created
    by the UI SDK and will be released automatically.

  Callback function on Linux:

      void (*preview_cb)(Display *dpy, Drawable win);

    Executed after the video frame is rendered into the window. Display and
    Drawable identify the X window and can be used for drawing over the frame.

****************************************************************************/
sdvr_err_ui_e sdvr_ui_set_preview_callback(sdvr_chan_handle_t handle,
										   int preview_id,
										   sdvr_ui_preview_cb cb)
{
	channel_draw_info_t * pdi;

    if (!m_init_ok)
        return SDVR_ERR_UI_NO_INIT;

	if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE ||
		preview_id < 0 || preview_id >= SDVR_MAX_DRAW_REGIONS)
        return SDVR_ERR_UI_INVALID_PARAM;

    if ((pdi = (channel_draw_info_t *)ssdk_get_ui_context(handle)) == NULL)
        return SDVR_ERR_UI_CHANNEL_NOT_START;

	OS_UMUTEX_LOCK(g_draw_lock);

	pdi->di[preview_id].preview_cb = cb;

	OS_UMUTEX_UNLOCK(g_draw_lock);
	return SDVR_UI_ERR_NONE;
}

/****************************************************************************
  Occasionally, the window region displaying the video frames may need
  to be repainted as the result of another window or application covering it.
  In this case, you will receive a paint message to repaint the region.

  Call this function to refresh the content of all the regions that were
  being previewed.

  @Note:@ This function can only be used in conjunction with
  sdvr_ui_start_video_preview() and sdvr_ui_stop_video_preview(). If you
  are using the low level UI API, calling this function will have no affect
  on the display.

  Parameters:

    None.

  Returns:

    None.

  Remark:

    It is recommended although not required to call sdvr_ui_clear_yuv() for the
    desired region before calling this function.
****************************************************************************/
void sdvr_ui_refresh()
{
    int chan_num,j;
    int board_index;
    sdvr_err_e err;
    sdvr_chan_handle_t handle;
    channel_draw_info_t * pdi;
    int num_disp_chan = 0;
    int max_enc_chan_num;
    int max_dec_chan_num;

    if (!m_init_ok)
        return;

    OS_UMUTEX_LOCK(g_draw_lock);
    for (board_index = 0; board_index < MAX_PCI_BOARDS && num_disp_chan < m_num_disp_chan; board_index++)
    {
        err = ssdk_validate_board_ndx(board_index);
        if (err == SDVR_ERR_INVALID_BOARD)
            break;  // We reached to the end of DVR installed boards
        if (err == SDVR_ERR_BOARD_NOT_CONNECTED)
            continue;   // This board is not connected

        /* Refersh all encoder and HMO-only channels*/
        max_enc_chan_num = ssdk_get_max_chan_num(board_index, SDVR_CHAN_TYPE_ENCODER);

        for (chan_num=0; chan_num < max_enc_chan_num &&
            num_disp_chan < m_num_disp_chan; chan_num++)
        {
            handle = ssdk_create_chan_handle(board_index,
                                     SDVR_CHAN_TYPE_ENCODER,
                                     chan_num);
            err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER);
            if (err == SDVR_ERR_INVALID_CHAN_HANDLE)
                continue;

            pdi = (channel_draw_info_t *)ssdk_get_ui_context(handle);

            if (pdi && pdi->reg_count > 0)
            {
                sdvr_ui_region_t	src_region;

                num_disp_chan++;

			    src_region.top_left_x = src_region.top_left_y=0;
			    src_region.width      = pdi->s_width;
			    src_region.height     = pdi->s_height;
                for (j=0; j < SDVR_MAX_DRAW_REGIONS;j++)
                {
                    if (pdi->di[j].hwnd != INVALID_HWND && pdi->save_surface!=0)
                    {
					    draw_yuv(pdi->di[j].hwnd,&(pdi->di[j].rect),
							pdi->save_surface,&src_region, pdi->di[j].preview_cb);
                    }
                }
            }
        } //for (chan_num=0; i < MAX_CHANNEL_DRAW_INFO; chan_num++)

        /* Refresh all the decoder channels */
        max_dec_chan_num = ssdk_get_max_chan_num(board_index, SDVR_CHAN_TYPE_DECODER);

        for (chan_num=0; chan_num < max_dec_chan_num &&
            num_disp_chan < m_num_disp_chan; chan_num++)
        {
            handle = ssdk_create_chan_handle(board_index,
                                     SDVR_CHAN_TYPE_DECODER,
                                     chan_num);
            err = ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_DECODER) ;
            if (err == SDVR_ERR_INVALID_CHAN_HANDLE)
                continue;

            pdi = (channel_draw_info_t *)ssdk_get_ui_context(handle);

            if (pdi && pdi->reg_count > 0)
            {
                sdvr_ui_region_t	src_region;

                num_disp_chan++;

			    src_region.top_left_x = src_region.top_left_y=0;
			    src_region.width      = pdi->s_width;
			    src_region.height     = pdi->s_height;
                for (j=0; j < SDVR_MAX_DRAW_REGIONS;j++)
                {
                    if (pdi->di[j].hwnd != INVALID_HWND && pdi->save_surface!=0)
                    {
						draw_yuv(pdi->di[j].hwnd,&(pdi->di[j].rect),
							pdi->save_surface,&src_region, pdi->di[j].preview_cb);
                    }
                }
            }
        } //for (chan_num=0; i < MAX_CHANNEL_DRAW_INFO; chan_num++)

    } //for (board_index = 0; board_index < MAX_PCI_BOARDS; board_index++)
    OS_UMUTEX_UNLOCK(g_draw_lock);
}
