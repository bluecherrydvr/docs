/*
 * Generic DCT based hybrid video encoder
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard.
 * Copyright (c) 2002-2004 Michael Niedermayer
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
 * @file mpegvideo.h
 * mpegvideo header.
 */

#ifndef __MPEGVIDEO_H__
#define __MPEGVIDEO_H__

#include "dsputil.h"
#include "bitstream.h"
#include "parser.h"


#define MAX_PICTURE_COUNT 32

/**
 * Picture.
 */
typedef struct Picture{
    FF_COMMON_FRAME

    /**
     * halfpel luma planes.
     */
    int16_t (*motion_val_base[2])[2];
    uint32_t *mb_type_base;
#define MB_TYPE_INTRA MB_TYPE_INTRA4x4 //default mb_type if there is just one type
#define IS_INTRA4x4(a)   ((a)&MB_TYPE_INTRA4x4)
#define IS_INTRA16x16(a) ((a)&MB_TYPE_INTRA16x16)
#define IS_PCM(a)        ((a)&MB_TYPE_INTRA_PCM)
#define IS_INTRA(a)      ((a)&7)
#define IS_INTER(a)      ((a)&(MB_TYPE_16x16|MB_TYPE_16x8|MB_TYPE_8x16|MB_TYPE_8x8))
#define IS_SKIP(a)       ((a)&MB_TYPE_SKIP)
#define IS_INTRA_PCM(a)  ((a)&MB_TYPE_INTRA_PCM)
#define IS_INTERLACED(a) ((a)&MB_TYPE_INTERLACED)
#define IS_DIRECT(a)     ((a)&MB_TYPE_DIRECT)
#define IS_GMC(a)        ((a)&MB_TYPE_GMC)
#define IS_16X16(a)      ((a)&MB_TYPE_16x16)
#define IS_16X8(a)       ((a)&MB_TYPE_16x8)
#define IS_8X16(a)       ((a)&MB_TYPE_8x16)
#define IS_8X8(a)        ((a)&MB_TYPE_8x8)
#define IS_SUB_8X8(a)    ((a)&MB_TYPE_16x16) //note reused
#define IS_SUB_8X4(a)    ((a)&MB_TYPE_16x8)  //note reused
#define IS_SUB_4X8(a)    ((a)&MB_TYPE_8x16)  //note reused
#define IS_SUB_4X4(a)    ((a)&MB_TYPE_8x8)   //note reused
#define IS_ACPRED(a)     ((a)&MB_TYPE_ACPRED)
#define IS_DIR(a, part, list) ((a) & (MB_TYPE_P0L0<<((part)+2*(list))))
#define USES_LIST(a, list) ((a) & ((MB_TYPE_P0L0|MB_TYPE_P1L0)<<(2*(list)))) ///< does this mb use listX, note does not work if subMBs


    int field_poc[2];           ///< h264 top/bottom POC
    int poc;                    ///< h264 frame POC
    int frame_num;              ///< h264 frame_num (raw frame_num from slice header)
    int pic_id;                 /**< h264 pic_num (short -> no wrap version of pic_num,
                                     pic_num & max_pic_num; long -> long_pic_num) */
    int long_ref;               ///< 1->long term reference 0->short term reference
    int ref_poc[2][2][16];      ///< h264 POCs of the frames used as reference (FIXME need per slice)
    int ref_count[2][2];        ///< number of entries in ref_poc              (FIXME need per slice)
    int mbaff;                  ///< h264 1 -> MBAFF frame 0-> not MBAFF

} Picture;

/**
 * MpegEncContext.
 */
typedef struct MpegEncContext {
    struct AVCodecContext *avctx;
    /* the following parameters must be initialized before encoding */
    int width, height;///< picture size. must be a multiple of 16
   

    enum CodecID codec_id;     /* see CODEC_ID_xxx */
    /* the following fields are managed internally by the encoder */

    /** bit output */
    PutBitContext pb;

    /* sequence parameters */
    int context_initialized;
    int coded_picture_number;  ///< used to set pic->coded_picture_number, should not be used for/by anything else
    int mb_width, mb_height;   ///< number of MBs horizontally & vertically
    int mb_stride;             ///< mb_width+1 used for some arrays to allow simple addressing of left & top MBs without sig11
    int b8_stride;             ///< 2*mb_width+1 used for some 8x8 block arrays to allow simple addressing
    int b4_stride;             ///< 4*mb_width+1 used for some 4x4 block arrays to allow simple addressing
    int h_edge_pos, v_edge_pos;///< horizontal / vertical position of the right/bottom edge (pixel replication)
    int mb_num;                ///< number of MBs of a picture
    int linesize;              ///< line size, in bytes, may be different from width
    int uvlinesize;            ///< line size, for chroma in bytes, may be different from width
    Picture *picture;          ///< main picture buffer

    /**
     * copy of the previous picture structure.
     * note, linesize & data, might not match the previous picture (for field pictures)
     */
    Picture last_picture;

    /**
     * copy of the next picture structure.
     * note, linesize & data, might not match the next picture (for field pictures)
     */
    Picture next_picture;

    /**
     * copy of the current picture structure.
     * note, linesize & data, might not match the current picture (for field pictures)
     */
    Picture current_picture;    ///< buffer to store the decompressed current picture

    Picture *last_picture_ptr;     ///< pointer to the previous picture.
    Picture *next_picture_ptr;     ///< pointer to the next picture (for bidir pred)
    Picture *current_picture_ptr;  ///< pointer to the current picture
    int16_t *dc_val_base;
    int16_t *dc_val[3];            ///< used for mpeg4 DC prediction, all 3 arrays must be continuous
    
    uint8_t *mbskip_table;        /**< used to avoid copy if macroblock skipped (for black regions for example)
                                   and used for b-frame encoding & decoding (contains skip table of next P Frame) */
    uint8_t *mbintra_table;       ///< used to avoid setting {ac, dc, cbp}-pred stuff to zero on inter MB decoding
    uint8_t *allocated_edge_emu_buffer;
    uint8_t *edge_emu_buffer;     ///< points into the middle of allocated_edge_emu_buffer
    uint8_t *obmc_scratchpad;
   
    int qscale;                 ///< QP
    int chroma_qscale;          ///< chroma QP
    int pict_type;              ///< FF_I_TYPE, FF_P_TYPE, FF_B_TYPE, ...
    int dropable;
 
    /* motion compensation */
    int unrestricted_mv;        ///< mv can point outside of the coded picture

    DSPContext dsp;             ///< pointers for accelerated dsp functions
    int mv_dir;
#define MV_DIR_FORWARD   1
#define MV_DIR_BACKWARD  2
#define MV_DIRECT        4 ///< bidirectional mode where the difference equals the MV of the last P/S/I-Frame (mpeg4)
    int mv_type;
#define MV_TYPE_16X16       0   ///< 1 vector for the whole mb
#define MV_TYPE_8X8         1   ///< 4 vectors (h263, mpeg4 4MV)
#define MV_TYPE_16X8        2   ///< 2 vectors, one per 16x8 block
#define MV_TYPE_FIELD       3   ///< 2 vectors, one per field
#define MV_TYPE_DMV         4   ///< 2 vectors, special mpeg2 Dual Prime Vectors
    /**motion vectors for a macroblock
       first coordinate : 0 = forward 1 = backward
       second "         : depend on type
       third  "         : 0 = x, 1 = y
    */
    int mv[2][4][2];
 

    int hurry_up;     /**< when set to 1 during decoding, b frames will be skipped
                         when set to 2 idct/dequant will be skipped too */

    /* macroblock layer */
    int mb_x, mb_y;
    int mb_skip_run;
    int mb_intra;

    int block_index[6]; ///< index to current MB in block based arrays with edges
    uint8_t *dest[3];

    int *mb_index2xy;        ///< mb_index -> mb_x + mb_y*mb_stride

    /* error concealment / resync */
    int error_count;
    uint8_t *error_status_table;       ///< table of the error status of each MB
#define VP_START            1          ///< current MB is the first after a resync marker
#define AC_ERROR            2
#define DC_ERROR            4
#define MV_ERROR            8
#define AC_END              16
#define DC_END              32
#define MV_END              64
//FIXME some prefix?

    int resync_mb_x;                 ///< x position of last resync marker
    int resync_mb_y;                 ///< y position of last resync marker
    int mb_num_left;                 ///< number of MBs left in this video packet (for partitioned Slices only)
    int error_recognition;

    ParseContext parse_context;
    /* mpeg4 specific */
    int64_t time;                   ///< time of current frame
    uint16_t pp_time;               ///< time distance between the last 2 p,s,i frames
    uint16_t pb_time;               ///< time distance between the last b and p,s,i frame
    int data_partitioning;           ///< data partitioning flag from header
    int partitioned_frame;           ///< is current frame partitioned
    int low_delay;                   ///< no reordering needed / has no b-frames
  

    /* decompression specific */
    GetBitContext gb;

    int picture_structure;
/* picture type */
#define PICT_TOP_FIELD     1
#define PICT_BOTTOM_FIELD  2
#define PICT_FRAME         3

    int top_field_first;
    int alternate_scan;
#define CHROMA_420 1
#define CHROMA_422 2
#define CHROMA_444 3
    int chroma_x_shift;//depend on pix_format, that depend on chroma_format
    int chroma_y_shift;

    int interlaced_dct;
    int first_slice;
    int first_field;         ///< is 1 for the first field of a field picture 0 otherwise

    DCTELEM (*block)[64]; ///< points to one of the following blocks
    DCTELEM (*blocks)[8][64]; // for HQ mode we need to keep the best block
    int (*decode_mb)(struct MpegEncContext *s, DCTELEM block[6][64]); // used by some codecs to avoid a switch()

} MpegEncContext;


int MPV_common_init(MpegEncContext *s);
void MPV_common_end(MpegEncContext *s);
void MPV_decode_mb(MpegEncContext *s, DCTELEM block[12][64]);
int MPV_frame_start(MpegEncContext *s, AVCodecContext *avctx);
void MPV_frame_end(MpegEncContext *s);
void ff_mpeg_flush(AVCodecContext *avctx);
int ff_find_unused_picture(MpegEncContext *s, int shared);

void ff_er_frame_start(MpegEncContext *s);
void ff_er_frame_end(MpegEncContext *s);
void ff_er_add_slice(MpegEncContext *s, int startx, int starty, int endx, int endy, int status);

#endif /* __MPEGVIDEO_H__ */

