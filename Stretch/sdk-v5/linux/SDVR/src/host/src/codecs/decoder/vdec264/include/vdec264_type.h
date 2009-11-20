/************************************************************************
 *  H.264 / AVC / MPEG4 part10 Decoder
 ************************************************************************/

#ifndef __VDEC264_TYPE_H__
#define __VDEC264_TYPE_H__

#include "vdec264.h"
#include "avcodec.h"
#include "define.h"
#include "ls.h"
#include <stdlib.h>

/* H264 decoder handle */
struct vdec264_handle
{
    AVCodecContext *ctx;
    sx_bool is_opened;
    ls_t *av_frame_empty_list;
    ls_t *av_frame_full_list;
    ls_t *picture_empty_list;
    ls_t *picture_full_list;

};

#endif



