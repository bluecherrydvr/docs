/*
 * MMX optimized DSP utils
 * Copyright (c) 2000, 2001 Fabrice Bellard.
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
 *
 * MMX optimization by Nick Kurshev <nickols_k@mail.ru>
 */

#include "x86_cpu.h"
#include "dsputil.h"
#include "dsputil_mmx.h"
#include "mmx.h"

//#undef NDEBUG
//#include <assert.h>

int mm_flags; /* multimedia extension flags */

/* pixel operations */
DECLARE_ALIGNED_8 (const uint64_t, ff_bone) = 0x0101010101010101ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_wtwo) = 0x0002000200020002ULL;

DECLARE_ALIGNED_16(const uint64_t, ff_pdw_80000000[2]) =
{0x8000000080000000ULL, 0x8000000080000000ULL};

DECLARE_ALIGNED_8 (const uint64_t, ff_pw_3  ) = 0x0003000300030003ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pw_4  ) = 0x0004000400040004ULL;
DECLARE_ALIGNED_16(const xmm_t,    ff_pw_5  ) = {0x0005000500050005ULL, 0x0005000500050005ULL};
DECLARE_ALIGNED_16(const xmm_t,    ff_pw_8  ) = {0x0008000800080008ULL, 0x0008000800080008ULL};
DECLARE_ALIGNED_8 (const uint64_t, ff_pw_15 ) = 0x000F000F000F000FULL;
DECLARE_ALIGNED_16(const xmm_t,    ff_pw_16 ) = {0x0010001000100010ULL, 0x0010001000100010ULL};
DECLARE_ALIGNED_8 (const uint64_t, ff_pw_20 ) = 0x0014001400140014ULL;
DECLARE_ALIGNED_16(const xmm_t,    ff_pw_28 ) = {0x001C001C001C001CULL, 0x001C001C001C001CULL};
DECLARE_ALIGNED_16(const xmm_t,    ff_pw_32 ) = {0x0020002000200020ULL, 0x0020002000200020ULL};
DECLARE_ALIGNED_8 (const uint64_t, ff_pw_42 ) = 0x002A002A002A002AULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pw_64 ) = 0x0040004000400040ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pw_96 ) = 0x0060006000600060ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pw_128) = 0x0080008000800080ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pw_255) = 0x00ff00ff00ff00ffULL;

DECLARE_ALIGNED_8 (const uint64_t, ff_pb_1  ) = 0x0101010101010101ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pb_3  ) = 0x0303030303030303ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pb_7  ) = 0x0707070707070707ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pb_3F ) = 0x3F3F3F3F3F3F3F3FULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pb_A1 ) = 0xA1A1A1A1A1A1A1A1ULL;
DECLARE_ALIGNED_8 (const uint64_t, ff_pb_FC ) = 0xFCFCFCFCFCFCFCFCULL;

DECLARE_ALIGNED_16(const double, ff_pd_1[2]) = { 1.0, 1.0 };
DECLARE_ALIGNED_16(const double, ff_pd_2[2]) = { 2.0, 2.0 };

#define JUMPALIGN() asm volatile (ASMALIGN(3)::)
#define MOVQ_ZERO(regd)  asm volatile ("pxor %%" #regd ", %%" #regd ::)

#define MOVQ_BFE(regd) \
    asm volatile ( \
    "pcmpeqd %%" #regd ", %%" #regd " \n\t"\
    "paddb %%" #regd ", %%" #regd " \n\t" ::)

#ifndef PIC
#define MOVQ_BONE(regd)  asm volatile ("movq %0, %%" #regd " \n\t" ::"m"(ff_bone))
#define MOVQ_WTWO(regd)  asm volatile ("movq %0, %%" #regd " \n\t" ::"m"(ff_wtwo))
#else
// for shared library it's better to use this way for accessing constants
// pcmpeqd -> -1
#define MOVQ_BONE(regd) \
    asm volatile ( \
    "pcmpeqd %%" #regd ", %%" #regd " \n\t" \
    "psrlw $15, %%" #regd " \n\t" \
    "packuswb %%" #regd ", %%" #regd " \n\t" ::)

#define MOVQ_WTWO(regd) \
    asm volatile ( \
    "pcmpeqd %%" #regd ", %%" #regd " \n\t" \
    "psrlw $15, %%" #regd " \n\t" \
    "psllw $1, %%" #regd " \n\t"::)

#endif

// using regr as temporary and for the output result
// first argument is unmodifed and second is trashed
// regfe is supposed to contain 0xfefefefefefefefe
#define PAVGB_MMX_NO_RND(rega, regb, regr, regfe) \
    "movq " #rega ", " #regr "  \n\t"\
    "pand " #regb ", " #regr "  \n\t"\
    "pxor " #rega ", " #regb "  \n\t"\
    "pand " #regfe "," #regb "  \n\t"\
    "psrlq $1, " #regb "        \n\t"\
    "paddb " #regb ", " #regr " \n\t"

#define PAVGB_MMX(rega, regb, regr, regfe) \
    "movq " #rega ", " #regr "  \n\t"\
    "por  " #regb ", " #regr "  \n\t"\
    "pxor " #rega ", " #regb "  \n\t"\
    "pand " #regfe "," #regb "  \n\t"\
    "psrlq $1, " #regb "        \n\t"\
    "psubb " #regb ", " #regr " \n\t"

// mm6 is supposed to contain 0xfefefefefefefefe
#define PAVGBP_MMX_NO_RND(rega, regb, regr,  regc, regd, regp) \
    "movq " #rega ", " #regr "  \n\t"\
    "movq " #regc ", " #regp "  \n\t"\
    "pand " #regb ", " #regr "  \n\t"\
    "pand " #regd ", " #regp "  \n\t"\
    "pxor " #rega ", " #regb "  \n\t"\
    "pxor " #regc ", " #regd "  \n\t"\
    "pand %%mm6, " #regb "      \n\t"\
    "pand %%mm6, " #regd "      \n\t"\
    "psrlq $1, " #regb "        \n\t"\
    "psrlq $1, " #regd "        \n\t"\
    "paddb " #regb ", " #regr " \n\t"\
    "paddb " #regd ", " #regp " \n\t"

#define PAVGBP_MMX(rega, regb, regr, regc, regd, regp) \
    "movq " #rega ", " #regr "  \n\t"\
    "movq " #regc ", " #regp "  \n\t"\
    "por  " #regb ", " #regr "  \n\t"\
    "por  " #regd ", " #regp "  \n\t"\
    "pxor " #rega ", " #regb "  \n\t"\
    "pxor " #regc ", " #regd "  \n\t"\
    "pand %%mm6, " #regb "      \n\t"\
    "pand %%mm6, " #regd "      \n\t"\
    "psrlq $1, " #regd "        \n\t"\
    "psrlq $1, " #regb "        \n\t"\
    "psubb " #regb ", " #regr " \n\t"\
    "psubb " #regd ", " #regp " \n\t"

/***********************************/
/* MMX no rounding */
#define DEF(x, y) x ## _no_rnd_ ## y ##_mmx
#define SET_RND  MOVQ_WONE
#define PAVGBP(a, b, c, d, e, f)        PAVGBP_MMX_NO_RND(a, b, c, d, e, f)
#define PAVGB(a, b, c, e)               PAVGB_MMX_NO_RND(a, b, c, e)

#include "dsputil_mmx_rnd.h"

#undef DEF
#undef SET_RND
#undef PAVGBP
#undef PAVGB
/***********************************/
/* MMX rounding */

#define DEF(x, y) x ## _ ## y ##_mmx
#define SET_RND  MOVQ_WTWO
#define PAVGBP(a, b, c, d, e, f)        PAVGBP_MMX(a, b, c, d, e, f)
#define PAVGB(a, b, c, e)               PAVGB_MMX(a, b, c, e)

#include "dsputil_mmx_rnd.h"

#undef DEF
#undef SET_RND
#undef PAVGBP
#undef PAVGB

/***********************************/
/* 3Dnow specific */

#define DEF(x) x ## _3dnow
#define PAVGB "pavgusb"

#include "dsputil_mmx_avg.h"

#undef DEF
#undef PAVGB

/***********************************/
/* MMX2 specific */

#define DEF(x) x ## _mmx2

/* Introduced only in MMX2 set */
#define PAVGB "pavgb"

#include "dsputil_mmx_avg.h"

#undef DEF
#undef PAVGB

#define put_no_rnd_pixels16_mmx put_pixels16_mmx
#define put_no_rnd_pixels8_mmx put_pixels8_mmx
#define put_pixels16_mmx2 put_pixels16_mmx
#define put_pixels8_mmx2 put_pixels8_mmx
#define put_pixels4_mmx2 put_pixels4_mmx
#define put_no_rnd_pixels16_mmx2 put_no_rnd_pixels16_mmx
#define put_no_rnd_pixels8_mmx2 put_no_rnd_pixels8_mmx
#define put_pixels16_3dnow put_pixels16_mmx
#define put_pixels8_3dnow put_pixels8_mmx
#define put_pixels4_3dnow put_pixels4_mmx
#define put_no_rnd_pixels16_3dnow put_no_rnd_pixels16_mmx
#define put_no_rnd_pixels8_3dnow put_no_rnd_pixels8_mmx

/***********************************/
/* standard MMX */



static DECLARE_ALIGNED_8(const unsigned char, vector128[8]) =
  { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };


void add_pixels_clamped_mmx(const DCTELEM *block, uint8_t *pixels, int line_size)
{
    const DCTELEM *p;
    uint8_t *pix;
    int i;

    /* read the pixels */
    p = block;
    pix = pixels;
    MOVQ_ZERO(mm7);
    i = 4;
    do {
        asm volatile(
                "movq   (%2), %%mm0     \n\t"
                "movq   8(%2), %%mm1    \n\t"
                "movq   16(%2), %%mm2   \n\t"
                "movq   24(%2), %%mm3   \n\t"
                "movq   %0, %%mm4       \n\t"
                "movq   %1, %%mm6       \n\t"
                "movq   %%mm4, %%mm5    \n\t"
                "punpcklbw %%mm7, %%mm4 \n\t"
                "punpckhbw %%mm7, %%mm5 \n\t"
                "paddsw %%mm4, %%mm0    \n\t"
                "paddsw %%mm5, %%mm1    \n\t"
                "movq   %%mm6, %%mm5    \n\t"
                "punpcklbw %%mm7, %%mm6 \n\t"
                "punpckhbw %%mm7, %%mm5 \n\t"
                "paddsw %%mm6, %%mm2    \n\t"
                "paddsw %%mm5, %%mm3    \n\t"
                "packuswb %%mm1, %%mm0  \n\t"
                "packuswb %%mm3, %%mm2  \n\t"
                "movq   %%mm0, %0       \n\t"
                "movq   %%mm2, %1       \n\t"
                :"+m"(*pix), "+m"(*(pix+line_size))
                :"r"(p)
                :"memory");
        pix += line_size*2;
        p += 16;
    } while (--i);
}

static void put_pixels4_mmx(uint8_t *block, const uint8_t *pixels, int line_size, int h)
{
    asm volatile(
         "lea (%3, %3), %%"REG_a"       \n\t"
         ASMALIGN(3)
         "1:                            \n\t"
         "movd (%1), %%mm0              \n\t"
         "movd (%1, %3), %%mm1          \n\t"
         "movd %%mm0, (%2)              \n\t"
         "movd %%mm1, (%2, %3)          \n\t"
         "add %%"REG_a", %1             \n\t"
         "add %%"REG_a", %2             \n\t"
         "movd (%1), %%mm0              \n\t"
         "movd (%1, %3), %%mm1          \n\t"
         "movd %%mm0, (%2)              \n\t"
         "movd %%mm1, (%2, %3)          \n\t"
         "add %%"REG_a", %1             \n\t"
         "add %%"REG_a", %2             \n\t"
         "subl $4, %0                   \n\t"
         "jnz 1b                        \n\t"
         : "+g"(h), "+r" (pixels),  "+r" (block)
         : "r"((x86_reg)line_size)
         : "%"REG_a, "memory"
        );
}

static void put_pixels8_mmx(uint8_t *block, const uint8_t *pixels, int line_size, int h)
{
    asm volatile(
         "lea (%3, %3), %%"REG_a"       \n\t"
         ASMALIGN(3)
         "1:                            \n\t"
         "movq (%1), %%mm0              \n\t"
         "movq (%1, %3), %%mm1          \n\t"
         "movq %%mm0, (%2)              \n\t"
         "movq %%mm1, (%2, %3)          \n\t"
         "add %%"REG_a", %1             \n\t"
         "add %%"REG_a", %2             \n\t"
         "movq (%1), %%mm0              \n\t"
         "movq (%1, %3), %%mm1          \n\t"
         "movq %%mm0, (%2)              \n\t"
         "movq %%mm1, (%2, %3)          \n\t"
         "add %%"REG_a", %1             \n\t"
         "add %%"REG_a", %2             \n\t"
         "subl $4, %0                   \n\t"
         "jnz 1b                        \n\t"
         : "+g"(h), "+r" (pixels),  "+r" (block)
         : "r"((x86_reg)line_size)
         : "%"REG_a, "memory"
        );
}

static void put_pixels16_mmx(uint8_t *block, const uint8_t *pixels, int line_size, int h)
{
    asm volatile(
         "lea (%3, %3), %%"REG_a"       \n\t"
         ASMALIGN(3)
         "1:                            \n\t"
         "movq (%1), %%mm0              \n\t"
         "movq 8(%1), %%mm4             \n\t"
         "movq (%1, %3), %%mm1          \n\t"
         "movq 8(%1, %3), %%mm5         \n\t"
         "movq %%mm0, (%2)              \n\t"
         "movq %%mm4, 8(%2)             \n\t"
         "movq %%mm1, (%2, %3)          \n\t"
         "movq %%mm5, 8(%2, %3)         \n\t"
         "add %%"REG_a", %1             \n\t"
         "add %%"REG_a", %2             \n\t"
         "movq (%1), %%mm0              \n\t"
         "movq 8(%1), %%mm4             \n\t"
         "movq (%1, %3), %%mm1          \n\t"
         "movq 8(%1, %3), %%mm5         \n\t"
         "movq %%mm0, (%2)              \n\t"
         "movq %%mm4, 8(%2)             \n\t"
         "movq %%mm1, (%2, %3)          \n\t"
         "movq %%mm5, 8(%2, %3)         \n\t"
         "add %%"REG_a", %1             \n\t"
         "add %%"REG_a", %2             \n\t"
         "subl $4, %0                   \n\t"
         "jnz 1b                        \n\t"
         : "+g"(h), "+r" (pixels),  "+r" (block)
         : "r"((x86_reg)line_size)
         : "%"REG_a, "memory"
        );
}

static void put_pixels16_sse2(uint8_t *block, const uint8_t *pixels, int line_size, int h)
{
    asm volatile(
         "1:                            \n\t"
         "movdqu (%1), %%xmm0           \n\t"
         "movdqu (%1,%3), %%xmm1        \n\t"
         "movdqu (%1,%3,2), %%xmm2      \n\t"
         "movdqu (%1,%4), %%xmm3        \n\t"
         "movdqa %%xmm0, (%2)           \n\t"
         "movdqa %%xmm1, (%2,%3)        \n\t"
         "movdqa %%xmm2, (%2,%3,2)      \n\t"
         "movdqa %%xmm3, (%2,%4)        \n\t"
         "subl $4, %0                   \n\t"
         "lea (%1,%3,4), %1             \n\t"
         "lea (%2,%3,4), %2             \n\t"
         "jnz 1b                        \n\t"
         : "+g"(h), "+r" (pixels),  "+r" (block)
         : "r"((x86_reg)line_size), "r"((x86_reg)3L*line_size)
         : "memory"
        );
}

static void avg_pixels16_sse2(uint8_t *block, const uint8_t *pixels, int line_size, int h)
{
    asm volatile(
         "1:                            \n\t"
         "movdqu (%1), %%xmm0           \n\t"
         "movdqu (%1,%3), %%xmm1        \n\t"
         "movdqu (%1,%3,2), %%xmm2      \n\t"
         "movdqu (%1,%4), %%xmm3        \n\t"
         "pavgb  (%2), %%xmm0           \n\t"
         "pavgb  (%2,%3), %%xmm1        \n\t"
         "pavgb  (%2,%3,2), %%xmm2      \n\t"
         "pavgb  (%2,%4), %%xmm3        \n\t"
         "movdqa %%xmm0, (%2)           \n\t"
         "movdqa %%xmm1, (%2,%3)        \n\t"
         "movdqa %%xmm2, (%2,%3,2)      \n\t"
         "movdqa %%xmm3, (%2,%4)        \n\t"
         "subl $4, %0                   \n\t"
         "lea (%1,%3,4), %1             \n\t"
         "lea (%2,%3,4), %2             \n\t"
         "jnz 1b                        \n\t"
         : "+g"(h), "+r" (pixels),  "+r" (block)
         : "r"((x86_reg)line_size), "r"((x86_reg)3L*line_size)
         : "memory"
        );
}

static void clear_blocks_mmx(DCTELEM *blocks)
{
    asm volatile(
                "pxor %%mm7, %%mm7              \n\t"
                "mov $-128*6, %%"REG_a"         \n\t"
                "1:                             \n\t"
                "movq %%mm7, (%0, %%"REG_a")    \n\t"
                "movq %%mm7, 8(%0, %%"REG_a")   \n\t"
                "movq %%mm7, 16(%0, %%"REG_a")  \n\t"
                "movq %%mm7, 24(%0, %%"REG_a")  \n\t"
                "add $32, %%"REG_a"             \n\t"
                " js 1b                         \n\t"
                : : "r" (((uint8_t *)blocks)+128*6)
                : "%"REG_a
        );
}

static void add_bytes_mmx(uint8_t *dst, uint8_t *src, int w){
    x86_reg i=0;
    asm volatile(
        "jmp 2f                         \n\t"
        "1:                             \n\t"
        "movq  (%1, %0), %%mm0          \n\t"
        "movq  (%2, %0), %%mm1          \n\t"
        "paddb %%mm0, %%mm1             \n\t"
        "movq %%mm1, (%2, %0)           \n\t"
        "movq 8(%1, %0), %%mm0          \n\t"
        "movq 8(%2, %0), %%mm1          \n\t"
        "paddb %%mm0, %%mm1             \n\t"
        "movq %%mm1, 8(%2, %0)          \n\t"
        "add $16, %0                    \n\t"
        "2:                             \n\t"
        "cmp %3, %0                     \n\t"
        " js 1b                         \n\t"
        : "+r" (i)
        : "r"(src), "r"(dst), "r"((x86_reg)w-15)
    );
    for(; i<w; i++)
        dst[i+0] += src[i+0];
}

static void add_bytes_l2_mmx(uint8_t *dst, uint8_t *src1, uint8_t *src2, int w){
    x86_reg i=0;
    asm volatile(
        "jmp 2f                         \n\t"
        "1:                             \n\t"
        "movq   (%2, %0), %%mm0         \n\t"
        "movq  8(%2, %0), %%mm1         \n\t"
        "paddb  (%3, %0), %%mm0         \n\t"
        "paddb 8(%3, %0), %%mm1         \n\t"
        "movq %%mm0,  (%1, %0)          \n\t"
        "movq %%mm1, 8(%1, %0)          \n\t"
        "add $16, %0                    \n\t"
        "2:                             \n\t"
        "cmp %4, %0                     \n\t"
        " js 1b                         \n\t"
        : "+r" (i)
        : "r"(dst), "r"(src1), "r"(src2), "r"((x86_reg)w-15)
    );
    for(; i<w; i++)
        dst[i] = src1[i] + src2[i];
}


static inline void transpose4x4(uint8_t *dst, uint8_t *src, int dst_stride, int src_stride){
    asm volatile( //FIXME could save 1 instruction if done as 8x4 ...
        "movd  %4, %%mm0                \n\t"
        "movd  %5, %%mm1                \n\t"
        "movd  %6, %%mm2                \n\t"
        "movd  %7, %%mm3                \n\t"
        "punpcklbw %%mm1, %%mm0         \n\t"
        "punpcklbw %%mm3, %%mm2         \n\t"
        "movq %%mm0, %%mm1              \n\t"
        "punpcklwd %%mm2, %%mm0         \n\t"
        "punpckhwd %%mm2, %%mm1         \n\t"
        "movd  %%mm0, %0                \n\t"
        "punpckhdq %%mm0, %%mm0         \n\t"
        "movd  %%mm0, %1                \n\t"
        "movd  %%mm1, %2                \n\t"
        "punpckhdq %%mm1, %%mm1         \n\t"
        "movd  %%mm1, %3                \n\t"

        : "=m" (*(uint32_t*)(dst + 0*dst_stride)),
          "=m" (*(uint32_t*)(dst + 1*dst_stride)),
          "=m" (*(uint32_t*)(dst + 2*dst_stride)),
          "=m" (*(uint32_t*)(dst + 3*dst_stride))
        :  "m" (*(uint32_t*)(src + 0*src_stride)),
           "m" (*(uint32_t*)(src + 1*src_stride)),
           "m" (*(uint32_t*)(src + 2*src_stride)),
           "m" (*(uint32_t*)(src + 3*src_stride))
    );
}


/* draw the edges of width 'w' of an image of size width, height
   this mmx version can only handle w==8 || w==16 */
static void draw_edges_mmx(uint8_t *buf, int wrap, int width, int height, int w)
{
    uint8_t *ptr, *last_line;
    int i;

    last_line = buf + (height - 1) * wrap;
    /* left and right */
    ptr = buf;
    if(w==8)
    {
        asm volatile(
                "1:                             \n\t"
                "movd (%0), %%mm0               \n\t"
                "punpcklbw %%mm0, %%mm0         \n\t"
                "punpcklwd %%mm0, %%mm0         \n\t"
                "punpckldq %%mm0, %%mm0         \n\t"
                "movq %%mm0, -8(%0)             \n\t"
                "movq -8(%0, %2), %%mm1         \n\t"
                "punpckhbw %%mm1, %%mm1         \n\t"
                "punpckhwd %%mm1, %%mm1         \n\t"
                "punpckhdq %%mm1, %%mm1         \n\t"
                "movq %%mm1, (%0, %2)           \n\t"
                "add %1, %0                     \n\t"
                "cmp %3, %0                     \n\t"
                " jb 1b                         \n\t"
                : "+r" (ptr)
                : "r" ((x86_reg)wrap), "r" ((x86_reg)width), "r" (ptr + wrap*height)
        );
    }
    else
    {
        asm volatile(
                "1:                             \n\t"
                "movd (%0), %%mm0               \n\t"
                "punpcklbw %%mm0, %%mm0         \n\t"
                "punpcklwd %%mm0, %%mm0         \n\t"
                "punpckldq %%mm0, %%mm0         \n\t"
                "movq %%mm0, -8(%0)             \n\t"
                "movq %%mm0, -16(%0)            \n\t"
                "movq -8(%0, %2), %%mm1         \n\t"
                "punpckhbw %%mm1, %%mm1         \n\t"
                "punpckhwd %%mm1, %%mm1         \n\t"
                "punpckhdq %%mm1, %%mm1         \n\t"
                "movq %%mm1, (%0, %2)           \n\t"
                "movq %%mm1, 8(%0, %2)          \n\t"
                "add %1, %0                     \n\t"
                "cmp %3, %0                     \n\t"
                " jb 1b                         \n\t"
                : "+r" (ptr)
                : "r" ((x86_reg)wrap), "r" ((x86_reg)width), "r" (ptr + wrap*height)
        );
    }

    for(i=0;i<w;i+=4) {
        /* top and bottom (and hopefully also the corners) */
        ptr= buf - (i + 1) * wrap - w;
        asm volatile(
                "1:                             \n\t"
                "movq (%1, %0), %%mm0           \n\t"
                "movq %%mm0, (%0)               \n\t"
                "movq %%mm0, (%0, %2)           \n\t"
                "movq %%mm0, (%0, %2, 2)        \n\t"
                "movq %%mm0, (%0, %3)           \n\t"
                "add $8, %0                     \n\t"
                "cmp %4, %0                     \n\t"
                " jb 1b                         \n\t"
                : "+r" (ptr)
                : "r" ((x86_reg)buf - (x86_reg)ptr - w), "r" ((x86_reg)-wrap), "r" ((x86_reg)-wrap*3), "r" (ptr+width+2*w)
        );
        ptr= last_line + (i + 1) * wrap - w;
        asm volatile(
                "1:                             \n\t"
                "movq (%1, %0), %%mm0           \n\t"
                "movq %%mm0, (%0)               \n\t"
                "movq %%mm0, (%0, %2)           \n\t"
                "movq %%mm0, (%0, %2, 2)        \n\t"
                "movq %%mm0, (%0, %3)           \n\t"
                "add $8, %0                     \n\t"
                "cmp %4, %0                     \n\t"
                " jb 1b                         \n\t"
                : "+r" (ptr)
                : "r" ((x86_reg)last_line - (x86_reg)ptr - w), "r" ((x86_reg)wrap), "r" ((x86_reg)wrap*3), "r" (ptr+width+2*w)
        );
    }
}

#define PAETH(cpu, abs3)\
static void add_png_paeth_prediction_##cpu(uint8_t *dst, uint8_t *src, uint8_t *top, int w, int bpp)\
{\
    x86_reg i = -bpp;\
    x86_reg end = w-3;\
    asm volatile(\
        "pxor      %%mm7, %%mm7 \n"\
        "movd    (%1,%0), %%mm0 \n"\
        "movd    (%2,%0), %%mm1 \n"\
        "punpcklbw %%mm7, %%mm0 \n"\
        "punpcklbw %%mm7, %%mm1 \n"\
        "add       %4, %0 \n"\
        "1: \n"\
        "movq      %%mm1, %%mm2 \n"\
        "movd    (%2,%0), %%mm1 \n"\
        "movq      %%mm2, %%mm3 \n"\
        "punpcklbw %%mm7, %%mm1 \n"\
        "movq      %%mm2, %%mm4 \n"\
        "psubw     %%mm1, %%mm3 \n"\
        "psubw     %%mm0, %%mm4 \n"\
        "movq      %%mm3, %%mm5 \n"\
        "paddw     %%mm4, %%mm5 \n"\
        abs3\
        "movq      %%mm4, %%mm6 \n"\
        "pminsw    %%mm5, %%mm6 \n"\
        "pcmpgtw   %%mm6, %%mm3 \n"\
        "pcmpgtw   %%mm5, %%mm4 \n"\
        "movq      %%mm4, %%mm6 \n"\
        "pand      %%mm3, %%mm4 \n"\
        "pandn     %%mm3, %%mm6 \n"\
        "pandn     %%mm0, %%mm3 \n"\
        "movd    (%3,%0), %%mm0 \n"\
        "pand      %%mm1, %%mm6 \n"\
        "pand      %%mm4, %%mm2 \n"\
        "punpcklbw %%mm7, %%mm0 \n"\
        "movq      %6,    %%mm5 \n"\
        "paddw     %%mm6, %%mm0 \n"\
        "paddw     %%mm2, %%mm3 \n"\
        "paddw     %%mm3, %%mm0 \n"\
        "pand      %%mm5, %%mm0 \n"\
        "movq      %%mm0, %%mm3 \n"\
        "packuswb  %%mm3, %%mm3 \n"\
        "movd      %%mm3, (%1,%0) \n"\
        "add       %4, %0 \n"\
        "cmp       %5, %0 \n"\
        "jle 1b \n"\
        :"+r"(i)\
        :"r"(dst), "r"(top), "r"(src), "r"((x86_reg)bpp), "g"(end),\
         "m"(ff_pw_255)\
        :"memory"\
    );\
}

#define ABS3_MMX2\
        "psubw     %%mm5, %%mm7 \n"\
        "pmaxsw    %%mm7, %%mm5 \n"\
        "pxor      %%mm6, %%mm6 \n"\
        "pxor      %%mm7, %%mm7 \n"\
        "psubw     %%mm3, %%mm6 \n"\
        "psubw     %%mm4, %%mm7 \n"\
        "pmaxsw    %%mm6, %%mm3 \n"\
        "pmaxsw    %%mm7, %%mm4 \n"\
        "pxor      %%mm7, %%mm7 \n"

#define ABS3_SSSE3\
        "pabsw     %%mm3, %%mm3 \n"\
        "pabsw     %%mm4, %%mm4 \n"\
        "pabsw     %%mm5, %%mm5 \n"

PAETH(mmx2, ABS3_MMX2)
#ifdef HAVE_SSSE3
PAETH(ssse3, ABS3_SSSE3)
#endif

#define QPEL_V_LOW(m3,m4,m5,m6, pw_20, pw_3, rnd, in0, in1, in2, in7, out, OP)\
        "paddw " #m4 ", " #m3 "           \n\t" /* x1 */\
        "movq "MANGLE(ff_pw_20)", %%mm4   \n\t" /* 20 */\
        "pmullw " #m3 ", %%mm4            \n\t" /* 20x1 */\
        "movq "#in7", " #m3 "             \n\t" /* d */\
        "movq "#in0", %%mm5               \n\t" /* D */\
        "paddw " #m3 ", %%mm5             \n\t" /* x4 */\
        "psubw %%mm5, %%mm4               \n\t" /* 20x1 - x4 */\
        "movq "#in1", %%mm5               \n\t" /* C */\
        "movq "#in2", %%mm6               \n\t" /* B */\
        "paddw " #m6 ", %%mm5             \n\t" /* x3 */\
        "paddw " #m5 ", %%mm6             \n\t" /* x2 */\
        "paddw %%mm6, %%mm6               \n\t" /* 2x2 */\
        "psubw %%mm6, %%mm5               \n\t" /* -2x2 + x3 */\
        "pmullw "MANGLE(ff_pw_3)", %%mm5  \n\t" /* -6x2 + 3x3 */\
        "paddw " #rnd ", %%mm4            \n\t" /* x2 */\
        "paddw %%mm4, %%mm5               \n\t" /* 20x1 - 6x2 + 3x3 - x4 */\
        "psraw $5, %%mm5                  \n\t"\
        "packuswb %%mm5, %%mm5            \n\t"\
        OP(%%mm5, out, %%mm7, d)

#define QPEL_BASE(OPNAME, ROUNDER, RND, OP_MMX2, OP_3DNOW)\
static void OPNAME ## mpeg4_qpel16_h_lowpass_mmx2(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int h){\
    uint64_t temp;\
\
    asm volatile(\
        "pxor %%mm7, %%mm7                \n\t"\
        "1:                               \n\t"\
        "movq  (%0), %%mm0                \n\t" /* ABCDEFGH */\
        "movq %%mm0, %%mm1                \n\t" /* ABCDEFGH */\
        "movq %%mm0, %%mm2                \n\t" /* ABCDEFGH */\
        "punpcklbw %%mm7, %%mm0           \n\t" /* 0A0B0C0D */\
        "punpckhbw %%mm7, %%mm1           \n\t" /* 0E0F0G0H */\
        "pshufw $0x90, %%mm0, %%mm5       \n\t" /* 0A0A0B0C */\
        "pshufw $0x41, %%mm0, %%mm6       \n\t" /* 0B0A0A0B */\
        "movq %%mm2, %%mm3                \n\t" /* ABCDEFGH */\
        "movq %%mm2, %%mm4                \n\t" /* ABCDEFGH */\
        "psllq $8, %%mm2                  \n\t" /* 0ABCDEFG */\
        "psllq $16, %%mm3                 \n\t" /* 00ABCDEF */\
        "psllq $24, %%mm4                 \n\t" /* 000ABCDE */\
        "punpckhbw %%mm7, %%mm2           \n\t" /* 0D0E0F0G */\
        "punpckhbw %%mm7, %%mm3           \n\t" /* 0C0D0E0F */\
        "punpckhbw %%mm7, %%mm4           \n\t" /* 0B0C0D0E */\
        "paddw %%mm3, %%mm5               \n\t" /* b */\
        "paddw %%mm2, %%mm6               \n\t" /* c */\
        "paddw %%mm5, %%mm5               \n\t" /* 2b */\
        "psubw %%mm5, %%mm6               \n\t" /* c - 2b */\
        "pshufw $0x06, %%mm0, %%mm5       \n\t" /* 0C0B0A0A */\
        "pmullw "MANGLE(ff_pw_3)", %%mm6  \n\t" /* 3c - 6b */\
        "paddw %%mm4, %%mm0               \n\t" /* a */\
        "paddw %%mm1, %%mm5               \n\t" /* d */\
        "pmullw "MANGLE(ff_pw_20)", %%mm0 \n\t" /* 20a */\
        "psubw %%mm5, %%mm0               \n\t" /* 20a - d */\
        "paddw %6, %%mm6                  \n\t"\
        "paddw %%mm6, %%mm0               \n\t" /* 20a - 6b + 3c - d */\
        "psraw $5, %%mm0                  \n\t"\
        "movq %%mm0, %5                   \n\t"\
        /* mm1=EFGH, mm2=DEFG, mm3=CDEF, mm4=BCDE, mm7=0 */\
        \
        "movq 5(%0), %%mm0                \n\t" /* FGHIJKLM */\
        "movq %%mm0, %%mm5                \n\t" /* FGHIJKLM */\
        "movq %%mm0, %%mm6                \n\t" /* FGHIJKLM */\
        "psrlq $8, %%mm0                  \n\t" /* GHIJKLM0 */\
        "psrlq $16, %%mm5                 \n\t" /* HIJKLM00 */\
        "punpcklbw %%mm7, %%mm0           \n\t" /* 0G0H0I0J */\
        "punpcklbw %%mm7, %%mm5           \n\t" /* 0H0I0J0K */\
        "paddw %%mm0, %%mm2               \n\t" /* b */\
        "paddw %%mm5, %%mm3               \n\t" /* c */\
        "paddw %%mm2, %%mm2               \n\t" /* 2b */\
        "psubw %%mm2, %%mm3               \n\t" /* c - 2b */\
        "movq %%mm6, %%mm2                \n\t" /* FGHIJKLM */\
        "psrlq $24, %%mm6                 \n\t" /* IJKLM000 */\
        "punpcklbw %%mm7, %%mm2           \n\t" /* 0F0G0H0I */\
        "punpcklbw %%mm7, %%mm6           \n\t" /* 0I0J0K0L */\
        "pmullw "MANGLE(ff_pw_3)", %%mm3  \n\t" /* 3c - 6b */\
        "paddw %%mm2, %%mm1               \n\t" /* a */\
        "paddw %%mm6, %%mm4               \n\t" /* d */\
        "pmullw "MANGLE(ff_pw_20)", %%mm1 \n\t" /* 20a */\
        "psubw %%mm4, %%mm3               \n\t" /* - 6b +3c - d */\
        "paddw %6, %%mm1                  \n\t"\
        "paddw %%mm1, %%mm3               \n\t" /* 20a - 6b +3c - d */\
        "psraw $5, %%mm3                  \n\t"\
        "movq %5, %%mm1                   \n\t"\
        "packuswb %%mm3, %%mm1            \n\t"\
        OP_MMX2(%%mm1, (%1),%%mm4, q)\
        /* mm0= GHIJ, mm2=FGHI, mm5=HIJK, mm6=IJKL, mm7=0 */\
        \
        "movq 9(%0), %%mm1                \n\t" /* JKLMNOPQ */\
        "movq %%mm1, %%mm4                \n\t" /* JKLMNOPQ */\
        "movq %%mm1, %%mm3                \n\t" /* JKLMNOPQ */\
        "psrlq $8, %%mm1                  \n\t" /* KLMNOPQ0 */\
        "psrlq $16, %%mm4                 \n\t" /* LMNOPQ00 */\
        "punpcklbw %%mm7, %%mm1           \n\t" /* 0K0L0M0N */\
        "punpcklbw %%mm7, %%mm4           \n\t" /* 0L0M0N0O */\
        "paddw %%mm1, %%mm5               \n\t" /* b */\
        "paddw %%mm4, %%mm0               \n\t" /* c */\
        "paddw %%mm5, %%mm5               \n\t" /* 2b */\
        "psubw %%mm5, %%mm0               \n\t" /* c - 2b */\
        "movq %%mm3, %%mm5                \n\t" /* JKLMNOPQ */\
        "psrlq $24, %%mm3                 \n\t" /* MNOPQ000 */\
        "pmullw "MANGLE(ff_pw_3)", %%mm0  \n\t" /* 3c - 6b */\
        "punpcklbw %%mm7, %%mm3           \n\t" /* 0M0N0O0P */\
        "paddw %%mm3, %%mm2               \n\t" /* d */\
        "psubw %%mm2, %%mm0               \n\t" /* -6b + 3c - d */\
        "movq %%mm5, %%mm2                \n\t" /* JKLMNOPQ */\
        "punpcklbw %%mm7, %%mm2           \n\t" /* 0J0K0L0M */\
        "punpckhbw %%mm7, %%mm5           \n\t" /* 0N0O0P0Q */\
        "paddw %%mm2, %%mm6               \n\t" /* a */\
        "pmullw "MANGLE(ff_pw_20)", %%mm6 \n\t" /* 20a */\
        "paddw %6, %%mm0                  \n\t"\
        "paddw %%mm6, %%mm0               \n\t" /* 20a - 6b + 3c - d */\
        "psraw $5, %%mm0                  \n\t"\
        /* mm1=KLMN, mm2=JKLM, mm3=MNOP, mm4=LMNO, mm5=NOPQ mm7=0 */\
        \
        "paddw %%mm5, %%mm3               \n\t" /* a */\
        "pshufw $0xF9, %%mm5, %%mm6       \n\t" /* 0O0P0Q0Q */\
        "paddw %%mm4, %%mm6               \n\t" /* b */\
        "pshufw $0xBE, %%mm5, %%mm4       \n\t" /* 0P0Q0Q0P */\
        "pshufw $0x6F, %%mm5, %%mm5       \n\t" /* 0Q0Q0P0O */\
        "paddw %%mm1, %%mm4               \n\t" /* c */\
        "paddw %%mm2, %%mm5               \n\t" /* d */\
        "paddw %%mm6, %%mm6               \n\t" /* 2b */\
        "psubw %%mm6, %%mm4               \n\t" /* c - 2b */\
        "pmullw "MANGLE(ff_pw_20)", %%mm3 \n\t" /* 20a */\
        "pmullw "MANGLE(ff_pw_3)", %%mm4  \n\t" /* 3c - 6b */\
        "psubw %%mm5, %%mm3               \n\t" /* -6b + 3c - d */\
        "paddw %6, %%mm4                  \n\t"\
        "paddw %%mm3, %%mm4               \n\t" /* 20a - 6b + 3c - d */\
        "psraw $5, %%mm4                  \n\t"\
        "packuswb %%mm4, %%mm0            \n\t"\
        OP_MMX2(%%mm0, 8(%1), %%mm4, q)\
        \
        "add %3, %0                       \n\t"\
        "add %4, %1                       \n\t"\
        "decl %2                          \n\t"\
        " jnz 1b                          \n\t"\
        : "+a"(src), "+c"(dst), "+D"(h)\
        : "d"((x86_reg)srcStride), "S"((x86_reg)dstStride), /*"m"(ff_pw_20), "m"(ff_pw_3),*/ "m"(temp), "m"(ROUNDER)\
        : "memory"\
    );\
}\
\
static void OPNAME ## mpeg4_qpel16_h_lowpass_3dnow(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int h){\
    int i;\
    int16_t temp[16];\
    /* quick HACK, XXX FIXME MUST be optimized */\
    for(i=0; i<h; i++)\
    {\
        temp[ 0]= (src[ 0]+src[ 1])*20 - (src[ 0]+src[ 2])*6 + (src[ 1]+src[ 3])*3 - (src[ 2]+src[ 4]);\
        temp[ 1]= (src[ 1]+src[ 2])*20 - (src[ 0]+src[ 3])*6 + (src[ 0]+src[ 4])*3 - (src[ 1]+src[ 5]);\
        temp[ 2]= (src[ 2]+src[ 3])*20 - (src[ 1]+src[ 4])*6 + (src[ 0]+src[ 5])*3 - (src[ 0]+src[ 6]);\
        temp[ 3]= (src[ 3]+src[ 4])*20 - (src[ 2]+src[ 5])*6 + (src[ 1]+src[ 6])*3 - (src[ 0]+src[ 7]);\
        temp[ 4]= (src[ 4]+src[ 5])*20 - (src[ 3]+src[ 6])*6 + (src[ 2]+src[ 7])*3 - (src[ 1]+src[ 8]);\
        temp[ 5]= (src[ 5]+src[ 6])*20 - (src[ 4]+src[ 7])*6 + (src[ 3]+src[ 8])*3 - (src[ 2]+src[ 9]);\
        temp[ 6]= (src[ 6]+src[ 7])*20 - (src[ 5]+src[ 8])*6 + (src[ 4]+src[ 9])*3 - (src[ 3]+src[10]);\
        temp[ 7]= (src[ 7]+src[ 8])*20 - (src[ 6]+src[ 9])*6 + (src[ 5]+src[10])*3 - (src[ 4]+src[11]);\
        temp[ 8]= (src[ 8]+src[ 9])*20 - (src[ 7]+src[10])*6 + (src[ 6]+src[11])*3 - (src[ 5]+src[12]);\
        temp[ 9]= (src[ 9]+src[10])*20 - (src[ 8]+src[11])*6 + (src[ 7]+src[12])*3 - (src[ 6]+src[13]);\
        temp[10]= (src[10]+src[11])*20 - (src[ 9]+src[12])*6 + (src[ 8]+src[13])*3 - (src[ 7]+src[14]);\
        temp[11]= (src[11]+src[12])*20 - (src[10]+src[13])*6 + (src[ 9]+src[14])*3 - (src[ 8]+src[15]);\
        temp[12]= (src[12]+src[13])*20 - (src[11]+src[14])*6 + (src[10]+src[15])*3 - (src[ 9]+src[16]);\
        temp[13]= (src[13]+src[14])*20 - (src[12]+src[15])*6 + (src[11]+src[16])*3 - (src[10]+src[16]);\
        temp[14]= (src[14]+src[15])*20 - (src[13]+src[16])*6 + (src[12]+src[16])*3 - (src[11]+src[15]);\
        temp[15]= (src[15]+src[16])*20 - (src[14]+src[16])*6 + (src[13]+src[15])*3 - (src[12]+src[14]);\
        asm volatile(\
            "movq (%0), %%mm0               \n\t"\
            "movq 8(%0), %%mm1              \n\t"\
            "paddw %2, %%mm0                \n\t"\
            "paddw %2, %%mm1                \n\t"\
            "psraw $5, %%mm0                \n\t"\
            "psraw $5, %%mm1                \n\t"\
            "packuswb %%mm1, %%mm0          \n\t"\
            OP_3DNOW(%%mm0, (%1), %%mm1, q)\
            "movq 16(%0), %%mm0             \n\t"\
            "movq 24(%0), %%mm1             \n\t"\
            "paddw %2, %%mm0                \n\t"\
            "paddw %2, %%mm1                \n\t"\
            "psraw $5, %%mm0                \n\t"\
            "psraw $5, %%mm1                \n\t"\
            "packuswb %%mm1, %%mm0          \n\t"\
            OP_3DNOW(%%mm0, 8(%1), %%mm1, q)\
            :: "r"(temp), "r"(dst), "m"(ROUNDER)\
            : "memory"\
        );\
        dst+=dstStride;\
        src+=srcStride;\
    }\
}\
\
static void OPNAME ## mpeg4_qpel8_h_lowpass_mmx2(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int h){\
    asm volatile(\
        "pxor %%mm7, %%mm7                \n\t"\
        "1:                               \n\t"\
        "movq  (%0), %%mm0                \n\t" /* ABCDEFGH */\
        "movq %%mm0, %%mm1                \n\t" /* ABCDEFGH */\
        "movq %%mm0, %%mm2                \n\t" /* ABCDEFGH */\
        "punpcklbw %%mm7, %%mm0           \n\t" /* 0A0B0C0D */\
        "punpckhbw %%mm7, %%mm1           \n\t" /* 0E0F0G0H */\
        "pshufw $0x90, %%mm0, %%mm5       \n\t" /* 0A0A0B0C */\
        "pshufw $0x41, %%mm0, %%mm6       \n\t" /* 0B0A0A0B */\
        "movq %%mm2, %%mm3                \n\t" /* ABCDEFGH */\
        "movq %%mm2, %%mm4                \n\t" /* ABCDEFGH */\
        "psllq $8, %%mm2                  \n\t" /* 0ABCDEFG */\
        "psllq $16, %%mm3                 \n\t" /* 00ABCDEF */\
        "psllq $24, %%mm4                 \n\t" /* 000ABCDE */\
        "punpckhbw %%mm7, %%mm2           \n\t" /* 0D0E0F0G */\
        "punpckhbw %%mm7, %%mm3           \n\t" /* 0C0D0E0F */\
        "punpckhbw %%mm7, %%mm4           \n\t" /* 0B0C0D0E */\
        "paddw %%mm3, %%mm5               \n\t" /* b */\
        "paddw %%mm2, %%mm6               \n\t" /* c */\
        "paddw %%mm5, %%mm5               \n\t" /* 2b */\
        "psubw %%mm5, %%mm6               \n\t" /* c - 2b */\
        "pshufw $0x06, %%mm0, %%mm5       \n\t" /* 0C0B0A0A */\
        "pmullw "MANGLE(ff_pw_3)", %%mm6  \n\t" /* 3c - 6b */\
        "paddw %%mm4, %%mm0               \n\t" /* a */\
        "paddw %%mm1, %%mm5               \n\t" /* d */\
        "pmullw "MANGLE(ff_pw_20)", %%mm0 \n\t" /* 20a */\
        "psubw %%mm5, %%mm0               \n\t" /* 20a - d */\
        "paddw %5, %%mm6                  \n\t"\
        "paddw %%mm6, %%mm0               \n\t" /* 20a - 6b + 3c - d */\
        "psraw $5, %%mm0                  \n\t"\
        /* mm1=EFGH, mm2=DEFG, mm3=CDEF, mm4=BCDE, mm7=0 */\
        \
        "movd 5(%0), %%mm5                \n\t" /* FGHI */\
        "punpcklbw %%mm7, %%mm5           \n\t" /* 0F0G0H0I */\
        "pshufw $0xF9, %%mm5, %%mm6       \n\t" /* 0G0H0I0I */\
        "paddw %%mm5, %%mm1               \n\t" /* a */\
        "paddw %%mm6, %%mm2               \n\t" /* b */\
        "pshufw $0xBE, %%mm5, %%mm6       \n\t" /* 0H0I0I0H */\
        "pshufw $0x6F, %%mm5, %%mm5       \n\t" /* 0I0I0H0G */\
        "paddw %%mm6, %%mm3               \n\t" /* c */\
        "paddw %%mm5, %%mm4               \n\t" /* d */\
        "paddw %%mm2, %%mm2               \n\t" /* 2b */\
        "psubw %%mm2, %%mm3               \n\t" /* c - 2b */\
        "pmullw "MANGLE(ff_pw_20)", %%mm1 \n\t" /* 20a */\
        "pmullw "MANGLE(ff_pw_3)", %%mm3  \n\t" /* 3c - 6b */\
        "psubw %%mm4, %%mm3               \n\t" /* -6b + 3c - d */\
        "paddw %5, %%mm1                  \n\t"\
        "paddw %%mm1, %%mm3               \n\t" /* 20a - 6b + 3c - d */\
        "psraw $5, %%mm3                  \n\t"\
        "packuswb %%mm3, %%mm0            \n\t"\
        OP_MMX2(%%mm0, (%1), %%mm4, q)\
        \
        "add %3, %0                       \n\t"\
        "add %4, %1                       \n\t"\
        "decl %2                          \n\t"\
        " jnz 1b                          \n\t"\
        : "+a"(src), "+c"(dst), "+d"(h)\
        : "S"((x86_reg)srcStride), "D"((x86_reg)dstStride), /*"m"(ff_pw_20), "m"(ff_pw_3),*/ "m"(ROUNDER)\
        : "memory"\
    );\
}\
\
static void OPNAME ## mpeg4_qpel8_h_lowpass_3dnow(uint8_t *dst, uint8_t *src, int dstStride, int srcStride, int h){\
    int i;\
    int16_t temp[8];\
    /* quick HACK, XXX FIXME MUST be optimized */\
    for(i=0; i<h; i++)\
    {\
        temp[ 0]= (src[ 0]+src[ 1])*20 - (src[ 0]+src[ 2])*6 + (src[ 1]+src[ 3])*3 - (src[ 2]+src[ 4]);\
        temp[ 1]= (src[ 1]+src[ 2])*20 - (src[ 0]+src[ 3])*6 + (src[ 0]+src[ 4])*3 - (src[ 1]+src[ 5]);\
        temp[ 2]= (src[ 2]+src[ 3])*20 - (src[ 1]+src[ 4])*6 + (src[ 0]+src[ 5])*3 - (src[ 0]+src[ 6]);\
        temp[ 3]= (src[ 3]+src[ 4])*20 - (src[ 2]+src[ 5])*6 + (src[ 1]+src[ 6])*3 - (src[ 0]+src[ 7]);\
        temp[ 4]= (src[ 4]+src[ 5])*20 - (src[ 3]+src[ 6])*6 + (src[ 2]+src[ 7])*3 - (src[ 1]+src[ 8]);\
        temp[ 5]= (src[ 5]+src[ 6])*20 - (src[ 4]+src[ 7])*6 + (src[ 3]+src[ 8])*3 - (src[ 2]+src[ 8]);\
        temp[ 6]= (src[ 6]+src[ 7])*20 - (src[ 5]+src[ 8])*6 + (src[ 4]+src[ 8])*3 - (src[ 3]+src[ 7]);\
        temp[ 7]= (src[ 7]+src[ 8])*20 - (src[ 6]+src[ 8])*6 + (src[ 5]+src[ 7])*3 - (src[ 4]+src[ 6]);\
        asm volatile(\
            "movq (%0), %%mm0           \n\t"\
            "movq 8(%0), %%mm1          \n\t"\
            "paddw %2, %%mm0            \n\t"\
            "paddw %2, %%mm1            \n\t"\
            "psraw $5, %%mm0            \n\t"\
            "psraw $5, %%mm1            \n\t"\
            "packuswb %%mm1, %%mm0      \n\t"\
            OP_3DNOW(%%mm0, (%1), %%mm1, q)\
            :: "r"(temp), "r"(dst), "m"(ROUNDER)\
            :"memory"\
        );\
        dst+=dstStride;\
        src+=srcStride;\
    }\
}

#define QPEL_OP(OPNAME, ROUNDER, RND, OP, MMX)\
\
static void OPNAME ## mpeg4_qpel16_v_lowpass_ ## MMX(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    uint64_t temp[17*4];\
    uint64_t *temp_ptr= temp;\
    int count= 17;\
\
    /*FIXME unroll */\
    asm volatile(\
        "pxor %%mm7, %%mm7              \n\t"\
        "1:                             \n\t"\
        "movq (%0), %%mm0               \n\t"\
        "movq (%0), %%mm1               \n\t"\
        "movq 8(%0), %%mm2              \n\t"\
        "movq 8(%0), %%mm3              \n\t"\
        "punpcklbw %%mm7, %%mm0         \n\t"\
        "punpckhbw %%mm7, %%mm1         \n\t"\
        "punpcklbw %%mm7, %%mm2         \n\t"\
        "punpckhbw %%mm7, %%mm3         \n\t"\
        "movq %%mm0, (%1)               \n\t"\
        "movq %%mm1, 17*8(%1)           \n\t"\
        "movq %%mm2, 2*17*8(%1)         \n\t"\
        "movq %%mm3, 3*17*8(%1)         \n\t"\
        "add $8, %1                     \n\t"\
        "add %3, %0                     \n\t"\
        "decl %2                        \n\t"\
        " jnz 1b                        \n\t"\
        : "+r" (src), "+r" (temp_ptr), "+r"(count)\
        : "r" ((x86_reg)srcStride)\
        : "memory"\
    );\
    \
    temp_ptr= temp;\
    count=4;\
    \
/*FIXME reorder for speed */\
    asm volatile(\
        /*"pxor %%mm7, %%mm7              \n\t"*/\
        "1:                             \n\t"\
        "movq (%0), %%mm0               \n\t"\
        "movq 8(%0), %%mm1              \n\t"\
        "movq 16(%0), %%mm2             \n\t"\
        "movq 24(%0), %%mm3             \n\t"\
        QPEL_V_LOW(%%mm0, %%mm1, %%mm2, %%mm3, %5, %6, %5, 16(%0),  8(%0),   (%0), 32(%0), (%1), OP)\
        QPEL_V_LOW(%%mm1, %%mm2, %%mm3, %%mm0, %5, %6, %5,  8(%0),   (%0),   (%0), 40(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm2, %%mm3, %%mm0, %%mm1, %5, %6, %5,   (%0),   (%0),  8(%0), 48(%0), (%1), OP)\
        \
        QPEL_V_LOW(%%mm3, %%mm0, %%mm1, %%mm2, %5, %6, %5,   (%0),  8(%0), 16(%0), 56(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm0, %%mm1, %%mm2, %%mm3, %5, %6, %5,  8(%0), 16(%0), 24(%0), 64(%0), (%1), OP)\
        QPEL_V_LOW(%%mm1, %%mm2, %%mm3, %%mm0, %5, %6, %5, 16(%0), 24(%0), 32(%0), 72(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm2, %%mm3, %%mm0, %%mm1, %5, %6, %5, 24(%0), 32(%0), 40(%0), 80(%0), (%1), OP)\
        QPEL_V_LOW(%%mm3, %%mm0, %%mm1, %%mm2, %5, %6, %5, 32(%0), 40(%0), 48(%0), 88(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm0, %%mm1, %%mm2, %%mm3, %5, %6, %5, 40(%0), 48(%0), 56(%0), 96(%0), (%1), OP)\
        QPEL_V_LOW(%%mm1, %%mm2, %%mm3, %%mm0, %5, %6, %5, 48(%0), 56(%0), 64(%0),104(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm2, %%mm3, %%mm0, %%mm1, %5, %6, %5, 56(%0), 64(%0), 72(%0),112(%0), (%1), OP)\
        QPEL_V_LOW(%%mm3, %%mm0, %%mm1, %%mm2, %5, %6, %5, 64(%0), 72(%0), 80(%0),120(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm0, %%mm1, %%mm2, %%mm3, %5, %6, %5, 72(%0), 80(%0), 88(%0),128(%0), (%1), OP)\
        \
        QPEL_V_LOW(%%mm1, %%mm2, %%mm3, %%mm0, %5, %6, %5, 80(%0), 88(%0), 96(%0),128(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"  \
        QPEL_V_LOW(%%mm2, %%mm3, %%mm0, %%mm1, %5, %6, %5, 88(%0), 96(%0),104(%0),120(%0), (%1), OP)\
        QPEL_V_LOW(%%mm3, %%mm0, %%mm1, %%mm2, %5, %6, %5, 96(%0),104(%0),112(%0),112(%0), (%1, %3), OP)\
        \
        "add $136, %0                   \n\t"\
        "add %6, %1                     \n\t"\
        "decl %2                        \n\t"\
        " jnz 1b                        \n\t"\
        \
        : "+r"(temp_ptr), "+r"(dst), "+g"(count)\
        : "r"((x86_reg)dstStride), "r"(2*(x86_reg)dstStride), /*"m"(ff_pw_20), "m"(ff_pw_3),*/ "m"(ROUNDER), "g"(4-14*(x86_reg)dstStride)\
        :"memory"\
    );\
}\
\
static void OPNAME ## mpeg4_qpel8_v_lowpass_ ## MMX(uint8_t *dst, uint8_t *src, int dstStride, int srcStride){\
    uint64_t temp[9*2];\
    uint64_t *temp_ptr= temp;\
    int count= 9;\
\
    /*FIXME unroll */\
    asm volatile(\
        "pxor %%mm7, %%mm7              \n\t"\
        "1:                             \n\t"\
        "movq (%0), %%mm0               \n\t"\
        "movq (%0), %%mm1               \n\t"\
        "punpcklbw %%mm7, %%mm0         \n\t"\
        "punpckhbw %%mm7, %%mm1         \n\t"\
        "movq %%mm0, (%1)               \n\t"\
        "movq %%mm1, 9*8(%1)            \n\t"\
        "add $8, %1                     \n\t"\
        "add %3, %0                     \n\t"\
        "decl %2                        \n\t"\
        " jnz 1b                        \n\t"\
        : "+r" (src), "+r" (temp_ptr), "+r"(count)\
        : "r" ((x86_reg)srcStride)\
        : "memory"\
    );\
    \
    temp_ptr= temp;\
    count=2;\
    \
/*FIXME reorder for speed */\
    asm volatile(\
        /*"pxor %%mm7, %%mm7              \n\t"*/\
        "1:                             \n\t"\
        "movq (%0), %%mm0               \n\t"\
        "movq 8(%0), %%mm1              \n\t"\
        "movq 16(%0), %%mm2             \n\t"\
        "movq 24(%0), %%mm3             \n\t"\
        QPEL_V_LOW(%%mm0, %%mm1, %%mm2, %%mm3, %5, %6, %5, 16(%0),  8(%0),   (%0), 32(%0), (%1), OP)\
        QPEL_V_LOW(%%mm1, %%mm2, %%mm3, %%mm0, %5, %6, %5,  8(%0),   (%0),   (%0), 40(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm2, %%mm3, %%mm0, %%mm1, %5, %6, %5,   (%0),   (%0),  8(%0), 48(%0), (%1), OP)\
        \
        QPEL_V_LOW(%%mm3, %%mm0, %%mm1, %%mm2, %5, %6, %5,   (%0),  8(%0), 16(%0), 56(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm0, %%mm1, %%mm2, %%mm3, %5, %6, %5,  8(%0), 16(%0), 24(%0), 64(%0), (%1), OP)\
        \
        QPEL_V_LOW(%%mm1, %%mm2, %%mm3, %%mm0, %5, %6, %5, 16(%0), 24(%0), 32(%0), 64(%0), (%1, %3), OP)\
        "add %4, %1                     \n\t"\
        QPEL_V_LOW(%%mm2, %%mm3, %%mm0, %%mm1, %5, %6, %5, 24(%0), 32(%0), 40(%0), 56(%0), (%1), OP)\
        QPEL_V_LOW(%%mm3, %%mm0, %%mm1, %%mm2, %5, %6, %5, 32(%0), 40(%0), 48(%0), 48(%0), (%1, %3), OP)\
                \
        "add $72, %0                    \n\t"\
        "add %6, %1                     \n\t"\
        "decl %2                        \n\t"\
        " jnz 1b                        \n\t"\
         \
        : "+r"(temp_ptr), "+r"(dst), "+g"(count)\
        : "r"((x86_reg)dstStride), "r"(2*(x86_reg)dstStride), /*"m"(ff_pw_20), "m"(ff_pw_3),*/ "m"(ROUNDER), "g"(4-6*(x86_reg)dstStride)\
        : "memory"\
   );\
}\
\
static void OPNAME ## qpel8_mc00_ ## MMX (uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## pixels8_ ## MMX(dst, src, stride, 8);\
}\
\
static void OPNAME ## qpel8_mc10_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t temp[8];\
    uint8_t * const half= (uint8_t*)temp;\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(half, src, 8, stride, 8);\
    OPNAME ## pixels8_l2_ ## MMX(dst, src, half, stride, stride, 8);\
}\
\
static void OPNAME ## qpel8_mc20_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## mpeg4_qpel8_h_lowpass_ ## MMX(dst, src, stride, stride, 8);\
}\
\
static void OPNAME ## qpel8_mc30_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t temp[8];\
    uint8_t * const half= (uint8_t*)temp;\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(half, src, 8, stride, 8);\
    OPNAME ## pixels8_l2_ ## MMX(dst, src+1, half, stride, stride, 8);\
}\
\
static void OPNAME ## qpel8_mc01_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t temp[8];\
    uint8_t * const half= (uint8_t*)temp;\
    put ## RND ## mpeg4_qpel8_v_lowpass_ ## MMX(half, src, 8, stride);\
    OPNAME ## pixels8_l2_ ## MMX(dst, src, half, stride, stride, 8);\
}\
\
static void OPNAME ## qpel8_mc02_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## mpeg4_qpel8_v_lowpass_ ## MMX(dst, src, stride, stride);\
}\
\
static void OPNAME ## qpel8_mc03_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t temp[8];\
    uint8_t * const half= (uint8_t*)temp;\
    put ## RND ## mpeg4_qpel8_v_lowpass_ ## MMX(half, src, 8, stride);\
    OPNAME ## pixels8_l2_ ## MMX(dst, src+stride, half, stride, stride, 8);\
}\
static void OPNAME ## qpel8_mc11_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[8 + 9];\
    uint8_t * const halfH= ((uint8_t*)half) + 64;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    put ## RND ## pixels8_l2_ ## MMX(halfH, src, halfH, 8, stride, 9);\
    put ## RND ## mpeg4_qpel8_v_lowpass_ ## MMX(halfHV, halfH, 8, 8);\
    OPNAME ## pixels8_l2_ ## MMX(dst, halfH, halfHV, stride, 8, 8);\
}\
static void OPNAME ## qpel8_mc31_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[8 + 9];\
    uint8_t * const halfH= ((uint8_t*)half) + 64;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    put ## RND ## pixels8_l2_ ## MMX(halfH, src+1, halfH, 8, stride, 9);\
    put ## RND ## mpeg4_qpel8_v_lowpass_ ## MMX(halfHV, halfH, 8, 8);\
    OPNAME ## pixels8_l2_ ## MMX(dst, halfH, halfHV, stride, 8, 8);\
}\
static void OPNAME ## qpel8_mc13_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[8 + 9];\
    uint8_t * const halfH= ((uint8_t*)half) + 64;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    put ## RND ## pixels8_l2_ ## MMX(halfH, src, halfH, 8, stride, 9);\
    put ## RND ## mpeg4_qpel8_v_lowpass_ ## MMX(halfHV, halfH, 8, 8);\
    OPNAME ## pixels8_l2_ ## MMX(dst, halfH+8, halfHV, stride, 8, 8);\
}\
static void OPNAME ## qpel8_mc33_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[8 + 9];\
    uint8_t * const halfH= ((uint8_t*)half) + 64;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    put ## RND ## pixels8_l2_ ## MMX(halfH, src+1, halfH, 8, stride, 9);\
    put ## RND ## mpeg4_qpel8_v_lowpass_ ## MMX(halfHV, halfH, 8, 8);\
    OPNAME ## pixels8_l2_ ## MMX(dst, halfH+8, halfHV, stride, 8, 8);\
}\
static void OPNAME ## qpel8_mc21_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[8 + 9];\
    uint8_t * const halfH= ((uint8_t*)half) + 64;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    put ## RND ## mpeg4_qpel8_v_lowpass_ ## MMX(halfHV, halfH, 8, 8);\
    OPNAME ## pixels8_l2_ ## MMX(dst, halfH, halfHV, stride, 8, 8);\
}\
static void OPNAME ## qpel8_mc23_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[8 + 9];\
    uint8_t * const halfH= ((uint8_t*)half) + 64;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    put ## RND ## mpeg4_qpel8_v_lowpass_ ## MMX(halfHV, halfH, 8, 8);\
    OPNAME ## pixels8_l2_ ## MMX(dst, halfH+8, halfHV, stride, 8, 8);\
}\
static void OPNAME ## qpel8_mc12_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[8 + 9];\
    uint8_t * const halfH= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    put ## RND ## pixels8_l2_ ## MMX(halfH, src, halfH, 8, stride, 9);\
    OPNAME ## mpeg4_qpel8_v_lowpass_ ## MMX(dst, halfH, stride, 8);\
}\
static void OPNAME ## qpel8_mc32_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[8 + 9];\
    uint8_t * const halfH= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    put ## RND ## pixels8_l2_ ## MMX(halfH, src+1, halfH, 8, stride, 9);\
    OPNAME ## mpeg4_qpel8_v_lowpass_ ## MMX(dst, halfH, stride, 8);\
}\
static void OPNAME ## qpel8_mc22_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[9];\
    uint8_t * const halfH= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel8_h_lowpass_ ## MMX(halfH, src, 8, stride, 9);\
    OPNAME ## mpeg4_qpel8_v_lowpass_ ## MMX(dst, halfH, stride, 8);\
}\
static void OPNAME ## qpel16_mc00_ ## MMX (uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## pixels16_ ## MMX(dst, src, stride, 16);\
}\
\
static void OPNAME ## qpel16_mc10_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t temp[32];\
    uint8_t * const half= (uint8_t*)temp;\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(half, src, 16, stride, 16);\
    OPNAME ## pixels16_l2_ ## MMX(dst, src, half, stride, stride, 16);\
}\
\
static void OPNAME ## qpel16_mc20_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## mpeg4_qpel16_h_lowpass_ ## MMX(dst, src, stride, stride, 16);\
}\
\
static void OPNAME ## qpel16_mc30_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t temp[32];\
    uint8_t * const half= (uint8_t*)temp;\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(half, src, 16, stride, 16);\
    OPNAME ## pixels16_l2_ ## MMX(dst, src+1, half, stride, stride, 16);\
}\
\
static void OPNAME ## qpel16_mc01_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t temp[32];\
    uint8_t * const half= (uint8_t*)temp;\
    put ## RND ## mpeg4_qpel16_v_lowpass_ ## MMX(half, src, 16, stride);\
    OPNAME ## pixels16_l2_ ## MMX(dst, src, half, stride, stride, 16);\
}\
\
static void OPNAME ## qpel16_mc02_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## mpeg4_qpel16_v_lowpass_ ## MMX(dst, src, stride, stride);\
}\
\
static void OPNAME ## qpel16_mc03_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t temp[32];\
    uint8_t * const half= (uint8_t*)temp;\
    put ## RND ## mpeg4_qpel16_v_lowpass_ ## MMX(half, src, 16, stride);\
    OPNAME ## pixels16_l2_ ## MMX(dst, src+stride, half, stride, stride, 16);\
}\
static void OPNAME ## qpel16_mc11_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[16*2 + 17*2];\
    uint8_t * const halfH= ((uint8_t*)half) + 256;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    put ## RND ## pixels16_l2_ ## MMX(halfH, src, halfH, 16, stride, 17);\
    put ## RND ## mpeg4_qpel16_v_lowpass_ ## MMX(halfHV, halfH, 16, 16);\
    OPNAME ## pixels16_l2_ ## MMX(dst, halfH, halfHV, stride, 16, 16);\
}\
static void OPNAME ## qpel16_mc31_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[16*2 + 17*2];\
    uint8_t * const halfH= ((uint8_t*)half) + 256;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    put ## RND ## pixels16_l2_ ## MMX(halfH, src+1, halfH, 16, stride, 17);\
    put ## RND ## mpeg4_qpel16_v_lowpass_ ## MMX(halfHV, halfH, 16, 16);\
    OPNAME ## pixels16_l2_ ## MMX(dst, halfH, halfHV, stride, 16, 16);\
}\
static void OPNAME ## qpel16_mc13_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[16*2 + 17*2];\
    uint8_t * const halfH= ((uint8_t*)half) + 256;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    put ## RND ## pixels16_l2_ ## MMX(halfH, src, halfH, 16, stride, 17);\
    put ## RND ## mpeg4_qpel16_v_lowpass_ ## MMX(halfHV, halfH, 16, 16);\
    OPNAME ## pixels16_l2_ ## MMX(dst, halfH+16, halfHV, stride, 16, 16);\
}\
static void OPNAME ## qpel16_mc33_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[16*2 + 17*2];\
    uint8_t * const halfH= ((uint8_t*)half) + 256;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    put ## RND ## pixels16_l2_ ## MMX(halfH, src+1, halfH, 16, stride, 17);\
    put ## RND ## mpeg4_qpel16_v_lowpass_ ## MMX(halfHV, halfH, 16, 16);\
    OPNAME ## pixels16_l2_ ## MMX(dst, halfH+16, halfHV, stride, 16, 16);\
}\
static void OPNAME ## qpel16_mc21_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[16*2 + 17*2];\
    uint8_t * const halfH= ((uint8_t*)half) + 256;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    put ## RND ## mpeg4_qpel16_v_lowpass_ ## MMX(halfHV, halfH, 16, 16);\
    OPNAME ## pixels16_l2_ ## MMX(dst, halfH, halfHV, stride, 16, 16);\
}\
static void OPNAME ## qpel16_mc23_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[16*2 + 17*2];\
    uint8_t * const halfH= ((uint8_t*)half) + 256;\
    uint8_t * const halfHV= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    put ## RND ## mpeg4_qpel16_v_lowpass_ ## MMX(halfHV, halfH, 16, 16);\
    OPNAME ## pixels16_l2_ ## MMX(dst, halfH+16, halfHV, stride, 16, 16);\
}\
static void OPNAME ## qpel16_mc12_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[17*2];\
    uint8_t * const halfH= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    put ## RND ## pixels16_l2_ ## MMX(halfH, src, halfH, 16, stride, 17);\
    OPNAME ## mpeg4_qpel16_v_lowpass_ ## MMX(dst, halfH, stride, 16);\
}\
static void OPNAME ## qpel16_mc32_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[17*2];\
    uint8_t * const halfH= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    put ## RND ## pixels16_l2_ ## MMX(halfH, src+1, halfH, 16, stride, 17);\
    OPNAME ## mpeg4_qpel16_v_lowpass_ ## MMX(dst, halfH, stride, 16);\
}\
static void OPNAME ## qpel16_mc22_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    uint64_t half[17*2];\
    uint8_t * const halfH= ((uint8_t*)half);\
    put ## RND ## mpeg4_qpel16_h_lowpass_ ## MMX(halfH, src, 16, stride, 17);\
    OPNAME ## mpeg4_qpel16_v_lowpass_ ## MMX(dst, halfH, stride, 16);\
}

#define PUT_OP(a,b,temp, size) "mov" #size " " #a ", " #b "        \n\t"
#define AVG_3DNOW_OP(a,b,temp, size) \
"mov" #size " " #b ", " #temp "   \n\t"\
"pavgusb " #temp ", " #a "        \n\t"\
"mov" #size " " #a ", " #b "      \n\t"
#define AVG_MMX2_OP(a,b,temp, size) \
"mov" #size " " #b ", " #temp "   \n\t"\
"pavgb " #temp ", " #a "          \n\t"\
"mov" #size " " #a ", " #b "      \n\t"

QPEL_BASE(put_       , ff_pw_16, _       , PUT_OP, PUT_OP)
QPEL_BASE(avg_       , ff_pw_16, _       , AVG_MMX2_OP, AVG_3DNOW_OP)
QPEL_BASE(put_no_rnd_, ff_pw_15, _no_rnd_, PUT_OP, PUT_OP)
QPEL_OP(put_       , ff_pw_16, _       , PUT_OP, 3dnow)
QPEL_OP(avg_       , ff_pw_16, _       , AVG_3DNOW_OP, 3dnow)
QPEL_OP(put_no_rnd_, ff_pw_15, _no_rnd_, PUT_OP, 3dnow)
QPEL_OP(put_       , ff_pw_16, _       , PUT_OP, mmx2)
QPEL_OP(avg_       , ff_pw_16, _       , AVG_MMX2_OP, mmx2)
QPEL_OP(put_no_rnd_, ff_pw_15, _no_rnd_, PUT_OP, mmx2)

/***********************************/
/* bilinear qpel: not compliant to any spec, only for -lavdopts fast */

#define QPEL_2TAP_XY(OPNAME, SIZE, MMX, XY, HPEL)\
static void OPNAME ## 2tap_qpel ## SIZE ## _mc ## XY ## _ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## pixels ## SIZE ## HPEL(dst, src, stride, SIZE);\
}
#define QPEL_2TAP_L3(OPNAME, SIZE, MMX, XY, S0, S1, S2)\
static void OPNAME ## 2tap_qpel ## SIZE ## _mc ## XY ## _ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## 2tap_qpel ## SIZE ## _l3_ ## MMX(dst, src+S0, stride, SIZE, S1, S2);\
}

#define QPEL_2TAP(OPNAME, SIZE, MMX)\
QPEL_2TAP_XY(OPNAME, SIZE, MMX, 20, _x2_ ## MMX)\
QPEL_2TAP_XY(OPNAME, SIZE, MMX, 02, _y2_ ## MMX)\
QPEL_2TAP_XY(OPNAME, SIZE, MMX, 22, _xy2_mmx)\
static const qpel_mc_func OPNAME ## 2tap_qpel ## SIZE ## _mc00_ ## MMX =\
                          OPNAME ## qpel ## SIZE ## _mc00_ ## MMX;\
static const qpel_mc_func OPNAME ## 2tap_qpel ## SIZE ## _mc21_ ## MMX =\
                          OPNAME ## 2tap_qpel ## SIZE ## _mc20_ ## MMX;\
static const qpel_mc_func OPNAME ## 2tap_qpel ## SIZE ## _mc12_ ## MMX =\
                          OPNAME ## 2tap_qpel ## SIZE ## _mc02_ ## MMX;\
static void OPNAME ## 2tap_qpel ## SIZE ## _mc32_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## pixels ## SIZE ## _y2_ ## MMX(dst, src+1, stride, SIZE);\
}\
static void OPNAME ## 2tap_qpel ## SIZE ## _mc23_ ## MMX(uint8_t *dst, uint8_t *src, int stride){\
    OPNAME ## pixels ## SIZE ## _x2_ ## MMX(dst, src+stride, stride, SIZE);\
}\
QPEL_2TAP_L3(OPNAME, SIZE, MMX, 10, 0,         1,       0)\
QPEL_2TAP_L3(OPNAME, SIZE, MMX, 30, 1,        -1,       0)\
QPEL_2TAP_L3(OPNAME, SIZE, MMX, 01, 0,         stride,  0)\
QPEL_2TAP_L3(OPNAME, SIZE, MMX, 03, stride,   -stride,  0)\
QPEL_2TAP_L3(OPNAME, SIZE, MMX, 11, 0,         stride,  1)\
QPEL_2TAP_L3(OPNAME, SIZE, MMX, 31, 1,         stride, -1)\
QPEL_2TAP_L3(OPNAME, SIZE, MMX, 13, stride,   -stride,  1)\
QPEL_2TAP_L3(OPNAME, SIZE, MMX, 33, stride+1, -stride, -1)\

QPEL_2TAP(put_, 16, mmx2)
QPEL_2TAP(avg_, 16, mmx2)
QPEL_2TAP(put_,  8, mmx2)
QPEL_2TAP(avg_,  8, mmx2)
QPEL_2TAP(put_, 16, 3dnow)
QPEL_2TAP(avg_, 16, 3dnow)
QPEL_2TAP(put_,  8, 3dnow)
QPEL_2TAP(avg_,  8, 3dnow)


#if 0
static void just_return() { return; }
#endif

#define PREFETCH(name, op) \
static void name(void *mem, int stride, int h){\
    const uint8_t *p= mem;\
    do{\
        asm volatile(#op" %0" :: "m"(*p));\
        p+= stride;\
    }while(--h);\
}
PREFETCH(prefetch_mmx2,  prefetcht0)
PREFETCH(prefetch_3dnow, prefetch)
#undef PREFETCH

#include "h264dsp_mmx.c"


/* external functions, from idct_mmx.c */
void ff_mmx_idct(DCTELEM *block);
void ff_mmxext_idct(DCTELEM *block);

void dsputil_init_mmx(DSPContext* c, AVCodecContext *avctx)
{
    mm_flags = mm_support();

   
    if (avctx->dsp_mask) {
        if (avctx->dsp_mask & FF_MM_FORCE)
            mm_flags |= (avctx->dsp_mask & 0xffff);
        else
            mm_flags &= ~(avctx->dsp_mask & 0xffff);
    }

#if 1
    av_log(avctx, AV_LOG_INFO, "libavcodec: CPU flags:");
    if (mm_flags & MM_MMX)
        av_log(avctx, AV_LOG_INFO, " mmx");
    if (mm_flags & MM_MMXEXT)
        av_log(avctx, AV_LOG_INFO, " mmxext");
    if (mm_flags & MM_3DNOW)
        av_log(avctx, AV_LOG_INFO, " 3dnow");
    if (mm_flags & MM_SSE)
        av_log(avctx, AV_LOG_INFO, " sse");
    if (mm_flags & MM_SSE2)
        av_log(avctx, AV_LOG_INFO, " sse2");
    av_log(avctx, AV_LOG_INFO, "\n");
#endif

    if (mm_flags & MM_MMX) {
        c->clear_blocks = clear_blocks_mmx;

        c->draw_edges = draw_edges_mmx;

        c->put_h264_chroma_pixels_tab[0]= put_h264_chroma_mc8_mmx_rnd;
        c->put_h264_chroma_pixels_tab[1]= put_h264_chroma_mc4_mmx;
       
        c->h264_idct_dc_add=
        c->h264_idct_add= ff_h264_idct_add_mmx;
        c->h264_idct8_dc_add=
        c->h264_idct8_add= ff_h264_idct8_add_mmx;
        if (mm_flags & MM_SSE2)
            c->h264_idct8_add= ff_h264_idct8_add_sse2;

        if (mm_flags & MM_MMXEXT) {
            c->prefetch = prefetch_mmx2;

            c->h264_idct_dc_add= ff_h264_idct_dc_add_mmx2;
            c->h264_idct8_dc_add= ff_h264_idct8_dc_add_mmx2;


#define SET_QPEL_FUNCS(PFX, IDX, SIZE, CPU) \
            c->PFX ## _pixels_tab[IDX][ 0] = PFX ## SIZE ## _mc00_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 1] = PFX ## SIZE ## _mc10_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 2] = PFX ## SIZE ## _mc20_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 3] = PFX ## SIZE ## _mc30_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 4] = PFX ## SIZE ## _mc01_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 5] = PFX ## SIZE ## _mc11_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 6] = PFX ## SIZE ## _mc21_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 7] = PFX ## SIZE ## _mc31_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 8] = PFX ## SIZE ## _mc02_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][ 9] = PFX ## SIZE ## _mc12_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][10] = PFX ## SIZE ## _mc22_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][11] = PFX ## SIZE ## _mc32_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][12] = PFX ## SIZE ## _mc03_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][13] = PFX ## SIZE ## _mc13_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][14] = PFX ## SIZE ## _mc23_ ## CPU; \
            c->PFX ## _pixels_tab[IDX][15] = PFX ## SIZE ## _mc33_ ## CPU

            SET_QPEL_FUNCS(put_h264_qpel, 0, 16, mmx2);
            SET_QPEL_FUNCS(put_h264_qpel, 1, 8, mmx2);
            SET_QPEL_FUNCS(put_h264_qpel, 2, 4, mmx2);
            SET_QPEL_FUNCS(avg_h264_qpel, 0, 16, mmx2);
            SET_QPEL_FUNCS(avg_h264_qpel, 1, 8, mmx2);
            SET_QPEL_FUNCS(avg_h264_qpel, 2, 4, mmx2);


            c->avg_h264_chroma_pixels_tab[0]= avg_h264_chroma_mc8_mmx2_rnd;
            c->avg_h264_chroma_pixels_tab[1]= avg_h264_chroma_mc4_mmx2;
            c->avg_h264_chroma_pixels_tab[2]= avg_h264_chroma_mc2_mmx2;
            c->put_h264_chroma_pixels_tab[2]= put_h264_chroma_mc2_mmx2;
            c->h264_v_loop_filter_luma= h264_v_loop_filter_luma_mmx2;
            c->h264_h_loop_filter_luma= h264_h_loop_filter_luma_mmx2;
            c->h264_v_loop_filter_chroma= h264_v_loop_filter_chroma_mmx2;
            c->h264_h_loop_filter_chroma= h264_h_loop_filter_chroma_mmx2;
            c->h264_v_loop_filter_chroma_intra= h264_v_loop_filter_chroma_intra_mmx2;
            c->h264_h_loop_filter_chroma_intra= h264_h_loop_filter_chroma_intra_mmx2;
            c->h264_loop_filter_strength= h264_loop_filter_strength_mmx2;

            c->weight_h264_pixels_tab[0]= ff_h264_weight_16x16_mmx2;
            c->weight_h264_pixels_tab[1]= ff_h264_weight_16x8_mmx2;
            c->weight_h264_pixels_tab[2]= ff_h264_weight_8x16_mmx2;
            c->weight_h264_pixels_tab[3]= ff_h264_weight_8x8_mmx2;
            c->weight_h264_pixels_tab[4]= ff_h264_weight_8x4_mmx2;
            c->weight_h264_pixels_tab[5]= ff_h264_weight_4x8_mmx2;
            c->weight_h264_pixels_tab[6]= ff_h264_weight_4x4_mmx2;
            c->weight_h264_pixels_tab[7]= ff_h264_weight_4x2_mmx2;

            c->biweight_h264_pixels_tab[0]= ff_h264_biweight_16x16_mmx2;
            c->biweight_h264_pixels_tab[1]= ff_h264_biweight_16x8_mmx2;
            c->biweight_h264_pixels_tab[2]= ff_h264_biweight_8x16_mmx2;
            c->biweight_h264_pixels_tab[3]= ff_h264_biweight_8x8_mmx2;
            c->biweight_h264_pixels_tab[4]= ff_h264_biweight_8x4_mmx2;
            c->biweight_h264_pixels_tab[5]= ff_h264_biweight_4x8_mmx2;
            c->biweight_h264_pixels_tab[6]= ff_h264_biweight_4x4_mmx2;
            c->biweight_h264_pixels_tab[7]= ff_h264_biweight_4x2_mmx2;
        } else if (mm_flags & MM_3DNOW) {
            c->prefetch = prefetch_3dnow;

            SET_QPEL_FUNCS(put_h264_qpel, 0, 16, 3dnow);
            SET_QPEL_FUNCS(put_h264_qpel, 1, 8, 3dnow);
            SET_QPEL_FUNCS(put_h264_qpel, 2, 4, 3dnow);
            SET_QPEL_FUNCS(avg_h264_qpel, 0, 16, 3dnow);
            SET_QPEL_FUNCS(avg_h264_qpel, 1, 8, 3dnow);
            SET_QPEL_FUNCS(avg_h264_qpel, 2, 4, 3dnow);

            c->avg_h264_chroma_pixels_tab[0]= avg_h264_chroma_mc8_3dnow_rnd;
            c->avg_h264_chroma_pixels_tab[1]= avg_h264_chroma_mc4_3dnow;
        }


#define H264_QPEL_FUNCS(x, y, CPU)\
            c->put_h264_qpel_pixels_tab[0][x+y*4] = put_h264_qpel16_mc##x##y##_##CPU;\
            c->put_h264_qpel_pixels_tab[1][x+y*4] = put_h264_qpel8_mc##x##y##_##CPU;\
            c->avg_h264_qpel_pixels_tab[0][x+y*4] = avg_h264_qpel16_mc##x##y##_##CPU;\
            c->avg_h264_qpel_pixels_tab[1][x+y*4] = avg_h264_qpel8_mc##x##y##_##CPU;
        if((mm_flags & MM_SSE2) && !(mm_flags & MM_3DNOW)){
            
            H264_QPEL_FUNCS(0, 0, sse2);
        }
        if(mm_flags & MM_SSE2){
            H264_QPEL_FUNCS(0, 1, sse2);
            H264_QPEL_FUNCS(0, 2, sse2);
            H264_QPEL_FUNCS(0, 3, sse2);
            H264_QPEL_FUNCS(1, 1, sse2);
            H264_QPEL_FUNCS(1, 2, sse2);
            H264_QPEL_FUNCS(1, 3, sse2);
            H264_QPEL_FUNCS(2, 1, sse2);
            H264_QPEL_FUNCS(2, 2, sse2);
            H264_QPEL_FUNCS(2, 3, sse2);
            H264_QPEL_FUNCS(3, 1, sse2);
            H264_QPEL_FUNCS(3, 2, sse2);
            H264_QPEL_FUNCS(3, 3, sse2);
        }
#ifdef HAVE_SSSE3
        if(mm_flags & MM_SSSE3){
            H264_QPEL_FUNCS(1, 0, ssse3);
            H264_QPEL_FUNCS(1, 1, ssse3);
            H264_QPEL_FUNCS(1, 2, ssse3);
            H264_QPEL_FUNCS(1, 3, ssse3);
            H264_QPEL_FUNCS(2, 0, ssse3);
            H264_QPEL_FUNCS(2, 1, ssse3);
            H264_QPEL_FUNCS(2, 2, ssse3);
            H264_QPEL_FUNCS(2, 3, ssse3);
            H264_QPEL_FUNCS(3, 0, ssse3);
            H264_QPEL_FUNCS(3, 1, ssse3);
            H264_QPEL_FUNCS(3, 2, ssse3);
            H264_QPEL_FUNCS(3, 3, ssse3);
            c->put_h264_chroma_pixels_tab[0]= put_h264_chroma_mc8_ssse3_rnd;
            c->avg_h264_chroma_pixels_tab[0]= avg_h264_chroma_mc8_ssse3_rnd;
            c->put_h264_chroma_pixels_tab[1]= put_h264_chroma_mc4_ssse3;
            c->avg_h264_chroma_pixels_tab[1]= avg_h264_chroma_mc4_ssse3;
        }
#endif

    }
}

