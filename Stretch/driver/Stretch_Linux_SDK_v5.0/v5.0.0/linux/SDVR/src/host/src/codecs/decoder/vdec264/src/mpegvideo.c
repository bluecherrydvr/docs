/*
 * The simplest mpeg encoder (well, it was the simplest!)
 * Copyright (c) 2000,2001 Fabrice Bellard.
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * 4MV & hq & B-frame encoding stuff by Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file mpegvideo.c
 * The simplest mpeg encoder (well, it was the simplest!).
 */

#include "avcodec.h"
#include "dsputil.h"
#include "mpegvideo.h"
#include "mpegvideo_common.h"
#include "internal.h"
#include <limits.h>
#include <string.h>



void copy_picture(Picture *dst, Picture *src){
    *dst = *src;
    dst->type = FF_BUFFER_TYPE_COPY;
}

/**
 * allocates a Picture
 * The pixels are allocated/set by calling get_buffer() if shared=0
 */
int alloc_picture(MpegEncContext *s, Picture *pic, int shared){
    const int big_mb_num = s->mb_stride*(s->mb_height+1) + 1; //the +1 is needed so memset(,,stride*height) does not sig11
    const int mb_array_size = s->mb_stride*s->mb_height;
    const int b8_array_size = s->b8_stride*s->mb_height*2;
    const int b4_array_size = s->b4_stride*s->mb_height*4;
    int i;
    int r = -1;

    if(shared){
        assert(pic->data[0]);
        assert(pic->type == 0 || pic->type == FF_BUFFER_TYPE_SHARED);
        pic->type = FF_BUFFER_TYPE_SHARED;
    }else{
        assert(!pic->data[0]);

        r = s->avctx->get_buffer(s->avctx, (AVFrame*)pic);

        if(r < 0 || !pic->type || !pic->data[0]){
            av_log(s->avctx, AV_LOG_ERROR, "get_buffer() failed (%d %d %p)\n", r, pic->type, pic->data[0]);
            return -1;
        }

        if(s->linesize && (s->linesize != pic->linesize[0] || s->uvlinesize != pic->linesize[1])){
            av_log(s->avctx, AV_LOG_ERROR, "get_buffer() failed (stride changed)\n");
            s->avctx->release_buffer(s->avctx, (AVFrame*)pic);
            return -1;
        }

        if(pic->linesize[1] != pic->linesize[2]){
            av_log(s->avctx, AV_LOG_ERROR, "get_buffer() failed (uv stride mismatch)\n");
            s->avctx->release_buffer(s->avctx, (AVFrame*)pic);
            return -1;
        }

        s->linesize  = pic->linesize[0];
        s->uvlinesize= pic->linesize[1];
    }

    if(pic->qscale_table == NULL){
        CHECKED_ALLOCZ(pic->qscale_table , mb_array_size * sizeof(uint8_t))
        CHECKED_ALLOCZ(pic->mb_type_base , (big_mb_num + s->mb_stride) * sizeof(uint32_t))
        pic->mb_type= pic->mb_type_base + 2*s->mb_stride + 1;
        
        for(i = 0; i < 2; i++){
            CHECKED_ALLOCZ(pic->motion_val_base[i], 2 * (b4_array_size+4)  * sizeof(int16_t))
            pic->motion_val[i]= pic->motion_val_base[i]+4;
            CHECKED_ALLOCZ(pic->ref_index[i], b8_array_size * sizeof(uint8_t))
        }
    }

    return 0;
fail: //for the CHECKED_ALLOCZ macro
    if(r >= 0)
        s->avctx->release_buffer(s->avctx, (AVFrame*)pic);
    return -1;
}

/**
 * deallocates a picture
 */
static void free_picture(MpegEncContext *s, Picture *pic){
    int i;

    if(pic->data[0] && pic->type!=FF_BUFFER_TYPE_SHARED){
        s->avctx->release_buffer(s->avctx, (AVFrame*)pic);
    }

    av_freep(&pic->qscale_table);
    av_freep(&pic->mb_type_base);
    pic->mb_type = NULL;
    for(i = 0; i < 2; i++){
        av_freep(&pic->motion_val_base[i]);
        av_freep(&pic->ref_index[i]);
    }

    if(pic->type == FF_BUFFER_TYPE_SHARED){
        for(i = 0; i < 4; i++){
            pic->base[i] =
            pic->data[i] = NULL;
        }
        pic->type = 0;
    }
}

static int init_duplicate_context(MpegEncContext *s){
    // edge emu needs blocksize + filter length - 1 (=17x17 for halfpel / 21x21 for h264)
    CHECKED_ALLOCZ(s->allocated_edge_emu_buffer, (s->width+64)*2*21*2); //(width + edge + align)*interlaced*MBsize*tolerance
    s->edge_emu_buffer = s->allocated_edge_emu_buffer + (s->width+64)*2*21;

    CHECKED_ALLOCZ(s->blocks, 64*12*2 * sizeof(DCTELEM))
    s->block = s->blocks[0];

    return 0;
fail:
    return -1; 
}

static void free_duplicate_context(MpegEncContext *s){
    if(s == NULL) return;

    av_freep(&s->allocated_edge_emu_buffer); 
    s->edge_emu_buffer = NULL;
 
    av_freep(&s->blocks);
    s->block = NULL;
}


/**
 * sets the given MpegEncContext to common defaults (same for encoding and decoding).
 * the changed fields will not depend upon the prior state of the MpegEncContext.
 */
void MPV_common_defaults(MpegEncContext *s){

    s->picture_structure = PICT_FRAME;
    s->coded_picture_number = 0;
}


/**
 * init common structure for both encoder and decoder.
 * this assumes that some variables like width/height are already set
 */
int MPV_common_init(MpegEncContext *s)
{
    int y_size, c_size, yc_size, i, mb_array_size, x, y;

    if((s->width || s->height) && avcodec_check_dimensions(s->avctx, s->width, s->height))
        return -1;

    s->mb_height = (s->height + 15) / 16;
    s->mb_width  = (s->width  + 15) / 16;
    s->mb_stride = s->mb_width + 1;
    s->b8_stride = s->mb_width*2 + 1;
    s->b4_stride = s->mb_width*4 + 1;
    s->mb_num = s->mb_width * s->mb_height;

   
    dsputil_init(&s->dsp, s->avctx);

    mb_array_size = s->mb_height * s->mb_stride;

    /* set default edge pos, will be overriden in decode_header if needed */
    s->h_edge_pos = s->mb_width*16;
    s->v_edge_pos = s->mb_height*16;

    y_size = s->b8_stride * (2 * s->mb_height + 1);
    c_size = s->mb_stride * (s->mb_height + 1);
    yc_size = y_size + 2 * c_size;


    CHECKED_ALLOCZ(s->mb_index2xy, (s->mb_num+1)*sizeof(int)) 
    for(y = 0; y < s->mb_height; y++){
        for(x = 0; x < s->mb_width; x++){
            s->mb_index2xy[ x + y*s->mb_width ] = x + y*s->mb_stride;
        }
    }
    s->mb_index2xy[ s->mb_num ] = (s->mb_height-1)*s->mb_stride + s->mb_width; 

    CHECKED_ALLOCZ(s->picture, MAX_PICTURE_COUNT * sizeof(Picture))
    CHECKED_ALLOCZ(s->error_status_table, mb_array_size*sizeof(uint8_t))

    
    /* dc values */
    CHECKED_ALLOCZ(s->dc_val_base, yc_size * sizeof(int16_t));
    s->dc_val[0] = s->dc_val_base + s->b8_stride + 1;
    s->dc_val[1] = s->dc_val_base + y_size + s->mb_stride + 1;
    s->dc_val[2] = s->dc_val[1] + c_size;
    for(i = 0;i < yc_size;i++)
        s->dc_val_base[i] = 1024;
   

    /* which mb is a intra block */
    CHECKED_ALLOCZ(s->mbintra_table, mb_array_size);
    memset(s->mbintra_table, 1, mb_array_size);

    /* init macroblock skip table */
    CHECKED_ALLOCZ(s->mbskip_table, mb_array_size+2);

    if(init_duplicate_context(s) < 0)
        goto fail;

    s->context_initialized = 1;

    return 0;
 fail:
    MPV_common_end(s);
    return -1;
}

/* init common structure for both encoder and decoder */
void MPV_common_end(MpegEncContext *s)
{
    int i;

    free_duplicate_context(s);
  
    av_freep(&s->parse_context.buffer);
    s->parse_context.buffer_size=0;

    av_freep(&s->dc_val_base);
    av_freep(&s->mbintra_table);
    av_freep(&s->mbskip_table);
    av_freep(&s->error_status_table);
    av_freep(&s->mb_index2xy);
 
    if(s->picture){
        for(i = 0; i < MAX_PICTURE_COUNT; i++){
            free_picture(s, &s->picture[i]);
        }
    }
    av_freep(&s->picture);
    s->context_initialized = 0;
    s->last_picture_ptr=
    s->next_picture_ptr=
    s->current_picture_ptr = NULL;
    s->linesize = s->uvlinesize= 0;

    avcodec_default_free_buffers(s->avctx);
}


int ff_find_unused_picture(MpegEncContext *s, int shared){
    int i;

    if(shared){
        for(i = 0; i < MAX_PICTURE_COUNT; i++){
            if(s->picture[i].data[0] == NULL && s->picture[i].type == 0) return i;
        }
    }else{
        for(i = 0; i < MAX_PICTURE_COUNT; i++){
            if(s->picture[i].data[0] == NULL) return i;
        }
    }

    av_log(s->avctx, AV_LOG_FATAL, "Internal error, picture buffer overflow\n");
    return -1;
}

/**
 * generic function for encode/decode called after coding/decoding the header and before a frame is coded/decoded
 */
int MPV_frame_start(MpegEncContext *s, AVCodecContext *avctx)
{
    int i;
    AVFrame *pic;
    
    /* release non reference frames */
    for(i = 0; i < MAX_PICTURE_COUNT; i++){
        if(s->picture[i].data[0] && !s->picture[i].reference){
            s->avctx->release_buffer(s->avctx, (AVFrame*)&s->picture[i]);
        }
    }

    if(s->current_picture_ptr && s->current_picture_ptr->data[0] == NULL)
        pic = (AVFrame*)s->current_picture_ptr; //we already have a unused image (maybe it was set before reading the header)
    else{
        i = ff_find_unused_picture(s, 0);
        if(i < 0) return -1;
        pic = (AVFrame*)&s->picture[i];
    }

    pic->reference = 0;
    if(!s->dropable){
        pic->reference = s->picture_structure;
    }

    pic->coded_picture_number = s->coded_picture_number++;

    if( alloc_picture(s, (Picture*)pic, 0) < 0)
        return -1;

    s->current_picture_ptr = (Picture*)pic;
    s->current_picture_ptr->top_field_first = s->top_field_first; 
    s->current_picture_ptr->interlaced_frame = 0;
    s->current_picture_ptr->pict_type = s->pict_type;

    copy_picture(&s->current_picture, s->current_picture_ptr);

    if (s->pict_type != FF_B_TYPE) {
        s->last_picture_ptr = s->next_picture_ptr;
        if(!s->dropable)
            s->next_picture_ptr = s->current_picture_ptr;
    }

    if(s->last_picture_ptr) copy_picture(&s->last_picture, s->last_picture_ptr);
    if(s->next_picture_ptr) copy_picture(&s->next_picture, s->next_picture_ptr);

    assert(s->pict_type == FF_I_TYPE || (s->last_picture_ptr && s->last_picture_ptr->data[0]));

    s->hurry_up = s->avctx->hurry_up;
    s->error_recognition = avctx->error_recognition;

    return 0;
}

/* generic function for encode/decode called after a frame has been coded/decoded */
void MPV_frame_end(MpegEncContext *s)
{
    if(s->unrestricted_mv && s->current_picture.reference) {
            s->dsp.draw_edges(s->current_picture.data[0], s->linesize  , s->h_edge_pos   , s->v_edge_pos   , EDGE_WIDTH  );
            s->dsp.draw_edges(s->current_picture.data[1], s->uvlinesize, s->h_edge_pos>>1, s->v_edge_pos>>1, EDGE_WIDTH/2);
            s->dsp.draw_edges(s->current_picture.data[2], s->uvlinesize, s->h_edge_pos>>1, s->v_edge_pos>>1, EDGE_WIDTH/2);
    }
    emms_c();

#if 0
        /* copy back current_picture variables */
    for(i=0; i<MAX_PICTURE_COUNT; i++){
        if(s->picture[i].data[0] == s->current_picture.data[0]){
            s->picture[i]= s->current_picture;
            break;
        }
    }
    assert(i<MAX_PICTURE_COUNT);
#endif

    // clear copies, to avoid confusion
#if 0
    memset(&s->last_picture, 0, sizeof(Picture));
    memset(&s->next_picture, 0, sizeof(Picture));
    memset(&s->current_picture, 0, sizeof(Picture));
#endif
}


void ff_mpeg_flush(AVCodecContext *avctx){
    int i;
    MpegEncContext *s = avctx->priv_data;

    if(s==NULL || s->picture==NULL)
        return;

    for(i=0; i<MAX_PICTURE_COUNT; i++){
       if(s->picture[i].data[0] && (   s->picture[i].type == FF_BUFFER_TYPE_INTERNAL
                                    || s->picture[i].type == FF_BUFFER_TYPE_USER))
        avctx->release_buffer(avctx, (AVFrame*)&s->picture[i]);
    }
    s->current_picture_ptr = s->last_picture_ptr = s->next_picture_ptr = NULL;

    s->mb_x= s->mb_y= 0;

    s->parse_context.state= -1;
    s->parse_context.frame_start_found= 0;
    s->parse_context.overread= 0;
    s->parse_context.overread_index= 0;
    s->parse_context.index= 0;
    s->parse_context.last_index= 0;
    s->pp_time=0;
}

