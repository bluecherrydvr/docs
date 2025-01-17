/*
 * Misc image conversion routines
 * most functionality is exported to the public API, see avcodec.h
 *
 * Copyright (c) 2008 Vitor Sessak
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

#ifndef __IMGCONVERT_H__
#define __IMGCONVERT_H__

#include "avcodec.h"


void picture_get_chroma_sub_sample(int pix_fmt, int *h_shift, int *v_shift);
int picture_fill_linesize(AVPicture *picture, int pix_fmt, int width);
int picture_fill_pointer(AVPicture *picture, uint8_t *ptr, int pix_fmt, int height);


#endif /* __IMGCONVERT_H__ */
