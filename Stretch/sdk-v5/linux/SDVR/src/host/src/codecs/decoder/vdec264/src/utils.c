/*
 * utils for libavcodec
 * Copyright (c) 2001 Fabrice Bellard.
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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
 * @file utils.c
 * utils.
 */

#include "avcodec.h"
#include "dsputil.h"
#include "imgconvert.h"
#include "define.h"
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#if !defined(HAVE_MKSTEMP)
#include <fcntl.h>
#endif
#include <stddef.h>
#include <string.h>


void *av_fast_realloc(void *ptr, unsigned int *size, unsigned int min_size)
{
    if(min_size < *size)
        return ptr;

    *size= FFMAX(17*min_size/16 + 32, min_size);

    ptr= av_realloc(ptr, *size);
    if(!ptr) //we could set this to the unmodified min_size but this is safer if the user lost the ptr and uses NULL now
        *size= 0;

    return ptr;
}


typedef struct InternalBuffer{
    uint8_t *base[4];
    uint8_t *data[4];
    int linesize[4];
    int width, height;
    enum PixelFormat pix_fmt;
}InternalBuffer;

#define INTERNAL_BUFFER_SIZE 32

#define ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

void avcodec_align_dimensions(AVCodecContext *s, int *width, int *height)
{
    int w_align = 1;
    int h_align = 1;

    switch(s->pix_fmt){
    case PIX_FMT_YUV420P:
    case PIX_FMT_YUV422P:
    case PIX_FMT_YUV444P:
        w_align = 16; 
        h_align = 16;
        break;
    default:
        w_align = 1;
        h_align = 1;
        break;
    }

    *width = ALIGN(*width , w_align);
    *height= ALIGN(*height, h_align);
}

int avcodec_check_dimensions(void *av_log_ctx, unsigned int w, unsigned int h)
{
    if((int)w > 0 && (int)h > 0 && (w+128)*(uint64_t)(h+128) < INT_MAX/4)
        return 0;

    av_log(av_log_ctx, AV_LOG_ERROR, "picture size invalid (%ux%u)\n", w, h);
    return -1;
}

int avcodec_default_get_buffer(AVCodecContext *s, AVFrame *pic)
{
    int i;
    int w = s->width;
    int h = s->height;
    InternalBuffer *buf;

    if(pic->data[0] != NULL) {
        av_log(s, AV_LOG_ERROR, "pic->data[0]!=NULL in avcodec_default_get_buffer\n");
        return -1;
    }
    if(s->internal_buffer_count >= INTERNAL_BUFFER_SIZE) {
        av_log(s, AV_LOG_ERROR, "internal_buffer_count overflow (missing release_buffer?)\n");
        return -1;
    }

    if(avcodec_check_dimensions(s, w, h))
        return -1;

    if(s->internal_buffer == NULL){
        s->internal_buffer = av_mallocz((INTERNAL_BUFFER_SIZE+1)*sizeof(InternalBuffer));
    }

    buf = &((InternalBuffer*)s->internal_buffer)[s->internal_buffer_count];
    
    if(buf->base[0] && (buf->width != w || buf->height != h || buf->pix_fmt != s->pix_fmt)){
        for(i = 0; i < 4; i++){
            av_freep(&buf->base[i]);
            buf->data[i] = NULL;
        }
    }

    if(!buf->base[0]){
        int h_chroma_shift, v_chroma_shift;
        int size[4] = {0};
        int tmpsize;
        AVPicture picture;

        picture_get_chroma_sub_sample(s->pix_fmt, &h_chroma_shift, &v_chroma_shift);

        avcodec_align_dimensions(s, &w, &h);
        w += EDGE_WIDTH*2;
        h += EDGE_WIDTH*2;
        
        avcodec_align_dimensions(s, &w, &h);

        picture_fill_linesize(&picture, s->pix_fmt, w);

        for (i = 0; i < 4; i++)
            picture.linesize[i] = ALIGN(picture.linesize[i], STRIDE_ALIGN);

        tmpsize = picture_fill_pointer(&picture, NULL, s->pix_fmt, h);

        for (i = 0; i < 3 && picture.data[i+1]; i++)
            size[i] = picture.data[i+1] - picture.data[i];
        size[i] = tmpsize - (picture.data[i] - picture.data[0]);

        memset(buf->base, 0, sizeof(buf->base));
        memset(buf->data, 0, sizeof(buf->data));

        for(i = 0; i < 4 && size[i]; i++){
            const int h_shift = i==0 ? 0 : h_chroma_shift;
            const int v_shift = i==0 ? 0 : v_chroma_shift;

            buf->linesize[i] = picture.linesize[i];

            buf->base[i] = av_malloc(size[i]+16); 
            if(buf->base[i] == NULL) return -1;
            memset(buf->base[i], 128, size[i]);

            buf->data[i] = buf->base[i] + ALIGN((buf->linesize[i]*EDGE_WIDTH>>v_shift) + (EDGE_WIDTH>>h_shift), STRIDE_ALIGN);
        }
        buf->width  = s->width;
        buf->height = s->height;
        buf->pix_fmt= s->pix_fmt;
    }
    pic->type = FF_BUFFER_TYPE_INTERNAL;

    for(i = 0; i < 4; i++){
        pic->base[i] = buf->base[i];
        pic->data[i] = buf->data[i];
        pic->linesize[i] = buf->linesize[i];
    }
    s->internal_buffer_count++;

    return 0;
}

void avcodec_default_release_buffer(AVCodecContext *s, AVFrame *pic)
{
    int i;
    InternalBuffer *buf = NULL, *last;

    assert(pic->type == FF_BUFFER_TYPE_INTERNAL);
    assert(s->internal_buffer_count);

    for(i = 0; i < s->internal_buffer_count; i++){ //just 3-5 checks so is not worth to optimize
        buf = &((InternalBuffer*)s->internal_buffer)[i];
        if(buf->data[0] == pic->data[0])
            break;
    }
    assert(i < s->internal_buffer_count);
    s->internal_buffer_count--;
    last = &((InternalBuffer*)s->internal_buffer)[s->internal_buffer_count];

    FFSWAP(InternalBuffer, *buf, *last);

    for(i = 0; i < 4; i++){
        pic->data[i] = NULL;
    }
}


int avcodec_default_execute(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2),void **arg, int *ret, int count)
{
    int i;

    for(i = 0; i < count; i++){
        int r = func(c, arg[i]);
        if(ret) ret[i] = r;
    }
    return 0;
}


AVCodecContext *avcodec_alloc_context(void)
{
    AVCodecContext *avctx = av_mallocz(sizeof(AVCodecContext));
    if(avctx == NULL) return NULL;
   
    avctx->get_buffer = avcodec_default_get_buffer;
    avctx->release_buffer = avcodec_default_release_buffer;
    avctx->execute = avcodec_default_execute;
    avctx->pix_fmt = PIX_FMT_NONE;

    return avctx;
}

AVFrame *avcodec_alloc_frame(void)
{
    AVFrame *pic = av_malloc(sizeof(AVFrame));
    if(pic == NULL) return NULL;

    memset(pic, 0, sizeof(AVFrame));

    pic->pts = AV_NOPTS_VALUE;
  
    return pic;
}

int avcodec_open(AVCodecContext *avctx, AVCodec *codec)
{
    if(avctx->codec || !codec)
        return -1;

    if(codec->priv_data_size > 0) {
        avctx->priv_data = av_mallocz(codec->priv_data_size);
        if(!avctx->priv_data)
            return -1;
    } else {
        avctx->priv_data = NULL;
    }

    avctx->codec = codec;
    avctx->frame_number = 0;
  
    if(avctx->codec->init){
        int ret = avctx->codec->init(avctx);
        if(ret < 0) 
            return -1;
    }
   
    return 0;
}


int  avcodec_decode_video(AVCodecContext *avctx, AVFrame *picture,
                         int *got_picture_ptr,
                         const uint8_t *buf, int buf_size)
{
    int ret;

    *got_picture_ptr = 0;
    
    ret = avctx->codec->decode(avctx, picture, got_picture_ptr, buf, buf_size);

    emms_c(); //needed to avoid an emms_c() call before every return;

    if (*got_picture_ptr)
        avctx->frame_number++;
    
    return ret;
}


int avcodec_close(AVCodecContext *avctx)
{
    if (avctx->codec->close)
        avctx->codec->close(avctx);
    avcodec_default_free_buffers(avctx);
    av_freep(&avctx->priv_data);
    av_freep(&avctx->codec);
    return 0;
}

void avcodec_init(void)
{
    static int initialized = 0;

    if (initialized != 0)
        return;
    initialized = 1;

    dsputil_static_init();
}

void avcodec_flush_buffers(AVCodecContext *avctx)
{
    if(avctx->codec->flush)
        avctx->codec->flush(avctx);
}

void avcodec_default_free_buffers(AVCodecContext *s){
    int i, j;

    if(s->internal_buffer == NULL) return;

    for(i = 0; i < INTERNAL_BUFFER_SIZE; i++){
        InternalBuffer *buf = &((InternalBuffer*)s->internal_buffer)[i];
        for(j = 0; j < 4; j++){
            av_freep(&buf->base[j]);
            buf->data[j] = NULL;
        }
    }
    av_freep(&s->internal_buffer);

    s->internal_buffer_count = 0;
}




