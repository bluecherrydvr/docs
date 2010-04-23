/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
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
 * @file common.h
 * common internal and external API header
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#if defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
#    define CONFIG_WIN32
#endif
#if defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__) && !defined(EMULATE_INTTYPES)
#    define EMULATE_INTTYPES
#endif
#ifndef EMULATE_INTTYPES
#   include <inttypes.h>
#else
    typedef signed char  int8_t;
    typedef signed short int16_t;
    typedef signed int   int32_t;
    typedef unsigned char  uint8_t;
    typedef unsigned short uint16_t;
    typedef unsigned int   uint32_t;
#   ifdef CONFIG_WIN32
        typedef signed __int64   int64_t;
        typedef unsigned __int64 uint64_t;
#   else /* other OS */
        typedef signed long long   int64_t;
        typedef unsigned long long uint64_t;
#   endif /* other OS */
#endif /* EMULATE_INTTYPES */


#ifndef INT64_C
#define INT64_C(c)     (c ## LL)
#define UINT64_C(c)    (c ## ULL)
#endif


#ifdef HAVE_AV_CONFIG_H
/* only include the following when compiling package */
#    include "define.h"

#    include <stdlib.h>
#    include <stdio.h>
#    include <string.h>
#    include <ctype.h>
#    include <limits.h>
#    include <errno.h>
#    include <math.h>
#endif /* HAVE_AV_CONFIG_H */

#if defined(_MSC_VER) & !defined(__cplusplus)
#    define inline __inline
#endif

#ifndef av_always_inline
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define av_always_inline __attribute__((always_inline)) inline
#else
#    define av_always_inline inline
#endif
#endif

#ifndef av_noinline
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define av_noinline __attribute__((noinline))
#else
#    define av_noinline
#endif
#endif

#ifndef av_pure
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define av_pure __attribute__((pure))
#else
#    define av_pure
#endif
#endif

#ifndef av_const
#if defined(__GNUC__) && (__GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ > 5)
#    define av_const __attribute__((const))
#else
#    define av_const
#endif
#endif

#ifndef av_cold
#if defined(__GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ > 2)
#    define av_cold __attribute__((cold))
#else
#    define av_cold
#endif
#endif

#ifdef HAVE_AV_CONFIG_H
#    include "internal.h"
#endif /* HAVE_AV_CONFIG_H */

#ifndef attribute_deprecated
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define attribute_deprecated __attribute__((deprecated))
#else
#    define attribute_deprecated
#endif
#endif

#ifndef av_unused
#if defined(__GNUC__)
#    define av_unused __attribute__((unused))
#else
#    define av_unused
#endif
#endif

#include "mem.h"

//rounded divison & shift
#define RSHIFT(a,b) ((a) > 0 ? ((a) + ((1<<(b))>>1))>>(b) : ((a) + ((1<<(b))>>1)-1)>>(b))
/* assume b>0 */
#define ROUNDED_DIV(a,b) (((a)>0 ? (a) + ((b)>>1) : (a) - ((b)>>1))/(b))
#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)

#define FFSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)


/* misc math functions */
static const uint8_t ff_log2_tab[256]={
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

static inline av_const int av_log2(unsigned int v)
{
    int n = 0;
    if (v & 0xffff0000) {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += ff_log2_tab[v];

    return n;
}

static inline av_const int av_log2_16bit(unsigned int v)
{
    int n = 0;
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += ff_log2_tab[v];

    return n;
}

/* median of 3 */
static inline av_const int mid_pred(int a, int b, int c)
{
#ifdef HAVE_CMOV
    int i=b;
    asm volatile(
        "cmp    %2, %1 \n\t"
        "cmovg  %1, %0 \n\t"
        "cmovg  %2, %1 \n\t"
        "cmp    %3, %1 \n\t"
        "cmovl  %3, %1 \n\t"
        "cmp    %1, %0 \n\t"
        "cmovg  %1, %0 \n\t"
        :"+&r"(i), "+&r"(a)
        :"r"(b), "r"(c)
    );
    return i;
#elif 0
    int t= (a-b)&((a-b)>>31);
    a-=t;
    b+=t;
    b-= (b-c)&((b-c)>>31);
    b+= (a-b)&((a-b)>>31);

    return b;
#else
    if(a>b){
        if(c>b){
            if(c>a) b=a;
            else    b=c;
        }
    }else{
        if(b>c){
            if(c>a) b=c;
            else    b=a;
        }
    }
    return b;
#endif
}

/**
 * clip a signed integer value into the amin-amax range
 * @param a value to clip
 * @param amin minimum value of the clip range
 * @param amax maximum value of the clip range
 * @return clipped value
 */
static inline av_const int av_clip(int a, int amin, int amax)
{
    if      (a < amin) return amin;
    else if (a > amax) return amax;
    else               return a;
}

/**
 * clip a signed integer value into the 0-255 range
 * @param a value to clip
 * @return clipped value
 */
static inline av_const uint8_t av_clip_uint8(int a)
{
    if (a&(~255)) return (-a)>>31;
    else          return a;
}

/**
 * clip a signed integer value into the -32768,32767 range
 * @param a value to clip
 * @return clipped value
 */
static inline av_const int16_t av_clip_int16(int a)
{
    if ((a+32768) & ~65535) return (a>>31) ^ 32767;
    else                    return a;
}

/**
 * clip a float value into the amin-amax range
 * @param a value to clip
 * @param amin minimum value of the clip range
 * @param amax maximum value of the clip range
 * @return clipped value
 */
static inline av_const float av_clipf(float a, float amin, float amax)
{
    if      (a < amin) return amin;
    else if (a > amax) return amax;
    else               return a;
}


/**
 * converts fourcc string to int
 */
static inline av_pure int ff_get_fourcc(const char *s){
#ifdef HAVE_AV_CONFIG_H
    assert( strlen(s)==4 );
#endif

    return (s[0]) + (s[1]<<8) + (s[2]<<16) + (s[3]<<24);
}




#if defined(ARCH_X86) || defined(ARCH_POWERPC) || defined(ARCH_BFIN)
#define AV_READ_TIME read_time
#if defined(ARCH_X86_64)
static inline uint64_t read_time(void)
{
    uint64_t a, d;
    asm volatile("rdtsc\n\t"
                 : "=a" (a), "=d" (d));
    return (d << 32) | (a & 0xffffffff);
}
#elif defined(ARCH_X86_32)
static inline long long read_time(void)
{
    long long l;
    asm volatile("rdtsc\n\t"
                 : "=A" (l));
    return l;
}
#elif ARCH_BFIN
static inline uint64_t read_time(void)
{
    union {
        struct {
            unsigned lo;
            unsigned hi;
        } p;
        unsigned long long c;
    } t;
    asm volatile ("%0=cycles; %1=cycles2;" : "=d" (t.p.lo), "=d" (t.p.hi));
    return t.c;
}
#else //FIXME check ppc64
static inline uint64_t read_time(void)
{
    uint32_t tbu, tbl, temp;

     /* from section 2.2.1 of the 32-bit PowerPC PEM */
     asm volatile(
         "1:\n"
         "mftbu  %2\n"
         "mftb   %0\n"
         "mftbu  %1\n"
         "cmpw   %2,%1\n"
         "bne    1b\n"
     : "=r"(tbl), "=r"(tbu), "=r"(temp)
     :
     : "cc");

     return (((uint64_t)tbu)<<32) | (uint64_t)tbl;
}
#endif
#elif defined(HAVE_GETHRTIME)
#define AV_READ_TIME gethrtime
#endif

#ifdef AV_READ_TIME
#define START_TIMER \
uint64_t tend;\
uint64_t tstart= AV_READ_TIME();\

#define STOP_TIMER(id) \
tend= AV_READ_TIME();\
{\
    static uint64_t tsum=0;\
    static int tcount=0;\
    static int tskip_count=0;\
    if(tcount<2 || tend - tstart < FFMAX(8*tsum/tcount, 2000)){\
        tsum+= tend - tstart;\
        tcount++;\
    }else\
        tskip_count++;\
    if(((tcount+tskip_count)&(tcount+tskip_count-1))==0){\
        av_log(NULL, AV_LOG_ERROR, "%"PRIu64" dezicycles in %s, %d runs, %d skips\n",\
               tsum*10/tcount, id, tcount, tskip_count);\
    }\
}
#else
#define START_TIMER
#define STOP_TIMER(id) {}
#endif


#endif /* __COMMON_H__ */
