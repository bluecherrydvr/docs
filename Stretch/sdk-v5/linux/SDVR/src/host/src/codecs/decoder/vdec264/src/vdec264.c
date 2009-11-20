/************************************************************************
 *  H.264 / AVC / MPEG4 part10 Decoder
 ************************************************************************/

#include "vdec264_type.h"
#include "dsputil.h"
#include "mem.h"
#include "h264.h"


/**********************************************************************************
    Initialize H264 video decoder.

	@Arguments@:

        "handle" - a newly created decoder instance.

    @Returns@: @VDEC264_OK@ when successful, error messages otherwise.
**********************************************************************************/
vdec264_err_e vdec264_open( vdec264_t *handle )
{
    vdec264_t instance = NULL;
    AVCodecContext *ctx;
    AVCodec *codec;
   
    avcodec_init();

    if(handle == NULL)
        return VDEC264_NULL_POINTER;

    instance = (vdec264_t)av_mallocz(sizeof(struct vdec264_handle));
    if(instance == NULL)
        return VDEC264_INSUFFICIENT_MEMORY;

    ctx = instance->ctx = avcodec_alloc_context();
    if(ctx == NULL)
        return VDEC264_INSUFFICIENT_MEMORY;
   
    codec = h264_decoder_init();
    if(codec == NULL)
        return VDEC264_INSUFFICIENT_MEMORY;

    if (avcodec_open(ctx, codec) < 0) 
        return VDEC264_FAIL;

    instance->is_opened = true;
    instance->av_frame_empty_list = ls_alloc();
    instance->av_frame_full_list = ls_alloc();
    instance->picture_empty_list = ls_alloc();
    instance->picture_full_list = ls_alloc();
  
    *handle = instance;

    return VDEC264_OK;

}

/**********************************************************************************
    Frees the memory associated with the decoder instance "handle".

    @Arguments@:

        "handle" - A previously opened decoder instance to be freed.

    @Returns@: @VDEC264_OK@ when successful, error messages otherwise.
**********************************************************************************/
void vdec264_close( vdec264_t handle )
{
    if(handle) {
        if(handle->ctx) {
            avcodec_close(handle->ctx);
            av_free(handle->ctx);
            handle->ctx = NULL;
        }

        if(handle->av_frame_empty_list) {
            AVFrame *frame;
            while( ( frame = ls_pop_head(handle->av_frame_empty_list) ) != NULL ) {
	            av_free(frame);
            }

            ls_free(handle->av_frame_empty_list);
            handle->av_frame_empty_list = NULL;
        }
        
        if(handle->av_frame_full_list) {
            AVFrame *frame;
            while( ( frame = ls_pop_head(handle->av_frame_full_list) ) != NULL ) {
	            av_free(frame);
            }

            ls_free(handle->av_frame_full_list);
            handle->av_frame_full_list = NULL;
        }

        if(handle->picture_empty_list) {
            vdec264_picture_t *picture;
            while( ( picture = ls_pop_head(handle->picture_empty_list) ) != NULL ) {
	            free(picture);
            }

            ls_free(handle->picture_empty_list);
            handle->picture_empty_list = NULL;
        }
        
        if(handle->picture_full_list) {
            vdec264_picture_t *picture;
            while( ( picture = ls_pop_head(handle->picture_full_list) ) != NULL ) {
	            free(picture);
            }

            ls_free(handle->picture_full_list);
            handle->picture_full_list = NULL;
        }

        av_free(handle);
    }
}

/**********************************************************************************
    Decode the bitstream stored in the input buffer and returns the number of bytes
    have been consumed. 

    @Arguments@:

        "handle" - A previously opened and configured decoder instance.

        "buf" - Pointer to the input buffer.

        "buf_size" - Size of input buffer.

        "consumed" - Number of bytes have been consumed.

    @Returns@: @VDEC264_OK@ when successful, error messages otherwise.
**********************************************************************************/
vdec264_err_e vdec264_decode( vdec264_t handle, const sx_uint8 *buf, sx_int32 buf_size, sx_int32 *consumed)
{
    AVFrame *av_frame;
    vdec264_picture_t *picture;
    sx_int32 got_picture;
    sx_int32 len;

    if(handle == NULL || buf == NULL || consumed == NULL)
        return VDEC264_NULL_POINTER;

    if(buf_size < 0)
        return VDEC264_INVALID_PARAM;

    if(!handle->is_opened)
        return VDEC264_NOT_OPEN;

    av_frame = (AVFrame*)ls_pop_head(handle->av_frame_empty_list);
    if(av_frame == NULL) {
        av_frame = avcodec_alloc_frame();
        if (av_frame == NULL)
            return VDEC264_INSUFFICIENT_MEMORY;
    }

    picture = (vdec264_picture_t*)ls_pop_head(handle->picture_empty_list);
    if(picture == NULL) {
        picture = malloc(sizeof(vdec264_picture_t));
        if(picture == NULL)
            return VDEC264_INSUFFICIENT_MEMORY;
    }

    len = avcodec_decode_video(handle->ctx, av_frame, &got_picture, buf, buf_size);
    if(len < 0)
        return VDEC264_FAIL;

    *consumed = len;
    if(got_picture) {
        sx_int32 i;
        for(i = 0; i < 3; i++) {
            picture->plane[i] = av_frame->data[i];
            picture->padded_width[i] = av_frame->linesize[i];
            picture->width = handle->ctx->width;
            picture->height = handle->ctx->height;
            picture->pic_type = av_frame->pict_type;
            picture->pic_num = av_frame->coded_picture_number;
        }
        ls_push_tail(handle->picture_full_list, (void*)picture);
        ls_push_tail(handle->av_frame_full_list, (void*)av_frame);
    } else {
        ls_push_tail(handle->picture_empty_list, (void*)picture);
        ls_push_tail(handle->av_frame_empty_list, (void*)av_frame);
    }

    return VDEC264_OK;

}

/**********************************************************************************
    Returns a picture buffer to the decoder instance "handle" to reuse.

    @Arguments@:

        "handle" - A previously opened and configured decoder instance.

        "picture" - A picture buffer to be reused.

    @Returns@: @VDEC264_OK@ when successful, error messages otherwise.
**********************************************************************************/
vdec264_err_e vdec264_put_picture_empty( vdec264_t handle, vdec264_picture_t *picture )
{
    ls_entry_t *entry;
    sx_bool found = false;
    sx_int32 index = 0;

    if( handle == NULL || picture == NULL )
	    return VDEC264_NULL_POINTER;

    entry = ls_get_head_entry( handle->av_frame_full_list );
    while( entry && !found) {
        AVFrame *av_frame = (AVFrame*)ls_entry_get_data(entry);
        if( av_frame->data[0] == picture->plane[0] &&
            av_frame->data[1] == picture->plane[1] &&
            av_frame->data[2] == picture->plane[2] ) {
                found = true;
                ls_push_tail(handle->av_frame_empty_list, (void*)av_frame);
                ls_pop_head_n(handle->av_frame_full_list, index);
        }
        entry = ls_entry_get_prev(entry);
        index++;
    }

    if(!found)
        return VDEC264_FAIL;
            
    ls_push_tail( handle->picture_empty_list, (void*)picture );

    return VDEC264_OK;

}

/**********************************************************************************
    Returns a picture buffer that contains one decoded frame.
    After the picture buffer is written to a file or displayed,
    the buffer must be given back to the decoder for reuse, by calling
    @vdec264_put_picture_empty@.

    @Arguments@:

        "handle" - A previously opened and configured decoder instance.

        "picture" - Picture buffer that contains one decoded frame.

    @Returns@: @VDEC264_OK@ when successful, error messages otherwise.
**********************************************************************************/
vdec264_err_e vdec264_get_picture_full( vdec264_t handle, vdec264_picture_t **picture )
{
    if( handle == NULL || picture == NULL )
        return VDEC264_NULL_POINTER;

    *picture = (vdec264_picture_t*) ls_pop_head( handle->picture_full_list );

    return VDEC264_OK;

}

