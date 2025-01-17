/*
 * Rational numbers
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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
 * @file rational.h
 * Rational numbers.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#ifndef __RATIONAL_H__
#define __RATIONAL_H__


#include "common.h"

/**
 * Rational number num/den.
 */
typedef struct AVRational{
    int num; ///< numerator
    int den; ///< denominator
} AVRational;

/**
 * Compare two rationals.
 * @param a first rational
 * @param b second rational
 * @return 0 if a==b, 1 if a>b and -1 if a<b.
 */
static inline int av_cmp_q(AVRational a, AVRational b){
    const int64_t tmp = a.num * (int64_t)b.den - b.num * (int64_t)a.den;

    if(tmp) return (tmp>>63)|1;
    else    return 0;
}

/**
 * Rational to double conversion.
 * @param a rational to convert
 * @return (double) a
 */
static inline double av_q2d(AVRational a){
    return a.num / (double) a.den;
}

/**
 * Reduce a fraction.
 * This is useful for framerate calculations.
 * @param dst_nom destination numerator
 * @param dst_den destination denominator
 * @param nom source numerator
 * @param den source denominator
 * @param max the maximum allowed for dst_nom & dst_den
 * @return 1 if exact, 0 otherwise
 */
int av_reduce(int *dst_nom, int *dst_den, int64_t nom, int64_t den, int64_t max);



#endif /* __RATIONAL_H__ */
