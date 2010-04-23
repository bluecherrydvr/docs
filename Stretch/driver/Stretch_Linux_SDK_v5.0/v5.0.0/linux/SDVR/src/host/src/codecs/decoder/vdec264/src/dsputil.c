/*
 * DSP utils
 * Copyright (c) 2000, 2001 Fabrice Bellard.
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * gmc & q-pel & 32/64 bit based MC by Michael Niedermayer <michaelni@gmx.at>
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
 * @file dsputil.c
 * DSP utils
 */

#include "avcodec.h"
#include "dsputil.h"
#include "define.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <memory.h>

uint8_t ff_cropTbl[256 + 2 * MAX_NEG_CROP] = {0, };
uint32_t ff_squareTbl[512] = {0, };

/* a*inverse[b]>>32 == a/b for all 0<=a<=65536 && 2<=b<=255 */
const uint32_t ff_inverse[256]={
         0, 4294967295U,2147483648U,1431655766, 1073741824,  858993460,  715827883,  613566757,
 536870912,  477218589,  429496730,  390451573,  357913942,  330382100,  306783379,  286331154,
 268435456,  252645136,  238609295,  226050911,  214748365,  204522253,  195225787,  186737709,
 178956971,  171798692,  165191050,  159072863,  153391690,  148102321,  143165577,  138547333,
 134217728,  130150525,  126322568,  122713352,  119304648,  116080198,  113025456,  110127367,
 107374183,  104755300,  102261127,   99882961,   97612894,   95443718,   93368855,   91382283,
  89478486,   87652394,   85899346,   84215046,   82595525,   81037119,   79536432,   78090315,
  76695845,   75350304,   74051161,   72796056,   71582789,   70409300,   69273667,   68174085,
  67108864,   66076420,   65075263,   64103990,   63161284,   62245903,   61356676,   60492498,
  59652324,   58835169,   58040099,   57266231,   56512728,   55778797,   55063684,   54366675,
  53687092,   53024288,   52377650,   51746594,   51130564,   50529028,   49941481,   49367441,
  48806447,   48258060,   47721859,   47197443,   46684428,   46182445,   45691142,   45210183,
  44739243,   44278014,   43826197,   43383509,   42949673,   42524429,   42107523,   41698712,
  41297763,   40904451,   40518560,   40139882,   39768216,   39403370,   39045158,   38693400,
  38347923,   38008561,   37675152,   37347542,   37025581,   36709123,   36398028,   36092163,
  35791395,   35495598,   35204650,   34918434,   34636834,   34359739,   34087043,   33818641,
  33554432,   33294321,   33038210,   32786010,   32537632,   32292988,   32051995,   31814573,
  31580642,   31350127,   31122952,   30899046,   30678338,   30460761,   30246249,   30034737,
  29826162,   29620465,   29417585,   29217465,   29020050,   28825284,   28633116,   28443493,
  28256364,   28071682,   27889399,   27709467,   27531842,   27356480,   27183338,   27012373,
  26843546,   26676816,   26512144,   26349493,   26188825,   26030105,   25873297,   25718368,
  25565282,   25414008,   25264514,   25116768,   24970741,   24826401,   24683721,   24542671,
  24403224,   24265352,   24129030,   23994231,   23860930,   23729102,   23598722,   23469767,
  23342214,   23216040,   23091223,   22967740,   22845571,   22724695,   22605092,   22486740,
  22369622,   22253717,   22139007,   22025474,   21913099,   21801865,   21691755,   21582751,
  21474837,   21367997,   21262215,   21157475,   21053762,   20951060,   20849356,   20748635,
  20648882,   20550083,   20452226,   20355296,   20259280,   20164166,   20069941,   19976593,
  19884108,   19792477,   19701685,   19611723,   19522579,   19434242,   19346700,   19259944,
  19173962,   19088744,   19004281,   18920561,   18837576,   18755316,   18673771,   18592933,
  18512791,   18433337,   18354562,   18276457,   18199014,   18122225,   18046082,   17970575,
  17895698,   17821442,   17747799,   17674763,   17602325,   17530479,   17459217,   17388532,
  17318417,   17248865,   17179870,   17111424,   17043522,   16976156,   16909321,   16843010,
};


/* draw the edges of width 'w' of an image of size width, height */
//FIXME check that this is ok for mpeg4 interlaced
static void draw_edges_c(uint8_t *buf, int wrap, int width, int height, int w)
{
    uint8_t *ptr, *last_line;
    int i;

    last_line = buf + (height - 1) * wrap;
    for(i=0;i<w;i++) {
        /* top and bottom */
        memcpy(buf - (i + 1) * wrap, buf, width);
        memcpy(last_line + (i + 1) * wrap, last_line, width);
    }
    /* left and right */
    ptr = buf;
    for(i=0;i<height;i++) {
        memset(ptr - w, ptr[0], w);
        memset(ptr + width, ptr[width-1], w);
        ptr += wrap;
    }
    /* corners */
    for(i=0;i<w;i++) {
        memset(buf - (i + 1) * wrap - w, buf[0], w); /* top left */
        memset(buf - (i + 1) * wrap + width, buf[width-1], w); /* top right */
        memset(last_line + (i + 1) * wrap - w, last_line[0], w); /* top left */
        memset(last_line + (i + 1) * wrap + width, last_line[width-1], w); /* top right */
    }
}

/**
 * Copies a rectangular area of samples to a temporary buffer and replicates the boarder samples.
 * @param buf destination buffer
 * @param src source buffer
 * @param linesize number of bytes between 2 vertically adjacent samples in both the source and destination buffers
 * @param block_w width of block
 * @param block_h height of block
 * @param src_x x coordinate of the top left sample of the block in the source buffer
 * @param src_y y coordinate of the top left sample of the block in the source buffer
 * @param w width of the source buffer
 * @param h height of the source buffer
 */
void ff_emulated_edge_mc(uint8_t *buf, uint8_t *src, int linesize, int block_w, int block_h,
                                    int src_x, int src_y, int w, int h){
    int x, y;
    int start_y, start_x, end_y, end_x;

    if(src_y>= h){
        src+= (h-1-src_y)*linesize;
        src_y=h-1;
    }else if(src_y<=-block_h){
        src+= (1-block_h-src_y)*linesize;
        src_y=1-block_h;
    }
    if(src_x>= w){
        src+= (w-1-src_x);
        src_x=w-1;
    }else if(src_x<=-block_w){
        src+= (1-block_w-src_x);
        src_x=1-block_w;
    }

    start_y= FFMAX(0, -src_y);
    start_x= FFMAX(0, -src_x);
    end_y= FFMIN(block_h, h-src_y);
    end_x= FFMIN(block_w, w-src_x);

    // copy existing part
    for(y=start_y; y<end_y; y++){
        for(x=start_x; x<end_x; x++){
            buf[x + y*linesize]= src[x + y*linesize];
        }
    }

    //top
    for(y=0; y<start_y; y++){
        for(x=start_x; x<end_x; x++){
            buf[x + y*linesize]= buf[x + start_y*linesize];
        }
    }

    //bottom
    for(y=end_y; y<block_h; y++){
        for(x=start_x; x<end_x; x++){
            buf[x + y*linesize]= buf[x + (end_y-1)*linesize];
        }
    }

    for(y=0; y<block_h; y++){
       //left
        for(x=0; x<start_x; x++){
            buf[x + y*linesize]= buf[start_x + y*linesize];
        }

       //right
        for(x=end_x; x<block_w; x++){
            buf[x + y*linesize]= buf[end_x - 1 + y*linesize];
        }
    }
}


static void add_pixels8_c(uint8_t *restrict pixels, DCTELEM *block, int line_size)
{
    int i;
    for(i=0;i<8;i++) {
        pixels[0] += block[0];
        pixels[1] += block[1];
        pixels[2] += block[2];
        pixels[3] += block[3];
        pixels[4] += block[4];
        pixels[5] += block[5];
        pixels[6] += block[6];
        pixels[7] += block[7];
        pixels += line_size;
        block += 8;
    }
}

static void add_pixels4_c(uint8_t *restrict pixels, DCTELEM *block, int line_size)
{
    int i;
    for(i=0;i<4;i++) {
        pixels[0] += block[0];
        pixels[1] += block[1];
        pixels[2] += block[2];
        pixels[3] += block[3];
        pixels += line_size;
        block += 4;
    }
}


#if 0

#define PIXOP2(OPNAME, OP) \
static void OPNAME ## _pixels(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
    int i;\
    for(i=0; i<h; i++){\
        OP(*((uint64_t*)block), AV_RN64(pixels));\
        pixels+=line_size;\
        block +=line_size;\
    }\
}\
\
static void OPNAME ## _no_rnd_pixels_x2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
    int i;\
    for(i=0; i<h; i++){\
        const uint64_t a= AV_RN64(pixels  );\
        const uint64_t b= AV_RN64(pixels+1);\
        OP(*((uint64_t*)block), (a&b) + (((a^b)&0xFEFEFEFEFEFEFEFEULL)>>1));\
        pixels+=line_size;\
        block +=line_size;\
    }\
}\
\
static void OPNAME ## _pixels_x2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
    int i;\
    for(i=0; i<h; i++){\
        const uint64_t a= AV_RN64(pixels  );\
        const uint64_t b= AV_RN64(pixels+1);\
        OP(*((uint64_t*)block), (a|b) - (((a^b)&0xFEFEFEFEFEFEFEFEULL)>>1));\
        pixels+=line_size;\
        block +=line_size;\
    }\
}\
\
static void OPNAME ## _no_rnd_pixels_y2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
    int i;\
    for(i=0; i<h; i++){\
        const uint64_t a= AV_RN64(pixels          );\
        const uint64_t b= AV_RN64(pixels+line_size);\
        OP(*((uint64_t*)block), (a&b) + (((a^b)&0xFEFEFEFEFEFEFEFEULL)>>1));\
        pixels+=line_size;\
        block +=line_size;\
    }\
}\
\
static void OPNAME ## _pixels_y2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
    int i;\
    for(i=0; i<h; i++){\
        const uint64_t a= AV_RN64(pixels          );\
        const uint64_t b= AV_RN64(pixels+line_size);\
        OP(*((uint64_t*)block), (a|b) - (((a^b)&0xFEFEFEFEFEFEFEFEULL)>>1));\
        pixels+=line_size;\
        block +=line_size;\
    }\
}\
\
static void OPNAME ## _pixels_xy2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
        int i;\
        const uint64_t a= AV_RN64(pixels  );\
        const uint64_t b= AV_RN64(pixels+1);\
        uint64_t l0=  (a&0x0303030303030303ULL)\
                    + (b&0x0303030303030303ULL)\
                    + 0x0202020202020202ULL;\
        uint64_t h0= ((a&0xFCFCFCFCFCFCFCFCULL)>>2)\
                   + ((b&0xFCFCFCFCFCFCFCFCULL)>>2);\
        uint64_t l1,h1;\
\
        pixels+=line_size;\
        for(i=0; i<h; i+=2){\
            uint64_t a= AV_RN64(pixels  );\
            uint64_t b= AV_RN64(pixels+1);\
            l1=  (a&0x0303030303030303ULL)\
               + (b&0x0303030303030303ULL);\
            h1= ((a&0xFCFCFCFCFCFCFCFCULL)>>2)\
              + ((b&0xFCFCFCFCFCFCFCFCULL)>>2);\
            OP(*((uint64_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0F0F0F0F0FULL));\
            pixels+=line_size;\
            block +=line_size;\
            a= AV_RN64(pixels  );\
            b= AV_RN64(pixels+1);\
            l0=  (a&0x0303030303030303ULL)\
               + (b&0x0303030303030303ULL)\
               + 0x0202020202020202ULL;\
            h0= ((a&0xFCFCFCFCFCFCFCFCULL)>>2)\
              + ((b&0xFCFCFCFCFCFCFCFCULL)>>2);\
            OP(*((uint64_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0F0F0F0F0FULL));\
            pixels+=line_size;\
            block +=line_size;\
        }\
}\
\
static void OPNAME ## _no_rnd_pixels_xy2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
        int i;\
        const uint64_t a= AV_RN64(pixels  );\
        const uint64_t b= AV_RN64(pixels+1);\
        uint64_t l0=  (a&0x0303030303030303ULL)\
                    + (b&0x0303030303030303ULL)\
                    + 0x0101010101010101ULL;\
        uint64_t h0= ((a&0xFCFCFCFCFCFCFCFCULL)>>2)\
                   + ((b&0xFCFCFCFCFCFCFCFCULL)>>2);\
        uint64_t l1,h1;\
\
        pixels+=line_size;\
        for(i=0; i<h; i+=2){\
            uint64_t a= AV_RN64(pixels  );\
            uint64_t b= AV_RN64(pixels+1);\
            l1=  (a&0x0303030303030303ULL)\
               + (b&0x0303030303030303ULL);\
            h1= ((a&0xFCFCFCFCFCFCFCFCULL)>>2)\
              + ((b&0xFCFCFCFCFCFCFCFCULL)>>2);\
            OP(*((uint64_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0F0F0F0F0FULL));\
            pixels+=line_size;\
            block +=line_size;\
            a= AV_RN64(pixels  );\
            b= AV_RN64(pixels+1);\
            l0=  (a&0x0303030303030303ULL)\
               + (b&0x0303030303030303ULL)\
               + 0x0101010101010101ULL;\
            h0= ((a&0xFCFCFCFCFCFCFCFCULL)>>2)\
              + ((b&0xFCFCFCFCFCFCFCFCULL)>>2);\
            OP(*((uint64_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0F0F0F0F0FULL));\
            pixels+=line_size;\
            block +=line_size;\
        }\
}\
\
CALL_2X_PIXELS(OPNAME ## _pixels16_c    , OPNAME ## _pixels_c    , 8)\
CALL_2X_PIXELS(OPNAME ## _pixels16_x2_c , OPNAME ## _pixels_x2_c , 8)\
CALL_2X_PIXELS(OPNAME ## _pixels16_y2_c , OPNAME ## _pixels_y2_c , 8)\
CALL_2X_PIXELS(OPNAME ## _pixels16_xy2_c, OPNAME ## _pixels_xy2_c, 8)\
CALL_2X_PIXELS(OPNAME ## _no_rnd_pixels16_x2_c , OPNAME ## _no_rnd_pixels_x2_c , 8)\
CALL_2X_PIXELS(OPNAME ## _no_rnd_pixels16_y2_c , OPNAME ## _no_rnd_pixels_y2_c , 8)\
CALL_2X_PIXELS(OPNAME ## _no_rnd_pixels16_xy2_c, OPNAME ## _no_rnd_pixels_xy2_c, 8)

#define op_avg(a, b) a = ( ((a)|(b)) - ((((a)^(b))&0xFEFEFEFEFEFEFEFEULL)>>1) )
#else // 64 bit variant

#define PIXOP2(OPNAME, OP) \
static void OPNAME ## _pixels2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    int i;\
    for(i=0; i<h; i++){\
        OP(*((uint16_t*)(block  )), AV_RN16(pixels  ));\
        pixels+=line_size;\
        block +=line_size;\
    }\
}\
static void OPNAME ## _pixels4_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    int i;\
    for(i=0; i<h; i++){\
        OP(*((uint32_t*)(block  )), AV_RN32(pixels  ));\
        pixels+=line_size;\
        block +=line_size;\
    }\
}\
static void OPNAME ## _pixels8_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    int i;\
    for(i=0; i<h; i++){\
        OP(*((uint32_t*)(block  )), AV_RN32(pixels  ));\
        OP(*((uint32_t*)(block+4)), AV_RN32(pixels+4));\
        pixels+=line_size;\
        block +=line_size;\
    }\
}\
static inline void OPNAME ## _no_rnd_pixels8_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _pixels8_c(block, pixels, line_size, h);\
}\
\
static inline void OPNAME ## _no_rnd_pixels8_l2(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, \
                                                int src_stride1, int src_stride2, int h){\
    int i;\
    for(i=0; i<h; i++){\
        uint32_t a,b;\
        a= AV_RN32(&src1[i*src_stride1  ]);\
        b= AV_RN32(&src2[i*src_stride2  ]);\
        OP(*((uint32_t*)&dst[i*dst_stride  ]), no_rnd_avg32(a, b));\
        a= AV_RN32(&src1[i*src_stride1+4]);\
        b= AV_RN32(&src2[i*src_stride2+4]);\
        OP(*((uint32_t*)&dst[i*dst_stride+4]), no_rnd_avg32(a, b));\
    }\
}\
\
static inline void OPNAME ## _pixels8_l2(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, \
                                                int src_stride1, int src_stride2, int h){\
    int i;\
    for(i=0; i<h; i++){\
        uint32_t a,b;\
        a= AV_RN32(&src1[i*src_stride1  ]);\
        b= AV_RN32(&src2[i*src_stride2  ]);\
        OP(*((uint32_t*)&dst[i*dst_stride  ]), rnd_avg32(a, b));\
        a= AV_RN32(&src1[i*src_stride1+4]);\
        b= AV_RN32(&src2[i*src_stride2+4]);\
        OP(*((uint32_t*)&dst[i*dst_stride+4]), rnd_avg32(a, b));\
    }\
}\
\
static inline void OPNAME ## _pixels4_l2(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, \
                                                int src_stride1, int src_stride2, int h){\
    int i;\
    for(i=0; i<h; i++){\
        uint32_t a,b;\
        a= AV_RN32(&src1[i*src_stride1  ]);\
        b= AV_RN32(&src2[i*src_stride2  ]);\
        OP(*((uint32_t*)&dst[i*dst_stride  ]), rnd_avg32(a, b));\
    }\
}\
\
static inline void OPNAME ## _pixels2_l2(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, \
                                                int src_stride1, int src_stride2, int h){\
    int i;\
    for(i=0; i<h; i++){\
        uint32_t a,b;\
        a= AV_RN16(&src1[i*src_stride1  ]);\
        b= AV_RN16(&src2[i*src_stride2  ]);\
        OP(*((uint16_t*)&dst[i*dst_stride  ]), rnd_avg32(a, b));\
    }\
}\
\
static inline void OPNAME ## _pixels16_l2(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, \
                                                int src_stride1, int src_stride2, int h){\
    OPNAME ## _pixels8_l2(dst  , src1  , src2  , dst_stride, src_stride1, src_stride2, h);\
    OPNAME ## _pixels8_l2(dst+8, src1+8, src2+8, dst_stride, src_stride1, src_stride2, h);\
}\
\
static inline void OPNAME ## _no_rnd_pixels16_l2(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, \
                                                int src_stride1, int src_stride2, int h){\
    OPNAME ## _no_rnd_pixels8_l2(dst  , src1  , src2  , dst_stride, src_stride1, src_stride2, h);\
    OPNAME ## _no_rnd_pixels8_l2(dst+8, src1+8, src2+8, dst_stride, src_stride1, src_stride2, h);\
}\
\
static inline void OPNAME ## _no_rnd_pixels8_x2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _no_rnd_pixels8_l2(block, pixels, pixels+1, line_size, line_size, line_size, h);\
}\
\
static inline void OPNAME ## _pixels8_x2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _pixels8_l2(block, pixels, pixels+1, line_size, line_size, line_size, h);\
}\
\
static inline void OPNAME ## _no_rnd_pixels8_y2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _no_rnd_pixels8_l2(block, pixels, pixels+line_size, line_size, line_size, line_size, h);\
}\
\
static inline void OPNAME ## _pixels8_y2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _pixels8_l2(block, pixels, pixels+line_size, line_size, line_size, line_size, h);\
}\
\
static inline void OPNAME ## _pixels8_l4(uint8_t *dst, const uint8_t *src1, uint8_t *src2, uint8_t *src3, uint8_t *src4,\
                 int dst_stride, int src_stride1, int src_stride2,int src_stride3,int src_stride4, int h){\
    int i;\
    for(i=0; i<h; i++){\
        uint32_t a, b, c, d, l0, l1, h0, h1;\
        a= AV_RN32(&src1[i*src_stride1]);\
        b= AV_RN32(&src2[i*src_stride2]);\
        c= AV_RN32(&src3[i*src_stride3]);\
        d= AV_RN32(&src4[i*src_stride4]);\
        l0=  (a&0x03030303UL)\
           + (b&0x03030303UL)\
           + 0x02020202UL;\
        h0= ((a&0xFCFCFCFCUL)>>2)\
          + ((b&0xFCFCFCFCUL)>>2);\
        l1=  (c&0x03030303UL)\
           + (d&0x03030303UL);\
        h1= ((c&0xFCFCFCFCUL)>>2)\
          + ((d&0xFCFCFCFCUL)>>2);\
        OP(*((uint32_t*)&dst[i*dst_stride]), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
        a= AV_RN32(&src1[i*src_stride1+4]);\
        b= AV_RN32(&src2[i*src_stride2+4]);\
        c= AV_RN32(&src3[i*src_stride3+4]);\
        d= AV_RN32(&src4[i*src_stride4+4]);\
        l0=  (a&0x03030303UL)\
           + (b&0x03030303UL)\
           + 0x02020202UL;\
        h0= ((a&0xFCFCFCFCUL)>>2)\
          + ((b&0xFCFCFCFCUL)>>2);\
        l1=  (c&0x03030303UL)\
           + (d&0x03030303UL);\
        h1= ((c&0xFCFCFCFCUL)>>2)\
          + ((d&0xFCFCFCFCUL)>>2);\
        OP(*((uint32_t*)&dst[i*dst_stride+4]), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
    }\
}\
\
static inline void OPNAME ## _pixels4_x2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _pixels4_l2(block, pixels, pixels+1, line_size, line_size, line_size, h);\
}\
\
static inline void OPNAME ## _pixels4_y2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _pixels4_l2(block, pixels, pixels+line_size, line_size, line_size, line_size, h);\
}\
\
static inline void OPNAME ## _pixels2_x2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _pixels2_l2(block, pixels, pixels+1, line_size, line_size, line_size, h);\
}\
\
static inline void OPNAME ## _pixels2_y2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h){\
    OPNAME ## _pixels2_l2(block, pixels, pixels+line_size, line_size, line_size, line_size, h);\
}\
\
static inline void OPNAME ## _no_rnd_pixels8_l4(uint8_t *dst, const uint8_t *src1, uint8_t *src2, uint8_t *src3, uint8_t *src4,\
                 int dst_stride, int src_stride1, int src_stride2,int src_stride3,int src_stride4, int h){\
    int i;\
    for(i=0; i<h; i++){\
        uint32_t a, b, c, d, l0, l1, h0, h1;\
        a= AV_RN32(&src1[i*src_stride1]);\
        b= AV_RN32(&src2[i*src_stride2]);\
        c= AV_RN32(&src3[i*src_stride3]);\
        d= AV_RN32(&src4[i*src_stride4]);\
        l0=  (a&0x03030303UL)\
           + (b&0x03030303UL)\
           + 0x01010101UL;\
        h0= ((a&0xFCFCFCFCUL)>>2)\
          + ((b&0xFCFCFCFCUL)>>2);\
        l1=  (c&0x03030303UL)\
           + (d&0x03030303UL);\
        h1= ((c&0xFCFCFCFCUL)>>2)\
          + ((d&0xFCFCFCFCUL)>>2);\
        OP(*((uint32_t*)&dst[i*dst_stride]), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
        a= AV_RN32(&src1[i*src_stride1+4]);\
        b= AV_RN32(&src2[i*src_stride2+4]);\
        c= AV_RN32(&src3[i*src_stride3+4]);\
        d= AV_RN32(&src4[i*src_stride4+4]);\
        l0=  (a&0x03030303UL)\
           + (b&0x03030303UL)\
           + 0x01010101UL;\
        h0= ((a&0xFCFCFCFCUL)>>2)\
          + ((b&0xFCFCFCFCUL)>>2);\
        l1=  (c&0x03030303UL)\
           + (d&0x03030303UL);\
        h1= ((c&0xFCFCFCFCUL)>>2)\
          + ((d&0xFCFCFCFCUL)>>2);\
        OP(*((uint32_t*)&dst[i*dst_stride+4]), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
    }\
}\
static inline void OPNAME ## _pixels16_l4(uint8_t *dst, const uint8_t *src1, uint8_t *src2, uint8_t *src3, uint8_t *src4,\
                 int dst_stride, int src_stride1, int src_stride2,int src_stride3,int src_stride4, int h){\
    OPNAME ## _pixels8_l4(dst  , src1  , src2  , src3  , src4  , dst_stride, src_stride1, src_stride2, src_stride3, src_stride4, h);\
    OPNAME ## _pixels8_l4(dst+8, src1+8, src2+8, src3+8, src4+8, dst_stride, src_stride1, src_stride2, src_stride3, src_stride4, h);\
}\
static inline void OPNAME ## _no_rnd_pixels16_l4(uint8_t *dst, const uint8_t *src1, uint8_t *src2, uint8_t *src3, uint8_t *src4,\
                 int dst_stride, int src_stride1, int src_stride2,int src_stride3,int src_stride4, int h){\
    OPNAME ## _no_rnd_pixels8_l4(dst  , src1  , src2  , src3  , src4  , dst_stride, src_stride1, src_stride2, src_stride3, src_stride4, h);\
    OPNAME ## _no_rnd_pixels8_l4(dst+8, src1+8, src2+8, src3+8, src4+8, dst_stride, src_stride1, src_stride2, src_stride3, src_stride4, h);\
}\
\
static inline void OPNAME ## _pixels2_xy2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
        int i, a0, b0, a1, b1;\
        a0= pixels[0];\
        b0= pixels[1] + 2;\
        a0 += b0;\
        b0 += pixels[2];\
\
        pixels+=line_size;\
        for(i=0; i<h; i+=2){\
            a1= pixels[0];\
            b1= pixels[1];\
            a1 += b1;\
            b1 += pixels[2];\
\
            block[0]= (a1+a0)>>2; /* FIXME non put */\
            block[1]= (b1+b0)>>2;\
\
            pixels+=line_size;\
            block +=line_size;\
\
            a0= pixels[0];\
            b0= pixels[1] + 2;\
            a0 += b0;\
            b0 += pixels[2];\
\
            block[0]= (a1+a0)>>2;\
            block[1]= (b1+b0)>>2;\
            pixels+=line_size;\
            block +=line_size;\
        }\
}\
\
static inline void OPNAME ## _pixels4_xy2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
        int i;\
        const uint32_t a= AV_RN32(pixels  );\
        const uint32_t b= AV_RN32(pixels+1);\
        uint32_t l0=  (a&0x03030303UL)\
                    + (b&0x03030303UL)\
                    + 0x02020202UL;\
        uint32_t h0= ((a&0xFCFCFCFCUL)>>2)\
                   + ((b&0xFCFCFCFCUL)>>2);\
        uint32_t l1,h1;\
\
        pixels+=line_size;\
        for(i=0; i<h; i+=2){\
            uint32_t a= AV_RN32(pixels  );\
            uint32_t b= AV_RN32(pixels+1);\
            l1=  (a&0x03030303UL)\
               + (b&0x03030303UL);\
            h1= ((a&0xFCFCFCFCUL)>>2)\
              + ((b&0xFCFCFCFCUL)>>2);\
            OP(*((uint32_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
            pixels+=line_size;\
            block +=line_size;\
            a= AV_RN32(pixels  );\
            b= AV_RN32(pixels+1);\
            l0=  (a&0x03030303UL)\
               + (b&0x03030303UL)\
               + 0x02020202UL;\
            h0= ((a&0xFCFCFCFCUL)>>2)\
              + ((b&0xFCFCFCFCUL)>>2);\
            OP(*((uint32_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
            pixels+=line_size;\
            block +=line_size;\
        }\
}\
\
static inline void OPNAME ## _pixels8_xy2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
    int j;\
    for(j=0; j<2; j++){\
        int i;\
        const uint32_t a= AV_RN32(pixels  );\
        const uint32_t b= AV_RN32(pixels+1);\
        uint32_t l0=  (a&0x03030303UL)\
                    + (b&0x03030303UL)\
                    + 0x02020202UL;\
        uint32_t h0= ((a&0xFCFCFCFCUL)>>2)\
                   + ((b&0xFCFCFCFCUL)>>2);\
        uint32_t l1,h1;\
\
        pixels+=line_size;\
        for(i=0; i<h; i+=2){\
            uint32_t a= AV_RN32(pixels  );\
            uint32_t b= AV_RN32(pixels+1);\
            l1=  (a&0x03030303UL)\
               + (b&0x03030303UL);\
            h1= ((a&0xFCFCFCFCUL)>>2)\
              + ((b&0xFCFCFCFCUL)>>2);\
            OP(*((uint32_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
            pixels+=line_size;\
            block +=line_size;\
            a= AV_RN32(pixels  );\
            b= AV_RN32(pixels+1);\
            l0=  (a&0x03030303UL)\
               + (b&0x03030303UL)\
               + 0x02020202UL;\
            h0= ((a&0xFCFCFCFCUL)>>2)\
              + ((b&0xFCFCFCFCUL)>>2);\
            OP(*((uint32_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
            pixels+=line_size;\
            block +=line_size;\
        }\
        pixels+=4-line_size*(h+1);\
        block +=4-line_size*h;\
    }\
}\
\
static inline void OPNAME ## _no_rnd_pixels8_xy2_c(uint8_t *block, const uint8_t *pixels, int line_size, int h)\
{\
    int j;\
    for(j=0; j<2; j++){\
        int i;\
        const uint32_t a= AV_RN32(pixels  );\
        const uint32_t b= AV_RN32(pixels+1);\
        uint32_t l0=  (a&0x03030303UL)\
                    + (b&0x03030303UL)\
                    + 0x01010101UL;\
        uint32_t h0= ((a&0xFCFCFCFCUL)>>2)\
                   + ((b&0xFCFCFCFCUL)>>2);\
        uint32_t l1,h1;\
\
        pixels+=line_size;\
        for(i=0; i<h; i+=2){\
            uint32_t a= AV_RN32(pixels  );\
            uint32_t b= AV_RN32(pixels+1);\
            l1=  (a&0x03030303UL)\
               + (b&0x03030303UL);\
            h1= ((a&0xFCFCFCFCUL)>>2)\
              + ((b&0xFCFCFCFCUL)>>2);\
            OP(*((uint32_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
            pixels+=line_size;\
            block +=line_size;\
            a= AV_RN32(pixels  );\
            b= AV_RN32(pixels+1);\
            l0=  (a&0x03030303UL)\
               + (b&0x03030303UL)\
               + 0x01010101UL;\
            h0= ((a&0xFCFCFCFCUL)>>2)\
              + ((b&0xFCFCFCFCUL)>>2);\
            OP(*((uint32_t*)block), h0+h1+(((l0+l1)>>2)&0x0F0F0F0FUL));\
            pixels+=line_size;\
            block +=line_size;\
        }\
        pixels+=4-line_size*(h+1);\
        block +=4-line_size*h;\
    }\
}\
\
CALL_2X_PIXELS(OPNAME ## _pixels16_c  , OPNAME ## _pixels8_c  , 8)\
CALL_2X_PIXELS(OPNAME ## _pixels16_x2_c , OPNAME ## _pixels8_x2_c , 8)\
CALL_2X_PIXELS(OPNAME ## _pixels16_y2_c , OPNAME ## _pixels8_y2_c , 8)\
CALL_2X_PIXELS(OPNAME ## _pixels16_xy2_c, OPNAME ## _pixels8_xy2_c, 8)\
CALL_2X_PIXELS(OPNAME ## _no_rnd_pixels16_c  , OPNAME ## _pixels8_c         , 8)\
CALL_2X_PIXELS(OPNAME ## _no_rnd_pixels16_x2_c , OPNAME ## _no_rnd_pixels8_x2_c , 8)\
CALL_2X_PIXELS(OPNAME ## _no_rnd_pixels16_y2_c , OPNAME ## _no_rnd_pixels8_y2_c , 8)\
CALL_2X_PIXELS(OPNAME ## _no_rnd_pixels16_xy2_c, OPNAME ## _no_rnd_pixels8_xy2_c, 8)\

#define op_avg(a, b) a = rnd_avg32(a, b)
#endif
#define op_put(a, b) a = b

PIXOP2(avg, op_avg)
PIXOP2(put, op_put)
#undef op_avg
#undef op_put


#define H264_CHROMA_MC(OPNAME, OP)\
static void OPNAME ## h264_chroma_mc2_c(uint8_t *dst/*align 8*/, uint8_t *src/*align 1*/, int stride, int h, int x, int y){\
    const int A=(8-x)*(8-y);\
    const int B=(  x)*(8-y);\
    const int C=(8-x)*(  y);\
    const int D=(  x)*(  y);\
    int i;\
    \
    assert(x<8 && y<8 && x>=0 && y>=0);\
\
    if(D){\
        for(i=0; i<h; i++){\
            OP(dst[0], (A*src[0] + B*src[1] + C*src[stride+0] + D*src[stride+1]));\
            OP(dst[1], (A*src[1] + B*src[2] + C*src[stride+1] + D*src[stride+2]));\
            dst+= stride;\
            src+= stride;\
        }\
    }else{\
        const int E= B+C;\
        const int step= C ? stride : 1;\
        for(i=0; i<h; i++){\
            OP(dst[0], (A*src[0] + E*src[step+0]));\
            OP(dst[1], (A*src[1] + E*src[step+1]));\
            dst+= stride;\
            src+= stride;\
        }\
    }\
}\
\
static void OPNAME ## h264_chroma_mc4_c(uint8_t *dst/*align 8*/, uint8_t *src/*align 1*/, int stride, int h, int x, int y){\
    const int A=(8-x)*(8-y);\
    const int B=(  x)*(8-y);\
    const int C=(8-x)*(  y);\
    const int D=(  x)*(  y);\
    int i;\
    \
    assert(x<8 && y<8 && x>=0 && y>=0);\
\
    if(D){\
        for(i=0; i<h; i++){\
            OP(dst[0], (A*src[0] + B*src[1] + C*src[stride+0] + D*src[stride+1]));\
            OP(dst[1], (A*src[1] + B*src[2] + C*src[stride+1] + D*src[stride+2]));\
            OP(dst[2], (A*src[2] + B*src[3] + C*src[stride+2] + D*src[stride+3]));\
            OP(dst[3], (A*src[3] + B*src[4] + C*src[stride+3] + D*src[stride+4]));\
            dst+= stride;\
            src+= stride;\
        }\
    }else{\
        const int E= B+C;\
        const int step= C ? stride : 1;\
        for(i=0; i<h; i++){\
            OP(dst[0], (A*src[0] + E*src[step+0]));\
            OP(dst[1], (A*src[1] + E*src[step+1]));\
            OP(dst[2], (A*src[2] + E*src[step+2]));\
            OP(dst[3], (A*src[3] + E*src[step+3]));\
            dst+= stride;\
            src+= stride;\
        }\
    }\
}\
\
static void OPNAME ## h264_chroma_mc8_c(uint8_t *dst/*align 8*/, uint8_t *src/*align 1*/, int stride, int h, int x, int y){\
    const int A=(8-x)*(8-y);\
    const int B=(  x)*(8-y);\
    const int C=(8-x)*(  y);\
    const int D=(  x)*(  y);\
    int i;\
    \
    assert(x<8 && y<8 && x>=0 && y>=0);\
\
    if(D){\
        for(i=0; i<h; i++){\
            OP(dst[0], (A*src[0] + B*src[1] + C*src[stride+0] + D*src[stride+1]));\
            OP(dst[1], (A*src[1] + B*src[2] + C*src[stride+1] + D*src[stride+2]));\
            OP(dst[2], (A*src[2] + B*src[3] + C*src[stride+2] + D*src[stride+3]));\
            OP(dst[3], (A*src[3] + B*src[4] + C*src[stride+3] + D*src[stride+4]));\
            OP(dst[4], (A*src[4] + B*src[5] + C*src[stride+4] + D*src[stride+5]));\
            OP(dst[5], (A*src[5] + B*src[6] + C*src[stride+5] + D*src[stride+6]));\
            OP(dst[6], (A*src[6] + B*src[7] + C*src[stride+6] + D*src[stride+7]));\
            OP(dst[7], (A*src[7] + B*src[8] + C*src[stride+7] + D*src[stride+8]));\
            dst+= stride;\
            src+= stride;\
        }\
    }else{\
        const int E= B+C;\
        const int step= C ? stride : 1;\
        for(i=0; i<h; i++){\
            OP(dst[0], (A*src[0] + E*src[step+0]));\
            OP(dst[1], (A*src[1] + E*src[step+1]));\
            OP(dst[2], (A*src[2] + E*src[step+2]));\
            OP(dst[3], (A*src[3] + E*src[step+3]));\
            OP(dst[4], (A*src[4] + E*src[step+4]));\
            OP(dst[5], (A*src[5] + E*src[step+5]));\
            OP(dst[6], (A*src[6] + E*src[step+6]));\
            OP(dst[7], (A*src[7] + E*src[step+7]));\
            dst+= stride;\
            src+= stride;\
        }\
    }\
}

#define op_avg(a, b) a = (((a)+(((b) + 32)>>6)+1)>>1)
#define op_put(a, b) a = (((b) + 32)>>6)

H264_CHROMA_MC(put_       , op_put)
H264_CHROMA_MC(avg_       , op_avg)
#undef op_avg
#undef op_put


#define op_avg(a, b) a = (((a)+cm[((b) + 16)>>5]+1)>>1)
#define op_put(a, b) a = cm[((b) + 16)>>5]
#define op_put_no_rnd(a, b) a = cm[((b) + 15)>>5]


#undef op_avg
#undef op_put
#undef op_put_no_rnd

#if 1
#define H264_LOWPASS(OPNAME, OP, OP2) \
static av_unused void OPNAME ## h264_qpel2_h_lowpass(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    const int h=2;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    for(i=0; i<h; i++)\
    {\
        OP(dst[0], (src[0]+src[1])*20 - (src[-1]+src[2])*5 + (src[-2]+src[3]));\
        OP(dst[1], (src[1]+src[2])*20 - (src[0 ]+src[3])*5 + (src[-1]+src[4]));\
        dst+=dstStride;\
        src+=srcStride;\
    }\
}\
\
static av_unused void OPNAME ## h264_qpel2_v_lowpass(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    const int w=2;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    for(i=0; i<w; i++)\
    {\
        const int srcB= src[-2*srcStride];\
        const int srcA= src[-1*srcStride];\
        const int src0= src[0 *srcStride];\
        const int src1= src[1 *srcStride];\
        const int src2= src[2 *srcStride];\
        const int src3= src[3 *srcStride];\
        const int src4= src[4 *srcStride];\
        OP(dst[0*dstStride], (src0+src1)*20 - (srcA+src2)*5 + (srcB+src3));\
        OP(dst[1*dstStride], (src1+src2)*20 - (src0+src3)*5 + (srcA+src4));\
        dst++;\
        src++;\
    }\
}\
\
static av_unused void OPNAME ## h264_qpel2_hv_lowpass(uint8_t *dst, int16_t *tmp, uint8_t *src, int dstStride, int tmpStride, int srcStride){\
    const int h=2;\
    const int w=2;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    src -= 2*srcStride;\
    for(i=0; i<h+5; i++)\
    {\
        tmp[0]= (src[0]+src[1])*20 - (src[-1]+src[2])*5 + (src[-2]+src[3]);\
        tmp[1]= (src[1]+src[2])*20 - (src[0 ]+src[3])*5 + (src[-1]+src[4]);\
        tmp+=tmpStride;\
        src+=srcStride;\
    }\
    tmp -= tmpStride*(h+5-2);\
    for(i=0; i<w; i++)\
    {\
        const int tmpB= tmp[-2*tmpStride];\
        const int tmpA= tmp[-1*tmpStride];\
        const int tmp0= tmp[0 *tmpStride];\
        const int tmp1= tmp[1 *tmpStride];\
        const int tmp2= tmp[2 *tmpStride];\
        const int tmp3= tmp[3 *tmpStride];\
        const int tmp4= tmp[4 *tmpStride];\
        OP2(dst[0*dstStride], (tmp0+tmp1)*20 - (tmpA+tmp2)*5 + (tmpB+tmp3));\
        OP2(dst[1*dstStride], (tmp1+tmp2)*20 - (tmp0+tmp3)*5 + (tmpA+tmp4));\
        dst++;\
        tmp++;\
    }\
}\
static void OPNAME ## h264_qpel4_h_lowpass(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    const int h=4;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    for(i=0; i<h; i++)\
    {\
        OP(dst[0], (src[0]+src[1])*20 - (src[-1]+src[2])*5 + (src[-2]+src[3]));\
        OP(dst[1], (src[1]+src[2])*20 - (src[0 ]+src[3])*5 + (src[-1]+src[4]));\
        OP(dst[2], (src[2]+src[3])*20 - (src[1 ]+src[4])*5 + (src[0 ]+src[5]));\
        OP(dst[3], (src[3]+src[4])*20 - (src[2 ]+src[5])*5 + (src[1 ]+src[6]));\
        dst+=dstStride;\
        src+=srcStride;\
    }\
}\
\
static void OPNAME ## h264_qpel4_v_lowpass(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    const int w=4;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    for(i=0; i<w; i++)\
    {\
        const int srcB= src[-2*srcStride];\
        const int srcA= src[-1*srcStride];\
        const int src0= src[0 *srcStride];\
        const int src1= src[1 *srcStride];\
        const int src2= src[2 *srcStride];\
        const int src3= src[3 *srcStride];\
        const int src4= src[4 *srcStride];\
        const int src5= src[5 *srcStride];\
        const int src6= src[6 *srcStride];\
        OP(dst[0*dstStride], (src0+src1)*20 - (srcA+src2)*5 + (srcB+src3));\
        OP(dst[1*dstStride], (src1+src2)*20 - (src0+src3)*5 + (srcA+src4));\
        OP(dst[2*dstStride], (src2+src3)*20 - (src1+src4)*5 + (src0+src5));\
        OP(dst[3*dstStride], (src3+src4)*20 - (src2+src5)*5 + (src1+src6));\
        dst++;\
        src++;\
    }\
}\
\
static void OPNAME ## h264_qpel4_hv_lowpass(uint8_t *dst, int16_t *tmp, uint8_t *src, int dstStride, int tmpStride, int srcStride){\
    const int h=4;\
    const int w=4;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    src -= 2*srcStride;\
    for(i=0; i<h+5; i++)\
    {\
        tmp[0]= (src[0]+src[1])*20 - (src[-1]+src[2])*5 + (src[-2]+src[3]);\
        tmp[1]= (src[1]+src[2])*20 - (src[0 ]+src[3])*5 + (src[-1]+src[4]);\
        tmp[2]= (src[2]+src[3])*20 - (src[1 ]+src[4])*5 + (src[0 ]+src[5]);\
        tmp[3]= (src[3]+src[4])*20 - (src[2 ]+src[5])*5 + (src[1 ]+src[6]);\
        tmp+=tmpStride;\
        src+=srcStride;\
    }\
    tmp -= tmpStride*(h+5-2);\
    for(i=0; i<w; i++)\
    {\
        const int tmpB= tmp[-2*tmpStride];\
        const int tmpA= tmp[-1*tmpStride];\
        const int tmp0= tmp[0 *tmpStride];\
        const int tmp1= tmp[1 *tmpStride];\
        const int tmp2= tmp[2 *tmpStride];\
        const int tmp3= tmp[3 *tmpStride];\
        const int tmp4= tmp[4 *tmpStride];\
        const int tmp5= tmp[5 *tmpStride];\
        const int tmp6= tmp[6 *tmpStride];\
        OP2(dst[0*dstStride], (tmp0+tmp1)*20 - (tmpA+tmp2)*5 + (tmpB+tmp3));\
        OP2(dst[1*dstStride], (tmp1+tmp2)*20 - (tmp0+tmp3)*5 + (tmpA+tmp4));\
        OP2(dst[2*dstStride], (tmp2+tmp3)*20 - (tmp1+tmp4)*5 + (tmp0+tmp5));\
        OP2(dst[3*dstStride], (tmp3+tmp4)*20 - (tmp2+tmp5)*5 + (tmp1+tmp6));\
        dst++;\
        tmp++;\
    }\
}\
\
static void OPNAME ## h264_qpel8_h_lowpass(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    const int h=8;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    for(i=0; i<h; i++)\
    {\
        OP(dst[0], (src[0]+src[1])*20 - (src[-1]+src[2])*5 + (src[-2]+src[3 ]));\
        OP(dst[1], (src[1]+src[2])*20 - (src[0 ]+src[3])*5 + (src[-1]+src[4 ]));\
        OP(dst[2], (src[2]+src[3])*20 - (src[1 ]+src[4])*5 + (src[0 ]+src[5 ]));\
        OP(dst[3], (src[3]+src[4])*20 - (src[2 ]+src[5])*5 + (src[1 ]+src[6 ]));\
        OP(dst[4], (src[4]+src[5])*20 - (src[3 ]+src[6])*5 + (src[2 ]+src[7 ]));\
        OP(dst[5], (src[5]+src[6])*20 - (src[4 ]+src[7])*5 + (src[3 ]+src[8 ]));\
        OP(dst[6], (src[6]+src[7])*20 - (src[5 ]+src[8])*5 + (src[4 ]+src[9 ]));\
        OP(dst[7], (src[7]+src[8])*20 - (src[6 ]+src[9])*5 + (src[5 ]+src[10]));\
        dst+=dstStride;\
        src+=srcStride;\
    }\
}\
\
static void OPNAME ## h264_qpel8_v_lowpass(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    const int w=8;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    for(i=0; i<w; i++)\
    {\
        const int srcB= src[-2*srcStride];\
        const int srcA= src[-1*srcStride];\
        const int src0= src[0 *srcStride];\
        const int src1= src[1 *srcStride];\
        const int src2= src[2 *srcStride];\
        const int src3= src[3 *srcStride];\
        const int src4= src[4 *srcStride];\
        const int src5= src[5 *srcStride];\
        const int src6= src[6 *srcStride];\
        const int src7= src[7 *srcStride];\
        const int src8= src[8 *srcStride];\
        const int src9= src[9 *srcStride];\
        const int src10=src[10*srcStride];\
        OP(dst[0*dstStride], (src0+src1)*20 - (srcA+src2)*5 + (srcB+src3));\
        OP(dst[1*dstStride], (src1+src2)*20 - (src0+src3)*5 + (srcA+src4));\
        OP(dst[2*dstStride], (src2+src3)*20 - (src1+src4)*5 + (src0+src5));\
        OP(dst[3*dstStride], (src3+src4)*20 - (src2+src5)*5 + (src1+src6));\
        OP(dst[4*dstStride], (src4+src5)*20 - (src3+src6)*5 + (src2+src7));\
        OP(dst[5*dstStride], (src5+src6)*20 - (src4+src7)*5 + (src3+src8));\
        OP(dst[6*dstStride], (src6+src7)*20 - (src5+src8)*5 + (src4+src9));\
        OP(dst[7*dstStride], (src7+src8)*20 - (src6+src9)*5 + (src5+src10));\
        dst++;\
        src++;\
    }\
}\
\
static void OPNAME ## h264_qpel8_hv_lowpass(uint8_t *dst, int16_t *tmp, uint8_t *src, int dstStride, int tmpStride, int srcStride){\
    const int h=8;\
    const int w=8;\
    uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;\
    int i;\
    src -= 2*srcStride;\
    for(i=0; i<h+5; i++)\
    {\
        tmp[0]= (src[0]+src[1])*20 - (src[-1]+src[2])*5 + (src[-2]+src[3 ]);\
        tmp[1]= (src[1]+src[2])*20 - (src[0 ]+src[3])*5 + (src[-1]+src[4 ]);\
        tmp[2]= (src[2]+src[3])*20 - (src[1 ]+src[4])*5 + (src[0 ]+src[5 ]);\
        tmp[3]= (src[3]+src[4])*20 - (src[2 ]+src[5])*5 + (src[1 ]+src[6 ]);\
        tmp[4]= (src[4]+src[5])*20 - (src[3 ]+src[6])*5 + (src[2 ]+src[7 ]);\
        tmp[5]= (src[5]+src[6])*20 - (src[4 ]+src[7])*5 + (src[3 ]+src[8 ]);\
        tmp[6]= (src[6]+src[7])*20 - (src[5 ]+src[8])*5 + (src[4 ]+src[9 ]);\
        tmp[7]= (src[7]+src[8])*20 - (src[6 ]+src[9])*5 + (src[5 ]+src[10]);\
        tmp+=tmpStride;\
        src+=srcStride;\
    }\
    tmp -= tmpStride*(h+5-2);\
    for(i=0; i<w; i++)\
    {\
        const int tmpB= tmp[-2*tmpStride];\
        const int tmpA= tmp[-1*tmpStride];\
        const int tmp0= tmp[0 *tmpStride];\
        const int tmp1= tmp[1 *tmpStride];\
        const int tmp2= tmp[2 *tmpStride];\
        const int tmp3= tmp[3 *tmpStride];\
        const int tmp4= tmp[4 *tmpStride];\
        const int tmp5= tmp[5 *tmpStride];\
        const int tmp6= tmp[6 *tmpStride];\
        const int tmp7= tmp[7 *tmpStride];\
        const int tmp8= tmp[8 *tmpStride];\
        const int tmp9= tmp[9 *tmpStride];\
        const int tmp10=tmp[10*tmpStride];\
        OP2(dst[0*dstStride], (tmp0+tmp1)*20 - (tmpA+tmp2)*5 + (tmpB+tmp3));\
        OP2(dst[1*dstStride], (tmp1+tmp2)*20 - (tmp0+tmp3)*5 + (tmpA+tmp4));\
        OP2(dst[2*dstStride], (tmp2+tmp3)*20 - (tmp1+tmp4)*5 + (tmp0+tmp5));\
        OP2(dst[3*dstStride], (tmp3+tmp4)*20 - (tmp2+tmp5)*5 + (tmp1+tmp6));\
        OP2(dst[4*dstStride], (tmp4+tmp5)*20 - (tmp3+tmp6)*5 + (tmp2+tmp7));\
        OP2(dst[5*dstStride], (tmp5+tmp6)*20 - (tmp4+tmp7)*5 + (tmp3+tmp8));\
        OP2(dst[6*dstStride], (tmp6+tmp7)*20 - (tmp5+tmp8)*5 + (tmp4+tmp9));\
        OP2(dst[7*dstStride], (tmp7+tmp8)*20 - (tmp6+tmp9)*5 + (tmp5+tmp10));\
        dst++;\
        tmp++;\
    }\
}\
\
static void OPNAME ## h264_qpel16_v_lowpass(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    OPNAME ## h264_qpel8_v_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## h264_qpel8_v_lowpass(dst+8, src+8, dstStride, srcStride);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## h264_qpel8_v_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## h264_qpel8_v_lowpass(dst+8, src+8, dstStride, srcStride);\
}\
\
static void OPNAME ## h264_qpel16_h_lowpass(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    OPNAME ## h264_qpel8_h_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## h264_qpel8_h_lowpass(dst+8, src+8, dstStride, srcStride);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## h264_qpel8_h_lowpass(dst  , src  , dstStride, srcStride);\
    OPNAME ## h264_qpel8_h_lowpass(dst+8, src+8, dstStride, srcStride);\
}\
\
static void OPNAME ## h264_qpel16_hv_lowpass(uint8_t *dst, int16_t *tmp, uint8_t *src, int dstStride, int tmpStride, int srcStride){\
    OPNAME ## h264_qpel8_hv_lowpass(dst  , tmp  , src  , dstStride, tmpStride, srcStride);\
    OPNAME ## h264_qpel8_hv_lowpass(dst+8, tmp+8, src+8, dstStride, tmpStride, srcStride);\
    src += 8*srcStride;\
    dst += 8*dstStride;\
    OPNAME ## h264_qpel8_hv_lowpass(dst  , tmp  , src  , dstStride, tmpStride, srcStride);\
    OPNAME ## h264_qpel8_hv_lowpass(dst+8, tmp+8, src+8, dstStride, tmpStride, srcStride);\
}\

#define H264_MC(OPNAME, SIZE) \
static void OPNAME ## h264_qpel ## SIZE ## _mc00_c (uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## pixels ## SIZE ## _c(dst, src, stride, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc10_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t half[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass(half, src, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2(dst, src, half, stride, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc20_c(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## h264_qpel ## SIZE ## _h_lowpass(dst, src, stride, stride);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc30_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t half[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass(half, src, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2(dst, src+1, half, stride, stride, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc01_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    uint8_t half[SIZE*SIZE];\
    copy_block ## SIZE (full, src - stride*2, SIZE,  stride, SIZE + 5);\
    put_h264_qpel ## SIZE ## _v_lowpass(half, full_mid, SIZE, SIZE);\
    OPNAME ## pixels ## SIZE ## _l2(dst, full_mid, half, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc02_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    copy_block ## SIZE (full, src - stride*2, SIZE,  stride, SIZE + 5);\
    OPNAME ## h264_qpel ## SIZE ## _v_lowpass(dst, full_mid, stride, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc03_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    uint8_t half[SIZE*SIZE];\
    copy_block ## SIZE (full, src - stride*2, SIZE,  stride, SIZE + 5);\
    put_h264_qpel ## SIZE ## _v_lowpass(half, full_mid, SIZE, SIZE);\
    OPNAME ## pixels ## SIZE ## _l2(dst, full_mid+SIZE, half, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc11_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    uint8_t halfH[SIZE*SIZE];\
    uint8_t halfV[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass(halfH, src, SIZE, stride);\
    copy_block ## SIZE (full, src - stride*2, SIZE,  stride, SIZE + 5);\
    put_h264_qpel ## SIZE ## _v_lowpass(halfV, full_mid, SIZE, SIZE);\
    OPNAME ## pixels ## SIZE ## _l2(dst, halfH, halfV, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc31_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    uint8_t halfH[SIZE*SIZE];\
    uint8_t halfV[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass(halfH, src, SIZE, stride);\
    copy_block ## SIZE (full, src - stride*2 + 1, SIZE,  stride, SIZE + 5);\
    put_h264_qpel ## SIZE ## _v_lowpass(halfV, full_mid, SIZE, SIZE);\
    OPNAME ## pixels ## SIZE ## _l2(dst, halfH, halfV, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc13_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    uint8_t halfH[SIZE*SIZE];\
    uint8_t halfV[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass(halfH, src + stride, SIZE, stride);\
    copy_block ## SIZE (full, src - stride*2, SIZE,  stride, SIZE + 5);\
    put_h264_qpel ## SIZE ## _v_lowpass(halfV, full_mid, SIZE, SIZE);\
    OPNAME ## pixels ## SIZE ## _l2(dst, halfH, halfV, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc33_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    uint8_t halfH[SIZE*SIZE];\
    uint8_t halfV[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass(halfH, src + stride, SIZE, stride);\
    copy_block ## SIZE (full, src - stride*2 + 1, SIZE,  stride, SIZE + 5);\
    put_h264_qpel ## SIZE ## _v_lowpass(halfV, full_mid, SIZE, SIZE);\
    OPNAME ## pixels ## SIZE ## _l2(dst, halfH, halfV, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc22_c(uint8_t *dst, uint8_t *src, int stride){\
    int16_t tmp[SIZE*(SIZE+5)];\
    OPNAME ## h264_qpel ## SIZE ## _hv_lowpass(dst, tmp, src, stride, SIZE, stride);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc21_c(uint8_t *dst, uint8_t *src, int stride){\
    int16_t tmp[SIZE*(SIZE+5)];\
    uint8_t halfH[SIZE*SIZE];\
    uint8_t halfHV[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass(halfH, src, SIZE, stride);\
    put_h264_qpel ## SIZE ## _hv_lowpass(halfHV, tmp, src, SIZE, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2(dst, halfH, halfHV, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc23_c(uint8_t *dst, uint8_t *src, int stride){\
    int16_t tmp[SIZE*(SIZE+5)];\
    uint8_t halfH[SIZE*SIZE];\
    uint8_t halfHV[SIZE*SIZE];\
    put_h264_qpel ## SIZE ## _h_lowpass(halfH, src + stride, SIZE, stride);\
    put_h264_qpel ## SIZE ## _hv_lowpass(halfHV, tmp, src, SIZE, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2(dst, halfH, halfHV, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc12_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    int16_t tmp[SIZE*(SIZE+5)];\
    uint8_t halfV[SIZE*SIZE];\
    uint8_t halfHV[SIZE*SIZE];\
    copy_block ## SIZE (full, src - stride*2, SIZE,  stride, SIZE + 5);\
    put_h264_qpel ## SIZE ## _v_lowpass(halfV, full_mid, SIZE, SIZE);\
    put_h264_qpel ## SIZE ## _hv_lowpass(halfHV, tmp, src, SIZE, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2(dst, halfV, halfHV, stride, SIZE, SIZE, SIZE);\
}\
\
static void OPNAME ## h264_qpel ## SIZE ## _mc32_c(uint8_t *dst, uint8_t *src, int stride){\
    uint8_t full[SIZE*(SIZE+5)];\
    uint8_t * const full_mid= full + SIZE*2;\
    int16_t tmp[SIZE*(SIZE+5)];\
    uint8_t halfV[SIZE*SIZE];\
    uint8_t halfHV[SIZE*SIZE];\
    copy_block ## SIZE (full, src - stride*2 + 1, SIZE,  stride, SIZE + 5);\
    put_h264_qpel ## SIZE ## _v_lowpass(halfV, full_mid, SIZE, SIZE);\
    put_h264_qpel ## SIZE ## _hv_lowpass(halfHV, tmp, src, SIZE, SIZE, stride);\
    OPNAME ## pixels ## SIZE ## _l2(dst, halfV, halfHV, stride, SIZE, SIZE, SIZE);\
}\

#define op_avg(a, b)  a = (((a)+cm[((b) + 16)>>5]+1)>>1)
#define op_put(a, b)  a = cm[((b) + 16)>>5]
#define op2_avg(a, b)  a = (((a)+cm[((b) + 512)>>10]+1)>>1)
#define op2_put(a, b)  a = cm[((b) + 512)>>10]

H264_LOWPASS(put_       , op_put, op2_put)
H264_LOWPASS(avg_       , op_avg, op2_avg)
H264_MC(put_, 2)
H264_MC(put_, 4)
H264_MC(put_, 8)
H264_MC(put_, 16)
H264_MC(avg_, 4)
H264_MC(avg_, 8)
H264_MC(avg_, 16)

#undef op_avg
#undef op_put
#undef op2_avg
#undef op2_put
#endif

#define op_scale1(x)  block[x] = av_clip_uint8( (block[x]*weight + offset) >> log2_denom )
#define op_scale2(x)  dst[x] = av_clip_uint8( (src[x]*weights + dst[x]*weightd + offset) >> (log2_denom+1))
#define H264_WEIGHT(W,H) \
static void weight_h264_pixels ## W ## x ## H ## _c(uint8_t *block, int stride, int log2_denom, int weight, int offset){ \
    int y; \
    offset <<= log2_denom; \
    if(log2_denom) offset += 1<<(log2_denom-1); \
    for(y=0; y<H; y++, block += stride){ \
        op_scale1(0); \
        op_scale1(1); \
        if(W==2) continue; \
        op_scale1(2); \
        op_scale1(3); \
        if(W==4) continue; \
        op_scale1(4); \
        op_scale1(5); \
        op_scale1(6); \
        op_scale1(7); \
        if(W==8) continue; \
        op_scale1(8); \
        op_scale1(9); \
        op_scale1(10); \
        op_scale1(11); \
        op_scale1(12); \
        op_scale1(13); \
        op_scale1(14); \
        op_scale1(15); \
    } \
} \
static void biweight_h264_pixels ## W ## x ## H ## _c(uint8_t *dst, uint8_t *src, int stride, int log2_denom, int weightd, int weights, int offset){ \
    int y; \
    offset = ((offset + 1) | 1) << log2_denom; \
    for(y=0; y<H; y++, dst += stride, src += stride){ \
        op_scale2(0); \
        op_scale2(1); \
        if(W==2) continue; \
        op_scale2(2); \
        op_scale2(3); \
        if(W==4) continue; \
        op_scale2(4); \
        op_scale2(5); \
        op_scale2(6); \
        op_scale2(7); \
        if(W==8) continue; \
        op_scale2(8); \
        op_scale2(9); \
        op_scale2(10); \
        op_scale2(11); \
        op_scale2(12); \
        op_scale2(13); \
        op_scale2(14); \
        op_scale2(15); \
    } \
}

H264_WEIGHT(16,16)
H264_WEIGHT(16,8)
H264_WEIGHT(8,16)
H264_WEIGHT(8,8)
H264_WEIGHT(8,4)
H264_WEIGHT(4,8)
H264_WEIGHT(4,4)
H264_WEIGHT(4,2)
H264_WEIGHT(2,4)
H264_WEIGHT(2,2)

#undef op_scale1
#undef op_scale2
#undef H264_WEIGHT


static inline void h264_loop_filter_luma_c(uint8_t *pix, int xstride, int ystride, int alpha, int beta, int8_t *tc0)
{
    int i, d;
    for( i = 0; i < 4; i++ ) {
        if( tc0[i] < 0 ) {
            pix += 4*ystride;
            continue;
        }
        for( d = 0; d < 4; d++ ) {
            const int p0 = pix[-1*xstride];
            const int p1 = pix[-2*xstride];
            const int p2 = pix[-3*xstride];
            const int q0 = pix[0];
            const int q1 = pix[1*xstride];
            const int q2 = pix[2*xstride];

            if( FFABS( p0 - q0 ) < alpha &&
                FFABS( p1 - p0 ) < beta &&
                FFABS( q1 - q0 ) < beta ) {

                int tc = tc0[i];
                int i_delta;

                if( FFABS( p2 - p0 ) < beta ) {
                    pix[-2*xstride] = p1 + av_clip( (( p2 + ( ( p0 + q0 + 1 ) >> 1 ) ) >> 1) - p1, -tc0[i], tc0[i] );
                    tc++;
                }
                if( FFABS( q2 - q0 ) < beta ) {
                    pix[   xstride] = q1 + av_clip( (( q2 + ( ( p0 + q0 + 1 ) >> 1 ) ) >> 1) - q1, -tc0[i], tc0[i] );
                    tc++;
                }

                i_delta = av_clip( (((q0 - p0 ) << 2) + (p1 - q1) + 4) >> 3, -tc, tc );
                pix[-xstride] = av_clip_uint8( p0 + i_delta );    /* p0' */
                pix[0]        = av_clip_uint8( q0 - i_delta );    /* q0' */
            }
            pix += ystride;
        }
    }
}
static void h264_v_loop_filter_luma_c(uint8_t *pix, int stride, int alpha, int beta, int8_t *tc0)
{
    h264_loop_filter_luma_c(pix, stride, 1, alpha, beta, tc0);
}
static void h264_h_loop_filter_luma_c(uint8_t *pix, int stride, int alpha, int beta, int8_t *tc0)
{
    h264_loop_filter_luma_c(pix, 1, stride, alpha, beta, tc0);
}

static inline void h264_loop_filter_chroma_c(uint8_t *pix, int xstride, int ystride, int alpha, int beta, int8_t *tc0)
{
    int i, d;
    for( i = 0; i < 4; i++ ) {
        const int tc = tc0[i];
        if( tc <= 0 ) {
            pix += 2*ystride;
            continue;
        }
        for( d = 0; d < 2; d++ ) {
            const int p0 = pix[-1*xstride];
            const int p1 = pix[-2*xstride];
            const int q0 = pix[0];
            const int q1 = pix[1*xstride];

            if( FFABS( p0 - q0 ) < alpha &&
                FFABS( p1 - p0 ) < beta &&
                FFABS( q1 - q0 ) < beta ) {

                int delta = av_clip( (((q0 - p0 ) << 2) + (p1 - q1) + 4) >> 3, -tc, tc );

                pix[-xstride] = av_clip_uint8( p0 + delta );    /* p0' */
                pix[0]        = av_clip_uint8( q0 - delta );    /* q0' */
            }
            pix += ystride;
        }
    }
}
static void h264_v_loop_filter_chroma_c(uint8_t *pix, int stride, int alpha, int beta, int8_t *tc0)
{
    h264_loop_filter_chroma_c(pix, stride, 1, alpha, beta, tc0);
}
static void h264_h_loop_filter_chroma_c(uint8_t *pix, int stride, int alpha, int beta, int8_t *tc0)
{
    h264_loop_filter_chroma_c(pix, 1, stride, alpha, beta, tc0);
}

static inline void h264_loop_filter_chroma_intra_c(uint8_t *pix, int xstride, int ystride, int alpha, int beta)
{
    int d;
    for( d = 0; d < 8; d++ ) {
        const int p0 = pix[-1*xstride];
        const int p1 = pix[-2*xstride];
        const int q0 = pix[0];
        const int q1 = pix[1*xstride];

        if( FFABS( p0 - q0 ) < alpha &&
            FFABS( p1 - p0 ) < beta &&
            FFABS( q1 - q0 ) < beta ) {

            pix[-xstride] = ( 2*p1 + p0 + q1 + 2 ) >> 2;   /* p0' */
            pix[0]        = ( 2*q1 + q0 + p1 + 2 ) >> 2;   /* q0' */
        }
        pix += ystride;
    }
}
static void h264_v_loop_filter_chroma_intra_c(uint8_t *pix, int stride, int alpha, int beta)
{
    h264_loop_filter_chroma_intra_c(pix, stride, 1, alpha, beta);
}
static void h264_h_loop_filter_chroma_intra_c(uint8_t *pix, int stride, int alpha, int beta)
{
    h264_loop_filter_chroma_intra_c(pix, 1, stride, alpha, beta);
}

static inline int pix_abs16_c(void *v, uint8_t *pix1, uint8_t *pix2, int line_size, int h)
{
    int s, i;

    s = 0;
    for(i=0;i<h;i++) {
        s += abs(pix1[0] - pix2[0]);
        s += abs(pix1[1] - pix2[1]);
        s += abs(pix1[2] - pix2[2]);
        s += abs(pix1[3] - pix2[3]);
        s += abs(pix1[4] - pix2[4]);
        s += abs(pix1[5] - pix2[5]);
        s += abs(pix1[6] - pix2[6]);
        s += abs(pix1[7] - pix2[7]);
        s += abs(pix1[8] - pix2[8]);
        s += abs(pix1[9] - pix2[9]);
        s += abs(pix1[10] - pix2[10]);
        s += abs(pix1[11] - pix2[11]);
        s += abs(pix1[12] - pix2[12]);
        s += abs(pix1[13] - pix2[13]);
        s += abs(pix1[14] - pix2[14]);
        s += abs(pix1[15] - pix2[15]);
        pix1 += line_size;
        pix2 += line_size;
    }
    return s;
}

static inline int pix_abs8_c(void *v, uint8_t *pix1, uint8_t *pix2, int line_size, int h)
{
    int s, i;

    s = 0;
    for(i=0;i<h;i++) {
        s += abs(pix1[0] - pix2[0]);
        s += abs(pix1[1] - pix2[1]);
        s += abs(pix1[2] - pix2[2]);
        s += abs(pix1[3] - pix2[3]);
        s += abs(pix1[4] - pix2[4]);
        s += abs(pix1[5] - pix2[5]);
        s += abs(pix1[6] - pix2[6]);
        s += abs(pix1[7] - pix2[7]);
        pix1 += line_size;
        pix2 += line_size;
    }
    return s;
}


/**
 * memset(blocks, 0, sizeof(DCTELEM)*6*64)
 */
static void clear_blocks_c(DCTELEM *blocks)
{
    memset(blocks, 0, sizeof(DCTELEM)*6*64);
}

static void just_return(void *mem av_unused, int stride av_unused, int h av_unused) { return; }

/* init static data */
void dsputil_static_init(void)
{
    int i;

    for(i = 0; i < 256; i++) 
        ff_cropTbl[i + MAX_NEG_CROP] = i;

    for(i = 0;i < MAX_NEG_CROP; i++) {
        ff_cropTbl[i] = 0;
        ff_cropTbl[i + MAX_NEG_CROP + 256] = 255;
    }

    for(i = 0; i < 512; i++) {
        ff_squareTbl[i] = (i - 256) * (i - 256);
    }
}

int ff_check_alignment(void){
    static int did_fail=0;
    DECLARE_ALIGNED_16(int, aligned);

    if((long)&aligned & 15){
        if(!did_fail){
#if defined(HAVE_MMX) || defined(HAVE_ALTIVEC)
            av_log(NULL, AV_LOG_ERROR,
                "Compiler did not align stack variables. Libavcodec has been miscompiled\n"
                "and may be very slow or crash. This is not a bug in libavcodec,\n"
                "but in the compiler. You may try recompiling using gcc >= 4.2.\n"
                "Do not report crashes to FFmpeg developers.\n");
#endif
            did_fail=1;
        }
        return -1;
    }
    return 0;
}

void dsputil_init(DSPContext* c, AVCodecContext *avctx)
{
    ff_check_alignment();

    c->h264_idct_add = ff_h264_idct_add_c;
    c->h264_idct8_add = ff_h264_idct8_add_c;
    c->h264_idct_dc_add = ff_h264_idct_dc_add_c;
    c->h264_idct8_dc_add = ff_h264_idct8_dc_add_c;
    c->add_pixels8 = add_pixels8_c;
    c->add_pixels4 = add_pixels4_c;
    c->clear_blocks = clear_blocks_c;

   


#define dspfunc(PFX, IDX, NUM) \
    c->PFX ## _pixels_tab[IDX][ 0] = PFX ## NUM ## _mc00_c; \
    c->PFX ## _pixels_tab[IDX][ 1] = PFX ## NUM ## _mc10_c; \
    c->PFX ## _pixels_tab[IDX][ 2] = PFX ## NUM ## _mc20_c; \
    c->PFX ## _pixels_tab[IDX][ 3] = PFX ## NUM ## _mc30_c; \
    c->PFX ## _pixels_tab[IDX][ 4] = PFX ## NUM ## _mc01_c; \
    c->PFX ## _pixels_tab[IDX][ 5] = PFX ## NUM ## _mc11_c; \
    c->PFX ## _pixels_tab[IDX][ 6] = PFX ## NUM ## _mc21_c; \
    c->PFX ## _pixels_tab[IDX][ 7] = PFX ## NUM ## _mc31_c; \
    c->PFX ## _pixels_tab[IDX][ 8] = PFX ## NUM ## _mc02_c; \
    c->PFX ## _pixels_tab[IDX][ 9] = PFX ## NUM ## _mc12_c; \
    c->PFX ## _pixels_tab[IDX][10] = PFX ## NUM ## _mc22_c; \
    c->PFX ## _pixels_tab[IDX][11] = PFX ## NUM ## _mc32_c; \
    c->PFX ## _pixels_tab[IDX][12] = PFX ## NUM ## _mc03_c; \
    c->PFX ## _pixels_tab[IDX][13] = PFX ## NUM ## _mc13_c; \
    c->PFX ## _pixels_tab[IDX][14] = PFX ## NUM ## _mc23_c; \
    c->PFX ## _pixels_tab[IDX][15] = PFX ## NUM ## _mc33_c


    dspfunc(put_h264_qpel, 0, 16);
    dspfunc(put_h264_qpel, 1, 8);
    dspfunc(put_h264_qpel, 2, 4);
    dspfunc(put_h264_qpel, 3, 2);
    dspfunc(avg_h264_qpel, 0, 16);
    dspfunc(avg_h264_qpel, 1, 8);
    dspfunc(avg_h264_qpel, 2, 4);
#undef dspfunc

    c->put_h264_chroma_pixels_tab[0]= put_h264_chroma_mc8_c;
    c->put_h264_chroma_pixels_tab[1]= put_h264_chroma_mc4_c;
    c->put_h264_chroma_pixels_tab[2]= put_h264_chroma_mc2_c;
    c->avg_h264_chroma_pixels_tab[0]= avg_h264_chroma_mc8_c;
    c->avg_h264_chroma_pixels_tab[1]= avg_h264_chroma_mc4_c;
    c->avg_h264_chroma_pixels_tab[2]= avg_h264_chroma_mc2_c;
    

    c->weight_h264_pixels_tab[0]= weight_h264_pixels16x16_c;
    c->weight_h264_pixels_tab[1]= weight_h264_pixels16x8_c;
    c->weight_h264_pixels_tab[2]= weight_h264_pixels8x16_c;
    c->weight_h264_pixels_tab[3]= weight_h264_pixels8x8_c;
    c->weight_h264_pixels_tab[4]= weight_h264_pixels8x4_c;
    c->weight_h264_pixels_tab[5]= weight_h264_pixels4x8_c;
    c->weight_h264_pixels_tab[6]= weight_h264_pixels4x4_c;
    c->weight_h264_pixels_tab[7]= weight_h264_pixels4x2_c;
    c->weight_h264_pixels_tab[8]= weight_h264_pixels2x4_c;
    c->weight_h264_pixels_tab[9]= weight_h264_pixels2x2_c;
    c->biweight_h264_pixels_tab[0]= biweight_h264_pixels16x16_c;
    c->biweight_h264_pixels_tab[1]= biweight_h264_pixels16x8_c;
    c->biweight_h264_pixels_tab[2]= biweight_h264_pixels8x16_c;
    c->biweight_h264_pixels_tab[3]= biweight_h264_pixels8x8_c;
    c->biweight_h264_pixels_tab[4]= biweight_h264_pixels8x4_c;
    c->biweight_h264_pixels_tab[5]= biweight_h264_pixels4x8_c;
    c->biweight_h264_pixels_tab[6]= biweight_h264_pixels4x4_c;
    c->biweight_h264_pixels_tab[7]= biweight_h264_pixels4x2_c;
    c->biweight_h264_pixels_tab[8]= biweight_h264_pixels2x4_c;
    c->biweight_h264_pixels_tab[9]= biweight_h264_pixels2x2_c;

    c->draw_edges = draw_edges_c;
    c->sad[0]= pix_abs16_c;
    c->sad[1]= pix_abs8_c;

    c->h264_v_loop_filter_luma= h264_v_loop_filter_luma_c;
    c->h264_h_loop_filter_luma= h264_h_loop_filter_luma_c;
    c->h264_v_loop_filter_chroma= h264_v_loop_filter_chroma_c;
    c->h264_h_loop_filter_chroma= h264_h_loop_filter_chroma_c;
    c->h264_v_loop_filter_chroma_intra= h264_v_loop_filter_chroma_intra_c;
    c->h264_h_loop_filter_chroma_intra= h264_h_loop_filter_chroma_intra_c;
    c->h264_loop_filter_strength= NULL;

    c->prefetch= just_return;


#if defined(HAVE_MMX)
    dsputil_init_mmx   (c, avctx);	
#endif

}

