/*
 * Misc image conversion routines
 * Copyright (c) 2001, 2002, 2003 Fabrice Bellard.
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
 * @file imgconvert.c
 * misc image conversion routines
 */

/* TODO:
 * - write 'ffimg' program to test all the image related stuff
 * - move all api to slice based system
 * - integrate deinterlacing, postprocessing and scaling in the conversion process
 */

#include "imgconvert.h"
#include <stdlib.h>
#include <memory.h>


void picture_get_chroma_sub_sample(int pix_fmt, int *h_shift, int *v_shift)
{
    if(pix_fmt == PIX_FMT_YUV420P) { 
        *h_shift = 1;
        *v_shift = 1;
    } else if(pix_fmt == PIX_FMT_YUV422P) {
        *h_shift = 1;
        *v_shift = 0;
    } else {
        *h_shift = 0;
        *v_shift = 0;
    }
}

int picture_fill_linesize(AVPicture *picture, int pix_fmt, int width)
{
    int w2;
    int x_chroma_shift;

    memset(picture->linesize, 0, sizeof(picture->linesize));

    if(pix_fmt == PIX_FMT_YUV420P || pix_fmt == PIX_FMT_YUV422P)
        x_chroma_shift = 1;
    else if(pix_fmt == PIX_FMT_YUV444P)
        x_chroma_shift = 0;
    else
        return -1;

    w2 = (width + (1 << x_chroma_shift) - 1) >> x_chroma_shift;
    picture->linesize[0] = width;
    picture->linesize[1] = w2;
    picture->linesize[2] = w2;
   
    return 0;
}

int picture_fill_pointer(AVPicture *picture, uint8_t *ptr, int pix_fmt,
                    int height)
{
    int size, h2, size2;
    int y_chroma_shift;
    
    if(pix_fmt == PIX_FMT_YUV420P)
        y_chroma_shift = 1;
    else if(pix_fmt == PIX_FMT_YUV422P || pix_fmt == PIX_FMT_YUV444P)
        y_chroma_shift = 0;
    else {
        picture->data[0] = NULL;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->data[3] = NULL;
        return -1;
    }

    size = picture->linesize[0] * height;
    
    h2 = (height + (1 << y_chroma_shift) - 1) >> y_chroma_shift;
    size2 = picture->linesize[1] * h2;
    picture->data[0] = ptr;
    picture->data[1] = picture->data[0] + size;
    picture->data[2] = picture->data[1] + size2;
    picture->data[3] = NULL;

    return size + 2 * size2;
   
}


