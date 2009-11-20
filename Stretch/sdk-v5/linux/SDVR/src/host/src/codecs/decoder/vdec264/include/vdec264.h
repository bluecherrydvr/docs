/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/
#ifndef __VDEC264_H__
#define __VDEC264_H__

/**********************************************************************************
    PACKAGE: vdec264 -- H.264 / AVC / MPEG4-10 decoder

    DESCRIPTION:

    SECTION: Include
    {
    #include "vdec264.h"
    }

    SECTION: Usage model

    SUBSECTION: Creating an decoder instance

    An decoder instance must first be created before it can be used to decoder 
    H264 video stream.  vdec264_open() is used to create a decoder instance.  
    When the decoder instance is no longer needed, it needs to be freed with vdec264_close().

    SUBSECTION: Decoding video stream

    A decoder instance can be used to decoder video stream by calling vdec264_decode(). 
    The inputs to this function include a pointer to a H264 bitstream buffer and another parameter
    to specify the buffer size. Since the input buffer may contain bitstream data for multiple frames or
    only part of a frame, the user needs to keep track of the amount of data has been consumed by the decoder.
    Please see the sample code below for more information.

    SUBSECTION: Receiving decoded pictures

    The decoded picture is stored in @vdec264_picture_t@.
    Each call to vdec264_get_picture_full() returns a decoded picture in display order if there is one available.
    Once the decoded picture is written to a file or displayed, the picture buffer must be given back
    to the decoder instance for reuse, by calling vdec264_put_picture_empty().

    SUBSECTION: Pseudo code
    {
        #include "vdec264.h"

        vdec264_t handle;
        vdec264_picture_t *picture;
        sx_uint8 inbuf[INBUF_SIZE], *inbuf_ptr;
        sx_int32 size, consumed;
        vdec264_err_e err;

        // create an decoder instance
        vdec264_open(&handle);

        while(...) {
            // fill input bitstream buffer
            size = fill_buffer(inbuf, ...);
			
            inbuf_ptr = inbuf;

             while (size > 0) {
                err = vdec264_decode(handle, inbuf_ptr, size, &consumed);
                assert(err == VDEC264_OK);
                
                err = vdec264_get_picture_full(handle, &picture);
                assert(err == VDEC264_OK);

                if(picture) {
                    // we have one decoded picture, do something with it
                    process_picture(picture, ...);

                    err = vdec264_put_picture_empty(handle, picture);
                }

                size -= consumed;
                inbuf_ptr += consumed;
            } 
        }

        vdec264_close(handle);
    }
**********************************************************************************/

#include "sx-types.h"

/**********************************************************************************
    Definitions
**********************************************************************************/
#if defined(__cplusplus)
   #define EXTERN_VDEC264              extern "C"
#else
   #define EXTERN_VDEC264              extern
#endif


/**********************************************************************************
    H264 picture type.
**********************************************************************************/
typedef enum {
    H264_I_TYPE = 1,
    H264_P_TYPE = 2, 
    H264_B_TYPE = 3,
    H264_SI_TYPE = 5,
    H264_SP_TYPE = 6 
} vdec264_pic_type_e;


/**********************************************************************************
    Common return status for @vdec264_sc@ routines.

    @VDEC264_OK@ - no error, operation succeeded.

    @VDEC264_NULL_POINTER@ - null pointer.

    @VDEC264_INSUFFICIENT_MEMORY@ - failed to allocate memory.

    @VDEC264_INVALID_PARAM@ - invalid parameter.

    @VDEC264_NOT_OPEN@ - decoder is not open.

    @VDEC264_FAIL@ - other decoder error.
**********************************************************************************/
typedef enum {
    VDEC264_OK                    = 0,          // no error, operation succeeded.
    VDEC264_NULL_POINTER          = -1,         // null pointer.
    VDEC264_INSUFFICIENT_MEMORY   = -3,         // failed to allocate memory.
    VDEC264_INVALID_PARAM         = -4,	        // invalid parameter.
    VDEC264_NOT_OPEN              = -5,         // decoder is not open.
    VDEC264_FAIL                  = -6          // other decoder error.
} vdec264_err_e;


/**********************************************************************************
    data structure to hold decoded picture.

    @plane@ - pointer to the picture Y, U, V planes.

    @padded_width@ - padded width of picture plane.

    @width@ - picture width.

    @height@ - picture height.

    @pic_num@ - coded picture number.

    @pic_type@ - picture type, see @vdec264_pic_type_e@ for details.
**********************************************************************************/
typedef struct {
    sx_uint8 *plane[3];                   
    sx_int32 padded_width[3];
    sx_int32 width;
    sx_int32 height;
    sx_int32 pic_num;
   vdec264_pic_type_e pic_type;
} vdec264_picture_t;

/**********************************************************************************
    A H264 video decoder handle.  This handle is allocated and
    initialized by vdec264_open().  It can be freed by vdec264_close().
    Once created, the handle serves as an decoder instance.  Multiple
    decoder instances can exist at the same time.
**********************************************************************************/
typedef struct vdec264_handle*	 vdec264_t;

EXTERN_VDEC264 vdec264_err_e vdec264_open( vdec264_t *handle );
EXTERN_VDEC264 void vdec264_close( vdec264_t handle );
EXTERN_VDEC264 vdec264_err_e vdec264_decode( vdec264_t handle, const sx_uint8 *buf, sx_int32 buf_size, sx_int32 *consumed);
EXTERN_VDEC264 vdec264_err_e vdec264_put_picture_empty( vdec264_t handle, vdec264_picture_t *picture );
EXTERN_VDEC264 vdec264_err_e vdec264_get_picture_full( vdec264_t handle, vdec264_picture_t **picture );


#endif
