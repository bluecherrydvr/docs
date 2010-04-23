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
 * @file rational.c
 * Rational numbers
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include <math.h>
#include <limits.h>
#include "rational.h"
#include <assert.h>

static int64_t ff_gcd(int64_t a, int64_t b)
{
      if(b) return ff_gcd(b, a%b);
      else  return a;
}


int av_reduce(int *dst_nom, int *dst_den, int64_t nom, int64_t den, int64_t max){
    AVRational a0 = {0,1}, a1 = {1,0};
    int sign = (nom < 0) ^ (den < 0);
    int64_t gcd = ff_gcd(FFABS(nom), FFABS(den));

    if(gcd){
        nom = FFABS(nom)/gcd;
        den = FFABS(den)/gcd;
    }
    if(nom <= max && den <= max){
        a1.num = (int)nom; ///< numerator
        a1.den = (int)den; ///< denominator
		
        den = 0;
    }

    while(den){
        int64_t x = nom / den;
        int64_t next_den = nom - den*x;
        int64_t a2n = x*a1.num + a0.num;
        int64_t a2d = x*a1.den + a0.den;

        if(a2n > max || a2d > max){
            if(a1.num) x = (max - a0.num) / a1.num;
            if(a1.den) x = FFMIN(x, (max - a0.den) / a1.den);

            if (den*(2*x*a1.den + a0.den) > nom*a1.den) {
                a1.num = (int)(x*a1.num + a0.num);
                a1.den = (int)(x*a1.den + a0.den);
            }
            break;
        }

        a0 = a1;
        a1.num = (int)a2n;
        a1.den = (int)a2d;
        nom = den;
        den = next_den;
    }
    assert(ff_gcd(a1.num, a1.den) <= 1U);

    *dst_nom = sign ? -a1.num : a1.num;
    *dst_den = a1.den;

    return den == 0;
}

